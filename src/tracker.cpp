/*
 * tracker.cpp
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

#include <QPainter>
#include <QPaintEvent>
#include "song.h"
#include "track.h"
#include "block.h"
#include "tracker.h"

QHash<int, char> Tracker::keyToNote;

Tracker::Tracker(QWidget *parent) :
    QWidget(parent),
    visibleLines(0),
    startY(0),
    visibleTracks(0),
    startX(0),
    trackWidth(0),
    cursorLine(0),
#ifdef __APPLE__
    font("Monaco", 11),
#else
    font("Monospace", 11),
#endif
    fontWidth(0),
    fontHeight(0),
    fontAscent(0),
    backgroundBrush(Qt::black),
    backgroundCursorBrush(Qt::yellow),
    notesBrush(Qt::gray),
    miscellaneousBrush(Qt::green),
    pixmap(NULL),
    song_(NULL),
    block_(NULL),
    commandPage_(0),
    line_(0),
    oldLine(-1),
    tracks(0),
    cursorTrack_(0),
    cursorItem_(0),
    leftmostTrack(0),
    inSelectionMode(false),
    selectionStartTrack(-1),
    selectionStartLine(-1),
    selectionEndTrack(-1),
    selectionEndLine(-1),
    mouseSelecting(false),
    mouseButton(Qt::NoButton),
    chordStatus(0),
    instrument_(0),
    octave_(3),
    inEditMode(false),
    inChordMode(false)
{
    font.setStyleHint(QFont::TypeWriter);
    font.setStyleStrategy(QFont::ForceIntegerMetrics);
    calculateFontSize();
    initColors();

    if (keyToNote.isEmpty()) {
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

    backgroundBrush = colors[ColorBackground];
    backgroundCursorBrush = colors[ColorBackgroundCursor];
    notesBrush = colors[ColorNotes];
    miscellaneousBrush = colors[ColorBackgroundSelection];

    initDisplay(geometry().width(), geometry().height());
}

void Tracker::setTracks(int tracks)
{
    if (this->tracks != tracks) {
        this->tracks = tracks;

        // Make sure the cursor is inside the tracker
        if (cursorTrack_ >= tracks) {
            cursorTrack_ = tracks - 1;
        }

        initDisplay(geometry().width(), geometry().height());
        queueDraw();

        emit trackChanged(leftmostTrack, tracks, visibleTracks);
    }
}

void Tracker::setCommandPage(int commandPage)
{
    if (commandPage < 0) {
        commandPage = block_->commandPages() - 1;
    } else if (commandPage >= block_->commandPages()) {
        commandPage = 0;
    }

    if (commandPage_ != commandPage) {
        commandPage_ = commandPage;
        emit commandPageChanged(commandPage_);

        queueDraw();
    }
}

void Tracker::setLine(int line)
{
    if (block_ == NULL) {
        return;
    }

    line %= block_->length();

    if (line_ != line) {
        line_ = line;
        emit lineChanged(line, block_->length(), visibleLines);

        queueDraw();
    }
}

void Tracker::setSong(Song *song)
{
    if (song_ != NULL) {
        disconnect(song_, SIGNAL(trackMutedOrSoloed()), this, SLOT(queueDraw()));
        disconnect(song_, SIGNAL(trackNameChanged()), this, SLOT(queueDraw()));
    }

    song_ = song;

    if (song_ != NULL) {
        connect(song_, SIGNAL(trackMutedOrSoloed()), this, SLOT(queueDraw()));
        connect(song_, SIGNAL(trackNameChanged()), this, SLOT(queueDraw()));
    }
}

void Tracker::setBlock(Block *block)
{
    if (block_ != block) {
        if (block_ != NULL) {
            disconnect(block_, SIGNAL(areaChanged(int,int,int,int)), this, SLOT(redrawArea(int,int,int,int)));
            disconnect(block_, SIGNAL(tracksChanged(int)), this, SLOT(setTracks(int)));
            disconnect(block_, SIGNAL(lengthChanged(int)), this, SLOT(checkBounds()));
            disconnect(block_, SIGNAL(commandPagesChanged(int)), this, SLOT(checkBounds()));
        }

        block_ = block;
        if (block != NULL) {
            connect(block, SIGNAL(areaChanged(int,int,int,int)), this, SLOT(redrawArea(int,int,int,int)));
            connect(block, SIGNAL(tracksChanged(int)), this, SLOT(setTracks(int)));
            connect(block, SIGNAL(lengthChanged(int)), this, SLOT(checkBounds()));
            connect(block, SIGNAL(commandPagesChanged(int)), this, SLOT(checkBounds()));

            setTracks(block->tracks());

            // Make sure the cursor is inside the tracker
            if (line_ >= block->length()) {
                line_ = block->length() - 1;
            }

            if (cursorTrack_ >= block->tracks()) {
                cursorTrack_ = block->tracks() - 1;
            }

            emit lineChanged(line_, block->length(), visibleLines);

            if (commandPage_ >= block->commandPages()) {
                commandPage_ = block->commandPages() - 1;
                emit commandPageChanged(commandPage_);
            }
        }
        queueDraw();
    }
}

void Tracker::setLeftmostTrack(int leftmostTrack)
{
    if (this->leftmostTrack != leftmostTrack) {
        if (!(leftmostTrack + visibleTracks <= tracks)) {
            return;
        }

        this->leftmostTrack = leftmostTrack;
        queueDraw();

        if (cursorTrack_ < leftmostTrack) {
            cursorTrack_ = leftmostTrack;
        } else if (cursorTrack_ >= leftmostTrack + visibleTracks) {
            cursorTrack_ = leftmostTrack + visibleTracks - 1;
        }

        emit trackChanged(leftmostTrack, tracks, visibleTracks);
    }
}

void Tracker::setVisibleArea()
{
    if (leftmostTrack + visibleTracks > tracks) {
        setLeftmostTrack(tracks - visibleTracks);
    } else if (cursorTrack_ < leftmostTrack) {
        setLeftmostTrack(cursorTrack_);
    } else if (cursorTrack_ >= leftmostTrack + visibleTracks) {
        setLeftmostTrack(cursorTrack_ - visibleTracks + 1);
    }
}

void Tracker::stepCursorItem(int direction)
{
    if (!(direction == -1 || direction == 1)) {
        return;
    }

    cursorItem_ += direction;
    if (cursorItem_ > 6) {
        cursorItem_ %= 7;
        stepCursorTrack(direction);
    } else if (cursorItem_ < 0) {
        cursorItem_ = 0;
        stepCursorTrack(direction);
        cursorItem_ = 6;
    } else {
        setVisibleArea();
        queueDraw();
    }
}

void Tracker::setCursorItem(int cursorItem)
{
    cursorItem_ = cursorItem % 7;
    setVisibleArea();
    queueDraw();
}

void Tracker::stepCursorTrack(int direction)
{
    if (direction < 0 && cursorItem_ > 0) {
        direction = 0;
    }
    cursorItem_ = 0;
    cursorTrack_ += direction;

    if (cursorTrack_ < 0) {
        cursorTrack_ = tracks - 1;
    } else if (cursorTrack_ >= tracks) {
        cursorTrack_ = 0;
    }

    setVisibleArea();
    queueDraw();

    emit cursorTrackChanged(cursorTrack_);
}

void Tracker::stepCursorLine(int direction)
{
    int line = line_ + direction;

    while (line < 0) {
        line += block_->length();
    }
    line %= block_->length();

    setLine(line);
}

void Tracker::markSelection(bool enable)
{
    if (!enable) {
        selectionEndTrack = cursorTrack_;
        selectionEndLine = line_;
        inSelectionMode = false;
    } else {
        selectionStartTrack = selectionEndTrack = cursorTrack_;
        selectionStartLine = selectionEndLine = line_;
        inSelectionMode = true;
        queueDraw();
    }

    emit selectionChanged(selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
}

void Tracker::clearMarkSelection()
{
    if (selectionStartTrack != -1) {
        selectionStartTrack = selectionEndTrack = -1;
        selectionStartLine = selectionEndLine = -1;
        inSelectionMode = false;
        queueDraw();

        emit selectionChanged(selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
    }
}

bool Tracker::isInSelectionMode() const
{
    return inSelectionMode;
}

bool Tracker::isInEditMode() const
{
    return inEditMode;
}

bool Tracker::isInChordMode() const
{
    return inChordMode;
}

void Tracker::noteToString(unsigned char note, unsigned char instrument, unsigned char effect, unsigned char value, char *buffer)
{
    static const char *const noteNames[128] = {
      "---",
      "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
      "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
      "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
      "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
      "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
      "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
      "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7",
      "C-8", "C#8", "D-8", "D#8", "E-8", "F-8", "F#8", "G-8", "G#8", "A-8", "A#8", "B-8",
      "C-9", "C#9", "D-9", "D#9", "E-9", "F-9", "F#9", "G-9", "G#9", "A-9", "A#9", "B-9",
      "C-A", "C#A", "D-A", "D#A", "E-A", "F-A", "F#A", "G-A", "G#A", "A-A", "A#A", "B-A",
      "C-B", "C#B", "D-B", "D#B", "E-B", "F-B", "F#B"
    };

    static const char hexMap[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
        'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
        'W', 'X', 'Y', 'Z'
    };

    buffer[0] = noteNames[note][0];
    buffer[1] = noteNames[note][1];
    buffer[2] = noteNames[note][2];
    buffer[3] = ' ';
    buffer[4] = hexMap[(instrument & 0xf0) >> 4];
    buffer[5] = hexMap[instrument & 0x0f];
    buffer[6] = ' ';
    buffer[7] = hexMap[(effect & 0xf0) >> 4];
    buffer[8] = hexMap[effect & 0x0f];
    buffer[9] = ' ';
    buffer[10] = hexMap[(value & 0xf0) >> 4];
    buffer[11] = hexMap[value & 0x0f];
    buffer[12] = ' ';
    buffer[13] = 0;
}

void Tracker::clearNotesLine(int y, int line)
{
    // cursor line
    QBrush brush = line == line_ ? backgroundCursorBrush : backgroundBrush;

    QPainter painter(pixmap);
    painter.fillRect(0, y, geometry().width(), fontHeight, brush);
}

void Tracker::printNotesLine(int y, int track, int tracks, int line, int cursor)
{
    char buf[TRACKER_TRACK_WIDTH + 1];
    int rbs, rbe, cbs, cbe;

    if (!(track + tracks <= block_->tracks())) {
        return;
    }

    clearNotesLine(y, line);

    // Figure out which rows/columns should be highlighted
    if (inSelectionMode) {
        rbs = selectionStartLine;
        rbe = line_;
        cbs = selectionStartTrack;
        cbe = cursorTrack_;
    } else {
        rbs = selectionStartLine;
        rbe = selectionEndLine;
        cbs = selectionStartTrack;
        cbe = selectionEndTrack;
    }

    QPainter painter(pixmap);
    int fontY = y + fontAscent;

    // The row number
    sprintf(buf, "%03d", line);
    painter.setPen(notesBrush.color());
    painter.setBackground(cursor ? backgroundCursorBrush : backgroundBrush);
    painter.setFont(font);
    painter.drawText(1, fontY, buf);

    // The notes
    int j = 0;
    buf[TRACKER_TRACK_WIDTH] = 0;
    for (tracks += track; track < tracks; track++, j++) {
        noteToString(block_->note(line, track), block_->instrument(line, track), block_->command(line, track, commandPage_), block_->commandValue(line, track, commandPage_), buf);

        QBrush brush;
        if (cursor) {
            brush = backgroundCursorBrush;
        } else if (line >= rbs && line <= rbe && track >= cbs && track <= cbe) {
            brush = miscellaneousBrush;
        }
        painter.fillRect(startX + (j * TRACKER_TRACK_WIDTH) * fontWidth, y, trackWidth, fontHeight, brush);
        painter.drawText(startX + (j * TRACKER_TRACK_WIDTH) * fontWidth, fontY, buf);
    }
}

void Tracker::printNotes(int x, int y, int width, int height, int cursorLine, bool enableCursor)
{
    Q_UNUSED(x)
    Q_UNUSED(width)

    // Limit y and h to the actually used window portion
    int my = y - startY;
    if (my < 0) {
        height += my;
        my = 0;
    }
    if (my + height > fontHeight * visibleLines) {
        height = fontHeight * visibleLines - my;
    }

    // Calculate first and last line to be redrawn
    int firstLine = my / fontHeight;
    int lastLine = (my + height - 1) / fontHeight;

    // Print the notes
    int scry = startY + firstLine * fontHeight;
    for (int line = firstLine; line <= lastLine; line++, scry += fontHeight) {
        int actualLine = line + cursorLine - this->cursorLine;
        if (block_ != NULL && actualLine >= 0 && actualLine < block_->length()) {
            printNotesLine(scry, leftmostTrack, visibleTracks, actualLine, (enableCursor && actualLine == cursorLine) ? 1 : 0);
        } else {
            QPainter painter(pixmap);
            painter.fillRect(0, scry, geometry().width(), fontHeight, backgroundBrush);
        }
    }
}

void Tracker::printBars()
{
    // Draw the separation bars
    QPainter painter(pixmap);
    painter.setPen(colors[ColorBars]);
    int x1 = startX - 3;
    for (int track = 0; track <= visibleTracks; track++, x1 += trackWidth) {
        painter.drawLine(x1, 0, x1, geometry().height() - 1);
    }
}

void Tracker::printTrackHeaders()
{
    int x = startX - 3;

    // Clear top and bottom
    QPainter painter(pixmap);
    painter.fillRect(QRectF(0, 0, geometry().width(), startY), backgroundBrush);
    painter.fillRect(QRectF(0, startY + visibleLines * fontHeight, geometry().width(), geometry().height() - (startY + visibleLines * fontHeight)), backgroundBrush);

    // Write channel names
    if (song_ != NULL) {
        for (int i = 1; i <= visibleTracks; i++, x += trackWidth) {
            QString name = song_->track(i + leftmostTrack - 1)->name();
            QString buf = QString("%1: %2").arg(i + leftmostTrack).arg(name).left(TRACKER_TRACK_WIDTH - 1);
            QColor color;

            if (song_->track(i + leftmostTrack - 1)->isMuted() && !song_->track(i + leftmostTrack - 1)->isSolo()) {
                color = colors[ColorTrackHeaderMute];
            } else if (!song_->track(i + leftmostTrack - 1)->isMuted() && song_->track(i + leftmostTrack - 1)->isSolo()) {
                color = colors[ColorTrackHeaderSolo];
            } else if (song_->track(i + leftmostTrack - 1)->isMuted() && song_->track(i + leftmostTrack - 1)->isSolo()) {
                color = colors[ColorTrackHeaderMuteSolo];
            } else {
                color = colors[ColorTrackHeader];
            }

            painter.setPen(color);
            painter.setFont(font);
            painter.drawText(x + 2, fontAscent, buf);
        }
    }
}

void Tracker::printCursor()
{
    if (!(cursorTrack_ >= leftmostTrack && cursorTrack_ < leftmostTrack + visibleTracks)) {
        return;
    }

    if (!((unsigned int)cursorItem_ <= 6)) {
        return;
    }

    int width = 1;
    int x = 0;
    int y = startY + cursorLine * fontHeight;

    switch (cursorItem_) {
    case 0:			/* note */
        width = 3;
        break;
    case 1:			/* instrument 0 */
        x = 4;
        break;
    case 2:			/* instrument 1 */
        x = 5;
        break;
    case 3:			/* effect 0 */
        x = 7;
        break;
    case 4:			/* effect 1 */
        x = 8;
        break;
    case 5:			/* effect 2 */
        x = 10;
        break;
    case 6:			/* effect 3 */
        x = 11;
        break;
    default:
        break;
    }

    x = x * fontWidth + startX + (cursorTrack_ - leftmostTrack) * trackWidth - 1;

    QPainter painter(pixmap);
    painter.setPen(colors[ColorCursor]);
    painter.drawRect(x, y, width * fontWidth + 1, fontHeight - 1);
}

