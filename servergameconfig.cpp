#include "servergameconfig.h"
#include "ui_servergameconfig.h"
#include <QDebug>

serverGameConfig::serverGameConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::serverGameConfig)
{
    ui->setupUi(this);

    setWindowTitle("Game Configuration");
    ui->spinBox->setDisabled(true);
    ui->blackButton->setChecked(true);
    ui->whiteButton->setChecked(false);
    side = 0;
    limitEnable = 0;
    timeLimit = 30;
    connect(ui->blackButton, SIGNAL(clicked()), this, SLOT(blackClicked()));
    connect(ui->whiteButton, SIGNAL(clicked()), this, SLOT(whiteClicked()));
    connect(ui->checkBox, SIGNAL(stateChanged(int)), this, SLOT(checkboxChanged(int)));
    connect(ui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(timeChanged(int)));
    connect(ui->acceptButton, SIGNAL(clicked()), this, SLOT(sendSignal()));
}

serverGameConfig::~serverGameConfig() {
    delete ui;
}

void serverGameConfig::blackClicked() {
    ui->whiteButton->setChecked(false);
    side = 0;
}

void serverGameConfig::whiteClicked() {
    ui->blackButton->setChecked(false);
    side = 1;
}

void serverGameConfig::checkboxChanged(int state) {
    //qDebug() << "!!!!!";
    limitEnable = state;
    ui->spinBox->setEnabled(limitEnable);
}

void serverGameConfig::timeChanged(int time) {
    timeLimit = time;
}

void serverGameConfig::sendSignal() {
    QString sig = QString::number(side) + "-" + QString::number(limitEnable) + "-" + QString::number(timeLimit);
    emit gameConfigResult(sig);
    accept();
}
