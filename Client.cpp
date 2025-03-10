#include "Client.h"

Client::Client(QWidget *parent) : QWidget(parent)
{
    socket = new QTcpSocket(this);

    // Zone d'affichage des messages
    textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);

    // Champ pour saisir un message
    msgBox = new QLineEdit(this);

    // Bouton pour envoyer le message
    QPushButton *sendButton = new QPushButton("Envoyer", this);
    connect(sendButton, &QPushButton::clicked, this, &Client::sendMessage);

    // Mise en page verticale
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(textEdit);
    layout->addWidget(msgBox);
    layout->addWidget(sendButton);
    setLayout(layout);

    setWindowTitle("Client de Chat");
    resize(400, 300);

    // Connexion au serveur
    socket->connectToHost("127.0.0.1", 1234);
    connect(socket, &QTcpSocket::connected, this, &Client::onConnected);
    connect(socket, &QTcpSocket::readyRead, this, &Client::onReadyRead);
}

void Client::onConnected()
{
    textEdit->append("✅ Connecté au serveur.");
}

void Client::onReadyRead()
{
    QByteArray data = socket->readAll();
    textEdit->append("💬 " + QString::fromUtf8(data));
}

void Client::sendMessage()
{
    QString message = msgBox->text().trimmed();
    if (!message.isEmpty()) {
        socket->write(message.toUtf8());
        textEdit->append("🧑 Vous : " + message);
        msgBox->clear();
    }
}
