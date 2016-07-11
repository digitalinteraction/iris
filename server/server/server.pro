#-------------------------------------------------
#
# Project created by QtCreator 2016-07-07T08:47:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = server
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../../network/DebugTransfer.cpp \
    ../../network/NetworkControl.cpp \
    ../../network/Packetbuffer.cpp \
    ../../network/ReliableTransfer.cpp \
    ../../network/SerialCon.cpp \
    ../../network/Topology.cpp \
    ../../network/UnreliableTransfer.cpp \
    ../../network/crc/crc.c \
    handleinput.cpp \
    list_map.cpp

HEADERS  += mainwindow.h \
    ../../network/crc/crc.h \
    ../../network/DebugTransfer.h \
    ../../network/NetworkControl.h \
    ../../network/Packetbuffer.h \
    ../../network/ReliableTransfer.h \
    ../../network/SerialCon.h \
    ../../network/Topology.h \
    ../../network/UnreliableTransfer.h \
    handleinput.h \
    list_map.h

FORMS    += mainwindow.ui

#CONFIG += console
