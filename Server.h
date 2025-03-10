#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QCoreApplication>
#include <QDebug>

class Server : public QTcpServer {
    Q_OBJECT

public:
    Server() {
        if (!this->listen(QHostAddress::Any, 1234)) {
            qCritical() << "Server failed to start!";
            return;
        }
        qDebug() << "Server started, waiting for connections...";
    }

protected:
    void incomingConnection(qintptr socketDescriptor) override {
        QTcpSocket *clientSocket = new QTcpSocket(this);
        if (!clientSocket->setSocketDescriptor(socketDescriptor)) {
            delete clientSocket;
            return;
        }
        connect(clientSocket, &QTcpSocket::readyRead, this, &Server::onDataReceived);
        connect(clientSocket, &QTcpSocket::disconnected, this, &Server::onClientDisconnected);
        clients.append(clientSocket);
        qDebug() << "Client connected!";
    }

private slots:
    void onDataReceived() {
        QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
        if (clientSocket) {
            QByteArray message = clientSocket->readAll();
            qDebug() << "Message received:" << message;
            broadcastMessage(message);
        }
    }

    void onClientDisconnected() {
        QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
        if (clientSocket) {
            clients.removeAll(clientSocket);
            clientSocket->deleteLater();
            qDebug() << "Client disconnected!";
        }
    }

    void broadcastMessage(const QByteArray &message) {
        for (QTcpSocket *client : qAsConst(clients)) {
            if (client->state() == QTcpSocket::ConnectedState) {
                client->write(message);
            }
        }
    }

private:
    QList<QTcpSocket *> clients;
};
