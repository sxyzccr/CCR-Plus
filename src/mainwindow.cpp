#include "status.h"
#include "version.h"
#include "contestinfo.h"
#include "configuredialog.h"
#include "createfiledialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QMimeData>
#include <QTextStream>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QDesktopServices>

using namespace std;

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->label_bottom->setText(QString(
                             "<p align=\"right\">"
                             "<span style=\"font-size:11pt; color:#505050;\">版本 </span>"
                             "<span style=\"font-size:11pt; color:#00a1f1;\">%1</span>"
                             "</p>").arg(VERSION));

    splitter = new QSplitter(ui->centralWidget);
    close_button = new QToolButton();
    detail_table = new DetailTable(splitter);
    board_table = new BoardTable(splitter);
    judger = new JudgeThread();

    close_button->setIcon(style()->standardPixmap(QStyle::SP_TitleBarCloseButton));
    close_button->setToolTip("关闭当前竞赛");
    close_button->setStyleSheet("background-color:transparent");
    close_button->setFocusPolicy(Qt::NoFocus);
    close_button->hide();
    ui->menuBar->setCornerWidget(close_button);

    splitter->setStretchFactor(1, 1);
    splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    splitter->setOrientation(Qt::Horizontal);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(detail_table);
    splitter->addWidget(board_table);
    splitter->hide();
    ui->verticalLayout->addWidget(splitter);

    CreateActions();
    UpdateRecentContest(true);

    // Set connect
    connect(close_button, &QToolButton::clicked, this, &MainWindow::on_action_close_triggered);

    connect(board_table, &QTableWidget::cellClicked,       detail_table, &DetailTable::showDetailEvent);
    connect(board_table, &QTableWidget::cellDoubleClicked, this,         &MainWindow::StartJudging);

    connect(board_table->horizontalHeader(), &QHeaderView::sectionClicked, board_table, &BoardTable::onSortTable);
    connect(board_table->horizontalHeader(), &QHeaderView::sectionMoved,   board_table, &BoardTable::onSectionMove);

    connect(judger, &JudgeThread::titleDetailFinished, detail_table, &DetailTable::addTitleDetail);
    connect(judger, &JudgeThread::noteDetailFinished,  detail_table, &DetailTable::addNoteDetail);
    connect(judger, &JudgeThread::pointDetailFinished, detail_table, &DetailTable::addPointDetail);
    connect(judger, &JudgeThread::scoreDetailFinished, detail_table, &DetailTable::addScoreDetail);

    connect(judger, &JudgeThread::itemJudgeFinished,   board_table, &BoardTable::onSetItemUnselected);
    connect(judger, &JudgeThread::playerLabelChanged,  board_table, &BoardTable::onUpdatePlayerLabel);
    connect(judger, &JudgeThread::problemLabelChanged, board_table, &BoardTable::onUpdateProblemLabel);

    this->activateWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete judger;
}

// Last contest path
static QString last_contest;

void MainWindow::UpdateRecentContest(bool updateRecentListWidget)
{
    QSettings set("ccr.ini", QSettings::IniFormat);
    QStringList list = set.value("RecentContestList").toStringList();
    last_contest = set.value("LastContest").toString();
    int n = min(list.size(), MAX_RECENT_CONTEST);
    for (int i = 0; i < n; i++)
    {
        QString s = QString("&%1 %2").arg(i + 1).arg(list[i]);
        action_recent_list[i]->setText(s);
        action_recent_list[i]->setData(list[i]);
        action_recent_list[i]->setVisible(true);
    }
    for (int i = n; i < MAX_RECENT_CONTEST; i++) action_recent_list[i]->setVisible(false);
    ui->menu_recent->setEnabled((bool)n);

    if (!updateRecentListWidget) return;
    ui->listWidget_recent->clear();
    for (auto i : list)
    {
        QPixmap icon = ContestInfo::Info::CreateIcon(i + "/");
        QListWidgetItem* item = new QListWidgetItem(QIcon(icon), i);
        item->setToolTip(item->text());
        item->setFlags(item->flags()^Qt::ItemIsDragEnabled);
        ui->listWidget_recent->addItem(item);
        if (i == last_contest) ui->listWidget_recent->setCurrentItem(item);
    }
}

