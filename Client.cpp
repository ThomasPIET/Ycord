#include <QCoreApplication>
#include <QTcpSocket>
#include <QTextStream>
#include <QDebug>

class Client : public QObject {
    Q_OBJECT

public:
    Client(const QString &host, int port) {
        socket = new QTcpSocket(this);
        connect(socket, &QTcpSocket::readyRead, this, &Client::onDataReceived);
        connect(socket, &QTcpSocket::connected, this, &Client::onConnected);
        connect(socket, &QTcpSocket::disconnected, this, &Client::onDisconnected);

        socket->connectToHost(host, port);
    }

    void sendMessage(const QString &message) {
        if (socket->state() == QTcpSocket::ConnectedState) {
            socket->write(message.toUtf8());
        }
    }

private slots:
    void onConnected() {
        qDebug() << "Connected to server!";
    }

    void onDataReceived() {
        QByteArray message = socket->readAll();
        qDebug() << "Message from server:" << message;
    }

    void onDisconnected() {
        qDebug() << "Disconnected from server!";
    }

private:
    QTcpSocket *socket;
};

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    Client client("localhost", 1234);

    QTextStream inputStream(stdin);
    QString message;

    while (true) {
        qDebug() << "Enter message to send (or 'exit' to quit):";
        message = inputStream.readLine();
        if (message == "exit") {
            break;
        }
        client.sendMessage(message);
    }

    return a.exec();
}

#include "Client.moc"
