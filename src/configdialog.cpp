#include "contestinfo.h"
#include "configdialog.h"
#include "ui_configdialog.h"

#include <QVector>
#include <QCheckBox>
#include <QMessageBox>

using namespace std;

ConfigDialog::ConfigDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    alreadyChangingData = false;
    ui->tableView->horizontalHeader()->setMaximumHeight(25);
    ui->tableView->horizontalHeader()->setSectionsMovable(true);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    loadProblems();

    connect(&model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(dataChangedEvent(QModelIndex, QModelIndex)));
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::setModelData(int c)
{
    Problem* problem = NULL;
    if (c < ContestInfo::info.problemNum) problem = &ContestInfo::info.problems[c];

    auto configNew = [&]()
    {
        model.item(0, c)->setData("传统型", Qt::EditRole);
        model.item(1, c)->setData(1, Qt::EditRole);
        model.item(2, c)->setData(128, Qt::EditRole);
        model.item(3, c)->setData("全文比较", Qt::EditRole);
        model.item(4, c)->setData(2, Qt::EditRole);
        model.item(4, c)->setToolTip("true");

        for (int i = 0; i < 4; i++) model.item(i, c)->setFont(BOLD_FONT);
        model.horizontalHeaderItem(c)->setFont(BOLD_FONT);
    };

    if (!problem || !problem->que.size()) { configNew(); return; }

    model.item(4, c)->setToolTip("false");
    model.item(3, c)->setData((problem->checker == "fulltext" || problem->checker == "fulltext.exe") ? "全文比较" : problem->checker == ".exe" ? "" : problem->checker, Qt::EditRole);
    if (problem->type == ProblemType::Traditional)
    {
        model.item(0, c)->setData("传统型", Qt::EditRole);
        double minT = 1e9, maxT = 0, minM = 1e9, maxM = 0;
        for (auto info : problem->que)
        {
            minT = min(minT, info.timeLim), maxT = max(maxT, info.timeLim);
            minM = min(minM, info.memLim), maxM = max(maxM, info.memLim);
        }
        if (minT > maxT || minM > maxM) {configNew(); return;}
        if (minT == maxT) model.item(1, c)->setData(minT, Qt::EditRole);
        else if (0 <= minT && maxT <= 3600) model.item(1, c)->setData(QString("%1~%2").arg(minT).arg(maxT), Qt::EditRole);
        else
        {
            model.item(1, c)->setData(QString("无效"), Qt::EditRole);
            model.item(1, c)->setFont(BOLD_FONT);
        }

        if (minM == maxM) model.item(2, c)->setData(minM, Qt::EditRole);
        else if (0 <= minM && maxM <= 8192) model.item(2, c)->setData(QString("%1~%2").arg(minM).arg(maxM), Qt::EditRole);
        else
        {
            model.item(2, c)->setData(QString("无效"), Qt::EditRole);
            model.item(2, c)->setFont(BOLD_FONT);
        }
    }
    else if (problem->type == ProblemType::AnswersOnly)
    {
        model.item(0, c)->setData("提交答案型", Qt::EditRole);
        model.item(1, c)->setText("");
        model.item(1, c)->setEditable(false);
        model.item(2, c)->setText("");
        model.item(2, c)->setEditable(false);
    }
    else
    {
        model.item(0, c)->setData("无效", Qt::EditRole);
        model.item(0, c)->setFont(BOLD_FONT);
        model.item(1, c)->setText("");
        model.item(1, c)->setEditable(false);
        model.item(2, c)->setText("");
        model.item(2, c)->setEditable(false);
    }
}

void ConfigDialog::loadProblems()
{
    for (auto i : ContestInfo::info.problemOrder) problemList.append(ContestInfo::info.problems[i].name);
    QStringList tmp = QDir(ContestInfo::info.dataPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto i : tmp) if (!problemList.count(i)) problemList.append(i);
    num = problemList.size();

    model.setRowCount(5);
    model.setVerticalHeaderLabels({"题目类型", "时间限制", "内存限制", "比较方式", "清空原配置"});
    model.verticalHeaderItem(0)->setToolTip("试题的类型。");
    model.verticalHeaderItem(1)->setToolTip("试题每个测试点拥有的运行时间上限(仅限传统型试题)。单位: 秒(s)");
    model.verticalHeaderItem(2)->setToolTip("试题每个测试点拥有的运行内存上限(仅限传统型试题)。单位: 兆字节(MB)");
    model.verticalHeaderItem(3)->setToolTip("选手程序输出文件(或答案文件)与标准输出文件的比较方式。");
    model.verticalHeaderItem(4)->setToolTip("清空原来的所有配置。");
    for (int i = 0; i < 5; i++) model.verticalHeaderItem(i)->setTextAlignment(Qt::AlignCenter);

    model.setColumnCount(num);
    model.setHorizontalHeaderLabels(problemList);
    ui->tableView->setModel(&model);
    ui->tableView->setItemDelegate(&delegate);
    delegate.problemList = problemList;
    ui->tableView->horizontalHeader()->setTextElideMode(Qt::ElideRight);

    for (int i = 0; i < num; i++)
    {
        model.horizontalHeaderItem(i)->setToolTip(problemList[i]);
        for (int j = 0; j < 5; j++) model.setData(model.index(j, i), Qt::AlignCenter, Qt::TextAlignmentRole);
        setModelData(i);
        for (int j = 0; j < 4; j++) model.item(j, i)->setToolTip(model.item(j, i)->text());
    }

    int w = num * ui->tableView->horizontalHeader()->defaultSectionSize() + ui->tableView->verticalHeader()->width() + 2;
    int h = 5 * ui->tableView->verticalHeader()->defaultSectionSize() + ui->tableView->horizontalHeader()->height() + 2;
    w = min(max(w, ui->tableView->minimumWidth()), ui->tableView->maximumWidth());
    if (num > 12) h += 17;
    ui->tableView->setFixedSize(w, h);
    this->setFixedSize(w + 22, h + 100);
    //qDebug()<<w<<h;
}

