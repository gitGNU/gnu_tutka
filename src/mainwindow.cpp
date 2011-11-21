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

#include <QKeyEvent>
#include "editor.h"
#include "song.h"
#include "ui_tutka.h"
#include "mainwindow.h"

MainWindow::MainWindow(Editor *editor, QWidget *parent) :
    QMainWindow(parent),
    editor(editor),
    mainWindow(new Ui::MainWindow)
{
    mainWindow->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete mainWindow;
}

void MainWindow::refreshAll()
{
    mainWindow->trackerMain->reset();
    mainWindow->trackerMain->setSong(editor->song());
    mainWindow->trackerMain->setPattern(editor->song()->block(0));
    mainWindow->trackerMain->setNumChannels(editor->song()->block(0)->tracks());
}

#ifdef TODO
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    //    if (GTK_IS_ENTRY(GTK_WINDOW(widget)->focus_widget))
    //      return handled;

    bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
    bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
    bool alt = (event->modifiers() & Qt::AltModifier) != 0;
    bool handled = false;

    if (ctrl) {
        switch (event->key()) {
        case 'b':
            /* CTRL-B: Selection mode on/off */
            if (mainWindow->trackerMain->isInSelectionMode()) {
                mainWindow->trackerMain->markSelection(false);
            } else {
                mainWindow->trackerMain->markSelection(true);
            }
            handled = true;
            break;
        case 'k':
            /* CTRL-K: Clear until the end of the track */
            //editor_song_block_current_clear(editor, mainWindow->trackerMain->cursor_ch, mainWindow->trackerMain->patpos, mainWindow->trackerMain->cursor_ch, editor_song_block_current_length_get(editor) - 1);
            mainWindow->trackerMain->redraw();
            handled = true;
            break;
        case Qt::Key_Left:
            /* CTRL-Left: Previous instrument */
            if (editor->instrument > 0) {
                editor->instrument--;

                /* Make sure the instrument exists */
                editor_song_instrument_current_check( editor);
                gui_instrument_refresh( gui);
            }
            handled = true;
            break;
        case Qt::Key_Right:
            /* CTRL-Right: Previous instrument */
            editor->instrument++;

            /* Make sure the instrument exists */
            editor_song_instrument_current_check( editor);
            gui_instrument_refresh( gui);

            handled = true;
            break;
        case Qt::Key_Tab:
            /* CTRL-Shift-Tab: Next command page */
            editor_set_commandpage(editor, editor->cmdpage + 1);
            gui_info_refresh(gui);
            handled = true;
            break;
        case Qt::Key_Delete: {
            if (widget == gmainWindow->window_main) {
                int i, n = editor_song_block_current_commandpages_get(editor);

                /* Delete command and refresh */
                for (i = 0; i < n; i++) {
//                    editor_song_block_current_command_full_set(editor, editor->line, mainWindow->trackerMain->cursor_ch, i, 0, 0);
                }
//                mainWindow->trackerMain->redrawRow(editor->line);

//                editor_set_line(editor, editor->line + editor->space);

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
                if (event->key() >= '1' && event->key() <= '9')
                    channel = event->key() - '1';
                if (channel < editor_song_maxtracks_get(editor)) {
                    editor_song_track_solo_set(editor, channel, editor_song_track_solo_get(editor, channel) ? 0 : 1);
                    gui_trackvolumes_refresh(gui);
                }
            } else {
                /* CTRL+0-9: Set space value */
                GtkSpinButton *spinbutton = GTK_SPIN_BUTTON(glade_xml_get_widget(gmainWindow->xml, "spinbutton_space"));

                editor->space = event->key() - '0';

                g_signal_handlers_block_by_func(spinbutton, on_spinbutton_space_changed, gui);
                gtk_spin_button_set_value(spinbutton, editor->space);
                g_signal_handlers_unblock_by_func(spinbutton, on_spinbutton_space_changed, gui);

                handled = true;
                break;
            }
        }
        }
    } else if (shift) {
        switch (event->key()) {
        case Qt::Key_Left:
            /* Shift-Left: Next position */
            if (editor->position > 0)
                editor_set_position(editor, editor->position - 1);
            handled = true;
            break;
        case Qt::Key_Right:
            /* Shift-Right: Previous position */
            if (editor->position < editor_song_playseq_current_length_get(editor) - 1)
                editor_set_position(editor, editor->position + 1);
            handled = true;
            break;
        case Qt::Key_Tab:
            /* Shift-Tab: Previous track - only in main window */
            if (widget == gmainWindow->window_main) {
                if (mainWindow->trackerMain->cursor_item > 0) {
                    mainWindow->trackerMain->cursor_item = 0;
                    mainWindow->trackerMain->stepCursorChannel(0);
                } else {
                    mainWindow->trackerMain->stepCursorChannel(-1);
                }
                handled = true;
            }
            break;
        case Qt::Key_Delete: {
            if (widget == gmainWindow->window_main) {
                int i, n = editor_song_block_current_commandpages_get(editor);

                /* Delete note+command and refresh */
                editor_song_block_current_note_set(editor, editor->line, mainWindow->trackerMain->cursor_ch, 0, 0, 0);
                for (i = 0; i < n; i++) {
                    editor_song_block_current_command_full_set(editor, editor->line, mainWindow->trackerMain->cursor_ch, i, 0, 0);
                }
                mainWindow->trackerMain->redrawRow(editor->line);

                editor_set_line(editor, editor->line + editor->space);

                handled = true;
            }
            break;
        }
        case Qt::Key_BackSpace:
            /* Shift-Backspace: Insert row */
            if (widget == gmainWindow->window_main) {
                if (editor->edit) {
                    if (alt) {
                        editor_song_block_current_insert_line_track_all( editor);
                    } else {
                        editor_song_block_current_insert_line(editor, mainWindow->trackerMain->cursor_ch);
                    }
                    mainWindow->trackerMain->redraw();
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
            editor_set_block(editor, editor->block - 1);
            handled = true;
            break;
        case Qt::Key_Right:
            /* Alt-Right: Next block */
            editor_set_block(editor, editor->block + 1);
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
            if (channel < editor_song_maxtracks_get(editor)) {
                editor_song_track_mute_set(editor, channel, editor_song_track_mute_get(editor, channel) ? 0 : 1);
                gui_trackvolumes_refresh( gui);
            }
            break;
        }
        case Qt::Key_BackSpace:
            /* Backspace: delete line */
            if (widget == gmainWindow->window_main) {
                editor_song_block_current_delete_line_track_all( editor);
                mainWindow->trackerMain->redraw();
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
        if (mainWindow->trackerMain->cursor_item == 0) {
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
                if (editor->chord) {
                    bool found = false;
                    int i;

                    /* Chord mode: check if the key has already been pressed (key repeat) */
                    for (i = 0; i < keyboard_num_keys_down; i++)
                        if (keyboard_keys_down[i] == event->key()) {
                            found = true;
                            break;
                        }

                    /* Go to next channel if the current key has not been pressed down yet */
                    if (!found) {
                        /* Add the key to the keys down list */
                        keyboard_num_keys_down++;
                        keyboard_keys_down = realloc(keyboard_keys_down, keyboard_num_keys_down * sizeof(guint));
                        keyboard_keys_down[keyboard_num_keys_down - 1] = event->key();

                        if (editor->edit) {
                            /* Set note and refresh */
                            editor_song_block_current_note_set(editor, editor->line, mainWindow->trackerMain->cursor_ch, editor->octave, data, editor->instrument + 1);
                            mainWindow->trackerMain->redrawRow(editor->line);
                        }

                        mainWindow->trackerMain->stepCursorChannel(1);
                        editor->chordstatus++;
                    }
                } else if (editor->edit) {
                    /* Set note and refresh */
                    editor_song_block_current_note_set(editor, editor->line, mainWindow->trackerMain->cursor_ch, editor->octave, data, editor->instrument + 1);
                    mainWindow->trackerMain->redrawRow(editor->line);
                    editor_set_line(editor, editor->line + editor->space);
                }
                handled = true;
            }
        } else if (editor->edit) {
            switch (mainWindow->trackerMain->cursor_item) {
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
                    int ins = editor_song_block_current_instrument_get(editor, editor->line, mainWindow->trackerMain->cursor_ch);
                    if (mainWindow->trackerMain->cursor_item == 1) {
                        ins = (ins & 0x0f) | (data << 4);
                    } else {
                        ins = (ins & 0xf0) | data;
                    }
                    editor_song_block_current_instrument_set(editor, editor->line, mainWindow->trackerMain->cursor_ch, ins);
                    mainWindow->trackerMain->redrawRow(editor->line);
                    editor_set_line(editor, editor->line + editor->space);
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
                    editor_song_block_current_command_set(editor, editor->line, mainWindow->trackerMain->cursor_ch, editor->cmdpage, mainWindow->trackerMain->cursor_item - 3, data);
                    mainWindow->trackerMain->redrawRow(editor->line);
                    editor_set_line(editor, editor->line + editor->space);
                    handled = true;
                }
                break;
            }
        }

        switch (event->key()) {
        case Qt::Key_Shift_R:
            /* Right shift: Play block */
            editor_player_start(editor, MODE_PLAY_BLOCK, 0);
            handled = true;
            break;
        case Qt::Key_Control_R:
            /* Right Control: Play song */
            editor_player_start(editor, MODE_PLAY_SONG, 0);
            handled = true;
            break;
        case Qt::Key_space:
            /* If the song is playing, stop */
            if (editor_player_mode_get(editor) != MODE_IDLE)
                editor_player_stop( editor);
            else {
                /* Otherwise toggle edit mode */
                GtkToggleButton *togglebutton = GTK_TOGGLE_BUTTON(glade_xml_get_widget(gmainWindow->xml, "checkbutton_edit"));

                editor->edit ^= 1;

                g_signal_handlers_block_by_func(togglebutton, on_checkbutton_edit_toggled, gui);
                gtk_toggle_button_set_active(togglebutton, editor->edit);
                g_signal_handlers_unblock_by_func(togglebutton, on_checkbutton_edit_toggled, gui);
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
            GtkComboBox *combobox = GTK_COMBO_BOX(glade_xml_get_widget(gmainWindow->xml, "combobox_keyboard"));

            editor->octave = event->key() - Qt::Key_F1;

            g_signal_handlers_block_by_func(combobox, on_combobox_keyboard_changed, gui);
            gtk_combo_box_set_active(combobox, event->key() - Qt::Key_F1);
            g_signal_handlers_unblock_by_func(combobox, on_combobox_keyboard_changed, gui);

            handled = true;
            break;
        }
        case Qt::Key_Num_Lock:
            instrument = 0;
            break;
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
            if (widget == gmainWindow->window_main) {
                editor_set_line(editor, editor->line + 1);
                handled = true;
            }
            break;
        case Qt::Key_Up:
            /* Up: Go up */
            if (widget == gmainWindow->window_main) {
                editor_set_line(editor, editor->line - 1);
                handled = true;
            }
            break;
        case Qt::Key_Left:
            /* Left: Go left */
            if (widget == gmainWindow->window_main) {
                mainWindow->trackerMain->stepCursorItem(-1);
                handled = true;
            }
            break;
        case Qt::Key_Right:
            /* Right: Go right */
            if (widget == gmainWindow->window_main) {
                mainWindow->trackerMain->stepCursorItem(1);
                handled = true;
            }
            break;
        case Qt::Key_Tab:
            /* Tab: Next track */
            if (widget == gmainWindow->window_main) {
                mainWindow->trackerMain->cursor_item = 0;
                mainWindow->trackerMain->stepCursorChannel(1);
                handled = true;
            }
            break;
        case Qt::Key_Home:
            /* End: Go to the beginning of block */
            if (widget == gmainWindow->window_main) {
                editor_set_line(editor, 0);
                handled = true;
            }
            break;
        case Qt::Key_End:
            /* End: Go to the end of block */
            if (widget == gmainWindow->window_main) {
                editor_set_line(editor, editor_song_block_current_length_get(editor) - 1);
                handled = true;
            }
            break;
        case Qt::Key_Page_Down:
            /* Page down: Go down 8 lines */
            if (widget == gmainWindow->window_main) {
                editor_set_line(editor, editor->line + 8);
                handled = true;
            }
            break;
        case Qt::Key_Page_Up:
            /* Page up: Go up 8 lines */
            if (widget == gmainWindow->window_main) {
                editor_set_line(editor, editor->line - 8);
                handled = true;
            }
            break;
        case Qt::Key_BackSpace:
            /* Backspace: delete line */
            if (widget == gmainWindow->window_main) {
                editor_song_block_current_delete_line(editor, mainWindow->trackerMain->cursor_ch);
                mainWindow->trackerMain->redraw();
                handled = true;
            }
            break;
        }

        /* Select an instrument if an instrument selection key was pressed */
        if (instrument != -1) {
            editor->instrument = instrument;

            /* Make sure the instrument exists */
            editor_song_instrument_current_check( editor);
            gui_instrument_refresh( gui);
            handled = true;
        }

        /* Play note if a key was pressed but not if cursor is in cmd pos */
        if (note != -1 && mainWindow->trackerMain->cursor_item == 0) {
            editor_player_play_note(editor, note, mainWindow->trackerMain->cursor_ch);
            handled = true;
        }
    }

    if (handled) {
        event->accept();
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent * event)
{
    /* Key has been released */
    if (!ctrl && !shift) {
        if (editor->chord && mainWindow->trackerMain->cursor_item == 0) {
            int i;

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
                for (i = 0; i < keyboard_num_keys_down; i++)
                    if (keyboard_keys_down[i] == event->key()) {
                        while (i < keyboard_num_keys_down - 1) {
                            keyboard_keys_down[i] = keyboard_keys_down[i + 1];
                            i++;
                        }
                        keyboard_num_keys_down--;
                        keyboard_keys_down = realloc(keyboard_keys_down, keyboard_num_keys_down * sizeof(guint));
                        break;
                    }

                /* Go to the previous channel */
                mainWindow->trackerMain->stepCursorChannel(-1);
                editor->chordstatus--;

                /* If all chord notes have been released go to the next line */
                if (editor->chordstatus == 0 && editor->edit) {
                    editor_set_line(editor, editor->line + editor->space);
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
#endif
