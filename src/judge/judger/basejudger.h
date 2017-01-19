#ifndef BASEJUDGER_H
#define BASEJUDGER_H

#include <QProcess>

#include "common/global.h"
#include "common/player.h"
#include "common/problem.h"

struct TestCaseResult
{
    TestCaseResult(double score = 0, double time = 0, char state = ' ', const QString& note = "") :
        score(score), time(time), state(state), note(note) {}

    double score, time;
    char state;
    QString note;
};



class BaseJudger : public QObject
{
    Q_OBJECT
public:
    explicit BaseJudger(const QString& testDir, Player* player, const Problem* problem, QObject *parent = 0);
    virtual ~BaseJudger();

    // Getter member functions
    QString TmpDirectory() const { return tmp_dir; }
    QString SrcDirectory() const { return src_dir; }
    QString DataDirectory() const { return data_dir; }
    QString WorkingDirectory() const { return working_dir; }

    /// 测评一位选手的一道题
    ResultSummary Judge();

    /// 停止测评(目前没什么卵用)
    void StopJudge() { emit judgeStoped(); }

protected:
    Player* player;
    const Problem* problem;

    /// 检查输出文件
    TestCaseResult checkOutput(const QString& inFile, const QString& ansFile, const QString& outFile) const;

    /// 编译
    Global::CompileResult compile(const Compiler* compiler, QString& resultNote) const;

    /// 在时间内存限制下运行程序
    TestCaseResult runProgram(const QString& exe, double timeLim, double memLim) const;

    /// 该题是否需要编译
    virtual bool needCompile() const = 0;

    /// 一个测试点的测评
    virtual TestCaseResult judgeTestCase(const TestCase* point) const = 0;

private:
    QString test_dir, tmp_dir, data_dir, src_dir, working_dir;

    // 阻塞最多 ms 毫秒，直到建立/删除目录，返回：true 成功，false 失败
    bool waitForMakeTmpDirectory(int ms) const;
    bool waitForCleanTmpDirectory(int ms) const;
    bool waitForMakeWorkingDirectory(int num, int ms);
    bool waitForCleanWorkingDirectory(int num, int ms) const;

    /// 监控进程，时间限制为 ms 毫秒，返回：true 正常退出，false 超时或被杀死
    bool monitorProcess(QProcess* process, int ms) const;

    /// 一个测试点的测评，多一些准备和收尾工作
    TestCaseResult judgeTestCaseEvent(const TestCase* point, int num);

signals:
    void judgeStoped();
    void titleDetailFinished(const QString& title);
    void noteDetailFinished(const QString& note, const QString& state);
    void pointDetailFinished(int num, const QString& note, const QString& state, const QString& inOut, int subTaskLen);
    void scoreDetailFinished(int subTaskLen, int score, int sumScore);
};

#endif // BASEJUDGER_H
