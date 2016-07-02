CONFIG -= qt
TARGET = fulltext
TEMPLATE = app

QMAKE_CXXFLAGS += -static

win32 {
    SOURCES += fulltext_gbk.cpp
}
unix {
    SOURCES += fulltext_utf8.cpp
}
