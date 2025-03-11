#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QHostAddress>
#include <QTcpSocket>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("YCord - Chat");
    this->setWindowState(Qt::WindowMaximized);

    socket = new QTcpSocket(this);
    // Connecter le bouton "Envoyer" à la fonction d'envoi du message
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::on_sendButton_clicked);
    // Connecter le socket pour recevoir les messages du serveur
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onMessageReceived);
    // Connexion au serveur
    socket->connectToHost(QHostAddress::LocalHost, 1234);

}

// Définition du destructeur pour libérer les ressources

MainWindow::~MainWindow()
{
    delete ui;
    delete socket; // Libérer le socket

}

void MainWindow::on_sendButton_clicked()
{
    QString message = ui->msgBox->toPlainText(); // messageInput est un QLineEdit dans le .ui
    if (!message.isEmpty()) {
        socket->write(message.toUtf8());
        qDebug() << "Message envoyé: " << message;
        ui->msgBox->clear(); // Effacer le champ de saisie après l'envoi
    }
}
void MainWindow::onMessageReceived()
{
    QByteArray message = socket->readAll();
    qDebug() << "Message reçu: " << message;
}


