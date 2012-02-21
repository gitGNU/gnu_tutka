/*
 * mainwindow.h
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

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include "player.h"
#include <QMainWindow>
#include <QHash>

namespace Ui {
    class MainWindow;
}

class InstrumentPropertiesDialog;
class QFileDialog;
class PreferencesDialog;
class TrackVolumesDialog;
class TransposeDialog;
class ExpandShrinkDialog;
class ChangeInstrumentDialog;
class SectionListDialog;
class SongPropertiesDialog;
class PlayingSequenceDialog;
class PlayingSequenceListDialog;
class BlockListDialog;
class MessageListDialog;
class Song;
class Block;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Player *player, QWidget *parent = 0);
    virtual ~MainWindow();

    virtual bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void setSong(Song *song);
    void setSection(unsigned int section);
    void setPlayseq(unsigned int playseq);
    void setPosition(unsigned int position);
    void setBlock(unsigned int block);
    void setCommandPage(unsigned int commandPage);
    void setMode(Player::Mode mode);
    void setTime(unsigned int time);
    void setInstrument(int instrument);
    void setSelection(int startTrack, int startLine, int endTrack, int endLine);
    void showAbout();
    void cutSelection();
    void copySelection();
    void pasteSelection();
    void clearSelection();
    void cutBlock();
    void copyBlock();
    void pasteBlock();
    void clearBlock();
    void selectAllBlock();
    void cutTrack();
    void copyTrack();
    void pasteTrack();
    void clearTrack();
    void selectAllTrack();

private:
    bool keyPress(QKeyEvent *event);
    bool keyRelease(QKeyEvent *event);

    Player *player;
    Ui::MainWindow *ui;
    InstrumentPropertiesDialog *instrumentPropertiesDialog;
    QFileDialog *openDialog;
    QFileDialog *saveDialog;
    PreferencesDialog *preferencesDialog;
    TrackVolumesDialog *trackVolumesDialog;
    TransposeDialog *transposeDialog;
    ExpandShrinkDialog *expandShrinkDialog;
    ChangeInstrumentDialog *changeInstrumentDialog;
    SectionListDialog *sectionListDialog;
    SongPropertiesDialog *songPropertiesDialog;
    PlayingSequenceDialog *playingSequenceDialog;
    PlayingSequenceListDialog *playingSequenceListDialog;
    BlockListDialog *blockListDialog;
    MessageListDialog *messageListDialog;
    Song *song;
    Block *copySelection_;
    Block *copyBlock_;
    Block *copyTrack_;
    QList<int> keyboardKeysDown;
    int chordStatus;
    unsigned int block;
    int instrument;
    int selectionStartTrack;
    int selectionStartLine;
    int selectionEndTrack;
    int selectionEndLine;
    QHash<int, char> keyToNote;
};

#endif /* MAINWINDOW_H_ */
