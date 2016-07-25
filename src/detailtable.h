#ifndef DETAILTABLE_H
#define DETAILTABLE_H

#include "player.h"
#include "problem.h"

#include <QTableWidget>
#include <QElapsedTimer>

class DetailTable : public QTableWidget
{
    Q_OBJECT
public:
    explicit DetailTable(QWidget* parent = 0);
    virtual ~DetailTable();

    /// 界面初始化设置
    void Setup();

    /// 清空 detail 表格
    void ClearDetail();

    /// 在开始测评时启动一个计时器，防止多次点击导致测评结果的 detail 变成普通的 detail
    void StartLastJudgeTimer() { last_judge_timer.start(); }

    /// 调整滚动条
    void AdjustScrollBar();

    /// 显示某选手某一题的 detail
    void ShowProblemDetail(Player* player, Problem* problem);

public slots:
    /// 标题单元格
    void onAddTitleDetail(int row, const QString& title);

    /// 信息显示单元格
    void onAddNoteDetail(int row, const QString& note, const QString& state = "N");

    /// 测试点单元格
    void onAddPointDetail(int row, int num, const QString& note, const QString& state, const QString& file, int len);

    /// 分数单元格
    void onAddScoreDetail(int row, int len, int score, int sumScore);

    /// 根据在 board 中点击的位置，显示普通的 detail 表格
    void onShowDetail(int r, int c);

    /// 显示配置试题后的 detail 表格
    void onShowConfigDetail();

private:
    bool is_scrollBar_at_bottom;
    QElapsedTimer last_judge_timer;
};

#endif // DETAILTABLE_H
