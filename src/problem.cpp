#include "global.h"
#include "problem.h"

#include <QtXml>

using namespace std;

Problem::Problem(const QString& na)
{
    name = dir = exe = na;
    inFile = na + ".in";
    outFile = na + ".out";
    sumScore = 100;
    codeLim = 100;
    timeLim_checker = 10;
    type = OtherType;
}

Problem::~Problem()
{

}

void Problem::Clear()
{
    tasks.clear();
    que.clear();
    compilers.clear();
}

QString Problem::addSuff(QString file)
{
#ifdef Q_OS_WIN
    if (!file.endsWith(".exe")) file += ".exe";
#endif
    return file;
}

QString Problem::removeSuff(QString file)
{
#ifdef Q_OS_WIN
    if (file.endsWith(".exe")) file.remove(file.length() - 4, 4);
#endif
    return file;
}

void Problem::ReadConfig()
{
    sumScore = 0;
    QFile file(Global::g_contest.data_path + name + "/.prb");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QDomDocument doc;
    if (!doc.setContent(&file)) {file.close(); return;}

    QDomElement root = doc.documentElement();
    if (root.isNull() || root.tagName() != "problem") {file.close(); return;}
    QString s = root.attribute("type");
    if (s == "TRA" || s == "TRA_0_4") type = Traditional;
    else if (s == "ANS" || s == "ANS_0_4") type = AnswersOnly;
    else if (s == "INT" || s == "INT_0_4") type = Interactive;

    QDomNodeList list = root.childNodes();
    for (int i = 0; i < list.count(); i++)
    {
        QDomElement a = list.item(i).toElement();
        if (a.tagName() == "source")
        {
            if (a.hasAttribute("dir")) dir = a.attribute("dir");
            if (a.hasAttribute("file")) exe = addSuff(a.attribute("file"));
            if (a.hasAttribute("code")) codeLim = a.attribute("code").toDouble();

            QDomNodeList l = a.childNodes();
            for (int j = 0; j < l.count(); j++)
            {
                QDomElement b = l.item(j).toElement();
                if (b.tagName() == "language")
                {
                    CompilerInfo x(b.attribute("cmd"), b.attribute("file"));
                    //if (x.file.endsWith(".cpp")||x.file.endsWith(".c")) x.cmd+=" -static";
                    if (b.hasAttribute("time")) x.timeLim = b.attribute("time").toInt();
                    compilers.push_back(x);
                }
            }
        }
        else if (a.tagName() == "task")
        {
            inFile = a.attribute("input");
            outFile = a.attribute("output");
            checker = a.attribute("checker");
            if (checker == "全文比较") checker = "fulltext";
            checker = addSuff(checker);
            if (a.hasAttribute("time")) timeLim_checker = a.attribute("time").toInt();

            QDomNodeList l = a.childNodes();
            for (int j = 0; j < l.count(); j++)
            {
                QDomElement b = l.item(j).toElement();
                if (b.tagName() == "subtask")
                {
                    Subtask sub(b.attribute("score").toInt());
                    QDomNodeList ll = b.childNodes();
                    for (int k = 0; k < ll.count(); k++)
                    {
                        QDomElement c = ll.item(k).toElement();
                        if (c.tagName() == "point")
                        {
                            Info x(c.attribute("time").toDouble(), c.attribute("mem").toDouble());
                            x.in = c.attribute("in");
                            x.out = c.attribute("out");
                            x.sub = c.attribute("sub");
                            sub.point.push_back(que.size());
                            que.push_back(x);
                        }
                    }
                    sumScore += sub.score;
                    tasks.push_back(sub);
                }
            }
        }
    }
    file.close();
}

bool Problem::SaveConfig()
{
    QDomDocument doc;
    QDomProcessingInstruction xml = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xml);
    QDomElement root = doc.createElement("problem");
    root.setAttribute("type", type == ProblemType::Traditional ? "TRA_0_4" : type == ProblemType::AnswersOnly ? "ANS_0_4" : "UNKNOWN");
    root.setAttribute("maker", "ccr-plus");
    doc.appendChild(root);

    QDomElement source = doc.createElement("source");
    source.setAttribute("dir", dir);
    root.appendChild(source);
    if (type == ProblemType::Traditional)
    {
        source.setAttribute("file", removeSuff(exe));
        source.setAttribute("code", codeLim);
        for (auto i : compilers)
        {
            QDomElement lang = doc.createElement("language");
            lang.setAttribute("cmd", i.cmd);
            lang.setAttribute("file", i.file);
            lang.setAttribute("time", i.timeLim);
            source.appendChild(lang);
        }
    }

    QDomElement task = doc.createElement("task");
    if (type == ProblemType::Traditional)
    {
        task.setAttribute("input", inFile);
        task.setAttribute("output", outFile);
    }
    task.setAttribute("checker", removeSuff(checker));
    task.setAttribute("time", timeLim_checker);
    root.appendChild(task);
    for (auto i : tasks)
    {
        QDomElement subtask = doc.createElement("subtask");
        subtask.setAttribute("score", i.score);
        task.appendChild(subtask);
        for (auto j : i.point)
        {
            QDomElement point = doc.createElement("point");
            point.setAttribute("in", que[j].in);
            point.setAttribute("out", que[j].out);
            if (type == ProblemType::AnswersOnly) point.setAttribute("sub", que[j].sub);
            if (type == ProblemType::Traditional)
            {
                point.setAttribute("time", que[j].timeLim);
                point.setAttribute("mem", que[j].memLim);
            }
            subtask.appendChild(point);
        }
    }

    QFile file(Global::g_contest.data_path + name + "/.prb");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&file);
    doc.save(out, 4);
    file.close();
    return true;
}

