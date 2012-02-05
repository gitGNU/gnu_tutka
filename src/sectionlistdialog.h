#ifndef SECTIONLISTDIALOG_H
#define SECTIONLISTDIALOG_H

#include <QDialog>

namespace Ui {
    class SectionListDialog;
}

class SectionListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SectionListDialog(QWidget *parent = 0);
    ~SectionListDialog();

private:
    Ui::SectionListDialog *ui;
};

#endif // SECTIONLISTDIALOG_H
