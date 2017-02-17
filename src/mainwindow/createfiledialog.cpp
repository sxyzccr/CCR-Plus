#include "mainwindow/createfiledialog.h"
#include "ui_createfiledialog.h"

CreateFileDialog::CreateFileDialog(const Player* player, const Problem* problem, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::CreateFileDialog),
    player(player), problem(problem)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    for (int i = 0; i < problem->CompilerCount(); i++)
    {
        const Compiler* compiler = problem->CompilerAt(i);
        ui->listWidget->addItem(compiler->SourceFile());
        ui->listWidget->item(i)->setToolTip(compiler->SourceFile());
    }
    ui->listWidget->setCurrentRow(0);

    this->setMinimumSize(this->sizeHint());
}

CreateFileDialog::~CreateFileDialog()
{
    delete ui;
}

void CreateFileDialog::accept()
{
    selected_file = ui->listWidget->currentItem()->text();
    return QDialog::accept();
}

void CreateFileDialog::on_listWidget_itemDoubleClicked(QListWidgetItem*)
{
    this->accept();
}
