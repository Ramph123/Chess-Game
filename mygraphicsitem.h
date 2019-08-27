#ifndef MYGRAPHICSITEM_H
#define MYGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QBrush>

class MyGraphicsItem : public QGraphicsRectItem
{
public:
    MyGraphicsItem(int inputRow, int inputCol, QColor color, qreal x = -30, qreal y = -30, qreal width = 60, qreal height=60,QGraphicsItem *parent = nullptr);
    ~MyGraphicsItem();

    void setType() { type = ""; }
    void setType(QString side, QString chess) { type = side + "_" + chess; }
    void setMargin(QColor color) { marginColor = color; update(); }
    void setActiveness(bool in) { activeness = in; }

protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void mousePressEvent(QGraphicsSceneMouseEvent* event);

private:
    QString type;
    QColor bgdColor;
    QColor dark, light;
    QColor marginColor;
    int row, col;

    bool activeness;
};

#endif // MYGRAPHICSITEM_H
