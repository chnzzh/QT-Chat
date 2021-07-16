#include "server.h"

Server::Server(QWidget *parent) : QWidget(parent)
{
    server = new QTcpServer(this);
    allClients = new QVector<QTcpSocket*>;
    allFriends = new QMap<QTcpSocket*, QString>;
    allFriendGroup = new QMap<QTcpSocket*, QString>;
    sqlFriend = new QMap<QVector<QString>, QVector<ChatInfo>>;
    sqlGroup = new QMap<QString, QVector<ChatInfo>>;
    friendChat = new QMap<QVector<QString>, QTcpSocket*>;
    groupChat = new QMap<QString, QVector<QTcpSocket*>>;
    flagFileTransfering = false;

    sqlFriendFile = new QMap<QVector<QString>, QVector<QString>>;

    server->setMaxPendingConnections(10);   // 最多连接数
    connect(server, SIGNAL(newConnection()), this, SLOT(newClientConnection()));

    if (server->listen(QHostAddress::Any, 8001))
    {
        qDebug() << "Server started on port 8001";
    }
    else
    {
        qDebug() << "ERR: Server can't start " + server->errorString();
    }

    groupsVector.push_back("世界频道");
    QVector<QTcpSocket*> vt;
    groupChat->insert("世界频道", vt);
    QVector<QString> vs;
    groupClients.insert("世界频道", vs);
}

void Server::sendHelloToClients()
{
    for (int i = 0; i < allClients->size(); i++)
    {
        QTcpSocket* client = allClients->at(i);
        if (client->isOpen() && client->isWritable())
        {
            client->write("Hello");
        }
    }
}

void Server::newClientConnection(void)
{
    QTcpSocket* client = server->nextPendingConnection();
    QString ip = client->peerAddress().toString();
    QString port = QString::number(client->peerPort());
    connect(client, &QTcpSocket::disconnected, this, &Server::socketDisconnected);
    connect(client, &QTcpSocket::readyRead, this, &Server::socketReadyRead);
    connect(client, &QTcpSocket::stateChanged, this, &Server::socketStateChanged);

    allClients->push_back(client);
    qDebug() << "Socket connected from " + ip + ":" + port;
}

void Server::socketDisconnected(void)
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(QObject::sender());
    QString socketIpAddress = client->peerAddress().toString();
    QString port = QString::number(client->peerPort());
    qDebug() << "Socket disconnected from " + socketIpAddress + ":" + port;
    // 如果断开的是home，需要将其从friends中排除
    if (allFriends->contains(client))
    {

        friendsVector.removeOne(allFriends->value(client));
        allFriends->remove(client);
        // 向所有好朋友发送更新的列表
        foreach (QTcpSocket* s, allFriends->keys()) {
            qDebug() << s->peerPort();
            sendFriendsVector(s);
        }
        return;
    }
    // 如果断开的为client，需要从friendChat中排除
    else if (friendChat->values().contains(client))
    {
        QVector<QString> k = friendChat->key(client);
        friendChat->remove(k);
        return;
    }
    // 断开的为群组对话
    else if (allFriendGroup->contains(client))
    {
        QString clientName = allFriends->value(client);
        allFriends->remove(client);
        foreach (QVector<QTcpSocket*> v, groupChat->values())
        {
            if (v.contains(client))
            {
                v.removeOne(client);
                break;
            }
        }
        foreach (QVector<QString> v, groupClients.values())
        {
            if (v.contains(clientName))
            {
                v.removeOne(clientName);
                break;
            }
        }
        return;
    }
}

