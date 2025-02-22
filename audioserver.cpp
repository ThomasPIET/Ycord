#include "audioserver.h"

QByteArray audioBuffer;

// Constructeur du serveur audio
AudioServer::AudioServer(quint16 port, QObject *parent)
    : QObject(parent), tcpServer(new QTcpServer(this)), clientSocket(nullptr)
{
    // Configuration du périphérique de sortie audio
    outputDevice = QMediaDevices::defaultAudioOutput();
    format.setSampleRate(16000);                 // Fréquence d'échantillonnage à 16kHz
    format.setChannelCount(1);                   // Audio mono
    format.setSampleFormat(QAudioFormat::Int16); // Format 16 bits

    // Vérification si le format audio est supporté
    if (!outputDevice.isFormatSupported(format))
    {
        qWarning() << "Format non supporté, utilisation du format par défaut.";
        format = outputDevice.preferredFormat();
    }

    // Création du sink audio pour la lecture
    // QAudioSink est une classe Qt qui permet de lire des données audio
    audioSink = new QAudioSink(outputDevice, format);

    // Démarrage du serveur TCP sur le port spécifié
    if (!tcpServer->listen(QHostAddress::Any, port))
    {
        qCritical() << "Impossible de démarrer le serveur :" << tcpServer->errorString();
        return;
    }

    // Connection du signal de nouvelle connexion
    connect(tcpServer, &QTcpServer::newConnection, this, &AudioServer::NewConnection);
}

// Destructeur : nettoyage des ressources
AudioServer::~AudioServer()
{
    stopServer();
    delete audioSink;
    delete tcpServer;
}

// Démarre le serveur s'il n'est pas déjà en écoute
void AudioServer::startServer()
{
    if (!tcpServer->isListening())
    {
        tcpServer->listen(QHostAddress::Any, tcpServer->serverPort());
    }
}

// Arrête le serveur et ferme les connexions
void AudioServer::stopServer()
{
    if (clientSocket)
    {
        clientSocket->close();
    }
    tcpServer->close();
}

// Gère les nouvelles connexions entrantes
void AudioServer::NewConnection()
{
    clientSocket = tcpServer->nextPendingConnection();

    if (clientSocket)
    {
        qDebug() << "🔌 Nouveau client connecté depuis" << clientSocket->peerAddress().toString();
        connect(clientSocket, &QTcpSocket::readyRead, this, &AudioServer::readAudioData);
    }
    else
    {
        qDebug() << "⚠️ Erreur : Aucune connexion en attente.";
    }
}

// Traite les données audio reçues du client
void AudioServer::readAudioData()
{
    // Récupération du socket qui a envoyé les données
    QTcpSocket *senderClient = qobject_cast<QTcpSocket *>(sender());
    if (!senderClient)
        return;

    // Lecture des données tant qu'il y en a de disponibles
    while (senderClient->bytesAvailable() > 0)
    {
        QByteArray audioData = senderClient->readAll();

        if (!audioData.isEmpty())
        {
            qDebug() << "🔊 Paquet reçu de" << senderClient->peerAddress().toString()
                     << ", taille =" << audioData.size() << " octets";

            const int MAX_BUFFER_SIZE = 320000; // 320000 octets = 16000 échantillons à 16 bits
            audioBuffer.append(audioData);

            // Vérification de la taille du buffer pour éviter les débordements
            if (audioBuffer.size() > MAX_BUFFER_SIZE)
            {
                qWarning() << "⚠️ Buffer trop plein ! Suppression de" << (audioBuffer.size() - MAX_BUFFER_SIZE) << "octets.";
                audioBuffer.remove(0, audioBuffer.size() - MAX_BUFFER_SIZE);
            }

            // Initialisation du périphérique de sortie si nécessaire
            if (!device)
            {
                device = audioSink->start();
                qDebug() << "🎧 Lecture audio démarrée.";
            }

            const int CHUNK_SIZE = 3200; // Taille des morceaux audio à traiter (200ms à 16kHz)

            // Traitement du buffer par morceaux
            while (audioBuffer.size() >= CHUNK_SIZE)
            {
                QByteArray chunk = audioBuffer.left(CHUNK_SIZE);
                audioBuffer.remove(0, CHUNK_SIZE);

                device->write(chunk); // Envoi des données audio au périphérique de sortie
            }
        }
    }
}
