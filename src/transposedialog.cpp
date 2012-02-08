#include "song.h"
#include "transposedialog.h"
#include "ui_transposedialog.h"

TransposeDialog::TransposeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransposeDialog),
    song(NULL),
    block(0),
    track(0),
    instrument(0)
{
    ui->setupUi(this);

    connect(ui->pushButtonTranspose, SIGNAL(clicked()), this, SLOT(transpose()));
}

TransposeDialog::~TransposeDialog()
{
    delete ui;
}

void TransposeDialog::showSong()
{
    ui->comboBoxArea->setCurrentIndex(0);
    show();
}

void TransposeDialog::showBlock()
{
    ui->comboBoxArea->setCurrentIndex(1);
    show();
}

void TransposeDialog::showTrack()
{
    ui->comboBoxArea->setCurrentIndex(2);
    show();
}

void TransposeDialog::showSelection()
{
    ui->comboBoxArea->setCurrentIndex(3);
    show();
}

void TransposeDialog::setSong(Song *song)
{
    this->song = song;
}

void TransposeDialog::setBlock(unsigned int block)
{
    this->block = block;
}

void TransposeDialog::setTrack(int track)
{
    this->track = track;
}

void TransposeDialog::setInstrument(int instrument)
{
    this->instrument = instrument;
}

void TransposeDialog::transpose()
{
    int halfNotes = 0;

    switch (ui->comboBoxMode->currentIndex()) {
    case 0:
        halfNotes = 12;
        break;
    case 1:
        halfNotes = 1;
        break;
    case 2:
        halfNotes = -1;
        break;
    case 3:
        halfNotes = -12;
        break;
    default:
        break;
    }

    int instrument = ui->comboBoxInstruments->currentIndex() == 1 ? this->instrument : -1;

    switch (ui->comboBoxArea->currentIndex()) {
    case SONG:
        song->transpose(instrument, halfNotes);
        break;
    case BLOCK: {
        Block *block = song->block(this->block);
        block->transpose(instrument, halfNotes, 0, 0, block->tracks() - 1, block->length() - 1);
        break;
    }
    case TRACK: {
        Block *block = song->block(this->block);
        block->transpose(instrument, halfNotes, track, 0, track, block->length() - 1);
        break;
    }
    case SELECTION:
        // TODO
//        if (tracker->sel_start_ch >= 0 && tracker->sel_end_ch >= 0 && tracker->sel_start_row >= 0 && tracker->sel_end_row >= 0)
//            editor_song_block_current_transpose(instrument, halfNotes, tracker->sel_start_ch, tracker->sel_start_row, tracker->sel_end_ch, tracker->sel_end_row);
        break;
    default:
        break;
    }
}
