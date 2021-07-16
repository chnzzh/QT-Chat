#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>
#include <QTcpSocket>
#include <QDebug>
#include "chatinfo.h"
#include <QHostAddress>
#include <QThread>
#include <QFileDialog>

namespace Ui {
class Client;
}

class Client : public QDialog
{
    Q_OBJECT

public:
    explicit Client(QDialog *parent = nullptr);
    ~Client();

    void initClient(QString selfName, QString chatTarget, QString chatType);
    void setAddressPort(QString, int);

private slots:

    void socketConnected(void);
    void socketDisconnected(void);
    void socketReadyRead(void);
    void socketReConnect(void);

    void on_pushButtonRetry_clicked();

    void on_pushButtonSend_clicked();

    void on_pushButtonBack_clicked();

    void on_pushButtonFile_clicked();

private:
    Ui::Client *ui;
    QTcpSocket* socket;
    bool flagConnected;
    bool flagFirstMsgBeenSent;
    QString userName;
    QString target;
    QString type;       // FRIEND / GROUP

    QString address = "127.0.0.1";
    int port = 8001;
    qint64 nextBufSize = 0;

    quint64 haveRecv = 0;   // 已经发送大小
    quint64 fileRecvSize = 0;   // 文件大小
    bool flagFileTransfering;
    void fileTransfer(QTcpSocket* source);

    QFile *recvFile;

};

#endif // CLIENT_H
