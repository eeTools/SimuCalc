
VERSION = "0.2.0"
RELEASE = ""

TEMPLATE = app
TARGET = simucalc

QT += widgets
QT += script

SOURCES      = $$files( $$PWD/src/*.cpp, true )
HEADERS      = $$files( $$PWD/src/*.h, true )
#TRANSLATIONS = $$files( $$PWD/resources/translations/*.ts )
FORMS       += $$files( $$PWD/src/*.ui, true )
RESOURCES    = ../src/application.qrc

INCLUDEPATH += ../src \
    ../src \
    ../src/dataplotwidget

QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-implicit-fallthrough
QMAKE_CXXFLAGS -= -fPIC
QMAKE_CXXFLAGS += -fno-pic
QMAKE_CXXFLAGS += -Ofast
QMAKE_CXXFLAGS_DEBUG -= -O
QMAKE_CXXFLAGS_DEBUG -= -O1
QMAKE_CXXFLAGS_DEBUG -= -O2
QMAKE_CXXFLAGS_DEBUG -= -O3
QMAKE_CXXFLAGS_DEBUG += -O0

win32 {
    OS = Windows
    QMAKE_LIBS += -lwsock32
    RC_ICONS += ../src/icons/simucalc-128.ico
}
linux {
    OS = Linux
    QMAKE_LFLAGS += -no-pie
}
macx {
    OS = MacOs
    QMAKE_LFLAGS += -no-pie
    ICON = ../src/icons/simucalc-128.icns
}

CONFIG += qt 
CONFIG += warn_on
CONFIG += no_qml_debug
CONFIG *= c++11

DEFINES += APP_VERSION=\\\"$$VERSION$$RELEASE\\\"

BUILD_DATE = $$system(date +\"\\\"%d-%m-%y\\\"\")
DEFINES += BUILDDATE=\\\"$$BUILD_DATE\\\"


TARGET_NAME   = simucalc_$$VERSION$$RELEASE
TARGET_PREFIX = $$BUILD_DIR/executables/$$TARGET_NAME

OBJECTS_DIR *= $$OUT_PWD/build/objects
MOC_DIR     *= $$OUT_PWD/build/moc
INCLUDEPATH += $$MOC_DIR

DESTDIR = $$TARGET_PREFIX

macx {
QMAKE_CC = /usr/local/Cellar/gcc@7/7.5.0_4/bin/gcc-7
QMAKE_CXX = /usr/local/Cellar/gcc@7/7.5.0_4/bin/g++-7
QMAKE_LINK = /usr/local/Cellar/gcc@7/7.5.0_4/bin/g++-7

    QMAKE_CXXFLAGS -= -stdlib=libc++
    QMAKE_LFLAGS   -= -stdlib=libc++

    mkpath( $$TARGET_PREFIX/oscope.app )
}

message( "-----------------------------------")
message( "    "                               )
message( "    "$$TARGET_NAME for $$OS         )
message( "    "                               )
message( "          Qt version: "$$QT_VERSION )
message( "    "                               )
message( "    Destination Folder:"            )
message( $$TARGET_PREFIX                      )
message( "-----------------------------------")
