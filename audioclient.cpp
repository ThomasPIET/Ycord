#include "audioclient.h"

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
                 qDebug() << "✅ Connexion réussie au serveur"; 
                startStreaming(); });

    connect(tcpSocket, &QTcpSocket::errorOccurred, this, [&](QAbstractSocket::SocketError error)
            { qWarning() << "⚠️ Erreur de connexion TCP:" << tcpSocket->errorString(); });

    qDebug() << "📡 Tentative de connexion à " << serverIp << ":" << serverPort;

    tcpSocket->connectToHost(serverIp, serverPort);
}

AudioClient::~AudioClient()
{
    stopStreaming();
    delete audioSource;
}

void AudioClient::startStreaming()
{

    if (tcpSocket->state() == QAbstractSocket::ConnectedState)
    {

        device = audioSource->start();

        QObject::connect(device, &QIODevice::readyRead, [this]()
                         {
            QByteArray audioData = device->readAll();


            if (!audioData.isEmpty()) {
                tcpSocket->write(audioData);

            } });

        qDebug() << "🎤 Streaming audio vers " << serverIp << ":" << serverPort;
    }
    else
    {
        qWarning() << "⚠️ Impossible de se connecter au serveur.";
    }
}

void AudioClient::stopStreaming()
{
    audioSource->stop();
    tcpSocket->close();
    qDebug() << "⏹️ Streaming audio arrêté.";
}
