#include "version.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

//#include <QDebug>
#include <QPainter>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    splitter=new QSplitter(ui->centralWidget);
    detailTable=new DetailTable(splitter);
    boardTable=new BoardTable(splitter);
    judger=new JudgeThread();
    closeButton=new QToolButton();

    closeButton->setIcon(style()->standardPixmap(QStyle::SP_TitleBarCloseButton));
    closeButton->setToolTip("关闭当前竞赛");
    closeButton->setStyleSheet("background-color:transparent");
    closeButton->setFocusPolicy(Qt::NoFocus);

    ui->menuBar->setCornerWidget(closeButton);
    ui->label_2->setText(QString(
        "<p align=\"right\">"
            "<span style=\"font-size:11pt; color:#505050;\">版本 </span>"
            "<span style=\"font-size:11pt; color:#00a1f1;\">%1</span>"
        "</p>").arg(VERSION));

    splitter->setStretchFactor(1,1);
    splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    splitter->setOrientation(Qt::Horizontal);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(detailTable);
    splitter->addWidget(boardTable);
    ui->verticalLayout->addWidget(splitter);

    splitter->hide();
    closeButton->hide();

    connect(closeButton,SIGNAL(clicked()),this,SLOT(on_actionClose_triggered()));

    connect(this,SIGNAL(stopJudgingSignal()),judger,SLOT(stopJudging()),Qt::DirectConnection);
    connect(judger,SIGNAL(sig1(QLabel*,QString,QString,QString)),this,SLOT(solt1(QLabel*,QString,QString,QString)));
    connect(judger,SIGNAL(sig2(Player*,int,Player::Result*,int)),this,SLOT(solt2(Player*,int,Player::Result*,int)));

    connect(judger,SIGNAL(sig3(int,QString)),this,SLOT(slot3(int,QString)));
    connect(judger,SIGNAL(sig4(int,QString,QString)),this,SLOT(slot4(int,QString,QString)));
    connect(judger,SIGNAL(sig5(int,int,QString,QString,QString,int)),this,SLOT(slot5(int,int,QString,QString,QString,int)));
    connect(judger,SIGNAL(sig6(int,int,int,int)),this,SLOT(slot6(int,int,int,int)));

    connect(judger,SIGNAL(sig7(int,int)),this,SLOT(slot7(int,int)));
    connect(judger,SIGNAL(sig8(int,int)),this,SLOT(slot8(int,int)));

    connect(boardTable->horizontalHeader(),SIGNAL(sectionClicked(int)),boardTable,SLOT(sortEvent(int)));
    connect(boardTable->horizontalHeader(),SIGNAL(sectionMoved(int,int,int)),boardTable,SLOT(sectionMoveEvent(int,int,int)));

    connect(boardTable,SIGNAL(cellClicked(int,int)),detailTable,SLOT(showDetailEvent(int,int)));
    connect(boardTable,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(judgeEvent(int,int)));

    createActions();
    updateRecentContest(true);
    this->activateWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete judger;
    //QMainWindow.setWindowFlags
}

void MainWindow::clear() //清空结构
{
    Global::clear();
    boardTable->clearBoard();
    detailTable->clearDetail();
}

void MainWindow::openRecentContest()
{
    if (Global::alreadyJudging)
    {
        stopEvent();
        QMessageBox::warning(this,"",QString("测评被终止！"));
    }

    QAction *action=dynamic_cast<QAction*>(sender());
    if (action) loadContestEvent(action->data().toString());
}

QPixmap MainWindow::createIcon(const QString&path)
{
    QStringList list=readProblemOrder(path);
    QImage image=QPixmap(":/icon/image/folder.png").toImage();
    QPainter painter(&image);
    painter.setFont(QFont("Times New Roman",15,0,true));
    QFontMetrics fm=painter.fontMetrics();
    pair<int,int> pos[3]={{15,38},{15,60},{15,82}};
    int n=min(list.size(),3);
    for (int i=0; i<n; i++)
    {
        QString text=QString(" %1 ").arg(list[i]);
        int width=min(fm.width(text),109-pos[i].first);
        int height=fm.height();
        painter.drawText(QRectF(pos[i].first,pos[i].second,width,height),text);
    }
    return QPixmap::fromImage(image);
}

