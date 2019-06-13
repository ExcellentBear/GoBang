#-------------------------------------------------
#
# Project created by QtCreator 2019-06-10T16:51:44
#
#-------------------------------------------------

QT       += core gui
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GoBang
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    algthread.cpp

HEADERS  += mainwindow.h \
    algthread.h \
    MetaTypes.h

FORMS    += mainwindow.ui

