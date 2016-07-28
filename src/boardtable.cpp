#include "global.h"
#include "boardtable.h"
#include "contest.h"

#include <QScrollBar>
#include <QHeaderView>

using namespace std;

BoardTable::BoardTable(QWidget* parent) : QTableWidget(parent),
    pre_highlighted_col(-1), already_moving_section(false)
{
    Setup();
}

BoardTable::~BoardTable()
{

}

void BoardTable::Setup()
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

    pre_highlighted_col = -1;
    already_moving_section = false;
}

void BoardTable::ClearBoard()
{
    this->clear();
    this->setRowCount(0);
    this->setColumnCount(2);

    pre_highlighted_col = -1;
    already_moving_section = false;
}

void BoardTable::ResizePlayerLabel()
{
    int len = 75;
    for (auto i : Global::g_contest.players)
    {
        QLabel* tmp = i->NameLabel();
        len = max(len, QFontMetrics(Global::FONT).width(tmp->text()) + 30);
    }
    this->horizontalHeader()->resizeSection(0, len);
}

void BoardTable::SetSumResultLabel(ResultLabel* tmp, int sum)
{
    Global::LabelStyle x = Global::StyleNone;
    ResultSummary res = tmp->Result();

    tmp->setText(QFontMetrics(tmp->font()).elidedText(QString::number(res.score), Qt::ElideRight, 60 - 5));
    tmp->setToolTip(QString("总用时: %1s").arg(res.time, 0, 'f', 2));

    if (!sum) x = Global::StyleInvalidOrStateCFS;
    else if (res.score > sum) x = Global::StyleScoreGreater_100;
    else if (res.score < 0) x = Global::StyleScoreLess_0, tmp->setToolTip("无效的总分");
    else if (res.score == 0) x = Global::StyleScore_0;
    else x = Global::LabelStyle(res.score * 10 / sum);

    tmp->setStyleSheet(ResultLabel::GetLabelStyleSheet(x));
    tmp->SetLabelStyle(x);
}

void BoardTable::SetProblemResultLabel(ResultLabel* tmp, int sum)
{
    Global::LabelStyle x = Global::StyleNone;
    ResultSummary res = tmp->Result();

    tmp->setText(QFontMetrics(tmp->font()).elidedText(QString::number(res.score), Qt::ElideRight, 85 - 5));
    tmp->setToolTip(QString("用时: %1s").arg(res.time, 0, 'f', 2));

    switch (res.state)
    {
    case 'N':
        if (!sum) x = Global::StyleInvalidOrStateCFS;
        else if (res.score > sum) x = Global::StyleScoreGreater_100;
        else if (res.score < 0) x = Global::StyleScoreLess_0, tmp->setToolTip("无效的得分");
        else if (res.score == 0) x = Global::StyleScore_0;
        else x = Global::LabelStyle(res.score * 10 / sum);
        break;
    case 'C':
        x = Global::StyleInvalidOrStateCFS;
        tmp->setText("+");
        tmp->setToolTip("编译错误");
        break;
    case 'F':
        x = Global::StyleInvalidOrStateCFS;
        tmp->setText("-");
        tmp->setToolTip("找不到文件");
        break;
    case 'S':
        x = Global::StyleInvalidOrStateCFS;
        tmp->setText("=");
        tmp->setToolTip("超过代码长度限制");
        break;
    case 'E':
        x = Global::StyleStateE;
        tmp->setToolTip("测评器或校验器出错");
        break;
    case ' ':
        x = Global::StyleNone;
        tmp->setText("");
        tmp->setToolTip("未测评");
        break;
    }

    tmp->setStyleSheet(ResultLabel::GetLabelStyleSheet(x));
    tmp->SetLabelStyle(x);
}

