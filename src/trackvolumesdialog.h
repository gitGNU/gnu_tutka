#ifndef TRACKVOLUMESDIALOG_H
#define TRACKVOLUMESDIALOG_H

#include <QDialog>

namespace Ui {
    class TrackVolumesDialog;
}

class TrackVolumesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TrackVolumesDialog(QWidget *parent = 0);
    ~TrackVolumesDialog();

private:
    Ui::TrackVolumesDialog *ui;
};

#endif // TRACKVOLUMESDIALOG_H
