#include "client.h"
#include "ui_client.h"

Client::Client(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::Client)
{
    ui->setupUi(this);
}

void Client::initClient(QString selfName, QString chatTarget, QString chatType)
{
    userName = selfName;
    target = chatTarget;
    type = chatType;
    flagConnected = false;
    flagFirstMsgBeenSent = false;
    flagFileTransfering = false;

    socket = new QTcpSocket();
    connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
    socket->connectToHost(address, port);
    if (!socket->waitForConnected(2000))
    {
        QString msg = "<div style=\"color:red;\">";
        msg.append("Server not online! Please try again.");
        msg.append("</div>");
        ui->textBrowserRecv->append(msg);
        ui->pushButtonRetry->setEnabled(true);
    }
    else
    {
        ui->pushButtonRetry->setEnabled(false);
    }
    if (type == "GROUP")
    {
        ui->progressBar->hide();
        ui->pushButtonFile->hide();
    }

}

void Client::setAddressPort(QString targetAddress, int targetPort)
{
    address = targetAddress;
    port = targetPort;
}

Client::~Client()
{
    socket->disconnectFromHost();
    delete socket;
    delete ui;
}


void Client::socketConnected(void)
{
    QString msg = "<div style=\"color:green;\">";
    msg.append("Connected to server.");
    msg.append("</div>");
    ui->textBrowserRecv->append(msg);
    flagConnected = true;
    ui->pushButtonRetry->setEnabled(false);

    ChatInfo ci;
    if (type == "FRIEND")
    {
        ci.createInitFriendChatPacket(userName, target);
    }
    else if(type == "GROUP")
    {
        ci.createInitGroupChatPacket(userName, target);
    }
    // qDebug() << ci.sendBuf.size() << ci.sendBuf;
    socket->write(ci.sendBuf);
}

void Client::socketDisconnected(void)
{
    QString msg = "<div style=\"color:red;\">";
    msg.append("Disconnected from server.");
    msg.append("</div>");
    ui->textBrowserRecv->append(msg);
    flagConnected = false;
    ui->pushButtonRetry->setEnabled(true);
}

void Client::socketReadyRead(void)
{
    if (flagFileTransfering)
    {
        fileTransfer(socket);
    }

    if (socket->bytesAvailable() < (qint64)sizeof(qint64))
    {
        return;
    }
    QDataStream dataStream(socket->read(sizeof (qint64)));
    dataStream >> nextBufSize;
    // qDebug() << "bufsize:" << nextBufSize;

    while ((qint64)socket->bytesAvailable() < nextBufSize)
    {
        QThread::msleep(10);
    }

    QByteArray recvBuf = socket->read(nextBufSize);
    // qDebug() << recvBuf.size() << recvBuf;
    ChatInfo ci(recvBuf);
    // qDebug() << "RECVED" << ci.sendTime << ci.sendUserName << ci.target;
    QString msg;

    if (ci.type == "FRIEND_FILE_HEAD")
    {
        msg.append("<div style=\"color:pink;\">");
        msg.append("[收到文件]("+ci.sendUserName+") [" + ci.sendTime + "] :" + ci.sendText);
        msg.append("</div>");
        this->ui->textBrowserRecv->append(msg);
        haveRecv = 0;
        fileRecvSize = ci.fileLength;    // 文件大小
        flagFileTransfering = true;
        ui->progressBar->setMaximum(fileRecvSize);

        QString path = "./save/" + userName;
        // 文件夹创建
        QDir dir;
        if (!dir.exists(path))
        {
            bool res = dir.mkpath(path);
            // qDebug() << "新建文件夹" << res;
        }
        path += "/" + ci.sendText;

        if (path.isEmpty() == false){
                recvFile = new QFile();
                //关联文件名字
                recvFile->setFileName(path);
                recvFile->open(QIODevice::WriteOnly|QIODevice::Append);
            }

        if (socket->bytesAvailable() > 0)
            fileTransfer(socket);
    }
    else
    {
        if (ci.sendUserName == userName)
        {
            msg.append("<div style=\"color:black;font-weight:bold\">");
            msg.append("(我) [" + ci.sendTime + "] :" + ci.sendText);
        }
        else
        {
            msg.append("<div style=\"color:black;\">");
            msg.append("(" + ci.sendUserName+ ")[" + ci.sendTime + "] :" + ci.sendText);
        }
        msg.append("</div>");
        this->ui->textBrowserRecv->append(msg);
    }


    if((qint64)socket->bytesAvailable() >= (qint64)sizeof(qint64))
        socketReadyRead();
}

