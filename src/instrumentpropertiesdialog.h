#ifndef INSTRUMENTPROPERTIESDIALOG_H
#define INSTRUMENTPROPERTIESDIALOG_H

#include <QDialog>

namespace Ui {
    class InstrumentPropertiesDialog;
}

class InstrumentPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InstrumentPropertiesDialog(QWidget *parent = 0);
    ~InstrumentPropertiesDialog();

private:
    Ui::InstrumentPropertiesDialog *ui;
};

#endif // INSTRUMENTPROPERTIESDIALOG_H
