#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "common/const.h"

class QSplitter;
class QToolButton;
class QListWidgetItem;
class Player;
class Problem;
class JudgeThread;
class BoardTable;
class DetailTable;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow* ui;
    QSplitter* splitter;
    QToolButton* close_button;

    BoardTable* board_table;
    DetailTable* detail_table;
    JudgeThread* judge_thread;

    // Context menus and actions
    QMenu *menu_recent_list, *menu_table, *menu_header;
    QAction *action_create_file, *action_edit_file, *action_create_dir, *action_open_dir, *action_remove_dir, *action_configure,
            *action_remove_recent, *action_clean_recent, *action_recent_list[Global::MAX_RECENT_CONTEST];

    bool is_locked;

    // 上锁与解锁
    void LockTable();
    void UnlockTable();

    /// 检查更新，是否只当有更新时才显示对话框
    void CheckUpdates(bool dontShowError);

    /// 更新最近打开的竞赛列表，是否更新 listWidget_recent
    void UpdateRecentContest(bool);

    /// 载入竞赛
    void LoadContest(const QString& path);

    /// 关闭竞赛，是否退出程序
    void CloseContest(bool isExit = false);

    /// 载入表格 (刷新)
    void LoadTable();

    /// 清空 board
    void ClearTable();

    /// 停止测评
    void StopJudging();

    /// 开始测评 board 中第 row 行 column 列的项目，row = column = -1 测评选中项目，row = column = -2 测评未测项目
    void StartJudging(int row, int column);

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
    void onConfigure();

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
    void on_action_update_triggered();
    void on_action_about_triggered();

protected:
    virtual void closeEvent(QCloseEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
};

#endif // MAINWINDOW_H
