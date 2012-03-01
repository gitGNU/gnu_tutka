/* mmd.cpp
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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "mmd.h"

#define getlong(a) (((*(a+0))<<24)|((*(a+1))<<16)|((*(a+2))<<8)|((*(a+3))))
#define getword(a) (((*(a+0))<<8)|((*(a+1))))
#define getbyte(a) (*(a))
#define putlong(a, b) *(a)=((b)>>24); *(a+1)=(((b)>>16)&0xff); *(a+2)=(((b)>>8)&0xff); *(a+3)=((b)&0xff)
#define putword(a, b) *(a)=((b)>>8); *(a+1)=((b)&0xff)
#define putbyte(a, b) *(a)=(b)

int MMD2_length_get(struct MMD2 *mmd)
{
    if (mmd == NULL) {
        return 0;
    }

    int length = 0, i, j;

    /* MMD2 */
    length += sizeof(struct MMD2);
    /* PlaySeqs */
    for (i = 0; i < mmd->song->numpseqs; i++) {
        length += sizeof(struct PlaySeq) + (mmd->song->playseqtable[i]->length - 1) * sizeof(unsigned short);
    }
    length += mmd->song->numpseqs * sizeof(int);
    /* sectiontable */
    length += mmd->song->songlen * sizeof(unsigned short);
    /* trackvols */
    length += ((mmd->song->numtracks + 1) & 0xfffffffe);
    /* trackpans */
    length += ((mmd->song->numtracks + 1) & 0xfffffffe);
    /* MMD2Song */
    length += sizeof(struct MMD2song);
    /* SampleArr */
    if (mmd->smplarr != NULL)
        length += mmd->song->numsamples * sizeof(int);

    /* Blockarr */
    for (i = 0; i < mmd->song->numblocks; i++) {
        if (mmd->blockarr[i]->info != NULL) {
            if (mmd->blockarr[i]->info->hlmask != NULL) {
                /* hlmask */
                length += (1 + mmd->blockarr[i]->lines / 32) * sizeof(unsigned int);
            }
            if (mmd->blockarr[i]->info->blockname != NULL) {
                length += (mmd->blockarr[i]->info->blocknamelen + 1) & 0xfffffffe;
            }
            if (mmd->blockarr[i]->info->pagetable != NULL) {
                for (j = 0; j < mmd->blockarr[i]->info->pagetable->num_pages; j++) {
                    /* page */
                    length += (mmd->blockarr[i]->numtracks * (mmd->blockarr[i]->lines + 1)) * sizeof(unsigned short);
                }
                /* pagetable */
                length += sizeof(struct BlockCmdPageTable) + mmd->blockarr[i]->info->pagetable->num_pages * sizeof(int);
            }
            /* blockinfo */
            length += sizeof(struct BlockInfo);
        }
        /* block */
        length += sizeof(struct MMD1Block) + 4 * mmd->blockarr[i]->numtracks * (mmd->blockarr[i]->lines + 1);
    }
    length += mmd->song->numblocks * sizeof(int);

    if (mmd->expdata != NULL) {
        /* annotext */
        if (mmd->expdata->annotxt != NULL)
            length += (mmd->expdata->annolen + 1) & 0xfffffffe;
        /* instrext */
        if (mmd->expdata->exp_smp != NULL) {
            for (i = 0; i < mmd->expdata->s_ext_entries; i++)
                length += mmd->expdata->s_ext_entrsz;
        }
        /* instrinfo */
        if (mmd->expdata->iinfo != NULL) {
            for (i = 0; i < mmd->expdata->i_ext_entries; i++)
                length += mmd->expdata->i_ext_entrsz;
        }
        /* notationinfo */
        if (mmd->expdata->n_info != NULL)
            length += sizeof(struct NotationInfo);
        /* mmdinfo */
        if (mmd->expdata->mmdinfo != NULL)
            length += sizeof(struct MMDInfo) + ((mmd->expdata->mmdinfo->length + 1) & 0xfffffffe);
        /* songname */
        if (mmd->expdata->songname != NULL)
            length += (mmd->expdata->songnamelen + 1) & 0xfffffffe;
        if (mmd->expdata->dumps != NULL) {
            struct MMDDump **dumps = (struct MMDDump **)((char *)mmd->expdata->dumps + sizeof(struct MMDDumpData));
            /* dump data */
            for (i = 0; i < mmd->expdata->dumps->numdumps; i++) {
                /* dump */
                length += ((dumps[i]->length + 1) & 0xfffffffe);
                length += 10 + dumps[i]->ext_len;
            }
            length += sizeof(struct MMDDumpData) + mmd->expdata->dumps->numdumps * sizeof(int);
        }
        if (mmd->expdata->mmdrexx != NULL) {
            if (mmd->expdata->mmdrexx->trigcmd != NULL) {
                struct MMDARexxTrigCmd *trig = mmd->expdata->mmdrexx->trigcmd;
                while (trig != NULL) {
                    trig = trig->next;
                    if (trig->cmd != NULL)
                        length += (trig->cmd_len + 2) & 0xfffffffe;
                    if (trig->port != NULL)
                        length += (trig->port_len + 2) & 0xfffffffe;
                    /* mmdarexxtrigcmd */
                    length += sizeof(struct MMDARexxTrigCmd);
                }
                /* mmdarexx */
                length += sizeof(struct MMDARexx);
            }
        }
        if (mmd->expdata->mmdcmd3x != NULL) {
            /* types */
            length += mmd->expdata->mmdcmd3x->num_of_settings;
            if ((mmd->expdata->mmdcmd3x->num_of_settings & 1) == 1)
                length++;
            /* numbers */
            length += 2 * mmd->expdata->mmdcmd3x->num_of_settings;
            /* mmdcmd3x */
            length += 12;
        }
        /* MMD0exp */
        length += sizeof(struct MMD0exp);
    }

    return length;
}

/* Loads an MMD2 file and returns a pointer to its MMD0 structure */
struct MMD2 *MMD2_load(const char *filename)
{
    if (filename == NULL) {
        return NULL;
    }

    FILE *file;
    struct MMD2 *mmd;
    int flength;
    unsigned char *data;

    if ((file = fopen(filename, "r")) == NULL)
        return NULL;

    /* Get file length */
    fseek(file, 0, SEEK_END);
    flength = ftell(file);
    rewind(file);

    /* Allocate memory and read file */
    data = (unsigned char *)malloc(flength);
    fread(data, sizeof(unsigned char), flength, file);

    mmd = MMD2_parse(data, 0);

    free(data);
    fclose(file);
    return mmd;
}

/* Saves an MMD2 file */
void MMD2_save(struct MMD2 *mmd, const char *filename)
{
    if (mmd == NULL || filename == NULL) {
        return;
    }

    FILE *file;
    int offsets[16], i, j;
    unsigned char *data;

    if (mmd->modlen == 0)
        mmd->modlen = MMD2_length_get(mmd);

    data = (unsigned char *)calloc(mmd->modlen, sizeof(unsigned char));

    /* MMD2 */
    putlong(data + 0, mmd->id);
    putlong(data + 4, mmd->modlen);
    putbyte(data + 20, mmd->mmdflags);
    putword(data + 48, mmd->actplayline);
    putbyte(data + 51, mmd->extra_songs);
    offsets[0] = sizeof(struct MMD2);
    /* PlaySeqs */
    offsets[1] = offsets[0];
    for (i = 0; i < mmd->song->numpseqs; i++)
        offsets[1] += sizeof(struct PlaySeq) + (mmd->song->playseqtable[i]->length - 1) * sizeof(unsigned short);
    /* offsets[1]: playseqtable */
    for (i = 0; i < mmd->song->numpseqs; i++) {
        memcpy(data + offsets[0], mmd->song->playseqtable[i]->name, 32);
        putword(data + offsets[0] + 40, mmd->song->playseqtable[i]->length);
        for (j = 0; j < mmd->song->playseqtable[i]->length; j++) {
            putword(data + offsets[0] + 42 + j * sizeof(unsigned short), mmd->song->playseqtable[i]->seq[j])
                    ;
        }
        putlong(data + offsets[1] + i * sizeof(int), offsets[0]);
        offsets[0] += sizeof(struct PlaySeq) + (mmd->song->playseqtable[i]->length - 1) * sizeof(unsigned short);
    }
    offsets[0] += mmd->song->numpseqs * sizeof(int);
    /* sectiontable */
    for (i = 0; i < mmd->song->songlen; i++) {
        putword(data + offsets[0] + i * sizeof(unsigned short), mmd->song->sectiontable[i])
                ;
    }
    offsets[0] += mmd->song->songlen * sizeof(unsigned short);
    /* trackvols */
    for (i = 0; i < mmd->song->numtracks; i++) {
        putbyte(data + offsets[0] + i, mmd->song->trackvols[i]);
    }
    offsets[0] += ((mmd->song->numtracks + 1) & 0xfffffffe);
    /* trackpans */
    for (i = 0; i < mmd->song->numtracks; i++) {
        putbyte(data + offsets[0] + i, mmd->song->trackpans[i]);
    }
    offsets[0] += ((mmd->song->numtracks + 1) & 0xfffffffe);

    /* MMD2Song */
    putlong(data + 8, offsets[0]);
    for (i = 0; i < 63; i++) {
        putword(data + offsets[0] + i * sizeof(struct MMD0sample) + 0, mmd->song->sample[i].rep);
        putword(data + offsets[0] + i * sizeof(struct MMD0sample) + 2, mmd->song->sample[i].replen);
        putbyte(data + offsets[0] + i * sizeof(struct MMD0sample) + 4, mmd->song->sample[i].midich);
        putbyte(data + offsets[0] + i * sizeof(struct MMD0sample) + 5, mmd->song->sample[i].midipreset);
        putbyte(data + offsets[0] + i * sizeof(struct MMD0sample) + 6, mmd->song->sample[i].svol);
        putbyte(data + offsets[0] + i * sizeof(struct MMD0sample) + 7, mmd->song->sample[i].strans);
    }
    putword(data + offsets[0] + 504, mmd->song->numblocks);
    putword(data + offsets[0] + 506, mmd->song->songlen);
    putlong(data + offsets[0] + 508, offsets[1]);
    putlong(data + offsets[0] + 512, offsets[1] + mmd->song->numpseqs * sizeof(int));
    putlong(data + offsets[0] + 516, offsets[1] + mmd->song->numpseqs * sizeof(int) + mmd->song->songlen * sizeof(unsigned short));
    putword(data + offsets[0] + 520, mmd->song->numtracks);
    putword(data + offsets[0] + 522, mmd->song->numpseqs);
    putlong(data + offsets[0] + 524, offsets[0] - ((mmd->song->numtracks + 1) & 0xfffffffe));
    putlong(data + offsets[0] + 528, mmd->song->flags3);
    putword(data + offsets[0] + 532, mmd->song->voladj);
    putword(data + offsets[0] + 534, mmd->song->channels);
    putbyte(data + offsets[0] + 536, mmd->song->mix_echotype);
    putbyte(data + offsets[0] + 537, mmd->song->mix_echodepth);
    putword(data + offsets[0] + 538, mmd->song->mix_echolen);
    putbyte(data + offsets[0] + 540, mmd->song->mix_stereosep);
    putword(data + offsets[0] + 764, mmd->song->deftempo);
    putbyte(data + offsets[0] + 766, mmd->song->playtransp);
    putbyte(data + offsets[0] + 767, mmd->song->flags);
    putbyte(data + offsets[0] + 768, mmd->song->flags2);
    putbyte(data + offsets[0] + 769, mmd->song->tempo2);
    putbyte(data + offsets[0] + 786, mmd->song->mastervol);
    putbyte(data + offsets[0] + 787, mmd->song->numsamples);

    offsets[0] += sizeof(struct MMD2song);
    /* SampleArr */
    if (mmd->smplarr != NULL) {
        /* This needs fixing. This points to the space after MMD0exp. */
        putlong(data + 24, offsets[0]);
        offsets[0] += mmd->song->numsamples * sizeof(int);
    }

    /* Block 0 */
    offsets[1] = offsets[0];
    for (i = 0; i < mmd->song->numblocks; i++) {
        offsets[1] += sizeof(struct MMD1Block) + 4 * mmd->blockarr[i]->numtracks * (mmd->blockarr[i]->lines + 1);
        if (mmd->blockarr[i]->info != NULL) {
            offsets[1] += sizeof(struct BlockInfo);
            if (mmd->blockarr[i]->info->hlmask != NULL)
                offsets[1] += (1 + mmd->blockarr[i]->lines / 32) * sizeof(unsigned int);
            if (mmd->blockarr[i]->info->blockname != NULL)
                offsets[1] += (mmd->blockarr[i]->info->blocknamelen + 1) & 0xfffffffe;
            if (mmd->blockarr[i]->info->pagetable != NULL)
                offsets[1] += sizeof(struct BlockCmdPageTable) + mmd->blockarr[i]->info->pagetable->num_pages * sizeof(unsigned short *) + mmd->blockarr[i]->info->pagetable->num_pages * (mmd->blockarr[i]->numtracks * (mmd->blockarr[i]->lines + 1)) * sizeof(unsigned short);
        }
    }

    /* Blockarr */
    putlong(data + 16, offsets[1]);
    for (i = 0; i < mmd->song->numblocks; i++) {
        offsets[2] = 0;
        if (mmd->blockarr[i]->info != NULL) {
            offsets[3] = 0;
            offsets[4] = 0;
            offsets[5] = 0;
            if (mmd->blockarr[i]->info->hlmask != NULL) {
                /* hlmask */
                offsets[3] = offsets[0];
                for (j = 0; j < 1 + mmd->blockarr[i]->lines / 32; j++) {
                    putlong(data + offsets[0] + j * sizeof(int), mmd->blockarr[i]->info->hlmask[j])
                            ;
                }
                offsets[0] += (1 + mmd->blockarr[i]->lines / 32) * sizeof(unsigned int);
            }
            if (mmd->blockarr[i]->info->blockname != NULL) {
                offsets[4] = offsets[0];
                memcpy(data + offsets[0], mmd->blockarr[i]->info->blockname, mmd->blockarr[i]->info->blocknamelen);
                offsets[0] += (mmd->blockarr[i]->info->blocknamelen + 1) & 0xfffffffe;
            }
            if (mmd->blockarr[i]->info->pagetable != NULL) {
                offsets[2] = offsets[0];
                for (j = 0; j < mmd->blockarr[i]->info->pagetable->num_pages; j++)
                    offsets[2] += (mmd->blockarr[i]->numtracks * (mmd->blockarr[i]->lines + 1)) * sizeof(unsigned short);

                for (j = 0; j < mmd->blockarr[i]->info->pagetable->num_pages; j++) {
                    /* page */
                    memcpy(data + offsets[0], mmd->blockarr[i]->info->pagetable->page[j], (mmd->blockarr[i]->numtracks * (mmd->blockarr[i]->lines + 1)) * sizeof(unsigned short));
                    putlong(data + offsets[2] + 4 + sizeof(int) * j, offsets[0]);
                    offsets[0] += (mmd->blockarr[i]->numtracks * (mmd->blockarr[i]->lines + 1)) * sizeof(unsigned short);
                }
                /* pagetable */
                offsets[5] = offsets[0];
                putword(data + offsets[0] + 0, mmd->blockarr[i]->info->pagetable->num_pages);
                putword(data + offsets[0] + 2, mmd->blockarr[i]->info->pagetable->reserved);
                offsets[0] += sizeof(struct BlockCmdPageTable) + mmd->blockarr[i]->info->pagetable->num_pages * sizeof(int);
            }
            /* blockinfo */
            offsets[2] = offsets[0];
            putlong(data + offsets[0] + 0, offsets[3]);
            putlong(data + offsets[0] + 4, offsets[4]);
            putlong(data + offsets[0] + 8, mmd->blockarr[i]->info->blocknamelen);
            putlong(data + offsets[0] + 12, offsets[5]);
            offsets[0] += sizeof(struct BlockInfo);
        }
        /* block */
        putlong(data + offsets[1] + i * sizeof(int), offsets[0]);
        putword(data + offsets[0] + 0, mmd->blockarr[i]->numtracks);
        putword(data + offsets[0] + 2, mmd->blockarr[i]->lines);
        putlong(data + offsets[0] + 4, offsets[2]);
        memcpy(data + offsets[0] + 8, (unsigned char *)mmd->blockarr[i] + sizeof(struct MMD1Block), 4 * mmd->blockarr[i]->numtracks * (mmd->blockarr[i]->lines + 1));
        offsets[0] += sizeof(struct MMD1Block) + 4 * mmd->blockarr[i]->numtracks * (mmd->blockarr[i]->lines + 1);
    }
    offsets[0] += mmd->song->numblocks * sizeof(int);

    if (mmd->expdata != NULL) {
        offsets[1] = 0;
        /* annotext */
        if (mmd->expdata->annotxt != NULL) {
            offsets[1] = offsets[0];
            memcpy(data + offsets[0], mmd->expdata->annotxt, mmd->expdata->annolen);
            offsets[0] += (mmd->expdata->annolen + 1) & 0xfffffffe;
        }
        /* instrext */
        offsets[2] = offsets[0];
        if (mmd->expdata->exp_smp != NULL) {
            for (i = 0; i < mmd->expdata->s_ext_entries; i++) {
                putbyte(data + offsets[0] + 0, mmd->expdata->exp_smp[i].hold);
                putbyte(data + offsets[0] + 1, mmd->expdata->exp_smp[i].decay);
                putbyte(data + offsets[0] + 2, mmd->expdata->exp_smp[i].suppress_midi_off);
                putbyte(data + offsets[0] + 3, mmd->expdata->exp_smp[i].finetune);
                if (mmd->expdata->s_ext_entrsz > 4) {
                    putbyte(data + offsets[0] + 4, mmd->expdata->exp_smp[i].default_pitch);
                    putbyte(data + offsets[0] + 5, mmd->expdata->exp_smp[i].instr_flags);
                    putword(data + offsets[0] + 6, mmd->expdata->exp_smp[i].long_midi_preset);
                    if (mmd->expdata->s_ext_entrsz > 8) {
                        putbyte(data + offsets[0] + 8, mmd->expdata->exp_smp[i].output_device);
                        if (mmd->expdata->s_ext_entrsz > 10) {
                            putlong(data + offsets[0] + 10, mmd->expdata->exp_smp[i].long_repeat);
                            putlong(data + offsets[0] + 14, mmd->expdata->exp_smp[i].long_replen)
                                    ;
                        }
                    }
                }
                offsets[0] += mmd->expdata->s_ext_entrsz;
            }
        }
        /* instrinfo */
        offsets[3] = 0;
        if (mmd->expdata->iinfo != NULL) {
            offsets[3] = offsets[0];
            for (i = 0; i < mmd->expdata->i_ext_entries; i++) {
                memcpy(data + offsets[0], mmd->expdata->iinfo[i].name, sizeof(struct MMDInstrInfo));
                offsets[0] += mmd->expdata->i_ext_entrsz;
            }
        }
        /* notationinfo */
        offsets[4] = 0;
        if (mmd->expdata->n_info != NULL) {
            offsets[4] = offsets[0];
            putbyte(data + offsets[0] + 0, mmd->expdata->n_info->n_of_sharps);
            putbyte(data + offsets[0] + 1, mmd->expdata->n_info->flags);
            putword(data + offsets[0] + 2, mmd->expdata->n_info->trksel[0]);
            putword(data + offsets[0] + 4, mmd->expdata->n_info->trksel[1]);
            putword(data + offsets[0] + 6, mmd->expdata->n_info->trksel[2]);
            putword(data + offsets[0] + 8, mmd->expdata->n_info->trksel[3]);
            putword(data + offsets[0] + 10, mmd->expdata->n_info->trksel[4]);
            memcpy(data + offsets[0] + 12, mmd->expdata->n_info->trkshow, 96);
            offsets[0] += sizeof(struct NotationInfo);
        }
        /* mmdinfo */
        offsets[5] = 0;
        if (mmd->expdata->mmdinfo != NULL) {
            offsets[5] = offsets[0];
            putword(data + offsets[0] + 6, mmd->expdata->mmdinfo->type);
            putlong(data + offsets[0] + 8, mmd->expdata->mmdinfo->length);
            memcpy(data + offsets[0] + 12, ((unsigned char *)mmd->expdata->mmdinfo) + sizeof(struct MMDInfo), mmd->expdata->mmdinfo->length);
            offsets[0] += sizeof(struct MMDInfo) + ((mmd->expdata->mmdinfo->length + 1) & 0xfffffffe);
        }
        /* songname */
        offsets[6] = 0;
        if (mmd->expdata->songname != NULL) {
            offsets[6] = offsets[0];
            memcpy(data + offsets[0], mmd->expdata->songname, mmd->expdata->songnamelen);
            offsets[0] += (mmd->expdata->songnamelen + 1) & 0xfffffffe;
        }
        /* Dumps */
        offsets[7] = 0;
        if (mmd->expdata->dumps != NULL) {
            struct MMDDump **dumps = (struct MMDDump **)((char *)mmd->expdata->dumps + sizeof(struct MMDDumpData));

            offsets[7] = offsets[0];

            for (i = 0; i < mmd->expdata->dumps->numdumps; i++)
                offsets[7] += ((dumps[i]->length + 1) & 0xfffffffe) + 10 + dumps[i]->ext_len;
            putword(data + offsets[7] + 0, mmd->expdata->dumps->numdumps);

            /* dump data */
            for (i = 0; i < mmd->expdata->dumps->numdumps; i++) {
                /* dump */
                memcpy(data + offsets[0], dumps[i]->data, dumps[i]->length);
                offsets[0] += ((dumps[i]->length + 1) & 0xfffffffe);
                putlong(data + offsets[0] + 0, dumps[i]->length);
                putlong(data + offsets[0] + 4, offsets[0] - ((dumps[i]->length + 1) & 0xfffffffe));
                putword(data + offsets[0] + 8, dumps[i]->ext_len);
                putlong(data + offsets[7] + 8 + i * sizeof(int), offsets[0]);
                if (dumps[i]->ext_len >= 20)
                    memcpy(data + offsets[0] + 10, dumps[i]->name, dumps[i]->ext_len);
                offsets[0] += 10 + dumps[i]->ext_len;
            }
            offsets[0] += sizeof(struct MMDDumpData) + mmd->expdata->dumps->numdumps * sizeof(int);
        }
        /* MMDArexx */
        offsets[8] = 0;
        if (mmd->expdata->mmdrexx != NULL) {
            if (mmd->expdata->mmdrexx->trigcmd != NULL) {
                struct MMDARexxTrigCmd *trig = mmd->expdata->mmdrexx->trigcmd;
                struct MMDARexxTrigCmd **trigs;
                i = 0;
                while (trig != NULL) {
                    trig = trig->next;
                    i++;
                }
                trigs = (struct MMDARexxTrigCmd **)calloc(i + 1, sizeof(struct MMDARexxTrigCmd *));
                trig = mmd->expdata->mmdrexx->trigcmd;
                while (trig != NULL) {
                    trigs[--i] = trig;
                    trig = trig->next;
                }
                offsets[13] = 0;
                for (i = 0; trigs[i] != NULL; i++) {
                    offsets[14] = 0;
                    offsets[15] = 0;
                    if (trigs[i]->cmd != NULL) {
                        offsets[14] = offsets[0];
                        /* command */
                        memcpy(data + offsets[0], trigs[i]->cmd, trigs[i]->cmd_len);
                        offsets[0] += (trigs[i]->cmd_len + 2) & 0xfffffffe;
                    }
                    if (trigs[i]->port != NULL) {
                        offsets[15] = offsets[0];
                        /* port */
                        memcpy(data + offsets[0], trigs[i]->port, trigs[i]->port_len);
                        offsets[0] += (trigs[i]->port_len + 2) & 0xfffffffe;
                    }
                    /* mmdarexxtrigcmd */
                    putlong(data + offsets[0], offsets[13]);
                    putbyte(data + offsets[0] + 4, trigs[i]->cmdnum);
                    putword(data + offsets[0] + 6, trigs[i]->cmdtype);
                    putlong(data + offsets[0] + 8, offsets[14]);
                    putlong(data + offsets[0] + 12, offsets[15]);
                    putword(data + offsets[0] + 16, trigs[i]->cmd_len);
                    putword(data + offsets[0] + 18, trigs[i]->port_len);
                    offsets[13] = offsets[0];
                    offsets[0] += sizeof(struct MMDARexxTrigCmd);
                }
                free(trigs);
                /* mmdarexx */
                offsets[8] = offsets[0];
                putword(data + offsets[0] + 2, mmd->expdata->mmdrexx->trigcmdlen);
                putlong(data + offsets[0] + 4, offsets[13]);
                offsets[0] += sizeof(struct MMDARexx);
            }
        }
        /* MMDCmd3x */
        offsets[9] = 0;
        if (mmd->expdata->mmdcmd3x != NULL) {
            putlong(data + offsets[0] + ((mmd->expdata->mmdcmd3x->num_of_settings + 1) & 0xfffffffe) + mmd->expdata->mmdcmd3x->num_of_settings * sizeof(unsigned short) + 4, offsets[0]);
            /* types */
            for (i = 0; i < mmd->expdata->mmdcmd3x->num_of_settings; i++) {
                putbyte(data + offsets[0], mmd->expdata->mmdcmd3x->ctrlr_types[i]);
                offsets[0]++;
            }
            if ((mmd->expdata->mmdcmd3x->num_of_settings & 1) == 1)
                offsets[0]++;
            putlong(data + offsets[0] + mmd->expdata->mmdcmd3x->num_of_settings * sizeof(unsigned short) + 8, offsets[0]);
            /* numbers */
            for (i = 0; i < mmd->expdata->mmdcmd3x->num_of_settings; i++) {
                putword(data + offsets[0], mmd->expdata->mmdcmd3x->ctrlr_numbers[i]);
                offsets[0] += 2;
            }
            offsets[9] = offsets[0];
            /* mmdcmd3x */
            putbyte(data + offsets[0], mmd->expdata->mmdcmd3x->struct_vers);
            putword(data + offsets[0] + 2, mmd->expdata->mmdcmd3x->num_of_settings);
            offsets[0] += 12;
        }
        /* MMD0exp */
        putlong(data + 32, offsets[0]);
        putlong(data + offsets[0] + 4, offsets[2]);
        putword(data + offsets[0] + 8, mmd->expdata->s_ext_entries);
        putword(data + offsets[0] + 10, mmd->expdata->s_ext_entrsz);
        putlong(data + offsets[0] + 12, offsets[1]);
        putlong(data + offsets[0] + 16, mmd->expdata->annolen);
        putlong(data + offsets[0] + 20, offsets[3]);
        putword(data + offsets[0] + 24, mmd->expdata->i_ext_entries);
        putword(data + offsets[0] + 26, mmd->expdata->i_ext_entrsz);
        putlong(data + offsets[0] + 28, mmd->expdata->jumpmask);
        putbyte(data + offsets[0] + 32, mmd->expdata->channelsplit[0]);
        putbyte(data + offsets[0] + 33, mmd->expdata->channelsplit[1]);
        putbyte(data + offsets[0] + 34, mmd->expdata->channelsplit[2]);
        putbyte(data + offsets[0] + 35, mmd->expdata->channelsplit[3]);
        putlong(data + offsets[0] + 40, offsets[4]);
        putlong(data + offsets[0] + 44, offsets[6]);
        putlong(data + offsets[0] + 48, mmd->expdata->songnamelen);
        putlong(data + offsets[0] + 52, offsets[7]);
        putlong(data + offsets[0] + 56, offsets[5]);
        putlong(data + offsets[0] + 60, offsets[8]);
        putlong(data + offsets[0] + 64, offsets[9]);
        offsets[0] += sizeof(struct MMD0exp);
    }

    if ((file = fopen(filename, "w")) == NULL) {
        free(data);
        return;
    }
    fwrite(data, sizeof(unsigned char), mmd->modlen, file);
    free(data);
    fclose(file);
}

