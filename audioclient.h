#ifndef AUDIOCLIENT_H
#define AUDIOCLIENT_H

#include <QTcpSocket>
#include <QAudioSource>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QIODevice>
#include <QDebug>
#include <QAudioFormat>
#include <QObject>

class AudioClient : public QObject
{
    Q_OBJECT

public:
    AudioClient(const QString &serverIp, quint16 port, const QString &clientName = "", QObject *parent = nullptr);
    ~AudioClient();

    void startStreaming();
    void stopStreaming();
    void setClientName(const QString &name, QTcpSocket *socket);

private:
    QTcpSocket *tcpSocket;
    QAudioSource *audioSource;
    QIODevice *device;
    QAudioFormat format;
    QAudioDevice inputDevice;
    QString serverIp;
    quint16 serverPort;
    QString clientName;
    QByteArray sendBuffer;
};
#endif
