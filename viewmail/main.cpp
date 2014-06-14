#include "mailview.h"
#include "htmlmail.h"
#include <QApplication>

#include <vmime/vmime.hpp>
#include <vmime/platforms/posix/posixHandler.hpp>

#include <iostream>
#include <fstream>

#include <QtNetwork>
#include <QtCore>
#include <QtWebKit>

using vmime::utility::ref;

int main(int argc, char *argv[])
{
    vmime::platform::setHandler<vmime::platforms::posix::posixHandler>();

    QApplication a(argc, argv);

    MailView w;
    w.show();

    HTMLMailMessage* msg = new HTMLMailMessage(&w);
    if (argc <= 1) {
        msg->load(std::cin);
    } else {
        msg->load(argv[1]);
    }
    w.setMessage(msg);

    return a.exec();
}
