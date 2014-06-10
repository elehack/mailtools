#ifndef HTMLMAIL_H
#define HTMLMAIL_H

#include <QObject>
#include <vmime/vmime.hpp>

class HTMLMailMessage : public QObject
{
    Q_OBJECT
public:
    explicit HTMLMailMessage(QObject *parent = 0);

    void load(QString fn);

    void dump();

signals:

public slots:

private:
    QString fileName;
    vmime::ref<vmime::message> message;
};

#endif // HTMLMAIL_H