void Tracker::drawClever(const QRect &area)
{
    if (!isVisible() || block_ == NULL) {
        return;
    }

    int dist = line_ - oldLine;
    if (dist != 0) {
        oldLine = line_;
        int absdist = abs(dist);
        int redrawcnt = visibleLines;
        int y = startY;

        // Scrolling less than half the tracker area so that the cursor is still visible after scrolling?
        if (absdist <= cursorLine) {
            // Remove the cursor row before scrolling
            printNotes(0, (cursorLine) * fontHeight + startY, geometry().width(), fontHeight, oldLine - dist, false);
        }

        // Scroll the stuff already drawn on the screen
        if (absdist < visibleLines) {
            QPainter painter(pixmap);
            if (dist > 0) {
                // go down in pattern -- scroll up
                redrawcnt = absdist;
                painter.drawPixmap(0, y, *pixmap, 0, y + (absdist * fontHeight), geometry().width(), (visibleLines - absdist) * fontHeight);
                y += (visibleLines - absdist) * fontHeight;
            } else if (dist < 0) {
                // go up in pattern -- scroll down
                redrawcnt = absdist;
                painter.drawPixmap(0, y + (absdist * fontHeight), *pixmap, 0, y, geometry().width(), (visibleLines - absdist) * fontHeight);
            }
        }

        // Print the new rows that are now visible
        printNotes(0, y, geometry().width(), redrawcnt * fontHeight, oldLine, true);
    }

    // Redraw the cursor row to include the cursor
    printNotes(0, (cursorLine) * fontHeight + startY, geometry().width(), fontHeight, line_, true);
    printCursor();

    // Print the channel headers and separator bars
    printTrackHeaders();
    printBars();

    QPainter painter(this);
    painter.drawPixmap(area.x(), area.y(), area.width(), area.height(), *pixmap);
}

