#include "global.h"
#include "problem.h"
#include "advancedconfiguredialog.h"
#include "ui_advancedconfiguredialog.h"

#include <QDebug>
#include <QStandardItemModel>

using namespace std;

AdvancedConfigureDialog::AdvancedConfigureDialog(const vector<Problem*>& problems, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdvancedConfigureDialog), old_problems(problems)
{
    ui->setupUi(this);

    for (int i = 0; i < problems.size(); i++)
    {
        Problem* prob = new Problem(problems[i]);
        this->problems.push_back(prob);
        ui->listWidget->addItem(prob->Name());
        ui->listWidget->item(i)->setToolTip(prob->Name());
    }

    QStandardItemModel* model;
    QStandardItem* item;

    model = new QStandardItemModel(ui->comboBox_type);
    item = new QStandardItem("传统型");
    item->setToolTip("传统型");
    item->setFont(ui->comboBox_type->font());
    model->appendRow(item);
    item = new QStandardItem("提交答案型");
    item->setToolTip("提交答案型");
    item->setFont(ui->comboBox_type->font());
    model->appendRow(item);
    ui->comboBox_type->setModel(model);
    ui->comboBox_type->setView(new QListView(ui->comboBox_type));

    model = new QStandardItemModel(ui->comboBox_internal);
    auto& internal_checker = Problem::INTERNAL_CHECKER_MAP;
    for (auto i : internal_checker)
    {
        auto checker = i.second;
        item = new QStandardItem(checker.first);
        item->setToolTip(checker.second);
        model->appendRow(item);
    }
    ui->comboBox_internal->setModel(model);
    ui->comboBox_internal->setView(new QListView(ui->comboBox_internal));


    ui->tableWidget_compiler->horizontalHeader()->setFixedHeight(22);
    ui->tableWidget_compiler->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->tableWidget_compiler->verticalHeader()->setSectionsMovable(true);
    ui->tableWidget_compiler->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);


    ui->widget_empty->hide();
    test_case_table = new TestCaseTable(ui->tab_testCase);
    test_case_table->setMinimumHeight(test_case_table->verticalHeader()->defaultSectionSize() * 20 +
                                      test_case_table->horizontalHeader()->height() +
                                      test_case_table->frameWidth() * 2);
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

