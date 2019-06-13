#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qRegisterMetaType<ChessStatus>("ChessStatus");
    qRegisterMetaType<ChessStatus>("ChessStatus&");

    cur_size_ = 15;
    turn_ = true;
    InitTableWidget(cur_size_);

    status_ = ChessStatus(cur_size_, QVector<int>(cur_size_, 0));
    QString title = QString("人机大战   your turn");
    setWindowTitle(title);

    connect(ui->menu1, SIGNAL(triggered(QAction*)), this, SLOT(TableActions(QAction*)));
    connect(ui->menu2, SIGNAL(triggered(QAction*)), this, SLOT(TableActions(QAction*)));
    connect(ui->menu3, SIGNAL(triggered(QAction*)), this, SLOT(GetCurLevel(QAction*)));
    connect(ui->tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(YourTurn(int,int)));

    thread_ = new QThread(this);
    alg_ = new AlgThread(0);
    alg_->moveToThread(thread_);
    alg_id_ = 0;
    thread_->start();
    connect(this, SIGNAL(sendStatus(ChessStatus&, QPoint, int)),alg_, SLOT(RecieveData(ChessStatus&, QPoint, int)), Qt::QueuedConnection);
    connect(this, SIGNAL(setlevel(int)), alg_, SLOT(setLevel(int)), Qt::QueuedConnection);
    connect(alg_, SIGNAL(sendResult(int,int, int)), this, SLOT(NoYourTurn(int,int, int)), Qt::QueuedConnection);  
}

MainWindow::~MainWindow()
{
    thread_->terminate();
    thread_->wait();
    alg_->deleteLater();
    delete ui;
}

void MainWindow::InitTableWidget(int size){
    QRect rect = ui->centralWidget->geometry();
    int col = rect.width()/size;
    int row = (rect.height()-15)/size;
    width_ = col;
    height_ = row;
    ui->tableWidget->setGeometry(rect);
    ui->tableWidget->setColumnCount(size);
    ui->tableWidget->setRowCount(size);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setVisible(false);
    ui->tableWidget->setShowGrid(false);
    for(int i=0; i<size; i++){
        ui->tableWidget->setColumnWidth(i,col);
        ui->tableWidget->setRowHeight(i, row);
    }
    for(int i=0; i<size; i++){
        for(int j=0; j<size; j++){
          if(ui->tableWidget->item(i,j)){
              ui->tableWidget->item(i, j)->setBackground(QBrush(GenerateImage(ui->tableWidget->item(i, j)->type())));
              ui->tableWidget->item(i,j)->setFlags(Qt::NoItemFlags);
          }else{
             ui->tableWidget->setItem(i, j, new QTableWidgetItem("", 0));
             ui->tableWidget->item(i, j)->setBackground(QBrush(GenerateImage(ui->tableWidget->item(i, j)->type())));
             ui->tableWidget->item(i,j)->setFlags(Qt::NoItemFlags);
          }
        }
    }
    cur_size_ = size;
}

void MainWindow::resizeEvent(QResizeEvent *){
   InitTableWidget(cur_size_); 
}

void MainWindow::SetPointEmpty(QPoint p){
    ui->tableWidget->setItem(p.x(), p.y(), new QTableWidgetItem(0));
    ui->tableWidget->item(p.x(), p.y())->setBackground(QBrush(GenerateImage(0)));
    ui->tableWidget->item(p.x(), p.y())->setFlags(Qt::NoItemFlags);
}

void MainWindow::TableActions(QAction *obj){
    if(obj==ui->action15x15 ){
        ui->tableWidget->clearContents();
        steps_.clear();
        turn_ = true;
        InitTableWidget(15);
        status_ = ChessStatus(cur_size_, QVector<int>(cur_size_, 0));
    }

    else if(obj == ui->actionBegin ){
        ui->tableWidget->clearContents();
        turn_ = true;
        steps_.clear();
        InitTableWidget(cur_size_);
        status_ = ChessStatus(cur_size_, QVector<int>(cur_size_, 0));
    }

    else if(obj == ui->actionBack ){
        if(!steps_.empty()) {
            QPoint p = steps_.top();
            steps_.pop();
            SetPointEmpty(p);
            turn_ = status_[p.x()][p.y()] == 1?true:false;
            status_[p.x()][p.y()] = 0;
            if(!turn_){
                p = steps_.top();
                steps_.pop();
                SetPointEmpty(p);
                turn_ = status_[p.x()][p.y()] == 1?true:false;
                status_[p.x()][p.y()] = 0;
            }
            QString title = QString("人机大战  your turn");
            setWindowTitle(title);
        }
    }
}

void MainWindow::LayDownChess(QPoint p){
    if(turn_){
        ui->tableWidget->setItem(p.x(), p.y(), new QTableWidgetItem("", 1));
        ui->tableWidget->item(p.x(), p.y())->setBackground(QBrush(GenerateImage(1)));
        ui->tableWidget->item(p.x(), p.y())->setFlags(Qt::NoItemFlags);
    }else{
        ui->tableWidget->setItem(p.x(), p.y(), new QTableWidgetItem("", 2));
        ui->tableWidget->item(p.x(), p.y())->setBackground(QBrush(GenerateImage(2)));
        ui->tableWidget->item(p.x(), p.y())->setFlags(Qt::NoItemFlags);
    }
    steps_.push(p);
    status_[p.x()][p.y()] = turn_? 1:2;
    turn_ = !turn_;
    QString title = QString("人机大战   ");
    title += turn_ ? QString("your turn"): QString("robot`s turn");
    setWindowTitle(title);
    int winner = AlgThread::has_win(status_);
    if(winner) gameover(winner);
}

void MainWindow::YourTurn(int x, int y){
    if(!turn_) return;
    if(ui->tableWidget->item(x, y) && ui->tableWidget->item(x, y)->type() !=0) return;
    QPoint p(x,y);
    LayDownChess(p);
    sendStatus(status_, p, alg_id_);
}

void MainWindow::NoYourTurn(int x, int y, int id){
    if(turn_ || id != alg_id_) return;
    if(ui->tableWidget->item(x, y) && ui->tableWidget->item(x, y)->type() !=0) return;
    LayDownChess(QPoint(x, y));
    alg_id_++;
}

void MainWindow::GetCurLevel(QAction *ac){
    if(ac == ui->actionLv1) emit setlevel(1);
    else if(ac == ui->actionLv2) emit setlevel(3);
}

QPixmap MainWindow::GenerateImage(int type){
    QPixmap pix(width_, height_);
    pix.fill(QColor(255, 215, 134));
    QPainter painter(&pix);
    if(type==0){
        QPen pen;
        pen.setColor(Qt::black);
        pen.setWidth(2);
        painter.setPen(pen);
        painter.drawLine(width_/2, 0, width_/2, height_);
        painter.drawLine(0, height_/2, width_, height_/2);
    }else if(type==1){
        painter.setPen(Qt::black);
        painter.setBrush(QBrush(Qt::black));
        painter.drawEllipse(QPoint(width_/2,height_/2), width_/2, height_/2);

    }else if(type==2){
        painter.setPen(Qt::white);
        painter.setBrush(QBrush(Qt::white));
        painter.drawEllipse(QPoint(width_/2,height_/2), width_/2, height_/2);
    }
    return pix;
}

void MainWindow::gameover(int player){
    QString winner = player==1 ? "you win":"robot win";
    QMessageBox::information(this, "Game Over", winner);
}
