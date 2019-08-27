#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QGraphicsScene>
#include "mygraphicsitem.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void connection();
    void openGame();
    void newGame();
    void giveUp();
    void askDraw();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *m_scene;

    QAction *connectionAction;
    QAction *openAction;
    QAction *newAction;
    QAction *giveUpAction;
    QAction *drawAction;

    MyGraphicsItem *chess[8][8];
    void initChess();
};

#endif // MAINWINDOW_H
