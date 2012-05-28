/*
 * player.h
 *
 * Copyright 2002-2012 Vesa Halttunen
 *
 * This file is part of Tutka.
 *
 * Tutka is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tutka is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tutka; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QVector>
#include <QSharedPointer>

class Song;
class Block;
class MIDI;
class Scheduler;

class Player : public QThread {
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
    enum Command {
        CommandPreviousCommandValue = 0x00,
        CommandPitchWheel = 0x01,
        CommandEndBlock = 0x02,
        CommandPlayseqPosition = 0x03,
        CommandProgramChange = 0x07,
        CommandSendMessage = 0x08,
        CommandHold = 0x09,
        CommandRetrigger = 0x0a,
        CommandDelay = 0x0b,
        CommandVelocity = 0x0c,
        CommandChannelPressure = 0x0d,
        CommandTicksPerLine = 0x0e,
        CommandTempo = 0x0f,
        CommandNotDefined = 0x10,
        CommandMidiControllers = 0x80
    };

    // Player mode
    enum Mode {
        ModeIdle,
        ModePlaySong,
        ModePlayBlock
    };

    enum ExternalSync {
        Off,
        Midi
    };

    // Creates a player
    Player(MIDI *midi, const QString &path = QString(), QObject *parent = NULL);
    Player(MIDI *midi, Song *song, QObject *parent = NULL);
    // Closes a player
    virtual ~Player();

    unsigned int section() const;
    unsigned int playseq() const;
    unsigned int position() const;
    unsigned int block() const;
    unsigned int line() const;
    Mode mode() const;

    // Plays a song without any scheduling (for export)
    void playWithoutScheduling();

    // Plays a note using given instrument on a given channel
    void playNote(unsigned int, unsigned char, unsigned char, unsigned char);
    // Stops notes playing on muted tracks
    void stopMuted();
    // Stops notes playing at the moment
    void stopNotes();
    // Handles a command
    void handleCommand(QSharedPointer<TrackStatus>, unsigned char, unsigned char, unsigned char, unsigned char, unsigned int *, int *, int *);
    // Resets the player time
    void resetTime(bool);

    // Set player position
    void setLine(int);
    void setTick(int);

    // Lock the player
    void lock();
    // Unlock the player
    void unlock();

    // Set the scheduler of a player
    void setExternalSync(ExternalSync externalSync);
    void setScheduler(Scheduler *scheduler);

    MIDI *midi() const;

public slots:
    void playSong();
    void playBlock();
    void continueSong();
    void continueBlock();
    // Kills the player thread
    void stop();
    // Stops all notes
    void stopAllNotes();
    // Resets the pitch wheel on all channels
    void resetPitch();
    void setSong(const QString &path = QString());
    // A method to notify the player about an incoming sync signal
    void externalSync(unsigned int ticks = 1);

    // Set player position
    void setSection(int);
    void setPlayseq(int);
    void setPosition(int);
    void setBlock(int);

private slots:
    // Initializes the player
    void init();

    // Reallocate track status array
    void trackStatusCreate();

    // Refreshes playseq from section and block from position
    void updateLocation(bool blocksChanged = false);

    // Refreshes playseq from section and block from position taking into account that the number of blocks has changed
    void updateLocation(int blocks);

    // Notifies the player that MIDI interfaces have changed
    void remapMidiOutputs();

    // Checks whether some tracks are soloed or not
    void checkSolo();

signals:
    void songChanged(Song *song);
    void sectionChanged(unsigned int section);
    void playseqChanged(unsigned int playseq);
    void positionChanged(unsigned int position);
    void blockChanged(unsigned int block);
    void lineChanged(int line);
    void modeChanged(Player::Mode mode);
    void timeChanged(unsigned int time);
    void locationUpdated();

protected:
    virtual void run();

private:
    // 128 MIDI controllers plus aftertouch, channel pressure and pitch wheel
    enum Values {
      VALUES = (128 + 3),
      VALUES_AFTERTOUCH = 128,
      VALUES_CHANNEL_PRESSURE = 129,
      VALUES_PITCH_WHEEL = 130
    };

    // Starts the player thread
    void play(Mode, bool);

    // Advances in section and jumps to the beginning if necessary
    bool nextSection();

    // Advances in playing sequence and jumps to next section if necessary
    bool nextPosition();

    // Current location in song
    unsigned int section_, playseq_, position_, block_, line_, tick;
    // The song currently being played
    Song *song;
    // The previous song being destroyed
    Song *oldSong;
    // Player mode
    Mode mode_;
    // Player scheduling mode
    Scheduler *scheduler;
    ExternalSync syncMode;
    // Status of tracks; notes playing
    QList<QSharedPointer<TrackStatus> > trackStatuses;
    // MIDI controller values; one for each controller on each channel
    QList<QVector<unsigned char> > midiControllerValues;
    // For measuring how long the song has been playing
    struct timeval playingStarted, playedSoFar;
    // Ticks passed after playing started
    unsigned int ticksSoFar;
    // Mutex for the player thread
    QMutex mutex;
    // Cond for external sync
    QWaitCondition externalSync_;
    // External sync tick count
    int externalSyncTicks;
    // Kill player flag (the mutex must be used when accessing)
    bool killThread;
    // MIDI subsystem
    MIDI *midi_;
    // Indicates whether some tracks are soloed or not
    unsigned int solo;
    // The command to be executed after the current line
    unsigned char postCommand, postValue;
    // Indicates whether the tempo has changed
    bool tempoChanged;
};

#endif // PLAYER_H_
