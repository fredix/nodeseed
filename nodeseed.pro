#-------------------------------------------------
#
# Project created by QtCreator 2012-05-16T16:58:38
#
#-------------------------------------------------

QT       += core gui ftp

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = nodeseed
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    nodeftp.cpp

HEADERS  += mainwindow.h \
    nodeftp.h

FORMS    += mainwindow.ui

include(./externals/qtftp/modules/qt_ftp.pri)


INCLUDEPATH += ./externals/qtftp/include/QtFtp

LIBS += ./externals/qtftp/lib/libQt5Ftp.a

win32 {
win32: LIBS += "C:\Users\fredix\Documents\GitHub\pumit_uploader\externals\qtftp\lib\Qt5Ftp.lib"
}
