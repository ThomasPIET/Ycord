#include "channelhandler.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>

#include <QWebSocket>
#include <QEventLoop>
#include <QDebug>
#include <QRandomGenerator>
#include <QTimer>

#include <QJsonObject>

ChannelHandler::ChannelHandler() {}

// Garde en mémoire le nom eet l@ IP
void ChannelHandler::setInfos(QString username, QString ip) {
    this->username = username;
    this->ip = ip;

    this->port = this->getRandomPort();
    this->callPort = this->getRandomPort();
}

int ChannelHandler::getRandomPort() {
    return QRandomGenerator::global()->bounded(1024, 65536);
}

QString ChannelHandler::getUser(){
    return this->username;
}

// Initialise le socket vers le serveur source et le self-host tcp
void ChannelHandler::init() {
    qDebug() << "Nom d'utilisateur:" << this->username;
    qDebug() << "IP du serveur:" << this->ip;

    this->initSelfSocket();
    this->initServerSocket();
}

    void ChannelHandler::initSelfSocket() {
        this->selfSocketServer = new QWebSocketServer("Self WebSocket Server", QWebSocketServer::NonSecureMode, this);

        if (selfSocketServer->listen(QHostAddress::Any, this->port)) {
            qDebug() << "Serveur WebSocket local démarré sur le port" << this->port;

            QObject::connect(selfSocketServer, &QWebSocketServer::newConnection, this, [&]() {
                QWebSocket *clientSocket = selfSocketServer->nextPendingConnection();

                QObject::connect(clientSocket, &QWebSocket::textMessageReceived, [&,clientSocket](const QString &message) {
                    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());

                    if (doc.isObject()) {
                        QJsonObject jsonObj = doc.object();
                        QString action = jsonObj["action"].toString();

                        if (action == "message"){
                            QString msg = jsonObj["message"].toString();
                            QString src = jsonObj["source"].toString();
                            emit newMessage(msg,src);
                        }else if (action == "isHostingCall"){
                            QJsonObject jsonObj;
                            jsonObj["action"] = "hostState";
                            jsonObj["source"] = this->identity;
                            jsonObj["callTo"] = this->hostingCall;
                            jsonObj["port"] = this->callPort;

                            QJsonDocument doc(jsonObj);
                            QByteArray jsonData = doc.toJson();

                            clientSocket->sendTextMessage(QString(jsonData));
                        }
                    }

                });
            });
        } else {
            qDebug() << "Impossible de démarrer le serveur WebSocket local sur le port" << this->port;
        }
    }

void ChannelHandler::switchChannel(QString ip){
    this->currentChannel = ip;
    qDebug() << "switched channel " + QString("ws://" + ip);

    if (channelSocket.state() == QAbstractSocket::ConnectedState) {
        channelSocket.disconnect();
    }

    QObject::connect(&channelSocket, &QWebSocket::connected, [&]() {
        qDebug() << "Connecté a " << this->currentChannel;
    });

    QObject::connect(&channelSocket, &QWebSocket::textMessageReceived, [&](const QString &message) {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (doc.isObject()) {
            QJsonObject jsonObj = doc.object();
            QString action = jsonObj["action"].toString();

            if (action == "hostState"){
                QString callTo = jsonObj["callTo"].toString();
                QString src = jsonObj["source"].toString();
                int callPort = jsonObj["port"].toInt();

                if (callTo == ""){
                    qDebug() << "Host";
                    this->hostingCall = this->identity;

                    this->server = new AudioServer(this->callPort);
                    this->server->startServer();
                }else{
                    qDebug() << "Client" << callPort;
                    this->client = new AudioClient(src.split(":").at(0),callPort,this->getUser());
                }
            }
        }
    });

    QObject::connect(&channelSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), [&](QAbstractSocket::SocketError error) {
        qDebug() << "Erreur WebSocket :" << channelSocket.errorString();
    });

    channelSocket.open(QUrl("ws://" + currentChannel));
}

void ChannelHandler::closeCall(){
    this->hostingCall = "";

    if (this->server != nullptr){
        this->server->stopServer();
        delete this->server;
        this->server = nullptr;
    }

    if (this->client != nullptr){
        this->client->stopStreaming();
        delete this->client;
        this->client = nullptr;
    }
}

void ChannelHandler::attemptCall() {
    channelSocket.sendTextMessage("{\"action\":\"isHostingCall\"}");
}

void ChannelHandler::requestMessage(QString msg) {
    if (!currentChannel.isEmpty() && this->identity != "") {
        QJsonObject jsonObj;
        jsonObj["action"] = "message";
        jsonObj["message"] = msg;
        jsonObj["source"] = this->identity;

        QJsonDocument doc(jsonObj);
        QByteArray jsonData = doc.toJson();

        channelSocket.sendTextMessage(QString(jsonData));
        emit newMessage(msg,this->identity);
    }
}

void ChannelHandler::initServerSocket() {
    QObject::connect(&mainServerSocket, &QWebSocket::connected, [&]() {
        qDebug() << "Connecté au serveur WebSocket principal";
        this->sendPort();
        this->getClients();

        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &ChannelHandler::getClients);
        timer->start(5000);
    });

    QObject::connect(&mainServerSocket, &QWebSocket::textMessageReceived, [&](const QString &message) {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (doc.isObject()) {
            QJsonObject jsonObj = doc.object();
            QString action = jsonObj["action"].toString();

            if (action == "users"){
                    QJsonArray clientsArray = jsonObj["users"].toArray();
                    QStringList clientList;
                    for (const QJsonValue &value : clientsArray) {
                        if (value.isString()) {
                            clientList.append(value.toString());
                        }
                    }
                    this->clients = clientList;
                    emit clientListUpdated(this->clients);
            }else if (action == "identity"){
                QString _id = jsonObj["identity"].toString();
                this->identity = _id;
                qDebug() << "Updated identity " + _id;
            }
        }
    });


    QObject::connect(&mainServerSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), [&](QAbstractSocket::SocketError error) {
        qDebug() << "Erreur WebSocket :" << mainServerSocket.errorString();
    });

    mainServerSocket.open(QUrl("ws://" + this->ip + ":8080"));
}


void ChannelHandler::sendPort() {
    if (mainServerSocket.state() == QAbstractSocket::ConnectedState) {
        mainServerSocket.sendTextMessage("{\"action\": \"setPort\", \"port\": \"" + QString::number(this->port) + "\"}");
    }
}

// Recup les clients sur le réseau
void ChannelHandler::getClients() {
    if (mainServerSocket.state() == QAbstractSocket::ConnectedState) {
        mainServerSocket.sendTextMessage("{\"action\": \"getUsers\"}");
    }
}
