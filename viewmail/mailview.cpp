#include "mailview.h"
#include "mailnetworkmanager.h"
#include "htmlmail.h"
#include <mustache.h>

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

static QString
formatMailAddress(vmime::ref<vmime::mailbox> addr)
{
    QString name = decodeText(addr->getName());
    std::string rawAddress = addr->getEmail(); // FIXME bad encoding
    QString address = QString::fromUtf8(rawAddress.c_str());

    if (name.isEmpty()) {
        return address;
    } else {
        return name + " <" + address + ">";
    }
}

static QVariantHash
makeHeaderContext(vmime::ref<vmime::message> msg)
{
    QVariantHash hash;
    auto header = msg->getHeader();
    auto from = header->From()->getValue().dynamicCast<vmime::mailbox>();
    hash["subject"] = decodeText(header->Subject()->getValue().dynamicCast<vmime::text>());
    hash["from"] = formatMailAddress(from);

    QString recipients;
    auto recips = header->To()->getValue().dynamicCast<vmime::addressList>();
    for (auto recip: recips->getAddressList()) {
        if (!recipients.isEmpty()) {
            recipients += ", ";
        }
        recipients += formatMailAddress(recip.dynamicCast<vmime::mailbox>());
    }
    hash["to"] = recipients;

    qDebug() <<"Mail from" <<hash["from"];

    return hash;
}

void
MailView::updateHeader(vmime::ref<vmime::message> message)
{
    QResource templateResource(":/viewmail/header-template.txt");
    if (!templateResource.isValid()) {
        qWarning("template is not valid");
    }
    QString tmpl = QString::fromUtf8((const char*) templateResource.data());
    qDebug() <<"Header template:" <<tmpl;

    Mustache::Renderer renderer;
    Mustache::QtVariantContext context(makeHeaderContext(message));

    QString headerText;
    {
        QTextStream output(&headerText);
        output << renderer.render(tmpl, &context);
    }
    qDebug() <<"Header text:" <<headerText;

    internal->header->setText(headerText);
}

void
MailView::setMessage(HTMLMailMessage *msg)
{
    internal->network->activeMessage(msg);
    updateHeader(msg->getMessage());
}
