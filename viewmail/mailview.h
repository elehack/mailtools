#ifndef MAILVIEW_H
#define MAILVIEW_H

#include <QMainWindow>
#include <QUrl>

#include "gmime-decls.h"

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


    void handleBlockedImage(const QNetworkRequest& req);

    void bodyLoadStarted();

private:
    void updateHeader(GMimeMessage* message);

    MailViewInternal* const internal;
    Ui::MailViewWindow* ui;
};

#endif // MAILVIEW_H
