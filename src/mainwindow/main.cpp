#include <QTranslator>
#include <QApplication>

#include "mainwindow/mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    translator.load(":/trans/qt_zh_CN.qm");
    a.installTranslator(&translator);

    qRegisterMetaType<Global::LabelStyle>("Global::LabelStyle");

    MainWindow w;
    w.show();

    return a.exec();
}
