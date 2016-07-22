#ifndef CONTEST_H
#define CONTEST_H

#include "player.h"
#include "problem.h"

#include <vector>
#include <QDir>

struct Contest
{
    QString name, path, src_path, data_path, result_path;

    int player_num, problem_num, sum_score;
    std::vector<Player> players;
    std::vector<Problem> problems;

    QList<int> problem_order;
    bool is_list_used;

    /// 创建竞赛图标
    static QPixmap CreateIcon(const QString& path);

    /// 读取 problemOrder
    static QStringList ReadProblemOrder(const QString& path);

    /// 清空信息
    void Clear();

    /// 读取目录下的所有文件夹
    QStringList ReadFolders(const QString& path);

    /// 保存 problemOrder
    void SaveProblemOrder(const QStringList& list);

    /// 读取竞赛信息
    void ReadContestInfo();

    /// 缓存测评结果到 .reslist
    void SaveResultCache();

    /// 读取选手名单，是否保存为 .list
    void ReadPlayerList(QFile& fileName, bool isSaveList = false);

    /// 导出选手成绩
    void ExportPlayerScore(QFile& fileName);

    /// 设置路径
    void SetPath(const QString& path)
    {
        this->name = QDir(path).dirName();
        this->path = path + "/";
        this->data_path = path + "/data/";
        this->src_path = path + "/src/";
        this->result_path = path + "/result/";
    }
};

#endif // CONTEST_H
