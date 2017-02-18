#include "configure/testcase/testcasetable.h"
#include "configure/testcase/addtestcasedialog.h"
#include "configure/testcase/testcasetabwidget.h"
#include "ui_testcasetabwidget.h"

TestCaseTabWidget::TestCaseTabWidget(QWidget* parent) :
    ConfigureTabWidget(parent),
    ui(new Ui::TestCaseTabWidget)
{
    ui->setupUi(this);

    connect(ui->tableWidget, &TestCaseTable::testCaseSelectionChanged, this, &TestCaseTabWidget::onTestCaseSelectionChanged);
}

TestCaseTabWidget::~TestCaseTabWidget()
{
    delete ui;
}

void TestCaseTabWidget::ShowProblemConfiguration(Problem* problem)
{
    if (current_problem) ChacheConfiguration();
    current_problem = problem;

    ui->tableWidget->LoadTestCases(problem);
    ui->label_score->setText(QString::number(ui->tableWidget->SumScore()));
}

void TestCaseTabWidget::ChangeProblemType(Global::ProblemType /*type*/)
{
    ui->tableWidget->LoadTestCases(current_problem);
}

void TestCaseTabWidget::Reset()
{
    on_pushButton_reset_clicked();
}

void TestCaseTabWidget::ChacheConfiguration()
{
    current_problem->ClearTestCases();
    current_problem->SetScore(ui->tableWidget->SumScore());

    Subtask* sub = nullptr;
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {

        if (ui->tableWidget->ScoreItemTopRow(i) == i)
        {
            sub = new Subtask(ui->tableWidget->ScoreAt(i));
            current_problem->AppendSubtask(sub);
        }

        TestCase* point = new TestCase(ui->tableWidget->TestCaseAt(i));
        sub->Append(point);
        current_problem->AppendTestCase(point);
    }
}