void AdvancedConfigureDialog::loadFromProblem(Problem* problem)
{
    if (problem->Type() == Global::Traditional)
        ui->comboBox_type->setCurrentIndex(0);
    else if (problem->Type() == Global::AnswersOnly)
        ui->comboBox_type->setCurrentIndex(1);

    ui->lineEdit_dir->setText(problem->Directory());
    ui->spinBox_codeLim->setValue(problem->CodeLengthLimit());

    ui->lineEdit_exe->setText(Problem::RemoveFileExtension(problem->ExecutableFile()));
    ui->lineEdit_inFile->setText(problem->InFile());
    ui->lineEdit_outFile->setText(problem->OutFile());

    ui->comboBox_custom->clear();
    ui->comboBox_internal->setCurrentIndex(0);
    QStandardItemModel* model = new QStandardItemModel(ui->comboBox_custom);
    QStringList dirs = { QDir().currentPath() + "/checker",
                         Global::g_contest.data_path + problem->Name(),
                       };
    for (auto dir : dirs)
    {
#ifdef Q_OS_WIN
        QStringList list = QDir(dir).entryList(QDir::Files);
#else
        QStringList list = QDir(dir).entryList(QDir::Files | QDir::Executable);
#endif
        for (auto checker : list)
        {
#ifdef Q_OS_WIN
            if (!checker.endsWith(".exe")) continue;
#endif
            if (Problem::IsInternalChecker(checker)) continue;

            QStandardItem* item = new QStandardItem(checker);
            item->setToolTip(QString("%1 (位置: %2)").arg(checker, dir));
            model->appendRow(item);
        }
    }
    ui->comboBox_custom->setModel(model);
    ui->comboBox_custom->setView(new QListView(ui->comboBox_custom));

    if (Problem::IsInternalChecker(problem->Checker()))
    {
        ui->radioButton_internal->setChecked(true);
        ui->radioButton_custom->setChecked(false);
        ui->comboBox_internal->setEnabled(true);
        ui->comboBox_custom->setEnabled(false);
        ui->comboBox_internal->setCurrentIndex(ui->comboBox_internal->findText(problem->InternalCheckerName()));
    }
    else
    {
        ui->radioButton_internal->setChecked(false);
        ui->radioButton_custom->setChecked(true);
        ui->comboBox_internal->setEnabled(false);
        ui->comboBox_custom->setEnabled(true);
        ui->comboBox_custom->setCurrentIndex(ui->comboBox_custom->findText(problem->Checker()));
    }
    ui->spinBox_checkerTimeLim->setValue(problem->CheckerTimeLimit());

    ui->tableWidget_compiler->clearContents();
    ui->tableWidget_compiler->setRowCount(0);
    for (int i = 0; i < problem->CompilerCount(); i++)
    {
        Compiler* compiler = problem->CompilerAt(i);
        QTableWidgetItem* item;
        ui->tableWidget_compiler->insertRow(i);

        item = new QTableWidgetItem;
        if (compiler->SourceFile().endsWith(".c"))
            item->setText("C 语言");
        else if (compiler->SourceFile().endsWith(".cpp"))
            item->setText("C++ 语言");
        else if (compiler->SourceFile().endsWith(".pas"))
            item->setText("Pascal 语言");
        else
            item->setText("其他语言");
        item->setToolTip(item->text());
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget_compiler->setVerticalHeaderItem(i, item);

        item = new QTableWidgetItem(compiler->SourceFile());
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget_compiler->setItem(i, 0, item);

        item = new QTableWidgetItem(compiler->Cmd());
        ui->tableWidget_compiler->setItem(i, 1, item);
    }

    ui->label_score->setText(QString::number(test_case_table->SumScore()));
    test_case_table->LoadTestCases(problem);
}



void AdvancedConfigureDialog::accept()
{
    QDialog::accept();
}

void AdvancedConfigureDialog::onListWidgetCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    current_problem = problems[ui->listWidget->row(current)];
    ui->label_problem->setText(current_problem->Name());
    loadFromProblem(current_problem);
    qDebug()<<test_case_table->sizeHint().height()<<' '<<test_case_table->height();
}

void AdvancedConfigureDialog::on_radioButton_internal_clicked()
{
    ui->radioButton_internal->setChecked(true);
    ui->radioButton_custom->setChecked(false);
    ui->comboBox_internal->setEnabled(true);
    ui->comboBox_custom->setEnabled(false);
    ui->comboBox_internal->setCurrentIndex(0);
}

void AdvancedConfigureDialog::on_radioButton_custom_clicked()
{
    ui->radioButton_internal->setChecked(false);
    ui->radioButton_custom->setChecked(true);
    ui->comboBox_internal->setEnabled(false);
    ui->comboBox_custom->setEnabled(true);
    ui->comboBox_custom->setCurrentIndex(0);
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

void AdvancedConfigureDialog::on_pushButton_resetSubmit_clicked()
{
    ui->lineEdit_dir->setText(current_problem->Name());
    ui->spinBox_codeLim->setValue(100);
}

void AdvancedConfigureDialog::on_pushButton_resetRun_clicked()
{
    ui->lineEdit_exe->setText(current_problem->Name());
    ui->lineEdit_inFile->setText(current_problem->Name() + ".in");
    ui->lineEdit_outFile->setText(current_problem->Name() + ".out");
}

void AdvancedConfigureDialog::on_pushButton_resetChecker_clicked()
{
    on_radioButton_internal_clicked();
    ui->spinBox_checkerTimeLim->setValue(10);
}
