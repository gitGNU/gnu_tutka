#include "song.h"
#include "expandshrinkdialog.h"
#include "ui_expandshrinkdialog.h"

ExpandShrinkDialog::ExpandShrinkDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExpandShrinkDialog),
    song(NULL),
    block(0),
    track(0),
    selectionStartTrack(-1),
    selectionStartLine(-1),
    selectionEndTrack(-1),
    selectionEndLine(-1)
{
    ui->setupUi(this);

    connect(ui->pushButtonExpand, SIGNAL(clicked()), this, SLOT(expand()));
    connect(ui->pushButtonShrink, SIGNAL(clicked()), this, SLOT(shrink()));
}

ExpandShrinkDialog::~ExpandShrinkDialog()
{
    delete ui;
}

void ExpandShrinkDialog::showSong()
{
    ui->comboBoxArea->setCurrentIndex(0);
    show();
}

void ExpandShrinkDialog::showBlock()
{
    ui->comboBoxArea->setCurrentIndex(1);
    show();
}

void ExpandShrinkDialog::showTrack()
{
    ui->comboBoxArea->setCurrentIndex(2);
    show();
}

void ExpandShrinkDialog::showSelection()
{
    ui->comboBoxArea->setCurrentIndex(3);
    show();
}

void ExpandShrinkDialog::setSong(Song *song)
{
    this->song = song;
}

void ExpandShrinkDialog::setBlock(unsigned int block)
{
    this->block = block;
}

void ExpandShrinkDialog::setTrack(int track)
{
    this->track = track;
}

void ExpandShrinkDialog::setSelection(int startTrack, int startLine, int endTrack, int endLine)
{
    selectionStartTrack = startTrack;
    selectionStartLine = startLine;
    selectionEndTrack = endTrack;
    selectionEndLine = endLine;
}

void ExpandShrinkDialog::expand()
{
    expandShrink(false);
}

void ExpandShrinkDialog::shrink()
{
    expandShrink(true);
}

void ExpandShrinkDialog::expandShrink(bool shrink)
{
    int factor = ui->spinBoxFactor->value();

    if (factor < 2) {
        return;
    }

    if (shrink) {
        factor = -factor;
    }

    switch (ui->comboBoxArea->currentIndex()) {
    case SONG:
        song->expandShrink(factor);
        break;
    case BLOCK: {
        Block *block = song->block(this->block);
        block->expandShrink(factor, 0, 0, block->tracks() - 1, block->length() - 1);
        break;
    }
    case TRACK: {
        Block *block = song->block(this->block);
        block->expandShrink(factor, track, 0, track, block->length() - 1);
        break;
    }
    case SELECTION:
        if (selectionStartTrack >= 0 && selectionStartLine >= 0 && selectionEndTrack >= 0 && selectionEndLine >= 0) {
            Block *block = song->block(this->block);
            block->expandShrink(factor, selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
        }
        break;
    default:
        break;
    }
}
