#include "mailview.h"
#include "mailnetworkmanager.h"
#include "htmlmail.h"

#include <QtGui>
#include <QStatusBar>
#include <QtWebKit>
#if QT_VERSION >= 0x050000
#include <QtWebKitWidgets>
#endif

struct MailViewInternal
{
    QWebView *view;
    QWebPage *page;
    MailNetworkManager* network;
};

MailView::MailView(QWidget *parent)
    : QMainWindow(parent), internal(new MailViewInternal)
{
    QWebView *view = new QWebView;
    internal->view = view;
    setCentralWidget(view);
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
MailView::browseUrl(const QUrl& url)
{
    QDesktopServices::openUrl(url);
}

void
MailView::showUrl(const QString& link, const QString& title)
{
    QStatusBar* sb = statusBar();
    if (link.isEmpty()) {
        sb->clearMessage();
    } else {
        sb->showMessage(link);
    }
}

void
MailView::setMessage(HTMLMailMessage *msg)
{
    internal->network->activeMessage(msg);
}
