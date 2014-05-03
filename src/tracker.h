/*
 * tracker.h
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

#ifndef TRACKER_H
#define TRACKER_H

#include <QPixmap>
#include <QWidget>

class Song;
class Block;

#define TRACKER_TRACK_WIDTH 13

class Tracker : public QWidget
{
    Q_OBJECT

public:
    explicit Tracker(QWidget *parent = 0);
    void setCommandPage(int commandPage);
    bool isInSelectionMode();
    void markSelection(bool enable);
    void stepCursorTrack(int direction);
    void reset();
    Song *song() const;
    Block *block() const;
    int track() const;
    int line() const;
    int commandPage() const;
    int cursorTrack() const;
    int cursorItem() const;
    void stepCursorItem(int direction);
    void stepCursorLine(int direction);
    void setCursorItem(int cursorItem);
    void clearMarkSelection();
    void setSelection(int startTrack, int startLine, int endTrack, int endLine);

public slots:
    void setSong(Song *song);
    void setBlock(unsigned int block);
    void setLine(int line);
    void setLeftmostTrack(int leftmostTrack);

private slots:
    void redrawArea(int startTrack, int startLine, int endTrack, int endLine);
    void setTracks(int tracks);
    void checkBounds();
    void queueDraw();

signals:
    void lineChanged(int line, int length, int visibleLines);
    void trackChanged(int track, int tracks, int visibleTracks);
    void cursorTrackChanged(int track);
    void selectionChanged(int startTrack, int startLine, int endTrack, int endLine);
    void commandPageChanged(int commandPage);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void paintEvent(QPaintEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void showEvent(QShowEvent *event);

private:
    enum Color {
        ColorBackground,
        ColorBackgroundCursor,
        ColorBackgroundSelection,
        ColorNotes,
        ColorBars,
        ColorTrackHeader,
        ColorTrackHeaderMute,
        ColorTrackHeaderSolo,
        ColorTrackHeaderMuteSolo,
        ColorCursor,
        ColorLast
    };

    void setVisibleArea();
    void noteToString(unsigned char note, unsigned char instrument, unsigned char effect, unsigned char value, char *buf);
    void clearNotesLine(int y, int line);
    void printNotesLine(int y, int track, int tracks, int line, int cursor);
    void printNotes(int x, int y, int width, int height, int cursorLine, bool enableCursor);
    void printBars();
    void printTrackHeaders();
    void printCursor();
    void drawClever(const QRect &area);
    void drawStupid(const QRect &area);
    void initDisplay(int width, int height);
    void initColors();
    void calculateFontSize();
    bool setFont(const QString &fontname);
    // If selecting, mouse is used to select in pattern
    void mouseToCursorPos(int x, int y, int *cursorTrack, int *cursorItem, int *line);

    int visibleLines;
    int startY;
    int visibleTracks;
    int startX;
    int trackWidth;
    int cursorLine;

    QFont font;
    int fontWidth;
    int fontHeight;
    int fontAscent;

    QBrush backgroundBrush;
    QBrush backgroundCursorBrush;
    QBrush notesBrush;
    QBrush miscellaneousBrush;
    QColor colors[ColorLast];
    QPixmap *pixmap;

    Song *song_;
    Block *block_;
    int commandPage_;
    int line_;
    int oldLine;
    int tracks;

    int cursorTrack_;
    int cursorItem_;
    int leftmostTrack;

    // Block selection stuff
    bool inSelectionMode;
    int selectionStartTrack;
    int selectionStartLine;
    int selectionEndTrack;
    int selectionEndLine;

    bool mouseSelecting;
    Qt::MouseButton mouseButton;
};

#endif // TRACKER_H
