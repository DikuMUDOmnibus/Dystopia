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


/***************************************************************************
 Snippet: Soundex parser.
 Author:  Richard Woolcock (aka KaVir).
 Date:    20th December 2000.
 ***************************************************************************
 This code is copyright (C) 2000 by Richard Woolcock.  It may be used and
 distributed freely, as long as you don't remove this copyright notice.
 ***************************************************************************/


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <dirent.h>
#include <math.h>

#include "utility.h"

LIST          *  poll_list        = NULL;
LIST          *  change_list      = NULL;
STACK         *  change_free      = NULL;
STACK         *  alias_free       = NULL;
STACK         *  account_free     = NULL;
STACK         *  area_affect_free = NULL;
STACK         *  snoop_free       = NULL;
STACK         *  session_free     = NULL;
LIST          *  auction_list     = NULL;
STACK         *  auction_free     = NULL;

bool gFound;
int  auction_id = 0;

#define  _NO_STRING_LIMIT_      -1
#define  COLOR_TAG              '#'
#define  ANSI_STRING            "80rRgGoylLpPcC79nu"
#define  REPLACE_STRING         "-#+"
#define  RID                    ROOM_INDEX_DATA
#define  KEY_SIZE               4

bool  examine_room         ( RID *pRoom, RID *tRoom, AREA_DATA *pArea, int steps );
void  dijkstra             ( RID *chRoom, RID *victRoom );
RID  *heap_getMinElement   ( HEAP *heap );
HEAP *init_heap            ( RID *root );
bool  is_an_a_word         ( char c );
void  newbie_hint          ( CHAR_DATA *ch, char *hint );
int  _cprintf              ( char *buf, int maxlen, char *ptr, va_list ap );

static char LetterConversion ( char chLetter );


/* modify hps CANNOT cause a player to go below 1 hps */
void modify_hps(CHAR_DATA *ch, int modifier)
{
  if (modifier < 0 &&  ch->hit <= 1)
    return;

  ch->hit += modifier;
  ch->hit = URANGE(1, ch->hit, ch->max_hit);
}

void modify_mana(CHAR_DATA *ch, int modifier)
{
  ch->mana += modifier;
  ch->mana = URANGE(0, ch->mana, ch->max_mana);
}

void modify_move(CHAR_DATA *ch, int modifier)
{
  ch->move += modifier;
  ch->move = URANGE(0, ch->move, ch->max_move);
}

char *GetSoundexKey( const char *szTxt )
{
   int iOldIndex = 0; /* Loop index for the old (szTxt) string */
   int iNewIndex = 0; /* Loop index for the new (s_a_chSoundex) string */
   static char s_a_chSoundex[2][KEY_SIZE+1]; /* Stores the new string */
   static unsigned iSoundex; /* Determines which s_a_chSoundex is used */

   iSoundex++; /* Switch to the other s_a_chSoundex array */

   s_a_chSoundex[iSoundex%2][0] = '\0'; /* Clear any previous data */

   /* Copy the first character without conversion */
   if ( ( s_a_chSoundex[iSoundex%2][iNewIndex++] = tolower(szTxt[iOldIndex++]) ) )
   {
      do /* Loop through szTxt */
      {
         char chLetter; /* Stores the soundex value of a letter */

         /* Double/triple/etc letters are treated as single letters */
         while ( szTxt[iOldIndex] != '\0' && tolower(szTxt[iOldIndex]) == tolower(szTxt[iOldIndex+1]) )
         {
            iOldIndex++;
            continue;
         }

         /* Convert the letter into its soundex value and store it */
         chLetter = LetterConversion((char)tolower(szTxt[iOldIndex]));

         /* Ignore NUL and 0 characters and only store KEY_SIZE characters */
         if ( chLetter != '\0' && chLetter != '0' && iNewIndex < KEY_SIZE )
         {
            /* Store the soundex value */
            s_a_chSoundex[iSoundex%2][iNewIndex++] = chLetter;
         }
      }
      while ( szTxt[iOldIndex++] != '\0' );

      /* If less than KEY_SIZE characters were copied, buffer with zeros */
      while ( iNewIndex < KEY_SIZE )
      {
         s_a_chSoundex[iSoundex%2][iNewIndex++] = '0';
      }

      /* Add the NUL terminator to the end of the soundex string */
      s_a_chSoundex[iSoundex%2][iNewIndex] = '\0';
   }

   /* Return the address of the soundex string */
   return ( s_a_chSoundex[iSoundex%2] );
}

int SoundexMatch( char *szFirst, char *szSecond )
{
   int iMatch = 0; /* Number of matching characters found */
   int iMax   = 0; /* Total number of characters compared */

   /* Make sure that both strings are of the correct size */
   if ( strlen( szFirst ) == KEY_SIZE && strlen( szSecond ) == KEY_SIZE )
   {
      int i; /* Loop counter */

      /* Loop through both strings */
      for ( i = 0; i < KEY_SIZE; i++ )
      {
         /* If either of the values are not NUL */
         if ( szFirst[i] != '0' || szSecond[i] != '0' )
         {
            iMax++; /* Increment the maximum */
         }

         /* If BOTH values are not NUL */
         if ( szFirst[i] != '0' && szSecond[i] != '0' )
         {
            /* Check for a match */
            if ( szFirst[i] == szSecond[i] )
            {
               iMatch++; /* A match was found */
            }
         }
      }
   }

   /* Return the percentage match */
   return ( iMatch * 100 / iMax );
}

static char LetterConversion( char chLetter )
{
   const char * kszSoundexData = "01230120022455012623010202";
   char chResult; /* Store the soundex value, or NUL */

   if ( islower(chLetter) )
   {
      /* Determine the soundex value associated with the letter */
      chResult = kszSoundexData[ (chLetter - 'a') ];
   }
   else /* it's not a lowercase letter */
   {
      /* NUL means there is no associated soundex value */
      chResult = '\0';
   }

   /* Return the soundex value, or NUL if there isn't one */
   return ( chResult );
}

char *get_time_left(int secs)
{
  int hours = 0, mins = 0;
  static char buf[MAX_STRING_LENGTH];
  char temp[MAX_INPUT_LENGTH];

  /* make sure we have a positive number */
  if (secs < 0)
    secs = 0;

  buf[0] = '\0'; 
  hours = secs / 3600;
  secs = secs % 3600;
  mins = secs / 60;
  secs = secs % 60; 
 
  if (hours)
  {
    sprintf(temp, "%d hour%s", hours, (hours != 1) ? "s" : "");
    strcat(buf, temp);
  }
  if (mins)
  {
    sprintf(temp, "%s%d minute%s", (hours != 0) ? " " : "", mins, (mins != 1) ? "s" : "");
    strcat(buf, temp);
  }
  if (secs)
  {
    sprintf(temp, "%s%d second%s", (hours || mins) ? " " : "", secs, (secs != 1) ? "s" : "");
    strcat(buf, temp);
  }

  /* make sure it contains something */
  if (!hours && !mins && !secs)
  {
    sprintf(buf, "1 second");
  }

  return buf;
}

BUFFER *box_text(char *txt, char *title)
{
  static BUFFER *buf;
  char temp[MAX_INPUT_LENGTH];
  int maxlen = 0;
  int titlelen = collen(title);
  int i, j = 0;

  /* We only allocate memory for the buffer the very first
   * time we call this function. All other times we just clear
   * the old buffer and reuse the memory.
   */
  if (!buf)
    buf = buffer_new(MAX_STRING_LENGTH);
  else
    buffer_clear(buf);

  /* find the maximum length of a single line in txt */
  for (i = 0; txt[i] != '\0'; i++)
  {
    switch(txt[i])
    {
      case '\n':
        temp[j] = '\0';
        if (collen(temp) > maxlen)
          maxlen = collen(temp);
        j = 0;
        break;
      case '\r':
        break;
      default:
        temp[j++] = txt[i];
        break;
    }
  }

  /* roundup of maxlen to a size 6 */
  for (i = 6; i <= 72; i += 6)
  {
    if (maxlen <= i)
    {
      maxlen = i;
      break;
    }
  }

  /* calculate space needed for title */
  for (i = 6; i <= 72; i += 6)
  {
    if (titlelen <= i)
    {
      titlelen = i;
      break;
    }
  }

  /* make sure we have room for brackets */
  if (titlelen < collen(title) + 4)
    titlelen += 6;

  /* make sure that txt/title is of a proper size */
  if (maxlen > 72)
  {
    bug("box_text: text to wide.", 0);
    bprintf(buf, "You have encountered a bug, please report this.\n\r");
    return buf;
  }
  else if (maxlen < titlelen + 14)
  {
    bug("box_text: title to wide.", 0);
    bprintf(buf, "You have encountered a bug, please report this.\n\r");
    return buf; 
  }

  /* create the top border */
  {
    bool noteven = ((maxlen - titlelen) % 12);
    int blocks = (maxlen - titlelen) / 12;
    int spaces = titlelen - collen(title) - 4;
    j = 0;

    sprintf(temp, "  #o,=");
    for (i = 0; i < blocks; i++)
      strcat(temp, "~-:|:-");
    strcat(temp, "#0[#C ");
    for (i = 0; i < spaces / 2; i++)
      strcat(temp, " ");
    strcat(temp, title);
    for (i = spaces / 2; i < spaces; i++)
      strcat(temp, " ");
    strcat(temp, " #0]#o");
    for (i = 0; i < blocks; i++)
      strcat(temp, "-:|:-~");
    if (noteven)
      strcat(temp, "-:|:-~");
    strcat(temp, "=.#n");

    bprintf(buf, "%s\n\r", temp);
  }

  /* attach all the middle text */
  j = 0;
  bprintf(buf, " #o(  ");
  for (i = 0; i < maxlen; i++)
    bprintf(buf, " ");
  bprintf(buf, "  )#n\n\r");
  for (i = 0; txt[i] != '\0'; i++)
  {
    switch(txt[i])
    {
      case '\n':
        temp[j] = '\0';
        j = 0;
        titlelen = maxlen - collen(temp);
        bprintf(buf, " #o(#n  %s", temp);
        while (titlelen > 0)
        {
          bprintf(buf, " ");
          titlelen--;
        }
        bprintf(buf, "  #o)#n\n\r");
        break;
      case '\r':
        break;
      default:
        temp[j++] = txt[i];
        break;
    }
  }
  bprintf(buf, " #o(  ");
  for (i = 0; i < maxlen; i++)
    bprintf(buf, " ");
  bprintf(buf, "  )#n\n\r");

  /* create the bottom border */
  {
    bool noteven = (maxlen % 12);
    int blocks = maxlen / 12;

    sprintf(temp, "  #o`=");
    for (i = 0; i < blocks; i++)
      strcat(temp, "~-:|:-");
    for (i = 0; i < blocks; i++)
      strcat(temp, "-:|:-~");
    if (noteven)
      strcat(temp, "-:|:-~");
    strcat(temp, "=`#n");
   
    bprintf(buf, "%s\n\r", temp);
  }

  return buf;
}

BUFFER *identify_obj(OBJ_DATA *obj)
{
  BUFFER *buf = buffer_new(MAX_STRING_LENGTH);
  AFFECT_DATA *paf;
  ITERATOR *pIter;
  char *seperator = "#o-:|:-~-:|:-~-:|:-~-:|:-~-:|:--:|:-~-:|:-~-:|:-~-:|:-~-:|:-~-:|:-~-:|:-#n";
  char *align = "bugged";
  char temp[MAX_INPUT_LENGTH];
  bool affect_line = FALSE;
  int itemtype, i = 0;

  if (obj == NULL)
  {
    bprintf(buf, "obj is NULL.\n\r");
    return buf;
  }

  bprintf(buf, "%-72.72s\n\n\r", obj->short_descr);

  if (IS_OBJ_STAT(obj, ITEM_RARE) || IS_OBJ_STAT(obj, ITEM_SENTIENT))
  {
    bprintf(buf, "Value : #C%3d#n   Weight : #C%3d#n   Type : #C%s#n   Rank : %s\n\r",
      obj->cost, obj->weight, item_type_name(obj), get_rare_rank_name(obj->cost));
  }
  else
  {
    bprintf(buf, "Value : #C%3d#n   Weight : #C%3d#n   Type : #C%s#n\n\r",
      obj->cost, obj->weight, item_type_name(obj));
  }

  bprintf(buf, "Owner : #C%s#n  Keywords : #C%-40.40s#n\n\r",
    (obj->questowner[0] != '\0') ? obj->questowner : "noone", obj->name);

  if (IS_OBJ_STAT(obj, ITEM_SENTIENT))
  {
    char *sentient = "bugged";

    if (obj->sentient_points >= 165)
      sentient = "highly intelligent";
    else if (obj->sentient_points >= 125)
      sentient = "intelligent";
    else if (obj->sentient_points >= 90)
      sentient = "slightly intelligent";
    else if (obj->sentient_points >= 70)
      sentient = "highly empathic";
    else if (obj->sentient_points >= 45)
      sentient = "empathic";
    else
      sentient = "slightly empathic";

    bprintf(buf, "This item is sentient, rated #C%s#n\n\r", sentient);
  }

  bprintf(buf, "\n\r %s\n\n\r", seperator);

  if (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))
    align = "Anti-Good";
  else if (IS_OBJ_STAT(obj, ITEM_ANTI_EVIL))
    align = "Anti-Evil";
  else if (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL))
    align = "Anti-Neutral";
  else
    align = "No Align";

  bprintf(buf, " [%c] %-18.18s  [%c] %-18.18s  [%c] %-18.18s\n\r",
    (str_cmp(align, "No Align")) ? 'o' : ' ', align,
    (IS_OBJ_STAT(obj, ITEM_LOYAL)) ? 'o' : ' ', "Loyal",
    (IS_OBJ_STAT(obj, ITEM_UNBREAKABLE)) ? 'o' : ' ', "Unbreakable");

  bprintf(buf, " [%c] %-18.18s  [%c] %-18.18s  [%c] %-18.18s\n\r",
    (IS_OBJ_STAT(obj, ITEM_NOREPAIR)) ? 'o' : ' ', "Cannot Repair",
    (IS_SET(obj->quest, QUEST_GIANTSTONE)) ? 'o' : ' ', "Stoneshaped",
    (IS_SET(obj->quest, QUEST_SPELLPROOF)) ? 'o' : ' ', "Spell Proofed");

  if (obj->item_type == ITEM_WEAPON)
  {
    bprintf(buf, " [%c] %-18.18s  [%c] %-18.18s  [%c] %-18.18s\n\r",
      (IS_SET(obj->quest, QUEST_ENCHANTED)) ? 'o' : ' ', "Enchanted",
      (IS_OBJ_STAT(obj, ITEM_FAE_BLAST)) ? 'o' : ' ', "Fae Blast/Tune",
      (object_is_affected(obj, OAFF_FROSTBITE)) ? 'o' : ' ', "Frostblasted");
  }

  switch(obj->item_type)
  {
    default:
      break;
    case ITEM_WEAPON:
      bprintf(buf, "\n\r %s\n\n\r", seperator);

      if (obj->value[0] >= 1000)
        itemtype = obj->value[0] - ((obj->value[0] / 1000) * 1000);
      else
        itemtype = obj->value[0];

      switch(obj->value[3])
      {
        default:
          align = "bugged";
          break;
        case 0:
          align = "unarmed";
          break;
        case 1:
          align = "slicing";
          break;
        case 2:
          align = "stabbing";
          break;
        case 3:
          align = "slashing";
          break;
        case 4:
          align = "whipping";
          break;
        case 5:
          align = "clawing";
          break;
        case 6:
          align = "blasting";
          break;
        case 7:
          align = "pounding";
          break;
        case 8:
          align = "crushing";
          break;
        case 9:
          align = "grepping";
          break;
        case 10:
          align = "biting";
          break;
        case 11:
          align = "piercing";
          break;
        case 12:
          align = "sucking";
          break;
      }

      bprintf(buf, "This weapon deals from %d to %d (average %d) #y%s#n damage.\n\r",
        obj->value[1], obj->value[1] * obj->value[2], (obj->value[1] * obj->value[2]) / 2, align);

      if (itemtype > 0 && itemtype < MAX_SKILL)
        bprintf(buf, "\n\rThis weapon can cast the '%s' spell.\n\r", skill_table[itemtype].name);

      if (obj->value[0] >= 1000)
      {
        if (itemtype <= 0 || itemtype >= MAX_SKILL)
          bprintf(buf, "\n\r");

        itemtype = obj->value[0] / 1000;

        if (itemtype == OBJECT_BLIND)
          bprintf(buf, "This weapon radiates an aura of darkness.\n\r");
        else if (itemtype == OBJECT_DETECTINVIS)
          bprintf(buf, "This weapon allows the wielder to see invisible things.\n\r");
        else if (itemtype == OBJECT_FLYING)
          bprintf(buf, "This weapon grants the power of flight.\n\r");
        else if (itemtype == OBJECT_INFRARED)
          bprintf(buf, "This weapon allows the wielder to see in the dark.\n\r");
        else if (itemtype == OBJECT_INVISIBLE)
          bprintf(buf, "This weapon renders the wielder invisible to the human eye.\n\r");
        else if (itemtype == OBJECT_PASSDOOR)
          bprintf(buf, "This weapon allows the wielder to walk through solid doors.\n\r");
        else if (itemtype == OBJECT_PROTECT)
          bprintf(buf, "This holy weapon protects the wielder from evil.\n\r");
        else if (itemtype == OBJECT_PROTECTGOOD)
          bprintf(buf, "This unholy weapon protects the wielder from good.\n\r");
        else if (itemtype == OBJECT_SANCTUARY)
          bprintf(buf, "This ancient weapon protects the wielder in combat.\n\r");
        else if (itemtype == OBJECT_DETECTHIDDEN)
          bprintf(buf, "This ancient weapon allows the wielder to detect things hidden.\n\r");
        else if (itemtype == OBJECT_SNEAK)
          bprintf(buf, "This crafty weapon allows the wielder to walk in complete silence.\n\r");
        else if (itemtype == OBJECT_CHAOSSHIELD)
          bprintf(buf, "This ancient weapon surrounds its wielder with a shield of chaos.\n\r");
        else if (itemtype == OBJECT_REGENERATE)
          bprintf(buf, "This ancient weapon regenerates the wounds of its wielder.\n\r");
        else if (itemtype == OBJECT_SPEED)
          bprintf(buf, "This ancient weapon allows its wielder to move at supernatural speed.\n\r");
        else if (itemtype == OBJECT_VORPAL)
          bprintf(buf, "This razor sharp weapon can slice through armour without difficulty.\n\r");
        else if (itemtype == OBJECT_RESISTANCE)
          bprintf(buf, "This ancient weapon grants superior protection to its wielder.\n\r");
        else if (itemtype == OBJECT_VISION)
          bprintf(buf, "This ancient weapon grants its wielder supernatural vision.\n\r");
        else if (itemtype == OBJECT_STALKER)
          bprintf(buf, "This ancient weapon makes its wielder fleet-footed.\n\r");
        else if (itemtype == OBJECT_VANISH)
          bprintf(buf, "This ancient weapon conceals its wielder from sight.\n\r");
        else
          bprintf(buf, "This item is bugged...please report it.\n\r");
      }
      break;
    case ITEM_ARMOR:
      bprintf(buf, "\n\r %s\n\n\r", seperator);
      bprintf(buf, "This object has an armour class rating of #C%d#n\n\r", obj->value[0]);

      if (obj->value[3] > 0)
      {
        if (obj->value[3] == OBJECT_BLIND)
          bprintf(buf, "This object radiates an aura of darkness.\n\r");
        else if (obj->value[3] == OBJECT_DETECTINVIS)
          bprintf(buf, "This item allows the wearer to see invisible things.\n\r");
        else if (obj->value[3] == OBJECT_FLYING)
          bprintf(buf, "This object grants the power of flight.\n\r");
        else if (obj->value[3] == OBJECT_INFRARED)
          bprintf(buf, "This item allows the wearer to see in the dark.\n\r");
        else if (obj->value[3] == OBJECT_INVISIBLE)
          bprintf(buf, "This object renders the wearer invisible to the human eye.\n\r");
        else if (obj->value[3] == OBJECT_PASSDOOR)
          bprintf(buf, "This object allows the wearer to walk through solid doors.\n\r");
        else if (obj->value[3] == OBJECT_PROTECT)
          bprintf(buf, "This holy relic protects the wearer from evil.\n\r");
        else if (obj->value[3] == OBJECT_PROTECTGOOD)
          bprintf(buf, "This unholy relic protects the wearer from good.\n\r");
        else if (obj->value[3] == OBJECT_SANCTUARY)
          bprintf(buf, "This ancient relic protects the wearer in combat.\n\r");
        else if (obj->value[3] == OBJECT_DETECTHIDDEN)
          bprintf(buf, "This ancient relic allows the wearer to detect hidden things.\n\r");
        else if (obj->value[3] == OBJECT_SNEAK) 
          bprintf(buf, "This crafty item allows the wearer to walk in complete silence.\n\r");
        else if (obj->value[3] == OBJECT_CHAOSSHIELD)
          bprintf(buf, "This ancient item surrounds its wearer with a shield of chaos.\n\r");
        else if (obj->value[3] == OBJECT_REGENERATE)
          bprintf(buf, "This ancient item regenerates the wounds of its wearer.\n\r");   
        else if (obj->value[3] == OBJECT_SPEED)   
          bprintf(buf, "This ancient item allows its wearer to move at supernatural speed.\n\r");
        else if (obj->value[3] == OBJECT_VORPAL)
          bprintf(buf, "This item is bugged - please report this.\n\r");
        else if (obj->value[3] == OBJECT_RESISTANCE)
          bprintf(buf, "This ancient item grants superior protection to its wearer.\n\r");
        else if (obj->value[3] == OBJECT_VISION)
          bprintf(buf, "This ancient item grants its wearer supernatural vision.\n\r");
        else if (obj->value[3] == OBJECT_STALKER)
          bprintf(buf, "This ancient item makes its wearer fleet-footed.\n\r");
        else if (obj->value[3] == OBJECT_VANISH)
          bprintf(buf, "This ancient item conceals its wearer from sight.\n\r");
        else
          bprintf(buf, "This item is bugged...please report it.\n\r");
      }
      break;
    case ITEM_FAETOKEN:
      bprintf(buf, "\n\r %s\n\n\r", seperator);
      bprintf(buf, "This token can be energized for a spell of: '%s'.\n\r", skill_table[obj->value[0]].name);
      break;
    case ITEM_PILL:
    case ITEM_SCROLL:
    case ITEM_POTION:
      bprintf(buf, "\n\r %s\n\n\r", seperator);
      bprintf(buf, "This can be used to cast level #C%d#n spells of:\n\n\r", obj->value[0]);

      if (obj->value[1] >= 0 && obj->value[1] < MAX_SKILL)
        bprintf(buf, "  [o] '%s'.\n\r", skill_table[obj->value[1]].name);
      if (obj->value[2] >= 0 && obj->value[2] < MAX_SKILL)
        bprintf(buf, "  [o] '%s'.\n\r", skill_table[obj->value[2]].name);
      if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
        bprintf(buf, "  [o] '%s'.\n\r", skill_table[obj->value[3]].name);
      break;
    case ITEM_QUEST:
      bprintf(buf, "\n\r %s\n\n\r", seperator);
      bprintf(buf, "This object can be deposited for #y%d#n goldcrowns.\n\r", obj->value[0]);
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      bprintf(buf, "\n\r %s\n\n\r", seperator);
      bprintf(buf, "This object has %d(%d) charges of level %d ", obj->value[1], obj->value[2], obj->value[0]);
        
      if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
        bprintf(buf, "'%s'.\n\r", skill_table[obj->value[3]].name);
      else
        bprintf(buf, "'some unknown spell'.\n\r");
      break;
  }

  pIter = AllocIterator(obj->pIndexData->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->location != APPLY_NONE && paf->modifier != 0)
    {
      if (affect_line == FALSE)
      {
        affect_line = TRUE;
        bprintf(buf, "\n\r %s\n\n\r", seperator);
      }

      sprintf(temp, "Affects %s by %d", affect_loc_name(paf->location), paf->modifier);
      bprintf(buf, "%-34.34s%s", temp, (i++ % 2) ? "\n\r" : "  ");
    }
  }

  pIter = AllocIterator(obj->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->location != APPLY_NONE && paf->modifier != 0)
    {
      if (affect_line == FALSE)
      {
        affect_line = TRUE;
        bprintf(buf, "\n\r %s\n\n\r", seperator);
      }

      sprintf(temp, "Affects %s by %d", affect_loc_name(paf->location), paf->modifier);
      bprintf(buf, "%-34.34s%s", temp, (i++ % 2) ? "\n\r" : "  ");
    }
  }

  if (affect_line && i % 2)
    bprintf(buf, "\n\r");

  return buf;
}

