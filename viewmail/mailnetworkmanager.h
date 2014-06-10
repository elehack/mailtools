#ifndef MAILNETWORKMANAGER_H
#define MAILNETWORKMANAGER_H

#include <QNetworkAccessManager>

class MailNetworkManager : public QNetworkAccessManager
{
    Q_OBJECT
    Q_PROPERTY(bool remoteEnabled READ remoteEnabled WRITE remoteEnabled NOTIFY remoteEnabledChanged)
public:
    explicit MailNetworkManager(QObject *parent = 0);

    bool remoteEnabled() const;
    void remoteEnabled(bool enabled);

signals:
    void remoteEnabledChanged(bool value);
    void blockedRequest(const QNetworkRequest& req);

public slots:

protected:
    virtual QNetworkReply* createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0);

private:
    bool f_remoteEnabled;
};

#endif // MAILNETWORKMANAGER_H
