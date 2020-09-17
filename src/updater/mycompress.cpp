#include <QDir>

#include "mycompress.h"
#include <quazip/quazipfile.h>

static bool copyData(QIODevice& inFile, QIODevice& outFile)
{
    while (!inFile.atEnd())
    {
        char buf[4096];
        qint64 readLen = inFile.read(buf, 4096);
        if (readLen <= 0)
            return false;
        if (outFile.write(buf, readLen) != readLen)
            return false;
    }
    return true;
}

QString MyCompress::extractFile(QuaZip* zip, QString fileName, QString fileDest)
{
    // zip: oggetto dove aggiungere il file
    // filename: nome del file reale
    // fileincompress: nome del file all'interno del file compresso

    // Apro il file compresso
    if (!fileName.isEmpty())
        zip->setCurrentFile(fileName);
    fileName = zip->getCurrentFileName();

    QuaZipFile inFile(zip);
    if (!inFile.open(QIODevice::ReadOnly) || inFile.getZipError() != UNZ_OK)
        return QString("无法解压文件 \"%1\" (错误代码 %2)。").arg(fileName).arg(inFile.getZipError());

    // Controllo esistenza cartella file risultato
    QDir curDir;
    if (fileDest.endsWith('/'))
    {
        if (!curDir.mkpath(fileDest))
            return QString("无法创建目录 \"%1\"。").arg(fileDest);
    }
    else
    {
        if (!curDir.mkpath(QFileInfo(fileDest).absolutePath()))
            return QString("无法创建目录 \"%1\"。").arg(QFileInfo(fileDest).absolutePath());
    }

    QuaZipFileInfo64 info;
    if (!zip->getCurrentFileInfo(&info))
        return QString("无法获取文件 \"%1\" 的信息。").arg(fileName);

    QFile::Permissions srcPerm = info.getPermissions();
    if (fileDest.endsWith('/') && QFileInfo(fileDest).isDir())
    {
        if (srcPerm != 0)
            QFile(fileDest).setPermissions(srcPerm);
        return "";
    }

    // Apro il file risultato
    QFile outFile;
    outFile.setFileName(fileDest);
    if (!outFile.open(QIODevice::WriteOnly))
        return QString("无法写入文件 \"%1\"。").arg(fileDest);

    // Copio i dati
    if (!copyData(inFile, outFile) || inFile.getZipError() != UNZ_OK)
    {
        outFile.close();
        return QString("解压文件 \"%1\" 失败(错误代码 %1)。").arg(fileName).arg(inFile.getZipError());
    }
    outFile.close();

    // Chiudo i file
    inFile.close();
    if (inFile.getZipError() != UNZ_OK)
        return QString("解压文件 \"%1\" 失败(错误代码 %1)。").arg(fileName).arg(zip->getCurrentFileName()).arg(inFile.getZipError());

    if (srcPerm != 0)
        outFile.setPermissions(srcPerm);
    return "";
}

QString MyCompress::extractDir(QString fileCompressed, QString dirName, QString dirDest)
{
    QuaZip zip(fileCompressed);
    QDir directory(dirDest);
    if (!dirName.isEmpty() && !dirName.endsWith('/')) dirName.append('/');

    if (!zip.open(QuaZip::mdUnzip))
        return QString("压缩文件 \"%1\" 无法打开(错误代码 %2)。").arg(QFileInfo(fileCompressed).fileName()).arg(zip.getZipError());

    for (bool f = zip.goToFirstFile(); f; f = zip.goToNextFile())
    {
        QString name = zip.getCurrentFileName();

        if (name.startsWith(dirName) && name != dirName)
        {
            QString error = extractFile(&zip, "", directory.absoluteFilePath(name.mid(dirName.length())));
            if (!error.isEmpty()) return error;
        }
    }

    // Chiudo il file zip
    zip.close();
    if (zip.getZipError() != 0)
        return QString("压缩文件 \"%1\" 解压失败(错误代码 %2)。").arg(QFileInfo(fileCompressed).fileName()).arg(zip.getZipError());

    return "";
}
