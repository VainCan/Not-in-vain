TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    ../allCRC.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    ../allCRC.h

