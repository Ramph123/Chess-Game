#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("Chess Game"));
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);

    connected = false;
    activeness = false;
    statusUpdate();

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

void MainWindow::initChess() {
    chess[0][0]->setType("white", "rook");
    chess[0][1]->setType("white", "knight");
    chess[0][2]->setType("white", "bishop");
    chess[0][3]->setType("white", "queen");
    chess[0][4]->setType("white", "king");
    chess[0][5]->setType("white", "bishop");
    chess[0][6]->setType("white", "knight");
    chess[0][7]->setType("white", "rook");
    for(int i = 0; i < 8; i ++)
        chess[1][i]->setType("white","pawn");
    chess[7][0]->setType("black", "rook");
    chess[7][1]->setType("black", "knight");
    chess[7][2]->setType("black", "bishop");
    chess[7][3]->setType("black", "queen");
    chess[7][4]->setType("black", "king");
    chess[7][5]->setType("black", "bishop");
    chess[7][6]->setType("black", "knight");
    chess[7][7]->setType("black", "rook");
    for(int i = 0; i < 8; i ++)
        chess[6][i]->setType("black","pawn");
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
        ui->statusLabel->setText("Connecting as server...  (Address: " + ipAdd + ":" + QString::number(port) + ")");
        if(!listenSocket)
            this->listenSocket = new QTcpServer;
        this->listenSocket->listen(QHostAddress::Any,port);
        connect(this->listenSocket,SIGNAL(newConnection()),this,SLOT(acceptConnection()));
    }
    else {      //client
        ui->statusLabel->setText("Connecting as client...");
        this->readWriteSocket = new QTcpSocket;
        this->readWriteSocket->connectToHost(QHostAddress(ipAdd),port);
        if(readWriteSocket->waitForConnected()) {
            emit connectSuccess();
            newGame();
            connected = true;
            statusUpdate();
            connect(this->readWriteSocket,SIGNAL(readyRead()),this,SLOT(recvMessage()));
        }
    }
}

void MainWindow::acceptConnection()
{
    this->readWriteSocket =this->listenSocket->nextPendingConnection();
    //qDebug() << "????";
    newGame();
    repaint();
    connected = true;
    //qDebug() << "!!!!";
    statusUpdate();
    //qDebug() << "####";
    connect(this->readWriteSocket, SIGNAL(disconnected()), this, SLOT(disconnect()));
    connect(this->readWriteSocket,SIGNAL(readyRead()),this,SLOT(recvMessage()));
    emit connectSuccess();
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
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Ask for draw", "Opponent asks for a draw. Do you agree?");
        if(reply == QMessageBox::No)
            readWriteSocket->write("Refuse draw");
        else {
            readWriteSocket->write("Accept draw");
            QMessageBox::information(this, "Draw", "Draw!");
            endGame();
        }
    }
    else if(info == "Refuse draw") {
        QMessageBox::information(this, "Reply", "Opponent refuses a draw!");
    }
    else if(info == "Accept draw") {
        QMessageBox::information(this, "Reply", "Opponent accepts a draw!");
        endGame();
    }
    else if(info == "New game") {
        QMessageBox::information(this, "New Game", "Opponent starts a new game!");
        newGame_Passive();
    }
}

void MainWindow::abort() {
    if(mode == 0) {
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
    readWriteSocket->disconnectFromHost();
    readWriteSocket->close();
    readWriteSocket = nullptr;
    connected = false;
    statusUpdate();
}

void MainWindow::endGame() {
    qDebug() << "endgame";
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
    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            chess[i][j]->setActiveness(true);
            chess[i][j]->setType();
        }
    }
    activeness = true;
    initChess();
    repaint();
    if(connected)
        readWriteSocket->write("New game");
}

void MainWindow::newGame_Passive() {
    if(activeness)
        return;
    for(int i = 0; i < 8; i ++) {
        for(int j = 0; j < 8; j ++) {
            chess[i][j]->setActiveness(true);
            chess[i][j]->setType();
        }
    }
    activeness = true;
    initChess();
    repaint();
}

void MainWindow::giveUp() {
    if(!activeness || !connected)
        return;
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Admit defeat", "Are you sure you want to give up?");
    qDebug() << "giveup";
    if(reply == QMessageBox::No)
        return;
    readWriteSocket->write("Admit defeat");
    //QTextStream stream(readWriteSocket);
    //stream << "Admit defeat";
}

void MainWindow::askDraw() {
    if(!activeness || !connected)
        return;
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Ask draw", "Are you sure you want to ask for a draw?");
    qDebug() << "askDraw";
    if(reply == QMessageBox::No)
        return;
    readWriteSocket->write("Ask draw");
}
