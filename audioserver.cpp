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
    format.setSampleRate(48000);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Float);
    // Ajout de logs de débogage
    qDebug() << "🎵 Périphérique audio par défaut:" << outputDevice.description();
    qDebug() << "🎵 Format demandé:";
    qDebug() << "   - Fréquence:" << format.sampleRate();
    qDebug() << "   - Canaux:" << format.channelCount();
    qDebug() << "   - Format:" << format.sampleFormat();

    if (!outputDevice.isFormatSupported(format))
    {
        qWarning() << "⚠️ Format non supporté, utilisation du format par défaut.";
        format = outputDevice.preferredFormat();
        qDebug() << "🎵 Nouveau format:";
        qDebug() << "   - Fréquence:" << format.sampleRate();
        qDebug() << "   - Canaux:" << format.channelCount();
        qDebug() << "   - Format:" << format.sampleFormat();
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

            audioBuffer.append(data);
            /*
             Tant qu'on peut extraire un paquet complet (taille + données) du buffer,
             on dépile ces octets du buffer et on les envoie au périphérique audio.
             - On lit d'abord la taille du bloc (4 octets) : blockSize
             - Si on ne dispose pas encore de toutes les données du paquet (blockSize),
               on sort de la boucle, car on attendra d'autres octets (prochain readyRead()).
             - Sinon, on extrait le bloc audio et on l'écrit dans le QAudioSink.
             Cette boucle se répète tant qu'il reste au moins un bloc audio complet
             dans le buffer */
            while (true)
            {
                if (audioBuffer.size() < (int)sizeof(qint32))
                {
                    break;
                }

                QDataStream stream(&audioBuffer, QIODevice::ReadOnly);
                stream.setByteOrder(QDataStream::LittleEndian);
                qint32 blockSize = 0;
                stream >> blockSize;

                if (audioBuffer.size() < (int)sizeof(qint32) + blockSize)
                {
                    break;
                }

                audioBuffer.remove(0, sizeof(qint32));
                QByteArray rawAudioData = audioBuffer.left(blockSize);
                audioBuffer.remove(0, blockSize);

                if (!device)
                {
                    device = audioSink->start();
                    device = audioSink->start();
                    qDebug() << "🎧 Lecture audio démarrée.";
                    qDebug() << "🎧 État de l'audio sink:" << audioSink->state();
                    qDebug() << "🎧 Taille du buffer:" << audioSink->bufferSize();
                }
                device->write(rawAudioData);
            }
        }
    }
}