QT       += core gui opengl concurrent xml charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
SOURCES += BatchSeqFileProcessing2D.cpp \
           ../../src/MosquitoesInSeq.cpp \
           ../../src/MosquitoPosition.cpp \
           ../../src/MosquitoesInFrame.cpp \ 
           ../../src/iSeq.cpp \
           ../../src/Metadata.cpp \
           ../../src/MyVideoReader.cpp \
           ../../src/Header.cpp \
           ../../src/Image.cpp
HEADERS  = ../../src/iSeq.hpp \
           ../../src/MosquitoesInSeq.hpp \
           ../../src/MosquitoPosition.hpp \
           ../../src/MosquitoesInFrame.hpp \ 
           ../../src/Metadata.hpp \
           ../../src/MyVideoReader.hpp \
           ../../src/Header.hpp \
           ../../src/Image.hpp

win32{
#TARGET = ../../BatchSeqFileProcessing2D
}
unix{
target.path = /usr/local/bin
INSTALLS += target
}

INCLUDEPATH += ../../src

#CONFIG += debug
CONFIG += c++11
CONFIG += link_pkgconfig
CONFIG += console
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

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

QMAKE_LFLAGS_RELEASE -= -O1

