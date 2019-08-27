#include "mygraphicsitem.h"
#include <QPainter>
#include <QPen>
#include <QRectF>

MyGraphicsItem::MyGraphicsItem(qreal x, qreal y, qreal width, qreal height,QGraphicsItem *parent):
    QGraphicsRectItem(x,y,width,height,parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
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
    QGraphicsRectItem::paint(painter, option, widget);
    if(type != "")
        painter->drawImage(QRectF(-25,-25,50,50),QImage(":/pic/pic/" + type + ".png"));
}
