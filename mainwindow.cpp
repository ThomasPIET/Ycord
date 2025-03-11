#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("YCord - Chat");
    this->setWindowState(Qt::WindowMaximized);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setUser(QString user){
    QLabel *localUser = ui->localUser;
    localUser->setText(user);
}

void MainWindow::clearUsers(){
    QList<QPushButton *> buttons = ui->users->findChildren<QPushButton*>();

    for (QPushButton *button : buttons) {
        delete button;
    }
}

void MainWindow::setChannelName(QString name){
    QLabel *localUser = ui->channelName;
    localUser->setText(name);
}

void MainWindow::clearMessages() {
    QLayout* layout = ui->msgArea->layout();
    QLayoutItem* item;

    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}


void MainWindow::displayMessage(QString message) {
    QLabel* label = new QLabel(message);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignLeft);
    ui->msgArea->layout()->addWidget(label);
}

void MainWindow::addUser(QString ip){
    QPushButton *button = new QPushButton(ip, this);

    connect(button, &QPushButton::clicked, [this, ip]() {
        emit channelSelected(ip);
    });

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(ui->users->layout());
    if (layout) {
        layout->addWidget(button);
    }
}

void MainWindow::on_msgBox_textChanged()
{
    QTextEdit *box = ui->msgBox;
    QString text = box->toPlainText();

    if (text.contains("\n")){
        box->setText("");
        emit messageRequest(text);
    }
}


void MainWindow::on_callBtn_clicked()
{
    emit callButtonPressed();
}

void MainWindow::updateCallText(QString msg){
    QPushButton *btn = this->ui->callBtn;
    btn->setText(msg);
}

QString MainWindow::getCallText(){
    QPushButton *btn = this->ui->callBtn;
    return btn->text();
}