void ConfigDialog::dataChangedEvent(const QModelIndex& tl, const QModelIndex& br)
{
    if (alreadyChangingData) return;
    alreadyChangingData = true;

    int r = tl.row(), c = tl.column();
    QString text = model.item(r, c)->text();
    model.item(r, c)->setFont(BOLD_FONT);
    model.item(r, c)->setToolTip(text);
    model.horizontalHeaderItem(c)->setFont(BOLD_FONT);

    if (r == 4)
    {
        if (c >= ContestInfo::info.problemNum) model.item(r, c)->setData(2, Qt::EditRole);
        else if (model.item(r, c)->data(Qt::EditRole).toBool())
        {
            for (int i = 0; i <= 3; i++)
            {
                if (!i) model.item(i, c)->setData("传统型", Qt::EditRole);
                else if (i == 3) model.item(i, c)->setData("全文比较", Qt::EditRole);
                else model.item(i, c)->setData(i == 1 ? 1 : 128, Qt::EditRole);
                model.item(i, c)->setToolTip(model.item(i, c)->text());
                model.item(i, c)->setFont(BOLD_FONT);
                model.item(i, c)->setEditable(true);
            }
        }
    }
    if (!r)
    {
        if (model.data(tl) == "提交答案型")
        {
            for (int i = 1; i <= 2; i++)
            {
                model.item(i, c)->setData("", Qt::EditRole);
                model.item(i, c)->setToolTip("");
                model.item(i, c)->setEditable(false);
            }
        }
        else if (model.item(r, c)->text() == "传统型")
        {
            for (int i = 1; i <= 2; i++)
            {
                model.item(i, c)->setData(i == 1 ? 1 : 128, Qt::EditRole);
                model.item(i, c)->setToolTip(model.item(i, c)->text());
                model.item(i, c)->setFont(BOLD_FONT);
                model.item(i, c)->setEditable(true);
            }
        }
    }
    alreadyChangingData = false;
}

void ConfigDialog::on_pushButton_clicked()
{
    QMessageBox::information(this, "Sorry", "Will coming soon...");
}

void ConfigDialog::accept()
{
    for (int i = 0; i < num; i++)
        for (int j = 0; j < 3; j++) if (model.item(j, i)->text() == "无效")
            {
                QMessageBox::critical(this, "保存配置失败", "存在无效的设置！");
                return;
            }

    QStringList list;
    for (int i = 0; i < num; i++)
    {
        int t = ui->tableView->horizontalHeader()->logicalIndex(i);
        if (model.horizontalHeaderItem(t)->font().bold())
        {
            QString type = model.item(0, t)->data(Qt::EditRole).toString();
            QString checker = model.item(3, t)->data(Qt::EditRole).toString();
            double tim = model.item(1, t)->data(Qt::EditRole).toDouble();
            double mem = model.item(2, t)->data(Qt::EditRole).toDouble();
            if (!model.item(0, t)->font().bold()) type = "";
            if (!model.item(3, t)->font().bold()) checker = "";
            if (!model.item(1, t)->font().bold()) tim = -1;
            if (!model.item(2, t)->font().bold()) mem = -1;
            Problem prob(problemList[t]);
            //qDebug()<<problemList[t]<<type<<checker<<tim<<mem;
            if (model.item(4, t)->data(Qt::EditRole).toBool()) prob.configureNew(type, tim, mem, checker);
            else
            {
                prob = ContestInfo::info.problems[t];
                prob.configure(type, tim, mem, checker);
            }
            if (!prob.saveConfig())
            {
                QMessageBox::critical(this, "保存配置失败", "无法写入配置文件！");
                return;
            }
        }
        list.append(problemList[t]);
    }
    //qDebug()<<list;
    ContestInfo::info.saveProblemOrder(list);
    QDialog::accept();
}
