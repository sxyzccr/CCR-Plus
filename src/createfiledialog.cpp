#include "createfiledialog.h"
#include "ui_createfiledialog.h"

CreateFileDialog::CreateFileDialog(QWidget* parent, Player* ply, Problem* prob) :
    QDialog(parent),
    ui(new Ui::CreateFileDialog),
    player(ply), problem(prob)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    for (int i = 0; i < problem->CompilerCount(); i++)
    {
        Compiler* compiler = problem->CompilerAt(i);
        ui->listWidget->addItem(compiler->SourceFile());
        ui->listWidget->item(i)->setToolTip(compiler->SourceFile());
    }
    ui->listWidget->setCurrentRow(0);
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
