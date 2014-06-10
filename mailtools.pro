#-------------------------------------------------
#
# Project created by QtCreator 2014-06-07T20:19:05
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += link_pkgconfig

TARGET = mailtools
TEMPLATE = app


SOURCES += main.cpp\
        mailview.cpp \
    htmlmail.cpp \
    mailnetworkmanager.cpp

HEADERS  += mailview.h \
    htmlmail.h \
    mailnetworkmanager.h

PKGCONFIG += vmime
QMAKE_CXXFLAGS += -std=gnu++11