/* Frees an MMD0 structure and all associated data */
void MMD2_free(struct MMD2 *mmd)
{
    if (mmd != NULL) {
        int i;

        for (i = 0; i < mmd->song->numblocks - 1; i++) {
            MMD1Block_free(mmd->blockarr[i]);
        }
        free(mmd->blockarr);
        MMD2song_free(mmd->song);
        free(mmd->expdata);
        free(mmd);
    }
}

/* Frees an MMD2song structure and all associated data */
void MMD2song_free(struct MMD2song *song)
{
    if (song != NULL) {
        unsigned short i;

        for (i = 0; i < song->numpseqs; i++) {
            if (song->playseqtable[i]) {
                free(song->playseqtable[i]);
                song->playseqtable[i] = 0;
            }
        }
        free(song);
    }
}

/* Frees an MMD1Block structure and all associated data */
void MMD1Block_free(struct MMD1Block *block)
{
    if (block != NULL) {
        int i;

        if (block->info) {
            if (block->info->pagetable) {
                for (i = 0; i < block->info->pagetable->num_pages - 1; i++) {
                    free(block->info->pagetable->page[i]);
                }
                free(block->info->pagetable);
            }
            free(block->info);
        }
        free(block);
    }
}

void PlaySeq_free(struct PlaySeq *playseq)
{
    free(playseq);
}

