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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "dystopia.h"

/* Point to the location on the net where the sound files
 * are located. If no location are given, the players will
 * have to download the sound files before they use the
 * sound protocol.
 */
#define SOUND_URL  "http://calim.kyndig.com/sounds/"

void do_music(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch)) return;

  if (IS_SET(ch->act, PLR_MUSIC))
  {
    REMOVE_BIT(ch->act, PLR_MUSIC);
    send_to_char("Music disabled.\n\r", ch);
    send_to_char("!!MUSIC(Off)", ch);
  }
  else
  {
    SET_BIT(ch->act, PLR_MUSIC);
    send_to_char("Music enabled.\n\r", ch);
    update_music(ch);
  }
}

void do_sound(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch)) return;

  if (ch->desc == NULL) return;

  if (ch->desc->msp)
  {
    ch->desc->msp = FALSE;
    send_to_char("Sound disabled.\n\r", ch);
  }
  else
  {
    ch->desc->msp = TRUE;
    send_to_char("Sound enabled.\n\r", ch);
  }
}

void sound_to_char(char *txt, CHAR_DATA *ch)
{
  char buf[MAX_INPUT_LENGTH];

  if (!ch->desc || !ch->desc->msp) return;

  sprintf(buf, "!!SOUND(%s U=%s)", txt, SOUND_URL);
  send_to_char(buf, ch);
}

void sound_to_room(char *txt, CHAR_DATA *ch)
{
  CHAR_DATA *gch;
  ITERATOR *pIter;
  char buf[MAX_INPUT_LENGTH];

  sprintf(buf, "!!SOUND(%s U=%s)", txt, SOUND_URL);
  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!gch->desc || !gch->desc->msp) continue;

    send_to_char(buf, gch);
  }
}

void update_music(CHAR_DATA *ch)
{
  char buf[200];

  /* FIX ME - we have no real music midi files yet, so we just return */
  return;

  if (!ch->in_room || !ch->in_room->area) return;
  if (!IS_SET(ch->act, PLR_MUSIC)) return;
  send_to_char("!!MUSIC(Off)", ch);
  sprintf(buf, "!!MUSIC(%s L=-1 U=%s)",
    ch->in_room->area->music, SOUND_URL);
  send_to_char(buf, ch);
}
