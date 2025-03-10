#include "audioserver.h"

QByteArray audioBuffer;

AudioServer::AudioServer(quint16 port, QObject *parent)
    : QObject(parent), tcpServer(new QTcpServer(this)), clientSocket(nullptr)
{

    /*
    Configuration du périphérique de sortie audio :
    Format audio : 16 bits, 16 kHz, mono
    */
    outputDevice = QMediaDevices::defaultAudioOutput();
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    if (!outputDevice.isFormatSupported(format))
    {
        qWarning() << "Format non supporté, utilisation du format par défaut.";
        format = outputDevice.preferredFormat();
    }

    /*
    Création du sink audio pour la lecture
    QAudioSink est une classe Qt qui permet de lire des données audio
    */
    audioSink = new QAudioSink(outputDevice, format);

    if (!tcpServer->listen(QHostAddress::Any, port))
    {
        qCritical() << "Impossible de démarrer le serveur :" << tcpServer->errorString();
        return;
    }

    connect(tcpServer, &QTcpServer::newConnection, this, &AudioServer::NewConnection);
}

AudioServer::~AudioServer()
{
    stopServer();
    delete audioSink;
    delete tcpServer;
}

void AudioServer::startServer()
{
    if (!tcpServer->isListening())
    {
        tcpServer->listen(QHostAddress::Any, tcpServer->serverPort());
    }
}

void AudioServer::stopServer()
{
    if (clientSocket)
    {
        clientSocket->close();
    }
    tcpServer->close();
}

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

void AudioServer::readAudioData()
{
    QTcpSocket *senderClient = qobject_cast<QTcpSocket *>(sender());
    if (!senderClient)
        return;

    while (senderClient->bytesAvailable() > 0)
    {
        QByteArray audioData = senderClient->readAll();

        if (!audioData.isEmpty())
        {
            qDebug() << "🔊 Paquet reçu de" << senderClient->peerAddress().toString()
                     << ", taille =" << audioData.size() << " octets";

            const int MAX_BUFFER_SIZE = 320000; // 320000 octets = 16000 échantillons à 16 bits
            audioBuffer.append(audioData);

            if (audioBuffer.size() > MAX_BUFFER_SIZE)
            {
                qWarning() << "⚠️ Buffer trop plein ! Suppression de" << (audioBuffer.size() - MAX_BUFFER_SIZE) << "octets.";
                audioBuffer.remove(0, audioBuffer.size() - MAX_BUFFER_SIZE);
            }

            if (!device)
            {
                device = audioSink->start();
                qDebug() << "🎧 Lecture audio démarrée.";
            }

            const int CHUNK_SIZE = 3200; // Taille des morceaux audio à traiter (200ms à 16kHz)

            while (audioBuffer.size() >= CHUNK_SIZE)
            {
                QByteArray chunk = audioBuffer.left(CHUNK_SIZE);
                audioBuffer.remove(0, CHUNK_SIZE);

                device->write(chunk); // Envoi des données audio au périphérique de sortie
            }
        }
    }
}
