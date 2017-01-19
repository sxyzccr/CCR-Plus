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
    virtual void ShowProblemConfiguration(Problem* problem) override;
    /// 重置为默认
    virtual void Reset() override;

    /// 缓存配置，只在切换题目时进行
    void ChacheConfiguration();

private:
    Ui::TestCaseTabWidget* ui;

private slots:
    void onTestCaseSelectionChanged();

    void on_tableWidget_doubleClicked(const QModelIndex& index);
    void on_pushButton_addTestCase_clicked();
    void on_pushButton_addSubTestCase_clicked();
    void on_pushButton_delete_clicked();
    void on_pushButton_up_clicked();
    void on_pushButton_down_clicked();
    void on_pushButton_merge_clicked();
    void on_pushButton_split_clicked();
    void on_pushButton_reset_clicked();
};

#endif // TESTCASECONFIGUREWIDGET_H
