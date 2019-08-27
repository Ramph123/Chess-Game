#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QGraphicsScene>
#include "mygraphicsitem.h"
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#include "connectdialog.h"

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
    void startConnection(QString);
    void acceptConnection();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *m_scene;
    connectDialog *connectConfigure;

    QAction *connectionAction;
    QAction *openAction;
    QAction *newAction;
    QAction *giveUpAction;
    QAction *drawAction;

    MyGraphicsItem *chess[8][8];
    void initChess();

    void statusUpdate();

    int mode; // 0: server 1: client
    QString ipAdd;
    int port;
    bool connected;
    QTcpServer  *listenSocket;
    QTcpSocket  *readWriteSocket;
};

#endif // MAINWINDOW_H
