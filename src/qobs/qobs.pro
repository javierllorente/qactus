QT += core network
TARGET = qobs
TEMPLATE = lib
DEFINES += QOBS_LIBRARY
include(../defines.pri)

SOURCES += obspackage.cpp \
    obsaccess.cpp \
    obsxmlreader.cpp \
    obsrequest.cpp \
    obs.cpp \
    obsfile.cpp \
    obsresult.cpp \
    obsrevision.cpp
HEADERS += obspackage.h \
    obsaccess.h \
    obsxmlreader.h \
    obsrequest.h \
    obs.h \
    obsfile.h \
    obsresult.h \
    obsrevision.h
