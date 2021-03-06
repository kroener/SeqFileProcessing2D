QT       += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
SOURCES += trackPositions.cpp \
           ../../src/MosquitoesInSeq.cpp \
           ../../src/MosquitoPosition.cpp \
           ../../src/MosquitoesInFrame.cpp \ 
           ../../src/MosquitoRegistry.cpp \
           ../../src/TrackPoint.cpp \
           ../../src/iSeq.cpp \
           ../../src/Metadata.cpp \
           ../../src/MyVideoReader.cpp \
           ../../src/Header.cpp \
           ../../src/Image.cpp
HEADERS  = ../../src/MosquitoesInSeq.hpp \
           ../../src/MosquitoPosition.hpp \
           ../../src/MosquitoesInFrame.hpp \ 
           ../../src/MosquitoRegistry.hpp \
           ../../src/TrackPoint.hpp \
           ../../src/iSeq.hpp \
           ../../src/Metadata.hpp \
           ../../src/MyVideoReader.hpp \
           ../../src/Header.hpp \
           ../../src/Image.hpp

win32{
#TARGET = ../../trackPositions
}
unix{
target.path = /usr/local/bin
INSTALLS += target
}

INCLUDEPATH += ../../src

CONFIG += c++11
CONFIG += console
CONFIG += link_pkgconfig
PKGCONFIG += opencv
PKGCONFIG += libavdevice
PKGCONFIG += libavformat
PKGCONFIG += libavfilter
PKGCONFIG += libavcodec
PKGCONFIG += libswresample
PKGCONFIG += libswscale
PKGCONFIG += libavutil
PKGCONFIG += icu-uc
PKGCONFIG += icu-io

QMAKE_CXXFLAGS += -fopenmp \
                  -D_LARGEFILE64_SOURCE \
                  -D_FILE_OFFSET_BITS=64 \
                  -D__STDC_FORMAT_MACROS

LIBS += -fopenmp
