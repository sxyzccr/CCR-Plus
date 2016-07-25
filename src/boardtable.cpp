#include "global.h"
#include "boardtable.h"
#include "contest.h"

#include <QScrollBar>
#include <QHeaderView>

using namespace std;

BoardTable::BoardTable(QWidget* parent) : QTableWidget(parent)
{
    setup();
}

BoardTable::~BoardTable()
{

}

void BoardTable::setup()
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

void BoardTable::clearBoard()
{
    preHeaderClicked = -1;
    this->clear();
    this->setRowCount(0);
    this->setColumnCount(2);
}

void BoardTable::showProblemSumResult(ResultLabel* tmp, int sum, int Highlighted)
{
    int x = -1;
    ResultSummary res = tmp->GetResult();
    bool isHighlighted = preHeaderClicked == Highlighted;

    tmp->setText(QFontMetrics(tmp->font()).elidedText(QString::number(res.score), Qt::ElideRight, 60 - 5));
    tmp->setToolTip(QString("总用时: %1s").arg(res.time, 0, 'f', 2));

    if (!sum) x = 14;
    else if (res.score > sum) x = 12;
    else if (res.score < 0) x = 13, tmp->setToolTip("无效的总分");
    else if (res.score == 0) x = 11;
    else x = res.score * 10 / sum;

    if (x >= 0)
        tmp->setStyleSheet(isHighlighted ? LABEL_STYLE_HARD[x] + "QLabel{border-width:1px;}" : LABEL_STYLE_SOFT[x]);
    else
        tmp->setStyleSheet("");

    tmp->SetLabelStyle(x);
}

void BoardTable::showProblemResult(ResultLabel* tmp, int sum, int Highlighted)
{
    int x = -1;
    ResultSummary res = tmp->GetResult();
    bool isHighlighted = preHeaderClicked == Highlighted;

    tmp->setText(QFontMetrics(tmp->font()).elidedText(QString::number(res.score), Qt::ElideRight, 85 - 5));
    tmp->setToolTip(QString("用时: %1s").arg(res.time, 0, 'f', 2));

    switch (res.state)
    {
    case 'N':
        if (!sum) x = 14;
        else if (res.score > sum) x = 12;
        else if (res.score < 0) x = 13, tmp->setToolTip("无效的得分");
        else if (res.score == 0) x = 11;
        else x = res.score * 10 / sum;
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

    tmp->SetLabelStyle(x);
}

void BoardTable::showResult()
{
    int row = 0;
    for (auto& i : Global::g_contest.players)
    {
        ResultLabel* tmp;

        tmp = i.GetNameLabel();
        tmp->setToolTip(i.GetName());
        tmp->setIndent(15);
        tmp->setAlignment(Qt::AlignCenter);
        tmp->SetLabelStyle(-1);
        this->setCellWidget(row, 0, tmp);

        i.SetSpecialNameLabel();

        tmp = i.GetSumLabel();
        tmp->setAlignment(Qt::AlignCenter);
        showProblemSumResult(tmp, Global::g_contest.sum_score, false);
        this->setCellWidget(row, 1, tmp);

        int col = 0;
        for (int j = 0; j < Global::g_contest.problem_num; j++)
        {
            tmp = i.GetProbLabel(j);
            tmp->setAlignment(Qt::AlignCenter);
            showProblemResult(tmp, Global::g_contest.problems[col].sumScore, false);
            this->setCellWidget(row, col + 2, tmp);
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
        QLabel* tmp = i.GetNameLabel();
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

void BoardTable::sortByName() { sort(Global::g_contest.players.begin(), Global::g_contest.players.end(), cmpName); }
void BoardTable::sortBySumScore() { sort(Global::g_contest.players.begin(), Global::g_contest.players.end(), cmpSumScore); }
void BoardTable::sortByProblem(int p)
{
    char F[128];
    F[' '] = 0, F['F'] = 1, F['S'] = 2, F['C'] = 3, F['E'] = 4, F['N'] = 5;
    sort(Global::g_contest.players.begin(), Global::g_contest.players.end(), [&](const Player& x, const Player& y)
    {
        if (!x.GetProbLabel(p)->GetScore() && !y.GetProbLabel(p)->GetScore())
            return  F[x.GetProbLabel(p)->GetState()] <  F[y.GetProbLabel(p)->GetState()] ||
                   (F[x.GetProbLabel(p)->GetState()] == F[y.GetProbLabel(p)->GetState()] && cmpSumScore(x, y));
        else
            return  x.GetProbLabel(p)->GetResult() <  y.GetProbLabel(p)->GetResult() ||
                   (x.GetProbLabel(p)->GetResult() == y.GetProbLabel(p)->GetResult() && cmpSumScore(x, y));
    });
}

void BoardTable::clearHighlighted(int c)
{
    if (c >= 0)
        for (int i = 0; i < Global::g_contest.player_num; i++)
        {
            int t = GetLogicalRow(i);
            Player* p = &Global::g_contest.players[t];
            ResultLabel* tmp = p->GetLabel(c);
            if (tmp->GetLabelStyle() >= 0) tmp->setStyleSheet(LABEL_STYLE_SOFT[tmp->GetLabelStyle()]);
        }
}

void BoardTable::setHighlighted(int c)
{
    if (c >= 0)
        for (int i = 0; i < Global::g_contest.player_num; i++) //auto i:players)
        {
            int t = GetLogicalRow(i);
            Player* p = &Global::g_contest.players[t];
            ResultLabel* tmp = p->GetLabel(c);
            if (tmp->GetLabelStyle() >= 0)
                tmp->setStyleSheet(LABEL_STYLE_HARD[tmp->GetLabelStyle()] + "QLabel{border-width:1px;}");
            else if (c)
                tmp->setStyleSheet("");
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
        this->item(i.GetID(), c)->setData(Qt::DisplayRole, k);
        i.SetID(GetLogicalRow(k));
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

void BoardTable::onUpdateProblemLabel(Player* ply, int c, int sum)
{
    if (Global::g_contest_closed) return;
    if (c == 1)
        showProblemSumResult(ply->GetSumLabel(), sum, 1);
    else
        showProblemResult(ply->GetProbLabel(c - 2), sum, c);
}
