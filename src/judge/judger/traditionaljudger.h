#ifndef TRADITIONALJUDGER_H
#define TRADITIONALJUDGER_H

#include "judge/judger/basejudger.h"

class TraditionalJudger : public BaseJudger
{
public:
    explicit TraditionalJudger(const QString& testDir, Player* player, Problem* problem, QObject *parent = 0) :
        BaseJudger(testDir, player, problem, parent) {}

protected:
    bool needCompile() const override { return true; }
    virtual TestCaseResult judgeTestCase(TestCase* point) const override;
};

#endif // TRADITIONALJUDGER_H
