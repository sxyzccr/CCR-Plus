#include "global.h"
#include "judgethread.h"

#include <QtXml>
#include <QCoreApplication>

using namespace std;

JudgeThread::JudgeThread(QObject* parent) : QThread(parent)
{

}

JudgeThread::~JudgeThread()
{

}

void JudgeThread::setup(int r, int c, const QString& dir)
{
    this->r = r, this->c = c;
    testDir = dir;
    tmpDir = dir + ".tmp/";
    judgeList.clear();
}

void JudgeThread::initialize(const QString& name, Problem* prob)
{
    notCleared.clear();
    problem = prob;
    playerName = name;
    dataDir = testDir + "data/" + prob->Name() + "/";
    srcDir = testDir + "src/" + name + "/" + prob->Directory() + "/";
    //qDebug()<<prob->exe<<prob->checker;
}

bool JudgeThread::waitForMadeTmpDir(int ms)
{
    QDir dir(testDir), tmp(tmpDir);
    QElapsedTimer timer;
    for (timer.start(); timer.elapsed() <= ms;)
    {
        if (dir.mkdir(".tmp")) return true;
        if (Global::g_is_judge_stoped) break;
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }
    return tmp.exists();
}

bool JudgeThread::waitForClearedTmpDir(int ms)
{
    QDir dir(tmpDir);
    QElapsedTimer timer;
    for (timer.start(); dir.exists() && timer.elapsed() <= ms;)
    {
        if (dir.removeRecursively()) return true;
        if (Global::g_is_judge_stoped) break;
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }
    return !dir.exists();
}

bool JudgeThread::waitForFinished(int ms)
{
    QElapsedTimer timer;
    for (timer.start(); this->isRunning() && timer.elapsed() <= ms;)
    {
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }
    return this->isFinished();
}

bool JudgeThread::makeJudgeDir(int num)
{
    QDir dir(tmpDir);
    QString str(QString("judge_thread_%1_%2_%3").arg(playerName).arg(problem->Name()).arg(num));
    judgeDir = tmpDir + str + "/";
    if (!dir.exists()) return false;
    if (!dir.mkdir(str)) return false;
    if (problem->Type() == Global::Traditional)
    {
        QFile exe(tmpDir + problem->ExecutableFile());
        if (!exe.copy(judgeDir + problem->ExecutableFile())) return false;
    }
    return true;
}

