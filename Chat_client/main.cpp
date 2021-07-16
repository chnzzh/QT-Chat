#include "home.h"
#include "login.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Login l;
    Home h;
    l.setHome(&h);
    if(l.exec() == QDialog::Accepted)
    {
        h.initHome();
        h.show();
        return a.exec();
    }
    else
    {
        return 0;
    }

}
