#include "song.h"
#include "songpropertiesdialog.h"
#include "ui_songpropertiesdialog.h"

SongPropertiesDialog::SongPropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SongPropertiesDialog),
    song(NULL)
{
    ui->setupUi(this);
}

SongPropertiesDialog::~SongPropertiesDialog()
{
    delete ui;
}

void SongPropertiesDialog::setSong(Song *song)
{
    if (this->song != song) {
        if (this->song != NULL) {
            disconnect(ui->lineEditName, SIGNAL(textChanged(QString)), this->song, SLOT(setName(QString)));
            disconnect(ui->horizontalSliderTempo, SIGNAL(valueChanged(int)), this->song, SLOT(setTempo(int)));
            disconnect(ui->horizontalSliderTicksPerLine, SIGNAL(valueChanged(int)), this->song, SLOT(setTPL(int)));
        }

        this->song = song;

        connect(ui->lineEditName, SIGNAL(textChanged(QString)), song, SLOT(setName(QString)));
        connect(ui->horizontalSliderTempo, SIGNAL(valueChanged(int)), song, SLOT(setTempo(int)));
        connect(ui->horizontalSliderTicksPerLine, SIGNAL(valueChanged(int)), song, SLOT(setTPL(int)));
        connect(ui->checkBoxSendMidiSync, SIGNAL(toggled(bool)), song, SLOT(setSendSync(bool)));

        ui->lineEditName->blockSignals(true);
        ui->lineEditName->setText(song->name());
        ui->lineEditName->blockSignals(false);
        ui->horizontalSliderTempo->blockSignals(true);
        ui->horizontalSliderTempo->setValue(song->tempo());
        ui->horizontalSliderTempo->blockSignals(false);
        ui->horizontalSliderTicksPerLine->blockSignals(true);
        ui->horizontalSliderTicksPerLine->setValue(song->ticksPerLine());
        ui->horizontalSliderTicksPerLine->blockSignals(false);
        ui->checkBoxSendMidiSync->blockSignals(true);
        ui->checkBoxSendMidiSync->setChecked(song->sendSync());
        ui->checkBoxSendMidiSync->blockSignals(false);
    }
}
