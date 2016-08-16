#include "testcasetable.h"

#include <QApplication>
#include <QHeaderView>
#include <QDebug>
#include <QLabel>

using namespace std;

TestCaseTable::TestCaseTable(QWidget* parent) :
    QTableWidget(parent), unselect_score_item(nullptr)
{
    this->setFocusPolicy(Qt::NoFocus);
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setSelectionMode(QAbstractItemView::ContiguousSelection);
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setAlternatingRowColors(true);
    this->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->setStyleSheet(QLatin1String(
                            "QHeaderView"
                            "{"
                            "  background:#FFFFFF;"
                            //"}"
                            //"QTableWidget::item:alternate"
                            //"{"
                            //"  color:rgb(0,0,0);"
                            //"  background-color:#FFFFFF;"
                            //"}"
                            //"QTableWidget::item:!alternate"
                            //"{"
                            //"  color:rgb(0,0,0);"
                            //"  background-color:#F8F8F8;"
                            "}"));

    this->horizontalHeader()->setDefaultSectionSize(80);
    this->horizontalHeader()->setMinimumSectionSize(80);
    this->horizontalHeader()->setFixedHeight(22);
    this->horizontalHeader()->setStretchLastSection(true);
    this->horizontalHeader()->setHighlightSections(false);

    this->verticalHeader()->setDefaultSectionSize(22);
    this->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
    this->verticalHeader()->setHighlightSections(false);

    connect(this, &QTableWidget::itemSelectionChanged, this, &TestCaseTable::onItemSelectionChanged);
    connect(this,&QTableWidget::currentItemChanged, this, [this]()
    {
        int top, bottom;
        SelectionType type = GetSelectionType(&top, &bottom);
        if (type != NoSelection && ScoreItemBottomRow(bottom) == bottom && unselect_score_item)
            unselect_score_item->setFlags(unselect_score_item->flags() | Qt::ItemIsSelectable);
    });
}

void TestCaseTable::LoadTestCases(Problem *problem)
{
    score_item.clear();

    this->clear();
    this->setRowCount(0);
    this->setColumnCount(0);

    if (problem == nullptr)
    {
        return;
    }

    if (problem->Type() == Global::Traditional)
    {
        this->setColumnCount(5);
        this->setHorizontalHeaderLabels({"输入文件", "输出文件", "时间限制", "内存限制", "分值"});
    }
    else if (problem->Type() == Global::AnswersOnly)
    {
        this->setColumnCount(4);
        this->setHorizontalHeaderLabels({"输入文件", "输出文件", "提交文件", "分值"});
    }
    else
        return;
    score_column = this->columnCount() - 1;

    int rows = 0;
    for (int i = 0; i < problem->SubtaskCount(); i++)
    {
        Subtask* sub = problem->SubtaskAt(i);
        QTableWidgetItem* tmp;
        int len = 0;
        for (TestCase* point : *sub)
        {
            this->insertRow(rows);

            tmp = new QTableWidgetItem(point->InFile());
            tmp->setTextAlignment(Qt::AlignCenter);
            tmp->setToolTip(tmp->text());
            this->setItem(rows, 0, tmp);

            tmp = new QTableWidgetItem(point->OutFile());
            tmp->setTextAlignment(Qt::AlignCenter);
            tmp->setToolTip(tmp->text());
            this->setItem(rows, 1, tmp);

            if (problem->Type() == Global::Traditional)
            {
                tmp = new QTableWidgetItem(QString::number(point->TimeLimit()));
                tmp->setTextAlignment(Qt::AlignCenter);
                tmp->setToolTip(tmp->text());
                this->setItem(rows, 2, tmp);

                tmp = new QTableWidgetItem(QString::number(point->MemoryLimit()));
                tmp->setTextAlignment(Qt::AlignCenter);
                tmp->setToolTip(tmp->text());
                this->setItem(rows, 3, tmp);
            }
            else if (problem->Type() == Global::AnswersOnly)
            {
                tmp = new QTableWidgetItem(point->SubmitFile());
                tmp->setTextAlignment(Qt::AlignCenter);
                tmp->setToolTip(tmp->text());
                this->setItem(rows, 2, tmp);
            }

            tmp = new QTableWidgetItem(QString::number(sub->Score()));
            tmp->setTextAlignment(Qt::AlignCenter);
            tmp->setToolTip(tmp->text());
            tmp->setBackgroundColor(QColor(255, 255, 255));
            this->setItem(rows, score_column, tmp);

            this->setVerticalHeaderItem(rows, new QTableWidgetItem(QString::number(rows + 1)));
            len++, rows++;
            score_item.push_back(this->item(rows - len, score_column));
        }

        if (len > 1) this->setSpan(rows - len, score_column, len, 1);
    }
}

