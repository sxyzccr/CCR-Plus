#ifndef JUDGETHREAD_H
#define JUDGETHREAD_H

#include <QThread>

#include "common/player.h"
#include "common/problem.h"

class JudgeThread: public QThread
{
    Q_OBJECT
public:
    explicit JudgeThread(int row, int column, QObject* parent = nullptr);
    ~JudgeThread() {}

    /// 阻塞最多 ms 毫秒，直到线程结束
    bool WaitForFinished(int ms);

    /// 添加要测评的项
    void AppendProblem(int row, int column) { judge_list.append(qMakePair(row, column)); }

    /// 停止测评(目前没什么卵用~)
    void StopJudge() { emit judgeStoped(); }

private:
    static bool is_judging;

    int row, column;
    QList<QPair<int, int>> judge_list;

    /// 清空正在测评项的 ResultLabel
    void cleanResultLabel(Player* player, int column);

    /// 测评一位选手的一道题
    void judgeProblem(Player* player, int column);

    /// 线程启动点
    void run() override;

signals:
    void judgeStoped();

    void titleDetailFinished(const QString& title);
    void noteDetailFinished(const QString& note, const QString& state);
    void pointDetailFinished(int num, const QString& note, const QString& state, const QString& inOut, int subTaskLen);
    void scoreDetailFinished(int subTaskLen, int score, int sumScore);

    void itemJudgeFinished(int row, int column);
    void labelTextChanged(ResultLabel* tmp, const QString& text, const QString& toolTip, Global::LabelStyle style);
    void sumLabelChanged(Player* player);
    void problemLabelChanged(Player* player, int column);
};

#endif // JUDGETHREAD_H
