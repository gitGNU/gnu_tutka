/*
 * gui.cpp
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
#include <QKeyEvent>
#include "player.h"
#include "song.h"
#include "ui_tutka.h"
#include "gui.h"

GUI::GUI(Player *player, QWidget *parent) :
    QMainWindow(parent),
    player(player),
    mainWindow(new Ui::MainWindow),
    chordStatus(0)
{
    mainWindow->setupUi(this);

    connect(player, SIGNAL(songChanged(Song *)), mainWindow->trackerMain, SLOT(setSong(Song *)));
    connect(player, SIGNAL(blockChanged(Block *)), mainWindow->trackerMain, SLOT(setPattern(Block *)));
    connect(player, SIGNAL(lineChanged(unsigned int)), mainWindow->trackerMain, SLOT(setLine(unsigned int)));
    connect(mainWindow->buttonPlaySong, SIGNAL(clicked()), player, SLOT(playSong()));
    connect(mainWindow->buttonPlayBlock, SIGNAL(clicked()), player, SLOT(playBlock()));
    connect(mainWindow->buttonContinueSong, SIGNAL(clicked()), player, SLOT(continueSong()));
    connect(mainWindow->buttonContinueBlock, SIGNAL(clicked()), player, SLOT(continueBlock()));
    connect(mainWindow->buttonStop, SIGNAL(clicked()), player, SLOT(stop()));
}

GUI::~GUI()
{
    delete mainWindow;
}

void GUI::keyPressEvent(QKeyEvent *event)
{
    if (dynamic_cast<QLineEdit *>(QApplication::focusWidget()) != NULL) {
        return;
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
            if (mainWindow->spinBoxInstrument->value() > 0) {
                mainWindow->spinBoxInstrument->setValue(mainWindow->spinBoxInstrument->value() - 1);

                /* Make sure the instrument exists */
                song->checkInstrument(mainWindow->spinBoxInstrument->value(), 0);
            }
            handled = true;
            break;
        case Qt::Key_Right:
            /* CTRL-Right: Previous instrument */
            mainWindow->spinBoxInstrument->setValue(mainWindow->spinBoxInstrument->value() + 1);

            /* Make sure the instrument exists */
            song->checkInstrument(mainWindow->spinBoxInstrument->value(), 0);

            handled = true;
            break;
        case Qt::Key_Tab:
            /* CTRL-Shift-Tab: Next command page */
            tracker->setCommandPage(tracker->commandPage() + 1);
            handled = true;
            break;
        case Qt::Key_Delete: {
            if (QApplication::focusWidget() == this) {
                /* Delete command and refresh */
                for (int i = 0; i < block->commandPages(); i++) {
                    block->setCommandFull(tracker->line(), tracker->track(), i, 0, 0);
                }
                tracker->redrawRow(tracker->line());

                player->setLine(player->line() + mainWindow->spinBoxSpace->value());

                handled = true;
            }
            break;
        }
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            if (alt) {
                /* CTRL+Alt+0-9: Solo tracks */
                int channel = 9;
                if (event->key() >= '1' && event->key() <= '9') {
                    channel = event->key() - '1';
                }
                if (channel < song->maxTracks()) {
                    song->track(channel)->setSolo(!song->track(channel)->isSolo());
                }
            } else {
                /* CTRL+0-9: Set space value */
                // TODO
                mainWindow->spinBoxSpace->blockSignals(true);
                mainWindow->spinBoxSpace->setValue(event->key() - '0');
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
        case Qt::Key_Tab:
            /* Shift-Tab: Previous track - only in main window */
            if (QApplication::focusWidget() == this) {
                tracker->stepCursorChannel(-1);
                handled = true;
            }
            break;
        case Qt::Key_Delete: {
            if (QApplication::focusWidget() == this) {
                int i, n = block->commandPages();

                /* Delete note+command and refresh */
                block->setNote(tracker->line(), tracker->cursorChannel(), 0, 0, 0);
                for (i = 0; i < n; i++) {
                    block->setCommandFull(tracker->line(), tracker->cursorChannel(), i, 0, 0);
                }
                tracker->redrawRow(tracker->line());

                player->setLine(player->line() + mainWindow->spinBoxSpace->value());

                handled = true;
            }
            break;
        }
        case Qt::Key_Backspace:
            /* Shift-Backspace: Insert row */
            if (QApplication::focusWidget() == this) {
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
            if (event->key() >= '1' && event->key() <= '9')
                channel = event->key() - '1';
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
            switch (event->key()) {
            case 'z':
                data = 1;
                break;
            case 's':
                data = 2;
                break;
            case 'x':
                data = 3;
                break;
            case 'd':
                data = 4;
                break;
            case 'c':
                data = 5;
                break;
            case 'v':
                data = 6;
                break;
            case 'g':
                data = 7;
                break;
            case 'b':
                data = 8;
                break;
            case 'h':
                data = 9;
                break;
            case 'n':
                data = 10;
                break;
            case 'j':
                data = 11;
                break;
            case 'm':
                data = 12;
                break;
            case 'q':
            case ',':
                data = 13;
                break;
            case '2':
            case 'l':
                data = 14;
                break;
            case 'w':
            case '.':
                data = 15;
                break;
            case '3':
            case 0xf4: /* oumlaut */
                data = 16;
                break;
            case 'e':
            case '-':
                data = 17;
                break;
            case 'r':
                data = 18;
                break;
            case '5':
                data = 19;
                break;
            case 't':
                data = 20;
                break;
            case '6':
                data = 21;
                break;
            case 'y':
                data = 22;
                break;
            case '7':
                data = 23;
                break;
            case 'u':
                data = 24;
                break;
            case 'i':
                data = 25;
                break;
            case '9':
                data = 26;
                break;
            case 'o':
                data = 27;
                break;
            case '0':
                data = 28;
                break;
            case 'p':
                data = 29;
                break;
            case 229:
                data = 30;
                break;
            case 180:
                data = 31;
                break;
            case Qt::Key_Delete:
                data = 0;
                break;
            }

            if (data != -1) {
                if (mainWindow->checkBoxChord->isChecked()) {

                    /* Chord mode: check if the key has already been pressed (key repeat) */
                    bool found = keyboardKeysDown.contains(event->key());

                    /* Go to next channel if the current key has not been pressed down yet */
                    if (!found) {
                        /* Add the key to the keys down list */
                        keyboardKeysDown.append(event->key());

                        if (mainWindow->checkBoxEdit->isChecked()) {
                            /* Set note and refresh */
                            block->setNote(tracker->line(), tracker->cursorChannel(), mainWindow->comboBoxKeyboardOctaves->currentIndex(), data, mainWindow->spinBoxInstrument->value() + 1);
                            tracker->redrawRow(tracker->line());
                        }

                        tracker->stepCursorChannel(1);
                        chordStatus++;
                    }
                } else if (mainWindow->checkBoxEdit->isChecked()) {
                    /* Set note and refresh */
                    block->setNote(tracker->line(), tracker->cursorChannel(), mainWindow->comboBoxKeyboardOctaves->currentIndex(), data, mainWindow->spinBoxInstrument->value() + 1);
                    tracker->redrawRow(tracker->line());
                    player->setLine(player->line() + mainWindow->spinBoxSpace->value());
                }
                handled = true;
            }
        } else if (mainWindow->checkBoxEdit->isChecked()) {
            switch (tracker->cursorItem()) {
            case 1:
            case 2:
                /* Editing instrument */
                if (event->key() >= '0' && event->key() <= '9')
                    data = event->key() - '0';
                else if (event->key() >= 'a' && event->key() <= 'f')
                    data = 10 + event->key() - 'a';
                else if (event->key() == Qt::Key_Delete)
                    data = 0;

                if (data != -1) {
                    /* Set instrument and refresh */
                    int ins = block->instrument(tracker->line(), tracker->cursorChannel());
                    if (tracker->cursorItem() == 1) {
                        ins = (ins & 0x0f) | (data << 4);
                    } else {
                        ins = (ins & 0xf0) | data;
                    }
                    block->setInstrument(tracker->line(), tracker->cursorChannel(), ins);
                    tracker->redrawRow(tracker->line());
                    player->setLine(player->line() + mainWindow->spinBoxSpace->value());
                    handled = true;
                }
                break;
            case 3:
            case 4:
            case 5:
            case 6:
                /* Editing effects */
                if (event->key() >= '0' && event->key() <= '9')
                    data = event->key() - '0';
                else if (event->key() >= 'a' && event->key() <= 'f')
                    data = 10 + event->key() - 'a';
                else if (event->key() == Qt::Key_Delete)
                    data = 0;

                if (data != -1) {
                    /* Set effect and refresh */
                    block->setCommand(tracker->line(), tracker->cursorChannel(), tracker->commandPage(), tracker->cursorItem() - 3, data);
                    tracker->redrawRow(tracker->line());
                    player->setLine(player->line() + mainWindow->spinBoxSpace->value());
                    handled = true;
                }
                break;
            }
        }

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
        case 'z':
            note = 0;
            break;
        case 's':
            note = 1;
            break;
        case 'x':
            note = 2;
            break;
        case 'd':
            note = 3;
            break;
        case 'c':
            note = 4;
            break;
        case 'v':
            note = 5;
            break;
        case 'g':
            note = 6;
            break;
        case 'b':
            note = 7;
            break;
        case 'h':
            note = 8;
            break;
        case 'n':
            note = 9;
            break;
        case 'j':
            note = 10;
            break;
        case 'm':
            note = 11;
            break;
        case 'q':
        case ',':
            note = 12;
            break;
        case '2':
        case 'l':
            note = 13;
            break;
        case 'w':
        case '.':
            note = 14;
            break;
        case '3':
        case 0xf4: /* oumlaut */
            note = 15;
            break;
        case 'e':
        case '-':
            note = 16;
            break;
        case 'r':
            note = 17;
            break;
        case '5':
            note = 18;
            break;
        case 't':
            note = 19;
            break;
        case '6':
            note = 20;
            break;
        case 'y':
            note = 21;
            break;
        case '7':
            note = 22;
            break;
        case 'u':
            note = 23;
            break;
        case 'i':
            note = 24;
            break;
        case '9':
            note = 25;
            break;
        case 'o':
            note = 26;
            break;
        case '0':
            note = 27;
            break;
        case 'p':
            note = 28;
            break;
        case 229:
            note = 29;
            break;
        case 180:
            note = 30;
            break;
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
        if (instrument != -1) {
            mainWindow->spinBoxInstrument->setValue(instrument);

            /* Make sure the instrument exists */
            song->checkInstrument(mainWindow->spinBoxInstrument->value(), 0);
            handled = true;
        }

        /* Play note if a key was pressed but not if cursor is in cmd pos */
        if (note != -1 && tracker->cursorItem() == 0) {
            player->playNote(mainWindow->spinBoxInstrument->value(), mainWindow->comboBoxKeyboardOctaves->currentIndex() * 12 + note, 127, tracker->cursorChannel());
            handled = true;
        }
    }

    if (handled) {
        event->accept();
    }
}

