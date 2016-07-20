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
    ~JudgeThread();

    int r, c;

    void setup(int r, int c, const QString& dir);
    bool waitForClearedTmpDir(int ms);  //return:   1:success 0:fail
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
    double judgeTraditionalTask(Problem::Info* info, QString& note, QString& state, double& usedTime);
    double judgeAnswersOnlyTask(Problem::Info* info, QString& note, QString& state);
    void judgeProblem(Player* player, Problem* problem, char& state, int& sumScore, double& sumTime, QString& detail);
    double judgeTask(Problem::Info* info, QString& note, QString& state, double& usedTime, int testNum); //return ratio
    void initialize(const QString& name, Problem* prob);
    void saveHTMLResult(Player* player);
    CompileResult compile(const Problem::CompilerInfo& compiler, QString& note);

signals:
    void titleDetailFinished(int rows, const QString& title);
    void noteDetailFinished(int rows, const QString& note, const QString& state);
    void pointDetailFinished(int rows, int num, const QString& note, const QString& state, const QString& file, int len);
    void scoreDetailFinished(int rows, int len, int score, int sumScore);

    void itemJudgeFinished(int r, int c);
    void playerLabelChanged(QLabel* label, const QString& s1, const QString& s2, const QString& s3);
    void problemLabelChanged(Player* ply, int c, Player::Result* res, int sum);
};

#endif // JUDGETHREAD_H
