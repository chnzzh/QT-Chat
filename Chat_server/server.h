#ifndef SERVER_H
#define SERVER_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QDebug>

#include "chatinfo.h"
#include <QThread>

class Server : public QWidget
{
    Q_OBJECT
public:
    explicit Server(QWidget *parent = nullptr);
    void startServer(void);
    void sendHelloToClients(void);
    void sendHistory(QVector<QString> a2b);

public slots:
    void newClientConnection(void);
    void socketDisconnected(void);
    void socketReadyRead(void);
    void socketStateChanged(QAbstractSocket::SocketState state);

private:
    QTcpServer *server;
    QVector<QTcpSocket*>   *allClients;
    QMap<QTcpSocket*, QString> *allFriends;     // socket与home的对应
    QMap<QTcpSocket*, QString> *allFriendGroup;     // socket与client的对应
    QVector<QString> friendsVector;
    QMap<QVector<QString>, QTcpSocket*> *friendChat;

    QVector<QString> groupsVector;
    QMap<QString, QVector<QTcpSocket*>> *groupChat;
    QMap<QString, QVector<QString>> groupClients;

    QMap<QVector<QString>, QVector<ChatInfo>> *sqlFriend;
    QMap<QString, QVector<ChatInfo>> *sqlGroup;

    QMap<QVector<QString>, QVector<QString>> *sqlFriendFile;
    QMap<QString, QVector<ChatInfo>> *sqlGroupFile;

    void sendFriendsVector(QTcpSocket*);
    void sendGroupsVector(QTcpSocket*);

    void fileTransfer(QTcpSocket* source, QTcpSocket* target);
    quint64 haveSent = 0;   // 已经发送大小
    quint64 fileSize = 0;   // 文件大小
    QTcpSocket * fileTarget;

    bool flagFileTransfering;

signals:

};

#endif // SERVER_H
