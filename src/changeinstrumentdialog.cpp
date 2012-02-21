#include "song.h"
#include "changeinstrumentdialog.h"
#include "ui_changeinstrumentdialog.h"

ChangeInstrumentDialog::ChangeInstrumentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeInstrumentDialog),
    song(NULL),
    block(0),
    track(0),
    selectionStartTrack(-1),
    selectionStartLine(-1),
    selectionEndTrack(-1),
    selectionEndLine(-1)
{
    ui->setupUi(this);

    connect(ui->pushButtonSwap, SIGNAL(clicked()), this, SLOT(swap()));
    connect(ui->pushButtonChange, SIGNAL(clicked()), this, SLOT(change()));
}

ChangeInstrumentDialog::~ChangeInstrumentDialog()
{
    delete ui;
}

void ChangeInstrumentDialog::showSong()
{
    ui->comboBoxArea->setCurrentIndex(0);
    show();
}

void ChangeInstrumentDialog::showBlock()
{
    ui->comboBoxArea->setCurrentIndex(1);
    show();
}

void ChangeInstrumentDialog::showTrack()
{
    ui->comboBoxArea->setCurrentIndex(2);
    show();
}

void ChangeInstrumentDialog::showSelection()
{
    ui->comboBoxArea->setCurrentIndex(3);
    show();
}


void ChangeInstrumentDialog::setSong(Song *song)
{
    this->song = song;
}

void ChangeInstrumentDialog::setBlock(unsigned int block)
{
    this->block = block;
}

void ChangeInstrumentDialog::setTrack(int track)
{
    this->track = track;
}

void ChangeInstrumentDialog::setSelection(int startTrack, int startLine, int endTrack, int endLine)
{
    selectionStartTrack = startTrack;
    selectionStartLine = startLine;
    selectionEndTrack = endTrack;
    selectionEndLine = endLine;
}

void ChangeInstrumentDialog::swap()
{
    change(true);
}

void ChangeInstrumentDialog::change(bool swap)
{
    int from = ui->spinBoxFrom->value();
    int to = ui->spinBoxTo->value();

    switch (ui->comboBoxArea->currentIndex()) {
    case SONG:
        song->changeInstrument(from, to, swap);
        break;
    case BLOCK: {
        Block *block = song->block(this->block);
        block->changeInstrument(from, to, swap, 0, 0, block->tracks() - 1, block->length() - 1);
        break;
    }
    case TRACK: {
        Block *block = song->block(this->block);
        block->changeInstrument(from, to, swap, track, 0, track, block->length() - 1);
        break;
    }
    case SELECTION:
        if (selectionStartTrack >= 0 && selectionStartLine >= 0 && selectionEndTrack >= 0 && selectionEndLine >= 0) {
            Block *block = song->block(this->block);
            block->changeInstrument(from, to, swap, selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
        }
        break;
    default:
        break;
    }

    if (swap) {
        song->checkInstrument(from);
    }
    song->checkInstrument(to);
}
