#ifndef SECTIONLISTDIALOG_H
#define SECTIONLISTDIALOG_H

#include <QDialog>

namespace Ui {
    class SectionListDialog;
}

class Song;
class SectionListTableModel;
class SpinBoxDelegate;

class SectionListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SectionListDialog(QWidget *parent = 0);
    ~SectionListDialog();

public slots:
    void setSong(Song *song);
    void setSection(unsigned int section);

private slots:
    void insertSection();
    void appendSection();
    void deleteSection();

private:
    Ui::SectionListDialog *ui;
    Song *song;
    SectionListTableModel *sectionListTableModel;
    SpinBoxDelegate *spinBoxDelegate;
};

#endif // SECTIONLISTDIALOG_H
