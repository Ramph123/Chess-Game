#include "clientgameconfig.h"
#include "ui_clientgameconfig.h"

clientGameConfig::clientGameConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::clientGameConfig)
{
    ui->setupUi(this);
    setWindowTitle("Game Configuration");
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(accept()));
}

clientGameConfig::~clientGameConfig() {
    delete ui;
}

void clientGameConfig::setSide(bool side) {
    QString thisSide = (side)?"White":"Black";
    ui->sideLabel->setText(thisSide);
}
void clientGameConfig::setTimeLimit(bool enable, int time) {
    if(!enable)
        ui->timeLabel->setText("No time limit");
    else {
        ui->timeLabel->setText(QString::number(time) + " seconds");
    }
}