void band_description(CHAR_DATA *ch)
{
  char *tname;
  char name[MAX_INPUT_LENGTH];
  char short_descr[MAX_INPUT_LENGTH];
  char long_descr[MAX_INPUT_LENGTH];
  char keyword[MAX_INPUT_LENGTH];
  int i;

  if (ch->gcount <= 1)
  {
    free_string(ch->name);
    free_string(ch->short_descr);
    free_string(ch->long_descr);
    ch->name = str_dup(ch->pIndexData->player_name);
    ch->short_descr = str_dup(ch->pIndexData->short_descr);
    ch->long_descr = str_dup(ch->pIndexData->long_descr);
    REMOVE_BIT(ch->newbits, NEW_BANDED);
    return;
  }

  /* first time we do this or not */
  if (!IS_SET(ch->newbits, NEW_BANDED))
  {
    tname = one_argument(ch->short_descr, name);
    one_argument(tname, name);

    /* I hope this works well :) */
    i = strlen(name);
    switch(name[i - 1])
    {
      default:
        name[i] = 's';
        name[i+1] = '\0';
        break;
      case 's':
      case 'S':
      case 'x':
      case 'X':
        name[i] = 'e';
        name[i+1] = 's';
        name[i+2] = '\0';
        break;
    }

    SET_BIT(ch->newbits, NEW_BANDED);
  }
  else
  {
    tname = one_argument(ch->short_descr, name);
    tname = one_argument(tname, name);
    tname = one_argument(tname, name);
    one_argument(tname, name);
  }

  if (ch->gcount >= 100)
    tname = "legion";
  if (ch->gcount >= 50)
    tname = "swarm";
  else if (ch->gcount >= 25)
    tname = "throng";
  else if (ch->gcount >= 10)
    tname = "horde";
  else if (ch->gcount >= 5)
    tname = "pack";
  else if (ch->gcount >= 3)
    tname = "group";
  else
    tname = "team";

  sprintf(short_descr, "A %s of %s", tname, name);
  sprintf(long_descr, "A %s of %s are standing here.\n\r", tname, name);
  sprintf(keyword, "%s %s", ch->pIndexData->player_name, tname);

  free_string(ch->short_descr);
  free_string(ch->long_descr);
  free_string(ch->name);
  ch->short_descr = str_dup(short_descr);
  ch->long_descr = str_dup(long_descr);
  ch->name = str_dup(keyword);
}

void show_class_help(DESCRIPTOR_DATA *d, int class)
{
  write_to_buffer(d, "\n\r", 0);

  switch(class)
  {
    default:
      write_to_buffer(d, "There is no description of this class.\n\n\r", 0);
      break;
    case CLASS_GIANT:
      write_to_buffer(d,
      "#9#uGiant#n\n\r"
      "The giant is the strongest of the races, using pure brawn and brute force to\n\r"
      "kill their enemies. Being low on brainpower gives the giant access to few\n\r"
      "commands, but those that it has are usually geared towards dealing massive\n\r"
      "damage. Playing a giant requires much stamina and hard work, though it is\n\r"
      "a reasonable easy class to learn to play.\n\r"
      "\n\r", 0);
      break;
    case CLASS_SHADOW:
      write_to_buffer(d,
      "#9#uShadowlord#n\n\r"
      "The shadowlord is an assassin and a rogue. It skulks in the shadows, attacks\n\r"
      "when least expected and vanishes before the victim can retaliate. The shade\n\r"
      "relies on high skills with all weapon styles and a wide range of combat\n\r"
      "techniques and special attacks. The shadowlord may require some fast typing\n\r"
      "and timing to play correct.\n\r"
      "\n\r", 0);
      break;
    case CLASS_FAE:
      write_to_buffer(d,
      "#9#uFae#n\n\r"
      "The fae is a force of pure energy, which is able to manipulate the physical\n\r"
      "world through concentration and timing. Unable to wield most equipment, the fae\n\r"
      "relies on blasting weapons and energy attacks, and the ability to channel and\n\r"
      "store several deadly charges of energy in its own body. Before picking this\n\r"
      "class you should be aware that it, unlike the other classes, takes a few hours\n\r", 0);
      write_to_buffer(d,
      "of play before this class can train avatar. Read HELP REQUIREMENT when you enter\n\r"
      "the game - it lists all the details.\n\r"
      "\n\r", 0);
      break;
    case CLASS_WARLOCK:
      write_to_buffer(d,
      "#9#uWarlock#n\n\r"
      "The warlock is a spellcaster and usually weak on close combat. Through magical\n\r"
      "affects and rites the warlock is able to muster many arcane defences, allowing\n\r"
      "it some amount of protection when engaged in combat. A wide range of spells and\n\r"
      "skills allows the warlock to adept to any situation.\n\r"
      "\n\r", 0);
      break;
  }
}

char *dot_it_up(int value)
{
  char temp[MAX_INPUT_LENGTH];
  static char buf[MAX_INPUT_LENGTH];
  int i, j, size, dots;

  sprintf(temp, "%d", value);
  size = strlen(temp);
  dots = (size - 1) / 3;

  for (i = 0, j = -1 * dots; i < size; i++)
  {
    buf[size - 1 - j++] = temp[size - 1 - i];

    if ((i + 1) % 3 == 0 && (i + 1) < size)
      buf[size - 1 - j++] = '.';
  }
  buf[size + dots] = '\0';

  return buf;
}

int reduce_cost(CHAR_DATA *ch, CHAR_DATA *victim, int cost, int type)
{
  if (victim != NULL && !IS_NPC(victim))
    return cost;

  if (type == eMana)
    cost *= (100 - UMAX(0, get_curr_int(ch) - 25));
  else if (type == eMove)
    cost *= (100 - UMAX(0, get_curr_wis(ch) - 25));
  else if (type == eHit)
    cost *= (100 - UMAX(0, get_curr_con(ch) - 25));
  else
    cost *= 100;

  return (cost / 100);
}

char *smudge_time(time_t pTime)
{
  static char buf[MAX_STRING_LENGTH];
  char *strtime = ctime(&pTime);
  int hours;

  strtime[10] = '\0';
  strtime[13] = '\0';
  hours = atoi(&strtime[11]);

  if (hours <= 5)
    sprintf(buf, "the early morning of %s", &strtime[4]);
  else if (hours <= 10)
    sprintf(buf, "the morning of %s", &strtime[4]);
  else if (hours <= 14)
    sprintf(buf, "the noon of %s", &strtime[4]);
  else if (hours <= 17)
    sprintf(buf, "the afternoon of %s", &strtime[4]);
  else
    sprintf(buf, "the evening of %s", &strtime[4]);

  return buf;
}

bool can_use_command(CHAR_DATA *ch, int cmd)
{
  if (cmd_table[cmd].race == 0)
    return TRUE;

  if (IS_NPC(ch) || ch->class != cmd_table[cmd].race)
    return FALSE;

  if (cmd_table[cmd].classtype == 0)
    return TRUE;

  if (cmd_table[cmd].classtype == CP_LEVEL && ch->pcdata->powers[cmd_table[cmd].powertype] >= cmd_table[cmd].powerlevel)
    return TRUE;

  if (cmd_table[cmd].classtype == CP_BIT && IS_SET(ch->pcdata->powers[cmd_table[cmd].powertype], cmd_table[cmd].powerlevel))
    return TRUE;

  if (cmd_table[cmd].classtype == CP_MASTERY && IS_SET(ch->newbits, NEW_MASTERY))
    return TRUE;

  return FALSE;
}

bool can_use_skill(CHAR_DATA *ch, int sn)
{
  if (skill_table[sn].race == 0)
    return TRUE;

  if (IS_NPC(ch) || ch->class != skill_table[sn].race)
    return FALSE;

  if (skill_table[sn].classtype == 0)
    return TRUE;

  if (skill_table[sn].classtype == CP_LEVEL && ch->pcdata->powers[skill_table[sn].powertype] >= skill_table[sn].powerlevel)
    return TRUE;

  if (skill_table[sn].classtype == CP_BIT && IS_SET(ch->pcdata->powers[skill_table[sn].powertype], skill_table[sn].powerlevel))
    return TRUE;

  if (skill_table[sn].classtype == CP_MASTERY && IS_SET(ch->newbits, NEW_MASTERY))
    return TRUE;

  return FALSE;
}

AUCTION_DATA *alloc_auction()
{
  AUCTION_DATA *auction;

  if ((auction = (AUCTION_DATA *) PopStack(auction_free)) == NULL)
  {
    auction = calloc(1, sizeof(*auction));
  }

  auction->obj = NULL;
  auction->seller_name = str_dup("");
  auction->seller_account = str_dup("");
  auction->bidder_name = str_dup("");
  auction->bidder_account = str_dup("");
  auction->expire = current_time + 48 * 60 * 60;
  auction->bid = 0;
  auction->bidout = 0;
  auction->id = ++auction_id;

  return auction;
}

void free_auction(AUCTION_DATA *auction)
{
  DetachFromList(auction, auction_list);

  free_string(auction->seller_name);
  free_string(auction->seller_account);
  free_string(auction->bidder_name);
  free_string(auction->bidder_account);

  if (auction->obj)
  {
    bug("free_auction: object will be lost forever.", 0);
    free_obj(auction->obj);
  }

  PushStack(auction, auction_free);
}

bool object_is_affected(OBJ_DATA *obj, int bit)
{
  AFFECT_DATA *paf;
  ITERATOR *pIter;

  pIter = AllocIterator(obj->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->type == bit)
      return TRUE;
  }

  return FALSE;
}

int next_rank_in(CHAR_DATA *ch)
{
  int currentMight = getMight(ch);
  int currentRank = getRank(ch, 1);
  int nextRank = getRank(ch, 2);

  /* return -1 if there are no more ranks */
  if (currentMight >= RANK_ALMIGHTY)
    return -1;

  return (100 * (currentMight - currentRank) / (nextRank - currentRank));
}

bool account_exists(char *name)
{
  FILE *fp;
  char pfile[MAX_STRING_LENGTH];

  /* try to open the account */
  sprintf(pfile, "../accounts/%s/account.dat", capitalize(name));
  if ((fp = fopen(pfile, "r")) == NULL)
    return FALSE;

  fclose(fp);

  return TRUE;
}

SNOOP_DATA *alloc_snoop()
{
  SNOOP_DATA *snoop;

  if ((snoop = (SNOOP_DATA *) PopStack(snoop_free)) == NULL)
    snoop = malloc(sizeof(*snoop));

  snoop->snooper = NULL;
  snoop->flags = 0;

  return snoop;
}

void free_snoop(DESCRIPTOR_DATA *d, SNOOP_DATA *snoop)
{
  DetachFromList(snoop, d->snoops);
  PushStack(snoop, snoop_free);
}

char *strip_returns(char *txt)
{
  static char buf[MAX_STRING_LENGTH];
  int i = 0;

  while (*txt != '\0')
  {
    if (*txt != '\r')
      buf[i++] = *txt;

    txt++;
  }
  buf[i] = '\0';

  return buf;
}

void aggress(CHAR_DATA *ch, CHAR_DATA *victim)
{
  if (!IS_NPC(ch) && !IS_NPC(victim) && (ch->level < 3 || victim->level < 3))
    return;

  if (ch == victim)
    return;

  update_feed(ch, victim);

  if (victim->fighting == NULL)
    set_fighting(victim, ch);
  if (ch->fighting == NULL)
    set_fighting(ch, victim);

  if (!IS_NPC(victim) && ch->fight_timer <= 0)
    ch->fight_timer = 3;
}

void stop_spectating(CHAR_DATA *ch)
{
  ITERATOR *pIter;

  REMOVE_BIT(ch->pcdata->tempflag, TEMP_SPECTATE);

  if (ch->desc)
  {
    SNOOP_DATA *snoop;

    pIter = AllocIterator(ch->desc->snoops);
    while ((snoop = (SNOOP_DATA *) NextInList(pIter)) != NULL)
    {
      if (snoop->snooper->character)
        act("$n no longer allows spectating.", ch, NULL, snoop->snooper->character, TO_VICT);

      free_snoop(ch->desc, snoop);
    }
  }
}

void update_auctions()
{
  AUCTION_DATA *auction;
  ACCOUNT_DATA *account;
  ITERATOR *pIter, *pIter2;
  CHAR_DATA *ch;
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  char *player;
  bool online = FALSE;
  bool sold = TRUE;

  pIter = AllocIterator(auction_list);
  while ((auction = (AUCTION_DATA *) NextInList(pIter)) != NULL)
  {
    if (auction->expire <= current_time)
    {
      if ((obj = auction->obj) == NULL)
      {
	bug("update_auction: auction without object removed.", 0);
	free_auction(auction);
	continue;
      }

      if (auction->bidder_name[0] != '\0')
      {
	/* pay the seller 95% of the selling price */
	if ((account = load_account(auction->seller_account)) != NULL)
	{
	  account->goldcrowns += 19 * auction->bid / 20;
	  save_account(account);
	  close_account(account);
	}

        /* give 5% to the auction fond */
        muddata.questpool += auction->bid / 20;

	/* who should have the item? */
	player = auction->bidder_name;

	/* make a note to seller and buyer */
	sprintf(buf, "Greetings,\n\n\r%s has sold an item to %s for %d gold.\n\r"
                     "The item is : %s#n\n\n\r"
                     "5%% of the gold was donated to the auction fond.\n\n\r"
                     "The Auction Code.\n\r",
          auction->seller_name, auction->bidder_name, auction->bid, obj->short_descr);
	sprintf(buf2, "%s %s", auction->seller_name, auction->bidder_name);
	make_note("Personal", "Auction Code", buf2, "Auction completed", 2, buf, 0);
      }
      else
      {
	sprintf(buf, "Greetings %s,\n\n\r"
                     "The item : %s\n\r"
                     "was not sold, and has been returned to your inventory.\n\n\r"
                     "The Auction Code.\n\r",
          auction->seller_name, obj->short_descr);
	make_note("Personal", "Auction Code", auction->seller_name, "Auction failed", 1, buf, 0);

        sold = FALSE;
	player = auction->seller_name;
      }

      /* put the object into the game */
      AttachToList(obj, object_list);
      auction->obj = NULL;

      /* put the item in player's inventory */
      pIter2 = AllocIterator(char_list);
      while ((ch = (CHAR_DATA *) NextInList(pIter2)) != NULL)
      {
	if (IS_NPC(ch))
	  continue;

	if (!str_cmp(player, ch->name))
	{
	  online = TRUE;

	  obj_to_char(obj, ch);
	  act("$p appears in $n's hands in a blast of lightning.", ch, obj, NULL, TO_ROOM);
	  act("$p appears in your hands in a blast of lightning.", ch, obj, NULL, TO_CHAR);

          if (sold)
            send_to_char("#RYou have won the auction!!#n\n\r", ch);
          else
            send_to_char("#RYour item has been returned.#n\n\r", ch);
	  break;
	}
      }

      /* player not online, load and save */
      if (!online)
      {
	int status;
        char silent_string[] = { 27, 27, '\0' };

        /* load the characters whois info */
        if ((ch = load_char_whois(player, &status)) == NULL)
        {
          if (status == 0)
            bug("update_auction: buyer does not exist.", 0);
          else
            bug("update_auction: something unexpected happened.", 0);
          extract_obj(obj);
        }
        else
        {
          if ((account = load_account(ch->pcdata->account)) == NULL)
          {
            bug("update_auction: the players account does not exists.", 0);
	    extract_obj(obj);
            free_char(ch);
          }
          else
          {
            DESCRIPTOR_DATA *d;

            if ((d = (DESCRIPTOR_DATA *) PopStack(descriptor_free)) == NULL)
            {
              d = calloc(1, sizeof(*d));
            }

            d->account = account;
            if (load_char_obj(d, capitalize(player)))
            {
              /* connect to char_list */
              AttachToList(d->character, char_list);

	      /* put the player into the game */
              if (d->character->in_room != NULL)
                char_to_room(d->character, d->character->in_room, TRUE);
              else
                char_to_room(d->character, get_room_index(ROOM_VNUM_CITYCENTER), TRUE);

              /* give the item to the character */
              obj_to_char(obj, d->character);

              /* clear up after us */
              save_char_obj(d->character);
              close_account(account);
              free_char(ch);
              PushStack(d, descriptor_free);

	      /* quit the loaded character */
              ch = d->character;
              ch->desc = NULL;
              do_quit(ch, silent_string);
            }
            else
            {
              bug("update_auction: players pfile is missing.", 0);
              free_char(ch);
              close_account(account);
              PushStack(d, descriptor_free);
	      extract_obj(obj);
            }
          }
        }
      }

      free_auction(auction);
    }
  }

  save_auctions();
}

bool ccenter_not_stock()
{
  if (muddata.ccenter[CCENTER_MIN_EXP] != CCENTER_MIN_EXP_DEFAULT)
    return TRUE;

  if (muddata.ccenter[CCENTER_MAX_EXP] != CCENTER_MAX_EXP_DEFAULT)
    return TRUE;

  if (muddata.ccenter[CCENTER_EXP_LEVEL] != CCENTER_EXP_LEVEL_DEFAULT)
    return TRUE;

  if (muddata.ccenter[CCENTER_QPS_LEVEL] != CCENTER_QPS_LEVEL_DEFAULT)
    return TRUE;

  return FALSE;
}

void mob_outstance(CHAR_DATA *ch)
{
  CHAR_DATA *victim;
  int stance[3];
  int newstance, i;

  /* need someone to outstance */
  if ((victim = ch->fighting) == NULL)
    return;

  switch(victim->stance[0])
  {
    default:
      return;
      break;
    case STANCE_BULL:
      stance[0] = STANCE_CRAB;
      stance[1] = STANCE_CRANE;
      stance[2] = STANCE_DRAGON;
      break;
    case STANCE_VIPER:
      stance[0] = STANCE_VIPER;
      stance[1] = STANCE_BULL;
      stance[2] = STANCE_MANTIS;
      break;
    case STANCE_MONGOOSE:
      stance[0] = STANCE_VIPER;
      stance[1] = STANCE_BULL;
      stance[2] = STANCE_SWALLOW;
      break;
    case STANCE_CRAB:
      stance[0] = STANCE_BULL;
      stance[1] = STANCE_CRANE;
      stance[2] = STANCE_MONKEY;
      break;
    case STANCE_CRANE:
      stance[0] = STANCE_CRANE;
      stance[1] = STANCE_MONGOOSE;
      stance[2] = STANCE_TIGER;
      break;
    case STANCE_TIGER:
      stance[0] = STANCE_MANTIS;
      stance[1] = STANCE_TIGER;
      stance[2] = STANCE_BULL;
      break;
    case STANCE_MANTIS:
      stance[0] = STANCE_TIGER;
      stance[1] = STANCE_MANTIS;
      stance[2] = STANCE_CRAB;
      break;
    case STANCE_MONKEY:
      stance[0] = STANCE_MONKEY;
      stance[1] = STANCE_DRAGON;
      stance[2] = STANCE_BULL;
      break;
    case STANCE_DRAGON:
      stance[0] = STANCE_SWALLOW;
      stance[1] = STANCE_DRAGON;
      stance[2] = STANCE_BULL;
      break;
    case STANCE_SWALLOW:
      stance[0] = STANCE_DRAGON;
      stance[1] = STANCE_SWALLOW;
      stance[2] = STANCE_VIPER;
      break;
  }

  /* already doing quite well */
  if (ch->stance[0] == stance[0] ||
      ch->stance[0] == stance[1] ||
      ch->stance[0] == stance[2])
    return;

  if (ch->stance[stance[0]] > 0)
    newstance = stance[0];
  else if (ch->stance[stance[1]] > 0)
    newstance = stance[1];
  else if (ch->stance[stance[2]] > 0)
    newstance = stance[2];
  else
  {
    for (i = 10; i > 0; i--)
    {
      if (ch->stance[i] > 0)
      {
        newstance = i;
        break;
      }
    }

    /* this mob have no stances at all */
    if (i <= 0)
      return;
  }

  /* already in this stance? */
  if (newstance == ch->stance[0])
    return;

  /* relax from old fighting stance */
  if (ch->stance[0] != 0)
  {
    act("$n relaxes from $s fighting stance.", ch, NULL, NULL, TO_ROOM);
    ch->stance[0] = 0;
  }

  switch(newstance)
  {
    default:
      break;
    case STANCE_BULL:
      do_stance(ch, "bull");
      break;
    case STANCE_CRAB:
      do_stance(ch, "crab");
      break;
    case STANCE_CRANE:
      do_stance(ch, "crane");
      break;
    case STANCE_VIPER:
      do_stance(ch, "viper");
      break;
    case STANCE_MONGOOSE:
      do_stance(ch, "mongoose");
      break;
    case STANCE_SWALLOW:
      do_stance(ch, "swallow");
      break;
    case STANCE_DRAGON:
      do_stance(ch, "dragon");
      break;
    case STANCE_TIGER:
      do_stance(ch, "tiger");
      break;
    case STANCE_MANTIS:
      do_stance(ch, "mantis");
      break;
    case STANCE_MONKEY:
      do_stance(ch, "monkey");
      break;
  }  
}

char *col_scale(int current, int max)
{
  static char buf[MAX_STRING_LENGTH];

  if (current < 1)
  {
    sprintf(buf, "#R%d#n", current);
  }
  else if (max > 0)
  {
    if (100 * current / max < 25)
      sprintf(buf, "#R%d#n", current);
    else if (100 * current / max < 50)
      sprintf(buf, "#L%d#n", current);
    else if (100 * current / max < 75)
      sprintf(buf, "#G%d#n", current);
    else if (100 * current / max < 100)
      sprintf(buf, "#y%d#n", current);
    else
      sprintf(buf, "#C%d#n", current);
  }
  else
  {
    sprintf(buf, "#C%d#n", current);
  }

  return buf;
}

int getGold(CHAR_DATA *ch)
{
  if (IS_NPC(ch) || ch->desc == NULL)
    return 0;

  return ch->desc->account->goldcrowns;
}

void setGold(CHAR_DATA *ch, int amount)
{
  if (IS_NPC(ch) || ch->desc == NULL)
    return;

  ch->desc->account->goldcrowns += amount;

  save_account(ch->desc->account);
}

int getGoldTotal(CHAR_DATA *ch)
{
  if (IS_NPC(ch) || ch->desc == NULL)
    return 0;
  
  return ch->desc->account->goldtotal;
}

void login_char(CHAR_DATA *ch, bool new_player)
{
  DESCRIPTOR_DATA *d = ch->desc;
  char *strtime;

  /* set lasthost */
  if (d)
  {
    free_string(ch->lasthost);
    ch->lasthost = str_dup(HOSTNAME(d));
  }
  else
  {
    free_string(ch->lasthost);
    ch->lasthost = str_dup("(unknown)");
  }

  /* set last login */
  strtime = ctime(&current_time);
  strtime[strlen(strtime) - 1] = '\0';
  free_string(ch->lasttime);
  ch->lasttime = str_dup(strtime);

  /* connect to char_list */
  AttachToList(ch, char_list);

  /* initialize player events */
  init_events_player(ch);

  if (d)
  {
    strip_event_socket(d, EVENT_SOCKET_IDLE);
    d->connected = CON_PLAYING;
  }

  send_to_char("\n\r", ch);
  if (new_player)
  {
    char_to_room(ch, get_room_index(ROOM_VNUM_SCHOOL), TRUE);
    do_look(ch, "auto");
  }
  else if (ch->in_room != NULL)
  {
    char_to_room(ch, ch->in_room, TRUE);
    do_look(ch, "auto");
  }
  else
  {
    char_to_room(ch, get_room_index(ROOM_VNUM_CITYCENTER), TRUE);
    do_look(ch, "auto");
  }

  /* move out of arena */
  if (in_arena(ch) || in_fortress(ch))
  {
    send_to_char("\n\r#RYou have been transfered!!#n\n\n\r", ch);
    char_from_room(ch);
    char_to_room(ch, get_room_index(ROOM_VNUM_CITYCENTER), TRUE);
  }

  /* update archmage information */
  if (ch->class == CLASS_WARLOCK)
    update_archmage(ch);

  /* and update membership of kingdoms */
  update_kingdom_membership(ch, TRUE);

  /* changed the pfile structure ? */
  update_revision(ch);
}

bool check_feed(CHAR_DATA *attacker, CHAR_DATA *defender)
{
  FEED_DATA *feed;
  ITERATOR *pIter;

  if (IS_NPC(attacker) || IS_NPC(defender))
    return FALSE;

  pIter = AllocIterator(defender->pcdata->feeders);
  while ((feed = (FEED_DATA *) NextInList(pIter)) != NULL)
  {
    /* it's obviously not a feed if the attacker does the killing */
    if (feed->playerid == attacker->pcdata->playerid)
      continue;

    /* it's not a feed if the aggressor in feed_data is low enough might
     * or the fight would have been considered fair (decap wise).
     */
    if (feed->fair || 80 * feed->might / 100 <= getMight(defender))
      continue;

    /* someone who was not the attacker and who wouldn't normally be
     * able to kill this person has done substantial damage to this
     * player, and thus this is considered a feed.
     */
    return TRUE;
  }

  return FALSE;
}

bool can_kill_lowbie(CHAR_DATA *highbie, CHAR_DATA *lowbie)
{
  FEED_DATA *feed;
  ITERATOR *pIter;

  if (IS_NPC(highbie) || IS_NPC(lowbie))
    return FALSE;

  pIter = AllocIterator(highbie->pcdata->feeders);
  while ((feed = (FEED_DATA *) NextInList(pIter)) != NULL)
  {
    if (feed->playerid == lowbie->pcdata->playerid)
      return TRUE;
  }

  return FALSE;
}

