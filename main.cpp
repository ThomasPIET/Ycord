#include "mainwindow.h"
#include "login.h"
#include "settings.h"

#include <QApplication>
#include <QLocale>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    settings s;
    MainWindow w;
    login l;

    QSettings settings("YCord","Data");
    QString savedUsername = settings.value("username").toString();

    if (savedUsername != ""){
        l.set_saved_name(savedUsername);
    }

    QObject::connect(&l,&login::connected,[&](QString name){
        settings.setValue("username",name);
        l.close();
        w.show();
        s.show();
    });

    l.show();
    return a.exec();
}
