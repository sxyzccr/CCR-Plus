#include "contest.h"

#include <map>
#include <QtXml>
#include <QPainter>

using namespace std;

QPixmap Contest::CreateIcon(const QString& contestPath)
{
    QStringList list = ReadProblemOrder(contestPath);
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

QStringList Contest::ReadProblemOrder(const QString& contestPath)
{
    QFile file(contestPath + ".ccr");
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

void Contest::Clear()
{
    player_num = problem_num = 0;
    sum_score = 0;

    for (auto i : problems) i.clear();
    for (auto i : players) i.clear();

    problems.clear();
    players.clear();
    problem_order.clear();
}

QStringList Contest::ReadFolders(const QString& path)
{
    QDir dir(path);
    QStringList list = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (path == data_path)
    {
        QStringList tmp;
        for (auto i : list) if (QFile::exists(data_path + i + "/.prb")) tmp.append(i);
        return tmp;
    }
    else return list;
}

void Contest::SaveProblemOrder(const QStringList& list)
{
    QFile file(path + ".ccr");
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

void Contest::ReadContestInfo()
{
    Clear();

    // 读取 src 文件夹, data 文件夹与 .ccr 文件
    QStringList playerName = ReadFolders(src_path),
                problemName = ReadFolders(data_path),
                order = ReadProblemOrder(path);

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
        problem_order.push_back(problem_num);
        problemID[name] = problem_num;
        problems.push_back(Problem(name));

        problem_num++;
    }

    for (auto name : playerName)
    {
        playerID[name] = player_num;
        Player p(name, player_num);
        for (int i = 0; i < problem_num; i++) p.problem.push_back(Player::Result());
        players.push_back(p);

        player_num++;
    }

    // 读取 result 文件夹
    QFile file(result_path + ".reslist");
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
                file.setFileName(result_path + i.first + "/" + j.first + ".res");
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
    for (auto& i : problems) i.readConfig();

    for (auto i : problems) sum_score += i.sumScore;
    for (auto& i : players) i.calcSum();
}

void Contest::SaveResultCache()
{
    QFile file(result_path + ".reslist");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        for (int i = 0; i < players.size(); i++)
        {
            int t = GetLogicalRow(i);
            Player* p = &players[t];
            for (auto j : problem_order)
            {
                Player::Result* r = &p->problem[j];
                out << p->name << '/' << problems[j].name << '/' << r->score << '/' << r->usedTime << '/' << r->state << '/' << endl;
            }
        }
        file.close();
    }
}

void Contest::ReadPlayerList(QFile& file, bool isSaveList)
{
    map<QString, QString> list;
    list.clear();
    QTextStream in(&file);
    for (; !in.atEnd();)
    {
        QString s = in.readLine();
        //in.setCodec("UTF-8");
        QStringList line = s.split(",");
        for (auto& i : line) i = i.trimmed(), i.remove('\"');
        if (line.size() >= 2 && line[0].size() && line[1].size()) list[line[0]] = line[1];
    }

    if (isSaveList)
    {
        QFile f(path + ".list");
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&f);
            //out.setCodec("UTF-8");
            for (auto i : list) out << i.first << "," << i.second << endl;
            f.close();
        }
    }

    for (auto& i : players)
        if (!i.type)
        {
            QLabel* tmp = i.label[0];
            if (list.count(i.name))
            {
                i.name_list = list[i.name];
                if (i.name_list.length() == 2) tmp->setText(QString("%1 [%2   %3]").arg(i.name, i.name_list.at(0), i.name_list.at(1)));
                else tmp->setText(QString("%1 [%2]").arg(i.name, i.name_list));
                tmp->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                tmp->setStyleSheet("");
            }
            else tmp->setStyleSheet("QLabel{color:rgb(120,120,120);}");
        }
}

void Contest::ExportPlayerScore(QFile& file)
{
    QTextStream out(&file);
    //out.setCodec("UTF-8");
    if (is_list_used) out << QString("编号,") << QString("姓名,"); else out << QString("选手,");
    out << QString("总分,");
    for (auto j : problem_order) out << QString("\"%1\",").arg(problems[j].name);
    out << endl;
    for (int i = 0; i < players.size(); i++)
    {
        int t = GetLogicalRow(i);
        Player* p = &players[t];
        out << QString("\"%1\",").arg(p->name);
        if (is_list_used) out << QString("\"%1\",").arg(p->name_list);
        out << p->sum.score << ",";
        for (auto j : problem_order) out << p->problem[j].score << ",";
        out << endl;
    }
    file.close();
}
