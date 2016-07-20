#include "contestinfo.h"

#include <map>
#include <QtXml>
#include <QPainter>

using namespace std;

namespace ContestInfo
{
Info info;
}

void ContestInfo::Info::Clear()
{
    playerNum = problemNum = 0;
    sumScore = 0;

    for (auto i : problems) i.clear();
    for (auto i : players) i.clear();

    problems.clear();
    players.clear();
    problemOrder.clear();
}

QPixmap ContestInfo::Info::CreateIcon(const QString& path)
{
    QStringList list = ReadProblemOrder(path);
    QImage image = QPixmap(":/icon/image/folder.png").toImage();
    QPainter painter(&image);
    painter.setFont(QFont("Times New Roman", 15, 0, true));
    QFontMetrics fm = painter.fontMetrics();
    QPair<int, int> pos[3] = {{15, 38}, {15, 60}, {15, 82}};
    int n = min(list.size(), 3);
    for (int i = 0; i < n; i++)
    {
        QString text = QString(" %1 ").arg(list[i]);
        int width = min(fm.width(text), 109 - pos[i].first);
        int height = fm.height();
        painter.drawText(QRectF(pos[i].first, pos[i].second, width, height), text);
    }
    return QPixmap::fromImage(image);
}

QStringList ContestInfo::Info::ReadFolders(const QString& path)
{
    QDir dir(path);
    QStringList list = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (path == dataPath)
    {
        QStringList tmp;
        for (auto i : list) if (QFile::exists(dataPath + i + "/.prb")) tmp.append(i);
        return tmp;
    }
    else return list;
}

QStringList ContestInfo::Info::ReadProblemOrder(const QString& path)
{
    QFile file(path + ".ccr");
    QStringList problem;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QDomDocument doc;
        if (doc.setContent(&file))
        {
            QDomElement root = doc.documentElement();
            if (root.isNull() || root.tagName() != "contest") {file.close(); return problem;}
            QDomNodeList list = root.childNodes();
            for (int i = 0; i < list.count(); i++)
            {
                QDomElement a = list.item(i).toElement();
                if (a.tagName() == "order")
                {
                    QDomNodeList l = a.childNodes();
                    for (int j = 0; j < l.size(); j++)
                    {
                        QDomElement b = l.item(j).toElement();
                        if (b.tagName() == "problem") problem.append(b.text());
                    }
                }
            }
        }
        file.close();
    }
    return problem;
}

void ContestInfo::Info::SaveProblemOrder(const QStringList& list)
{
    QFile file(contestPath + ".ccr");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<contest>\n";
    out << "    <order>\n";
    for (auto i : list) out << QString("        <problem>%1</problem>\n").arg(i);
    out << "    </order>\n";
    out << "</contest>\n";
    file.close();
}

void ContestInfo::Info::ReadContestInfo()
{
    Clear();

    // 读取 src 文件夹, data 文件夹与 .ccr 文件
    QStringList playerName = ReadFolders(ContestInfo::info.srcPath),
                problemName = ReadFolders(ContestInfo::info.dataPath),
                order = ReadProblemOrder(ContestInfo::info.contestPath);

    map<QString, int> playerID, problemID;
    problemID.clear();
    playerID.clear();

    QStringList tmp;
    for (auto i : order) if (problemName.count(i)) tmp.append(i);
    for (auto i : problemName) if (!tmp.count(i)) tmp.append(i);
    problemName = tmp;
    SaveProblemOrder(problemName);

    for (auto name : problemName)
    {
        problemOrder.push_back(problemNum);
        problemID[name] = problemNum;
        problems.push_back(Problem(name));

        problemNum++;
    }

    for (auto name : playerName)
    {
        playerID[name] = playerNum;
        Player p(name, playerNum);
        for (int i = 0; i < problemNum; i++) p.problem.push_back(Player::Result());
        players.push_back(p);

        playerNum++;
    }

    // 读取 result 文件夹
    QFile file(resultPath + ".reslist");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        for (; !in.atEnd();)
        {
            QString str = in.readLine();
            QStringList list = str.split("/");
            QString a = list[0], b = list[1];
            int s = list[2].toInt();
            double t = list[3].toDouble();
            char c = list[4][0].toLatin1();
            if (!playerID.count(a) || !problemID.count(b)) continue;
            int x = playerID[a], y = problemID[b];

            // 兼容旧版
            if (c == 'R') c = 'N';
            if (c == 'O') c = 'N';
            if (c != 'N' && c != 'C' && c != 'F' && c != 'S' && c != 'E') c = 0;
            players[x].problem[y].state = c;
            players[x].problem[y].score = s;
            players[x].problem[y].usedTime = t;
        }
        file.close();
    }
    for (auto i : problemID)
        for (auto j : playerID)
            if (!players[j.second].problem[i.second].state)
            {
                file.setFileName(resultPath + i.first + "/" + j.first + ".res");
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
                QDomDocument doc;
                if (!doc.setContent(&file)) {file.close(); continue;}
                QDomElement root = doc.documentElement();
                //qDebug()<<i.first<<i.second<<j.first<<j.second<<root.attribute("score")<<root.attribute("time")<<root.attribute("state");

                int s = root.attribute("score").toInt();
                double t = root.attribute("time").toDouble();
                char c = root.attribute("state")[0].toLatin1();
                int x = j.second, y = i.second;

                // 兼容旧版
                if (c == 'R') c = 'N';
                if (c == 'O') c = 'N';
                if (c != 'N' && c != 'C' && c != 'F' && c != 'S' && c != 'E') c = 0;
                players[x].problem[y].state = c;
                players[x].problem[y].score = s;
                players[x].problem[y].usedTime = t;

                file.close();
            }

    // 读取 .prb 文件
    for (auto& i : info.problems) i.readConfig();

    for (auto i : info.problems) ContestInfo::info.sumScore += i.sumScore;
    for (auto& i : info.players) i.calcSum();
}