typedef QPair<QString, QString> Pair;

QList<QPair<QString, QString>> Problem::getInAndOutFile()
{
    QString dir = Global::g_contest.data_path + name;
    QStringList list = QDir(dir).entryList(QDir::Files);
    const QStringList in({".in", ".inp", "in", "inp"}), out({".out", ".ans", ".ou", ".an", ".sol", ".res", ".std", "out", "ans", "ou", "an", "sol", "res", "std"});
    QList<Pair> Q[in.size()][out.size()];
    int ma = 0;

    for (int i = 0; i < in.size(); i++)
        for (int j = 0; j < out.size(); j++)
        {
            QString a = in[i], b = out[j];
            QMap<Pair, int> F;
            for (auto s : list)
            {
                int p = s.indexOf(a), q = s.indexOf(b);
                if (p != -1) F[Pair(s.left(p), s.right(s.length() - a.length() - p))]++;
                if (q != -1) F[Pair(s.left(q), s.right(s.length() - b.length() - q))]++;
            }
            for (auto k = F.constBegin(); k != F.constEnd(); k++)
                if (k.value() == 2) Q[i][j].append(Pair(k.key().first + a + k.key().second, k.key().first + b + k.key().second));
            ma = max(ma, Q[i][j].size());
        }
    for (int i = 0; i < in.size(); i++)
        for (int j = 0; j < out.size(); j++)
            if (ma - Q[i][j].size() <= 3) return Q[i][j];
    return QList<Pair>();
}

Problem::CompilerInfo Problem::getCompiler(const QString& playerName)
{
    for (auto i : compilers)
        if (QFile(Global::g_contest.src_path + playerName + "/" + dir + "/" + i.file).exists()) return i;
    return CompilerInfo();
}

void Problem::Configure(const QString& typ, double timeLim, double memLim, const QString& check)
{
    if (typ.size())
    {
        if (typ == "传统型") type = ProblemType::Traditional;
        else if (typ == "提交答案型") type = ProblemType::AnswersOnly;
    }
    if (check.size()) checker = addSuff(check == "全文比较" ? "fulltext" : check);
    exe = addSuff(exe);

    for (auto& i : que)
    {
        if (timeLim >= 0) i.timeLim = timeLim;
        if (memLim >= 0) i.memLim = memLim;
    }
}

void Problem::ConfigureNew(const QString& typ, double timeLim, double memLim, const QString& check)
{
    if (typ.size())
    {
        if (typ == "传统型") type = ProblemType::Traditional;
        else if (typ == "提交答案型") type = ProblemType::AnswersOnly;
    }
    if (check.size()) checker = addSuff(check == "全文比较" ? "fulltext" : check);
    exe = addSuff(exe);

    QList<QPair<QString, QString>> list = getInAndOutFile();
    //qDebug()<<list;
    sort(list.begin(), list.end(), [&](const Pair & a, const Pair & b)
    {
        QCollator c;
        c.setNumericMode(true);
        return c.compare(a.first, b.first) == -1;
    });
    int num = list.size(), sum = sumScore;
    QList<int> score;
    for (int i = 0; i < num; i++) score.append(sumScore / num), sum -= score[i];
    for (int i = num - 1; sum && i >= 0; i--) score[i]++, sum--;

    if (type == ProblemType::Traditional)
    {
        compilers.push_back(CompilerInfo(QString("gcc -o %1 %1.c -lm -static").arg(name), QString("%1.c").arg(name)));
        compilers.push_back(CompilerInfo(QString("g++ -o %1 %1.cpp -lm -static").arg(name), QString("%1.cpp").arg(name)));
        compilers.push_back(CompilerInfo(QString("fpc %1.pas").arg(name), QString("%1.pas").arg(name)));
    }

    for (int i = 0; i < num; i++)
    {
        Subtask sub(score[i]);
        Info x(timeLim, memLim);
        x.in = list[i].first;
        x.out = list[i].second;
        if (type == ProblemType::AnswersOnly) x.sub = QString("%1%2.out").arg(name).arg(i + 1);
        sub.point.push_back(que.size());
        que.push_back(x);
        tasks.push_back(sub);
    }
}
