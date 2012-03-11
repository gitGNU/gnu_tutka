/*
 * player.cpp
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

#include <cstddef>
#include <cstdlib>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#ifdef __linux
#include <linux/rtc.h>
#endif
#include <QThread>
#include <QTimer>
#include <QFile>
#include "song.h"
#include "track.h"
#include "instrument.h"
#include "midiinterface.h"
#include "midi.h"
#include "mmd.h"
#include "conversion.h"
#include "player.h"

#define MAXIMUM_RTC_FREQ 2048
#define MINIMUM_RTC_FREQ 512

Player::Player(MIDI *midi, const QString &path, QObject *parent) :
    QThread(parent),
    section_(0),
    playseq_(0),
    position_(0),
    block_(0),
    line_(0),
    tick(0),
    song(new Song(path)),
    mode_(ModeIdle),
    sched(SchedulingNone),
    syncMode(Off),
    ticksSoFar(0),
    externalSyncTicks(0),
    killThread(false),
    midi_(midi),
    rtc(-1),
    rtcFrequency(0),
    rtcPIE(false),
    solo(0),
    postCommand(0),
    postValue(0)
{
    connect(midi, SIGNAL(outputsChanged()), this, SLOT(remapMidiOutputs()));

    QTimer::singleShot(0, this, SLOT(init()));
}

Player::Player(MIDI *midi, Song *song, QObject *parent) :
    QThread(parent),
    section_(0),
    playseq_(0),
    position_(0),
    block_(0),
    line_(0),
    tick(0),
    song(song),
    mode_(ModeIdle),
    sched(SchedulingNone),
    syncMode(Off),
    ticksSoFar(0),
    externalSyncTicks(0),
    killThread(false),
    midi_(midi),
    rtc(-1),
    rtcFrequency(0),
    rtcPIE(false),
    solo(0),
    postCommand(0),
    postValue(0)
{
    connect(midi, SIGNAL(outputsChanged()), this, SLOT(remapMidiOutputs()));

    QTimer::singleShot(0, this, SLOT(init()));
}

Player::~Player()
{
    // Stop the player
    stop();

    // Close RTC
    if (rtc != -1) {
        close(rtc);
    }
}

void Player::refreshPlayseqAndBlock()
{
    unsigned int oldSection = section_;
    unsigned int oldPlayseq = playseq_;
    unsigned int oldPosition = position_;
    unsigned int oldBlock = block_;

    if (section_ >= song->sections()) {
        section_ = 0;
    }
    playseq_ = song->section(section_);

    if (position_ >= song->playseq(playseq_)->length()) {
        position_ = 0;
    }

    block_ = song->playseq(playseq_)->at(position_);

    if (section_ != oldSection) {
        emit sectionChanged(section_);
    }
    if (playseq_ != oldPlayseq) {
        emit playseqChanged(playseq_);
    }
    if (position_ != oldPosition) {
        emit positionChanged(position_);
    }
    if (block_ != oldBlock) {
        emit blockChanged(block_);
    }
}

bool Player::nextSection()
{
    unsigned int oldSection = section_;
    section_++;

    bool looped = section_ >= song->sections();
    if (looped) {
        section_ = 0;
    }

    if (section_ != oldSection) {
        emit sectionChanged(section_);
    }

    return looped;
}

bool Player::nextPosition()
{
    unsigned int oldPosition = position_;
    position_++;

    bool looped = position_ >= song->playseq(playseq_)->length();
    if (looped) {
        position_ = 0;
    }

    if (position_ != oldPosition) {
        emit positionChanged(position_);
    }

    return looped ? nextSection() : false;
}

void Player::playNote(unsigned int instrumentNumber, unsigned char note, unsigned char volume, unsigned char track)
{
    // Notes are played if the track is not muted and no tracks are soloed or the current track is soloed
    if (!song->track(track)->isMuted() && (!solo || (solo && song->track(track)->isSolo()))) {
        QSharedPointer<TrackStatus> trackStatus = this->trackStatus[track];

        // Stop currently playing note
        if (trackStatus->note != -1) {
            midi_->output(trackStatus->midiInterface)->noteOff(trackStatus->midiChannel, trackStatus->note, 127);
            trackStatus->note = -1;
        }

        // Don't play a note if the instrument does not exist
        Instrument *instrument = song->instrument(instrumentNumber);
        if (instrument != NULL) {
            trackStatus->instrument = instrumentNumber;

            // Update track status for the selected output
            trackStatus->note = note + instrument->transpose();
            trackStatus->midiChannel = instrument->midiChannel();
            trackStatus->midiInterface = instrument->midiInterface();
            trackStatus->volume = instrument->defaultVelocity() * volume / 127 * song->track(track)->volume() / 127 * song->masterVolume() / 127;
            trackStatus->hold = instrument->hold() > 0 ? instrument->hold() : -1;

            // Make sure the volume isn't too large
            if (trackStatus->volume < 0) {
                trackStatus->volume = 127;
            }

            // Play note
            midi_->output(instrument->midiInterface())->noteOn(trackStatus->midiChannel, trackStatus->note, trackStatus->volume);
        }
    }
}

void Player::stopMuted()
{
    for (int track = 0; track < song->maxTracks(); track++) {
        if (song->track(track)->isMuted() || (solo && !song->track(track)->isSolo())) {
            QSharedPointer<TrackStatus> trackStatus = this->trackStatus[track];
            if (trackStatus->note != -1) {
                midi_->output(trackStatus->midiInterface)->noteOff(trackStatus->midiChannel, trackStatus->note, 127);
                trackStatus->note = -1;
            }
            trackStatus->baseNote = -1;
        }
    }
}

void Player::stopNotes()
{
    for (int track = 0; track < song->maxTracks(); track++) {
        QSharedPointer<TrackStatus> trackStatus = this->trackStatus[track];
        if (trackStatus->note != -1) {
            midi_->output(trackStatus->midiInterface)->noteOff(trackStatus->midiChannel, trackStatus->note, 127);

            trackStatus->midiChannel = -1;
            trackStatus->note = -1;
            trackStatus->volume = -1;
            trackStatus->hold = -1;
        }
        trackStatus->baseNote = -1;
    }
}

void Player::stopAllNotes()
{
    for (int midiChannel = 0; midiChannel < 16; midiChannel++) {
        for (int note = 0; note < 128; note++) {
            for (int output = 0; output < midi_->outputs(); output++) {
                midi_->output(output)->noteOff(midiChannel, note, 127);
            }
        }
    }
}

void Player::resetPitch()
{
    for (int midiChannel = 0; midiChannel < 16; midiChannel++) {
        for (int output = 0; output < midi_->outputs(); output++) {
            midi_->output(output)->pitchWheel(midiChannel, 64);
        }
    }
}

void Player::handleCommand(QSharedPointer<TrackStatus> trackStatus, unsigned char note, unsigned char instrument, unsigned char command, unsigned char value, unsigned int *volume, int *delay, int *hold)
{
    if (command == 0 && value == 0) {
        return;
    }

    int midiInterface;
    int midiChannel;

    // Check which MIDI interface/channel pairs the command will affect
    if (instrument != 0) {
        // Instrument number defines MIDI interfaces/channels
        midiInterface = song->instrument(instrument - 1)->midiInterface();
        midiChannel = song->instrument(instrument - 1)->midiChannel();
    } else {
        // Note playing defines MIDI interfaces/channels
        midiInterface = trackStatus->midiInterface;
        midiChannel = trackStatus->midiChannel;
    }

    QSharedPointer<MIDIInterface> output = midi_->output(midiInterface);

    // Check for previous command if any
    if (command == CommandPreviousCommandValue) {
        if (value != 0) {
            command = trackStatus->previousCommand;
        }
    } else {
        trackStatus->previousCommand = command;
    }

    switch (command) {
    case CommandPitchWheel:
        // Program change can be sent if the MIDI channel is known
        if (midiChannel != -1) {
            if (value < 0x80) {
                if (tick == 0) {
                    output->pitchWheel(midiChannel, value);
                    midiControllerValues[midiInterface][midiChannel * VALUES + VALUES_PITCH_WHEEL] = value;
                }
            } else {
                if (tick < song->ticksPerLine() - 1) {
                    int delta = (value - 0x80 - midiControllerValues[midiInterface][midiChannel * VALUES + VALUES_PITCH_WHEEL]) / ((int)song->ticksPerLine() - 1);

                    output->pitchWheel(midiChannel, midiControllerValues[midiInterface][midiChannel * VALUES + VALUES_PITCH_WHEEL] + tick * delta);
                } else {
                    output->pitchWheel(midiChannel, value - 0x80);
                    midiControllerValues[midiInterface][midiChannel * VALUES + VALUES_PITCH_WHEEL] = value - 0x80;
                }
            }
        }
        break;
    case CommandProgramChange:
        // Pitch wheel can be set if the MIDI channel is known
        if (midiChannel != -1 && tick == 0) {
            output->programChange(midiChannel, value & 0x7f);
        }
        break;
    case CommandEndBlock:
        // Only on last tick
        if (tick == song->ticksPerLine() - 1) {
            postCommand = CommandEndBlock;
            postValue = value;
        }
        break;
    case CommandPlayseqPosition:
        // Only on last tick
        if (tick == song->ticksPerLine() - 1) {
            postCommand = CommandPlayseqPosition;
            postValue = value;
        }
        break;
    case CommandSendMessage:
        // Only on first tick
        if (tick == 0 && value < song->messages()) {
            output->writeRaw(song->message(value)->data());
        }
        break;
    case CommandHold:
        *hold = value;
        break;
    case CommandRetrigger:
        *delay = -value;
        break;
    case CommandDelay:
        *delay = value;
        break;
    case CommandVelocity:
        if (note != 0) {
            *volume = value;
            if (midiChannel != -1) {
                midiControllerValues[midiInterface][midiChannel * VALUES + VALUES_AFTERTOUCH] = value;
            }
        } else {
            // Note playing defines MIDI channel
            midiChannel = trackStatus->midiChannel;

            if (midiChannel != -1) {
                if (value < 0x80) {
                    if (tick == 0) {
                        if (value > 0) {
                            output->aftertouch(midiChannel, trackStatus->note, value);
                            midiControllerValues[midiInterface][midiChannel * VALUES + VALUES_AFTERTOUCH] = value;
                        } else {
                            output->noteOff(midiChannel, trackStatus->note, 127);
                            trackStatus->note = -1;
                        }
                    }
                } else {
                    if (tick < song->ticksPerLine() - 1) {
                        int delta = (value - 0x80 - midiControllerValues[midiInterface][midiChannel * VALUES + VALUES_AFTERTOUCH]) / ((int)song->ticksPerLine() - 1);

                        output->aftertouch(midiChannel, trackStatus->note, midiControllerValues[midiInterface][midiChannel * VALUES + VALUES_AFTERTOUCH] + tick * delta);
                    } else {
                        output->aftertouch(midiChannel, trackStatus->note, value - 0x80);
                        midiControllerValues[midiInterface][midiChannel * VALUES + VALUES_AFTERTOUCH] = value - 0x80;
                    }
                }
            }
        }
        break;
    case CommandChannelPressure:
        // Channel pressure can be set if the MIDI channel is known
        if (midiChannel != -1) {
            if (value < 0x80) {
                if (tick == 0) {
                    output->channelPressure(midiChannel, value);
                    midiControllerValues[midiInterface][midiChannel * VALUES + VALUES_CHANNEL_PRESSURE] = value;
                }
            } else {
                if (tick < song->ticksPerLine() - 1) {
                    int delta = (value - 0x80 - midiControllerValues[midiInterface][midiChannel * VALUES + VALUES_CHANNEL_PRESSURE]) / ((int)song->ticksPerLine() - 1);

                    output->channelPressure(midiChannel, midiControllerValues[midiInterface][midiChannel * VALUES + VALUES_CHANNEL_PRESSURE] + tick * delta);
                } else {
                    output->channelPressure(midiChannel, value - 0x80);
                    midiControllerValues[midiInterface][midiChannel * VALUES + VALUES_CHANNEL_PRESSURE] = value - 0x80;
                }
            }
        }
        break;
    case CommandTicksPerLine:
        if (value == 0) {
            // Only on last tick
            if (tick == song->ticksPerLine() - 1) {
                postCommand = CommandTicksPerLine;
            }
        } else {
            song->setTPL(value);
        }
        break;
    case CommandTempo:
        if (value == 0) {
            // Only on last tick
            if (tick == song->ticksPerLine() - 1) {
                postCommand = CommandTempo;
            }
        } else {
            song->setTempo(value);
            output->tempo(value);
        }
        break;
    }

    // Handle MIDI controllers
    if (command >= CommandMidiControllers) {
        // MIDI controllers can be set if the MIDI channel is known
        if (midiChannel != -1) {
            if (value < 0x80) {
                if (tick == 0) {
                    output->controller(midiChannel, command - CommandMidiControllers, value);
                    midiControllerValues[midiInterface][midiChannel * VALUES + command - CommandMidiControllers] = value;
                }
            } else {
                if (tick < song->ticksPerLine() - 1) {
                    int delta = (value - 0x80 - midiControllerValues[midiInterface][midiChannel * VALUES + command - CommandMidiControllers]) / ((int)song->ticksPerLine() - 1);

                    output->controller(midiChannel, command - CommandMidiControllers, midiControllerValues[midiInterface][midiChannel * VALUES + command - CommandMidiControllers] + tick * delta);
                } else {
                    output->controller(midiChannel, command - CommandMidiControllers, value - 0x80);
                    midiControllerValues[midiInterface][midiChannel * VALUES + command - CommandMidiControllers] = value - 0x80;
                }
            }
        }
    }
}

void Player::run()
{
    struct timespec req, rem;
    struct timeval next, now;
    ExternalSync prevsyncMode = syncMode;
    unsigned int oldTime = (unsigned int)-1;

    tick = 0;
    ticksSoFar = 0;
    next.tv_sec = playingStarted.tv_sec;
    next.tv_usec = playingStarted.tv_usec;

    while (true) {
        bool looped = false;
        unsigned int oldLine = line_;

        // Lock
        mutex.lock();
        song->lock();

        if (syncMode != Off) {
            if (externalSyncTicks == 0) {
                // Wait for a sync signal to come in
                externalSync_.wait(&mutex);
            }
            if (externalSyncTicks > 0) {
                externalSyncTicks--;
            }
        } else {
            if (sched == SchedulingRTC || sched == SchedulingNanoSleep) {
                // If the scheduler has changed from external sync reset the timing
                if (prevsyncMode != Off) {
                    gettimeofday(&next, NULL);
                }
                prevsyncMode = syncMode;

                // Calculate time of next tick (tick every 1000000/((BPM/60)*24) usecs)
                next.tv_sec += (25 / song->tempo()) / 10;
                next.tv_usec += ((2500000 / song->tempo()) % 1000000);
                while (next.tv_usec > 1000000) {
                    next.tv_sec++;
                    next.tv_usec -= 1000000;
                }

                gettimeofday(&now, NULL);

                // Select scheduling type
                if (sched == SchedulingNanoSleep) {
                    // Nanosleep: calculate difference between now and the next tick
                    req.tv_sec = next.tv_sec - now.tv_sec;
                    req.tv_nsec = (next.tv_usec - now.tv_usec) * 1000;
                    while (req.tv_nsec < 0) {
                        req.tv_sec--;
                        req.tv_nsec += 1000000000;
                    }

                    // Sleep until the next tick if necessary
                    song->unlock();
                    mutex.unlock();
                    if (req.tv_sec >= 0) {
                        while (nanosleep(&req, &rem) == -1) { };
                    }
                    mutex.lock();
                    song->lock();
                } else {
                    // Make sure periodic interrupts are on
                    if (!rtcPIE) {
    #ifdef __linux
                        ioctl(rtc, RTC_PIE_ON, 0);
    #endif
                        rtcPIE = true;
                    }

                    // Rtc: read RTC interrupts until it's time to play again
                    song->unlock();
                    mutex.unlock();
                    while ((next.tv_sec - now.tv_sec) > 0 || ((next.tv_sec - now.tv_sec) == 0 && (next.tv_usec - now.tv_usec) > 1000000 / rtcFrequency)) {
                        unsigned long l;
                        ssize_t n = read(rtc, &l, sizeof(unsigned long));
                        Q_UNUSED(n)
                        gettimeofday(&now, NULL);
                    }
                    while ((next.tv_sec - now.tv_sec) > 0 || ((next.tv_sec - now.tv_sec) == 0 && (next.tv_usec - now.tv_usec) > 0)) {
                        gettimeofday(&now, NULL);
                    }
                    mutex.lock();
                    song->lock();
                }
            }
        }

        // Handle this tick
        for (int output = 0; output < midi_->outputs(); output++) {
            midi_->output(output)->setTick(ticksSoFar);
        }

        Block *block = song->block(block_);

        // Send MIDI sync if requested
        if (song->sendSync()) {
            for (int output = 0; output < midi_->outputs(); output++) {
                midi_->output(output)->clock();
            }
        }

        // The block may have changed, make sure the line won't overflow
        if (line_ >= block->length()) {
            line_ %= block->length();
        }

        // Stop notes if there are new notes about to be played
        for (int track = 0; track < block->tracks(); track++) {
            QSharedPointer<TrackStatus> trackStatus = this->trackStatus[track];

            // The track is taken into account if the track is not muted and no tracks are soloed or the current track is soloed
            if (!song->track(track)->isMuted() && (!solo || (solo && song->track(track)->isSolo()))) {
                int delay = 0;
                unsigned char basenote = block->note(line_, track);
                unsigned char instrument = block->instrument(line_, track);
                unsigned char note = basenote;

                if (note != 0) {
                    // Use previous instrument if none defined
                    int arpeggioInstrument = instrument > 0 ? (instrument - 1) : trackStatus->instrument;
                    trackStatus->line = 0;
                    if (song->instrument(arpeggioInstrument)->arpeggio() != NULL) {
                        // Add arpeggio note to the track's base note
                        Block *arpeggio = song->instrument(arpeggioInstrument)->arpeggio();
                        note = basenote + ((char)arpeggio->note(trackStatus->line, 0) - (char)song->instrument(arpeggioInstrument)->basenote());
                    }
                } else {
                    // Play arpeggio of previous instrument if any
                    int arpeggioinstrument = trackStatus->instrument;
                    if (arpeggioinstrument >= 0 && song->instrument(arpeggioinstrument)->arpeggio() != NULL) {
                        // Add arpeggio note to the track's base note
                        Block *arpeggio = song->instrument(arpeggioinstrument)->arpeggio();
                        note = trackStatus->baseNote + ((char)arpeggio->note(trackStatus->line, 0) - (char)song->instrument(arpeggioinstrument)->basenote());
                    }
                }

                if (note != 0) {
                    for (int commandPage = 0; commandPage < block->commandPages(); commandPage++) {
                        unsigned char command = block->command(line_, track, commandPage);
                        unsigned char value = block->commandValue(line_, track, commandPage);

                        if (command != 0 || value != 0) {
                            // Check for previous command if any
                            if (command == CommandPreviousCommandValue) {
                                if (value != 0) {
                                    command = trackStatus->previousCommand;
                                }
                            } else {
                                trackStatus->previousCommand = command;
                            }

                            switch (command) {
                            case CommandRetrigger:
                                delay = -value;
                                break;
                            case CommandDelay:
                                delay = value;
                                break;
                            }
                        }
                    }

                    // Stop currently playing note
                    if ((delay >= 0 && tick == delay) || (delay < 0 && tick % (-delay) == 0)) {
                        if (trackStatus->note != -1) {
                            midi_->output(trackStatus->midiInterface)->noteOff(trackStatus->midiChannel, trackStatus->note, 127);
                            trackStatus->note = -1;
                        }
                    }
                }
            }
        }

        // Play notes scheduled to be played
        for (int track = 0; track < block->tracks(); track++) {
            QSharedPointer<TrackStatus> trackStatus = this->trackStatus[track];

            // The track is taken into account if the track is not muted and no tracks are soloed or the current track is soloed
            if (!song->track(track)->isMuted() && (!solo || (solo && song->track(track)->isSolo()))) {
                unsigned int volume = 127;
                int delay = 0, hold = -1;
                unsigned char basenote = block->note(line_, track);
                unsigned char instrument = block->instrument(line_, track);
                unsigned char note = basenote;
                Block *arpeggio = NULL;

                if (note != 0) {
                    // Use previous instrument if none defined
                    int arpeggioinstrument = instrument > 0 ? (instrument - 1) : trackStatus->instrument;
                    trackStatus->line = 0;
                    if (song->instrument(arpeggioinstrument)->arpeggio() != NULL) {
                        // Add arpeggio note to the track's base note
                        arpeggio = song->instrument(arpeggioinstrument)->arpeggio();
                        note = basenote + ((char)arpeggio->note(trackStatus->line, 0) - (char)song->instrument(arpeggioinstrument)->basenote());
                    }
                } else {
                    // Play arpeggio of previous instrument if any
                    int arpeggioinstrument = trackStatus->instrument;
                    if (arpeggioinstrument >= 0 && song->instrument(arpeggioinstrument)->arpeggio() != NULL) {
                        // Add arpeggio note to the track's base note
                        arpeggio = song->instrument(arpeggioinstrument)->arpeggio();
                        note = trackStatus->baseNote + ((char)arpeggio->note(trackStatus->line, 0) - (char)song->instrument(arpeggioinstrument)->basenote());
                    }
                }

                if (arpeggio != NULL) {
                    // Handle commands on all arpeggio command pages
                    for (int commandPage = 0; commandPage < arpeggio->commandPages(); commandPage++) {
                        unsigned char command = arpeggio->command(trackStatus->line, 0, commandPage);
                        unsigned char value = arpeggio->commandValue(trackStatus->line, 0, commandPage);
                        handleCommand(trackStatus, note, instrument, command, value, &volume, &delay, &hold);
                    }
                }

                // Handle commands on all command pages
                for (int commandPage = 0; commandPage < block->commandPages(); commandPage++) {
                    unsigned char command = block->command(line_, track, commandPage);
                    unsigned char value = block->commandValue(line_, track, commandPage);
                    handleCommand(trackStatus, note, instrument, command, value, &volume, &delay, &hold);
                }

                // Set the channel base note and instrument regardless of whether there's an actual note to be played right now
                if (basenote != 0) {
                    trackStatus->baseNote = basenote;
                    if (instrument != 0) {
                        trackStatus->instrument = instrument - 1;
                    }
                }

                // Is there a note to play?
                if (note != 0 && ((delay >= 0 && tick == delay) || (delay < 0 && tick % (-delay) == 0))) {
                    Instrument *instr = NULL;

                    note--;

                    // Use previous instrument if none defined
                    if (instrument == 0) {
                        instrument = trackStatus->instrument + 1;
                    }

                    // Play note if instrument is defined
                    if (instrument != 0) {
                        playNote(instrument - 1, note, volume, track);

                        if (instrument <= song->instruments()) {
                            instr = song->instrument(instrument - 1);
                        }
                    }

                    if (instr != NULL) {
                        // If no hold value was defined use the instrument's hold value
                        if (hold == -1) {
                            hold = instr->hold();
                        }

                        trackStatus->hold = hold == 0 ? -1 : hold;
                    }
                }

                // First tick, no note but instrument defined?
                if (tick == 0 && note == 0 && instrument > 0 && trackStatus->hold >= 0) {
                    if (song->instrument(instrument - 1)->midiInterface() == trackStatus->midiInterface) {
                        trackStatus->hold += song->instrument(instrument - 1)->hold();
                    }
                }
            }
        }

        // Decrement hold times of notes and stop notes that should be stopped
        for (int track = 0; track < song->maxTracks(); track++) {
            QSharedPointer<TrackStatus> trackStatus = this->trackStatus[track];
            if (trackStatus->hold >= 0) {
                trackStatus->hold--;
                if (trackStatus->hold < 0 && trackStatus->note != -1) {
                    midi_->output(trackStatus->midiInterface)->noteOff(trackStatus->midiChannel, trackStatus->note, 127);
                    trackStatus->note = -1;
                }
            }
        }

        // Next tick
        ticksSoFar++;
        tick++;
        tick %= song->ticksPerLine();

        // Advance and handle post commands if ticksperline ticks have passed
        if (tick == 0) {
            bool changeBlock = false;

            line_++;

            // Advance arpeggios
            for (int track = 0; track < song->maxTracks(); track++) {
                QSharedPointer<TrackStatus> trackStatus = this->trackStatus[track];
                int arpeggioInstrument = trackStatus->instrument;

                if (arpeggioInstrument >= 0 && trackStatus->baseNote >= 0) {
                    Instrument *instrument = song->instrument(arpeggioInstrument);
                    if (instrument->arpeggio() != NULL) {
                        trackStatus->line++;
                        trackStatus->line %= instrument->arpeggio()->length();
                    }
                }
            }

            switch (postCommand) {
            case CommandEndBlock:
                line_ = postValue;
                if (mode_ == ModePlaySong) {
                    looped = nextPosition();
                    changeBlock = true;
                }
                break;
            case CommandPlayseqPosition:
                line_ = 0;
                position_ = postValue;
                if (position_ >= song->playseq(playseq_)->length()) {
                    position_ = 0;
                    looped = nextSection();
                }
                changeBlock = true;
                break;
            case CommandTempo:
                // COMMAND_TPL and COMMAND_TEMPO can only mean "stop" as stop cmds
                killThread = true;
                return; // TODO ??
                break;
            default:
                // Advance in block
                if (line_ >= song->block(block_)->length()) {
                    line_ = 0;
                    if (mode_ == ModePlaySong) {
                        looped = nextPosition();
                        changeBlock = true;
                    }
                }
                break;
            }
            postCommand = 0;
            postValue = 0;

            if (changeBlock) {
                refreshPlayseqAndBlock();
            }
        }

        // Check whether this thread should be killed
        if (killThread || (sched == SchedulingNone && looped)) {
            break;
        }
        song->unlock();
        mutex.unlock();

        if (line_ != oldLine) {
            emit lineChanged(line_);
        }

        if (sched != SchedulingNone) {
            gettimeofday(&now, NULL);

            unsigned int time = (unsigned int)(playedSoFar.tv_sec * 1000 + playedSoFar.tv_usec / 1000 + (now.tv_sec * 1000 + now.tv_usec / 1000) - (playingStarted.tv_sec * 1000 + playingStarted.tv_usec / 1000)) / 1000;

            if (time != oldTime) {
                emit timeChanged(time);
                oldTime = time;
            }
        }
    }

    if (sched != SchedulingNone) {
        // Calculate how long the song has been playing
        gettimeofday(&now, NULL);
        now.tv_sec -= playingStarted.tv_sec;
        if (now.tv_usec >= playingStarted.tv_usec) {
            now.tv_usec -= playingStarted.tv_usec;
        } else {
            now.tv_usec += 1000000 - playingStarted.tv_usec;
            now.tv_sec--;
        }
        playedSoFar.tv_sec += now.tv_sec;
        playedSoFar.tv_usec += now.tv_usec;
        if (playedSoFar.tv_usec > 1000000) {
            playedSoFar.tv_sec++;
            playedSoFar.tv_usec -= 1000000;
        }

        // Turn off periodic interrupts
        if (sched == SchedulingRTC && rtcPIE) {
#ifdef __linux
            ioctl(rtc, RTC_PIE_OFF, 0);
#endif
            rtcPIE = false;
        }

        // Stop all playing notes
        stopNotes();
    }

    // The mutex is locked if the thread was killed and loop broken
    song->unlock();
    mutex.unlock();
}

void Player::playWithoutScheduling()
{
    sched = SchedulingNone;
    mode_ = ModePlaySong;
    for (int instrument = 0; instrument < song->instruments(); instrument++) {
        song->instrument(instrument)->setMidiInterface(0);
    }
    midi_->output(0)->tempo(song->tempo());
    run();
    stopNotes();
}

void Player::play(Mode mode, bool cont)
{
    stop();

    Mode oldMode = mode_;
    mode_ = mode;
    tick = 0;
    ticksSoFar = 0;

    switch (mode) {
    case ModePlaySong:
        if (!cont) {
            section_ = 0;
            position_ = 0;
            line_ = 0;
        }
        refreshPlayseqAndBlock();
        break;
    case ModePlayBlock:
        if (!cont) {
            line_ = 0;
        }
        break;
    default:
        break;
    }

    // Get the starting time
    resetTime(!cont);

    // For some reason the priority setting crashes with realtime Jack
    //            if (editor == NULL || editor_player_get_external_sync(editor) != EXTERNAL_SYNC_JACK_START_ONLY)
    start(syncMode == Off ? QThread::TimeCriticalPriority : QThread::NormalPriority);

    if (mode_ != oldMode) {
        emit modeChanged(mode_);
    }
}

void Player::stop()
{
    if (mode_ != ModeIdle) {
        mode_ = ModeIdle;
        emit modeChanged(mode_);
    }

    if (isRunning()) {
        // Mark the thread for killing
        mutex.lock();
        killThread = true;
        mutex.unlock();

        // If external sync is used send sync to get out of the sync wait loop
        if (syncMode != Off) {
            externalSync(0);
        }

        // Wait until the thread is dead
        wait();
        killThread = false;
    } else {
        stopNotes();
    }
}

void Player::playSong()
{
    play(ModePlaySong, false);
}

void Player::playBlock()
{
    play(ModePlayBlock, false);
}

void Player::continueSong()
{
    play(ModePlaySong, true);
}

void Player::continueBlock()
{
    play(ModePlayBlock, true);
}

void Player::trackStatusCreate()
{
    int maxTracks = song != NULL ? song->maxTracks() : 0;

    // Free the extraneous status structures
    while (trackStatus.count() > maxTracks) {
        trackStatus.removeLast();
    }

    // Set new tracks to -1
    while (trackStatus.count() < maxTracks) {
        TrackStatus *status = new TrackStatus;
        status->instrument = -1;
        status->previousCommand = 0;
        status->note = -1;
        status->midiChannel = -1;
        status->volume = -1;
        status->hold = -1;
        trackStatus.append(QSharedPointer<TrackStatus>(status));
    }
}

void Player::setSong(const QString &path)
{
    Song *oldSong = song;
    song = NULL;

    QFile file(path);
    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        QByteArray header = file.read(4);
        if (header.length() == 4) {
            const char *data = header.data();
            if (data[0] == (ID_MMD2 >> 24) && data[1] == ((ID_MMD2 >> 16) & 0xff) && data[2] == ((ID_MMD2 >> 8) & 0xff) && data[3] == (ID_MMD2 & 0xff)) {
                song = mmd2ToSong(MMD2_load(path.toUtf8().constData()));
            }
        }
    }

    if (song == NULL) {
        song = new Song(path);
    }

    init();

    connect(song, SIGNAL(blocksChanged(uint)), this, SLOT(refreshPlayseqAndBlock()));
    connect(song, SIGNAL(playseqsChanged(uint)), this, SLOT(refreshPlayseqAndBlock()));
    connect(song, SIGNAL(sectionsChanged(uint)), this, SLOT(refreshPlayseqAndBlock()));

    if (oldSong != NULL) {
        delete oldSong;
    }
}

void Player::init()
{
    remapMidiOutputs();

    // Recreate the track status array
    trackStatusCreate();
    connect(this->song, SIGNAL(maxTracksChanged(uint)), this, SLOT(trackStatusCreate()));

    // Check solo status
    checkSolo();

    // Send messages to be autosent
    for (int message = 0; message < song->messages(); message++) {
        if (song->message(message)->isAutoSend()) {
            for (int output = 0; output < midi_->outputs(); output++) {
                midi_->output(output)->writeRaw(song->message(message)->data());
            }
        }
    }

    emit songChanged(song);

    // Reset to the beginning
    block_ = (unsigned int)-1;
    section_ = (unsigned int)-1;
    playseq_ = (unsigned int)-1;
    position_ = (unsigned int)-1;
    line_ = (unsigned int)-1;
    refreshPlayseqAndBlock();
}

void Player::setSection(int section)
{
    unsigned int oldSection = section_;

    // Bounds checking
    if (section < 0) {
        section = 0;
    } else if (section >= song->sections()) {
        section = song->sections() - 1;
    }

    mutex.lock();
    section_ = section;
    mutex.unlock();

    if (section_ != oldSection) {
        emit sectionChanged(section_);
    }
}

void Player::setPlayseq(int playseq)
{
    unsigned int oldPlayseq = playseq_;

    // Bounds checking
    if (playseq < 0) {
        playseq = 0;
    } else if (playseq >= song->playseqs()) {
        playseq = song->playseqs() - 1;
    }

    mutex.lock();
    playseq_ = playseq;
    mutex.unlock();

    if (playseq_ != oldPlayseq) {
        emit playseqChanged(playseq_);
    }
}

void Player::setPosition(int position)
{
    unsigned int oldPosition = position_;

    mutex.lock();

    // Bounds checking
    if (position < 0) {
        position = 0;
    } else if (position >= song->playseq(playseq_)->length()) {
        position = song->playseq(playseq_)->length() - 1;
    }

    position_ = position;

    mutex.unlock();

    if (position_ != oldPosition) {
        emit positionChanged(position_);
    }
}

void Player::setBlock(int block)
{
    unsigned int oldBlock = block_;

    // Bounds checking
    if (block < 0) {
        block = 0;
    } else if (block >= song->blocks()) {
        block = song->blocks() - 1;
    }

    mutex.lock();
    block_ = block;
    mutex.unlock();

    if (block_ != oldBlock) {
        emit blockChanged(block_);
    }
}

void Player::setLine(int line)
{
    unsigned int oldLine = line_;

    mutex.lock();

    // Bounds checking
    while (line < 0) {
        line += song->block(block_)->length();
    }
    while (line >= song->block(block_)->length()) {
        line -= song->block(block_)->length();
    }

    line_ = line;

    mutex.unlock();

    if (line_ != oldLine) {
        emit lineChanged(line_);
    }
}

void Player::setTick(int tick)
{
    mutex.lock();
    this->tick = tick;
    mutex.unlock();
}

void Player::resetTime(bool resetSofar)
{
    gettimeofday(&playingStarted, NULL);

    if (resetSofar) {
        playedSoFar.tv_sec = 0;
        playedSoFar.tv_usec = 0;
    }
}

void Player::setSolo(unsigned int solo)
{
    this->solo = solo;
}

void Player::checkSolo()
{
    int solo = 0;

    for (int track = 0; track < song->maxTracks(); track++) {
        solo |= song->track(track)->isSolo();
    }

    this->solo = solo;
}

void Player::remapMidiOutputs()
{
    for (int instrument = 0; instrument < song->instruments(); instrument++) {
        int output = midi_->output(song->instrument(instrument)->midiInterfaceName());
        if (output >= 0) {
            song->instrument(instrument)->setMidiInterface(output);
        }
    }

    // Recreate the track status array
    trackStatusCreate();

    // Remove extraneous controller values
    while (midi_->outputs() < midiControllerValues.count()) {
        midiControllerValues.removeLast();
    }

    // Create new controller values
    for (int output = midiControllerValues.count(); output < midi_->outputs(); output++) {
        midiControllerValues.append(QVector<unsigned char>(16 * VALUES));
    }
}

void Player::lock()
{
    mutex.lock();
}

void Player::unlock()
{
    mutex.unlock();
}

void Player::externalSync(unsigned int ticks)
{
    mutex.lock();
    if (mode_ != ModeIdle) {
        externalSyncTicks += ticks;
    }
    externalSync_.wakeAll();
    mutex.unlock();
}

void Player::setExternalSync(ExternalSync externalSync)
{
    ExternalSync prevsyncMode = syncMode;

    syncMode = externalSync;

    if (syncMode == Off && prevsyncMode != Off) {
        this->externalSync(0);
    }
}

void Player::setScheduler(Scheduling scheduler)
{
    mutex.lock();

    sched = scheduler;
    if (sched == SchedulingRTC) {
        rtc = open("/dev/rtc", O_RDONLY);
        if (rtc == -1) {
            sched = SchedulingNanoSleep;
        } else {
            bool success = false;
            rtcFrequency = MAXIMUM_RTC_FREQ;
            rtcPIE = 0;

#ifdef __linux
            // RTC scheduling requested: try every 2^N value down from MAXIMUM_RTC_FREQ to MINIMUM_RTC_FREQ
            while (rtcFrequency >= MINIMUM_RTC_FREQ && !success) {
                if (ioctl(rtc, RTC_UIE_OFF, 0) == -1 || ioctl(rtc, RTC_IRQP_SET, rtcFrequency) == -1) {
                    rtcFrequency /= 2;
                } else {
                    success = true;
                }
            }
#endif

            // If all frequencies failed fall back to nanosleep()
            if (!success) {
                close(rtc);
                rtc = -1;
                sched = SchedulingNanoSleep;
            }
        }
    } else {
        if (rtc != -1) {
            close(rtc);
        }
        rtc = -1;
    }

    mutex.unlock();
}

unsigned int Player::section() const
{
    return section_;
}

unsigned int Player::playseq() const
{
    return playseq_;
}

unsigned int Player::position() const
{
    return position_;
}

unsigned int Player::block() const
{
    return block_;
}

unsigned int Player::line() const
{
    return line_;
}

Player::Mode Player::mode() const
{
    return mode_;
}

MIDI *Player::midi() const
{
    return midi_;
}
