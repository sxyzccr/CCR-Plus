#ifndef PLAYER_H
#define PLAYER_H

#include "const.h"

#include <QLabel>

class Player
{
public:
    explicit Player(const QString& na = "", int x = 0);
    ~Player();

    void calcSum();
    void clear();

    class Result
    {
    public:
        Result() { score = 0, usedTime = 0, state = 0, detail = ""; }

        double usedTime;
        int score;
        char state;
        QString detail;
        /*state:
         *N: 正常
         *C: 编译错误
         *F: 找不到文件
         *S: 超过代码长度限制
         *E: 测评器或校验器出错
         * : 未测评
         */

        bool operator <(const Result& B) const
        {
            return score < B.score || (score == B.score && usedTime > B.usedTime);
        }

        bool operator ==(const Result& B) const
        {
            return score == B.score && usedTime == B.usedTime;
        }
    } sum;

    int id, type; //type =  0:normal; -1:std; -2:jyk; 1:bogang;
    QString name, name_list;
    std::vector<Result> problem;
    std::vector<int> style;
    std::vector<QLabel*> label;
};

#endif // PLAYER_H
