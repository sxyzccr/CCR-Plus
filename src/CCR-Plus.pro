QT += core gui xml widgets

TARGET = CCR-Plus
TEMPLATE = app

CONFIG += c++11

SOURCES += boardtable.cpp \
    configdialog.cpp \
    createfiledialog.cpp \
    detailtable.cpp \
    itemdelegate.cpp \
    judgethread.cpp \
    main.cpp\
    mainwindow.cpp \
    player.cpp \
    problem.cpp \
    contestinfo.cpp \
    const.cpp \
    status.cpp

HEADERS += boardtable.h \
    configdialog.h \
    createfiledialog.h \
    detailtable.h \
    itemdelegate.h \
    judgethread.h \
    mainwindow.h \
    player.h \
    problem.h \
    version.h \
    const.h \
    contestinfo.h \
    status.h

UI_DIR = forms
RCC_DIR = rcfile

FORMS += forms/mainwindow.ui \
    forms/configdialog.ui \
    forms/createfiledialog.ui

RESOURCES = rcfile/image.qrc
RC_FILE = rcfile/icon.rc

win32: LIBS += -lpsapi