void MainWindow::updateRecentContest(bool loadRecentList)
{
    QSettings set("ccr.ini",QSettings::IniFormat);
    QStringList list=set.value("RecentContestList").toStringList();
    lastContest=set.value("LastContest").toString();
    int n=min(list.size(),20);
    for (int i=0; i<n; i++)
    {
        QString s=QString("&%1 %2").arg(i+1).arg(list[i]);
        actionR[i]->setText(s);
        actionR[i]->setData(list[i]);
        actionR[i]->setVisible(true);
    }
    for (int i=n; i<20; i++) actionR[i]->setVisible(false);
    ui->menuRecent->setEnabled(n);

    if (!loadRecentList) return;
    ui->recentList->clear();
    for (auto i:list)
    {
        QPixmap icon=createIcon(i+"/");
        QListWidgetItem*item=new QListWidgetItem(QIcon(icon),i);
        item->setToolTip(item->text());
        item->setFlags(item->flags()^Qt::ItemIsDragEnabled);
        ui->recentList->addItem(item);
        if (i==lastContest) ui->recentList->setCurrentItem(item);
    }
}

void MainWindow::loadContestEvent(const QString &path)
{
    //qDebug()<<"  && BG 0"<<path;

    if (!QDir(path).exists())
    {
        QMessageBox::critical(this,"无法打开竞赛",QString("%1\n路径不存在或已被移除。").arg(path));
        QSettings set("ccr.ini",QSettings::IniFormat);
        QStringList list=set.value("RecentContestList").toStringList();
        list.removeAll(path);
        set.setValue("RecentContestList",list);
        updateRecentContest(true);
        return;
    }

    //qDebug()<<"  && BG xxxx"<<path;

    QDir dir1(path+"/src"),dir2(path+"/data");
    if ((!dir1.exists()||!dir2.exists())&&QMessageBox::question(this,"找不到试题或选手目录","是否继续打开并创建子目录？")==QMessageBox::No) return;

    if (!dir1.exists()&&!QDir(path).mkdir("src"))
    {
        QMessageBox::critical(this,"无法创建子目录","创建子目录失败！");
        return;
    }
    if (!dir2.exists()&&!QDir(path).mkdir("data"))
    {
        QMessageBox::critical(this,"无法创建子目录","创建子目录失败！");
        return;
    }

    //qDebug()<<"  && BG "<<path;

    if (ui->actionClose->isEnabled()) closeContestEvent();
    Global::testPath=path+"/";
    judger->setDir(Global::testPath);
    Global::dataPath=path+"/data/";
    Global::srcPath=path+"/src/";
    Global::resultPath=path+"/result/";
    Global::testName=QDir(path).dirName();
    lastContest=path;

    //qDebug()<<"  && BG 2 "<<path;

    QSettings set("ccr.ini",QSettings::IniFormat);
    QStringList list=set.value("RecentContestList").toStringList();
    if (!list.count(path)) list.prepend(path);
    set.setValue("RecentContestList",list);
    set.setValue("LastContest",lastContest);
    //qDebug()<<list;
    updateRecentContest(false);
    //qDebug()<<"  && BG 3 "<<path;

    ui->label->hide();
    ui->label_2->hide();
    ui->recentList->hide();
    closeButton->show();
    this->setWindowTitle(path+" - CCR Plus 测评器");
    ui->actionClose->setEnabled(true);

    ui->actionConfig->setEnabled(true);
    ui->actionList->setEnabled(true);
    ui->actionExport->setEnabled(true);
    ui->actionRefresh->setEnabled(true);
    ui->actionJudgeSelect->setEnabled(true);
    ui->actionJudgeUnjudged->setEnabled(true);
    ui->actionJudgeAll->setEnabled(true);
    ui->actionStop->setEnabled(false);

    judgeStoped=false;
    contestClosed=false;
    Global::alreadyJudging=false;

    //boardTable->setup();
    //detailTable->setup();

    loadBoardEvent();
}

void MainWindow::loadBoardEvent()
{
    if (Global::alreadyJudging) return;

    if (!QDir(Global::testPath).exists())
    {
        QString path=QDir(Global::testPath).path();
        QMessageBox::critical(this,"无法打开竞赛",QString("%1\n路径不存在或已被移除。").arg(path));
        QSettings set("ccr.ini",QSettings::IniFormat);
        QStringList list=set.value("RecentContestList").toStringList();
        list.removeAll(path);
        set.setValue("RecentContestList",list);
        on_actionClose_triggered();
        return;
    }

    clear();
    prepare();
    boardTable->showResult();
    splitter->show();

    //读取.list文件
    QFile file(Global::testPath+".list");
    if (file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        Global::isListUsed=true;
        ui->actionList->setChecked(true);
        readPlayerList(file,0);
        file.close();
    }
    else
    {
        Global::isListUsed=false;
        ui->actionList->setChecked(false);
    }
}

