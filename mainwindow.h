#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
   explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_sendButton_clicked();  // Slot pour l'événement de clic sur le bouton "Envoyer"
    void onMessageReceived();      // Slot pour recevoir les messages

private:
    Ui::MainWindow *ui;
    QTcpSocket *socket;

};
#endif // MAINWINDOW_H
