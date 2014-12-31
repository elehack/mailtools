#include <gmime/gmime.h>

#include <iostream>
#include <fstream>

#include "htmlmail.h"

#include <QtDebug>

static GMimeObject*
getSinglePart(HTMLMailMessage* msg, GMimeObject* message);
static GMimePart*
getPartById(HTMLMailMessage* msg, GMimePart* message, QString cid);
static GMimeObject*
getRelatedRoot(HTMLMailMessage* msg, GMimeMultipart* message);
static GMimeObject*
findBody(HTMLMailMessage* msg, GMimeObject* message);
static GMimeObject*
pickAlternative(HTMLMailMessage* msg, GMimeMultipart* message);

static GMimeContentType*
contentType(GMimeObject* message)
{
    return g_mime_object_get_content_type(GMIME_OBJECT(message));
}

static void print_msg(GMimeObject* msg, std::string pfx = "") {
    auto mtype = contentType(msg);
    std::cout <<pfx <<"msg {\n";
    std::cout <<pfx <<"  type: "
        <<g_mime_content_type_get_media_type(mtype)
        <<"/"
        <<g_mime_content_type_get_media_subtype(mtype)
        <<std::endl;
    std::cout <<pfx <<"  CID: " <<g_mime_object_get_content_id(GMIME_OBJECT(msg))
        <<std::endl;
    if (GMIME_IS_MULTIPART(msg)) {
        GMimeMultipart *mp = GMIME_MULTIPART(msg);
        int n = g_mime_multipart_get_count(mp);
        for (int i = 0; i < n; i++) {
            print_msg(g_mime_multipart_get_part(mp, i), pfx + "  ");
        }
    }
    std::cout <<pfx <<"}\n";
}

HTMLMailMessage::HTMLMailMessage(QObject *parent) :
    QObject(parent), message(NULL)
{
}

HTMLMailMessage::~HTMLMailMessage()
{
    if (message) {
        g_object_unref(message);
    }
}

void
HTMLMailMessage::load_stdin()
{
    guint8 buffer[4096];
    ssize_t nbytes;
    GByteArray *bytes = g_byte_array_new();

    while ((nbytes = read(0, buffer, 4096))) {
        if (nbytes < 0) {
            perror("viewmail");
            exit(2);
        }
        g_byte_array_append(bytes, buffer, nbytes);
    }

    GMimeStream* stream = g_mime_stream_mem_new_with_byte_array(bytes);
    g_mime_stream_mem_set_owner(GMIME_STREAM_MEM(stream), true);

    GMimeParser* parser = g_mime_parser_new_with_stream(stream);

    message = g_mime_parser_construct_message(parser);

    g_object_unref(parser);
    g_object_unref(stream);
}

void
HTMLMailMessage::load_file(const char *fn)
{
    FILE* file = fopen(fn, "r");
    GMimeStream* stream = g_mime_stream_file_new(file);
    GMimeParser* parser = g_mime_parser_new_with_stream(stream);

    message = g_mime_parser_construct_message(parser);

    g_object_unref(parser);
    g_object_unref(stream);
}

void
HTMLMailMessage::dump()
{
    print_msg(GMIME_OBJECT(message));
}

GMimeMessage*
HTMLMailMessage::getMessage() const
{
    return message;
}

static GMimeObject*
getSinglePart(HTMLMailMessage* msg, GMimeObject* message)
{
    auto ctype = contentType(message);
    std::string subtype = g_mime_content_type_get_media_subtype(ctype);
    GMimeMultipart* mp = GMIME_IS_MULTIPART(message) ? GMIME_MULTIPART(message) : NULL;
    if (mp == NULL) {
        qDebug("found single-part message");
        return message;
    } else if (subtype == "related") {
        qDebug("extracting from multipart/related");
        return getRelatedRoot(msg, mp);
    } else if (subtype == "mixed" || subtype == "signed") {
        qDebug("extracting from multipart/%s", subtype.c_str());
        return findBody(msg, g_mime_multipart_get_part(mp, 0));
    } else if (subtype == "alternative") {
        return pickAlternative(msg, mp);
    } else {
        return NULL;
    }
}

static GMimeObject*
getRelatedRoot(HTMLMailMessage* msg, GMimeMultipart* message)
{
    auto ctype = g_mime_object_get_content_type(GMIME_OBJECT(message));
    const char *start = g_mime_content_type_get_parameter(ctype, "start");
    if (start != NULL) {
        qDebug("start with content-id %s", start);
        GMimeObject *obj = g_mime_multipart_get_subpart_from_content_id(message, start);
        return findBody(msg, obj);
    } else {
        qDebug() <<"starts with first element";
        return findBody(msg, g_mime_multipart_get_part(message, 0));
    }
}

static GMimeObject*
findBody(HTMLMailMessage* msg, GMimeObject* message)
{
    return getSinglePart(msg, message);
}

static GMimeObject*
pickAlternative(HTMLMailMessage* msg, GMimeMultipart* message) {
    auto ctype = g_mime_object_get_content_type(GMIME_OBJECT(message));
    std::string subtype = g_mime_content_type_get_media_subtype(ctype);
    g_return_val_if_fail(subtype == "alternative", NULL);

    GMimeObject *plain = NULL;
    int count = g_mime_multipart_get_count(message);
    qDebug() <<"scanning" <<count <<"parts";
    for (int i = 0; i < count; i++) {
        GMimeObject* part = g_mime_multipart_get_part(message, i);
        if (GMIME_IS_MULTIPART(part)) {
            part = getSinglePart(msg, part);
        }
        auto mtype = g_mime_object_get_content_type(part);
        std::string type = g_mime_content_type_get_media_type(mtype);
        std::string subtype = g_mime_content_type_get_media_subtype(mtype);
        if (type == "text") {
            if (subtype == "html") {
                qDebug() <<"found text/html part";
                return part;
            } else if (subtype == "plain") {
                qDebug() <<"found text/plain part";
                plain = part;
            }
        }
    }
    return plain;
}

GMimeObject*
HTMLMailMessage::getBody()
{
    return findBody(this, g_mime_message_get_mime_part(message));
}

GMimeObject*
HTMLMailMessage::getRelatedPart(QString cid)
{
    qDebug() <<"retrieving part" <<cid;
    QByteArray bytes = cid.toUtf8();
    return g_mime_multipart_get_subpart_from_content_id(
            GMIME_MULTIPART(g_mime_message_get_mime_part(message)),
            bytes.data());
}
