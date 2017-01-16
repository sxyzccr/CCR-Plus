#include <QStandardItemModel>

#include "common/global.h"
#include "configure/advancedconfiguredialog.h"
#include "ui_advancedconfiguredialog.h"

AdvancedConfigureDialog::AdvancedConfigureDialog(const QList<Problem*>& problems, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdvancedConfigureDialog), old_problems(problems)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    for (int i = 0; i < problems.size(); i++)
    {
        Problem* prob = new Problem(problems[i]);
        this->problems.append(prob);
        ui->listWidget->addItem(prob->Name());
        ui->listWidget->item(i)->setToolTip(prob->Name());
    }

    QStandardItemModel* model;
    QStandardItem* item;

    model = new QStandardItemModel(ui->comboBox_type);
    item = new QStandardItem("传统型");
    item->setToolTip("传统型");
    item->setFont(ui->comboBox_type->font());
    model->appendRow(item);
    item = new QStandardItem("提交答案型");
    item->setToolTip("提交答案型");
    item->setFont(ui->comboBox_type->font());
    model->appendRow(item);
    ui->comboBox_type->setModel(model);
    ui->comboBox_type->setView(new QListView(ui->comboBox_type));

    general_configure_widget = new GeneralConfigureWidget();
    compiler_configure_widget = new CompilerConfigureWidget();
    testcase_configure_widget = new TestCaseConfigureWidget();
    ui->tabWidget->addTab(general_configure_widget, tr("通用"));
    ui->tabWidget->addTab(compiler_configure_widget, tr("编译器"));
    ui->tabWidget->addTab(testcase_configure_widget, tr("测试点"));

    connect(ui->listWidget, &QListWidget::currentItemChanged, this, &AdvancedConfigureDialog::onListWidgetCurrentItemChanged);
    ui->listWidget->setCurrentRow(0);
}

AdvancedConfigureDialog::~AdvancedConfigureDialog()
{
    delete ui;
}

void AdvancedConfigureDialog::loadFromProblem(Problem* problem)
{
    if (problem->Type() == Global::Traditional)
        ui->comboBox_type->setCurrentIndex(0);
    else if (problem->Type() == Global::AnswersOnly)
        ui->comboBox_type->setCurrentIndex(1);

    general_configure_widget->LoadFromProblem(problem);
    compiler_configure_widget->LoadFromProblem(problem);
    testcase_configure_widget->LoadFromProblem(problem);
}



void AdvancedConfigureDialog::accept()
{
    QDialog::accept();
}

void AdvancedConfigureDialog::onListWidgetCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
    current_problem = problems[ui->listWidget->row(current)];
    ui->label_problem->setText(current_problem->Name());
    loadFromProblem(current_problem);
}
