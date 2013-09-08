#-------------------------------------------------
#
# Project created by QtCreator 2013-09-08T11:19:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = qt-gui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qt-custom-plot/qcustomplot.cpp \
    animationthread.cpp

HEADERS  += mainwindow.h \
    qt-custom-plot/qcustomplot.h \
    animationthread.h
