/* rm - rf build &&mkdir build &&cd build &&cmake..&&make &&./ ChatVocal */

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include "audioserver.h"
#include "audioclient.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Chat Vocal Qt6");
    window.resize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(&window);

    QLabel *ipLabel = new QLabel("Adresse IP du serveur :", &window);
    QLineEdit *ipInput = new QLineEdit(&window);
    ipInput->setPlaceholderText("127.0.0.1");

    QLabel *portLabel = new QLabel("Port :", &window);
    QLineEdit *portInput = new QLineEdit(&window);
    portInput->setPlaceholderText("12345");

    QPushButton *serverButton = new QPushButton("Démarrer le serveur", &window);
    QPushButton *clientButton = new QPushButton("Démarrer le client", &window);
    QPushButton *stopButton = new QPushButton("Arrêter", &window);

    layout->addWidget(ipLabel);
    layout->addWidget(ipInput);
    layout->addWidget(portLabel);
    layout->addWidget(portInput);
    layout->addWidget(serverButton);
    layout->addWidget(clientButton);
    layout->addWidget(stopButton);

    AudioServer *server = nullptr;
    AudioClient *client = nullptr;

    QObject::connect(serverButton, &QPushButton::clicked, [&]()
                     {
        quint16 port = portInput->text().toUShort();
        server = new AudioServer(port);
        server->startServer();
        qDebug() << "🖥️ Serveur lancé sur le port" << port; });

    QObject::connect(clientButton, &QPushButton::clicked, [&]()
                     {
        QString ip = ipInput->text();
        quint16 port = portInput->text().toUShort();
        client = new AudioClient(ip, port);
        qDebug() << "🎤 Client connecté à" << ip << ":" << port; });

    QObject::connect(stopButton, &QPushButton::clicked, [&]()
                     {
        if (server) {
            server->stopServer();
            delete server;
            server = nullptr;
            qDebug() << "🛑 Serveur arrêté.";
        }
        if (client) {
            client->stopStreaming();
            delete client;
            client = nullptr;
            qDebug() << "🛑 Client arrêté.";
        } });

    window.setLayout(layout);
    window.show();
    return app.exec();
}
