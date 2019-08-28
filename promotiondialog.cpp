#include "promotiondialog.h"
#include "ui_promotiondialog.h"
#include <QDebug>

promotionDialog::promotionDialog(QString side, int row, int col, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::promotionDialog),
    side(side), row(row), col(col)
{
    ui->setupUi(this);

    setWindowTitle("Promotion");
    ui->queenPic->setPixmap(QPixmap::fromImage
               (QImage(":/pic/pic/" + side + "_queen.png").scaled
                (60,60,Qt::KeepAspectRatio)));
    ui->rookPic->setPixmap(QPixmap::fromImage
               (QImage(":/pic/pic/" + side + "_rook.png").scaled
                (60,60,Qt::KeepAspectRatio)));
    ui->knightPic->setPixmap(QPixmap::fromImage
               (QImage(":/pic/pic/" + side + "_knight.png").scaled
                (60,60,Qt::KeepAspectRatio)));
    ui->bishopPic->setPixmap(QPixmap::fromImage
               (QImage(":/pic/pic/" + side + "_bishop.png").scaled
                (60,60,Qt::KeepAspectRatio)));
    connect(ui->queenButton, SIGNAL(clicked()), this, SLOT(queenPushed()));
    connect(ui->rookButton, SIGNAL(clicked()), this, SLOT(rookPushed()));
    connect(ui->knightButton, SIGNAL(clicked()), this, SLOT(knightPushed()));
    connect(ui->bishopButton, SIGNAL(clicked()), this, SLOT(bishopPushed()));
}

promotionDialog::~promotionDialog() {
    delete ui;
}

void promotionDialog::queenPushed() {
    emit returnAns("queen", row, col);
    accept();
}

void promotionDialog::rookPushed() {
    emit returnAns("rook", row, col);
    accept();
}

void promotionDialog::knightPushed() {
    emit returnAns("knight", row, col);
    accept();
}

void promotionDialog::bishopPushed() {
    emit returnAns("bishop", row, col);
    accept();
}
