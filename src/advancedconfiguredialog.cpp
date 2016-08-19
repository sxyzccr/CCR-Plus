#include "global.h"
#include "problem.h"
#include "advancedconfiguredialog.h"
#include "ui_advancedconfiguredialog.h"

using namespace std;

AdvancedConfigureDialog::AdvancedConfigureDialog(const QStringList& list, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdvancedConfigureDialog), problem_list(list)
{
    ui->setupUi(this);

    for (int i = 0; i < problem_list.size(); i++)
    {
        QString name = problem_list[i];
        ui->listWidget->addItem(name);
        ui->listWidget->item(i)->setToolTip(name);
        if (Global::g_contest.ProblemIndex(name) == -1) ui->listWidget->item(i)->setFont(Global::BOLD_FONT);
    }

    ui->tableWidget_compiler->horizontalHeader()->setFixedHeight(22);

    ui->tableWidget_compiler->verticalHeader()->setSectionsMovable(true);
    ui->tableWidget_compiler->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);


    ui->widget_empty->hide();
    test_case_table = new TestCaseTable(ui->tab_testCase);
    ui->gridLayout_testCase->addWidget(test_case_table, 1, 0, 12, 3);

    connect(ui->listWidget, &QListWidget::currentItemChanged, this, &AdvancedConfigureDialog::onListWidgetCurrentItemChanged);
    connect(test_case_table, &TestCaseTable::testCaseSelectionChanged, this, &AdvancedConfigureDialog::onTestCaseSelectionChanged);
    connect(ui->pushButton_up,    &QPushButton::clicked, test_case_table, &TestCaseTable::MoveUpSelection);
    connect(ui->pushButton_down,  &QPushButton::clicked, test_case_table, &TestCaseTable::MoveDownSelection);
    connect(ui->pushButton_merge, &QPushButton::clicked, test_case_table, &TestCaseTable::MergeSelection);
    connect(ui->pushButton_split, &QPushButton::clicked, test_case_table, &TestCaseTable::SplitSelection);

    ui->listWidget->setCurrentRow(0);
}

AdvancedConfigureDialog::~AdvancedConfigureDialog()
{
    delete ui;
}

void AdvancedConfigureDialog::accept()
{
    QDialog::accept();
}

void AdvancedConfigureDialog::loadFromProblem(Problem* problem)
{
    test_case_table->LoadTestCases(problem);
    ui->label_score->setText(QString::number(test_case_table->SumScore()));
}

void AdvancedConfigureDialog::onListWidgetCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    ui->label_problem->setText(current->text());

    int p = Global::g_contest.ProblemIndex(current->text());
    loadFromProblem(p == -1 ? nullptr : Global::g_contest.problems[p]);
}

void AdvancedConfigureDialog::onTestCaseSelectionChanged()
{
    ui->pushButton_addTestCase->setEnabled(test_case_table->CanAddTestCase());
    ui->pushButton_addSubTestCase->setEnabled(test_case_table->CanAddSubTestCase());
    ui->pushButton_removeTestCase->setEnabled(test_case_table->CanRemoveTestCase());
    ui->pushButton_up->setEnabled(test_case_table->CanMoveUp());
    ui->pushButton_down->setEnabled(test_case_table->CanMoveDown());
    ui->pushButton_merge->setEnabled(test_case_table->CanMerge());
    ui->pushButton_split->setEnabled(test_case_table->CanSplit());

    int top, bottom;
    TestCaseTable::SelectionType type = test_case_table->GetSelectionType(&top, &bottom);
    switch (type)
    {
    case TestCaseTable::SelectOnePackage:
    case TestCaseTable::SelectOneTestCasePackage:
        ui->label_selectionInfo->setText(QString("已选择: 1 个测试点，%1 组测试数据").arg(bottom - top + 1));
        break;
    case TestCaseTable::SelectOneSubTestCase:
        ui->label_selectionInfo->setText("已选择: 1 组测试数据");
        break;
    case TestCaseTable::SelectMultiplePackage:
    case TestCaseTable::SelectMultipleTestCasePackage:
    {
        int t = 0;
        for (int i = top; i <= bottom; i++)
            if (test_case_table->ScoreItemTopRow(i) == i) t++;
        ui->label_selectionInfo->setText(QString("已选择: %1 个测试点，%2 组测试数据").arg(t).arg(bottom - top + 1));
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

void AdvancedConfigureDialog::on_pushButton_addTestCase_clicked()
{
    test_case_table->AddTestCase(new TestCase(1, 128, "a.in", "a.out"), 10);
    ui->label_score->setText(QString::number(test_case_table->SumScore()));
}

void AdvancedConfigureDialog::on_pushButton_addSubTestCase_clicked()
{
    test_case_table->AddSubTestCase(new TestCase(1, 128, "a.in", "a.out"));
}

void AdvancedConfigureDialog::on_pushButton_removeTestCase_clicked()
{
    test_case_table->RemoveSelection();
    ui->label_score->setText(QString::number(test_case_table->SumScore()));
}
