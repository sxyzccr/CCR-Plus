#ifndef CONFIGUREWIDGET_H
#define CONFIGUREWIDGET_H

#include <QWidget>

#include "common/problem.h"

class ConfigureTabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigureTabWidget(QWidget* parent = nullptr);
    virtual ~ConfigureTabWidget() {}

    /// 载入 problem，显示具体配置
    virtual void ShowProblemConfiguration(Problem* problem) = 0;
    /// 转换题目类型
    virtual void ChangeProblemType(Global::ProblemType type) = 0;
    /// 重置为默认
    virtual void Reset() = 0;

protected:
    Problem* current_problem;
};

#endif // CONFIGUREWIDGET_H
