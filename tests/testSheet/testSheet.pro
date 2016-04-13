!include(../tests.pri) {
    error( "Could not include ../tests.pri!" )
}

QT += testlib

TARGET = tst_testsheet
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += tst_testsheet.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
