#include "mailnetworkmanager.h"
#include "statichttpreply.h"

#include <QUrl>

MailNetworkManager::MailNetworkManager(QObject *parent) :
    QNetworkAccessManager(parent)
{
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
    if (scheme == "cid") {
        if (op == GetOperation) {
            vmime::ref<vmime::bodyPart> part;
            if (url.path() == "ROOT") {
                part = theMessage->getBody();
            } else {
                part = theMessage->getRelatedPart(url.path());
            }
            if (part) {
                return makeReply(req, part);
            } else {
                return StaticHTTPReply::notFound(req);
            }
        } else {
            return StaticHTTPReply::denied(op, req);
        }
    } else if (f_remoteEnabled) {
        return QNetworkAccessManager::createRequest(op, req, outgoingData);
    } else {
        return StaticHTTPReply::denied(op, req);
    }
}

QNetworkReply*
MailNetworkManager::makeReply(const QNetworkRequest& req, vmime::ref<vmime::bodyPart> content)
{
    auto ctype = content->getHeader()->ContentType()->getValue()->generate();
    QString mediaType = QString::fromStdString(ctype);

    // FIXME Don't copy the data quite so much
    vmime::string data;
    {
        vmime::utility::outputStreamStringAdapter os(data);
        content->getBody()->getContents()->extract(os);
    }
    QByteArray buffer(data.c_str(), data.size());
    return StaticHTTPReply::ok(req, buffer, mediaType);
}
