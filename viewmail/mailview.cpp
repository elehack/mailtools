#include "mailview.h"
#include "mailnetworkmanager.h"
#include "htmlmail.h"
#include "ui_mailview.h"

#include <QtGui>
#include <QStatusBar>
#include <QtWebKit>
#if QT_VERSION >= 0x050000
#include <QtWebKitWidgets>
#endif

static const vmime::charset VMIME_UTF8("UTF-8");

struct MailViewInternal
{
    QWebPage* page;
    MailNetworkManager* network;
    int blockCount;
};

MailView::MailView(QWidget *parent)
    : QMainWindow(parent), internal(new MailViewInternal), ui(new Ui::MailViewWindow)
{
    ui->setupUi(this);

    internal->page = ui->bodyView->page();
#ifndef QT_NO_DEBUG
    internal->page->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
#endif
    internal->page->settings()->setAttribute(QWebSettings::JavascriptEnabled, false);

    connect(ui->bodyView, SIGNAL(linkClicked(QUrl)),
            SLOT(browseUrl(QUrl)));
    internal->page->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(internal->page, SIGNAL(linkHovered(QString,QString,QString)),
            SLOT(showUrl(QString,QString)));

    internal->network = new MailNetworkManager(internal->page->networkAccessManager(), this);
    internal->page->setNetworkAccessManager(internal->network);
    connect(internal->network, SIGNAL(messageLoaded(HTMLMailMessage*)),
            SLOT(browseToRoot()), Qt::QueuedConnection);
    connect(internal->network, SIGNAL(remoteEnabledChanged(bool)),
            ui->bodyView, SLOT(reload()),
            Qt::QueuedConnection);

    connect(ui->actionPrint, SIGNAL(triggered()), SLOT(showPrintDialog()));
    addAction(ui->actionQuit);
    addAction(ui->actionPrint);

    connect(internal->network, SIGNAL(blockedRequest(const QNetworkRequest&)),
            SLOT(handleBlockedImage(const QNetworkRequest&)));
}

MailView::~MailView()
{
    delete internal;
    delete ui;
}

void
MailView::browseToRoot()
{
    ui->bodyView->load(QUrl("cid:ROOT"));
}

void
MailView::browseUrl(QUrl url)
{
    QDesktopServices::openUrl(url);
}

void
MailView::showUrl(QString link, QString title)
{
    QStatusBar* sb = ui->statusbar;
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
decodeText(const vmime::ref<vmime::headerField>& text) {
    return decodeText(*(text->getValue().dynamicCast<const vmime::text>()));
}

static QString
renderMailAddress(vmime::ref<vmime::mailbox> addr)
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

static QString
renderMailAddress(vmime::ref<vmime::headerField> header)
{
    return renderMailAddress(header->getValue().dynamicCast<vmime::mailbox>());
}

static QString
renderAddressList(std::vector<vmime::ref<vmime::address>> addresses)
{
    QStringList strings;
    for (auto addr: addresses) {
        strings << renderMailAddress(addr.dynamicCast<vmime::mailbox>());
    }
    return strings.join(", ");
}

static QString
renderAddressList(vmime::ref<vmime::headerField> addresses)
{
    return renderAddressList(addresses->getValue().dynamicCast<vmime::addressList>()->getAddressList());
}

void
MailView::updateHeader(vmime::ref<vmime::message> message)
{
    auto header = message->getHeader();
    ui->subject->setText(decodeText(header->Subject()));
    ui->from->setText(renderMailAddress(header->From()));

    ui->to->setText(renderAddressList(header->To()));
    auto cc = header->Cc();
    auto ccList = cc->getValue().dynamicCast<vmime::addressList>();
    if (ccList->isEmpty()) {
        ui->cc->setVisible(false);
        ui->ccLabel->setVisible(false);
    } else {
        ui->cc->setVisible(true);
        ui->ccLabel->setVisible(true);
        ui->cc->setText(renderAddressList(cc));
    }
}

void
MailView::setMessage(HTMLMailMessage *msg)
{
    ui->loadImages->setVisible(false);
    internal->blockCount = 0;
    internal->network->activeMessage(msg);
    updateHeader(msg->getMessage());
}

void
MailView::showPrintDialog()
{
    QPrintDialog* dlg = new QPrintDialog(this);
    connect(dlg, SIGNAL(accepted(QPrinter*)),
            ui->bodyView, SLOT(print(QPrinter*)));
    dlg->show();
}

void
MailView::handleBlockedImage(const QNetworkRequest &req)
{
    ui->loadImages->setVisible(true);
    internal->blockCount += 1;
    qDebug() <<"Blocked" <<internal->blockCount <<"images";
    ui->loadImages->setText(QString("Load %1 blocked images").arg(internal->blockCount));
}
