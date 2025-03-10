#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QWidget>
#include <QTcpSocket>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

class Client : public QWidget
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);

private slots:
    void onConnected();
    void onReadyRead();
    void sendMessage();

private:
    QTcpSocket *socket;
    QTextEdit *textEdit;
    QLineEdit *msgBox;
};

#endif // CHATCLIENT_H