bool JudgeThread::waitForClearJudgeDir(int num, int ms)
{
    QDir dir(tmpDir + QString("judge_thread_%1_%2_%3").arg(playerName).arg(problem->Name()).arg(num));
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

bool JudgeThread::monitorProcess(QProcess* process, int ms)
{
    QElapsedTimer timer;
    timer.start();
    for (; timer.elapsed() <= ms;)
    {
        if (process->state() != QProcess::Running) return true;
        if (Global::g_is_judge_stoped) return false;
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }
    return false;
}

Global::CompileResult JudgeThread::compile(Compiler* compiler, QString& note)
{
    QFile file(srcDir + compiler->SourceFile());
    if (!file.copy(tmpDir + compiler->SourceFile())) {note = "选手源代码拷贝失败"; return Global::OtherCompileError;}

    QProcess* process = new QProcess();
    //connect(this,SIGNAL(stopJudgingSignal()),process,SLOT(kill()));
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setWorkingDirectory(tmpDir);
    process->start(compiler->Cmd());
    if (!process->waitForStarted(-1))
    {
        note = "无效的编译器";
        delete process;
        return Global::InvalidCompiler;
    }

    bool ok = monitorProcess(process, compiler->TimeLimit() * 1000);
    if (!ok)
    {
        process->kill();
        delete process;
        if (Global::g_is_judge_stoped) return Global::CompileKilled;
        note = "编译超时";
        return Global::CompileTimeLimitExceeded;
    }
    if (process->exitCode())
    {
        note = "编译错误:\n" + QString::fromLocal8Bit(process->readAllStandardOutput());
        //qDebug()<<note.length();
        if (note.length() > 51200) note = "编译错误过多";
        delete process;
        return Global::CompileError;
    }
    delete process;
    return Global::CompileSuccessfully;
}

int pid;

#ifdef Q_OS_WIN

#include <windows.h>
#include <psapi.h>

bool JudgeThread::runProgram(double timeLim, double memLim, QString& note, QString& state, double& usedTime)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    PROCESS_MEMORY_COUNTERS pmc;

    auto EndProcess = [&](PROCESS_INFORMATION& pi)
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    };

    usedTime = 0;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    if (!CreateProcess(0, (wchar_t*)(judgeDir + problem->ExecutableFile()).utf16(), 0, 0, FALSE, CREATE_NO_WINDOW, 0, (wchar_t*)judgeDir.utf16(), &si, &pi))
    {
        note = "无法运行程序", state = "E";
        return 0;
    }

    bool ok = false;
    QElapsedTimer timer;
    timer.start();
    for (; timer.elapsed() <= timeLim * 1000 + 10;)
    {
        if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0)
        {
            ok = true;
            break;
        }
        if (Global::g_is_judge_stoped)
        {
            TerminateProcess(pi.hProcess, 0);
            EndProcess(pi);
            return 0;
        }
        GetProcessMemoryInfo(pi.hProcess, &pmc, sizeof(pmc));
        if (pmc.PeakPagefileUsage > memLim * (1 << 20))
        {
            TerminateProcess(pi.hProcess, 0);
            EndProcess(pi);
            note = "超过内存限制", state = 'M';
            return 0;
        }
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    if (exitCode && exitCode != STILL_ACTIVE)
    {
        EndProcess(pi);
        note = QString("运行时错误: %1").arg((int)exitCode), state = 'R';
        return 0;
    }

    if (!ok)
    {
        TerminateProcess(pi.hProcess, 0);
        EndProcess(pi);
        note = "超过时间限制", state = 'T', usedTime = timeLim;
        return 0;
    }

    FILETIME ct, et, kt, ut;
    SYSTEMTIME st;
    GetProcessTimes(pi.hProcess, &ct, &et, &kt, &ut);
    GetProcessMemoryInfo(pi.hProcess, &pmc, sizeof(pmc));
    EndProcess(pi);

    FileTimeToSystemTime(&ut, &st);
    usedTime = st.wHour * 3600 + st.wMinute * 60 + st.wSecond + st.wMilliseconds / 1000.0;
    if (usedTime > timeLim)
    {
        note = "超过时间限制", state = 'T', usedTime = timeLim;
        return 0;
    }
    note = QString("时间: %1s 内存: %2MB").arg(usedTime, 0, 'f', 2).arg(pmc.PeakPagefileUsage / 1024.0 / 1024.0, 0, 'f', 2);
    return 1;
}
#endif

double JudgeThread::judgeOutput(const QString& inFile, const QString& ansFile, const QString& outFile, QString& note, QString& state)
{
//    qDebug()<<inFile<<ansFile<<outFile;
    //  QThread::msleep(3000);
    if (!QFile(ansFile).exists()) {note = "找不到标准输出文件", state = 'E'; return 0;}
    if (!QFile(outFile).exists()) {note = "程序无输出", state = 'O'; return 0;}

    QProcess* process = new QProcess();
    //connect(this,SIGNAL(stopJudgingSignal()),process,SLOT(kill()));
    process->setWorkingDirectory(judgeDir);
    QString checkerDir = dataDir;
    if (QFile(QDir().currentPath() + "/checker/" + problem->Checker()).exists()) checkerDir = QDir().currentPath() + "/checker/";
#ifdef Q_OS_LINUX
    QProcess::execute(QString("chmod +wx \"%1\"").arg(checkerDir + problem->Checker()));
#endif
    process->start(checkerDir + problem->Checker(), QStringList({inFile, ansFile, outFile, ".result"}));
    //qDebug()<<checkerDir<<problem->checker;
    if (!process->waitForStarted(-1))
    {
        note = "无效的校验器", state = "E";
        delete process;
        return 0;
    }

    bool ok = monitorProcess(process, problem->CheckerTimeLimit() * 1000);
    if (!ok)
    {
        process->kill();
        delete process;
        if (Global::g_is_judge_stoped) return 0;
        note = "校验器超时或崩溃", state = "E";
        return 0;
    }
    if (process->exitCode())
    {
        note = "校验器崩溃", state = "E";
        delete process;
        return 0;
    }
    delete process;

    QFile file(judgeDir + ".result");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        note = "找不到校验器输出", state = "E";
        return 0;
    }
    QTextStream in(&file);
    QString s = in.readLine().trimmed();
    double ratio = s.toDouble(&ok);
    if (!ok || ratio < 0 || ratio > 1)
    {
        file.close();
        if (s.length() >= 13) s = s.left(11) + "…";
        note = QString("校验器输出格式错误 第一行: \"%1\"").arg(s), state = "E";
        return 0;
    }
    s = in.readLine().trimmed();
    file.close();

    if (problem->Type() == Global::Traditional && ratio == 1.0) s += (s.size() ? " " : "") + note;
    if (ratio == 0) state = "W", note = s == "" ? "答案错误" : s;
    else if (ratio == 1.0) state = "A", note = s == "" ? "答案正确" : s;
    else state = "P", note = s == "" ? "部分正确" : s;

    return ratio;
}

