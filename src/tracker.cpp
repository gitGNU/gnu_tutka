/*
 * tracker.cpp
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

#include <QPainter>
#include <QPaintEvent>
#include <cstdio>
#include "song.h"
#include "track.h"
#include "block.h"
#include "tracker.h"

Tracker::Tracker(QWidget *parent) :
    QWidget(parent),
    visibleLines(0),
    startY(0),
    visibleTracks(0),
    startX(0),
    trackWidth(0),
    cursorLine(0),
    font("Monospace", 11),
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
    mouseButton(Qt::NoButton)
{
    font.setStyleHint(QFont::TypeWriter);
    calculateFontSize();
    initColors();

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
        if (cursorTrack_ >= this->tracks) {
            cursorTrack_ = this->tracks - 1;
        }

        initDisplay(geometry().width(), geometry().height());
        queueDraw();

        emit xpanningChanged(leftmostTrack, this->tracks, visibleTracks);
    }
}

void Tracker::setCommandPage(int commandPage)
{
    if (commandPage_ != commandPage) {
        commandPage_ = commandPage;

        queueDraw();
    }
}

void Tracker::setLine(unsigned int line)
{
    if (block_ == NULL) {
        return;
    }

    line %= block_->length();

    if (line_ != line) {
        line_ = line;
        emit patposChanged(line, block_->length(), visibleLines);

        queueDraw();
    }
}

void Tracker::redraw()
{
    queueDraw();
}

void Tracker::redrawRow(int)
{
    // This is yet to be optimized :-)
    redraw();
}

void Tracker::redrawCurrentRow()
{
    redrawRow(line_);
}

void Tracker::setSong(Song *song)
{
    song_ = song;
}

void Tracker::setBlock(unsigned int number)
{
    Block *block = song_ != NULL ? song_->block(number) : NULL;

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

            emit patposChanged(line_, block->length(), visibleLines);
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

        if (cursorTrack_ < this->leftmostTrack) {
            cursorTrack_ = this->leftmostTrack;
        } else if (cursorTrack_ >= this->leftmostTrack + visibleTracks) {
            cursorTrack_ = this->leftmostTrack + visibleTracks - 1;
        }

        emit xpanningChanged(this->leftmostTrack, tracks, visibleTracks);
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
        redraw();
    }

    emit selectionChanged(selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
}

void Tracker::clearMarkSelection()
{
    if (selectionStartTrack != -1) {
        selectionStartTrack = selectionEndTrack = -1;
        selectionStartLine = selectionEndLine = -1;
        inSelectionMode = false;
        redraw();

        emit selectionChanged(selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
    }
}

bool Tracker::isInSelectionMode()
{
    return inSelectionMode;
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
            QString buf = QString("%1: %2").arg(i + leftmostTrack).arg(name);
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
    painter.drawRect(x, y, width * fontWidth, fontHeight - 1);
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
        emit patposChanged(line_, block_->length(), visibleLines);
        emit xpanningChanged(leftmostTrack, tracks, visibleTracks);
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
        *line = this->line_ - HPatHalf - 1;
    } else if (y > visibleLines * fontHeight) {
        *line = this->line_ + HPatHalf + 1;
    } else {
        *line = (y - startY) / fontHeight;
        if (this->line_ <= *line) {
            *line = this->line_ + *line - HPatHalf;
        } else {
            *line = this->line_ - (HPatHalf - *line);
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
            redraw();
            emit selectionChanged(selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
        } else if (mouseButton == Qt::RightButton) {
            // Tracker cursor posititioning and clear block mark if any
            if (inSelectionMode || selectionStartTrack != -1) {
                selectionStartTrack = selectionEndTrack = -1;
                selectionStartLine = selectionEndLine = -1;
                inSelectionMode = false;
                redraw();
                emit selectionChanged(selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
            }
            mouseToCursorPos(x, y, &cursorTrack, &cursorItem, &line);
            if (cursorTrack != this->cursorTrack_ || cursorItem != this->cursorItem_) {
                this->cursorTrack_ = cursorTrack;
                this->cursorItem_ = cursorItem;
                setVisibleArea();
            }
            if (line != this->line_) {
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
        redraw();
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

void Tracker::redrawArea(int startTrack, int startLine, int endTrack, int endLine)
{
    Q_UNUSED(startTrack)
    Q_UNUSED(startLine)
    Q_UNUSED(endTrack)
    Q_UNUSED(endLine)

    redraw();
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
    redraw();
    emit selectionChanged(selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
}
