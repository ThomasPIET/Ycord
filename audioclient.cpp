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
AudioClient::AudioClient(const QString &ip, quint16 port, QObject *parent)
    : QObject(parent), serverIp(ip), serverPort(port)
{
    inputDevice = QMediaDevices::defaultAudioInput();

    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    if (!inputDevice.isFormatSupported(format))
    {
        qWarning() << "Format audio non supporté, utilisation du format préféré.";
        format = inputDevice.preferredFormat();
    }

    audioSource = new QAudioSource(inputDevice, format);

    tcpSocket = new QTcpSocket(this);

    connect(tcpSocket, &QTcpSocket::connected, this, [&]()
            {
                qDebug() << "✅ Connexion réussie au serveur" << serverIp << ":" << serverPort;
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

        const int CHUNK_SIZE = 3200;

        /*
        Connecte le signal readyRead du périphérique d'entrée à une lambda.
        Ce signal est émis dès que de nouvelles données audio sont disponibles.
        */
        QObject::connect(device, &QIODevice::readyRead, [this, CHUNK_SIZE]()
                         {
            sendBuffer.append(device->readAll());
/*             qDebug() << "🔊 Buffer de données audio accumulées:" << sendBuffer.size() << "octets";
 */
            while (sendBuffer.size() >= CHUNK_SIZE)
            {
                QByteArray chunk = sendBuffer.left(CHUNK_SIZE);
/*                 qDebug() << "🔊 Envoi d'un chunk de taille:" << chunk.size() << "octets";
 */                tcpSocket->write(chunk);
                sendBuffer.remove(0, CHUNK_SIZE);
            } });

        qDebug() << "🎤 Streaming audio vers" << serverIp << ":" << serverPort;
    }
    else
    {

        qWarning() << "⚠️ Impossible de se connecter au serveur.";
    }
}

/*
 * Arrête la capture audio et ferme la connexion TCP.
 *
 * Cette méthode :
 *  - Stoppe la source audio pour interrompre la capture.
 *  - Ferme le socket TCP afin de libérer la connexion réseau.
 */
void AudioClient::stopStreaming()
{
    audioSource->stop();
    tcpSocket->close();
    qDebug() << "⏹️ Streaming audio arrêté.";
}
