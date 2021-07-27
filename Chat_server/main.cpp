#include <QApplication>
#include "server.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Server* myserver;
    myserver = new Server();

    return a.exec();
}