struct MMD2 *MMD2_parse(unsigned char *base, int offset)
{
    unsigned char *data = base + offset, *data2, *data3;
    struct MMD2 *mmd = NULL;
    int i;

    /* Check whether the file begins with MMD2 or not */
    if (data[0] == 0x4d && data[1] == 0x4d && data[2] == 0x44 && data[3] == 0x32) {
        mmd = (struct MMD2 *)calloc(1, sizeof(struct MMD2));
        /* Read in MMD2 structure */
        mmd->id = getlong(data);
        mmd->modlen = getlong(data + 4);
        mmd->song = MMD2song_parse(base, getlong(data + 8));
        mmd->psecnum = getword(data + 12);
        mmd->pseq = getword(data + 14);
        mmd->blockarr = (struct MMD1Block **)calloc(mmd->song->numblocks, sizeof(struct MMD1Block *));
        for (i = 0; i < mmd->song->numblocks; i++)
            mmd->blockarr[i] = MMD1Block_parse(base, getlong(base + getlong(data + 16) + i * 4));
        mmd->mmdflags = getbyte(data + 20);
        if (getlong(data + 24) != 0) {
            mmd->smplarr = (struct InstrHdr **)calloc(mmd->song->numsamples, sizeof(struct InstrHdr *));
            data2 = base + getlong(data + 24);
            /* samplearr */
            for (i = 0; i < mmd->song->numsamples; i++) {
                mmd->smplarr[i] = (struct InstrHdr *)calloc(1, sizeof(struct InstrHdr));
                if (getlong(data2 + i * 4) != 0) {
                    data3 = base + getlong(data2 + i * 4);
                    mmd->smplarr[i]->length = getlong(data3);
                    mmd->smplarr[i]->type = getword(data3 + 4);
                }
            }
        }
        mmd->expdata = MMD0exp_parse(base, getlong(data + 32));
        mmd->pstate = getword(data + 40);
        mmd->pblock = getword(data + 42);
        mmd->pline = getword(data + 44);
        mmd->pseqnum = getword(data + 46);
        mmd->actplayline = getword(data + 48);
        mmd->counter = getbyte(data + 50);
        mmd->extra_songs = getbyte(data + 51);
    }
    return mmd;
}

