/*
 * expandshrinkdialog.h
 *
 * Copyright 2002-2012 Vesa Halttunen
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

#ifndef EXPANDSHRINKDIALOG_H
#define EXPANDSHRINKDIALOG_H

#include <QDialog>

namespace Ui {
    class ExpandShrinkDialog;
}

class Song;

class ExpandShrinkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExpandShrinkDialog(QWidget *parent = NULL);
    ~ExpandShrinkDialog();

public slots:
    void makeVisible();
    void showSong();
    void showBlock();
    void showTrack();
    void showSelection();
    void setSong(Song *song);
    void setBlock(unsigned int block);
    void setTrack(int track);
    void setSelection(int startTrack, int startLine, int endTrack, int endLine);

private slots:
    void expand();
    void shrink();

private:
    enum Area {
        AreaSong,
        AreaBlock,
        AreaTrack,
        AreaSelection
    };

    Ui::ExpandShrinkDialog *ui;
    Song *song;
    unsigned int block;
    unsigned int track;
    int selectionStartTrack;
    int selectionStartLine;
    int selectionEndTrack;
    int selectionEndLine;

    void expandShrink(bool shrink);
};

#endif // EXPANDSHRINKDIALOG_H
