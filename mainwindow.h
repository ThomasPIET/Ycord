#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setUser(QString username);
    void addUser(QString ip);
    void clearUsers();
    void setChannelName(QString name);
    void displayMessage(QString message);
    void clearMessages();
    void updateCallText(QString msg);
    QString getCallText();
private slots:
    void on_msgBox_textChanged();
    void on_callBtn_clicked();

private:
    Ui::MainWindow *ui;

signals:
    void channelSelected(QString ip);
    void messageRequest(QString msg);
    void callButtonPressed();
};
#endif // MAINWINDOW_H
