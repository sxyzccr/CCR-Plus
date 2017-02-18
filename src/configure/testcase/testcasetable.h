#ifndef TESTCASETABLE_H
#define TESTCASETABLE_H

#include <QTableWidget>

class Problem;
class TestCase;

class TestCaseTable : public QTableWidget
{
    Q_OBJECT
public:
    enum SelectionType
    {
        NoSelection,                    // 没有选中
        SelectOnePackage,               // 选中一个测试点，包含两组及以上测试数据
        SelectOneSubTestCase,           // 选中一组测试数据，且不是一个测试点
        SelectOneTestCasePackage,       // 选中一个测试点，只包含一组测试数据
        SelectMultiplePackage,          // 选中多个测试点，且至少有一个测试点包含两组及以上测试数据
        SelectMultipleSubTestCase,      // 选中多组测试数据，且均位于一个测试点
        SelectMultipleTestCasePackage,  // 选中多个测试点，且都只包含一组测试数据
        SelectDiscontinuous,            // 选中的测试数据不连续（不会出现这种情况）
        OtherSelection                  // 其他情况（选中多组测试数据，且位于多个测试点）
    };

    explicit TestCaseTable(QWidget* parent = nullptr);
    ~TestCaseTable() {}

    // Getter member functions
    int SumScore() const { return sum_score; }
    bool CanEdit() const { return can_edit; }
    bool CanAddTestCase() const { return can_add; }
    bool CanAddSubTestCase() const { return can_add_sub; }
    bool CanDelete() const { return can_delete; }
    bool CanMoveUp() const { return can_up; }
    bool CanMoveDown() const { return can_down; }
    bool CanMerge() const { return can_merge; }
    bool CanSplit() const { return can_split; }

    QTableWidgetItem* ScoreItemAt(int row) { return score_items[row]; }
    int ScoreAt(int row) { return score_items[row]->text().toInt(); }
    int ScoreItemTopRow(int row) { return score_items[row]->row(); }
    int ScoreItemBottomRow(int row)
    {
        int top = score_items[row]->row();
        return top + this->rowSpan(top, 0) - 1;
    }

    void LoadTestCases(const Problem* problem);
    TestCaseTable::SelectionType GetSelectionType(int* _top, int* _bottom);
    TestCase TestCaseAt(int row);
    void ChangeScore(int row, int score);
    void ChangeTestCase(int row, const TestCase& point);

public slots:
    void AddTestCase(const TestCase& point, int score);
    void AddSubTestCase(const TestCase& point);
    void DeleteSelection();
    void MoveUpSelection();
    void MoveDownSelection();
    void MergeSelection();
    void SplitSelection();

private:
    const Problem* problem; // 只管显示，禁止修改 problem
    QList<QTableWidgetItem*> score_items;
    QTableWidgetItem* unselect_score_item;
    int sum_score;
    bool can_edit, can_add, can_add_sub, can_delete, can_up, can_down, can_merge, can_split;

    QAction *action_in, *action_out, *action_sub;
    QMenu* menu;

    void addItem(int row, int column, const QString& text)
    {
        QTableWidgetItem* item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignCenter);
        item->setToolTip(text);
        this->setItem(row, column, item);
    }
    void swapTestCase(int row1, int row2);
    void swapPackage(int topRow1, int topRow2);

private slots:
    void onItemSelectionChanged();

    void onOpenInFile();
    void onOpenOutFile();
    void onHeaderContextMenuEvent(const QPoint& pos);
    void onContextMenuEvent(const QPoint& pos);

signals:
    void testCaseSelectionChanged();
};

#endif // TESTCASETABLE_H
