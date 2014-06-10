#include "htmlmail.h"

#include <iostream>
#include <fstream>

using vmime::ref;

static void print_msg(ref<vmime::bodyPart> msg, std::string pfx = "") {
    auto ctype = msg->getHeader()->ContentType();
    std::cout <<pfx <<"msg " << ctype->getName();
    std::cout <<" " <<ctype->getValue()->generate();
    std::cout <<" {\n";
    std::cout <<pfx <<"  CID: " <<msg->getHeader()->ContentId()->generate() <<std::endl;
    for (auto part: msg->getBody()->getPartList()) {
        print_msg(part, pfx + "  ");
    }
    std::cout <<pfx <<"}\n";
}

HTMLMailMessage::HTMLMailMessage(QObject *parent) :
    QObject(parent)
{
}

void
HTMLMailMessage::load(QString fn)
{
    fileName = fn;

    std::ifstream input(fn.toStdString());
    vmime::utility::inputStreamAdapter adap(input);
    vmime::string data;
    vmime::utility::outputStreamStringAdapter os(data);
    vmime::utility::bufferedStreamCopy(adap, os);

    message = vmime::create<vmime::message>();
    message->parse(data);
}

void
HTMLMailMessage::dump()
{
    print_msg(message);
}
