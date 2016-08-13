#include "global.h"
#include "advancedconfiguredialog.h"
#include "ui_advancedconfiguredialog.h"

#include <QDebug>

using namespace std;

AdvancedConfigureDialog::AdvancedConfigureDialog(const QStringList& list, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdvancedConfigureDialog), problem_list(list)
{
    ui->setupUi(this);

    for (int i = 0; i < problem_list.size(); i++)
    {
        QString name = problem_list[i];
        ui->listWidget->addItem(name);
        ui->listWidget->item(i)->setToolTip(name);
        if (Global::g_contest.ProblemIndex(name) == -1) ui->listWidget->item(i)->setFont(Global::BOLD_FONT);
    }
    ui->listWidget->setCurrentRow(0);

    ui->tableWidget_compiler->horizontalHeader()->setFixedHeight(22);

    ui->tableWidget_compiler->verticalHeader()->setSectionsMovable(true);
    ui->tableWidget_compiler->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);


    ui->tableWidget_testCase->horizontalHeader()->setFixedHeight(22);

    ui->tableWidget_testCase->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget_testCase->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
}

AdvancedConfigureDialog::~AdvancedConfigureDialog()
{
    delete ui;
}

void AdvancedConfigureDialog::accept()
{
    QDialog::accept();
}

void AdvancedConfigureDialog::loadFromProblem(Problem* problem)
{
    row_span_top.clear();
    row_span_bottom.clear();

    ui->tableWidget_testCase->clear();
    ui->tableWidget_testCase->setRowCount(0);

    if (problem == nullptr)
    {
        return;
    }

    if (problem->Type() == Global::Traditional)
    {
        ui->tableWidget_testCase->setColumnCount(5);
        ui->tableWidget_testCase->setHorizontalHeaderLabels({"输入文件", "输出文件", "时间限制", "内存限制", "分值"});
    }
    else if (problem->Type() == Global::AnswersOnly)
    {
        ui->tableWidget_testCase->setColumnCount(4);
        ui->tableWidget_testCase->setHorizontalHeaderLabels({"输入文件", "输出文件", "提交文件", "分值"});
    }
    else return;
    int scoreColumn = ui->tableWidget_testCase->columnCount() - 1;

    int rows = 0;
    for (int i = 0; i < problem->SubtaskCount(); i++)
    {
        Subtask* sub = problem->SubtaskAt(i);
        QTableWidgetItem* tmp;
        int len = 0;
        for (TestCase* point : *sub)
        {
            ui->tableWidget_testCase->insertRow(rows);

            tmp = new QTableWidgetItem(point->InFile());
            tmp->setTextAlignment(Qt::AlignCenter);
            tmp->setToolTip(tmp->text());
            ui->tableWidget_testCase->setItem(rows, 0, tmp);

            tmp = new QTableWidgetItem(point->OutFile());
            tmp->setTextAlignment(Qt::AlignCenter);
            tmp->setToolTip(tmp->text());
            ui->tableWidget_testCase->setItem(rows, 1, tmp);

            if (problem->Type() == Global::Traditional)
            {
                tmp = new QTableWidgetItem(QString::number(point->TimeLimit()));
                tmp->setTextAlignment(Qt::AlignCenter);
                tmp->setToolTip(tmp->text());
                ui->tableWidget_testCase->setItem(rows, 2, tmp);

                tmp = new QTableWidgetItem(QString::number(point->MemoryLimit()));
                tmp->setTextAlignment(Qt::AlignCenter);
                tmp->setToolTip(tmp->text());
                ui->tableWidget_testCase->setItem(rows, 3, tmp);
            }
            else if (problem->Type() == Global::AnswersOnly)
            {
                tmp = new QTableWidgetItem(point->SubmitFile());
                tmp->setTextAlignment(Qt::AlignCenter);
                tmp->setToolTip(tmp->text());
                ui->tableWidget_testCase->setItem(rows, 2, tmp);
            }

            ui->tableWidget_testCase->setVerticalHeaderItem(rows, new QTableWidgetItem(QString::number(rows + 1)));
            len++, rows++;
        }
        tmp = new QTableWidgetItem(QString::number(sub->Score()));
        tmp->setTextAlignment(Qt::AlignCenter);
        tmp->setToolTip(tmp->text());
        tmp->setBackgroundColor(QColor(255, 255, 255));
        ui->tableWidget_testCase->setItem(rows - len, scoreColumn, tmp);
        if (len > 1) ui->tableWidget_testCase->setSpan(rows - len, scoreColumn, len, 1);
        for (int i = 0; i < len; i++)
        {
            row_span_top.push_back(rows - len);
            row_span_bottom.push_back(rows - 1);
        }
    }
    ui->label_score->setText(QString::number(problem->Score()));
}

void AdvancedConfigureDialog::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    ui->label_problem->setText(current->text());

    int p = Global::g_contest.ProblemIndex(current->text());
    loadFromProblem(p == -1 ? nullptr : Global::g_contest.problems[p]);

//    for (int i = 0; i < ui->listWidget->count(); i++)
//        qDebug()<<ui->listWidget->item(i)->text();
}

void AdvancedConfigureDialog::on_tableWidget_testCase_itemSelectionChanged()
{
    auto list = ui->tableWidget_testCase->selectedRanges();
    //qDebug()<<"fuck>>>>>>>>";
    //qDebug()<<list.size();
    //for (auto i : list) qDebug()<<i.rowCount();
    //qDebug()<<"<<<<<<<<<<<<<fuck";

    int top = 1e9, bottom = -1e9, spanTop = 1e9, spanBottom = -1e9;
    for (auto i : list)
    {
        top = min(top, i.topRow()), bottom = max(bottom, i.bottomRow());
        spanTop = min(spanTop, row_span_top[i.topRow()]), spanBottom = max(spanBottom, row_span_bottom[i.bottomRow()]);
    }

    if (bottom - top == spanBottom - spanTop && bottom > top && row_span_top[spanBottom] > spanTop)
        ui->pushButton_merge->setEnabled(true);
    else
        ui->pushButton_merge->setEnabled(false);
}

void AdvancedConfigureDialog::on_tableWidget_testCase_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    //qDebug()<<current->row()<<' '<<current->column();
    ui->pushButton_addTestCase->setEnabled(true);
    ui->pushButton_removeTestCase->setEnabled(true);
}

void AdvancedConfigureDialog::on_pushButton_merge_clicked()
{
    auto list = ui->tableWidget_testCase->selectedRanges();
    int top = 1e9, bottom = -1e9, spanTop = 1e9, spanBottom = -1e9;
    for (auto i : list)
    {
        top = min(top, i.topRow()), bottom = max(bottom, i.bottomRow());
        spanTop = min(spanTop, row_span_top[i.topRow()]), spanBottom = max(spanBottom, row_span_bottom[i.bottomRow()]);
    }
    int scoreColumn = ui->tableWidget_testCase->columnCount() - 1;
    if (bottom - top == spanBottom - spanTop && bottom > top && row_span_top[spanBottom] > spanTop)
    {
        int sum = 0;
        for (int i = top; i <= bottom; i++)
        {
            if (row_span_top[i] == i)
                sum += ui->tableWidget_testCase->item(i, scoreColumn)->text().toInt();
            row_span_top[i] = top, row_span_bottom[i] = bottom;
        }
        ui->tableWidget_testCase->setSpan(top, scoreColumn, bottom - top + 1, 1);
        ui->tableWidget_testCase->item(top, scoreColumn)->setText(QString::number(sum));
        ui->tableWidget_testCase->item(top, scoreColumn)->setToolTip(QString::number(sum));
    }
}
