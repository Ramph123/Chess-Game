#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include "clientgameconfig.h"
#include "promotiondialog.h"
#include <QTimer>
#include <iostream>
#include <QFile>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("Chess Game"));
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);
    ui->lcdNumber->setDisabled(true);

    config = nullptr;
    connected = false;
    activeness = false;
    statusUpdate();
    side = 0;
    limitEnable = 0;
    timeLimit = -1;
    timer = new QTimer;
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));

    connectionAction = new QAction("Connection Option", this);
    connectionAction->setShortcuts(QKeySequence::Copy);
    connect(connectionAction, &QAction::triggered, this, &MainWindow::connection);
    openAction = new QAction("Load game", this);
    openAction->setShortcuts(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openGame);
    saveAction = new QAction("Save game", this);
    saveAction->setShortcuts(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveGame);
    newAction = new QAction("New Game", this);
    newAction->setShortcuts(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newGame);
    giveUpAction = new QAction("Admit defeat", this);
    connect(giveUpAction, &QAction::triggered, this, &MainWindow::giveUp);
    drawAction = new QAction("Ask for draw", this);
    connect(drawAction, &QAction::triggered, this, &MainWindow::askDraw);
    connect(ui->defeatButton, SIGNAL(clicked()), this, SLOT(giveUp()));
    connect(ui->drawButton, SIGNAL(clicked()), this, SLOT(askDraw()));

    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(connectionAction);
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);

    QMenu *opMenu = menuBar()->addMenu(tr("Operation"));
    opMenu->addAction(giveUpAction);
    opMenu->addAction(drawAction);

    m_scene = new QGraphicsScene;
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    QColor bgd;
    bgd.setRgb(236,236,236);
    m_scene->setBackgroundBrush(QBrush(bgd));

    QColor dark, light;
    dark.setRgb(184,146,114);
    light.setRgb(247,228,195);

    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            QColor color = ((i+j)%2==0) ? light : dark;
            chess[i][j] = new MyGraphicsItem(i, j, color);
                chess[i][j]->setPos(j*60, (7-i)*60);
            m_scene->addItem(chess[i][j]);
        }
    }

    QGraphicsRectItem *item = new QGraphicsRectItem(-30,-30,480,480);
    QPen pen(Qt::black, 5);
    item->setPen(pen);
    m_scene->addItem(item);

    update();
}

QString MainWindow::side2String(bool in) {
    if(in == 1)
        return "white";
    else
        return "black";
}

void MainWindow::initBoard() {

    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            if(side == 0)
                chess[i][j]->setPos((7-j)*60, i*60);
            else
                chess[i][j]->setPos(j*60, (7-i)*60);
        }
    }
    update();
}