void MainWindow::LoadContest(const QString& path)
{
    if (!QDir(path).exists())
    {
        QMessageBox::critical(this, "无法打开竞赛", QString("%1\n路径不存在或已被移除。").arg(path));
        QSettings set("ccr.ini", QSettings::IniFormat);
        QStringList list = set.value("RecentContestList").toStringList();
        list.removeAll(path);
        set.setValue("RecentContestList", list);
        UpdateRecentContest(true);
        return;
    }

    QDir dir1(path + "/src"), dir2(path + "/data");
    if ((!dir1.exists() || !dir2.exists()) && QMessageBox::question(this, "找不到试题或选手目录", "是否继续打开并创建子目录？") == QMessageBox::No) return;

    if (!dir1.exists() && !QDir(path).mkdir("src"))
    {
        QMessageBox::critical(this, "无法创建子目录", "创建子目录失败！");
        return;
    }
    if (!dir2.exists() && !QDir(path).mkdir("data"))
    {
        QMessageBox::critical(this, "无法创建子目录", "创建子目录失败！");
        return;
    }

    last_contest = path;
    ContestInfo::info.SetPath(path);

    QSettings set("ccr.ini", QSettings::IniFormat);
    QStringList list = set.value("RecentContestList").toStringList();
    if (!list.count(path)) list.prepend(path);
    set.setValue("RecentContestList", list);
    set.setValue("LastContest", last_contest);
    UpdateRecentContest(false);

    ui->label_top->hide();
    ui->label_bottom->hide();
    ui->listWidget_recent->hide();
    close_button->show();

    this->setWindowTitle(path + " - CCR Plus 测评器");
    ui->action_close->setEnabled(true);
    ui->action_configure->setEnabled(true);
    ui->action_set_list->setEnabled(true);
    ui->action_export->setEnabled(true);
    ui->action_refresh->setEnabled(true);
    ui->action_judge_selected->setEnabled(true);
    ui->action_judge_unjudged->setEnabled(true);
    ui->action_judge_all->setEnabled(true);
    ui->action_stop->setEnabled(false);

    Status::g_judge_stoped = false;
    Status::g_contest_closed = false;

    //boardTable->setup();
    //detailTable->setup();

    LoadBoard();
}

void MainWindow::CloseContest(bool isExit)
{
    Status::g_contest_closed = true;
    StopJudging();
    SaveResultList();
    splitter->hide();
    judger->waitForClearedTmpDir(2000);
    ClearBoard();
    ContestInfo::info.Clear();

    if (isExit) return;

    UpdateRecentContest(true);
    ui->label_top->show();
    ui->label_bottom->show();
    ui->listWidget_recent->show();
    close_button->hide();

    this->setWindowTitle("CCR Plus 测评器");
    ui->action_close->setEnabled(false);
    ui->action_configure->setEnabled(false);
    ui->action_set_list->setEnabled(false);
    ui->action_export->setEnabled(false);
    ui->action_refresh->setEnabled(false);
    ui->action_judge_selected->setEnabled(false);
    ui->action_judge_unjudged->setEnabled(false);
    ui->action_judge_all->setEnabled(false);
    ui->action_stop->setEnabled(false);
}

void MainWindow::LoadBoard()
{
    if (Status::g_is_judging) return;

    if (!QDir(ContestInfo::info.contestPath).exists())
    {
        QString path = QDir(ContestInfo::info.contestPath).path();
        QMessageBox::critical(this, "无法打开竞赛", QString("%1\n路径不存在或已被移除。").arg(path));
        QSettings set("ccr.ini", QSettings::IniFormat);
        QStringList list = set.value("RecentContestList").toStringList();
        list.removeAll(path);
        set.setValue("RecentContestList", list);
        on_action_close_triggered();
        return;
    }

    ContestInfo::info.ReadContestInfo();
    SaveResultList();

    // 显示 boardTable
    ClearBoard();
    splitter->show();

    board_table->setRowCount(ContestInfo::info.playerNum);
    board_table->setColumnCount(ContestInfo::info.problemNum + 2);
    QStringList headerLabels = {"选手", "总分"};
    for (auto i : ContestInfo::info.problems) headerLabels.append(i.name);
    board_table->setHorizontalHeaderLabels(headerLabels);
    for (int i = 0; i < ContestInfo::info.problemNum + 2; i++) board_table->horizontalHeaderItem(i)->setToolTip(headerLabels[i]);
    board_table->showResult();
    board_table->resizePlayerLabel();

    // 读取 .list 文件
    QFile file(ContestInfo::info.contestPath + ".list");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ContestInfo::info.isListUsed = true;
        ui->action_set_list->setChecked(true);
        ReadPlayerList(file, false);
        file.close();
    }
    else
    {
        ContestInfo::info.isListUsed = false;
        ui->action_set_list->setChecked(false);
    }
}

