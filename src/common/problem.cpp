#include <QMap>
#include <QDir>
#include <QFile>
#include <QCollator>
#include <QTextStream>
#include <QDomDocument>

#include <algorithm>

#include "common/global.h"
#include "common/problem.h"
#include "common/version.h"

const QStringList Compiler::BUILTIN_COMPILER_CMD =
{
    "gcc -o %2 %1.c -lm -static",
    "g++ -o %2 %1.cpp -lm -static",
    "fpc %1.pas",
};

const QStringList Compiler::BUILTIN_COMPILER_FILE =
{
    "%1.c",
    "%1.cpp",
    "%1.pas",
};



const QMap<QString, QPair<QString, QString>> Problem::BUILTIN_CHECKER_MAP =
{
    {"fulltext", qMakePair(QString("全文比较"), QString("全文比较(过滤行末空格及文末回车)"))}
};

const QRegularExpression Problem::name_reg_exp("[\\\\/:*?\"<>|]");

QString Problem::CheckFileNameValid(const QString& name)
{
    if (name.trimmed().isEmpty()) return "%1不能为空。";
    else if (name.contains(name_reg_exp)) return "%1不能包含下列非法字符：\\/:*?\"<>|";
    else return "";
}

QString Problem::FromBuiltinCheckerName(const QString& name)
{
    for (auto i = BUILTIN_CHECKER_MAP.begin(); i != BUILTIN_CHECKER_MAP.end(); i++)
        if (name == i.value().first) return AddFileExtension(i.key());
    return AddFileExtension(name);
}

bool Problem::IsBuiltinChecker(const QString& checker)
{
    return BUILTIN_CHECKER_MAP.find(RemoveFileExtension(checker)) != BUILTIN_CHECKER_MAP.end();
}



Problem::Problem(const QString& name) :
    type(Global::OtherProblemType),
    name(name), dir(name), exe(AddFileExtension(name)), checker(),
    in_file(name + ".in"), out_file(name + ".out"),
    score(100), checker_time_lim(10), code_len_lim(100)
{

}

Problem::Problem(const Problem& problem) :
    type(problem.type),
    name(problem.name), dir(problem.dir), exe(problem.exe), checker(problem.checker),
    in_file(problem.in_file), out_file(problem.out_file),
    score(problem.score), checker_time_lim(problem.checker_time_lim), code_len_lim(problem.code_len_lim)
{
    for (auto i : problem.cases) cases.append(new TestCase(*i));
    for (auto i : problem.compilers) compilers.append(new Compiler(*i));
    int t = 0;
    for (auto i : problem.subtasks)
    {
        Subtask* sub = new Subtask(i->Score());
        for (int j = 0; j < i->Size(); j++) sub->Append(cases[t++]);
        subtasks.append(sub);
    }
}

void Problem::ClearCompilers()
{
    for (auto i : compilers) delete i;
    compilers.clear();
}

void Problem::ClearTestCases()
{
    for (auto i : cases) delete i;
    for (auto i : subtasks) delete i;
    cases.clear();
    subtasks.clear();
}

void Problem::Clear()
{
    ClearCompilers();
    ClearTestCases();
}

bool Problem::isValid()
{
    if (!CheckFileNameValid(dir).isEmpty()) return false;
    if (type == Global::AnswersOnly) return true;

    return CheckFileNameValid(exe).isEmpty() && !exe.contains(' ') && !exe.contains('\t') &&
           CheckFileNameValid(in_file).isEmpty() && CheckFileNameValid(out_file).isEmpty();
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
                    this->compilers.append(x);
                }
            }
        }
        else if (a.tagName() == "task")
        {
            if (a.hasAttribute("input"))  in_file  = a.attribute("input");
            if (a.hasAttribute("output")) out_file = a.attribute("output");
            checker = FromBuiltinCheckerName(a.attribute("checker"));
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
                            this->cases.append(x);
                        }
                    }
                    score += sub->Score();
                    this->subtasks.append(sub);
                }
            }
        }
    }
    file.close();
}

bool Problem::SaveConfiguration() const
{
    QDomDocument doc;
    QDomProcessingInstruction xml = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xml);
    QDomElement root = doc.createElement("problem");
    root.setAttribute("type", type == Global::Traditional ? "TRA_0_4" : type == Global::AnswersOnly ? "ANS_0_4" : "UNKNOWN");
    root.setAttribute("maker", "ccr-plus");
    root.setAttribute("version", VERSION_SHORTER);
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

