!include(../../../bakery_plugins.pri) {
    error( "Could not include ../../../bakery_plugins.pri!" )
}

TARGET = plg_edgematcherplugin
TEMPLATE = app

SOURCES += \
    plg_edgematcherplugin.cpp
HEADERS += \
    plg_edgematcherplugin.h

