#ifndef HOME_H
#define HOME_H

#include "client.h"

#include <QMainWindow>
#include <QStringListModel>

namespace Ui {
class Home;
}

class Home : public QMainWindow
{
    Q_OBJECT

public:
    explicit Home(QWidget *parent = nullptr);
    ~Home();

    void recvUserName(QString recvedUserName);
    void initHome();

private slots:
    void socketConnected(void);
    void socketDisconnected(void);
    void socketReadyRead(void);
    void socketReConnect(void);

    void on_pushButtonRetry_clicked();

    void on_pushButtonRefresh_clicked();

    void on_listViewFriends_doubleClicked(const QModelIndex &index);

    void on_listViewGroups_doubleClicked(const QModelIndex &index);

private:
    Ui::Home *ui;

    QTcpSocket *socket;
    QString userName;
    bool flagConnected;
    QString address = "127.0.0.1";
    int port = 8001;
    QStringListModel *friendsModel;
    QStringListModel *groupsModel;
    qint64 nextBufSize = 0;

    //聊天窗口
    Client *chatWindow;
};

#endif // HOME_H
