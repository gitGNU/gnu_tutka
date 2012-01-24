/*
 * player.h
 *
 * Copyright 2002-2011 vesuri
 *
 * This file is part of Tutka2.
 *
 * Tutka2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tutka2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tutka2; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QVector>
#include <QSharedPointer>

class Song;
class Block;
class MIDI;

class Player : public QObject {
    Q_OBJECT

    // Track status values
    class TrackStatus {
    private:
      char baseNote;
      char instrument;
      char line;
      char previousCommand;
      char midiInterface;
      char midiChannel;
      char volume;
      char note;
      char hold;

      friend class Player;
    };
public:
    // Player mode
    enum Mode {
      IDLE,
      PLAY_SONG,
      PLAY_BLOCK
    };

    enum Command {
      COMMAND_PREVIOUS_COMMAND_VALUE = 0x00,
      COMMAND_PITCH_WHEEL = 0x01,
      COMMAND_END_BLOCK = 0x02,
      COMMAND_PLAYSEQ_POSITION = 0x03,
      COMMAND_PROGRAM_CHANGE = 0x07,
      COMMAND_SEND_MESSAGE = 0x08,
      COMMAND_HOLD = 0x09,
      COMMAND_RETRIGGER = 0x0a,
      COMMAND_DELAY = 0x0b,
      COMMAND_VELOCITY = 0x0c,
      COMMAND_CHANNEL_PRESSURE = 0x0d,
      COMMAND_TPL = 0x0e,
      COMMAND_TEMPO = 0x0f,
      COMMAND_NOT_DEFINED = 0x10,
      COMMAND_MIDI_CONTROLLERS = 0x80
    };

    Player(MIDI *midi, QObject *parent = NULL);
    virtual ~Player();

    void setSong(Song *song);
    int line() const;
    int position() const;
    int section() const;
    int block() const;
    Mode mode() const;

    // Plays a song to a MIDI interface witout any scheduling (for export)
//    void midi_export(struct song *, struct midi_interface *);
    // Creates a player for a song
//    struct player *open(struct song *, struct editor *, struct midi *);
    // Closes a player for a song
//    void close();

    // Starts the player thread
    void start(Mode, int, int, int, bool);
    // Kills the player thread
    void stop();

    // Plays a note using given instrument on a given channel
    void playNote(unsigned int, unsigned char, unsigned char, unsigned char);
    // Stops notes playing on muted tracks
    void stopMuted();
    // Stops notes playing at the moment
    void stopNotes();
    // Stops all notes
    void stopAllNotes();
    // Resets the pitch wheel on all channels
    void resetPitch();
    // Handles a command
    void handleCommand(QSharedPointer<TrackStatus>, unsigned char, unsigned char, unsigned char, unsigned char, unsigned int *, int *, int *);
    // Reallocate track status array
    void trackStatusCreate();
    // Resets the player time
    void resetTime(bool);

    // Set player position
    void setSection(int);
    void setPlayseq(int);
    void setPosition(int);
    void setBlock(int);
    void setLine(int);
    void setTick(int);

    // Sets whether some tracks are considered soloed or not
    void setSolo(unsigned int);
    // Checks whether some tracks are soloed or not
    void checkSolo();
    // Notifies the player that MIDI interfaces have changed
    void midiChanged();

    // Lock the player
    void lock();
    // Unlock the player
    void unlock();

    // A method to notify the player about an incoming sync signal
    void externalSync(unsigned int);

    // Set the scheduler of a player
    void setScheduler(unsigned int);

signals:
    void songChanged(Song *song);
    void blockChanged(Block *block);

private:
    // 128 MIDI controllers plus aftertouch, channel pressure and pitch wheel
    enum Values {
      VALUES = (128 + 3),
      VALUES_AFTERTOUCH = 128,
      VALUES_CHANNEL_PRESSURE = 129,
      VALUES_PITCH_WHEEL = 130
    };

    enum Scheduling {
      SCHED_NONE,
      SCHED_RTC,
      SCHED_NANOSLEEP,
      SCHED_EXTERNAL_SYNC
    };

    // Refreshes playseq from section and block from position
    void refreshPlayseqAndBlock();
    // Advances in section and jumps to the beginning if necessary
    bool nextSection();
    // Advances in playing sequence and jumps to next section if necessary
    bool nextPlayseq();

    void thread();

    // Current location in song
    unsigned int section_, playseq_, position_, block_, line_, tick;
    // The song currently being played
    Song *song;
    // Player mode
    Mode mode_;
    // Player scheduling mode
    unsigned int sched;
    // Status of tracks; notes playing
    QList<QSharedPointer<TrackStatus> > trackStatus;
    // MIDI controller values; one for each controller on each channel
    QList<QVector<unsigned char> > midiControllerValues;
    // For measuring how long the song has been playing
    struct timeval playingStarted, playedSoFar;
    // Ticks passed after playing started
    unsigned int ticksSoFar;
    // Player thread pointer
    QThread *thread_;
    // Mutex for the player thread
    QMutex mutex;
    // Cond for external sync
    QWaitCondition externalSync_;
    // External sync tick count
    int externalSyncTicks;
    // Kill player flag (the mutex must be used when accessing)
    bool killThread;
    // MIDI subsystem
    MIDI *midi;
    // RTC device file descriptor
    int rtc;
    // Obtained RTC frequency
    int rtcFrequency;
    // RTC periodic interrupts enabled
    bool rtcPIE;
    // Indicates whether some tracks are soloed or not
    unsigned int solo;
    // The command to be executed after the current line
    unsigned char postCommand, postValue;
    // Indicates whether the tempo has changed
    bool tempoChanged;
};

#endif // PLAYER_H_
