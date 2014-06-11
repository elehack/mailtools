#include "statichttpreply.h"

#include <QtCore>
#include <QtDebug>

StaticHTTPReply::StaticHTTPReply(QObject *parent) :
    QNetworkReply(parent), offset(0)
{
}

void
StaticHTTPReply::abort()
{
}

bool
StaticHTTPReply::isSequential() const
{
    return true;
}

qint64
StaticHTTPReply::bytesAvailable() const
{
    return bytes.size() - offset + QIODevice::bytesAvailable();
}

qint64
StaticHTTPReply::readData(char* data, qint64 maxSize)
{
    qDebug() <<"reading" <<maxSize <<"bytes of" <<(bytes.size() - offset) <<"available";

    qint64 sz = std::min(bytes.length() - offset, maxSize);
    qstrncpy(data, bytes.constData() + offset, sz);
    offset += sz;
    return sz;
}

void
StaticHTTPReply::indicateReady()
{
    emit metaDataChanged();
    open(ReadOnly | Unbuffered);
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
    reply->setHeader(QNetworkRequest::ContentTypeHeader, QVariant(contentType));

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
    reply->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);

    QTimer::singleShot(100, reply, SLOT(indicateReady()));

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

    QTimer::singleShot(100, reply, SLOT(indicateReady()));

    return reply;
}

void
StaticHTTPReply::setBuffer(const QByteArray& buf)
{
    bytes = buf;
    offset = 0;
}
