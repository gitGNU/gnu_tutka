#include "song.h"
#include "playingsequencelisttablemodel.h"
#include "playingsequencelistdialog.h"
#include "ui_playingsequencelistdialog.h"

PlayingSequenceListDialog::PlayingSequenceListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlayingSequenceListDialog),
    song(NULL),
    playingSequenceListTableModel(new PlayingSequenceListTableModel(this))
{
    ui->setupUi(this);

    ui->tableView->setModel(playingSequenceListTableModel);

    connect(ui->pushButtonInsertNew, SIGNAL(clicked()), this, SLOT(insertPlayingSequence()));
    connect(ui->pushButtonAppendNew, SIGNAL(clicked()), this, SLOT(appendPlayingSequence()));
    connect(ui->pushButtonDelete, SIGNAL(clicked()), this, SLOT(deletePlayingSequence()));
}

PlayingSequenceListDialog::~PlayingSequenceListDialog()
{
    delete ui;
}

void PlayingSequenceListDialog::setSong(Song *song)
{
    this->song = song;
    playingSequenceListTableModel->setSong(song);
}

void PlayingSequenceListDialog::setPlayingSequence(unsigned int playingSequence)
{
    ui->tableView->selectRow(playingSequence);
}

void PlayingSequenceListDialog::insertPlayingSequence()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertPlayseq(indexes.first().row());
    }
}

void PlayingSequenceListDialog::appendPlayingSequence()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertPlayseq(song->playseqs());
    }
}

void PlayingSequenceListDialog::deletePlayingSequence()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->deletePlayseq(indexes.first().row());
    }
}
