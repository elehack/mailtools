#include "addressbook.h"

#include <QtCore>

struct AddressBookPrivate
{
    QString fileName;
    bool dirty;
};

AddressBook::AddressBook(QObject *parent) :
    QObject(parent), data(new AddressBookPrivate)
{
    data->dirty = false;
}

AddressBook::~AddressBook()
{
    delete data;
}

QString
AddressBook::fileName() const
{
    return data->fileName;
}

void
AddressBook::setFileName(const QString &fn)
{
    data->fileName = fn;
}

bool
AddressBook::dirty() const
{
    return data->dirty;
}

void
AddressBook::setDirty()
{
    bool invoke = !data->dirty;
    data->dirty = true;
    if (invoke) {
        emit dirtied();
    }
}
