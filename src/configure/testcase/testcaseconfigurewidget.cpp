#include "configure/testcase/testcasetable.h"
#include "configure/testcase/addtestcasedialog.h"
#include "configure/testcase/testcaseconfigurewidget.h"
#include "ui_testcaseconfigurewidget.h"

TestCaseConfigureWidget::TestCaseConfigureWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TestCaseConfigureWidget)
{
    ui->setupUi(this);

    ui->tableWidget_testcase->setMinimumHeight(ui->tableWidget_testcase->verticalHeader()->defaultSectionSize() * 20 +
                                               ui->tableWidget_testcase->horizontalHeader()->height() +
                                               ui->tableWidget_testcase->frameWidth() * 2);

    connect(ui->tableWidget_testcase, &TestCaseTable::testCaseSelectionChanged, this, &TestCaseConfigureWidget::onTestCaseSelectionChanged);
}

TestCaseConfigureWidget::~TestCaseConfigureWidget()
{
    delete ui;
}

void TestCaseConfigureWidget::LoadFromProblem(Problem* problem)
{
    current_problem = problem;

    ui->tableWidget_testcase->LoadTestCases(problem);
    ui->label_score->setText(QString::number(ui->tableWidget_testcase->SumScore()));
}

void TestCaseConfigureWidget::onTestCaseSelectionChanged()
{
    ui->pushButton_addTestCase->setEnabled(ui->tableWidget_testcase->CanAddTestCase());
    ui->pushButton_addSubTestCase->setEnabled(ui->tableWidget_testcase->CanAddSubTestCase());
    ui->pushButton_removeTestCase->setEnabled(ui->tableWidget_testcase->CanRemoveTestCase());
    ui->pushButton_up->setEnabled(ui->tableWidget_testcase->CanMoveUp());
    ui->pushButton_down->setEnabled(ui->tableWidget_testcase->CanMoveDown());
    ui->pushButton_merge->setEnabled(ui->tableWidget_testcase->CanMerge());
    ui->pushButton_split->setEnabled(ui->tableWidget_testcase->CanSplit());

    int top, bottom;
    TestCaseTable::SelectionType type = ui->tableWidget_testcase->GetSelectionType(&top, &bottom);
    switch (type)
    {
    case TestCaseTable::SelectOnePackage:
    case TestCaseTable::SelectOneTestCasePackage:
        ui->label_selectionInfo->setText(QString("已选择: 1 个测试点，%1 组测试数据，共 %2 分").arg(bottom - top + 1).arg(ui->tableWidget_testcase->ScoreAt(top)));
        break;
    case TestCaseTable::SelectOneSubTestCase:
        ui->label_selectionInfo->setText("已选择: 1 组测试数据");
        break;
    case TestCaseTable::SelectMultiplePackage:
    case TestCaseTable::SelectMultipleTestCasePackage:
    {
        int t = 0, s = 0;
        for (int i = top; i <= bottom; i++)
            if (ui->tableWidget_testcase->ScoreItemTopRow(i) == i) t++, s += ui->tableWidget_testcase->ScoreAt(i);
        ui->label_selectionInfo->setText(QString("已选择: %1 个测试点，%2 组测试数据，共 %3 分").arg(t).arg(bottom - top + 1).arg(s));
        break;
    }
    case TestCaseTable::SelectMultipleSubTestCase:
        ui->label_selectionInfo->setText(QString("已选择: %1 组测试数据").arg(bottom - top + 1));
        break;
    case TestCaseTable::OtherSelection:
        ui->label_selectionInfo->setText(QString("已选择: %1 组测试数据，位于多个测试点").arg(bottom - top + 1));
        break;
    default:
        ui->label_selectionInfo->setText("");
        break;
    }
}



void TestCaseConfigureWidget::on_tableWidget_testcase_doubleClicked(const QModelIndex& index)
{
    int id = index.row();
    if (!index.column())
    {
        AddTestCaseDialog dialog(current_problem, nullptr, AddTestCaseDialog::EditScore, -1, this, ui->tableWidget_testcase->ScoreAt(id));
        if (dialog.exec() == QDialog::Accepted)
        {
            ui->tableWidget_testcase->ChangeScore(id, dialog.GetScore());
            ui->label_score->setText(QString::number(ui->tableWidget_testcase->SumScore()));
        }
    }
    else
    {
        TestCase* point = current_problem->TestCaseAt(id);

        AddTestCaseDialog dialog(current_problem, point, AddTestCaseDialog::EditSubTestCase, index.column(), this);
        if (dialog.exec() == QDialog::Accepted)
            ui->tableWidget_testcase->ChangeTestCase(id, dialog.GetTestCase());
    }
    onTestCaseSelectionChanged();
}

void TestCaseConfigureWidget::on_pushButton_addTestCase_clicked()
{
    int id = 1;
    auto list = ui->tableWidget_testcase->selectedItems();
    if (list.size()) id = list.first()->row() + 2;

    TestCase* point;
    if (current_problem->Type() == Global::Traditional)
        point = new TestCase(1, 128, QString("%1%2.in").arg(current_problem->Name()).arg(id),
                                     QString("%1%2.out").arg(current_problem->Name()).arg(id));
    else
        point = new TestCase(0, 0, QString("%1%2.in").arg(current_problem->Name()).arg(id),
                                   QString("%1%2.out").arg(current_problem->Name()).arg(id),
                                   QString("%1%2.out").arg(current_problem->Name()).arg(id));

    AddTestCaseDialog dialog(current_problem, point, AddTestCaseDialog::AddTestCase, -1, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        ui->tableWidget_testcase->AddTestCase(dialog.GetTestCase(), dialog.GetScore());
        ui->label_score->setText(QString::number(ui->tableWidget_testcase->SumScore()));
    }
}

void TestCaseConfigureWidget::on_pushButton_addSubTestCase_clicked()
{
    int id = 1;
    auto list = ui->tableWidget_testcase->selectedItems();
    if (list.size()) id = list.first()->row() + 2;

    TestCase* point;
    if (current_problem->Type() == Global::Traditional)
        point = new TestCase(1, 128, QString("%1%2.in").arg(current_problem->Name()).arg(id),
                                     QString("%1%2.out").arg(current_problem->Name()).arg(id));
    else
        point = new TestCase(0, 0, QString("%1%2.in").arg(current_problem->Name()).arg(id),
                                   QString("%1%2.out").arg(current_problem->Name()).arg(id),
                                   QString("%1%2.out").arg(current_problem->Name()).arg(id));

    AddTestCaseDialog dialog(current_problem, point, AddTestCaseDialog::AddSubTestCase, -1, this);
    if (dialog.exec() == QDialog::Accepted)
        ui->tableWidget_testcase->AddSubTestCase(dialog.GetTestCase());
}

void TestCaseConfigureWidget::on_pushButton_removeTestCase_clicked()
{
    ui->tableWidget_testcase->RemoveSelection();
    ui->label_score->setText(QString::number(ui->tableWidget_testcase->SumScore()));
}

void TestCaseConfigureWidget::on_pushButton_up_clicked()
{
    ui->tableWidget_testcase->MoveUpSelection();
}

void TestCaseConfigureWidget::on_pushButton_down_clicked()
{
    ui->tableWidget_testcase->MoveDownSelection();
}

void TestCaseConfigureWidget::on_pushButton_merge_clicked()
{
    ui->tableWidget_testcase->MergeSelection();
}

void TestCaseConfigureWidget::on_pushButton_split_clicked()
{
    ui->tableWidget_testcase->SplitSelection();
}

void TestCaseConfigureWidget::on_pushButton_resetTestCase_clicked()
{

}