void Server::socketReadyRead(void)
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(QObject::sender());
    QString socketIpAddress = client->peerAddress().toString();
    QString port = QString::number(client->peerPort());

    if (flagFileTransfering)
    {
        fileTransfer(client, fileTarget);
    }

    qint64 nextBufSize = 0;

    if (client->bytesAvailable() < (qint64)sizeof(qint64))
    {
        return;
    }
    QDataStream dataStream(client->read(sizeof (qint64)));
    dataStream >> nextBufSize;
    qDebug() << "bufsize:" << " + " << nextBufSize;

    QByteArray recvBuf = client->read(nextBufSize);

    ChatInfo ci(recvBuf);
    ci.createOriPacket();
    qDebug() << "Message: " + ci.sendText + " (" + socketIpAddress + ":" + port + ")" + ci.sendTime + ci.sendUserName;

    // 更新列表请求
    if (ci.type == "REQ")
    {
        // 如果不在列表中，将其加入
        if(!allFriends->contains(client))
        {
            allFriends->insert(client, ci.sendUserName);
            friendsVector.push_back(ci.sendUserName);

            sendGroupsVector(client);
            // 向所有好朋友发送列表
            foreach (QTcpSocket* s, allFriends->keys()) {
                qDebug() << s->peerPort();
                sendFriendsVector(s);
            }
        }
        // 如果在列表中，只向其发送列表就可以
        else
        {
            sendFriendsVector(client);
        }
        qDebug() << "send list to" << client->peerPort();
    }
    // 注册client窗口
    else if (ci.type == "INIT_FRIEND_CHAT")
    {
        QVector<QString> a2b;
        a2b.push_back(ci.sendUserName);
        a2b.push_back(ci.target);
        if (!friendChat->contains(a2b))
        {
            qDebug() << "注册新窗口";
            // 是新窗口
            friendChat->insert(a2b, client);
            sendHistory(a2b);
        }
    }
    else if (ci.type == "INIT_GROUP_CHAT")
    {
        QString groupName = ci.target;
        groupChat->find(groupName)->push_back(client);
        groupClients.find(groupName)->push_back(ci.sendUserName);
        allFriendGroup->insert(client, ci.sendUserName);
    }
    // 个人聊天
    else if (ci.type == "FRIEND_MSG")
    {
        qDebug() << ci.sendUserName + "to" + ci.target + ":" +ci.sendText;
        QVector<QString> a2b, b2a;
        a2b.push_back(ci.sendUserName);
        a2b.push_back(ci.target);
        b2a.push_back(ci.target);
        b2a.push_back(ci.sendUserName);

        // 向数据库写入信息
        if (!sqlFriend->contains(a2b))
        {
            QVector<ChatInfo> infoVector;
            infoVector.push_back(ci);
            sqlFriend->insert(a2b, infoVector);
            sqlFriend->insert(b2a, infoVector);
        }
        else {
            QMap<QVector<QString>, QVector<ChatInfo>>::Iterator it;
            it = sqlFriend->find(a2b);
            it.value().push_back(ci);
            it = sqlFriend->find(b2a);
            it.value().push_back(ci);
        }

        qDebug() << sqlFriend->find(a2b).value().size();
        // 发送给sender和target
        if (client->isOpen() && client->isWritable())
        {
            client->write(ci.sendBuf);
        }

        if (friendChat->contains(b2a))
        {
            QTcpSocket* target = friendChat->value(b2a);
            if (target->isOpen() && target->isWritable())
            {
                target->write(ci.sendBuf);
            }
        }

    }

    else if (ci.type == "GROUP_MSG")
    {
        qDebug() << ci.sendUserName + "to" + ci.target + ":" +ci.sendText;
        QString groupName = ci.target;

        // 向数据库写入信息
        if (!sqlGroup->contains(groupName))
        {
            QVector<ChatInfo> infoVector;
            infoVector.push_back(ci);
            sqlGroup->insert(groupName, infoVector);
            sqlGroup->insert(groupName, infoVector);
        }
        else {
            sqlGroup->find(groupName).value().push_back(ci);
        }

        // 发送给群组中的人
        foreach (QTcpSocket* socket, groupChat->value(ci.target))
        {
            if (socket->isOpen() && socket->isWritable())
            {
                socket->write(ci.sendBuf);
            }
        }

    }
    else if (ci.type == "FRIEND_FILE_HEAD")
    {
        QVector<QString> b2a;
        b2a.push_back(ci.target);
        b2a.push_back(ci.sendUserName);
        if (friendChat->contains(b2a))
        {
            flagFileTransfering = true;
            fileSize = ci.fileLength;
            haveSent = 0;
            fileTarget = friendChat->value(b2a);
            fileTarget->write(ci.sendBuf);
            fileTransfer(client, fileTarget);
        }
    }
    // 向所有人发送收到的信息
    /*for (int i = 0; i < allClients->size(); i++)
    {
        QTcpSocket* client = allClients->at(i);
        if (client->isOpen() && client->isWritable())
        {
            client->write(recvBuf);
        }
    }*/
}

void Server::socketStateChanged(QAbstractSocket::SocketState state)
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(QObject::sender());
    QString socketIpAddress = client->peerAddress().toString();
    int port = client->peerPort();
    QString desc;
    // simply print out a relevant message according to its new state
    if (state == QAbstractSocket::UnconnectedState)
        desc = "The socket is not connected.";
    else if (state == QAbstractSocket::HostLookupState)
        desc = "The socket is performing a host name lookup.";
    else if (state == QAbstractSocket::ConnectingState)
        desc = "The socket has started establishing a connection.";
    else if (state == QAbstractSocket::ConnectedState)
        desc = "A connection is established.";
    else if (state == QAbstractSocket::BoundState)
        desc = "The socket is bound to an address and port.";
    else if (state == QAbstractSocket::ClosingState)
        desc = "The socket is about to close (data may still be waiting to be written).";
    else if (state == QAbstractSocket::ListeningState)
        desc = "For internal use only.";
    qDebug() << "Socket state changed (" + socketIpAddress + ":" + QString::number(port) + "): " + desc;
}

void Server::sendFriendsVector(QTcpSocket* client)
{
    ChatInfo ci;
    ci.createVectorPacket("FRIENDS", friendsVector);
    if (client->isOpen() && client->isWritable())
    {
        client->write(ci.sendBuf);
    }
}

void Server::sendGroupsVector(QTcpSocket* client)
{
    ChatInfo ci;
    ci.createVectorPacket("GROUPS", groupsVector);
    if (client->isOpen() && client->isWritable())
    {
        client->write(ci.sendBuf);
    }
}

void Server::sendHistory(QVector<QString> a2b)
{
    if (sqlFriend->contains(a2b) && sqlFriend->value(a2b).size())
    {
        qDebug() << "有历史信息 ";
        QTcpSocket *client = friendChat->value(a2b);
        for (int i = 0; i < sqlFriend->value(a2b).size(); i++) {
            while(true)
            {
                if (client->isOpen() && client->isWritable())
                {
                    client->write(sqlFriend->value(a2b).at(i).sendBuf);
                    break;
                }
                QThread::msleep(10);
            }

            qDebug() << i << sqlFriend->value(a2b).at(i).sendBuf.size() << sqlFriend->value(a2b).at(i).sendBuf;
        }
    }
}

void Server::fileTransfer(QTcpSocket* source, QTcpSocket* target)
{
    QByteArray fileBuf;
    qDebug() << "-----------------------------" << source->bytesAvailable();
    fileBuf = source->readAll();
    haveSent += fileBuf.size();
    target->write(fileBuf);
    qDebug() << haveSent;
    if (haveSent >= fileSize)
    {
        flagFileTransfering = false;
        qDebug() << fileBuf;
    }
}
