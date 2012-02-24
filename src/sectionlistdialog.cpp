#include "song.h"
#include "spinboxdelegate.h"
#include "sectionlisttablemodel.h"
#include "sectionlistdialog.h"
#include "ui_sectionlistdialog.h"

SectionListDialog::SectionListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SectionListDialog),
    song(NULL),
    sectionTableModel(new SectionListTableModel(this)),
    spinBoxDelegate(new SpinBoxDelegate(this))
{
    ui->setupUi(this);

    ui->tableView->setModel(sectionTableModel);
    ui->tableView->setItemDelegateForColumn(1, spinBoxDelegate);
    ui->tableView->setItemDelegateForColumn(2, spinBoxDelegate);
    ui->tableView->setItemDelegateForColumn(3, spinBoxDelegate);

    connect(ui->pushButtonInsert, SIGNAL(clicked()), this, SLOT(insertSection()));
    connect(ui->pushButtonAppend, SIGNAL(clicked()), this, SLOT(appendSection()));
    connect(ui->pushButtonDelete, SIGNAL(clicked()), this, SLOT(deleteSection()));
}

SectionListDialog::~SectionListDialog()
{
    delete ui;
}

void SectionListDialog::setSong(Song *song)
{
    this->song = song;
    sectionTableModel->setSong(song);
}

void SectionListDialog::setSection(unsigned int section)
{
    ui->tableView->selectRow(section);
}

void SectionListDialog::insertSection()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertSection(indexes.first().row());
    }
}

void SectionListDialog::appendSection()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertSection(song->sections());
    }
}

void SectionListDialog::deleteSection()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->deleteSection(indexes.first().row());
    }
}
