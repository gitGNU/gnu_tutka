/*
 * mainwindow.cpp
 *
 * Copyright 2002-2011 vesuri
 *
 * This file is part of Tutka2.
 *
 * Tutka2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tutka2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tutka2; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <cstdio>
#include <QApplication>
#include <QKeyEvent>
#include <QFileDialog>
#include <QMessageBox>
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
#include "song.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"

MainWindow::MainWindow(Player *player, QWidget *parent) :
    QMainWindow(parent),
    player(player),
    mainWindow(new Ui::MainWindow),
    openDialog(new QFileDialog),
    saveDialog(new QFileDialog),
    preferencesDialog(new PreferencesDialog),
    trackVolumesDialog(new TrackVolumesDialog),
    transposeDialog(new TransposeDialog),
    expandShrinkDialog(new ExpandShrinkDialog),
    changeInstrumentDialog(new ChangeInstrumentDialog),
    sectionListDialog(new SectionListDialog),
    songPropertiesDialog(new SongPropertiesDialog),
    playingSequenceDialog(new PlayingSequenceDialog),
    playingSequenceListDialog(new PlayingSequenceListDialog),
    blockListDialog(new BlockListDialog),
    messageListDialog(new MessageListDialog),
    chordStatus(0)
{
    mainWindow->setupUi(this);
    qApp->installEventFilter(this);
    setFocus(Qt::ActiveWindowFocusReason);

    connect(player, SIGNAL(songChanged(Song *)), this, SLOT(setSong(Song *)));
    connect(player, SIGNAL(songChanged(Song *)), mainWindow->trackerMain, SLOT(setSong(Song *)));
    connect(player, SIGNAL(sectionChanged(unsigned int)), this, SLOT(setSection(unsigned int)));
    connect(player, SIGNAL(playseqChanged(unsigned int)), this, SLOT(setPlayseq(unsigned int)));
    connect(player, SIGNAL(positionChanged(unsigned int)), this, SLOT(setPosition(unsigned int)));
    connect(player, SIGNAL(blockChanged(unsigned int)), this, SLOT(setBlock(unsigned int)));
    connect(player, SIGNAL(blockChanged(unsigned int)), mainWindow->trackerMain, SLOT(setBlock(unsigned int)));
    connect(player, SIGNAL(lineChanged(unsigned int)), mainWindow->trackerMain, SLOT(setLine(unsigned int)));
    connect(player, SIGNAL(modeChanged(Player::Mode)), this, SLOT(setMode(Player::Mode)));
    connect(player, SIGNAL(timeChanged(unsigned int)), this, SLOT(setTime(unsigned int)));
    connect(mainWindow->buttonPlaySong, SIGNAL(clicked()), player, SLOT(playSong()));
    connect(mainWindow->buttonPlayBlock, SIGNAL(clicked()), player, SLOT(playBlock()));
    connect(mainWindow->buttonContinueSong, SIGNAL(clicked()), player, SLOT(continueSong()));
    connect(mainWindow->buttonContinueBlock, SIGNAL(clicked()), player, SLOT(continueBlock()));
    connect(mainWindow->buttonStop, SIGNAL(clicked()), player, SLOT(stop()));
    connect(mainWindow->actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(mainWindow->actionPreferences, SIGNAL(triggered()), preferencesDialog, SLOT(show()));
    connect(mainWindow->actionTrack_Volumes, SIGNAL(triggered()), trackVolumesDialog, SLOT(show()));
    connect(mainWindow->actionTranspose, SIGNAL(triggered()), transposeDialog, SLOT(show()));
    connect(mainWindow->actionTranspose_2, SIGNAL(triggered()), transposeDialog, SLOT(show()));
    connect(mainWindow->actionTranspose_3, SIGNAL(triggered()), transposeDialog, SLOT(show()));
    connect(mainWindow->actionTranspose_4, SIGNAL(triggered()), transposeDialog, SLOT(show()));
    connect(mainWindow->actionExpand_Shrink, SIGNAL(triggered()), expandShrinkDialog, SLOT(show()));
    connect(mainWindow->actionExpand_Shrink_2, SIGNAL(triggered()), expandShrinkDialog, SLOT(show()));
    connect(mainWindow->actionExpand_Shrink_3, SIGNAL(triggered()), expandShrinkDialog, SLOT(show()));
    connect(mainWindow->actionExpand_Shrink_4, SIGNAL(triggered()), expandShrinkDialog, SLOT(show()));
    connect(mainWindow->actionChange_Instrument, SIGNAL(triggered()), changeInstrumentDialog, SLOT(show()));
    connect(mainWindow->actionChange_Instrument_2, SIGNAL(triggered()), changeInstrumentDialog, SLOT(show()));
    connect(mainWindow->actionChange_Instrument_3, SIGNAL(triggered()), changeInstrumentDialog, SLOT(show()));
    connect(mainWindow->actionChange_Instrument_4, SIGNAL(triggered()), changeInstrumentDialog, SLOT(show()));
    connect(mainWindow->actionProperties, SIGNAL(triggered()), songPropertiesDialog, SLOT(show()));
    connect(mainWindow->actionSection_List, SIGNAL(triggered()), sectionListDialog, SLOT(show()));
    connect(mainWindow->actionEdit_Current, SIGNAL(triggered()), playingSequenceDialog, SLOT(show()));
    connect(mainWindow->actionList, SIGNAL(triggered()), playingSequenceListDialog, SLOT(show()));
    connect(mainWindow->actionList_2, SIGNAL(triggered()), blockListDialog, SLOT(show()));
    connect(mainWindow->actionMessage_List, SIGNAL(triggered()), messageListDialog, SLOT(show()));
    connect(mainWindow->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));

    keyToNote.insert(Qt::Key_Z, 1);
    keyToNote.insert(Qt::Key_S, 2);
    keyToNote.insert(Qt::Key_X, 3);
    keyToNote.insert(Qt::Key_D, 4);
    keyToNote.insert(Qt::Key_C, 5);
    keyToNote.insert(Qt::Key_V, 6);
    keyToNote.insert(Qt::Key_G, 7);
    keyToNote.insert(Qt::Key_B, 8);
    keyToNote.insert(Qt::Key_H, 9);
    keyToNote.insert(Qt::Key_N, 10);
    keyToNote.insert(Qt::Key_J, 11);
    keyToNote.insert(Qt::Key_M, 12);
    keyToNote.insert(Qt::Key_Q, 13);
    keyToNote.insert(Qt::Key_Comma, 13);
    keyToNote.insert(Qt::Key_2, 14);
    keyToNote.insert(Qt::Key_L, 14);
    keyToNote.insert(Qt::Key_W, 15);
    keyToNote.insert(Qt::Key_Period, 15);
    keyToNote.insert(Qt::Key_3, 16);
    keyToNote.insert(Qt::Key_Odiaeresis, 16);
    keyToNote.insert(Qt::Key_E, 17);
    keyToNote.insert(Qt::Key_Minus, 17);
    keyToNote.insert(Qt::Key_R, 18);
    keyToNote.insert(Qt::Key_5, 19);
    keyToNote.insert(Qt::Key_T, 20);
    keyToNote.insert(Qt::Key_6, 21);
    keyToNote.insert(Qt::Key_Y, 22);
    keyToNote.insert(Qt::Key_7, 23);
    keyToNote.insert(Qt::Key_U, 24);
    keyToNote.insert(Qt::Key_I, 25);
    keyToNote.insert(Qt::Key_9, 26);
    keyToNote.insert(Qt::Key_O, 27);
    keyToNote.insert(Qt::Key_0, 28);
    keyToNote.insert(Qt::Key_P, 29);
    keyToNote.insert(Qt::Key_Aring, 30);
    keyToNote.insert(Qt::Key_acute, 31);
    keyToNote.insert(Qt::Key_Delete, 0);
}

MainWindow::~MainWindow()
{
    delete mainWindow;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    bool handled = false;

    if (event->type() == QEvent::KeyPress) {
        handled = keyPress(static_cast<QKeyEvent *>(event));
    } else if (event->type() == QEvent::KeyRelease) {
        handled = keyRelease(static_cast<QKeyEvent *>(event));
    }

    return handled ? true : QMainWindow::eventFilter(watched, event);
}

bool MainWindow::keyPress(QKeyEvent *event)
{
    if (dynamic_cast<QLineEdit *>(QApplication::focusWidget()) != NULL) {
        return false;
    }

    Tracker *tracker = mainWindow->trackerMain;
    Song *song = tracker->song();
    Block *block = tracker->block();
    bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
    bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
    bool alt = (event->modifiers() & Qt::AltModifier) != 0;
    bool handled = false;

    if (ctrl) {
        switch (event->key()) {
        case 'b':
            /* CTRL-B: Selection mode on/off */
            tracker->markSelection(!tracker->isInSelectionMode());
            handled = true;
            break;
        case 'k':
            /* CTRL-K: Clear until the end of the track */
            block->clear(tracker->track(), tracker->line(), tracker->track(), block->length() - 1);
            tracker->redraw();
            handled = true;
            break;
        case Qt::Key_Left:
            /* CTRL-Left: Previous instrument */
            if (mainWindow->spinBoxInstrument->value() > 1) {
                mainWindow->spinBoxInstrument->setValue(mainWindow->spinBoxInstrument->value() - 1);

                /* Make sure the instrument exists */
                song->checkInstrument(mainWindow->spinBoxInstrument->value() - 1, 0);
            }
            handled = true;
            break;
        case Qt::Key_Right:
            /* CTRL-Right: Previous instrument */
            mainWindow->spinBoxInstrument->setValue(mainWindow->spinBoxInstrument->value() + 1);

            /* Make sure the instrument exists */
            song->checkInstrument(mainWindow->spinBoxInstrument->value() - 1, 0);

            handled = true;
            break;
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            /* CTRL-Shift-Tab: Next command page */
            tracker->setCommandPage(tracker->commandPage() + 1);
            handled = true;
            break;
        case Qt::Key_Delete: {
            if (QApplication::activeWindow() == this) {
                /* Delete command and refresh */
                for (int i = 0; i < block->commandPages(); i++) {
                    block->setCommandFull(tracker->line(), tracker->track(), i, 0, 0);
                }
                tracker->redrawRow(tracker->line());

                tracker->setLine(tracker->line() + mainWindow->spinBoxSpace->value());

                handled = true;
            }
            break;
        }
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
                /* CTRL+Alt+0-9: Solo tracks */
                int channel = 9;
                if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9) {
                    channel = event->key() - Qt::Key_1;
                }
                if (channel < song->maxTracks()) {
                    song->track(channel)->setSolo(!song->track(channel)->isSolo());
                    tracker->redraw();
                }
            } else {
                /* CTRL+0-9: Set space value */
                // TODO
                mainWindow->spinBoxSpace->blockSignals(true);
                mainWindow->spinBoxSpace->setValue(event->key() - Qt::Key_0);
                mainWindow->spinBoxSpace->blockSignals(false);

                handled = true;
                break;
            }
        }
        }
    } else if (shift) {
        switch (event->key()) {
        case Qt::Key_Left:
            /* Shift-Left: Previous position */
            player->setPosition(player->position() - 1);
            handled = true;
            break;
        case Qt::Key_Right:
            /* Shift-Right: Next position */
            player->setPosition(player->position() + 1);
            handled = true;
            break;
        case Qt::Key_Backtab:
            /* Shift-Tab: Previous track - only in main window */
            if (QApplication::activeWindow() == this) {
                tracker->stepCursorChannel(-1);
                handled = true;
            }
            break;
        case Qt::Key_Delete: {
            if (QApplication::activeWindow() == this) {
                int n = block->commandPages();

                /* Delete note+command and refresh */
                block->setNote(tracker->line(), tracker->cursorChannel(), 0, 0, 0);
                for (int commandPage = 0; commandPage < n; commandPage++) {
                    block->setCommandFull(tracker->line(), tracker->cursorChannel(), commandPage, 0, 0);
                }
                tracker->redrawRow(tracker->line());

                tracker->setLine(tracker->line() + mainWindow->spinBoxSpace->value());

                handled = true;
            }
            break;
        }
        case Qt::Key_Backspace:
            /* Shift-Backspace: Insert row */
            if (QApplication::activeWindow() == this) {
                if (mainWindow->checkBoxEdit->isChecked()) {
                    block->insertLine(tracker->line(), alt ? -1 : tracker->cursorChannel());
                    tracker->redraw();
                }
                handled = true;
            }
            break;
        default:
            break;
        }
    } else if (alt) {
        switch (event->key()) {
        case Qt::Key_Left:
            /* Alt-Left: Previous block */
            //TODO
            //editor->setBlock(editor->block - 1);
            handled = true;
            break;
        case Qt::Key_Right:
            /* Alt-Right: Next block */
            //TODO
            //editor->setBlock(editor->block + 1);
            handled = true;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0': {
            /* Alt+0-9: Mute tracks */
            int channel = 9;
            if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9) {
                channel = event->key() - Qt::Key_1;
            }

            if (channel < song->maxTracks()) {
                song->track(channel)->setMute(!song->track(channel)->isMuted());
            }
            break;
        }
        case Qt::Key_Backspace:
            /* Backspace: delete line */
            if (QApplication::activeWindow() == this) {
                block->deleteLine(tracker->line());
                tracker->redraw();
                handled = true;
            }
            break;
        default:
            break;
        }
    } else {
        char instrument = -1;
        char note = -1;
        char data = -1;
        if (tracker->cursorItem() == 0) {
            /* Editing notes */
            data = keyToNote.value(event->key(), -1);

            if (data >= 0) {
                if (mainWindow->checkBoxChord->isChecked()) {

                    /* Chord mode: check if the key has already been pressed (key repeat) */
                    bool found = keyboardKeysDown.contains(event->key());

                    /* Go to next channel if the current key has not been pressed down yet */
                    if (!found) {
                        /* Add the key to the keys down list */
                        keyboardKeysDown.append(event->key());

                        if (mainWindow->checkBoxEdit->isChecked()) {
                            /* Set note and refresh */
                            block->setNote(tracker->line(), tracker->cursorChannel(), mainWindow->comboBoxKeyboardOctaves->currentIndex(), data, mainWindow->spinBoxInstrument->value());
                            tracker->redrawRow(tracker->line());
                        }

                        tracker->stepCursorChannel(1);
                        chordStatus++;
                    }
                } else if (mainWindow->checkBoxEdit->isChecked()) {
                    /* Set note and refresh */
                    block->setNote(tracker->line(), tracker->cursorChannel(), mainWindow->comboBoxKeyboardOctaves->currentIndex(), data, mainWindow->spinBoxInstrument->value());
                    tracker->redrawRow(tracker->line());
                    tracker->setLine(tracker->line() + mainWindow->spinBoxSpace->value());
                }
                handled = true;
            }
        } else if (mainWindow->checkBoxEdit->isChecked()) {
            switch (tracker->cursorItem()) {
            case 1:
            case 2:
                /* Editing instrument */
                if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9) {
                    data = event->key() - Qt::Key_0;
                } else if (event->key() >= Qt::Key_A && event->key() <= Qt::Key_F) {
                    data = 10 + event->key() - Qt::Key_A;
                } else if (event->key() == Qt::Key_Delete) {
                    data = 0;
                }

                if (data >= 0) {
                    /* Set instrument and refresh */
                    int ins = block->instrument(tracker->line(), tracker->cursorChannel());
                    if (tracker->cursorItem() == 1) {
                        ins = (ins & 0x0f) | (data << 4);
                    } else {
                        ins = (ins & 0xf0) | data;
                    }
                    block->setInstrument(tracker->line(), tracker->cursorChannel(), ins);
                    tracker->redrawRow(tracker->line());
                    tracker->setLine(tracker->line() + mainWindow->spinBoxSpace->value());
                    handled = true;
                }
                break;
            case 3:
            case 4:
            case 5:
            case 6:
                /* Editing effects */
                if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9) {
                    data = event->key() - Qt::Key_0;
                } else if (event->key() >= Qt::Key_A && event->key() <= Qt::Key_F) {
                    data = 10 + event->key() - Qt::Key_A;
                } else if (event->key() == Qt::Key_Delete) {
                    data = 0;
                }

                if (data >= 0) {
                    /* Set effect and refresh */
                    block->setCommand(tracker->line(), tracker->cursorChannel(), tracker->commandPage(), tracker->cursorItem() - 3, data);
                    tracker->redrawRow(tracker->line());
                    tracker->setLine(tracker->line() + mainWindow->spinBoxSpace->value());
                    handled = true;
                }
                break;
            }
        }

        note = keyToNote.value(event->key(), -1) - 1;
        switch (event->key()) {
        case Qt::Key_Shift:
            /* Right shift: Play block */
            player->playBlock();
            handled = true;
            break;
        case Qt::Key_Control:
            /* Right Control: Play song */
            player->playSong();
            handled = true;
            break;
        case Qt::Key_Space:
            /* If the song is playing, stop */
            if (player->mode() != Player::IDLE) {
                player->stop();
            } else {
                /* Otherwise toggle edit mode */
                mainWindow->checkBoxEdit->blockSignals(true);
                mainWindow->checkBoxEdit->setChecked(!mainWindow->checkBoxEdit->isChecked());
                mainWindow->checkBoxEdit->blockSignals(false);
            }
            handled = true;
            break;
        case Qt::Key_F1:
        case Qt::Key_F2:
        case Qt::Key_F3:
        case Qt::Key_F4:
        case Qt::Key_F5:
        case Qt::Key_F6:
        case Qt::Key_F7:
        case Qt::Key_F8:
        case Qt::Key_F9:
        case Qt::Key_F10:
        case Qt::Key_F11: {
            /* Set active keyboard octave */
            mainWindow->comboBoxKeyboardOctaves->blockSignals(true);
            mainWindow->comboBoxKeyboardOctaves->setCurrentIndex(event->key() - Qt::Key_F1);
            mainWindow->comboBoxKeyboardOctaves->blockSignals(false);

            handled = true;
            break;
        }
        case Qt::Key_NumLock:
            instrument = 0;
            break;
#ifdef TODO
        case Qt::Key_KP_Divide:
            instrument = 1;
            break;
        case Qt::Key_KP_Multiply:
            instrument = 2;
            break;
        case Qt::Key_KP_Subtract:
            instrument = 3;
            break;
        case Qt::Key_KP_7:
        case Qt::Key_KP_Home:
            instrument = 4;
            break;
        case Qt::Key_KP_8:
        case Qt::Key_KP_Up:
            instrument = 5;
            break;
        case Qt::Key_KP_9:
        case Qt::Key_KP_Page_Up:
            instrument = 6;
            break;
        case Qt::Key_KP_Add:
            instrument = 7;
            break;
        case Qt::Key_KP_4:
        case Qt::Key_KP_Left:
            instrument = 8;
            break;
        case Qt::Key_KP_5:
        case Qt::Key_KP_Begin:
            instrument = 9;
            break;
        case Qt::Key_KP_6:
        case Qt::Key_KP_Right:
            instrument = 10;
            break;
        case Qt::Key_KP_1:
        case Qt::Key_KP_End:
            instrument = 11;
            break;
        case Qt::Key_KP_2:
        case Qt::Key_KP_Down:
            instrument = 12;
            break;
        case Qt::Key_KP_3:
        case Qt::Key_KP_Page_Down:
            instrument = 13;
            break;
        case Qt::Key_Enter:
            instrument = 14;
            break;
        case Qt::Key_KP_0:
        case Qt::Key_KP_Insert:
            instrument = 15;
            break;
        case Qt::Key_KP_Decimal:
        case Qt::Key_KP_Delete:
            instrument = 16;
            break;
#endif
        case Qt::Key_Down:
            /* Down: Go down -*/
            if (QApplication::activeWindow() == this) {
                tracker->setLine(tracker->line() + 1);
                handled = true;
            }
            break;
        case Qt::Key_Up:
            /* Up: Go up */
            if (QApplication::activeWindow() == this) {
                tracker->setLine(tracker->line() - 1);
                handled = true;
            }
            break;
        case Qt::Key_Left:
            /* Left: Go left */
            if (QApplication::activeWindow() == this) {
                tracker->stepCursorItem(-1);
                handled = true;
            }
            break;
        case Qt::Key_Right:
            /* Right: Go right */
            if (QApplication::activeWindow() == this) {
                tracker->stepCursorItem(1);
                handled = true;
            }
            break;
        case Qt::Key_Tab:
            /* Tab: Next track */
            if (QApplication::activeWindow() == this) {
                tracker->setCursorItem(0);
                tracker->stepCursorChannel(1);
                handled = true;
            }
            break;
        case Qt::Key_Home:
            /* End: Go to the beginning of block */
            if (QApplication::activeWindow() == this) {
                tracker->setLine(0);
                handled = true;
            }
            break;
        case Qt::Key_End:
            /* End: Go to the end of block */
            if (QApplication::activeWindow() == this) {
                tracker->setLine(block->length() - 1);
                handled = true;
            }
            break;
        case Qt::Key_PageDown:
            /* Page down: Go down 8 lines */
            if (QApplication::activeWindow() == this) {
                tracker->setLine(tracker->line() + 8);
                handled = true;
            }
            break;
        case Qt::Key_PageUp:
            /* Page up: Go up 8 lines */
            if (QApplication::activeWindow() == this) {
                tracker->setLine(tracker->line() - 8);
                handled = true;
            }
            break;
        case Qt::Key_Backspace:
            /* Backspace: delete line */
            if (QApplication::activeWindow() == this) {
                block->deleteLine(tracker->line(), tracker->cursorChannel());
                tracker->redraw();
                handled = true;
            }
            break;
        }

        /* Select an instrument if an instrument selection key was pressed */
        if (instrument >= 0) {
            mainWindow->spinBoxInstrument->setValue(instrument);

            /* Make sure the instrument exists */
            song->checkInstrument(mainWindow->spinBoxInstrument->value() - 1, 0);
            handled = true;
        }

        /* Play note if a key was pressed but not if cursor is in cmd pos */
        if (note >= 0 && tracker->cursorItem() == 0) {
            player->playNote(mainWindow->spinBoxInstrument->value() - 1, mainWindow->comboBoxKeyboardOctaves->currentIndex() * 12 + note, 127, tracker->cursorChannel());
            handled = true;
        }
    }

    return handled;
}

