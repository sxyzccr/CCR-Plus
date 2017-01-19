#include <QStandardItemModel>

#include "common/global.h"
#include "configure/advancedconfiguredialog.h"
#include "configure/general/generaltabwidget.h"
#include "configure/compiler/compilertabwidget.h"
#include "configure/testcase/testcasetabwidget.h"
#include "ui_advancedconfiguredialog.h"

AdvancedConfigureDialog::AdvancedConfigureDialog(const QList<Problem*>& problems, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdvancedConfigureDialog), current_problem(nullptr), old_problems(problems), load_finished(false)
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

    ui->tabWidget->addTab(new GeneralTabWidget(), tr("通用"));
    ui->tabWidget->addTab(new CompilerTabWidget(), tr("编译器"));
    ui->tabWidget->addTab(new TestCaseTabWidget(), tr("测试点"));

    connect(ui->listWidget, &QListWidget::currentItemChanged, this, &AdvancedConfigureDialog::onListWidgetCurrentItemChanged);
    ui->listWidget->setCurrentRow(0);
}

AdvancedConfigureDialog::~AdvancedConfigureDialog()
{
    delete ui;
}

void AdvancedConfigureDialog::loadFromProblem(Problem* problem)
{
    load_finished = false;

    if (problem->Type() == Global::Traditional)
        ui->comboBox_type->setCurrentIndex(0);
    else if (problem->Type() == Global::AnswersOnly)
        ui->comboBox_type->setCurrentIndex(1);

    for (int i = 0; i < ui->tabWidget->count(); i++)
        static_cast<ConfigureTabWidget*>(ui->tabWidget->widget(i))->ShowProblemConfiguration(problem);

    load_finished = true;
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



void AdvancedConfigureDialog::on_pushButton_reset_clicked()
{
    for (int i = 0; i < ui->tabWidget->count(); i++)
        static_cast<ConfigureTabWidget*>(ui->tabWidget->widget(i))->Reset();
}

void AdvancedConfigureDialog::on_comboBox_type_currentIndexChanged(int index)
{
    if (!load_finished) return;

    Global::ProblemType type;
    if (!index) type = Global::Traditional;
    else if (index == 1) type = Global::AnswersOnly;

    static_cast<TestCaseTabWidget*>(ui->tabWidget->widget(2))->ChacheConfiguration();
    current_problem->ChangeProblemType(type);

    for (int i = 0; i < ui->tabWidget->count(); i++)
        static_cast<ConfigureTabWidget*>(ui->tabWidget->widget(i))->ChangeProblemType(type);
}
