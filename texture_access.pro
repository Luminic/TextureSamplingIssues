######################################################################
# Automatically generated by qmake (3.1) Fri Aug 14 14:29:16 2020
######################################################################

TEMPLATE = app
TARGET = texture_access
INCLUDEPATH += .

QT += core gui widgets

CONFIG += debug

OBJECTS_DIR = generated_files
MOC_DIR = generated_files


# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += OpenGLWindow.hpp Shader.hpp
SOURCES += main.cpp OpenGLWindow.cpp Shader.cpp
