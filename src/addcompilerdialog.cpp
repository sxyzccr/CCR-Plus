#include "addcompilerdialog.h"
#include "ui_addcompilerdialog.h"

#include <QDebug>

AddCompilerDialog::AddCompilerDialog(Problem* problem, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddCompilerDialog), problem(problem)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    ui->comboBox_type->setCurrentIndex(0);
    this->setFixedHeight(this->sizeHint().height());
}

AddCompilerDialog::~AddCompilerDialog()
{
    delete ui;
}

void AddCompilerDialog::accept()
{
    file = ui->lineEdit_file->text();
    cmd = ui->lineEdit_cmd->text();
    time_lim = ui->spinBox_timeLim->value();
    if (file.isEmpty())
    {
        ui->label_error->setText("源程序文件名为空。");
        ui->lineEdit_file->setFocus();
        return;
    }
    switch (ui->comboBox_type->currentIndex())
    {
    case 0: // C
        if (!file.endsWith(".c"))
        {
            ui->label_error->setText("源程序扩展名要求为\".c\"");
            ui->lineEdit_file->setFocus();
            return;
        }
        break;
    case 1: // C++
        if (!file.endsWith(".cpp"))
        {
            ui->label_error->setText("源程序扩展名要求为\".cpp\"");
            ui->lineEdit_file->setFocus();
            return;
        }
        break;
    case 2: // Pascal
        if (!file.endsWith(".pas"))
        {
            ui->label_error->setText("源程序扩展名要求为\".pas\"");
            ui->lineEdit_file->setFocus();
            return;
        }
        break;
    }
    QDialog::accept();
}



void AddCompilerDialog::onChangeCmd()
{
    QString file = ui->lineEdit_file->text();
    QString args = ui->lineEdit_args->text();
    switch (ui->comboBox_type->currentIndex())
    {
    case 0: // C
        ui->lineEdit_cmd->setText(QString("gcc -o %1 %2 %3").arg(problem->ExecutableFile()).arg(file).arg(args));
        break;
    case 1: // C++
        ui->lineEdit_cmd->setText(QString("g++ -o %1 %2 %3").arg(problem->ExecutableFile()).arg(file).arg(args));
        break;
    case 2: // Pascal
        ui->lineEdit_cmd->setText(QString("fpc %1 %2").arg(file).arg(args));
        break;
    }
}

void AddCompilerDialog::on_comboBox_type_currentIndexChanged(int index)
{
    ui->label_error->setText("");
    switch (index)
    {
    case 0: // C
        ui->lineEdit_file->setText(problem->Name() + ".c");
        ui->lineEdit_args->setText("-lm -static");
        ui->lineEdit_cmd->setReadOnly(true);
        ui->label_4->setEnabled(true);
        ui->lineEdit_args->setEnabled(true);
        break;
    case 1: // C++
        ui->lineEdit_file->setText(problem->Name() + ".cpp");
        ui->lineEdit_args->setText("-lm -static");
        ui->lineEdit_cmd->setReadOnly(true);
        ui->label_4->setEnabled(true);
        ui->lineEdit_args->setEnabled(true);
        break;
    case 2: // Pascal
        ui->lineEdit_file->setText(problem->Name() + ".pas");
        ui->lineEdit_args->setText("");
        ui->lineEdit_cmd->setReadOnly(true);
        ui->label_4->setEnabled(true);
        ui->lineEdit_args->setEnabled(true);
        break;
    case 3: // Other
        ui->lineEdit_file->setText("");
        ui->lineEdit_args->setText("");
        ui->lineEdit_cmd->setText("");
        ui->lineEdit_cmd->setReadOnly(false);
        ui->label_4->setEnabled(false);
        ui->lineEdit_args->setEnabled(false);
        break;
    }
}

void AddCompilerDialog::on_lineEdit_file_textChanged(const QString& /*arg1*/)
{
    onChangeCmd();
}

void AddCompilerDialog::on_lineEdit_args_textChanged(const QString& /*arg1*/)
{
    onChangeCmd();
}
