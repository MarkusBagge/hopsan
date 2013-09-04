# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )

TARGET = SymHop
TEMPLATE = lib
CONFIG += shared
DESTDIR = $${PWD}/../bin

QT += xml core gui

TARGET = $${TARGET}$${DEBUG_EXT}


#--------------------------------------------------
# Add the include path to our self, (SymHop)
INCLUDEPATH *= $${PWD}/include/
#--------------------------------------------------

# -------------------------------------------------
# Project files
# -------------------------------------------------

SOURCES += src/SymHop.cc

HEADERS += include/SymHop.h









