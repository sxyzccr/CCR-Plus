#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "global.h"
#include "player.h"
#include "problem.h"
#include "boardtable.h"
#include "detailtable.h"
#include "judgethread.h"
#include "configdialog.h"
#include "createfiledialog.h"

#include <QSplitter>
#include <QToolButton>
#include <QListWidget>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void judgeEvent(int,int);
    void stopEvent();
    void openRecentContest();
    void updateRecentContest(bool);
    void loadContestEvent(const QString &path);
    void loadBoardEvent();
    void closeContestEvent();

    void removeRecent_action();
    void cleanRecent_action();
    void openFile(QString dir);
    void editFile_action();
    void createFile_action();
    void openDir_action();
    void createDir_action();
    void removeDir_action();
    void recentListMenuEvent(const QPoint &pos);
    void headerMenuEvent(const QPoint &pos);
    void tableMenuEvent(const QPoint &pos);

    void on_actionOpen_triggered();
    void on_actionClose_triggered();
    void on_actionConfig_triggered();
    void on_actionList_triggered();
    void on_actionExport_triggered();
    void on_actionExit_triggered();

    void on_actionRefresh_triggered();
    void on_actionJudgeSelect_triggered();
    void on_actionJudgeUnjudged_triggered();
    void on_actionJudgeAll_triggered();
    void on_actionStop_triggered();

    void on_actionAbout_triggered();

    void on_recentList_itemDoubleClicked(QListWidgetItem *item);

    void solt1(QLabel*label,const QString&s1,const QString&s2,const QString&s3);
    void solt2(Player*ply,int c,Player::Result*res,int sum);

    void slot3(int row, const QString &title);
    void slot4(int row, const QString &note, const QString&state);
    void slot5(int row, int num, const QString &note, const QString &state, const QString &file, int len);
    void slot6(int row, int len, int score, int sumScore);

    void slot7(int r,int c);
    void slot8(int r,int c);

    void on_actionHelp_triggered();

protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

private:
    Ui::MainWindow *ui;
    QSplitter*splitter;
    BoardTable*boardTable;
    DetailTable*detailTable;
    int playerLabelLength;
    JudgeThread*judger;
    bool judgeStoped,contestClosed;
    QMenu*tableMenu,*headerMenu,*recentListMenu;
    QAction*actionEditFile,*actionOpenDir,*actionCreateFile,*actionRemoveDir,*actionCreateDir,*actionRemove,*actionClean,*actionR[20];
    QString dir_action,file_action,path_action,lastContest;
    Player*player_action;
    Problem*problem_action;
    QToolButton*closeButton;

    QStringList readFolders(const QString &path);
    QStringList readProblemOrder(const QString &path);
    QPixmap createIcon(const QString &path);

    void clear();
    void prepare();
    void createActions();
    void saveResultList();
    void readPlayerList(QFile &fileName, bool isCSV);

signals:
    void stopJudgingSignal();
    void closeContestSignal();
};

#endif // MAINWINDOW_H
