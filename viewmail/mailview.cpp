#include "mailview.h"
#include "mailnetworkmanager.h"
#include "htmlmail.h"

#include <QtGui>
#include <QStatusBar>
#include <QtWebKit>
#if QT_VERSION >= 0x050000
#include <QtWebKitWidgets>
#endif

static const vmime::charset VMIME_UTF8("UTF-8");

struct MailViewInternal
{
    QWebView *view;
    QWebPage *page;
    QLabel* header;
    MailNetworkManager* network;
};

MailView::MailView(QWidget *parent)
    : QMainWindow(parent), internal(new MailViewInternal)
{
    QBoxLayout* box = new QVBoxLayout;
    internal->header = new QLabel;
    box->addWidget(internal->header);

    QWebView *view = new QWebView;
    internal->view = view;
    box->addWidget(view, 1);

    QWidget* widget = new QWidget;
    setCentralWidget(widget);
    widget->setLayout(box);

    internal->page = view->page();
#ifndef QT_NO_DEBUG
    internal->page->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
#endif
    internal->page->settings()->setAttribute(QWebSettings::JavascriptEnabled, false);

    connect(view, SIGNAL(linkClicked(QUrl)), SLOT(browseUrl(QUrl)));
    internal->page->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(internal->page, SIGNAL(linkHovered(QString,QString,QString)),
            SLOT(showUrl(QString,QString)));

    internal->network = new MailNetworkManager(internal->page->networkAccessManager(), this);
    internal->page->setNetworkAccessManager(internal->network);
    connect(internal->network, SIGNAL(messageLoaded(HTMLMailMessage*)),
            SLOT(browseToRoot()), Qt::QueuedConnection);
    connect(internal->network, SIGNAL(remoteEnabledChanged(bool)),
            view, SLOT(reload()),
            Qt::QueuedConnection);

    QAction* action = new QAction("&Quit", this);
    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence::Quit);
    shortcuts.append(QKeySequence(Qt::Key_Q));
    action->setShortcuts(shortcuts);
    connect(action, SIGNAL(triggered()), SLOT(close()));
    addAction(action);
}

MailView::~MailView()
{
    delete internal;
}

void
MailView::browseToRoot()
{
    internal->view->load(QUrl("cid:ROOT"));
}

void
MailView::browseUrl(QUrl url)
{
    QDesktopServices::openUrl(url);
}

void
MailView::showUrl(QString link, QString title)
{
    QStatusBar* sb = statusBar();
    if (link.isEmpty()) {
        sb->clearMessage();
    } else {
        sb->showMessage(link);
    }
}

static QString
decodeText(const vmime::text& text) {
    std::string decoded = text.getConvertedText(VMIME_UTF8);
    return QString::fromUtf8(decoded.c_str());
}

static QString
decodeText(const vmime::ref<vmime::text>& text) {
    return decodeText(*text);
}

static void
renderMailAddress(vmime::ref<vmime::mailbox> addr, QTextStream& output)
{
    QString name = decodeText(addr->getName());
    std::string rawAddress = addr->getEmail(); // FIXME bad encoding
    QString address = QString::fromUtf8(rawAddress.c_str());

    if (name.isEmpty()) {
        output << address;
    } else {
        output << name << "< " << address <<">";
    }
}

static void
renderAddressList(std::vector<vmime::ref<vmime::address>> addresses, QTextStream& output)
{
    bool first = true;
    for (auto recip: addresses) {
        if (first) {
            first = false;
        } else {
            output << ", ";
        }
        renderMailAddress(recip.dynamicCast<vmime::mailbox>(), output);
    }
}

static QString
renderHeader(vmime::ref<vmime::message> msg)
{
    QString result;
    QTextStream output(&result);

    auto header = msg->getHeader();

    output <<"<b>Subject:</b> "
          << decodeText(header->Subject()->getValue().dynamicCast<vmime::text>())
          <<"<br>\n";

    auto from = header->From()->getValue().dynamicCast<vmime::mailbox>();
    output <<"<b>From:</b> ";
    renderMailAddress(from, output);
    output <<"<br>\n";

    auto recips = header->To()->getValue().dynamicCast<vmime::addressList>();
    output <<"<b>To:</b> ";
    renderAddressList(recips->getAddressList(), output);

    output.flush();

    return result;
}

void
MailView::updateHeader(vmime::ref<vmime::message> message)
{
    QString headerText = renderHeader(message);

    internal->header->setText(headerText);
}

void
MailView::setMessage(HTMLMailMessage *msg)
{
    internal->network->activeMessage(msg);
    updateHeader(msg->getMessage());
}
