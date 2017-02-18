#ifndef ANSWERONLYJUDGER_H
#define ANSWERONLYJUDGER_H

#include "judge/judger/basejudger.h"

class AnswerOnlyJudger : public BaseJudger
{
public:
    explicit AnswerOnlyJudger(const QString& testDir, const Player* player, const Problem* problem, QObject* parent = nullptr) :
        BaseJudger(testDir, player, problem, parent) {}

protected:
    virtual bool needCompile() const override { return false; }
    virtual TestCaseResult judgeTestCase(const TestCase* point) const override;
};

#endif // ANSWERONLYJUDGER_H
