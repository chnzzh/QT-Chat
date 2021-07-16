#include "login.h"
#include "ui_login.h"

Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    this->setWindowTitle("提示");
}

Login::~Login()
{
    delete ui;
}

void Login::setHome(Home* home)
{
    h = home;
}

void Login::on_pushButtonOK_clicked()
{
    if (ui->lineEditName->text().isEmpty())
    {
        QMessageBox::information(NULL, "提示", "昵称为空", QMessageBox::Yes, QMessageBox::Yes);
    }
    else
    {
        h->recvUserName(ui->lineEditName->text());
        accept();
    }
}