double JudgeThread::judgeTraditionalTask(TestCase* point, QString& note, QString& state, double& usedTime)
{
    QFile file(dataDir + point->InFile());
    if (!file.exists()) {note = "找不到标准输入文件: " + point->InFile(), state = "E"; return 0;}
    if (!file.copy(judgeDir + problem->InFile())) {note = "标准输入文件拷贝失败", state = "E"; return 0;}

#ifdef Q_OS_LINUX
    QProcess::execute(QString("chmod +wx \"%1\"").arg(QDir().currentPath() + "/monitor"));

    QProcess* process = new QProcess();
    //connect(this,SIGNAL(stopJudgingSignal(),process,SLOT(kill()));
    process->setWorkingDirectory(judgeDir);
    process->start(QDir().currentPath() + "/monitor", QStringList({ problem->ExecutableFile(),
                                                                    QString::number(point->TimeLimit()),
                                                                    QString::number(point->MemoryLimit())
                                                                  }));
    if (!process->waitForStarted(-1))
    {
        note = "无法运行进程监视器", state = "E";
        delete process;
        return 0;
    }

    bool ok = monitorProcess(process, point->TimeLimit() * 1000 + 10);
    QString out = QString::fromLocal8Bit(process->readAllStandardOutput());
    if (!ok)
    {
        process->kill();
        delete process;
        if (Global::g_is_judge_stoped) return 0;
        note = "超过时间限制", state = "T";
        return 0;
    }
    if (process->exitCode())
    {
        switch (process->exitCode())
        {
        case 1:
            state = "E"; break;
        case 2:
            state = "R"; break;
        case 3:
            state = "T"; break;
        case 4:
            state = "M"; break;
        }
        note = out;
        delete process;
        return 0;
    }
    delete process;

    QTextStream in(&out, QIODevice::ReadOnly);
    note = in.readLine().trimmed();
    in >> usedTime;
#else
    if (!runProgram(point->TimeLimit(), point->MemoryLimit(), note, state, usedTime)) return 0;
#endif

    return judgeOutput(dataDir + point->InFile(), dataDir + point->OutFile(), judgeDir + problem->OutFile(), note, state);
}


double JudgeThread::judgeAnswersOnlyTask(TestCase* point, QString& note, QString& state)
{
    QFile file(srcDir + point->SubmitFile());
    if (!file.exists()) {note = "未提交", state = "U"; return 0;}
    if (!file.copy(judgeDir + point->SubmitFile())) {note = "未知错误", state = "E"; return 0;}

    return judgeOutput(dataDir + point->InFile(), dataDir + point->OutFile(), judgeDir + point->SubmitFile(), note, state);
}

double JudgeThread::judgeTask(TestCase* point, QString& note, QString& state, double& usedTime, int testNum)
{
    if (Global::g_is_judge_stoped) return 0;
    //qDebug()<<testNum<<info->in<<info->out;
    double res = 0;
    if (!makeJudgeDir(testNum)) note = "无法创建临时文件", state = "E";
    else if (problem->Type() == Global::Traditional) res = judgeTraditionalTask(point, note, state, usedTime);
    else if (problem->Type() == Global::AnswersOnly) res = judgeAnswersOnlyTask(point, note, state);

    //QCoreApplication::processEvents();
    //msleep(10);

    if (!waitForClearJudgeDir(testNum, 2000) && problem->Type() == Global::Traditional) notCleared.insert(testNum); //note="运行时错误: 未知",state="R",,res=0;
    for (auto i : notCleared) if (waitForClearJudgeDir(i, 2000)) notCleared.erase(i);

    return res;
}

