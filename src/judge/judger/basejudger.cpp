#include <QSet>
#include <QThread>
#include <QTextStream>
#include <QDomDocument>
#include <QElapsedTimer>
#include <QCoreApplication>

#include "judge/judger/basejudger.h"

BaseJudger::BaseJudger(const QString& testDir, Player* player, const Problem* problem, QObject *parent) : QObject(parent),
    player(player), problem(problem),
    test_dir(testDir), tmp_dir(testDir + ".tmp/"),
    data_dir(testDir + "data/" + problem->Name() + "/"),
    src_dir(testDir + "src/" + player->Name() + "/" + problem->Directory() + "/")
{

}

BaseJudger::~BaseJudger()
{
    waitForCleanTmpDirectory(2000);
}

bool BaseJudger::waitForMakeTmpDirectory(int ms) const
{
    QDir dir(test_dir), tmp(tmp_dir);
    QElapsedTimer timer;
    for (timer.start(); !tmp.exists() && timer.elapsed() <= ms;)
    {
        if (dir.mkdir(".tmp")) return true;
        if (Global::g_is_judge_stoped) break;
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }
    return tmp.exists();
}

bool BaseJudger::waitForCleanTmpDirectory(int ms) const
{
    QDir tmp(tmp_dir);
    QElapsedTimer timer;
    for (timer.start(); tmp.exists() && timer.elapsed() <= ms;)
    {
        if (tmp.removeRecursively()) return true;
        if (Global::g_is_judge_stoped) break;
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }
    return !tmp.exists();
}

bool BaseJudger::waitForMakeWorkingDirectory(int num, int ms)
{
    QDir dir(tmp_dir);
    QString sub(QString("judge_thread_%1_%2_%3").arg(player->Name()).arg(problem->Name()).arg(num));
    working_dir = tmp_dir + sub + "/";
    QDir workingDir(working_dir);
    QElapsedTimer timer;
    for (timer.start(); !workingDir.exists() && timer.elapsed() <= ms;)
    {
        if (dir.mkdir(sub)) return true;
        if (Global::g_is_judge_stoped) break;
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }
    return workingDir.exists();
}

bool BaseJudger::waitForCleanWorkingDirectory(int num, int ms) const
{
    QDir dir(tmp_dir + QString("judge_thread_%1_%2_%3").arg(player->Name()).arg(problem->Name()).arg(num));
    QElapsedTimer timer;
    timer.start();
    for (; dir.exists() && timer.elapsed() <= ms;)
    {
        if (dir.removeRecursively()) return true;
        if (Global::g_is_judge_stoped) break;
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }
    return !dir.exists();
}

bool BaseJudger::monitorProcess(QProcess* process, int ms) const
{
    QElapsedTimer timer;
    timer.start();
    for (; timer.elapsed() <= ms;)
    {
        if (process->state() != QProcess::Running) return true;
        if (Global::g_is_judge_stoped)
        {
            process->kill();
            return false;
        }
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }
    process->kill();
    return false;
}

TestCaseResult BaseJudger::judgeTestCaseEvent(const TestCase* point, int testNum)
{
    if (Global::g_is_judge_stoped) return TestCaseResult();

    TestCaseResult res;
    if (!waitForMakeWorkingDirectory(testNum, 2000))
        res = TestCaseResult(0, 0, 'E', "无法创建临时文件");
    else
        res = judgeTestCase(point);

    static QSet<int> not_clean;
    if (!waitForCleanWorkingDirectory(testNum, 2000)) not_clean.insert(testNum);
    for (auto i : not_clean)
        if (waitForCleanWorkingDirectory(i, 2000)) not_clean.remove(i);

    return res;
}

