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
#include "servergameconfig.h"
#include <vector>
#include <QTimer>

using std::vector;

namespace Ui {
class MainWindow;
}

struct Point {
    int row,col;
};

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
    void saveGame();
    void newGame();
    void newGame_Passive();
    void loadGame();
    void loadStart();
    void askLoad();
    void giveUp();
    void askDraw();
    void startConnection(QString);
    void acceptConnection();
    void connectTimeout();
    void disconnect();
    void abort();
    void recvMessage();
    void gameConfig(QString);
    void acceptStart();
    void checkerClicked(int, int);
    void updateTime();
    void promote(QString, int, int);

private:
    Ui::MainWindow *ui;
    QGraphicsScene *m_scene;
    connectDialog *connectConfigure;
    serverGameConfig *config;

    QAction *connectionAction;
    QAction *openAction;
    QAction *newAction;
    QAction *giveUpAction;
    QAction *drawAction;
    QAction *saveAction;

    MyGraphicsItem *chess[8][8];
    void initBoard();
    void initChess();
    void endGame(bool flag = false);

    void statusUpdate();

    int mode; // 0: server 1: client
    QString ipAdd;
    int port;
    bool connected;
    QTcpServer  *listenSocket;
    QTcpSocket  *readWriteSocket;
    bool activeness;

    bool side; // 0: black  1:white
    bool limitEnable;
    int timeLimit;
    QTimer *timer;
    int timeRemain;
    void initTimer();

    bool curSide;
    int selected;
    int prevRow, prevCol;
    QString side2String(bool in);
    void clickChess(int row, int col);
    void moveChess(int prevRow, int prevCol, int destRow, int destCol);
    void turnUpdate();
    vector<Point> accessible;
    void getAccessible(int row, int col);
    void paintAccessible();
    void cleanAccessible();

    bool judge();
    int oppoControl[8][8];
    void getControl();
    void stalemate();

    bool castling;
};

#endif // MAINWINDOW_H
