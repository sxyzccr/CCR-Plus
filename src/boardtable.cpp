#include "global.h"
#include "boardtable.h"
#include "contest.h"

#include <QScrollBar>
#include <QHeaderView>

using namespace std;

BoardTable::BoardTable(QWidget* parent) : QTableWidget(parent)
{
    this->setColumnCount(2);
    this->setMinimumSize(QSize(140, 250));
    this->setFocusPolicy(Qt::NoFocus);
    this->setFrameShape(QFrame::NoFrame);
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setProperty("showDropIndicator", QVariant(false));
    this->setDragDropOverwriteMode(false);
    this->setAlternatingRowColors(true);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setStyleSheet(QLatin1String(
                            "QHeaderView"
                            "{"
                            "	background:white;"
                            "}"
                            "QTableWidget"
                            "{"
                            "	color:rgba(0,0,0,0);"
                            "	gridline-color:rgb(242,242,242);"
                            "}"
                            "QTableWidget::item:alternate:!selected"
                            "{"
                            "   background-color:rgb(255,255,255);"
                            "}"
                            "QTableWidget::item:!alternate:!selected"
                            "{"
                            "   background-color:rgb(248,248,248);"
                            "}"
                            "QTableWidget::item:selected"
                            "{"
                            "	color:rgba(0,0,0,0);"
                            "	background-color:rgb(203,232,246);"
                            "	border-width:1px;"
                            "   border-style:solid;"
                            "	border-color:rgb(38,160,218);"
                            "}"
                            "QLabel"
                            "{"
                            "	margin:1px;"
                            "	border-radius:3px;"
                            "   border-style:solid;"
                            "	border-color:rgba(0,0,0,120);"
                            "}"));

    this->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->horizontalHeader()->setDefaultSectionSize(85);
    this->horizontalHeader()->setHighlightSections(false);
    this->horizontalHeader()->setMinimumSectionSize(60);
    this->horizontalHeader()->setProperty("showSortIndicator", QVariant(false));
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->horizontalHeader()->resizeSection(1, 60);
    this->horizontalHeader()->setSectionsMovable(true);
    this->horizontalHeader()->setSortIndicatorShown(true);
    this->horizontalHeader()->setTextElideMode(Qt::ElideRight);

    this->verticalHeader()->setDefaultSectionSize(27);
    this->verticalHeader()->setHighlightSections(false);
    this->verticalHeader()->setMinimumSectionSize(27);
    this->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
    this->verticalHeader()->setMinimumWidth(22);

    alreadyMovingSection = false;
}

BoardTable::~BoardTable()
{

}

void BoardTable::clearBoard()
{
    preHeaderClicked = -1;
    this->clear();
    this->setRowCount(0);
    this->setColumnCount(2);
}

int BoardTable::showProblemSumResult(QLabel* tmp, Player::Result* res, int sum, int Highlighted)
{
    int x = -1;
    bool isHighlighted = preHeaderClicked == Highlighted;
    tmp->setText(QFontMetrics(tmp->font()).elidedText(QString::number(res->score), Qt::ElideRight, 60 - 5));
    tmp->setToolTip(QString("总用时: %1s").arg(res->usedTime, 0, 'f', 2));

    if (!sum) x = 14;
    else if (res->score > sum) x = 12;
    else if (res->score < 0) x = 13, tmp->setToolTip("无效的总分");
    else if (res->score == 0) x = 11;
    else x = res->score * 10 / sum;

    if (x >= 0)
        tmp->setStyleSheet(isHighlighted ? LABEL_STYLE_HARD[x] + "QLabel{border-width:1px;}" : LABEL_STYLE_SOFT[x]);
    else
        tmp->setStyleSheet("");
    return x;
}

