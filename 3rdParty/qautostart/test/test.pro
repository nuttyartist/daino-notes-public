QT -= gui
QT += testlib

CONFIG += c++11 console
CONFIG -= app_bundle

include(./../src/qautostart.pri)

SOURCES += main.cpp \
    testsuit.cpp

HEADERS += \
    testsuit.h
