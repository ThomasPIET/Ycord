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
    // Récupère le périphérique d'entrée audio par défaut
    inputDevice = QMediaDevices::defaultAudioInput();

    // Configuration du format audio :
    // - Fréquence d'échantillonnage : 16 kHz
    // - Mono pour réduire la taille des données et la complexité du traitement
    // - Format des échantillons : entier 16 bits
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    // Vérifie que le format choisi est supporté par le périphérique d'entrée.
    // Si ce n'est pas le cas, on passe au format préféré proposé par le périphérique.
    if (!inputDevice.isFormatSupported(format))
    {
        qWarning() << "Format audio non supporté, utilisation du format préféré.";
        format = inputDevice.preferredFormat();
    }

    // Création de la source audio pour la capture, qui va émettre des données audio selon le format configuré
    audioSource = new QAudioSource(inputDevice, format);

    // Création et configuration du socket TCP pour établir la connexion avec le serveur.
    tcpSocket = new QTcpSocket(this);

    // Connexion du signal de connexion réussie du socket à une lambda pour démarrer le streaming.
    connect(tcpSocket, &QTcpSocket::connected, this, [&]()
            {
                qDebug() << "✅ Connexion réussie au serveur";
                startStreaming(); });

    // Gestion des erreurs de connexion en affichant un message d'erreur explicite
    connect(tcpSocket, &QTcpSocket::errorOccurred, this, [&](QAbstractSocket::SocketError error)
            {
                Q_UNUSED(error);
                qWarning() << "⚠️ Erreur de connexion TCP:" << tcpSocket->errorString(); });

    qDebug() << "📡 Tentative de connexion à" << serverIp << ":" << serverPort;

    // Démarrage de la tentative de connexion au serveur spécifié.
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
    // Vérifie que le socket est en état connecté avant de démarrer le streaming.
    if (tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        // Démarre la capture audio et récupère le QIODevice associé pour accéder aux données audio.
        device = audioSource->start();

        const int CHUNK_SIZE = 3200;

        // Connecte le signal readyRead du périphérique d'entrée à une lambda.
        // Ce signal est émis dès que de nouvelles données audio sont disponibles.
        QObject::connect(device, &QIODevice::readyRead, [this, CHUNK_SIZE]()
                         {
            // Accumuler les données lues dans le buffer membre sendBuffer.
            sendBuffer.append(device->readAll());
            qDebug() << "🔊 Buffer de données audio accumulées:" << sendBuffer.size() << "octets";

            // Tant que le buffer contient au moins un chunk complet, on l'envoie.
            while (sendBuffer.size() >= CHUNK_SIZE)
            {
                QByteArray chunk = sendBuffer.left(CHUNK_SIZE);
                qDebug() << "🔊 Envoi d'un chunk de taille:" << chunk.size() << "octets";
                tcpSocket->write(chunk);
                sendBuffer.remove(0, CHUNK_SIZE);
            } });

        qDebug() << "🎤 Streaming audio vers" << serverIp << ":" << serverPort;
    }
    else
    {
        // Si la connexion n'est pas établie, on affiche un avertissement.
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
    audioSource->stop(); // Arrête la capture audio.
    tcpSocket->close();  // Ferme la connexion TCP.
    qDebug() << "⏹️ Streaming audio arrêté.";
}
