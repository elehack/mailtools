#include <gmime/gmime.h>

#include "mailnetworkmanager.h"
#include "statichttpreply.h"

#include <QtNetwork>
#include <QtDebug>

MailNetworkManager::MailNetworkManager(QNetworkAccessManager* old, QObject *parent) :
    QNetworkAccessManager(parent), f_remoteEnabled(false)
{
    setCache(old->cache());
    setCookieJar(old->cookieJar());
    setProxy(old->proxy());
    setProxyFactory(old->proxyFactory());
}

bool
MailNetworkManager::remoteEnabled() const
{
    return f_remoteEnabled;
}

void
MailNetworkManager::remoteEnabled(bool enabled)
{
    f_remoteEnabled = enabled;
    emit remoteEnabledChanged(enabled);
}

HTMLMailMessage*
MailNetworkManager::activeMessage()
{
    return theMessage;
}

void
MailNetworkManager::activeMessage(HTMLMailMessage* msg)
{
    theMessage = msg;
    emit messageLoaded(msg);
}

QNetworkReply*
MailNetworkManager::createRequest(Operation op, const QNetworkRequest& req, QIODevice* outgoingData)
{
    QUrl url = req.url();
    QString scheme = url.scheme();
    QNetworkReply* reply = NULL;
    if (scheme == "cid") {
        if (op == GetOperation) {
            qDebug() <<"getting component for" <<url;
            GMimeObject* part = NULL;
            if (url.path() == "ROOT") {
                part = theMessage->getBody();
            } else {
                part = theMessage->getRelatedPart(url.path());
            }
            if (part) {
                reply = makeReply(req, part);
            } else {
                qDebug() <<url <<"not found";
                reply = StaticHTTPReply::notFound(req);
            }
        } else {
            qDebug() <<"operation" <<op <<"not supported for CID URLs";
            reply = StaticHTTPReply::denied(op, req);
        }
    } else if (f_remoteEnabled && (url.scheme() == "http" || url.scheme() == "https")) {
        qDebug() <<"delegating request for" <<url;
        return QNetworkAccessManager::createRequest(op, req, outgoingData);
    } else {
        qDebug() <<"denying remote request for" <<url;
        reply = StaticHTTPReply::denied(op, req);
        emit blockedRequest(req);
    }
    connect(reply, SIGNAL(finished()), SLOT(replyFinished()));
    return reply;
}

void
MailNetworkManager::replyFinished()
{
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
    qDebug() <<"reply" <<reply <<"finished";
    emit finished(reply);
}

QNetworkReply*
MailNetworkManager::makeReply(const QNetworkRequest& req, GMimeObject* content)
{
    auto ctype = g_mime_object_get_content_type(content);
    char *mtstr = g_mime_content_type_to_string(ctype);
    QString mediaType = QString::fromUtf8(mtstr);
    g_free(mtstr);
    const char *enc = g_mime_content_type_get_parameter(ctype, "charset");
    if (enc != NULL) {
        mediaType += "; charset=\"";
        mediaType += QString::fromUtf8(enc);
        mediaType += "\"";
    }

    g_return_val_if_fail(GMIME_IS_PART(content), NULL);

    GMimeStream *stream = g_mime_stream_mem_new();
    GMimeDataWrapper *wrapper = g_mime_part_get_content_object(GMIME_PART(content));
    g_mime_data_wrapper_write_to_stream(wrapper, stream);
    GByteArray *bytes = g_mime_stream_mem_get_byte_array(GMIME_STREAM_MEM(stream));

    QByteArray buffer((const char*) bytes->data, bytes->len);
    g_object_unref(stream);

    qDebug() <<"making reply of length" <<buffer.size() <<"and type" <<mediaType;

    return StaticHTTPReply::ok(req, buffer, mediaType);
}

void
MailNetworkManager::enableRemoteRequests()
{
    remoteEnabled(true);
}