void update_feed(CHAR_DATA *attacker, CHAR_DATA *defender)
{
  FEED_DATA *feed;
  ITERATOR *pIter;

  if (IS_NPC(attacker) || IS_NPC(defender))
    return;

  /* first we check if the attacker already has a feed_data
   * from the defender, in which case we do nothing.
   */
  pIter = AllocIterator(attacker->pcdata->feeders);
  while ((feed = (FEED_DATA *) NextInList(pIter)) != NULL)
  {
    if (feed->playerid == defender->pcdata->playerid)
      return;
  }

  /* then we check if the defender has the attacker as a feed_data,
   * in which case we update the time on that feed_data.
   */
  pIter = AllocIterator(defender->pcdata->feeders);
  while ((feed = (FEED_DATA *) NextInList(pIter)) != NULL)
  {
    if (feed->playerid == attacker->pcdata->playerid)
    {
      feed->time = 6;
      return;
    }
  }

  /* we only make new feed_data entries if the player is at a decent
   * health level - there is no point in remembering someone who really
   * didn't help with the killing.
   */
  if (defender->hit < 25 * defender->max_hit / 100)
    return;

  /* if neither the attacker or the defender has any memory of
   * eachother, we add a feed flag to the defender with all the
   * information needed.
   */
  feed = alloc_feed();
  feed->playerid = attacker->pcdata->playerid;
  feed->might = getMight(attacker);
  feed->fair = fair_fight(attacker, defender);

  /* link it to the defenders list of feed_data */
  AttachToList(feed, defender->pcdata->feeders);
}

void free_feed(CHAR_DATA *ch, FEED_DATA *feed)
{
  if (!feed || !ch || IS_NPC(ch))
  {
    bug("free_feed: Freeing bad feed data.", 0);
    return;
  }

  DetachFromList(feed, ch->pcdata->feeders);
  PushStack(feed, feed_free);
}

SESSION_DATA *alloc_session()
{
  SESSION_DATA *session;

  if ((session = (SESSION_DATA *) PopStack(session_free)) == NULL)
  {
    session = calloc(1, sizeof(*session));
  }

  session->mana = 0;
  session->move = 0;
  session->hit = 0;
  session->exp = 0;
  session->gold = 0;
  session->quests = 0;
  session->mkills = 0;
  session->pkills = 0;

  return session;
}

void free_session(CHAR_DATA *ch)
{
  if (!ch || IS_NPC(ch) || !ch->pcdata->session)
  {
    bug("free_session: Freeing bad feed data.", 0);
    return;
  }

  PushStack(ch->pcdata->session, session_free);
  ch->pcdata->session = NULL;
}

FEED_DATA *alloc_feed()
{
  FEED_DATA *feed;

  if ((feed = (FEED_DATA *) PopStack(feed_free)) == NULL)
  {
    feed = calloc(1, sizeof(*feed));
  }

  feed->time     = 6;  /* 20-24 seconds */
  feed->playerid = 0;
  feed->might    = 0;
  feed->fair     = FALSE;

  return feed;
}

void setGoldTotal(CHAR_DATA *ch, int amount)
{
  if (IS_NPC(ch) || ch->desc == NULL)
    return;  
 
  ch->desc->account->goldtotal += amount;
}

void damage_obj(CHAR_DATA *ch, OBJ_DATA *obj, int dam)
{
  /* natural toughness resist all damage ? */
  if (dam - obj->toughness < 0)
    return;

  /* deal damage to object */
  if ((dam - obj->toughness > obj->resistance))
    obj->condition -= obj->resistance;
  else
    obj->condition -= (dam - obj->toughness);

  /* break item ? */
  if (obj->condition < 1)
    break_obj(ch, obj);
}

bool account_olc_area(ACCOUNT_DATA *account, AREA_DATA *pArea)
{
  char buf[MAX_INPUT_LENGTH];
  char *ptr;

  if (pArea == NULL || account == NULL)
    return FALSE;

  if (account->level == CODER_ACCOUNT)
    return TRUE;

  if (pArea->builders[0] == '\0')
    return FALSE;

  if (strstr(pArea->builders, "All"))
    return TRUE;

  if (account->players[0] == '\0')
    return FALSE;

  ptr = get_token(account->players, buf);
  do
  {
    if (strstr(pArea->builders, buf))
      return TRUE;

    ptr = get_token(ptr, buf);                             
    ptr = get_token(ptr, buf);
    ptr = get_token(ptr, buf);
    ptr = get_token(ptr, buf);
  } while (*ptr != '\0');

  return FALSE;
}

void char_popup(CHAR_DATA *ch)
{
  ACCOUNT_DATA *account;
  int max_popup_newbie = 8;

  if (IS_NPC(ch) || ch->desc == NULL) return;
  if ((account = ch->desc->account) == NULL) return;
  if (account->popup / 3 >= max_popup_newbie) return;

  ++account->popup;

  if (account->popup % 3) return;

  switch(account->popup / 3)
  {
    default:
      break;
    case 1:
      newbie_hint(ch, "Before you enter the game, you should first pick up a set of "
                      "equipment. You can find a decent set of starting equipment in "
                      "the area where you start. The newbie monsters each hold a piece "
                      "or two, and most of them has some important spells on them.");
      break;
    case 2:
      newbie_hint(ch, "Once you have left the newbie portal, you should type 'AREAS' "
                      "to get a list of areas (and the walking directions). The areas "
                      "are listed in a growing order of strength, and you are encouraged "
                      "to visit one of the top areas. Remember to train avatar once you "
                      "reach 2000 hitpoints.");
      break;
    case 3:
      newbie_hint(ch, "Training your skills in both stances and different weapons is "
                      "usually a good thing. The better you are at using weapons, the "
                      "better you also become at avoiding being hit. The Crane and "
                      "Mongoose stance are good stances for killing monsters.");
      break;
    case 4:
      newbie_hint(ch, "If you lose your newbie equipment, or for some reason want to "
                      "go back to the newbie portal, you should practice and cast the "
                      "newbiespell (type 'practice newbiespell' then 'cast newbiespell').");
      break;
    case 5:
      newbie_hint(ch, "All the helpfiles on this MUD can be read from the games homepage. "
                      "Helpfiles are not the only thing available - the homepage also allows "
                      "you to read and write notes, see who is online, and change your "
                      "account preferences. Read HELP HOMEPAGE for the address.");
      break;
    case 6:
      newbie_hint(ch, "There are several commands available that can be used to change your "
                      "settings and preferences. You can use these commands to make things "
                      "easier for yourself. Try the following commands : enhcombat, brief, "
                      "config, alias.");
      break;
    case 7:
      newbie_hint(ch, "Teaming together with other players to kill monsters for experience "
                      "is usually a good idea, though you should be aware that experience "
                      "from killing monsters is awarded by merit, and not by simply being "
                      "a member of the group killing the monsters. If you want to know more "
                      "about the experience system, you should read HELP GROUPEXP.");
      break;
    case 8:
      newbie_hint(ch, "Equipment comes in various strength, and though the equipment that "
                      "you get at the newbie portal can bring you a long way, there exists "
                      "better and more powerful equipment that will help you fight better. "
                      "You can find a list of equipment types, strengths and where to get that "
                      "in the EQINFO helpfile.");
      break;
  }
}

void newbie_hint(CHAR_DATA *ch, char *hint)
{
  BUFFER *buf = buffer_new(MAX_STRING_LENGTH);

  bprintf(buf, " %s\n\n\r",
    get_dystopia_banner("MUD Information", 72));

  bprintf(buf, "  %s\n\n\r%s\n\r",
    line_indent(hint, 2, 71),
    get_dystopia_banner("", 72));

  send_to_char(buf->data, ch);
  buffer_free(buf);
}

void fix_magicskill(CHAR_DATA *ch, bool stolen)
{
  int magic[5];
  int i;

  if (IS_NPC(ch)) return;

  switch(ch->class)
  {
    default:
      bug("Fix_magicskill: unknown class %d.", ch->class);
      return;
      break;
    case 0:
    case CLASS_GIANT:
    case CLASS_SHADOW:
    case CLASS_FAE:
      for (i = 0; i < 5; i++)
        magic[i] = 200;
      break;
    case CLASS_WARLOCK:
      for (i = 0; i < 5; i++)
      {
        if (ch->pcdata->powers[WARLOCK_RANK] == WLCK_RNK_WARLOCK)
          magic[i] = 220;
        else if (ch->pcdata->powers[WARLOCK_RANK] == WLCK_RNK_ARCHMAGE)
          magic[i] = 240;
        else
          magic[i] = 200;
      }
      break;
  }

  /* add generation */
  for (i = 0; i < 5; i++)
  {
    if (ch->generation == 1)
      magic[i] += magic[i] * 0.12;
    else if (ch->generation == 2)
      magic[i] += magic[i] * 0.06;

    /* if you steal to generation 1 or 2, you gain a 6% boost */
    if (stolen && (ch->generation == 1 || ch->generation == 2))
      ch->spl[i] += ch->spl[i] * 0.06;

    /* cut down to maximum skill */
    if (ch->spl[i] > magic[i])
      ch->spl[i] = magic[i];
  }
}

void fix_weaponskill(CHAR_DATA *ch, bool stolen)
{
  int weap[13];
  int i;

  switch(ch->class)
  {
    default:
      bug("Fix_weaponskill: unknown class %d.", ch->class);
      return;
      break;
    case 0:
      for (i = 0; i < 13; i++)
        weap[i] = 200;
      break;
    case CLASS_GIANT:
      for (i = 0; i < 13; i++)
        weap[i] = 300;
      break;
    case CLASS_SHADOW:
      for (i = 0; i < 13; i++)
        weap[i] = 350;
      break;
    case CLASS_WARLOCK:
      for (i = 0; i < 13; i++)
        weap[i] = 250;
      break;
    case CLASS_FAE:
      for (i = 0; i < 13; i++)
        weap[i] = 225;
      weap[6] = 600;
      break;
  }

  /* add generation */
  for (i = 0; i < 13; i++)
  {
    if (ch->generation == 1)
      weap[i] += weap[i] * 0.2;
    else if (ch->generation == 2)
      weap[i] += weap[i] * 0.1;

    /* if you steal to generation 1 or 2, you gain a 10% boost */
    if (stolen && (ch->generation == 1 || ch->generation == 2))
      ch->wpn[i] += ch->wpn[i] * 0.1;

    /* cut down to maximum skill */
    if (ch->wpn[i] > weap[i])
      ch->wpn[i] = weap[i];
  }
}

void obj_say(OBJ_DATA *obj, char *txt, char *type)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *ch;

  /* do we have someone listening ? */
  if ((ch = obj->carried_by) != NULL)
    ;
  else if (obj->in_room != NULL && SizeOfList(obj->in_room->people) > 0)
  {
    ch = (CHAR_DATA *) FirstInList(obj->in_room->people);
  }
  else return;

  sprintf(buf, "$p %s '#y%s#n'.", (type != NULL) ? type : "says", txt);

  act(buf, ch, obj, NULL, TO_ALL);
}

bool is_silenced(CHAR_DATA *ch)
{
  /* mobiles can always chat */
  if (IS_NPC(ch)) return FALSE;

  /* linkdead players cannot chat */
  if (ch->desc == FALSE || ch->desc->account == FALSE)
    return TRUE;

  /* silenced players cannot chat */
  if (IS_SET(ch->desc->account->flags, ACCOUNT_FLAG_SILENCED))
    return TRUE;

  /* he/she/it can chat */
  return FALSE;
}

bool pre_reboot_actions(bool reboot)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *gch;
  ITERATOR *pIter;
  FILE *fp = NULL;

  /* try to open copyover file */
  if (reboot && (fp = fopen(COPYOVER_FILE, "w")) == NULL)
  {
    log_string("Could not write to copyover file: %s", COPYOVER_FILE);
    perror("do_copyover:fopen");
    return FALSE;
  }

  /* Store all mudwide data before doing anything else */
  save_muddata();
  save_polls();
  save_kingdoms();
  save_artifact_table();
  save_archmages();
  save_auctions();

  /* close webserver */
  close_webserver();

  /* run through the char_list, and do whatever */
  pIter = AllocIterator(char_list);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_NPC(gch)) continue;

    /* restore player */
    restore_player(gch);

    /* call all items */
    call_all(gch);
  }

  /* run through the char_list, and save the pfiles.
   * It should be noted that we do _not_ save above to
   * avoid object dublication due to call_all().
   */
  pIter = AllocIterator(char_list);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_NPC(gch)) continue;

    /* save player */
    save_char_obj(gch);
  }

  /* Have to disable compression when doing a copyover */
  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if ((gch = d->character) == NULL || d->connected != CON_PLAYING)
    {
      write_to_descriptor_2(d->descriptor, "\n\rSorry, we are rebooting. Come back in 30 seconds.\n\r", 0);
      close_socket(d);
      continue;
    }

    if (d->out_compress)
    {
      write_to_buffer(d, "Disabling Compression.\n\n\r", 0);
      compressEnd(d);
    }

    if (!reboot) continue;

    fprintf(fp, "%d %s %s %s\n", d->descriptor, gch->name, gch->pcdata->account, HOSTNAME(d));
  }

  if (reboot)
  {
    fprintf(fp, "-1\n");
    fclose(fp);
  }

  /* recycle descriptors - we just want them to close */
  recycle_descriptors();

  return TRUE;
}

char *replace_letter_with_word(char *txt, char letter, char *word)
{
  static char buf[MAX_STRING_LENGTH];
  int i = 0, j = 0;

  while (txt[i] != '\0')
  {
    if (UPPER(txt[i]) == UPPER(letter) &&
       (txt[i + 1] == ' ' || txt[i + 1] == '.' || txt[i + 1] == '\0') &&
       (i == 0 || txt[i - 1] == ' ' || txt[i - 1] == '.'))
    {
      int k = 0;

      while (word[k] != '\0')
      {
        if (k == 0 && UPPER(txt[i]) == txt[i])
        {
          buf[j++] = UPPER(word[k]);
          k++;
        }
        else
        {
          buf[j++] = word[k++];
        }
      }
    }
    else
    {
      buf[j++] = txt[i];
    }

    i++;
  }
  buf[j] = '\0';

  return buf;
}

char *get_date_string(time_t tm )
{
  static char buf[100];
  char *ptr = ctime(&tm);

  /* the day of the month */
  if (ptr[8] != ' ')
    buf[0] = ptr[8];
  else
    buf[0] = '0';
  buf[1] = ptr[9];
  buf[2] = '-';

  /* find the month */
  if (!memcmp(&ptr[4], "Jan", 3))
    buf[3] = '0', buf[4] = '1', buf[5] = '-';
  else if (!memcmp(&ptr[4], "Feb", 3))
    buf[3] = '0', buf[4] = '2', buf[5] = '-';
  else if (!memcmp(&ptr[4], "Mar", 3))
    buf[3] = '0', buf[4] = '3', buf[5] = '-';
  else if (!memcmp(&ptr[4], "Apr", 3))
    buf[3] = '0', buf[4] = '4', buf[5] = '-';
  else if (!memcmp(&ptr[4], "May", 3))
    buf[3] = '0', buf[4] = '5', buf[5] = '-';
  else if (!memcmp(&ptr[4], "Jun", 3))
    buf[3] = '0', buf[4] = '6', buf[5] = '-';
  else if (!memcmp(&ptr[4], "Jul", 3))
    buf[3] = '0', buf[4] = '7', buf[5] = '-';
  else if (!memcmp(&ptr[4], "Aug", 3))
    buf[3] = '0', buf[4] = '8', buf[5] = '-';
  else if (!memcmp(&ptr[4], "Sep", 3))
    buf[3] = '0', buf[4] = '9', buf[5] = '-';
  else if (!memcmp(&ptr[4], "Oct", 3))
    buf[3] = '1', buf[4] = '0', buf[5] = '-';
  else if (!memcmp(&ptr[4], "Nov", 3))
    buf[3] = '1', buf[4] = '1', buf[5] = '-';
  else if (!memcmp(&ptr[4], "Dec", 3))
    buf[3] = '1', buf[4] = '2', buf[5] = '-';
  else
    buf[3] = '?', buf[4] = '?', buf[5] = '-';

  buf[6] = ptr[22];
  buf[7] = ptr[23];
  buf[8] = '\0';

  return buf;
}

int check_pkstatus(CHAR_DATA *ch, CHAR_DATA *victim)
{
  int value = 0;

  if (fair_fight(ch, victim))
    value += 1;
  if (fair_fight(victim, ch))
    value += 2;

  return value;
}

CHAR_DATA *get_online_player(int playerid)
{
  CHAR_DATA *ch;
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if ((ch = d->character) == NULL) continue;
    if (ch->in_room == NULL) continue;
    if (ch->pcdata->playerid != playerid) continue;

    return ch;
  }

  return NULL;
}

void check_top10(CHAR_DATA *ch)
{
  TOP10_ENTRY *entry;
  ITERATOR *pIter;
  bool removed = FALSE, changed = FALSE;

  if (IS_NPC(ch)) return;

  pIter = AllocIterator(top10_list);
  while ((entry = (TOP10_ENTRY *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(entry->name, ch->name))
      break;
  }

  /* if already in list */
  if (entry != NULL)
  {
    DetachAtIterator(pIter);
    free_top10entry(entry);
    removed = TRUE;
  }

  /* scan list, see where the player fits */
  pIter = AllocIterator(top10_list);  
  while ((entry = (TOP10_ENTRY *) NextInList(pIter)) != NULL)
  {
    if (get_ratio(ch) > entry->pkscore ||
       (get_ratio(ch) == entry->pkscore && ch->pkill > entry->pkills) ||
       (get_ratio(ch) == entry->pkscore && ch->pkill == entry->pkills &&
       ((ch->played + (int) (current_time - ch->logon)) / 3600) > entry->hours))
      break;
  }

  if (entry == NULL && removed == TRUE)
  {
    entry = alloc_top10entry(ch);

    AttachToEndOfList(entry, top10_list);
    changed = TRUE;
  }
  else if (entry != NULL)
  {
    TOP10_ENTRY *newentry;

    newentry = alloc_top10entry(ch);
    AttachToListBeforeItem(newentry, top10_list, entry);

    changed = TRUE;
  }

  /* remove the last entry in the list */
  if (changed && removed == FALSE)
  {
    TOP10_ENTRY *lastEntry = NULL;

    pIter = AllocIterator(top10_list);
    while ((entry = (TOP10_ENTRY *) NextInList(pIter)) != NULL)
    {
      lastEntry = entry;
    }

    if (lastEntry != NULL)
    {
      DetachAtIterator(pIter);
      free_top10entry(lastEntry);
    }
  }

  if (changed)
    save_top10();
}

/*
 * Knuth-Morris-Pratt Pattern Matching Algorithm (sensitive)
 */
bool is_contained2(const char *astr, const char *bstr)
{
  int n = strlen(bstr), m = strlen(astr), i = 1, j = 0;
  int *f;

  /* if the pattern is longer than the string */
  if (m > n) return FALSE;

  f = malloc(m * sizeof(int));

  f[0] = 0;

  /* calculating the error fuction f[] */
  while (i < m)
  {
    if (astr[j] == astr[i])
    {
      f[i] = j + 1;
      i++; j++;
    }
    else if (j > 0) j = f[j - 1];
    else
    {
      f[i] = 0;
      i++;
    }
  }

  j = 0;

  /* KMP algorith */
  for (i = 0; i < n; i++)
  {
    while (j > 0 && astr[j] != bstr[i])
      j = f[j-1];
    if (astr[j] == bstr[i]) j++;
    if (j == m)
    {
      free(f);
      return TRUE;
    }
  }

  free(f);
  return FALSE;
}

/*  
 * Knuth-Morris-Pratt Pattern Matching Algorithm (insensitive)
 */
bool is_contained(const char *astr, const char *bstr)
{
  int n = strlen(bstr), m = strlen(astr), i = 1, j = 0;
  int *f;
      
  /* if the pattern is longer than the string */
  if (m > n) return FALSE;

  f = malloc(m * sizeof(int));

  f[0] = 0;
    
  /* calculating the error fuction f[] */
  while (i < m)
  {
    if (UPPER(astr[j]) == UPPER(astr[i]))
    {
      f[i] = j + 1;
      i++; j++;  
    }
    else if (j > 0) j = f[j - 1];
    else
    {
      f[i] = 0;
      i++;
    }
  }

  j = 0;
    
  /* KMP algorith */
  for (i = 0; i < n; i++)
  {
    while (j > 0 && UPPER(astr[j]) != UPPER(bstr[i]))
      j = f[j-1];  
    if (UPPER(astr[j]) == UPPER(bstr[i])) j++;
    if (j == m)
    {
      free(f);
      return TRUE;
    }
  }

  free(f);
  return FALSE;
}

int strlen2(const char *s)
{
  int i, b, count=0;

  if (s[0] == '\0') return 0;
  b = strlen(s);
  for (i = 0; i < b; i++)
  {
    if (s[i] == '#') count++;
  }
  return (b + 7 * count);
}

void clearstats(CHAR_DATA *ch)
{
  ITERATOR *pIter;
  AFFECT_DATA *paf;

  if (IS_NPC(ch)) return;

  pIter = AllocIterator(ch->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    affect_remove(ch, paf);
  }

  powerdown(ch);
  save_char_obj(ch);

  send_to_char("Your stats have been cleared.  Please rewear your equipment.\n\r",ch);
}

void ragnarok_stop()
{
  ITERATOR *pIter;
  DESCRIPTOR_DATA *d;
  
  ragnarok = FALSE;
  do_info(NULL,"#CPeace has been restored in the realms, the time of ragnarok is no more#n");

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->character && d->connected == CON_PLAYING)
    {
      d->character->fight_timer = 0;
      d->character->pcdata->safe_counter = 5;
      call_all(d->character);
      restore_player(d->character);
    }
  }
  return;
}

void logout_message(CHAR_DATA *ch)
{
  ITERATOR *pIter;
  static char * const he_she  [] = { "XX", "he",  "she" };
  static char * const him_her [] = { "XX", "him", "her" };
  static char * const his_her [] = { "XX", "his", "her" };

  DESCRIPTOR_DATA *d;
  char buf[400]; /* that should be plenty. */
  const char *dmess;
  const char *i;
  char *ptr2;
  char *ptr;
  int size;

  size = strlen2(ch->pcdata->logoutmessage);
  if (size > 380)
  {
    bug("Bad logoutmessage.",0);
    return;
  }

  ptr2  = "#C<- #RLeaves #C->#n ";
  ptr   = buf;
  dmess = ch->pcdata->logoutmessage;
      
  while ((*ptr = *ptr2) != '\0')
    ++ptr, ++ptr2;

  while (*dmess != '\0')
  {
    if ( *dmess != '$' )
    {
      *ptr++ = *dmess++;
      continue;
    }
    ++dmess;
    switch (*dmess)
    {
      default:  i = ""; break;
      case 'n': i = ch->name; break;
      case 'e': i = he_she  [URANGE(1, ch->sex, 2)]; break;
      case 'm': i = him_her [URANGE(1, ch->sex, 2)]; break;
      case 's': i = his_her [URANGE(1, ch->sex, 2)]; break;
    }
    ++dmess;
    /* copying the data into the pointer */
    while ((*ptr = *i) != '\0')
      ++ptr, ++i;
  }
  *ptr++ = '\n';
  *ptr++ = '\r';  

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected != CON_PLAYING ) continue;
    write_to_buffer( d, buf, ptr - buf );
  }
}

void tie_message(CHAR_DATA *ch, CHAR_DATA *victim)
{
  static char * const he_she  [] = { "XX", "he",  "she" };
  static char * const him_her [] = { "XX", "him", "her" };
  static char * const his_her [] = { "XX", "his", "her" };
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;
  char buf[400]; /* that should be plenty. */
  const char *dmess;
  const char *i;
  char *ptr2;
  char *ptr;
  int size;

  size = strlen2(ch->pcdata->tiemessage);
  if (size > 380)
  {
    bug("Bad tiemessage.",0);
    return;
  }

  ptr2  = "#C<- #RTie #C->#n ";
  ptr   = buf;
  dmess = ch->pcdata->tiemessage;  

  while ((*ptr = *ptr2) != '\0')
    ++ptr, ++ptr2;

  while (*dmess != '\0')
  {
    if ( *dmess != '$' )
    {
      *ptr++ = *dmess++;
      continue;
    }
    ++dmess;
    switch (*dmess)
    {
      default:  i = ""; break;
      case 'n': i = ch->name; break;
      case 'e': i = he_she  [URANGE(1, ch->sex, 2)]; break;
      case 'm': i = him_her [URANGE(1, ch->sex, 2)]; break;
      case 's': i = his_her [URANGE(1, ch->sex, 2)]; break;
      case 'N': i = victim->name; break;
      case 'S': i = his_her [URANGE(1, victim->sex, 2)]; break;
      case 'M': i = him_her [URANGE(1, victim->sex, 2)]; break;
      case 'E': i = he_she  [URANGE(1, victim->sex, 2)]; break;
    }
    ++dmess;
    /* copying the data into the pointer */
    while ((*ptr = *i) != '\0')
      ++ptr, ++i;
  }
  *ptr++ = '\n';
  *ptr++ = '\r';

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected != CON_PLAYING ) continue;
    write_to_buffer( d, buf, ptr - buf );
  }
  return;
}

