#include <QSettings>
#include <QMimeData>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QDesktopServices>

#include "common/global.h"
#include "common/version.h"
#include "configure/configuredialog.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/createfiledialog.h"
#include "ui_mainwindow.h"

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
    close_button = new QToolButton(ui->menuBar);
    detail_table = new DetailTable(splitter);
    board_table = new BoardTable(splitter);
    is_locked = false;

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

    // Set connect
    connect(close_button, &QToolButton::clicked, this, &MainWindow::on_action_close_triggered);

    connect(board_table, &QTableWidget::cellClicked,       detail_table, &DetailTable::onShowDetail);
    connect(board_table, &QTableWidget::cellDoubleClicked, this,         &MainWindow::StartJudging);

    CreateActions();
    UpdateRecentContest(true);

    this->activateWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::LockTable()
{
    is_locked = true;
    board_table->Lock();
    detail_table->Lock();
}

void MainWindow::UnlockTable()
{
    is_locked = false;
    board_table->Unlock();
    detail_table->Unlock();
}

// Last contest path
static QString lastContest;

void MainWindow::UpdateRecentContest(bool updateRecentListWidget)
{
    QSettings set("ccr.ini", QSettings::IniFormat);
    QStringList list = set.value("RecentContestList").toStringList();
    lastContest = set.value("LastContest").toString();
    int n = std::min(list.size(), Global::MAX_RECENT_CONTEST);
    for (int i = 0; i < n; i++)
    {
        QString s = QString("&%1 %2").arg(i + 1).arg(list[i]);
        action_recent_list[i]->setText(s);
        action_recent_list[i]->setData(list[i]);
        action_recent_list[i]->setVisible(true);
    }
    for (int i = n; i < Global::MAX_RECENT_CONTEST; i++) action_recent_list[i]->setVisible(false);
    ui->menu_recent->setEnabled((bool)n);

    if (!updateRecentListWidget) return;
    ui->listWidget_recent->clear();
    for (auto i : list)
    {
        QPixmap icon = Contest::CreateIcon(i + "/");
        QListWidgetItem* item = new QListWidgetItem(QIcon(icon), i, ui->listWidget_recent);
        item->setToolTip(item->text());
        item->setFlags(item->flags()^Qt::ItemIsDragEnabled);
        ui->listWidget_recent->addItem(item);
        if (i == lastContest) ui->listWidget_recent->setCurrentItem(item);
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
    if (!dir1.exists() || !dir2.exists())
        if (QMessageBox::question(this, "找不到试题或选手目录", "是否继续打开并创建子目录？") == QMessageBox::No) return;

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

    if (!Global::g_is_contest_closed) CloseContest();
    lastContest = path;
    Global::g_contest.SetPath(path);

    QSettings set("ccr.ini", QSettings::IniFormat);
    QStringList list = set.value("RecentContestList").toStringList();
    if (!list.count(path)) list.prepend(path);
    set.setValue("RecentContestList", list);
    set.setValue("LastContest", lastContest);
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

    Global::g_is_judge_stoped = false;
    Global::g_is_contest_closed = false;

    LoadTable();
}

void MainWindow::CloseContest(bool isExit)
{
    Global::g_is_contest_closed = true;
    StopJudging();
    splitter->hide();
    ClearTable();
    Global::g_contest.Clear();

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

void MainWindow::LoadTable()
{
    if (is_locked) return;

    if (!QDir(Global::g_contest.path).exists())
    {
        QString path = QDir(Global::g_contest.path).path();
        QMessageBox::critical(this, "无法打开竞赛", QString("%1\n路径不存在或已被移除。").arg(path));
        QSettings set("ccr.ini", QSettings::IniFormat);
        QStringList list = set.value("RecentContestList").toStringList();
        list.removeAll(path);
        set.setValue("RecentContestList", list);
        on_action_close_triggered();
        return;
    }

    Global::g_contest.ReadContestInfo();
    Global::g_contest.SaveResultCache();

    // 显示 boardTable
    ClearTable();
    board_table->ShowResult();
    splitter->show();

    // 读取 .list 文件
    QFile file(Global::g_contest.path + ".list");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Global::g_contest.is_list_used = true;
        ui->action_set_list->setChecked(true);
        Global::g_contest.ReadPlayerList(file);
        file.close();
    }
    else
    {
        Global::g_contest.is_list_used = false;
        ui->action_set_list->setChecked(false);
    }

    board_table->ResizePlayerLabel();
}

void MainWindow::ClearTable()
{
    board_table->ClearBoard();
    detail_table->ClearDetail();
}

void MainWindow::StopJudging()
{
    //qDebug()<<"STOP";
    Global::g_is_judge_stoped = true;
    if (is_locked)
    {
        judge_thread->StopJudge();
        judge_thread->WaitForFinished(2000);
    }
    Global::g_contest.SaveResultCache();
    UnlockTable();
}

void MainWindow::StartJudging(int row, int column)
{
    if (is_locked) return;

    Global::g_is_judge_stoped = false;

    board_table->ClearHighlighted();
    board_table->horizontalHeader()->setSectionsMovable(false);
    board_table->horizontalHeader()->setSortIndicatorShown(false);
    board_table->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    board_table->setSelectionMode(QAbstractItemView::NoSelection);

    detail_table->StartLastJudgeTimer();
    detail_table->ClearDetail();

    LockTable();

    ui->action_configure->setEnabled(false);
    ui->action_set_list->setEnabled(false);
    ui->action_export->setEnabled(false);
    ui->action_refresh->setEnabled(false);
    ui->action_judge_selected->setEnabled(false);
    ui->action_judge_unjudged->setEnabled(false);
    ui->action_judge_all->setEnabled(false);
    ui->action_stop->setEnabled(true);

    judge_thread = new JudgeThread(row, column, this);

    connect(judge_thread, &JudgeThread::titleDetailFinished, detail_table, &DetailTable::onAddTitleDetail);
    connect(judge_thread, &JudgeThread::noteDetailFinished,  detail_table, &DetailTable::onAddNoteDetail);
    connect(judge_thread, &JudgeThread::pointDetailFinished, detail_table, &DetailTable::onAddPointDetail);
    connect(judge_thread, &JudgeThread::scoreDetailFinished, detail_table, &DetailTable::onAddScoreDetail);

    connect(judge_thread, &JudgeThread::itemJudgeFinished,   board_table, &BoardTable::onSetItemUnselected);
    connect(judge_thread, &JudgeThread::labelTextChanged,    board_table, &BoardTable::onUpdateLabelText);
    connect(judge_thread, &JudgeThread::sumLabelChanged,     board_table, &BoardTable::onUpdateSumLabel);
    connect(judge_thread, &JudgeThread::problemLabelChanged, board_table, &BoardTable::onUpdateProblemLabel);

    if (row == -1) // Judge selected
    {
        for (int i = 0; i < Global::g_contest.player_num; i++)
            for (auto j : Global::g_contest.problem_order)
                if (board_table->item(i, j + 2)->isSelected())
                    judge_thread->AppendProblem(i, j + 2);
    }
    else if (row == -2) // Judge unjudged
    {
        for (int i = 0; i < Global::g_contest.player_num; i++)
            for (auto j : Global::g_contest.problem_order)
                if (Global::g_contest.players[Global::GetLogicalRow(i)]->ProblemLabelAt(j)->State() == ' ')
                {
                    judge_thread->AppendProblem(i, j + 2);
                    board_table->item(i, j + 2)->setSelected(true);
                }
    }
    else if (column <= 1) // Judge one player's all problems
    {
        for (auto j : Global::g_contest.problem_order)
            board_table->item(row, j + 2)->setSelected(true);
    }
    else if (column > 1) // Judge one
        judge_thread->AppendProblem(row, column);

    QEventLoop* eventLoop = new QEventLoop(this);
    connect(judge_thread, &QThread::finished, eventLoop, &QEventLoop::quit);
    judge_thread->start();
    eventLoop->exec();
    delete eventLoop;
    delete judge_thread;

    UnlockTable();

    board_table->horizontalHeader()->setSectionsMovable(true);
    board_table->setSelectionMode(QAbstractItemView::ExtendedSelection);

    if (Global::g_is_contest_closed) return;

    ui->action_configure->setEnabled(true);
    ui->action_set_list->setEnabled(true);
    ui->action_export->setEnabled(true);
    ui->action_refresh->setEnabled(true);
    ui->action_judge_selected->setEnabled(true);
    ui->action_judge_unjudged->setEnabled(true);
    ui->action_judge_all->setEnabled(true);
    ui->action_stop->setEnabled(false);

    this->activateWindow();

    Global::g_contest.SaveResultCache();
}

void MainWindow::CreateActions()
{
    for (int i = 0; i < Global::MAX_RECENT_CONTEST; i++)
    {
        action_recent_list[i] = new QAction(this);
        action_recent_list[i]->setVisible(false);
        ui->menu_recent->addAction(action_recent_list[i]);
        connect(action_recent_list[i], SIGNAL(triggered()), this, SLOT(onOpenRecentContest()));
    }
    ui->menu_recent->addSeparator();

    menu_recent_list = new QMenu(ui->listWidget_recent);
    menu_header = new QMenu(board_table->horizontalHeader());
    menu_table = new QMenu(board_table);
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
    CreateFileDialog dialog(playerByAction, problemByAction, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        if (!QDir(dirByAction).exists()) QDir().mkpath(dirByAction);
        QString s = dialog.GetSelectedFile();
        QFile file(dirByAction + s);
        file.open(QIODevice::WriteOnly);
        file.close();
        QDesktopServices::openUrl(QUrl::fromLocalFile(dirByAction + s));
    }
}

void MainWindow::onEditFile()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(dirByAction + fileByAction));
}

