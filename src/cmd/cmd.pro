!include(../../bakery_dist.pri) {
    error( "Could not include ../../bakery.pri!" )
}

!include(../../bakery_link.pri) {
    error( "Could not include ../bakery_link.pri!" )
}

QT += core

TARGET = bakery-cmd
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp
