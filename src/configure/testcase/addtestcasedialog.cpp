#include <QFile>
#include <QDebug>
#include <QFileDialog>

#include "common/global.h"
#include "configure/testcase/addtestcasedialog.h"
#include "ui_addtestcasedialog.h"

AddTestCaseDialog::AddTestCaseDialog(const Problem* problem, TestCaseType type, const TestCase* point, int focusRow, QWidget* parent, int score) :
    QDialog(parent),
    ui(new Ui::AddTestCaseDialog), problem(problem), point(nullptr), score(score), type(type)
{
    Q_ASSERT(type == AddIOTM || type == AddSIOTM || type == EditS || type == EditIOTM || type == EditSIOTM);

    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    ui->spinBox_score->setValue(score);
    if (type != EditS)
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
    case AddSIOTM:
        this->setWindowTitle("添加测试点 - " + problem->Name());
        break;
    case AddIOTM:
        this->setWindowTitle("添加测试数据 - " + problem->Name());
        ui->label_1->hide();
        ui->spinBox_score->hide();
        line--;
        break;
    case EditSIOTM:
        this->setWindowTitle("编辑测试点 - " + problem->Name());
        break;
    case EditIOTM:
        this->setWindowTitle("编辑测试数据 - " + problem->Name());
        ui->label_1->hide();
        ui->spinBox_score->hide();
        line--;
        break;
    case EditS:
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
    default:
        break;
    }

    if (type != EditS)
    {
        if (problem->Type() == Global::Traditional)
        {
            ui->label_4->hide();
            ui->lineEdit_submitFile->hide();
            line--;
        }
        else if (problem->Type() == Global::AnswersOnly)
        {
            ui->label_5->hide();
            ui->label_6->hide();
            ui->spinBox_timeLim->hide();
            ui->spinBox_memLim->hide();
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

    this->setFixedHeight((line + 1) * 26 + 18 + (line + 1) * 6 + 18);
}

AddTestCaseDialog::AddTestCaseDialog(const Problem* problem, AddTestCaseDialog::TestCaseType type,
                                     double minTime, double maxTime, double minMemory, double maxMemory,
                                     int minScore, int maxScore, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::AddTestCaseDialog), problem(problem), point(nullptr), score(0), type(type), load_finished(false)
{
    Q_ASSERT(type == EditTM || type == EditSTM);

    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    ui->label_2->hide();
    ui->label_3->hide();
    ui->label_4->hide();
    ui->lineEdit_inFile->hide();
    ui->lineEdit_outFile->hide();
    ui->lineEdit_submitFile->hide();
    ui->pushButton_browseInFile->hide();
    ui->pushButton_browseOutFile->hide();
    int line = 3;
    if (type == EditTM)
    {
        ui->label_1->hide();
        ui->spinBox_score->hide();
        line--;
    }
    if (problem->Type() == Global::AnswersOnly)
    {
        ui->label_5->hide();
        ui->label_6->hide();
        ui->spinBox_timeLim->hide();
        ui->spinBox_memLim->hide();
        line -= 2;
    }

    ui->spinBox_score->setValue(minScore);
    ui->spinBox_timeLim->setValue(minTime);
    ui->spinBox_memLim->setValue(minMemory);
    if (minScore != maxScore)
    {
        ui->spinBox_score->setMinimum(0);
        ui->spinBox_score->setValue(0);
        ui->spinBox_score->setSpecialValueText(QString("%1 ~ %2").arg(minScore).arg(maxScore));
        ui->spinBox_score->setFixedWidth(ui->spinBox_score->sizeHint().width());
    }
    if (minTime != maxTime)
    {
        ui->spinBox_timeLim->setMinimum(0);
        ui->spinBox_timeLim->setValue(0);
        ui->spinBox_timeLim->setSpecialValueText(QString("%1 ~ %2 s").arg(minTime).arg(maxTime));
        ui->spinBox_timeLim->setFixedWidth(ui->spinBox_timeLim->sizeHint().width());
    }
    if (minMemory != maxMemory)
    {
        ui->spinBox_memLim->setMinimum(0);
        ui->spinBox_memLim->setValue(0);
        ui->spinBox_memLim->setSpecialValueText(QString("%1 ~ %2 MB").arg(minMemory).arg(maxMemory));
        ui->spinBox_memLim->setFixedWidth(ui->spinBox_memLim->sizeHint().width());
    }

    this->setFixedWidth(this->sizeHint().width());
    this->setFixedHeight((line + 1) * 26 + 18 + (line + 1) * 6 + 18);
    load_finished = true;
}

AddTestCaseDialog::~AddTestCaseDialog()
{
    if (point) delete point;
    delete ui;
}

void AddTestCaseDialog::accept()
{
    score = ui->spinBox_score->specialValueText().isEmpty() ? ui->spinBox_score->value() : -1;

    if (type == EditS)
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
    if (!ui->lineEdit_submitFile->styleSheet().isEmpty())
    {
        ui->label_error->setText("无效的提交文件。");
        ui->lineEdit_submitFile->setFocus();
        return;
    }

    if (problem->Type() == Global::Traditional)
        point = new TestCase(ui->spinBox_timeLim->specialValueText().isEmpty() ? ui->spinBox_timeLim->value() : -1,
                             ui->spinBox_memLim->specialValueText().isEmpty() ? ui->spinBox_memLim->value() : -1,
                             ui->lineEdit_inFile->text(), ui->lineEdit_outFile->text());
    else if (problem->Type() == Global::AnswersOnly)
        point = new TestCase(0, 0, ui->lineEdit_inFile->text(), ui->lineEdit_outFile->text(), ui->lineEdit_submitFile->text());

    QDialog::accept();
}



void AddTestCaseDialog::on_lineEdit_inFile_textChanged(const QString& text)
{
    QString dir = Global::g_contest.data_path + problem->Name() + "/";
    QString msg = Problem::CheckFileNameValid(text);
    if (!QFile::exists(dir + text))
    {
        ui->lineEdit_inFile->setStyleSheet("QLineEdit{color:red;}");
        ui->lineEdit_inFile->setToolTip(QString("文件 \"%1\" 不在测试数据目录 \"%2\" 中。").arg(text).arg(dir));
    }
    else if (!msg.isEmpty())
    {
        ui->lineEdit_inFile->setStyleSheet("QLineEdit{color:red;}");
        ui->lineEdit_inFile->setToolTip(msg.arg("输入文件"));
    }
    else
    {
        ui->lineEdit_inFile->setStyleSheet("");
        ui->lineEdit_inFile->setToolTip(QString("位置: %1").arg(dir));
    }
    ui->label_error->setText("");
}

void AddTestCaseDialog::on_lineEdit_outFile_textChanged(const QString& text)
{
    QString dir = Global::g_contest.data_path + problem->Name() + "/";
    QString msg = Problem::CheckFileNameValid(text);
    if (!QFile::exists(dir + text))
    {
        ui->lineEdit_outFile->setStyleSheet("QLineEdit{color:red;}");
        ui->lineEdit_outFile->setToolTip(QString("文件 \"%1\" 不在测试数据目录 \"%2\" 中。").arg(text).arg(dir));
    }
    else if (!msg.isEmpty())
    {
        ui->lineEdit_outFile->setStyleSheet("QLineEdit{color:red;}");
        ui->lineEdit_outFile->setToolTip(msg.arg("输出文件"));
    }
    else
    {
        ui->lineEdit_outFile->setStyleSheet("");
        ui->lineEdit_outFile->setToolTip(QString("位置: %1").arg(dir));
    }
    ui->label_error->setText("");
}

void AddTestCaseDialog::on_lineEdit_submitFile_textChanged(const QString& text)
{
    QString msg = Problem::CheckFileNameValid(text);
    if (!msg.isEmpty())
    {
        ui->lineEdit_submitFile->setStyleSheet("QLineEdit{color:red;}");
        ui->lineEdit_submitFile->setToolTip(msg.arg("提交文件"));
    }
    else
    {
        ui->lineEdit_submitFile->setStyleSheet("");
        ui->lineEdit_submitFile->setToolTip("");
    }
    ui->label_error->setText("");
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

void AddTestCaseDialog::on_spinBox_score_valueChanged(int)
{
    if (load_finished) ui->spinBox_score->setSpecialValueText("");
}

void AddTestCaseDialog::on_spinBox_timeLim_valueChanged(double)
{
    if (load_finished)
    {
        ui->spinBox_timeLim->setSpecialValueText("");
        ui->spinBox_timeLim->setFixedWidth(85);
    }
}

void AddTestCaseDialog::on_spinBox_memLim_valueChanged(double)
{
    if (load_finished)
    {
        ui->spinBox_memLim->setSpecialValueText("");
        ui->spinBox_memLim->setFixedWidth(85);
    }
}