void Client::socketReConnect(void)
{
    if(!flagConnected)
    {
        socket->connectToHost(address, port);
    }
    if(!socket->waitForConnected(2000))
    {
        QString msg = "<div style=\"color:red;\">";
        msg.append("Server not online! Please try again.");
        msg.append("</div>");
        ui->textBrowserRecv->append(msg);
        ui->pushButtonRetry->setEnabled(true);
    }
}

void Client::on_pushButtonRetry_clicked()
{
    socketReConnect();
}

void Client::on_pushButtonSend_clicked()
{
    QString sendText = ui->textEditInput->toPlainText();
    if(!sendText.isEmpty())
    {
        /*if (!flagFirstMsgBeenSent)
        {
            QString msg = "<div style=\"color:blue;\">";
            msg.append("以上为历史信息");
            msg.append("</div>");
            flagFirstMsgBeenSent = true;
            ui->textBrowserRecv->append(msg);
        }*/
        QString ip = socket->localAddress().toString();
        QString port = QString::number(socket->localPort());

        ChatInfo ci;
        if (type == "FRIEND")
        {
            ci.createFriendChatPacket(sendText, userName, target);
        }
        else if (type == "GROUP")
        {
            ci.createGroupChatPacket(sendText, userName, target);
        }
        socket->write(ci.sendBuf);
    }
    ui->textEditInput->clear();
}

void Client::on_pushButtonBack_clicked()
{
    this->close();
}

void Client::on_pushButtonFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("上传文件"), "/home");
    // qDebug()<<"fileurl : "<< fileName;

    QFile *f = new QFile(fileName);
    if(!f->open(QFile::ReadOnly))
    {
        // qDebug() << "open file error!";
        return;
    }

    ChatInfo ci;
    ci.createFriendFileHeadPacket(f->size(), fileName.right(fileName.size()- fileName.lastIndexOf('/')-1), userName, target);

    socket->write(ci.sendBuf);

    // 下面开始传输文件
    quint64 bufSize = 4*1024;   // 每次发送的buf大小
    quint64 haveSent = 0;   // 已经发送大小
    quint64 fileSize = f->size();    // 文件大小
    QByteArray sendBuf;

    ui->progressBar->setMaximum(fileSize);
    ui->progressBar->setValue(haveSent);

    while (haveSent != fileSize) {
        sendBuf = f->read(qMin(bufSize, fileSize-haveSent));
        socket->write(sendBuf);
        haveSent += qMin(bufSize, fileSize-haveSent);
        ui->progressBar->setValue(haveSent);

        // qDebug() << haveSent;
    }
    // qDebug() << sendBuf << f->size();
    f->close();

    QString msg;
    msg.append("<div style=\"color:LightSkyBlue;\">");
    msg.append("[发送文件成功]("+ci.sendUserName+") [" + ci.sendTime + "] :" + ci.sendText);
    msg.append("</div>");
    this->ui->textBrowserRecv->append(msg);
}

void Client::fileTransfer(QTcpSocket* source)
{
    QByteArray fileBuf;
    // qDebug() << "-----------------------------" << source->bytesAvailable();
    fileBuf = source->readAll();
    //创建文件
    recvFile->write(fileBuf);
    haveRecv += fileBuf.size();
    // qDebug() << haveRecv;
    ui->progressBar->setValue(haveRecv);
    if (haveRecv >= fileRecvSize)
    {
        flagFileTransfering = false;
        // qDebug() << fileBuf;
        recvFile->close();

        QString msg;
        msg.append("<div style=\"color:lightseagreen;\">");
        msg.append("文件已保存到目录：" + recvFile->fileName() + "</div>");

        ui->textBrowserRecv->append(msg);
    }
}
