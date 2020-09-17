QT += core widgets network
QT -= gui

TARGET = upgrader
TEMPLATE = app

CONFIG += c++11

include(quazip/quazip.pri)

SOURCES += updaterconst.cpp \
    main.cpp \
    checkupdatesdialog.cpp \
    downloaddialog.cpp \
    mycompress.cpp

HEADERS += updaterconst.h \
    checkupdatesdialog.h \
    downloaddialog.h \
    mycompress.h

FORMS += checkupdatesdialog.ui \
    downloaddialog.ui

RESOURCES = ../../resources/image.qrc \
    ../../resources/trans.qrc

RC_FILE = ../../resources/updater_icon.rc

win32: LIBS += -lole32
