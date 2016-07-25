#ifndef BOARDTABLE_H
#define BOARDTABLE_H

#include "player.h"

#include <QTableWidget>

class BoardTable : public QTableWidget
{
    Q_OBJECT
public:
    explicit BoardTable(QWidget* parent = 0);
    virtual ~BoardTable();

    void setup();
    void clearBoard();
    void showResult();
    void resizePlayerLabel();
    void clearHighlighted(int c);
    void showProblemSumResult(ResultLabel* tmp, int sum, int Highlighted);
    void showProblemResult(ResultLabel* tmp, int sum, int Highlighted);

    int preHeaderClicked;

public slots:
    void onSectionMove(int i, int oldV, int newV);
    void onSortTable(int);

    void onSetItemUnselected(int r, int c);
    void onUpdatePlayerLabel(QLabel *label, const QString &text, const QString &toolTip, const QString &styleSheet);
    void onUpdateProblemLabel(Player *ply, int c, int sum);

private:
    void sortByName();
    void sortBySumScore();
    void sortByProblem(int);
    void setHighlighted(int c);

    bool alreadyMovingSection;
};

#endif // BOARDTABLE_H
