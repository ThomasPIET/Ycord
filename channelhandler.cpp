#include "channelhandler.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>

#include <QWebSocket>
#include <QEventLoop>
#include <QDebug>
#include <QRandomGenerator>
#include <QTimer>

ChannelHandler::ChannelHandler() {}

// Garde en mémoire le nom eet l@ IP
void ChannelHandler::setInfos(QString username, QString ip) {
    this->username = username;
    this->ip = ip;

    this->port = this->getRandomPort();
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
            qDebug() << "Un client s'est connecté au serveur WebSocket local.";

            QObject::connect(clientSocket, &QWebSocket::textMessageReceived, [&](const QString &message) {
                qDebug() << "Message reçu du client:" << message;
                emit newMessage(message);
            });

            QObject::connect(clientSocket, &QWebSocket::disconnected, [clientSocket]() {
                qDebug() << "Le client a été déconnecté.";
                clientSocket->deleteLater();
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

    QObject::connect(&channelSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), [&](QAbstractSocket::SocketError error) {
        qDebug() << "Erreur WebSocket :" << channelSocket.errorString();
    });

    channelSocket.open(QUrl("ws://" + currentChannel));
}

void ChannelHandler::requestMessage(QString msg){
    if (currentChannel != ""){
        channelSocket.sendTextMessage(msg);
        emit newMessage(msg);
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

        if (doc.isArray()) {
            QJsonArray clientsArray = doc.array();

            QStringList clientList;
            for (const QJsonValue &value : clientsArray) {
                if (value.isString()) {
                    clientList.append(value.toString());
                }
            }

            this->clients = clientList;
            emit clientListUpdated(this->clients);
        } else {
            qDebug() << "Le message reçu n'est pas un tableau JSON valide.";
        }
    });


    QObject::connect(&mainServerSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), [&](QAbstractSocket::SocketError error) {
        qDebug() << "Erreur WebSocket :" << mainServerSocket.errorString();
    });

    mainServerSocket.open(QUrl("ws://" + this->ip + ":8080"));
}


// Envoie le port du serveur
void ChannelHandler::sendPort() {
    if (mainServerSocket.state() == QAbstractSocket::ConnectedState) {
        mainServerSocket.sendTextMessage("sendPort " + QString::number(this->port));
    }
}

// Recup les clients sur le réseau
void ChannelHandler::getClients() {
    if (mainServerSocket.state() == QAbstractSocket::ConnectedState) {
        mainServerSocket.sendTextMessage("getUsers");
    }
}
