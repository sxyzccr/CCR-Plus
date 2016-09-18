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
    explicit DetailTable(QWidget* parent = nullptr);
    ~DetailTable() {}

    // 上锁与解锁，防止测评时点击
    void Lock() { is_locked = true; }
    void Unlock() { is_locked = false; }

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
    void onAddTitleDetail(const QString& title);

    /// 信息显示单元格
    void onAddNoteDetail(const QString& note, const QString& state = "N");

    /// 测试点单元格
    void onAddPointDetail(int num, const QString& note, const QString& state, const QString& inOut, int subTaskLen);

    /// 分数单元格
    void onAddScoreDetail(int subTaskLen, int score, int sumScore);

    /// 根据在 board 中点击的位置，显示普通的 detail 表格
    void onShowDetail(int row, int column);

    /// 显示配置试题后的 detail 表格
    void onShowConfigurationDetail();

private:
    bool is_scrollBar_at_bottom, is_show_detail, is_locked;
    QElapsedTimer last_judge_timer;
    int rows;
};

#endif // DETAILTABLE_H
