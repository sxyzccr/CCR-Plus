#include "global.h"
#include "configuredialog.h"
#include "ui_configuredialog.h"

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

    configure_table = new ConfigureTable(problem_list, this);
    ui->widget_empty->hide();
    ui->gridLayout->addWidget(configure_table, 0, 0, 1, 2);

    this->setFixedSize(configure_table->width() + 22, configure_table->height() + 100);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
}

ConfigureDialog::~ConfigureDialog()
{
    delete ui;
}

void ConfigureDialog::accept()
{
    for (int i = 0; i < problem_list.size(); i++)
        for (int j = 0; j < 3; j++) if (configure_table->ItemText(j, i) == "无效")
            {
                QMessageBox::critical(this, "保存配置失败", "存在无效的设置！");
                return;
            }

    QStringList list;
    for (int i = 0; i < problem_list.size(); i++)
    {
        int t = configure_table->horizontalHeader()->logicalIndex(i);
        if (configure_table->IsColumnChanged(t))
        {
            QString type = configure_table->ItemText(0, t);
            QString checker = configure_table->ItemText(3, t);
            double tim = configure_table->ItemData(1, t).toDouble();
            double mem = configure_table->ItemData(2, t).toDouble();
            if (!configure_table->IsItemChanged(0, t)) type = "";
            if (!configure_table->IsItemChanged(3, t)) checker = "";
            if (!configure_table->IsItemChanged(1, t)) tim = -1;
            if (!configure_table->IsItemChanged(2, t)) mem = -1;
            Problem* tmp = new Problem(problem_list[t]);
            Problem* prob;
            //qDebug()<<problem_list[t]<<type<<checker<<tim<<mem;
            if (configure_table->ItemData(4, t).toBool())
            {
                prob = tmp;
                prob->ConfigureNew(type, tim, mem, checker);
            }
            else
            {
                prob = Global::g_contest.problems[t];
                prob->Configure(type, tim, mem, checker);
            }
            if (!prob->SaveConfiguration())
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
