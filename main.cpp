#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include "audiomanager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Chat Vocal Qt6");
    window.resize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(&window);

    QLabel *nameLabel = new QLabel("Nom du client :", &window);
    QLineEdit *nameInput = new QLineEdit(&window);
    nameInput->setPlaceholderText("Nom du client");
    nameInput->setText("Nom du client");

    QLabel *ipLabel = new QLabel("Adresse IP du serveur :", &window);
    QLineEdit *ipInput = new QLineEdit(&window);
    ipInput->setPlaceholderText("127.0.0.1");
    ipInput->setText("127.0.0.1");

    QLabel *portLabel = new QLabel("Port :", &window);
    QLineEdit *portInput = new QLineEdit(&window);
    portInput->setPlaceholderText("12345");
    portInput->setText("12345");

    QPushButton *serverButton = new QPushButton("Démarrer le serveur", &window);
    QPushButton *clientButton = new QPushButton("Démarrer le client", &window);
    QPushButton *bothButton = new QPushButton("Démarrer serveur et client", &window);
    QPushButton *stopButton = new QPushButton("Arrêter", &window);

    layout->addWidget(nameLabel);
    layout->addWidget(nameInput);
    layout->addWidget(ipLabel);
    layout->addWidget(ipInput);
    layout->addWidget(portLabel);
    layout->addWidget(portInput);
    layout->addWidget(serverButton);
    layout->addWidget(clientButton);
    layout->addWidget(bothButton);
    layout->addWidget(stopButton);

    // Créer une instance unique d'AudioManager
    AudioManager *audioManager = new AudioManager();

    QObject::connect(serverButton, &QPushButton::clicked, [&]()
                     {
        quint16 port = portInput->text().toUShort();
        
        // Réinitialiser AudioManager pour fonctionner en mode serveur uniquement
        delete audioManager;
        audioManager = new AudioManager(port);
        
        qDebug() << "🖥️ Serveur lancé sur le port" << port; });

    QObject::connect(clientButton, &QPushButton::clicked, [&]()
                     {
        QString ip = ipInput->text();
        quint16 port = portInput->text().toUShort();
        QString clientName = nameInput->text();
        
        // Réinitialiser AudioManager pour fonctionner en mode client uniquement
        delete audioManager;
        audioManager = new AudioManager(0, ip, port, clientName);
        
        qDebug() << "🎤 Client connecté à" << ip << ":" << port << "avec le nom" << clientName; });

    QObject::connect(bothButton, &QPushButton::clicked, [&]()
                     {
        QString ip = ipInput->text();
        quint16 port = portInput->text().toUShort();
        QString clientName = nameInput->text();
        
        // Réinitialiser AudioManager pour fonctionner en mode combiné (serveur + client)
        delete audioManager;
        audioManager = new AudioManager(port, ip, port, clientName);
        
        qDebug() << "🔄 Mode combiné : serveur sur le port" << port << "et client connecté à" << ip << ":" << port; });

    QObject::connect(stopButton, &QPushButton::clicked, [&]()
                     {
        // Nettoyer et réinitialiser
        delete audioManager;
        audioManager = new AudioManager();
        qDebug() << "🛑 Serveur et client arrêtés."; });

    window.setLayout(layout);
    window.show();
    return app.exec();
}