#include "answeronlyjudger.h"

TestCaseResult AnswerOnlyJudger::judgeTestCase(TestCase* point) const
{
    QFile file(this->SrcDirectory() + point->SubmitFile());
    if (!file.exists()) return TestCaseResult(0, 0, 'U', "未提交");
    if (!file.copy(this->WorkingDirectory() + point->SubmitFile())) return TestCaseResult(0, 0, 'E', "选手提交文件拷贝失败");

    TestCaseResult res = checkOutput(this->DataDirectory() + point->InFile(),
                                     this->DataDirectory() + point->OutFile(),
                                     this->WorkingDirectory() + point->SubmitFile());
    if (res.score == 0.0 && res.note.isEmpty())
        res.note = "答案错误";
    else if (res.score < 1.0 && res.note.isEmpty())
        res.note = "部分正确";
    else if (res.score == 1.0 && res.note.isEmpty())
        res.note = "答案正确";
    return res;
}
