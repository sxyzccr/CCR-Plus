CONFIG -= qt
TARGET = monitor
TEMPLATE = app

QMAKE_LFLAGS += -static

SOURCES += monitor.c

win32: LIBS += -lpthread -lpsapi