void BoardTable::ShowResult()
{
    int row = 0;
    for (auto i : Global::g_contest.players)
    {
        ResultLabel* tmp;

        tmp = i->NameLabel();
        tmp->setToolTip(i->Name());
        tmp->setIndent(15);
        tmp->setAlignment(Qt::AlignCenter);
        tmp->SetLabelStyle(Global::StyleNone);
        this->setCellWidget(row, 0, tmp);

        i->SetSpecialNameLabel();

        tmp = i->SumLabel();
        tmp->setAlignment(Qt::AlignCenter);
        SetSumResultLabel(tmp, Global::g_contest.sum_score);
        this->setCellWidget(row, 1, tmp);

        int col = 0;
        for (int j = 0; j < Global::g_contest.problem_num; j++)
        {
            tmp = i->ProblemLabelAt(j);
            tmp->setAlignment(Qt::AlignCenter);
            SetProblemResultLabel(tmp, Global::g_contest.problems[col]->Score());
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

void BoardTable::SetHighlighted(int c)
{
    if (c <= 0) return;
    for (auto i : Global::g_contest.players)
    {
        ResultLabel* tmp = i->LabelAt(c);
        tmp->setStyleSheet(ResultLabel::GetLabelStyleSheet(tmp->LabelStyle(), true));
    }
}

void BoardTable::ClearHighlighted(int c)
{
    if (c <= 0) return;
    for (auto i : Global::g_contest.players)
    {
        ResultLabel* tmp = i->LabelAt(c);
        tmp->setStyleSheet(ResultLabel::GetLabelStyleSheet(tmp->LabelStyle()));
    }
}



void BoardTable::onSectionMove(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
    //qDebug()<<i<<oldV<<newV;
    if (already_moving_section) return;
    if (logicalIndex <= 1 || newVisualIndex <= 1) // 前两列不能移动，其他列也不能移动到前两列
    {
        already_moving_section = true;
        this->horizontalHeader()->moveSection(newVisualIndex, oldVisualIndex);
        already_moving_section = false;
    }
    else
    {
        Global::g_contest.problem_order.move(oldVisualIndex - 2, newVisualIndex - 2);
        QStringList list;
        for (auto i : Global::g_contest.problem_order) list.append(Global::g_contest.problems[i]->Name());
        Global::g_contest.SaveProblemOrder(list);
    }
}

void BoardTable::onSortTable(int c)
{
    if (Global::g_is_judging) return;

    if (pre_highlighted_col != c)
    {
        ClearHighlighted(pre_highlighted_col);
        SetHighlighted(c);
    }

    this->horizontalHeader()->setSortIndicatorShown(true);
    if (pre_highlighted_col != c && c >= 1) this->horizontalHeader()->setSortIndicator(c, Qt::DescendingOrder);
    Global::g_pre_sort_order = this->horizontalHeader()->sortIndicatorOrder();

    if (!c)
        Global::g_contest.SortPlayers(CmpName);
    else if (c == 1)
        Global::g_contest.SortPlayers(CmpSumScore);
    else
    {
        Global::g_sort_key_col = c - 2;
        Global::g_contest.SortPlayers(CmpProblem);
    }

    int k = 0;
    for (auto i : Global::g_contest.players)
    {
        this->item(i->Id(), c)->setData(Qt::DisplayRole, k);
        i->SetId(Global::GetLogicalRow(k));
        k++;
    }
    this->sortByColumn(c);
    this->verticalScrollBar()->setValue(0);
    //  for (int i=0; i<playerNum; i++) qDebug()<<ui->tableWidget->item(i,c)->data(Qt::DisplayRole);

    pre_highlighted_col = c;
}

void BoardTable::onSetItemUnselected(int r, int c)
{
    if (Global::g_is_contest_closed) return;
    this->item(r, c)->setSelected(false);
}

void BoardTable::onUpdateResultLabelText(ResultLabel* tmp, const QString& text, const QString& toolTip, Global::LabelStyle style)
{
    if (Global::g_is_contest_closed) return;
    tmp->setText(text);
    tmp->setToolTip(toolTip);
    tmp->setStyleSheet(ResultLabel::GetLabelStyleSheet(style));
    tmp->SetLabelStyle(style);
}

void BoardTable::onUpdateSumResultLabel(Player* player, int sum)
{
    if (Global::g_is_contest_closed) return;
    SetSumResultLabel(player->SumLabel(), sum);
}

void BoardTable::onUpdateProblemResultLabel(Player* player, int c, int sum)
{
    if (Global::g_is_contest_closed) return;
    SetProblemResultLabel(player->ProblemLabelAt(c - 2), sum);
}
