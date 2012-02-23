#include "song.h"
#include "playseq.h"
#include "spinboxdelegate.h"
#include "playingsequencetablemodel.h"
#include "playingsequencedialog.h"
#include "ui_playingsequencedialog.h"

PlayingSequenceDialog::PlayingSequenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlayingSequenceDialog),
    song(NULL),
    playseq(NULL),
    playingSequenceTableModel(new PlayingSequenceTableModel(this)),
    spinBoxDelegate(new SpinBoxDelegate(this))
{
    ui->setupUi(this);

    ui->tableView->setModel(playingSequenceTableModel);
    ui->tableView->setItemDelegateForColumn(1, spinBoxDelegate);
    ui->tableView->setItemDelegateForColumn(2, spinBoxDelegate);
    ui->tableView->setItemDelegateForColumn(3, spinBoxDelegate);

    connect(ui->pushButtonInsert, SIGNAL(clicked()), this, SLOT(insertBlock()));
    connect(ui->pushButtonAppend, SIGNAL(clicked()), this, SLOT(appendBlock()));
    connect(ui->pushButtonDelete, SIGNAL(clicked()), this, SLOT(deleteBlock()));
}

PlayingSequenceDialog::~PlayingSequenceDialog()
{
    delete ui;
}

void PlayingSequenceDialog::setSong(Song *song)
{
    this->song = song;
    playingSequenceTableModel->setSong(song);
}

void PlayingSequenceDialog::setPlayseq(unsigned int playseq)
{
    this->playseq = song->playseq(playseq);
    playingSequenceTableModel->setPlayseq(this->playseq);
}

void PlayingSequenceDialog::setPosition(unsigned int position)
{
    ui->tableView->selectRow(position);
}

void PlayingSequenceDialog::insertBlock()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        playseq->insert(indexes.first().row());
    }
}

void PlayingSequenceDialog::appendBlock()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        playseq->insert(playseq->length());
    }
}

void PlayingSequenceDialog::deleteBlock()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        playseq->remove(indexes.first().row());
    }
}
