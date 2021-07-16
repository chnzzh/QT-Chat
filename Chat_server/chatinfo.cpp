#include "chatinfo.h"

ChatInfo::ChatInfo()
{

}

ChatInfo::ChatInfo(QByteArray &recvBuf)
{
    QDataStream tempDS(&recvBuf, QIODevice::ReadWrite);
    tempDS >> type;
    tempDS >> target;
    tempDS >> sendUserName;
    tempDS >> sendTime;

    if(type == "VECTOR")
        tempDS >> sendVector;
    else if (type == "FRIEND_FILE_HEAD")
    {
        tempDS >> fileLength;
        tempDS >> sendText;
    }
    else
        tempDS >> sendText;
}

void ChatInfo::createREQPacket(QString ciSendUserName)
{
    type = "REQ";
    target = "SERVER";
    sendUserName = ciSendUserName;
    sendTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    sendText = "Request for List";

    createOriPacket();
}

void ChatInfo::createVectorPacket(QString ciTarget, QVector<QString> ciVector)
{
    type = "VECTOR";
    target = ciTarget;
    sendUserName = "SERVER";
    sendTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    sendVector = ciVector;

    createOriPacket();
}

void ChatInfo::createFriendChatPacket(QString ciSendText, QString ciSendUserName, QString ciTarget)
{
    type = "FRIEND_MSG";
    target = ciTarget;
    sendUserName = ciSendUserName;
    sendTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    sendText = ciSendText;

    createOriPacket();
}

void ChatInfo::createGroupChatPacket(QString ciSendText, QString ciSendUserName, QString ciTarget)
{
    type = "GROUP_MSG";
    target = ciTarget;
    sendUserName = ciSendUserName;
    sendTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    sendText = ciSendText;

    createOriPacket();
}

void ChatInfo::createInitFriendChatPacket(QString ciSendUserName, QString ciTarget)
{
    type = "INIT_FRIEND_CHAT";
    target = ciTarget;
    sendUserName = ciSendUserName;
    sendTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    sendText = "Register the client windows";

    createOriPacket();
}

void ChatInfo::createInitGroupChatPacket(QString ciSendUserName, QString ciTarget)
{
    type = "INIT_GROUP_CHAT";
    target = ciTarget;
    sendUserName = ciSendUserName;
    sendTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    sendText = "Register the group windows";

    createOriPacket();
}

void ChatInfo::createOriPacket()
{
    sendBuf.clear();
    QDataStream tempDS(&sendBuf, QIODevice::ReadWrite);
    tempDS << (qint64) 0;
    tempDS << type;
    tempDS << target;
    tempDS << sendUserName;
    tempDS << sendTime;
    if(type == "VECTOR")
        tempDS << sendVector;
    else if (type == "FRIEND_FILE_HEAD")
    {
        tempDS << fileLength;
        tempDS << sendText;
    }
    else
        tempDS << sendText;
    tempDS.device()->seek(0);
    tempDS << (qint64) (sendBuf.size() - sizeof (qint64));
}

void ChatInfo::createFriendFileHeadPacket(qint64 ciFileLength, QString ciFileName, QString ciSendUserName, QString ciTarget)
{
    type = "FRIEND_FILE_HEAD";
    target = ciTarget;
    sendUserName = ciSendUserName;
    sendTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    sendText = ciFileName;
    fileLength = ciFileLength;

    createOriPacket();
}
