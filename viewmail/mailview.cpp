#include <gmime/gmime.h>

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
    connect(ui->bodyView, SIGNAL(loadStarted()),
            SLOT(bodyLoadStarted()));
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
    connect(ui->actionEnableRemoteImages, SIGNAL(triggered()),
            internal->network, SLOT(enableRemoteRequests()));
}

MailView::~MailView()
{
    delete internal;
    delete ui;
}

void
MailView::browseToRoot()
{
    ui->loadImages->setVisible(false);
    internal->blockCount = 0;
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

void
MailView::updateHeader(GMimeMessage* message)
{
    ui->subject->setText(QString::fromUtf8(g_mime_message_get_subject(message)));

    ui->from->setText(QString::fromUtf8(g_mime_message_get_sender(message)));

    InternetAddressList *addr_list;
    char *addr_string;

    addr_list = g_mime_message_get_recipients(message, GMIME_RECIPIENT_TYPE_TO);
    addr_string = internet_address_list_to_string(addr_list, false);
    ui->to->setText(QString::fromUtf8(addr_string));
    g_free(addr_string);

    addr_list = g_mime_message_get_recipients(message, GMIME_RECIPIENT_TYPE_CC);
    if (addr_list != NULL) {
        addr_string = internet_address_list_to_string(addr_list, false);
    } else {
        addr_string = NULL;
    }
    if (addr_string != NULL) {
        ui->cc->setText(QString::fromUtf8(addr_string));
        g_free(addr_string);
        ui->cc->setVisible(true);
        ui->ccLabel->setVisible(true);
    } else {
        ui->cc->setVisible(false);
        ui->ccLabel->setVisible(false);
    }
}

void
MailView::bodyLoadStarted()
{
    ui->loadImages->setVisible(false);
    internal->blockCount = 0;
}

void
MailView::setMessage(HTMLMailMessage *msg)
{
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
