#ifndef MESSAGELISTDIALOG_H
#define MESSAGELISTDIALOG_H

#include <QDialog>

namespace Ui {
    class MessageListDialog;
}

class MessageListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MessageListDialog(QWidget *parent = 0);
    ~MessageListDialog();

private:
    Ui::MessageListDialog *ui;
};

#endif // MESSAGELISTDIALOG_H