void MainWindow::initChess() {
    chess[0][0]->setChess("white", "rook");
    chess[0][1]->setChess("white", "knight");
    chess[0][2]->setChess("white", "bishop");
    chess[0][3]->setChess("white", "queen");
    chess[0][4]->setChess("white", "king");
    chess[0][5]->setChess("white", "bishop");
    chess[0][6]->setChess("white", "knight");
    chess[0][7]->setChess("white", "rook");
    for(int i = 0; i < 8; i ++)
        chess[1][i]->setChess("white","pawn");
    chess[7][0]->setChess("black", "rook");
    chess[7][1]->setChess("black", "knight");
    chess[7][2]->setChess("black", "bishop");
    chess[7][3]->setChess("black", "queen");
    chess[7][4]->setChess("black", "king");
    chess[7][5]->setChess("black", "bishop");
    chess[7][6]->setChess("black", "knight");
    chess[7][7]->setChess("black", "rook");
    for(int i = 0; i < 8; i ++)
        chess[6][i]->setChess("black","pawn");
    update();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::statusUpdate() {
    if(connected) {
        if(mode == 0) {
            ui->statusLabel->setText("Connected as server. Address: " + ipAdd + ":" + QString::number(port));
        }
        else {
            ui->statusLabel->setText("Connected as client. Server address: " + ipAdd + ":" + QString::number(port));
        }
    }
    else {
            ui->statusLabel->setText("Disconnected");
    }
    repaint();
}

void MainWindow::connection() {
    if(connected)
        disconnect1();
    statusUpdate();
    connectConfigure = new connectDialog(this);
    connectConfigure->setModal(false);
    connect(connectConfigure, SIGNAL(ipAddress(QString)), this, SLOT(startConnection(QString)));
    connect(connectConfigure, SIGNAL(abort()), this, SLOT(abort()));
    connectConfigure->show();
}

void MainWindow::startConnection(QString ipAddress) {
    int ip1, ip2, ip3, ip4;
    sscanf(ipAddress.toLatin1().data(), "%d %d.%d.%d.%d:%d", &mode, &ip1, &ip2, &ip3, &ip4, &port);
    ipAdd = QString::number(ip1) + "." + QString::number(ip2) + "." + QString::number(ip3) + "." + QString::number(ip4);
    if(mode == 0) {     // server
        //qDebug() << ipAdd << port;
        ui->statusLabel->setText("Connecting as server...  (Address: " + ipAdd + ":" + QString::number(port) + ")");
        this->listenSocket = new QTcpServer;
        //qDebug() << listenSocket;
        this->listenSocket->listen(QHostAddress(ipAdd),port);
        //qDebug() << "%%%";
        connect(this->listenSocket,SIGNAL(newConnection()),this,SLOT(acceptConnection()));
    }
    else {      //client
        ui->statusLabel->setText("Connecting as client...");
        this->readWriteSocket = new QTcpSocket;
        this->readWriteSocket->connectToHost(QHostAddress(ipAdd),port);
        QTimer *timer = new QTimer;
        timer->start(15000);
        timer->setSingleShot(true);
        connect(timer, SIGNAL(timeout()), this, SLOT(connectTimeout()));
        if(readWriteSocket->waitForConnected()) {
            timer->stop();
            emit connectSuccess();
            connected = true;
            statusUpdate();
            connect(this->readWriteSocket,SIGNAL(readyRead()),this,SLOT(recvMessage()));
            beatCnt = 0;
            beatTimer = new QTimer(readWriteSocket);
            beatTimer->start(1000);
            connect(beatTimer, &QTimer::timeout, this, &MainWindow::keepAlive);
            QMessageBox::information(this, "success", "Connection established.");
            //ui->turnLabel->setText("WAITING CONFIG");
            update();
        }
    }
}

void MainWindow::connectTimeout() {
    QMessageBox::critical(this, "timeout", "Connect Timeout!");
    disconnect1();
}

void MainWindow::acceptConnection()
{
    if(readWriteSocket != nullptr)
        return;
    this->readWriteSocket = this->listenSocket->nextPendingConnection();
    emit connectSuccess();
    connected = true;
    statusUpdate();
    //ui->turnLabel->setText("WAITING CONFIG");
    //update();
    config = new serverGameConfig;
    config->setModal(false);
    connect(config, SIGNAL(gameConfigResult(QString)), this, SLOT(gameConfig(QString)));
    //connect(config, SIGNAL(gameConfigResult(QString)), this, SLOT(newGame_Passive()));
    //config->show();
    //newGame_Passive();
    connect(this->readWriteSocket, SIGNAL(disconnected()), this, SLOT(disconnect1()));
    connect(this->readWriteSocket,SIGNAL(readyRead()),this,SLOT(recvMessage()));
    beatCnt = 0;
    beatTimer = new QTimer(readWriteSocket);
    beatTimer->start(1000);
    connect(beatTimer, &QTimer::timeout, this, &MainWindow::keepAlive);
    QMessageBox::information(this, "success", "Connection established.");
}

void MainWindow::keepAlive() {
    beatCnt ++;
    qDebug() << "Beat" << beatCnt;
    if (beatCnt == 1)
        readWriteSocket->write("Heartbeat\n");
    else if (beatCnt > 5) {
        QMessageBox::information(this, "Disconnect", "Connection failed!");
        //readWriteSocket->disconnectFromHost();
        disconnect1();
    }
}

void MainWindow::gameConfig(QString result) {
    int in_side, in_enable, in_time, in_flag;
    sscanf(result.toLatin1().data(), "%d-%d-%d-%d", &in_side, &in_enable, &in_time, &in_flag);
    side = (in_side) ? true : false;
    bool flag = (in_flag) ? true : false;
    //qDebug() << "flag" << flag;
    limitEnable = (in_enable) ? true : false;
    if(limitEnable)
        timeLimit = in_time;
    ui->turnLabel->setText("WAITING CONFIRM");
    QString oppoSide = (side) ? "0" : "1";
    QString oppoLimit = (limitEnable) ? "1" : "0";
    QString oppoTime = QString::number(timeLimit);
    QString block;
    if(!flag)
        block = "Config-" + oppoSide + "-" + oppoLimit + "-" + oppoTime + "\n";
    else {
        //QMessageBox::information(this, "a", "A");
        block = "ConLoad-" + oppoSide + "-" + oppoLimit + "-" + oppoTime + "\n";
    }
    readWriteSocket->write(block.toLatin1().data());
}

void MainWindow::acceptStart() {
    readWriteSocket->write("Game start");
    newGame_Passive();
}

void MainWindow::recvMessage()
{
    qDebug() << "read";
    QString info1;
    //QDataStream stream(readWriteSocket);
    //stream >> info;
    info1 += this->readWriteSocket->readAll();
    QStringList inList = info1.split("\n", QString::SkipEmptyParts);
    //qDebug() << info;
    QString info;
    for(int i = 0; i < inList.size(); i ++) {
        info = inList.at(i);
        qDebug() << info;
        if(info == "Heartbeat") {
            beatCnt = 0;
            continue;
        }
        else if(info == "Stop check") {
            beatTimer->stop();
        }
        else if(info == "Start check") {
            beatTimer->start(1000);
        }
        else if(info == "Admit defeat") {
            readWriteSocket->write("Stop check\n");
            beatTimer->stop();
            QMessageBox::information(this, "information", "Opponent admits defeat. You win!");
            readWriteSocket->write("Start check\n");
            beatTimer->start();
            readWriteSocket->write("Defeat information get\n");
            //stream << "Defeat information get";
            endGame();
        }
        else if(info == "Defeat information get") {
            readWriteSocket->write("Stop check\n");
            beatTimer->stop();
            QMessageBox::information(this, "information", "You Lose!");
            readWriteSocket->write("Start check\n");
            beatTimer->start();
            endGame();
        }
        else if(info == "Ask draw") {
            timer->stop();
            ui->turnLabel->setText("WAITING DECISION ...");
            readWriteSocket->write("Stop check\n");
            beatTimer->stop();
            QMessageBox::StandardButton reply = QMessageBox::question(this, "Ask for draw", "Opponent asks for a draw. Do you agree?");
            readWriteSocket->write("Start check\n");
            beatTimer->start(1000);
            if(reply == QMessageBox::No) {
                readWriteSocket->write("Refuse draw\n");
                if(limitEnable)
                    timer->start(1000);
                turnUpdate();
            }
            else {
                readWriteSocket->write("Accept draw\n");
                QMessageBox::information(this, "Draw", "Draw!");
                endGame();
            }
        }
        else if(info == "Refuse draw") {
            if(limitEnable)
                timer->start(1000);
            readWriteSocket->write("Stop check\n");
            beatTimer->stop();
            QMessageBox::information(this, "Reply", "Opponent refuses a draw!");
            readWriteSocket->write("Start check\n");
            beatTimer->start(1000);
            turnUpdate();
        }
        else if(info == "Accept draw") {
            readWriteSocket->write("Stop check\n");
            beatTimer->stop();
            QMessageBox::information(this, "Reply", "Opponent accepts a draw!");
            readWriteSocket->write("Start check\n");
            beatTimer->start(1000);
            endGame();
        }
        else if(info == "New game") {
            ui->turnLabel->setText("NEW GAME!");
            update();
        }
        else if(info == "Ask new game") {
            //QMessageBox::information(this, "New game", "Client asks for a new game");
            newGame();
        }
        else if(info.left(6) == "Config") {
            readWriteSocket->write("Start check\n");
            beatTimer->start(2000);
            int in_side, in_enable, in_time;
            sscanf(info.toLatin1().data(), "Config-%d-%d-%d", &in_side, &in_enable, &in_time);
            side = (in_side) ? true : false;
            limitEnable = (in_enable) ? true : false;
            if(limitEnable)
                timeLimit = in_time;
            //QString sideString = (side)?"white":"black";
            clientGameConfig *dialog = new clientGameConfig;
            dialog->setSide(side);
            dialog->setTimeLimit(limitEnable, timeLimit);
            dialog->show();
            connect(dialog, SIGNAL(startGame()), this, SLOT(acceptStart()));
            //readWriteSocket->write("Game start");
        }
        else if(info.left(7) == "ConLoad") {
            readWriteSocket->write("Start check\n");
            beatTimer->start(1000);
            int in_side, in_enable, in_time;
            sscanf(info.toLatin1().data(), "ConLoad-%d-%d-%d", &in_side, &in_enable, &in_time);
            side = (in_side) ? true : false;
            limitEnable = (in_enable) ? true : false;
            if(limitEnable)
                timeLimit = in_time;
            //QString sideString = (side)?"white":"black";
            clientGameConfig *dialog = new clientGameConfig;
            dialog->setSide(side);
            dialog->setTimeLimit(limitEnable, timeLimit);
            dialog->show();
            connect(dialog, SIGNAL(startGame()), this, SLOT(askLoad()));
        }
        else if(info == "Game start") {
            newGame_Passive();
        }
        else if(info.left(5) == "Click") {
            int row, col;
            sscanf(info.toLatin1().data(), "Click %d-%d", &row, &col);
            //clickChess(row, col);
            getAccessible(row, col);
            //paintAccessible();
            prevRow = row;
            prevCol = col;
        }
        else if(info.left(4) == "Move") {
            int row, col;
            sscanf(info.toLatin1().data(), "Move %d-%d", &row, &col);
            if(row == prevRow && col == prevCol) {
                chess[prevRow][prevCol]->setMargin(Qt::transparent);
                cleanAccessible();
                prevRow = prevCol = -1;
            }
            else {
                moveChess(prevRow, prevCol, row, col);
                cleanAccessible();
                chess[prevRow][prevCol]->setMargin(Qt::transparent);
                prevRow = prevCol = -1;
                if(judge()) {
                    stalemate();
                }
                cleanAccessible();
                curSide = !curSide;
                turnUpdate();
                if(limitEnable) {
                    timeRemain = timeLimit;
                    updateTime();
                    timer->start(1000);
                }
            }
        }
        else if(info == "Stalemate") {
            readWriteSocket->write("Stop check\n");
            beatTimer->stop();
            QMessageBox::information(this, "Stalemate", "Stalemate!");
            readWriteSocket->write("Start check\n");
            beatTimer->start(1000);
            endGame();
        }
        else if(info.left(9) == "Checkmate") {
            int kingRow, kingCol;
            sscanf(info.toLatin1().data(), "Checkmate %d-%d", &kingRow, &kingCol);
            chess[kingRow][kingCol]->setMargin(Qt::red);
            moveChess(prevRow, prevCol, kingRow, kingCol);
            readWriteSocket->write("Stop check\n");
            beatTimer->stop();
            QMessageBox::information(this, "Checkmate", "You lose!");
            readWriteSocket->write("Start check\n");
            beatTimer->start(1000);
            endGame();
        }
        else if(info.left(9) == "Promotion") {
            int row, col, in;
            sscanf(info.toLatin1().data(), "Promotion %d-%d-%d", &row, &col, &in);
            QString target = "";
            if(in == 1)
                target = "queen";
            else if(in == 2)
                target = "rook";
            else if(in == 3)
                target = "knight";
            else if(in == 4)
                target = "bishop";
            qDebug() << row << col << target;
            moveChess(prevRow, prevCol, row, col);
            chess[row][col]->setChess(chess[row][col]->getSide(), target);
            cleanAccessible();
            chess[prevRow][prevCol]->setMargin(Qt::transparent);
            prevRow = prevCol = -1;
            judge();
            curSide = !curSide;
            turnUpdate();
            if(limitEnable) {
                timeRemain = timeLimit;
                updateTime();
                timer->start(1000);
            }
        }
        else if(info.left(4) == "Long") {
            if(info == "Long black") {
                moveChess(7,4,7,2);
                moveChess(7,0,7,3);
                cleanAccessible();
                chess[prevRow][prevCol]->setMargin(Qt::transparent);
                prevRow = prevCol = -1;
                judge();
                curSide = !curSide;
                turnUpdate();
                if(limitEnable) {
                    timeRemain = timeLimit;
                    updateTime();
                    timer->start(1000);
                }
            }
            else if(info == "Long white") {
                moveChess(0,4,0,2);
                moveChess(0,0,0,3);
                cleanAccessible();
                chess[prevRow][prevCol]->setMargin(Qt::transparent);
                prevRow = prevCol = -1;
                judge();
                curSide = !curSide;
                turnUpdate();
                if(limitEnable) {
                    timeRemain = timeLimit;
                    updateTime();
                    timer->start(1000);
                }
            }
        }
        else if(info.left(5) == "Short") {
            if(info == "Short black") {
                moveChess(7,4,7,6);
                moveChess(7,7,7,5);
                cleanAccessible();
                chess[prevRow][prevCol]->setMargin(Qt::transparent);
                prevRow = prevCol = -1;
                judge();
                curSide = !curSide;
                turnUpdate();
                if(limitEnable) {
                    timeRemain = timeLimit;
                    updateTime();
                    timer->start(1000);
                }
            }
            else if(info == "Short white") {
                moveChess(0,4,0,6);
                moveChess(0,7,0,5);
                cleanAccessible();
                chess[prevRow][prevCol]->setMargin(Qt::transparent);
                prevRow = prevCol = -1;
                judge();
                curSide = !curSide;
                turnUpdate();
                if(limitEnable) {
                    timeRemain = timeLimit;
                    updateTime();
                    timer->start(1000);
                }
            }
        }
        else if(info == "Open") {
            ui->turnLabel->setText("LOAD GAME");
            endGame();
            for(int i = 0; i < 8; i ++) {
                for(int j = 0; j < 8; j ++) {
                    chess[i][j]->setChess();
                }
            }
            repaint();
        }
        else if(info == "AskLoad") {
            loadGame();
        }
        else if(info == "AskOpen") {
            readWriteSocket->write("Stop check\n");
            beatTimer->stop();
            QMessageBox::information(this, "open", "Client asks for loading a game");
            readWriteSocket->write("Start check\n");
            beatTimer->start(1000);
            openGame();
        }
        else if(info.startsWith("w") || info.startsWith("b")) {
            if(info.startsWith("w"))
                curSide = 1;
            else
                curSide = 0;
            char *in = info.toLatin1().data();
            int pos = 1;
            for(int i = 0; i < 8; i ++) {
                for(int j = 0; j < 8; j ++) {
                    if(in[pos] == 'g')
                        chess[i][j]->setChess("white", "king");
                    else if(in[pos] == 'q')
                        chess[i][j]->setChess("white", "queen");
                    else if(in[pos] == 'r')
                        chess[i][j]->setChess("white", "rook");
                    else if(in[pos] == 'k')
                        chess[i][j]->setChess("white", "knight");
                    else if(in[pos] == 'h')
                        chess[i][j]->setChess("white", "bishop");
                    else if(in[pos] == 'p')
                        chess[i][j]->setChess("white", "pawn");
                    else if(in[pos] == 'G')
                        chess[i][j]->setChess("black", "king");
                    else if(in[pos] == 'Q')
                        chess[i][j]->setChess("black", "queen");
                    else if(in[pos] == 'R')
                        chess[i][j]->setChess("black", "rook");
                    else if(in[pos] == 'K')
                        chess[i][j]->setChess("black", "knight");
                    else if(in[pos] == 'H')
                        chess[i][j]->setChess("black", "bishop");
                    else if(in[pos] == 'P')
                        chess[i][j]->setChess("black", "pawn");
                    pos ++;
                }
            }
            loadStart();
        }
        else if(info == "Disconnect") {
            connected = false;
            disconnect1();
        }
    }
}

void MainWindow::abort() {
    if(mode == 0) {
        endGame(true);
        listenSocket->deleteLater();
        listenSocket = nullptr;
        connected = false;
        statusUpdate();
    }
    else {
        disconnect1();
    }
}

void MainWindow::disconnect1() {
    endGame();
    //readWriteSocket->disconnectFromHost();
    readWriteSocket->deleteLater();
    readWriteSocket = nullptr;
    connected = false;
    statusUpdate();
}

void MainWindow::endGame(bool flag) {
    qDebug() << "endgame";
    if(!flag)
        ui->turnLabel->setText("GAME ENDED");
    if(timer->isActive()) {
        timer->stop();
        ui->lcdNumber->display(0);
        ui->lcdNumber->setDisabled(true);
    }
    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            chess[i][j]->setActiveness(false);
            disconnect(chess[i][j], SIGNAL(checkerClicked(int,int)), this, SLOT(checkerClicked(int,int)));
        }
    }
    activeness = false;
}

