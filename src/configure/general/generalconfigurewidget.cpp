#include <QListView>
#include <QStandardItemModel>

#include "common/global.h"
#include "configure/general/generalconfigurewidget.h"
#include "ui_generalconfigurewidget.h"

GeneralConfigureWidget::GeneralConfigureWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::GeneralConfigureWidget)
{
    ui->setupUi(this);

    QStandardItemModel* model;
    QStandardItem* item;

    model = new QStandardItemModel(ui->comboBox_internal);
    auto& internal_checker = Problem::INTERNAL_CHECKER_MAP;
    for (auto checker : internal_checker)
    {
        item = new QStandardItem(checker.first);
        item->setToolTip(checker.second);
        model->appendRow(item);
    }
    ui->comboBox_internal->setModel(model);
    ui->comboBox_internal->setView(new QListView(ui->comboBox_internal));
}

GeneralConfigureWidget::~GeneralConfigureWidget()
{
    delete ui;
}

void GeneralConfigureWidget::LoadFromProblem(Problem* problem)
{
    current_problem = problem;

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
        ui->comboBox_internal->setCurrentText(problem->InternalCheckerName());
    }
    else
    {
        ui->radioButton_internal->setChecked(false);
        ui->radioButton_custom->setChecked(true);
        ui->comboBox_internal->setEnabled(false);
        ui->comboBox_custom->setEnabled(true);
        ui->comboBox_custom->setCurrentText(problem->Checker());
    }
    ui->spinBox_checkerTimeLim->setValue(problem->CheckerTimeLimit());
}



void GeneralConfigureWidget::on_radioButton_internal_clicked()
{
    ui->radioButton_internal->setChecked(true);
    ui->radioButton_custom->setChecked(false);
    ui->comboBox_internal->setEnabled(true);
    ui->comboBox_custom->setEnabled(false);
    ui->comboBox_internal->setCurrentIndex(0);
}

void GeneralConfigureWidget::on_radioButton_custom_clicked()
{
    ui->radioButton_internal->setChecked(false);
    ui->radioButton_custom->setChecked(true);
    ui->comboBox_internal->setEnabled(false);
    ui->comboBox_custom->setEnabled(true);
    ui->comboBox_custom->setCurrentIndex(0);
}

void GeneralConfigureWidget::on_pushButton_resetSubmit_clicked()
{
    ui->lineEdit_dir->setText(current_problem->Name());
    ui->spinBox_codeLim->setValue(100);
}

void GeneralConfigureWidget::on_pushButton_resetRun_clicked()
{
    ui->lineEdit_exe->setText(current_problem->Name());
    ui->lineEdit_inFile->setText(current_problem->Name() + ".in");
    ui->lineEdit_outFile->setText(current_problem->Name() + ".out");
}

void GeneralConfigureWidget::on_pushButton_resetChecker_clicked()
{
    on_radioButton_internal_clicked();
    ui->spinBox_checkerTimeLim->setValue(10);
}
