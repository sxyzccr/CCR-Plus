#ifndef MYCOMPRESS_H
#define MYCOMPRESS_H

#include <QString>
#include <quazip/quazip.h>

class MyCompress
{
public:
    static QString extractDir(QString fileCompressed, QString dirName = QString(), QString dirDest = QString());

private:
    static QString extractFile(QuaZip* zip, QString fileName, QString fileDest);
};

#endif // MYCOMPRESS_H
