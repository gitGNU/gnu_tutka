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
    sliderwithlabel.cpp \
    sectionlisttablemodel.cpp \
    playingsequencelisttablemodel.cpp \
    messagelisttablemodel.cpp \
    track.cpp \
    clickablelabel.cpp \
    inputmidiinterfacestablemodel.cpp \
    outputmidiinterfacestablemodel.cpp \
    mmd.cpp \
    conversion.cpp \
    smf.cpp \
    buffermidi.cpp \
    buffermidiinterface.cpp \
    scheduler.cpp \
    schedulerrtc.cpp \
    schedulernanosleep.cpp \
    helpdialog.cpp \
    tutkadialog.cpp

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
    sliderwithlabel.h \
    sectionlisttablemodel.h \
    playingsequencelisttablemodel.h \
    messagelisttablemodel.h \
    track.h \
    clickablelabel.h \
    inputmidiinterfacestablemodel.h \
    outputmidiinterfacestablemodel.h \
    mmd.h \
    conversion.h \
    smf.h \
    buffermidi.h \
    buffermidiinterface.h \
    scheduler.h \
    schedulerrtc.h \
    schedulernanosleep.h \
    helpdialog.h \
    tutkadialog.h

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
    messagelistdialog.ui \
    helpdialog.ui

TEMPLATE = app
TARGET = tutka
QT += xml widgets gui
DEFINES += QT_NO_DEBUG_OUTPUT
TRANSLATIONS += tutka_fi.ts tutka_cs.ts tutka_fr.ts
ICON = tutka.icns
QMAKE_INFO_PLIST = tutka.plist

QMAKE_EXTRA_COMPILERS += lrelease
lrelease.input         = TRANSLATIONS
lrelease.output        = ${QMAKE_FILE_BASE}.qm
lrelease.commands      = $$[QT_INSTALL_BINS]/lrelease ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_BASE}.qm
lrelease.CONFIG       += no_link target_predeps

unix:!macx:SOURCES += alsamidi.cpp alsamidiinterface.cpp
unix:!macx:HEADERS += alsamidi.h alsamidiinterface.h
unix:!macx:LIBS += -lasound

macx:SOURCES += coremidi.cpp coremidiinterface.cpp
macx:HEADERS += coremidi.h coremidiinterface.h
macx:LIBS += -framework CoreAudio -framework CoreMidi -framework CoreFoundation
macx:INCLUDEPATH += /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/System/Library/Frameworks/CoreMIDI.framework/Versions/A/Headers
macx:TARGET = Tutka

target.path = /usr/bin

desktop.path = /usr/share/applications
desktop.files = tutka.desktop

icon48.path = /usr/share/icons/hicolor/48x48/apps
icon48.files = icons/48x48/tutka.png

icon512.path = /usr/share/icons/hicolor/512x512/apps
icon512.files = icons/512x512/tutka.png

translations.path = /usr/share/tutka/translations
translations.files = *.qm

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

INSTALLS += target desktop icon48 icon512 translations




































