#include "mygraphicsitem.h"
#include <QPainter>
#include <QPen>
#include <QRectF>
#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>

MyGraphicsItem::MyGraphicsItem(int inputRow, int inputCol, QColor color, qreal x, qreal y, qreal width, qreal height,QGraphicsItem *parent):
    QGraphicsRectItem(x,y,width,height,parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
    bgdColor = color;
    marginColor = Qt::transparent;
    row = inputRow;
    col = inputCol;
    activeness = false;
}

MyGraphicsItem::~MyGraphicsItem()
{

}

QRectF MyGraphicsItem::boundingRect() const
{
    return QRectF(-30, -30, 60, 60);
}

void MyGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
//    Q_UNUSED(option)
//    Q_UNUSED(widget)
//    QBrush brush(QColor(200,125,125));
//    painter->setBrush(brush);
//    painter->drawRect(-100, -100, 200, 200);
    QPen pen(Qt::black, 2);
    //painter->setPen(pen);
    QGraphicsRectItem::setPen(pen);
    QGraphicsRectItem::setBrush(QBrush(bgdColor));
    QGraphicsRectItem::paint(painter, option, widget);
    QPen newpen(marginColor, 5);
    painter->setPen(newpen);
    painter->drawRect(-28,-28,56,56);
    painter->setPen(pen);
    if(type != "")
        painter->drawImage(QRectF(-25,-25,50,50),QImage(":/pic/pic/" + type + ".png"));
}

void MyGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if(!activeness)
        return;
    if(event->button() == Qt::LeftButton) {
//        if(marginColor != Qt::transparent) {
//            marginColor = Qt::transparent;
//        }
//        else {
//            marginColor = Qt::blue;
//        }
//        update();
        emit checkerClicked(row, col);
    }
}