void TestCaseTabWidget::onTestCaseSelectionChanged()
{
    ui->pushButton_edit->setEnabled(ui->tableWidget->CanEdit());
    ui->pushButton_addTestCase->setEnabled(ui->tableWidget->CanAddTestCase());
    ui->pushButton_addSubTestCase->setEnabled(ui->tableWidget->CanAddSubTestCase());
    ui->pushButton_delete->setEnabled(ui->tableWidget->CanDelete());
    ui->pushButton_up->setEnabled(ui->tableWidget->CanMoveUp());
    ui->pushButton_down->setEnabled(ui->tableWidget->CanMoveDown());
    ui->pushButton_merge->setEnabled(ui->tableWidget->CanMerge());
    ui->pushButton_split->setEnabled(ui->tableWidget->CanSplit());

    int top, bottom;
    TestCaseTable::SelectionType type = ui->tableWidget->GetSelectionType(&top, &bottom);
    switch (type)
    {
    case TestCaseTable::SelectOnePackage:
    case TestCaseTable::SelectOneTestCasePackage:
        ui->label_selectionInfo->setText(QString("已选择: 1 个测试点，%1 组测试数据，共 %2 分").arg(bottom - top + 1).arg(ui->tableWidget->ScoreAt(top)));
        break;
    case TestCaseTable::SelectOneSubTestCase:
        ui->label_selectionInfo->setText("已选择: 1 组测试数据");
        break;
    case TestCaseTable::SelectMultiplePackage:
    case TestCaseTable::SelectMultipleTestCasePackage:
    {
        int t = 0, s = 0;
        for (int i = top; i <= bottom; i++)
            if (ui->tableWidget->ScoreItemTopRow(i) == i) t++, s += ui->tableWidget->ScoreAt(i);
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



void TestCaseTabWidget::on_tableWidget_doubleClicked(const QModelIndex& index)
{
    int id = index.row();
    if (!index.column()) // 编辑分值
    {
        AddTestCaseDialog dialog(current_problem, AddTestCaseDialog::EditS, nullptr, -1, this, ui->tableWidget->ScoreAt(id));
        if (dialog.exec() == QDialog::Accepted)
        {
            ui->tableWidget->ChangeScore(id, dialog.GetScore());
            ui->label_score->setText(QString::number(ui->tableWidget->SumScore()));
        }
    }
    else // 编辑测试数据
    {
        TestCase point = ui->tableWidget->TestCaseAt(id);
        AddTestCaseDialog::TestCaseType testCaseType = AddTestCaseDialog::EditIOTM;
        if (ui->tableWidget->ScoreItemTopRow(id) == ui->tableWidget->ScoreItemBottomRow(id))
            testCaseType = AddTestCaseDialog::EditSIOTM;

        AddTestCaseDialog dialog(current_problem, testCaseType, &point, index.column(), this, ui->tableWidget->ScoreAt(id));

        if (dialog.exec() == QDialog::Accepted)
        {
            ui->tableWidget->ChangeTestCase(id, dialog.GetTestCase());
            if (testCaseType == AddTestCaseDialog::EditSIOTM)
            {
                ui->tableWidget->ChangeScore(id, dialog.GetScore());
                ui->label_score->setText(QString::number(ui->tableWidget->SumScore()));
            }
        }
    }
    onTestCaseSelectionChanged();
}

void TestCaseTabWidget::on_pushButton_edit_clicked()
{
    int top, bottom;
    TestCaseTable::SelectionType selectionType = ui->tableWidget->GetSelectionType(&top, &bottom);
    AddTestCaseDialog::TestCaseType testCaseType;

    switch (selectionType)
    {
    case TestCaseTable::SelectOnePackage:
    case TestCaseTable::SelectMultiplePackage:
    case TestCaseTable::SelectMultipleTestCasePackage:
        testCaseType = AddTestCaseDialog::EditSTM;
        break;
    case TestCaseTable::OtherSelection:
    case TestCaseTable::SelectMultipleSubTestCase:
        testCaseType = AddTestCaseDialog::EditTM;
        break;
    case TestCaseTable::SelectOneTestCasePackage:
        testCaseType = AddTestCaseDialog::EditSIOTM;
        break;
    case TestCaseTable::SelectOneSubTestCase:
        testCaseType = AddTestCaseDialog::EditIOTM;
        break;
    default:
        return;
    }

    if (testCaseType == AddTestCaseDialog::EditSIOTM || testCaseType == AddTestCaseDialog::EditIOTM)
    {
        TestCase point = ui->tableWidget->TestCaseAt(top);
        AddTestCaseDialog dialog(current_problem, testCaseType, &point, -1, this, ui->tableWidget->ScoreAt(top));

        if (dialog.exec() == QDialog::Accepted)
        {
            ui->tableWidget->ChangeTestCase(top, dialog.GetTestCase());
            if (testCaseType == AddTestCaseDialog::EditSIOTM)
            {
                ui->tableWidget->ChangeScore(top, dialog.GetScore());
                ui->label_score->setText(QString::number(ui->tableWidget->SumScore()));
            }
        }
    }
    else
    {
        double minT = 1e9, maxT = 0, minM = 1e9, maxM = 0;
        int minS = 1e9, maxS = 0;
        for (int i = top; i <= bottom; i++)
        {
            minT = std::min(minT, ui->tableWidget->TestCaseAt(i).TimeLimit());
            maxT = std::max(maxT, ui->tableWidget->TestCaseAt(i).TimeLimit());
            minM = std::min(minM, ui->tableWidget->TestCaseAt(i).MemoryLimit());
            maxM = std::max(maxM, ui->tableWidget->TestCaseAt(i).MemoryLimit());
            minS = std::min(minS, ui->tableWidget->ScoreAt(i));
            maxS = std::max(maxS, ui->tableWidget->ScoreAt(i));
        }
        AddTestCaseDialog dialog(current_problem, testCaseType, minT, maxT, minM, maxM, minS, maxS, this);

        if (dialog.exec() == QDialog::Accepted)
        {
            for (int i = top; i <= bottom; i++)
            {
                if (testCaseType == AddTestCaseDialog::EditSTM && ui->tableWidget->ScoreItemTopRow(i) == i)
                    ui->tableWidget->ChangeScore(i, dialog.GetScore());
                ui->tableWidget->ChangeTestCase(i, dialog.GetTestCase());
            }
            ui->label_score->setText(QString::number(ui->tableWidget->SumScore()));
        }
    }
}

void TestCaseTabWidget::on_pushButton_addTestCase_clicked()
{
    int id = 1;
    auto list = ui->tableWidget->selectedItems();
    if (list.size()) id = list.first()->row() + 2;

    const TestCase* point = nullptr;
    if (current_problem->Type() == Global::Traditional)
        point = new TestCase(1, 128, QString("%1%2.in").arg(current_problem->Name()).arg(id),
                                     QString("%1%2.out").arg(current_problem->Name()).arg(id));
    else if (current_problem->Type() == Global::AnswersOnly)
        point = new TestCase(0, 0, QString("%1%2.in").arg(current_problem->Name()).arg(id),
                                   QString("%1%2.out").arg(current_problem->Name()).arg(id),
                                   QString("%1%2.out").arg(current_problem->Name()).arg(id));

    AddTestCaseDialog dialog(current_problem, AddTestCaseDialog::AddSIOTM, point, -1, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        ui->tableWidget->AddTestCase(dialog.GetTestCase(), dialog.GetScore());
        ui->label_score->setText(QString::number(ui->tableWidget->SumScore()));
    }

    if (point) delete point;
}

void TestCaseTabWidget::on_pushButton_addSubTestCase_clicked()
{
    int id = 1;
    auto list = ui->tableWidget->selectedItems();
    if (list.size()) id = list.first()->row() + 2;

    const TestCase* point = nullptr;
    if (current_problem->Type() == Global::Traditional)
        point = new TestCase(1, 128, QString("%1%2.in").arg(current_problem->Name()).arg(id),
                                     QString("%1%2.out").arg(current_problem->Name()).arg(id));
    else if (current_problem->Type() == Global::AnswersOnly)
        point = new TestCase(0, 0, QString("%1%2.in").arg(current_problem->Name()).arg(id),
                                   QString("%1%2.out").arg(current_problem->Name()).arg(id),
                                   QString("%1%2.out").arg(current_problem->Name()).arg(id));

    AddTestCaseDialog dialog(current_problem, AddTestCaseDialog::AddIOTM, point, -1, this);
    if (dialog.exec() == QDialog::Accepted)
        ui->tableWidget->AddSubTestCase(dialog.GetTestCase());

    if (point) delete point;
}

void TestCaseTabWidget::on_pushButton_delete_clicked()
{
    ui->tableWidget->DeleteSelection();
    ui->label_score->setText(QString::number(ui->tableWidget->SumScore()));
}

void TestCaseTabWidget::on_pushButton_up_clicked()
{
    ui->tableWidget->MoveUpSelection();
}

void TestCaseTabWidget::on_pushButton_down_clicked()
{
    ui->tableWidget->MoveDownSelection();
}

void TestCaseTabWidget::on_pushButton_merge_clicked()
{
    ui->tableWidget->MergeSelection();
}

void TestCaseTabWidget::on_pushButton_split_clicked()
{
    ui->tableWidget->SplitSelection();
}

void TestCaseTabWidget::on_pushButton_reset_clicked()
{
    if (current_problem->Type() == Global::Traditional)
        current_problem->ResetTestCases(1, 128);
    else if (current_problem->Type() == Global::AnswersOnly)
        current_problem->ResetTestCases(0, 0);

    ui->tableWidget->LoadTestCases(current_problem);
    ui->label_score->setText(QString::number(ui->tableWidget->SumScore()));
}
