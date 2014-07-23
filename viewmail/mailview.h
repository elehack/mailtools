#ifndef MAILVIEW_H
#define MAILVIEW_H

#include <QMainWindow>
#include <QUrl>

#include <vmime/vmime.hpp>

class MailViewInternal;
class HTMLMailMessage;
class QNetworkReply;
class QNetworkRequest;

namespace Ui {
    class MailViewWindow;
}

class MailView : public QMainWindow
{
    Q_OBJECT

public:
    MailView(QWidget *parent = 0);
    ~MailView();

    void setMessage(HTMLMailMessage* msg);

public slots:
    void browseToRoot();
    void showPrintDialog();

    void browseUrl(QUrl url);
    void showUrl(QString link, QString title);

    void updateHeader(vmime::ref<vmime::message> message);

    void handleBlockedImage(const QNetworkRequest& req);

private:
    MailViewInternal* const internal;
    Ui::MailViewWindow* ui;
};

#endif // MAILVIEW_H
