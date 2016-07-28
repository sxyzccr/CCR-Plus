#ifndef JUDGETHREAD_H
#define JUDGETHREAD_H

#include "player.h"
#include "problem.h"
#include "boardtable.h"
#include "detailtable.h"

#include <set>
#include <QThread>
#include <QProcess>

class JudgeThread: public QThread
{
    Q_OBJECT
public:
    explicit JudgeThread(QObject* parent = 0);
    virtual ~JudgeThread();

    int r, c;

    void setup(int r, int c, const QString& dir);
    bool waitForClearedTmpDir(int ms);  //return:   1:success 0:fail
    bool waitForFinished(int ms);
    void appendProblem(const QPair<int, int>& p) { judgeList.append(p); }
    void run();

private:
    QString testDir, srcDir, dataDir, tmpDir, judgeDir, playerName;
    Problem* problem;
    std::set<int> notCleared;
    QList<QPair<int, int>> judgeList;
    int rows;

    bool makeJudgeDir(int num); //return:   1:success 0:fail
    bool waitForMadeTmpDir(int ms); //return:   1:success 0:fail
    bool waitForClearJudgeDir(int num, int ms); //return:   1:success 0:fail
    bool monitorProcess(QProcess* process, int ms);  //return:   1:EXITED 0:KILLED or TLE
    bool runProgram(double timeLim, double memLim, QString& note, QString& state, double& usedTime); //return:   1:normal 0:killed
    double judgeOutput(const QString& inFile, const QString& ansFile, const QString& outFile, QString& note, QString& state);
    double judgeTraditionalTask(TestCase* ponit, QString& note, QString& state, double& usedTime);
    double judgeAnswersOnlyTask(TestCase* point, QString& note, QString& state);
    void judgeProblem(Player* player, Problem* problem, char& state, int& sumScore, double& sumTime, QString& detail);
    double judgeTask(TestCase* point, QString& note, QString& state, double& usedTime, int testNum); //return ratio
    void initialize(const QString& name, Problem* prob);
    void saveHTMLResult(Player* player);
    Global::CompileResult compile(Compiler* compiler, QString& note);

signals:
    void titleDetailFinished(int rows, const QString& title);
    void noteDetailFinished(int rows, const QString& note, const QString& state);
    void pointDetailFinished(int rows, int num, const QString& note, const QString& state, const QString& inOut, int len);
    void scoreDetailFinished(int rows, int len, int score, int sumScore);

    void itemJudgeFinished(int r, int c);
    void resultLabelTextChanged(ResultLabel* tmp, const QString& text, const QString& toolTip, Global::LabelStyle style);
    void sumResultLabelChanged(Player* player, int sum);
    void problemResultLabelChanged(Player* player, int c, int sum);
};

#endif // JUDGETHREAD_H
