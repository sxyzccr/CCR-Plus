#include "global.h"
#include "problem.h"

#include <QCollator>
#include <QTextStream>
#include <QDomDocument>

using namespace std;

const map<QString, pair<QString, QString>> Problem::INTERNAL_CHECKER_MAP =
{
    {"fulltext", make_pair("全文比较", "全文比较(过滤行末空格及文末回车)")}
};

QString Problem::FromInternalCheckerName(const QString& name)
{
    for (auto i : INTERNAL_CHECKER_MAP)
        if (name == i.second.first) return AddFileExtension(i.first);
    return AddFileExtension(name);
}



Problem::Problem(const QString& name) :
    type(Global::OtherProblemType),
    name(name), dir(name), exe(AddFileExtension(name)), checker(),
    in_file(name + ".in"), out_file(name + ".out"),
    score(100), checker_time_lim(10), code_len_lim(100)
{

}

Problem::Problem(Problem *problem) :
    type(problem->type),
    name(problem->name), dir(problem->dir), exe(problem->exe), checker(problem->checker),
    in_file(problem->in_file), out_file(problem->out_file),
    score(problem->score), checker_time_lim(problem->checker_time_lim), code_len_lim(problem->code_len_lim)
{
    for (auto i : problem->cases) cases.push_back(new TestCase(*i));
    for (auto i : problem->compilers) compilers.push_back(new Compiler(*i));
    int t = 0;
    for (auto i : problem->subtasks)
    {
        Subtask* sub = new Subtask(i->Score());
        for (int j = 0; j < i->Size(); j++) sub->Append(cases[t++]);
        subtasks.push_back(sub);
    }
}

void Problem::Clear()
{
    for (auto i : cases) delete i;
    for (auto i : subtasks) delete i;
    for (auto i : compilers) delete i;
    cases.clear();
    subtasks.clear();
    compilers.clear();
}

void Problem::ReadConfiguration()
{
    score = 0;
    QFile file(Global::g_contest.data_path + name + "/.prb");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QDomDocument doc;
    if (!doc.setContent(&file)) { file.close(); return; }

    QDomElement root = doc.documentElement();
    if (root.isNull() || root.tagName() != "problem") { file.close(); return; }
    QString s = root.attribute("type");
    if (s == "TRA" || s == "TRA_0_4") type = Global::Traditional;
    else if (s == "ANS" || s == "ANS_0_4") type = Global::AnswersOnly;
    else if (s == "INT" || s == "INT_0_4") type = Global::Interactive;

    QDomNodeList list = root.childNodes();
    for (int i = 0; i < list.count(); i++)
    {
        QDomElement a = list.item(i).toElement();
        if (a.tagName() == "source")
        {
            if (a.hasAttribute("dir")) dir = a.attribute("dir");
            if (a.hasAttribute("file")) exe = AddFileExtension(a.attribute("file"));
            if (a.hasAttribute("code")) code_len_lim = a.attribute("code").toDouble();

            QDomNodeList l = a.childNodes();
            for (int j = 0; j < l.count(); j++)
            {
                QDomElement b = l.item(j).toElement();
                if (b.tagName() == "language")
                {
                    Compiler* x;
                    if (b.hasAttribute("time"))
                        x =  new Compiler(b.attribute("cmd"), b.attribute("file"), b.attribute("time").toInt());
                    else
                        x =  new Compiler(b.attribute("cmd"), b.attribute("file"));
                    //if (x.file.endsWith(".cpp")||x.file.endsWith(".c")) x.cmd+=" -static";
                    this->compilers.push_back(x);
                }
            }
        }
        else if (a.tagName() == "task")
        {
            if (a.hasAttribute("input"))  in_file  = a.attribute("input");
            if (a.hasAttribute("output")) out_file = a.attribute("output");
            checker = FromInternalCheckerName(a.attribute("checker"));
            if (a.hasAttribute("time")) checker_time_lim = a.attribute("time").toInt();

            QDomNodeList l = a.childNodes();
            for (int j = 0; j < l.count(); j++)
            {
                QDomElement b = l.item(j).toElement();
                if (b.tagName() == "subtask")
                {
                    Subtask* sub = new Subtask(b.attribute("score").toInt());
                    QDomNodeList ll = b.childNodes();
                    for (int k = 0; k < ll.count(); k++)
                    {
                        QDomElement c = ll.item(k).toElement();
                        if (c.tagName() == "point")
                        {
                            TestCase* x = new TestCase(c.attribute("time").toDouble(), c.attribute("mem").toDouble(),
                                                       c.attribute("in"), c.attribute("out"), c.attribute("sub"));
                            sub->Append(x);
                            this->cases.push_back(x);
                        }
                    }
                    score += sub->Score();
                    this->subtasks.push_back(sub);
                }
            }
        }
    }
    file.close();
}

