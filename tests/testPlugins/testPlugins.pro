!include(../tests.pri) {
    error( "Could not include ../tests.pri!" )
}

QT       += testlib

TARGET = tst_testplugins
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_testplugins.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

RESOURCES += \
    testfiles.qrc
