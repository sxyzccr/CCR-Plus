#include "judge/judger/traditionaljudger.h"

TestCaseResult TraditionalJudger::judgeTestCase(TestCase* point) const
{
    QFile file(this->DataDirectory() + point->InFile());
    if (!file.exists()) return TestCaseResult(0, 0, 'E', "找不到标准输入文件: " + point->InFile());
    if (!file.copy(this->WorkingDirectory() + problem->InFile())) return TestCaseResult(0, 0, 'E', "标准输入文件拷贝失败");
    file.setFileName(this->TmpDirectory() + problem->ExecutableFile());
    if (!file.copy(this->WorkingDirectory() + problem->ExecutableFile())) return TestCaseResult(0, 0, 'E', "选手可执行文件拷贝失败");

    TestCaseResult res = runProgram(problem->ExecutableFile(), point->TimeLimit(), point->MemoryLimit());
    if (res.score == 1.0)
    {
        TestCaseResult checkResult = checkOutput(this->DataDirectory() + point->InFile(),
                                                 this->DataDirectory() + point->OutFile(),
                                                 this->WorkingDirectory() + problem->OutFile());
        res.score = checkResult.score;
        res.state = checkResult.state;
        if (checkResult.score == 0.0)
            res.note = checkResult.note.isEmpty() ? "答案错误" : checkResult.note;
        else if (checkResult.score < 1.0 && checkResult.note.isEmpty())
            res.note = checkResult.note.isEmpty() ? "部分正确" : checkResult.note;
        else if (checkResult.score == 1.0 && !checkResult.note.isEmpty())
            res.note += " " + checkResult.note;
    }
    return res;
}
