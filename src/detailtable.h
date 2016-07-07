#ifndef DETAILTABLE_H
#define DETAILTABLE_H

#include "global.h"

#include <QTableWidget>

class DetailTable : public QTableWidget
{
    Q_OBJECT
public:
    explicit DetailTable(QWidget* parent = 0);
    ~DetailTable();

public slots:
    void showDetailEvent(int, int);
    void adjustScrollbar();
    void clearDetail();
    void addTitleDetail(int& row, const QString& title);
    void addNoteDetail(int& row, const QString& note, const QString& state = "N");
    void addPointDetail(int& row, int num, const QString& note, const QString& state, const QString& file);
    void addScoreDetail(int row, int len, int score, int sumScore);
    void showConfigDetail();

private:
    void setup();
    void showProblemDetail(Player* player, Problem* problem);

    bool isScrollBarAtBottom;
};

#endif // DETAILTABLE_H
