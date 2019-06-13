#include "algthread.h"

#include <QDebug>
#include <iostream>

AlgThread::AlgThread(QObject *parent) :
    QObject(parent)
{
    lv_ = 3;
}


void AlgThread::setLevel(int lv){
    lv_ = lv;
}

void AlgThread::RecieveData(ChessStatus &status, QPoint p, int id){
     QPoint next_step;
     search(true, lv_, lv_, -INT_MAX, INT_MAX, status, p, next_step);
     emit sendResult(next_step.x(), next_step.y(), id);
}

int AlgThread::has_win(ChessStatus &status){
    for(int i=0; i<status.size()-4; i++){
        for(int j=0; j<status.size()-4; j++){
            if(status[i][j] == 0) continue;
            else{
                if(i<status.size()-4 && status[i][j]==status[i+1][j] && status[i+1][j]==status[i+2][j] && status[i+2][j]==status[i+3][j] && status[i+3][j]==status[i+4][j]) return status[i][j];

                if(i<status.size()-4 && j<status.size()-4 && status[i][j]==status[i+1][j+1] && status[i+1][j+1]==status[i+2][j+2]
                   && status[i+2][j+2]==status[i+3][j+3] && status[i+3][j+3]==status[i+4][j+4]) return status[i][j];

                if(j<status.size()-4&& status[i][j]==status[i][j+1] && status[i][j+1]==status[i][j+2] && status[i][j+2]==status[i][j+3]
                        && status[i][j+3]==status[i][j+4]) return status[i][j];

                if(i>=4 && j<status.size()-4 && status[i][j]==status[i-1][j+1] && status[i-1][j+1] == status[i-2][j+2]
                        && status[i-2][j+2] == status[i-3][j+3] && status[i-3][j+3] == status[i-4][j+4]) return status[i][j];
            }
        }
    }
    return 0;
}

std::set<GobangLine> AlgThread::GetLinesNeedEvalue(ChessStatus &status, bool is_ai){
    std::set<GobangLine> result;
    int color = is_ai? 2 : 1;
    for(int i=0; i<BoardWidth; i++){
        for(int j=0; j<BoardWidth; j++){
            if(status[i][j]==color){
                result.insert(GobangLine(i,j,0));
                result.insert(GobangLine(i,j,1));
                result.insert(GobangLine(i,j,2));
                result.insert(GobangLine(i,j,3));
            }
        }
    }
    return result;
}

int AlgThread::GetLineScore(const GobangLine &line, ChessStatus &status, bool is_ai)
{
    int n_color = is_ai ? 1 : 2;
    int score = 0;
    int directx=0, directy=0;
    switch (line.axis) {
    case 0:{
        directx = 1;
        break;
    }
    case 1:{
        directy = 1;
        break;
    }
    case 2:{
        directx = 1;
        directy = 1;
        break;
    }
    default:
        directx = -1;
        directy = 1;
        break;
    }
    int i=line.y;
    int j = line.x;
    std::bitset<5> bits;
    int index = 0;
    int next = 1;
    while(i>=0 && i<BoardWidth && j>=0 && j<BoardWidth){
        if(status[i][j]==n_color){
            index=0;
        }else{
            bits[index++] = status[i][j]==0? 0:1;
            if(index==5){
                if(i+directy>=0 && i+directy<BoardWidth && j+directx>=0 && j+directx<BoardWidth) next=status[i+directy][j+directx];
                else next = 1;
                score += shape_score.LoadScore(bits, next);
                index =4;
                bits >>= 1;
            }
        }
        i += directy;
        j += directx;

    }
    //wight of location;
    if(line.axis<2) score += 14-abs(7-line.x) - abs(7-line.y);
    else if(line.axis == 2) score += 14 -line.x-line.y;
    else score += line.x-line.y;
    return score;
}

int AlgThread::EvalueScore(ChessStatus &status, bool is_ai){
    int score = 0;
    std::set<GobangLine> lines = GetLinesNeedEvalue(status, is_ai);
    for(GobangLine line: lines){
        score += GetLineScore(line, status, is_ai);
    }
    return score;
}

int AlgThread::search(bool is_ai, int cur_depth, int src_depth, int alpha, int beta, ChessStatus &status, QPoint last_step, QPoint &next_step){
    if(has_win(status) || cur_depth==0) return EvalueScore(status, is_ai) - EvalueScore(status, !is_ai)*0.3;  //评分机制比较简陋进攻/防守不明显
    int i = last_step.x();
    int j = last_step.y();
    int z = 0;

    while(i-z>=0 || j-z>=0 || i+z<BoardWidth || j+z < BoardWidth){ //沿着最近落子处评估可落子位置,不采用过多空间， 4个分枝逻辑一致
        z++;
        if(i-z>=0){
            int jj=j-z>=0? j-z:0;
            while(jj<BoardWidth && jj<= j+z){
                if(status[i-z][jj]==0){
                    status[i-z][jj] = is_ai?2:1;
                    last_step = QPoint(i-z, jj);
                    int score = - search(!is_ai,cur_depth-1, src_depth, -beta, -alpha, status, last_step, next_step);
                    status[i-z][jj]= 0;
                    if(score > alpha){
                        alpha = score;
                        if(cur_depth==src_depth) next_step = last_step;
                        if(alpha >= beta)return alpha;
                    }

                }
                jj++;
            }
        }
        if(i+z<BoardWidth){
            int jj=j-z>=0? j-z:0;
            while(jj<BoardWidth && jj<= j+z){
                if(status[i+z][jj]==0){
                    status[i+z][jj] = is_ai?2:1;
                    last_step = QPoint(i+z, jj);
                    int score = - search(!is_ai,cur_depth-1, src_depth, -beta, -alpha, status, last_step, next_step);
                    status[i+z][jj]= 0;
                    if(score > alpha){
                        alpha = score;
                        if(cur_depth==src_depth) next_step = last_step;
                        if(alpha >= beta)return alpha;
                    }
                }
                jj++;
            }
        }
        if(j-z>=0){
            int ii=i-z>=-1? i-z+1:1;
            while(ii<BoardWidth && ii< i+z){
                if(status[ii][j-z]==0){
                    status[ii][j-z] = is_ai?2:1;
                    last_step = QPoint(ii, j-z);
                    int score = - search(!is_ai, cur_depth-1, src_depth, -beta, -alpha, status, last_step, next_step);
                    status[ii][j-z]= 0;
                    if(score > alpha){
                        alpha = score;
                        if(cur_depth==src_depth) next_step = last_step;
                        if(alpha >= beta)return alpha;
                    }
                }
                ii++;
            }

        }
        if(j+z<BoardWidth){
            int ii=i-z>=-1? i-z+1:1;
            while(ii<BoardWidth && ii< i+z){
                if(status[ii][j+z]==0){
                    status[ii][j+z] = is_ai?2:1;
                    last_step = QPoint(ii, j+z);
                    int score = - search(!is_ai,cur_depth-1, src_depth, -beta, -alpha, status, last_step, next_step);
                    status[ii][j+z]= 0;
                    if(score > alpha){
                        alpha = score;
                        if(cur_depth==src_depth) next_step = last_step;
                        if(alpha >= beta)return alpha;
                    }
                }
                ii++;
            }
        }
    }

    return alpha;
}
