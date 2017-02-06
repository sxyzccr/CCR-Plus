#include <QDir>
#include <QFile>
#include <QString>
#include <QProcess>
#include <iostream>
#include <QDebug>
#include <QTranslator>
#include <QApplication>

#include "../common/version.h"
#include "checkupdatesdialog.h"

#if defined(Q_OS_WIN)
    const QString updaterName = "upgrader.exe"
#elif defined(Q_OS_LINUX)
    const QString updaterName = "upgrader";
#endif
const QString tempPath = QDir::tempPath() + "/CCR-Plus/";
const QString tempUpdaterPath = tempPath + updaterName;

#if defined(Q_OS_WIN)
#include <windows.h>

void killProcess(int pid)
{

}
#elif defined(Q_OS_LINUX)
void killProcess(int pid)
{

}
#endif

void cleanTemp()
{
    QDir(tempPath).removeRecursively();
    QDir().mkpath(tempPath);
}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    translator.load(":/trans/qt_zh_CN.qm");
    a.installTranslator(&translator);

    bool _c = argc == 1, _d = false, _n = false, _h = false, _v = false;
    int pid = 0;
    QString url, dir;

    for (int i = 1 ; i < argc; i++)
    {
        if (!strcmp(argv[i], "-c")) _c = true;
        else if (!strcmp(argv[i], "-p"))
        {
            if (i + 1 < argc) pid = QString(argv[i + 1]).toInt();
            i++;
        }
        else if (!strcmp(argv[i], "-d"))
        {
            _d = true;
            if (i + 1 < argc) url = QString::fromLocal8Bit(argv[i + 1]);
            if (i + 2 < argc) dir = QString::fromLocal8Bit(argv[i + 2]);
            i += 2;
        }
        else if (!strcmp(argv[i], "-n")) _n = true;
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) { _h = true; break; }
        else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) { _v = true; break; }
        else { _h = true; break; }
    }

    if (_h) // help info
    {
        std::cout << "Usage: " << updaterName.toStdString() << " [options]\n";
        std::cout << "  -h, --help           Display this information\n";
        std::cout << "  -v, --version        Display the version information\n";
        std::cout << "  -c                   Check for updates\n";
        std::cout << "  -n                   Do not show message if no updates found\n";
        std::cout << "  -p <pid>             Kill the process <pid> when accepted to download updates\n";
        std::cout << "  -d <url> <dir>       Download the file from <url> and install in the directory <dir>\n";
    }
    else if (_v) // version info
    {
        std::cout << "Update manager for CCR Plus judging environment.\n";
        std::cout << VERSION_LONG << "\n";
        std::cout << "Copyright(C) 2017 Yuekai Jia. All rights reserved.\n";
    }
    else if (_c) // check updates
    {
        cleanTemp();
        QString appPath = QCoreApplication::applicationDirPath();
        QString updaterPath = appPath + "/" + updaterName;

        for (;;)
        {
            CheckUpdatesDialog dialog(_n);
            if (dialog.exec() == QDialog::Accepted)
            {
                if (dialog.NeedRecheck()) continue;
                QFile::copy(updaterPath, tempUpdaterPath);
                QProcess::startDetached(tempUpdaterPath, { "-d", dialog.DownloadUrl(), appPath });
                if (pid > 0) killProcess(pid);
            }
            break;
        }
        return 0;
    }
    else if (_d) // download new files
    {

    }
}
