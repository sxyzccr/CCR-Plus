#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

#include <QDialog>

class QFile;
class QTimer;
class QAbstractButton;
class QNetworkReply;
class QNetworkAccessManager;

namespace Ui
{
class DownloadDialog;
}

class DownloadDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DownloadDialog(const QString urlString, const QString installDir, QWidget* parent = nullptr);
    ~DownloadDialog();

    /// 需要重新下载
    bool NeedRedownload() const { return need_redownload; }

private:
    Ui::DownloadDialog* ui;
    QPushButton* redownload_button;

    QNetworkAccessManager* manager;
    QNetworkReply* reply;
    QTimer* timer;

    QString url_string, file_name, download_dir, install_dir;
    QFile *file_downloaded, *file_tmp;
    qint64 last_bytes, current_bytes, total_bytes;
    bool download_ok, uncompress_ok, need_redownload;

    /// 显示错误
    void showError(const QString& msg, bool isBlocked = true);

    /// 解压下载文件
    bool uncompress();

    /// 更新成功
    void updateSuccessed();

private slots:
    void onTimeOut();
    void onStartDownload();
    void onRedownload();
    void onUpdateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();

    void on_buttonBox_clicked(QAbstractButton* button);
};

#endif // DOWNLOADDIALOG_H
