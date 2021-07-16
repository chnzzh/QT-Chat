#ifndef CHATINFO_H
#define CHATINFO_H

#include <QDateTime>
#include <QDataStream>
#include <QVector>

class ChatInfo
{
public:
    ChatInfo();
    ChatInfo(QByteArray &buf);
    // 用于home更新列表

    void createREQPacket(QString ciSendUserName);
    void createVectorPacket(QString ciType, QVector<QString> ciVector);

    void createInitFriendChatPacket(QString ciSendUserName, QString ciTarget);
    void createInitGroupChatPacket(QString ciSendUserName, QString ciTarget);
    void createFriendChatPacket(QString ciSendText, QString ciSendUserName, QString ciTarget);
    void createGroupChatPacket(QString ciSendText, QString ciSendUserName, QString ciTarget);
    void createFriendFileHeadPacket(qint64 ciFileLength, QString fileName, QString ciSendUserName, QString ciTarget);

    void createOriPacket();
    QString sendTime;
    QString sendText;
    QString sendUserName;
    QString target;
    QString type = "MSG";
    qint64 fileLength = 0;
    QVector<QString> sendVector;

    QByteArray sendBuf;

private:

};

#endif // CHATINFO_H
