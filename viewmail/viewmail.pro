PREFIX ?= $$(HOME)/.local

QT       += core gui network webkit
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webkitwidgets

CONFIG += link_pkgconfig

TARGET = viewmail
TEMPLATE = app

SOURCES += main.cpp \
		   mailview.cpp \
		   htmlmail.cpp \
		   mailnetworkmanager.cpp \
		   statichttpreply.cpp

HEADERS += mailview.h \
		   htmlmail.h \
		   mailnetworkmanager.h \
		   statichttpreply.h

PKGCONFIG += vmime
QMAKE_CXXFLAGS += -std=gnu++11

target.path = $$PREFIX/bin
INSTALLS += target
