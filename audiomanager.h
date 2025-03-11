#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include "audioclient.h"
#include "audioserver.h"

class AudioManager : public QObject
{
    Q_OBJECT

public:
    explicit AudioManager(quint16 serverPort = 45678,
                          const QString &remoteIp = "",
                          quint16 remotePort = 0,
                          const QString &clientName = "",
                          QObject *parent = nullptr);
    ~AudioManager();

    // Méthodes de contrôle serveur
    void startServer();
    void stopServer();

    // Méthodes de contrôle client
    void connectToServer(const QString &serverIp, quint16 port);
    void startStreaming();
    void stopStreaming();
    void setClientName(const QString &name);

private:
    AudioClient *client;
    AudioServer *server;

    bool clientEnabled;
    bool serverEnabled;
};

#endif // AUDIOMANAGER_H