int BoardTable::showProblemResult(QLabel* tmp, Player::Result* res, int sum, int Highlighted)
{
    int x = -1;
    bool isHighlighted = preHeaderClicked == Highlighted;
    tmp->setText(QFontMetrics(tmp->font()).elidedText(QString::number(res->score), Qt::ElideRight, 85 - 5));
    tmp->setToolTip(QString("用时: %1s").arg(res->usedTime, 0, 'f', 2));

    switch (res->state)
    {
    case 'N':
        if (!sum) x = 14;
        else if (res->score > sum) x = 12;
        else if (res->score < 0) x = 13, tmp->setToolTip("无效的得分");
        else if (res->score == 0) x = 11;
        else x = res->score * 10 / sum;
        break;
    case 'C':
        x = 14;
        tmp->setText("+");
        tmp->setToolTip("编译错误");
        break;
    case 'F':
        x = 14;
        tmp->setText("-");
        tmp->setToolTip("找不到文件");
        break;
    case 'S':
        x = 14;
        tmp->setText("=");
        tmp->setToolTip("超过代码长度限制");
        break;
    case 'E':
        x = 16;
        tmp->setToolTip("测评器或校验器出错");
        break;
    case ' ':
        x = -1;
        tmp->setText("");
        tmp->setToolTip("未测评");
        break;
    }
    if (x >= 0)
        tmp->setStyleSheet(isHighlighted ? LABEL_STYLE_HARD[x] + "QLabel{border-width:1px;}" : LABEL_STYLE_SOFT[x]);
    else
        tmp->setStyleSheet("");
    return x;
}

void BoardTable::showResult()
{
    int row = 0;
    for (auto& i : Global::g_contest.players)
    {
        QLabel* tmp;

        tmp = new QLabel(i.name);
        tmp->setToolTip(i.name);
        tmp->setIndent(15);
        tmp->setAlignment(Qt::AlignCenter);
        this->setCellWidget(row, 0, tmp);
        i.label.push_back(tmp);

        if (i.name == "std") i.type = -1, tmp->setText("标准程序"), tmp->setStyleSheet("QLabel{color:blue;font:bold;}");
        else if (i.name.toLower() == "jyk" || i.name == "贾越凯") i.type = -2, tmp->setStyleSheet("QLabel{color:red;font:bold;}");
        else if (i.name.toLower() == "bogang" || i.name.toLower() == "bg" || i.name == "伯刚") i.type = 1, tmp->setStyleSheet("QLabel{color:rgb(31,151,63);font:bold;}");
        i.style.push_back(-1);

        tmp = new QLabel;
        tmp->setAlignment(Qt::AlignCenter);
        this->setCellWidget(row, 1, tmp);
        i.label.push_back(tmp);
        i.style.push_back(showProblemSumResult(tmp, &i.sum, Global::g_contest.sum_score, false));

        int col = 0;
        for (auto j : i.problem)
        {
            tmp = new QLabel;
            tmp->setAlignment(Qt::AlignCenter);
            this->setCellWidget(row, col + 2, tmp);
            i.label.push_back(tmp);
            i.style.push_back(showProblemResult(tmp, &j, Global::g_contest.problems[col].sumScore, false));
            col++;
        }
        row++;
    }
    for (int r = 0; r < Global::g_contest.player_num; r++)
        for (int c = 0; c < Global::g_contest.problem_num + 2; c++)
        {
            QTableWidgetItem* bg = new QTableWidgetItem;
            if (c <= 1) bg->setFlags(bg->flags()^Qt::ItemIsSelectable);
            this->setItem(r, c, bg);
            this->item(r, c)->setData(Qt::DisplayRole, r);
        }
    onSortTable(1);
}

void BoardTable::resizePlayerLabel()
{
    int len = 75;
    for (auto& i : Global::g_contest.players)
    {
        QLabel* tmp = i.label[0];
        len = max(len, QFontMetrics(FONT).width(tmp->text()) + 30);
    }
    horizontalHeader()->resizeSection(0, len);
}

void BoardTable::onSectionMove(int i, int oldV, int newV)
{
    //qDebug()<<i<<oldV<<newV;
    if (alreadyMovingSection) return;
    if (i <= 1 || newV <= 1) alreadyMovingSection = true, this->horizontalHeader()->moveSection(newV, oldV), alreadyMovingSection = false;
    else
    {
        Global::g_contest.problem_order.move(oldV - 2, newV - 2);
        QStringList list;
        for (auto i : Global::g_contest.problem_order) list.append(Global::g_contest.problems[i].name);
        Global::g_contest.SaveProblemOrder(list);
    }
}