void MainWindow::stopEvent()
{
    //qDebug()<<"STOP";
    judgeStoped=true;
    emit stopJudgingSignal();
    //judger->stop();
}

void MainWindow::closeContestEvent()
{
    contestClosed=true;
    stopEvent();
    saveResultList();
    splitter->hide();
    judger->waitForClearedTmpDir(2000);
    clear();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (ui->actionClose->isEnabled()) closeContestEvent();
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (!ui->actionClose->isEnabled()&&(event->key()==Qt::Key_Enter||event->key()==Qt::Key_Return)&&ui->recentList->currentItem()->isSelected())
        on_recentList_itemDoubleClicked(ui->recentList->currentItem());
    else QWidget::keyPressEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    auto data=event->mimeData();
    if (data->hasUrls()&&data->urls().size()==1&&QDir(data->urls().at(0).toLocalFile()).exists()) event->acceptProposedAction();
    else event->ignore();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    auto data=event->mimeData();
    if (data->hasUrls()&&data->urls().size()==1)
    {
        //qDebug()<<"  GB -1 "<<data->urls().size();
        QString path=data->urls().first().toLocalFile();
        if (path.size()>=2&&path[0]=='/'&&path[1]=='/') path.remove(0,1);
        if (QDir(path).exists())
        {
            if (Global::alreadyJudging)
            {
                stopEvent();
                QMessageBox::warning(this,"",QString("测评被终止！"));
            }
            loadContestEvent(path);
        }
    }
}

void MainWindow::removeRecent_action()
{
    QSettings set("ccr.ini",QSettings::IniFormat);
    QStringList list=set.value("RecentContestList").toStringList();
    list.removeAll(path_action);
    set.setValue("RecentContestList",list);
    updateRecentContest(true);
}

void MainWindow::cleanRecent_action()
{
    QSettings set("ccr.ini",QSettings::IniFormat);
    set.clear();
    updateRecentContest(true);
}

void MainWindow::editFile_action()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir_action+file_action));
}

void MainWindow::createFile_action()
{
    CreateFileDialog dialog(this,player_action,problem_action);
    //dialog.ui.
    if (dialog.exec())
    {
        if (!QDir(dir_action).exists()) QDir().mkpath(dir_action);
        QString s=dialog.selectedFile;
        QFile file(dir_action+s);
        file.open(QIODevice::WriteOnly);
        file.close();
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir_action+s));
    }
}

void MainWindow::openDir_action()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir_action));
}

void MainWindow::createDir_action()
{
    QDir().mkpath(dir_action);
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir_action));
}

void MainWindow::removeDir_action()
{
    QString t1,t2;
    if (dir_action==Global::srcPath+file_action+"/") t1="删除选手",t2=QString("确实要删除选手 \"%1\" 及其目录中的所有文件吗？").arg(file_action);
    else if (dir_action==Global::dataPath+file_action+"/") t1="删除试题",t2=QString("确实要删除试题 \"%1\" 及其目录中的所有文件吗？").arg(file_action);
    if (QMessageBox::question(this,t1,t2)==QMessageBox::Yes)
    {
        QDir(dir_action).removeRecursively();
        if (t1=="删除试题") QDir(Global::resultPath+file_action).removeRecursively();
        loadBoardEvent();
    }
}

