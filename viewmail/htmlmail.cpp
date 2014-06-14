#include "htmlmail.h"

#include <iostream>
#include <fstream>

#include <QtDebug>

using vmime::ref;

static ref<vmime::mediaType>
contentType(ref<vmime::bodyPart> message);
static ref<vmime::bodyPart>
getSinglePart(HTMLMailMessage* msg, ref<vmime::bodyPart> message);
static ref<vmime::bodyPart>
getPartById(HTMLMailMessage* msg, ref<vmime::bodyPart> message, QString cid);
static ref<vmime::bodyPart>
getRelatedRoot(HTMLMailMessage* msg, ref<vmime::bodyPart> message);
static ref<vmime::bodyPart>
findBody(HTMLMailMessage* msg, ref<vmime::bodyPart> message);
static ref<vmime::bodyPart>
pickAlternative(HTMLMailMessage* msg, ref<vmime::bodyPart> message);

static ref<vmime::mediaType>
contentType(ref<vmime::bodyPart> message)
{
    auto ctype = message->getHeader()->ContentType();
    return ctype->getValue().dynamicCast<vmime::mediaType>();
}

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
HTMLMailMessage::load(std::istream &str)
{
    vmime::utility::inputStreamAdapter adap(str);
    vmime::string data;
    vmime::utility::outputStreamStringAdapter os(data);
    vmime::utility::bufferedStreamCopy(adap, os);

    message = vmime::create<vmime::message>();
    message->parse(data);
}

void
HTMLMailMessage::load(QString fn)
{
    fileName = fn;

    std::ifstream input(fn.toStdString());
    load(input);
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
getSinglePart(HTMLMailMessage* msg, ref<vmime::bodyPart> message)
{
    auto ctype = message->getHeader()->ContentType();
    auto mtype = ctype->getValue().dynamicCast<vmime::mediaType>();
    std::string subtype = mtype->getSubType();
    if (mtype->getType() != "multipart") {
        qDebug("found single-part message");
        return message;
    } else if (subtype == "related") {
        qDebug("extracting from multipart/related");
        return getRelatedRoot(msg, message);
    } else if (subtype == "mixed" || subtype == "signed") {
        qDebug("extracting from multipart/%s", subtype.c_str());
        return findBody(msg, message->getBody()->getPartAt(0));
    } else if (subtype == "alternative") {
        return pickAlternative(msg, message);
    } else {
        return NULL;
    }
}

static ref<vmime::bodyPart>
getPartById(HTMLMailMessage* msg, ref<vmime::bodyPart> message, std::string cid)
{
    auto hdr = message->getHeader()->ContentId()->getValue();
    auto mid = hdr.dynamicCast<vmime::messageId>();

    if (mid && mid->getId() == cid) {
        return message;
    }

    for (auto part: message->getBody()->getPartList()) {
        auto found = getPartById(msg, part, cid);
        if (found) {
            return found;
        }
    }
    return NULL;
}

static ref<vmime::bodyPart>
getRelatedRoot(HTMLMailMessage* msg, ref<vmime::bodyPart> message)
{
    auto mrfld = message->getHeader()->ContentType().dynamicCast<vmime::parameterizedHeaderField>();
    if (mrfld->hasParameter("start")) {
        std::string start = mrfld->getParameter("start")->getValue().getBuffer();
        QString st = QString::fromStdString(start);
        qDebug() <<"starts with content-id" << st;
        return findBody(msg, getPartById(msg, message, start));
    } else {
        qDebug() <<"starts with first element";
        return findBody(msg, message->getBody()->getPartAt(0));
    }
}

static ref<vmime::bodyPart>
findBody(HTMLMailMessage* msg, ref<vmime::bodyPart> message)
{
    return getSinglePart(msg, message);
}

static ref<vmime::bodyPart>
pickAlternative(HTMLMailMessage* msg, ref<vmime::bodyPart> message) {
    auto mctype = message->getHeader()->ContentType();
    auto mmtype = mctype->getValue().dynamicCast<vmime::mediaType>();
    if (mmtype->getType() != "multipart" || mmtype->getSubType() != "alternative") {
        return message;
    }

    ref<vmime::bodyPart> plain;
    qDebug() <<"scanning" <<message->getBody()->getPartList().size() <<"parts";
    for (ref<vmime::bodyPart> part: message->getBody()->getPartList()) {
        auto mtype = contentType(message);
        if (mtype->getType() == "multipart") {
            part = getSinglePart(msg, part);
            mtype = contentType(part);
        }
        if (mtype->getType() == "text") {
            if (mtype->getSubType() == "html") {
                qDebug() <<"found text/html part";
                return part;
            } else if (mtype->getSubType() == "plain") {
                qDebug() <<"found text/plain part";
                plain = part;
            }
        }
    }
    return plain;
}

ref<vmime::bodyPart>
HTMLMailMessage::getBody()
{
    return findBody(this, message);
}

ref<vmime::bodyPart>
HTMLMailMessage::getRelatedPart(QString cid)
{
    qDebug() <<"retrieving part" <<cid;
    return getPartById(this, message, cid.toStdString());
}

static void
walkMessageForContentIds(ref<vmime::bodyPart> msg, QSet<QString>& idSet)
{
    auto hdr = msg->getHeader()->ContentId()->getValue().dynamicCast<vmime::messageId>();
    if (hdr) {
        idSet.insert(QString::fromStdString(hdr->getId()));
    }
    for (auto part: msg->getBody()->getPartList()) {
        walkMessageForContentIds(part, idSet);
    }
}

QSet<QString>
HTMLMailMessage::getContentIds()
{
    QSet<QString> set;
    walkMessageForContentIds(message, set);
    return set;
}
