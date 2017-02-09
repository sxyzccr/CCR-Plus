#include <QDir>
#include <QFile>
#include <QTimer>
#include <QPushButton>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include "updaterconst.h"
#include "mycompress.h"
#include "downloaddialog.h"
#include "ui_downloaddialog.h"

DownloadDialog::DownloadDialog(const QString urlString, const QString installDir, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::DownloadDialog),
    redownload_button(new QPushButton("重新下载(&R)", this)),
    manager(new QNetworkAccessManager(this)), timer(new QTimer(this)),
    url_string(urlString), file_name(QFileInfo(urlString).fileName()), download_dir(QDir::tempPath() + "/CCR-Plus/"), install_dir(installDir),
    file_downloaded(new QFile(download_dir + file_name, this)), file_tmp(new QFile(download_dir + file_name + ".download", this)),
    last_bytes(0), current_bytes(0), total_bytes(0),
    download_ok(false), uncompress_ok(false), need_redownload(false)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    ui->buttonBox->addButton(redownload_button, QDialogButtonBox::AcceptRole);
    redownload_button->hide();
    this->setFixedHeight(this->sizeHint().height());

    file_downloaded->remove();
    file_tmp->remove();
    if (!file_tmp->open(QIODevice::WriteOnly))
    {
        showError("下载失败: 无法创建临时文件。", false);
        return;
    }

    connect(redownload_button, &QPushButton::clicked, this, &DownloadDialog::onRedownload);
    connect(timer, &QTimer::timeout, this, &DownloadDialog::onTimeOut);

    timer->start(500);
}

DownloadDialog::~DownloadDialog()
{
    delete ui;
}



void DownloadDialog::showError(const QString& msg, bool isBlocked)
{
    ui->label_info->setText(msg);
    ui->label_size->hide();
    ui->progressBar->hide();
    redownload_button->show();
    this->setFixedHeight(120);

    if (isBlocked)
    {
        QEventLoop loop;
        connect(this, &QDialog::finished, &loop, &QEventLoop::quit);
        loop.exec();
    }
}

bool DownloadDialog::uncompress()
{
    ui->label_info->setText("正在解压...");
    ui->buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
    QApplication::processEvents();

    QString error = MyCompress::extractDir(file_downloaded->fileName(), Updater::APP_NAME, install_dir);
    if (!error.isEmpty())
    {
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Retry | QDialogButtonBox::Cancel);
        showError(QString("解压失败: %1").arg(error));
        return false;
    }
    uncompress_ok = true;
    return true;
}

void DownloadDialog::updateSuccessed()
{
    ui->label_info->setText("更新成功!");
    ui->label_size->hide();
    ui->progressBar->hide();
    ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
    redownload_button->setText("立即启动(&S)");
    this->setFixedHeight(120);
}



void DownloadDialog::onTimeOut()
{
    if (timer->interval() == 500) // start download
    {
        onStartDownload();
        timer->start(Updater::CONNECT_TIME_OUT);
        return;
    }
    else if (timer->interval() == Updater::CONNECT_TIME_OUT) // connect timeout
    {
        if (reply->isRunning())
        {
            reply->abort();
            showError("下载失败: 连接超时，请检查网络连接。");
        }
        return;
    }

    static int zero_speed_times = 0;
    qint64 speed = current_bytes - last_bytes;
    last_bytes = current_bytes;
    ui->progressBar->setValue(current_bytes);

    if (!speed)
        zero_speed_times++;
    else
        zero_speed_times = 0;

//    if (zero_speed_times > Updater::CONNECT_TIME_OUT / 1000)
//    {
//        reply->abort();
//        showError("下载失败: 连接超时，请检查网络连接。");
//        return;
//    }
    if (speed || zero_speed_times >= 2)
    {
        QString info = QString("%1/s - %2").arg(Updater::FormatFileSize(speed)).arg(Updater::FormatFileSize(current_bytes));
        if (total_bytes) info += QString(", 共 %1").arg(Updater::FormatFileSize(total_bytes));
        if (speed && total_bytes) info += QString(", 还剩 %1").arg(Updater::FormatTime(1.0 * (total_bytes - current_bytes) / speed + 0.5));
        ui->label_size->setText(info);
    }
}

void DownloadDialog::onStartDownload()
{
    QNetworkRequest request = QNetworkRequest(QUrl(url_string));
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    reply = manager->get(request);

    connect(this, &QDialog::finished, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, this, &DownloadDialog::onDownloadFinished);
    connect(reply, &QNetworkReply::downloadProgress, this, &DownloadDialog::onUpdateDownloadProgress);
    connect(reply, &QNetworkReply::readyRead, this, [this]()
    {
        file_tmp->write(reply->readAll());
    });
}

void DownloadDialog::onRedownload()
{
    if (!download_ok) reply->abort();
    if (!uncompress_ok) need_redownload = true;
    QDialog::accept();
}

void DownloadDialog::onUpdateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    current_bytes = bytesReceived;
    total_bytes = bytesTotal == -1 ? 0 : bytesTotal;
    ui->progressBar->setMaximum(total_bytes);
    if (total_bytes) ui->progressBar->setTextVisible(true);

    if (timer->interval() == Updater::CONNECT_TIME_OUT) timer->stop();
    if (!timer->isActive())
    {
        timer->start(1000);
        ui->label_info->setText(QString("正在下载 %1 ...").arg(file_name));
        ui->progressBar->setValue(0);
        redownload_button->show();
        onTimeOut();
    }
}

void DownloadDialog::onDownloadFinished()
{
    file_tmp->close();
    timer->stop();
    if (reply->error() == QNetworkReply::NoError)
    {
        ui->label_info->setText("下载完成");
        ui->label_size->setText("");
        ui->progressBar->setValue(total_bytes);
        redownload_button->hide();
        download_ok = true;
        QApplication::processEvents();

        if (file_downloaded->exists()) file_downloaded->remove();
        if (!file_tmp->rename(file_downloaded->fileName()))
            showError(QString("下载失败: 无法创建文件 \"%1\"。").arg(file_downloaded->fileName()));
        else if (uncompress()) updateSuccessed();
    }
    else
    {
        if (reply->error() != QNetworkReply::OperationCanceledError)
            showError(QString("下载失败: 无法访问更新服务器(错误代码 %1)，请检查网络连接。").arg(reply->error()));
        file_tmp->remove();
    }
    reply->deleteLater();
}



void DownloadDialog::on_buttonBox_clicked(QAbstractButton* button)
{
    if (ui->buttonBox->standardButton(button) == QDialogButtonBox::Retry)
        if (uncompress()) updateSuccessed();
}
