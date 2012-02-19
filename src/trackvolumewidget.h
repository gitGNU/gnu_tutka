#ifndef TRACKVOLUMEWIDGET_H
#define TRACKVOLUMEWIDGET_H

#include <QWidget>

class Track;
class QLabel;
class QSlider;
class QCheckBox;
class QLineEdit;

class TrackVolumeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TrackVolumeWidget(unsigned int number, Track *track, QWidget *parent = 0);

private slots:
    void updateVolumeLabel(int volume);

private:
    unsigned int trackNumber;
    Track *track;
    QLabel *volumeLabel;
    QSlider *volumeSlider;
    QCheckBox *muteCheckBox;
    QCheckBox *soloCheckBox;
    QLineEdit *nameLineEdit;
};

#endif // TRACKVOLUMEWIDGET_H
