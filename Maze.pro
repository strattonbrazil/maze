#-------------------------------------------------
#
# Project created by QtCreator 2016-08-22T21:39:12
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Maze
TEMPLATE = app
INCLUDEPATH += /usr/local/include/bullet/
LIBS += -L/usr/local/lib/ -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lBox2D

SOURCES += main.cpp\
        mainwindow.cpp \
    mazeview.cpp \
    maze.cpp \
    player.cpp \
    shader.cpp

HEADERS  += mainwindow.h \
    mazeview.h \
    maze.h \
    player.h \
    shader.h

FORMS    += mainwindow.ui
