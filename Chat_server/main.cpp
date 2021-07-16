#include "mainwindow.h"

#include <QApplication>
#include "server.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // MainWindow w;
    // w.show();
    Server* myserver;
    myserver = new Server();

    return a.exec();
}
