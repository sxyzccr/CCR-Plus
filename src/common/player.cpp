#include <QFile>
#include <QTextStream>
#include <QDomDocument>

#include "common/global.h"
#include "common/player.h"
#include "common/problem.h"
#include "common/version.h"

const QStringList ResultLabel::COLOR_NAME_LIST =
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
    QColor color(0, 0, 0), background(COLOR_NAME_LIST[(size_t)style]);
    if (!isHighlighted) color.setAlpha(128), background.setAlpha(192);

    QString s = QString("QLabel{color:%1;background:%2;}").arg(color.name(QColor::HexArgb)).arg(background.name(QColor::HexArgb));
    if (isHighlighted) s += "QLabel{border-width:1px;}";
    return s;
}



Player::Player(const QString& name, int id, int probNum) :
    id(id), priority(0), name(name), name_in_list(""),
    name_label(new ResultLabel(name)), sum_label(new ResultLabel)
{
    for (int i = 0; i < probNum; i++) prob_label.append(new ResultLabel);
}

void Player::CalcSum()
{
    sum_label->SetResult(ResultSummary());
    for (auto i : prob_label)
    {
        if (!i->State()) i->SetState(' ');
        if (i->State() != 'N' && i->State() != 'E') i->SetScore(0), i->SetTime(0);
        sum_label->Plus(i->Result());
    }
}

QString Player::GetNameWithList() const
{
    if (Global::g_contest.is_list_used && !priority && !name_in_list.isEmpty())
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
        if (!nameInList.isEmpty())
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
    const QList<SpecialNameMap> list =
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
            if (!i.name_show.isEmpty()) tmp->setText(i.name_show);
            tmp->setStyleSheet(i.style);
        }
}

