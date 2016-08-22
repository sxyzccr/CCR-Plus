#include "global.h"
#include "addtestcasedialog.h"
#include "ui_addtestcasedialog.h"

#include <QFile>
#include <QFileDialog>
#include <QDebug>

AddTestCaseDialog::AddTestCaseDialog(Problem* problem, TestCase* point, bool isScoreShow, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddTestCaseDialog), problem(problem), point(point), score(10)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    this->setWindowTitle("添加测试点 - " + problem->Name());

    ui->spinBox_score->setValue(score);
    ui->lineEdit_inFile->setText(point->InFile());
    ui->lineEdit_outFile->setText(point->OutFile());
    ui->lineEdit_submitFile->setText(point->SubmitFile());
    ui->spinBox_timeLim->setValue(point->TimeLimit());
    ui->spinBox_memLim->setValue(point->MemoryLimit());

    int line = 6;
    if (!isScoreShow) ui->spinBox_score->hide(), line--;
    if (problem->Type() == Global::Traditional)
    {
        ui->lineEdit_submitFile->hide();
        ui->label_4->hide();
        line--;
    }
    else
    {
        ui->spinBox_timeLim->hide();
        ui->spinBox_memLim->hide();
        ui->label_5->hide();
        ui->label_6->hide();
        line -= 2;
    }
    this->setFixedHeight((line + 1) * 23 + 12 + (line + 1) * 6 + 18);
}

AddTestCaseDialog::~AddTestCaseDialog()
{
    delete ui;
}

void AddTestCaseDialog::accept()
{
    score = ui->spinBox_score->value();
    point->SetInFile(ui->lineEdit_inFile->text());
    point->SetOutFile(ui->lineEdit_outFile->text());

    if (!ui->lineEdit_inFile->styleSheet().isEmpty())
    {
        ui->label_error->setText("无效的输入文件。");
        ui->lineEdit_inFile->setFocus();
        return;
    }

    if (!ui->lineEdit_outFile->styleSheet().isEmpty())
    {
        ui->label_error->setText("无效的输出文件。");
        ui->lineEdit_outFile->setFocus();
        return;
    }

    if (problem->Type() == Global::Traditional)
    {
        point->SetTimeLimit(ui->spinBox_timeLim->value());
        point->SetMemoryLimit(ui->spinBox_memLim->value());
    }
    else if (problem->Type() == Global::AnswersOnly)
    {
        point->SetSubmitFile(ui->lineEdit_submitFile->text());
        if (point->SubmitFile().isEmpty())
        {
            ui->label_error->setText("提交文件不能为空。");
            ui->lineEdit_submitFile->setFocus();
            return;
        }
    }
    QDialog::accept();
}



void AddTestCaseDialog::on_lineEdit_inFile_textChanged(const QString &arg1)
{
    QString dir = Global::g_contest.data_path + problem->Name() + "/";
    if (arg1.isEmpty())
    {
        ui->lineEdit_inFile->setStyleSheet("QLineEdit{color:red;}");
        ui->lineEdit_inFile->setToolTip(QString("输入文件不能为空。").arg(arg1).arg(dir));
    }
    if (!QFile::exists(dir + arg1))
    {
        ui->lineEdit_inFile->setStyleSheet("QLineEdit{color:red;}");
        ui->lineEdit_inFile->setToolTip(QString("文件 \"%1\" 不在数据目录 \"%2\" 中。").arg(arg1).arg(dir));
    }
    else
    {
        ui->lineEdit_inFile->setStyleSheet("");
        ui->lineEdit_inFile->setToolTip(QString("位置: %1").arg(dir));
    }
}

void AddTestCaseDialog::on_lineEdit_outFile_textChanged(const QString &arg1)
{
    QString dir = Global::g_contest.data_path + problem->Name() + "/";
    if (arg1.isEmpty())
    {
        ui->lineEdit_outFile->setStyleSheet("QLineEdit{color:red;}");
        ui->lineEdit_outFile->setToolTip(QString("输出文件不能为空。").arg(arg1).arg(dir));
    }
    if (!QFile::exists(dir + arg1))
    {
        ui->lineEdit_outFile->setStyleSheet("QLineEdit{color:red;}");
        ui->lineEdit_outFile->setToolTip(QString("文件 \"%1\" 不在数据目录 \"%2\" 中。").arg(arg1).arg(dir));
    }
    else
    {
        ui->lineEdit_outFile->setStyleSheet("");
        ui->lineEdit_outFile->setToolTip(QString("位置: %1").arg(dir));
    }
}

void AddTestCaseDialog::on_pushButton_browseInFile_clicked()
{
    QString fileName = QFileInfo(QFileDialog::getOpenFileName(this,
                                                              "选择输入文件",
                                                              Global::g_contest.data_path + problem->Name() + "/" + ui->lineEdit_inFile->text(),
                                                              "输入文件 (*.in);;文本文件 (*.txt);;所有文件 (*)")).fileName();
    if (!fileName.isEmpty()) ui->lineEdit_inFile->setText(fileName);
}

void AddTestCaseDialog::on_pushButton_browseOutFile_clicked()
{
    QString fileName = QFileInfo(QFileDialog::getOpenFileName(this,
                                                              "选择输出文件",
                                                              Global::g_contest.data_path + problem->Name() + "/" + ui->lineEdit_outFile->text(),
                                                              "输出文件 (*.out *.ans);;文本文件 (*.txt);;所有文件 (*)")).fileName();
    if (!fileName.isEmpty()) ui->lineEdit_outFile->setText(fileName);
}