TestCaseResult BaseJudger::checkOutput(const QString& inFile, const QString& ansFile, const QString& outFile) const
{
    if (!QFile(ansFile).exists()) return TestCaseResult(0, 0, 'E', "找不到标准输出文件");
    if (!QFile(outFile).exists()) return TestCaseResult(0, 0, 'O', "程序无输出");

    QProcess process;
    process.setWorkingDirectory(working_dir);
    QString checkerDir;
    if (QFile(QDir().currentPath() + "/checker/" + problem->Checker()).exists())
        checkerDir = QDir().currentPath() + "/checker/";
    else
        checkerDir = data_dir;
#ifdef Q_OS_LINUX
    QProcess::execute(QString("chmod +wx \"%1\"").arg(checkerDir + problem->Checker()));
#endif
    process.start(checkerDir + problem->Checker(), QStringList({inFile, ansFile, outFile, ".result"}));

    if (!process.waitForStarted(-1))
        return TestCaseResult(0, 0, 'E', "无效的校验器");

    bool ok = monitorProcess(&process, problem->CheckerTimeLimit() * 1000);
    bool finished = process.waitForFinished(2000);

    if (!finished && process.state() == QProcess::Running)
        return TestCaseResult(0, 0, 'E', "无法结束校验器进程");
    else if (Global::g_is_judge_stoped)
        return TestCaseResult();
    else if (!ok)
        return TestCaseResult(0, 0, 'E', "校验器超时或崩溃");
    else if (process.exitCode())
        return TestCaseResult(0, 0, 'E', "校验器崩溃");

    QFile file(working_dir + ".result");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return TestCaseResult(0, 0, 'E', "找不到校验器输出");

    QTextStream in(&file);
    QString note = in.readLine().trimmed();
    double score = note.toDouble(&ok);
    if (!ok || score < 0 || score > 1)
    {
        file.close();
        if (note.length() >= 13) note = note.left(11) + "…";
        return TestCaseResult(0, 0, 'E', QString("校验器输出格式错误 第一行: \"%1\"").arg(note));
    }
    note = in.readLine().trimmed();
    file.close();

    return TestCaseResult(score, 0, score == 0 ? 'W' : score == 1.0 ? 'A' : 'P', note);
}

Global::CompileResult BaseJudger::compile(const Compiler* compiler, QString& resultNote) const
{
    QFile file(src_dir + compiler->SourceFile());
    if (!file.copy(tmp_dir + compiler->SourceFile()))
        return resultNote = "选手源代码拷贝失败", Global::OtherCompileError;

    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.setWorkingDirectory(tmp_dir);
    process.start(compiler->Cmd());
    if (!process.waitForStarted(-1))
        return resultNote = "无效的编译器", Global::InvalidCompiler;

    bool ok = monitorProcess(&process, compiler->TimeLimit() * 1000);
    bool finished = process.waitForFinished(2000);

    if (!finished && process.state() == QProcess::Running)
        return resultNote = "无法结束编译器进程", Global::OtherCompileError;
    else if (Global::g_is_judge_stoped)
        return Global::CompileKilled;
    else if (!ok)
        return resultNote = "编译超时", Global::CompileTimeLimitExceeded;
    else if (process.exitCode())
    {
        resultNote = "编译错误:\n" + QString::fromLocal8Bit(process.readAllStandardOutput());
        if (resultNote.length() > 51200) resultNote = "编译错误过多";
        return Global::CompileError;
    }
    else
        return Global::CompileSuccessfully;
}

#ifdef Q_OS_WIN

#include <windows.h>
#include <psapi.h>

/// 进程结束后的清理
inline void onProcessFinished(const PROCESS_INFORMATION& pi)
{
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}

TestCaseResult BaseJudger::runProgram(const QString& exe, double timeLim, double memLim) const
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    PROCESS_MEMORY_COUNTERS pmc;
    FILETIME ct, et, kt, ut;
    SYSTEMTIME _kt, _ut;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    if (!CreateProcess(0, (LPTSTR)(working_dir + exe).utf16(), 0, 0, FALSE, CREATE_NO_WINDOW, 0, (LPCTSTR)working_dir.utf16(), &si, &pi))
        return TestCaseResult(0, 0, 'E', QString("无法运行程序: %1").arg((unsigned int)GetLastError()));

    QElapsedTimer timer;
    timer.start();
    for (;;)
    {
        if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0) break;
        if (Global::g_is_judge_stoped)
        {
            TerminateProcess(pi.hProcess, 0);
            onProcessFinished(pi);
            return TestCaseResult();
        }

        GetProcessMemoryInfo(pi.hProcess, &pmc, sizeof(pmc));
        if (pmc.PeakPagefileUsage > memLim * (1<<20))
        {
            TerminateProcess(pi.hProcess, 0);
            onProcessFinished(pi);
            return TestCaseResult(0, 0, 'M', "超过内存限制");
        }

        GetProcessTimes(pi.hProcess, &ct, &et, &kt, &ut);
        FileTimeToSystemTime(&kt, &_kt);
        FileTimeToSystemTime(&ut, &_ut);
        double usedTime = _ut.wHour * 3600 + _ut.wMinute * 60 + _ut.wSecond + _ut.wMilliseconds / 1000.0;
        double kernelTime = _kt.wHour * 3600 + _kt.wMinute * 60 + _kt.wSecond + _kt.wMilliseconds / 1000.0;
        double blockTime = timer.elapsed() / 1000.0 - usedTime - kernelTime;

        if (usedTime > timeLim || kernelTime > timeLim || blockTime > 1.5)
        {
            TerminateProcess(pi.hProcess, 0);
            onProcessFinished(pi);
            if (usedTime > timeLim)
                return TestCaseResult(0, timeLim, 'T', "超过时间限制");
            else if (kernelTime > timeLim)
                return TestCaseResult(0, timeLim, 'T', "系统 CPU 时间过长");
            else
                return TestCaseResult(0, timeLim, 'T', "进程被阻塞");
        }

        QCoreApplication::processEvents();
        QThread::msleep(10);
    }

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    if (exitCode && exitCode != STILL_ACTIVE)
    {
        onProcessFinished(pi);
        return TestCaseResult(0, 0, 'R', QString("运行时错误: %1").arg((unsigned int)exitCode));
    }

    GetProcessMemoryInfo(pi.hProcess, &pmc, sizeof(pmc));
    double usedMemory = pmc.PeakPagefileUsage / 1024.0 / 1024.0;

    GetProcessTimes(pi.hProcess, &ct, &et, &kt, &ut);
    FileTimeToSystemTime(&ut, &_ut);
    double usedTime = _ut.wHour * 3600 + _ut.wMinute * 60 + _ut.wSecond + _ut.wMilliseconds / 1000.0;

    onProcessFinished(pi);

    if (usedMemory > memLim)
        return TestCaseResult(0, 0, 'M', "超过内存限制");
    if (usedTime > timeLim)
        return TestCaseResult(0, timeLim, 'T', "超过时间限制");

    return TestCaseResult(1.0, usedTime, ' ', QString("时间: %1s 内存: %2MB").arg(usedTime, 0, 'f', 2)
                                                                             .arg(usedMemory, 0, 'f', 2));
}

