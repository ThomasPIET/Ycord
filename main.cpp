#include "uihandler.h"

#include <QApplication>
#include <QLocale>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    UiHandler ui1;
    ui1.init();

    return a.exec();
}
