#ifndef HTMLMAIL_H
#define HTMLMAIL_H

#include <QObject>
#include <iostream>

#include "gmime-decls.h"

class HTMLMailMessage : public QObject
{
    Q_OBJECT
public:
    explicit HTMLMailMessage(QObject *parent = 0);
    virtual ~HTMLMailMessage();

    void load_stdin();
    void load_file(const char *fn);

    void dump();

    GMimeMessage* getMessage() const;

    /**
     * @brief Get the body for a message.
     * @return The message body (HTML if available).
     */
    GMimeObject* getBody();

    /**
     * @brief Get an attached part for a message.
     * @param cid The attached part.
     * @return The attachment identified by content-id CID.
     */
    GMimeObject* getRelatedPart(QString cid);

signals:

public slots:

private:
    QString fileName;
    GMimeMessage* message;
};

#endif // HTMLMAIL_H