void MainWindow::newGame() {
    if(activeness)
        return;
    if(!connected) {
        QMessageBox::critical(this, "No connection", "Please connect first!");
        return;
    }
    ui->turnLabel->setText("WAITING CONFIG");
    if(config == nullptr) { //client
        readWriteSocket->write("Ask new game\n");
    }
    else {
        config->show();
        readWriteSocket->write("New game\n");
    }
}

void MainWindow::newGame_Passive() {
    ui->turnLabel->clear();
    if(activeness)
        return;
    initBoard();
    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            chess[i][j]->setActiveness(true);
            chess[i][j]->setChess();
            chess[i][j]->setMargin(Qt::transparent);
            connect(chess[i][j], SIGNAL(checkerClicked(int,int)), this, SLOT(checkerClicked(int,int)));
        }
    }
    activeness = true;
    curSide = 1;
    selected = 0;
    prevRow = prevCol = -1;
    castling = false;
    turnUpdate();
    initTimer();
    initChess();
    repaint();
}

void MainWindow::initTimer() {
    if(limitEnable == false) {
        //QMessageBox::information(this, "a", "a");
        ui->lcdNumber->setDisabled(true);
        timer->stop();
        return;
    }
    ui->lcdNumber->setEnabled(true);
    timeRemain = timeLimit;
    updateTime();
    timer->start(1000);
}