void MainWindow::ClearBoard()
{
    board_table->clearBoard();
    detail_table->clearDetail();
}

void MainWindow::SaveResultList()
{
    QFile file(ContestInfo::info.resultPath + ".reslist");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        for (int i = 0; i < ContestInfo::info.players.size(); i++)
        {
            int t = GetLogicalRow(i);
            Player* p = &ContestInfo::info.players[t];
            for (auto j : ContestInfo::info.problemOrder)
            {
                Player::Result* r = &p->problem[j];
                out << p->name << '/' << ContestInfo::info.problems[j].name << '/' << r->score << '/' << r->usedTime << '/' << r->state << '/' << endl;
            }
        }
        file.close();
    }
}

void MainWindow::ReadPlayerList(QFile& file, bool isCSV)
{
    map<QString, QString> list;
    list.clear();
    QTextStream in(&file);
    for (; !in.atEnd();)
    {
        QString s = in.readLine();
        //in.setCodec("UTF-8");
        QStringList line = s.split(",");
        for (auto& i : line) i = i.trimmed(), i.remove('\"');
        if (line.size() >= 2 && line[0].size() && line[1].size()) list[line[0]] = line[1];
    }

    if (isCSV)
    {
        QFile f(ContestInfo::info.contestPath + ".list");
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&f);
            //out.setCodec("UTF-8");
            for (auto i : list) out << i.first << "," << i.second << endl;
            f.close();
        }
    }

    for (auto& i : ContestInfo::info.players)
        if (!i.type)
        {
            QLabel* tmp = i.label[0];
            if (list.count(i.name))
            {
                i.name_list = list[i.name];
                if (i.name_list.length() == 2) tmp->setText(QString("%1 [%2   %3]").arg(i.name, i.name_list.at(0), i.name_list.at(1)));
                else tmp->setText(QString("%1 [%2]").arg(i.name, i.name_list));
                tmp->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                tmp->setStyleSheet("");
            }
            else tmp->setStyleSheet("QLabel{color:rgb(120,120,120);}");
        }

    board_table->resizePlayerLabel();
}

void MainWindow::ExportPlayerScore()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出成绩", ContestInfo::info.contestPath + ContestInfo::info.contestName + ".csv", "CSV (逗号分隔) (*.csv)");
    if (!fileName.size()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        QMessageBox::critical(this, "导出成绩失败", file.errorString());
    else
    {
        QTextStream out(&file);
        //out.setCodec("UTF-8");
        if (ContestInfo::info.isListUsed) out << tr("编号,") << tr("姓名,"); else out << tr("选手,");
        out << tr("总分,");
        for (auto j : ContestInfo::info.problemOrder) out << QString("\"%1\",").arg(ContestInfo::info.problems[j].name);
        out << endl;
        for (int i = 0; i < ContestInfo::info.players.size(); i++)
        {
            int t = GetLogicalRow(i);
            Player* p = &ContestInfo::info.players[t];
            out << QString("\"%1\",").arg(p->name);
            if (ContestInfo::info.isListUsed) out << QString("\"%1\",").arg(p->name_list);
            out << p->sum.score << ",";
            for (auto j : ContestInfo::info.problemOrder) out << p->problem[j].score << ",";
            out << endl;
        }
        file.close();
    }
}

void MainWindow::StopJudging()
{
    //qDebug()<<"STOP";
    Status::g_judge_stoped = true;
    //judger->stop();
}

