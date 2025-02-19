#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>

namespace Ui {
class login;
}

class login : public QWidget
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr);
    void set_saved_name(QString name);
    ~login();

private slots:
    void on_pushButton_clicked();

private:
    Ui::login *ui;

signals:
    void connected(QString name);
};

#endif // LOGIN_H
