#include "audioclient.h"

/*
 * Constructeur de la classe AudioClient.
 * Gère la capture audio depuis le périphérique d'entrée par défaut et son envoi vers un serveur via TCP.
 *
 * Paramètres :
 *  - ip: Adresse IP du serveur destinataire.
 *  - port: Port d'écoute du serveur.
 *  - parent: Pointeur vers l'objet parent pour la gestion de la mémoire (optionnel).
 */
AudioClient::AudioClient(const QString &ip, quint16 port, const QString &iClientName, QObject *parent)
    : QObject(parent), serverIp(ip), serverPort(port), clientName(iClientName)
{

    inputDevice = QMediaDevices::defaultAudioInput();

    format.setSampleRate(48000);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Float);

    if (!inputDevice.isFormatSupported(format))
    {
        qWarning() << "Format audio non supporté, utilisation du format préféré.";
        format = inputDevice.preferredFormat();
    }

    audioSource = new QAudioSource(inputDevice, format);

    tcpSocket = new QTcpSocket(this);

    connect(tcpSocket, &QTcpSocket::connected, this, [&]()
            {
                qDebug() << "✅ Connexion réussie au serveur" << serverIp << ":" << serverPort << "avec le nom" << clientName;
                if (!clientName.isEmpty())
                {
                    qDebug() << "🔌 Nom du client envoyé au serveur:" << clientName;
                    setClientName(clientName, tcpSocket);
               }

                startStreaming(); });

    connect(tcpSocket, &QTcpSocket::errorOccurred, this, [&](QAbstractSocket::SocketError error)
            {
                Q_UNUSED(error);
                qWarning() << "⚠️ Erreur de connexion TCP:" << tcpSocket->errorString(); });

    qDebug() << "📡 Tentative de connexion à" << serverIp << ":" << serverPort;

    tcpSocket->connectToHost(serverIp, serverPort);
}

/*
 * Destructeur de AudioClient.
 * Arrête le streaming audio et libère les ressources associées.
 */
AudioClient::~AudioClient()
{
    stopStreaming();
    delete audioSource;
}

void AudioClient::setClientName(const QString &name, QTcpSocket *socket)
{
    clientName = name;

    if (socket->state() == QAbstractSocket::ConnectedState)
    {
        QByteArray namePacket;
        namePacket.append("CLIENT_NAME:" + clientName.toUtf8());
        socket->write(namePacket);
        qDebug() << "🔌 Nom du client envoyé au serveur:" << clientName;
    }
}

/*
 * Démarre la capture audio et l'envoi des données vers le serveur.
 *
 * Ce procédé consiste à :
 *  - Vérifier que la connexion TCP est bien établie.
 *  - Démarrer la capture audio via QAudioSource.
 *  - Connecter le signal readyRead du périphérique d'entrée pour lire et transmettre dès que des données sont disponibles.
 */
void AudioClient::startStreaming()
{
    if (tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        device = audioSource->start();
        const int CHUNK_SIZE = 64;

        QObject::connect(device, &QIODevice::readyRead, [this, CHUNK_SIZE]()
                         {
            sendBuffer.append(device->readAll());
            
          while (sendBuffer.size() >= CHUNK_SIZE) 
            {
                QByteArray packet;
                QDataStream stream(&packet, QIODevice::WriteOnly);
            /*  
            Définit l'ordre des octets du flux de données en tant que "Little Endian", 
            ce qui signifie que les octets les moins significatifs sont stockés en premier. 
            Cela est important pour assurer la compatibilité lors de l'écriture et de la lecture des données audio 
            */
                stream.setByteOrder(QDataStream::LittleEndian);
                QByteArray chunk = sendBuffer.left(CHUNK_SIZE);
                stream << chunk;
                qDebug() << "🎤 Envoi du paquet audio de taille" << chunk.size() << "octets" << "au serveur" << serverIp << ":" << serverPort << "avec le nom" << clientName;
                tcpSocket->write(packet);
                sendBuffer.remove(0, CHUNK_SIZE);
            } });

        tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
        qDebug() << "🎤 Streaming audio vers" << serverIp << ":" << serverPort;
    }
}

void AudioClient::stopStreaming()
{
    audioSource->stop();
    tcpSocket->close();
    qDebug() << "⏹️ Streaming audio arrêté.";
}