void MainWindow::updateTime() {
    ui->lcdNumber->display(timeRemain);
    if(timeRemain == 0) {
        if(curSide == side)
            QMessageBox::information(this, "Timeout", "Timeout! You lose!");
        else
            QMessageBox::information(this, "Timeout", "Opponent timeout! You win!");
        timer->stop();
        endGame();
    }
    timeRemain --;
}

void MainWindow::giveUp() {
    if(!activeness || !connected)
        return;
    readWriteSocket->write("Stop check\n");
    beatTimer->stop();
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Admit defeat", "Are you sure you want to admit defeat?");
    qDebug() << "giveup";
    readWriteSocket->write("Start check\n");
    beatTimer->start(1000);
    if(reply == QMessageBox::No) {
        return;
    }
    readWriteSocket->write("Admit defeat\n");
    //QTextStream stream(readWriteSocket);
    //stream << "Admit defeat";
}

void MainWindow::askDraw() {
    if(!activeness || !connected)
        return;
    readWriteSocket->write("Stop check\n");
    beatTimer->stop();
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Ask draw", "Are you sure you want to ask for a draw?");
    qDebug() << "askDraw";
    readWriteSocket->write("Start check\n");
    beatTimer->start(1000);
    if(reply == QMessageBox::No) {
        return;
    }
    timer->stop();
    ui->turnLabel->setText("WATING REPLY ...");
    readWriteSocket->write("Ask draw\n");
}

void MainWindow::turnUpdate() {
    qDebug() << "current" << curSide << "side" << side;
    if(curSide == side) {
        ui->turnLabel->setText("YOUR TURN !");
    }
    else {
        ui->turnLabel->setText("WAITING OPPONENT ...");
    }
    update();
}

void MainWindow::moveChess(int prevRow, int prevCol, int destRow, int destCol) {
    chess[destRow][destCol]->setChess(chess[prevRow][prevCol]->getSide(), chess[prevRow][prevCol]->getType());
    chess[prevRow][prevCol]->setChess();
}

void MainWindow::clickChess(int row, int col) {
    chess[row][col]->setMargin(Qt::blue);
}

