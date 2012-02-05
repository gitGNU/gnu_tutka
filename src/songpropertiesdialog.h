#ifndef SONGPROPERTIESDIALOG_H
#define SONGPROPERTIESDIALOG_H

#include <QDialog>

namespace Ui {
    class SongPropertiesDialog;
}

class SongPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SongPropertiesDialog(QWidget *parent = 0);
    ~SongPropertiesDialog();

private:
    Ui::SongPropertiesDialog *ui;
};

#endif // SONGPROPERTIESDIALOG_H
