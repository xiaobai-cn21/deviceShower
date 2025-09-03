#-------------------------------------------------
#
# Project created by QtCreator 2025-09-03T13:29:45
#
#-------------------------------------------------

QT       += core gui network widgets charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
    deviceshower.cpp \
    gauge.cpp \
    leddisplay.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    deviceshower.h \
    gauge.h \
    leddisplay.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment
qnx {
    target.path = /tmp/$${TARGET}/bin
}
unix:!android {
    target.path = /opt/$${TARGET}/bin
}
!isEmpty(target.path): INSTALLS += target
