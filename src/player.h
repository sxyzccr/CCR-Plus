#ifndef PLAYER_H
#define PLAYER_H

#include "const.h"

#include <QLabel>

struct ResultSummary
{
    ResultSummary(int score = 0, double time = 0, char state = 0, QString detail = "") :
        score(score), time(time), state(state), detail(detail) {}

    int score;
    double time;
    char state;
    QString detail;
    /* state:
     * 'N': 正常
     * 'C': 编译错误
     * 'F': 找不到文件
     * 'S': 超过代码长度限制
     * 'E': 测评器或校验器出错
     * ' ': 未测评
     */

    bool operator <(const ResultSummary& B) const
    {
        return score < B.score || (score == B.score && time > B.time);
    }

    bool operator ==(const ResultSummary& B) const
    {
        return score == B.score && time == B.time;
    }

    ResultSummary& operator +=(const ResultSummary& B)
    {
        score += B.score;
        time += B.time;
        return *this;
    }

    ResultSummary& operator -=(const ResultSummary& B)
    {
        score -= B.score;
        time -= B.time;
        return *this;
    }
};

class ResultLabel : public QLabel
{
public:
    explicit ResultLabel() : QLabel(), res(), style(Global::StyleNone) {}
    explicit ResultLabel(const QString& text) : QLabel(text), res(), style(Global::StyleNone) {}
    virtual ~ResultLabel() {}

    /// 根据 style 和是否高亮计算 StyleSheet 字符串
    static QString GetLabelStyleSheet(Global::LabelStyle style, bool isHighlighted = false);

    // Getter member functions
    int GetScore() const { return res.score; }
    double GetTime() const { return res.time; }
    char GetState() const { return res.state; }
    ResultSummary GetResult() const { return res; }
    Global::LabelStyle GetLabelStyle() const { return style; }

    // Setter member functions
    void SetScore(int score) { res.score = score; }
    void SetTime(double time) { res.time = time; }
    void SetState(char state) { res.state = state; }
    void SetResult(const ResultSummary& _res) { res = _res; }
    void SetResult(int score = 0, double time = 0, char state = 0, QString detail = "") { res = ResultSummary(score, time, state, detail); }
    void SetLabelStyle(Global::LabelStyle _style) { style = _style; }

    // Plus or subtract ResultSummary
    void Plus(const ResultSummary& _res) { res += _res; }
    void Subtract(const ResultSummary& _res) { res -= _res; }

private:
    static const QString COLOR_NAME[17];

    ResultSummary res;
    Global::LabelStyle style;
};

class Player
{
public:
    explicit Player(const QString& name = "", int id = 0, int probNum = 0);
    virtual ~Player();

    // Getter member functions
    int GetID() const { return id; }
    QString GetName() const { return name; }
    QString GetNameInList() const { return name_in_list; }
    ResultLabel* GetNameLabel() const { return name_label; }
    ResultLabel* GetSumLabel() const { return sum_label; }
    ResultLabel* GetProbLabel(int c) const { return prob_label[c]; }
    ResultLabel* GetLabel(int c) const { return !c ? name_label : c == 1 ? sum_label : prob_label[c - 2]; }

    // Setter member functions
    void SetID(int _id) { id = _id; }

    /// 清空
    void Clear();

    /// 根据每题的 ResultSummary 计算总的 ResultSummary
    void CalcSum();

    /// 根据是否使用名单确定 name
    QString GetNameWithList() const;

    /// 不使用名单，设置 name_label 的文本
    void SetNameLabelWithoutList();

    /// 使用名单，设置 name_label 的文本
    void SetNameLabelWithList(const QString& nameInList);

    /// 设置特殊用户的 name_label 文本
    void SetSpecialNameLabel();

    /// 比较函数
    friend bool CmpName(const Player& x, const Player& y);
    friend bool CmpSumScore(const Player& x, const Player& y);
    friend bool CmpProblem(const Player& x, const Player& y);

private:
    int id, priority;
    QString name, name_in_list;

    ResultLabel *name_label, *sum_label;
    std::vector<ResultLabel*> prob_label;
};

inline bool CmpName(const Player& x, const Player& y)
{
    return x.priority > y.priority || (x.priority == y.priority && x.name < y.name);
}

inline bool CmpSumScore(const Player& x, const Player& y)
{
    return  x.sum_label->GetResult() <  y.sum_label->GetResult()||
           (x.sum_label->GetResult() == y.sum_label->GetResult() && !CmpName(x, y));
}

bool CmpProblem(const Player& x, const Player& y);

#endif // PLAYER_H
