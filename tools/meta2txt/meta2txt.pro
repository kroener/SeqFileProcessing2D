TEMPLATE = app
SOURCES += meta2txt.cpp \
../../src/Metadata.cpp
HEADERS  = ../../src/Metadata.hpp

win32{
#TARGET = ../../meta2txt
}
unix{
target.path = /usr/local/bin
INSTALLS += target
}
INCLUDEPATH += ../../src

CONFIG += c++11
CONFIG += console

QMAKE_CXXFLAGS += -fopenmp \
                  -D_LARGEFILE64_SOURCE \
                  -D_FILE_OFFSET_BITS=64 \
                  -D__STDC_FORMAT_MACROS

LIBS += -fopenmp

