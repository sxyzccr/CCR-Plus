CONFIG -= qt
TARGET = monitor
TEMPLATE = app

QMAKE_CXXFLAGS += -static

SOURCES += monitor.c

win32: LIBS += -lpthread -lpsapi
