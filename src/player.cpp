#include "global.h"
#include "player.h"

#include <vector>

using namespace std;

Player::Player(const QString& name, int id, int probNum) :
    id(id), priority(0), name(name), name_in_list("")
{
    name_label = new ResultLabel(name);
    sum_label = new ResultLabel();
    for (int i = 0; i < probNum; i++) prob_label.push_back(new ResultLabel());
}

Player::~Player()
{

}

void Player::Clear()
{
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
            tmp->setStyleSheet("QLabel{color:rgb(120,120,120);}");
    }
}

void Player::SetSpecialNameLabel()
{
    struct SpecialNameMap{ QString name_orig, name_show, style; int priority; };
    const vector<SpecialNameMap> list =
    {
        {"std",    "标准程序",  "QLabel{color:blue;font:bold;}",           1},
        {"jyk",    "",         "QLabel{color:red;font:bold;}",            2},
        {"贾越凯",  "",         "QLabel{color:red;font:bold;}",            2},
        {"bg",     "",         "QLabel{color:rgb(31,151,63);font:bold;}", -1},
        {"bogang", "",         "QLabel{color:rgb(31,151,63);font:bold;}", -1},
        {"伯刚",    "",         "QLabel{color:rgb(31,151,63);font:bold;}", -1}
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
