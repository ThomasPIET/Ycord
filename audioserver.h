#ifndef AUDIOSERVER_H
#define AUDIOSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QAudioSink>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QIODevice>
#include <QDebug>
#include <QObject>

class AudioServer : public QObject
{
    Q_OBJECT

public:
    explicit AudioServer(quint16 port, QObject *parent = nullptr);
    ~AudioServer();

    void startServer();
    void stopServer();

private slots:
    void NewConnection();
    void readAudioData();

private:
    QTcpServer *tcpServer;

    QList<QTcpSocket *> clientSocket;
    QAudioSink *audioSink;
    QIODevice *device;
    QAudioFormat format;
    QAudioDevice outputDevice;
};

#endif