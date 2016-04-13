!include(../../bakery_dist.pri) {
    error( "Could not include ../../bakery_dist.pri!" )
}

TARGET = bakery
TEMPLATE = lib

DEFINES += BAKERY_LIBRARY

VERSION = 1.0.0

SOURCES += bakery.cpp \
    shape.cpp \
    sheet.cpp \
    plugins.cpp

HEADERS += bakery.h \
    shape.h \
    sheet.h \
    plugins.h \
    global.h \
    helpers.hpp