bool MainWindow::keyRelease(QKeyEvent * event)
{
    Tracker *tracker = mainWindow->trackerMain;
    bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
    bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
    bool handled = false;

    /* Key has been released */
    if (!ctrl && !shift) {
        if (mainWindow->checkBoxChord->isChecked() && tracker->cursorItem() == 0) {
            if (event->key() != Qt::Key_Delete && keyToNote.contains(event->key())) {
                /* Find the key from the keys down list and remove it */
                keyboardKeysDown.removeAll(event->key());

                /* Go to the previous channel */
                tracker->stepCursorChannel(-1);
                chordStatus--;

                /* If all chord notes have been released go to the next line */
                if (chordStatus == 0 && mainWindow->checkBoxEdit->isChecked()) {
                    tracker->setLine(tracker->line() + mainWindow->spinBoxSpace->value());
                }
                handled = true;
            }
        }
    }

    return handled;
}

void MainWindow::setSong(Song *song)
{
    this->song = song;

    setSection(player->section());
    setPlayseq(player->playseq());
    setPosition(player->position());
    setBlock(player->block());
    setCommandPage(mainWindow->trackerMain->commandPage());
    setMode(player->mode());
    setTime(0);
}

void MainWindow::setSection(unsigned int section)
{
    mainWindow->labelSection->setText(QString("Section %1/%2").arg(section + 1).arg(song->sections()));
}