void MainWindow::StartJudging(int r, int c)
{
    //qDebug()<<r<<c;
    if (Status::g_is_judging) return;

    Status::g_is_judging = true;
    Status::g_judge_stoped = false;

    board_table->clearHighlighted(board_table->preHeaderClicked);
    board_table->preHeaderClicked = -1;
    board_table->setSelectionMode(QAbstractItemView::NoSelection);
    board_table->horizontalHeader()->setSectionsMovable(false);
    board_table->horizontalHeader()->setSortIndicatorShown(false);
    board_table->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);

    ui->action_configure->setEnabled(false);
    ui->action_set_list->setEnabled(false);
    ui->action_export->setEnabled(false);
    ui->action_refresh->setEnabled(false);
    ui->action_judge_selected->setEnabled(false);
    ui->action_judge_unjudged->setEnabled(false);
    ui->action_judge_all->setEnabled(false);
    ui->action_stop->setEnabled(true);

    detail_table->startLastJudgeTimer();
    detail_table->clearDetail();

    judger->setup(r, c, ContestInfo::info.contestPath);

    if (r == -1) // Judge selected
    {
        for (int i = 0; i < ContestInfo::info.playerNum; i++)
            for (auto j : ContestInfo::info.problemOrder)
                if (board_table->item(i, j + 2)->isSelected())
                    judger->appendProblem(qMakePair(i, j + 2));
    }
    else if (r == -2) // Judge unjudged
    {
        for (int i = 0; i < ContestInfo::info.playerNum; i++)
            for (auto j : ContestInfo::info.problemOrder)
                if (ContestInfo::info.players[GetLogicalRow(i)].problem[j].state == ' ')
                    judger->appendProblem(qMakePair(i, j + 2));
    }
    else if (c > 1) // Judge one
        judger->appendProblem(qMakePair(r, c));

    QEventLoop* eventLoop = new QEventLoop(this);
    connect(judger, SIGNAL(finished()), eventLoop, SLOT(quit()));
    judger->start();
    eventLoop->exec();
    delete eventLoop;

    Status::g_is_judging = false;

    board_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    board_table->horizontalHeader()->setSectionsMovable(true);

    ui->action_configure->setEnabled(true);
    ui->action_set_list->setEnabled(true);
    ui->action_export->setEnabled(true);
    ui->action_refresh->setEnabled(true);
    ui->action_judge_selected->setEnabled(true);
    ui->action_judge_unjudged->setEnabled(true);
    ui->action_judge_all->setEnabled(true);
    ui->action_stop->setEnabled(false);

    this->activateWindow();

    SaveResultList();
    judger->waitForClearedTmpDir(2000);
}

void MainWindow::CreateActions()
{
    for (int i = 0; i < MAX_RECENT_CONTEST; i++)
    {
        action_recent_list[i] = new QAction(this);
        action_recent_list[i]->setVisible(false);
        ui->menu_recent->addAction(action_recent_list[i]);
        connect(action_recent_list[i], SIGNAL(triggered()), this, SLOT(onOpenRecentContest()));
    }
    ui->menu_recent->addSeparator();

    menu_recent_list = new QMenu(this);
    menu_table = new QMenu(board_table);
    menu_header = new QMenu(board_table->horizontalHeader());
    action_clean_recent = new QAction("清空列表(&C)", this);
    action_remove_recent = new QAction("从列表中移除(&R)", this);
    action_edit_file = new QAction("编辑源代码(&E)...", this);
    action_create_file = new QAction("创建源代码(&W)...", this);
    action_open_dir = new QAction("打开目录(&O)...", this);
    action_create_dir = new QAction("创建并打开目录(&C)...", this);
    action_remove_dir = new QAction("删除目录(&R)...", this);
    ui->menu_recent->addAction(action_clean_recent);

    connect(action_clean_recent,  &QAction::triggered, this, &MainWindow::onCleanRecentContest);
    connect(action_remove_recent, &QAction::triggered, this, &MainWindow::onRemoveRecentContest);
    connect(action_edit_file,     &QAction::triggered, this, &MainWindow::onEditFile);
    connect(action_create_file,   &QAction::triggered, this, &MainWindow::onCreateFile);
    connect(action_open_dir,      &QAction::triggered, this, &MainWindow::onOpenDir);
    connect(action_create_dir,    &QAction::triggered, this, &MainWindow::onCreateDir);
    connect(action_remove_dir,    &QAction::triggered, this, &MainWindow::onRemoveDir);

    board_table->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget_recent,           &QWidget::customContextMenuRequested, this, &MainWindow::onMenuRecentListEvent);
    connect(board_table->horizontalHeader(), &QWidget::customContextMenuRequested, this, &MainWindow::onMenuHeaderEvent);
    connect(board_table,                     &QWidget::customContextMenuRequested, this, &MainWindow::onMenuTableEvent);
}