void login_message(CHAR_DATA *ch)
{
  static char * const he_she  [] = { "XX", "he",  "she" };
  static char * const him_her [] = { "XX", "him", "her" };
  static char * const his_her [] = { "XX", "his", "her" };
  ITERATOR *pIter;
  DESCRIPTOR_DATA *d;
  char buf[400]; /* that should be plenty. */
  const char *dmess;
  const char *i; 
  char *ptr2;
  char *ptr;
  int size;
    
  size = strlen2(ch->pcdata->loginmessage); 
  if (size > 380)
  {
    bug("Bad loginmessage.",0);
    return;
  }

  ptr2  = "#C<- #REnters #C->#n ";
  ptr   = buf;
  dmess = ch->pcdata->loginmessage;

  while ((*ptr = *ptr2) != '\0')
    ++ptr, ++ptr2;
   
  while (*dmess != '\0')
  {
    if ( *dmess != '$' )
    {
      *ptr++ = *dmess++;
      continue;
    }
    ++dmess;
    switch (*dmess)
    {
      default:  i = ""; break;
      case 'n': i = ch->name; break;
      case 'e': i = he_she  [URANGE(1, ch->sex, 2)]; break;
      case 'm': i = him_her [URANGE(1, ch->sex, 2)]; break;
      case 's': i = his_her [URANGE(1, ch->sex, 2)]; break;
    }
    ++dmess;
    /* copying the data into the pointer */
    while ((*ptr = *i) != '\0')
      ++ptr, ++i;
  }
  *ptr++ = '\n';
  *ptr++ = '\r';
    
  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected != CON_PLAYING ) continue;
    write_to_buffer( d, buf, ptr - buf );
  }
}

void avatar_message(CHAR_DATA *ch)
{
  static char * const he_she  [] = { "XX", "he",  "she" };
  static char * const him_her [] = { "XX", "him", "her" };
  static char * const his_her [] = { "XX", "his", "her" };
  ITERATOR *pIter;
  DESCRIPTOR_DATA *d;   
  char buf[400]; /* that should be plenty. */
  const char *dmess;
  const char *i;
  char *ptr2;
  char *ptr;
  int size;
      
  size = strlen2(ch->pcdata->avatarmessage);
  if (size > 380)
  {
    bug("Bad loginmessage.",0);
    return;
  }
  
  ptr2  = "#C<- #RAvatar #C->#n ";
  ptr   = buf;
  dmess = ch->pcdata->avatarmessage;
   
  while ((*ptr = *ptr2) != '\0')
    ++ptr, ++ptr2;
    
  while (*dmess != '\0')
  {
    if ( *dmess != '$' )
    {
      *ptr++ = *dmess++;
      continue;
    }
    ++dmess;
    switch (*dmess) 
    {
      default:  i = ""; break;
      case 'n': i = ch->name; break;
      case 'e': i = he_she  [URANGE(1, ch->sex, 2)]; break;
      case 'm': i = him_her [URANGE(1, ch->sex, 2)]; break;
      case 's': i = his_her [URANGE(1, ch->sex, 2)]; break;
    }
    ++dmess;
    /* copying the data into the pointer */
    while ((*ptr = *i) != '\0')
      ++ptr, ++i;
  }
  *ptr++ = '\n';
  *ptr++ = '\r';
   
  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected != CON_PLAYING ) continue;
    write_to_buffer( d, buf, ptr - buf );
  }
}

int get_ratio(CHAR_DATA *ch)
{
  long ratio;

  if (IS_NPC(ch))
    return 0;

  if ((ch->pkill + ch->pdeath) == 0)
    ratio = 0;
  else if (ch->pkill > ch->pdeath)
    ratio = ch->pkill * 100 * ((ch->pkill * ch->pkill) - (ch->pdeath * ch->pdeath))/((ch->pkill + ch->pdeath) * (ch->pkill + ch->pdeath));
  else if (ch->pkill > 0)
    ratio = (-100) * (ch->pdeath - ch->pkill) / ch->pkill;
  else
    ratio = (-100) * ch->pdeath;

  /* add status */
  ratio = (UMAX(1, (int) sqrt(100 * ch->pcdata->status)) * ratio) / 100;

  return (int) ratio;
}

void death_info(char *str)
{
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];
    
  if (str[0] == '\0') return;
     
  sprintf(buf, "#C<- #RDeath #C->#n %s\n\r", str);

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected == CON_PLAYING && d->character != NULL)
      send_to_char(buf, d->character);
  }
  return;
}

void avatar_info(char *str)
{
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];
    
  if (str[0] == '\0') return;
     
  sprintf(buf, "#C<- #RAvatar #C->#n %s\n\r", str);

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected == CON_PLAYING && d->character != NULL)
      send_to_char(buf, d->character);
  }
  return;
}

void leave_info(char *str)
{
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];
    
  if (str[0] == '\0') return;
     
  sprintf(buf, "#C<- #RLeaves #C->#n %s\n\r", str);

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected == CON_PLAYING && d->character != NULL)
      send_to_char(buf, d->character);
  }
}

void enter_info(char *str)
{
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];
    
  if (str[0] == '\0') return;
     
  sprintf(buf, "#C<- #REnters #C->#n %s\n\r", str);

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected == CON_PLAYING && d->character != NULL)
      send_to_char(buf, d->character);
  }
}

int getMobMight(MOB_INDEX_DATA *pMob)
{
  int mod = 0;

  mod += URANGE(-12, pMob->extra_parry / 4, 12);
  mod += URANGE(-12, pMob->extra_dodge / 4, 12);
  mod += URANGE(0, 3 * pMob->extra_attack / 2, 22);
  mod += URANGE(-5, pMob->dam_modifier / 10, 20);
  mod += URANGE(-25, pMob->toughness / 4, 25);

  return (((100 + mod) * pMob->level) / 100);
}

/* 40K hps + 5K per evolution - 10 pl per 1K gives 400 pl + 50 pl/lvl
 * each 66 points in a spell gives 1 pl, 5x3 = 15 pl for all spells
 * each 40 points in a weapon gives 1 pl, 13x5 = 65 pl for all weapons
 * each 40 points in a stance gives 1 pl, 10x5 = 50 pl for all stances
 * the mastery item will grant an additional 10 pl, so a mastered char
 * will gain an additional 140 pl (equal to 14K hps). Each evolve grants
 * a mandatory 50 pl for increased damcap/dodge/etc.
 *
 * Fully mastered non-evoled character with max hps has 540 pl
 * Each evolve with maxing of hps grants 100 extra pl.
 *
 * I suggest allowing for 4 evolves before reaching almighty status.
 * Also remember that each pk power grants 10 pl points.
 */
int getMight(CHAR_DATA *ch)
{
  AFFECT_DATA *paf;
  OBJ_DATA *obj;
  ITERATOR *pIter;
  int spellhps = 0, classpower = 0;
  int objhps = 0, might, i;
  int classlevel = 100;

  if (IS_NPC(ch))
    return getMobMight(ch->pIndexData);

  /* check for items that modify hitpoints */
  for (i = 0; i < MAX_WEAR; i++)
  {
    if ((obj = get_eq_char(ch, i)) == NULL) continue;

    pIter = AllocIterator(obj->pIndexData->affected);
    while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
    {
      if (paf->location == APPLY_HIT)
        objhps += paf->modifier;
    }

    pIter = AllocIterator(obj->affected);
    while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
    {
      if (paf->location == APPLY_HIT)                       
        objhps += paf->modifier;
    }
  }

  /* check for spell affects that modify hitpoints */
  pIter = AllocIterator(ch->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->location == APPLY_HIT)
      spellhps += paf->modifier;   
  }

  /* set base from hitpoints */
  might = (ch->max_hit - (spellhps + objhps)) / 100;

  /* add stances, spells, weapons and mastery */
  for (i = 0; i < 5; i++)  might += UMIN(3, ch->spl[i] / 66);
  for (i = 0; i < 13; i++) might += UMIN(5, ch->wpn[i] / 40);
  for (i = 1; i < 11; i++) might += UMIN(5, ch->stance[i] / 40);
  if (IS_SET(ch->newbits, NEW_MASTERY)) might += 10;

  /* 7.5 points per status point */
  might += 15 * ch->pcdata->status / 2;

  /* add a MAX 100 score for class powers */
  switch(ch->class)
  {
    default:
      break;
    case CLASS_WARLOCK:
      classpower += ch->pcdata->powers[SPHERE_SUMMONING] * 3;
      classpower += ch->pcdata->powers[SPHERE_NECROMANCY] * 3;
      classpower += ch->pcdata->powers[SPHERE_ABJURATION] * 4;
      classpower += ch->pcdata->powers[SPHERE_INVOCATION] * 4;
      classpower += ch->pcdata->powers[SPHERE_ENCHANTMENT] * 4;
      classpower += ch->pcdata->powers[SPHERE_DIVINATION] * 3;

      /* evolves */
      if (IS_SET(ch->pcdata->powers[EVOLVE_1], WARLOCK_EVOLVE_CONTINGENCY))
        might += 20;
      if (IS_SET(ch->pcdata->powers[EVOLVE_1], WARLOCK_EVOLVE_TIME))
        might += 5;
      if (IS_SET(ch->pcdata->powers[EVOLVE_1], WARLOCK_EVOLVE_MOUNTAIN))
        might += 10;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_OLDAGE))
        might += 10;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_TIMETRAVEL))
        might += 25;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_HUNTINGSTARS))
        might += 5;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_STITCHES))
        might += 5;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_DISJUNCTION))
        might += 5;

      /* set class level */ 
      classlevel = 100;
      break;
    case CLASS_FAE:
      classpower += ch->pcdata->powers[FAE_PLASMA] * 2;
      classpower += ch->pcdata->powers[FAE_MATTER] * 2;
      classpower += ch->pcdata->powers[FAE_ENERGY] * 2;
      classpower += ch->pcdata->powers[FAE_WILL] * 2;
      classpower += ch->pcdata->powers[DISC_FAE_ARCANE] * 3;
      classpower += ch->pcdata->powers[DISC_FAE_NATURE] * 3;

      /* evolves */
      if (IS_SET(ch->pcdata->powers[EVOLVE_1], FAE_EVOLVE_NATURE))
        might += 10;
      if (IS_SET(ch->pcdata->powers[EVOLVE_1], FAE_EVOLVE_ARCANE))
        might += 10;
      if (IS_SET(ch->pcdata->powers[EVOLVE_1], FAE_EVOLVE_SACRIFICE))
        might += 5;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_BLASTBEAMS))
        might += 5;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_DEFLECT))
        might += 5;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_ABSORB))
        might += 5;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_FREEZE))
        might += 5;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_ACIDBLOOD))
        might += 10;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_PBLAST))
        might += 5;

      /* set class level */
      classlevel = 100;
      break;
    case CLASS_GIANT:
      classpower += ch->pcdata->powers[GIANT_RANK] * 10;
      if (IS_SET(ch->pcdata->powers[GIANT_GIFTS], GGIFT_LEATHERSKIN))
        classpower += 13;
      if (IS_SET(ch->pcdata->powers[GIANT_GIFTS], GGIFT_REVIVAL))
        classpower += 13;
      if (IS_SET(ch->pcdata->powers[GIANT_GIFTS], GGIFT_EARTHPUNCH))
        classpower += 13;
      if (IS_SET(ch->pcdata->powers[GIANT_GIFTS], GGIFT_LONGLEGS))
        classpower += 13;

      /* evolves */
      if (IS_SET(ch->pcdata->powers[EVOLVE_1], GIANT_EVOLVE_FIRE))
        might += 5;
      if (IS_SET(ch->pcdata->powers[EVOLVE_1], GIANT_EVOLVE_WATER))
        might += 20;
      if (IS_SET(ch->pcdata->powers[EVOLVE_1], GIANT_EVOLVE_WARRIOR))
        might += 15;
      if (IS_SET(ch->pcdata->powers[EVOLVE_1], GIANT_EVOLVE_SHAMAN))
        might += 15;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_HASTE))
        might += 5;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_SLOW))
        might += 5;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_DEATHFRENZY))
        might += 15;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_CHOP))
        might += 15;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_IGNITE))
        might += 5;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_WHIRLWIND))
        might += 15;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_EARTHFLUX))
        might += 25;

      /* set class level */ 
      classlevel = 100;
      break;
    case CLASS_SHADOW:
      classpower += ch->pcdata->powers[SHADOW_MARTIAL] * 2;
      if (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_SPIRIT))
        classpower += 20;
      if (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_TPACT))
        classpower += 20;
      if (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_DPACT))
        classpower += 20;

      /* evolves */
      if (IS_SET(ch->pcdata->powers[EVOLVE_1], SHADOW_EVOLVE_TENDRILS))
        might += 10;
      if (IS_SET(ch->pcdata->powers[EVOLVE_1], SHADOW_EVOLVE_CONFUSION))
        might += 10;
      if (IS_SET(ch->pcdata->powers[EVOLVE_1], SHADOW_EVOLVE_ASSASSIN))
        might += 5;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_PLANESHRED))
        might += 5;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_MINDBOOST))
        might += 50;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_MINDBLANK))
        might += 20;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_BLOODTHEFT))
        might += 20;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_RAZORPUNCH))
        might += 10;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_GAROTTE))
        might += 10;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_ACIDTENDRILS))
        might += 10;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_MIRROR))
        might += 25;
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_FROSTBLAST))
        might += 20;

      /* set class level */
      classlevel = 100;
      break;
  }

  /* balance classes - the classlevel should be between 90 and 110 */
  might += UMIN(100, classpower);
  might *= classlevel;
  might /= 100;

  return might;
}

int getRank(CHAR_DATA *ch, int bFull)
{
  int might;

  if (IS_NPC(ch))
    return 0;

  might = getMight(ch);

  if (might < RANK_CADET)
    return ((bFull) ? ((bFull == 1) ? RANK_WANNABE : RANK_CADET) : 0);
  else if (might < RANK_PRIVATE)
    return ((bFull) ? ((bFull == 1) ? RANK_CADET : RANK_PRIVATE) : 1);
  else if (might < RANK_VETERAN)
    return ((bFull) ? ((bFull == 1) ? RANK_PRIVATE : RANK_VETERAN) : 2);
  else if (might < RANK_ADVENTURER)
    return ((bFull) ? ((bFull == 1) ? RANK_VETERAN : RANK_ADVENTURER) : 3);
  else if (might < RANK_HERO)
    return ((bFull) ? ((bFull == 1) ? RANK_ADVENTURER : RANK_HERO) : 4);
  else if (might < RANK_LEGENDARY)
    return ((bFull) ? ((bFull == 1) ? RANK_HERO : RANK_LEGENDARY) : 5);
  else if (might < RANK_MASTER)
    return ((bFull) ? ((bFull == 1) ? RANK_LEGENDARY : RANK_MASTER) : 6);
  else if (might < RANK_CAPTAIN)
    return ((bFull) ? ((bFull == 1) ? RANK_MASTER : RANK_CAPTAIN) : 7);
  else if (might < RANK_GENERAL)
    return ((bFull) ? ((bFull == 1) ? RANK_CAPTAIN : RANK_GENERAL) : 8);
  else if (might < RANK_DUKE)
    return ((bFull) ? ((bFull == 1) ? RANK_GENERAL : RANK_DUKE) : 9);
  else if (might < RANK_BARON)
    return ((bFull) ? ((bFull == 1) ? RANK_DUKE : RANK_BARON) : 10);
  else if (might < RANK_KING)
    return ((bFull) ? ((bFull == 1) ? RANK_BARON : RANK_KING) : 11);
  else if (might < RANK_SUPREME)
    return ((bFull) ? ((bFull == 1) ? RANK_KING : RANK_SUPREME) : 12);
  else if (might < RANK_RULER)
    return ((bFull) ? ((bFull == 1) ? RANK_SUPREME : RANK_RULER) : 13);
  else if (might < RANK_ALMIGHTY)
    return ((bFull) ? ((bFull == 1) ? RANK_RULER : RANK_ALMIGHTY) : 14);

  return ((bFull) ? RANK_ALMIGHTY : 15);
}

void forge_affect(OBJ_DATA *obj, int value)
{
  AFFECT_DATA paf;

  paf.type           = 0;
  paf.duration       = -1;
  paf.location       = APPLY_HITROLL;
  paf.modifier       = value;   
  paf.bitvector      = 0;
  affect_to_obj(obj, &paf);
    
  paf.type           = 0;
  paf.duration       = -1;
  paf.location       = APPLY_DAMROLL;
  paf.modifier       = value;
  paf.bitvector      = 0;
  affect_to_obj(obj, &paf);
}

void dump_last_command()
{
  FILE *fp;
  char buf[MAX_STRING_LENGTH];

  fp = fopen("../txt/crash.txt", "a");
  if (cmd_done)
    fprintf (fp,"Last command typed : %s  (thread count : %d) (command executed without flaws)\n",last_command, thread_count);
  else
    fprintf (fp,"Last command typed : %s  (thread count : %d) (crash happened during this command)\n",last_command, thread_count);
  fclose(fp);

  /*
   * creates a note to the immortals
   */
  sprintf(buf, "It seems we have crashed, the last command typed was\n\n\r");
  strcat(buf, last_command);
  if (cmd_done)
    strcat(buf, "  (This command didn't crash)");
  else
    strcat(buf, "  (This command crashed)");
  strcat(buf, "\n\n\rRegards,\n\n\rThe Crash Code\n\r");
  make_note("Immortal", "Crash Code", "imm", "We Crashed", 7, buf, 0);
} 

void update_revision(CHAR_DATA *ch)
{
  if (IS_NPC(ch)) return;
  if (ch->pcdata->revision == CURRENT_REVISION) return;

  /*
   * We don't end cases with break, since we want the player to be fully updated.
   */
  switch (ch->pcdata->revision)
  {
    /* examples
    case 0:
      if (ch->stance[0] == -1)
        ch->stance[0] = STANCE_NONE;
      ch->pcdata->revision++;
    case 1:
      REMOVE_BIT(ch->extra, 65536);
      ch->pcdata->revision++;
    */

    /* dummy entry */
    case 0:
      break;
  }
}

bool in_fortress(CHAR_DATA *ch)
{
  if (!ch->in_room) return FALSE;
  if (ch->in_room->vnum >= 151 && ch->in_room->vnum <= 170) return TRUE;
  return FALSE;
}

bool in_newbiezone(CHAR_DATA *ch)
{
  if (!ch->in_room || IS_NPC(ch)) return FALSE;
  if (ch->in_room->vnum >= 4895 && ch->in_room->vnum <= 4930) return TRUE;
  return FALSE;
}

bool in_arena(CHAR_DATA *ch)
{
  if (!ch->in_room)
    return FALSE;

  if (ch->in_room->vnum >= 101 && ch->in_room->vnum <= 150)
    return TRUE;

  return FALSE;
}

void increase_total_output(int clenght)
{
  muddata.total_output += clenght;
}

void update_mudinfo()
{
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;
  int i, pcount = 0;
  bool integrity = FALSE;

  /*
   * Each week, the data is stored to a file, and
   * the variable cleared.
   */
  if (muddata.mudinfo[MUDINFO_UPDATED] > 20160)
  {
    write_mudinfo_database();
    for (i = 0; i < (MUDINFO_MAX - 2); i++)
    {
      muddata.mudinfo[i] = 0;
    }
    log_string("Mudinfo database updated.");

    integrity = TRUE;
  }

  /* Increase update count */
  muddata.mudinfo[MUDINFO_UPDATED]++;

  /* Outdate the output data */

  if (muddata.total_output > muddata.mudinfo[MUDINFO_DATA_PEAK])
    muddata.mudinfo[MUDINFO_DATA_PEAK] = muddata.total_output;

  /* The stored data */
  if (muddata.mudinfo[MUDINFO_BYTE] > 1048576) /* 1 megabyte */
  {
    muddata.mudinfo[MUDINFO_MBYTE]++;
    muddata.mudinfo[MUDINFO_BYTE] -= 1048576;
  }
  muddata.mudinfo[MUDINFO_BYTE] += muddata.total_output;
  
  /* The temp data */
  if (muddata.mudinfo[MUDINFO_BYTE_S] > 1048576) /* 1 megabyte */
  {
    muddata.mudinfo[MUDINFO_MBYTE_S]++;
    muddata.mudinfo[MUDINFO_BYTE_S] -= 1048576;
  }
  muddata.mudinfo[MUDINFO_BYTE_S] += muddata.total_output;
 
  /* We clear the counter */
  muddata.total_output = 0;

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected == CON_PLAYING) 
    {
      if (d->character)
      {
        if (d->character->level < 7)
        {
          pcount++;
          if (d->out_compress) muddata.mudinfo[MUDINFO_MCCP_USERS]++;
          else muddata.mudinfo[MUDINFO_OTHER_USERS]++;
          if (d->msp) muddata.mudinfo[MUDINFO_MSP_USERS]++;
          if (d->mxp) muddata.mudinfo[MUDINFO_MXP_USERS]++;
        }
      }
    }
  }  

  if (pcount > muddata.mudinfo[MUDINFO_PEAK_USERS])
    muddata.mudinfo[MUDINFO_PEAK_USERS] = pcount;

  save_muddata();

  if (integrity)
    system_integrity();
}

void recycle_descriptors()
{
  DESCRIPTOR_DATA *dclose;
  ITERATOR *pIter;

  pIter = AllocIterator(descriptor_list);
  while ((dclose = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!dclose->bClosed) continue;
    if (!dclose->bResolved) continue;

    DetachAtIterator(pIter);

    /*
     * Clear out that memory
     */
    free_string(dclose->hostname);
    free_string(dclose->ip_address);
    free(dclose->outbuf);
 
    /*
     * Mccp
     */
    if (dclose->out_compress)
    {
      deflateEnd(dclose->out_compress);
      free(dclose->out_compress_buf);
      free(dclose->out_compress);
    }

    /*     
     * Bye bye mr. Descriptor.
     */
    close(dclose->descriptor);


    /* clear the snoop list */
    FreeList(dclose->snoops);

    /*   
     * And then we recycle
     */
    PushStack(dclose, descriptor_free);
  }
}

int get_next_playerid()
{
  muddata.top_playerid++;
  save_muddata();
  return muddata.top_playerid;
}

/*
 * Writes a string straight to stderr
 */
void log_string2(const char *str, ...)
{
  char buf[MAX_STRING_LENGTH];
  char datef[MAX_INPUT_LENGTH];
  char logfile[MAX_INPUT_LENGTH];
  char *strtime = ctime(&current_time);
  va_list args;
  FILE *fp;

  /* generate logentry */
  va_start(args, str);
  vsprintf(buf, str, args);
  va_end(args);

  /* generate logfile name */
  strtime[7] = '\0';
  strtime[10] = '\0';
  sprintf(logfile, "../log/%s-%s.txt", &strtime[4], &strtime[8]);

  /* try to open logfile */
  if ((fp = fopen(logfile, "a")) == NULL)
  {
    logchan("log_string2: cannot open logfile");
    return;
  }

  /* generate dateformat */
  strtime[19] = '\0';
  sprintf(datef, "%s %s %s", &strtime[4], &strtime[8], &strtime[11]);

  /* output log to file */
  fprintf(fp, "%s :: %s\n", datef, buf);

  /* close logfile */
  fclose(fp);
}

void recycle_dummys()
{
  DUMMY_ARG *dummy;
  ITERATOR *pIter;

  pIter = AllocIterator(dummy_list);
  while ((dummy = (DUMMY_ARG *) NextInList(pIter)) != NULL)
  {
    if (dummy->status == 1) continue;  /* being used */

    DetachAtIterator(pIter);
    PushStack(dummy, dummy_free);
  }
}

