#ifndef MESSAGELISTDIALOG_H
#define MESSAGELISTDIALOG_H

#include <QDialog>

namespace Ui {
    class MessageListDialog;
}

class MIDI;
class Song;
class MessageListTableModel;
class SpinBoxDelegate;

class MessageListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MessageListDialog(MIDI *midi, QWidget *parent = 0);
    ~MessageListDialog();

public slots:
    void setSong(Song *song);

private slots:
    void insertMessage();
    void appendMessage();
    void deleteMessage();
    void sendMessage();
    void receiveMessage();
    void loadMessage();
    void saveMessage();

private:
    Ui::MessageListDialog *ui;
    MIDI *midi;
    Song *song;
    MessageListTableModel *messageListTableModel;
    SpinBoxDelegate *spinBoxDelegate;
};

#endif // MESSAGELISTDIALOG_H