inline bool cmpName(const Player& x, const Player& y) { return x.type < y.type || (x.type == y.type && x.name < y.name); }
inline bool cmpSumScore(const Player& x, const Player& y) { return x.sum < y.sum || (x.sum == y.sum && !cmpName(x, y)); }

void BoardTable::sortByName() { sort(Global::g_contest.players.begin(), Global::g_contest.players.end(), cmpName); }
void BoardTable::sortBySumScore() { sort(Global::g_contest.players.begin(), Global::g_contest.players.end(), cmpSumScore); }
void BoardTable::sortByProblem(int p)
{
    char F[128];
    F[' '] = 0, F['F'] = 1, F['S'] = 2, F['C'] = 3, F['E'] = 4, F['N'] = 5;
    sort(Global::g_contest.players.begin(), Global::g_contest.players.end(), [&](const Player & x, const Player & y)
    {
        if (!x.problem[p].score && !y.problem[p].score) return F[x.problem[p].state] < F[y.problem[p].state] || (F[x.problem[p].state] == F[y.problem[p].state] && cmpSumScore(x, y));
        return x.problem[p] < y.problem[p] || (x.problem[p] == y.problem[p] && cmpSumScore(x, y));
    });
}

void BoardTable::clearHighlighted(int c)
{
    if (c >= 0)
        for (int i = 0; i < Global::g_contest.player_num; i++)
        {
            int t = GetLogicalRow(i);
            Player* p = &Global::g_contest.players[t];
            if (p->style[c] >= 0) p->label[c]->setStyleSheet(LABEL_STYLE_SOFT[p->style[c]]);
            // if (c>1) setSelected(t,c,p->selected[c-2]);
        }
}

void BoardTable::setHighlighted(int c)
{
    if (c >= 0)
        for (int i = 0; i < Global::g_contest.player_num; i++) //auto i:players)
        {
            int t = GetLogicalRow(i);
            Player* p = &Global::g_contest.players[t];
            if (p->style[c] >= 0)
                p->label[c]->setStyleSheet(LABEL_STYLE_HARD[p->style[c]] + "QLabel{border-width:1px;}");
            else if (c)
                p->label[c]->setStyleSheet("");
            //if (c>1) setSelected(t,c,p->selected[c-2]);
        }
}

void BoardTable::onSortTable(int c)
{
    if (Global::g_is_judging) return;

    if (preHeaderClicked != c)
    {
        clearHighlighted(preHeaderClicked);
        setHighlighted(c);
    }

    this->horizontalHeader()->setSortIndicatorShown(true);
    if (preHeaderClicked != c && c) this->horizontalHeader()->setSortIndicator(c, Qt::DescendingOrder);
    Global::g_pre_sort_order = this->horizontalHeader()->sortIndicatorOrder();

    if (!c) sortByName();
    else if (c == 1) sortBySumScore();
    else sortByProblem(c - 2);

    int k = 0;
    for (auto& i : Global::g_contest.players)
    {
        this->item(i.id, c)->setData(Qt::DisplayRole, k);
        i.id = GetLogicalRow(k);
        k++;
    }
    this->sortByColumn(c);
    this->verticalScrollBar()->setValue(0);
    //  for (int i=0; i<playerNum; i++) qDebug()<<ui->tableWidget->item(i,c)->data(Qt::DisplayRole);

    preHeaderClicked = c;
}

void BoardTable::onSetItemUnselected(int r, int c)
{
    if (Global::g_contest_closed) return;
    this->item(r, c)->setSelected(false);
}

void BoardTable::onUpdatePlayerLabel(QLabel* label, const QString& text, const QString& toolTip, const QString& styleSheet)
{
    if (Global::g_contest_closed) return;
    label->setText(text);
    label->setToolTip(toolTip);
    label->setStyleSheet(styleSheet);
}

void BoardTable::onUpdateProblemLabel(Player* ply, int c, Player::Result* res, int sum)
{
    if (Global::g_contest_closed) return;
    if (c == 1)
        ply->style[1] = this->showProblemSumResult(ply->label[1], res, sum, 1);
    else
        ply->style[c] = this->showProblemResult(ply->label[c], res, sum, c);
}
