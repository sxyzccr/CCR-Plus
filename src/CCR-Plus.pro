QT += core gui xml widgets

TARGET = CCR-Plus
TEMPLATE = app

CONFIG += c++11

SOURCES += common/const.cpp \
    common/global.cpp \
    common/player.cpp \
    common/problem.cpp \
    common/contest.cpp \
    judge/judgethread.cpp \
    judge/judger/basejudger.cpp \
    judge/judger/answeronlyjudger.cpp \
    judge/judger/traditionaljudger.cpp \
    configure/configuredialog.cpp \
    configure/configuretable.cpp \
    configure/advancedconfiguredialog.cpp \
    configure/general/generalconfigurewidget.cpp \
    configure/compiler/addcompilerdialog.cpp \
    configure/compiler/compilerconfigurewidget.cpp \
    configure/testcase/testcasetable.cpp \
    configure/testcase/addtestcasedialog.cpp \
    configure/testcase/testcaseconfigurewidget.cpp \
    mainwindow/main.cpp\
    mainwindow/mainwindow.cpp \
    mainwindow/boardtable.cpp \
    mainwindow/detailtable.cpp \
    mainwindow/createfiledialog.cpp

HEADERS += common/const.h \
    common/global.h \
    common/version.h \
    common/player.h \
    common/problem.h \
    common/contest.h \
    judge/judgethread.h \
    judge/judger/basejudger.h \
    judge/judger/answeronlyjudger.h \
    judge/judger/traditionaljudger.h \
    configure/configuredialog.h \
    configure/configuretable.h \
    configure/advancedconfiguredialog.h \
    configure/general/generalconfigurewidget.h \
    configure/compiler/addcompilerdialog.h \
    configure/compiler/compilerconfigurewidget.h \
    configure/testcase/testcasetable.h \
    configure/testcase/addtestcasedialog.h \
    configure/testcase/testcaseconfigurewidget.h \
    mainwindow/mainwindow.h \
    mainwindow/boardtable.h \
    mainwindow/detailtable.h \
    mainwindow/createfiledialog.h

FORMS += configure/configuredialog.ui \
    configure/advancedconfiguredialog.ui \
    configure/general/generalconfigurewidget.ui \
    configure/compiler/addcompilerdialog.ui \
    configure/compiler/compilerconfigurewidget.ui \
    configure/testcase/addtestcasedialog.ui \
    configure/testcase/testcaseconfigurewidget.ui \
    mainwindow/mainwindow.ui \
    mainwindow/createfiledialog.ui

RESOURCES = ../resources/image.qrc \
    ../resources/trans.qrc

RC_FILE = ../resources/icon.rc

win32: LIBS += -lpsapi
