#include "audiomanager.h"

AudioManager::AudioManager(quint16 serverPort, const QString &remoteIp,
                           quint16 remotePort, const QString &clientName,
                           QObject *parent)
    : QObject(parent), clientEnabled(false), serverEnabled(false)
{
    // Initialiser le serveur si un port est spécifié
    if (serverPort > 0)
    {
        server = new AudioServer(serverPort, this);
        server->startServer();
        serverEnabled = true;
        qDebug() << "🔊 AudioManager: Serveur démarré sur le port" << serverPort;
    }
    else
    {
        server = nullptr;
    }

    // Initialiser le client si une adresse IP est spécifiée
    if (!remoteIp.isEmpty() && remotePort > 0)
    {
        client = new AudioClient(remoteIp, remotePort, clientName, this);
        clientEnabled = true;
        qDebug() << "🎤 AudioManager: Client connecté à" << remoteIp << ":" << remotePort;
    }
    else
    {
        client = nullptr;
    }
}

AudioManager::~AudioManager()
{
    if (server)
    {
        server->stopServer();
    }

    // Les objets sont automatiquement détruits car ils sont enfants de cette classe (QObject)
}

// Méthodes de contrôle serveur
void AudioManager::startServer()
{
    if (server)
    {
        server->startServer();
    }
    else
    {
        qWarning() << "⚠️ AudioManager: Impossible de démarrer le serveur, non initialisé";
    }
}

void AudioManager::stopServer()
{
    if (server)
    {
        server->stopServer();
    }
}

// Méthodes de contrôle client
void AudioManager::connectToServer(const QString &serverIp, quint16 port)
{
    if (client)
    {
        // Pour reconnecter, il faut recréer l'objet AudioClient
        delete client;
    }

    client = new AudioClient(serverIp, port, "", this);
    clientEnabled = true;
}

void AudioManager::startStreaming()
{
    if (client)
    {
        client->startStreaming();
    }
    else
    {
        qWarning() << "⚠️ AudioManager: Impossible de démarrer le streaming, client non initialisé";
    }
}

void AudioManager::stopStreaming()
{
    if (client)
    {
        client->stopStreaming();
    }
}

void AudioManager::setClientName(const QString &name)
{
    if (client)
    {
        client->setClientName(name, nullptr); // Le socket est géré en interne dans AudioClient
    }
    else
    {
        qWarning() << "⚠️ AudioManager: Impossible de définir le nom du client, non initialisé";
    }
}