// Used by actions
static QString dirByAction, fileByAction;
static Player* playerByAction;
static Problem* problemByAction;

void MainWindow::onCreateFile()
{
    CreateFileDialog dialog(this, playerByAction, problemByAction);
    if (dialog.exec())
    {
        if (!QDir(dirByAction).exists()) QDir().mkpath(dirByAction);
        QString s = dialog.getSelectedFile();
        QFile file(dirByAction + s);
        file.open(QIODevice::WriteOnly);
        file.close();
        QDesktopServices::openUrl(QUrl("file:///" + dirByAction + s));
    }
}

void MainWindow::onEditFile()
{
    QDesktopServices::openUrl(QUrl("file:///" + dirByAction + fileByAction));
}

void MainWindow::onCreateDir()
{
    QDir().mkpath(dirByAction);
    QDesktopServices::openUrl(QUrl("file:///" + dirByAction));
}

void MainWindow::onOpenDir()
{
    QDesktopServices::openUrl(QUrl("file:///" + dirByAction));
}

void MainWindow::onRemoveDir()
{
    QString t1, t2;
    if (dirByAction == ContestInfo::info.srcPath + fileByAction + "/")
        t1 = "删除选手", t2 = QString("确实要删除选手 \"%1\" 及其目录中的所有文件吗？").arg(fileByAction);
    else if (dirByAction == ContestInfo::info.dataPath + fileByAction + "/")
        t1 = "删除试题", t2 = QString("确实要删除试题 \"%1\" 及其目录中的所有文件吗？").arg(fileByAction);
    if (QMessageBox::question(this, t1, t2) == QMessageBox::Yes)
    {
        QDir(dirByAction).removeRecursively();
        if (t1 == "删除试题") QDir(ContestInfo::info.resultPath + fileByAction).removeRecursively();
        LoadBoard();
    }
}

void MainWindow::onOpenRecentContest()
{
    if (Status::g_is_judging)
    {
        StopJudging();
        QMessageBox::warning(this, "", QString("测评被终止！"));
    }

    QAction* action = dynamic_cast<QAction*>(sender());
    if (action) LoadContest(action->data().toString());
}

void MainWindow::onRemoveRecentContest()
{
    QSettings set("ccr.ini", QSettings::IniFormat);
    QStringList list = set.value("RecentContestList").toStringList();
    list.removeAll(dirByAction);
    set.setValue("RecentContestList", list);
    UpdateRecentContest(true);
}

void MainWindow::onCleanRecentContest()
{
    QSettings set("ccr.ini", QSettings::IniFormat);
    set.clear();
    UpdateRecentContest(true);
}

void MainWindow::onMenuRecentListEvent(const QPoint& pos)
{
    action_open_dir->setEnabled(true);
    action_remove_recent->setEnabled(true);
    action_clean_recent->setEnabled(ui->menu_recent->isEnabled());

    menu_recent_list->clear();
    QListWidgetItem* item = ui->listWidget_recent->itemAt(pos);
    if (item)
    {
        dirByAction = item->text();
        action_open_dir->setText("打开目录(&O)...");
        menu_recent_list->addAction(action_open_dir);
        menu_recent_list->addSeparator();
        menu_recent_list->addAction(action_remove_recent);
    }
    menu_recent_list->addAction(action_clean_recent);
    menu_recent_list->popup(QCursor::pos());
}

