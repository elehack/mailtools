#ifndef MAILVIEW_H
#define MAILVIEW_H

#include <QMainWindow>

class MailView : public QMainWindow
{
    Q_OBJECT

public:
    MailView(QWidget *parent = 0);
    ~MailView();
};

#endif // MAILVIEW_H
