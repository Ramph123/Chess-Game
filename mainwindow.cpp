#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include "clientgameconfig.h"
#include <QTimer>
#include <iostream>

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
    dark.setRgb(174,136,104);
    light.setRgb(237,218,185);

    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            QColor color = ((i+j)%2==0) ? light : dark;
            chess[i][j] = new MyGraphicsItem(i, j, color);
            chess[i][j]->setPos(j*60, (7-i)*60);
            m_scene->addItem(chess[i][j]);
        }
    }
    update();
    QGraphicsRectItem *item = new QGraphicsRectItem(-30,-30,480,480);
    QPen pen(Qt::black, 5);
    item->setPen(pen);
    m_scene->addItem(item);
}

QString MainWindow::side2String(bool in) {
    return (in) ? "black" : "white";
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
    connected = false;
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
            //QMessageBox::information(this, "success", "Connection established. Please wait for server to configure the game.");
            ui->turnLabel->setText("WAITING CONFIG");
            update();
        }
    }
}

void MainWindow::connectTimeout() {
    QMessageBox::critical(this, "timeout", "Connect Timeout!");
    disconnect();
}

void MainWindow::acceptConnection()
{
    this->readWriteSocket =this->listenSocket->nextPendingConnection();
    emit connectSuccess();
    listenSocket->close();
    connected = true;
    statusUpdate();
    ui->turnLabel->setText("WAITING CONFIG");
    update();
    config = new serverGameConfig;
    config->setModal(false);
    connect(config, SIGNAL(gameConfigResult(QString)), this, SLOT(gameConfig(QString)));
    //connect(config, SIGNAL(gameConfigResult(QString)), this, SLOT(newGame_Passive()));
    config->show();
    //newGame_Passive();
    connect(this->readWriteSocket, SIGNAL(disconnected()), this, SLOT(disconnect()));
    connect(this->readWriteSocket,SIGNAL(readyRead()),this,SLOT(recvMessage()));
}

void MainWindow::gameConfig(QString result) {
    int in_side, in_enable, in_time;
    sscanf(result.toLatin1().data(), "%d-%d-%d", &in_side, &in_enable, &in_time);
    side = (in_side) ? true : false;
    limitEnable = (in_enable) ? true : false;
    if(limitEnable)
        timeLimit = in_time;
    ui->turnLabel->setText("WAITING CONFIRM");
    QString oppoSide = (side) ? "0" : "1";
    QString oppoLimit = (limitEnable) ? "1" : "0";
    QString oppoTime = QString::number(timeLimit);
    QString block = "Config-" + oppoSide + "-" + oppoLimit + "-" + oppoTime;
    readWriteSocket->write(block.toLatin1().data());
}

void MainWindow::acceptStart() {
    readWriteSocket->write("Game start");
    newGame_Passive();
}

