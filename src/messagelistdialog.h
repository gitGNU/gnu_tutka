#ifndef MESSAGELISTDIALOG_H
#define MESSAGELISTDIALOG_H

#include <QDialog>

namespace Ui {
    class MessageListDialog;
}

class Song;
class MessageListTableModel;
class SpinBoxDelegate;
class CheckBoxDelegate;

class MessageListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MessageListDialog(QWidget *parent = 0);
    ~MessageListDialog();

public slots:
    void setSong(Song *song);

private slots:
    void insertMessage();
    void appendMessage();
    void deleteMessage();

private:
    Ui::MessageListDialog *ui;
    Song *song;
    MessageListTableModel *messageListTableModel;
    SpinBoxDelegate *spinBoxDelegate;
    CheckBoxDelegate *checkBoxDelegate;
};

#endif // MESSAGELISTDIALOG_H