void Tracker::drawStupid(const QRect &area)
{
    oldLine = -2 * visibleLines;
    update(area);
}

void Tracker::initDisplay(int width, int height)
{
    int lineNumbersSpace = 3 * fontWidth;

    height -= fontHeight;
    visibleLines = height / fontHeight;
    if (!(visibleLines % 2)) {
        visibleLines--;
    }
    cursorLine = visibleLines / 2;
    startY = (height - fontHeight * visibleLines) / 2 + fontHeight;

    trackWidth = TRACKER_TRACK_WIDTH * fontWidth;
    int u = width - lineNumbersSpace - 10;
    visibleTracks = u / trackWidth;

    if (visibleTracks > tracks) {
        visibleTracks = tracks;
    }

    startX = (u - visibleTracks * trackWidth) / 2 + lineNumbersSpace + 5;
    setVisibleArea();

    if (block_) {
        emit lineChanged(line_, block_->length(), visibleLines);
        emit trackChanged(leftmostTrack, tracks, visibleTracks);
    }

    delete pixmap;
    pixmap = new QPixmap(geometry().width(), geometry().height());
    pixmap->fill(Qt::transparent);
}

void Tracker::reset()
{
    line_ = 0;
    cursorTrack_ = 0;
    cursorItem_ = 0;
    leftmostTrack = 0;
    block_ = NULL;
    initDisplay(geometry().width(), geometry().height());
    setVisibleArea();
    queueDraw();
}

