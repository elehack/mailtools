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

    for (int i = 1; i < argc; i++) {
        HTMLMailMessage* msg = new HTMLMailMessage();
        msg->load(argv[i]);
        auto root = msg->getBody();
        std::cout <<argv[i] <<": ";
        if (root) {
            std::cout <<root->getHeader()->ContentType()->getValue()->generate() <<std::endl;
        } else {
            std::cout <<"cannot find root (ctype: "
                        <<msg->getMessage()->getHeader()->ContentType()->getValue()->generate()
                          <<")\n";
        }
        delete msg;
    }

    MailView w;
    w.show();

    return a.exec();
}
