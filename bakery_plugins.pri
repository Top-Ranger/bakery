!include(./bakery_link.pri) {
    error( "Could not include ./bakery_link.pri!" )
}

CONFIG -= app_bundle
DESTDIR = $$PWD/dist/plugins