void Tracker::initColors()
{
    static const int default_colors[] = {
      10, 20, 30,
      100, 100, 100,
      50, 60, 70,
      230, 230, 230,
      170, 170, 200,
      230, 200, 0,
      115, 100, 0,
      255, 230, 200,
      172, 150, 0,
      250, 100, 50
    };

    const int *p = default_colors;
    QColor *c = colors;

    for (int n = 0; n < ColorLast; n++, c++) {
        c->setRed(*p++);
        c->setGreen(*p++);
        c->setBlue(*p++);
    }
}

void Tracker::calculateFontSize()
{
    QFontMetrics metrics(font);
    fontWidth = metrics.width('0');
    fontHeight = metrics.ascent() + 1;
    fontAscent = metrics.ascent();
}

bool Tracker::setFont(const QString &fontname)
{
    font.setFamily(fontname);

    calculateFontSize();
    reset();

    return true;
}

void Tracker::mouseToCursorPos(int x, int y, int *cursorTrack, int *cursorItem, int *line)
{
    int HPatHalf;

    // Calc the column (channel and pos in channel)
    if (x < startX) {
        if (leftmostTrack) {
            *cursorTrack = leftmostTrack - 1;
        } else {
            *cursorTrack = leftmostTrack;
        }
        *cursorItem = 0;
    } else if (x > startX + visibleTracks * trackWidth) {
        if (leftmostTrack + visibleTracks < tracks) {
            *cursorTrack = leftmostTrack + visibleTracks;
            *cursorItem = 0;
        } else {
            *cursorTrack = tracks - 1;
            *cursorItem = 6;
        }
    } else {
        /* WTF */
        *cursorTrack = leftmostTrack + ((x - startX) / trackWidth);
        *cursorItem = (x - (startX + (*cursorTrack - leftmostTrack) * trackWidth)) / fontWidth;
        if (*cursorItem < 4) {
            *cursorItem = 0;
        } else if (*cursorItem == 4) {
            *cursorItem = 1;
        } else if (*cursorItem == 5 || *cursorItem == 6) {
            *cursorItem = 2;
        } else if (*cursorItem == 7) {
            *cursorItem = 3;
        } else if (*cursorItem == 8 || *cursorItem == 9) {
            *cursorItem = 4;
        } else if (*cursorItem == 10) {
            *cursorItem = 5;
        } else if (*cursorItem >= 11) {
            *cursorItem = 6;
        }
    }

    // Calc the row
    HPatHalf = visibleLines / 2;
    if (y < startY) {
        *line = line_ - HPatHalf - 1;
    } else if (y > visibleLines * fontHeight) {
        *line = line_ + HPatHalf + 1;
    } else {
        *line = (y - startY) / fontHeight;
        if (line_ <= *line) {
            *line = line_ + *line - HPatHalf;
        } else {
            *line = line_ - (HPatHalf - *line);
        }
    }
    if (*line < 0) {
        *line = 0;
    } else if (*line >= block_->length()) {
        *line = block_->length() - 1;
    }
}

