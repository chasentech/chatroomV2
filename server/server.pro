TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    src/main.cpp \
    src/wrap.cpp

HEADERS += \
    include/wrap.h
