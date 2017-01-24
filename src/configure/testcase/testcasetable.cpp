#include <QHeaderView>

#include "configure/testcase/testcasetable.h"

TestCaseTable::TestCaseTable(QWidget* parent) :
    QTableWidget(parent), problem(nullptr), sum_score(0)
{
    this->horizontalHeader()->setFixedHeight(25);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    this->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);

    connect(this, &QTableWidget::itemSelectionChanged, this, &TestCaseTable::onItemSelectionChanged);
    connect(this, &QTableWidget::itemClicked, this, [this](QTableWidgetItem* item)
    {
        if (!item->column())
        {
            int p = item->row();
            QTableWidgetSelectionRange range(p, 0, ScoreItemBottomRow(p), this->columnCount() - 1);
            this->setRangeSelected(range, true);
        }
    });
    connect(this, &QTableWidget::currentItemChanged, this, [this]()
    {
        int top, bottom;
        SelectionType type = GetSelectionType(&top, &bottom);
        if (type != NoSelection && ScoreItemBottomRow(bottom) == bottom && unselect_score_item)
            unselect_score_item->setFlags(unselect_score_item->flags() | Qt::ItemIsSelectable);
    });
}

void TestCaseTable::LoadTestCases(const Problem* problem)
{
    score_items.clear();

    this->clear();
    this->setRowCount(0);
    this->setColumnCount(0);
    this->problem = problem;
    this->sum_score = problem->Score();
    this->unselect_score_item = nullptr;

    if (problem == nullptr) return;
    if (problem->Type() == Global::Traditional)
    {
        this->setColumnCount(5);
        this->setHorizontalHeaderLabels({"分值", "输入文件", "输出文件", "时间限制", "内存限制"});
        this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
        this->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
        this->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
        this->setColumnWidth(0, 70);
        this->setColumnWidth(3, 70);
        this->setColumnWidth(4, 70);
    }
    else if (problem->Type() == Global::AnswersOnly)
    {
        this->setColumnCount(4);
        this->setHorizontalHeaderLabels({"分值", "输入文件", "输出文件", "提交文件"});
        this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
        this->setColumnWidth(0, 70);
    }
    else
        return;

    int rows = 0;
    for (int i = 0; i < problem->SubtaskCount(); i++)
    {
        const Subtask* sub = problem->SubtaskAt(i);
        int len = 0;
        for (auto point : *sub)
        {
            this->insertRow(rows);

            addItem(rows, 1, point->InFile());
            addItem(rows, 2, point->OutFile());

            if (problem->Type() == Global::Traditional)
            {
                addItem(rows, 3, QString::number(point->TimeLimit()));
                addItem(rows, 4, QString::number(point->MemoryLimit()));
            }
            else if (problem->Type() == Global::AnswersOnly)
                addItem(rows, 3, point->SubmitFile());

            addItem(rows, 0, QString::number(sub->Score()));
            this->item(rows, 0)->setBackgroundColor(QColor(255, 255, 255));

            len++, rows++;
            score_items.append(this->item(rows - len, 0));
        }
        if (len > 1) this->setSpan(rows - len, 0, len, 1);
    }
}

TestCaseTable::SelectionType TestCaseTable::GetSelectionType(int* _top, int* _bottom)
{
    auto list = this->selectedRanges();
    int top = 1e9, bottom = -1e9;
    for (auto i : list)
    {
        top = std::min(top, i.topRow());
        bottom = std::max(bottom, i.bottomRow());
    }
    *_top = top, *_bottom = bottom;
    if (top > bottom) return NoSelection;

    for (int i = top; i <= bottom; i++)
        if (!this->item(i, 1)->isSelected())
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

    if (top == bottom) return SelectOneSubTestCase;
    if (ScoreItemAt(top) == ScoreItemAt(bottom)) return SelectMultipleSubTestCase;
    return OtherSelection;
}

