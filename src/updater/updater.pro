QT += core gui xml widgets network

TARGET = upgrader
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp \
    checkupdatesdialog.cpp

HEADERS += checkupdatesdialog.h

FORMS += checkupdatesdialog.ui

RESOURCES = ../../resources/trans.qrc

#RC_FILE = ../resources/icon.rc
