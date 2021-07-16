#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QMessageBox>
#include "home.h"

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

    void setHome(Home*);


private slots:
    void on_pushButtonOK_clicked();

private:
    Ui::Login *ui;
    Home* h;

};

#endif // LOGIN_H
