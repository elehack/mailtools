#ifndef MAILVIEW_H
#define MAILVIEW_H

#include <QMainWindow>
#include <QUrl>

class MailViewInternal;
class HTMLMailMessage;
class QNetworkReply;

class MailView : public QMainWindow
{
    Q_OBJECT

public:
    MailView(QWidget *parent = 0);
    ~MailView();

    void setMessage(HTMLMailMessage* msg);

public slots:
    void browseToRoot();

    void browseUrl(const QUrl& url);
    void showUrl(const QString& link, const QString& title);

private:
    MailViewInternal* const internal;
};

#endif // MAILVIEW_H
