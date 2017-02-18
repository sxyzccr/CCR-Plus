#ifndef BOARDTABLE_H
#define BOARDTABLE_H

#include <QTableWidget>

#include "common/const.h"

class ResultLabel;
class Player;

class BoardTable : public QTableWidget
{
    Q_OBJECT
public:
    explicit BoardTable(QWidget* parent = nullptr);
    ~BoardTable() {}

    // 上锁与解锁，防止测评时点击
    void Lock() { is_locked = true; }
    void Unlock() { is_locked = false; }

    /// 清空 board 表格
    void ClearBoard();

    /// 根据选手的名字长度调整标签
    void ResizePlayerLabel();

    /// 设置选手总分标签
    void SetSumLabel(ResultLabel* tmp);

    /// 设置选手某一列的标签
    void SetProblemLabel(ResultLabel* tmp, int sum);

    /// 在 board 上显示结果
    void ShowResult();

    /// 设置某列高亮
    void SetHighlighted(int column);

    /// 取消某列高亮
    void ClearHighlighted(int column);

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
    void onSortTable(int column);

    /// 取消选中单元格
    void onSetItemUnselected(int row, int column);

    /// 更新标签文本
    void onUpdateLabelText(ResultLabel* tmp, const QString& text, const QString& toolTip, Global::LabelStyle style);

    /// 更新选手总分标签
    void onUpdateSumLabel(Player* player);

    /// 更新选手某一列的标签
    void onUpdateProblemLabel(Player* player, int column);

private:
    int pre_highlighted_col, sum_label_width, problem_label_width;
    bool already_moving_section, is_locked;
};

#endif // BOARDTABLE_H
