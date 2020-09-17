#include <QDir>
#include <QFile>
#include <QString>
#include <QProcess>
#include <QTranslator>
#include <QApplication>
#include <QStandardPaths>
#include <iostream>

#include "../common/version.h"
#include "updaterconst.h"
#include "downloaddialog.h"
#include "checkupdatesdialog.h"

const QString tempPath = QDir::tempPath() + "/" + Updater::APP_NAME + "/";
const QString tempUpdaterPath = tempPath + Updater::UPDATER_NAME;

#if defined(Q_OS_WIN)

#define INITGUID
#include <shobjidl.h>
#include <shlguid.h>
#include <windows.h>

/// 杀死进程 pid
void killProcess(int pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess != NULL) TerminateProcess(hProcess, 0);
}

/// 创建快捷方式
bool createShortcut(const QString& installDir, const QString& destinationDir)
{
    CoInitialize(NULL);
    IShellLink* pShellLink = NULL;

    HRESULT hres;
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_ALL, IID_IShellLink, (void**)&pShellLink);

    LPCTSTR appPath = (LPCTSTR)QDir(installDir).absoluteFilePath(Updater::RUN_APP_CMD).utf16();
    LPCTSTR lnkPath = (LPCTSTR)QDir(destinationDir).absoluteFilePath("CCR Plus 测评器.lnk").utf16();

    if (SUCCEEDED(hres))
    {
        pShellLink->SetPath(appPath);
        pShellLink->SetDescription(L"CCR Plus 测评器");
        pShellLink->SetWorkingDirectory((LPCTSTR)installDir.utf16());

        IPersistFile* pPersistFile;
        hres = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);

        if (SUCCEEDED(hres))
        {
            hres = pPersistFile->Save(lnkPath, TRUE);
            pPersistFile->Release();
        }
        pShellLink->Release();
    }
    CoUninitialize();

    return SUCCEEDED(hres);
}

#elif defined(Q_OS_LINUX)

/// 杀死进程 pid
void killProcess(int pid)
{
    QProcess::execute(QString("kill -9 %1").arg(pid));
}

/// 创建快捷方式
bool createShortcut(const QString& installDir, const QString& destinationDir)
{
    return !QProcess::execute(QString("%1/install.sh").arg(installDir), { installDir, destinationDir });
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

    if (!QDir(tempPath).exists()) QDir().mkpath(tempPath);

// Debug for download
//    for (;;)
//    {
//        //DownloadDialog dialog("file:///C:/Users/Equation/Desktop/CCR-Plus_1.0.0.150712_release_windows_x86.zip", "C:/Users/equation/Desktop/a");
//        //DownloadDialog dialog("file:///home/equation/Desktop/CCR-Plus_v1.1.0_linux_x64.zip", "/home/equation/Desktop/a");
//        //DownloadDialog dialog("https://svwh.dl.sourceforge.net/project/ccr-plus/Beta/1.0.0.150712_beta2/CCR-Plus_1.0.0.150712_beta2_linux_x64.zip", "/home/equation/Desktop/a");

//        dialog.exec();
//        qDebug()<<dialog.result();
//        if (dialog.result() == QDialog::Accepted)
//        {
//            if (dialog.NeedRedownload()) continue;
//        }
//        if (dialog.ShouldCreateShortcut())
//            createShortcut("/home/equation/Desktop/a", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

//        return 0;
//    }

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

    if (_h || (!_c && !_d && !_v)) // help info
    {
        std::cout << "Usage: " << Updater::UPDATER_NAME.toStdString() << " [options]\n";
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
        QString updaterPath = appPath + "/" + Updater::UPDATER_NAME;

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
        //url = "file:///C:/Users/Equation/Desktop/CCR-Plus_1.0.0.150712_release_windows_x86.zip";
        //url = "file:///home/equation/Desktop/CCR-Plus_v1.1.0_linux_x64.zip";
        //url = "https://svwh.dl.sourceforge.net/project/ccr-plus/Beta/1.0.0.150712_beta2/CCR-Plus_1.0.0.150712_beta2_linux_x64.zip";
        for (;;)
        {
            DownloadDialog dialog(url, dir);
            if (dialog.exec() == QDialog::Accepted)
            {
                if (dialog.NeedRedownload()) continue;
                QProcess::startDetached(QDir(dir).absoluteFilePath(Updater::RUN_APP_CMD), {}, dir);
            }
            if (dialog.ShouldCreateShortcut())
                createShortcut(dir, QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

            break;
        }
    }
}