void MainWindow::onMenuHeaderEvent(const QPoint& pos)
{
    menu_header->clear();
    int c = board_table->horizontalHeader()->logicalIndexAt(pos);
    if (c >= 0)
    {
        action_create_dir->setEnabled(true);
        action_open_dir->setEnabled(true);

        if (!c) dirByAction = ContestInfo::info.srcPath;
        else if (c == 1) dirByAction = ContestInfo::info.dataPath;
        else dirByAction = ContestInfo::info.dataPath + ContestInfo::info.problems[c - 2].name + "/";

        if (QDir(dirByAction).exists())
        {
            if (!c) action_open_dir->setText("打开选手目录(&O)...");
            else if (c == 1) action_open_dir->setText("打开试题目录(&O)...");
            else action_open_dir->setText("打开目录(&O)...");
            menu_header->addAction(action_open_dir);
            if (c > 1)
            {
                fileByAction = ContestInfo::info.problems[c - 2].name;
                menu_header->addSeparator();
                action_remove_dir->setText(QString("删除试题 \"%1\"(&R)").arg(fileByAction));
                menu_header->addAction(action_remove_dir);
            }
        }
        else menu_header->addAction(action_create_dir);

        menu_header->popup(QCursor::pos());
    }
}

void MainWindow::onMenuTableEvent(const QPoint& pos)
{
    action_create_file->setEnabled(true);
    action_create_dir->setEnabled(true);
    action_open_dir->setEnabled(true);
    action_open_dir->setText("打开目录(&O)...");
    menu_table->clear();
    QTableWidgetItem* item = board_table->itemAt(pos);
    if (item)
    {
        int r = item->row(), c = item->column();
        detail_table->showDetailEvent(r, c);
        r = GetLogicalRow(r);
        Player* player = &ContestInfo::info.players[r];
        if (c > 1)
        {
            Problem* problem = &ContestInfo::info.problems[c - 2];
            dirByAction = ContestInfo::info.srcPath + player->name + "/" + problem->dir + "/";
            fileByAction = problem->getCompiler(player->name).file;

            if (fileByAction == "")
            {
                playerByAction = player, problemByAction = problem;
                if (problem->type == ProblemType::AnswersOnly || !problem->compilers.size())
                    action_create_file->setEnabled(false);
                menu_table->addAction(action_create_file);
            }
            else
            {
                action_edit_file->setText(QString("编辑 \"%1\" (&E)...").arg(fileByAction));
                menu_table->addAction(action_edit_file);
            }

            menu_table->addSeparator();

            if (QDir(dirByAction).exists()) menu_table->addAction(action_open_dir);
            else if (problem->type == ProblemType::AnswersOnly || !problem->compilers.size())
                menu_table->addAction(action_create_dir);
            else
            {
                action_open_dir->setEnabled(false);
                menu_table->addAction(action_open_dir);
            }
        }
        else
        {
            dirByAction = ContestInfo::info.srcPath + player->name + "/";
            fileByAction = player->name;

            if (QDir(dirByAction).exists())
            {
                menu_table->addAction(action_open_dir);
                menu_table->addSeparator();
                action_remove_dir->setText(QString("删除选手 \"%1\"(&R)").arg(fileByAction));
                menu_table->addAction(action_remove_dir);
            }
            else menu_table->addAction(action_create_dir);
        }
    }
    else
    {
        menu_table->addAction(ui->action_refresh);
        menu_table->addSeparator();
        menu_table->addAction(ui->action_judge_selected);
        menu_table->addAction(ui->action_judge_unjudged);
        menu_table->addAction(ui->action_judge_all);

        menu_table->addAction(ui->action_stop);
    }
    menu_table->popup(QCursor::pos());
}

void MainWindow::on_listWidget_recent_itemDoubleClicked(QListWidgetItem* item)
{
    if (item) LoadContest(item->text());
}

void MainWindow::on_action_open_triggered()
{
    if (Status::g_is_judging)
    {
        StopJudging();
        QMessageBox::warning(this, "", QString("测评被终止！"));
    }

    QDir dir(last_contest);
    dir.cdUp();
    QString path = QFileDialog::getExistingDirectory(this, "打开竞赛目录", dir.path());
    if (path.size()) LoadContest(path);
}

void MainWindow::on_action_close_triggered()
{
    CloseContest(false);
}

void MainWindow::on_action_configure_triggered()
{
    ConfigDialog dialog(this);
    if (dialog.exec())
    {
        LoadBoard();
        detail_table->showConfigDetail();
    }
}