bool check_help_soundex(char *argument, CHAR_DATA *ch)
{
  HELP_DATA *pHelp;
  BUFFER *buf;
  char arg[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  char keyword[MAX_INPUT_LENGTH];
  char *str;
  bool found = FALSE;
  int i = 0;

  one_argument(argument, arg);

  if (arg[0] == '\0')
    return FALSE;

  buf = buffer_new(MAX_STRING_LENGTH);

  bprintf(buf, "%s\n\r", get_dystopia_banner("Not Found", 80));
  bprintf(buf, " The helpfile you asked for did not exist, perhaps one of the following entries\n\r");
  bprintf(buf, " are what you are looking for. The list contains all helpfiles that are spelled\n\r");
  bprintf(buf, " roughly the same as or contains the keyword '#G%s#n'\n\n\r", argument);

  pIter = AllocIterator(help_list);
  while ((pHelp = (HELP_DATA *) NextInList(pIter)) != NULL)
  {
    bool match = FALSE;

    if (pHelp->level > ch->level)
      continue;

    if (SoundexMatch(GetSoundexKey(arg), GetSoundexKey(pHelp->name)) > 75)
      match = TRUE;
    else
    {
      str = pHelp->keyword;
      str = one_argument(str, keyword);
      while (keyword[0] != '\0')
      {
        if (!str_prefix(argument, keyword))
        {
          match = TRUE;
          break;
        }
        str = one_argument(str, keyword);
      }
    }

    if (match)
    {
      found = TRUE;
      bprintf(buf, "  %-22.22s%s", pHelp->name, (++i % 3) ? "  " : "\n\r");
    }
  }

  bprintf(buf, "%s\n\r%s\n\r", (i % 3) ? "\n" : "", get_dystopia_banner("", 80));

  if (found)
    send_to_char(buf->data, ch);

  buffer_free(buf);

  return found;
}

/*
 * New system to replace status, called fair fight, it measures the
 * difference between two players, giving them points for their
 * stances, powers, and stats. If they are within each others range,
 * the call will return TRUE, if not FALSE. Call for fair_fight when
 * you need to see if a fight is fair (ie. decapping).
 */
bool fair_fight(CHAR_DATA *ch, CHAR_DATA *victim)
{
  int iAggr, iDef;

  if (IS_NPC(ch) || IS_NPC(victim)) return TRUE;

  /* both players must be pkready */
  if (!IS_SET(ch->extra, EXTRA_PKREADY) || !IS_SET(victim->extra, EXTRA_PKREADY))
    return FALSE;

  /*
   * All the people that shouldn't be fighting anyway
   */
  if (ch == victim) return FALSE;
  if (ch->level != 3 || victim->level != 3) return FALSE;

  iAggr = getMight(ch);
  iDef  = getMight(victim);

  /* This is the lower limit for pk */
  if (iDef < RANK_CADET || iAggr < RANK_CADET) return FALSE;

  if (!str_cmp(ch->pcdata->retaliation[0], victim->name)) return TRUE;
  if (!str_cmp(ch->pcdata->retaliation[1], victim->name)) return TRUE;
  if (!str_cmp(ch->pcdata->last_decap[0], victim->name)) return FALSE;

  /* can always attack an almighty */
  if (iDef >= RANK_ALMIGHTY)
    return TRUE;

  /* 2000 extra hps per paradox counter */
  iAggr += ch->pcdata->mean_paradox_counter * 20;
  iDef += victim->pcdata->mean_paradox_counter * 20;

  /*
   * Checking to see if they are in range. (82.5%)
   */
  if (((33 * iAggr) / 40) > iDef) return FALSE;

  /*
   * They passed the test, FIGHT children.
   */
  return TRUE;
}

void special_decap_message(CHAR_DATA *ch, CHAR_DATA *victim)
{  
  static char * const he_she  [] = { "XX", "he",  "she" };
  static char * const him_her [] = { "XX", "him", "her" };
  static char * const his_her [] = { "XX", "his", "her" };
  ITERATOR *pIter;
  DESCRIPTOR_DATA *d;
  char buf[400]; /* that should be plenty. */
  const char *dmess;
  const char *i;
  char *ptr2;
  char *ptr;
  int size;
      
  size = strlen2(ch->pcdata->decapmessage);
  if (size > 380)
  {
    bug("Bad decapmessage.",0);
    return;
  }
  
  ptr2  = "#C<- #RDeath #C->#n ";
  ptr   = buf;
  dmess = ch->pcdata->decapmessage;
  
  while ((*ptr = *ptr2) != '\0')
    ++ptr, ++ptr2;

  while (*dmess != '\0')
  {
    if ( *dmess != '$' )
    {
      *ptr++ = *dmess++;
      continue; 
    }
    ++dmess;
    switch (*dmess)
    {
      default:  i = ""; break;
      case 'n': i = ch->name; break;
      case 'e': i = he_she  [URANGE(1, ch->sex, 2)]; break;
      case 'm': i = him_her [URANGE(1, ch->sex, 2)]; break;
      case 's': i = his_her [URANGE(1, ch->sex, 2)]; break;
      case 'N': i = victim->name; break;
      case 'S': i = his_her [URANGE(1, victim->sex, 2)]; break;
      case 'M': i = him_her [URANGE(1, victim->sex, 2)]; break;
      case 'E': i = he_she  [URANGE(1, victim->sex, 2)]; break;
    }
    ++dmess;
    /* copying the data into the pointer */
    while ((*ptr = *i) != '\0')
      ++ptr, ++i;
  }
  *ptr++ = '\n';
  *ptr++ = '\r';
     
  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected != CON_PLAYING ) continue;
    write_to_buffer( d, buf, ptr - buf );
  }  
}

void update_polls()
{ 
  POLL_DATA *poll;
  ITERATOR *pIter;

  pIter = AllocIterator(poll_list);
  while ((poll = (POLL_DATA *) NextInList(pIter)) != NULL)  
  {
    if (poll->expire < current_time)
      complete_poll(poll);
  }
}

/* FIX ME - leaks a bit
 * The vote's are not recycled....
 */
void complete_poll(POLL_DATA *poll)
{
  VOTE_DATA *vote;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int i;

  /* clear vote count */
  for (i = 0; i < MAX_VOTE_OPTIONS; i++)
    poll->vcount[i] = 0;

  /* free memory and update vote count */
  pIter = AllocIterator(poll->votes);
  while ((vote = (VOTE_DATA *) NextInList(pIter)) != NULL)
  {
    free_string(vote->phost);
    free_string(vote->pname);
    poll->vcount[vote->choice - 1]++;
  }

  DetachFromList(poll, poll_list);

  sprintf(buf, "Polling on %s completed.\n\n\r", poll->name);
  for (i = 0; i < MAX_VOTE_OPTIONS; i++)
  {
    if (str_cmp(poll->options[i], "<null>"))
    {
      sprintf(buf2, "[%2d votes] %s.\n\r",
        poll->vcount[i], line_indent(poll->options[i], 12, 79));
      strcat(buf, buf2);
    }
    free_string(poll->options[i]);
  }
  strcat(buf, "\n\rRegards, The Polling Code\n\r");

  /* free last fields in structure */
  free_string(poll->name);
  free_string(poll->description);

  save_polls();
  do_info(NULL, "Poll completed, check the announce board for details.");
  make_note("Announce", "Polling Code", "all", "Poll Completed", 7, buf, 0);
}

void force_account_reload(ACCOUNT_DATA *account)
{
  ACCOUNT_DATA *nAccount;
  DESCRIPTOR_DATA *d;
  char buf[MAX_STRING_LENGTH];
  ITERATOR *pIter;

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!d->account) continue;
    if (d->account == account) continue;
    if (str_cmp(d->account->owner, account->owner)) continue;

    if ((nAccount = load_account(account->owner)) == NULL)
    {
      sprintf(buf, "force_account_reload: cannot open account %s", account->owner);
      bug(buf, 0);
      return;
    }
    close_account(d->account);
    d->account = nAccount;
  }
}

/*
 * This function handles the actual removing of the change
 */
bool remove_change(int i)
{
  CHANGE_DATA *change; 
  ITERATOR *pIter;
     
  pIter = AllocIterator(change_list);
  while ((change = (CHANGE_DATA *) NextInList(pIter)) != NULL)
  {
    if (--i > 0) continue;

    /* clearing out the strings */
    free_string(change->imm);
    free_string(change->text);
    free_string(change->date);

    DetachAtIterator(pIter);
    PushStack(change, change_free);

    return TRUE;
  }
  
  return FALSE;
} 

/*
 * should always be called with a size that's 0 mod 4
 */
char *get_dystopia_banner(char *title, int size)
{
  int tSize = collen(title);
  int patternsize, bufferspaces = 2, i, blcks;
  static char buf[200];

  /* just so we can use strcat */
  buf[0] = '\0';

  /* comment this out if you feel like it... */
  if (size % 4)
    log_string("Warning, calling get_dystopia_banner with a weird size");

  /* if we dont want a title, let's fix that quick */
  if (tSize == 0)
  {
    blcks = size / 4;

    strcat(buf, "#0");
    for (i = 0; i < blcks/2; i++)
      strcat(buf, "<>==");
    if (blcks % 2)
      strcat(buf, "====");
    for (i = 0; i < blcks/2; i++)
      strcat(buf, "==<>");
    strcat(buf, "#n");

    return buf;
  }

  /* how much do we spend on patterns, and how much on spaces */
  patternsize = size - (tSize + 2);
  bufferspaces += patternsize % 8;
  patternsize -= patternsize % 8;
  blcks = patternsize / 4;

  if (blcks < 1)
  {
    strcat(buf, "#0<>== #G");
    strcat(buf, title);
    strcat(buf, " #0==<>#n");  
  }
  else
  {
    /* first add patterns */
    strcat(buf, "#0");
    for (i = 0; i < blcks/2; i++)
      strcat(buf, "<>==");

    /* add spaces, title, spaces */
    for (i = 0; i < bufferspaces/2; i++)
      strcat(buf, " ");
    strcat(buf, "#G");
    strcat(buf, title);
    strcat(buf, "#0");
    if (bufferspaces % 2)
      strcat(buf, " ");
    for (i = 0; i < bufferspaces/2; i++)
      strcat(buf, " ");

    /* then add the rest of the pattern */
    for (i = 0; i < blcks/2; i++)
      strcat(buf, "==<>");
    strcat(buf, "#n");
  }

  return buf;
}

int calc_ratio(int a, int b)
{
  int ratio;

  if (b == 0 && a > 0) ratio = 100;
  else if ((a + b) != 0) ratio = (100*a)/(a+b);
  else ratio = 0;
  return ratio;
}

char *strip_ansi(char *str)
{
  static char buf[MAX_STRING_LENGTH];
  char *ptr;

  buf[0] = '\0';
  ptr = buf;

  while (*str != '\0')
  {
    if (*str != '#')
      *ptr++ = *str++;
    else
    {
      switch(*(++str))
      {
        default:
          *ptr++ = '#';
          *ptr++ = *str++;
          break;
        case '\0':
          break;
        case '8':
        case '0':
        case 'r':
        case 'R':
        case 'g':
        case 'G':
        case 'o':
        case 'y':
        case 'l':
        case 'L':
        case 'p':
        case 'P':
        case 'c':
        case 'C':
        case '7':
        case '9':
          str++;
          break;
      }
    }
  }
  *ptr = '\0';

  return buf;
}

char *line_indent(char *text, int wBegin, int wMax)
{
  static char buf[MAX_STRING_LENGTH] = { '\0' };
  char tempbuf[MAX_STRING_LENGTH];
  int i = 0, ptr = 0, j = 0;

  /* skip past leading spaces */
  while (*text != '\0' && *text == ' ')
    text++;

  /* remove all linebreaks and double spaces */
  while (*text != '\0')
  {
    switch(*text)
    {
      default:
        tempbuf[i++] = *text++;
        break;
      case '\r':
      case '\n':
        if (*(++text) != ' ' && *text != '\n' && *text != '\r')
          tempbuf[i++] = ' ';
        break;
      case ' ':
        if (*(text + 1) == ' ')
          text++;
        else
          tempbuf[i++] = *text++;
        break;
    }
  }
  tempbuf[i] = '\0';

  /* add linebreaks when needed */
  do 
  {
    char *s;

    /* using string_restrict allows for better color aligning */
    s = string_restrict(&tempbuf[ptr], wMax - wBegin);
    i = strlen(s);

    /* find first space (backwards) */
    if (tempbuf[ptr + i] != '\0')
    {
      while (*(s + i) != ' ' && i > 0)
        i--;
    }

    /* calculate how much we want to copy */
    if (i != 0)
    {
      *(s + i) = '\0';
      ptr += i;
    }
    else
      ptr += strlen(s);

    /* copy the string */
    while ((buf[j] = *s++) != '\0')
      j++;

    /* break if we must */
    if (tempbuf[ptr++] == '\0')
      break;

    /* aftermatch */
    buf[j++] = '\n';
    buf[j++] = '\r'; /* stupid windows telnet clients */
    for (i = wBegin; i > 0; i--)
      buf[j++] = ' ';

  } while(1);

  return buf;
}

char *get_exits(CHAR_DATA *ch)
{
  static char buf[MAX_STRING_LENGTH];
  EXIT_DATA *pexit;
  EVENT_DATA *event;
  ITERATOR *pIter;
  bool found;
  int door;
  int wlck_doors[4];
  int tracked_dir = -1;

  /* reset warlock doors */
  for (door = 0; door < 4; door++)
    wlck_doors[door] = 0;

  buf[0] = '\0';

  if (!check_blind(ch)) return "";
 
  sprintf(buf, "#0[#GExits#9:#C");

  pIter = AllocIterator(ch->in_room->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == EVENT_ROOM_MISDIRECT)
    {
      char temp[MAX_INPUT_LENGTH];
      int newdir;

      one_argument(event->argument, temp);
      newdir = atoi(temp);

      wlck_doors[newdir] = 1;
    }
  }

  if (IS_CLASS(ch, CLASS_FAE) && (event = event_isset_mobile(ch, EVENT_PLAYER_BLOODTASTE)) != NULL)
  {
    char *tracked = event->argument;
    char *ptr;
    char name[MAX_STRING_LENGTH];
    char direction[MAX_STRING_LENGTH];
    int age = 0;

    pIter = AllocIterator(ch->in_room->events);
    while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
    {
      /* check all tracks in the room */
      if (event->type == EVENT_ROOM_TRACKS)
      {
        ptr = one_argument(event->argument, name);
        one_argument(ptr, direction);

        /* freshest tracks only */
        if (age >= event_pulses_left(event))
          continue;

        /* only track the tracked player */
        if (str_cmp(name, tracked))
          continue;

        age = event_pulses_left(event);
        if (!str_cmp(direction, "north"))
          tracked_dir = DIR_NORTH;
        else if (!str_cmp(direction, "south"))
          tracked_dir = DIR_SOUTH;
        else if (!str_cmp(direction, "west"))
          tracked_dir = DIR_WEST;
        else if (!str_cmp(direction, "east"))
          tracked_dir = DIR_EAST;
        else if (!str_cmp(direction, "up"))
          tracked_dir = DIR_UP;
        else if (!str_cmp(direction, "down"))
          tracked_dir = DIR_DOWN;
      }
    }
  }

  found = FALSE;
  for (door = 0; door <= 5; door++)
  {
    if (((pexit = ch->in_room->exit[door]) != NULL && pexit->to_room != NULL) ||
         (door < 4 && wlck_doors[door] != 0))
    {
      found = TRUE;
      if (pexit && IS_SET(pexit->exit_info, EX_CLOSED))
      {
        strcat(buf, " #0(");

        /* fae bloodtracking */
        if (tracked_dir == door)
          strcat(buf, "#R");
        else
          strcat(buf, "#C");

        strcat(buf, "<SEND href=\"open ");
        strcat(buf, dir_name[door]);
        strcat(buf, "\">");
        strcat(buf, dir_name[door]);
        strcat(buf, "</SEND>#0)#C");
      }
      else
      {  
        strcat(buf, " <SEND href=\"");
        strcat(buf, dir_name[door]);
        strcat(buf, "\">");

        /* fae bloodtracking */
        if (tracked_dir == door)
          strcat(buf, "#R");

        strcat(buf, dir_name[door]);

        /* fae bloodtracking */
        if (tracked_dir == door)
          strcat(buf, "#C");

        strcat(buf, "</SEND>");
      }
    }
  }
  if (!found)
    strcat(buf, " none");
  strcat(buf, "#0]#n\n\r");

  return buf;
}

bool examine_room(RID *pRoom, RID *tRoom, AREA_DATA *pArea, int steps)
{
  int door;

  /* been here before, out of the area or can we get here faster */
  if (pRoom->area != pArea) return FALSE;
  if (pRoom->visited) return FALSE;
  if (pRoom->steps < steps) return FALSE;

  /* Have we found the room we are searching for */
  if (pRoom == tRoom)
    return TRUE;

  /* mark the room so we know we have been here */
  pRoom->visited = TRUE;

  /* Depth first traversel of all exits */
  for (door = 0; door < 6; door++)
  {
    if (pRoom->exit[door] == NULL) continue;
    if (pRoom->exit[door]->to_room == NULL) continue;

    /* assume we are walking the right way */
    pRoom->exit[door]->color = TRUE;

    /* recursive return */
    if (examine_room(pRoom->exit[door]->to_room, tRoom, pArea, steps + 1))
      return TRUE;

    /* it seems we did not walk the right way */
    pRoom->exit[door]->color = FALSE;
  }
  return FALSE;
}

HEAP *init_heap(RID *root)
{
  AREA_DATA *pArea;
  RID *pRoom;
  HEAP *heap;
  int i, size, vnum;

  if ((pArea = root->area) == NULL) return NULL;
  size = pArea->uvnum - pArea->lvnum;

  heap = calloc(1, sizeof(*heap));
  heap->knude = calloc(2 * (size + 1), sizeof(ROOM_INDEX_DATA *));
  heap->size = 2 * (size + 1);

  /* we want the root at the beginning */
  heap->knude[1]             = root;
  heap->knude[1]->steps      = 0;
  heap->knude[1]->heap_index = 1;

  /* initializing the rest of the heap */
  for (i = 2, vnum = pArea->lvnum; vnum < pArea->uvnum; vnum++)
  {
    if ((pRoom = get_room_index(vnum)))
    {
      if (pRoom == root) continue;
      heap->knude[i]             = pRoom;
      heap->knude[i]->steps      = 2 * heap->size;
      heap->knude[i]->heap_index = i;
      i++;
    }
  }

  heap->iVertice = i - 1;

  /* setting the rest to NULL */
  for (; i < heap->size; i++)
    heap->knude[i] = NULL;

  return heap;
}

/*
 * Finds the smallest element and returns it after
 * making sure the heap is in perfect order after
 * the removal of the vertice with the smallest
 * element.
 */
RID *heap_getMinElement(HEAP *heap)
{
  RID *tRoom;
  RID *pRoom;
  bool done = FALSE;
  int i = 1;

  /* this is the element we wish to return */
  pRoom = heap->knude[1];

  /* We move the last vertice to the front */
  heap->knude[1] = heap->knude[heap->iVertice];
  heap->knude[1]->heap_index = 1;

  /* Decrease the size of the heap and remove the last entry */
  heap->knude[heap->iVertice] = NULL;
  heap->iVertice--;

  /* Swap places till it fits */
  while(!done)
  {
    if (heap->knude[i] == NULL)
      done = TRUE;
    else if (heap->knude[2*i] == NULL)
      done = TRUE;
    else if (heap->knude[2*i+1] == NULL)
    {
      if (heap->knude[i]->steps > heap->knude[2*i]->steps)
      {
        tRoom                        = heap->knude[i];
        heap->knude[i]               = heap->knude[2*i];
        heap->knude[i]->heap_index   = i;
        heap->knude[2*i]             = tRoom;
        heap->knude[2*i]->heap_index = 2*i;
        i = 2*i;
      }
      done = TRUE;
    }
    else if (heap->knude[i]->steps <= heap->knude[2*i]->steps &&
        heap->knude[i]->steps <= heap->knude[2*i+1]->steps)
      done = TRUE;
    else if (heap->knude[2*i]->steps <= heap->knude[2*i+1]->steps)
    {
      tRoom                        = heap->knude[i];
      heap->knude[i]               = heap->knude[2*i];
      heap->knude[i]->heap_index   = i;
      heap->knude[2*i]             = tRoom;
      heap->knude[2*i]->heap_index = 2*i;
      i = 2*i;
    }
    else
    {
      tRoom                          = heap->knude[i];
      heap->knude[i]                 = heap->knude[2*i+1];
      heap->knude[i]->heap_index     = i;
      heap->knude[2*i+1]             = tRoom;
      heap->knude[2*i+1]->heap_index = 2*i+1;
      i = 2*i+1;
    }
  }

  /* return the element */
  return pRoom;
}

void dijkstra(RID *chRoom, RID *victRoom)
{
  RID *pRoom;
  RID *tRoom;
  RID *xRoom;
  HEAP *heap;
  int door, x;
  bool stop;

  /* allocate a new heap */
  heap = init_heap(chRoom);

  /* find shortest amounts of steps to each room */
  while (heap->iVertice)
  {
    if ((pRoom = heap_getMinElement(heap)) == NULL)
    {
      bug("Dijstra: Getting NULL room", 0);
      return;
    }
    if (pRoom == victRoom)
      gFound = TRUE;

    /* update all exits */
    for (door = 0; door < 6; door++)
    {
      if (pRoom->exit[door] == NULL) continue;   
      if (pRoom->exit[door]->to_room == NULL) continue;

      /* update step count, and swap in the heap */
      if (pRoom->exit[door]->to_room->steps > pRoom->steps + 1)
      {
        xRoom = pRoom->exit[door]->to_room;
        xRoom->steps = pRoom->steps + 1;
        stop = FALSE;
        while ((x = xRoom->heap_index) != 1 && !stop)
        {
          if (heap->knude[x/2]->steps > xRoom->steps)
          {
            tRoom                       = heap->knude[x/2];
            heap->knude[x/2]            = xRoom;
            xRoom->heap_index           = xRoom->heap_index/2;
            heap->knude[x]              = tRoom;
            heap->knude[x]->heap_index  = x;
          }
          else stop = TRUE;
        }
      }
    }
  }

  /* free the heap */
  free(heap->knude);
  free(heap);
}

char *pathfind(CHAR_DATA *ch, CHAR_DATA *victim)
{
  int const exit_names [] = { 'n', 'e', 's', 'w', 'u', 'd' };
  RID *pRoom;
  AREA_DATA *pArea;
  static char path[MAX_STRING_LENGTH]; /* should be plenty. */
  int iPath = 0, vnum, door;
  bool found;

  if (!ch->in_room || !victim->in_room) return NULL;
  if (ch->in_room == victim->in_room) return NULL;
  if ((pArea = ch->in_room->area) != victim->in_room->area) return NULL;

  /* initialize all rooms in the area */
  for (vnum = pArea->lvnum; vnum < pArea->uvnum; vnum++)
  {
    if ((pRoom = get_room_index(vnum)))
    {
      pRoom->visited = FALSE;
      for (door = 0; door < 6; door++)
      {
        if (pRoom->exit[door] == NULL) continue;
        pRoom->exit[door]->color = FALSE;
      }
    }
  }

  /* initialize variables */
  pRoom = ch->in_room;
  gFound = FALSE;

  /* In the first run, we only count steps, no coloring is done */
  dijkstra(pRoom, victim->in_room);

  /* If the target room was never found, we return NULL */
  if (!gFound) return NULL;

  /* in the second run, we color the shortest path using the step counts */
  if (!examine_room(pRoom, victim->in_room, pArea, 0))
    return NULL;

  /* then we follow the trace */
  while (pRoom != victim->in_room)
  {
    found = FALSE;
    for (door = 0; door < 6 && !found; door++)  
    {
      if (pRoom->exit[door] == NULL) continue;
      if (pRoom->exit[door]->to_room == NULL) continue;
      if (!pRoom->exit[door]->color) continue;

      pRoom->exit[door]->color = FALSE;
      found = TRUE;
      path[iPath] = exit_names[door];
      iPath++;
      pRoom = pRoom->exit[door]->to_room;
    }
    if (!found)
    {
      bug("Pathfind: Fatal Error in %d.", pRoom->vnum); 
      return NULL;
    }
  }  

  path[iPath] = '\0';
  return path;
}

/*
 * Takes the expanded, unparsed string that makes up an alias,
 * and the set of arguments given to this alias, which we must
 * parse into the expanded string. It will also set a counter
 * on the socket, preventing aliases from being used for the
 * next X input lines, where X is the amount of lines this alias
 * expands to. Anything beyond the first line in the alias is
 * then stored in output_rest and only the first line is returned.
 *
 * It is assumed that output_rest has a size of MAX_STRING_LENGTH for
 * bound checking. The code might be a bit dirty, but it gets
 * the job done, and if I can live with it, so can you :)
 */
char *parse_alias(char *pString, char *argument, char *output_rest, DESCRIPTOR_DATA *dsock)
{
  static char buf[MAX_STRING_LENGTH];
  char argX[MAX_STRING_LENGTH];
  char *pStr, *skipper;
  bool first_line = TRUE;
  int i, size_1 = 0, size_2 = 0;

  buf[0] = '\0';
  pStr = buf;

  while (*pString != '\0')
  {
    if (*pString == '%' || *pString == '@')
    {
      if (isdigit(*(++pString)) && (i = *pString - '0') > 0)
      {
	/* find the Xth argument */
	if (*(pString-1) == '%')
	{
          skipper = get_token(argument, argX);
          while (--i > 0)
            skipper = get_token(skipper, argX);
	}
        else
	{
	  if (i == 1)
	    strcpy(argX, argument);
          else
	  {
	    i--;
            skipper = get_token(argument, argX);
            while (--i > 0)
              skipper = get_token(skipper, argX);

	    /* we want the tail, remember */
	    strcpy(argX, skipper);
	  }
	}
	if (first_line)
	{
	  /* bounds checking */
          *pStr = '\0';
	  size_1 += strlen(argX);
          if (size_1 >= MAX_STRING_LENGTH - 1)
          {
            output_rest = "";
            return ("\n\r");
          }
	  i = 0;
          while ((*pStr++ = argX[i++]) != '\0') ;
	  pStr--;
	}
	else
	{
	  /* bounds checking */
          *output_rest = '\0';
	  size_2 += strlen(argX);
          if (size_2 >= MAX_STRING_LENGTH - 1)
          {
            output_rest = "";
            return buf; /* we are so nice */
          }
	  i = 0;
          while ((*output_rest++ = argX[i++]) != '\0') ;
	  output_rest--;
	}
        pString++;
      }
      else if (first_line)
      {
        if (++size_1 < MAX_STRING_LENGTH - 1) /* safe bounds checking */
  	  *pStr++ = *(pString-1);
      }
      else
      {
        if (++size_2 < MAX_STRING_LENGTH - 1) /* safe bounds checking */
          *output_rest++ = *(pString-1);
      }
    }
    else if (*pString == '\n' || *pString == '\r')
    {
      if (first_line)
      {
	/* scan past this initial linebreak, we will add our own */
	while (*pString == '\n' || *pString == '\r') pString++;

	/* add a terminator to the first line and stop adding to this line */
	*pStr = '\0';
	first_line = FALSE;
      }
      else
      {
	/* scan past this initial linebreak, we will add our own */
	while (*pString == '\n' || *pString == '\r') pString++;

	size_2 += 2;
	if (size_2 < MAX_STRING_LENGTH - 1)
	{
          *output_rest++ = '\n';
	  *output_rest++ = '\r';
	}
      }
      dsock->alias_block++;
    }
    else if (first_line)
    {
      if (++size_1 < MAX_STRING_LENGTH - 1) /* safe bounds checking */
        *pStr++ = *pString++;
    }
    else
    {
      if (++size_2 < MAX_STRING_LENGTH - 1) /* safe bounds checking */
        *output_rest++ = *pString++;
    }
  }
  *output_rest = '\0';

  return buf;
}

