#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

class Server : public QWidget
{
    Q_OBJECT

public:
    Server(QWidget *parent = nullptr);

private slots:
    void startServer();
    void onNewConnection();
    void onReadyRead();

private:
    QTcpServer *server;
    QTextEdit *textEdit;
};

#endif // CHATSERVER_H