void MainWindow::getAccessible(int row, int col) {
    MyGraphicsItem* cur = chess[row][col];
    if(cur->getType() == "pawn") {
        Point newPoint;
        if(cur->getSide() == "white") {
            if(chess[row+1][col]->getSide() == "") {
                newPoint.row = row + 1;
                newPoint.col = col;
                accessible.push_back(newPoint);
            }
            if(chess[row+1][col+1]->getSide() == "black"){
                newPoint.row = row + 1;
                newPoint.col = col + 1;
                accessible.push_back(newPoint);
            }
            if(chess[row+1][col-1]->getSide() == "black"){
                newPoint.row = row + 1;
                newPoint.col = col - 1;
                accessible.push_back(newPoint);
            }
            if(row == 1 && chess[row+2][col]->getSide() == ""){
                newPoint.row = row + 2;
                newPoint.col = col;
                accessible.push_back(newPoint);
            }
        }
        else {
            if(chess[row-1][col]->getSide() == "") {
                newPoint.row = row - 1;
                newPoint.col = col;
                accessible.push_back(newPoint);
            }
            if(chess[row-1][col+1]->getSide() == "white"){
                newPoint.row = row - 1;
                newPoint.col = col + 1;
                accessible.push_back(newPoint);
            }
            if(chess[row-1][col-1]->getSide() == "white"){
                newPoint.row = row - 1;
                newPoint.col = col - 1;
                accessible.push_back(newPoint);
            }
            if(row == 6 && chess[row-2][col]->getSide() == ""){
                newPoint.row = row - 2;
                newPoint.col = col;
                accessible.push_back(newPoint);
            }
        }
    }
    else if(cur->getType() == "king") {
        Point newPoint;
        if(row+1 < 8 && col+1 < 8 && chess[row+1][col+1]->getSide() != cur->getSide()) {
            newPoint.row = row+1;
            newPoint.col = col+1;
            accessible.push_back(newPoint);
        }
        if(row+1 < 8 && chess[row+1][col]->getSide() != cur->getSide()) {
            newPoint.row = row+1;
            newPoint.col = col;
            accessible.push_back(newPoint);
        }
        if(row+1 < 8 && col-1 >= 0 && chess[row+1][col-1]->getSide() != cur->getSide()) {
            newPoint.row = row+1;
            newPoint.col = col-1;
            accessible.push_back(newPoint);
        }
        if(col+1 < 8 && chess[row][col+1]->getSide() != cur->getSide()) {
            newPoint.row = row;
            newPoint.col = col+1;
            accessible.push_back(newPoint);
        }
        if(col-1 >= 0 && chess[row][col-1]->getSide() != cur->getSide()) {
            newPoint.row = row;
            newPoint.col = col-1;
            accessible.push_back(newPoint);
        }
        if(row-1 >= 0 && col+1 < 8 && chess[row-1][col+1]->getSide() != cur->getSide()) {
            newPoint.row = row-1;
            newPoint.col = col+1;
            accessible.push_back(newPoint);
        }
        if(row-1 >= 0 && chess[row-1][col]->getSide() != cur->getSide()) {
            newPoint.row = row-1;
            newPoint.col = col;
            accessible.push_back(newPoint);
        }
        if(row-1 >= 0 && col-1 >= 0 && chess[row-1][col-1]->getSide() != cur->getSide()) {
            newPoint.row = row-1;
            newPoint.col = col-1;
            accessible.push_back(newPoint);
        }
    }
    else if(cur->getType() == "rook") {
        Point newPoint;
        int delta = 1;
        while(row-delta >= 0 && chess[row-delta][col]->getSide() != cur->getSide()) {
            newPoint.row = row-delta;
            newPoint.col = col;
            accessible.push_back(newPoint);
            if(chess[row-delta][col]->getSide() != "")
                break;
            delta ++;
        }
        delta = 1;
        while(row+delta < 8 && chess[row+delta][col]->getSide() != cur->getSide()) {
            newPoint.row = row+delta;
            newPoint.col = col;
            accessible.push_back(newPoint);
            if(chess[row+delta][col]->getSide() != "")
                break;
            delta ++;
        }
        delta = 1;
        while(col-delta >= 0 && chess[row][col-delta]->getSide() != cur->getSide()) {
            newPoint.row = row;
            newPoint.col = col-delta;
            accessible.push_back(newPoint);
            if(chess[row][col-delta]->getSide() != "")
                break;
            delta ++;
        }
        delta = 1;
        while(col+delta < 8 && chess[row][col+delta]->getSide() != cur->getSide()) {
            newPoint.row = row;
            newPoint.col = col+delta;
            accessible.push_back(newPoint);
            if(chess[row][col+delta]->getSide() != "")
                break;
            delta ++;
        }
    }
    else if(cur->getType() == "bishop") {
        Point newPoint;
        int delta = 1;
        while(row-delta >= 0 && col-delta >= 0 && chess[row-delta][col-delta]->getSide() != cur->getSide()) {
            newPoint.row = row-delta;
            newPoint.col = col-delta;
            accessible.push_back(newPoint);
            if(chess[row-delta][col-delta]->getSide() != "")
                break;
            delta ++;
        }
        delta = 1;
        while(row-delta >= 0 && col+delta < 8 && chess[row-delta][col+delta]->getSide() != cur->getSide()) {
            newPoint.row = row-delta;
            newPoint.col = col+delta;
            accessible.push_back(newPoint);
            if(chess[row-delta][col+delta]->getSide() != "")
                break;
            delta ++;
        }
        delta = 1;
        while(row+delta < 8 && col-delta >= 0 && chess[row+delta][col-delta]->getSide() != cur->getSide()) {
            newPoint.row = row+delta;
            newPoint.col = col-delta;
            accessible.push_back(newPoint);
            if(chess[row+delta][col-delta]->getSide() != "")
                break;
            delta ++;
        }
        delta = 1;
        while(row+delta < 8 && col+delta < 8 && chess[row+delta][col+delta]->getSide() != cur->getSide()) {
            newPoint.row = row+delta;
            newPoint.col = col+delta;
            accessible.push_back(newPoint);
            if(chess[row+delta][col+delta]->getSide() != "")
                break;
            delta ++;
        }
    }
    else if(cur->getType() == "queen") {
        Point newPoint;
        int delta = 1;
        while(row-delta >= 0 && chess[row-delta][col]->getSide() != cur->getSide()) {
            newPoint.row = row-delta;
            newPoint.col = col;
            accessible.push_back(newPoint);
            if(chess[row-delta][col]->getSide() != "")
                break;
            delta ++;
        }
        delta = 1;
        while(row+delta < 8 && chess[row+delta][col]->getSide() != cur->getSide()) {
            newPoint.row = row+delta;
            newPoint.col = col;
            accessible.push_back(newPoint);
            if(chess[row+delta][col]->getSide() != "")
                break;
            delta ++;
        }
        delta = 1;
        while(col-delta >= 0 && chess[row][col-delta]->getSide() != cur->getSide()) {
            newPoint.row = row;
            newPoint.col = col-delta;
            accessible.push_back(newPoint);
            if(chess[row][col-delta]->getSide() != "")
                break;
            delta ++;
        }
        delta = 1;
        while(col+delta < 8 && chess[row][col+delta]->getSide() != cur->getSide()) {
            newPoint.row = row;
            newPoint.col = col+delta;
            accessible.push_back(newPoint);
            if(chess[row][col+delta]->getSide() != "")
                break;
            delta ++;
        }
        delta = 1;
        while(row-delta >= 0 && col-delta >= 0 && chess[row-delta][col-delta]->getSide() != cur->getSide()) {
            newPoint.row = row-delta;
            newPoint.col = col-delta;
            accessible.push_back(newPoint);
            if(chess[row-delta][col-delta]->getSide() != "")
                break;
            delta ++;
        }
        delta = 1;
        while(row-delta >= 0 && col+delta < 8 && chess[row-delta][col+delta]->getSide() != cur->getSide()) {
            newPoint.row = row-delta;
            newPoint.col = col+delta;
            accessible.push_back(newPoint);
            if(chess[row-delta][col+delta]->getSide() != "")
                break;
            delta ++;
        }
        delta = 1;
        while(row+delta < 8 && col-delta >= 0 && chess[row+delta][col-delta]->getSide() != cur->getSide()) {
            newPoint.row = row+delta;
            newPoint.col = col-delta;
            accessible.push_back(newPoint);
            if(chess[row+delta][col-delta]->getSide() != "")
                break;
            delta ++;
        }
        delta = 1;
        while(row+delta < 8 && col+delta < 8 && chess[row+delta][col+delta]->getSide() != cur->getSide()) {
            newPoint.row = row+delta;
            newPoint.col = col+delta;
            accessible.push_back(newPoint);
            if(chess[row+delta][col+delta]->getSide() != "")
                break;
            delta ++;
        }
    }
    else if(cur->getType() == "knight") {
        Point newPoint;
        if(row+2 < 8 && col+1 < 8 && chess[row+2][col+1]->getSide() != cur->getSide()) {
            newPoint.row = row+2;
            newPoint.col = col+1;
            accessible.push_back(newPoint);
        }
        if(row+2 < 8 && col-1 >= 0 && chess[row+2][col-1]->getSide() != cur->getSide()) {
            newPoint.row = row+2;
            newPoint.col = col-1;
            accessible.push_back(newPoint);
        }
        if(row-2 >= 0 && col+1 < 8 && chess[row-2][col+1]->getSide() != cur->getSide()) {
            newPoint.row = row-2;
            newPoint.col = col+1;
            accessible.push_back(newPoint);
        }
        if(row-2 >= 0 && col-1 >= 0 && chess[row-2][col-1]->getSide() != cur->getSide()) {
            newPoint.row = row-2;
            newPoint.col = col-1;
            accessible.push_back(newPoint);
        }
        if(row+1 < 8 && col+2 < 8 && chess[row+1][col+2]->getSide() != cur->getSide()) {
            newPoint.row = row+1;
            newPoint.col = col+2;
            accessible.push_back(newPoint);
        }
        if(row+1 < 8 && col-2 >= 0 && chess[row+1][col-2]->getSide() != cur->getSide()) {
            newPoint.row = row+1;
            newPoint.col = col-2;
            accessible.push_back(newPoint);
        }
        if(row-1 >= 0 && col+2 < 8 && chess[row-1][col+2]->getSide() != cur->getSide()) {
            newPoint.row = row-1;
            newPoint.col = col+2;
            accessible.push_back(newPoint);
        }
        if(row-1 >= 0 && col-2 >= 0 && chess[row-1][col-2]->getSide() != cur->getSide()) {
            newPoint.row = row-1;
            newPoint.col = col-2;
            accessible.push_back(newPoint);
        }
    }
}

void MainWindow::paintAccessible() {
    for(int i = 0; i < accessible.size(); i ++) {
        int tarRow = accessible[i].row;
        int tarCol = accessible[i].col;
        if(chess[tarRow][tarCol]->getSide() != "") {
            QColor color;
            color.setRgb(204,0,51);
            chess[tarRow][tarCol]->setMargin(color);

        }
        else {
            QColor color;
            color.setRgb(0,255,127);
            chess[tarRow][tarCol]->setMargin(color);
        }
    }
}

void MainWindow::cleanAccessible() {
    for(int i = 0; i < accessible.size(); i ++) {
        chess[accessible[i].row][accessible[i].col]->setMargin(Qt::transparent);
    }
    while(accessible.size() != 0)
        accessible.pop_back();
}

