#include <QDebug>
#include <QTimer>
#include <QPushButton>
#include <QJsonArray>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include "../common/version.h"
#include "updaterconst.h"
#include "checkupdatesdialog.h"
#include "ui_checkupdatesdialog.h"

CheckUpdatesDialog::CheckUpdatesDialog(bool dontShowError, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::CheckUpdatesDialog),
    download_button(new QPushButton("立即下载(&D)", this)),
    manager(new QNetworkAccessManager(this)), timer(new QTimer(this)), dont_show_error(dontShowError), need_recheck(false)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    ui->label_now->setText(VERSION_SHORT);
    ui->label_1->hide();
    ui->label_2->hide();
    ui->label_3->hide();
    ui->label_now->hide();
    ui->label_latest->hide();
    ui->plainTextEdit->hide();
    download_button->hide();
    this->setFixedHeight(120);
    this->setVisible(!dontShowError);

    qDebug() << "Request URL:" << Updater::RELEASE_INFO_URL;
    reply = manager->get(QNetworkRequest(QUrl(Updater::RELEASE_INFO_URL)));

    connect(this, &QDialog::finished, reply, &QNetworkReply::abort);
    connect(timer, &QTimer::timeout, this, [this]()
    {
        if (reply->isRunning())
        {
            reply->abort();
            showError("检查更新失败: 连接超时，请检查网络连接。");
        }
    });
    connect(manager, &QNetworkAccessManager::finished, this, &CheckUpdatesDialog::onReplyFinished);

    timer->setSingleShot(true);
    timer->start(Updater::CONNECT_TIME_OUT);
}

CheckUpdatesDialog::~CheckUpdatesDialog()
{
    delete ui;
}

int CheckUpdatesDialog::exec()
{
    if (dont_show_error)
    {
        QEventLoop loop;
        connect(this, &QDialog::finished, &loop, &QEventLoop::quit);
        loop.exec();
        return this->result();
    }
    else
        return QDialog::exec();
}



void CheckUpdatesDialog::showError(const QString& msg)
{
    if (dont_show_error)
    {
        qDebug() << msg;
        QDialog::reject();
        return;
    }

    if (need_recheck) return;
    need_recheck = true;

    ui->label_info->setText(msg);
    ui->progressBar->hide();
    ui->buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Retry);

    QEventLoop loop;
    connect(this, &QDialog::finished, &loop, &QEventLoop::quit);
    loop.exec();
}

void CheckUpdatesDialog::noUpdates()
{
    if (dont_show_error)
    {
        qDebug() << "没有更新";
        QDialog::reject();
        return;
    }

    this->setWindowTitle("没有更新");
    ui->label_info->setText(QString("%1 已是最新版本 %2。").arg(Updater::APP_NAME).arg(VERSION_SHORT));
    ui->progressBar->hide();
}

void CheckUpdatesDialog::foundUpdates(const QString& version, const QString& log, int bytes)
{
    this->setWindowTitle("发现新版本");
    ui->label_latest->setText(version);
    ui->label_info->setText(QString("将会下载 %1。").arg(Updater::FormatFileSize(bytes)));
    ui->plainTextEdit->setPlainText(log);
    ui->buttonBox->addButton(download_button, QDialogButtonBox::AcceptRole);

    ui->label_1->show();
    ui->label_2->show();
    ui->label_3->show();
    ui->label_now->show();
    ui->label_latest->show();
    ui->plainTextEdit->show();
    ui->progressBar->hide();
    download_button->show();

    this->setFixedSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    this->resize(400, 300);
    this->show();
}



void CheckUpdatesDialog::onReplyFinished()
{
    qDebug() << "Status Code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    timer->stop();
    if (reply->error() != QNetworkReply::NoError)
    {
        if (reply->error() != QNetworkReply::OperationCanceledError)
            showError(QString("检查更新失败: 无法访问更新服务器(错误代码 %1)，请检查网络连接。").arg(reply->error()));
        reply->deleteLater();
    }
    else if (download_url.isEmpty())
    {
        QString errorString;
        QJsonObject json = Updater::ParseToJson(reply->readAll(), &errorString);
        if (!errorString.isEmpty())
        {
            showError(QString("检查更新失败: %1").arg(errorString));
            reply->deleteLater();
            return;
        }

        QString latestVersion = json.value("tag_name").toString();
        if (latestVersion.isEmpty())
        {
            showError("检查更新失败: 无法获取新版本信息。");
            reply->deleteLater();
            return;
        }
        qDebug() << "Latest Version:" << latestVersion;

        if (latestVersion != VERSION_SHORT)
        {
            download_size = 0;
            QJsonArray array = json.value("assets").toArray();
            for (int i = 0; i < array.size(); i++)
            {
                QJsonObject obj = array[i].toObject();
                if (obj.value("name") == Updater::GetFileNameByPlatform(latestVersion))
                {
                    download_url = obj.value("browser_download_url").toString();
                    download_size = obj.value("size").toInt();
                    break;
                }
            }
            if (download_url.isEmpty() || !download_size)
            {
                showError("检查更新失败: 无法获取新版本下载地址。");
                reply->deleteLater();
                return;
            }

            reply->deleteLater();
            QString log = json.value("body").toString();
            foundUpdates(latestVersion, log, download_size);
        }
        else
        {
            noUpdates();
            reply->deleteLater();
        }
    }
}
