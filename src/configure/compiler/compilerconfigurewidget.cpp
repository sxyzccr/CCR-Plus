#include <QMessageBox>

#include "configure/compiler/addcompilerdialog.h"
#include "configure/compiler/compilerconfigurewidget.h"
#include "ui_compilerconfigurewidget.h"

CompilerConfigureWidget::CompilerConfigureWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CompilerConfigureWidget)
{
    ui->setupUi(this);

    ui->tableWidget_compiler->horizontalHeader()->setFixedHeight(25);
    ui->tableWidget_compiler->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->tableWidget_compiler->verticalHeader()->setSectionsMovable(true);
    ui->tableWidget_compiler->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    connect(ui->tableWidget_compiler->verticalHeader(), &QHeaderView::sectionMoved, this, [this](int /*logicalIndex*/, int oldVisualIndex, int newVisualIndex)
    {
        current_problem->MoveCompiler(oldVisualIndex, newVisualIndex);
    });
}

CompilerConfigureWidget::~CompilerConfigureWidget()
{
    delete ui;
}

void CompilerConfigureWidget::LoadFromProblem(Problem* problem)
{
    current_problem = problem;

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
}



void CompilerConfigureWidget::on_tableWidget_compiler_doubleClicked(const QModelIndex& index)
{
    int row = index.row(), visualRow = ui->tableWidget_compiler->visualRow(row);
    Compiler* compiler = current_problem->CompilerAt(visualRow);
    AddCompilerDialog dialog(current_problem, compiler, this);
    if (dialog.exec() != QDialog::Accepted) return;

    QTableWidgetItem* item = ui->tableWidget_compiler->verticalHeaderItem(row);
    if (dialog.SourceFile().endsWith(".c"))
        item->setText("C 语言");
    else if (dialog.SourceFile().endsWith(".cpp"))
        item->setText("C++ 语言");
    else if (dialog.SourceFile().endsWith(".pas"))
        item->setText("Pascal 语言");
    else
        item->setText("自定义");
    item->setToolTip(item->text());

    ui->tableWidget_compiler->item(row, 0)->setText(dialog.SourceFile());
    ui->tableWidget_compiler->item(row, 1)->setText(dialog.Cmd());
}

void CompilerConfigureWidget::on_tableWidget_compiler_itemSelectionChanged()
{
    ui->pushButton_removeCompiler->setEnabled(ui->tableWidget_compiler->selectedItems().size());
}

void CompilerConfigureWidget::on_pushButton_addCompiler_clicked()
{
    auto list = ui->tableWidget_compiler->selectedItems();
    int row;
    if (!list.size()) row = 0; else row = ui->tableWidget_compiler->visualRow(list.first()->row()) + 1;

    AddCompilerDialog dialog(current_problem, nullptr, this);
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

    current_problem->InsertCompiler(row, new Compiler(compiler));
}

void CompilerConfigureWidget::on_pushButton_removeCompiler_clicked()
{
    auto list = ui->tableWidget_compiler->selectedItems();
    if (list.size())
    {
        int row = list.first()->row(), visualRow = ui->tableWidget_compiler->visualRow(row), gotoRow = -1, end;
        ui->tableWidget_compiler->removeRow(row);
        current_problem->RemoveCompiler(visualRow);
        for (int i = 0; i < ui->tableWidget_compiler->rowCount(); i++)
        {
            if (ui->tableWidget_compiler->visualRow(i) == visualRow) gotoRow = i;
            if (ui->tableWidget_compiler->visualRow(i) == ui->tableWidget_compiler->rowCount() - 1) end = i;
        }
        if (gotoRow < 0) gotoRow = end;
        ui->tableWidget_compiler->selectRow(gotoRow);
    }
}

void CompilerConfigureWidget::on_pushButton_resetCompiler_clicked()
{

}