TestCase TestCaseTable::TestCaseAt(int row)
{
    if (problem->Type() == Global::Traditional)
        return TestCase(this->item(row, 3)->text().toDouble(), this->item(row, 4)->text().toDouble(),
                        this->item(row, 1)->text(), this->item(row, 2)->text());
    else if (problem->Type() == Global::AnswersOnly)
        return TestCase(0, 0, this->item(row, 1)->text(), this->item(row, 2)->text(), this->item(row, 3)->text());
    else
        return TestCase(0, 0, "", "");
}

void TestCaseTable::ChangeScore(int row, int score)
{
    sum_score += score - this->ScoreItemAt(row)->text().toInt();
    this->ScoreItemAt(row)->setText(QString::number(score));
}

void TestCaseTable::ChangeTestCase(int row, const TestCase& point)
{
    if (!point.InFile().isEmpty()) this->item(row, 1)->setText(point.InFile());
    if (!point.OutFile().isEmpty()) this->item(row, 2)->setText(point.OutFile());

    if (problem->Type() == Global::Traditional)
    {
        if (point.TimeLimit() >= 0) this->item(row, 3)->setText(QString::number(point.TimeLimit()));
        if (point.MemoryLimit() >= 0) this->item(row, 4)->setText(QString::number(point.MemoryLimit()));
    }
    else if (problem->Type() == Global::AnswersOnly)
    {
        if (!point.SubmitFile().isEmpty()) this->item(row, 3)->setText(point.SubmitFile());
    }
}

void TestCaseTable::swapTestCase(int row1, int row2)
{
    for (int c = 1; c < this->columnCount(); c++)
    {
        QTableWidgetItem *item1 = this->takeItem(row1, c),
                         *item2 = this->takeItem(row2, c);
        this->setItem(row1, c, item2);
        this->setItem(row2, c, item1);
    }
}

void TestCaseTable::swapPackage(int topRow1, int topRow2)
{
    QList<QTableWidgetItem*> tmp;
    int p1 = topRow1, s1 = ScoreItemBottomRow(topRow1) - topRow1 + 1,
        p2 = topRow2, s2 = ScoreItemBottomRow(topRow2) - topRow2 + 1,
        p3 = p1 + s2;

    if (this->rowSpan(p1, 0) > 1) this->setSpan(p1, 0, 1, 1);
    if (this->rowSpan(p2, 0) > 1) this->setSpan(p2, 0, 1, 1);
    for (int c = 0; c < this->columnCount(); c++)
    {
        tmp.clear();
        for (int i = 0; i < s1; i++) tmp.append(this->takeItem(p1 + i, c));
        for (int i = 0; i < s2; i++) this->setItem(p1 + i, c, this->takeItem(p2 + i, c));
        for (int i = 0; i < s1; i++) this->setItem(p3 + i, c, tmp[i]);
    }
    for (int i = 0; i < s2; i++) score_items[p1 + i] = this->item(p1, 0);
    for (int i = 0; i < s1; i++) score_items[p3 + i] = this->item(p3, 0);
    if (s2 > 1) this->setSpan(p1, 0, s2, 1);
    if (s1 > 1) this->setSpan(p3, 0, s1, 1);
}



void TestCaseTable::AddTestCase(const TestCase& point, int score)
{
    int top, bottom, row;
    SelectionType type = GetSelectionType(&top, &bottom);
    if (type != NoSelection && type != SelectOnePackage && type != SelectOneTestCasePackage) return;

    if (type == NoSelection) bottom = -1;
    this->insertRow(row = bottom + 1);

    addItem(row, 1, point.InFile());
    addItem(row, 2, point.OutFile());

    if (problem->Type() == Global::Traditional)
    {
        addItem(row, 3, QString::number(point.TimeLimit()));
        addItem(row, 4, QString::number(point.MemoryLimit()));
    }
    else if (problem->Type() == Global::AnswersOnly)
        addItem(row, 3, point.SubmitFile());

    addItem(row, 0, QString::number(score));
    score_items.insert(score_items.begin() + row, this->item(row, 0));
    sum_score += score;
    this->selectRow(row);
    if (!this->hasFocus()) this->setFocus();
}

