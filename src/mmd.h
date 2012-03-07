/* mmd.h
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

#ifndef _MMD_H
#define _MMD_H

#define ID_MMD0 0x4d4d4430
#define ID_MMD1 0x4d4d4431
#define ID_MMD2 0x4d4d4432
#define ID_MMD3 0x4d4d4433
#define MMD_LOADTOFASTMEM 0x1

// 32 bit sizes of structures
#define sizeof_struct_MMD0 52
#define sizeof_struct_MMD2 52
#define sizeof_struct_MMD0sample 8
#define sizeof_struct_MMD0song 788
#define sizeof_struct_MMD2song 788
#define sizeof_struct_PlaySeq 44
#define sizeof_struct_BlockCmdPageTable 4
#define sizeof_struct_BlockInfo 36
#define sizeof_struct_MMD0Block 2
#define sizeof_struct_MMD1Block 8
#define sizeof_struct_InstrHdr 8
#define sizeof_struct_SynthInstr 536
#define sizeof_struct_MMD0exp 84
#define sizeof_struct_InstrExt 20
#define sizeof_struct_MMDInstrInfo 40
#define sizeof_struct_NotationInfo 108
#define sizeof_struct_MMDDumpData 8
#define sizeof_struct_MMDDump 32
#define sizeof_struct_MMDInfo 12
#define sizeof_struct_MMDARexx 8
#define sizeof_struct_MMDARexxTrigCmd 20
#define sizeof_struct_MMDMIDICmd3x 12

struct MMD0 {
    unsigned int id;
    unsigned int modlen;
    struct MMD0song *song;
    unsigned short psecnum; // for the player routine, MMD2 only
    unsigned short pseq; //  "   "   "   "
    struct MMD0Block **blockarr;
    unsigned char mmdflags;
    unsigned char reserved[3];
    struct InstrHdr **smplarr;
    unsigned int reserved2;
    struct MMD0exp *expdata;
    unsigned int reserved3;
    unsigned short pstate; // some data for the player routine
    unsigned short pblock;
    unsigned short pline;
    unsigned short pseqnum;
    short actplayline;
    unsigned char counter;
    unsigned char extra_songs; // number of songs - 1
};

struct MMD2 {
    unsigned int id;
    unsigned int modlen;
    struct MMD2song *song;
    unsigned short psecnum; // for the player routine, MMD2 only
    unsigned short pseq; //  "   "   "   "
    struct MMD1Block **blockarr;
    unsigned char mmdflags;
    unsigned char reserved[3];
    struct InstrHdr **smplarr;
    unsigned int reserved2;
    struct MMD0exp *expdata;
    unsigned int reserved3;
    unsigned short pstate; // some data for the player routine
    unsigned short pblock;
    unsigned short pline;
    unsigned short pseqnum;
    short actplayline;
    unsigned char counter;
    unsigned char extra_songs; // number of songs - 1
};

struct MMD0sample {
    unsigned short rep, replen; // offs: 0(s), 2(s)
    unsigned char midich; // offs: 4(s)
    unsigned char midipreset; // offs: 5(s)
    unsigned char svol; // offs: 6(s)
    char strans; // offs: 7(s)
};

struct MMD0song {
    struct MMD0sample sample[63]; // 63 * 8 bytes = 504 bytes
    unsigned short numblocks; // offs: 504
    unsigned short songlen; // offs: 506
    unsigned char playseq[256]; // offs: 508
    unsigned short deftempo; // offs: 764
    char playtransp; // offs: 766
    unsigned char flags; // offs: 767
    unsigned char flags2; // offs: 768
    unsigned char tempo2; // offs: 769
    unsigned char trkvol[16]; // offs: 770
    unsigned char mastervol; // offs: 786
    unsigned char numsamples; // offs: 787
}; // length = 788 bytes

#define FLAG_FILTERON 0x1
#define FLAG_JUMPINGON  0x2
#define FLAG_JUMP8TH    0x4
#define FLAG_INSTRSATT  0x8
#define FLAG_VOLHEX     0x10
#define FLAG_STSLIDE    0x20
#define FLAG_8CHANNEL   0x40
#define FLAG_SLOWHQ     0X80

#define FLAG2_BMASK 0x1F
#define FLAG2_BPM   0x20
#define FLAG2_MIX   0x80

struct MMD2song {
    struct MMD0sample sample[63];
    unsigned short numblocks;
    unsigned short songlen; // NOTE: number of sections in MMD2
    struct PlaySeq **playseqtable;
    unsigned short *sectiontable; // unsigned short section numbers
    unsigned char *trackvols; // unsigned char track volumes
    unsigned short numtracks; // max. number of tracks in the song (also the number of entries in 'trackvols' table)
    unsigned short numpseqs; // number of PlaySeqs in 'playseqtable'
    char *trackpans; // NULL means 'all centered
    unsigned int flags3; // see defs below
    unsigned short voladj; // volume adjust (%), 0 means 100
    unsigned short channels; // mixing channels, 0 means 4
    unsigned char mix_echotype; // 0 = nothing, 1 = normal, 2 = cross
    unsigned char mix_echodepth; // 1 - 6, 0 = default
    unsigned short mix_echolen; // echo length in milliseconds
    char mix_stereosep; // stereo separation
    unsigned char pad0[223]; // reserved for future expansion
    // Below fields are MMD0/MMD1-compatible (except pad1[])
    unsigned short deftempo;
    char playtransp;
    unsigned char flags;
    unsigned char flags2;
    unsigned char tempo2;
    unsigned char pad1[16]; // used to be trackvols, in MMD2 reserved
    unsigned char mastervol;
    unsigned char numsamples;
};

struct PlaySeq {
    char name[32]; // (0)  31 chars + \0
    unsigned int reserved[2]; // (32) for possible extensions
    unsigned short length; // (40) # of entries
    // Commented out, not all compilers may like it...
    unsigned short seq[0]; // (42) block numbers..
};

#define FLAG3_STEREO     0x1
#define FLAG3_FREEPAN    0x2

struct BlockCmdPageTable {
    unsigned short num_pages;
    unsigned short reserved;
    unsigned short *page[0];
};

struct BlockInfo {
    unsigned int *hlmask;
    char *blockname;
    unsigned int blocknamelen;
    struct BlockCmdPageTable *pagetable;
    unsigned int reserved[5];
};

struct MMD0Block {
    unsigned char numtracks, lines;
};

struct MMD1Block {
    unsigned short numtracks;
    unsigned short lines;
    struct BlockInfo *info;
};

struct InstrHdr {
    unsigned int length;
    short type;
    // Followed by actual data
};

struct SynthInstr {
    unsigned int length; // length of this struct
    short type; // -1 or -2 (offs: 4)
    unsigned char defaultdecay;
    unsigned char reserved[3];
    unsigned short rep;
    unsigned short replen;
    unsigned short voltbllen; // offs: 14
    unsigned short wftbllen; // offs: 16
    unsigned char volspeed; // offs: 18
    unsigned char wfspeed; // offs: 19
    unsigned short wforms; // offs: 20
    unsigned char voltbl[128]; // offs: 22
    unsigned char wftbl[128]; // offs: 150
    struct SynthWF *wf[64]; // offs: 278
};

struct MMD0exp {
    struct MMD0 *nextmod;
    struct InstrExt *exp_smp;
    unsigned short s_ext_entries;
    unsigned short s_ext_entrsz;
    unsigned char *annotxt;
    unsigned int annolen;
    struct MMDInstrInfo *iinfo;
    unsigned short i_ext_entries;
    unsigned short i_ext_entrsz;
    unsigned int jumpmask;
    unsigned short *rgbtable;
    unsigned char channelsplit[4];
    struct NotationInfo *n_info;
    char *songname;
    unsigned int songnamelen;
    struct MMDDumpData *dumps;
    struct MMDInfo *mmdinfo;
    struct MMDARexx *mmdrexx;
    struct MMDMIDICmd3x *mmdcmd3x;
    unsigned int reserved2[3];
    unsigned int tag_end;
};

struct InstrExt {
    unsigned char hold;
    unsigned char decay;
    unsigned char suppress_midi_off;
    char finetune;
    // Below fields saved by >= V5
    unsigned char default_pitch;
    unsigned char instr_flags;
    unsigned short long_midi_preset;
    // Below fields saved by >= V5.02
    unsigned char output_device;
    unsigned char reserved;
    // Below fields saved by >= V7
    unsigned int long_repeat;
    unsigned int long_replen;
};

struct MMDInstrInfo {
    unsigned char name[40];
};

struct NotationInfo {
    unsigned char n_of_sharps;
    unsigned char flags;
    short trksel[5];
    unsigned char trkshow[16];
    unsigned char trkghost[16];
    char notetr[63];
    unsigned char pad;
};

struct MMDDumpData {
    unsigned short numdumps;
    unsigned short reserved[3];
};

struct MMDDump {
    unsigned int length;
    unsigned char *data;
    unsigned short ext_len;
    // if ext_len >= 20:
    unsigned char name[20];
};

struct MMDInfo {
    struct MMDInfo *next;
    unsigned short reserved;
    unsigned short type;
    unsigned int length;
    // data follows...
};

struct MMDARexx {
    unsigned short res;
    unsigned short trigcmdlen;
    struct MMDARexxTrigCmd *trigcmd;
};

struct MMDARexxTrigCmd {
    struct MMDARexxTrigCmd *next;
    unsigned char cmdnum;
    unsigned char pad;
    short cmdtype;
    char *cmd;
    char *port;
    unsigned short cmd_len;
    unsigned short port_len;
};

struct MMDMIDICmd3x {
    // current version = 0
    unsigned char struct_vers;
    unsigned char pad;
    // number of Cmd3x settings (currently set to 15)
    unsigned short num_of_settings;
    // controller types [ignore unknown types!!]
    unsigned char *ctrlr_types;
    // controller numbers
    unsigned short *ctrlr_numbers;
};

#define	MCS_TYPE_STD_MSB 0
#define	MCS_TYPE_STD_LSB 1
#define	MCS_TYPE_RPN_MSB 2
#define	MCS_TYPE_RPN_LSB 3
#define	MCS_TYPE_NRPN_MSB 4
#define	MCS_TYPE_NRPN_LSB 5

// MMD alloc, free and parse functions
int MMD2_length_get(struct MMD2 *);
struct MMD2 *MMD2_load(const char *);
void MMD2_save(struct MMD2 *, const char *);
void MMD2_free(struct MMD2 *);
void MMD2song_free(struct MMD2song *);
void MMD1Block_free(struct MMD1Block *);
void PlaySeq_free(struct PlaySeq *);
struct MMD2 *MMD2_parse(unsigned char *, int);
struct MMD2song *MMD2song_parse(unsigned char *, int);
struct MMD1Block *MMD1Block_parse(unsigned char *, int);
struct MMD0exp *MMD0exp_parse(unsigned char *, int);

#endif
