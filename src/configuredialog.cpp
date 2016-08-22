#include "global.h"
#include "configuredialog.h"
#include "advancedconfiguredialog.h"
#include "ui_configuredialog.h"

#include <QCheckBox>
#include <QMessageBox>

using namespace std;

ConfigureDialog::ConfigureDialog(const QStringList& list, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::ConfigureDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    for (auto i : list)
    {
        Problem* prob = Global::g_contest.ProblemFromName(i);
        if (!prob || !prob->TestCaseCount())
            prob = new Problem(i);
        else
            prob = new Problem(prob);
        problems.push_back(prob);
    }

    configure_table = new ConfigureTable(problems, this);
    ui->widget_empty->hide();
    ui->gridLayout->addWidget(configure_table, 0, 0, 1, 2);

    this->setFixedSize(configure_table->width() + 22, configure_table->height() + 100);
}

ConfigureDialog::~ConfigureDialog()
{
    for (auto i : problems) delete i;
    problems.clear();
    delete ui;
}

void ConfigureDialog::accept()
{
    for (int i = 0; i < problems.size(); i++)
        for (int j = 0; j < 3; j++) if (configure_table->ItemText(j, i) == "无效")
            {
                QMessageBox::critical(this, "保存配置失败", "存在无效的设置！");
                return;
            }

    QStringList list;
    for (int i = 0; i < problems.size(); i++)
    {
        int t = configure_table->horizontalHeader()->logicalIndex(i);
        if (configure_table->IsColumnChanged(t))
        {
            if (!problems[t]->SaveConfiguration())
            {
                QMessageBox::critical(this, "保存配置失败", "无法写入配置文件！");
                return;
            }
        }
        list.append(problems[t]->Name());
    }
    //qDebug()<<list;
    Global::g_contest.SaveProblemOrder(list);
    QDialog::accept();
}



void ConfigureDialog::on_pushButton_clicked()
{
    AdvancedConfigureDialog dialog(problems, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        for (auto i : problems) delete i;
        problems = dialog.Problems();
    }
}