/* Allocates a new MMD2song structure and initializes it with values in data */
struct MMD2song *MMD2song_parse(unsigned char *base, int offset)
{
    unsigned char *data = base + offset, *data2;
    struct MMD2song *song;
    int i, j;

    song = (struct MMD2song *)(calloc(1, sizeof(struct MMD2song)));
    /* Read in MMD2song structure */
    for (i = 0; i < 63; i++) {
        song->sample[i].rep = getword(data + i * 8);
        song->sample[i].replen = getword(data + i * 8 + 2);
        song->sample[i].midich = getbyte(data + i * 8 + 4);
        song->sample[i].midipreset = getbyte(data + i * 8 + 5);
        song->sample[i].svol = getbyte(data + i * 8 + 6);
        song->sample[i].strans = getbyte(data + i * 8 + 7);
    }
    song->numblocks = getword(data + 504);
    song->songlen = getword(data + 506);
    song->sectiontable = (unsigned short *)calloc(song->songlen, sizeof(unsigned short));
    data2 = base + getlong(data + 512);
    /* sectiontable */
    for (i = 0; i < song->songlen; i++)
        song->sectiontable[i] = getword(data2 + i * 2);
    song->numtracks = getword(data + 520);
    song->trackvols = (unsigned char *)calloc(song->numtracks, sizeof(unsigned char));
    data2 = base + getlong(data + 516);
    /* trackvols */
    for (i = 0; i < song->numtracks; i++)
        song->trackvols[i] = getbyte(data2 + i);
    song->numpseqs = getword(data + 522);
    song->playseqtable = (struct PlaySeq **)calloc(song->numpseqs, sizeof(struct PlaySeq *));
    /* playseqtable */
    for (i = 0; i < song->numpseqs; i++) {
        data2 = base + getlong(base + getlong(data + 508) + i * 4);
        song->playseqtable[i] = (struct PlaySeq *)calloc(1, sizeof(struct PlaySeq) + 2 * getword(data2 + 40));

        memcpy(song->playseqtable[i]->name, data2, 32);
        song->playseqtable[i]->length = getword(data2 + 40);
        for (j = 0; j < song->playseqtable[i]->length; j++) {
            song->playseqtable[i]->seq[j] = 0;
            song->playseqtable[i]->seq[j] = getword(data2 + 42 + j * 2);
        }
    }
    song->trackpans = (char *)calloc(song->numtracks, sizeof(char));
    data2 = base + getlong(data + 524);
    for (i = 0; i < song->numtracks; i++)
        song->trackpans[i] = getbyte(data2 + i);
    song->flags3 = getlong(data + 528);
    song->voladj = getword(data + 532);
    song->channels = getword(data + 534);
    song->mix_echotype = getbyte(data + 536);
    song->mix_echodepth = getbyte(data + 537);
    song->mix_echolen = getword(data + 538);
    song->mix_stereosep = getbyte(data + 540);
    song->deftempo = getword(data + 764);
    song->playtransp = getbyte(data + 766);
    song->flags = getbyte(data + 767);
    song->flags2 = getbyte(data + 768);
    song->tempo2 = getbyte(data + 769);
    song->mastervol = getbyte(data + 786);
    song->numsamples = getbyte(data + 787);