void MainWindow::checkerClicked(int row, int col) {
    if(curSide != side)
        return;
    if(selected == 0 && chess[row][col]->getType() != "") {
        if((chess[row][col]->getSide() == "black" && side == 0) ||
                (chess[row][col]->getSide() == "white" && side == 1)) {
            clickChess(row, col);
            selected = 1;
            prevRow = row;
            prevCol = col;
            getAccessible(row, col);
            paintAccessible();
            if(!castling && row == 7 && col == 4 && chess[row][col]->getSide() == "black" && chess[row][col]->getType() == "king" &&
                    chess[7][0]->getSide() == "black" && chess[7][0]->getType() == "rook" &&
                    chess[7][1]->getSide() == "" && chess[7][2]->getSide() == "" && chess[7][3]->getSide() == "") {
                getControl();
                if(oppoControl[7][4] == 0 && oppoControl[7][3] == 0 && oppoControl[7][2] == 0) {
                    chess[7][2]->setMargin(Qt::magenta);
                }
            }
            if(!castling && row == 7 && col == 4 && chess[row][col]->getSide() == "black" && chess[row][col]->getType() == "king" &&
                    chess[7][7]->getSide() == "black" && chess[7][7]->getType() == "rook" &&
                    chess[7][5]->getSide() == "" && chess[7][6]->getSide() == "") {
                getControl();
                if(oppoControl[7][4] == 0 && oppoControl[7][5] == 0 && oppoControl[7][6] == 0) {
                    chess[7][6]->setMargin(Qt::magenta);
                }
            }
            if(!castling && row == 0 && col == 4 && chess[row][col]->getSide() == "white" && chess[row][col]->getType() == "king" &&
                    chess[0][0]->getSide() == "white" && chess[0][0]->getType() == "rook" &&
                    chess[0][1]->getSide() == "" && chess[0][2]->getSide() == "" && chess[0][3]->getSide() == "") {
                getControl();
                if(oppoControl[0][4] == 0 && oppoControl[0][3] == 0 && oppoControl[0][2] == 0) {
                    chess[0][2]->setMargin(Qt::magenta);
                }
            }
            if(!castling && row == 0 && col == 4 && chess[row][col]->getSide() == "white" && chess[row][col]->getType() == "king" &&
                    chess[0][7]->getSide() == "white" && chess[0][7]->getType() == "rook" &&
                    chess[0][5]->getSide() == "" && chess[0][6]->getSide() == "") {
                getControl();
                if(oppoControl[0][4] == 0 && oppoControl[0][5] == 0 && oppoControl[0][6] == 0) {
                    chess[0][6]->setMargin(Qt::magenta);
                }
            }
            QString block = "Click " + QString::number(row) + "-" + QString::number(col) + "\n";
            readWriteSocket->write(block.toLatin1().data());
        }
    }
    else {
        bool checkmate = false;
        if(chess[row][col]->getMargin() == Qt::transparent)
            return;
        if(row == prevRow && col == prevCol) {
            chess[row][col]->setMargin(Qt::transparent);
            cleanAccessible();
            if(row == 7 && col == 4 && chess[row][col]->getType() == "king" && chess[row][col]->getSide() == "black") {
                chess[7][2]->setMargin(Qt::transparent);
                chess[7][6]->setMargin(Qt::transparent);
            }
            if(row == 0 && col == 4 && chess[row][col]->getType() == "king" && chess[row][col]->getSide() == "white") {
                chess[0][2]->setMargin(Qt::transparent);
                chess[0][6]->setMargin(Qt::transparent);
            }
            prevRow = prevCol = -1;
            selected = 0;
        }
        else {
            if(chess[row][col]->getType() == "king")
                checkmate = true;
            moveChess(prevRow, prevCol, row, col);
            cleanAccessible();
            chess[prevRow][prevCol]->setMargin(Qt::transparent);
            if(prevRow == 7 && prevCol == 4 && chess[row][col]->getType() == "king" && chess[row][col]->getSide() == "black") {
                chess[7][2]->setMargin(Qt::transparent);
                chess[7][6]->setMargin(Qt::transparent);
            }
            if(prevRow == 0 && prevCol == 4 && chess[row][col]->getType() == "king" && chess[row][col]->getSide() == "white") {
                chess[0][2]->setMargin(Qt::transparent);
                chess[0][6]->setMargin(Qt::transparent);
            }
            prevRow = prevCol = -1;
            selected = 0;
            curSide = !curSide;
        }
        if(checkmate){
            QString block = "Checkmate " + QString::number(row) + "-" + QString::number(col) + "\n";
            readWriteSocket->write(block.toLatin1().data());
            QMessageBox::information(this, "Checkmate", "You win!");
            endGame();
        }
        else if(chess[row][col]->getType() == "pawn" && ((chess[row][col]->getSide() == "black" && row == 0) || (chess[row][col]->getSide() == "white" && row == 7))) {
            //QMessageBox::information(this, "promotion", "promotion");
            readWriteSocket->write("Stop check\n");
            beatTimer->stop();
            promotionDialog *d = new promotionDialog(chess[row][col]->getSide(), row, col);
            d->setModal(false);
            d->show();
            connect(d, SIGNAL(returnAns(QString, int, int)), this, SLOT(promote(QString, int, int)));
        }
        else if(row == 7 && col == 2 && chess[row][col]->getType() == "king" && chess[row][col]->getSide() == "black") {
            moveChess(7,0,7,3);
            castling = true;
            readWriteSocket->write("Long black\n");
            turnUpdate();
            if(limitEnable) {
                timeRemain = timeLimit;
                updateTime();
                timer->start(1000);
            }
        }
        else if(row == 7 && col == 6 && chess[row][col]->getType() == "king" && chess[row][col]->getSide() == "black") {
            moveChess(7,7,7,5);
            castling = true;
            readWriteSocket->write("Short black\n");
            turnUpdate();
            if(limitEnable) {
                timeRemain = timeLimit;
                updateTime();
                timer->start(1000);
            }
        }
        else if(row == 0 && col == 2 && chess[row][col]->getType() == "king" && chess[row][col]->getSide() == "white") {
            moveChess(0,0,0,3);
            castling = true;
            readWriteSocket->write("Long white\n");
            turnUpdate();
            if(limitEnable) {
                timeRemain = timeLimit;
                updateTime();
                timer->start(1000);
            }
        }
        else if(row == 0 && col == 6 && chess[row][col]->getType() == "king" && chess[row][col]->getSide() == "white") {
            moveChess(0,7,0,5);
            castling = true;
            readWriteSocket->write("Short white\n");
            turnUpdate();
            if(limitEnable) {
                timeRemain = timeLimit;
                updateTime();
                timer->start(1000);
            }
        }
        else {
            QString block = "Move " + QString::number(row) + "-" + QString::number(col) + "\n";
            readWriteSocket->write(block.toLatin1().data());
            turnUpdate();
            if(limitEnable) {
                timeRemain = timeLimit;
                updateTime();
                timer->start(1000);
            }
        }
    }
}

void MainWindow::promote(QString target, int row, int col) {
    readWriteSocket->write("Start check\n");
    beatTimer->start(1000);
    chess[row][col]->setChess(chess[row][col]->getSide(), target);
    repaint();
    int out = 0;
    if(target == "queen")
        out = 1;
    else if(target == "rook")
        out = 2;
    else if(target == "knight")
        out = 3;
    else if(target == "bishop")
        out = 4;
    QString block = "Promotion " + QString::number(row) + "-" + QString::number(col) + "-" + QString::number(out) + "\n";
    readWriteSocket->write(block.toLatin1().data());
    turnUpdate();
    if(limitEnable) {
        timeRemain = timeLimit;
        updateTime();
        timer->start(1000);
    }
}

