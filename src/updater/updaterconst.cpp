#include <QtMath>
#include <QJsonArray>
#include <QStringList>
#include <QJsonDocument>

#include "updaterconst.h"

QString Updater::GetFileNameByPlatform(const QString& version)
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

QString Updater::FormatFileSize(qint64 bytes)
{
    if (bytes <= 0) return "0 B";
    const QStringList units = { "B", "KB", "MB", "GB" };
    int index = qLn(bytes) / qLn(1024);
    double size = bytes / qPow(1024, index);
    return QString("%1 %2").arg(size, 0, 'f', 1).arg(units[index]);
}

QString Updater::FormatTime(int seconds)
{
    if (seconds < 60) return QString("%1 秒").arg(seconds);
    else
    {
        int hours = seconds / 3600;
        int minutes = (seconds % 3600) / 60;
        if (!minutes) return QString("%1 小时").arg(hours);
        else if (!hours) return QString("%1 分钟").arg(minutes);
        else return QString("%1 小时 %2 分钟").arg(hours).arg(minutes);
    }
}

QJsonObject Updater::ParseToJson(const QByteArray& data, QString* errorString)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (errorString)
    {
        *errorString = "";
        if (error.error)
            *errorString = QString("JSON 解析错误(错误代码 %1): %2").arg(error.error).arg(error.errorString());
        else if (!doc.isObject())
            *errorString = "JSON 解析失败, 不是 JSON 对象。";
    }
    return doc.object();
}
