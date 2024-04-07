#ifndef CONTEST_H
#define CONTEST_H

#include <QList>
#include <QString>
#include <algorithm>

class QFile;
class QPixmap;
class Player;
class Problem;

struct Contest
{
    ~Contest() { Clear(); }

    QString name, path, src_path, data_path, result_path;

    int player_num, problem_num, sum_score;
    QList<Player*> players;
    QList<Problem*> problems;

    QList<int> problem_order;
    bool is_list_used;

    /// 创建竞赛图标
    static QPixmap CreateIcon(const QString& path);

    /// 读取 problemOrder
    static QStringList ReadProblemOrder(const QString& path);

    /// 清空信息
    void Clear();

    /// 根据名称查找题目
    Problem* ProblemFromName(const QString& name);

    /// 读取目录下的所有文件夹
    QStringList ReadFolders(const QString& path);

    /// 保存 problemOrder
    void SaveProblemOrder(const QStringList& list);

    /// 读取竞赛信息
    void ReadContestInfo();

    /// 选手排序
    void SortPlayers(bool (*cmp)(const Player* x, const Player* y)) { std::sort(players.begin(), players.end(), cmp); }

    /// 缓存测评结果到 .reslist
    void SaveResultCache();

    /// 读取选手名单，是否保存为 .list
    void ReadPlayerList(QFile& fileName, bool isSaveList = false);

    /// 导出选手成绩
    void ExportPlayerScore(QFile& fileName);

    /// 设置路径
    void SetPath(const QString& path);
};

#endif // CONTEST_H