void Tracker::mousePressEvent(QMouseEvent *event)
{
    int x, y, cursorTrack, cursorItem, line;

    x = event->x();
    y = event->y();

    if (mouseSelecting && event->button() != Qt::LeftButton) {
        mouseSelecting = false;
    } else if (!mouseSelecting) {
        mouseButton = event->button();
        if (mouseButton == Qt::LeftButton) {
            // Start selecting block
            inSelectionMode = false;
            mouseToCursorPos(x, y, &selectionStartTrack, &cursorItem, &selectionStartLine);
            selectionEndLine = selectionStartLine;
            selectionEndTrack = selectionStartTrack;
            mouseSelecting = true;
            queueDraw();
            emit selectionChanged(selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
        } else if (mouseButton == Qt::RightButton) {
            // Tracker cursor posititioning and clear block mark if any
            if (inSelectionMode || selectionStartTrack != -1) {
                selectionStartTrack = selectionEndTrack = -1;
                selectionStartLine = selectionEndLine = -1;
                inSelectionMode = false;
                queueDraw();
                emit selectionChanged(selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
            }
            mouseToCursorPos(x, y, &cursorTrack, &cursorItem, &line);
            if (cursorTrack != cursorTrack_ || cursorItem != cursorItem_) {
                cursorTrack_ = cursorTrack;
                cursorItem_ = cursorItem;
                setVisibleArea();
            }
            if (line != line_) {
                setLine(line);
            }
            queueDraw();
        }
    }

    event->accept();
}

void Tracker::mouseMoveEvent(QMouseEvent *event)
{
    int x, y, cursorItem;

    if (!mouseSelecting) {
        return;
    }

    x = event->x();
    y = event->y();

    if (event->buttons() &= Qt::LeftButton && mouseSelecting) {
        mouseToCursorPos(x, y, &selectionEndTrack, &cursorItem, &selectionEndLine);

        if (x > startX + visibleTracks * trackWidth && leftmostTrack + visibleTracks < tracks) {
            selectionEndTrack++;
            setLeftmostTrack(leftmostTrack + 1);
        } else if (x < startX && leftmostTrack > 0) {
            selectionEndTrack--;
            setLeftmostTrack(leftmostTrack - 1);
        }
        if ((selectionEndLine > line_ + (visibleLines / 2)) || (y > geometry().height() && line_ < selectionEndLine)) {
            setLine(line_ + 1);
        } else if ((selectionEndLine < line_ - (visibleLines / 2)) || (y <= 0 && line_ > selectionEndLine)) {
            setLine(line_ - 1);
        }
        queueDraw();
        emit selectionChanged(selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
    }

    event->accept();
}

void Tracker::mouseReleaseEvent(QMouseEvent *event)
{
    if (mouseSelecting && event->button() == Qt::LeftButton) {
        mouseSelecting = false;
    }

    event->accept();
}

void Tracker::paintEvent(QPaintEvent *event)
{
    drawClever(event->rect());
}

void Tracker::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    drawStupid(QRect(QPoint(), geometry().size()));
}

void Tracker::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    initDisplay(geometry().width(), geometry().height());
    queueDraw();
}

void Tracker::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);

    bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
    bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
    bool alt = (event->modifiers() & Qt::AltModifier) != 0;

    if (ctrl) {
        switch (event->key()) {
        case Qt::Key_B:
            // CTRL-B: Selection mode on/off
            markSelection(!inSelectionMode);
            break;
        case Qt::Key_K:
            // CTRL-K: Clear until the end of the track
            block_->clear(cursorTrack_, line_, cursorTrack_, block_->length() - 1);
            break;
        case Qt::Key_Tab:
            // CTRL-Tab: Next command page
            setCommandPage(commandPage_ + 1);
            break;
        case Qt::Key_Backtab:
            // CTRL-Shift-Tab: Previous command page
            setCommandPage(commandPage_ - 1);
            break;
        case Qt::Key_Delete: {
            if (inEditMode) {
                // Delete command and refresh
                for (int i = 0; i < block_->commandPages(); i++) {
                    block_->setCommandFull(line_, cursorTrack_, i, 0, 0);
                }
                emit lineEdited();
            }
            break;
        }
        }
    } else if (shift) {
        switch (event->key()) {
        case Qt::Key_Backtab:
            // Shift-Tab: Previous track
            stepCursorTrack(-1);
            break;
        case Qt::Key_Delete:
            // Delete note+command and refresh
            if (inEditMode) {
                block_->setNote(line_, cursorTrack_, 0, 0, 0);
                for (int commandPage = 0; commandPage < block_->commandPages(); commandPage++) {
                    block_->setCommandFull(line_, cursorTrack_, commandPage, 0, 0);
                }
                emit lineEdited();
            }
            break;
        case Qt::Key_Backspace:
            // Shift-Backspace: Insert row
            if (inEditMode) {
                block_->insertLine(line_, alt ? -1 : cursorTrack_);
            }
            break;
        default:
            break;
        }
    } else if (alt) {
        switch (event->key()) {
        case Qt::Key_Backspace:
            // Backspace: delete line
            if (inEditMode) {
                block_->deleteLine(line_);
            }
            break;
        default:
            break;
        }
    } else {
        char note = -1;
        char data = -1;

        if (cursorItem_ == 0) {
            // Editing notes
            data = keyToNote.value(event->key(), -1);

            if (data >= 0) {
                note = data - 1;

                if (inChordMode) {
                    // Chord mode: check if the key has already been pressed (key repeat)
                    bool found = keyboardKeysDown.contains(event->key());

                    // Go to next channel if the current key has not been pressed down yet
                    if (!found) {
                        // Add the key to the keys down list
                        keyboardKeysDown.append(event->key());

                        if (inEditMode) {
                            // Set note and refresh
                            block_->setNote(line_, cursorTrack_, octave_, data, instrument_);
                        }

                        addChordNote();
                    }
                } else if (inEditMode) {
                    // Set note and refresh
                    block_->setNote(line_, cursorTrack_, octave_, data, instrument_);
                    emit lineEdited();
                }
            }
        } else if (inEditMode) {
            switch (cursorItem_) {
            case 1:
            case 2:
                // Editing instrument
                if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9) {
                    data = event->key() - Qt::Key_0;
                } else if (event->key() >= Qt::Key_A && event->key() <= Qt::Key_F) {
                    data = 10 + event->key() - Qt::Key_A;
                } else if (event->key() == Qt::Key_Delete) {
                    data = 0;
                }

                if (data >= 0) {
                    // Set instrument and refresh
                    int ins = block_->instrument(line_, cursorTrack_);
                    if (cursorItem_ == 1) {
                        ins = (ins & 0x0f) | (data << 4);
                    } else {
                        ins = (ins & 0xf0) | data;
                    }
                    block_->setInstrument(line_, cursorTrack_, ins);
                    emit lineEdited();
                }
                break;
            case 3:
            case 4:
            case 5:
            case 6:
                // Editing effects
                if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9) {
                    data = event->key() - Qt::Key_0;
                } else if (event->key() >= Qt::Key_A && event->key() <= Qt::Key_F) {
                    data = 10 + event->key() - Qt::Key_A;
                } else if (event->key() == Qt::Key_Delete) {
                    data = 0;
                }

                if (data >= 0) {
                    // Set effect and refresh
                    block_->setCommand(line_, cursorTrack_, commandPage_, cursorItem_ - 3, data);
                    emit lineEdited();
                }
                break;
            }
        }

        switch (event->key()) {
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
        case Qt::Key_F11:
            // F1-F11: Set active keyboard octave
            setOctave(event->key() - Qt::Key_F1);
            break;
        case Qt::Key_Left:
            // Left: Go left
            stepCursorItem(-1);
            break;
        case Qt::Key_Right:
            // Right: Go right
            stepCursorItem(1);
            break;
        case Qt::Key_Tab:
            // Tab: Next track
            setCursorItem(0);
            stepCursorTrack(1);
            break;
        case Qt::Key_Backspace:
            if (inEditMode) {
                // Backspace: delete line
                block_->deleteLine(line_, cursorTrack_);
                emit lineEdited();
            }
            break;
        default:
            break;
        }

        // Play note if a key was pressed but not if cursor is in cmd pos
        if (note >= 0 && cursorItem_ == 0) {
            emit notePressed(note);
        }
    }
}

