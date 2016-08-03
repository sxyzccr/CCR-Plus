#include "global.h"
#include "detailtable.h"

#include <QScrollBar>
#include <QHeaderView>
#include <QDomDocument>
#include <QCoreApplication>

using namespace std;

DetailTable::DetailTable(QWidget* parent) : QTableWidget(parent),
    is_scrollBar_at_bottom(false), is_locked(false), rows(0)
{
    this->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    this->setMinimumSize(QSize(320, 250));
    this->setFocusPolicy(Qt::NoFocus);
    this->setFrameShape(QFrame::NoFrame);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setProperty("showDropIndicator", QVariant(false));
    this->setDragDropOverwriteMode(false);
    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->setStyleSheet(QLatin1String(
                            "QHeaderView\n"
                            "{\n"
                            "	background:#FFFFFF;\n"
                            "}"));

    this->horizontalHeader()->setDefaultSectionSize(45);
    this->horizontalHeader()->setMinimumSectionSize(45);
    this->horizontalHeader()->setFixedHeight(25);
    this->horizontalHeader()->setStretchLastSection(true);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    this->verticalHeader()->setDefaultSectionSize(22);
    this->verticalHeader()->setMinimumSectionSize(22);
    this->verticalHeader()->setMinimumWidth(22);
    this->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
}

void DetailTable::ClearDetail()
{
    this->clear();
    this->setRowCount(0);
    this->setColumnCount(2);
    this->setHorizontalHeaderLabels({"得分", "详情"});
    this->verticalScrollBar()->setValue(0);

    is_scrollBar_at_bottom = false;
    is_show_detail = false;
    is_locked = false;
    rows = 0;
}

void DetailTable::AdjustScrollBar()
{
    QCoreApplication::processEvents();
    QScrollBar* bar = this->verticalScrollBar();
    if (is_scrollBar_at_bottom) bar->setValue(bar->maximum());
}

void DetailTable::ShowProblemDetail(Player* player, Problem* problem)
{
    rows = this->rowCount();
    QString title = player->GetNameWithList();
    if (title == "std") title = QString("\"%1\" 的标程").arg(problem->Name()); else title += " - " + problem->Name();
    onAddTitleDetail(title);

    QFile file(Global::g_contest.result_path + problem->Name() + "/" + player->Name() + ".res");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        onAddNoteDetail("无测评结果", " ");
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(&file))
    {
        file.close();
        onAddNoteDetail("无效的测评结果", " ");
        return;
    }
    QDomElement root = doc.documentElement();
    if (root.isNull() || root.tagName() != "task")
    {
        file.close();
        onAddNoteDetail("无效的测评结果", " ");
        return;
    }

    QDomNodeList list = root.childNodes();
    for (int i = 0; i < list.count(); i++)
        if (list.item(i).toElement().tagName() == "note")
            onAddNoteDetail(list.item(i).toElement().text(), root.attribute("state"));

    for (int i = 0, tot = 0, tasktot = 0; i < list.count(); i++)
    {
        QDomElement a = list.item(i).toElement();
        if (a.tagName() == "subtask")
        {
            QDomNodeList l = a.childNodes();
            int len = 0;
            for (int j = 0; j < l.count(); j++)
            {
                QDomElement b = l.item(j).toElement();
                if (b.tagName() == "point")
                {
                    onAddPointDetail(tot + 1, b.attribute("note"), b.attribute("state"), tot < problem->TestCaseCount() ?
                                                                                               problem->GetInOutString(problem->TestCaseAt(tot)) :
                                                                                               "", ++len);
                    tot++;
                }
            }

            if (len) onAddScoreDetail(len, a.attribute("score").toInt(), tasktot < problem->SubtaskCount() ?
                                                                                   problem->SubtaskAt(tasktot)->Score() :
                                                                                   0);
            tasktot++;
        }
    }
    file.close();
}



void DetailTable::onAddTitleDetail(const QString& title)
{
    if (Global::g_is_contest_closed) return;

    is_scrollBar_at_bottom = this->verticalScrollBar()->value() >= this->verticalScrollBar()->maximum() - 5;

    QTableWidgetItem* tmp = new QTableWidgetItem(title);
    tmp->setTextColor(Qt::white);
    tmp->setBackgroundColor(QColor(120, 120, 120));
    tmp->setToolTip(title);

    this->insertRow(rows);
    this->setItem(rows, 0, tmp);
    this->setSpan(rows, 0, 1, 2);
    this->setVerticalHeaderItem(rows, new QTableWidgetItem);
    rows++;

    if (!is_show_detail) this->AdjustScrollBar();
}

