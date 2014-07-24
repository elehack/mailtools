#ifndef MAILNETWORKMANAGER_H
#define MAILNETWORKMANAGER_H

#include <QtNetwork>

#include "htmlmail.h"

class MailNetworkManager : public QNetworkAccessManager
{
    Q_OBJECT
    Q_PROPERTY(bool remoteEnabled READ remoteEnabled WRITE remoteEnabled NOTIFY remoteEnabledChanged)
public:
    explicit MailNetworkManager(QNetworkAccessManager* old, QObject *parent = 0);

    bool remoteEnabled() const;
    void remoteEnabled(bool enabled);

    HTMLMailMessage* activeMessage();
    void activeMessage(HTMLMailMessage* msg);

signals:
    void remoteEnabledChanged(bool value);
    void blockedRequest(const QNetworkRequest& req);
    void messageLoaded(HTMLMailMessage* msg);

public slots:
    void enableRemoteRequests();

protected slots:
    void replyFinished();

protected:
    virtual QNetworkReply* createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0);

private:
    QNetworkReply* makeReply(const QNetworkRequest& req, vmime::ref<vmime::bodyPart> content);

    bool f_remoteEnabled;
    QPointer<HTMLMailMessage> theMessage;
};

#endif // MAILNETWORKMANAGER_H
