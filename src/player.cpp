#include "global.h"
#include "player.h"

#include <vector>

using namespace std;

Player::Player(const QString& name, int id, int probNum) :
    id(id), priority(0), name(name), name_in_list(""),
    name_label(new ResultLabel(name)), sum_label(new ResultLabel)
{
    for (int i = 0; i < probNum; i++) prob_label.push_back(new ResultLabel);
}

Player::~Player()
{
    //Clear();
}

const QString ResultLabel::COLOR_NAME[17] =
{
    "#EB1D00", // StyleScore_1_9
    "#E23A00", // StyleScore_10_19
    "#D95700", // StyleScore_20_29
    "#D17400", // StyleScore_30_39
    "#C89100", // StyleScore_40_49
    "#B99F00", // StyleScore_50_59
    "#A09E01", // StyleScore_60_69
    "#869D02", // StyleScore_70_79
    "#6B9C03", // StyleScore_80_89
    "#4BA604", // StyleScore_90_99
    "#33B906", // StyleScore_100
    "#F40000", // StyleScore_0
    "#06EE06", // StyleScoreGreater_100
    "#4B4B4B", // StyleScoreLess_0
    "#B4B4B4", // StyleInvalidOrStateCFS
    "#00A1F1", // StyleRunning
    "#E33ADA"  // StyleStateE
};

QString ResultLabel::GetLabelStyleSheet(Global::LabelStyle style, bool isHighlighted)
{
    if (style == Global::StyleNone) return "";
    QColor color(0, 0, 0), background(COLOR_NAME[(int)style]);
    if (!isHighlighted) color.setAlpha(128), background.setAlpha(192);

    QString s = QString("QLabel{color:%1;background:%2;}").arg(color.name(QColor::HexArgb)).arg(background.name(QColor::HexArgb));
    if (isHighlighted) s += "QLabel{border-width:1px;}";
    return s;
}

void Player::Clear()
{
    if (name_label) name_label->deleteLater();
    if (sum_label) sum_label->deleteLater();
    for (auto i : prob_label) i->deleteLater();
    prob_label.clear();
}

void Player::CalcSum()
{
    sum_label->SetResult(ResultSummary());
    for (auto i : prob_label)
    {
        if (!i->GetState()) i->SetState(' ');
        if (i->GetState() != 'N' && i->GetState() != 'E') i->SetScore(0), i->SetTime(0);
        sum_label->Plus(i->GetResult());
    }
}

QString Player::GetNameWithList() const
{
    if (Global::g_contest.is_list_used && !priority && name_in_list != "")
        return QString("%1 [%2]").arg(name, name_in_list);
    else
        return name;
}

void Player::SetNameLabelWithoutList()
{
    if (!this->priority)
    {
        QLabel* tmp = this->name_label;
        this->name_in_list = "", tmp->setText(this->name);
        tmp->setAlignment(Qt::AlignCenter);
        tmp->setStyleSheet("");
    }
}

void Player::SetNameLabelWithList(const QString& nameInList)
{
    if (!this->priority)
    {
        QLabel* tmp = this->name_label;
        if (nameInList != "")
        {
            this->name_in_list = nameInList;
            if (nameInList.length() == 2) tmp->setText(QString("%1 [%2   %3]").arg(this->name, nameInList.at(0), nameInList.at(1)));
            else tmp->setText(QString("%1 [%2]").arg(this->name, nameInList));
            tmp->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            tmp->setStyleSheet("");
        }
        else
            tmp->setStyleSheet("QLabel{color:#787878;}");
    }
}

void Player::SetSpecialNameLabel()
{
    struct SpecialNameMap{ QString name_orig, name_show, style; int priority; };
    const vector<SpecialNameMap> list =
    {
        {"std",    "标准程序", "QLabel{color:blue;font:bold;}",     1},
        {"jyk",    "",        "QLabel{color:red;font:bold;}",      2},
        {"贾越凯",  "",        "QLabel{color:red;font:bold;}",      2},
        {"bg",     "",        "QLabel{color:#1F973F;font:bold;}", -1},
        {"bogang", "",        "QLabel{color:#1F973F;font:bold;}", -1},
        {"伯刚",    "",        "QLabel{color:#1F973F;font:bold;}", -1}
    };

    QLabel* tmp = this->name_label;
    for (auto i : list)
        if (this->name.toLower() == i.name_orig)
        {
            this->priority = i.priority;
            if (i.name_show != "") tmp->setText(i.name_show);
            tmp->setStyleSheet(i.style);
        }
}

bool CmpProblem(const Player &x, const Player &y)
{
    static char F[128];
    F[' '] = 0, F['F'] = 1, F['S'] = 2, F['C'] = 3, F['E'] = 4, F['N'] = 5;
    ResultLabel *a = x.GetProbLabel(Global::g_sort_key_col),
                *b = y.GetProbLabel(Global::g_sort_key_col);

    if (!a->GetScore() && !b->GetScore())
        return  F[a->GetState()] <  F[b->GetState()] ||
               (F[a->GetState()] == F[b->GetState()] && CmpSumScore(x, y));
    else
        return  a->GetResult() <  b->GetResult() ||
               (a->GetResult() == b->GetResult() && CmpSumScore(x, y));
}