    return song;
}

struct MMD1Block *MMD1Block_parse(unsigned char *base, int offset)
{
    unsigned char *data = base + offset, *data2, *data3;
    struct MMD1Block *block = (struct MMD1Block *)calloc(1, sizeof(struct MMD1Block) + 4 * getword(data) * (getword(data + 2) + 1));
    int i, j, k;

    block->numtracks = getword(data);
    block->lines = getword(data + 2);
    /* BlockInfo */
    if (getlong(data + 4) != 0) {
        data2 = base + getlong(data + 4);
        block->info = (struct BlockInfo *)calloc(1, sizeof(struct BlockInfo));
        /* hlmask */
        if (getlong(data2) != 0) {
            data3 = base + getlong(data2);
            block->info->hlmask = (unsigned int *)calloc(1 + block->lines / 32, sizeof(unsigned int));
            for (i = 0; i < 1 + block->lines / 32; i++)
                block->info->hlmask[i] = getlong(data3 + i * sizeof(unsigned int));
        }
        /* blockname */
        block->info->blocknamelen = getlong(data2 + 8);
        if (block->info->blocknamelen > 0) {
            block->info->blockname = (char *)calloc(block->info->blocknamelen, sizeof(char));
            memcpy(block->info->blockname, base + getlong(data2 + 4), block->info->blocknamelen);
        }
        /* BlockCmdPageTables */
        if (getlong(data2 + 12) != 0) {
            data2 = base + getlong(data2 + 12);
            block->info->pagetable = (struct BlockCmdPageTable *)calloc(1, sizeof(struct BlockCmdPageTable) + getword(data2) * sizeof(unsigned short *));
            block->info->pagetable->num_pages = getword(data2);
            for (i = 0; i < block->info->pagetable->num_pages; i++) {
                data3 = base + getlong(data2 + 4 + i * 4);
                /* page */
                block->info->pagetable->page[i] = (unsigned short *)calloc(block->numtracks * (block->lines + 1) * 2, sizeof(unsigned short));
                for (j = 0; j < (block->lines + 1); j++)
                    for (k = 0; k < block->numtracks; k++) {
                        unsigned char *commands = (unsigned char *)block->info->pagetable->page[i];
                        commands[(j * block->numtracks + k) * 2] = getbyte(data3 + (j * block->numtracks + k) * 2);
                        commands[(j * block->numtracks + k) * 2 + 1] = getbyte(data3 + (j * block->numtracks + k) * 2 + 1);
                    }
            }
        }
    }