/*
 * takes something like "Say hello:get %1 %2:snicker %3" and
 * changes it to something like this
 *
 * Say hello\nget %1 %2\nsnicker %3\n
 */
char *create_alias_string(char *arg)
{
  static char buf[MAX_STRING_LENGTH];
  int i = 0;

  while (*arg != '\0')
  {
    switch(*arg)
    {
      default:
        buf[i++] = *arg++;
        break;
      case ':':
        while (i > 0 && buf[i - 1] == ' ')
          i--;

        buf[i++] = '\n';
        arg++;

        while (*arg != '\0' && *arg == ' ')
          arg++;

        break;
    }
  }

  buf[i++] = '\n';
  buf[i] = '\0';

  return buf;
}

bool check_alias(char *incom, DESCRIPTOR_DATA *dsock)
{
  ALIAS_DATA *alias;
  CHAR_DATA *dMob;
  ITERATOR *pIter;
  char first_word[MAX_STRING_LENGTH];
  char output_rest[MAX_STRING_LENGTH];
  char *argument, *output_first;

  /* only live playing players, thanks */
  if (dsock->connected != CON_PLAYING)
    return FALSE;

  /* let's see what we can do with this line of input */
  argument = one_argument(incom, first_word);

  /* if there are no player, we don't do anything at all */
  if ((dMob = dsock->character) == NULL)
    return FALSE;

  /* check to see if we want to parse the input */
  pIter = AllocIterator(dMob->pcdata->aliases);
  while ((alias = (ALIAS_DATA *) NextInList(pIter)) != NULL)
  {
    /* does this alias match ? */
    if (!str_cmp(first_word, alias->name))
    {
      /* let's parse this, and put the rest in the leftover department */
      output_first = parse_alias(alias->expand_string, argument, output_rest, dsock);

      /* copy this rounds output */
      strcpy(dsock->incomm, output_first);

      /* move the rest into a waiting position */
      if ((strlen(output_rest) + strlen(dsock->inbuf)) > MAX_STRING_LENGTH - 2)
	write_to_buffer(dsock, "\n\r!!!ALIAS OVERFLOW!!!\n\n\r", 0);
      else
      {
	char buf[MAX_STRING_LENGTH];

	sprintf(buf, "%s%s", output_rest, dsock->inbuf);
	strcpy(dsock->inbuf, buf);
      }
      return TRUE;
    }
  }

  /* we didn't find any matching aliases */
  return FALSE;
}

TOP10_ENTRY *alloc_top10entry(CHAR_DATA *ch)
{
  TOP10_ENTRY *entry;

  entry =  malloc(sizeof(*entry));

  if (ch == NULL || IS_NPC(ch))
  {
    entry->name         =  NULL;
    entry->pkscore      =  0;
    entry->pkills       =  0;
    entry->pdeaths      =  0;
    entry->hours        =  0;
  }
  else
  {
    entry->name         =  str_dup(ch->name);
    entry->pkscore      =  get_ratio(ch);
    entry->pkills       =  ch->pkill;
    entry->pdeaths      =  ch->pdeath;
    entry->hours        =  (ch->played + (int) (current_time - ch->logon)) / 3600;
  }

  return entry;
}

void free_top10entry(TOP10_ENTRY *entry)
{
  free_string(entry->name);
  free(entry);
}

ALIAS_DATA *alloc_alias()
{
  ALIAS_DATA *alias;

  if ((alias = (ALIAS_DATA *) PopStack(alias_free)) == NULL)
    alias = malloc(sizeof(*alias));

  alias->name = NULL;
  alias->expand_string = NULL;

  return alias;
}

void alias_from_player(CHAR_DATA *ch, ALIAS_DATA *alias)
{
  DetachFromList(alias, ch->pcdata->aliases);

  free_string(alias->name);
  free_string(alias->expand_string);

  PushStack(alias, alias_free);
}

void system_integrity()
{
  struct arti_type *artifact;
  DESCRIPTOR_DATA *d;
  KINGDOM_DATA *kingdom;
  TOP10_ENTRY *top10entry;
  ITERATOR *pIter, *pIter2;
  MEMBER_DATA *member;
  DIR *directory;
  struct dirent *entry;
  ACCOUNT_DATA *account;
  char buf[MAX_STRING_LENGTH];
  const int days = 28; /* 4 weeks inactivity */
  bool deleted, copyover = FALSE;
  int j;

  /* make a log entry */
  log_string2("system integrity check initiated.");

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    write_to_descriptor(d, "\n\r            [***] ATTENTION PLEASE [***]\n\r", 0);
    write_to_descriptor(d, " Calim's Cradle is doing a system integrity check,\n\r", 0);
    write_to_descriptor(d, " which will cause the MUD to stop for a few seconds\n\r", 0);
    write_to_descriptor(d, " while the integrity of kingdoms, accounts, etc is\n\r", 0);
    write_to_descriptor(d, " verified.   Please do not try to reconnect.....\n\n\r", 0);
  }

  /* reset class counts */
  for (j = 0; j < MAX_CLASS; j++)
    muddata.class_count[j] = 0;

  /* move this week's pk data to last weeks */
  for (j = 0; j < 3; j++)
  {
    muddata.pk_count_last[j] = muddata.pk_count_now[j];
    muddata.pk_count_now[j] = 0;
  }

  /* First we check all accounts, making sure they have
   * connected at least once during the last 4 weeks. If not
   * we will move them into a deleted directory, and remove
   * the players whois data file.
   */
  directory = opendir("../accounts/");
  for (entry = readdir(directory); entry; entry = readdir(directory))
  {
    if (!str_cmp(entry->d_name, ".") || !str_cmp(entry->d_name, "..") ||
        !str_cmp(entry->d_name, "whois") || !str_cmp(entry->d_name, "deleted"))
      continue;

    if ((account = load_account(entry->d_name)) != NULL)
    {
      deleted = FALSE;
      if (account->lastlogged <= current_time - (days * 24L * 3600L) && !IS_SET(account->flags, ACCOUNT_FLAG_VACATION))
      {
        char strsave[MAX_STRING_LENGTH];
        char *ptr = account->players;
        int i = 0;

        deleted = TRUE;
        ptr = one_argument(ptr, buf);
        while (ptr[0] != '\0')
        {
          if ((i++ % 4) == 0)
          {
            buf[0] = UPPER(buf[0]);
            sprintf(strsave, "rm ../accounts/whois/%s.whois", buf);
            system(strsave);
          }
          ptr = one_argument(ptr, buf);
        }
      }
      else
      {
        char *ptr = account->players;
        int i = 0;

        ptr = one_argument(ptr, buf);
        while (ptr[0] != '\0')
        {
          if ((i++ % 4) == 1)
          {
            for (j = 0; class_table[j].class_name[0] != '\0' && j < MAX_CLASS; j++)
            {
              if (!str_cmp(class_table[j].class_name, buf))
              {
                muddata.class_count[j]++;
                break;
              }
            }
          }
          ptr = one_argument(ptr, buf);
        }
      }

      close_account(account);
      if (deleted)
      {
        sprintf(buf, "mv ../accounts/%s ../accounts/deleted/", entry->d_name);
        system(buf);
      }
    }
  }
  closedir(directory);

  /* Secondly run through all kingdoms and locate deleted players,
   * and make sure they are removed from the kingdoms. Also remove
   * any invitations that no longer apply.
   */
  pIter = AllocIterator(kingdom_list);
  while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
  {
    int count = 0;

    /* if less than 7 hours active this week */
    if (kingdom->king_active < (7 * 60 * 15))
    {
      if (IS_SET(kingdom->flags, KINGFLAG_INACTIVE))
      {
        MEMBER_DATA *winner = NULL;
        int highestrank = 0;
        int highestmight = 0;

        pIter2 = AllocIterator(kingdom->members);
        while ((member = (MEMBER_DATA *) NextInList(pIter2)) != NULL)
        {
          if (!str_cmp(member->name, kingdom->leader)) continue;

          if (member->level >= highestrank)
          {
            if (member->level == highestrank && member->might <= highestmight)
              continue;

            highestmight = member->might;
            highestrank = member->level;
            winner = member;
          }
        }

        if (winner != NULL)
        {
          free_string(kingdom->leader);
          kingdom->leader = str_dup(winner->name);
        }
      }
      else
      {
        SET_BIT(kingdom->flags, KINGFLAG_INACTIVE);
        sprintf(buf, "This is a kingdom warning.\n\n\r"
                     "You have been online less than 7 hours the last 7 days,\n\r"
                     "and you will lose your kingship if you do not fill\n\r"
                     "your activity quota for the next week.\n\n\r"
                     "Kingdom Code.\n\r");

        make_note("Personal", "Kingdom Code", kingdom->leader, "Warning", 7, buf, 0);
      }
    }
    else
    {
      REMOVE_BIT(kingdom->flags, KINGFLAG_INACTIVE);
    }
    kingdom->king_active = 0;

    pIter2 = AllocIterator(kingdom->members);
    while ((member = (MEMBER_DATA *) NextInList(pIter2)) != NULL)
    {
      if (!char_exists(member->name))
      {
        DetachAtIterator(pIter2);
        free_member(member);
      }

      count++;
    }

    pIter2 = AllocIterator(kingdom->invited);
    while ((member = (MEMBER_DATA *) NextInList(pIter2)) != NULL)
    {
      if (!char_exists(member->name))
      {
        DetachAtIterator(pIter2);
        free_member(member);
      }
    }

    if (count <= 2 || !char_exists(kingdom->leader) || (count < 5 && IS_SET(kingdom->flags, KINGFLAG_WARNED)))
    {
      copyover = TRUE;
      delete_kingdom(kingdom);
    }
    else if (count < 5)
    {
      char message[MAX_STRING_LENGTH];

      strcpy(message, "You kingdom has less than 5 members.\n\n\r"
                      "You have one week to get your kingdom membership up,\n\r"
                      "and should you fail to meet this requirement, your kingdom\n\r"
                      "will be deleted.\n\n\r"
                      "This is an automated kingdom message...\n\r");

      make_note("Personal", "Kingdom Code", kingdom->shortname, "Warning", 7, message, 0);

      SET_BIT(kingdom->flags, KINGFLAG_WARNED);
    }
    else
    {
      int cost = get_kingdom_upkeep(kingdom);

      REMOVE_BIT(kingdom->flags, KINGFLAG_WARNED);

      if (kingdom->treasury < cost)
      {
        char message[MAX_STRING_LENGTH];

        strcpy(message, "You kingdom cannot pay the upkeep for your kingdom.\n\n\r"
                        "All retainers have gone on strike and all kingdom equipment is\n\r"
                        "prone to failure due to lack of repairs.\n\n\r"
                        "This is an automated kingdom message...\n\r");

        make_note("Personal", "Kingdom Code", kingdom->shortname, "Warning", 7, message, 0);

        SET_BIT(kingdom->flags, KINGFLAG_STRIKE);
      }
      else if (IS_SET(kingdom->flags, KINGFLAG_STRIKE))
      {
        char message[MAX_STRING_LENGTH];

        strcpy(message, "The kingdom strike is over, all retainers have gone back to\n\r"
                        "work, and repairs have been made to structure damages in the castle.\n\n\r"
                        "This is an automated kingdom message...\n\r");

        make_note("Personal", "Kingdom Code", kingdom->shortname, "Strike Over", 7, message, 0);

        REMOVE_BIT(kingdom->flags, KINGFLAG_STRIKE);
        kingdom->treasury -= cost;
      }
      else
      {
        kingdom->treasury -= cost;
      }
    }
  }

  /* now save all kingdoms */
  save_kingdoms();

  /* check the leaderboard and check the top10 board */
  if (!char_exists(leader_board.name[LEADER_PK]))
  {
    free_string(leader_board.name[LEADER_PK]);
    leader_board.name[LEADER_PK] = str_dup("noone");
    leader_board.number[LEADER_PK] = 0;
  }
  if (!char_exists(leader_board.name[LEADER_TIME]))
  {   
    free_string(leader_board.name[LEADER_TIME]);
    leader_board.name[LEADER_TIME] = str_dup("noone");
    leader_board.number[LEADER_TIME] = 0;
  }
  if (!char_exists(leader_board.name[LEADER_QUEST]))
  {   
    free_string(leader_board.name[LEADER_QUEST]);
    leader_board.name[LEADER_QUEST] = str_dup("noone");
    leader_board.number[LEADER_QUEST] = 0;
  }
  if (!char_exists(leader_board.name[LEADER_MOBKILLS]))
  {   
    free_string(leader_board.name[LEADER_MOBKILLS]);
    leader_board.name[LEADER_MOBKILLS] = str_dup("noone");
    leader_board.number[LEADER_MOBKILLS] = 0;
  }
  if (!char_exists(leader_board.name[LEADER_PKSCORE]))
  {   
    free_string(leader_board.name[LEADER_PKSCORE]);
    leader_board.name[LEADER_PKSCORE] = str_dup("noone");
    leader_board.number[LEADER_PKSCORE] = 0;
  }
  if (!char_exists(leader_board.name[LEADER_STATUS]))
  {
    free_string(leader_board.name[LEADER_STATUS]);
    leader_board.name[LEADER_STATUS] = str_dup("noone");
    leader_board.number[LEADER_STATUS] = 0;
  }
  if (!char_exists(leader_board.name[LEADER_ARENA]))
  {
    free_string(leader_board.name[LEADER_ARENA]);
    leader_board.name[LEADER_ARENA] = str_dup("noone");
    leader_board.number[LEADER_ARENA] = 0;
  }
  if (!char_exists(leader_board.name[LEADER_ACTIVE]))
  {
    free_string(leader_board.name[LEADER_ACTIVE]);
    leader_board.name[LEADER_ACTIVE] = str_dup("noone");
    leader_board.number[LEADER_ACTIVE] = 0;
  }
  else
  {
    /* FIX ME - load the char and update the leaderboards
     * activity counter for this player - to avoid idle players
     * taking this leader spot. You can use load_char_obj_finger()
     */
  }

  pIter = AllocIterator(top10_list);
  while ((top10entry = (TOP10_ENTRY *) NextInList(pIter)) != NULL)
  {
    if (!char_exists(top10entry->name))
    {
      free_string(top10entry->name);
      top10entry->name    = str_dup("noone");
      top10entry->pkscore = 0;
      top10entry->pkills  = 0;
      top10entry->pdeaths = 0;
      top10entry->hours   = 0;
    }
  }

  save_leaderboard();
  save_top10();

  /* check the artifact table */
  pIter = AllocIterator(artifact_table);
  while ((artifact = (struct arti_type *) NextInList(pIter)) != NULL)
  {
    if (str_cmp(artifact->owner, "noone") && (!char_exists(artifact->owner) || artifact->active < MIN_ARTIFACT_ACTIVE))
    {
      free_string(artifact->owner);
      artifact->owner = str_dup("noone");
    }

    /* reset active counter */
    artifact->active = 0;
  }
  save_artifact_table();

  /* check the archmage listings */
  for (j = PATH_NECROMANCY; j <= PATH_SUMMONING; j++)
  {
    /* deleted player or less than 2 hours played last week (900 per hour) */
    if (!char_exists(archmage_list[j].player) || archmage_list[j].active < 1800)
    {
      free_string(archmage_list[j].player);
      archmage_list[j].player = str_dup("noone");
    }

    /* reset active counter */
    archmage_list[j].active = 0;
  }
  save_archmages();

  if (copyover && pre_reboot_actions(TRUE))
  {
    extern int port, control;
    char buf2[100];

    pIter = AllocIterator(descriptor_list);
    while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
      write_to_descriptor_2(d->descriptor, "\n\r <*> A copyover is required - have patience  <*>\n\r", 0);

    do_asave(NULL, "world");

    sprintf(buf, "%d", port);
    sprintf(buf2, "%d", control);
    execl(EXE_FILE, "dystopia", buf, "copyover", buf2, (char *) NULL);

    /* damn */
    log_string("Copyover Failed!");
  }

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    write_to_buffer(d, " [DONE] Calim's Cradle integrity check completed.\n\r", 0);
  }

  log_string2("system integrity check completed.");
}

ACCOUNT_DATA *alloc_account()
{
  static ACCOUNT_DATA clear_account;
  ACCOUNT_DATA *account;
  int i;

  if ((account = (ACCOUNT_DATA *) PopStack(account_free)) == NULL)
  {
    account = malloc(sizeof(*account));
  }

  *account               =  clear_account;
  account->owner         =  str_dup("");
  account->password      =  str_dup("");
  account->new_password  =  str_dup("");
  account->players       =  str_dup("");
  account->reference     =  str_dup("");
  account->notes         =  str_dup("");
  account->level         =  PLAYER_ACCOUNT;
  account->denied        =  (time_t) 0;
  account->lastlogged    =  (time_t) 0;
  account->created       =  current_time;
  account->board         =  &boards[DEFAULT_BOARD];
  account->goldcrowns    =  0;
  account->goldtotal     =  0;
  account->timezone      =  0;
  account->p_count       =  0;
  account->flags         =  0;
  account->reimb_points  =  0;
  account->max_might     =  0;
  account->popup         =  0;

  for (i = 0; i < MAX_BOARD; i++)
    account->last_note[i] = 0;

  return account;
}

void close_account(ACCOUNT_DATA *account)
{
  /* so we can safely call this on NULL pointers */
  if (account == NULL)
    return;

  /* free the memory */
  free_string(account->players);
  free_string(account->password);
  free_string(account->owner);
  free_string(account->new_password);

  /* attach to free list */
  PushStack(account, account_free);
}

void show_options(DESCRIPTOR_DATA *dsock)
{
  char buf[MAX_STRING_LENGTH];
  struct plist *p_list;

  p_list = parse_player_list(dsock->account->players);

  if (dsock->connected != CON_PLAYING)
  {
    sprintf(buf, "\n\n\rWelcome %s. What's your game today ?\n\n\r", dsock->account->owner);
    write_to_buffer(dsock, buf, 0);
  }

  if (p_list->count > 0)
  {
    write_to_buffer(dsock, "      #9#uName          Class              Hours  #n\n\r", 0);
    sprintf(buf, "%s", p_list->text);
    write_to_buffer(dsock, buf, 0);
  }

  if (dsock->account->reimb_points)
  {
    sprintf(buf, "\n\r You have #C%d#n reimb points on this account.\n\n\r", dsock->account->reimb_points);
    write_to_buffer(dsock, buf,  0);
  }

  if (dsock->connected != CON_PLAYING)
  {
    sprintf(buf, " [C]  Create new player\n\r"
                 " [D]  Delete player\n\r"
                 "%s"
                 " [P]  Change Password\n\r"
                 " [Q]  Quit\n\n\r",
    (!can_refer(dsock->account)) ? "" : 
                 " [R]  Set Reference\n\r");
    write_to_buffer(dsock, buf, 0);
    write_to_buffer(dsock, "What will it be? ", 0);
  }

  free(p_list);
}

bool can_refer(ACCOUNT_DATA *account)
{
  if (account->reimb_points) return FALSE;
  if (account->max_might >= RANK_CADET) return FALSE;
  if (account->reference[0] != '\0') return FALSE;

  return TRUE;
}

struct plist *parse_player_list(char *list)
{
  CHAR_DATA *dMob;
  ITERATOR *pIter;
  struct plist *p_list;
  char tempbuf[MAX_STRING_LENGTH];
  bool first = TRUE;
  int total_time = 0;

  p_list = malloc(sizeof(*p_list));
  p_list->count = 0;

