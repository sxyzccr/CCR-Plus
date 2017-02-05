#ifndef DETAILTABLE_H
#define DETAILTABLE_H

#include <QTableWidget>
#include <QElapsedTimer>

#include "common/player.h"
#include "common/problem.h"

class DetailTable : public QTableWidget
{
    Q_OBJECT
public:
    explicit DetailTable(QWidget* parent = nullptr);
    ~DetailTable() {}

    // 上锁与解锁，防止测评时点击
    void Lock() { is_locked = true; }
    void Unlock() { is_locked = false; }

    /// 在开始测评时启动一个计时器，防止多次点击导致测评结果的 detail 变成普通的 detail
    void StartLastJudgeTimer() { last_judge_timer.start(); }

    /// 清空 detail 表格
    void ClearDetail();

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

    QList<const Player*> player_at;
    QList<const Problem*> problem_at;
    Player* current_player;
    Problem* current_problem;
    QAction *action_in, *action_out, *action_sub;
    QMenu* menu;

    /// 调整滚动条
    void adjustScrollBar();

    /// 显示某选手某一题的 detail
    void showProblemDetail(Player* player, Problem* problem);

private slots:
    void onOpenInFile();
    void onOpenOutFile();
    void onOpenSubmitFile();
    void onContextMenuEvent(const QPoint& pos);
};

#endif // DETAILTABLE_H