void MainWindow::setPlayseq(unsigned int playseq)
{
    mainWindow->labelPlayingSequence->setText(QString("Playing Sequence %1/%2").arg(playseq + 1).arg(song->playseqs()));
}

void MainWindow::setPosition(unsigned int position)
{
    mainWindow->labelPosition->setText(QString("Position %1/%2").arg(position + 1).arg(song->playseq(player->playseq())->length()));
}

void MainWindow::setBlock(unsigned int block)
{
    mainWindow->labelBlock->setText(QString("Block %1/%2").arg(block + 1).arg(song->blocks()));
}

void MainWindow::setCommandPage(unsigned int commandPage)
{
    mainWindow->labelCommandPage->setText(QString("Command Page %1/%2").arg(commandPage + 1).arg(song->block(player->block())->commandPages()));
}

void MainWindow::setMode(Player::Mode mode)
{
    switch (mode) {
    case Player::PLAY_SONG:
        mainWindow->labelStatus->setText(QString("Playing song"));
        break;
    case Player::PLAY_BLOCK:
        mainWindow->labelStatus->setText(QString("Playing block"));
        break;
    default:
        mainWindow->labelStatus->setText(QString("Stopped"));
        break;
    }
}

void MainWindow::setTime(unsigned int time)
{
    mainWindow->labelTimer->setText(QString("%1:%2").arg(time / 60, 2, 10, QChar('0')).arg(time % 60, 2, 10, QChar('0')));
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "About Tutka", "Tutka 2.0 alpha (C) 2012 Vesa Halttunen <vesuri@jormas.com>");
}
