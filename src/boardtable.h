#ifndef BOARDTABLE_H
#define BOARDTABLE_H

#include "global.h"

#include <QTableWidget>

class BoardTable : public QTableWidget
{
    Q_OBJECT
public:
    explicit BoardTable(QWidget* parent = 0);
    ~BoardTable();

    void clearBoard();
    void showResult();
    void clearHighlighted(int c);
    int showProblemSumResult(QLabel* tmp, Player::Result* res, int sum, int Highlighted); //return style
    int showProblemResult(QLabel* tmp, Player::Result* res, int sum, int Highlighted); //return style

    int preHeaderClicked;

public slots:
    void sectionMoveEvent(int i, int oldV, int newV);
    void sortEvent(int);

private:
    void setup();
    void sortByName();
    void sortBySumScore();
    void sortByProblem(int);
    void setHighlighted(int c);

    bool alreadyMovingSection;
};

#endif // BOARDTABLE_H
