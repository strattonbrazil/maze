#-------------------------------------------------
#
# Project created by QtCreator 2016-08-22T21:39:12
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Maze
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mazeview.cpp \
    maze.cpp

HEADERS  += mainwindow.h \
    mazeview.h \
    maze.h

FORMS    += mainwindow.ui
