#include "global.h"
#include "problem.h"
#include "addcompilerdialog.h"
#include "addtestcasedialog.h"
#include "advancedconfiguredialog.h"
#include "ui_advancedconfiguredialog.h"

#include <QDebug>
#include <QMessageBox>
#include <QStandardItemModel>

using namespace std;

AdvancedConfigureDialog::AdvancedConfigureDialog(const vector<Problem*>& problems, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdvancedConfigureDialog), old_problems(problems)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

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


    ui->tableWidget_compiler->horizontalHeader()->setFixedHeight(25);
    ui->tableWidget_compiler->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->tableWidget_compiler->verticalHeader()->setSectionsMovable(true);
    ui->tableWidget_compiler->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);


    ui->tableWidget_testcase->setMinimumHeight(ui->tableWidget_testcase->verticalHeader()->defaultSectionSize() * 20 +
                                               ui->tableWidget_testcase->horizontalHeader()->height() +
                                               ui->tableWidget_testcase->frameWidth() * 2);

    connect(ui->listWidget, &QListWidget::currentItemChanged, this, &AdvancedConfigureDialog::onListWidgetCurrentItemChanged);
    connect(ui->tableWidget_testcase, &TestCaseTable::testCaseSelectionChanged, this, &AdvancedConfigureDialog::onTestCaseSelectionChanged);

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
            item->setText("自定义");
        item->setToolTip(item->text());
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget_compiler->setVerticalHeaderItem(i, item);

        item = new QTableWidgetItem(compiler->SourceFile());
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget_compiler->setItem(i, 0, item);

        item = new QTableWidgetItem(compiler->Cmd());
        ui->tableWidget_compiler->setItem(i, 1, item);
    }

    ui->tableWidget_testcase->LoadTestCases(problem);
    ui->label_score->setText(QString::number(ui->tableWidget_testcase->SumScore()));
}



void AdvancedConfigureDialog::accept()
{
    QDialog::accept();
}

void AdvancedConfigureDialog::onListWidgetCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
    current_problem = problems[ui->listWidget->row(current)];
    ui->label_problem->setText(current_problem->Name());
    loadFromProblem(current_problem);
}

void AdvancedConfigureDialog::onTestCaseSelectionChanged()
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
            if (ui->tableWidget_testcase->ScoreItemTopRow(i) == i) t++;
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


void AdvancedConfigureDialog::on_tableWidget_compiler_itemSelectionChanged()
{
    ui->pushButton_removeCompiler->setEnabled(ui->tableWidget_compiler->selectedItems().size());
}

void AdvancedConfigureDialog::on_pushButton_addCompiler_clicked()
{
    auto list = ui->tableWidget_compiler->selectedItems();
    int row;
    if (!list.size()) row = 0; else row = ui->tableWidget_compiler->visualRow(list.first()->row()) + 1;

    AddCompilerDialog dialog(current_problem, this);
    if (dialog.exec() != QDialog::Accepted) return;

    Compiler compiler(dialog.Cmd(), dialog.SourceFile(), dialog.TimeLimit());
    for (int i = 0; i < ui->tableWidget_compiler->rowCount(); i++)
        if (ui->tableWidget_compiler->item(i, 0)->text() == compiler.SourceFile())
        {
            QMessageBox::critical(this, "添加编译器失败", "源程序文件名与已有编译器冲突！");
            return;
        }

    QTableWidgetItem* item = new QTableWidgetItem;
    ui->tableWidget_compiler->insertRow(row);

    if (compiler.SourceFile().endsWith(".c"))
        item->setText("C 语言");
    else if (compiler.SourceFile().endsWith(".cpp"))
        item->setText("C++ 语言");
    else if (compiler.SourceFile().endsWith(".pas"))
        item->setText("Pascal 语言");
    else
        item->setText("自定义");
    item->setToolTip(item->text());
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget_compiler->setVerticalHeaderItem(row, item);

    item = new QTableWidgetItem(compiler.SourceFile());
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget_compiler->setItem(row, 0, item);

    item = new QTableWidgetItem(compiler.Cmd());
    ui->tableWidget_compiler->setItem(row, 1, item);

    ui->tableWidget_compiler->selectRow(row);
    if (!ui->tableWidget_compiler->hasFocus()) ui->tableWidget_compiler->setFocus();
}

void AdvancedConfigureDialog::on_pushButton_removeCompiler_clicked()
{
    auto list = ui->tableWidget_compiler->selectedItems();
    if (list.size())
    {
        int row = list.first()->row(), visualRow = ui->tableWidget_compiler->visualRow(row), gotoRow = -1, end;
        ui->tableWidget_compiler->removeRow(row);
        for (int i = 0; i < ui->tableWidget_compiler->rowCount(); i++)
        {
            if (ui->tableWidget_compiler->visualRow(i) == visualRow) gotoRow = i;
            if (ui->tableWidget_compiler->visualRow(i) == ui->tableWidget_compiler->rowCount() - 1) end = i;
        }
        if (gotoRow < 0) gotoRow = end;
        ui->tableWidget_compiler->selectRow(gotoRow);
    }
}


void AdvancedConfigureDialog::on_tableWidget_testcase_doubleClicked(const QModelIndex& index)
{
    int id = index.row();
    if (!index.column())
    {
        AddTestCaseDialog dialog(current_problem, nullptr, AddTestCaseDialog::EditScore, this, ui->tableWidget_testcase->ScoreItemAt(id)->text().toInt());
        if (dialog.exec() == QDialog::Accepted)
        {
            ui->tableWidget_testcase->ChangeScore(id, dialog.GetScore());
            ui->label_score->setText(QString::number(ui->tableWidget_testcase->SumScore()));
        }
    }
    else
    {
        TestCase* point = current_problem->TestCaseAt(id);

        AddTestCaseDialog dialog(current_problem, point, AddTestCaseDialog::EditSubTestCase, this);
        if (dialog.exec() == QDialog::Accepted)
            ui->tableWidget_testcase->ChangeTestCase(id, dialog.GetTestCase());
    }
}

void AdvancedConfigureDialog::on_pushButton_addTestCase_clicked()
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

    AddTestCaseDialog dialog(current_problem, point, AddTestCaseDialog::AddTestCase, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        ui->tableWidget_testcase->AddTestCase(dialog.GetTestCase(), dialog.GetScore());
        ui->label_score->setText(QString::number(ui->tableWidget_testcase->SumScore()));
    }
}

void AdvancedConfigureDialog::on_pushButton_addSubTestCase_clicked()
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

    AddTestCaseDialog dialog(current_problem, point, AddTestCaseDialog::AddSubTestCase, this);
    if (dialog.exec() == QDialog::Accepted)
        ui->tableWidget_testcase->AddSubTestCase(dialog.GetTestCase());
}

void AdvancedConfigureDialog::on_pushButton_removeTestCase_clicked()
{
    ui->tableWidget_testcase->RemoveSelection();
    ui->label_score->setText(QString::number(ui->tableWidget_testcase->SumScore()));
}

void AdvancedConfigureDialog::on_pushButton_up_clicked()
{
    ui->tableWidget_testcase->MoveUpSelection();
}

void AdvancedConfigureDialog::on_pushButton_down_clicked()
{
    ui->tableWidget_testcase->MoveDownSelection();
}

void AdvancedConfigureDialog::on_pushButton_merge_clicked()
{
    ui->tableWidget_testcase->MergeSelection();
}

void AdvancedConfigureDialog::on_pushButton_split_clicked()
{
    ui->tableWidget_testcase->SplitSelection();
}

void AdvancedConfigureDialog::on_pushButton_resetTestCase_clicked()
{

}