void Player::SaveHTMLResult()
{
    QDomDocument doc, res;
    QDomElement html = doc.createElement("html");
    doc.appendChild(html);

    QDomElement head = doc.createElement("head");
    html.appendChild(head);
    QDomElement meta = doc.createElement("meta");
    meta.setAttribute("http-equiv", "Content-Type");
    meta.setAttribute("content", "text/html; charset=utf-8");
    head.appendChild(meta);
    meta = doc.createElement("title");
    meta.appendChild(doc.createTextNode(QString("%1 的测评结果").arg(name)));
    head.appendChild(meta);

    head = doc.createElement("body");
    html.appendChild(head);
    QDomElement table = doc.createElement("table");
    table.setAttribute("width", 500);
    table.setAttribute("cellpadding", 2);
    table.setAttribute("style", "font-family:verdana");
    head.appendChild(table);

    auto addRow = [&](const QString& note, const QString& state)
    {
        QDomElement tr = doc.createElement("tr");
        tr.setAttribute("height", 25);
        table.appendChild(tr);
        QDomElement td = doc.createElement("td");
        td.setAttribute("colspan", 3);

        QColor fg(80, 80, 80), bg(180, 180, 180);
        if (state == "E") fg.setRgb(0, 0, 0), bg.setRgb(227, 58, 218);
        if (state == " ") fg.setRgb(100, 100, 100), bg.setRgb(235, 235, 235);
        if (state == "title") fg.setRgb(255, 255, 255), bg.setRgb(120, 120, 120);
        if (state == "sum") fg.setRgb(0, 0, 0), bg.setRgb(235, 235, 235);

        td.setAttribute("style", QString("color:%1; background-color:%2").arg(fg.name(), bg.name()));
        td.appendChild(doc.createTextNode(note));
        tr.appendChild(td);
    };

    addRow(QString("总分: %1").arg(sum_label->Score()), "sum");

    for (auto p : Global::g_contest.problem_order)
    {
        QFile file(Global::g_contest.result_path + Global::g_contest.problems[p]->Name() + "/" + name + ".res");
        //qDebug()<<file.fileName();

        addRow(QString("%1: %2").arg(Global::g_contest.problems[p]->Name()).arg(prob_label[p]->Score()), "title");

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            addRow("无测评结果", " ");
            continue;
        }
        if (!res.setContent(&file)) { file.close(), addRow("无效的测评结果", " "); continue; }
        QDomElement rt = res.documentElement();
        if (rt.isNull() || rt.tagName() != "task") { file.close(), addRow("无效的测评结果", " "); continue; }

        QDomNodeList list = rt.childNodes();
        for (int i = 0; i < list.count(); i++)
            if (list.item(i).toElement().tagName() == "note")
                addRow(list.item(i).toElement().text(), rt.attribute("state"));

        for (int i = 0, tasktot = 0, tot = 0; i < list.count(); i++)
        {
            QDomElement a = list.item(i).toElement();
            if (a.tagName() == "subtask")
            {
                QDomNodeList l = a.childNodes();
                int k = 0, kk = 0;
                for (int j = 0; j < l.count(); j++)
                    if (l.item(j).toElement().tagName() == "point") kk++;
                for (int j = 0; j < l.count(); j++)
                {
                    QDomElement b = l.item(j).toElement();
                    if (b.tagName() == "point")
                    {
                        QDomElement tr = doc.createElement("tr");
                        tr.setAttribute("height", 25);
                        table.appendChild(tr);
                        QDomElement td = doc.createElement("td");
                        td.setAttribute("width", 25);
                        td.setAttribute("align", "right");
                        td.setAttribute("style", "background-color:#ebebeb"); //rgb(235,235,235)
                        td.appendChild(doc.createTextNode(QString::number(tot + 1)));
                        tr.appendChild(td);

                        if (!k)
                        {
                            td = doc.createElement("td");
                            td.setAttribute("width", 50);
                            td.setAttribute("rowspan", kk);
                            td.setAttribute("align", "center");
                            QColor color = Global::GetRatioColor(235, 235, 235, 0, 161, 241,
                                                                 a.attribute("score").toInt(),
                                                                 tasktot < Global::g_contest.problems[p]->SubtaskCount() ?
                                                                           Global::g_contest.problems[p]->SubtaskAt(tasktot)->Score() :
                                                                           0);
                            td.setAttribute("style", QString("background-color:%1").arg(color.name()));
                            td.appendChild(doc.createTextNode(a.attribute("score")));
                            tr.appendChild(td);
                        }

                        QColor o(255, 255, 255);
                        if (b.attribute("state").length() == 1)
                            switch (b.attribute("state")[0].toLatin1())
                            {
                            case 'A':
                                o.setRgb(51, 185, 6); //AC
                                break;
                            case 'C':
                            case 'E':
                                o.setRgb(227, 58, 218); //Error
                                break;
                            case 'I':
                            case 'U':
                                o.setRgb(235, 235, 235); //Ignore/UnSubmit
                                break;
                            case 'M':
                            case 'R':
                                o.setRgb(247, 63, 63); //MLE/RE
                                break;
                            case 'O':
                                o.setRgb(180, 180, 180); //No Output
                                break;
                            case 'P':
                                o.setRgb(143, 227, 60); //Partial
                                break;
                            case 'W':
                                o.setRgb(246, 123, 20); //WA
                                break;
                            case 'T':
                                o.setRgb(255, 187, 0); //TLE
                                break;
                            }
                        td = doc.createElement("td");
                        td.setAttribute("style", QString("background-color:%1").arg(o.name()));
                        td.appendChild(doc.createTextNode(b.attribute("note")));
                        tr.appendChild(td);
                        k++, tot++;
                    }
                }
                tasktot++;
            }
        }
        file.close();
    }
    QDomElement p = doc.createElement("p");
    p.appendChild(doc.createTextNode("本文件由 CCR Plus 测评器生成"));
    head.appendChild(p);

    p = doc.createElement("p");
    p.appendChild(doc.createTextNode(QString("版本: %1").arg(VERSION_LONG)));
    head.appendChild(p);

    p = doc.createElement("p");
    p.appendChild(doc.createTextNode("项目主页: "));
    QDomElement a = doc.createElement("a");
    a.setAttribute("href", "https://github.com/sxyzccr/CCR-Plus");
    a.appendChild(doc.createTextNode("https://github.com/sxyzccr/CCR-Plus"));
    p.appendChild(a);
    head.appendChild(p);

    p = doc.createElement("hr");
    p.setAttribute("width", 500);
    p.setAttribute("size", 1);
    p.setAttribute("align", "left");
    p.setAttribute("color", "#b4b4b4"); //rgb(180,180,180)
    head.appendChild(p);

    p = doc.createElement("p");
    p.appendChild(doc.createTextNode(QString("Copyright © %1 绍兴一中 贾越凯。保留所有权利。").arg(YEAR_STRING)));
    head.appendChild(p);

    QFile file(Global::g_contest.src_path + name + "/result.html");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    doc.save(out, 4);
    file.close();
}

bool CmpProblem(const Player* x, const Player* y)
{
    static char F[128];
    F[' '] = 0, F['F'] = 1, F['S'] = 2, F['C'] = 3, F['E'] = 4, F['N'] = 5;
    const ResultLabel *a = x->ProblemLabelAt(Global::g_sort_key_col),
                      *b = y->ProblemLabelAt(Global::g_sort_key_col);

    if (!a->Score() && !b->Score())
        return (F[(size_t)a->State()] <  F[(size_t)b->State()]) ||
               (F[(size_t)a->State()] == F[(size_t)b->State()] && CmpSumScore(x, y));
    else
        return (a->Result() <  b->Result()) ||
               (a->Result() == b->Result() && CmpSumScore(x, y));
}
