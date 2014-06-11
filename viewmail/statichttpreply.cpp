#include "statichttpreply.h"

#include <QtCore>

StaticHTTPReply::StaticHTTPReply(QObject *parent) :
    QNetworkReply(parent), position(0)
{
}

void
StaticHTTPReply::abort()
{
}

qint64
StaticHTTPReply::readData(char* data, qint64 maxSize)
{
    if (position >= bytes.size()) {
        return -1;
    }

    qint64 sz = std::min(bytes.length() - position, maxSize);
    qstrncpy(data, bytes.constData() + position, sz);
    position += sz;
    return sz;
}

void
StaticHTTPReply::indicateReady()
{
    emit metaDataChanged();
    emit readyRead();
    emit finished();
}

StaticHTTPReply*
StaticHTTPReply::ok(const QNetworkRequest& req,
                    const QByteArray& data, QString contentType)
{
    StaticHTTPReply* reply = new StaticHTTPReply();
    reply->setRequest(req);
    reply->setUrl(req.url());
    reply->setOperation(QNetworkAccessManager::GetOperation);

    reply->setBuffer(data);
    reply->setHeader(QNetworkRequest::ContentLengthHeader, data.size());
    reply->setHeader(QNetworkRequest::ContentTypeHeader, contentType);

    QTimer::singleShot(0, reply, SLOT(indicateReady()));

    return reply;
}

StaticHTTPReply*
StaticHTTPReply::notFound(const QNetworkRequest& req)
{
    QString message = "Object not found";
    QByteArray bytes = message.toAscii();

    StaticHTTPReply* reply = new StaticHTTPReply();
    reply->setRequest(req);
    reply->setUrl(req.url());
    reply->setOperation(QNetworkAccessManager::GetOperation);

    reply->setBuffer(bytes);
    reply->setError(NetworkError::ContentNotFoundError, "Not Found");
    reply->setHeader(QNetworkRequest::ContentLengthHeader, bytes.size());
    reply->setHeader(QNetworkRequest::ContentTypeHeader, "text/plain; charset=us-ascii");

    QTimer::singleShot(0, reply, SLOT(indicateReady()));

    return reply;
}

StaticHTTPReply*
StaticHTTPReply::denied(QNetworkAccessManager::Operation op, const QNetworkRequest& req)
{
    QString message = "Access denied";
    QByteArray bytes = message.toAscii();

    StaticHTTPReply* reply = new StaticHTTPReply();
    reply->setRequest(req);
    reply->setUrl(req.url());
    reply->setOperation(op);

    reply->setBuffer(bytes);
    reply->setError(NetworkError::ConnectionRefusedError, "Connection refused");
    reply->setHeader(QNetworkRequest::ContentLengthHeader, bytes.size());
    reply->setHeader(QNetworkRequest::ContentTypeHeader, "text/plain; charset=us-ascii");

    QTimer::singleShot(0, reply, SLOT(indicateReady()));

    return reply;
}

void
StaticHTTPReply::setBuffer(const QByteArray& buf)
{
    bytes = buf;
    position = 0;
}