bool Problem::SaveConfiguration()
{
    QDomDocument doc;
    QDomProcessingInstruction xml = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xml);
    QDomElement root = doc.createElement("problem");
    root.setAttribute("type", type == Global::Traditional ? "TRA_0_4" : type == Global::AnswersOnly ? "ANS_0_4" : "UNKNOWN");
    root.setAttribute("maker", "ccr-plus");
    doc.appendChild(root);

    QDomElement source = doc.createElement("source");
    source.setAttribute("dir", dir);
    root.appendChild(source);
    if (type == Global::Traditional)
    {
        source.setAttribute("file", RemoveFileExtension(exe));
        source.setAttribute("code", code_len_lim);
        for (auto i : compilers)
        {
            QDomElement lang = doc.createElement("language");
            lang.setAttribute("cmd", i->Cmd());
            lang.setAttribute("file", i->SourceFile());
            lang.setAttribute("time", i->TimeLimit());
            source.appendChild(lang);
        }
    }

    QDomElement task = doc.createElement("task");
    if (type == Global::Traditional)
    {
        task.setAttribute("input", in_file);
        task.setAttribute("output", out_file);
    }
    task.setAttribute("checker", RemoveFileExtension(checker));
    task.setAttribute("time", checker_time_lim);
    root.appendChild(task);
    for (auto i : subtasks)
    {
        QDomElement subtask = doc.createElement("subtask");
        subtask.setAttribute("score", i->Score());
        task.appendChild(subtask);
        for (auto j : *i)
        {
            QDomElement point = doc.createElement("point");
            point.setAttribute("in", j->InFile());
            point.setAttribute("out", j->OutFile());
            if (type == Global::AnswersOnly) point.setAttribute("sub", j->SubmitFile());
            if (type == Global::Traditional)
            {
                point.setAttribute("time", j->TimeLimit());
                point.setAttribute("mem", j->MemoryLimit());
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

void Problem::Configure(const QString& typ, double timeLim, double memLim, const QString& check)
{
    if (typ == "传统型")
    {
        type = Global::Traditional;
        if (!compilers.size())
        {
            compilers = { new Compiler(QString("gcc -o %1 %1.c -lm -static").arg(name), QString("%1.c").arg(name)),
                          new Compiler(QString("g++ -o %1 %1.cpp -lm -static").arg(name), QString("%1.cpp").arg(name)),
                          new Compiler(QString("fpc %1.pas").arg(name), QString("%1.pas").arg(name))
                        };
        }
    }
    else if (typ == "提交答案型")
    {
        type = Global::AnswersOnly;
        compilers.clear();
    }

    if (!check.isEmpty()) checker = FromInternalCheckerName(check);
    exe = AddFileExtension(exe);

    for (auto i : cases)
    {
        if (timeLim >= 0) i->SetTimeLimit(timeLim);
        if (memLim >= 0) i->SetMemoryLimit(memLim);
        if (typ == "提交答案型" && i->SubmitFile().isEmpty()) i->SetSubmitFile(i->OutFile());
    }
}

void Problem::ConfigureNew(const QString& typ, double timeLim, double memLim, const QString& check)
{
    Clear();

    if (typ == "传统型") type = Global::Traditional;
    else if (typ == "提交答案型") type = Global::AnswersOnly;

    if (!check.isEmpty()) checker = FromInternalCheckerName(check);
    exe = AddFileExtension(exe);

    QList<QPair<QString, QString>> list = GetInAndOutFile();
    //qDebug()<<list;

    int num = list.size(), sum = score;
    QList<int> scores;
    for (int i = 0; i < num; i++) scores.append(score / num), sum -= scores[i];
    for (int i = num - 1; sum && i >= 0; i--) scores[i]++, sum--;

    if (type == Global::Traditional)
    {
        compilers = { new Compiler(QString("gcc -o %1 %1.c -lm -static").arg(name), QString("%1.c").arg(name)),
                      new Compiler(QString("g++ -o %1 %1.cpp -lm -static").arg(name), QString("%1.cpp").arg(name)),
                      new Compiler(QString("fpc %1.pas").arg(name), QString("%1.pas").arg(name))
                    };
    }

    for (int i = 0; i < num; i++)
    {
        Subtask* sub = new Subtask(scores[i]);
        TestCase* point = new TestCase(timeLim, memLim, list[i].first, list[i].second);
        if (type == Global::AnswersOnly) point->SetSubmitFile(point->OutFile());
        sub->Append(point);
        this->cases.push_back(point);
        this->subtasks.push_back(sub);
    }
}

Compiler* Problem::GetCompiler(const QString& playerName)
{
    for (auto i : compilers)
        if (QFile(Global::g_contest.src_path + playerName + "/" + dir + "/" + i->SourceFile()).exists()) return i;
    return nullptr;
}

QList<QPair<QString, QString>> Problem::GetInAndOutFile()
{
    typedef QPair<QString, QString> InOutPair;

    QString dir = Global::g_contest.data_path + name;
    QStringList list = QDir(dir).entryList(QDir::Files);
    const QStringList in  = {".in", ".inp", "in", "inp"},
                      out = {".out", ".ans", ".ou", ".an", ".sol", ".res", ".std", "out", "ans", "ou", "an", "sol", "res", "std"};
    QList<InOutPair> Q[in.size()][out.size()], res;
    int ma = 0;

    for (int i = 0; i < in.size(); i++)
        for (int j = 0; j < out.size(); j++)
        {
            QString a = in[i], b = out[j];
            QMap<InOutPair, int> F;
            for (auto s : list)
            {
                int p = s.indexOf(a), q = s.indexOf(b);
                if (p != -1) F[qMakePair(s.left(p), s.right(s.length() - a.length() - p))]++;
                if (q != -1) F[qMakePair(s.left(q), s.right(s.length() - b.length() - q))]++;
            }
            for (auto k = F.constBegin(); k != F.constEnd(); k++)
                if (k.value() == 2) Q[i][j].append(qMakePair(k.key().first + a + k.key().second, k.key().first + b + k.key().second));
            ma = max(ma, Q[i][j].size());
        }
    for (int i = 0; i < in.size() && !res.size(); i++)
        for (int j = 0; j < out.size() && !res.size(); j++)
            if (ma - Q[i][j].size() <= 3) { res = Q[i][j]; break; }

    sort(res.begin(), res.end(), [&](const InOutPair& a, const InOutPair& b)
    {
        QCollator c;
        c.setNumericMode(true);
        return c.compare(a.first, b.first) == -1;
    });
    return res;
}
