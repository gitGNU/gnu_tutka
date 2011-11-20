#include <QPainter>
#include <QPaintEvent>
#include <cstdio>
#include "song.h"
#include "block.h"
#include "tracker.h"

Tracker::Tracker(QWidget *parent) :
    QWidget(parent),
    disp_rows(0),
    disp_starty(0),
    disp_numchans(0),
    disp_startx(0),
    disp_chanwidth(0),
    disp_cursor(0),
    fontdesc("Monospace", 9),
    fontw(0),
    fonth(0),
    fontc(0),
    bg_gc(Qt::black),
    bg_cursor_gc(Qt::yellow),
    notes_gc(Qt::gray),
    misc_gc(Qt::green),
    pixmap(NULL),
    idle_handler(0),
    song(NULL),
    curpattern(NULL),
    cmdpage(0),
    patpos(0),
    oldpos(-1),
    num_channels(0),
    cursor_ch(0),
    cursor_item(0),
    leftchan(0),
    inSelMode(false),
    sel_start_ch(-1),
    sel_start_row(-1),
    sel_end_ch(-1),
    sel_end_row(-1),
    mouse_selecting(false),
    button(-1)
{
    calculateFontSize();
    initColors();

    bg_gc = colors[TRACKERCOL_BG];
    bg_cursor_gc = colors[TRACKERCOL_BG_CURSOR];
    notes_gc = colors[TRACKERCOL_NOTES];
    misc_gc = colors[TRACKERCOL_BG_SELECTION];

    initDisplay(geometry().width(), geometry().height());
}

void Tracker::setNumChannels(int n)
{
    if (num_channels != n) {
        num_channels = n;

        // Make sure the cursor is inside the tracker
        if (cursor_ch >= num_channels) {
            cursor_ch = num_channels - 1;
        }

        initDisplay(geometry().width(), geometry().height());
        queueDraw();

        emit xpanningChanged(leftchan, num_channels, disp_numchans);
    }
}

void Tracker::setCmdpage(int cmdpage)
{
    if (this->cmdpage != cmdpage) {
        this->cmdpage = cmdpage;

        queueDraw();
    }
}

