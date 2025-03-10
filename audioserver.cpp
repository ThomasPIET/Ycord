#include "audioserver.h"

QByteArray audioBuffer;

AudioServer::AudioServer(quint16 port, QObject *parent)
    : QObject(parent), tcpServer(new QTcpServer(this)), clientSocket()
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
    foreach (QTcpSocket *socket, clientSocket)
    {
        if (socket)
        {
            socket->close();
            socket->deleteLater();
        }
    }
    clientSocket.clear();
    tcpServer->close();
}

void AudioServer::NewConnection()
{
    QTcpSocket *newSocket = tcpServer->nextPendingConnection();

    if (newSocket)
    {
        qDebug() << "🔌 Nouveau client connecté depuis" << newSocket->peerAddress().toString();
        connect(newSocket, &QTcpSocket::readyRead, this, &AudioServer::readAudioData);

        connect(newSocket, &QTcpSocket::disconnected, this, [this, newSocket]()
                {
            qDebug() << "🔌 Client déconnecté:" << newSocket->peerAddress().toString();
            clientSocket.removeOne(newSocket);
            clientNames.remove(newSocket);
            newSocket->deleteLater(); 
            qDebug() << "🔌 Nombre de clients connectés:" << clientSocket.size(); });

        clientSocket.append(newSocket);
        qDebug() << "🔌 Nombre de clients connectés:" << clientSocket.size();
    }
    else
    {
        qDebug() << "⚠️ Erreur : Aucune connexion en attente.";
    }
}

void AudioServer::readAudioData()
{

    QTcpSocket *senderClient = qobject_cast<QTcpSocket *>(sender());
    if (!senderClient || !clientSocket.contains(senderClient))
    {
        return;
    }

    while (senderClient->bytesAvailable() > 0)
    {
        QByteArray data = senderClient->readAll();

        if (data.startsWith("CLIENT_NAME:"))
        {
            QString clientName = data.mid(12);
            clientNames[senderClient] = clientName;
            qDebug() << "🔌 Client connecté:" << clientName;
            continue;
        }

        if (!data.isEmpty())
        {
            QString clientIdentifier = clientNames.value(senderClient, "Inconnu");

            qDebug() << "🔊 Paquet reçu de" << clientIdentifier
                     << ", taille =" << data.size() << " octets";

            const int MAX_BUFFER_SIZE = 320000;
            audioBuffer.append(data);

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

            const int CHUNK_SIZE = 3200;

            while (audioBuffer.size() >= CHUNK_SIZE)
            {
                QByteArray chunk = audioBuffer.left(CHUNK_SIZE);
                audioBuffer.remove(0, CHUNK_SIZE);

                device->write(chunk);
            }
        }
    }
}