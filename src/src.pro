MOC_DIR = .moc
OBJECTS_DIR = .obj

SOURCES += main.cpp \
           block.cpp \
           instrument.cpp \
           message.cpp \
           playseq.cpp \
           song.cpp \
           tracker.cpp \
           player.cpp \
           gui.cpp \
    midi.cpp

HEADERS += block.h \
           instrument.h \
           message.h \
           playseq.h \
           song.h \
           tracker.h \
           player.h \
           gui.h \
    midi.h

FORMS += tutka.ui

TEMPLATE = app
TARGET = tutka
QT += xml

target.path = /usr/bin

schemas.path = /usr/share/gconf/schemas
schemas.files = tutka.schemas

desktop.path = /usr/share/applications
desktop.files = tutka.desktop

icon.path = /usr/share/icons/blanco/80x80/apps
icon.files = tutka.png

QMAKE_CFLAGS += -std=gnu99
QMAKE_CXXFLAGS += \
    -g \
    -Werror
QMAKE_CXXFLAGS_WARN_ON += \
    -Wno-strict-overflow \
    -Wno-sign-compare
QMAKE_CLEAN += *.gcov \
    *.gcno \
    ./.obj/*.gcno

INSTALLS += target schemas desktop icon




