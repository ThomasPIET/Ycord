#include "uihandler.h"
#include <QDebug>
#include <QPushButton>


UiHandler::UiHandler() {
    qDebug() << "Test";
}

void UiHandler::init(){
    this->l.show();

    // Connect with username and IP
    QObject::connect(&l, &login::connected, [this](QString name, QString ip) {
        this->l.hide();
        this->m.show();

        m.setUser(name);
        c.setInfos(name,ip);

        c.init();
    });

    // Handle new channels
    QObject::connect(&c, &ChannelHandler::clientListUpdated, [this](QStringList clients) {
        this->m.clearUsers();
        for (const QString &ip : clients) {
            this->m.addUser(ip);
        }
    });

    // Handle channel switch to send messages
    QObject::connect(&m, &MainWindow::channelSelected, [this](QString channel) {
        m.clearMessages();
        m.setChannelName(channel);
        c.switchChannel(channel);
    });

    // Handles message requests
    QObject::connect(&m, &MainWindow::messageRequest, [this](QString msg) {
        c.requestMessage(c.getUser() +": " +msg);
    });

    // Shows message on the ui
    QObject::connect(&c, &ChannelHandler::newMessage, [this](QString msg, QString src) {
        if (src == c.identity || src == c.currentChannel){
            m.displayMessage(msg);
        }
    });
}
