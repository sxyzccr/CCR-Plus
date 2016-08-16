#include "global.h"
#include "advancedconfiguredialog.h"
#include "ui_advancedconfiguredialog.h"

#include <QDebug>

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
    ui->gridLayout_testCase->addWidget(test_case_table, 1, 0, 12, 2);

    connect(ui->listWidget, &QListWidget::currentItemChanged, this, &AdvancedConfigureDialog::onListWidgetCurrentItemChanged);
    connect(test_case_table, &TestCaseTable::testCaseSelectionChanged, this, &AdvancedConfigureDialog::onSetPushButtonsEnable);

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

    if (problem == nullptr)
        ui->label_score->setText("0");
    else
        ui->label_score->setText(QString::number(problem->Score()));
}

void AdvancedConfigureDialog::onListWidgetCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    ui->label_problem->setText(current->text());

    int p = Global::g_contest.ProblemIndex(current->text());
    loadFromProblem(p == -1 ? nullptr : Global::g_contest.problems[p]);
}

void AdvancedConfigureDialog::onSetPushButtonsEnable()
{
    ui->pushButton_addTestCase->setEnabled(test_case_table->CanAddTestCase());
    ui->pushButton_addSubTestCase->setEnabled(test_case_table->CanAddSubTestCase());
    ui->pushButton_removeTestCase->setEnabled(test_case_table->CanRemoveTestCase());
    ui->pushButton_up->setEnabled(test_case_table->CanMoveUp());
    ui->pushButton_down->setEnabled(test_case_table->CanMoveDown());
    ui->pushButton_merge->setEnabled(test_case_table->CanMerge());
    ui->pushButton_split->setEnabled(test_case_table->CanSplit());
}

void AdvancedConfigureDialog::on_pushButton_merge_clicked()
{
    test_case_table->MergeSelection();
}

void AdvancedConfigureDialog::on_pushButton_split_clicked()
{
    test_case_table->SplitSelection();
}
