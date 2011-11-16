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
    fontdesc("Monospace 9"),
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
    mouse_selecting(0),
    button(-1)
{
    calculateFontSize();
    initColors();

    bg_gc = colors[TRACKERCOL_BG];
    bg_cursor_gc = colors[TRACKERCOL_BG_CURSOR];
    notes_gc = colors[TRACKERCOL_NOTES];

    initDisplay(geometry().width(), geometry().height());

//    gdk_window_set_user_data(widgewindow, widget);
//    gdk_window_set_background(widgewindow, &colors[TRACKERCOL_BG]);

//    tracker_fonts_render(widget);
}

void Tracker::paintEvent(QPaintEvent *event)
{
    drawClever(event->rect());
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
//    adjust_xpanning(t);

    if (curpattern) {
//        g_signal_emit(GTK_OBJECT(t), tracker_signals[PATPOS_SIGNAL], 0, patpos, curpattern->length, disp_rows);
//        g_signal_emit(GTK_OBJECT(t), tracker_signals[XPANNING_SIGNAL], 0, leftchan, num_channels, disp_numchans);
    }

    delete pixmap;
    pixmap = new QPixmap(geometry().width(), geometry().height());
    pixmap->fill(Qt::transparent);
}

void Tracker::calculateFontSize()
{
    QFontMetrics metrics(fontdesc);
    fontw = metrics.width('X');
    fonth = metrics.height();
    fontc = metrics.height() / 2;
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
    QBrush gc = bg_gc;

    // cursor line
    if (pattern_row == patpos) {
        gc = bg_cursor_gc;
    }

    QPainter painter(pixmap);
    painter.fillRect(0, y, geometry().width(), fonth, bg_gc);
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
    painter.setBrush(notes_gc);

    // The row number
    sprintf(buf, "%03d", row);
    if (cursor) {
        painter.drawText(1, y, buf);
    }

    // The notes
    int j = 0;
    buf[TRACKER_CHANNEL_WIDTH] = 0;
    for (numch += ch; ch < numch; ch++, j++) {
        note2string(curpattern->note(row, ch), curpattern->instrument(row, ch), curpattern->command(row, ch, cmdpage), curpattern->commandValue(row, ch, cmdpage), buf);

        if (cursor) {
            painter.drawText(disp_startx + (j * TRACKER_CHANNEL_WIDTH) * fontw, y, buf);
        } else if (row >= rbs && row <= rbe && ch >= cbs && ch <= cbe) {
            painter.drawText(disp_startx + (j * TRACKER_CHANNEL_WIDTH) * fontw, y, buf);
        } else {
            painter.drawText(disp_startx + (j * TRACKER_CHANNEL_WIDTH) * fontw, y, buf);
        }
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
    misc_gc = colors[TRACKERCOL_BARS];
    QPainter painter(pixmap);
    painter.setBrush(misc_gc);
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

            if (song->track(i + leftchan - 1)->mute && !song->track(i + leftchan - 1)->solo) {
                misc_gc = colors[TRACKERCOL_CHANNEL_HEADER_MUTE];
            } else if (!song->track(i + leftchan - 1)->mute && song->track(i + leftchan - 1)->solo) {
                misc_gc = colors[TRACKERCOL_CHANNEL_HEADER_SOLO];
            } else if (song->track(i + leftchan - 1)->mute && song->track(i + leftchan - 1)->solo) {
                misc_gc = colors[TRACKERCOL_CHANNEL_HEADER_MUTE_SOLO];
            } else {
                misc_gc = colors[TRACKERCOL_CHANNEL_HEADER];
            }

            painter.setBrush(misc_gc);
            painter.drawText(x + 2, 0, buf);
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

    misc_gc = colors[TRACKERCOL_CURSOR];
    QPainter painter(pixmap);
    painter.setBrush(misc_gc);
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
            if (dist > 0) {
                /* go down in pattern -- scroll up */
                redrawcnt = absdist;
//                gdk_window_copy_area(win, bg_gc, 0, y, win, 0, y + (absdist * fonth), widgeallocation.width, (disp_rows - absdist) * fonth);
                y += (disp_rows - absdist) * fonth;
            } else if (dist < 0) {
                /* go up in pattern -- scroll down */
                redrawcnt = absdist;
//                gdk_window_copy_area(win, bg_gc, 0, y + (absdist * fonth), win, 0, y, widgeallocation.width, (disp_rows - absdist) * fonth);
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

//            g_signal_emit(GTK_OBJECT(t), tracker_signals[PATPOS_SIGNAL], 0, patpos, pattern->length, disp_rows);
        }
        update();
    }
}
