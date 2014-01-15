#-------------------------------------------------
#
# Project created by QtCreator 2013-12-18T10:57:30
#
#-------------------------------------------------

<<<<<<< HEAD
QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = VMI_alpha
TEMPLATE = app
INCLUDEPATH += ../qpcap

#LIBS +=  -Wl,-rpath,../qpcap -L../qpcap -lqpcap -lpcap -lxenstat
LIBS += -lxenstat -Wl,-rpath,../qpcap
SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp

SUBDIRS = qpcap \
    VMI
HEADERS  += mainwindow.h \
    qcustomplot.h

FORMS    += mainwindow.ui

unix:!macx: LIBS += -L$$OUT_PWD/../qpcap/ -lqpcap

INCLUDEPATH += $$PWD/../qpcap
DEPENDPATH += $$PWD/../qpcap

unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../qpcap/libqpcap.a
=======
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VMI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
>>>>>>> e05888cce7d2127ac34c0891fccb294cca5aeee2