#else

TestCaseResult BaseJudger::runProgram(const QString& exe, double timeLim, double memLim) const
{
    QProcess::execute(QString("chmod +wx \"%1\"").arg(QDir().currentPath() + "/monitor"));

    QProcess process;
    process.setWorkingDirectory(working_dir);
    process.start(QDir().currentPath() + "/monitor", QStringList({ exe,
                                                                   QString::number(timeLim),
                                                                   QString::number(memLim)
                                                                 }));
    if (!process.waitForStarted(-1))
        return TestCaseResult(0, 0, 'E', "无法运行进程监视器");

    bool ok = monitorProcess(&process, std::max(timeLim * 2000 + 200, 10000.0));
    bool finished = process.waitForFinished(2000);
    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());

    if (!finished && process.state() == QProcess::Running)
        return TestCaseResult(0, 0, 'E', "无法结束进程监视器");
    else if (Global::g_is_judge_stoped)
        return TestCaseResult();
    else if (!ok)
        return TestCaseResult(0, timeLim, 'T', "运行时间过长");
    else if (process.exitCode())
    {
        char state = ' ';
        switch (process.exitCode())
        {
        case 1:
            state = 'E'; break;
        case 2:
            state = 'R'; break;
        case 3:
            state = 'T'; break;
        case 4:
            state = 'M'; break;
        }
        return TestCaseResult(0, state == 'T' ? timeLim : 0, state, output);
    }

    double time;
    QTextStream in(&output, QIODevice::ReadOnly);
    QString note = in.readLine().trimmed();
    in >> time;

    return TestCaseResult(1.0, time, ' ', note);
}

#endif

