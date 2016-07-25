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

    /// 界面初始化设置
    void Setup();

    /// 清空 board 表格
    void ClearBoard();

    /// 根据选手的名字长度调整标签
    void ResizePlayerLabel();

    /// 设置选手总分标签
    void SetSumResultLabel(ResultLabel* tmp, int sum);

    /// 设置选手某一列的标签
    void SetProblemResultLabel(ResultLabel* tmp, int sum);

    /// 在 board 上显示结果
    void ShowResult();

    /// 设置某列高亮
    void SetHighlighted(int c);

    /// 取消某列高亮
    void ClearHighlighted(int c);

    /// 取消之前设置的高亮
    void ClearHighlighted()
    {
        ClearHighlighted(pre_highlighted_col);
        pre_highlighted_col = -1;
    }

public slots:
    /// 移动一列
    void onSectionMove(int logicalIndex, int oldVisualIndex, int newVisualIndex);

    /// 表格排序
    void onSortTable(int);

    /// 取消选中单元格
    void onSetItemUnselected(int r, int c);

    /// 更新标签文本
    void onUpdateResultLabelText(ResultLabel* tmp, const QString &text, const QString &toolTip, Global::LabelStyle style);

    /// 更新选手总分标签
    void onUpdateSumResultLabel(Player* player, int sum);

    /// 更新选手某一列的标签
    void onUpdateProblemResultLabel(Player* player, int c, int sum);

private:
    int pre_highlighted_col;
    bool already_moving_section;
};

#endif // BOARDTABLE_H
