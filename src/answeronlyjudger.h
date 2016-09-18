#ifndef ANSWERONLYJUDGER_H
#define ANSWERONLYJUDGER_H

#include "basejudger.h"

class AnswerOnlyJudger : public BaseJudger
{
public:
    explicit AnswerOnlyJudger(const QString& testDir, Player* player, Problem* problem, QObject* parent = nullptr) :
        BaseJudger(testDir, player, problem, parent) {}

protected:
    bool needCompile() const override { return false; }
    virtual TestCaseResult judgeTestCase(TestCase* point) const override;
};

#endif // ANSWERONLYJUDGER_H
