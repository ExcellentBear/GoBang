#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "MetaTypes.h"
#include "algthread.h"
#include <QMainWindow>
#include <QMetaType>
#include <QThread>
#include <QStack>
#include <QPoint>
#include <QTableWidgetItem>
#include <QPainter>
#include <QIcon>
#include <QPixmap>
#include <QPen>
#include <QPalette>
#include <QMessageBox>
#include <QDebug>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;


private slots:
    void TableActions(QAction *obj);
    void resizeEvent(QResizeEvent *evt);
    void YourTurn(int, int);
    void NoYourTurn(int x, int y, int id);
    void GetCurLevel(QAction* ac);
    void gameover(int player);

signals:
    void sendStatus(ChessStatus &status, QPoint p, int id);
    void setlevel(int lv);

private:
    QStack<QPoint> steps_;
    int alg_id_;
    int cur_size_;
    bool turn_;
    int width_;
    int height_;
    ChessStatus status_;
    QThread* thread_;
    AlgThread* alg_;
    void InitTableWidget(int size=15);
    void SetPointEmpty(QPoint p);
    void LayDownChess(QPoint p);
    QPixmap GenerateImage(int type);
};

#endif // MAINWINDOW_H
