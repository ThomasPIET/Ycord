#include "login.h"
#include "mainwindow.h"
#include "channelhandler.h"

#ifndef UIHANDLER_H
#define UIHANDLER_H

class UiHandler
{
public:
    UiHandler();
    void init();
private:
    login l;
    MainWindow m;
    ChannelHandler c;
};

#endif // UIHANDLER_H