void Tracker::setPatpos(int row)
{
    if (!((curpattern == NULL && row == 0) || (row < curpattern->length()))) {
        return;
    }

    if (patpos != row) {
        patpos = row;
        emit patposChanged(row, curpattern->length(), disp_rows);

        if (inSelMode) {
            // Force re-draw of patterns in block selection mode
            queueDraw();
        }
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
    redrawRow(patpos);
}

void Tracker::setSong(Song *song)
{
    this->song = song;
}

void Tracker::setPattern(Block *pattern)
{
    if (curpattern != pattern) {
        curpattern = pattern;
        if (pattern != NULL) {
            // Make sure the cursor is inside the tracker
            if (patpos >= pattern->length()) {
                patpos = pattern->length() - 1;
            }

            if (cursor_ch >= pattern->tracks()) {
                cursor_ch = pattern->tracks() - 1;
            }

            emit patposChanged(patpos, pattern->length(), disp_rows);
        }
        queueDraw();
    }
}

void Tracker::setXpanning(int left_channel)
{
    if (leftchan != left_channel) {
        if (!(left_channel + disp_numchans <= num_channels)) {
            return;
        }

        leftchan = left_channel;
        queueDraw();

        if (cursor_ch < leftchan) {
            cursor_ch = leftchan;
        } else if (cursor_ch >= leftchan + disp_numchans) {
            cursor_ch = leftchan + disp_numchans - 1;
        }

        emit xpanningChanged(leftchan, num_channels, disp_numchans);
    }
}

void Tracker::adjustXpanning()
{
    if (leftchan + disp_numchans > num_channels) {
        setXpanning(num_channels - disp_numchans);
    } else if (cursor_ch < leftchan) {
        setXpanning(cursor_ch);
    } else if (cursor_ch >= leftchan + disp_numchans) {
        setXpanning(cursor_ch - disp_numchans + 1);
    }
}

void Tracker::stepCursorItem(int direction)
{
    if (!(direction == -1 || direction == 1)) {
        return;
    }

    cursor_item += direction;
    if (cursor_item > 6) {
        cursor_item %= 7;
        stepCursorChannel(direction);
    } else if (cursor_item < 0) {
        cursor_item += 7;
        stepCursorChannel(direction);
    } else {
        adjustXpanning();
        queueDraw();
    }
}

void Tracker::stepCursorChannel(int direction)
{
    cursor_ch += direction;

    if (cursor_ch < 0) {
        cursor_ch = num_channels - 1;
    } else if (cursor_ch >= num_channels) {
        cursor_ch = 0;
    }

    adjustXpanning();

    if (inSelMode) {
        // Force re-draw of patterns in block selection mode
        queueDraw();
    }
}

void Tracker::stepCursorRow(int direction)
{
    int newpos = patpos + direction;

    while (newpos < 0) {
        newpos += curpattern->length();
    }
    newpos %= curpattern->length();

    setPatpos(newpos);
}

void Tracker::markSelection(bool enable)
{
    if (!enable) {
        sel_end_ch = cursor_ch;
        sel_end_row = patpos;
        inSelMode = false;
    } else {
        sel_start_ch = sel_end_ch = cursor_ch;
        sel_start_row = sel_end_row = patpos;
        inSelMode = true;
        redraw();
    }
}

void Tracker::clearMarkSelection()
{
    if (sel_start_ch != -1) {
        sel_start_ch = sel_end_ch = -1;
        sel_start_row = sel_end_row = -1;
        inSelMode = false;
        redraw();
    }
}

bool Tracker::isInSelectionMode()
{
    return inSelMode;
}

void Tracker::getSelectionRect(int *chStart, int *rowStart, int *nChannel, int *nRows)
{
    if (!inSelMode) {
        if (sel_start_ch <= sel_end_ch) {
            *nChannel = sel_end_ch - sel_start_ch + 1;
            *chStart = sel_start_ch;
        } else {
            *nChannel = sel_start_ch - sel_end_ch + 1;
            *chStart = sel_end_ch;
        }
        if (sel_start_row <= sel_end_row) {
            *nRows = sel_end_row - sel_start_row + 1;
            *rowStart = sel_start_row;
        } else {
            *nRows = sel_start_row - sel_end_row + 1;
            *rowStart = sel_end_row;
        }
    } else {
        if (sel_start_ch <= cursor_ch) {
            *nChannel = cursor_ch - sel_start_ch + 1;
            *chStart = sel_start_ch;
        } else {
            *nChannel = sel_start_ch - cursor_ch + 1;
            *chStart = cursor_ch;
        }
        if (sel_start_row <= patpos) {
            *nRows = patpos - sel_start_row + 1;
            *rowStart = sel_start_row;
        } else {
            *nRows = sel_start_row - patpos + 1;
            *rowStart = patpos;
        }
    }
}

bool Tracker::isValidSelection()
{
    return (sel_start_ch >= 0 && sel_start_ch < curpattern->tracks() && sel_end_ch >= 0 && sel_end_ch < curpattern->tracks() && sel_start_row >= 0 && sel_start_row < curpattern->length() && sel_end_row >= 0 && sel_end_row < curpattern->length());
}

void Tracker::note2string(unsigned char note, unsigned char instrument, unsigned char effect, unsigned char value, char *buf)
{
    static const char *const notenames[128] = {
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

    static const char hexmap[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
        'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
        'W', 'X', 'Y', 'Z'
    };

    buf[0] = notenames[note][0];
    buf[1] = notenames[note][1];
    buf[2] = notenames[note][2];
    buf[3] = ' ';
    buf[4] = hexmap[(instrument & 0xf0) >> 4];
    buf[5] = hexmap[instrument & 0x0f];
    buf[6] = ' ';
    buf[7] = hexmap[(effect & 0xf0) >> 4];
    buf[8] = hexmap[effect & 0x0f];
    buf[9] = ' ';
    buf[10] = hexmap[(value & 0xf0) >> 4];
    buf[11] = hexmap[value & 0x0f];
    buf[12] = ' ';
    buf[13] = 0;
}

void Tracker::clearNotesLine(int y, int pattern_row)
{
    // cursor line
    QBrush gc = pattern_row == patpos ? bg_cursor_gc : bg_gc;

    QPainter painter(pixmap);
    painter.fillRect(0, y, geometry().width(), fonth, gc);
}

void Tracker::printNotesLine(int y, int ch, int numch, int row, int cursor)
{
    char buf[TRACKER_CHANNEL_WIDTH + 1];
    int rbs, rbe, cbs, cbe;

    if (!(ch + numch <= curpattern->tracks())) {
        return;
    }

    clearNotesLine(y, row);

    // Figure out which rows/columns should be highlighted
    if (inSelMode) {
        rbs = sel_start_row;
        rbe = patpos;
        cbs = sel_start_ch;
        cbe = cursor_ch;
    } else {
        rbs = sel_start_row;
        rbe = sel_end_row;
        cbs = sel_start_ch;
        cbe = sel_end_ch;
    }

    QPainter painter(pixmap);
    int fontY = y + fontc;

    // The row number
    sprintf(buf, "%03d", row);
    painter.setPen(notes_gc.color());
    painter.setBackground(cursor ? bg_cursor_gc : bg_gc);
    painter.setFont(fontdesc);
    painter.drawText(1, fontY, buf);

    // The notes
    int j = 0;
    buf[TRACKER_CHANNEL_WIDTH] = 0;
    for (numch += ch; ch < numch; ch++, j++) {
        note2string(curpattern->note(row, ch), curpattern->instrument(row, ch), curpattern->command(row, ch, cmdpage), curpattern->commandValue(row, ch, cmdpage), buf);

        QBrush brush;
        if (cursor) {
            brush = bg_cursor_gc;
        } else if (row >= rbs && row <= rbe && ch >= cbs && ch <= cbe) {
            brush = misc_gc;
        }
        painter.fillRect(disp_startx + (j * TRACKER_CHANNEL_WIDTH) * fontw, y, disp_chanwidth, fonth, brush);
        painter.drawText(disp_startx + (j * TRACKER_CHANNEL_WIDTH) * fontw, fontY, buf);
    }
}

void Tracker::printNotes(int x, int y, int w, int h, int cursor_row, bool enable_cursor)
{
    Q_UNUSED(x)
    Q_UNUSED(w)

    // Limit y and h to the actually used window portion
    int my = y - disp_starty;
    if (my < 0) {
        h += my;
        my = 0;
    }
    if (my + h > fonth * disp_rows) {
        h = fonth * disp_rows - my;
    }

    // Calculate first and last line to be redrawn
    int n1 = my / fonth;
    int n2 = (my + h - 1) / fonth;

    // Print the notes
    int scry = disp_starty + n1 * fonth;
    for (int i = n1; i <= n2; i++, scry += fonth) {
        int n = i + cursor_row - disp_cursor;
        if (curpattern != NULL && n >= 0 && n < curpattern->length()) {
            printNotesLine(scry, leftchan, disp_numchans, n, (enable_cursor && n == cursor_row) ? 1 : 0);
        } else {
            QPainter painter(pixmap);
            painter.fillRect(0, scry, geometry().width(), fonth, bg_gc);
        }
    }
}

void Tracker::printBars()
{
    // Draw the separation bars
    QPainter painter(pixmap);
    painter.setPen(colors[TRACKERCOL_BARS]);
    int x1 = disp_startx - 3;
    for (int i = 0; i <= disp_numchans; i++, x1 += disp_chanwidth) {
        painter.drawLine(x1, 0, x1, geometry().height() - 1);
    }
}

void Tracker::printChannelHeaders()
{
    int x = disp_startx - 3;

    // Clear top and bottom
    QPainter painter(pixmap);
    painter.fillRect(QRectF(0, 0, geometry().width(), disp_starty), bg_gc);
    painter.fillRect(QRectF(0, disp_starty + disp_rows * fonth, geometry().width(), geometry().height() - (disp_starty + disp_rows * fonth)), bg_gc);

    // Write channel names
    if (song != NULL) {
        for (int i = 1; i <= disp_numchans; i++, x += disp_chanwidth) {
            QString name = song->track(i + leftchan - 1)->name;
            QString buf = QString("%1: %2").arg(i + leftchan).arg(name);
            QColor color;

            if (song->track(i + leftchan - 1)->mute && !song->track(i + leftchan - 1)->solo) {
                color = colors[TRACKERCOL_CHANNEL_HEADER_MUTE];
            } else if (!song->track(i + leftchan - 1)->mute && song->track(i + leftchan - 1)->solo) {
                color = colors[TRACKERCOL_CHANNEL_HEADER_SOLO];
            } else if (song->track(i + leftchan - 1)->mute && song->track(i + leftchan - 1)->solo) {
                color = colors[TRACKERCOL_CHANNEL_HEADER_MUTE_SOLO];
            } else {
                color = colors[TRACKERCOL_CHANNEL_HEADER];
            }

            painter.setPen(color);
            painter.setFont(fontdesc);
            painter.drawText(x + 2, fontc, buf);
        }
    }
}

void Tracker::printCursor()
{
    if (!(cursor_ch >= leftchan && cursor_ch < leftchan + disp_numchans)) {
        return;
    }

    if (!((unsigned int)cursor_item <= 6)) {
        return;
    }

    int width = 1;
    int x = 0;
    int y = disp_starty + disp_cursor * fonth;

    switch (cursor_item) {
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

    x = x * fontw + disp_startx + (cursor_ch - leftchan) * disp_chanwidth - 1;

    QPainter painter(pixmap);
    painter.setPen(colors[TRACKERCOL_CURSOR]);
    painter.drawRect(x, y, width * fontw, fonth - 1);
}

void Tracker::drawClever(const QRect &area)
{
    if (!isVisible() || curpattern == NULL) {
        return;
    }

    int dist = patpos - oldpos;
    if (dist != 0) {
        oldpos = patpos;
        int absdist = abs(dist);
        int redrawcnt = disp_rows;
        int y = disp_starty;

        // Scrolling less than half the tracker area so that the cursor is still visible after scrolling?
        if (absdist <= disp_cursor) {
            // Remove the cursor row before scrolling
            printNotes(0, (disp_cursor) * fonth + disp_starty, geometry().width(), fonth, oldpos - dist, false);
        }

        // Scroll the stuff already drawn on the screen
        if (absdist < disp_rows) {
            QPainter painter(pixmap);
            if (dist > 0) {
                // go down in pattern -- scroll up
                redrawcnt = absdist;
                painter.drawPixmap(0, y, *pixmap, 0, y + (absdist * fonth), geometry().width(), (disp_rows - absdist) * fonth);
                y += (disp_rows - absdist) * fonth;
            } else if (dist < 0) {
                // go up in pattern -- scroll down
                redrawcnt = absdist;
                painter.drawPixmap(0, y + (absdist * fonth), *pixmap, 0, y, geometry().width(), (disp_rows - absdist) * fonth);
            }
        }

        // Print the new rows that are now visible
        printNotes(0, y, geometry().width(), redrawcnt * fonth, oldpos, true);
    }

    // Redraw the cursor row to include the cursor
    printNotes(0, (disp_cursor) * fonth + disp_starty, geometry().width(), fonth, patpos, true);
    printCursor();

    // Print the channel headers and separator bars
    printChannelHeaders();
    printBars();

    QPainter painter(this);
    painter.drawPixmap(area.x(), area.y(), area.width(), area.height(), *pixmap);
}

void Tracker::drawStupid(const QRect &area)
{
    oldpos = -2 * disp_rows;
    update(area);
}

void Tracker::initDisplay(int width, int height)
{
    int line_numbers_space = 3 * fontw;

    height -= fonth;
    disp_rows = height / fonth;
    if (!(disp_rows % 2)) {
        disp_rows--;
    }
    disp_cursor = disp_rows / 2;
    disp_starty = (height - fonth * disp_rows) / 2 + fonth;

    disp_chanwidth = TRACKER_CHANNEL_WIDTH * fontw;
    int u = width - line_numbers_space - 10;
    disp_numchans = u / disp_chanwidth;

    if (disp_numchans > num_channels) {
        disp_numchans = num_channels;
    }

    disp_startx = (u - disp_numchans * disp_chanwidth) / 2 + line_numbers_space + 5;
    adjustXpanning();

    if (curpattern) {
        emit patposChanged(patpos, curpattern->length(), disp_rows);
        emit xpanningChanged(leftchan, num_channels, disp_numchans);
    }

    delete pixmap;
    pixmap = new QPixmap(geometry().width(), geometry().height());
    pixmap->fill(Qt::transparent);
}

void Tracker::reset()
{
    patpos = 0;
    cursor_ch = 0;
    cursor_item = 0;
    leftchan = 0;
    curpattern = NULL;
    initDisplay(geometry().width(), geometry().height());
    adjustXpanning();
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

    for (int n = 0; n < TRACKERCOL_LAST; n++, c++) {
        c->setRed(*p++);
        c->setGreen(*p++);
        c->setBlue(*p++);
    }
}

void Tracker::calculateFontSize()
{
    QFontMetrics metrics(fontdesc);
    fontw = metrics.width('X');
    fonth = metrics.height();
    fontc = metrics.ascent();
}

bool Tracker::setFont(const QString &fontname)
{
    fontdesc.setFamily(fontname);

    calculateFontSize();
    reset();

    return true;
}

void Tracker::mouseToCursorPos(int x, int y, int *cursor_ch, int *cursor_item, int *patpos)
{
    int HPatHalf;

    // Calc the column (channel and pos in channel)
    if (x < disp_startx) {
        if (leftchan) {
            *cursor_ch = leftchan - 1;
        } else {
            *cursor_ch = leftchan;
        }
        *cursor_item = 0;
    } else if (x > disp_startx + disp_numchans * disp_chanwidth) {
        if (leftchan + disp_numchans < num_channels) {
            *cursor_ch = leftchan + disp_numchans;
            *cursor_item = 0;
        } else {
            *cursor_ch = num_channels - 1;
            *cursor_item = 6;
        }
    } else {
        /* WTF */
        *cursor_ch = leftchan + ((x - disp_startx) / disp_chanwidth);
        *cursor_item = (x - (disp_startx + (*cursor_ch - leftchan) * disp_chanwidth)) / fontw;
        if (*cursor_item < 4) {
            *cursor_item = 0;
        } else if (*cursor_item == 4) {
            *cursor_item = 1;
        } else if (*cursor_item == 5 || *cursor_item == 6) {
            *cursor_item = 2;
        } else if (*cursor_item == 7) {
            *cursor_item = 3;
        } else if (*cursor_item == 8 || *cursor_item == 9) {
            *cursor_item = 4;
        } else if (*cursor_item == 10) {
            *cursor_item = 5;
        } else if (*cursor_item >= 11) {
            *cursor_item = 6;
        }
    }

    // Calc the row
    HPatHalf = disp_rows / 2;
    if (y < disp_starty) {
        *patpos = this->patpos - HPatHalf - 1;
    } else if (y > disp_rows * fonth) {
        *patpos = this->patpos + HPatHalf + 1;
    } else {
        *patpos = (y - disp_starty) / fonth;
        if (this->patpos <= *patpos) {
            *patpos = this->patpos + *patpos - HPatHalf;
        } else {
            *patpos = this->patpos - (HPatHalf - *patpos);
        }
    }
    if (*patpos < 0) {
        *patpos = 0;
    } else if (*patpos >= curpattern->length()) {
        *patpos = curpattern->length() - 1;
    }
}

void Tracker::mousePressEvent(QMouseEvent *event)
{
    int x, y, cursor_ch, cursor_item, patpos;

    x = event->x();
    y = event->y();

    if (mouse_selecting && event->button() != Qt::LeftButton) {
        mouse_selecting = false;
    } else if (!mouse_selecting) {
        button = event->button();
        if (button == Qt::LeftButton) {
            // Start selecting block
            emit blockmarkSet(1);
            inSelMode = false;
            mouseToCursorPos(x, y, &sel_start_ch, &cursor_item, &sel_start_row);
            sel_end_row = sel_start_row;
            sel_end_ch = sel_start_ch;
            mouse_selecting = true;
            redraw();
        } else if (button == Qt::RightButton) {
            // Tracker cursor posititioning and clear block mark if any
            if (inSelMode || sel_start_ch != -1) {
                emit blockmarkSet(0);
                sel_start_ch = sel_end_ch = -1;
                sel_start_row = sel_end_row = -1;
                inSelMode = false;
                redraw();
            }
            mouseToCursorPos(x, y, &cursor_ch, &cursor_item, &patpos);
            if (cursor_ch != cursor_ch || cursor_item != cursor_item) {
                cursor_ch = cursor_ch;
                cursor_item = cursor_item;
                adjustXpanning();
            }
            if (patpos != patpos) {
                setPatpos(patpos);
            }
            queueDraw();
        }
    }

    event->accept();
}

void Tracker::mouseMoveEvent(QMouseEvent *event)
{
    int x, y, cursor_item;

    if (!mouse_selecting) {
        return;
    }

    x = event->x();
    y = event->y();

    if (event->buttons() &= Qt::LeftButton && mouse_selecting) {
        mouseToCursorPos(x, y, &sel_end_ch, &cursor_item, &sel_end_row);

        if (x > disp_startx + disp_numchans * disp_chanwidth && leftchan + disp_numchans < num_channels) {
            sel_end_ch++;
            setXpanning(leftchan + 1);
        } else if (x < disp_startx && leftchan > 0) {
            sel_end_ch--;
            setXpanning(leftchan - 1);
        }
        if ((sel_end_row > patpos + (disp_rows / 2)) || (y > geometry().height() && patpos < sel_end_row)) {
            setPatpos(patpos + 1);
        } else if ((sel_end_row < patpos - (disp_rows / 2)) || (y <= 0 && patpos > sel_end_row)) {
            setPatpos(patpos - 1);
        }
        redraw();
    }

    event->accept();
}

void Tracker::mouseReleaseEvent(QMouseEvent *event)
{
    if (mouse_selecting && event->button() == Qt::LeftButton) {
        mouse_selecting = false;
        emit blockmarkSet(0);
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
    drawStupid(QRect(QPoint(), geometry().size()));
}

void Tracker::queueDraw()
{
    oldpos = -2 * disp_rows;
    update();
}
