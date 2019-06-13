#ifndef ALGTHREAD_H
#define ALGTHREAD_H

#include <QObject>
#include <queue>
#include <set>
#include <bitset>
#include <QPoint>
#include "MetaTypes.h"

#define BoardWidth 15

/*
算法部分，主要采用stl。
核心算法及评分机制copy自网络。

优点:
1.采用stl部分数据结构，代码量少，易读懂
2.算法中变量均未采用成员变量或者其它生命周期较长的变量保存，可读性强

缺点:
1.很多运算相关的值未保存，导致算法每次执行都需要计算一遍，增加了不必要的运算，降低了执行速度
*/


//棋子状态采用bitset保存,运算速度快。 缺点：bitset值只有两种状态0,1
//状态长度可能为6和5，故以5保存，next核查长度为6的情况
struct ShapeScore
{
    std::bitset<5> b1;
    std::bitset<5> b2;
    std::bitset<5> b3;
    std::bitset<5> b4;
    std::bitset<5> b5;
    std::bitset<5> b6;
    std::bitset<5> b7;
    std::bitset<5> b8;
    std::bitset<5> b9;
    std::bitset<5> b10;
    std::bitset<5> b11;
    std::bitset<5> b12;
    std::bitset<5> b13;
    //std::bitset<5> b14;
    std::bitset<5> b15;

    ShapeScore(){
        b1 =  std::bitset<5>("01100");
        b2 =  std::bitset<5>("00110");

        b3 =  std::bitset<5>("11010");

        b7 =  std::bitset<5>("01011"); //010110
        b8 =  std::bitset<5>("01101"); //011010

        b4 =  std::bitset<5>("00111");
        b5 =  std::bitset<5>("11100");

        b6 =  std::bitset<5>("01110");

        b9 =  std::bitset<5>("11101");
        b10 = std::bitset<5>("11011");
        b11 = std::bitset<5>("10111");
        b12 = std::bitset<5>("11110");
        b13 = std::bitset<5>("01111");
        //b14 = std::bitset<5>("01111"); //011110
        b15 = std::bitset<5>("11111");
    }

    //匹配得分
    int LoadScore(std::bitset<5> b, int next){
        if(b==b1) return 50;
        else if(b==b2) return  50;
        else if(b==b3) return  200;
        else if(b==b4) return  800;
        else if(b==b5) return  800;
        else if(b==b6) return  3200;
        else if(b==b7) {
            return next==0?3200:200;
        }
        else if(b==b8) {
            return next==0?3200:200;
        }
        else if(b==b9) return  3200;
        else if(b==b10) return 3200;
        else if(b==b11) return 3200;
        else if(b==b12) return 3200;
        else if(b==b13) {
            return next==0? 5e5 : 3200;
        }
        else if(b==b15) return 1e6;  //120*1e6<INT_MAX
        else return 0;
    }

};

//需要匹配的线
struct GobangLine{
    int x;
    int y;
    int axis; // 0横，1竖，2右下，3右上

    GobangLine(){x=0;y=0;axis=0;}

    GobangLine(int i, int j, int _axis){
        x=j;
        y=i;
        axis = _axis;
        stdline();
    }

    //标准化,使得直线表示唯一：x,y坐落在棋盘边上,axis为朝向
    void stdline(){
        switch (axis) {
        case 0:{
            x = 0;
            break;
        }
        case 1:{
            y = 0;
            break;
        }
        case 2:{
            while(x>0 && y>0){
                x--;
                y--;
            }
            break;
        }
        case 3:{
            while(x<14&&y>0){
                x++;
                y--;
            }
            break;
        }
        default:
            break;
        }
    }


    //用于std::find等函数
    bool operator==(const GobangLine &line) const{
         return x==line.x && y==line.y && axis==line.axis;
    }

    //用于 插入std::set结构
    bool operator <(const GobangLine &line) const{
         return 100*(axis+10)+10*x + y < 100*(line.axis+10)+ 10*line.x + line.y;
    }

};

class AlgThread : public QObject
{
    Q_OBJECT
public:
    explicit AlgThread(QObject *parent = 0);

signals:
    //发送下一步落子位置给窗口
    void sendResult(int x, int y, int id);

public slots:
    //设置棋力
    void setLevel(int lv);

    //接收棋盘落子状态
    void RecieveData(ChessStatus &status,  QPoint p, int id);

private:
    int lv_; //棋力值, 递归深度depth

    //==============algorithm 部分===========================================================================================================
    ShapeScore shape_score;
    //获取需要估分的某条线(即线上存在黑白子)
    std::set<GobangLine> GetLinesNeedEvalue(ChessStatus &status, bool is_ai);

    //评估该线的得分
    int GetLineScore(const GobangLine &line, ChessStatus &status, bool is_ai);

    //评估整个棋盘黑子或者白子的得分, status棋盘状态, is_ai是否为ai（true:白子, false:黑子）
    int EvalueScore(ChessStatus &status, bool is_ai);

    //查找落子位置[alpha-beta剪枝]
    int search(bool is_ai, int cur_depth, int src_depth, int alpha, int beta, ChessStatus &status, QPoint last_step, QPoint &next_step);
    //============== end ====================================================================================================================

public:
    //判断是否分胜负
    static int has_win(ChessStatus &status);

};

#endif // ALGTHREAD_H
