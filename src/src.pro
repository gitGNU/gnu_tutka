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
           mainwindow.cpp \
    midi.cpp \
    midiinterface.cpp \
    preferencesdialog.cpp \
    trackvolumesdialog.cpp \
    songpropertiesdialog.cpp \
    instrumentpropertiesdialog.cpp \
    transposedialog.cpp \
    expandshrinkdialog.cpp \
    changeinstrumentdialog.cpp \
    sectionlistdialog.cpp \
    playingsequencedialog.cpp \
    playingsequencelistdialog.cpp \
    blocklistdialog.cpp \
    messagelistdialog.cpp \
    instrumentspinbox.cpp \
    blocklisttablemodel.cpp \
    spinboxdelegate.cpp \
    trackvolumewidget.cpp \
    playingsequencetablemodel.cpp \
    sliderwithlabel.cpp

HEADERS += block.h \
           instrument.h \
           message.h \
           playseq.h \
           song.h \
           tracker.h \
           player.h \
           mainwindow.h \
    midi.h \
    midiinterface.h \
    preferencesdialog.h \
    trackvolumesdialog.h \
    songpropertiesdialog.h \
    instrumentpropertiesdialog.h \
    transposedialog.h \
    expandshrinkdialog.h \
    changeinstrumentdialog.h \
    sectionlistdialog.h \
    playingsequencedialog.h \
    playingsequencelistdialog.h \
    blocklistdialog.h \
    messagelistdialog.h \
    instrumentspinbox.h \
    blocklisttablemodel.h \
    spinboxdelegate.h \
    trackvolumewidget.h \
    playingsequencetablemodel.h \
    sliderwithlabel.h

FORMS += \
    mainwindow.ui \
    preferencesdialog.ui \
    trackvolumesdialog.ui \
    songpropertiesdialog.ui \
    instrumentpropertiesdialog.ui \
    transposedialog.ui \
    expandshrinkdialog.ui \
    changeinstrumentdialog.ui \
    sectionlistdialog.ui \
    playingsequencedialog.ui \
    playingsequencelistdialog.ui \
    blocklistdialog.ui \
    messagelistdialog.ui

TEMPLATE = app
TARGET = Tutka
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
































































