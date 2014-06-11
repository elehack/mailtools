#include "mailview.h"
#include "mailnetworkmanager.h"
#include "htmlmail.h"

#include <QtWebKit>

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
MailView::setMessage(HTMLMailMessage *msg)
{
    internal->network->activeMessage(msg);
}
