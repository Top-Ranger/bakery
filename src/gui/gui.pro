!include(../../bakery_dist.pri) {
    error( "Could not include ../../bakery.pri!" )
}

!include(../../bakery_link.pri) {
    error( "Could not include ../bakery_link.pri!" )
}

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG -= app_bundle
TARGET = bakery-gui
TEMPLATE = app

SOURCES += main.cpp\
    mainwindow.cpp \
    pluginoutputwidget.cpp \
    plugininputdialog.cpp \
    shapedialog.cpp \
    shapewidget.cpp \
    savedialog.cpp

HEADERS += mainwindow.h \
    pluginoutputwidget.h \
    plugininputdialog.h \
    helpers.hpp \
    shapedialog.h \
    shapewidget.h \
    savedialog.h

FORMS += mainwindow.ui \
    pluginoutputwidget.ui \
    plugininputdialog.ui \
    shapedialog.ui \
    shapewidget.ui \
    savedialog.ui

