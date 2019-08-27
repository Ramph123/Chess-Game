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

signals:
    void connectSuccess();

public slots:
    void connection();
    void openGame();
    void newGame();
    void giveUp();
    void askDraw();
    void startConnection(QString);
    void acceptConnection();
    void disconnect();
    void abort();
    void recvMessage();

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
    void endGame();

    void statusUpdate();

    int mode; // 0: server 1: client
    QString ipAdd;
    int port;
    bool connected;
    QTcpServer  *listenSocket;
    QTcpSocket  *readWriteSocket;
    bool activeness;

    void newGame_Passive();

    bool side; // 0: white  1:black
};

#endif // MAINWINDOW_H
