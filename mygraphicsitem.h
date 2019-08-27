#ifndef MYGRAPHICSITEM_H
#define MYGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QBrush>

class MyGraphicsItem : public QGraphicsRectItem
{
public:
    MyGraphicsItem(qreal x = -30, qreal y = -30, qreal width = 60, qreal height=60,QGraphicsItem *parent = nullptr);
    ~MyGraphicsItem();

    void setType(QString side, QString chess) {
        type = side + "_" + chess;
    }

protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QString type;
};

#endif // MYGRAPHICSITEM_H