void DetailTable::onAddNoteDetail(const QString& note, const QString& state)
{
    if (Global::g_is_contest_closed) return;

    is_scrollBar_at_bottom = this->verticalScrollBar()->value() >= this->verticalScrollBar()->maximum() - 5;

    QTableWidgetItem* tmp = new QTableWidgetItem(note);
    tmp->setToolTip(tmp->text());
    tmp->setTextColor(QColor(80, 80, 80));
    tmp->setBackgroundColor(QColor(180, 180, 180));
    if (state == "E") tmp->setTextColor(QColor(0, 0, 0)), tmp->setBackgroundColor(QColor(227, 58, 218));
    if (state == " " || state.isEmpty()) tmp->setTextColor(QColor(100, 100, 100)), tmp->setBackgroundColor(QColor(235, 235, 235));
    if (state.isEmpty()) tmp->setTextAlignment(Qt::AlignCenter);

    int a = tmp->text().split('\n').count(), b = min(a, 4) * 17 + 5;
    this->insertRow(rows);
    this->setItem(rows, 0, tmp);
    this->setSpan(rows, 0, 1, 2);
    this->setVerticalHeaderItem(rows, new QTableWidgetItem);
    this->verticalHeader()->resizeSection(rows, b);
    rows++;

    if (!is_show_detail) this->AdjustScrollBar();
}

void DetailTable::onAddPointDetail(int num, const QString& note, const QString& state, const QString& inOut, int subTaskLen)
{
    if (Global::g_is_contest_closed) return;

    is_scrollBar_at_bottom = this->verticalScrollBar()->value() >= this->verticalScrollBar()->maximum() - 5;

    QTableWidgetItem* tmp = new QTableWidgetItem(note);
    tmp->setToolTip(tmp->text());

    QColor o(255, 255, 255);
    if (state == "conf") o.setRgb(0, 161, 241); //Config
    if (state.length() == 1)
        switch (state[0].toLatin1())
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
    tmp->setBackgroundColor(o);

    this->insertRow(rows);
    this->setItem(rows, 1, tmp);

    QTableWidgetItem* t = new QTableWidgetItem(QString::number(num));
    t->setToolTip(inOut);
    this->setVerticalHeaderItem(rows, t);

    if (subTaskLen > 1) this->setSpan(rows - subTaskLen + 1, 0, subTaskLen, 1);
    rows++;

    if (!is_show_detail) this->AdjustScrollBar();
}

void DetailTable::onAddScoreDetail(int subTaskLen, int score, int sumScore)
{
    if (Global::g_is_contest_closed) return;

    QTableWidgetItem* tmp = new QTableWidgetItem(QString::number(score));
    tmp->setTextAlignment(Qt::AlignCenter);
    tmp->setToolTip(tmp->text());
    tmp->setBackgroundColor(Global::GetRatioColor(235, 235, 235, 0, 161, 241, score, sumScore));
    this->setItem(rows - subTaskLen, 0, tmp);
}

void DetailTable::onShowDetail(int row, int column)
{
    if (is_locked || (last_judge_timer.isValid() && last_judge_timer.elapsed() < 1000)) return;
    ClearDetail();
    is_show_detail = true;

    row = Global::GetLogicalRow(row);
    if (column > 1)
        ShowProblemDetail(Global::g_contest.players[row], Global::g_contest.problems[column - 2]);
    else
    {
        for (auto i : Global::g_contest.problem_order)
            ShowProblemDetail(Global::g_contest.players[row], Global::g_contest.problems[i]);
    }

    is_show_detail = false;
}

void DetailTable::onShowConfigDetail()
{
    is_show_detail = true;
    rows = this->rowCount();
    for (auto i : Global::g_contest.problem_order)
    {
        Problem* prob = Global::g_contest.problems[i];
        onAddTitleDetail(QString("\"%1\" 的配置结果").arg(prob->Name()));

        int t = 0;
        for (int i = 0; i < prob->SubtaskCount(); i++)
        {
            Subtask* sub = prob->SubtaskAt(i);
            int len = 0;
            for (TestCase* point : *sub) onAddPointDetail(++t, prob->GetInOutString(point), "conf", prob->GetInOutString(point), ++len);
            onAddScoreDetail(sub->Size(), sub->Score(), sub->Score());
        }
    }
    is_show_detail = false;
}
