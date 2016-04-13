!include(../../../bakery_plugins.pri) {
    error( "Could not include ../../../bakery_plugins.pri!" )
}

TARGET = plg_shapeshakerplugin
TEMPLATE = app
CONFIG += console

SOURCES += \
    plg_shapeshakerplugin.cpp
HEADERS += \
    plg_shapeshakerplugin.h
