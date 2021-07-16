#include "home.h"
#include "ui_home.h"

Home::Home(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Home)
{
    ui->setupUi(this);
    friendsModel = new QStringListModel(this);
    groupsModel = new QStringListModel(this);
    ui->listViewFriends->setModel(friendsModel);
    ui->listViewGroups->setModel(groupsModel);
}

Home::~Home()
{
    delete ui;
}

void Home::recvUserName(QString recvedUserName)
{
    userName = recvedUserName;
    ui->labelHello->setText("Hello, " + userName);
    this->setWindowTitle("主界面（" + userName + "）");
}

void Home::socketConnected(void)
{
    QString msg = "Connected to server.";
    ui->lineEditState->setText(msg);
    flagConnected = true;
    ui->pushButtonRetry->setEnabled(false);
}

void Home::socketDisconnected(void)
{
    QString msg = "Disconnected from server.";
    ui->lineEditState->setText(msg);
    flagConnected = false;
    ui->pushButtonRetry->setEnabled(true);
    friendsModel->removeRows(0, friendsModel->rowCount());
    groupsModel->removeRows(0, groupsModel->rowCount());
}

void Home::socketReadyRead(void)
{
    if (socket->bytesAvailable() < (qint64)sizeof(qint64))
    {
        return;
    }
    QDataStream dataStream(socket->read(sizeof (qint64)));
    dataStream >> nextBufSize;
    qDebug() << "bufsize:" << nextBufSize;

    while (socket->bytesAvailable() < nextBufSize)
    {
        QThread::msleep(10);
    }

    QByteArray recvBuf = socket->read(nextBufSize);
    ChatInfo ci(recvBuf);
    QString msg;
    msg.append("("+ci.sendUserName+") :");
    msg.append(ci.type);
    ui->lineEditState->setText(msg);
    if (ci.target == "FRIENDS")
    {
        QStringList qsl;
        for (int i = 0; i < ci.sendVector.size(); i++) {
            if (ci.sendVector.at(i) == userName)
                qsl += ci.sendVector.at(i) + "(我)";
            else
                qsl += ci.sendVector.at(i);
        }
        friendsModel->setStringList(qsl);
        qDebug() << "recvFriends" << ci.sendTime;
    }
    else if (ci.target == "GROUPS")
    {
        QStringList qsl;
        for (int i = 0; i < ci.sendVector.size(); i++) {
                qsl += ci.sendVector.at(i);
        }
        groupsModel->setStringList(qsl);
        qDebug() << "recvGroups" << ci.sendTime;
    }
    if(socket->bytesAvailable() >= (qint64)sizeof(qint64))
        socketReadyRead();
}

void Home::socketReConnect(void)
{
    if(!flagConnected)
    {
        socket->connectToHost(address, port);
    }
    if(!socket->waitForConnected(2000))
    {
        QString msg = "Server not online! Please try again.";
        ui->lineEditState->setText(msg);
        ui->pushButtonRetry->setEnabled(true);
    }
    else
    {
        ChatInfo ci;
        ci.createREQPacket(userName);
        socket->write(ci.sendBuf);
    }
}

void Home::on_pushButtonRetry_clicked()
{
    socketReConnect();
}

void Home::initHome()
{
    flagConnected = false;
    // 这个socket用于交换朋友和组列表信息
    socket = new QTcpSocket();
    connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
    socket->connectToHost(address, port);
    if (!socket->waitForConnected(2000))
    {
        QString msg = "Server not online! Please try again.";
        ui->lineEditState->setText(msg);
        ui->pushButtonRetry->setEnabled(true);
        ui->pushButtonRefresh->setEnabled(false);
    }
    else
    {
        // 连接到server，向server发送一条请求
        ui->pushButtonRetry->setEnabled(false);
        ChatInfo ci;
        ci.createREQPacket(userName);
        socket->write(ci.sendBuf);
        qDebug() << ci.sendBuf.size() << ci.sendBuf;
    }
}

void Home::on_pushButtonRefresh_clicked()
{
    ChatInfo ci;
    ci.createREQPacket(userName);
    socket->write(ci.sendBuf);
    qDebug() << "resend REQ";
}

void Home::on_listViewFriends_doubleClicked(const QModelIndex &index)
{
    qDebug() << index.data();
    chatWindow = new Client();
    chatWindow->setWindowTitle(index.data().toString());
    chatWindow->initClient(userName, index.data().toString(), "FRIEND");
    chatWindow->setWindowModality(Qt::ApplicationModal);
    if(chatWindow->exec() == QDialog::Rejected)
    {
        delete chatWindow;
    }
}

void Home::on_listViewGroups_doubleClicked(const QModelIndex &index)
{
    qDebug() << "注册群聊窗口" << index.data();
    chatWindow = new Client();
    chatWindow->setWindowTitle(index.data().toString());
    chatWindow->initClient(userName, index.data().toString(), "GROUP");
    chatWindow->setWindowModality(Qt::ApplicationModal);
    chatWindow->exec();
}
