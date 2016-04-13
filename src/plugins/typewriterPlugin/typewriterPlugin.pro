!include(../../../bakery_plugins.pri) {
    error( "Could not include ../../../bakery_plugins.pri!" )
}

TARGET = plg_typewriterplugin
TEMPLATE = app
CONFIG += console

SOURCES += \
    plg_typewriterplugin.cpp
HEADERS += \
    plg_typewriterplugin.h
