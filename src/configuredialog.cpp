#include "global.h"
#include "configuredialog.h"
#include "ui_configuredialog.h"

#include <QVector>
#include <QCheckBox>
#include <QMessageBox>

using namespace std;

ConfigureDialog::ConfigureDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::ConfigureDialog)
{
    ui->setupUi(this);

    problem_list.clear();
    for (auto i : Global::g_contest.problem_order) problem_list.append(Global::g_contest.problems[i]->Name());
    QStringList tmp = QDir(Global::g_contest.data_path).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto i : tmp) if (!problem_list.count(i)) problem_list.append(i);

    config_table = new ConfigTable(problem_list, this);
    ui->widget_empty->hide();
    ui->gridLayout->addWidget(config_table, 0, 0, 1, 2);

    this->setFixedSize(config_table->width() + 22, config_table->height() + 100);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
}

ConfigureDialog::~ConfigureDialog()
{
    delete ui;
}

void ConfigureDialog::accept()
{
    for (int i = 0; i < problem_list.size(); i++)
        for (int j = 0; j < 3; j++) if (config_table->ItemText(j, i) == "无效")
            {
                QMessageBox::critical(this, "保存配置失败", "存在无效的设置！");
                return;
            }

    QStringList list;
    for (int i = 0; i < problem_list.size(); i++)
    {
        int t = config_table->horizontalHeader()->logicalIndex(i);
        if (config_table->IsColumnChanged(t))
        {
            QString type = config_table->ItemText(0, t);
            QString checker = config_table->ItemText(3, t);
            double tim = config_table->ItemData(1, t).toDouble();
            double mem = config_table->ItemData(2, t).toDouble();
            if (!config_table->IsItemChanged(0, t)) type = "";
            if (!config_table->IsItemChanged(3, t)) checker = "";
            if (!config_table->IsItemChanged(1, t)) tim = -1;
            if (!config_table->IsItemChanged(2, t)) mem = -1;
            Problem* tmp = new Problem(problem_list[t]);
            Problem* prob;
            //qDebug()<<problem_list[t]<<type<<checker<<tim<<mem;
            if (config_table->ItemData(4, t).toBool())
            {
                prob = tmp;
                prob->ConfigureNew(type, tim, mem, checker);
            }
            else
            {
                prob = Global::g_contest.problems[t];
                prob->Configure(type, tim, mem, checker);
            }
            if (!prob->SaveConfig())
            {
                QMessageBox::critical(this, "保存配置失败", "无法写入配置文件！");
                delete tmp;
                return;
            }
            delete tmp;
        }
        list.append(problem_list[t]);
    }
    //qDebug()<<list;
    Global::g_contest.SaveProblemOrder(list);
    QDialog::accept();
}



void ConfigureDialog::on_pushButton_clicked()
{
    QMessageBox::information(this, "Sorry", "Will coming soon...");
}