  while (*list != '\0')
  {
    char name[20];
    char race[20];
    char flags[100];
    char *ptr1, *ptr2;
    int played = 0;
    int revision = 0;
    bool in_game = FALSE;

    name[0] = '\0'; ptr1 = name;
    race[0] = '\0'; ptr2 = race;

    /* get the name */
    while (*list != ' ')
      *ptr1++ = *list++;
    *ptr1 = '\0'; list++;

    /* is that player already logged on ?? */
    pIter = AllocIterator(char_list);
    while ((dMob = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (IS_NPC(dMob)) continue;
      if (!str_cmp(dMob->name, name))
	in_game = TRUE;
    }

    /* get the race */
    while (*list != ' ')
      *ptr2++ = *list++;
    *ptr2 = '\0'; list++;

    /* get the hours */
    while (*list != ' ')
    {
      played *= 10;
      played += *list++ - '0';
    }
    list++;

    /* get the revision */
    while (*list != ' ' && *list != '\0')
    {
      revision *= 10;
      revision += *list++ - '0';
    }
    if (*list == ' ') list++;

    if (in_game)
      sprintf(flags, "    (active)");
    else if (revision <= REVISION_FROZEN)
      sprintf(flags, "    (frozen)");
    else if (revision <= REVISION_OUTDATED)
      sprintf(flags, "    (outdated)");
    else if (revision < CURRENT_REVISION)
      sprintf(flags, "    (out of sync)");
    else
      flags[0] = '\0';

    p_list->count++;
    sprintf(tempbuf, " [%d]  %-12s  %-12s   %5d hour%s%s\n\r",
      p_list->count, name, race, played, (played == 1) ? " " : "s", flags);

    /* add up */
    total_time += played;

    if (first)
    {
      first = FALSE;
      sprintf(p_list->text, "%s", tempbuf);
    }
    else strcat(p_list->text, tempbuf);
  }

  /* should we add a line with the total time ? */
  if (!first)
  {
    sprintf(tempbuf, "%34s %5d hour%s total\n\r", " ", total_time, (total_time == 1) ? "" : "s");
    strcat(p_list->text, tempbuf);
  }

  return p_list;
}

char *get_option_login(char *list, int option)
{
  static char buf[MAX_STRING_LENGTH];
  int current = 1, i = 0;

  buf[0] = '\0';

  /* saves some time */
  if (option < 1) return NULL;

  while (*list != '\0')
  {
    /* get the name */
    while (*list != ' ')
    {
      if (current == option) buf[i++] = *list;
      list++;
    }
    if (current == option)
    {
      buf[i] = '\0';
      return buf;
    }
    list++;

    /* scan past class */
    while (*list++ != ' ') ;

    /* scan past hours */
    while (*list++ != ' ') ;

    /* scan past revision */
    while (*list != '\0' && *list++ != ' ') ;

    current++;
  }

  if (buf[0] == '\0')
    return NULL;
  else
    return buf;
}

/*
 * check reconnect is called before this procedure,
 * so we don't have to worry about this being the
 * same char as the one trying to logon.
 */
bool already_logged(char *name)
{
  CHAR_DATA *dMob;
  ITERATOR *pIter;

  pIter = AllocIterator(char_list);
  while ((dMob = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_NPC(dMob)) continue;
    if (!str_cmp(dMob->pcdata->account, name))
      return TRUE;
  }
  return FALSE;
}

void account_update(ACCOUNT_DATA *account, CHAR_DATA *dMob)
{
  char buf[MAX_STRING_LENGTH], name[MAX_STRING_LENGTH];
  char *ptr, *list;
  int k, i = 0, j = 0;
  int might = getMight(dMob);

  /* We should scan for references */
  if (account->reference[0] != '\0')
  {
    if (!IS_SET(account->flags, ACCOUNT_FLAG_REFER1) && might >= RANK_CADET)
    {
      ACCOUNT_DATA *reference;

      SET_BIT(account->flags, ACCOUNT_FLAG_REFER1);

      if ((reference = load_account(account->reference)) != NULL)
      {
        reference->reimb_points += 50;
        save_account(reference);
        close_account(reference);
      }
    }

    if (!IS_SET(account->flags, ACCOUNT_FLAG_REFER2) && might >= RANK_ADVENTURER)
    {
      ACCOUNT_DATA *reference;

      SET_BIT(account->flags, ACCOUNT_FLAG_REFER2);
 
      if ((reference = load_account(account->reference)) != NULL)
      {
        reference->reimb_points += 75;
        save_account(reference);
        close_account(reference);
      }
    }
  }

  buf[0] = '\0';
  ptr = buf;
  list = account->players;

  /* first we error check */
  if (!is_full_name(dMob->name, account->players))
  {
    sprintf(buf, "Account_update: %s not in %s's playerlist", dMob->name, account->owner);
    bug(buf, 0);
    return;
  }

  /* then we parse */
  while (1)
  {
    one_argument(list + j, name);

    if (!str_cmp(name, dMob->name))
    {
      /* scan past name and following space */
      while ((buf[i] = *(list + j)) != ' ')
      {
	i++, j++;
      }
      i++, j++;

      /* scan past old class and following space */
      while (*(list + j) != ' ')
        j++;
      j++;

      /* copy "new" class */
      for (k = 0; class_table[k].class_name[0] != '\0'; k++)
      {
        int x = 0;

        if (dMob->class != class_table[k].class_num) continue;

        while (class_table[k].class_name[x] != '\0')
        {
          buf[i++] = class_table[k].class_name[x++];
        }
      }
      buf[i++] = ' ';

      /* parse correct time */
      {
	char tempbuf[MAX_STRING_LENGTH];
	int count = 0;

	sprintf(tempbuf, "%d",
          (dMob->played + (int) (current_time - dMob->logon))/3600);

        /* copy new time */
        while ((buf[i] = tempbuf[count++]) != '\0')
	  i++;

	/* skip past the old time entry */
	while (*(list+j) != ' ')
	  j++;
        j++;
      }
      buf[i++] = ' ';

      /* insert revision control */
      {
        char tempbuf[MAX_STRING_LENGTH];
        int count = 0;

        sprintf(tempbuf, "%d", dMob->pcdata->revision);

        /* copy new revision */
        while ((buf[i] = tempbuf[count++]) != '\0')
          i++;
        buf[i++] = ' ';

        /* skip past the old revision entry */
        while (*(list+j) != '\0' && *(list+j) != ' ')
          j++;

        if (*(list + (j++)) == '\0')
        {
          buf[i - 1] = '\0';
          break;
        }
      }
    }
    else /* scan forward one entry */
    {
      /* scan past name */
      while ((buf[i] = *(list + j)) != ' ')
      {
        i++, j++;
      }
      i++, j++;

      /* scan past class */
      while ((buf[i] = *(list + j)) != ' ')
      {
        i++, j++;
      }
      i++, j++;

      /* scan past hours */
      while ((buf[i] = *(list + j)) != ' ')
      {
        i++, j++;
      }
      i++, j++;

      /* scan past revision */
      while ((buf[i] = *(list + j)) != '\0' && *(list + j) != ' ')
      {
        i++, j++;
      }
      i++;

      /* found the end */
      if (*(list + (j++)) == '\0')
        break;
    }
  }

  /* then we copy */
  free_string(account->players);
  account->players = str_dup(buf);

  /* update max_might */
  if (might > account->max_might)
    account->max_might = might;

  /* and finally we save */
  save_account(account);
}

void account_new_player(ACCOUNT_DATA *account, CHAR_DATA *dMob)
{
  char buf[MAX_STRING_LENGTH];

  sprintf(buf, "%s%s%s %s %d %d",
    account->players, (account->players[0] == '\0') ? "" : " ",
    dMob->name, class_lookup(dMob), dMob->played/3600, CURRENT_REVISION);
  free_string(account->players);
  account->players = str_dup(buf);
  account->p_count++;
  save_account(account);
}

void create_new_account(ACCOUNT_DATA *account)
{
  char buf[MAX_STRING_LENGTH];

  /* create the directory */
  sprintf(buf, "mkdir ../accounts/%s", account->owner);
  system(buf);

  /* save the account */
  save_account(account);
}

char *class_lookup(CHAR_DATA *ch)
{
  static char buf[MAX_STRING_LENGTH];
  int i;

  for (i = 0; class_table[i].class_name[0] != '\0'; i++)
  {
    if (ch->class != class_table[i].class_num) continue;

    sprintf(buf, "%s", class_table[i].class_name);
    return buf;
  }

  sprintf(buf, "None");
  return buf;
}

bool is_valid_class_choice(char *choice, CHAR_DATA *ch)
{
  int i = atoi(choice), j;

  if (!isdigit(choice[0]))
    return FALSE;

  /* let's see if it's in the table */
  for (j = 0; class_table[j].class_name[0] != '\0'; j++)
  {
    if (i == j || !str_cmp(class_table[j].class_name, choice))
    {
      if (class_table[j].autoclass == FALSE)
        return FALSE;

      ch->class = class_table[j].class_num;
      return TRUE;
    }
  }

  /* nope, wasn't here */
  return FALSE;
}

void show_class_options(DESCRIPTOR_DATA *d)
{
  char buf[MAX_STRING_LENGTH];
  bool devel = FALSE;
  int i, j;

  write_to_buffer(d, "\n\rPick your starting class\n\r", 0);

  for (j = 0; j < 2; j++)
  {
    if (j == 0)
      write_to_buffer(d, "\n\r#9 [**] Available Classes#n\n\r", 0);
    if (j == 1 && devel)
      write_to_buffer(d, "\n\r#9 [**] Under Construction#n\n\r", 0);

    for (i = 0; class_table[i].class_name[0] != '\0'; i++)
    {
      if (class_table[i].autoclass == FALSE)
      {
        devel = TRUE;

        if (j == 1)
          sprintf(buf, " [--]  %s\n\r", class_table[i].class_name);
        else
          continue;
      }
      else
      {
        if (j == 0)
          sprintf(buf, " [%2d]  %s\n\r", i, class_table[i].class_name);
        else
          continue;
      }

      write_to_buffer(d, buf, 0);
    }
  }

  write_to_buffer(d, "\n\rWhat is your pick? ", 0);
}


char *get_rare_rank_name(int value)
{
  static char buf[MAX_INPUT_LENGTH];

  if (value >= 280)
    sprintf(buf, "#9almighty#n");
  else if (value >= 260)
    sprintf(buf, "#CRuler#n");
  else if (value >= 240)
    sprintf(buf, "#oSupreme#n");
  else if (value >= 220)
    sprintf(buf, "#pKing#n");
  else if (value >= 200)
    sprintf(buf, "#RBaron#n");
  else if (value >= 180)
    sprintf(buf, "#yDuke#n");
  else if (value >= 160)
    sprintf(buf, "#gGeneral#n");
  else if (value >= 145)
    sprintf(buf, "#CCaptain#n");
  else if (value >= 130)
    sprintf(buf, "#RMaster#n");
  else if (value >= 115)
    sprintf(buf, "#yLegendary#n");
  else if (value >= 100)
    sprintf(buf, "#GHero#n");
  else if (value >= 85)
    sprintf(buf, "#LAdventurer#n");
  else if (value >= 70)
    sprintf(buf, "#rVeteran#n");
  else if (value >= 55)
    sprintf(buf, "#yPrivate#n");
  else if (value >= 40)
    sprintf(buf, "#oCadet#n");
  else
    sprintf(buf, "#pWannabe#n");

  return buf;
}

int get_rare_rank_value(int value)
{
  int might;

  if (value >= 280)
    might = RANK_ALMIGHTY;
  else if (value >= 260)
    might = RANK_RULER;
  else if (value >= 240)
    might = RANK_SUPREME;
  else if (value >= 220)
    might = RANK_KING;
  else if (value >= 200)
    might = RANK_BARON;
  else if (value >= 180)
    might = RANK_DUKE;
  else if (value >= 160)
    might = RANK_GENERAL;
  else if (value >= 145)
    might = RANK_CAPTAIN;
  else if (value >= 130)
    might = RANK_MASTER;
  else if (value >= 115)
    might = RANK_LEGENDARY;
  else if (value >= 100)
    might = RANK_HERO;
  else if (value >= 85)
    might = RANK_ADVENTURER;
  else if (value >= 70)
    might = RANK_VETERAN;
  else if (value >= 55)
    might = RANK_PRIVATE;
  else if (value >= 40)
    might = RANK_CADET;
  else
    might = RANK_WANNABE;

  return might;
}

OBJ_DATA *pop_rand_loweq()
{
  OBJ_DATA *obj;
  char *name = NULL;
  int type, enchantment, sn = 0, metal;
  char objname1[MAX_INPUT_LENGTH];
  char objname2[MAX_INPUT_LENGTH];
  char objname3[MAX_INPUT_LENGTH];
  AFFECT_DATA paf;

  if (number_percent() > 3)
    return NULL;

  enchantment = number_range(0, 5);
  metal = number_range(0, 2);

  /* what kind of item do we want */
  switch(number_range(0,2))
  {
    default:
      type = ITEM_ARMOR;
      break;
    case 2:
      type = ITEM_WEAPON;
      break;
  }

  /* create proto-object, and modify it */
  obj = create_object(get_obj_index(OBJ_VNUM_PROTOPLASM), 50);

  /* set values */
  obj->item_type    =  type;
  obj->weight       =  1;
  obj->cost         =  10;
  SET_BIT(obj->wear_flags, ITEM_TAKE);
  SET_BIT(obj->extra_flags, ITEM_RARE);

  /* set spell */
  if (type == ITEM_WEAPON)
    sn = enchantment_table[enchantment].sn_magic * 1000;
  else if (type == ITEM_ARMOR)
    sn = enchantment_table[enchantment].sn_magic;

  switch(type)
  {
    default:
      break;
    case ITEM_WEAPON:
      switch(number_range(1, 10))
      {
        default:
          name = "shortsword";
          obj->value[3] = 1;
          break;
        case 2:
          name = "dagger";
          obj->value[3] = 2;
          break;
        case 3:
          name = "longsword";
          obj->value[3] = 3;
          break;
        case 4:
          name = "whip";
          obj->value[3] = 4;
          break;
        case 5:
          name = "hammer";
          obj->value[3] = 6;
          break;
        case 6:
          name = "club";
          obj->value[3] = 7;
          break;
        case 7:
          name = "mace";
          obj->value[3] = 8;
          break;
        case 8:
          name = "kopesh";
          obj->value[3] = 10;
          break;
        case 9:
          name = "tentacle";
          obj->value[3] = 12;
          break;
        case 10:
          name = "sickle";
          obj->value[3] = 9;
          break;
      }
      obj->wear_flags += ITEM_WIELD;
      obj->value[0] = sn;
      obj->value[1] = number_range(15, 25);
      obj->value[2] = number_range(20, 40);
      break;
    case ITEM_ARMOR:
      switch(number_range(1, 14))
      {
        default:
          SET_BIT(obj->wear_flags, ITEM_WEAR_FINGER);
          name = "ring";
          break;
        case 2:
          SET_BIT(obj->wear_flags, ITEM_WEAR_NECK);
          name = "amulet";
          break;
        case 3:
          SET_BIT(obj->wear_flags, ITEM_WEAR_HEAD);
          name = "helmet";
          break;
        case 4: 
          SET_BIT(obj->wear_flags, ITEM_WEAR_FEET);
          name = "boots";
          break;
        case 5:
          SET_BIT(obj->wear_flags, ITEM_WEAR_HANDS);
          name = "gauntlets";
          break;
        case 6:
          SET_BIT(obj->wear_flags, ITEM_WEAR_ABOUT);
          name = "cloak";
          break;
        case 7:
          SET_BIT(obj->wear_flags, ITEM_WEAR_WAIST);   
          name = "belt";
          break;
        case 8:
          SET_BIT(obj->wear_flags, ITEM_WEAR_WRIST);
          name = "bracers";
          break;
        case 9:
          SET_BIT(obj->wear_flags, ITEM_WEAR_BODY);
          name = "mail";
          break;
        case 10:
          SET_BIT(obj->wear_flags, ITEM_WEAR_LEGS);
          name = "leggings";
          break;
        case 11:
          SET_BIT(obj->wear_flags, ITEM_WEAR_ARMS); 
          name = "sleeves";
          break;
        case 12:
          SET_BIT(obj->wear_flags, ITEM_WEAR_FACE);
          name = "visor";
          break;
        case 13:
          SET_BIT(obj->wear_flags, ITEM_WEAR_SHIELD);
          name = "buckler";
          break;
        case 14:
          obj->item_type = ITEM_LIGHT;
          name = "lantern";
          obj->value[2] = 100;
          break;
      }
      obj->value[0] = 15;
      obj->value[3] = sn;
      break;
  }

  /* add damroll, hitroll and armor class */
  paf.type       = 0;
  paf.duration   = -1;
  paf.location   = APPLY_HITROLL;
  paf.modifier   = number_range(4, 10);
  paf.bitvector  = 0;
  affect_to_obj(obj, &paf);

  paf.location   = APPLY_DAMROLL;
  paf.modifier   = number_range(4, 10);
  affect_to_obj(obj, &paf);

  paf.location   = APPLY_AC;
  paf.modifier   = (-1 * number_range(5, 15));
  affect_to_obj(obj, &paf);

  if (obj->item_type != ITEM_LIGHT)
  {
    sprintf(objname1, "%s %s %s",
      metal_table[metal].name, name, enchantment_table[enchantment].name);
    sprintf(objname2, "%s %s %s %s lies here.",
      (is_an_a_word(objname1[0])) ? "a" : "an",
      metal_table[metal].name, name, enchantment_table[enchantment].name);
  }
  else
  {
    sprintf(objname1, "%s %s", metal_table[metal].name, name);
    sprintf(objname2, "%s %s %s lies here.",
      (is_an_a_word(objname1[0])) ? "a" : "an", metal_table[metal].name, name);
  }
  sprintf(objname3, "%s %s", metal_table[metal].name, name);

  objname1[0] = UPPER(objname1[0]);
  objname2[0] = UPPER(objname2[0]);
  free_string(obj->short_descr);
  free_string(obj->description);
  free_string(obj->name);
  obj->short_descr = str_dup(objname1);  
  obj->description = str_dup(objname2);
  obj->name = str_dup(objname3);

  return obj;
}

/* Random Equipment Generator
 * --------------------------
 *
 * The following bit of code allows the mud to generate random pieces
 * of equipment from a given level-value. The equipment will range
 * from average starting equipment to sentient powerful equipment.
 */
OBJ_DATA *pop_rand_equipment(int level, bool forced)
{
  int chance = -1, type = -1;
  int metal = -1, spell = -1, enchantment = -1, jewel = -1;
  OBJ_DATA *obj;
  AFFECT_DATA paf;
  int damroll = 8, hitroll = 8, armor = 8, sn = -1;
  char objname1[MAX_STRING_LENGTH];
  char objname2[MAX_STRING_LENGTH];
  char objname3[MAX_STRING_LENGTH];
  char *name = "bugged";
  bool sentient = FALSE;
  int value = 10;

  /* should at least be of some useful level */
  if (level < 200 && !forced)
    return NULL;

  /* calculate chance of popping something */
  chance = URANGE(5, level/70, 10);

  /* check against the chance */
  if (!forced && (number_percent() > chance || level < 350))
  {
    /* 0.5% chance of popping something else */
    if (number_percent() <= 1 && number_percent() <= 50)
       return (pop_random_magic());

    return NULL;
  }

  /* what kind of item do we want */
  switch(number_range(0,2))
  {
    default:   /* armor is more common than weapons */
      type = ITEM_ARMOR;
      break;
    case 2:
      type = ITEM_WEAPON;
      break;
  }

  if (number_percent() < level / 60)
    metal = number_range(0, MAX_TYPE_METAL - 1);
  else if (number_percent() < level / 30)
    metal = number_range(0, MAX_TYPE_METAL / 2);

  if (number_percent() < level / 80)
    jewel = number_range(0, MAX_TYPE_JEWEL - 1);
  else if (number_percent() < level / 40)
    jewel = number_range(0, MAX_TYPE_JEWEL / 2);

  if (number_percent() < level / 80)
    spell = number_range(0, MAX_TYPE_SPELL - 1);
  else if (number_percent() < level / 40)
    spell = number_range(0, MAX_TYPE_SPELL / 2);

  if (number_percent() < level / 80)
    enchantment = number_range(0, MAX_TYPE_ENCHANTMENT - 1);
  else if (number_percent() < level / 40)
    enchantment = number_range(0, MAX_TYPE_ENCHANTMENT / 2);

  /* make sure it does at least one thing */
  if (metal == -1 && spell == -1 && enchantment == -1 && jewel == -1)
  {
    switch(number_range(0, 3))
    {
      case 0:
        enchantment = number_range(0, MAX_TYPE_ENCHANTMENT / 2);
        break;
      case 1:
        spell = number_range(0, MAX_TYPE_SPELL / 2);
        break;
      case 2:
        metal = number_range(0, MAX_TYPE_METAL / 2);
        break;
      case 3:
        jewel = number_range(0, MAX_TYPE_JEWEL / 2);
        break;
    }
  }

  if (metal != -1)
  {
    damroll += number_fuzzy(metal_table[metal].damroll);
    hitroll += number_fuzzy(metal_table[metal].hitroll);
    armor   += number_fuzzy(metal_table[metal].ac);
  }
  if (jewel != -1)
  {
    damroll += number_fuzzy(jewel_table[jewel].damroll);
    hitroll += number_fuzzy(jewel_table[jewel].hitroll);
    armor   += number_fuzzy(jewel_table[jewel].ac);
  }
  if (spell != -1)
  {
    damroll += number_fuzzy(spell_table[spell].damroll);
    hitroll += number_fuzzy(spell_table[spell].hitroll);
    armor   += number_fuzzy(spell_table[spell].ac);

    if (type == ITEM_WEAPON)
      sn = *spell_table[spell].sn_magic;
  }
  if (enchantment != -1)
  {
    damroll += number_fuzzy(enchantment_table[enchantment].damroll);
    hitroll += number_fuzzy(enchantment_table[enchantment].hitroll);
    armor   += number_fuzzy(enchantment_table[enchantment].ac);

    if (type == ITEM_WEAPON)
    {
      if (sn != -1)
        sn += enchantment_table[enchantment].sn_magic * 1000;
      else
        sn = enchantment_table[enchantment].sn_magic * 1000;
    }
    else if (type == ITEM_ARMOR)
      sn = enchantment_table[enchantment].sn_magic;
  }

  /* create proto-object, and modify it */
  obj = create_object(get_obj_index(OBJ_VNUM_PROTOPLASM), 50);

  /* set values */
  obj->item_type    =  type;
  obj->weight       =  1;
  SET_BIT(obj->wear_flags, ITEM_TAKE);

  /* calculate the value of the spell mods */
  if (metal != -1)
    value += metal_table[metal].value;
  if (enchantment != -1)
    value += enchantment_table[enchantment].value;
  if (jewel != -1)
    value += jewel_table[jewel].value;
  if (spell != -1)
    value += spell_table[spell].value;

  /* set the cost of the item to at least 25 */
  obj->cost = UMAX(25, value);

  /* check for sentient equipment */
  if (obj->cost > 130 && number_percent() > 90)
    sentient = TRUE;

  switch(type)
  {
    default:
      break;
    case ITEM_WEAPON:
      switch(number_range(1, 12))
      {
        case 1:
          name = "shortsword";
          obj->value[3] = DT_SLICE - 1000;
          break;
        case 2:
          name = "dagger";
          obj->value[3] = DT_STAB - 1000;
          break;
        case 3:
          name = "longsword";
          obj->value[3] = DT_SLASH - 1000;
          break;
        case 4:
          name = "whip";
          obj->value[3] = DT_WHIP - 1000;
          break;
        case 5:
          name = "hammer";
          obj->value[3] = DT_BLAST - 1000;
          break;
        case 6:
          name = "club";
          obj->value[3] = DT_POUND - 1000;
          break;
        case 7:
          name = "mace";
          obj->value[3] = DT_CRUSH - 1000;
          break;
        case 8:
          name = "fang";
          obj->value[3] = DT_BITE - 1000;
          break;
        case 9:
          name = "tentacle";
          obj->value[3] = DT_SUCK - 1000;
          break;
        case 10: 
          name = "sickle";
          obj->value[3] = DT_GREP - 1000;
          break;
        case 11:
          name = "spear";
          obj->value[3] = DT_PIERCE - 1000; 
          break;
        case 12:
          name = "claw";
          obj->value[3] = DT_CLAW - 1000;
          break;
      }
      obj->wear_flags += ITEM_WIELD;

      if (sn > 0)
        obj->value[0] = sn;

      /* weapon damage based on rank of weapon */
      obj->value[1] = 15 + UMAX(0, obj->cost - 40) / 7;
      obj->value[2] = 30 + UMAX(0, obj->cost - 40) / 4;

      /* add randomness of +/- 15% to both dice and dicesize (from -28% to +32% damage) */
      obj->value[1] = number_range(17 * obj->value[1] / 20, 23 * obj->value[1] / 20);
      obj->value[2] = number_range(17 * obj->value[2] / 20, 23 * obj->value[2] / 20);

      /* cap weapons at 50d90 */
      if (obj->value[1] > 50) obj->value[1] = number_fuzzy(50);
      if (obj->value[2] > 90) obj->value[2] = number_fuzzy(90);

      break;
    case ITEM_ARMOR:
      switch(number_range(1, 12))
      {
        case 1:
          SET_BIT(obj->wear_flags, ITEM_WEAR_FINGER);
          name = "ring";
          break;
        case 2:
          SET_BIT(obj->wear_flags, ITEM_WEAR_NECK);
          name = "amulet";
          break;
        case 3:
          SET_BIT(obj->wear_flags, ITEM_WEAR_HEAD);
          name = "helmet";
          break;
        case 4:
          SET_BIT(obj->wear_flags, ITEM_WEAR_FEET);
          name = "boots";
          break;
        case 5:
          SET_BIT(obj->wear_flags, ITEM_WEAR_HANDS);
          name = "gauntlets";
          break;
        case 6:
          SET_BIT(obj->wear_flags, ITEM_WEAR_ABOUT);
          name = "cloak";
          break;
        case 7:
          SET_BIT(obj->wear_flags, ITEM_WEAR_WAIST);
          name = "belt";
          break;
        case 8:
          SET_BIT(obj->wear_flags, ITEM_WEAR_WRIST);
          name = "bracers";
          break;
        case 9:
          SET_BIT(obj->wear_flags, ITEM_WEAR_BODY);
          name = "mail";
          break;
        case 10:
          SET_BIT(obj->wear_flags, ITEM_WEAR_LEGS); 
          name = "leggings";
          break;
        case 11:
          SET_BIT(obj->wear_flags, ITEM_WEAR_ARMS);
          name = "sleeves";
          break;
        case 12:
          SET_BIT(obj->wear_flags, ITEM_WEAR_FACE); 
          name = "visor";
          break;
      }
      obj->value[0] = URANGE(1, level / 100, 25);
      if (sn > 0) obj->value[3] = sn;
      break;
  }

  /* check for sentient equipment */
  if (sentient)
  {
    SET_BIT(obj->extra_flags, ITEM_SENTIENT);

    /* increase damroll/hitroll/armor by 25% */
    damroll = 5 * damroll / 4;
    hitroll = 5 * hitroll / 4;
    armor   = 5 * armor / 4;

    obj->sentient_points = obj->cost - 100;

    /* increase weapon damage by 10% */
    if (type == ITEM_WEAPON)
    {
      obj->value[1] = obj->value[1] * 11 / 10;
      obj->value[2] = obj->value[2] * 11 / 10;
    }
  }
  else
  {
    SET_BIT(obj->extra_flags, ITEM_RARE);
  }

  if (type == ITEM_WEAPON)
  {
    if (metal != -1)
    {
      obj->value[1] += metal_table[metal].weap;
      obj->value[2] += metal_table[metal].weap;
    }
    else if (enchantment != -1)
    {
      obj->value[1] += enchantment_table[enchantment].weap;
      obj->value[2] += enchantment_table[enchantment].weap;
    }
  }

  if (jewel != -1)
  {
    SET_BIT(obj->extra_flags, ITEM_GEMMED);

    switch(jewel)
    {
      default:
        break;
      case 0:
        paf.type       = 0;
        paf.duration   = -1;
        paf.location   = APPLY_STR;
        paf.modifier   = number_range(1, 3);
        paf.bitvector  = 0;
        affect_to_obj(obj, &paf);
        break;
      case 1:
        paf.type       = 0;
        paf.duration   = -1;
        paf.location   = APPLY_DEX;
        paf.modifier   = number_range(1, 3);
        paf.bitvector  = 0;
        affect_to_obj(obj, &paf);
        break;
      case 2:
        paf.type       = 0;
        paf.duration   = -1;
        paf.location   = APPLY_CON;
        paf.modifier   = number_range(1, 3);
        paf.bitvector  = 0;
        affect_to_obj(obj, &paf);
        break;
      case 3:
        paf.type       = 0;
        paf.duration   = -1;
        paf.location   = APPLY_WIS;
        paf.modifier   = number_range(1, 3);
        paf.bitvector  = 0;
        affect_to_obj(obj, &paf);
        break;
      case 4:
        paf.type       = 0;
        paf.duration   = -1;
        paf.location   = APPLY_INT;
        paf.modifier   = number_range(1, 3);
        paf.bitvector  = 0;
        affect_to_obj(obj, &paf);
        break;
    }
  }

  /* 1 out of 100 might have some special affect */
  if (number_percent() == 50)
  {
    int rand_affect = number_percent();

    if (rand_affect >= 90)
    {
      if (type == ITEM_ARMOR || type == ITEM_WEAPON)
        SET_BIT(obj->extra_flags, ITEM_UNBREAKABLE);
    }
    else if (rand_affect >= 80)
    {
      if (type == ITEM_ARMOR || type == ITEM_WEAPON)
      {
        damroll *= 2;
        hitroll *= 2;
        armor *= 2;
        SET_BIT(obj->extra_flags, ITEM_NOREPAIR);
      }
    }
    else if (rand_affect >= 50)
    {
      paf.type      = 0;
      paf.duration  = -1;
      switch(number_range(1, 3))
      {
        default:
          paf.location = APPLY_HIT;
          break;
        case 2:
          paf.location = APPLY_MOVE;
          break;
        case 3:
          paf.location = APPLY_MANA;
          break;
      }
      paf.modifier   = -35;
      paf.bitvector  = 0;
      affect_to_obj(obj, &paf);
    }
    else if (rand_affect >= 25)
    {
      paf.type       = 0;
      paf.duration   = -1;
      switch(number_range(1, 5))
      {
        default:
          paf.location = APPLY_WIS;
          break;
        case 2:
          paf.location = APPLY_CON;
          break;
        case 3:
          paf.location = APPLY_INT;
          break;
        case 4:
          paf.location = APPLY_DEX;
          break;
        case 5:
          paf.location = APPLY_STR;
          break;
      }
      paf.modifier   = number_range(1, 3);
      paf.bitvector  = 0;
      affect_to_obj(obj, &paf);
    }
    else
    {
      damroll++;
      armor++;
      hitroll++; 
    }
  }

  /* now cap damroll, hitroll and armor */
  if (IS_SET(obj->extra_flags, ITEM_RARE))
  {
    hitroll = UMIN(40, hitroll);
    damroll = UMIN(40, damroll);
    armor   = UMIN(50, armor);
  }
  else
  {
    hitroll = UMIN(50, hitroll);
    damroll = UMIN(50, damroll);
    armor   = UMIN(65, armor);
  }

  /* add damroll, hitroll and armor class */
  paf.type       = 0;
  paf.duration   = -1;
  paf.location   = APPLY_HITROLL;
  paf.modifier   = number_fuzzy(hitroll);
  paf.bitvector  = 0;
  affect_to_obj(obj, &paf);

  paf.location   = APPLY_DAMROLL;
  paf.modifier   = number_fuzzy(damroll);
  affect_to_obj(obj, &paf);

  paf.location   = APPLY_AC;
  paf.modifier   = (-1 * number_fuzzy(armor));
  affect_to_obj(obj, &paf);

  sprintf(objname1, "%s%s%s%s%s%s%s%s%s",
    (spell != -1) ? spell_table[spell].name : "",
    (spell != -1) ? " " : "",
    (metal != -1) ? metal_table[metal].name : "",
    (metal != -1) ? " " : "",
     name,
    (enchantment != -1) ? " " : "",
    (enchantment != -1) ? enchantment_table[enchantment].name : "",
    (jewel != -1) ? " encrusted with " : "",
    (jewel != -1) ? jewel_table[jewel].name : "");
  sprintf(objname2, "%s %s%s%s%s%s%s%s lies here.",
    (is_an_a_word(objname1[0])) ? "a" : "an",
    (spell != -1) ? spell_table[spell].name : "",
    (spell != -1) ? " " : "",
    (metal != -1) ? metal_table[metal].name : "",
    (metal != -1) ? " " : "",
     name,
    (enchantment != -1) ? " " : "",
    (enchantment != -1) ? enchantment_table[enchantment].name : "");
  sprintf(objname3, "%s%s%s%s%s",
    (spell != -1) ? spell_table[spell].name : "",
    (spell != -1) ? " " : "",
    (metal != -1) ? metal_table[metal].name : "",
    (metal != -1) ? " " : "", name);

  objname1[0] = UPPER(objname1[0]);
  objname2[0] = UPPER(objname2[0]);
  free_string(obj->short_descr);
  free_string(obj->description);
  free_string(obj->name);
  obj->short_descr = str_dup(objname1);
  obj->description = str_dup(objname2);
  obj->name = str_dup(objname3);

  return obj;
}

OBJ_DATA *pop_random_magic()
{
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];
  int tip = number_range(0, MAX_TYPE_TIPPED - 1);
  int spell, sn, i;

