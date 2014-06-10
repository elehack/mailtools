#include "mailnetworkmanager.h"

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

QNetworkReply*
MailNetworkManager::createRequest(Operation op, const QNetworkRequest& req, QIODevice* outgoingData)
{
    QUrl url = req.url();
    QString scheme = url.scheme();
    if (scheme == "cid") {
    } else if (f_remoteEnabled) {
        return QNetworkAccessManager::createRequest(op, req, outgoingData);
    } else {
    }
}
