#include "mainwindow.h"

#include <QTranslator>
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    translator.load(":/trans/rcfile/qt_zh_CN.qm");
    a.installTranslator(&translator);

    MainWindow w;
    w.show();

    return a.exec();
}