ResultSummary BaseJudger::Judge()
{
    QString title = player->GetNameWithList();
    if (title == "std") title = QString("\"%1\" 的标程").arg(problem->Name()); else title += " - " + problem->Name();
    emit titleDetailFinished(title);

    QDomDocument doc;
    QDomProcessingInstruction xml = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xml);
    QDomElement root = doc.createElement("task");
    doc.appendChild(root);

    auto saveXml = [&](ResultSummary result)
    {
        if (Global::g_is_judge_stoped) return ResultSummary();
        QFile file(Global::g_contest.result_path + problem->Name() + "/" + player->Name() + ".res");
        if (!QDir(Global::g_contest.result_path + problem->Name()).exists())
            QDir(Global::g_contest.result_path).mkpath(problem->Name());

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            emit noteDetailFinished("无法写入结果文件", "E");
            result.state = 'E', result.detail = "?";
            return result;
        }
        root.setAttribute("score", result.score);
        root.setAttribute("state", QChar(result.state));
        root.setAttribute("time", result.time);
        QTextStream out(&file);
        doc.save(out, 4);
        file.close();
        return result;
    };

    QString note;
    if (!waitForCleanTmpDirectory(2000))
        emit noteDetailFinished("临时目录无法清空", "E");
    if (Global::g_is_judge_stoped) return ResultSummary();

    if (problem->Type() == Global::OtherProblemType)
    {
        note = "无效的试题类型";
        emit noteDetailFinished(note, "E");
        QDomElement no = doc.createElement("note");
        no.appendChild(doc.createTextNode(note));
        root.appendChild(no);
        return saveXml(ResultSummary(0, 0, 'E', "?"));
    }

    if (needCompile())
    {
        const Compiler* compiler = problem->GetCompiler(player->Name());
        if (!compiler || compiler->SourceFile().isEmpty())
        {
            note = "找不到文件";
            emit noteDetailFinished(note, "N");
            QDomElement no = doc.createElement("note");
            no.appendChild(doc.createTextNode(note));
            root.appendChild(no);
            return saveXml(ResultSummary(0, 0, 'F', "-"));
        }

        int size = QFileInfo(src_dir + compiler->SourceFile()).size();
        if (size > problem->CodeLengthLimit() * 1024)
        {
            note = QString("超过代码长度限制:\n文件大小 = %1 KB > %2 KB").arg(size / 1024.0, 0, 'f', 2).arg(problem->CodeLengthLimit(), 0, 'f', 2);
            emit noteDetailFinished(note, "N");
            QDomElement no = doc.createElement("note");
            no.appendChild(doc.createTextNode(note));
            root.appendChild(no);
            return saveXml(ResultSummary(0, 0, 'S', "="));
        }

        Global::CompileResult res;
        if (waitForMakeTmpDirectory(2000))
            res = compile(compiler, note);
        else
            res = Global::OtherCompileError, note = "无法创建临时文件";

        switch (res)
        {
            case Global::CompileKilled:
                return ResultSummary();
            case Global::InvalidCompiler:
            case Global::OtherCompileError:
            {
                emit noteDetailFinished(note, "E");
                QDomElement no = doc.createElement("note");
                no.appendChild(doc.createTextNode(note));
                root.appendChild(no);
                return saveXml(ResultSummary(0, 0, 'E', "?"));
            }
            case Global::CompileError:
            case Global::CompileTimeLimitExceeded:
            {
                emit noteDetailFinished(note, "N");
                QDomElement no = doc.createElement("note");
                no.appendChild(doc.createTextNode(note));
                root.appendChild(no);
                return saveXml(ResultSummary(0, 0, 'C', "+"));
            }
            default:
                break;
        }
    }

    if (!waitForMakeTmpDirectory(2000))
    {
        note = "无法创建临时文件";
        emit noteDetailFinished(note, "E");
        QDomElement no = doc.createElement("note");
        no.appendChild(doc.createTextNode(note));
        root.appendChild(no);
        return saveXml(ResultSummary(0, 0, 'E', "?"));
    }

    if (Global::g_is_judge_stoped) return ResultSummary();

    int num = 0;
    bool error = false;
    ResultSummary result;
    for (int i = 0; i < problem->SubtaskCount(); i++)
    {
        const Subtask* sub = problem->SubtaskAt(i);
        double ratioMin = 1e9;
        bool ignore = false;
        int len = 0;
        QDomElement subtask = doc.createElement("subtask");
        root.appendChild(subtask);
        for (auto j : *sub)
        {
            len++, num++;
            QDomElement point = doc.createElement("point");
            subtask.appendChild(point);
            if (ignore)
            {
                if (Global::g_is_judge_stoped) return ResultSummary();
                emit pointDetailFinished(num, "忽略", "I", problem->GetInOutString(j), len);

                result.detail += "I";
                point.setAttribute("ratio", 0);
                point.setAttribute("note", "忽略");
                point.setAttribute("state", "I");
                continue;
            }
            TestCaseResult res = judgeTestCaseEvent(j, num);
            if (Global::g_is_judge_stoped) return ResultSummary();
            emit pointDetailFinished(num, res.note, QChar(res.state), problem->GetInOutString(j), len);

            result.time += res.time, result.detail += res.state;
            ratioMin = std::min(ratioMin, res.score);
            if (res.state == 'E') error = true;
            if (ratioMin == 0) ignore = true;
            point.setAttribute("ratio", res.score);
            point.setAttribute("note", res.note);
            point.setAttribute("state", QChar(res.state));

            QCoreApplication::processEvents();
            QThread::msleep(20);
        }
        int score = ratioMin * sub->Score() + 0.5;
        emit scoreDetailFinished(sub->Size(), score, sub->Score());

        result.score += score, result.detail += "|";
        subtask.setAttribute("score", score);
    }
    result.detail.remove(result.detail.length() - 1, 1);
    result.state = error ? 'E' : 'N';
    return saveXml(result);
}