void TestCaseTable::AddSubTestCase(const TestCase& point)
{
    int top, bottom, row;
    SelectionType type = GetSelectionType(&top, &bottom);
    if (type != SelectOnePackage && type != SelectOneSubTestCase && type != SelectOneTestCasePackage) return;

    int span = this->rowSpan(ScoreItemTopRow(top), 0) + 1;
    this->insertRow(row = bottom + 1);

    addItem(row, 1, point.InFile());
    addItem(row, 2, point.OutFile());

    if (problem->Type() == Global::Traditional)
    {
        addItem(row, 3, QString::number(point.TimeLimit()));
        addItem(row, 4, QString::number(point.MemoryLimit()));
    }
    else if (problem->Type() == Global::AnswersOnly)
        addItem(row, 3, point.SubmitFile());

    addItem(row, 0, "");
    this->setSpan(ScoreItemTopRow(top), 0, span, 1);
    score_items.insert(score_items.begin() + row, score_items[top]);
    this->selectRow(row);
}

void TestCaseTable::DeleteSelection()
{
    int top, bottom;
    SelectionType type = GetSelectionType(&top, &bottom);
    if (type == NoSelection || type == SelectDiscontinuous) return;

    this->clearSelection();
    for (int r = top; r <= bottom; r++)
    {
        if (ScoreItemTopRow(r) == r && ScoreItemBottomRow(r) > bottom)
        {
            int scoreBottom = ScoreItemBottomRow(r);
            QTableWidgetItem* item = this->item(bottom + 1, 0);
            item->setText(ScoreItemAt(r)->text());
            item->setToolTip(item->text());
            for (int i = bottom + 1; i <= scoreBottom; i++) score_items[i] = item;
        }
        else if (ScoreItemTopRow(r) == r)
            sum_score -= this->item(r, 0)->text().toInt();
        for (int c = 0; c < this->columnCount(); c++) this->takeItem(r, c);
    }

    for (int r = bottom + 1; r < this->rowCount(); r++)
    {
        int p = r - bottom - 1 + top;
        for (int c = 0; c < this->columnCount(); c++) this->setItem(p, c, this->takeItem(r, c));
    }

    for (int r = 0; r < this->rowCount(); r++)
        if (this->rowSpan(r, 0) > 1) this->setSpan(r, 0, 1, 1);

    for (int i = top; i <= bottom; i++) this->removeRow(this->rowCount() - 1);
    score_items.erase(score_items.begin() + top, score_items.begin() + bottom + 1);

    for (int i = 0, j; i < this->rowCount(); i++) if (ScoreItemTopRow(i) == i)
    {
        for (j = i; j < this->rowCount() && score_items[j] == score_items[i]; j++);
        if (j - i > 1) this->setSpan(i, 0, j - i, 1);
    }
}

void TestCaseTable::MoveUpSelection()
{
    int top, bottom;
    SelectionType type = GetSelectionType(&top, &bottom);
    this->clearSelection();
    if (type == SelectOneSubTestCase)
    {
        swapTestCase(top - 1, top);
        this->selectRow(top - 1);
    }
    else if (type == SelectOnePackage || type == SelectOneTestCasePackage)
    {
        int p = ScoreItemTopRow(top - 1);
        swapPackage(ScoreItemTopRow(top - 1), top);
        QTableWidgetSelectionRange range(p, 0, ScoreItemBottomRow(p), this->columnCount() - 1);
        this->setRangeSelected(range, true);
    }
}