    data2 = (unsigned char *)block + sizeof(struct MMD1Block);
    for (j = 0; j < (block->lines + 1); j++)
        for (k = 0; k < block->numtracks; k++) {
            data2[(j * block->numtracks + k) * 4] = getbyte(data + 8 + (j * block->numtracks + k) * 4);
            data2[(j * block->numtracks + k) * 4 + 1] = getbyte(data + 8 + (j * block->numtracks + k) * 4 + 1);
            data2[(j * block->numtracks + k) * 4 + 2] = getbyte(data + 8 + (j * block->numtracks + k) * 4 + 2);
            data2[(j * block->numtracks + k) * 4 + 3] = getbyte(data + 8 + (j * block->numtracks + k) * 4 + 3);
        }

    return block;
}

struct MMD0exp *MMD0exp_parse(unsigned char *base, int offset)
{
    unsigned char *data = base + offset, *data2, *data3;
    struct MMD0exp *mmd0exp = (struct MMD0exp *)calloc(1, sizeof(struct MMD0exp));
    int i;

    if (getlong(data) != 0)
        mmd0exp->nextmod = (struct MMD0 *)MMD2_parse(base, getlong(data));
    mmd0exp->s_ext_entries = getword(data + 8);
    mmd0exp->s_ext_entrsz = getword(data + 10);
    if (getlong(data + 4) != 0) {
        mmd0exp->exp_smp = (struct InstrExt *)calloc(mmd0exp->s_ext_entries, sizeof(struct InstrExt));
        data2 = base + getlong(data + 4);
        /* instrext */
        for (i = 0; i < mmd0exp->s_ext_entries; i++) {
            mmd0exp->exp_smp[i].hold = getbyte(data2 + i * mmd0exp->s_ext_entrsz);
            mmd0exp->exp_smp[i].decay = getbyte(data2 + i * mmd0exp->s_ext_entrsz + 1);
            mmd0exp->exp_smp[i].suppress_midi_off = getbyte(data2 + i * mmd0exp->s_ext_entrsz + 2);
            mmd0exp->exp_smp[i].finetune = getbyte(data2 + i * mmd0exp->s_ext_entrsz + 3);
            if (mmd0exp->s_ext_entrsz > 4) {
                mmd0exp->exp_smp[i].default_pitch = getbyte(data2 + i * mmd0exp->s_ext_entrsz + 4);
                mmd0exp->exp_smp[i].instr_flags = getbyte(data2 + i * mmd0exp->s_ext_entrsz + 5);
                mmd0exp->exp_smp[i].long_midi_preset = getword(data2 + i * mmd0exp->s_ext_entrsz + 6);
                if (mmd0exp->s_ext_entrsz > 8)
                    mmd0exp->exp_smp[i].output_device = getbyte(data2 + i * mmd0exp->s_ext_entrsz + 8);
            }
        }
    }
    mmd0exp->annolen = getlong(data + 16);
    if (getlong(data + 12) != 0) {
        mmd0exp->annotxt = (unsigned char *)calloc(mmd0exp->annolen, sizeof(unsigned char));
        memcpy(mmd0exp->annotxt, base + getlong(data + 12), mmd0exp->annolen);
    }
    /* annotext */
    mmd0exp->i_ext_entries = getword(data + 24);
    mmd0exp->i_ext_entrsz = getword(data + 26);
    mmd0exp->iinfo = (struct MMDInstrInfo *)calloc(mmd0exp->i_ext_entries, sizeof(struct MMDInstrInfo));
    for (i = 0; i < mmd0exp->i_ext_entries; i++)
        memcpy(&mmd0exp->iinfo[i], base + getlong(data + 20) + i * mmd0exp->i_ext_entrsz, sizeof(struct MMDInstrInfo));
    /* instrinfo */
    mmd0exp->jumpmask = getlong(data + 28);
    mmd0exp->rgbtable = (unsigned short *)calloc(8, sizeof(unsigned short));
    memcpy(mmd0exp->rgbtable, data + 32, 16);
    /* rgbtable */
    mmd0exp->channelsplit[0] = getbyte(data + 36);
    mmd0exp->channelsplit[1] = getbyte(data + 37);
    mmd0exp->channelsplit[2] = getbyte(data + 38);
    mmd0exp->channelsplit[3] = getbyte(data + 39);

