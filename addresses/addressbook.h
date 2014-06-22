#ifndef ADDRESSBOOK_H
#define ADDRESSBOOK_H

#include <QtCore>

class AddressBookPrivate;

class AddressBook : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)
    Q_PROPERTY(bool dirty READ dirty)
public:
    explicit AddressBook(QObject *parent = 0);
    ~AddressBook();

    QString fileName() const;
    void setFileName(const QString& fn);

    bool dirty() const;
    void setDirty();

signals:
    void dirtied();

public slots:

private:
    AddressBookPrivate *data;
};

#endif // ADDRESSBOOK_H