void MainWindow::onCreateDir()
{
    QDir().mkpath(dirByAction);
    QDesktopServices::openUrl(QUrl::fromLocalFile(dirByAction));
}

void MainWindow::onOpenDir()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(dirByAction));
}

void MainWindow::onRemoveDir()
{
    QString t1, t2;
    if (dirByAction == Global::g_contest.src_path + fileByAction + "/")
        t1 = "删除选手", t2 = QString("确实要删除选手 \"%1\" 及其目录中的所有文件吗？").arg(fileByAction);
    else if (dirByAction == Global::g_contest.data_path + fileByAction + "/")
        t1 = "删除试题", t2 = QString("确实要删除试题 \"%1\" 及其目录中的所有文件吗？").arg(fileByAction);
    if (QMessageBox::question(this, t1, t2) == QMessageBox::Yes)
    {
        QDir(dirByAction).removeRecursively();
        if (t1 == "删除试题") QDir(Global::g_contest.result_path + fileByAction).removeRecursively();
        LoadTable();
    }
}

void MainWindow::onOpenRecentContest()
{
    if (is_locked)
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

        if (!c) dirByAction = Global::g_contest.src_path;
        else if (c == 1) dirByAction = Global::g_contest.data_path;
        else dirByAction = Global::g_contest.data_path + Global::g_contest.problems[c - 2]->Name() + "/";

        if (QDir(dirByAction).exists())
        {
            if (!c) action_open_dir->setText("打开选手目录(&O)...");
            else if (c == 1) action_open_dir->setText("打开试题目录(&O)...");
            else action_open_dir->setText("打开目录(&O)...");
            menu_header->addAction(action_open_dir);
            if (c > 1)
            {
                fileByAction = Global::g_contest.problems[c - 2]->Name();
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
        detail_table->onShowDetail(r, c);
        r = Global::GetLogicalRow(r);
        Player* player = Global::g_contest.players[r];
        if (c > 1)
        {
            Problem* problem = Global::g_contest.problems[c - 2];
            dirByAction = Global::g_contest.src_path + player->Name() + "/" + problem->Directory() + "/";
            const Compiler* compiler = problem->GetCompiler(player->Name());
            fileByAction = !compiler ? "" : compiler->SourceFile();

            if (fileByAction.isEmpty())
            {
                playerByAction = player, problemByAction = problem;
                if (problem->Type() == Global::AnswersOnly || !problem->CompilerCount())
                    action_create_file->setEnabled(false);
                menu_table->addAction(action_create_file);
            }
            else
            {
                action_edit_file->setText(QString("编辑 \"%1\" (&E)...").arg(fileByAction));
                menu_table->addAction(action_edit_file);
            }

            menu_table->addSeparator();

            if (QDir(dirByAction).exists())
                menu_table->addAction(action_open_dir);
            else if (problem->Type() == Global::AnswersOnly || !problem->CompilerCount())
                menu_table->addAction(action_create_dir);
            else
            {
                action_open_dir->setEnabled(false);
                menu_table->addAction(action_open_dir);
            }
        }
        else
        {
            dirByAction = Global::g_contest.src_path + player->Name() + "/";
            fileByAction = player->Name();

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
    if (is_locked)
    {
        StopJudging();
        QMessageBox::warning(this, "", QString("测评被终止！"));
    }

    QDir dir(lastContest);
    dir.cdUp();
    QString path = QFileDialog::getExistingDirectory(this, "打开竞赛目录", dir.path());
    if (path.size()) LoadContest(path);
}

void MainWindow::on_action_close_triggered()
{
    CloseContest();
}

void MainWindow::on_action_configure_triggered()
{
    QStringList list;
    for (auto i : Global::g_contest.problem_order) list.append(Global::g_contest.problems[i]->Name());
    QStringList tmp = QDir(Global::g_contest.data_path).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto i : tmp)
        if (!list.count(i) && Problem::CheckFileNameValid(i).isEmpty()) list.append(i);

    ConfigureDialog dialog(list, this);
    connect(&dialog, &ConfigureDialog::applied, this, [this]()
    {
        LoadTable();
        detail_table->onShowConfigurationDetail();
    });
    dialog.exec();
}

void MainWindow::on_action_set_list_triggered()
{
    if (Global::g_contest.is_list_used)
    {
        if (QMessageBox::question(this, "移除选手名单", "确实要移除选手名单吗？") == QMessageBox::No)
        {
            Global::g_contest.is_list_used = true;
            ui->action_set_list->setChecked(true);
            return;
        }
        Global::g_contest.is_list_used = false;
        ui->action_set_list->setChecked(false);
        QFile(Global::g_contest.path + ".list").remove();

        for (auto i : Global::g_contest.players) i->SetNameLabelWithoutList();
        board_table->ResizePlayerLabel();
    }
    else
    {
        QString fileName = QFileDialog::getOpenFileName(this, "导入选手名单", Global::g_contest.path, "CSV (逗号分隔) (*.csv)");
        if (!fileName.size())
        {
            Global::g_contest.is_list_used = false;
            ui->action_set_list->setChecked(false);
            return;
        }
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            QMessageBox::critical(this, "导入名单失败", file.errorString());
        else
        {
            Global::g_contest.is_list_used = true;
            Global::g_contest.ReadPlayerList(file, true);
            ui->action_set_list->setChecked(true);
            board_table->ResizePlayerLabel();
            file.close();
        }
    }
    detail_table->ClearDetail();
}

void MainWindow::on_action_export_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出成绩", Global::g_contest.path + Global::g_contest.name + ".csv", "CSV (逗号分隔) (*.csv)");
    if (!fileName.size()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        QMessageBox::critical(this, "导出成绩失败", file.errorString());
    else
        Global::g_contest.ExportPlayerScore(file);
}

void MainWindow::on_action_exit_triggered()
{
    if (ui->action_close->isEnabled()) CloseContest(true);
    QApplication::exit();
}

void MainWindow::on_action_refresh_triggered()
{
    LoadTable();
}

void MainWindow::on_action_judge_selected_triggered()
{
    StartJudging(-1, -1);
}

void MainWindow::on_action_judge_unjudged_triggered()
{
    board_table->clearSelection();
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
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("关于 CCR Plus");
    msgBox.setText(QString(
                      "<h2>CCR Plus 测评器<br/></h2>"
                      "<p>版本：%1</p>"
                      "<p>构建时间：%2 - %3</p>"
                      "<p>Copyright © 2016 绍兴一中 贾越凯。保留所有权利。<br/></p>"
                      "<p>项目主页：<a href=\"https://github.com/sxyzccr/CCR-Plus\">https://github.com/sxyzccr/CCR-Plus</a></p>"
                      "<p>作者邮箱：<a href=\"mailto:equation618@gmail.com\">equation618@gmail.com</a><br/></p>"
                      "<p>本项目使用 <a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">GNU 通用公共许可证</a>。</p>"
                      "<p>感谢 <a href=\"http://code.google.com/p/project-lemon\">project-lemon</a> 等开源项目的支持。</p>"
                      ).arg(VERSION).arg(__DATE__).arg(__TIME__));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setIconPixmap(QPixmap(":/image/logo.png"));
    msgBox.exec();
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
            if (is_locked)
            {
                StopJudging();
                QMessageBox::warning(this, "", QString("测评被终止！"));
            }
            LoadContest(path);
        }
    }
}
