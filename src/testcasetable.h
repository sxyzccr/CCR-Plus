#ifndef TESTCASETABLE_H
#define TESTCASETABLE_H

#include "problem.h"

#include <QTableWidget>
//#include <QDragEnterEvent>
#include <QDebug>

class TestCaseTable : public QTableWidget
{
    Q_OBJECT
public:
    enum SelectionType
    {
        NoSelection,
        SelectOnePackage,
        SelectOneTestCase,
        SelectOneTestCasePackage,
        SelectMultiplePackage,
        SelectMultipleTestCasePackage,
        SelectDiscontinuous,
        OtherSelection
    };

    explicit TestCaseTable(QWidget* parent = 0);
    ~TestCaseTable() {}

    bool CanAddTestCase() const { return can_add; }
    bool CanAddSubTestCase() const { return can_add_sub; }
    bool CanRemoveTestCase() const { return can_remove; }
    bool CanMoveUp() const { return can_up; }
    bool CanMoveDown() const { return can_down; }
    bool CanMerge() const { return can_merge; }
    bool CanSplit() const { return can_split; }

    QTableWidgetItem* ScoreItemAt(int row) { return score_item[row]; }
    int ScoreItemTopRow(int row) { return score_item[row]->row(); }
    int ScoreItemBottomRow(int row)
    {
        int top = score_item[row]->row();
        return top + this->rowSpan(top, score_column) - 1;
    }

    void LoadTestCases(Problem* problem);
    TestCaseTable::SelectionType GetSelectionType(int *_top, int *_bottom);
    void MergeSelection();
    void SplitSelection();

private:
    int score_column;
    std::vector<QTableWidgetItem*> score_item;
    QTableWidgetItem* unselect_score_item;
    bool can_add, can_add_sub, can_remove, can_up, can_down, can_merge, can_split;

private slots:
    void onItemSelectionChanged();

signals:
    void testCaseSelectionChanged();
};

#endif // TESTCASETABLE_H
