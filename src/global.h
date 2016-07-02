#ifndef GLOBAL_H
#define GLOBAL_H

#include "header.h"
#include "player.h"
#include "problem.h"

class Global
{
public:
    static int playerNum,problemNum,sumScore;
    static vector<Player> players;
    static vector<Problem> problems;
    static QList<int> problemOrder;
    static QString testPath,srcPath,dataPath,resultPath,testName;
    static QFont font,boldFont;
    static QString labelStyle1[17],labelStyle2[17];
    static bool alreadyJudging,isListUsed;
    static Qt::SortOrder preSortOrder;
    static QElapsedTimer clickTimer;
    static QList<pair<int,int>> judgeList;

    static void clear();
    static int logicalRow(int visualRow);
    static QColor ratioColor(int rl, int gl, int bl, int rr, int gr, int br, int x, int y);
    static Problem::CompilerInfo getCompiler(Player *player, Problem *problem);
    static void saveProblemOrder(const QStringList &list);
};

#endif // GLOBAL_H
