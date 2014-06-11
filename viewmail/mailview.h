#ifndef MAILVIEW_H
#define MAILVIEW_H

#include <QMainWindow>

class MailViewInternal;
class HTMLMailMessage;

class MailView : public QMainWindow
{
    Q_OBJECT

public:
    MailView(QWidget *parent = 0);
    ~MailView();

    void setMessage(HTMLMailMessage* msg);

public slots:
    void browseToRoot();

private:
    MailViewInternal* const internal;
};

#endif // MAILVIEW_H