  if (number_range(1, 4) == 2)
    spell = number_range(0, MAX_TYPE_WANDSPELL - 1);
  else
    spell = number_range(0, MAX_TYPE_WANDSPELL / 2 - 1);

  if ((sn = skill_lookup(spell_wand_table[spell].spell)) < 0)
  {
    bug("pop_random_magic: bad sn.", 0);
    return NULL;
  }

  obj = create_object(get_obj_index(OBJ_VNUM_PROTOPLASM), 50);
  obj->weight = 1;

  if (skill_table[sn].target != 1 && skill_table[sn].target != 2)
    i = number_range(1, 5);
  else
    i = number_range(1, 6);

  switch(i)
  {
    default:
      obj->item_type = ITEM_SCROLL;
      obj->value[0]  = 50;
      obj->value[1]  = sn;
      obj->value[2]  = sn;
      obj->value[3]  = sn;

      /* set the objects name */
      sprintf(buf, "scroll %s", spell_wand_table[spell].name);
      free_string(obj->name);
      obj->name = str_dup(buf);
      sprintf(buf, "scroll of %s", spell_wand_table[spell].name);
      free_string(obj->short_descr);
      obj->short_descr = str_dup(buf);
      free_string(obj->description);
      obj->description = str_dup("a rolled-up scroll lies on the floor.");
      break;
    case 4:
    case 5:
      obj->item_type = ITEM_WAND;
      obj->value[0]  = 50;
      obj->value[1]  = tipped_table[tip].charges * 2;
      obj->value[2]  = tipped_table[tip].charges * 2;
      obj->value[3]  = sn;

      /* set the objects name */
      sprintf(buf, "wand %s", spell_wand_table[spell].name);
      free_string(obj->name);
      obj->name = str_dup(buf);
      sprintf(buf, "%s wand of %s", tipped_table[tip].name, spell_wand_table[spell].name);
      free_string(obj->short_descr);
      obj->short_descr = str_dup(buf);
      free_string(obj->description);
      obj->description = str_dup("a wand lies on the floor.");
      break;
    case 6:
      obj->item_type = ITEM_STAFF;
      obj->value[0]  = 50;
      obj->value[1]  = tipped_table[tip].charges;
      obj->value[2]  = tipped_table[tip].charges;
      obj->value[3]  = sn;

      /* set the objects name */
      sprintf(buf, "staff %s", spell_wand_table[spell].name);
      free_string(obj->name);
      obj->name = str_dup(buf);
      sprintf(buf, "%s staff of %s", tipped_table[tip].name, spell_wand_table[spell].name);
      free_string(obj->short_descr);
      obj->short_descr = str_dup(buf);
      free_string(obj->description);
      obj->description = str_dup("a staff rests against the wall.");
      break;
  }

  return obj;
}

/* This is the modifier tables - different types of equipment
 * uses different types of modifier tables. These tables also
 * contains prefix and suffix char pointers from which to generate
 * the names of the equipment.
 */

const struct type_tipped tipped_table[MAX_TYPE_TIPPED] =
{
  /* name            charges */
  /* ----------------------- */
  {  "wooden",           2  },
  {  "silver tipped",    3  },
  {  "crystal",          4  }
};

const struct type_spell_wand spell_wand_table[MAX_TYPE_WANDSPELL] =
{
  /* name               spell              */
  /* ------------------------------------- */
  {  "magic qunching", "dispel magic"      },
  {  "god's blessing", "godbless"          },
  {  "spirits",        "spirit kiss"       },
  {  "chaos blast",    "chaos blast"       },
  {  "healing",        "imp heal"          },
  {  "desancting",     "desanct"           },
  {  "firekiss",       "fire breath"       },
  {  "fire",           "imp fireball"      },
  {  "teleport",       "imp teleport"      },
  {  "gas blasts",     "gas blast"         }
};

/*
 * The lower half should always be better than
 * the upper half - the lower half is more commonly
 * found among popped equipment.
 */
const struct type_metal metal_table[MAX_TYPE_METAL] =
{
  /* name        damroll  hitroll  weapon-dam  armor class   value */
  /* ------------------------------------------------------------- */

  {  "bronze",       2,      2,        1,         5,          17  },
  {  "copper",       3,      3,        2,         8,          26  },
  {  "iron",         5,      5,        3,        10,          40  },
  {  "steel",        8,      8,        4,        15,          63  },
  {  "mithril",     11,     11,        5,        20,          86  },
  {  "adamantium",  14,     14,        6,        30,         114  }
};

const struct type_jewel jewel_table[MAX_TYPE_JEWEL] =
{
  /* name          damroll  hitroll  armor class  value */
  /* -------------------------------------------------- */
 
  {  "amethysts",     2,      2,        5,         17  },
  {  "sapphires",     3,      3,        6,         24  },
  {  "opals",         4,      4,        8,         32  },
  {  "rubies",        7,      7,       14,         56  },
  {  "diamonds",      9,      9,       18,         72  }
};

/*
 * The lower half should always be better than
 * the upper half - the lower half is more commonly
 * found among popped equipment.
 */
const struct type_spell spell_table[MAX_TYPE_SPELL] =
{
  /* name        damroll  hitroll    armor   spell type         value */
  /* ---------------------------------------------------------------- */

  {  "fiery",        3,      3,        1,   &gsn_firebreath,      19 },
  {  "magical",      3,      3,        1,   &gsn_magicmissile,    19 },
  {  "vampiric",     4,      4,        1,   &gsn_energydrain,     25 },
  {  "divine",       5,      5,        1,   &gsn_groupheal,       31 },
  {  "chaotic",      6,      6,        2,   &gsn_chaosblast,      38 },
  {  "immolating",   5,      5,        1,   &gsn_impfireball,     40 },
  {  "cursed",       7,      7,        2,   &gsn_curse,           44 },
  {  "poisonous",    8,      8,        2,   &gsn_poison,          50 },
  {  "screaming",    9,      9,        2,   &gsn_earthquake,      56 }
};

/*
 * The lower half should always be better than
 * the upper half - the lower half is more commonly
 * found among popped equipment.
 */
const struct type_enchantment enchantment_table[MAX_TYPE_ENCHANTMENT] =
{
  /* name                      dam   hit  armor  weap    spell                  value  */
  /* --------------------------------------------------------------------------------- */

  {  "of invis detection",       1,    1,    2,   1,     OBJECT_DETECTINVIS,      15  },
  {  "of passdoor",              1,    1,    2,   2,     OBJECT_PASSDOOR,         15  },
  {  "of regeneration",          2,    2,    2,   2,     OBJECT_REGENERATE,       17  },
  {  "of shadowsight",           2,    2,    2,   2,     OBJECT_INFRARED,         17  },
  {  "of see hidden",            2,    2,    2,   1,     OBJECT_DETECTHIDDEN,     17  },
  {  "of protection from good",  3,    3,    2,   4,     OBJECT_PROTECTGOOD,      23  },
  {  "of protection from evil",  3,    3,    2,   4,     OBJECT_PROTECT,          23  },
  {  "of invisibility",          3,    3,    5,   3,     OBJECT_INVISIBLE,        23  },
  {  "of chaos shield",          2,    2,    2,   3,     OBJECT_CHAOSSHIELD,      23  },
  {  "of sanctuary",             2,    2,    5,   1,     OBJECT_SANCTUARY,        25  },
  {  "of flying",                2,    2,    5,   4,     OBJECT_FLYING,           25  },
  {  "of speed",                 3,    2,    5,   3,     OBJECT_SPEED,            30  },
};

bool is_an_a_word(char c)
{
  switch(c)
  {
    default:
      break;
    case 'q':
    case 'e':
    case 'y':
    case 'u':
    case 'i':
    case 'o':
    case 'a':
      return FALSE;
      break;
  }

  return TRUE;
}

void show_help_player(char *txt, CHAR_DATA *ch)
{
  char buf[MAX_STRING_LENGTH];
  int i = 0;

  while (*txt != '\0')
  {
    switch(*txt)
    {
      default:
        buf[i++] = *txt++;
        break;
      case '\r':
        txt++;
        break;
      case '\n':
        txt++;
        buf[i++] = '<';
        buf[i++] = 'B';
        buf[i++] = 'R';
        buf[i++] = '>';
        break;
      case '@':
        if (*(++txt) == '@')
        {
          char helpentry[MAX_STRING_LENGTH];
          int j = 0;

          /* copy the help name to temporary buffer */
          while ((helpentry[j] = *(++txt)) != '@' && helpentry[j] != '\0')
            j++;
          helpentry[j] = '\0';
          txt++;

          buf[i] = '\0';
          strcat(buf, "<SEND href=\"help '");
          strcat(buf, helpentry);
          strcat(buf, "'\">");
          strcat(buf, helpentry);
          strcat(buf, "</SEND>");

          i += (strlen("<SEND href=\"help '") + 2 * j + strlen("</SEND>") + strlen("'\">"));
        }
        else
        {
          buf[i++] = '@';
        }
        break;
    }
  }
  buf[i] = '\0';

  mxp_to_char(buf, ch, MXP_ALL);
}

void powerdown(CHAR_DATA *ch)
{
  EVENT_DATA *event;
  AFFECT_DATA *paf;
  OBJ_DATA *obj;
  ITERATOR *pIter;

  if (IS_NPC(ch)) return;

  if (IS_CLASS(ch, CLASS_SHADOW))
  {
    REMOVE_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_AURASIGHT);
    REMOVE_BIT(ch->act, PLR_HIDE);
    REMOVE_BIT(ch->immune, IMM_SHIELDED);
  }
  else if (IS_CLASS(ch, CLASS_FAE))
  {
    REMOVE_BIT(ch->act, PLR_HIDE);
    REMOVE_BIT(ch->newbits, NEW_CUBEFORM);
    REMOVE_BIT(ch->pcdata->powers[FAE_BITS], FAE_GASEOUS);
    REMOVE_BIT(ch->newbits, NEW_FAEHALO);
    REMOVE_BIT(ch->newbits, NEW_PSPRAY);
    REMOVE_BIT(ch->newbits, NEW_CHAMELEON);
    REMOVE_BIT(ch->pcdata->powers[FAE_BITS], FAE_BLASTBEAMS);
    ch->pcdata->powers[FAE_SHIELD] = 0;
  }
  else if (IS_CLASS(ch, CLASS_GIANT))
  {
    ch->pcdata->powers[GIANT_STANDFIRM] = 0;
    REMOVE_BIT(ch->newbits, NEW_CUBEFORM);
    REMOVE_BIT(ch->extra, EXTRA_WINDWALK);
    REMOVE_BIT(ch->newbits, NEW_SINKHOLE);
    REMOVE_BIT(ch->newbits, NEW_MUDFORM);
  }
  else if (IS_CLASS(ch, CLASS_WARLOCK))
  {
    REMOVE_BIT(ch->newbits, NEW_IRONMIND);
    REMOVE_BIT(ch->affected_by, AFF_SHATTERSHIELD);
    REMOVE_BIT(ch->newbits, NEW_MOUNTAIN);
    REMOVE_BIT(ch->newbits, NEW_BACKLASH);
    REMOVE_BIT(ch->newbits, NEW_HSTARS);
    REMOVE_BIT(ch->pcdata->tempflag, TEMP_DOOMBOLT);
  }

  pIter = AllocIterator(ch->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    switch(event->type)
    {
      default:
        break;
      case EVENT_MOBILE_CASTING:
      case EVENT_PLAYER_FAE_PLASMA:
      case EVENT_PLAYER_FAE_ENERGY:
      case EVENT_PLAYER_FAE_WILL:
      case EVENT_PLAYER_FAE_MATTER:
      case EVENT_MOBILE_CONFUSED:
      case EVENT_PLAYER_DISPLACE:
      case EVENT_MOBILE_ACIDBLOOD:
      case EVENT_PLAYER_FAE_SIGIL:
      case EVENT_PLAYER_SPIDERS:
      case EVENT_PLAYER_PHANTOM:
      case EVENT_PLAYER_PVIPER_GREEN:
      case EVENT_PLAYER_PVIPER_RED:
      case EVENT_PLAYER_PVIPER_BLUE:
      case EVENT_PLAYER_PVIPER_YELLOW:
      case EVENT_PLAYER_PVIPER_PURPLE:
      case EVENT_PLAYER_HEATMETAL:
      case EVENT_PLAYER_WHIRLWIND:
      case EVENT_PLAYER_WITNESS:
        dequeue_event(event, TRUE);
        break;
    }
  }

  pIter = AllocIterator(ch->carrying);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->wear_loc != WEAR_NONE)
    {
      obj_from_char(obj);
      obj_to_char(obj, ch);
    }
  }

  /* strip all spells */
  pIter = AllocIterator(ch->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    affect_remove(ch, paf);
  }

  ch->affected_by = 0;
  ch->itemaffect = 0;

  /* remove all and any special flags */
  REMOVE_BIT(ch->extra, TIED_UP);
  REMOVE_BIT(ch->extra, GAGGED);
  REMOVE_BIT(ch->extra, BLINDFOLDED);
  REMOVE_BIT(ch->extra, EXTRA_PREGNANT);
  REMOVE_BIT(ch->extra, EXTRA_LABOUR);
  REMOVE_BIT(ch->newbits, NEW_STITCHES);
  REMOVE_BIT(ch->newbits, NEW_SHADOWPLANE);
  REMOVE_BIT(ch->newbits, NEW_TENDRIL1);
  REMOVE_BIT(ch->newbits, NEW_TENDRIL2);
  REMOVE_BIT(ch->newbits, NEW_TENDRIL3);

  /* heal any lost limbs */
  ch->loc_hp[0] = 0;
  ch->loc_hp[1] = 0;
  ch->loc_hp[2] = 0;
  ch->loc_hp[3] = 0;
  ch->loc_hp[4] = 0;
  ch->loc_hp[5] = 0;
  ch->loc_hp[6] = 0;

  ch->pcdata->mod_str = 0;
  ch->pcdata->mod_int = 0;
  ch->pcdata->mod_wis = 0;
  ch->pcdata->mod_dex = 0;
  ch->pcdata->mod_con = 0;

  ch->armor = 100;
  ch->hitroll = 0;
  ch->damroll = 0;
  ch->saving_throw = 0;
  ch->pcdata->sit_safe = 0;
  ch->position = POS_STANDING;
}

void break_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
  if (IS_OBJ_STAT(obj, ITEM_UNBREAKABLE))
  {
    obj->condition = 100;
    act("$p magically repairs itself.", ch, obj, NULL, TO_ALL);
    return;
  }

  act("$p falls broken to the ground.", ch, obj, NULL, TO_CHAR);
  act("$p falls broken to the ground.", ch, obj, NULL, TO_ROOM);
  obj_from_char(obj);
  if (ch->in_room && !IS_OBJ_STAT(obj, ITEM_NOREPAIR))
    obj_to_room(obj, ch->in_room);
  else
    extract_obj(obj);
}

void affect_to_area(AREA_DATA *pArea, AREA_AFFECT *paf)
{
  AREA_AFFECT *paf_new;

  if ((paf_new = (AREA_AFFECT *) PopStack(area_affect_free)) == NULL)
  {
    paf_new = calloc(1, sizeof(*paf_new));
  }

  *paf_new = *paf;
  AttachToList(paf_new, pArea->affects);
}

void affect_from_area(AREA_DATA *pArea, AREA_AFFECT *paf)
{
  DetachFromList(paf, pArea->affects);
  PushStack(paf, area_affect_free);
}

/* if owner is non-NULL, it will only return TRUE if the
 * affect has that owner.
 */
AREA_AFFECT *has_area_affect(AREA_DATA *pArea, int affect, int owner)
{
  AREA_AFFECT *paf;
  ITERATOR *pIter;

  pIter = AllocIterator(pArea->affects);
  while ((paf = (AREA_AFFECT *) NextInList(pIter)) != NULL)
  {
    if (paf->type == affect)
    {
      if (owner != 0 && paf->owner != owner)
        continue;
      else
        return paf;
    }
  }

  return NULL;
}

/*
 * Acts like sprintf(), but doesn't break alignment
 * due to colors. It only supports %d and %s. Returns
 * the amount of chars copied.
 */
int cprintf(char *buf, char *ptr, ...)
{
  va_list ap;

  va_start(ap, ptr);

  return _cprintf(buf, _NO_STRING_LIMIT_, ptr, ap);
}

/*
 * Just as cprintf(), but safer, since you can restrict
 * the maximum amount of copied chars. It will return
 * the amount of copied chars, unless the output was
 * truncated due to reaching maxlen before it was done
 * copying the entire string, in which case it will return -1.
 */
int cnprintf(char *buf, int maxlen, char *ptr, ...)
{
  va_list ap;

  va_start(ap, ptr);

  return _cprintf(buf, maxlen, ptr, ap);
}

int _cprintf(char *buf, int maxlen, char *ptr, va_list ap)
{
  char dirty[100];
  char *s;
  int i, copied = 0;
  bool bEnd = FALSE;

  while(*ptr != '\0')
  {
    bool reverse = FALSE;
    int size = 0, max_size = 0, j = 0;
    bEnd = FALSE;

    switch(*ptr)
    {
      default:
	*buf++ = *ptr++;

	if (++copied == maxlen)
	  goto done_copied;

	break;
      case '%':

	/* should we align this in reverse ? */
	if (*(ptr + 1) == '-')
	{
	  ptr++;
	  reverse = TRUE;
	}

	/* get the size, if any */
	while (isdigit(*(ptr + 1)))
	{
	  size *= 10;
	  size += *(++ptr) - '0';
	}

	/* any size restrictions ? */
	if (*(ptr + 1) == '.')
	{
	  ptr++;
	  while (isdigit(*(ptr + 1)))
          {
	    max_size *= 10;
	    max_size += *(++ptr) - '0';
          }
	}

	switch(*(++ptr))
	{
          default:
	    *buf++ = '%';

            if (++copied == maxlen)
	      goto done_copied;

	    break;
          case 's':
	    s = va_arg(ap, char *);
	    s = string_restrict(s, max_size);

	    size -= collen(s);

	    if (!reverse)
	    {
	      while(size-- > 0)
	      {
		*buf++ = ' ';

                if (++copied == maxlen)
	          goto done_copied;
	      }
	    }
	    while(*s != '\0')
	    {
	      *buf++ = *s++;

	      if (!reverse && *s == '\0')
		bEnd = TRUE;

              if (++copied == maxlen)
                goto done_copied;
	    }
	    if (reverse)
	    {
	      while(size-- > 0)
	      {
		*buf++ = ' ';

                if (size == 0)
	          bEnd = TRUE;

                if (++copied == maxlen)
	          goto done_copied;
	      }
	    }
	    ptr++;
	    break;
          case 'd':
	    i = va_arg(ap, int);

	    /* a little trick to see how long the number is */
	    sprintf(dirty, "%d", i);
	    size -= strlen(dirty);

	    if (!reverse)
	    {
	      while(size-- > 0)
	      {
		*buf++ = ' ';

                if (++copied == maxlen)
	          goto done_copied;
	      }
	    }

	    while (dirty[j] != '\0')
	    {
	      *buf++ = dirty[j++];

	      if (!reverse && dirty[j] == '\0')
		bEnd = TRUE;

              if (++copied == maxlen)
                goto done_copied;
	    }

	    if (reverse)
	    {
	      while(size-- > 0)
	      {
		*buf++ = ' ';

                if (size == 0)
	          bEnd = TRUE;

                if (++copied == maxlen)
	          goto done_copied;
	      }
	    }
	    ptr++;
	    break;
	}
	break;
    }
  }

  /*
   * this is our jumppoint, we use a goto for cleaner code,
   * some people may argue that one should never use goto's
   * while others will argue that refusing to use goto's no
   * matter what, can result in code that's horrible to read.
   */
 done_copied:
  *buf = '\0';

  /* if the output was truncated, we return -1 */
  if (*ptr != '\0' && (*(++ptr) != '\0' || !bEnd))
    copied = -1;

  /* clean up */
  va_end(ap);

  /* return how much we copied */
  return copied;
}

/*
 * This nifty little function calculates the length of a
 * string without the color tags. If you use other tags
 * than those mentioned here, then you should add them.
 */
int collen(const char *str)
{
  int len = 0;

  while (*str != '\0')
  {
    int i = 0, j = 0;
    bool found = FALSE;

    switch(*str)
    {
      default:
	len++, str++;
	break;
      case COLOR_TAG:
	str++;
	while (ANSI_STRING[i] != '\0' && !found)
	{
	  if (ANSI_STRING[i] == *str)
	  {
	    str++;
            found = TRUE;
	  }
	  i++;
	}
	while (REPLACE_STRING[j] != '\0' && !found)
	{
	  if (REPLACE_STRING[j] == *str)
	  {
	    len++, str++;
            found = TRUE;
	  }
	  j++;
	}
	if (!found)
          len++;
	break;
    }
  }

  return len;
}

/*
 * This nifty little function will return the
 * longest possible prefix of 'str' that can
 * be displayed in 'size' characters on a mud
 * client. (ie. it ignores ansi chars).
 */
char *string_restrict(char *str, int size)
{
  static char buf[MAX_STRING_LENGTH] = { '\0' };
  char *ptr = buf;
  int len = 0;
  bool done = FALSE;

  /* no size restrictions, we just return the string */
  if (size == 0)
    return str;

  while (*str != '\0' && !done)
  {
    int i = 0, j = 0;
    bool found = FALSE;

    switch(*str)
    {
      default:
	if (++len > size)
	{
	  done = TRUE;
	  break;
	}
	*ptr++ = *str++;
	break;
      case COLOR_TAG:
	str++;
	while (ANSI_STRING[i] != '\0' && !found)
	{
	  if (ANSI_STRING[i] == *str)
	  {
	    *ptr++ = COLOR_TAG;
	    *ptr++ = *str++;
            found = TRUE;
	  }
	  i++;
	}
	while (REPLACE_STRING[j] != '\0' && !found)
	{
	  if (REPLACE_STRING[j] == *str)
	  {
            if (++len > size)
            {
              done = TRUE;
	      break;
            }
	    *ptr++ = COLOR_TAG;
	    *ptr++ = *str++;
            found = TRUE;
	  }
	  j++;
	}
	if (!found)
	{
          if (++len > size)
          {
            done = TRUE;
	    break;
          }
          *ptr++ = COLOR_TAG;
	}
	break;
    }
  }
  *ptr = '\0';

  return buf;
}
