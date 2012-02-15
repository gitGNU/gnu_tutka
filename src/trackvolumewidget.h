#ifndef TRACKVOLUMEWIDGET_H
#define TRACKVOLUMEWIDGET_H

#include <QWidget>

class Track;
class QSlider;
class QCheckBox;
class QLineEdit;

class TrackVolumeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TrackVolumeWidget(unsigned int number, Track *track, QWidget *parent = 0);

private:
    unsigned int trackNumber;
    Track *track;
    QSlider *volumeSlider;
    QCheckBox *muteCheckBox;
    QCheckBox *soloCheckBox;
    QLineEdit *nameLineEdit;
};

#endif // TRACKVOLUMEWIDGET_H