void JudgeThread::judgeProblem(Player* player, Problem* problem, char& state, int& sumScore, double& sumTime, QString& detail)
{
    //QTableWidgetItem*tmp;
    //int row=detailTable->rowCount()-1;
    //qDebug()<<row<<player->name<<problem->name;
    QString title = player->GetNameWithList();
    if (title == "std") title = QString("\"%1\" 的标程").arg(problem->Name()); else title += " - " + problem->Name();
    emit titleDetailFinished(rows++, title);

    sumTime = 0, sumScore = 0, state = ' ', detail = "";
    bool error = false;

    QDomDocument doc;
    QDomProcessingInstruction xml = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xml);
    QDomElement root = doc.createElement("task");
    doc.appendChild(root);

    auto endXml = [&]()
    {
        if (Global::g_is_judge_stoped) return;
        QFile file(Global::g_contest.result_path + problem->Name() + "/" + player->Name() + ".res");
        if (!QDir(Global::g_contest.result_path).exists()) QDir(Global::g_contest.path).mkpath("result");
        if (!QDir(Global::g_contest.result_path + problem->Name()).exists()) QDir(Global::g_contest.result_path).mkdir(problem->Name());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            emit noteDetailFinished(rows++, "无法写入结果文件", "E");
            state = 'E', detail = "?";
            return;
        }
        root.setAttribute("score", sumScore);
        root.setAttribute("state", QString(state));
        root.setAttribute("time", sumTime);
        QTextStream out(&file);
        doc.save(out, 4);
        file.close();
    };

    this->initialize(player->Name(), problem);
    this->waitForClearedTmpDir(2000);
    /*if (!this->waitForClearedTmpDir(2000))
    {
        //detailTable->addNoteDetail(row,"临时目录无法清空","E");
        //detailTable->adjustScrollbar();
        emit noteDetailFinished(rows++,"临时目录无法清空","E");
        state='E',detail="?";
        QDomElement no=doc.createElement("note");
        no.appendChild(doc.createTextNode("临时目录无法清空"));
        root.appendChild(no);
        endXml();
        return;
    }*/
    if (Global::g_is_judge_stoped) {state = ' ', sumScore = 0, sumTime = 0; return;}
    if (problem->Type() == Global::Traditional)
    {
        QString note;
        Compiler* compiler = problem->GetCompiler(player->Name());
        if (!compiler || compiler->SourceFile().isEmpty())
        {
            emit noteDetailFinished(rows++, "找不到文件", "N");
            state = 'F', detail = "-";
            QDomElement no = doc.createElement("note");
            no.appendChild(doc.createTextNode("找不到文件"));
            root.appendChild(no);
            endXml();
            return;
        }

        int size = QFileInfo(this->srcDir + compiler->SourceFile()).size();
        if (size > problem->CodeLengthLimit() * 1024)
        {
            QString s = QString("超过代码长度限制:\n文件大小 = %1 KB > %2 KB").arg(size / 1024.0, 0, 'f', 2).arg(problem->CodeLengthLimit(), 0, 'f', 2);
            //qDebug()<<s;
            emit noteDetailFinished(rows++, s, "N");
            state = 'S', detail = "=";
            QDomElement no = doc.createElement("note");
            no.appendChild(doc.createTextNode(s));
            root.appendChild(no);
            endXml();
            return;
        }

        Global::CompileResult res;
        if (this->waitForMadeTmpDir(2000)) res = this->compile(compiler, note);
        else res = Global::OtherCompileError, note = "无法创建临时文件";

        if (res == Global::CompileKilled) return;
        else if (res == Global::OtherCompileError || res == Global::InvalidCompiler)
        {
            emit noteDetailFinished(rows++, note, "E");
            state = 'E', detail = "?";
            QDomElement no = doc.createElement("note");
            no.appendChild(doc.createTextNode(note));
            root.appendChild(no);
            endXml();
            return;
        }
        else if (res == Global::CompileError || res == Global::CompileTimeLimitExceeded)
        {
            emit noteDetailFinished(rows++, note, "N");
            state = 'C', detail = "+";
            QDomElement no = doc.createElement("note");
            no.appendChild(doc.createTextNode(note));
            root.appendChild(no);
            endXml();
            return;
        }
    }
    else if (problem->Type() == Global::AnswersOnly)
    {
        if (!this->waitForMadeTmpDir(2000))
        {
            emit noteDetailFinished(rows++, "无法创建临时文件", "E");
            state = 'E', detail = "?";
            QDomElement no = doc.createElement("note");
            no.appendChild(doc.createTextNode("无法创建临时文件"));
            root.appendChild(no);
            endXml();
            return;
        }
    }
    else
    {
        emit noteDetailFinished(rows++, "无效的试题类型", "E");
        state = 'E', detail = "?";
        QDomElement no = doc.createElement("note");
        no.appendChild(doc.createTextNode("无效的试题类型"));
        root.appendChild(no);
        endXml();
        return;
    }

    if (Global::g_is_judge_stoped) {state = ' ', sumScore = 0, sumTime = 0; return;}
    int num = 0;
    //bool isPackage=false;
    for (int i = 0; i < problem->SubtaskCount(); i++)
    {
        Subtask* sub = problem->SubtaskAt(i);
        double ratioMin = 1e9;
        bool ignore = false;
        int len = 0;
        QDomElement subtask = doc.createElement("subtask");
        root.appendChild(subtask);
        for (TestCase* j : *sub)
        {
            len++, num++;
            //QCoreApplication::processEvents();
            msleep(20);
            QDomElement point = doc.createElement("point");
            subtask.appendChild(point);
            if (ignore)
            {
                if (Global::g_is_judge_stoped) { state = ' ', sumScore = 0, sumTime = 0; return; }
                emit pointDetailFinished(rows++, j->Id(), "忽略", "I", problem->GetInOutString(j), len);
                detail += "I";
                point.setAttribute("ratio", 0);
                point.setAttribute("note", "忽略");
                point.setAttribute("state", "I");
                continue;
            }
            QString sta, note;
            double t = 0;
            double ratio = this->judgeTask(j, note, sta, t, num);
            if (Global::g_is_judge_stoped) { state = ' ', sumScore = 0, sumTime = 0; return; }
            emit pointDetailFinished(rows++, j->Id(), note, sta, problem->GetInOutString(j), len);
            point.setAttribute("ratio", ratio);
            point.setAttribute("note", note);
            point.setAttribute("state", sta);
            sumTime += t, ratioMin = min(ratioMin, ratio), detail += sta;
            if (sta == "E") error = true;
            if (ratioMin == 0) ignore = true;
        }
        int score = ratioMin * sub->Score() + 0.5;
        emit scoreDetailFinished(rows, sub->Size(), score, sub->Score());
        sumScore += score, detail += "|";
        subtask.setAttribute("score", score);
    }
    detail.remove(detail.length() - 1, 1);
    //if (!isPackage) detail.remove("|");
    state = error ? 'E' : 'N';
    endXml();
}