void MainWindow::on_action_set_list_triggered()
{
    if (ContestInfo::info.isListUsed)
    {
        if (QMessageBox::question(this, "移除选手名单", "确实要移除选手名单吗？") == QMessageBox::No)
        {
            ContestInfo::info.isListUsed = true;
            ui->action_set_list->setChecked(true);
            return;
        }
        ContestInfo::info.isListUsed = false;
        ui->action_set_list->setChecked(false);
        QFile(ContestInfo::info.contestPath + ".list").remove();

        for (auto& i : ContestInfo::info.players)
            if (!i.type)
            {
                QLabel* tmp = i.label[0];
                i.name_list = "", tmp->setText(i.name);
                tmp->setAlignment(Qt::AlignCenter);
                tmp->setStyleSheet("");
            }
        board_table->resizePlayerLabel();
    }
    else
    {
        QString fileName = QFileDialog::getOpenFileName(this, "导入选手名单", ContestInfo::info.contestPath, "CSV (逗号分隔) (*.csv)");
        if (!fileName.size())
        {
            ContestInfo::info.isListUsed = false;
            ui->action_set_list->setChecked(false);
            return;
        }
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            QMessageBox::critical(this, "导入名单失败", file.errorString());
        else
        {
            ContestInfo::info.isListUsed = true;
            ui->action_set_list->setChecked(true);
            ReadPlayerList(file, true);
            file.close();
        }
    }
    detail_table->clearDetail();
}

void MainWindow::on_action_export_triggered()
{
    ExportPlayerScore();
}

void MainWindow::on_action_exit_triggered()
{
    if (ui->action_close->isEnabled()) CloseContest(true);
    QApplication::exit();
}

void MainWindow::on_action_refresh_triggered()
{
    LoadBoard();
}

void MainWindow::on_action_judge_selected_triggered()
{
    StartJudging(-1, -1);
}

void MainWindow::on_action_judge_unjudged_triggered()
{
    StartJudging(-2, -2);
}

void MainWindow::on_action_judge_all_triggered()
{
    board_table->selectAll();
    StartJudging(-1, -1);
}

void MainWindow::on_action_stop_triggered()
{
    StopJudging();
}

void MainWindow::on_action_help_triggered()
{

}

void MainWindow::on_action_about_triggered()
{
    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setWindowTitle("关于 CCR Plus");
    msgBox->setText(QString(
                        "<h2>CCR Plus 测评器<br/></h2>"
                        "<p>版本：%1</p>"
                        "<p>构建时间：%2 - %3</p>"
                        "<p>Copyright © 2016 绍兴一中 贾越凯。保留所有权利。<br/></p>"
                        "<p>项目主页：<a href=\"https://github.com/sxyzccr/CCR-Plus\">https://github.com/sxyzccr/CCR-Plus</a></p>"
                        "<p>作者邮箱：<a href=\"mailto:equation618@gmail.com\">equation618@gmail.com</a><br/></p>"
                        "<p>本项目使用 <a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">GNU 通用公共许可证</a>。</p>"
                        "<p>感谢 <a href=\"http://code.google.com/p/project-lemon\">project-lemon</a> 等开源项目的支持。</p>"
                    ).arg(VERSION).arg(__DATE__).arg(__TIME__));
    msgBox->setStandardButtons(QMessageBox::Ok);
    msgBox->setIconPixmap(QPixmap(":/icon/image/logo.png"));
    msgBox->exec();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (ui->action_close->isEnabled()) CloseContest(true);
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (!ui->action_close->isEnabled() && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) && ui->listWidget_recent->currentItem()->isSelected())
        on_listWidget_recent_itemDoubleClicked(ui->listWidget_recent->currentItem());
    else QWidget::keyPressEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    auto data = event->mimeData();
    if (data->hasUrls() && data->urls().size() == 1 && QDir(data->urls().at(0).toLocalFile()).exists()) event->acceptProposedAction();
    else event->ignore();
}

void MainWindow::dropEvent(QDropEvent* event)
{
    auto data = event->mimeData();
    if (data->hasUrls() && data->urls().size() == 1)
    {
        QString path = data->urls().first().toLocalFile();
        if (path.size() >= 2 && path[0] == '/' && path[1] == '/') path.remove(0, 1);
        if (QDir(path).exists())
        {
            if (Status::g_is_judging)
            {
                StopJudging();
                QMessageBox::warning(this, "", QString("测评被终止！"));
            }
            LoadContest(path);
        }
    }
}
