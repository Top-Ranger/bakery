!include(../tests.pri) {
    error( "Could not include ../tests.pri!" )
}

QT       += testlib

TARGET = tst_testbakery
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_testbakery.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

RESOURCES += \
    testfiles.qrc
