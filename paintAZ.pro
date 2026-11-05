#-------------------------------------------------
#
# Project created by QtCreator 2017-03-15T11:02:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
VERSION = 1.1.0

TARGET = paintAZ
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    mainwindow.cpp \
    newfiledialog.cpp \
    memento.cpp \
    fillspace.cpp

HEADERS  += widget.h \
    mainwindow.h \
    newfiledialog.h \
    memento.h \
    fillspace.h

FORMS    += widget.ui \
    mainwindow.ui \
    newfiledialog.ui

RESOURCES += \
    resources.qrc
