#include "Server.h"

Server::Server(QWidget *parent) : QWidget(parent)
{
    server = new QTcpServer(this);
    textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);

    QPushButton *startButton = new QPushButton("Démarrer le Serveur", this);
    connect(startButton, &QPushButton::clicked, this, &Server::startServer);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(startButton);
    layout->addWidget(textEdit);
    setLayout(layout);

    setWindowTitle("Serveur de Chat");
    resize(400, 300);
}

void Server::startServer()
{
    if (!server->listen(QHostAddress::Any, 1234)) {
        textEdit->append("Erreur : Impossible de démarrer le serveur !");
        return;
    }

    connect(server, &QTcpServer::newConnection, this, &Server::onNewConnection);
    textEdit->append("Serveur en écoute sur le port 1234...");
}

void Server::onNewConnection()
{
    QTcpSocket *clientSocket = server->nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead, this, &Server::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);

    textEdit->append("Un client s'est connecté !");
}

void Server::onReadyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (clientSocket) {
        QByteArray data = clientSocket->readAll();
        textEdit->append("Message reçu : " + QString(data));

        // Renvoie le message aux autres clients
        for (QTcpSocket *client : server->findChildren<QTcpSocket *>()) {
            if (client != clientSocket) {
                client->write(data);
            }
        }
    }
}