bool MainWindow::judge() {
    getControl();
//    for(int i = 0; i < 8; i ++) {
//        for(int j = 0; j < 8; j ++) {
//            std::cout << oppoControl[i][j] << " ";
//        }
//        std::cout << std::endl;
//    }
    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            if(chess[i][j]->getSide() == side2String(side)) {
                if(chess[i][j]->getType() != "king") {
                    cleanAccessible();
                    getAccessible(i,j);
                    qDebug() << i << j << chess[i][j]->getType() << accessible.size();
                    if(accessible.size() != 0)
                        return false;
                    cleanAccessible();
                }
                else {
                    if(oppoControl[i][j] != 0) {
                        qDebug() << "!!!!!";
                        return false;
                    }
                    cleanAccessible();
                    getAccessible(i,j);
                    for(int k = 0; k < accessible.size(); k ++) {
                        qDebug() << "?????";
                        if(oppoControl[accessible[k].row][accessible[k].col] == 0) {
                            qDebug() << accessible[k].row << accessible[k].col;
                            return false;
                        }
                    }
                    cleanAccessible();
                }
            }
        }
    }
    return true;
}

void MainWindow::getControl() {
    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            oppoControl[i][j] = 0;
        }
    }
    for(int row = 0; row < 8; row ++) {
        for(int col = 0; col < 8; col ++) {
            MyGraphicsItem* cur = chess[row][col];
            if(cur->getSide() == side2String(!side)) {
                if(cur->getType() == "pawn") {
                    if(cur->getSide() == "white") {
                            oppoControl[row+1][col+1] ++;
                            oppoControl[row+1][col-1] ++;
                    }
                    else {
                            oppoControl[row-1][col+1] ++;
                            oppoControl[row-1][col-1]++;
                    }
                }
                else if(cur->getType() == "king") {
                    if(row+1 < 8 && col+1 < 8) {
                        oppoControl[row+1][col+1] ++;
                    }
                    if(row+1 < 8) {
                        oppoControl[row+1][col] ++;
                    }
                    if(row+1 < 8) {
                        oppoControl[row+1][col-1] ++;
                    }
                    if(col+1 < 8) {
                        oppoControl[row][col+1] ++;
                    }
                    if(col-1 >= 0) {
                        oppoControl[row][col-1] ++;
                    }
                    if(row-1 >= 0 && col+1 < 8) {
                        oppoControl[row-1][col+1] ++;
                    }
                    if(row-1 >= 0) {
                        oppoControl[row-1][col] ++;
                    }
                    if(row-1 >= 0 && col-1 >= 0) {
                        oppoControl[row-1][col-1] ++;
                    }
                }
                else if(cur->getType() == "rook") {
                    int delta = 1;
                    while(row-delta >= 0) {
                        oppoControl[row-delta][col] ++;
                        if(chess[row-delta][col]->getSide() != "")
                            break;
                        delta ++;
                    }
                    delta = 1;
                    while(row+delta < 8) {
                        oppoControl[row+delta][col] ++;
                        if(chess[row+delta][col]->getSide() != "")
                            break;
                        delta ++;
                    }
                    delta = 1;
                    while(col-delta >= 0) {
                        oppoControl[row][col-delta] ++;
                        if(chess[row][col-delta]->getSide() != "")
                            break;
                        delta ++;
                    }
                    delta = 1;
                    while(col+delta < 8) {
                        oppoControl[row][col+delta] ++;
                        if(chess[row][col+delta]->getSide() != "")
                            break;
                        delta ++;
                    }
                }
                else if(cur->getType() == "bishop") {
                    int delta = 1;
                    while(row-delta >= 0 && col-delta >= 0) {
                        oppoControl[row-delta][col-delta] ++;
                        if(chess[row-delta][col-delta]->getSide() != "")
                            break;
                        delta ++;
                    }
                    delta = 1;
                    while(row-delta >= 0 && col+delta < 8) {
                        oppoControl[row-delta][col+delta] ++;
                        if(chess[row-delta][col+delta]->getSide() != "")
                            break;
                        delta ++;
                    }
                    delta = 1;
                    while(row+delta < 8 && col-delta >= 0) {
                        oppoControl[row+delta][col-delta] ++;
                        if(chess[row+delta][col-delta]->getSide() != "")
                            break;
                        delta ++;
                    }
                    delta = 1;
                    while(row+delta < 8 && col+delta < 8) {
                        oppoControl[row+delta][col+delta] ++;
                        if(chess[row+delta][col+delta]->getSide() != "")
                            break;
                        delta ++;
                    }
                }
                else if(cur->getType() == "queen") {
                    int delta = 1;
                    while(row-delta >= 0) {
                        oppoControl[row-delta][col] ++;
                        if(chess[row-delta][col]->getSide() != "")
                            break;
                        delta ++;
                    }
                    delta = 1;
                    while(row+delta < 8) {
                        oppoControl[row+delta][col] ++;
                        if(chess[row+delta][col]->getSide() != "")
                            break;
                        delta ++;
                    }
                    delta = 1;
                    while(col-delta >= 0) {
                        oppoControl[row][col-delta] ++;
                        if(chess[row][col-delta]->getSide() != "")
                            break;
                        delta ++;
                    }
                    delta = 1;
                    while(col+delta < 8) {
                        oppoControl[row][col+delta] ++;
                        if(chess[row][col+delta]->getSide() != "")
                            break;
                        delta ++;
                    }
                    delta = 1;
                    while(row-delta >= 0 && col-delta >= 0) {
                        oppoControl[row-delta][col-delta] ++;
                        if(chess[row-delta][col-delta]->getSide() != "")
                            break;
                        delta ++;
                    }
                    delta = 1;
                    while(row-delta >= 0 && col+delta < 8) {
                        oppoControl[row-delta][col+delta] ++;
                        if(chess[row-delta][col+delta]->getSide() != "")
                            break;
                        delta ++;
                    }
                    delta = 1;
                    while(row+delta < 8 && col-delta >= 0) {
                        oppoControl[row+delta][col-delta] ++;
                        if(chess[row+delta][col-delta]->getSide() != "")
                            break;
                        delta ++;
                    }
                    delta = 1;
                    while(row+delta < 8 && col+delta < 8) {
                        oppoControl[row+delta][col+delta] ++;
                        if(chess[row+delta][col+delta]->getSide() != "")
                            break;
                        delta ++;
                    }

                }
                else if(cur->getType() == "knight") {
                    if(row+2 < 8 && col+1 < 8) {
                        oppoControl[row+2][col+1] ++;
                    }
                    if(row+2 < 8 && col-1 >= 0) {
                        oppoControl[row+2][col-1] ++;
                    }
                    if(row-2 >= 0 && col+1 < 8) {
                        oppoControl[row-2][col+1] ++;
                    }
                    if(row-2 >= 0 && col-1 >= 0) {
                        oppoControl[row-2][col-1] ++;
                    }
                    if(row+1 < 8 && col+2 < 8) {
                        oppoControl[row+1][col+2] ++;
                    }
                    if(row+1 < 8 && col-2 >= 0) {
                        oppoControl[row+1][col-2] ++;
                    }
                    if(row-1 >= 0 && col+2 < 8) {
                        oppoControl[row-1][col+2] ++;
                    }
                    if(row-1 >= 0 && col-2 >= 0) {
                        oppoControl[row-1][col-2] ++;
                    }
                }
            }
        }
    }
}

void MainWindow::stalemate() {
    readWriteSocket->write("Stalemate\n");
    QMessageBox::information(this, "Stalemate", "Stalemate!");
    endGame();
}

void MainWindow::openGame() {
    if(!connected)
        return;
    endGame();
    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            chess[i][j]->setChess();
            chess[i][j]->setMargin(Qt::transparent);
        }
    }
    if(config == nullptr) { //client
        ui->turnLabel->setText("LOAD GAME!");
        repaint();
        readWriteSocket->write("AskOpen\n");
    }
    else {
        ui->turnLabel->setText("LOAD GAME!");
        repaint();
        readWriteSocket->write("Open\n");
        readWriteSocket->write("Stop check\n");
        beatTimer->stop();
        serverGameConfig *newConfig = new serverGameConfig(true);
        newConfig->setModal(false);
        connect(newConfig, SIGNAL(gameConfigResult(QString)), this, SLOT(gameConfig(QString)));
        newConfig->show();
    }
}

