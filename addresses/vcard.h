#ifndef MAILTOOLS_VCARD_H
#define MAILTOOLS_VCARD_H

#include <QtCore>

namespace VCard {

class Param
{
public:
    Param();
    Param(const QString& name, const QString& value);
    ~Param();
};

class Property
{
};

class Card
{
};

}

#endif // MAILTOOLS_VCARD_H
