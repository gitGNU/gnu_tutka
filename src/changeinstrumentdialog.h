#ifndef CHANGEINSTRUMENTDIALOG_H
#define CHANGEINSTRUMENTDIALOG_H

#include <QDialog>

namespace Ui {
    class ChangeInstrumentDialog;
}

class ChangeInstrumentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeInstrumentDialog(QWidget *parent = 0);
    ~ChangeInstrumentDialog();

private:
    Ui::ChangeInstrumentDialog *ui;
};

#endif // CHANGEINSTRUMENTDIALOG_H
