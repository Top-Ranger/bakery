!include(../tests.pri) {
    error( "Could not include ../tests.pri!" )
}

QT += testlib

TARGET = tst_testshape
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += tst_testshape.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
