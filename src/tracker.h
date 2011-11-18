#ifndef TRACKER_H
#define TRACKER_H

#include <QPixmap>
#include <QWidget>

class Song;
class Block;

#define TRACKER_CHANNEL_WIDTH 13

enum {
  TRACKERCOL_BG,
  TRACKERCOL_BG_CURSOR,
  TRACKERCOL_BG_SELECTION,
  TRACKERCOL_NOTES,
  TRACKERCOL_BARS,
  TRACKERCOL_CHANNEL_HEADER,
  TRACKERCOL_CHANNEL_HEADER_MUTE,
  TRACKERCOL_CHANNEL_HEADER_SOLO,
  TRACKERCOL_CHANNEL_HEADER_MUTE_SOLO,
  TRACKERCOL_CURSOR,
  TRACKERCOL_LAST
};

class Tracker : public QWidget
{
    Q_OBJECT
public:
    explicit Tracker(QWidget *parent = 0);
    void setNumChannels(int n);
    void setCmdpage(int cmdpage);
    void setPatpos(int row);
    void redraw();
    void redrawRow(int row);
    void redrawCurrentRow();
    void setSong(Song *song);
    void setPattern(Block *pattern);
    void setXpanning(int left_channel);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void paintEvent(QPaintEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void showEvent(QShowEvent *event);

private:
    void adjustXpanning();
    void stepCursorItem(int direction);
    void stepCursorChannel(int direction);
    void stepCursorRow(int direction);
    void markSelection(bool enable);
    void clearMarkSelection();
    bool isInSelectionMode();
    void getSelectionRect(int *chStart, int *rowStart, int *nChannel, int *nRows);
    bool isValidSelection();
    void note2string(unsigned char note, unsigned char instrument, unsigned char effect, unsigned char value, char *buf);
    void clearNotesLine(int y, int pattern_row);
    void printNotesLine(int y, int ch, int numch, int row, int cursor);
    void printNotes(int x, int y, int w, int h, int cursor_row, bool enable_cursor);
    void printBars();
    void printChannelHeaders();
    void printCursor();
    void drawClever(const QRect &area);
    void drawStupid(const QRect &area);
    void initDisplay(int width, int height);
    void reset();
    void initColors();
    void calculateFontSize();
    bool setFont(const QString &fontname);
    // If selecting, mouse is used to select in pattern
    void mouseToCursorPos(int x, int y, int *cursor_ch, int *cursor_item, int *patpos);

    int disp_rows;
    int disp_starty;
    int disp_numchans;
    int disp_startx;
    int disp_chanwidth;
    int disp_cursor;

    QFont fontdesc;
    int fontw, fonth, fontc;

    QBrush bg_gc, bg_cursor_gc, notes_gc, misc_gc;
    QColor colors[TRACKERCOL_LAST];
    QPixmap *pixmap;
    uint idle_handler;

    Song *song;
    Block *curpattern;
    int cmdpage;
    int patpos, oldpos;
    int num_channels;

    int cursor_ch, cursor_item;
    int leftchan;

    /* Block selection stuff */
    bool inSelMode;
    int sel_start_ch, sel_start_row;
    int sel_end_ch, sel_end_row;

    int mouse_selecting;
    int button;

//    GdkPixmap **fonts_normal;
//    GdkPixmap **fonts_cursor;
//    GdkPixmap **fonts_select;
};

#endif // TRACKER_H