void MainWindow::createActions()
{
    for (int i=0; i<20; i++)
    {
        actionR[i]=new QAction(this);
        actionR[i]->setVisible(false);
        ui->menuRecent->addAction(actionR[i]);
        connect(actionR[i],SIGNAL(triggered()),this,SLOT(openRecentContest()));
    }
    ui->menuRecent->addSeparator();

    recentListMenu=new QMenu(this);
    tableMenu=new QMenu(boardTable);
    headerMenu=new QMenu(boardTable->horizontalHeader());
    actionClean=new QAction("清空列表(&C)",this);
    actionRemove=new QAction("从列表中移除(&R)",this);
    actionEditFile=new QAction("编辑源代码(&E)...",this);
    actionCreateFile=new QAction("创建源代码(&W)...",this);
    actionOpenDir=new QAction("打开目录(&O)...",this);
    actionCreateDir=new QAction("创建并打开目录(&C)...",this);
    actionRemoveDir=new QAction("删除目录(&R)...",this);
    ui->menuRecent->addAction(actionClean);

    connect(actionClean,SIGNAL(triggered()),this,SLOT(cleanRecent_action()));
    connect(actionRemove,SIGNAL(triggered()),this,SLOT(removeRecent_action()));
    connect(actionEditFile,SIGNAL(triggered()),this,SLOT(editFile_action()));
    connect(actionCreateFile,SIGNAL(triggered()),this,SLOT(createFile_action()));
    connect(actionOpenDir,SIGNAL(triggered()),this,SLOT(openDir_action()));
    connect(actionCreateDir,SIGNAL(triggered()),this,SLOT(createDir_action()));
    connect(actionRemoveDir,SIGNAL(triggered()),this,SLOT(removeDir_action()));

    boardTable->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->recentList,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(recentListMenuEvent(const QPoint&)));
    connect(boardTable->horizontalHeader(),SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(headerMenuEvent(const QPoint&)));
    connect(boardTable,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(tableMenuEvent(const QPoint&)));
}

void MainWindow::recentListMenuEvent(const QPoint&pos)
{
    actionClean->setEnabled(ui->menuRecent->isEnabled());
    actionRemove->setEnabled(true);

    recentListMenu->clear();
    QListWidgetItem*item=ui->recentList->itemAt(pos);
    if (item)
    {
        dir_action=path_action=item->text();
        actionOpenDir->setText("打开目录(&O)...");
        recentListMenu->addAction(actionOpenDir);
        recentListMenu->addSeparator();
        recentListMenu->addAction(actionRemove);
    }
    recentListMenu->addAction(actionClean);
    recentListMenu->popup(QCursor::pos());
}

void MainWindow::headerMenuEvent(const QPoint&pos)
{
    headerMenu->clear();
    int c=boardTable->horizontalHeader()->logicalIndexAt(pos);
    if (c>=0)
    {
        actionCreateFile->setEnabled(true);
        actionOpenDir->setEnabled(true);
        actionCreateDir->setEnabled(true);

        if (!c) dir_action=Global::srcPath;
        else if (c==1) dir_action=Global::dataPath;
        else dir_action=Global::dataPath+Global::problems[c-2].name+"/";

        if (QDir(dir_action).exists())
        {
            if (!c) actionOpenDir->setText("打开选手目录(&O)...");
            else if (c==1) actionOpenDir->setText("打开试题目录(&O)...");
            else actionOpenDir->setText("打开目录(&O)...");
            headerMenu->addAction(actionOpenDir);
            if (c>1)
            {
                file_action=Global::problems[c-2].name;
                headerMenu->addSeparator();
                actionRemoveDir->setText(QString("删除试题 \"%1\"(&R)").arg(file_action));
                headerMenu->addAction(actionRemoveDir);
            }
        }
        else headerMenu->addAction(actionCreateDir);

        headerMenu->popup(QCursor::pos());
    }
}

void MainWindow::tableMenuEvent(const QPoint&pos)
{
    actionCreateFile->setEnabled(true);
    actionOpenDir->setEnabled(true);
    actionCreateDir->setEnabled(true);
    actionOpenDir->setText("打开目录(&O)...");
    tableMenu->clear();
    QTableWidgetItem*item=boardTable->itemAt(pos);
    if (item)
    {
        int r=item->row(),c=item->column();
        detailTable->showDetailEvent(r,c);
        r=Global::logicalRow(r);
        Player*player=&Global::players[r];
        if (c>1)
        {
            Problem*problem=&Global::problems[c-2];
            dir_action=Global::srcPath+player->name+"/"+problem->dir+"/";
            file_action=Global::getCompiler(player,problem).file;

            if (file_action=="")
            {
                player_action=player,problem_action=problem;
                if (problem->type==AnswersOnly||!problem->compilers.size()) actionCreateFile->setEnabled(false);
                tableMenu->addAction(actionCreateFile);
            }
            else
            {
                actionEditFile->setText(QString("编辑 \"%1\" (&E)...").arg(file_action));
                tableMenu->addAction(actionEditFile);
            }

            tableMenu->addSeparator();

            if (QDir(dir_action).exists()) tableMenu->addAction(actionOpenDir);
            else if (problem->type==AnswersOnly||!problem->compilers.size()) tableMenu->addAction(actionCreateDir);
            else
            {
                actionOpenDir->setEnabled(false);
                tableMenu->addAction(actionOpenDir);
            }
        }
        else
        {
            dir_action=Global::srcPath+player->name+"/";
            file_action=player->name;

            if (QDir(dir_action).exists())
            {
                tableMenu->addAction(actionOpenDir);
                tableMenu->addSeparator();
                actionRemoveDir->setText(QString("删除选手 \"%1\"(&R)").arg(file_action));
                tableMenu->addAction(actionRemoveDir);
            }
            else tableMenu->addAction(actionCreateDir);
        }
    }
    else
    {
        tableMenu->addAction(ui->actionRefresh);
        tableMenu->addSeparator();
        tableMenu->addAction(ui->actionJudgeSelect);
        tableMenu->addAction(ui->actionJudgeUnjudged);
        tableMenu->addAction(ui->actionJudgeAll);

        tableMenu->addAction(ui->actionStop);
    }
    tableMenu->popup(QCursor::pos());
}