TestCaseTable::SelectionType TestCaseTable::GetSelectionType(int* _top, int* _bottom)
{
    auto list = this->selectedRanges();
    int top = 1e9, bottom = -1e9;
    for (auto i : list)
    {
        top = min(top, i.topRow());
        bottom = max(bottom, i.bottomRow());
    }
    *_top = top, *_bottom = bottom;
    if (top > bottom) return NoSelection;

    for (int i = top; i <= bottom; i++)
        if (!this->item(i, 0)->isSelected())
            return SelectDiscontinuous;

    if (top == ScoreItemTopRow(top) && bottom == ScoreItemBottomRow(bottom))
    {
        if (top == bottom)
            return SelectOneTestCasePackage;
        if (ScoreItemTopRow(bottom) == top)
            return SelectOnePackage;
        for (int i = top; i <= bottom; i++)
            if (ScoreItemTopRow(i) != ScoreItemBottomRow(i))
                return SelectMultiplePackage;
        return SelectMultipleTestCasePackage;
    }

    if (top == bottom) return SelectOneTestCase;
    return OtherSelection;
}

void TestCaseTable::MergeSelection()
{
    int top, bottom;
    SelectionType type = GetSelectionType(&top, &bottom);
    if (type != SelectMultiplePackage && type != SelectMultipleTestCasePackage) return;

    int sum = 0;
    QTableWidgetItem* scoreItem = score_item[top];
    for (int i = top; i <= bottom; i++)
    {
        if (score_item[i]->row() == i)
        {
            sum += score_item[i]->text().toInt();
            if (this->rowSpan(i, score_column) > 1) this->setSpan(i, score_column, 1, 1);
        }
        score_item[i] = scoreItem;
    }
    this->setSpan(top, score_column, bottom - top + 1, 1);
    scoreItem->setText(QString::number(sum));
    scoreItem->setToolTip(scoreItem->text());
    onItemSelectionChanged();
}

void TestCaseTable::SplitSelection()
{
    int top, bottom;
    SelectionType type = GetSelectionType(&top, &bottom);
    if (type != SelectOnePackage && type != SelectMultiplePackage) return;

    vector<int> score;
    for (int i = top; i <= bottom; i++)
    {
        if (score_item[i]->row() == i && this->rowSpan(i, score_column) > 1)
        {
            int len = this->rowSpan(i, score_column);
            int sum = score_item[i]->text().toInt(), s = sum / len;
            this->setSpan(i, score_column, 1, 1);
            score.clear();
            for (int j = 0; j < len; j++)
            {
                score_item[i + j] = this->item(i + j, score_column);
                score.push_back(s), sum -= s;
            }
            for (int j = len; sum; sum--) score[--j]++;
            for (int j = 0; j < len; j++)
            {
                score_item[i + j]->setText(QString::number(score[j]));
                score_item[i + j]->setToolTip(QString::number(score[j]));
            }
        }
    }
    onItemSelectionChanged();
}



void TestCaseTable::onItemSelectionChanged()
{
    can_add = can_add_sub = can_remove = can_up = can_down = can_merge = can_split = false;

    int top, bottom;
    SelectionType type = GetSelectionType(&top, &bottom);

    if (type == NoSelection) return;
    if (ScoreItemBottomRow(bottom) > bottom)
    {
        if (this->ScoreItemAt(bottom)->isSelected())
        {
            unselect_score_item = this->ScoreItemAt(bottom);
            unselect_score_item->setFlags(unselect_score_item->flags() ^ Qt::ItemIsSelectable);
        }
    }
    else
        this->ScoreItemAt(bottom)->setFlags(this->ScoreItemAt(bottom)->flags() | Qt::ItemIsSelectable);

    if (type == NoSelection || type == SelectOnePackage || type == SelectOneTestCasePackage) can_add = true;
    if (type == NoSelection || type == SelectDiscontinuous) return;

    can_remove = true;
    if (type == SelectMultiplePackage || type == SelectMultipleTestCasePackage) can_merge = true;
    if (type == SelectOnePackage || type == SelectMultiplePackage) can_split = true;

    if (type == SelectOnePackage || type == SelectOneTestCase || type == SelectOneTestCasePackage)
    {
        can_add_sub = true;
        if (!this->item(0, 0)->isSelected()) can_up = true;
        if (!this->item(this->rowCount() - 1, 0)->isSelected()) can_down = true;
    }
    emit testCaseSelectionChanged();
}
