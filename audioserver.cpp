#include "audioserver.h"

AudioServer::AudioServer(quint16 port, QObject *parent)
    : QObject(parent), tcpServer(new QTcpServer(this)), clientSocket(nullptr)
{
    outputDevice = QMediaDevices::defaultAudioOutput();
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    if (!outputDevice.isFormatSupported(format))
    {
        qWarning() << "Format non supporté, utilisation du format par défaut.";
        format = outputDevice.preferredFormat();
    }

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
    if (!clientSocket)
        return;

    while (clientSocket->bytesAvailable() > 0)
    {
        QByteArray audioData = clientSocket->readAll();

        if (!audioData.isEmpty())
        {
            qDebug() << "🔊 Paquet audio reçu : taille =" << audioData.size() << "octets de " << clientSocket->peerAddress().toString();

            if (!device)
            {
                device = audioSink->start();
            }
            device->write(audioData);
        }
    }
}