void GUI::keyReleaseEvent(QKeyEvent * event)
{
    Tracker *tracker = mainWindow->trackerMain;
    bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
    bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
    bool handled = false;

    /* Key has been released */
    if (!ctrl && !shift) {
        if (mainWindow->checkBoxChord->isChecked() && tracker->cursorItem() == 0) {
            switch (event->key()) {
            case 'z':
            case 's':
            case 'x':
            case 'd':
            case 'c':
            case 'v':
            case 'g':
            case 'b':
            case 'h':
            case 'n':
            case 'j':
            case 'm':
            case 'q':
            case ',':
            case '2':
            case 'l':
            case 'w':
            case '.':
            case '3':
            case 0xf4: /* oumlaut */
            case 'e':
            case '-':
            case 'r':
            case '5':
            case 't':
            case '6':
            case 'y':
            case '7':
            case 'u':
            case 'i':
            case '9':
            case 'o':
            case '0':
            case 'p':
            case 229:
            case 180:
                /* Find the key from the keys down list and remove it */
                keyboardKeysDown.removeAll(event->key());

                /* Go to the previous channel */
                tracker->stepCursorChannel(-1);
                chordStatus--;

                /* If all chord notes have been released go to the next line */
                if (chordStatus == 0 && mainWindow->checkBoxEdit->isChecked()) {
                    player->setLine(player->line() + mainWindow->spinBoxSpace->value());
                }
                handled = true;
                break;
            }
        }
    }

    if (handled) {
        event->accept();
    }
}
