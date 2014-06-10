#include "mailview.h"
#include "htmlmail.h"
#include <QApplication>

#include <vmime/vmime.hpp>
#include <vmime/platforms/posix/posixHandler.hpp>

#include <iostream>
#include <fstream>

using vmime::utility::ref;

int main(int argc, char *argv[])
{
    vmime::platform::setHandler<vmime::platforms::posix::posixHandler>();

    QApplication a(argc, argv);

    if (argc > 1) {
        HTMLMailMessage* msg = new HTMLMailMessage();
        msg->load(argv[1]);
        msg->dump();
        delete msg;
    }

    MailView w;
    w.show();

    return a.exec();
}