void MainWindow::recvMessage()
{
    qDebug() << "read";
    QString info;
    //QDataStream stream(readWriteSocket);
    //stream >> info;
    info += this->readWriteSocket->readAll();
    qDebug() << info;
    if(info == "Admit defeat") {
        QMessageBox::information(this, "information", "Opponent admits defeat. You win!");
        readWriteSocket->write("Defeat information get");
        //stream << "Defeat information get";
        endGame();
    }
    else if (info == "Defeat information get") {
        QMessageBox::information(this, "information", "You Lose!");
        endGame();
    }
    else if(info == "Ask draw") {
        timer->stop();
        ui->turnLabel->setText("WAITING DECISION ...");
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Ask for draw", "Opponent asks for a draw. Do you agree?");
        if(reply == QMessageBox::No) {
            readWriteSocket->write("Refuse draw");
            timer->start(1000);
            turnUpdate();
        }
        else {
            readWriteSocket->write("Accept draw");
            QMessageBox::information(this, "Draw", "Draw!");
            endGame();
        }
    }
    else if(info == "Refuse draw") {
        timer->start(1000);
        QMessageBox::information(this, "Reply", "Opponent refuses a draw!");
        turnUpdate();
    }
    else if(info == "Accept draw") {
        QMessageBox::information(this, "Reply", "Opponent accepts a draw!");
        endGame();
    }
    else if(info == "New game") {
        QMessageBox::information(this, "New Game", "server starts a new game!");
        ui->turnLabel->setText("WAITING CONFIG");
        update();
    }
    else if(info == "Ask new game") {
        QMessageBox::information(this, "New game", "Client asks for a new game");
        newGame();
    }
    else if(info.left(6) == "Config") {
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
    else if(info == "Game start") {
        newGame_Passive();
    }
    else if(info.left(5) == "Click") {
        int row, col;
        sscanf(info.toLatin1().data(), "Click %d-%d", &row, &col);
        clickChess(row, col);
        getAccessible(row, col);
        paintAccessible();
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
            cleanAccessible();
            getAccessible(prevRow, prevCol);
            for(int i = 0; i < accessible.size(); i ++) {
                oppoControl[accessible[i].row][accessible[i].col] --;
            }
            cleanAccessible();
            moveChess(prevRow, prevCol, row, col);
            getAccessible(row, col);
            for(int i = 0; i < accessible.size(); i ++) {
                oppoControl[accessible[i].row][accessible[i].col] ++;
            }
            cleanAccessible();
//            for(int r = 7; r >= 0; r --) {
//                for(int c = 0; c < 8; c ++) {
//                    std::cout << oppoControl[r][c] << " ";
//                }
//                std::cout << std::endl;
//            }
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
    else if(info == "Stalemate") {
        QMessageBox::information(this, "Stalemate", "Stalemate!");
        endGame();
    }
    else if(info.left(9) == "Checkmate") {
        int kingRow, kingCol;
        sscanf(info.toLatin1().data(), "Checkmate %d-%d", &kingRow, &kingCol);
        chess[kingRow][kingCol]->setMargin(Qt::red);
        moveChess(prevRow, prevCol, kingRow, kingCol);
        QMessageBox::information(this, "Checkmate", "You lose!");
        endGame();
    }
    else if(info.left(5) == "Check") {
        int kingRow, kingCol;
        sscanf(info.toLatin1().data(), "Check %d-%d", &kingRow, &kingCol);
        chess[kingRow][kingCol]->setMargin(Qt::red);
    }
}

void MainWindow::abort() {
    if(mode == 0) {
        endGame();
        listenSocket->close();
        listenSocket = nullptr;
        connected = false;
        statusUpdate();
    }
    else {
        disconnect();
    }
}

void MainWindow::disconnect() {
    endGame();
    readWriteSocket->disconnectFromHost();
    readWriteSocket->close();
    readWriteSocket = nullptr;
    connected = false;
    statusUpdate();
}

void MainWindow::endGame() {
    qDebug() << "endgame";
    ui->turnLabel->setText("GAME ENDED");
    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            chess[i][j]->setActiveness(false);
        }
    }
    activeness = false;
}

void MainWindow::openGame() {
    //
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
        readWriteSocket->write("Ask new game");
    }
    else {
        config->show();
        readWriteSocket->write("New game");
    }
}

