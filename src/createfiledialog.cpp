#include "createfiledialog.h"
#include "ui_createfiledialog.h"

CreateFileDialog::CreateFileDialog(QWidget* parent, Player* ply, Problem* prob) :
    QDialog(parent),
    ui(new Ui::CreateFileDialog),
    player(ply), problem(prob)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    int t = 0;
    for (auto i : problem->compilers)
    {
        ui->listWidget->addItem(i.file);
        ui->listWidget->item(t++)->setToolTip(i.file);
    }
    ui->listWidget->setCurrentRow(0);
}

CreateFileDialog::~CreateFileDialog()
{
    delete ui;
}

void CreateFileDialog::accept()
{
    selectedFile = ui->listWidget->currentItem()->text();
    return QDialog::accept();
}

void CreateFileDialog::on_listWidget_itemDoubleClicked(QListWidgetItem*)
{
    this->accept();
}