void JudgeThread::saveHTMLResult(Player* player)
{
    QDomDocument doc, res;
    QDomElement html = doc.createElement("html");
    doc.appendChild(html);

    QDomElement head = doc.createElement("head");
    html.appendChild(head);
    QDomElement meta = doc.createElement("meta");
    meta.setAttribute("http-equiv", "Content-Type");
    meta.setAttribute("content", "text/html; charset=utf-8");
    head.appendChild(meta);
    meta = doc.createElement("title");
    meta.appendChild(doc.createTextNode(QString("%1的测评结果").arg(player->Name())));
    head.appendChild(meta);

    head = doc.createElement("body");
    html.appendChild(head);
    QDomElement table = doc.createElement("table");
    table.setAttribute("width", 500);
    table.setAttribute("cellpadding", 2);
    table.setAttribute("style", "font-family:verdana");
    head.appendChild(table);

    auto addRow = [&](const QString & note, QString state)
    {
        QDomElement tr = doc.createElement("tr");
        tr.setAttribute("height", 25);
        table.appendChild(tr);
        QDomElement td = doc.createElement("td");
        td.setAttribute("colspan", 3);

        QColor fg(80, 80, 80), bg(180, 180, 180);
        if (state == "E") fg.setRgb(0, 0, 0), bg.setRgb(227, 58, 218);
        if (state == " ") fg.setRgb(100, 100, 100), bg.setRgb(235, 235, 235);
        if (state == "title") fg.setRgb(255, 255, 255), bg.setRgb(120, 120, 120);
        if (state == "sum") fg.setRgb(0, 0, 0), bg.setRgb(235, 235, 235);

        td.setAttribute("style", QString("color:%1; background-color:%2").arg(fg.name(), bg.name()));
        td.appendChild(doc.createTextNode(note));
        tr.appendChild(td);
    };

    addRow(QString("总分: %1").arg(player->SumLabel()->Score()), "sum");

    for (auto p : Global::g_contest.problem_order)
    {
        QString name = Global::g_contest.problems[p]->Name();
        QFile file(Global::g_contest.result_path + name + "/" + player->Name() + ".res");
        //qDebug()<<file.fileName();

        addRow(QString("%1: %2").arg(Global::g_contest.problems[p]->Name()).arg(player->ProblemLabelAt(p)->Score()), "title");

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            addRow("无测评结果", " ");
            continue;
        }
        if (!res.setContent(&file)) {file.close(), addRow("无效的测评结果", " "); continue;}
        QDomElement rt = res.documentElement();
        if (rt.isNull() || rt.tagName() != "task") {file.close(), addRow("无效的测评结果", " "); continue;}

        QDomNodeList list = rt.childNodes();
        for (int i = 0; i < list.count(); i++) if (list.item(i).toElement().tagName() == "note") addRow(list.item(i).toElement().text(), rt.attribute("state"));

        for (int i = 0, tasktot = 0, tot = 0; i < list.count(); i++)
        {
            QDomElement a = list.item(i).toElement();
            if (a.tagName() == "subtask")
            {
                QDomNodeList l = a.childNodes();
                int k = 0, kk = 0;
                for (int j = 0; j < l.count(); j++) if (l.item(j).toElement().tagName() == "point") kk++;
                for (int j = 0; j < l.count(); j++)
                {
                    QDomElement b = l.item(j).toElement();
                    if (b.tagName() == "point")
                    {
                        QDomElement tr = doc.createElement("tr");
                        tr.setAttribute("height", 25);
                        table.appendChild(tr);
                        QDomElement td = doc.createElement("td");
                        td.setAttribute("width", 25);
                        td.setAttribute("align", "right");
                        td.setAttribute("style", "background-color:#ebebeb"); //rgb(235,235,235)
                        td.appendChild(doc.createTextNode(QString::number(tot + 1)));
                        tr.appendChild(td);

                        if (!k)
                        {
                            td = doc.createElement("td");
                            td.setAttribute("width", 50);
                            td.setAttribute("rowspan", kk);
                            td.setAttribute("align", "center");
                            QColor color = Global::GetRatioColor(235, 235, 235, 0, 161, 241,
                                                                 a.attribute("score").toInt(),
                                                                 tasktot < Global::g_contest.problems[p]->SubtaskCount() ?
                                                                           Global::g_contest.problems[p]->SubtaskAt(tasktot)->Score() :
                                                                           0);
                            td.setAttribute("style", QString("background-color:%1").arg(color.name()));
                            td.appendChild(doc.createTextNode(a.attribute("score")));
                            tr.appendChild(td);
                        }

                        QColor o(255, 255, 255);
                        if (b.attribute("state").length() == 1)
                            switch (b.attribute("state")[0].toLatin1())
                            {
                            case 'A':
                                o.setRgb(51, 185, 6); //AC
                                break;
                            case 'C':
                            case 'E':
                                o.setRgb(227, 58, 218); //Error
                                break;
                            case 'I':
                            case 'U':
                                o.setRgb(235, 235, 235); //Ignore/UnSubmit
                                break;
                            case 'M':
                            case 'R':
                                o.setRgb(247, 63, 63); //MLE/RE
                                break;
                            case 'O':
                                o.setRgb(180, 180, 180); //No Output
                                break;
                            case 'P':
                                o.setRgb(143, 227, 60); //Partial
                                break;
                            case 'W':
                                o.setRgb(246, 123, 20); //WA
                                break;
                            case 'T':
                                o.setRgb(255, 187, 0); //TLE
                                break;
                            }
                        td = doc.createElement("td");
                        td.setAttribute("style", QString("background-color:%1").arg(o.name()));
                        td.appendChild(doc.createTextNode(b.attribute("note")));
                        tr.appendChild(td);
                        k++, tot++;
                    }
                }
                tasktot++;
            }
        }
        file.close();
    }
    QDomElement p = doc.createElement("p");
    p.appendChild(doc.createTextNode("本文件由 CCR Plus 测评器生成，如有疑问和建议请致信 "));
    QDomElement a = doc.createElement("a");
    a.setAttribute("href", "mailto:equation618@gmail.com");
    a.appendChild(doc.createTextNode("equation618@gmail.com"));
    p.appendChild(a);
    head.appendChild(p);

    p = doc.createElement("hr");
    p.setAttribute("width", 500);
    p.setAttribute("size", 1);
    p.setAttribute("align", "left");
    p.setAttribute("color", "#b4b4b4"); //rgb(180,180,180)
    head.appendChild(p);

    p = doc.createElement("p");
    p.appendChild(doc.createTextNode("绍兴一中 贾越凯"));
    head.appendChild(p);

    QFile file(Global::g_contest.src_path + player->Name() + "/result.html");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    doc.save(out, 4);
    file.close();
}

