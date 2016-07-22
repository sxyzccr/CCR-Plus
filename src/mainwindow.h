#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "player.h"
#include "problem.h"
#include "boardtable.h"
#include "detailtable.h"
#include "judgethread.h"

#include <QFile>
#include <QSplitter>
#include <QToolButton>
#include <QListWidget>
#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = 0);
    virtual ~MainWindow();

private:
    Ui::MainWindow* ui;
    QSplitter* splitter;
    QToolButton* close_button;

    BoardTable* board_table;
    DetailTable* detail_table;
    JudgeThread* judger;

    // Context menus and actions
    QMenu *menu_recent_list, *menu_table, *menu_header;
    QAction *action_create_file, *action_edit_file, *action_create_dir, *action_open_dir, *action_remove_dir,
            *action_remove_recent, *action_clean_recent, *action_recent_list[MAX_RECENT_CONTEST];

    /// 更新最近打开的竞赛列表，是否更新 recentListWidget
    void UpdateRecentContest(bool);

    /// 载入竞赛
    void LoadContest(const QString& path);

    /// 关闭竞赛，是否退出程序
    void CloseContest(bool isExit = false);

    /// 载入 board (刷新)
    void LoadBoard();

    /// 清空 board
    void ClearBoard();

    /// 停止测评
    void StopJudging();

    /// 开始测评 board 中第 r 行 c 列的项目，r = c = -1 测评选中项目，r = c = -2 测评未测项目
    void StartJudging(int r, int c);

    /// 创建动作
    void CreateActions();

private slots:
    // Context menu actions
    void onCreateFile();
    void onEditFile();
    void onCreateDir();
    void onOpenDir();
    void onRemoveDir();
    void onOpenRecentContest();
    void onRemoveRecentContest();
    void onCleanRecentContest();

    // Context menu events
    void onMenuRecentListEvent(const QPoint& pos);
    void onMenuHeaderEvent(const QPoint& pos);
    void onMenuTableEvent(const QPoint& pos);

    // listWidget_recent item action
    void on_listWidget_recent_itemDoubleClicked(QListWidgetItem* item);

    // Contest menu actions
    void on_action_open_triggered();
    void on_action_close_triggered();
    void on_action_configure_triggered();
    void on_action_set_list_triggered();
    void on_action_export_triggered();
    void on_action_exit_triggered();

    // Test menu actions
    void on_action_refresh_triggered();
    void on_action_judge_selected_triggered();
    void on_action_judge_unjudged_triggered();
    void on_action_judge_all_triggered();
    void on_action_stop_triggered();

    // Help menu actions
    void on_action_help_triggered();
    void on_action_about_triggered();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
};

#endif // MAINWINDOW_H