void MainWindow::prepare() //初始化
{
    //读取src文件夹, data文件夹与.ccr文件

    QStringList playerName=readFolders(Global::srcPath),problemName=readFolders(Global::dataPath),order=readProblemOrder(Global::testPath);
    map<QString,int> playerID,problemID;
    problemID.clear();
    playerID.clear();

    QStringList tmp;
    for (auto i:order) if (problemName.count(i)) tmp.append(i);
    for (auto i:problemName) if (!tmp.count(i)) tmp.append(i);
    problemName=tmp;
    Global::saveProblemOrder(problemName);
    for (auto i:problemName)
    {
        Problem tp(i);
        problemID[i]=Global::problemNum;
        Global::problemOrder.push_back(Global::problemNum++);
        Global::problems.push_back(tp);
    }

    playerLabelLength=75;
    for (auto i:playerName)
    {
        playerLabelLength=max(playerLabelLength,QFontMetrics(Global::font).width(i)+30);

        Player tp(i,Global::playerNum);
        playerID[i]=Global::playerNum;
        Global::playerNum++;

        for (int j=0; j<Global::problemNum; j++) tp.problem.push_back(Player::Result());
        Global::players.push_back(tp);
    }

    //qDebug()<<playerLabelLength;

    boardTable->horizontalHeader()->resizeSection(0,playerLabelLength);
    boardTable->setRowCount(playerName.size());
    boardTable->setColumnCount(Global::problemNum+2);
    boardTable->setHorizontalHeaderLabels(QStringList({"选手","总分"})+problemName);
    for (int i=0; i<Global::problemNum+2; i++)  boardTable->horizontalHeaderItem(i)->setToolTip(boardTable->horizontalHeaderItem(i)->text());

    //读取result文件夹

    QFile file(Global::resultPath+".reslist");
    if (file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QTextStream in(&file);
        for (;!in.atEnd();)
        {
            QString str=in.readLine();
            QStringList list=str.split("/");
            QString a=list[0],b=list[1];
            int s=list[2].toInt();
            double t=list[3].toDouble();
            char c=list[4][0].toLatin1();
            if (!playerID.count(a)||!problemID.count(b)) continue;
            int x=playerID[a],y=problemID[b];

            //兼容旧版
            if (c=='R') c='N';
            if (c=='O') c='N';
            if (c!='N'&&c!='C'&&c!='F'&&c!='S'&&c!='E') c=0;
            Global::players[x].problem[y].state=c;
            Global::players[x].problem[y].score=s;
            Global::players[x].problem[y].usedTime=t;
        }
        file.close();
    }
    for (auto i:problemID)
        for (auto j:playerID) if (!Global::players[j.second].problem[i.second].state)
        {
            file.setFileName(Global::resultPath+i.first+"/"+j.first+".res");
            if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) continue;
            QDomDocument doc;
            if (!doc.setContent(&file)) {file.close(); continue;}
            QDomElement root=doc.documentElement();
            //qDebug()<<i.first<<i.second<<j.first<<j.second<<root.attribute("score")<<root.attribute("time")<<root.attribute("state");

            int s=root.attribute("score").toInt();
            double t=root.attribute("time").toDouble();
            char c=root.attribute("state")[0].toLatin1();
            int x=j.second,y=i.second;

            //兼容旧版
            if (c=='R') c='N';
            if (c=='O') c='N';
            if (c!='N'&&c!='C'&&c!='F'&&c!='S'&&c!='E') c=0;
            Global::players[x].problem[y].state=c;
            Global::players[x].problem[y].score=s;
            Global::players[x].problem[y].usedTime=t;

            file.close();
        }

    //读取.prb文件

    for (auto&i:Global::problems) i.readConfig();

    for (auto i:Global::problems) Global::sumScore+=i.sumScore;
    for (auto&i:Global::players) i.calcSum();

    saveResultList();
}

