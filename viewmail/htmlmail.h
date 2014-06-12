#ifndef HTMLMAIL_H
#define HTMLMAIL_H

#include <QObject>
#include <vmime/vmime.hpp>

class HTMLMailMessage : public QObject
{
    Q_OBJECT
public:
    explicit HTMLMailMessage(QObject *parent = 0);

    void load(QString fn);

    void dump();

    const vmime::ref<vmime::message> getMessage() const;

    /**
     * @brief Get the body for a message.
     * @return The message body (HTML if available).
     */
    vmime::ref<vmime::bodyPart> getBody();

    /**
     * @brief Get an attached part for a message.
     * @param cid The attached part.
     * @return The attachment identified by content-id CID.
     */
    vmime::ref<vmime::bodyPart> getRelatedPart(QString cid);

    /**
     * @brief Get the content IDs in this message.
     * @return The set of content IDs.
     */
    QSet<QString> getContentIds();

signals:

public slots:

private:
    QString fileName;
    vmime::ref<vmime::message> message;
};

#endif // HTMLMAIL_H
