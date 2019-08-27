#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("Chess Game"));
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);

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

    m_scene=new QGraphicsScene;
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
            chess[i][j] = new MyGraphicsItem;
            chess[i][j]->setPos(j*60, (7-i)*60);
            chess[i][j]->setBrush(((i+j)%2==0) ? QBrush(dark) : QBrush(light));
            m_scene->addItem(chess[i][j]);
        }
    }
    update();
    initChess();
    QGraphicsRectItem *item=new QGraphicsRectItem(-30,-30,480,480);
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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connection() {
    //
}

void MainWindow::openGame() {
    //
}

void MainWindow::newGame() {
    //
}

void MainWindow::giveUp() {
    //
}

void MainWindow::askDraw() {
    //
}
