#ifndef CHANNELHANDLER_H
#define CHANNELHANDLER_H

#include <QObject>
#include <QStringList>
#include <QWebSocket>
#include <QWebSocketServer>

#include "audioserver.h"
#include "audioclient.h"

class ChannelHandler : public QObject
{
    Q_OBJECT

public:
    ChannelHandler();
    void setInfos(QString username, QString ip);
    void init();
    void getClients();
    void initServerSocket();
    void initSelfSocket();
    void sendPort();
    int getRandomPort();
    QString currentChannel;
    void switchChannel(QString ip);
    void requestMessage(QString msg);
    QString getUser();
    QString identity;
    QString hostingCall;
    void attemptCall();
    void closeCall();
    int callPort;

    AudioServer *server = nullptr;
    AudioClient *client = nullptr;
signals:
    void clientListUpdated(QStringList clients);
    void newMessage(QString msg,QString src);

private:
    QString username;
    QString ip;
    QStringList clients;
    int port;

    QWebSocket mainServerSocket;
    QWebSocket channelSocket;
    QWebSocketServer *selfSocketServer;
};

#endif // CHANNELHANDLER_H
