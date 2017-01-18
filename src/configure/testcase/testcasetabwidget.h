#ifndef TESTCASECONFIGUREWIDGET_H
#define TESTCASECONFIGUREWIDGET_H

#include "configure/configuretabwidget.h"

namespace Ui
{
class TestCaseTabWidget;
}

class TestCaseTabWidget : public ConfigureTabWidget
{
    Q_OBJECT
public:
    explicit TestCaseTabWidget(QWidget* parent = nullptr);
    ~TestCaseTabWidget();

    /// 载入 problem，显示具体配置
    void ShowProblemConfiguration(Problem* problem) override;
    /// 重置为默认
    void Reset() override;

private:
    Ui::TestCaseTabWidget* ui;

private slots:
    void onTestCaseSelectionChanged();

    void on_tableWidget_testcase_doubleClicked(const QModelIndex& index);
    void on_pushButton_addTestCase_clicked();
    void on_pushButton_addSubTestCase_clicked();
    void on_pushButton_removeTestCase_clicked();
    void on_pushButton_up_clicked();
    void on_pushButton_down_clicked();
    void on_pushButton_merge_clicked();
    void on_pushButton_split_clicked();
    void on_pushButton_resetTestCase_clicked();
};

#endif // TESTCASECONFIGUREWIDGET_H