QStringList MainWindow::readFolders(const QString&path)
{
    QDir dir(path);
    QStringList list=dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
    if (path==Global::dataPath)
    {
        QStringList tmp;
        for (auto i:list) if (QFile::exists(Global::dataPath+i+"/.prb")) tmp.append(i);
        return tmp;
    }
    else return list;
}

QStringList MainWindow::readProblemOrder(const QString&path)
{
    QFile file(path+".ccr");
    QStringList problem;
    if (file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QDomDocument doc;
        if (doc.setContent(&file))
        {
            QDomElement root=doc.documentElement();
            if (root.isNull()||root.tagName()!="contest") {file.close(); return problem;}
            QDomNodeList list=root.childNodes();
            int t=0;
            for (int i=0; i<list.count(); i++)
            {
                QDomElement a=list.item(i).toElement();
                if (a.tagName()=="order")
                {
                    QDomNodeList l=a.childNodes();
                    for (int j=0; j<l.size(); j++)
                    {
                        QDomElement b=l.item(j).toElement();
                        if (b.tagName()=="problem") problem.append(b.text());
                    }
                }
            }
        }
        file.close();
    }
    return problem;
}

void MainWindow::saveResultList()
{
    QFile file(Global::resultPath+".reslist");
    if (file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        for (int i=0; i<Global::players.size(); i++)
        {
            int t=Global::logicalRow(i);
            Player*p=&Global::players[t];
            for (auto j:Global::problemOrder)
            {
                Player::Result*r=&p->problem[j];
                out<<p->name<<'/'<<Global::problems[j].name<<'/'<<r->score<<'/'<<r->usedTime<<'/'<<r->state<<'/'<<endl;
            }
        }
        file.close();
    }
}

void MainWindow::judgeEvent(int r, int c)
{
    //qDebug()<<r<<c;
    if (Global::alreadyJudging) return;

    Global::alreadyJudging=true;
    Global::clickTimer.start();

    boardTable->clearHighlighted(boardTable->preHeaderClicked);
    boardTable->preHeaderClicked=-1;
    boardTable->setSelectionMode(QAbstractItemView::NoSelection);
    boardTable->horizontalHeader()->setSectionsMovable(false);
    boardTable->horizontalHeader()->setSortIndicatorShown(false);
    boardTable->horizontalHeader()->setSortIndicator(-1,Qt::AscendingOrder);
    ui->actionConfig->setEnabled(false);
    ui->actionList->setEnabled(false);
    ui->actionExport->setEnabled(false);
    ui->actionRefresh->setEnabled(false);
    ui->actionJudgeSelect->setEnabled(false);
    ui->actionJudgeUnjudged->setEnabled(false);
    ui->actionJudgeAll->setEnabled(false);
    ui->actionStop->setEnabled(true);

    detailTable->clearDetail();
    judgeStoped=false;

    Global::judgeList.clear();
    if (r==-1)
    {
        for (int i=0; i<Global::playerNum; i++)
            for (auto j:Global::problemOrder) if (boardTable->item(i,j+2)->isSelected()) Global::judgeList.append(make_pair(i,j+2));
    }
    else if (r==-2)
    {
        for (int i=0; i<Global::playerNum; i++)
            for (auto j:Global::problemOrder) if (Global::players[Global::logicalRow(i)].problem[j].state==' ') Global::judgeList.append(make_pair(i,j+2));
    }
    else if (c>1) Global::judgeList.append(make_pair(r,c));

    judger->r=r;
    judger->c=c;
    QEventLoop*eventLoop=new QEventLoop(this);
    connect(judger,SIGNAL(finished()),eventLoop,SLOT(quit()));
    judger->start();
    eventLoop->exec();
    delete eventLoop;

    Global::alreadyJudging=false;
    ui->actionConfig->setEnabled(true);
    ui->actionList->setEnabled(true);
    ui->actionExport->setEnabled(true);
    ui->actionRefresh->setEnabled(true);
    ui->actionJudgeSelect->setEnabled(true);
    ui->actionJudgeUnjudged->setEnabled(true);
    ui->actionJudgeAll->setEnabled(true);
    ui->actionStop->setEnabled(false);
    boardTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    boardTable->horizontalHeader()->setSectionsMovable(true);
    this->activateWindow();
   // ui->tableWidget->horizontalHeader()->setEnabled(true);

    saveResultList();
    judger->waitForClearedTmpDir(2000);
}