void MainWindow::askLoad() {
    readWriteSocket->write("AskLoad\n");
}

void MainWindow::loadGame() {
    //QMessageBox::information(this, "load", "load");
    readWriteSocket->write("Stop check\n");
    beatTimer->stop();
    QString path = QFileDialog::getOpenFileName(nullptr,"Open",QDir::homePath(),"Text File(*.txt)");
    readWriteSocket->write("Start check\n");
    beatTimer->start(1000);
    QFile *file = new QFile(path);
    QStringList inputList;
    if(file->exists()) {
        file->open(QIODevice::ReadOnly | QIODevice::Text);
        while (!file->atEnd()) {
            QByteArray line = file->readLine();
            QString str(line);
            str.chop(1);
            inputList.append(str);
        }
        file->close();
    }
    qDebug() << inputList;
    curSide = (inputList.at(0) == "white") ? 1 : 0;
    QString cur = inputList.at(0);
    for(int i = 1; i < inputList.size(); i ++) {
        QString command = inputList.at(i);
        if(command == "black" || command == "white") {
            cur = command;
            continue;
        }
        QStringList list = command.split(" ");
        QString type = list.at(0);
        if(list.size() < 3)
            continue;
        int row, col;
        for(int j = 2; j < list.size(); j ++) {
            char* in = list.at(j).toLatin1().data();
            switch(in[0]) {
                case 'a':
                    col = 0;
                    break;
                case 'b':
                    col = 1;
                    break;
                case 'c':
                    col = 2;
                    break;
                case 'd':
                    col = 3;
                    break;
                case 'e':
                    col = 4;
                    break;
                case 'f':
                    col = 5;
                    break;
                case 'g':
                    col = 6;
                    break;
                case 'h':
                    col = 7;
                    break;
            }
            row = in[1] - '1';
            chess[row][col]->setChess(cur, type);
        }
    }
    QString block = (curSide) ? "w" : "b";
    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            if(chess[i][j]->getSide() == "")
                block += "e";
            else if(chess[i][j]->getSide() == "white") {
                if(chess[i][j]->getType() == "king")
                    block += "g";
                else if(chess[i][j]->getType() == "queen")
                    block += "q";
                else if(chess[i][j]->getType() == "rook")
                    block += "r";
                else if(chess[i][j]->getType() == "knight")
                    block += "k";
                else if(chess[i][j]->getType() == "bishop")
                    block += "h";
                else if(chess[i][j]->getType() == "pawn")
                    block += "p";
            }
            else if(chess[i][j]->getSide() == "black") {
                if(chess[i][j]->getType() == "king")
                    block += "G";
                else if(chess[i][j]->getType() == "queen")
                    block += "Q";
                else if(chess[i][j]->getType() == "rook")
                    block += "R";
                else if(chess[i][j]->getType() == "knight")
                    block += "K";
                else if(chess[i][j]->getType() == "bishop")
                    block += "H";
                else if(chess[i][j]->getType() == "pawn")
                    block += "P";
            }
        }
    }
    block += "\n";
    readWriteSocket->write(block.toLatin1().data());
    loadStart();
}

void MainWindow::loadStart() {
    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            chess[i][j]->setMargin(Qt::transparent);
        }
    }
    initBoard();
    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            chess[i][j]->setActiveness(true);
            connect(chess[i][j], SIGNAL(checkerClicked(int,int)), this, SLOT(checkerClicked(int,int)));
        }
    }
    activeness = true;
    selected = 0;
    prevRow = prevCol = -1;
    castling = false;
    turnUpdate();
    initTimer();
    repaint();
}

void MainWindow::saveGame() {
    if(!activeness)
        return;
    vector<Point> chessCnt[2][6];
    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            if(chess[i][j]->getSide() == "black") {
                if(chess[i][j]->getType() == "king") {
                    Point p;
                    p.row = i; p.col = j;
                    chessCnt[0][0].push_back(p);
                }
                else if(chess[i][j]->getType() == "queen") {
                    Point p;
                    p.row = i; p.col = j;
                    chessCnt[0][1].push_back(p);
                }
                else if(chess[i][j]->getType() == "rook") {
                    Point p;
                    p.row = i; p.col = j;
                    chessCnt[0][2].push_back(p);
                }
                else if(chess[i][j]->getType() == "bishop") {
                    Point p;
                    p.row = i; p.col = j;
                    chessCnt[0][3].push_back(p);
                }
                else if(chess[i][j]->getType() == "knight") {
                    Point p;
                    p.row = i; p.col = j;
                    chessCnt[0][4].push_back(p);
                }
                else if(chess[i][j]->getType() == "pawn") {
                    Point p;
                    p.row = i; p.col = j;
                    chessCnt[0][5].push_back(p);
                }
            }
            else {
                if(chess[i][j]->getType() == "king") {
                    Point p;
                    p.row = i; p.col = j;
                    chessCnt[1][0].push_back(p);
                }
                else if(chess[i][j]->getType() == "queen") {
                    Point p;
                    p.row = i; p.col = j;
                    chessCnt[1][1].push_back(p);
                }
                else if(chess[i][j]->getType() == "rook") {
                    Point p;
                    p.row = i; p.col = j;
                    chessCnt[1][2].push_back(p);
                }
                else if(chess[i][j]->getType() == "bishop") {
                    Point p;
                    p.row = i; p.col = j;
                    chessCnt[1][3].push_back(p);
                }
                else if(chess[i][j]->getType() == "knight") {
                    Point p;
                    p.row = i; p.col = j;
                    chessCnt[1][4].push_back(p);
                }
                else if(chess[i][j]->getType() == "pawn") {
                    Point p;
                    p.row = i; p.col = j;
                    chessCnt[1][5].push_back(p);
                }
            }
        }
    }
    readWriteSocket->write("Stop check\n");
    beatTimer->stop();
    QString path = QFileDialog::getSaveFileName(this,tr("Open File"),".",tr("Text File(*.txt)"));
    readWriteSocket->write("Start check\n");
    beatTimer->start(1000);
    QFile *file = new QFile(path);
    file->open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(file);
    int cur;
    cur = (curSide == 1);
    for(int times = 0; times < 2; times++, cur = (cur+1)%2) {
        if(cur == 0) {
            out << "black\n";
        }
        else {
            out << "white\n";
        }
        for(int i = 0; i < 6; i ++) {
            if(chessCnt[cur][i].size() > 0) {
                switch (i) {
                    case 0:
                        out << "king ";
                        break;
                    case 1:
                        out << "queen ";
                        break;
                    case 2:
                        out << "rook ";
                        break;
                    case 3:
                        out << "bishop ";
                        break;
                    case 4:
                        out << "knight ";
                        break;
                    case 5:
                        out << "pawn ";
                        break;
                }
                out << chessCnt[cur][i].size() << " ";
                for(int j = 0; j < chessCnt[cur][i].size(); j ++) {
                    out << (char)((chessCnt[cur][i])[j].row + 'a');
                    out << (chessCnt[cur][i])[j].col;
                    out << " ";
                }
                out << "\n";
            }
        }
    }
    file->close();
}
