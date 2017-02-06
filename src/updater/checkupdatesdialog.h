#ifndef CHECKUPDATESDIALOG_H
#define CHECKUPDATESDIALOG_H

#include <QDialog>

class QTimer;
class QPushButton;
class QNetworkReply;
class QNetworkAccessManager;

namespace Ui
{
class CheckUpdatesDialog;
}

class CheckUpdatesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CheckUpdatesDialog(bool dontShowError = false, QWidget* parent = nullptr);
    ~CheckUpdatesDialog();

    /// 重新检查更新
    bool NeedRecheck() const { return need_recheck; }

    QString DownloadUrl() const { return download_url; }
    int DownloadSize() const { return download_size; }

public slots:
    virtual int exec() override;

private:
    Ui::CheckUpdatesDialog* ui;
    QPushButton* download_button;

    QNetworkAccessManager* manager;
    QNetworkReply* reply;
    QTimer* timer;

    bool dont_show_error, need_recheck;
    QString download_url;
    int download_size;

    /// 获取不同系统的下载文件名
    inline static QString getFileName(const QString& version)
    {
#if defined(Q_OS_WIN)
        return QString("CCR-Plus_%1_windows_x86.zip").arg(version);
#elif defined(Q_OS_LINUX)
#   if defined(Q_PROCESSOR_X86_64)
        return QString("CCR-Plus_%1_linux_x64.zip").arg(version);
#   else
        return QString("CCR-Plus_%1_linux_x86.zip").arg(version);
#   endif
#endif
    }

    /// 解析 JSON 数据
    inline static QJsonObject parseToJson(const QByteArray& data, QString* errorString = nullptr);

    /// 发生错误，会阻塞其他操作
    void showError(const QString& msg);

    /// 没有更新
    void noUpdates();

    /// 发现更新
    void foundUpdates(const QString& version, const QString& log, int bytes);

private slots:
    void onReplyFinished();
};

#endif // CHECKUPDATESDIALOG_H
