#ifndef TUTKADIALOG_H
#define TUTKADIALOG_H

#include <QDialog>

class TutkaDialog : public QDialog
{
public:
    TutkaDialog(QWidget *parent = 0);

protected:
    virtual void keyPressEvent(QKeyEvent *event);
};

#endif // TUTKADIALOG_H