void MainWindow::solt1(QLabel *label, const QString &s1, const QString &s2, const QString &s3)
{
    if (contestClosed) return;
    label->setText(s1);
    label->setToolTip(s2);
    label->setStyleSheet(s3);
}

void MainWindow::solt2(Player *ply, int c, Player::Result *res, int sum)
{
    if (contestClosed) return;
    if (c==1) ply->style[1]=boardTable->showProblemSumResult(ply->label[1],res,sum,1);
    else ply->style[c]=boardTable->showProblemResult(ply->label[c],res,sum,c);
}

void MainWindow::slot3(int row, const QString &title)
{
    if (contestClosed) return;
    detailTable->addTitleDetail(row,title);
    detailTable->adjustScrollbar();
}

void MainWindow::slot4(int row, const QString &note, const QString &state)
{
    if (contestClosed) return;
    detailTable->addNoteDetail(row,note,state);
    if (state==" ") detailTable->item(row,0)->setTextAlignment(Qt::AlignCenter);
    detailTable->adjustScrollbar();
}

void MainWindow::slot5(int row, int num, const QString &note, const QString &state, const QString &file,int len)
{
    if (contestClosed) return;
    detailTable->addPointDetail(row,num,note,state,file);
    if (len>1) detailTable->setSpan(row-len+1,0,len,1);
    detailTable->adjustScrollbar();
}

void MainWindow::slot6(int row, int len, int score, int sumScore)
{
    if (contestClosed) return;
    detailTable->addScoreDetail(row,len,score,sumScore);
}

void MainWindow::slot7(int r, int c)
{
    if (contestClosed) return;
    boardTable->setCurrentItem(boardTable->item(r,c));
}

void MainWindow::slot8(int r, int c)
{
    if (contestClosed) return;
    boardTable->item(r,c)->setSelected(false);
}

void MainWindow::readPlayerList(QFile&file,bool isCSV)
{
    map<QString,QString> list;
    list.clear();
    QTextStream in(&file);
    for (;!in.atEnd();)
    {
        QString s=in.readLine();
//        in.setCodec("UTF-8");
        QStringList line=s.split(",");
        for (auto&i:line) i=i.trimmed(),i.remove('\"');
        if (line.size()>=2&&line[0].size()&&line[1].size()) list[line[0]]=line[1];
    }

    if (isCSV)
    {
        QFile f(Global::testPath+".list");
        if (f.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            QTextStream out(&f);
//            out.setCodec("UTF-8");
            for (auto i:list) out<<i.first<<","<<i.second<<endl;
            f.close();
        }
    }

    int len=75;
    for (auto&i:Global::players) if (!i.type)
    {
        QLabel*tmp=i.label[0];
        if (list.count(i.name))
        {
            i.name_list=list[i.name];
            if (i.name_list.length()==2) tmp->setText(QString("%1 [%2   %3]").arg(i.name,i.name_list.at(0),i.name_list.at(1)));
            else tmp->setText(QString("%1 [%2]").arg(i.name,i.name_list));
            tmp->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            tmp->setStyleSheet("");
        }
        else tmp->setStyleSheet("QLabel{color:rgb(120,120,120);}");
        len=max(len,QFontMetrics(Global::font).width(tmp->text())+30);
    }
    boardTable->horizontalHeader()->resizeSection(0,len);
}

