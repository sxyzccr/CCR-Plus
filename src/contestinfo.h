#ifndef CONTESTDATA_H
#define CONTESTDATA_H

#include "player.h"
#include "problem.h"

#include <vector>
#include <QDir>

namespace ContestInfo
{

struct Info
{
    QString contestPath, srcPath, dataPath, resultPath, contestName;

    int playerNum, problemNum, sumScore;
    std::vector<Player> players;
    std::vector<Problem> problems;

    QList<int> problemOrder;
    bool isListUsed;

    /// 清空信息
    void Clear();

    /// 创建竞赛图标
    static QPixmap CreateIcon(const QString& path);

    /// 读取目录下的所有文件夹
    QStringList ReadFolders(const QString& path);

    /// 读取 problemOrder
    static QStringList ReadProblemOrder(const QString& path);

    /// 保存 problemOrder
    void SaveProblemOrder(const QStringList& list);

    /// 读取竞赛信息
    void ReadContestInfo();

    /// 设置路径
    void SetPath(const QString& path)
    {
        contestPath = path + "/";
        dataPath = path + "/data/";
        srcPath = path + "/src/";
        resultPath = path + "/result/";
        contestName = QDir(path).dirName();
    }

} extern info;

}


#endif // CONTESTDATA_H