void MainWindow::newGame_Passive() {
    ui->turnLabel->clear();
    if(activeness)
        return;
    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            chess[i][j]->setActiveness(true);
            chess[i][j]->setChess();
            connect(chess[i][j], SIGNAL(checkerClicked(int,int)), this, SLOT(checkerClicked(int,int)));
        }
    }
    activeness = true;
    curSide = 1;
    selected = 0;
    prevRow = prevCol = -1;
    isChecked = false;
    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            oppoControl[i][j] = 0;
        }
    }
    turnUpdate();
    initTimer();
    initChess();
    if(side == 1) {
        for(int i = 0; i < 8; i ++) {
            oppoControl[4][i] = 1;
            oppoControl[5][i] = 1;
        }
        oppoControl[5][0] = 2;
        oppoControl[5][2] = 2;
        oppoControl[5][5] = 2;
        oppoControl[5][7] = 2;
    }
    else {
        for(int i = 0; i < 8; i ++) {
            oppoControl[2][i] = 1;
            oppoControl[3][i] = 1;
        }
        oppoControl[2][0] = 2;
        oppoControl[2][2] = 2;
        oppoControl[2][5] = 2;
        oppoControl[2][7] = 2;
    }
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
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Admit defeat", "Are you sure you want to admit defeat?");
    qDebug() << "giveup";
    if(reply == QMessageBox::No) {
        return;
    }
    readWriteSocket->write("Admit defeat");
    //QTextStream stream(readWriteSocket);
    //stream << "Admit defeat";
}

void MainWindow::askDraw() {
    if(!activeness || !connected)
        return;
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Ask draw", "Are you sure you want to ask for a draw?");
    qDebug() << "askDraw";
    if(reply == QMessageBox::No) {
        return;
    }
    timer->stop();
    ui->turnLabel->setText("WATING REPLY ...");
    readWriteSocket->write("Ask draw");
}

void MainWindow::turnUpdate() {
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
            chess[tarRow][tarCol]->setMargin(Qt::green);
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
            QString block = "Click " + QString::number(row) + "-" + QString::number(col);
            readWriteSocket->write(block.toLatin1().data());
        }
    }
    else {
        bool checkmate = false;
        if(chess[row][col]->getMargin() == Qt::transparent)
            return;
        if(row == prevRow && col == prevCol) {
            if(isChecked && chess[row][col]->getType() == "king")
                chess[row][col]->setMargin(Qt::red);
            else
                chess[row][col]->setMargin(Qt::transparent);
            cleanAccessible();
            prevRow = prevCol = -1;
            selected = 0;
        }
        else {
            if(chess[row][col]->getType() == "king")
                checkmate = true;
            moveChess(prevRow, prevCol, row, col);
            cleanAccessible();
            chess[prevRow][prevCol]->setMargin(Qt::transparent);
            prevRow = prevCol = -1;
            selected = 0;
            curSide = !curSide;
        }
        if(checkmate){
            QString block = "Checkmate " + QString::number(row) + "-" + QString::number(col);
            readWriteSocket->write(block.toLatin1().data());
            QMessageBox::information(this, "Checkmate", "You win!");
            endGame();
        }
        else {
            QString block = "Move " + QString::number(row) + "-" + QString::number(col);
            readWriteSocket->write(block.toLatin1().data());
        }
        turnUpdate();
        if(limitEnable) {
            timeRemain = timeLimit;
            updateTime();
            timer->start(1000);
        }
    }
}

void MainWindow::judge() {
    bool check = false, checkmate = true;
    int kingRow, kingCol;
    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            if(chess[i][j]->getType() == "king" && chess[i][j]->getSide() == side2String(side)) {
                kingRow = i;
                kingCol = j;
            }
        }
    }
    if(oppoControl[kingRow][kingCol] != 0)
        check = true;
    cleanAccessible();
    getAccessible(kingRow, kingCol);
    if(accessible.size() == 0)
        checkmate = false;
    else {
        for(int i = 0; i < accessible.size(); i ++) {
            if(oppoControl[accessible[i].row][accessible[i].col] == 0) {
                checkmate = false;
                break;
            }
        }
    }
    cleanAccessible();
    if(check == true && checkmate == false) {
        QString block = "Check " + QString::number(kingRow) + "-" + QString::number(kingCol);
        chess[kingRow][kingCol]->setMargin(Qt::red);
        isChecked = true;
        readWriteSocket->write(block.toLatin1().data());
    }
    else if(check == false && checkmate == true) {
        readWriteSocket->write("Stalemate");
        endGame();
        QMessageBox::information(this, "Stalemate", "Stalemate!");
    }
}
