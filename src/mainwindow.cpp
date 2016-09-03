/*
 * mainwindow.cpp
 *
 * Copyright 2002-2016 Vesa Halttunen
 *
 * This file is part of Tutka.
 *
 * Tutka is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tutka is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tutka; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef __linux
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#endif
#include <QApplication>
#include <QKeyEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QScrollBar>
#include <QDesktopWidget>
#include "instrumentpropertiesdialog.h"
#include "preferencesdialog.h"
#include "trackvolumesdialog.h"
#include "transposedialog.h"
#include "expandshrinkdialog.h"
#include "changeinstrumentdialog.h"
#include "sectionlistdialog.h"
#include "songpropertiesdialog.h"
#include "playingsequencedialog.h"
#include "playingsequencelistdialog.h"
#include "blocklistdialog.h"
#include "messagelistdialog.h"
#include "helpdialog.h"
#include "song.h"
#include "track.h"
#include "block.h"
#include "conversion.h"
#include "mmd.h"
#include "smf.h"
#include "midi.h"
#include "midiinterface.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"

MainWindow::MainWindow(Player *player, QWidget *parent) :
    QMainWindow(parent),
    player(player),
    ui(new Ui::MainWindow),
    settings("nongnu.org", "Tutka"),
    instrumentPropertiesDialog(new InstrumentPropertiesDialog(player->midi())),
    openDialog(new QFileDialog(NULL, tr("Open file"), settings.value("Paths/songPath").toString(), tr("Tutka songs (*.tutka);;OctaMED SoundStudio songs (*.med)"))),
    preferencesDialog(new PreferencesDialog(player)),
    trackVolumesDialog(new TrackVolumesDialog),
    transposeDialog(new TransposeDialog),
    expandShrinkDialog(new ExpandShrinkDialog),
    changeInstrumentDialog(new ChangeInstrumentDialog),
    sectionListDialog(new SectionListDialog),
    songPropertiesDialog(new SongPropertiesDialog),
    playingSequenceDialog(new PlayingSequenceDialog),
    playingSequenceListDialog(new PlayingSequenceListDialog),
    blockListDialog(new BlockListDialog),
    messageListDialog(new MessageListDialog(player->midi())),
    helpDialog(new HelpDialog),
    externalSyncActionGroup(new QActionGroup(this)),
    song(NULL),
    copySelection_(NULL),
    copyBlock_(NULL),
    copyTrack_(NULL),
    block(0),
    playseq(0),
    instrument(0),
    selectionStartTrack(-1),
    selectionStartLine(-1),
    selectionEndTrack(-1),
    selectionEndLine(-1)
{
    ui->setupUi(this);
    ui->comboBoxKeyboardOctaves->setCurrentIndex(ui->tracker->octave());
    qApp->installEventFilter(this);
    setFocus(Qt::ActiveWindowFocusReason);

    externalSyncActionGroup->addAction(ui->actionExternalSyncOff);
    externalSyncActionGroup->addAction(ui->actionExternalSyncMidi);
    externalSyncActionGroup->setExclusive(true);

    connect(player->midi(), SIGNAL(inputReceived(QByteArray)), this, SLOT(handleMidiInput(QByteArray)));
    connect(player, SIGNAL(songChanged(Song *)), this, SLOT(setSong(Song *)));
    connect(player, SIGNAL(songChanged(Song *)), ui->tracker, SLOT(setSong(Song *)));
    connect(player, SIGNAL(songChanged(Song *)), transposeDialog, SLOT(setSong(Song *)));
    connect(player, SIGNAL(songChanged(Song *)), expandShrinkDialog, SLOT(setSong(Song *)));
    connect(player, SIGNAL(songChanged(Song *)), changeInstrumentDialog, SLOT(setSong(Song *)));
    connect(player, SIGNAL(songChanged(Song *)), trackVolumesDialog, SLOT(setSong(Song *)));
    connect(player, SIGNAL(songChanged(Song *)), songPropertiesDialog, SLOT(setSong(Song *)));
    connect(player, SIGNAL(songChanged(Song *)), sectionListDialog, SLOT(setSong(Song *)));
    connect(player, SIGNAL(songChanged(Song *)), blockListDialog, SLOT(setSong(Song *)));
    connect(player, SIGNAL(songChanged(Song *)), playingSequenceDialog, SLOT(setSong(Song *)));
    connect(player, SIGNAL(songChanged(Song *)), playingSequenceListDialog, SLOT(setSong(Song *)));
    connect(player, SIGNAL(songChanged(Song *)), messageListDialog, SLOT(setSong(Song *)));
    connect(player, SIGNAL(sectionChanged(unsigned int)), this, SLOT(setSection(unsigned int)));
    connect(player, SIGNAL(sectionChanged(unsigned int)), sectionListDialog, SLOT(setSection(unsigned int)));
    connect(player, SIGNAL(playseqChanged(unsigned int)), this, SLOT(setPlayseq(unsigned int)));
    connect(player, SIGNAL(playseqChanged(unsigned int)), playingSequenceListDialog, SLOT(setPlayingSequence(unsigned int)));
    connect(player, SIGNAL(playseqChanged(unsigned int)), playingSequenceDialog, SLOT(setPlayseq(unsigned int)));
    connect(player, SIGNAL(positionChanged(unsigned int)), this, SLOT(setPosition(unsigned int)));
    connect(player, SIGNAL(positionChanged(unsigned int)), playingSequenceDialog, SLOT(setPosition(unsigned int)));
    connect(player, SIGNAL(blockChanged(unsigned int)), this, SLOT(setBlock(unsigned int)));
    connect(player, SIGNAL(blockChanged(unsigned int)), transposeDialog, SLOT(setBlock(unsigned int)));
    connect(player, SIGNAL(blockChanged(unsigned int)), expandShrinkDialog, SLOT(setBlock(unsigned int)));
    connect(player, SIGNAL(blockChanged(unsigned int)), changeInstrumentDialog, SLOT(setBlock(unsigned int)));
    connect(player, SIGNAL(blockChanged(unsigned int)), blockListDialog, SLOT(setBlock(unsigned int)));
    connect(player, SIGNAL(lineChanged(int)), ui->tracker, SLOT(setLine(int)));
    connect(player, SIGNAL(modeChanged(Player::Mode)), this, SLOT(setMode(Player::Mode)));
    connect(player, SIGNAL(timeChanged(unsigned int)), this, SLOT(setTime(unsigned int)));
    connect(ui->tracker, SIGNAL(cursorTrackChanged(int)), transposeDialog, SLOT(setTrack(int)));
    connect(ui->tracker, SIGNAL(cursorTrackChanged(int)), expandShrinkDialog, SLOT(setTrack(int)));
    connect(ui->tracker, SIGNAL(cursorTrackChanged(int)), changeInstrumentDialog, SLOT(setTrack(int)));
    connect(ui->tracker, SIGNAL(selectionChanged(int, int, int, int)), this, SLOT(setSelection(int, int, int, int)));
    connect(ui->tracker, SIGNAL(selectionChanged(int, int, int, int)), transposeDialog, SLOT(setSelection(int, int, int, int)));
    connect(ui->tracker, SIGNAL(selectionChanged(int, int, int, int)), expandShrinkDialog, SLOT(setSelection(int, int, int, int)));
    connect(ui->tracker, SIGNAL(selectionChanged(int, int, int, int)), changeInstrumentDialog, SLOT(setSelection(int, int, int, int)));
    connect(ui->tracker, SIGNAL(trackChanged(int, int, int)), this, SLOT(setTrackerHorizontalScrollBar(int, int, int)));
    connect(ui->tracker, SIGNAL(lineChanged(int, int, int)), this, SLOT(setTrackerVerticalScrollBar(int, int, int)));
    connect(ui->tracker, SIGNAL(commandPageChanged(int)), this, SLOT(setCommandPage(int)));
    connect(ui->tracker, SIGNAL(lineEdited()), this, SLOT(advancePlayerBySpaceLines()));
    connect(ui->tracker, SIGNAL(notePressed(unsigned char)), this, SLOT(playPressedNote(unsigned char)));
    connect(ui->tracker, SIGNAL(octaveChanged(int)), ui->comboBoxKeyboardOctaves, SLOT(setCurrentIndex(int)));
    connect(ui->tracker, SIGNAL(setLineRequested(int)), player, SLOT(setLine(int)));
    connect(ui->horizontalScrollBarTracker, SIGNAL(valueChanged(int)), ui->tracker, SLOT(setLeftmostTrack(int)));
    connect(ui->verticalScrollBarTracker, SIGNAL(valueChanged(int)), ui->tracker, SLOT(setLine(int)));
    connect(ui->buttonPlaySong, SIGNAL(clicked()), player, SLOT(playSong()));
    connect(ui->buttonPlayBlock, SIGNAL(clicked()), player, SLOT(playBlock()));
    connect(ui->buttonContinueSong, SIGNAL(clicked()), player, SLOT(continueSong()));
    connect(ui->buttonContinueBlock, SIGNAL(clicked()), player, SLOT(continueBlock()));
    connect(ui->buttonStop, SIGNAL(clicked()), player, SLOT(stop()));
    connect(ui->buttonInstrumentProperties, SIGNAL(clicked()), instrumentPropertiesDialog, SLOT(makeVisible()));
    connect(ui->spinBoxInstrument, SIGNAL(valueChanged(int)), instrumentPropertiesDialog, SLOT(setInstrument(int)));
    connect(ui->spinBoxInstrument, SIGNAL(valueChanged(int)), transposeDialog, SLOT(setInstrument(int)));
    connect(ui->spinBoxInstrument, SIGNAL(valueChanged(int)), this, SLOT(setInstrument(int)));
    connect(ui->comboBoxKeyboardOctaves, SIGNAL(currentIndexChanged(int)), ui->tracker, SLOT(setOctave(int)));
    connect(ui->checkBoxEdit, SIGNAL(toggled(bool)), ui->tracker, SLOT(setEditMode(bool)));
    connect(ui->checkBoxChord, SIGNAL(toggled(bool)), ui->tracker, SLOT(setChordMode(bool)));
    connect(ui->actionFileNew, SIGNAL(triggered()), this, SLOT(newSong()));
    connect(ui->actionFileOpen, SIGNAL(triggered()), openDialog, SLOT(show()));
    connect(openDialog, SIGNAL(fileSelected(QString)), player, SLOT(setSong(QString)));
    connect(openDialog, SIGNAL(fileSelected(QString)), this, SLOT(setSongPath(QString)));
    connect(ui->actionFileSave, SIGNAL(triggered()), this, SLOT(save()));
    connect(ui->actionFileSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));
    connect(ui->actionFileQuit, SIGNAL(triggered()), this, SLOT(quit()));
    connect(ui->actionEditCut, SIGNAL(triggered()), this, SLOT(cutSelection()));
    connect(ui->actionEditCopy, SIGNAL(triggered()), this, SLOT(copySelection()));
    connect(ui->actionEditPaste, SIGNAL(triggered()), this, SLOT(pasteSelection()));
    connect(ui->actionEditClear, SIGNAL(triggered()), this, SLOT(clearSelection()));
    connect(ui->actionEditTranspose, SIGNAL(triggered()), transposeDialog, SLOT(showSelection()));
    connect(ui->actionEditExpandShrink, SIGNAL(triggered()), expandShrinkDialog, SLOT(showSelection()));
    connect(ui->actionEditChangeInstrument, SIGNAL(triggered()), changeInstrumentDialog, SLOT(showSelection()));
    connect(ui->actionSongTrackVolumes, SIGNAL(triggered()), trackVolumesDialog, SLOT(makeVisible()));
    connect(ui->actionSongTranspose, SIGNAL(triggered()), transposeDialog, SLOT(showSong()));
    connect(ui->actionSongExpandShrink, SIGNAL(triggered()), expandShrinkDialog, SLOT(showSong()));
    connect(ui->actionSongChangeInstrument, SIGNAL(triggered()), changeInstrumentDialog, SLOT(showSong()));
    connect(ui->actionPlayingSequenceEditCurrent, SIGNAL(triggered()), playingSequenceDialog, SLOT(makeVisible()));
    connect(ui->actionPlayingSequenceList, SIGNAL(triggered()), playingSequenceListDialog, SLOT(makeVisible()));
    connect(ui->actionBlockCut, SIGNAL(triggered()), this, SLOT(cutBlock()));
    connect(ui->actionBlockCopy, SIGNAL(triggered()), this, SLOT(copyBlock()));
    connect(ui->actionBlockPaste, SIGNAL(triggered()), this, SLOT(pasteBlock()));
    connect(ui->actionBlockClear, SIGNAL(triggered()), this, SLOT(clearBlock()));
    connect(ui->actionBlockSelectAll, SIGNAL(triggered()), this, SLOT(selectAllBlock()));
    connect(ui->actionBlockList, SIGNAL(triggered()), blockListDialog, SLOT(makeVisible()));
    connect(ui->actionBlockTranspose, SIGNAL(triggered()), transposeDialog, SLOT(showBlock()));
    connect(ui->actionBlockExpandShrink, SIGNAL(triggered()), expandShrinkDialog, SLOT(showBlock()));
    connect(ui->actionBlockChangeInstrument, SIGNAL(triggered()), changeInstrumentDialog, SLOT(showBlock()));
    connect(ui->actionBlockSplit, SIGNAL(triggered()), this, SLOT(splitBlock()));
    connect(ui->actionTrackCut, SIGNAL(triggered()), this, SLOT(cutTrack()));
    connect(ui->actionTrackCopy, SIGNAL(triggered()), this, SLOT(copyTrack()));
    connect(ui->actionTrackPaste, SIGNAL(triggered()), this, SLOT(pasteTrack()));
    connect(ui->actionTrackClear, SIGNAL(triggered()), this, SLOT(clearTrack()));
    connect(ui->actionTrackSelectAll, SIGNAL(triggered()), this, SLOT(selectAllTrack()));
    connect(ui->actionTrackInsert, SIGNAL(triggered()), this, SLOT(insertTrack()));
    connect(ui->actionTrackDelete, SIGNAL(triggered()), this, SLOT(deleteTrack()));
    connect(ui->actionTrackTranspose, SIGNAL(triggered()), transposeDialog, SLOT(showTrack()));
    connect(ui->actionTrackExpandShrink, SIGNAL(triggered()), expandShrinkDialog, SLOT(showTrack()));
    connect(ui->actionTrackChangeInstrument, SIGNAL(triggered()), changeInstrumentDialog, SLOT(showTrack()));
    connect(ui->actionSongProperties, SIGNAL(triggered()), songPropertiesDialog, SLOT(makeVisible()));
    connect(ui->actionSongSectionList, SIGNAL(triggered()), sectionListDialog, SLOT(makeVisible()));
    connect(externalSyncActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(setExternalSync()));
    connect(ui->actionMidiMessageList, SIGNAL(triggered()), messageListDialog, SLOT(makeVisible()));
    connect(ui->actionMidiKillAllNotes, SIGNAL(triggered()), player, SLOT(stopAllNotes()));
    connect(ui->actionMidiResetPitch, SIGNAL(triggered()), player, SLOT(resetPitch()));
    connect(ui->actionSettingsPreferences, SIGNAL(triggered()), preferencesDialog, SLOT(makeVisible()));
    connect(ui->actionHelpHelp, SIGNAL(triggered()), helpDialog, SLOT(makeVisible()));
    connect(ui->actionHelpAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(ui->labelSection, SIGNAL(clicked()), sectionListDialog, SLOT(makeVisible()));
    connect(ui->labelPosition, SIGNAL(clicked()), playingSequenceDialog, SLOT(makeVisible()));
    connect(ui->labelPlayingSequence, SIGNAL(clicked()), playingSequenceListDialog, SLOT(makeVisible()));
    connect(ui->labelBlock, SIGNAL(clicked()), blockListDialog, SLOT(makeVisible()));
    connect(blockListDialog, SIGNAL(blockSelected(int)), player, SLOT(setBlock(int)));
    connect(playingSequenceDialog, SIGNAL(positionSelected(int)), player, SLOT(setPosition(int)));
    connect(playingSequenceListDialog, SIGNAL(playingSequenceSelected(int)), player, SLOT(setPlayseq(int)));
    connect(sectionListDialog, SIGNAL(sectionSelected(int)), player, SLOT(setSection(int)));

    setGeometryFromString(this, settings.value("Windows/mainWindowGeometry", rectToString(qApp->desktop()->availableGeometry())).toString());
    setGeometryFromString(instrumentPropertiesDialog, settings.value("Windows/instrumentPropertiesDialogGeometry").toString());
    setGeometryFromString(openDialog, settings.value("Windows/openDialogGeometry").toString());
    setGeometryFromString(preferencesDialog, settings.value("Windows/preferencesDialogGeometry").toString());
    setGeometryFromString(trackVolumesDialog, settings.value("Windows/trackVolumesDialogGeometry").toString());
    setGeometryFromString(transposeDialog, settings.value("Windows/transposeDialogGeometry").toString());
    setGeometryFromString(expandShrinkDialog, settings.value("Windows/expandShrinkDialogGeometry").toString());
    setGeometryFromString(changeInstrumentDialog, settings.value("Windows/changeInstrumentDialogGeometry").toString());
    setGeometryFromString(sectionListDialog, settings.value("Windows/sectionListDialogGeometry").toString());
    setGeometryFromString(songPropertiesDialog, settings.value("Windows/songPropertiesDialogGeometry").toString());
    setGeometryFromString(playingSequenceDialog, settings.value("Windows/playingSequenceDialogGeometry").toString());
    setGeometryFromString(playingSequenceListDialog, settings.value("Windows/playingSequenceListDialogGeometry").toString());
    setGeometryFromString(blockListDialog, settings.value("Windows/blockListDialogGeometry").toString());
    setGeometryFromString(messageListDialog, settings.value("Windows/messageListDialogGeometry").toString());
    setGeometryFromString(helpDialog, settings.value("Windows/helpDialogGeometry").toString());
}

MainWindow::~MainWindow()
{
    settings.setValue("Windows/mainWindowGeometry", rectToString(geometry()));
    settings.setValue("Windows/instrumentPropertiesDialogGeometry", rectToString(instrumentPropertiesDialog->geometry()));
    settings.setValue("Windows/openDialogGeometry", rectToString(openDialog->geometry()));
    settings.setValue("Windows/preferencesDialogGeometry", rectToString(preferencesDialog->geometry()));
    settings.setValue("Windows/trackVolumesDialogGeometry", rectToString(trackVolumesDialog->geometry()));
    settings.setValue("Windows/transposeDialogGeometry", rectToString(transposeDialog->geometry()));
    settings.setValue("Windows/expandShrinkDialogGeometry", rectToString(expandShrinkDialog->geometry()));
    settings.setValue("Windows/changeInstrumentDialogGeometry", rectToString(changeInstrumentDialog->geometry()));
    settings.setValue("Windows/sectionListDialogGeometry", rectToString(sectionListDialog->geometry()));
    settings.setValue("Windows/songPropertiesDialogGeometry", rectToString(songPropertiesDialog->geometry()));
    settings.setValue("Windows/playingSequenceDialogGeometry", rectToString(playingSequenceDialog->geometry()));
    settings.setValue("Windows/playingSequenceListDialogGeometry", rectToString(playingSequenceListDialog->geometry()));
    settings.setValue("Windows/blockListDialogGeometry", rectToString(blockListDialog->geometry()));
    settings.setValue("Windows/messageListDialogGeometry", rectToString(messageListDialog->geometry()));
    settings.setValue("Windows/helpDialogGeometry", rectToString(helpDialog->geometry()));

    delete copySelection_;
    delete copyBlock_;
    delete copyTrack_;
    delete instrumentPropertiesDialog;
    delete openDialog;
    delete preferencesDialog;
    delete trackVolumesDialog;
    delete transposeDialog;
    delete expandShrinkDialog;
    delete changeInstrumentDialog;
    delete sectionListDialog;
    delete songPropertiesDialog;
    delete playingSequenceDialog;
    delete playingSequenceListDialog;
    delete blockListDialog;
    delete messageListDialog;
    delete helpDialog;
    delete externalSyncActionGroup;
    delete ui;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    bool handled = false;

    if ((event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) && watched->isWindowType() && dynamic_cast<QLineEdit *>(QApplication::focusWidget()) == NULL && dynamic_cast<QSpinBox *>(QApplication::focusWidget()) == NULL && dynamic_cast<Tracker *>(QApplication::focusWidget()) == NULL && dynamic_cast<QComboBox *>(QApplication::focusWidget()) == NULL) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (QApplication::activeWindow() == this || (keyEvent->modifiers() != Qt::NoModifier && keyEvent->key() != Qt::Key_Backtab) || (keyEvent->key() != Qt::Key_Tab && keyEvent->key() != Qt::Key_Backtab && keyEvent->key() != Qt::Key_Left && keyEvent->key() != Qt::Key_Right && keyEvent->key() != Qt::Key_Home && keyEvent->key() != Qt::Key_End && keyEvent->key() != Qt::Key_PageUp && keyEvent->key() != Qt::Key_PageDown)) {
            if (event->type() == QEvent::KeyPress) {
                handled = keyPress(keyEvent);
            } else if (event->type() == QEvent::KeyRelease) {
                handled = keyRelease(keyEvent);
            }
        }
    }

    return handled ? true : QMainWindow::eventFilter(watched, event);
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    if (event->delta() > 0) {
        player->setLine(player->line() - 4);
    } else if (event->delta() < 0) {
        player->setLine(player->line() + 4);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *)
{
    if (QApplication::focusWidget() != NULL) {
        QApplication::focusWidget()->clearFocus();
    }
}

bool MainWindow::keyPress(QKeyEvent *event)
{
    if (ui->menubar->activeAction() != NULL) {
        return false;
    }

    // TODO workaround: sometimes the menu bar somehow gets focus even when it has a no focus policy - make sure there is no focus
    ui->menubar->clearFocus();

    bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
    bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
    bool alt = (event->modifiers() & Qt::AltModifier) != 0;
#ifdef __APPLE__
    bool keypad = false;
#else
    bool keypad = (event->modifiers() & Qt::KeypadModifier) != 0;
#endif
    bool handled = false;

    if (ctrl) {
        switch (event->key()) {
#ifdef __linux
        case Qt::Key_Control:
            if (event->nativeVirtualKey() == XK_Control_R) {
                // Right Control: Play song
                player->playSong();
                handled = true;
            }
            break;
#endif
        case Qt::Key_Left:
            // CTRL-Left: Previous instrument
            if (ui->spinBoxInstrument->value() > 0) {
                ui->spinBoxInstrument->setValue(ui->spinBoxInstrument->value() - 1);

                // Make sure the instrument exists
                song->checkInstrument(ui->spinBoxInstrument->value());
            }
            handled = true;
            break;
        case Qt::Key_Right:
            // CTRL-Right: Previous instrument
            if (ui->spinBoxInstrument->value() < 255) {
                ui->spinBoxInstrument->setValue(ui->spinBoxInstrument->value() + 1);

                // Make sure the instrument exists
                song->checkInstrument(ui->spinBoxInstrument->value());
            }
            handled = true;
            break;
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9: {
            if (alt) {
                // CTRL+Alt+0-9: Solo tracks
                int channel = 9;
                if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9) {
                    channel = event->key() - Qt::Key_1;
                }
                if (channel < song->maxTracks()) {
                    song->track(channel)->setSolo(!song->track(channel)->isSolo());
                }
            } else {
                // CTRL+0-9: Set space value
                ui->spinBoxSpace->blockSignals(true);
                ui->spinBoxSpace->setValue(event->key() - Qt::Key_0);
                ui->spinBoxSpace->blockSignals(false);
            }
            handled = true;
            break;
        }
        }
    } else if (shift) {
        switch (event->key()) {
#ifdef __linux
        case Qt::Key_Shift:
            if (event->nativeVirtualKey() == XK_Shift_R) {
                // Right shift: Play block
                player->playBlock();
                handled = true;
            }
            break;
#endif
        case Qt::Key_Left:
            // Shift-Left: Previous position
            player->setPosition(player->position() - 1);
            handled = true;
            break;
        case Qt::Key_Right:
            // Shift-Right: Next position
            player->setPosition(player->position() + 1);
            handled = true;
            break;
        default:
            break;
        }
    } else if (alt) {
        switch (event->key()) {
        case Qt::Key_Left:
            // Alt-Left: Previous block
            player->setBlock(player->block() - 1);
            handled = true;
            break;
        case Qt::Key_Right:
            // Alt-Right: Next block
            player->setBlock(player->block() + 1);
            handled = true;
            break;
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
        case Qt::Key_0: {
            // Alt+0-9: Mute tracks
            int channel = 9;
            if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9) {
                channel = event->key() - Qt::Key_1;
            }

            if (channel < song->maxTracks()) {
                song->track(channel)->setMute(!song->track(channel)->isMuted());
            }
            handled = true;
            break;
        }
        default:
            break;
        }
    } else {
        char instrument = -1;

        if (keypad) {
            switch (event->key()) {
            case Qt::Key_Slash:
                instrument = 1;
                break;
            case Qt::Key_Asterisk:
                instrument = 2;
                break;
            case Qt::Key_Minus:
                instrument = 3;
                break;
            case Qt::Key_7:
            case Qt::Key_Home:
                instrument = 4;
                break;
            case Qt::Key_8:
            case Qt::Key_Up:
                instrument = 5;
                break;
            case Qt::Key_9:
            case Qt::Key_PageUp:
                instrument = 6;
                break;
            case Qt::Key_Plus:
                instrument = 7;
                break;
            case Qt::Key_4:
            case Qt::Key_Left:
                instrument = 8;
                break;
            case Qt::Key_5:
            case Qt::Key_Clear:
                instrument = 9;
                break;
            case Qt::Key_6:
            case Qt::Key_Right:
                instrument = 10;
                break;
            case Qt::Key_1:
            case Qt::Key_End:
                instrument = 11;
                break;
            case Qt::Key_2:
            case Qt::Key_Down:
                instrument = 12;
                break;
            case Qt::Key_3:
            case Qt::Key_PageDown:
                instrument = 13;
                break;
            case Qt::Key_Enter:
                instrument = 14;
                break;
            case Qt::Key_0:
            case Qt::Key_Insert:
                instrument = 15;
                break;
            case Qt::Key_Comma:
            case Qt::Key_Delete:
                instrument = 16;
                break;
            default:
                break;
            }
        } else {
            switch (event->key()) {
            case Qt::Key_NumLock:
                instrument = 0;
                break;
            case Qt::Key_Space:
                if (QApplication::activeWindow() == this) {
                    if (player->mode() != Player::ModeIdle) {
                        // If the song is playing, stop
                        player->stop();
                    } else {
                        // Otherwise toggle edit mode
                        ui->checkBoxEdit->setChecked(!ui->checkBoxEdit->isChecked());
                    }
                    handled = true;
                }
                break;
            case Qt::Key_Menu:
                // Menu: Continue song
                player->continueSong();
                handled = true;
                break;
            case Qt::Key_Meta:
#ifdef __linux
                // Windows: Continue block
                player->continueBlock();
#elif __APPLE__
                // Control: Play block
                player->playBlock();
#endif
                handled = true;
                break;
            default:
                break;
            }
        }

        // Select an instrument if an instrument selection key was pressed
        if (instrument >= 0) {
            ui->spinBoxInstrument->setValue(instrument);

            // Make sure the instrument exists
            song->checkInstrument(ui->spinBoxInstrument->value());
            handled = true;
        }
    }

    if (!handled) {
        ui->tracker->keyPressEvent(event);
    }

    return handled;
}

bool MainWindow::keyRelease(QKeyEvent *event)
{
    if (QApplication::activeWindow() == this) {
        ui->tracker->keyReleaseEvent(event);
    }

    return false;
}

void MainWindow::setSong(Song *song)
{
    if (this->song != NULL) {
        disconnect(ui->actionSettingsSendMidiSync, SIGNAL(triggered(bool)), this->song, SLOT(setSendSync(bool)));
        disconnect(this->song, SIGNAL(nameChanged()), this, SLOT(setWindowTitle()));
        disconnect(this->song, SIGNAL(modifiedChanged()), this, SLOT(setWindowTitle()));
    }

    this->song = song;

    setSection(player->section());
    setPlayseq(player->playseq());
    setPosition(player->position());
    setBlock(player->block());
    setCommandPage(ui->tracker->commandPage());
    setMode(player->mode());
    setTime(0);
    setInstrument(ui->spinBoxInstrument->value());
    ui->tracker->setLine(player->line());
    setWindowTitle();

    instrumentPropertiesDialog->setSong(song);
    instrumentPropertiesDialog->setInstrument(ui->spinBoxInstrument->value());
    ui->actionSettingsSendMidiSync->setChecked(song->sendSync());

    connect(ui->actionSettingsSendMidiSync, SIGNAL(triggered(bool)), song, SLOT(setSendSync(bool)));
    connect(this->song, SIGNAL(nameChanged()), this, SLOT(setWindowTitle()));
    connect(this->song, SIGNAL(modifiedChanged()), this, SLOT(setWindowTitle()));
}

void MainWindow::setSection(unsigned int section)
{
    ui->labelSection->setText(tr("Section %1/%2").arg(section + 1).arg(song->sections()));
}

void MainWindow::setPlayseq(unsigned int playseq)
{
    if (this->playseq < song->playseqs()) {
        disconnect(song->playseq(this->playseq), SIGNAL(nameChanged(QString)), this, SLOT(setPlayseq()));
        disconnect(song->playseq(this->playseq), SIGNAL(lengthChanged()), this, SLOT(setPosition()));
    }

    this->playseq = playseq;

    connect(song->playseq(playseq), SIGNAL(nameChanged(QString)), this, SLOT(setPlayseq()));
    connect(song->playseq(playseq), SIGNAL(lengthChanged()), this, SLOT(setPosition()));

    QString name = song->playseq(playseq)->name();
    if (name.isEmpty()) {
        ui->labelPlayingSequence->setText(tr("Playing Sequence %1/%2").arg(playseq + 1).arg(song->playseqs()));
    } else {
        ui->labelPlayingSequence->setText(tr("Playing Sequence %1/%2: %3").arg(playseq + 1).arg(song->playseqs()).arg(name));
    }
    ui->labelPosition->setText(tr("Position %1/%2").arg(player->position() + 1).arg(song->playseq(player->playseq())->length()));
}

void MainWindow::setPosition(unsigned int position)
{
    ui->labelPosition->setText(tr("Position %1/%2").arg(position + 1).arg(song->playseq(player->playseq())->length()));
}

void MainWindow::setBlock(unsigned int number)
{
    if (this->block < song->blocks()) {
        disconnect(song->block(this->block), SIGNAL(nameChanged(QString)), this, SLOT(setBlock()));
        disconnect(song->block(this->block), SIGNAL(commandPagesChanged(int)), this, SLOT(setCommandPage()));
        disconnect(song->block(this->block), SIGNAL(tracksChanged(int)), this, SLOT(setDeleteTrackVisibility()));
    }

    this->block = number;

    Block *block = song->block(number);
    connect(block, SIGNAL(nameChanged(QString)), this, SLOT(setBlock()));
    connect(block, SIGNAL(commandPagesChanged(int)), this, SLOT(setCommandPage()));
    connect(block, SIGNAL(tracksChanged(int)), this, SLOT(setDeleteTrackVisibility()));
    ui->tracker->setBlock(block);

    QString name = block->name();
    if (name.isEmpty()) {
        ui->labelBlock->setText(tr("Block %1/%2").arg(number + 1).arg(song->blocks()));
    } else {
        ui->labelBlock->setText(tr("Block %1/%2: %3").arg(number + 1).arg(song->blocks()).arg(name));
    }
    setCommandPage(ui->tracker->commandPage());
    setDeleteTrackVisibility();
}

void MainWindow::setCommandPage(int commandPage)
{
    ui->labelCommandPage->setText(tr("Command Page %1/%2").arg(commandPage + 1).arg(song->block(player->block())->commandPages()));
}

void MainWindow::setMode(Player::Mode mode)
{
    switch (mode) {
    case Player::ModePlaySong:
        ui->labelStatus->setText(tr("Playing song"));
        break;
    case Player::ModePlayBlock:
        ui->labelStatus->setText(tr("Playing block"));
        break;
    default:
        ui->labelStatus->setText(tr("Stopped"));
        break;
    }
}

void MainWindow::setTime(unsigned int time)
{
    ui->labelTimer->setText(QString("%1:%2").arg(time / 60, 2, 10, QChar('0')).arg(time % 60, 2, 10, QChar('0')));
}

void MainWindow::setInstrument(int instrument)
{
    if (instrument >= 0) {
        Instrument *oldInstrument = song->instrument(this->instrument);
        if (oldInstrument != NULL) {
            disconnect(oldInstrument, SIGNAL(nameChanged(QString)), ui->lineEditInstrument, SLOT(setText(QString)));
            disconnect(ui->lineEditInstrument, SIGNAL(textChanged(QString)), oldInstrument, SLOT(setName(QString)));
        }

        this->instrument = instrument;
        song->checkInstrument(this->instrument);

        Instrument *newInstrument = song->instrument(this->instrument);
        connect(newInstrument, SIGNAL(nameChanged(QString)), ui->lineEditInstrument, SLOT(setText(QString)));
        connect(ui->lineEditInstrument, SIGNAL(textChanged(QString)), newInstrument, SLOT(setName(QString)));

        ui->lineEditInstrument->blockSignals(true);
        ui->lineEditInstrument->setText(newInstrument->name());
        ui->lineEditInstrument->blockSignals(false);
        ui->tracker->setInstrument(instrument + 1);
    }
}

void MainWindow::setSelection(int startTrack, int startLine, int endTrack, int endLine)
{
    selectionStartTrack = startTrack;
    selectionStartLine = startLine;
    selectionEndTrack = endTrack;
    selectionEndLine = endLine;
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("About Tutka"), tr("Tutka 1.1.0\n\n© 2002-2016 Vesa Halttunen <vesuri@jormas.com>"));
}

void MainWindow::cutSelection()
{
    if (ui->tracker->isInSelectionMode()) {
        ui->tracker->markSelection(false);
    }

    delete copySelection_;
    copySelection_ = song->block(block)->copy(selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
    song->block(block)->clear(selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
    ui->tracker->clearMarkSelection();
}

void MainWindow::copySelection()
{
    if (ui->tracker->isInSelectionMode()) {
        ui->tracker->markSelection(false);
    }

    delete copySelection_;
    copySelection_ = song->block(block)->copy(selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
}

void MainWindow::pasteSelection()
{
    if (copySelection_ != NULL) {
        song->block(block)->paste(copySelection_, ui->tracker->track(), ui->tracker->line());
    }
}

void MainWindow::clearSelection()
{
    song->block(block)->clear(selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
}

void MainWindow::cutBlock()
{
    delete copyBlock_;
    copyBlock_ = song->block(block)->copy(0, 0, song->block(block)->tracks() - 1, song->block(block)->length() - 1);
    song->block(block)->clear(0, 0, song->block(block)->tracks() - 1, song->block(block)->length() - 1);
}

void MainWindow::copyBlock()
{
    delete copyBlock_;
    copyBlock_ = song->block(block)->copy(0, 0, song->block(block)->tracks() - 1, song->block(block)->length() - 1);
}

void MainWindow::pasteBlock()
{
    if (copyBlock_ != NULL) {
        song->block(block)->paste(copyBlock_, 0, 0);
    }
}

void MainWindow::clearBlock()
{
    song->block(block)->clear(0, 0, song->block(block)->tracks() - 1, song->block(block)->length() - 1);
}

void MainWindow::selectAllBlock()
{
    ui->tracker->setSelection(0, 0, song->block(block)->tracks() - 1, song->block(block)->length() - 1);
}

void MainWindow::splitBlock()
{
    song->splitBlock(block, ui->tracker->line());
}

void MainWindow::cutTrack()
{
    delete copyTrack_;
    copyTrack_ = song->block(block)->copy(ui->tracker->track(), 0, ui->tracker->track(), song->block(block)->length() - 1);
    song->block(block)->clear(ui->tracker->track(), 0, ui->tracker->track(), song->block(block)->length() - 1);
}

void MainWindow::copyTrack()
{
    delete copyTrack_;
    copyTrack_ = song->block(block)->copy(ui->tracker->track(), 0, ui->tracker->track(), song->block(block)->length() - 1);
}

void MainWindow::pasteTrack()
{
    if (copyTrack_ != NULL) {
        song->block(block)->paste(copyTrack_, ui->tracker->track(), 0);
    }
}

void MainWindow::clearTrack()
{
    song->block(block)->clear(ui->tracker->track(), 0, ui->tracker->track(), song->block(block)->length() - 1);
}

void MainWindow::selectAllTrack()
{
    ui->tracker->setSelection(ui->tracker->track(), 0, ui->tracker->track(), song->block(block)->length() - 1);
}

void MainWindow::insertTrack()
{
    song->block(block)->insertTrack(ui->tracker->track());
}

void MainWindow::deleteTrack()
{
    song->block(block)->deleteTrack(ui->tracker->track());
}

void MainWindow::setExternalSync()
{
    player->setExternalSync(externalSyncActionGroup->checkedAction() == ui->actionExternalSyncOff ? Player::Off : Player::Midi);
}

void MainWindow::save()
{
    QString path = song->path();

    if (path.isEmpty()) {
        saveAs();
    } else {
        song->save(path);
    }
}

void MainWindow::saveAs()
{
    QString path = QFileDialog::getSaveFileName(NULL, tr("Save as"), settings.value("Paths/songPath").toString(), tr("Tutka songs (*.tutka);;OctaMED SoundStudio songs (*.med);;Standard MIDI files (*.mid)"));

    if (!path.isEmpty()) {
        if (path.endsWith(".med") || path.endsWith(".mmd")) {
            struct MMD2 *mmd = songToMMD2(song);
            MMD2_save(mmd, path.toUtf8().constData());
            MMD2_free(mmd);
        } else if (path.endsWith(".mid")) {
            SMF *smf = songToSMF(song);
            smf->save(path);
            delete smf;
        } else {
            song->save(path);
        }

        setSongPath(path);
    }
}

void MainWindow::setSection()
{
    setSection(player->section());
}

void MainWindow::setPlayseq()
{
    setPlayseq(player->playseq());
}

void MainWindow::setPosition()
{
    setPosition(player->position());
}

void MainWindow::setBlock()
{
    setBlock(player->block());
}

void MainWindow::setCommandPage()
{
    setCommandPage(ui->tracker->commandPage());
}

void MainWindow::handleMidiInput(const QByteArray &data)
{
    if (data.length() == 0) {
        return;
    }

    Block *block = ui->tracker->block();
    switch (data[0] & 0xf0) {
    case 0x80:
        // Note off
        if (ui->checkBoxEdit->isChecked() && ui->checkBoxChord->isChecked()) {
            ui->tracker->removeChordNote();
        }
        break;
    case 0x90:
        // Note on
        if (ui->checkBoxEdit->isChecked()) {
            block->setNote(ui->tracker->line(), ui->tracker->cursorTrack(), data[1] / 12, data[1] % 12 + 1, ui->spinBoxInstrument->value() + 1);
            block->setCommandFull(ui->tracker->line(), ui->tracker->cursorTrack(), ui->tracker->commandPage(), Player::CommandVelocity, data[2]);
            if (ui->checkBoxChord->isChecked()) {
                ui->tracker->addChordNote();
            } else {
                advancePlayerBySpaceLines();
            }
        }
        break;
    case 0xb0:
        // MIDI controller
        if (ui->checkBoxEdit->isChecked() && ui->actionSettingsRecordControllers->isChecked()) {
            block->setCommandFull(ui->tracker->line(), ui->tracker->cursorTrack(), ui->tracker->commandPage(), Player::CommandMidiControllers + data[1], data[2]);
        }
        break;
    case 0xe0:
        // Pitch wheel
        if (ui->checkBoxEdit->isChecked()) {
            block->setCommandFull(ui->tracker->line(), ui->tracker->cursorTrack(), ui->tracker->commandPage(), Player::CommandPitchWheel, data[2]);
        }
        break;
    default:
        break;
    }
}

void MainWindow::setTrackerHorizontalScrollBar(int track, int tracks, int visibleTracks)
{
    ui->horizontalScrollBarTracker->blockSignals(true);
    ui->horizontalScrollBarTracker->setRange(0, tracks - visibleTracks);
    ui->horizontalScrollBarTracker->setValue(track);
    ui->horizontalScrollBarTracker->setPageStep(visibleTracks);
    ui->horizontalScrollBarTracker->blockSignals(false);
}

void MainWindow::setTrackerVerticalScrollBar(int line, int length, int visibleLines)
{
    ui->verticalScrollBarTracker->blockSignals(true);
    ui->verticalScrollBarTracker->setRange(0, length - 1);
    ui->verticalScrollBarTracker->setValue(line);
    ui->verticalScrollBarTracker->setPageStep(visibleLines);
    ui->verticalScrollBarTracker->blockSignals(false);
    ui->actionBlockSplit->setEnabled(line > 0);
}

void MainWindow::setSongPath(const QString &path)
{
    settings.setValue("Paths/songPath", QFileInfo(path).absolutePath());
}

void MainWindow::setDeleteTrackVisibility()
{
    if (song != NULL && block < song->blocks()) {
        ui->actionTrackDelete->setEnabled(song->block(block)->tracks() > 1);
    }
}

QRect MainWindow::stringToRect(const QString &string)
{
     QRegExp regExp("=?([\\d]+)[xX]([\\d]+)([+-][\\d]+)([+-][\\d]+)");
     if (regExp.indexIn(string) >= 0) {
         QStringList captures = regExp.capturedTexts();
         if (captures.length() == 5) {
             return QRect(captures.at(3).toInt(), captures.at(4).toInt(), captures.at(1).toInt(), captures.at(2).toInt());
         }
     }
     return QRect();
}

QString MainWindow::rectToString(const QRect &rect)
{
    return QString::number(rect.width()) + "x" + QString::number(rect.height()) + (rect.x() >= 0 ? QString("+") : QString()) + QString::number(rect.x()) + (rect.y() >= 0 ? QString("+") : QString()) + QString::number(rect.y());
}

void MainWindow::setGeometryFromString(QWidget *widget, const QString &string)
{
    if (widget != NULL) {
        QRect rect = stringToRect(string);
        if (rect.isValid()) {
            widget->setGeometry(rect);
        } else {
            QRect mainRect = qApp->desktop()->availableGeometry();
            QRect widgetRect = widget->geometry();
            widget->setGeometry(mainRect.x() + (mainRect.width() - widgetRect.width()) / 2, mainRect.y() + (mainRect.height() - widgetRect.height()) / 2, widgetRect.width(), widgetRect.height());
        }
    }
}

void MainWindow::setWindowTitle()
{
    QMainWindow::setWindowTitle(tr("Tutka: %1%2").arg(song->name()).arg(song->isModified() ? " (*)" : ""));
}

int MainWindow::showModifiedDialog() const
{
    QMessageBox messageBox;
    messageBox.setWindowTitle(tr("Tutka"));
    messageBox.setText(tr("The song has been modified."));
    messageBox.setInformativeText(tr("Do you want to save your changes?"));
    messageBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    messageBox.setDefaultButton(QMessageBox::Save);
    return messageBox.exec();
}

void MainWindow::newSong()
{
    if (song != NULL && song->isModified()) {
        switch (showModifiedDialog()) {
        case QMessageBox::Save:
            save();
            player->setSong();
            break;
        case QMessageBox::Discard:
            player->setSong();
            break;
        default:
            break;
        }
    } else {
        player->setSong();
    }
}

void MainWindow::quit()
{
    if (song != NULL && song->isModified()) {
        switch (showModifiedDialog()) {
        case QMessageBox::Save:
            save();
            qApp->quit();
            break;
        case QMessageBox::Discard:
            qApp->quit();
            break;
        default:
            break;
        }
    } else {
        qApp->quit();
    }
}

void MainWindow::advancePlayerBySpaceLines()
{
    player->setLine(player->line() + ui->spinBoxSpace->value());
}

void MainWindow::playPressedNote(unsigned char note)
{
    player->playNote(ui->spinBoxInstrument->value(), ui->comboBoxKeyboardOctaves->currentIndex() * 12 + note, 127, ui->tracker->cursorTrack());
}
