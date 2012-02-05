#include "playingsequencedialog.h"
#include "ui_playingsequencedialog.h"

PlayingSequenceDialog::PlayingSequenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlayingSequenceDialog)
{
    ui->setupUi(this);
}

PlayingSequenceDialog::~PlayingSequenceDialog()
{
    delete ui;
}
