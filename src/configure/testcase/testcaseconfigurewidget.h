#ifndef TESTCASECONFIGUREWIDGET_H
#define TESTCASECONFIGUREWIDGET_H

#include <QWidget>

#include "common/problem.h"

namespace Ui
{
class TestCaseConfigureWidget;
}

class TestCaseConfigureWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TestCaseConfigureWidget(QWidget* parent = nullptr);
    ~TestCaseConfigureWidget();

    void LoadFromProblem(Problem* problem);

private:
    Ui::TestCaseConfigureWidget* ui;
    Problem* current_problem;

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
