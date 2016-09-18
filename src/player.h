#ifndef PLAYER_H
#define PLAYER_H

#include "const.h"

#include <QLabel>

struct ResultSummary
{
    ResultSummary(int score = 0, double time = 0, char state = ' ', QString detail = "") :
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

    bool operator <(const ResultSummary& B) const { return score < B.score || (score == B.score && time > B.time); }
    bool operator ==(const ResultSummary& B) const { return score == B.score && time == B.time; }
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
    ~ResultLabel() {}

    /// 根据 style 和是否高亮计算 StyleSheet 字符串
    static QString GetLabelStyleSheet(Global::LabelStyle style, bool isHighlighted = false);

    // Getter member functions
    int Score() const { return res.score; }
    double Time() const { return res.time; }
    char State() const { return res.state; }
    ResultSummary Result() const { return res; }
    Global::LabelStyle LabelStyle() const { return style; }

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
    static const QStringList COLOR_NAME_LIST;

    ResultSummary res;
    Global::LabelStyle style;
};



class Player
{
public:
    explicit Player(const QString& name = "", int id = 0, int probNum = 0);

    // Getter member functions
    int Id() const { return id; }
    QString Name() const { return name; }
    QString NameInList() const { return name_in_list; }
    ResultLabel* NameLabel() const { return name_label; }
    ResultLabel* SumLabel() const { return sum_label; }
    ResultLabel* ProblemLabelAt(int column) const { return prob_label[column]; }
    ResultLabel* LabelAt(int column) const { return !column ? name_label : column == 1 ? sum_label : prob_label[column - 2]; }

    // Setter member functions
    void SetId(int _id) { id = _id; }

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

    /// 保存测评结果到HTML文件
    void SaveHTMLResult();

    /// 比较函数
    friend inline bool CmpName(Player* x, Player* y);
    friend inline bool CmpSumScore(Player* x, Player* y);
    friend bool CmpProblem(Player* x, Player* y);

private:
    int id, priority;
    QString name, name_in_list;

    ResultLabel *name_label, *sum_label;
    QList<ResultLabel*> prob_label;
};

inline bool CmpName(Player* x, Player* y)
{
    return x->priority > y->priority || (x->priority == y->priority && x->name < y->name);
}

inline bool CmpSumScore(Player* x, Player* y)
{
    return  x->sum_label->Result() <  y->sum_label->Result() ||
           (x->sum_label->Result() == y->sum_label->Result() && !CmpName(x, y));
}

bool CmpProblem(Player* x, Player* y);

#endif // PLAYER_H