void Tracker::keyReleaseEvent(QKeyEvent *event)
{
    QWidget::keyReleaseEvent(event);

    bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
    bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;

    // Key has been released
    if (!ctrl && !shift) {
        if (inChordMode && cursorItem_ == 0) {
            if (event->key() != Qt::Key_Delete && keyToNote.contains(event->key())) {
                // Find the key from the keys down list and remove it
                keyboardKeysDown.removeAll(event->key());

                removeChordNote();
            }
        }
    }
}

void Tracker::queueDraw()
{
    oldLine = -2 * visibleLines;
    update();
}

Song *Tracker::song() const
{
    return song_;
}

Block *Tracker::block() const
{
    return block_;
}

int Tracker::track() const
{
    return cursorTrack_;
}

int Tracker::line() const
{
    return line_;
}

int Tracker::commandPage() const
{
    return commandPage_;
}

int Tracker::cursorTrack() const
{
    return cursorTrack_;
}

int Tracker::cursorItem() const
{
    return cursorItem_;
}

int Tracker::octave() const
{
    return octave_;
}

void Tracker::redrawArea(int startTrack, int startLine, int endTrack, int endLine)
{
    Q_UNUSED(startTrack)
    Q_UNUSED(startLine)
    Q_UNUSED(endTrack)
    Q_UNUSED(endLine)

    queueDraw();
}