void JudgeThread::run()
{
    Global::g_is_judging = true;

    rows = -1;
    Player* ply;
    auto clear = [&](int c)
    {
        ResultLabel* tmp = ply->LabelAt(c);
        ply->SumLabel()->Subtract(tmp->Result());

        tmp->SetResult(0, 0, ' ');
        emit resultLabelTextChanged(tmp, "", "未测评", Global::StyleNone);

        QFile(Global::g_contest.result_path + Global::g_contest.problems[c - 2]->Name() + "/" + ply->Name() + ".res").remove();
    };
    auto work = [&](int c)
    {
        ResultSummary res;
        ResultLabel* tmp = ply->LabelAt(c);

        emit resultLabelTextChanged(tmp, "~", "正在测评...", Global::StyleRunning);

        judgeProblem(ply, Global::g_contest.problems[c - 2], res.state, res.score, res.time, res.detail);
        //QCoreApplication::processEvents();
        msleep(50);

        tmp->SetResult(res);
        ply->SumLabel()->Plus(res);

        emit problemResultLabelChanged(ply, c, Global::g_contest.problems[c - 2]->Score());
        emit sumResultLabelChanged(ply, Global::g_contest.sum_score);
        saveHTMLResult(ply);
    };


    if (!c || c == 1) // Judge one player's all problems
    {
        int t = Global::GetLogicalRow(r);
        ply = Global::g_contest.players[t];
        for (auto i : Global::g_contest.problem_order) clear(i + 2);
        emit sumResultLabelChanged(ply, Global::g_contest.sum_score);
        for (auto i : Global::g_contest.problem_order)
        {
            work(i + 2);
            if (Global::g_is_judge_stoped) break;
            emit itemJudgeFinished(r, i + 2);
        }
    }
    else // Judge tasks in judgeList
    {
        for (auto i : judgeList)
        {
            int t = Global::GetLogicalRow(i.first);
            ply = Global::g_contest.players[t];
            clear(i.second);
            emit sumResultLabelChanged(ply, Global::g_contest.sum_score);
            work(i.second);
            if (Global::g_is_judge_stoped) break;
            emit itemJudgeFinished(i.first, i.second);
        }
    }

    emit noteDetailFinished(rows, Global::g_is_judge_stoped ? "- 测评终止 -" : "- 测评结束 -", "");
    this->waitForClearedTmpDir(2000);
    Global::g_is_judging = false;
}
