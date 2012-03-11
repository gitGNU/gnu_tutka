/* conversion.cpp
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

#include <cstring>
#include "song.h"
#include "track.h"
#include "block.h"
#include "playseq.h"
#include "instrument.h"
#include "message.h"
#include "mmd.h"
#include "smf.h"
#include "buffermidi.h"
#include "midiinterface.h"
#include "player.h"
#include "conversion.h"

// Converts MMD command values at given location
static void mmd2CommandToSongCommand(unsigned char &command, unsigned char &value, struct MMDMIDICmd3x *mmdcmd3x)
{
    switch (command) {
    case Player::CommandPitchWheel:
        value = (value + 0x80) / 2;
        break;
    case Player::CommandPreviousCommandValue:
        if (value != 0) {
            command = 0x80;
            value &= 0x7f;
        }
        break;
    case Player::CommandVelocity:
        // MIDI volumes are in range 0-95 in OctaMED
        value = value * 0x80 / 0x60;
        break;
    case Player::CommandDelay:
        // Command 1F in MMD is both note delay and retrigger
        if (value > 15) {
            value = value >> 4;
         } else {
            command = Player::CommandRetrigger;
        }
        break;
    case Player::CommandTempo:
        // Values >0xf0 have special meanings in OctaMED
        if (value > 0xf0) {
            switch (value) {
            case 0xf1:
                // Makes a single note play twice
                command = Player::CommandRetrigger;
                value = 3;
                break;
            case 0xf2:
                // Delays the start of a note by half a line
                command = Player::CommandDelay;
                value = 3;
                break;
            case 0xf3:
                // Works like 0FF1 except the note is played three times
                command = Player::CommandRetrigger;
                value = 2;
                break;
            case 0xf4:
                // Delays the note one-third of a line
                command = Player::CommandDelay;
                value = 2;
                break;
            case 0xf5:
                // Delays the note two thirds of a line
                command = Player::CommandDelay;
                value = 4;
                break;
            case 0xfe:
                // Stops the song playing
                command = Player::CommandTempo;
                value = 0;
                break;
            case 0xff:
                // Stops the note on the current track
                command = Player::CommandVelocity;
                value = 0;
                break;
            default:
                command = 0;
                value = 0;
                break;
            }
        }
        break;
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3a:
    case 0x3b:
    case 0x3c:
    case 0x3d:
    case 0x3e:
    case 0x3f:
        if (mmdcmd3x->ctrlr_types[command - 0x31] == MCS_TYPE_STD_MSB) {
            command = mmdcmd3x->ctrlr_numbers[command - 0x31] + 0x80;
        }
        break;
    default:
        break;
    }
}

// Converts an MMD2 module to a song
Song *mmd2ToSong(struct MMD2 *mmd)
{
    if (mmd == NULL) {
        return NULL;
    }

    unsigned char commandconversion[256];

    // Create a table for converting MMD commands; values need special handling
    for (int i = 0; i < 256; i++) {
        commandconversion[i] = i;
    }
    commandconversion[0] = Player::CommandPreviousCommandValue;
    commandconversion[1] = Player::CommandPitchWheel;
    commandconversion[2] = Player::CommandPitchWheel;
    commandconversion[3] = Player::CommandPitchWheel;
    commandconversion[4] = Player::CommandMidiControllers + 1;
    commandconversion[5] = Player::CommandNotDefined;
    commandconversion[6] = Player::CommandNotDefined;
    commandconversion[7] = Player::CommandNotDefined;
    commandconversion[8] = Player::CommandHold;
    commandconversion[9] = Player::CommandTicksPerLine;
    commandconversion[10] = Player::CommandVelocity;
    commandconversion[11] = Player::CommandPlayseqPosition;
    commandconversion[12] = Player::CommandVelocity;
    commandconversion[13] = Player::CommandChannelPressure;
    commandconversion[14] = Player::CommandMidiControllers + 10;
    commandconversion[15] = Player::CommandTempo;
    commandconversion[16] = Player::CommandSendMessage;
    commandconversion[17] = Player::CommandNotDefined;
    commandconversion[18] = Player::CommandNotDefined;
    commandconversion[19] = Player::CommandPitchWheel;
    commandconversion[20] = Player::CommandNotDefined;
    commandconversion[21] = Player::CommandNotDefined;
    commandconversion[22] = Player::CommandMidiControllers + 7;
    commandconversion[23] = Player::CommandNotDefined;
    commandconversion[24] = Player::CommandNotDefined;
    commandconversion[25] = Player::CommandNotDefined;
    commandconversion[26] = Player::CommandNotDefined;
    commandconversion[27] = Player::CommandNotDefined;
    commandconversion[28] = Player::CommandNotDefined;
    commandconversion[29] = Player::CommandEndBlock;
    commandconversion[30] = Player::CommandNotDefined;
    commandconversion[31] = Player::CommandDelay;

    Song *song = new Song;
    if (mmd->expdata != NULL && mmd->expdata->songname != NULL) {
        song->setName(QString::fromLatin1(mmd->expdata->songname));
    } else {
        song->setName("Untitled");
    }

    if (mmd->song) {
        // Tempo
        song->setTempo(mmd->song->deftempo);
        song->setTPL(mmd->song->tempo2);

        // Sections
        for (int section = song->sections(); section < mmd->song->songlen; section++) {
            song->insertSection(section);
        }
        for (int section = 0; section < mmd->song->songlen; section++) {
            song->setSection(section, mmd->song->sectiontable[section]);
        }

        // Playing sequences
        for (int playseq = song->playseqs(); playseq < mmd->song->numpseqs; playseq++) {
            song->insertPlayseq(playseq);
        }
        for (int number = 0; number < mmd->song->numpseqs; number++) {
            struct PlaySeq *PlaySeq = mmd->song->playseqtable[number];

            Playseq *playseq = song->playseq(number);
            if (PlaySeq->name != NULL) {
                playseq->setName(QString::fromLatin1(PlaySeq->name));
            }
            while (playseq->length() < PlaySeq->length) {
                playseq->insert(playseq->length());
            }
            for (int position = 0; position < PlaySeq->length; position++) {
                playseq->set(position, PlaySeq->seq[position]);
            }
        }

        // Blocks
        song->block(0)->setLength(0);
        song->block(0)->setTracks(0);
        for (int block = song->blocks(); block < mmd->song->numblocks; block++) {
            song->insertBlock(block, block);
        }
        for (int number = 0; number < mmd->song->numblocks; number++) {
            Block *block = song->block(number);
            struct MMD1Block *MMD1Block = mmd->blockarr[number];
            unsigned char *notes = (unsigned char *)MMD1Block + sizeof(struct MMD1Block);
            block->setTracks(MMD1Block->numtracks);
            block->setLength(MMD1Block->lines + 1);
            block->setCommandPages((MMD1Block->info != NULL && MMD1Block->info->pagetable != NULL) ? (MMD1Block->info->pagetable->num_pages + 1) : 1);

            if (MMD1Block->info != NULL && MMD1Block->info->blockname != NULL) {
                block->setName(QString::fromLatin1(MMD1Block->info->blockname));
            }

            for (int line = 0; line < block->length(); line++)
                for (int track = 0; track < block->tracks(); track++) {
                    block->setNoteFull(line, track, notes[(line * block->tracks() + track) * 4], notes[(line * block->tracks() + track) * 4 + 1]);
                    block->setCommandFull(line, track, 0, commandconversion[notes[(line * block->tracks() + track) * 4 + 2]], notes[(line * block->tracks() + track) * 4 + 3]);

                    // Command values may need special handling
                    unsigned char command = block->command(line, track, 0);
                    unsigned char value = block->commandValue(line, track, 0);
                    mmd2CommandToSongCommand(command, value, mmd->expdata->mmdcmd3x);
                    block->setCommandFull(line, track, 0, command, value);
                }

            if (MMD1Block->info != NULL && MMD1Block->info->pagetable != NULL) {
                for (int commandPage = 0; commandPage < MMD1Block->info->pagetable->num_pages; commandPage++) {
                    for (int line = 0; line < (MMD1Block->lines + 1); line++)
                        for (int track = 0; track < MMD1Block->numtracks; track++) {
                            unsigned char *commands = (unsigned char *)MMD1Block->info->pagetable->page[commandPage];

                            block->setCommandFull(line, track, commandPage, commandconversion[commands[(line * MMD1Block->numtracks + track) * 2]], commands[(line * MMD1Block->numtracks + track) * 2 + 1]);

                            // Command values may need special handling
                            unsigned char command = block->command(line, track, commandPage);
                            unsigned char value = block->commandValue(line, track, commandPage);
                            mmd2CommandToSongCommand(command, value, mmd->expdata->mmdcmd3x);
                            block->setCommandFull(line, track, commandPage, command, value);
                        }
                }
            }
        }

        // Instruments
        song->checkInstrument(mmd->song->numsamples);
        for (int number = 0; number < mmd->song->numsamples; number++) {
            Instrument *instrument = song->instrument(number);
            if (mmd->expdata) {
                instrument->setName(QString::fromLatin1((char *)mmd->expdata->iinfo[number].name));
                instrument->setHold(mmd->expdata->exp_smp[number].hold != 0);
            }
            instrument->setMidiChannel(mmd->song->sample[number].midich - 1);
            instrument->setTranspose(mmd->song->sample[number].strans);
            if (mmd->song->sample[number].svol == 64) {
                instrument->setDefaultVelocity(127);
            } else {
                instrument->setDefaultVelocity(mmd->song->sample[number].svol * 2);
            }
        }

        // Track volumes
        QMetaObject::invokeMethod(song, "checkMaxTrcaks");
        for (int track = 0; track < mmd->song->numtracks; track++) {
            song->track(track)->setVolume(mmd->song->trackvols[track] < 64 ? (mmd->song->trackvols[track] * 2) : 127);
        }
        song->setMasterVolume(mmd->song->mastervol < 64 ? (mmd->song->mastervol * 2) : 127);

        // SysEX dumps
        if (mmd->expdata && mmd->expdata->dumps) {
            // Is this really the only way to do this? I know, this is horrible but this is what the lame MMD spec says
            struct MMDDump **dumps = (struct MMDDump **)((char *)mmd->expdata->dumps + sizeof(struct MMDDumpData));

            for (int message = song->messages(); message < mmd->expdata->dumps->numdumps; message++) {
                song->insertMessage(message);
            }
            for (int number = 0; number < mmd->expdata->dumps->numdumps; number++) {
                song->message(number)->setLength(dumps[number]->length);
                song->message(number)->setData((const char *)dumps[number]->data, dumps[number]->length);

                if (dumps[number]->ext_len >= 20) {
                    song->message(number)->setName(QString::fromLatin1((char *)dumps[number]->name));
                }
            }
        }
    } else {
        // Nothing can be done in this case... should never happen though
        delete song;
        song = new Song;
    }

    return song;
}

// Converts song command values at given location
static void songCommandToMMD2Command(unsigned char *command)
{
    switch (command[0]) {
    case 0x0c:
        // If there is no note convert C00 to FFF
        if (command[-2] == 0 && command[1] == 0) {
            command[0] = 0x0f;
            command[1] = 0xff;
        } else {
            command[1] = command[1] * 0x60 / 0x80;
        }
        break;
    case 0x13:
        command[1] = command[1] * 2 - 0x80;
        break;
    case 0x20:
        // Delay
        command[0] = 0x1f;
        command[1] <<= 4;
        break;
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3a:
    case 0x3b:
    case 0x3c:
    case 0x3d:
    case 0x3e:
    case 0x3f:
        // MMD3x: OctaMED doesn't support value interpolation
        command[1] &= 0x7f;
    default:
        break;
    }
}

// Converts a song to an MMD2 module
struct MMD2 *songToMMD2(Song *song)
{
    if (song == NULL) {
        return NULL;
    }

    // Create a table for converting song commands; values need special handling
    unsigned char commandConversion[256];
    for (int command = 0; command < 256; command++) {
        commandConversion[command] = command;
    }
    commandConversion[Player::CommandPitchWheel] = 0x13;
    commandConversion[Player::CommandEndBlock] = 0x1d;
    commandConversion[Player::CommandPlayseqPosition] = 0x0b;
    commandConversion[Player::CommandSendMessage] = 0x10;
    commandConversion[Player::CommandHold] = 0x08;
    commandConversion[Player::CommandRetrigger] = 0x1f;
    commandConversion[Player::CommandDelay] = 0x20;
    commandConversion[Player::CommandVelocity] = 0x0c;
    commandConversion[Player::CommandChannelPressure] = 0x0d;
    commandConversion[Player::CommandTicksPerLine] = 0x09;
    commandConversion[Player::CommandTempo] = 0x0f;

    struct MMD2 *mmd = (struct MMD2 *)calloc(1, sizeof(struct MMD2));
    mmd->id = ID_MMD2;
    mmd->song = (struct MMD2song *)(calloc(1, sizeof(struct MMD2song)));
    mmd->blockarr = (struct MMD1Block **)calloc(song->blocks(), sizeof(struct MMD1Block *));
    mmd->expdata = (struct MMD0exp *)calloc(1, sizeof(struct MMD0exp));

    // These are here to match default OctaMED settings
    mmd->mmdflags = MMD_LOADTOFASTMEM;
    mmd->actplayline = -1;
    mmd->song->flags3 = FLAG3_FREEPAN;
    mmd->song->voladj = 100;
    mmd->song->channels = 4;
    mmd->song->mix_echodepth = 2;
    mmd->song->mix_echolen = 150;

    // Song name
    if (!song->name().isEmpty()) {
        mmd->expdata->songname = strdup(song->name().toLatin1().constData());
        mmd->expdata->songnamelen = strlen(mmd->expdata->songname) + 1;
    }

    // Tempo
    mmd->song->deftempo = song->tempo();
    mmd->song->flags2 = (4 - 1) | FLAG2_BPM;
    mmd->song->tempo2 = song->ticksPerLine();

    // Sections
    mmd->song->songlen = song->sections();
    mmd->song->sectiontable = (unsigned short *)calloc(mmd->song->songlen, sizeof(unsigned char));
    for (int section = 0; section < song->sections(); section++) {
        mmd->song->sectiontable[section] = song->section(section);
    }

    // Playing sequences
    mmd->song->numpseqs = song->playseqs();
    mmd->song->playseqtable = (struct PlaySeq **)calloc(mmd->song->numpseqs, sizeof(struct PlaySeq *));
    for (int number = 0; number < song->playseqs(); number++) {
        Playseq *playseq = song->playseq(number);
        struct PlaySeq *PlaySeq = (struct PlaySeq *)calloc(1, sizeof(struct PlaySeq) + playseq->length() * sizeof(unsigned short));
        mmd->song->playseqtable[number] = PlaySeq;
        if (!playseq->name().isEmpty()) {
            strncpy(PlaySeq->name, playseq->name().toLatin1().constData(), 32);
            PlaySeq->name[31] = 0;
        }
        PlaySeq->length = playseq->length();
        for (int position = 0; position < mmd->song->playseqtable[number]->length; position++) {
            PlaySeq->seq[position] = playseq->at(position);
        }
    }

    // Blocks
    mmd->song->numblocks = song->blocks();
    mmd->blockarr = (struct MMD1Block **)calloc(mmd->song->numblocks, sizeof(struct MMD1Block *));
    for (int number = 0; number < mmd->song->numblocks; number++) {
        Block *block = song->block(number);
        struct MMD1Block *MMD1Block = (struct MMD1Block *)calloc(1, sizeof(struct MMD1Block) + 4 * block->tracks() * block->length());
        unsigned char *notes = (unsigned char *)MMD1Block + sizeof(struct MMD1Block);

        if (block->commandPages() > 1 || !block->name().isNull()) {
            MMD1Block->info = (BlockInfo *)calloc(1, sizeof(BlockInfo));
        }

        if (block->commandPages() > 1) {
            MMD1Block->info->pagetable = (BlockCmdPageTable *)calloc(1, sizeof(BlockCmdPageTable) + (block->commandPages() - 1) * sizeof(unsigned short *));
            MMD1Block->info->pagetable->num_pages = block->commandPages() - 1;
        }

        mmd->blockarr[number] = MMD1Block;

        if (!block->name().isNull()) {
            MMD1Block->info->blockname = strdup(block->name().toLatin1().constData());
            MMD1Block->info->blocknamelen = strlen(MMD1Block->info->blockname);
        }
        MMD1Block->numtracks = block->tracks();
        MMD1Block->lines = block->length() - 1;

        int controllersUsed = 0;
        for (int line = 0; line < (MMD1Block->lines + 1); line++)
            for (int track = 0; track < MMD1Block->numtracks; track++) {
                notes[(line * block->tracks() + track) * 4] = block->note(line, track);
                notes[(line * block->tracks() + track) * 4 + 1] = block->instrument(line, track);
                notes[(line * block->tracks() + track) * 4 + 2] = commandConversion[block->command(line, track, 0)];
                notes[(line * block->tracks() + track) * 4 + 3] = block->commandValue(line, track, 0);

                // Convert MIDI controllers to MMD3x commands
                if (notes[(line * block->tracks() + track) * 4 + 2] >= 0x80) {
                    if (mmd->expdata->mmdcmd3x == NULL) {
                        mmd->expdata->mmdcmd3x = (struct MMDMIDICmd3x *)calloc(1, sizeof(struct MMDMIDICmd3x));
                        mmd->expdata->mmdcmd3x->num_of_settings = 15;
                        mmd->expdata->mmdcmd3x->ctrlr_types = (unsigned char *)calloc(15, sizeof(unsigned char));
                        mmd->expdata->mmdcmd3x->ctrlr_numbers = (unsigned short *)calloc(15, sizeof(unsigned short));
                    }

                    // Check if the controller already has a MMD3x command
                    int controller;
                    for (controller = 0; controller < controllersUsed; controller++) {
                        if (mmd->expdata->mmdcmd3x->ctrlr_numbers[controller] == notes[(line * block->tracks() + track) * 4 + 2] - 0x80) {
                            break;
                        }
                    }

                    if (controller == controllersUsed) {
                        if (controllersUsed < 15) {
                            mmd->expdata->mmdcmd3x->ctrlr_numbers[controller] = notes[(line * block->tracks() + track) * 4 + 2] - 0x80;
                            controllersUsed++;
                            notes[(line * block->tracks() + track) * 4 + 2] = 0x31 + controller;
                        }
                    } else {
                        notes[(line * block->tracks() + track) * 4 + 2] = 0x31 + controller;
                    }
                }

                // Command values may need special handling
                songCommandToMMD2Command(&(notes[(line * block->tracks() + track) * 4 + 2]));
            }

        if (MMD1Block->info != NULL && MMD1Block->info->pagetable != NULL) {
            for (int commandPage = 0; commandPage < MMD1Block->info->pagetable->num_pages; commandPage++) {
                MMD1Block->info->pagetable->page[commandPage] = (unsigned short *)calloc(MMD1Block->numtracks * (MMD1Block->lines + 1) * 2, sizeof(unsigned short));
                for (int line = 0; line < (MMD1Block->lines + 1); line++)
                    for (int track = 0; track < MMD1Block->numtracks; track++) {
                        unsigned char *commands = (unsigned char *)MMD1Block->info->pagetable->page[commandPage];
                        commands[(line * MMD1Block->numtracks + track) * 2] = commandConversion[block->command(line, track, commandPage)];
                        commands[(line * MMD1Block->numtracks + track) * 2 + 1] = block->commandValue(line, track, commandPage);
                        // Command values may need special handling
                        songCommandToMMD2Command(&(commands[(line * MMD1Block->numtracks + track) * 2]));
                    }
            }
        }
    }

    // Instruments
    mmd->song->numsamples = song->instruments();
    mmd->smplarr = (struct InstrHdr **)calloc(mmd->song->numsamples, sizeof(struct InstrHdr *));
    mmd->expdata->s_ext_entries = mmd->song->numsamples;
    // I have no idea why this is not sizeof(struct InstrExt); like it should be
    mmd->expdata->s_ext_entrsz = 18;
    mmd->expdata->i_ext_entries = mmd->song->numsamples;
    // I have no idea why this is not sizeof(struct MMDInstrInfo); like it should be
    mmd->expdata->i_ext_entrsz = 42;
    mmd->expdata->exp_smp = (struct InstrExt *)calloc(mmd->expdata->s_ext_entries, sizeof(struct InstrExt));
    mmd->expdata->iinfo = (struct MMDInstrInfo *)calloc(mmd->expdata->i_ext_entries, sizeof(struct MMDInstrInfo));
    for (int instrument = 0; instrument < mmd->song->numsamples; instrument++) {
        strncpy((char *)mmd->expdata->iinfo[instrument].name, song->instrument(instrument)->name().toLatin1().constData(), 40);
        mmd->expdata->iinfo[instrument].name[39] = 0;
        mmd->expdata->exp_smp[instrument].hold = song->instrument(instrument)->hold();
        mmd->song->sample[instrument].midich = song->instrument(instrument)->midiChannel() + 1;
        mmd->song->sample[instrument].midipreset = 0;
        mmd->song->sample[instrument].strans = song->instrument(instrument)->transpose();
        mmd->song->sample[instrument].svol = (song->instrument(instrument)->defaultVelocity() + 1) / 2;
    }

    // Track volumes
    mmd->song->numtracks = song->maxTracks();
    mmd->song->trackvols = (unsigned char *)calloc(mmd->song->numtracks, sizeof(unsigned char));
    mmd->song->trackpans = (char *)calloc(mmd->song->numtracks, sizeof(char));
    for (int track = 0; track < mmd->song->numtracks; track++) {
        mmd->song->trackvols[track] = (song->track(track)->volume() + 1) / 2;
    }
    mmd->song->mastervol = (song->masterVolume() + 1) / 2;

    // SysEX dumps
    if (song->messages() > 0) {
        mmd->expdata->dumps = (struct MMDDumpData *)calloc(1, sizeof(struct MMDDumpData) + song->messages() * sizeof(struct MMDDump *));
        mmd->expdata->dumps->numdumps = song->messages();
        struct MMDDump **dumps = (struct MMDDump **)((char *)mmd->expdata->dumps + sizeof(struct MMDDumpData));
        for (int message = 0; message < mmd->expdata->dumps->numdumps; message++) {
            dumps[message] = (struct MMDDump *)calloc(1, sizeof(struct MMDDump));
            dumps[message]->length = song->message(message)->length();
            dumps[message]->data = (unsigned char *)calloc(dumps[message]->length, sizeof(unsigned char));
            memcpy(dumps[message]->data, song->message(message)->data().constData(), dumps[message]->length);
            if (!song->message(message)->name().isNull()) {
                strncpy((char *)dumps[message]->name, song->message(message)->name().toLatin1().constData(), 20);
                dumps[message]->name[19] = 0;
                dumps[message]->ext_len = strlen((const char *)dumps[message]->name);
            }
        }
    }

    return mmd;
}

// Converts a song to a standard MIDI file
SMF *songToSMF(Song *song)
{
    if (song == NULL) {
        return NULL;
    }

    SMF *smf = new SMF;
    BufferMIDI *midi = new BufferMIDI;
    Player *player = new Player(midi, song);
    QMetaObject::invokeMethod(player, "init");
    player->playWithoutScheduling();
    char endMTrk[3];
    endMTrk[0] = 0xff;
    endMTrk[1] = 0x2f;
    endMTrk[2] = 0x00;
    midi->output(0)->writeRaw(QByteArray(endMTrk, 3));
    smf->addTrack(midi->data());
    delete player;
    delete midi;

    return smf;
}