void Problem::ResetCompilers()
{
    ClearCompilers();

    if (type == Global::Traditional)
    {
        for (int i = 0; i < Compiler::BUILTIN_COMPILER_COUNT; i++)
        {
            Compiler* compiler;
            if (Compiler::BUILTIN_COMPILER_FILE[i] == "%1.pas") // Pascal 特判
            {
                if (RemoveFileExtension(exe) == name)
                    compiler = new Compiler(Compiler::BUILTIN_COMPILER_CMD[i].arg(name),
                                            Compiler::BUILTIN_COMPILER_FILE[i].arg(name));
                else
                    compiler = new Compiler(QString("fpc %1.pas -o'%2'").arg(name).arg(RemoveFileExtension(exe)),
                                            Compiler::BUILTIN_COMPILER_FILE[i].arg(name));
            }
            else
            {
                compiler = new Compiler(Compiler::BUILTIN_COMPILER_CMD[i].arg(name).arg(RemoveFileExtension(exe)),
                                        Compiler::BUILTIN_COMPILER_FILE[i].arg(name));
            }
            compilers.append(compiler);
        }
    }
}

void Problem::ResetTestCases(double timeLim, double memLim)
{
    ClearTestCases();

    QList<QPair<QString, QString>> list = getInAndOutFile();

    int num = list.size(), sum = score = 100;
    QList<int> scores;
    for (int i = 0; i < num; i++) scores.append(score / num), sum -= scores[i];
    for (int i = num - 1; sum && i >= 0; i--) scores[i]++, sum--;

    for (int i = 0; i < num; i++)
    {
        Subtask* sub = new Subtask(scores[i]);
        TestCase* point = new TestCase(timeLim, memLim, list[i].first, list[i].second);
        if (type == Global::AnswersOnly) point->SetSubmitFile(point->OutFile());
        sub->Append(point);
        this->cases.append(point);
        this->subtasks.append(sub);
    }
}

void Problem::Configure(Global::ProblemType type, double timeLim, double memLim, const QString& check)
{
    if (type == Global::Traditional)
    {
        this->type = type;
        if (!compilers.size()) ResetCompilers();
    }
    else if (type == Global::AnswersOnly)
    {
        this->type = type;
        ClearCompilers();
    }

    exe = AddFileExtension(exe);
    if (!check.isEmpty()) checker = FromBuiltinCheckerName(check);

    for (auto i : cases)
    {
        if (timeLim >= 0) i->SetTimeLimit(timeLim);
        if (memLim >= 0) i->SetMemoryLimit(memLim);
        if (type == Global::AnswersOnly && i->SubmitFile().isEmpty()) i->SetSubmitFile(i->OutFile());
    }
}

void Problem::ConfigureNew(Global::ProblemType type, double timeLim, double memLim, const QString& check)
{
    if (type == Global::Traditional || type == Global::AnswersOnly) this->type = type;

    dir = name;
    exe = AddFileExtension(exe);
    in_file = name + ".in";
    out_file = name + ".out";
    code_len_lim = 100;
    checker_time_lim = 10;
    if (!check.isEmpty()) checker = FromBuiltinCheckerName(check);

    ResetCompilers();
    ResetTestCases(timeLim, memLim);
}

void Problem::ChangeProblemType(Global::ProblemType type)
{
    code_len_lim = 100;

    switch (type)
    {
    case Global::Traditional:
        this->type = type;

        exe = AddFileExtension(name);
        in_file = name + ".in";
        out_file = name + ".out";

        ResetCompilers();
        for (auto point : cases)
        {
            point->SetTimeLimit(1);
            point->SetMemoryLimit(128);
            point->SetSubmitFile("");
        }
        break;

    case Global::AnswersOnly:
        this->type = type;

        ClearCompilers();

        for (auto point : cases)
        {
            point->SetTimeLimit(0);
            point->SetMemoryLimit(0);
            point->SetSubmitFile(point->OutFile());
        }
        break;

    default:
        break;
    }
}

const Compiler* Problem::GetCompiler(const QString& playerName) const
{
    for (auto i : compilers)
        if (QFile(Global::g_contest.src_path + playerName + "/" + dir + "/" + i->SourceFile()).exists()) return i;
    return nullptr;
}

QString Problem::BuiltinCheckerName() const
{
    auto p = BUILTIN_CHECKER_MAP.find(RemoveFileExtension(checker));
    if (p != BUILTIN_CHECKER_MAP.end()) return p.value().first; else return checker;
}



QList<QPair<QString, QString>> Problem::getInAndOutFile()
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
            ma = std::max((qsizetype)ma, Q[i][j].size());
        }
    for (int i = 0; i < in.size() && !res.size(); i++)
        for (int j = 0; j < out.size() && !res.size(); j++)
            if (ma - Q[i][j].size() <= 3) { res = Q[i][j]; break; }

    std::sort(res.begin(), res.end(), [&](const InOutPair& a, const InOutPair& b)
    {
        QCollator c;
        c.setNumericMode(true);
        return c.compare(a.first, b.first) == -1;
    });
    return res;
}
