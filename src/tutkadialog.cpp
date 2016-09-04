#include <QKeyEvent>
#include "tutkadialog.h"

TutkaDialog::TutkaDialog(QWidget *parent) : QDialog(parent)
{
}

void TutkaDialog::keyPressEvent(QKeyEvent *event)
{
    event->accept();
}
