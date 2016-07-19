#ifndef CONTESTDATA_H
#define CONTESTDATA_H

#include "player.h"
#include "problem.h"

#include <vector>

namespace ContestInfo
{

struct Info
{
    QString testPath, srcPath, dataPath, resultPath, testName;

    int playerNum, problemNum, sumScore;
    std::vector<Player> players;
    std::vector<Problem> problems;

    QList<int> problemOrder;
    bool isListUsed;

    void clear();
    void saveProblemOrder(const QStringList& list);
    void setPath(const QString& path)
    {
        testPath = path + "/";
        dataPath = path + "/data/";
        srcPath = path + "/src/";
        resultPath = path + "/result/";
        testName = QDir(path).dirName();
    }
} extern info;

}


#endif // CONTESTDATA_H