void Tracker::checkBounds()
{
    if (cursorTrack_ >= block_->tracks()) {
        cursorTrack_ = block_->tracks() - 1;
        queueDraw();
    }

    if (line_ >= block_->length()) {
        setLine(block_->length() - 1);
    }

    if (commandPage_ >= block_->commandPages()) {
        setCommandPage(block_->commandPages() - 1);
    }
}

void Tracker::setSelection(int startTrack, int startLine, int endTrack, int endLine)
{
    selectionStartTrack = startTrack;
    selectionStartLine = startLine;
    selectionEndTrack = endTrack;
    selectionEndLine = endLine;
    inSelectionMode = false;
    queueDraw();
    emit selectionChanged(selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
}

void Tracker::setEditMode(bool enabled)
{
    inEditMode = enabled;
}

void Tracker::setChordMode(bool enabled)
{
    inChordMode = enabled;
}

void Tracker::setInstrument(int instrument)
{
    instrument_ = instrument;
}

void Tracker::setOctave(int octave)
{
    if (octave_ != octave) {
        octave_ = octave;

        emit octaveChanged(octave_);
    }
}

void Tracker::addChordNote()
{
    stepCursorTrack(1);
    chordStatus++;
}

void Tracker::removeChordNote()
{
    // Go to the previous channel
    stepCursorTrack(-1);
    chordStatus--;

    // If all chord notes have been released go to the next line
    if (chordStatus == 0 && inEditMode) {
        emit lineEdited();
    }
}
