!include(./bakery_config.pri) {
    error( "Could not include ./bakery_config.pri!" )
}

unix: LIBS += -L$$PWD/dist -lbakery
win32: LIBS += -L$$PWD/dist -lbakery1
win64: LIBS += -L$$PWD/dist -lbakery1

INCLUDEPATH += $$PWD/src/lib
DEPENDPATH += $$PWD/src/lib
