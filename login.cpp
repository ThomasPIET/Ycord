#include "login.h"
#include "ui_login.h"

login::login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::login)
{
    ui->setupUi(this);
    this->setWindowTitle("YCord - Connect");
}

login::~login()
{
    delete ui;
}

void login::on_pushButton_clicked()
{
    QLineEdit *nameInput = ui->name;
    QLineEdit *ipInput = ui->ip;
    emit connected(nameInput->text(),ipInput->text());
}

void login::set_saved_name(QString name)
{
    QLineEdit *nameInput = ui->name;
    nameInput->setText(name);
}