void TestCaseTable::MoveDownSelection()
{
    int top, bottom;
    SelectionType type = GetSelectionType(&top, &bottom);
    this->clearSelection();
    if (type == SelectOneSubTestCase)
    {
        swapTestCase(top + 1, top);
        this->selectRow(top + 1);
    }
    else if (type == SelectOnePackage || type == SelectOneTestCasePackage)
    {
        swapPackage(top, ScoreItemBottomRow(top) + 1);
        int p = ScoreItemBottomRow(top) + 1;
        QTableWidgetSelectionRange range(p, 0, ScoreItemBottomRow(p), this->columnCount() - 1);
        this->setRangeSelected(range, true);
    }
}

void TestCaseTable::MergeSelection()
{
    int top, bottom;
    SelectionType type = GetSelectionType(&top, &bottom);
    if (type != SelectMultiplePackage && type != SelectMultipleTestCasePackage) return;

    int sum = 0;
    QTableWidgetItem* scoreItem = score_items[top];
    for (int i = top; i <= bottom; i++)
    {
        if (ScoreItemTopRow(i) == i)
        {
            sum += score_items[i]->text().toInt();
            if (this->rowSpan(i, 0) > 1) this->setSpan(i, 0, 1, 1);
        }
        score_items[i] = scoreItem;
    }
    this->setSpan(top, 0, bottom - top + 1, 1);
    scoreItem->setText(QString::number(sum));
    scoreItem->setToolTip(scoreItem->text());
    onItemSelectionChanged();
}

void TestCaseTable::SplitSelection()
{
    int top, bottom;
    SelectionType type = GetSelectionType(&top, &bottom);
    if (type != SelectOnePackage && type != SelectMultiplePackage) return;

    QList<int> score;
    for (int i = top; i <= bottom; i++)
    {
        if (ScoreItemTopRow(i) == i && this->rowSpan(i, 0) > 1)
        {
            int len = this->rowSpan(i, 0);
            int sum = score_items[i]->text().toInt(), s = sum / len;
            this->setSpan(i, 0, 1, 1);
            score.clear();
            for (int j = 0; j < len; j++)
            {
                score_items[i + j] = this->item(i + j, 0);
                score.append(s), sum -= s;
            }
            for (int j = len; sum; sum--) score[--j]++;
            for (int j = 0; j < len; j++)
            {
                score_items[i + j]->setText(QString::number(score[j]));
                score_items[i + j]->setToolTip(QString::number(score[j]));
            }
        }
    }
    onItemSelectionChanged();
}

void TestCaseTable::onItemSelectionChanged()
{
    can_edit = can_add = can_add_sub = can_delete = can_up = can_down = can_merge = can_split = false;

    int top, bottom;
    SelectionType type = GetSelectionType(&top, &bottom);

    if (type == NoSelection)
    {
        can_add = true;
        emit testCaseSelectionChanged();
        return;
    }
    if (type == SelectDiscontinuous)
    {
        emit testCaseSelectionChanged();
        return;
    }
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

    can_edit = can_delete = true;
    if ((type == SelectMultipleSubTestCase || type == OtherSelection) && problem->Type() == Global::AnswersOnly) can_edit = false;
    if (type == SelectOnePackage || type == SelectOneTestCasePackage) can_add = true;
    if (type == SelectMultiplePackage || type == SelectMultipleTestCasePackage) can_merge = true;
    if (type == SelectOnePackage || type == SelectMultiplePackage) can_split = true;
    if (type == SelectOnePackage || type == SelectOneSubTestCase || type == SelectOneTestCasePackage)
    {
        can_add_sub = true;
        if (type == SelectOneSubTestCase)
        {
            if (ScoreItemTopRow(top) < top) can_up = true;
            if (ScoreItemBottomRow(bottom) > bottom) can_down = true;
        }
        else
        {
            if (!this->item(0, 1)->isSelected()) can_up = true;
            if (!this->item(this->rowCount() - 1, 1)->isSelected()) can_down = true;
        }
    }
    emit testCaseSelectionChanged();
}