void MainWindow::on_actionList_triggered()
{
    if (Global::isListUsed)
    {
        if (QMessageBox::question(this,"移除选手名单","确实要移除选手名单吗？")==QMessageBox::No)
        {
            Global::isListUsed=true;
            ui->actionList->setChecked(true);
            return;
        }
        Global::isListUsed=false;
        ui->actionList->setChecked(false);
        QFile(Global::testPath+".list").remove();

        for (auto&i:Global::players) if (!i.type)
        {
            QLabel*tmp=i.label[0];
            i.name_list="",tmp->setText(i.name);
            tmp->setAlignment(Qt::AlignCenter);
            tmp->setStyleSheet("");
        }
        boardTable->horizontalHeader()->resizeSection(0,playerLabelLength);
    }
    else
    {
        QString fileName=QFileDialog::getOpenFileName(this,"导入选手名单",Global::testPath,"CSV (逗号分隔) (*.csv)");
        if (!fileName.size())
        {
            Global::isListUsed=false;
            ui->actionList->setChecked(false);
            return;
        }
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly|QIODevice::Text))
            QMessageBox::critical(this,"导入名单失败",file.errorString());
        else
        {
           Global::isListUsed=true;
           ui->actionList->setChecked(true);
           readPlayerList(file,1);
           file.close();
        }
    }
    detailTable->clearDetail();
}

void MainWindow::on_actionExport_triggered()
{
    QString fileName=QFileDialog::getSaveFileName(this,"导出成绩",Global::testPath+Global::testName+".csv","CSV (逗号分隔) (*.csv)");
    if (!fileName.size()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        QMessageBox::critical(this,"导出成绩失败",file.errorString());
    else
    {
        QTextStream out(&file);
        //out.setCodec("UTF-8");
        if (Global::isListUsed) out<<tr("编号,")<<tr("姓名,"); else out<<tr("选手,");
        out<<tr("总分,");
        for (auto j:Global::problemOrder) out<<QString("\"%1\",").arg(Global::problems[j].name);
        out<<endl;
        for (int i=0; i<Global::players.size(); i++)
        {
            int t=Global::logicalRow(i);
            Player*p=&Global::players[t];
            out<<QString("\"%1\",").arg(p->name);
            if (Global::isListUsed) out<<QString("\"%1\",").arg(p->name_list);
            out<<p->sum.score<<",";
            for (auto j:Global::problemOrder) out<<p->problem[j].score<<",";
            out<<endl;
        }
        file.close();
    }
}

void MainWindow::on_actionJudgeSelect_triggered()
{
    judgeEvent(-1,-1);
}

void MainWindow::on_actionJudgeUnjudged_triggered()
{
    judgeEvent(-2,-2);
    //ui->tableWidget->setCurrentItem(ui->tableWidget->item(11,5));
}

void MainWindow::on_actionJudgeAll_triggered()
{
    boardTable->selectAll();
    judgeEvent(-1,-1);
}

void MainWindow::on_actionRefresh_triggered()
{
    loadBoardEvent();
}

void MainWindow::on_actionStop_triggered()
{
    stopEvent();
}

void MainWindow::on_actionOpen_triggered()
{
    if (Global::alreadyJudging)
    {
        stopEvent();
        QMessageBox::warning(this,"",QString("测评被终止！"));
    }

    QDir dir(lastContest);
    dir.cdUp();
    QString path=QFileDialog::getExistingDirectory(this,"打开竞赛目录",dir.path());
    if (path.size()) loadContestEvent(path);
}

void MainWindow::on_actionClose_triggered()
{
    closeContestEvent();

    ui->label->show();
    ui->label_2->show();
    ui->recentList->show();
    closeButton->hide();
    updateRecentContest(true);
    this->setWindowTitle("CCR Plus 测评器");
    ui->actionClose->setEnabled(false);

    ui->actionConfig->setEnabled(false);
    ui->actionList->setEnabled(false);
    ui->actionExport->setEnabled(false);
    ui->actionRefresh->setEnabled(false);
    ui->actionJudgeSelect->setEnabled(false);
    ui->actionJudgeUnjudged->setEnabled(false);
    ui->actionJudgeAll->setEnabled(false);
    ui->actionStop->setEnabled(false);
    ui->actionList->setChecked(false);
}

void MainWindow::on_actionExit_triggered()
{
    if (ui->actionClose->isEnabled()) closeContestEvent();
    QApplication::exit();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox*msgBox=new QMessageBox(this);
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

void MainWindow::on_recentList_itemDoubleClicked(QListWidgetItem *item)
{
    if (item) loadContestEvent(item->text());
}

void MainWindow::on_actionConfig_triggered()
{
    ConfigDialog dialog(this);
    if (dialog.exec())
    {
        loadBoardEvent();
        detailTable->showConfigDetail();
    }
}

void MainWindow::on_actionHelp_triggered()
{

}
