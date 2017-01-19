#include <QMessageBox>

#include "configure/compiler/addcompilerdialog.h"
#include "configure/compiler/compilertabwidget.h"
#include "ui_compilertabwidget.h"

CompilerTabWidget::CompilerTabWidget(QWidget *parent) :
    ConfigureTabWidget(parent),
    ui(new Ui::CompilerTabWidget)
{
    ui->setupUi(this);

    ui->tableWidget->horizontalHeader()->setFixedHeight(25);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->tableWidget->verticalHeader()->setSectionsMovable(true);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    connect(ui->tableWidget->verticalHeader(), &QHeaderView::sectionMoved, this, [this](int /*logicalIndex*/, int oldVisualIndex, int newVisualIndex)
    {
        current_problem->MoveCompiler(oldVisualIndex, newVisualIndex);
    });
}

CompilerTabWidget::~CompilerTabWidget()
{
    delete ui;
}

void CompilerTabWidget::ShowProblemConfiguration(Problem* problem)
{
    current_problem = problem;

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    for (int i = 0; i < problem->CompilerCount(); i++)
    {
        Compiler* compiler = problem->CompilerAt(i);
        QTableWidgetItem* item;
        ui->tableWidget->insertRow(i);

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
        ui->tableWidget->setVerticalHeaderItem(i, item);

        item = new QTableWidgetItem(compiler->SourceFile());
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(i, 0, item);

        item = new QTableWidgetItem(compiler->Cmd());
        ui->tableWidget->setItem(i, 1, item);
    }
}

void CompilerTabWidget::Reset()
{
    on_pushButton_reset_clicked();
}



void CompilerTabWidget::on_tableWidget_doubleClicked(const QModelIndex& index)
{
    int row = index.row(), visualRow = ui->tableWidget->visualRow(row);
    Compiler* compiler = current_problem->CompilerAt(visualRow);
    AddCompilerDialog dialog(current_problem, compiler, index.column(), this);
    if (dialog.exec() != QDialog::Accepted) return;
    *compiler = dialog.GetCompiler();

    QTableWidgetItem* item = ui->tableWidget->verticalHeaderItem(row);
    if (compiler->SourceFile().endsWith(".c"))
        item->setText("C 语言");
    else if (compiler->SourceFile().endsWith(".cpp"))
        item->setText("C++ 语言");
    else if (compiler->SourceFile().endsWith(".pas"))
        item->setText("Pascal 语言");
    else
        item->setText("自定义");
    item->setToolTip(item->text());

    ui->tableWidget->item(row, 0)->setText(compiler->SourceFile());
    ui->tableWidget->item(row, 1)->setText(compiler->Cmd());
}

void CompilerTabWidget::on_tableWidget_itemSelectionChanged()
{
    ui->pushButton_delete->setEnabled(ui->tableWidget->selectedItems().size());
}

void CompilerTabWidget::on_pushButton_add_clicked()
{
    auto list = ui->tableWidget->selectedItems();
    int row;
    if (!list.size()) row = 0; else row = ui->tableWidget->visualRow(list.first()->row()) + 1;

    AddCompilerDialog dialog(current_problem, nullptr, -1, this);
    if (dialog.exec() != QDialog::Accepted) return;

    Compiler* compiler = new Compiler(dialog.GetCompiler());
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
        if (ui->tableWidget->item(i, 0)->text() == compiler->SourceFile())
        {
            QMessageBox::critical(this, "添加编译器失败", "源程序文件名与已有编译器冲突！");
            return;
        }

    QTableWidgetItem* item = new QTableWidgetItem;
    ui->tableWidget->insertRow(row);

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
    ui->tableWidget->setVerticalHeaderItem(row, item);

    item = new QTableWidgetItem(compiler->SourceFile());
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(row, 0, item);

    item = new QTableWidgetItem(compiler->Cmd());
    ui->tableWidget->setItem(row, 1, item);

    ui->tableWidget->selectRow(row);
    if (!ui->tableWidget->hasFocus()) ui->tableWidget->setFocus();

    current_problem->InsertCompiler(row, compiler);
}

void CompilerTabWidget::on_pushButton_delete_clicked()
{
    auto list = ui->tableWidget->selectedItems();
    if (list.size())
    {
        int row = list.first()->row(), visualRow = ui->tableWidget->visualRow(row), gotoRow = -1, end;
        ui->tableWidget->removeRow(row);
        current_problem->DeleteCompiler(visualRow);
        for (int i = 0; i < ui->tableWidget->rowCount(); i++)
        {
            if (ui->tableWidget->visualRow(i) == visualRow) gotoRow = i;
            if (ui->tableWidget->visualRow(i) == ui->tableWidget->rowCount() - 1) end = i;
        }
        if (gotoRow < 0) gotoRow = end;
        ui->tableWidget->selectRow(gotoRow);
    }
}

void CompilerTabWidget::on_pushButton_reset_clicked()
{
    current_problem->ResetCompilers();
    ShowProblemConfiguration(current_problem);
}
