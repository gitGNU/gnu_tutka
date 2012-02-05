#include "playingsequencelistdialog.h"
#include "ui_playingsequencelistdialog.h"

PlayingSequenceListDialog::PlayingSequenceListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlayingSequenceListDialog)
{
    ui->setupUi(this);
}

PlayingSequenceListDialog::~PlayingSequenceListDialog()
{
    delete ui;
}
