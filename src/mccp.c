/***************************************************************************
 * Dystopia 2 © 2000, 2001, 2002, 2003, 2004 & 2005 by Brian Graversen     *
 *                                                                         *
 * In order to use any part of this code, you must comply with the license *
 * files found in the license folder in the doc folder of this codebase.   *
 *                                                                         *
 * Dystopia MUD is based of Godwars by Richard Woolcock.                   *
 *                                                                         *
 * Godwars is based of Merc by Michael Chastain, Michael Quan and Mitchell *
 * Tse.                                                                    *
 *                                                                         *
 * Merc is based of DIKU by Sebastian Hammer, Michael Seifert, Hans Henrik *
 * Staerfeldt. Tom Madsen and Katja Nyboe.                                 *
 *                                                                         *
 * Any additional licenses, copyrights, etc that affects this sourcefile   *
 * will be mentioned just below this copyright notice.                     *
 ***************************************************************************/

/*
 * mccp.c - support functions for mccp (the Mud Client Compression Protocol)
 *
 * see http://homepages.ihug.co.nz/~icecube/compress/ and README.Rom24-mccp
 *
 * Copyright (c) 1999, Oliver Jowett <icecube@ihug.co.nz>.
 *
 * This code may be freely distributed and used if this copyright notice is
 * retained intact.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "dystopia.h"

unsigned char compress_start  [] = { IAC, SB, TELOPT_COMPRESS, WILL, SE, '\0' };

/*
 * Begin compressing data on `desc'
 */
bool compressStart(DESCRIPTOR_DATA *desc)
{
    z_stream *s;

    if (desc->out_compress) /* already compressing */
        return TRUE;

    /* allocate and init stream, buffer */
    s = (z_stream *) calloc(1, sizeof(*s));
    desc->out_compress_buf = (unsigned char *) calloc(1, COMPRESS_BUF_SIZE * sizeof(char *));

    s->next_in   = NULL;
    s->avail_in  = 0;
    s->next_out  = desc->out_compress_buf;
    s->avail_out = COMPRESS_BUF_SIZE;
 
    s->zalloc = Z_NULL;
    s->zfree  = Z_NULL;
    s->opaque = Z_NULL;

    if (deflateInit(s, 9) != Z_OK)
    {
      free(desc->out_compress_buf);
      free(s);
      return FALSE;
    }
    
    write_to_descriptor(desc, (char *) compress_start, strlen((char *) compress_start));

    /* now we're compressing */
    desc->out_compress = s;
    return TRUE;
}

/* Cleanly shut down compression on `desc' */
bool compressEnd(DESCRIPTOR_DATA *desc)
{
    unsigned char dummy[1];

    if (!desc->out_compress)
        return TRUE;

    desc->out_compress->avail_in = 0;
    desc->out_compress->next_in = dummy;
    
    /* No terminating signature is needed - receiver will get Z_STREAM_END */

    if (deflate(desc->out_compress, Z_FINISH) != Z_STREAM_END)
        return FALSE;
    
    if (!processCompressed(desc)) /* try to send any residual data */
        return FALSE;

    deflateEnd(desc->out_compress); 
    free(desc->out_compress_buf);
    free(desc->out_compress);
    desc->out_compress = NULL;
    desc->out_compress_buf = NULL;

    return TRUE;
}


/* Try to send any pending compressed-but-not-sent data in `desc' */
bool processCompressed(DESCRIPTOR_DATA *desc)
{
    int iStart, nBlock, nWrite, len;
 
    if (!desc->out_compress)
        return TRUE;
 
    /* Try to write out some data.. */
    len = desc->out_compress->next_out - desc->out_compress_buf;
    if (len > 0) {
        /* we have some data to write */

        /* logging the amount of data send */
        increase_total_output(len);

        for (iStart = 0; iStart < len; iStart += nWrite)
        {
            nBlock = UMIN (len - iStart, 4096);
            if ((nWrite = write (desc->descriptor, desc->out_compress_buf + iStart, nBlock)) < 0)
            {
                if (errno == EAGAIN ||
                    errno == ENOSR)
                    break;
    
                return FALSE; /* write error */
            }
    
            if (nWrite <= 0)
                break;
        }
    
        if (iStart) {
            /* We wrote "iStart" bytes */
            if (iStart < len)
                memmove(desc->out_compress_buf, desc->out_compress_buf+iStart, len - iStart);

            desc->out_compress->next_out = desc->out_compress_buf + len - iStart;
        }
    }
 
    return TRUE;
}
 
/* write_to_descriptor, the compressed case */
bool writeCompressed(DESCRIPTOR_DATA *desc, char *txt, int length)
{
    z_stream *s = desc->out_compress;   

    s->next_in = (unsigned char *)txt;
    s->avail_in = length;
    
    while (s->avail_in) {
        s->avail_out = COMPRESS_BUF_SIZE - (s->next_out - desc->out_compress_buf);
                
        if (s->avail_out) {
            int status = deflate(s, Z_SYNC_FLUSH);
    
            if (status != Z_OK) {
                /* Boom */
                return FALSE;
            }
        }
         
        /* Try to write out some data.. */
        if (!processCompressed(desc))
            return FALSE;
            
        /* loop */
    }
            
    /* Done. */
    return TRUE;
}
    
void do_compres( CHAR_DATA *ch, char *argument )
{
  send_to_char("If you want to use compress, spell it out.\n\r",ch);
  return;
}

/* User-level compression toggle */
void do_compress( CHAR_DATA *ch, char *argument )
{
    if (!ch->desc) {
        send_to_char("What descriptor?!\n", ch);
        return;
    }

    if (!ch->desc->out_compress) {
        if (!compressStart(ch->desc)) {
            send_to_char("Failed.\n", ch);
            return;
        }       
 
      send_to_char("Ok, compression enabled.\n", ch);
   } 
   else {
       if (!compressEnd(ch->desc)) {
           send_to_char("Failed.\n", ch);
            return;
        }
         
        send_to_char("Ok, compression disabled.\n", ch);
    }
}

void do_showcompress(CHAR_DATA *ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *gch;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];
  int count1 = 0;
  int count2 = 0;
  int count3 = 0;
  int count4 = 0;

  if (IS_NPC(ch)) return;

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected != CON_PLAYING) continue;
    if ((gch = d->character) == NULL) continue;
    if (gch->level > 6) continue;

    sprintf(buf, "%-15s [%s] MCCP   [%s] MXP   [%s] MSP   - %3d timetick - %2d idle\n\r",
      gch->name, (d->out_compress) ? "x" : " ",
      (d->mxp) ? "x" : " ", (d->msp) ? "x" : " ",
      gch->pcdata->time_tick, gch->timer);
    send_to_char(buf, ch);

    count1++;
    if (d->out_compress) count2++;
    if (d->mxp) count3++;
    if (d->msp) count4++;
  }
  sprintf(buf,"\n\r %2d/%d uses mccp\n\r %2d/%d uses mxp\n\r %2d/%d uses msp\n\r",
    count2, count1, count3, count1, count4, count1);
  send_to_char(buf, ch);
}

