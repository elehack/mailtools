#include <gmime/gmime.h>

#include "mailview.h"
#include "htmlmail.h"
#include <QApplication>

#include <cstdio>

#include <QtNetwork>
#include <QtCore>
#include <QtWebKit>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    g_mime_init(0);

    MailView w;
    w.show();
    w.activateWindow();
    w.raise();

    HTMLMailMessage* msg = new HTMLMailMessage(&w);
    if (argc <= 1) {
        msg->load_stdin();
    } else {
        msg->load_file(argv[1]);
    }
    w.setMessage(msg);

    int rc = a.exec();

    g_mime_shutdown();
    return rc;
}
