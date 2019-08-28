#ifndef MYGRAPHICSITEM_H
#define MYGRAPHICSITEM_H

#include <QObject>
#include <QGraphicsItem>
#include <QBrush>

class MyGraphicsItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    MyGraphicsItem(int inputRow, int inputCol, QColor color, qreal x = -30, qreal y = -30, qreal width = 60, qreal height=60,QGraphicsItem *parent = nullptr);
    ~MyGraphicsItem();

    void setChess() { color = ""; type = ""; }
    void setChess(QString side, QString chess) { color = side; type = chess; }
    void setMargin(QColor color) { marginColor = color; update(); }
    void setActiveness(bool in) { activeness = in; }
    QString getSide() { return color; }
    QString getType() { return type; }
    QColor getMargin() { return marginColor; }

signals:
    void checkerClicked(int, int);

protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void mousePressEvent(QGraphicsSceneMouseEvent* event);

private:
    QString color;
    QString type;
    QColor bgdColor;
    QColor dark, light;
    QColor marginColor;
    int row, col;

    bool activeness;
};

#endif // MYGRAPHICSITEM_H
