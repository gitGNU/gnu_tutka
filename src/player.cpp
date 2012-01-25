/*
 * player.cpp
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

#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#ifdef __linux
#include <linux/rtc.h>
#endif
#include <QThread>
#include "song.h"
#include "instrument.h"
#include "midiinterface.h"
#include "midi.h"
#include "player.h"

#define MAXIMUM_RTC_FREQ 2048
#define MINIMUM_RTC_FREQ 512

Player::Player(MIDI *midi, QObject *parent) :
    QThread(parent),
    section_(0),
    playseq_(0),
    position_(0),
    block_(0),
    line_(0),
    tick(0),
    song(NULL),
    mode_(IDLE),
    sched(SCHED_NANOSLEEP),
    ticksSoFar(0),
    externalSyncTicks(0),
    killThread(false),
    midi(midi),
    rtc(-1),
    rtcFrequency(0),
    rtcPIE(false),
    solo(0),
    postCommand(0),
    postValue(0),
    tempoChanged(false)
{
    midiChanged();
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
    if (section_ >= song->sections()) {
        section_ = 0;
    }
    playseq_ = song->section(section_);

    if (position_ >= song->playseq(playseq_)->length()) {
        position_ = 0;
    }

    block_ = song->playseq(playseq_)->at(position_);
    emit blockChanged(song->block(block_));
}

bool Player::nextSection()
{
    section_++;
    if (section_ >= song->sections()) {
        section_ = 0;
        return true;
    }
    return false;
}

bool Player::nextPlayseq()
{
    position_++;
    if (position_ >= song->playseq(playseq_)->length()) {
        position_ = 0;
        return nextSection();
    }
    return false;
}

void Player::playNote(unsigned int instrumentNumber, unsigned char note, unsigned char volume, unsigned char track)
{
    // Notes are played if the track is not muted and no tracks are soloed or the current track is soloed
    if (!song->track(track)->isMuted() && (!solo || (solo && song->track(track)->isSolo()))) {
        QSharedPointer<TrackStatus> trackStatus = this->trackStatus[track];

        // Stop currently playing note
        if (trackStatus->note != -1) {
            midi->output(trackStatus->midiInterface)->noteOff(trackStatus->midiChannel, trackStatus->note, 127);
            trackStatus->note = -1;
        }

        // Don't play a note if the instrument does not exist
        Instrument *instrument = song->instrument(instrumentNumber);
        if (instrument != NULL) {
            trackStatus->instrument = instrumentNumber;

            // Update track status for the selected output
            trackStatus->note = note + instrument->transpose();
            trackStatus->midiChannel = instrument->midiChannel();
            trackStatus->volume = instrument->defaultVelocity() * volume / 127 * song->track(track)->volume() / 127 * song->masterVolume() / 127;
            trackStatus->hold = instrument->hold() > 0 ? instrument->hold() : -1;

            // Make sure the volume isn't too large
            if (trackStatus->volume < 0) {
                trackStatus->volume = 127;
            }

            // Play note
            midi->output(instrument->midiInterface())->noteOn(trackStatus->midiChannel, trackStatus->note, trackStatus->volume);
        }
    }
}

void Player::stopMuted()
{
    for (int track = 0; track < song->maxTracks(); track++) {
        if (song->track(track)->isMuted() || (solo && !song->track(track)->isSolo())) {
            QSharedPointer<TrackStatus> trackStatus = this->trackStatus[track];
            if (trackStatus->note != -1) {
                midi->output(trackStatus->midiInterface)->noteOff(trackStatus->midiChannel, trackStatus->note, 127);
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
            midi->output(trackStatus->midiInterface)->noteOff(trackStatus->midiChannel, trackStatus->note, 127);

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
            for (int output = 0; output < midi->outputs(); output++) {
                midi->output(output)->noteOff(midiChannel, note, 127);
            }
        }
    }
}

void Player::resetPitch()
{
    for (int midiChannel = 0; midiChannel < 16; midiChannel++) {
        for (int output = 0; output < midi->outputs(); output++) {
            midi->output(output)->pitchWheel(midiChannel, 64);
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

    QSharedPointer<MIDIInterface> output = midi->output(midiInterface);

    // Check for previous command if any
    if (command == COMMAND_PREVIOUS_COMMAND_VALUE) {
        if (value != 0) {
            command = trackStatus->previousCommand;
        }
    } else {
        trackStatus->previousCommand = command;
    }

    switch (command) {
    case COMMAND_PITCH_WHEEL:
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
    case COMMAND_PROGRAM_CHANGE:
        // Pitch wheel can be set if the MIDI channel is known
        if (midiChannel != -1 && tick == 0) {
            output->programChange(midiChannel, value & 0x7f);
        }
        break;
    case COMMAND_END_BLOCK:
        // Only on last tick
        if (tick == song->ticksPerLine() - 1) {
            postCommand = COMMAND_END_BLOCK;
            postValue = value;
        }
        break;
    case COMMAND_PLAYSEQ_POSITION:
        // Only on last tick
        if (tick == song->ticksPerLine() - 1) {
            postCommand = COMMAND_PLAYSEQ_POSITION;
            postValue = value;
        }
        break;
    case COMMAND_SEND_MESSAGE:
        // Only on first tick
        if (tick == 0 && value < song->messages()) {
            output->writeRaw(song->message(value)->rawData(), song->message(value)->length());
        }
        break;
    case COMMAND_HOLD:
        *hold = value;
        break;
    case COMMAND_RETRIGGER:
        *delay = -value;
        break;
    case COMMAND_DELAY:
        *delay = value;
        break;
    case COMMAND_VELOCITY:
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
    case COMMAND_CHANNEL_PRESSURE:
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
    case COMMAND_TPL:
        if (value == 0) {
            // Only on last tick
            if (tick == song->ticksPerLine() - 1) {
                postCommand = COMMAND_TPL;
            }
        } else {
            song->setTPL(value);
            tempoChanged = 1;
        }
        break;
    case COMMAND_TEMPO:
        if (value == 0) {
            // Only on last tick
            if (tick == song->ticksPerLine() - 1) {
                postCommand = COMMAND_TEMPO;
            }
        } else {
            song->setTempo(value);
            output->tempo(value);
            tempoChanged = 1;
        }
        break;
    }

    // Handle MIDI controllers
    if (command >= COMMAND_MIDI_CONTROLLERS) {
        // MIDI controllers can be set if the MIDI channel is known
        if (midiChannel != -1) {
            if (value < 0x80) {
                if (tick == 0) {
                    output->controller(midiChannel, command - COMMAND_MIDI_CONTROLLERS, value);
                    midiControllerValues[midiInterface][midiChannel * VALUES + command - COMMAND_MIDI_CONTROLLERS] = value;
                }
            } else {
                if (tick < song->ticksPerLine() - 1) {
                    int delta = (value - 0x80 - midiControllerValues[midiInterface][midiChannel * VALUES + command - COMMAND_MIDI_CONTROLLERS]) / ((int)song->ticksPerLine() - 1);

                    output->controller(midiChannel, command - COMMAND_MIDI_CONTROLLERS, midiControllerValues[midiInterface][midiChannel * VALUES + command - COMMAND_MIDI_CONTROLLERS] + tick * delta);
                } else {
                    output->controller(midiChannel, command - COMMAND_MIDI_CONTROLLERS, value - 0x80);
                    midiControllerValues[midiInterface][midiChannel * VALUES + command - COMMAND_MIDI_CONTROLLERS] = value - 0x80;
                }
            }
        }
    }
}

void Player::run()
{
    struct timespec req, rem;
    struct timeval next, now;
    int prevsched = sched;

    tick = 0;
    ticksSoFar = 0;
    next.tv_sec = playingStarted.tv_sec;
    next.tv_usec = playingStarted.tv_usec;

    while (true) {
        struct timeval tod;
        bool looped = false;
        unsigned int oldLine = line_;

        // Lock
        mutex.lock();

        if (sched == SCHED_RTC || sched == SCHED_NANOSLEEP) {
            // If the scheduler has changed from external sync reset the timing
            if (prevsched == SCHED_EXTERNAL_SYNC) {
                gettimeofday(&next, NULL);
            }
            prevsched = sched;

            // Calculate time of next tick (tick every 1000000/((BPM/60)*24) usecs)
            next.tv_sec += (25 / song->tempo()) / 10;
            next.tv_usec += ((2500000 / song->tempo()) % 1000000);
            while (next.tv_usec > 1000000) {
                next.tv_sec++;
                next.tv_usec -= 1000000;
            }

            gettimeofday(&now, NULL);

            // Select scheduling type
            if (sched == SCHED_NANOSLEEP) {
                // Nanosleep: calculate difference between now and the next tick
                req.tv_sec = next.tv_sec - now.tv_sec;
                req.tv_nsec = (next.tv_usec - now.tv_usec) * 1000;
                while (req.tv_nsec < 0) {
                    req.tv_sec--;
                    req.tv_nsec += 1000000000;
                }

                // Sleep until the next tick if necessary
                mutex.unlock();
                if (req.tv_sec >= 0) {
                    while (nanosleep(&req, &rem) == -1) { };
                }
                mutex.lock();
            } else {
                // Make sure periodic interrupts are on
                if (!rtcPIE) {
#ifdef __linux
                    ioctl(rtc, RTC_PIE_ON, 0);
#endif
                    rtcPIE = true;
                }

                // Rtc: read RTC interrupts until it's time to play again
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
            }
        } else if (sched == SCHED_EXTERNAL_SYNC) {
            prevsched = sched;
            if (externalSyncTicks == 0) {
                // Wait for a sync signal to come in
                externalSync_.wait(&mutex);
            }
            if (externalSyncTicks > 0) {
                externalSyncTicks--;
            }
        } else {
            prevsched = sched;
        }

        // Handle this tick
        for (int output = 0; output < midi->outputs(); output++) {
            midi->output(output)->setTick(ticksSoFar);
        }

        Block *block = song->block(block_);

        // Send MIDI sync if requested
        if (song->sendSync()) {
            for (int output = 0; output < midi->outputs(); output++) {
                midi->output(output)->clock();
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
                            if (command == COMMAND_PREVIOUS_COMMAND_VALUE) {
                                if (value != 0) {
                                    command = trackStatus->previousCommand;
                                }
                            } else {
                                trackStatus->previousCommand = command;
                            }

                            switch (command) {
                            case COMMAND_RETRIGGER:
                                delay = -value;
                                break;
                            case COMMAND_DELAY:
                                delay = value;
                                break;
                            }
                        }
                    }

                    // Stop currently playing note
                    if ((delay >= 0 && tick == delay) || (delay < 0 && tick % (-delay) == 0)) {
                        if (trackStatus->note != -1) {
                            midi->output(trackStatus->midiInterface)->noteOff(trackStatus->midiChannel, trackStatus->note, 127);
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
                    midi->output(trackStatus->midiInterface)->noteOff(trackStatus->midiChannel, trackStatus->note, 127);
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
            int changeblock = 0;

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
            case COMMAND_END_BLOCK:
                line_ = postValue;
                if (mode_ == PLAY_SONG) {
                    looped = nextPlayseq();
                    changeblock = 1;
                }
                break;
            case COMMAND_PLAYSEQ_POSITION:
                line_ = 0;
                position_ = postValue;
                if (position_ >= song->playseq(playseq_)->length()) {
                    position_ = 0;
                    looped = nextSection();
                }
                changeblock = 1;
                break;
            case COMMAND_TEMPO:
                // COMMAND_TPL and COMMAND_TEMPO can only mean "stop" as stop cmds
                killThread = true;
                return;
                break;
            default:
                // Advance in block
                if (line_ >= song->block(block_)->length()) {
                    line_ = 0;
                    if (mode_ == PLAY_SONG) {
                        looped = nextPlayseq();
                        changeblock = 1;
                    }
                }
                break;
            }
            postCommand = 0;
            postValue = 0;

            if (changeblock == 1) {
                refreshPlayseqAndBlock();
            }

            if (sched != SCHED_NONE) {
                gettimeofday(&tod, NULL);
            }
        }

        // Check whether this thread should be killed
        if (killThread || (sched == SCHED_NONE && looped)) {
            break;
        }
        mutex.unlock();

        if (line_ != oldLine) {
            emit lineChanged(line_);
        }
    }

    if (sched != SCHED_NONE) {
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
        if (sched == SCHED_RTC && rtcPIE) {
#ifdef __linux
            ioctl(rtc, RTC_PIE_OFF, 0);
#endif
            rtcPIE = false;
        }

        // Stop all playing notes
        stopNotes();
    }

    // The mutex is locked if the thread was killed and loop broken
    mutex.unlock();
}

#ifdef TODO
// Plays a song to a MIDI device witout any scheduling (for export)
void Player::midiExport(Song *song, MIDIInterface *midi)
{
    int i, j;
    midi = (struct midi *)calloc(1, sizeof(struct midi));

    // Check how many MIDI interfaces are used in the song
    for (i = 0; i < song->numinstruments; i++) {
        struct instrument *instrument = song->instruments[i];
        for (j = 0; j < instrument->numoutputs; j++) {
            struct instrument_output *output = instrument->outputs[j];
            if (output->midiinterface > midi->numoutputs)
                midi->numoutputs = output->midiinterface;
        }
    }
    midi->numoutputs++;

    song = song;
    midi->outputs = (struct midi_interface **)calloc(midi->numoutputs, sizeof(struct midi_interface *));
    for (i = 0; i < midi->outputs(); i++)
        midi->output(i) = midi;
    sched = SCHED_NONE;
    mode = MODE_PLAY_SONG;
    Player::refresh_playseq_and_block(player);

    // Initialize track status array
    Player::trackstatus_create(player, 0);

    // Create MIDI controller values array
    midicontrollervalues = (unsigned char **)calloc(midi->numoutputs, sizeof(unsigned char *));
    for (i = 0; i < midi->numoutputs; i++)
        midicontrollervalues[i] = (unsigned char *)calloc(16 * VALUES, sizeof(unsigned char));

    // Send messages to be autosent
    for (i = 0; i < song->nummessages; i++) {
        if (song->messages[i]->autosend)
            midi_write_raw(midi->outputs[0], song->messages[i]->data, song->messages[i]->length);
    }

    // Set tempo
    midi_tempo(midi->outputs[0], song->tempo());

    Player::thread(player);
    Player::stop_notes(player);

    // Free the track status array
    if (trackstatus != NULL) {
        for (i = 0; i < song->maxtracks; i++) {
            for (j = 0; j < trackstatus[i]->numinterfaces; j++)
                free(trackstatus[i]->interfaces[j]);
            free(trackstatus[i]->interfaces);
            free(trackstatus[i]);
        }
        free(trackstatus);
    }

    // Free MIDI interface array
    free(midi->outputs);
    free(player);
}
#endif

void Player::play(Mode mode, bool cont)
{
    stop();

    mode_ = mode;
    tick = 0;
    ticksSoFar = 0;

    switch (mode) {
    case PLAY_SONG:
        if (!cont) {
            section_ = 0;
            position_ = 0;
            line_ = 0;
        }
        refreshPlayseqAndBlock();
        break;
    case PLAY_BLOCK:
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
    start(sched != SCHED_EXTERNAL_SYNC ? QThread::TimeCriticalPriority : QThread::NormalPriority);
}

void Player::stop()
{
    if (mode_ != IDLE) {
        mode_ = IDLE;
    }

    if (isRunning()) {
        // Mark the thread for killing
        mutex.lock();
        killThread = true;
        mutex.unlock();

        // If external sync is used send sync to get out of the sync wait loop
        if (sched == SCHED_EXTERNAL_SYNC) {
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
    play(PLAY_SONG, false);
}

void Player::playBlock()
{
    play(PLAY_BLOCK, false);
}

void Player::continueSong()
{
    play(PLAY_SONG, true);
}

void Player::continueBlock()
{
    play(PLAY_BLOCK, true);
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

void Player::setSong(Song *song)
{
    this->song = song;

    // Recreate the track status array
    trackStatusCreate();

    // Check solo status
    checkSolo();

    // Send messages to be autosent
    for (int message = 0; message < song->messages(); message++) {
        if (song->message(message)->isAutoSend()) {
            for (int output = 0; output < midi->outputs(); output++) {
                midi->output(output)->writeRaw(song->message(message)->rawData(), song->message(message)->length());
            }
        }
    }

    emit songChanged(song);

    // Reset to the beginning
    section_ = 0;
    position_ = 0;
    line_ = 0;
    refreshPlayseqAndBlock();
}

void Player::setSection(int section)
{
    // Bounds checking
    if (section < 0) {
        section = 0;
    } else if (section >= song->sections()) {
        section = song->sections() - 1;
    }

    mutex.lock();
    section_ = section;
    mutex.unlock();
}

void Player::setPlayseq(int playseq)
{
    // Bounds checking
    if (playseq < 0) {
        playseq = 0;
    } else if (playseq >= song->playseqs()) {
        playseq = song->playseqs() - 1;
    }

    mutex.lock();
    playseq_ = playseq;
    mutex.unlock();
}

void Player::setPosition(int position)
{
    mutex.lock();

    // Bounds checking
    if (position < 0) {
        position = 0;
    } else if (position >= song->playseq(playseq_)->length()) {
        position = song->playseq(playseq_)->length() - 1;
    }

    position_ = position;

    mutex.unlock();
}

void Player::setBlock(int block)
{
    // Bounds checking
    if (block < 0) {
        block = 0;
    } else if (block >= song->blocks()) {
        block = song->blocks() - 1;
    }

    mutex.lock();
    block_ = block;
    mutex.unlock();

    emit blockChanged(song->block(block_));
}

void Player::setLine(int line)
{
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

void Player::midiChanged()
{
    // Recreate the track status array
    trackStatusCreate();

    // Remove extraneous controller values
    while (midi->outputs() < midiControllerValues.count()) {
        midiControllerValues.removeLast();
    }

    // Create new controller values
    for (int output = midiControllerValues.count(); output < midi->outputs(); output++) {
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
    if (mode_ != IDLE) {
        externalSyncTicks += ticks;
    }
    externalSync_.wakeAll();
    mutex.unlock();
}

void Player::setScheduler(unsigned int scheduler)
{
    mutex.lock();

    sched = scheduler;
    if (sched == SCHED_RTC) {
        rtc = open("/dev/rtc", O_RDONLY);
        if (rtc == -1) {
            sched = SCHED_NANOSLEEP;
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
                sched = SCHED_NANOSLEEP;
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

int Player::line() const
{
    return line_;
}

int Player::position() const
{
    return position_;
}

int Player::section() const
{
    return section_;
}

int Player::block() const
{
    return block_;
}

Player::Mode Player::mode() const
{
    return mode_;
}