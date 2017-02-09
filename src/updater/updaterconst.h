#ifndef UPDATERCONST_H
#define UPDATERCONST_H

#include <QString>
#include <QJsonObject>

namespace Updater
{

//const QString APP_NAME = "arena9-win32-ia32";
const QString APP_NAME = "CCR-Plus";

#if defined(Q_OS_WIN)
const QString UPDATER_NAME = "upgrader.exe";
const QString RUN_APP_CMD = "CCR-Plus.exe";
#elif defined(Q_OS_LINUX)
const QString UPDATER_NAME = "upgrader";
const QString RUN_APP_CMD = "run.sh";
#endif

const QString RELEASE_INFO_URL = "https://api.github.com/repos/sxyzccr/CCR-Plus/releases/latest";

const int CONNECT_TIME_OUT = 60000;

/// 获取不同系统的下载文件名
QString GetFileNameByPlatform(const QString& version);

/// 格式化文件大小
QString FormatFileSize(qint64 bytes);

/// 格式化时间
QString FormatTime(int seconds);

/// 解析 JSON 数据
QJsonObject ParseToJson(const QByteArray& data, QString* errorString = nullptr);

}  // namespace Updater

#endif // UPDATERCONST_H
