#include "htmlmail.h"

#include <iostream>
#include <fstream>

#include <QtDebug>

using vmime::ref;

static ref<vmime::bodyPart>
getRelatedRoot(HTMLMailMessage* msg, ref<vmime::bodyPart> message);
static ref<vmime::bodyPart>
findBody(HTMLMailMessage* msg, ref<vmime::bodyPart> message);

static void print_msg(ref<vmime::bodyPart> msg, std::string pfx = "") {
    auto ctype = msg->getHeader()->ContentType();
    auto mtype = ctype->getValue().dynamicCast<vmime::mediaType>();
    std::cout <<pfx <<"msg " << ctype->getName();
    std::cout <<" " <<ctype->getValue()->generate();
    std::cout <<" {\n";
    std::cout <<pfx <<"  type: " <<mtype->getType() <<"/" <<mtype->getSubType() <<std::endl;
    std::cout <<pfx <<"  CID: " <<msg->getHeader()->ContentId()->getValue()->generate() <<std::endl;
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

const vmime::ref<vmime::message>
HTMLMailMessage::getMessage() const
{
    return message;
}

static ref<vmime::bodyPart>
getRelatedRoot(HTMLMailMessage* msg, ref<vmime::bodyPart> message)
{
    auto mrfld = message->getHeader()->ContentType().dynamicCast<vmime::parameterizedHeaderField>();
    if (mrfld->hasParameter("start")) {
        std::string start = mrfld->getParameter("start")->getValue().getBuffer();
        QString st = QString::fromStdString(start);
        qDebug() <<"starts with content-id" << st;
        return findBody(msg, msg->getRelatedPart(st));
    } else {
        qDebug() <<"starts with first element";
        return findBody(msg, message->getBody()->getPartAt(0));
    }
}

static ref<vmime::bodyPart>
findBody(HTMLMailMessage* msg, ref<vmime::bodyPart> message)
{
    auto ctype = message->getHeader()->ContentType();
    auto mtype = ctype->getValue().dynamicCast<vmime::mediaType>();
    if (mtype->getType() == "multipart") {
        std::string subtype = mtype->getSubType();
        if (subtype == "related") {
            qDebug("extracting from multipart/related");
            return getRelatedRoot(msg, message);
        } else if (subtype == "mixed" || subtype == "signed") {
            qDebug("extracting from multipart/%s", subtype.c_str());
            return findBody(msg, message->getBody()->getPartAt(0));
        } else if (subtype == "alternative") {
            qDebug("returning multipart/alternative");
            return message;
        } else {
            return NULL;
        }
    } else {
        qDebug("not multipart, returning");
        return message;
    }
}

static ref<vmime::bodyPart>
findHtml(ref<vmime::bodyPart> message) {
    auto mctype = message->getHeader()->ContentType();
    auto mmtype = mctype->getValue().dynamicCast<vmime::mediaType>();
    if (mmtype->getType() != "multipart") {
        return message;
    }
    ref<vmime::bodyPart> plain;
    qDebug() <<"scanning" <<message->getBody()->getPartList().size() <<"parts";
    for (auto part: message->getBody()->getPartList()) {
        auto ctype = part->getHeader()->ContentType();
        auto mtype = ctype->getValue().dynamicCast<vmime::mediaType>();
        if (mtype->getType() != "text") {
            continue;
        }
        if (mtype->getSubType() == "html") {
            qDebug() <<"found text/html part";
            return part;
        } else if (mtype->getSubType() == "plain") {
            qDebug() <<"found text/plain part";
            plain = part;
        }
    }
    return plain;
}

ref<vmime::bodyPart>
HTMLMailMessage::getBody()
{
    auto body = findBody(this, message);
    return findHtml(body);
}

ref<vmime::bodyPart>
HTMLMailMessage::getRelatedPart(QString cid)
{
    return NULL;
}
