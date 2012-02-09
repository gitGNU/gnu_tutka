#include "song.h"
#include "changeinstrumentdialog.h"
#include "ui_changeinstrumentdialog.h"

ChangeInstrumentDialog::ChangeInstrumentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeInstrumentDialog),
    song(NULL),
    block(0),
    track(0)
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
//        if (tracker->sel_start_ch >= 0 && tracker->sel_end_ch >= 0 && tracker->sel_start_row >= 0 && tracker->sel_end_row >= 0)
//            editor_song_block_current_changeinstrument(gui->editor, from, to, 1, tracker->sel_start_ch, tracker->sel_start_row, tracker->sel_end_ch, tracker->sel_end_row);
        break;
    default:
        break;
    }

    if (swap) {
        song->checkInstrument(from);
    }
    song->checkInstrument(to);
}
