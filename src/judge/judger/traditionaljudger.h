#ifndef TRADITIONALJUDGER_H
#define TRADITIONALJUDGER_H

#include "judge/judger/basejudger.h"

class TraditionalJudger : public BaseJudger
{
public:
    explicit TraditionalJudger(const QString& testDir, const Player* player, const Problem* problem, QObject* parent = nullptr) :
        BaseJudger(testDir, player, problem, parent) {}

protected:
    virtual bool needCompile() const override { return true; }
    virtual TestCaseResult judgeTestCase(const TestCase* point) const override;
};

#endif // TRADITIONALJUDGER_H
