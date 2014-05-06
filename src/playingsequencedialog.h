/*
 * playingsequencedialog.h
 *
 * Copyright 2002-2014 Vesa Halttunen
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

#ifndef PLAYINGSEQUENCEDIALOG_H
#define PLAYINGSEQUENCEDIALOG_H

#include <QDialog>
#include <QItemSelection>

namespace Ui {
    class PlayingSequenceDialog;
}

class Song;
class Playseq;
class PlayingSequenceTableModel;
class SpinBoxDelegate;

class PlayingSequenceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlayingSequenceDialog(QWidget *parent = 0);
    ~PlayingSequenceDialog();

signals:
    void positionSelected(int position);

public slots:
    void makeVisible();
    void setSong(Song *song);
    void setPlayseq(unsigned int playseq);
    void setPosition(unsigned int position);

private slots:
    void insertBlock();
    void appendBlock();
    void deleteBlock();
    void setSelection();
    void setPosition(const QItemSelection &selected = QItemSelection(), const QItemSelection &deselected = QItemSelection());
    void setWindowTitle();
    void setDeleteButtonVisibility();

private:
    Ui::PlayingSequenceDialog *ui;
    Song *song;
    Playseq *playseq;
    PlayingSequenceTableModel *playingSequenceTableModel;
    SpinBoxDelegate *spinBoxDelegate;
    int playseqNumber;
    int position;
};

#endif // PLAYINGSEQUENCEDIALOG_H
