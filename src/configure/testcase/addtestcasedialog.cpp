#include <QFile>
#include <QDebug>
#include <QFileDialog>

#include "common/global.h"
#include "configure/testcase/addtestcasedialog.h"
#include "ui_addtestcasedialog.h"

AddTestCaseDialog::AddTestCaseDialog(const Problem* problem, const TestCase* point, TestCaseType type, int focusRow, QWidget* parent, int score) :
    QDialog(parent),
    ui(new Ui::AddTestCaseDialog), problem(problem), point(nullptr), score(score), type(type)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    ui->spinBox_score->setValue(score);
    if (type != EditScore)
    {
        ui->lineEdit_inFile->setText(point->InFile());
        ui->lineEdit_outFile->setText(point->OutFile());
        ui->lineEdit_submitFile->setText(point->SubmitFile());
        ui->spinBox_timeLim->setValue(point->TimeLimit());
        ui->spinBox_memLim->setValue(point->MemoryLimit());
    }

    int line = 6;
    switch (type)
    {
    case AddTestCase:
        this->setWindowTitle("添加测试点 - " + problem->Name());
        break;
    case AddSubTestCase:
        this->setWindowTitle("添加测试数据 - " + problem->Name());
        ui->label_1->hide();
        ui->spinBox_score->hide();
        line--;
        break;
    case EditScore:
        this->setWindowTitle("编辑分值 - " + problem->Name());
        ui->label_2->hide();
        ui->label_3->hide();
        ui->label_4->hide();
        ui->label_5->hide();
        ui->label_6->hide();
        ui->lineEdit_inFile->hide();
        ui->lineEdit_outFile->hide();
        ui->lineEdit_submitFile->hide();
        ui->spinBox_timeLim->hide();
        ui->spinBox_memLim->hide();
        ui->pushButton_browseInFile->hide();
        ui->pushButton_browseOutFile->hide();
        line -= 5;
        this->setFixedWidth(this->sizeHint().width());
        ui->spinBox_score->setFocus();
        ui->spinBox_score->selectAll();
        break;
    case EditSubTestCase:
        this->setWindowTitle("编辑测试数据 - " + problem->Name());
        ui->label_1->hide();
        ui->spinBox_score->hide();
        line--;
        break;
    }

    if (type != EditScore)
    {
        if (problem->Type() == Global::Traditional)
        {
            ui->lineEdit_submitFile->hide();
            ui->label_4->hide();
            line--;
        }
        else if (problem->Type() == Global::AnswersOnly)
        {
            ui->spinBox_timeLim->hide();
            ui->spinBox_memLim->hide();
            ui->label_5->hide();
            ui->label_6->hide();
            line -= 2;
        }
    }

    if (focusRow == 1)
        ui->lineEdit_inFile->setFocus();
    else if (focusRow == 2)
        ui->lineEdit_outFile->setFocus();
    else if (focusRow == 3 && problem->Type() == Global::AnswersOnly)
        ui->lineEdit_submitFile->setFocus();
    else if (focusRow == 3 && problem->Type() == Global::Traditional)
        ui->spinBox_timeLim->setFocus();
    else if (focusRow == 4)
        ui->spinBox_memLim->setFocus();

    this->setFixedHeight((line + 1) * 26 + 12 + (line + 1) * 6 + 18);
}

AddTestCaseDialog::~AddTestCaseDialog()
{
    if (point) delete point;
    delete ui;
}

void AddTestCaseDialog::accept()
{
    score = ui->spinBox_score->value();

    if (type == EditScore)
    {
        QDialog::accept();
        return;
    }

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
    if (problem->Type() == Global::AnswersOnly && ui->lineEdit_submitFile->text().isEmpty())
    {
        ui->label_error->setText("提交文件不能为空。");
        ui->lineEdit_submitFile->setFocus();
        return;
    }

    if (problem->Type() == Global::Traditional)
        point = new TestCase(ui->spinBox_timeLim->value(), ui->spinBox_memLim->value(), ui->lineEdit_inFile->text(), ui->lineEdit_outFile->text());
    else if (problem->Type() == Global::AnswersOnly)
        point = new TestCase(0, 0, ui->lineEdit_inFile->text(), ui->lineEdit_outFile->text(), ui->lineEdit_submitFile->text());

    QDialog::accept();
}



void AddTestCaseDialog::on_lineEdit_inFile_textChanged(const QString& text)
{
    QString dir = Global::g_contest.data_path + problem->Name() + "/";
    if (text.isEmpty())
    {
        ui->lineEdit_inFile->setStyleSheet("QLineEdit{color:red;}");
        ui->lineEdit_inFile->setToolTip(QString("输入文件不能为空。"));
    }
    else if (!QFile::exists(dir + text))
    {
        ui->lineEdit_inFile->setStyleSheet("QLineEdit{color:red;}");
        ui->lineEdit_inFile->setToolTip(QString("文件 \"%1\" 不在测试数据目录 \"%2\" 中。").arg(text).arg(dir));
    }
    else
    {
        ui->lineEdit_inFile->setStyleSheet("");
        ui->lineEdit_inFile->setToolTip(QString("位置: %1").arg(dir));
    }
}

void AddTestCaseDialog::on_lineEdit_outFile_textChanged(const QString& text)
{
    QString dir = Global::g_contest.data_path + problem->Name() + "/";
    if (text.isEmpty())
    {
        ui->lineEdit_outFile->setStyleSheet("QLineEdit{color:red;}");
        ui->lineEdit_outFile->setToolTip(QString("输出文件不能为空。"));
    }
    else if (!QFile::exists(dir + text))
    {
        ui->lineEdit_outFile->setStyleSheet("QLineEdit{color:red;}");
        ui->lineEdit_outFile->setToolTip(QString("文件 \"%1\" 不在测试数据目录 \"%2\" 中。").arg(text).arg(dir));
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
