#ifndef PROBLEM_H
#define PROBLEM_H

#include "const.h"

#include <map>

class Compiler
{
public:
    explicit Compiler(QString cmd, QString file, int time = 10) :
        cmd(cmd), file(file), time_lim(time) {}

    // Getter member functions
    QString Cmd() const { return cmd; }
    QString SourceFile() const { return file; }
    int TimeLimit() const { return time_lim; }

private:
    const QString cmd, file;
    const int time_lim;
};



class TestCase
{
public:
    explicit TestCase(int id, double t, double m, const QString& in, const QString& out, const QString& sub = "") :
        id(id), time_lim(t), mem_lim(m), in(in), out(out), sub(sub) {}

    // Getter member functions
    int Id() const { return id; }
    double TimeLimit() const { return time_lim; }
    double MemoryLimit() const { return mem_lim; }
    QString InFile() const { return in; }
    QString OutFile() const { return out; }
    QString SubmitFile() const { return sub; }

    // Setter member functions
    void SetTimeLimit(double lim) { time_lim = lim; }
    void SetMemoryLimit(double lim) { mem_lim = lim; }
    void SetInFile(const QString& _in) { in = _in; }
    void SetOutFile(const QString& _out) { out = _out; }
    void SetSubmitFile(const QString& _sub) { sub = _sub; }

private:
    const int id;
    double time_lim, mem_lim;
    QString in, out, sub;
    QString note, state;
};



struct Subtask
{
public:
    explicit Subtask(int s) : score(s) {}

    // Getter member functions
    int Score() const { return score; }
    int Size() const { return cases.size(); }

    /// 添加测试点
    void append(TestCase* point) { cases.push_back(point); }

    // Iterators
    std::vector<TestCase*>::const_iterator begin() const { return cases.begin(); }
    std::vector<TestCase*>::const_iterator end() const { return cases.end(); }

private:
    const int score;
    std::vector<TestCase*> cases;
};



class Problem
{
public:
    explicit Problem(const QString& name = "");
    ~Problem() { Clear(); }

    /// 内置校验器列表，格式： <file, <name, toolTip>>
    static const std::map<QString, std::pair<QString, QString>> INTERNAL_CHECKER_MAP;

    /// 增加扩展名
    static QString AddFileExtension(QString file)
    {
#ifdef Q_OS_WIN
        if (!file.endsWith(".exe")) file += ".exe";
#endif
        return file;
    }

    /// 去除扩展名
    static QString RemoveFileExtension(QString file)
    {
#ifdef Q_OS_WIN
        if (file.endsWith(".exe")) file.remove(file.length() - 4, 4);
#endif
        return file;
    }

    /// 尝试从内置校验器名称获取校验器文件名，不是内置校验器返回原始文件名
    static QString FromInternalCheckerName(const QString& name);

    // Getter member functions
    QString Name() const { return name; }
    QString Directory() const { return dir; }
    QString ExecutableFile() const { return exe; }
    QString Checker() const { return checker; }
    QString InFile() const { return in_file; }
    QString OutFile() const { return out_file; }
    int Score() const { return score; }
    int CheckerTimeLimit() const { return checker_time_lim; }
    double CodeLengthLimit() const { return code_len_lim; }
    Global::ProblemType Type() const { return type; }

    // Vector traversal interface
    int TestCaseCount() const { return cases.size(); }
    int SubtaskCount() const { return subtasks.size(); }
    int CompilerCount() const { return compilers.size(); }
    TestCase* TestCaseAt(int t) const { return cases[t]; }
    Subtask* SubtaskAt(int t) const { return subtasks[t]; }
    Compiler* CompilerAt(int t) const { return compilers[t]; }

    /// 清空
    void Clear();

    /// 读入配置文件
    void ReadConfiguration();

    /// 保存配置文件
    bool SaveConfiguration();

    /// 配置试题，只修改时间限制 、内存限制、校验器等
    void Configure(const QString& typ, double timeLim, double memLim, const QString& check);

    /// 清空原来的配置，重新配置试题
    void ConfigureNew(const QString& typ, double timeLim, double memLim, const QString& check);

    /// 根据选手目录下的源文件自动选择编译器
    Compiler* GetCompiler(const QString& playerName);

    /// 自动获取输入输出文件对
    QList<QPair<QString, QString>> GetInAndOutFile();

    /// 尝试获取内置校验器名称，不是内置校验器返回原始文件名
    QString InternalCheckerName() const
    {
        auto p = INTERNAL_CHECKER_MAP.find(RemoveFileExtension(checker));
        if (p != INTERNAL_CHECKER_MAP.end()) return p->second.first; else return checker;
    }

    /// InOutString，格式如下
    QString GetInOutString(TestCase *point) const
    {
        if (!point) return "";
        QString s = QString("标准输入:\"%1\" 标准输出:\"%2\"").arg(point->InFile()).arg(point->OutFile());
        if (type == Global::AnswersOnly) s += QString(" 选手提交:\"%1\"").arg(point->SubmitFile());
        return s;
    }

private:
    QString name, dir, exe, checker, in_file, out_file;
    int score, checker_time_lim;
    double code_len_lim;
    Global::ProblemType type;

    std::vector<TestCase*> cases;
    std::vector<Subtask*> subtasks;
    std::vector<Compiler*> compilers;
};

#endif // PROBLEM_H