    if (getlong(data + 40) != 0) {
        /* notationinfo */
        mmd0exp->n_info = (struct NotationInfo *)calloc(1, sizeof(struct NotationInfo));
        data2 = base + getlong(data + 40);
        mmd0exp->n_info->n_of_sharps = getbyte(data2);
        mmd0exp->n_info->flags = getbyte(data2 + 1);
        mmd0exp->n_info->trksel[0] = getword(data2 + 2);
        mmd0exp->n_info->trksel[1] = getword(data2 + 4);
        mmd0exp->n_info->trksel[2] = getword(data2 + 6);
        mmd0exp->n_info->trksel[3] = getword(data2 + 8);
        mmd0exp->n_info->trksel[4] = getword(data2 + 10);
        memcpy(mmd0exp->n_info->trkshow, data2 + 12, 96);
    }
    /* songname */
    mmd0exp->songnamelen = getlong(data + 48);
    mmd0exp->songname = (char *)calloc(mmd0exp->songnamelen, sizeof(char));
    memcpy(mmd0exp->songname, base + getlong(data + 44), mmd0exp->songnamelen);

    if (getlong(data + 52) != 0) {
        struct MMDDump **dumps;

        /* dumps */
        data2 = base + getlong(data + 52);
        mmd0exp->dumps = (struct MMDDumpData *)calloc(1, sizeof(struct MMDDumpData) + getword(data2) * sizeof(struct MMDDump *));
        mmd0exp->dumps->numdumps = getword(data2);
        /* Is this really the only way to do this? I know, this is horrible but
     * this is what the lame MMD spec says */
        dumps = (struct MMDDump **)((char *)mmd0exp->dumps + sizeof(struct MMDDumpData));
        for (i = 0; i < mmd0exp->dumps->numdumps; i++) {
            data3 = base + getlong(data2 + 8 + 4 * i);
            /* dump */
            dumps[i] = (struct MMDDump *)calloc(1, sizeof(struct MMDDump));
            dumps[i]->length = getlong(data3);
            dumps[i]->ext_len = getword(data3 + 8);
            dumps[i]->data = (unsigned char *)calloc(dumps[i]->length, sizeof(unsigned char));
            /* dump data */
            memcpy(dumps[i]->data, base + getlong(data3 + 4), dumps[i]->length);

            dumps[i]->ext_len = getword(data3 + 8);
            if (dumps[i]->ext_len >= 20)
                memcpy(dumps[i]->name, data3 + 10, 20);
        }
    }

    /* mmdinfo */
    if (getlong(data + 56) != 0) {
        data2 = base + getlong(data + 56);
        mmd0exp->mmdinfo = (struct MMDInfo *)calloc(1, sizeof(struct MMDInfo) + getlong(data2 + 8));
        mmd0exp->mmdinfo->type = getword(data2 + 6);
        mmd0exp->mmdinfo->length = getlong(data2 + 8);
        memcpy(((unsigned char *)mmd0exp->mmdinfo) + sizeof(struct MMDInfo), data2 + 12, mmd0exp->mmdinfo->length);
    }

    /* mmdrexx */
    if (getlong(data + 60) != 0) {
        data2 = base + getlong(data + 60);
        mmd0exp->mmdrexx = (struct MMDARexx *)calloc(1, sizeof(struct MMDARexx));
        mmd0exp->mmdrexx->trigcmdlen = getword(data2 + 2);
        if (getlong(data2 + 4) != 0) {
            struct MMDARexxTrigCmd *trig;
            /* mmdarexxtrigcmd */
            data2 = base + getlong(data2 + 4);
            mmd0exp->mmdrexx->trigcmd = (struct MMDARexxTrigCmd *)calloc(1, sizeof(struct MMDARexxTrigCmd));
            trig = mmd0exp->mmdrexx->trigcmd;
            while (trig != NULL) {
                trig->cmdnum = getbyte(data2 + 4);
                trig->cmdtype = getword(data2 + 6);
                trig->cmd_len = getword(data2 + 16);
                trig->port_len = getword(data2 + 18);
                if (getlong(data2 + 8) != 0) {
                    trig->cmd = (char *)calloc(trig->cmd_len, sizeof(char));
                    /* command */
                    memcpy(trig->cmd, base + getlong(data2 + 8), trig->cmd_len);
                }
                if (getlong(data2 + 12) != 0) {
                    trig->port = (char *)calloc(trig->port_len, sizeof(char));
                    /* command */
                    memcpy(trig->port, base + getlong(data2 + 12), trig->port_len);
                }
                if (getlong(data2) != 0) {
                    trig->next = (struct MMDARexxTrigCmd *)calloc(1, sizeof(struct MMDARexxTrigCmd));
                    /* mmdarexxtrigcmd */
                    data2 = base + getlong(data2);
                }
                trig = trig->next;
            }
        }
    }

    /* mmdcmd3x */
    if (getlong(data + 64) != 0) {
        mmd0exp->mmdcmd3x = (struct MMDMIDICmd3x *)calloc(1, sizeof(struct MMDMIDICmd3x));
        data2 = base + getlong(data + 64);
        mmd0exp->mmdcmd3x->struct_vers = getbyte(data2);
        mmd0exp->mmdcmd3x->num_of_settings = getword(data2 + 2);
        mmd0exp->mmdcmd3x->ctrlr_types = (unsigned char *)calloc(mmd0exp->mmdcmd3x->num_of_settings, sizeof(unsigned char));
        mmd0exp->mmdcmd3x->ctrlr_numbers = (unsigned short *)calloc(mmd0exp->mmdcmd3x->num_of_settings, sizeof(unsigned short));
        data3 = base + getlong(data2 + 4);
        /* types */
        for (i = 0; i < mmd0exp->mmdcmd3x->num_of_settings; i++)
            mmd0exp->mmdcmd3x->ctrlr_types[i] = getbyte(data3 + i);
        data3 = base + getlong(data2 + 8);
        /* numbers */
        for (i = 0; i < mmd0exp->mmdcmd3x->num_of_settings; i++)
            mmd0exp->mmdcmd3x->ctrlr_numbers[i] = getword(data3 + i * 2);
    }

    return mmd0exp;
}
