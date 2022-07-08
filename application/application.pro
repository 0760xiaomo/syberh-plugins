#-------------------------------------------------
#
# Project created by QtCreator 2020-04-20T19:41:36
#
#-------------------------------------------------

QT       += gui widgets quick qml dbus

TARGET = application
TEMPLATE = lib

DEFINES += APPLICATION_LIBRARY

SOURCES += application.cpp

HEADERS += application.h\
        application_p.h \
        application_global.h


CONFIG += link_pkgconfig
CONFIG += C++11
CONFIG += plugin

PKGCONFIG += syberos-qt

INCLUDEPATH += $$[QT_INSTALL_HEADERS]/../syberos_application
LIBS += -L$$[QT_INSTALL_LIBS] -lsyberos-application

INCLUDEPATH += $$absolute_path("nativesdk/src", $$SYBERH_APP)
INCLUDEPATH += $$absolute_path("pluginmanager/src", $$SYBERH_APP)

LIB_OUT_DIR = $$absolute_path("lib", $$SYBERH_APP)
LIBS += -L$$LIB_OUT_DIR -lnativesdk -lpluginmanager


DESTDIR = $$absolute_path("plugins", $$SYBERH_APP)

CONFIG(release, debug|release){
    QMAKE_POST_LINK=$(STRIP) $(DESTDIR)$(TARGET)
}