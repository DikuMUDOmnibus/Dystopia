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

#define _XOPEN_SOURCE
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#include "dystopia.h"
#include "olc.h"

ROOM_INDEX_DATA *create_next_room    ( ROOM_INDEX_DATA *toRoom, int dir );
MOB_INDEX_DATA  *next_kingdom_mobile ( KINGDOM_DATA *kingdom, int vnum );

bool  event_object_cauldron  ( EVENT_DATA *event );
bool  is_valid_for_quest     ( CHAR_DATA *ch );
bool  questgroup_present     ( CHAR_DATA *ch );
char *get_rand_qw6           ( void );
char *get_rand_qw7           ( void );
char *get_rand_qw8           ( void );
int   get_next_hall          ( void );
void  load_building          ( FILE *fp, KINGDOM_DATA *kingdom );
char *getKingdomRank         ( int might );
int   count_mobs             ( KINGDOM_DATA *kingdom, int vnum );
int   count_rooms            ( KINGDOM_DATA *kingdom );
int   count_traps            ( KINGDOM_DATA *kingdom );

LIST          * kingdom_list         =  NULL;
KINGDOM_QUEST * the_kingdom_quest    =  NULL;
STACK         * member_free          =  NULL;

#ifdef KEY
#undef KEY
#endif
#define KEY(literal, field, value) \
{                                  \
  if (!str_cmp(word, literal))     \
  {                                \
    field = value;                 \
    found = TRUE;                  \
    break;                         \
  }                                \
}

char *questword_6[] =
{
  "advert",
  "afford",
  "asylum",
  "benign",
  "binder",
  "border",
  "cactus",
  "camera",
  "climax",
  "delphi",
  "demise",
  "divine",
  "engage",
  "entity",
  "excite",
  "female",
  "finale",
  "french",
  "goblin",
  "golden",
  "gringo",
  "hardly",
  "hangup",
  "hermes",
  "ignore",
  "import",  
  "indeed",

  /* END */
  ""
};

char *questword_7[] =
{
  "admirer",
  "animate",
  "aspirin",
  "belgium",
  "benefit",
  "berserk",
  "caltrop",
  "capital",
  "carcass",
  "defunct",
  "disrupt",
  "dresser",
  "emerald",
  "embrace",
  "enforce",
  "faculty",
  "failure",
  "frantic",
  "general",
  "gesture",
  "ginseng",
  "herring",
  "hunting",
  "hydrate",
  "illness",
  "improve",
  "inflate",

  /* END */
  ""
};

char *questword_8[] =
{
  "ambition",
  "assemble",
  "advocate",
  "befriend",
  "blanking",
  "bulgaria",
  "champion",
  "claymore",
  "coherent",
  "deformed",
  "delegate",
  "dementia",
  "electron",
  "eternity",
  "evermore",
  "flagship",
  "forecast",
  "freshman",
  "gargoyle",
  "girlhood",
  "graduate",
  "hesitate",
  "hellfire",
  "horsefly",
  "ignition",
  "illusion",
  "intrigue",

  /* END */
  ""
};


const struct flag_type kingdom_flags[] =
{
  { "canoutcast", KFLAG_CANOUTCAST,  TRUE  },
  { "caninvite",  KFLAG_CANINVITE,   TRUE  },
  { "canbuy",     KFLAG_CANBUY,      TRUE  },
  { "",           0,                 0     }
};

char *get_rand_qw6()
{
  int size, i;

  for (size = 0; questword_6[size][0] != '\0'; size++)
    ;

  i = number_range(0, size - 1);

  return (str_dup(questword_6[i]));
}

char *get_rand_qw7()
{
  int size, i;

  for (size = 0; questword_7[size][0] != '\0'; size++)
    ;

  i = number_range(0, size - 1);

  return (str_dup(questword_7[i]));
}

char *get_rand_qw8()
{
  int size, i;

  for (size = 0; questword_8[size][0] != '\0'; size++)
    ;

  i = number_range(0, size - 1);

  return (str_dup(questword_8[i]));
}

int count_mobs(KINGDOM_DATA *kingdom, int vnum)
{
  ROOM_INDEX_DATA *pRoom;
  RESET_DATA *pReset;
  ITERATOR *pIter;
  int i, count = 0;

  for (i = kingdom->vnums; i < kingdom->vnums + 100; i++)
  {
    if ((pRoom = get_room_index(i)) != NULL)
    {
      pIter = AllocIterator(pRoom->resets);
      while ((pReset = (RESET_DATA *) NextInList(pIter)) != NULL)
      {
        if (pReset->command == 'M' && pReset->arg1 == vnum)
          count++;
      }
    }
  }

  return count;
}

int count_traps(KINGDOM_DATA *kingdom)
{
  ROOM_INDEX_DATA *pRoom;
  int i, count = 0;

  for (i = kingdom->vnums; i < kingdom->vnums + 100; i++)
  {
    if ((pRoom = get_room_index(i)) != NULL)
    {
      if (IS_SET(pRoom->room_flags, ROOM_BLADE_BARRIER))
        count++;
      if (IS_SET(pRoom->room_flags, ROOM_DISPEL_MAGIC))
        count++;
    }
  }

  return count;
}

int count_rooms(KINGDOM_DATA *kingdom)
{
  ROOM_INDEX_DATA *pRoom;
  int i, count = 0;

  for (i = kingdom->vnums; i < kingdom->vnums + 100; i++)
  {
    if ((pRoom = get_room_index(i)) != NULL)
      count++;
  }

  return count;
}

int getKingdomMight(KINGDOM_DATA *kingdom)
{
  MEMBER_DATA *member;
  ITERATOR *pIter;
  int mighties[5];
  int i, low = 0;

  /* reset counters */
  for (i = 0; i < 5; i++)
    mighties[i] = 0;

  pIter = AllocIterator(kingdom->members);
  while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
  {
    if (member->might > mighties[low])
    {
      mighties[low] = member->might;

      /* find the new lowest might rate */
      for (i = 0; i < 5; i++)
      {
        if (mighties[i] < mighties[low])
          low = i;
      }
    }
  }

  return ((mighties[0] + mighties[1] + mighties[2] + mighties[3] + mighties[4]) / 5);
}

/* This is a really nasty bit of code, since we rely
 * on a copyover to clean up our mess (actually, calling
 * this function should always be followed by save_kingdoms()
 * and then copyover() )
 */
void delete_kingdom(KINGDOM_DATA *kingdom)
{
  AREA_DATA *area;
  ROOM_INDEX_DATA *pRoom;
  ITERATOR *pIter;

  /* need to remove the portals leading to the kingdom, if any */
  if ((pRoom = get_room_index(kingdom->entry)) != NULL)
  {
    RESET_DATA *pReset;

    pIter = AllocIterator(pRoom->resets);
    while ((pReset = (RESET_DATA *) NextInList(pIter)) != NULL)
    {
      if (pReset->command == 'O' && pReset->arg1 == kingdom->vnums + KVNUM_PORTAL)
      {
        DetachAtIterator(pIter);
        free_reset_data(pReset);
        break;
      }
    }
  }

  /* remove from list */
  DetachFromList(kingdom, kingdom_list);

  /* now unlink the kingdom area */
  pIter = AllocIterator(area_list);
  while ((area = (AREA_DATA *) NextInList(pIter)) != NULL)
  {
    if (area->lvnum == kingdom->vnums)
    {
      DetachAtIterator(pIter);
      break;
    }
  }
}

/* kingdom member    ::   5 goldcrowns
 * kingdom structure ::  10 goldcrowns
 * kingdom room      ::   5 goldcrowns
 * kingdom retainer  ::  25 goldcrowns
 */
int get_kingdom_upkeep(KINGDOM_DATA *kingdom)
{
  ITERATOR *pIter;
  int cost = 0, vnum;

  cost += SizeOfList(kingdom->members) * 5;

  for (vnum = kingdom->vnums; vnum < kingdom->vnums + 100; vnum++)
  {
    ROOM_INDEX_DATA *pRoom;
    RESET_DATA *pReset;

    if ((pRoom = get_room_index(vnum)) == NULL)
      continue;

    pIter = AllocIterator(pRoom->resets);
    while ((pReset = (RESET_DATA *) NextInList(pIter)) != NULL)
    {
      if (pReset->command == 'M')
        cost += 25;
    }

    cost += 5;
  }

  cost +=  SizeOfList(kingdom->buildings) * 10;
  cost += 10 * count_traps(kingdom);

  return cost;
}

void clear_kingdom(KINGDOM_DATA *kingdom)
{
  static KINGDOM_DATA kingdom_clear;

  *kingdom = kingdom_clear;

  /* reset kingdom data */
  kingdom->kingid       =  0;
  kingdom->pkills       =  0;
  kingdom->pdeaths      =  0;
  kingdom->taxrate      =  0;
  kingdom->treasury     =  0;
  kingdom->vnums        =  0;
  kingdom->flags        =  0;
  kingdom->king_active  =  0;
  kingdom->file         =  str_dup("");
  kingdom->longname     =  str_dup("");
  kingdom->whoname      =  str_dup("");
  kingdom->leader       =  str_dup("");
  kingdom->prefix       =  str_dup("");
  kingdom->suffix       =  str_dup("");  
  kingdom->shortname    =  str_dup("");
  kingdom->members      =  AllocList();
  kingdom->invited      =  AllocList();
  kingdom->buildings    =  AllocList();
}

bool member_of_other_kingdom(CHAR_DATA *ch, KINGDOM_DATA *kingdom)
{
  ACCOUNT_DATA *account;
  KINGDOM_DATA *king;

  /* actually a member of another kingdom ? */
  if ((king = get_kingdom(ch)) != NULL && king != kingdom)
    return TRUE;

  /* not online? sigh... */
  if (ch->desc == NULL)
    return FALSE;

  /* perhaps some other account char is an active member */
  if ((account = ch->desc->account) != NULL)
  {
    char player[MAX_INPUT_LENGTH];
    char *ptr = account->players;

    while (*ptr != '\0')
    {
      ptr = get_token(ptr, player);

      if (strlen(player) >= 3)
      {
        if ((king = get_kingdom2(player)) != NULL && king != kingdom)
          return TRUE;
      }

      /* scan forward to next character */
      ptr = get_token(ptr, player);
      ptr = get_token(ptr, player);
      ptr = get_token(ptr, player);
    }
  }

  return FALSE;
}

/* DIKU'ish load style - very practical */
void load_kingdoms()
{
  FILE *fp, *fpList;
  KINGDOM_DATA *kingdom;
  MEMBER_DATA *member;
  char buf[MAX_STRING_LENGTH];
  char fchar[MAX_STRING_LENGTH];
  char *word;
  bool found, done;

  log_string("Loading all kingdoms.");

  if ((fpList = fopen("../txt/kingdom.lst", "r")) == NULL)
  {
    bug("../txt/kingdom.lst does not exist.", 0);
    return;
  }

  for (;;)
  {
    strcpy(fchar, fread_word(fpList));

    if (fchar[0] == '\0' || !str_cmp(fchar, "END"))
    {
      log_string("All kingdoms loaded.");
      fclose(fpList);
      return;
    }

    if ((fp = fopen(fchar, "r")) == NULL)
    {
      sprintf(buf, "%s does not exist.", fchar);
      bug(buf, 0);
      continue;
    }

    /* allocate and clear this kingdom for use */
    kingdom = calloc(1, sizeof(*kingdom));

    /* reset kingdom data */
    clear_kingdom(kingdom);

    done = FALSE;
    while (!done)
    {
      word = feof(fp) ? "End" : fread_word(fp);
      found = FALSE;

      switch (UPPER(word[0]))
      {
        case 'B':
          if (!str_cmp(word, "BUILDING"))
          {
            load_building(fp, kingdom);
            found = TRUE;
          }
          break;
        case 'E':
          KEY("ENTRY", kingdom->entry, fread_number(fp));
          if (!str_cmp(word, "End"))
          {
            AttachToList(kingdom, kingdom_list);
            found = TRUE;
            done = TRUE;
          }
          break;
        case 'F':
          KEY("FILE", kingdom->file, fread_string(fp));
          KEY("FLAGS", kingdom->flags, fread_number(fp));
          break; 
        case 'I':
          if (!str_cmp(word, "INVITED"))
          {
            member = alloc_member();

            member->name = fread_string(fp);
            member->invited_by = fread_string(fp);
            member->flags = fread_number(fp);
            member->level = fread_number(fp);

            AttachToList(member, kingdom->invited);
            found = TRUE;
          }
          break;
        case 'K':
          KEY("KINGID", kingdom->kingid, fread_number(fp));
          KEY("KINGACTIVE", kingdom->king_active, fread_number(fp));
          break;
        case 'L':
          KEY("LEADER", kingdom->leader, fread_string(fp));
          KEY("LONGNAME", kingdom->longname, fread_string(fp));
          break;
        case 'M':
          if (!str_cmp(word, "MEMBER"))
          {
            member = alloc_member();

            member->name = fread_string(fp);
            member->invited_by = fread_string(fp);
            member->flags = fread_number(fp);
            member->level = fread_number(fp);
            member->might = fread_number(fp);
            member->pk    = fread_number(fp);
            member->pd    = fread_number(fp);

            AttachToList(member, kingdom->members);
            found = TRUE;
          }
          break;
        case 'P':
          KEY("PKILLS", kingdom->pkills, fread_number(fp));
          KEY("PDEATHS", kingdom->pdeaths, fread_number(fp));
          KEY("PREFIX", kingdom->prefix, fread_string(fp));
          break;
        case 'S':
          KEY("SUFFIX", kingdom->suffix, fread_string(fp));
          KEY("SHORTNAME", kingdom->shortname, fread_string(fp));
          break;
        case 'T':
          KEY("TREASURY", kingdom->treasury, fread_number(fp));
          KEY("TAXRATE", kingdom->taxrate, fread_number(fp));
          break;
        case 'V':
          KEY("VNUMS", kingdom->vnums, fread_number(fp));
          break;
        case 'W':
          KEY("WHONAME", kingdom->whoname, fread_string(fp));
          break;
      }

      if (!found)
      {
        sprintf(buf, "load_kingdom (%s): error at word: %s", fchar, word);
        bug(buf, 0);
        fclose(fp);
        abort();
      }
    }
    fclose(fp);
  }
}

void save_kingdoms()
{
  FILE *fp;
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;

  fp = fopen("../txt/kingdom.lst", "w");
  pIter = AllocIterator(kingdom_list);
  while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
  {
    save_kingdom(kingdom);
    fprintf(fp, "../kingdoms/%s.kd\n", kingdom->file);
  }
  fprintf(fp, "END\n");
  fclose(fp);
}

KINGDOM_DATA *vnum_kingdom(int vnum)
{
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;

  pIter = AllocIterator(kingdom_list);
  while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
  {
    if (kingdom->vnums <= vnum && kingdom->vnums + 99 >= vnum)
      return kingdom;
  }

  return NULL;
}

/* doesn't really do that much as of yet */
void do_kedit(CHAR_DATA *ch, char *argument)
{
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;
  char king[MAX_INPUT_LENGTH];
  char field[MAX_INPUT_LENGTH];
  char value[MAX_INPUT_LENGTH];

  if (str_cmp(ch->name, "<your name here>"))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  argument = one_argument(argument, king);
  argument = one_argument(argument, field);
  argument = one_argument(argument, value);

  if (king[0] == '\0')
  {
    send_to_char("Edit what kingdom?\n\r", ch);
    return;
  }

  pIter = AllocIterator(kingdom_list);
  while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(kingdom->shortname, king))
      break;
  }
  if (kingdom == NULL)
  {
    send_to_char("There is no kingdom by that name.\n\r", ch);
    return;
  }

  if (!str_cmp(field, "member"))
  {
    if (char_exists(value))
    {
      MEMBER_DATA *member = alloc_member();

      value[0] = UPPER(value[0]);

      member->name = str_dup(value);
      member->invited_by = str_dup("Original Member");
      AttachToList(member, kingdom->members);
      save_kingdom(kingdom);
      send_to_char("Ok.\n\r", ch);
    }
    else
    {
      send_to_char("That person does not exist.\n\r", ch);
    }
  }
  else if (!str_cmp(field, "king"))
  {
    if (char_exists(value))
    {
      value[0] = UPPER(value[0]);
      free_string(kingdom->leader);
      kingdom->leader = str_dup(value);
      save_kingdom(kingdom);
      send_to_char("Ok.\n\r", ch);
    }
    else
    {
      send_to_char("That person does not exist.\n\r", ch);
    }
  }
  else send_to_char("Syntax Error!\n\r", ch);
}

void save_kingdom(KINGDOM_DATA *kingdom)
{
  MEMBER_DATA *member;
  KINGDOM_STRUCTURE *building;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];
  char strSave[MAX_STRING_LENGTH];
  FILE *fp;

  sprintf(strSave, "../kingdoms/%s.kd", kingdom->file);
  if ((fp = fopen(strSave, "w")) == NULL)
  {
    sprintf(buf, "Cannot save file '%s'.", strSave);
    bug(buf, 0);
    return;
  }

  fprintf(fp, "FILE         %s~\n", kingdom->file);
  fprintf(fp, "WHONAME      %s~\n", kingdom->whoname);
  fprintf(fp, "LONGNAME     %s~\n", kingdom->longname);
  fprintf(fp, "SHORTNAME    %s~\n", kingdom->shortname);
  fprintf(fp, "KINGID       %d\n",  kingdom->kingid);
  fprintf(fp, "LEADER       %s~\n", kingdom->leader);
  fprintf(fp, "PKILLS       %d\n",  kingdom->pkills);
  fprintf(fp, "PDEATHS      %d\n",  kingdom->pdeaths);
  fprintf(fp, "PREFIX       %s~\n", kingdom->prefix);
  fprintf(fp, "SUFFIX       %s~\n", kingdom->suffix);
  fprintf(fp, "VNUMS        %d\n",  kingdom->vnums);
  fprintf(fp, "TREASURY     %d\n",  kingdom->treasury);
  fprintf(fp, "TAXRATE      %d\n",  kingdom->taxrate);
  fprintf(fp, "FLAGS        %d\n",  kingdom->flags);
  fprintf(fp, "KINGACTIVE   %d\n",  kingdom->king_active);
  fprintf(fp, "ENTRY        %d\n",  kingdom->entry);

  fprintf(fp, "\n");
  pIter = AllocIterator(kingdom->members);
  while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
  {
    fprintf(fp, "MEMBER       %s~ %s~ %d %d %d %d %d\n",
      member->name, member->invited_by, member->flags, member->level, member->might, member->pk, member->pd);
  }

  fprintf(fp, "\n");
  pIter = AllocIterator(kingdom->invited);
  while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
  {
    fprintf(fp, "INVITED      %s~ %s~ %d %d\n",
      member->name, member->invited_by, member->flags, member->level);
  }

  fprintf(fp, "\n");
  pIter = AllocIterator(kingdom->buildings);
  while ((building = (KINGDOM_STRUCTURE *) NextInList(pIter)) != NULL)
  {
    fprintf(fp, "BUILDING     %d %d %d %d %d %d\n",
      building->type, building->vnum, building->values[0],
      building->values[1], building->values[2], building->values[3]);
  }

  fprintf(fp, "\nEND\n");
  fclose(fp);
}

char *get_kingdomname(int kingid)
{
  static char buf[MAX_STRING_LENGTH];
  ITERATOR *pIter;
  KINGDOM_DATA *kingdom;

  pIter = AllocIterator(kingdom_list);
  while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
  {
    if (kingdom->kingid == kingid)
    {
      sprintf(buf, "%s", kingdom->whoname);
      return buf;
    }
  }

  sprintf(buf, " ");
  return buf;
}

void kingdom_look(CHAR_DATA *ch)
{
  char buf[MAX_STRING_LENGTH];
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter, *pIter2;

  pIter = AllocIterator(kingdom_list);
  while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
  {
    if (ch->in_room->vnum >= kingdom->vnums && ch->in_room->vnum <= kingdom->vnums + 99)
    {
      KINGDOM_STRUCTURE *building;

      pIter2 = AllocIterator(kingdom->buildings);
      while ((building = (KINGDOM_STRUCTURE *) NextInList(pIter2)) != NULL)
      {
        if (building->vnum != ch->in_room->vnum) continue;

        switch(building->type)
        {
          default:
            break;
          case KSTRUCT_CAULDRON:
            send_to_char(" [-] A simmering black cauldron stands here.\n\r", ch);
            break;
          case KSTRUCT_BALLISTA:
            sprintf(buf, " [-] %d ballista%s stands here.\n\r",
              building->values[0], (building->values[0] != 1) ? "s" : "");
            send_to_char(buf, ch);
            break;
        }
      }

      return;
    }
  }
}

void do_kreload(CHAR_DATA *ch, char *arguments)
{
  KINGDOM_STRUCTURE *building;
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;

  if (IS_NPC(ch)) return;
  if ((kingdom = get_kingdom(ch)) == NULL)
  {  
    send_to_char("You are not a member of any kingdom.\n\r", ch);
    return;
  }
  if (!in_kingdom_hall(ch))
  {
    send_to_char("You are not even in your kingdom halls.\n\r", ch);
    return;
  }

  pIter = AllocIterator(kingdom->buildings);
  while ((building = (KINGDOM_STRUCTURE *) NextInList(pIter)) != NULL)
  {
    if (building->vnum == ch->in_room->vnum && building->type == KSTRUCT_BALLISTA)
      break;
  }

  if (building == NULL)
  {
    send_to_char("There is no ballistas in this room.\n\r", ch);
    return;
  }
  if (building->values[1] > 0)
  {
    send_to_char("The ballista yard is already loaded and ready to fire.\n\r", ch);
    return;
  }
  building->values[1] = building->values[0];

  act("$n reloads the ballista yard.", ch, NULL, NULL, TO_ROOM);
  send_to_char("You reload the ballista yard.\n\r", ch);

  WAIT_STATE(ch, 12);
}

void do_kballista(CHAR_DATA *ch, char *argument)
{
  KINGDOM_STRUCTURE *building;
  KINGDOM_DATA *kingdom;
  CHAR_DATA *victim;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];
  int dam;

  if (IS_NPC(ch)) return;
  if ((kingdom = get_kingdom(ch)) == NULL)
  {
    send_to_char("You are not a member of any kingdom.\n\r", ch);
    return;
  }
  if (!in_kingdom_hall(ch))
  {
    send_to_char("You are not even in your kingdom halls.\n\r", ch);
    return;
  }

  pIter = AllocIterator(kingdom->buildings);
  while ((building = (KINGDOM_STRUCTURE *) NextInList(pIter)) != NULL)
  {
    if (building->vnum == ch->in_room->vnum && building->type == KSTRUCT_BALLISTA)
      break;
  }

  if (building == NULL)
  {
    send_to_char("There is no ballistas in this room.\n\r", ch);
    return;
  }
  if (building->values[1] <= 0)
  {
    send_to_char("You cannot find any unloaded ballistas.\n\r", ch);
    return;
  }

  if (IS_SET(kingdom->flags, KINGFLAG_STRIKE))
  {
    send_to_char("The ballista doesn't seem to be working.\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0' && ch->fighting == NULL)
  {
    send_to_char("Whom do you wish to fire the ballista at?\n\r", ch);
    return;
  }
  else if (arg[0] == '\0' && (victim = ch->fighting) != NULL)
    ;
  else if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  dam = number_range(1500, 2500);
  if (IS_AFFECTED(victim, AFF_SANCTUARY))
    dam /= 2;

  do
  {
    damage(ch, victim, NULL, dam, gsn_ballista);

    dam *= 9;
    dam /= 10;
  } while (--building->values[1] > 0);
}

void do_kmix(CHAR_DATA *ch, char *argument)
{
  KINGDOM_STRUCTURE *building;
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;
  OBJ_DATA *obj;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) return;
  if ((kingdom = get_kingdom(ch)) == NULL)
  {
    send_to_char("You are not a member of any kingdom.\n\r", ch);
    return;
  }
  if (!in_kingdom_hall(ch))
  {
    send_to_char("You are not even in your kingdom halls.\n\r", ch);
    return;
  }

  pIter = AllocIterator(kingdom->buildings);
  while ((building = (KINGDOM_STRUCTURE *) NextInList(pIter)) != NULL)
  {
    if (building->vnum == ch->in_room->vnum && building->type == KSTRUCT_CAULDRON)
      break;
  }

  if (building == NULL)
  {
    send_to_char("There is no cauldron in this room.\n\r", ch);
    return;
  }

  if (IS_SET(kingdom->flags, KINGFLAG_STRIKE))
  {
    send_to_char("The cauldron doesn't seem to be working.\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Syntax: kmix [item|check|flush]\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "check"))
  {
    printf_to_char(ch, "The cauldron contains #C%d#n items of average #C%d#n value.\n\r",
      building->values[0], building->values[1] / UMAX(1, building->values[0]));
    return;
  }

  if (!str_cmp(arg, "flush"))
  {
    building->values[0] = 0;
    building->values[1] = 0;
    save_kingdom(kingdom);
    send_to_char("You flush the contents of the cauldron.\n\r", ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You cannot find that item.\n\r", ch);
    return;
  }
  if (!IS_OBJ_STAT(obj, ITEM_RARE) && !IS_OBJ_STAT(obj, ITEM_SENTIENT))
  {
    send_to_char("You can only mix rare and sentient items.\n\r", ch);
    return;
  }
  if (obj->ownerid != ch->pcdata->playerid)
  {
    send_to_char("You do not own this item.\n\r", ch);
    return;
  }
  if (obj->cost < 25)
  {
    send_to_char("You can only mix items from value 25 and up.\n\r", ch);
    return;
  }

  /* cap big items */
  if (obj->cost > 135)
    obj->cost = 135;

  building->values[0]++;
  building->values[1] += obj->cost;

  act("You throw $p into the black cauldron.", ch, obj, NULL, TO_CHAR);
  act("$n throws $p into the black cauldron.", ch, obj, NULL, TO_ROOM);

  extract_obj(obj);

  if (building->values[0] >= 5)
  {
    EVENT_DATA *event;
    int value = (20 * building->values[1]) / building->values[0];
    int i = number_percent(), minL = 19, maxL = 24;

    /* little randomness */
    if (i < 5) minL = 15;
    if (i > 95) maxL = 28;

    /* 10 tries to pop the right item */
    for (i = 0; i < 10; i++)
    {
      obj = pop_rand_equipment(value, TRUE);

      /* find an item which is from -5% to 20% better than the average being mixed */
      if (obj->cost < (minL * building->values[1]) / (20 * building->values[0]) ||
          obj->cost > (maxL * building->values[1]) / (20 * building->values[0]))
      {
        extract_obj(obj);
      }
      else
      {
        char buf[MAX_INPUT_LENGTH];

        sprintf(buf, "%d", ch->pcdata->playerid);

        event           = alloc_event();
        event->fun      = &event_object_cauldron;
        event->type     = EVENT_OBJECT_CAULDRON;
        event->argument = str_dup(buf);
        add_event_object(event, obj, number_range(3, 5) * PULSE_PER_SECOND);

        building->values[0] = 0;
        building->values[1] = 0;
        save_kingdom(kingdom);

        act("The black cauldron starts boiling...", ch, NULL, NULL, TO_ALL);
        return;
      }
    }
  }

  save_kingdom(kingdom);
}

bool event_object_cauldron(EVENT_DATA *event)
{
  OBJ_DATA *obj;
  CHAR_DATA *ch;

  if ((obj = event->owner.obj) == NULL)
  {
    bug("event_object_cauldron: no owner.", 0);
    return FALSE;
  }

  if ((ch = get_online_player(atoi(event->argument))) == NULL)
  {
    extract_obj(obj);
    return FALSE;
  }

  obj_to_char(obj, ch);

  act("An air elemental appears in a blast of thunder and hands you $p.", ch, obj, NULL, TO_CHAR);
  act("An air elemental appears in a blast of thunder and hands $n $p.", ch, obj, NULL, TO_ROOM);

  return FALSE;
}

char *getKingdomRank(int might)
{
  static char buf[MAX_STRING_LENGTH];

  if (might >= RANK_ALMIGHTY)
    sprintf(buf, "Empire");
  else if (might >= RANK_SUPREME)
    sprintf(buf, "Dynasty");
  else if (might >= RANK_BARON)
    sprintf(buf, "Civilization");
  else if (might >= RANK_GENERAL)
    sprintf(buf, "Nation");
  else if (might >= RANK_MASTER)
    sprintf(buf, "Monarchy");
  else if (might > RANK_HERO)
    sprintf(buf, "City State");
  else if (might >= RANK_VETERAN)   
    sprintf(buf, "Chiefdom");
  else
    sprintf(buf, "Weenies");

  return buf;
}

void do_kwho(CHAR_DATA *ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *gch;
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;
  char temp[MAX_STRING_LENGTH];
  BUFFER *buf;
  char *title;

  if ((kingdom = get_kingdom(ch)) == NULL)
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  buf = buffer_new(MAX_STRING_LENGTH);
  bprintf(buf, " %s\n\n\r", get_dystopia_banner(kingdom->longname, 56));
  bprintf(buf, "   #GName         Health    Mana     Move      Rank\n\r %s\n\n\r",
    get_dystopia_banner("", 56));

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected != CON_PLAYING || (gch = d->character) == NULL)
      continue;

    if (gch->pcdata->kingdom != kingdom->kingid)
      continue;

    if (!str_cmp(kingdom->leader, gch->name))
    {
      if (gch->sex == SEX_FEMALE)
        title = "Queen";
      else
        title = "King";
    }
    else switch(gch->pcdata->kingdomrank)
    {
      default:
        title = "Peasant";
        break;
      case 2:
        title = "Jester";
        break;
      case 3:
        title = "Squire";
        break;
      case 4:
        if (gch->sex == SEX_FEMALE)
          title = "Lady at Arms";
        else
          title = "Man at Arms";
        break;
      case 5:
        title = "Knight";
        break;
      case 6:
        title = "Vassal";
        break;
      case 7:
        if (gch->sex == SEX_FEMALE)
          title = "Duchess";
        else
          title = "Duke";
        break;
      case 8:
        if (gch->sex == SEX_FEMALE)
          title = "Countess";
        else
          title = "Count";
        break;
      case 9:
        if (gch->sex == SEX_FEMALE)
          title = "Baroness";
        else
          title = "Baron";
        break;
      case 10:
        if (gch->sex == SEX_FEMALE)
          title = "Princess";
        else
          title = "Prince";
        break;
    }

    cprintf(temp, "   #G%-12s  %-7s  %-7s  %-7s   #G%-12s#n\n\r",
      can_see(ch, gch) ? gch->name : "someone",
      (gch->hit >= gch->max_hit) ? "#CPerfect#n" : (
      (gch->hit >= 75 * gch->max_hit / 100) ? "#yGood#n" : (
      (gch->hit >= 50 * gch->max_hit / 100) ? "#GFair#n" : (
      (gch->hit >= 25 * gch->max_hit / 100) ? "#LPoor#n" : "#RAwful#n"))),
      (gch->mana >= gch->max_mana) ? "#CPerfect#n" : (
      (gch->mana >= 75 * gch->max_mana / 100) ? "#yGood#n" : (
      (gch->mana >= 50 * gch->max_mana / 100) ? "#GFair#n" : (
      (gch->mana >= 25 * gch->max_mana / 100) ? "#LPoor#n" : "#RAwful#n"))),
      (gch->move >= gch->max_move) ? "#CPerfect#n" : (
      (gch->move >= 75 * gch->max_move / 100) ? "#yGood#n" : (
      (gch->move >= 50 * gch->max_move / 100) ? "#GFair#n" : (
      (gch->move >= 25 * gch->max_move / 100) ? "#LPoor#n" : "#RAwful#n"))),
      title);

    bprintf(buf, "%s", temp);
  }

  bprintf(buf, "\n\r %s\n\r", get_dystopia_banner("", 56));

  send_to_char(buf->data, ch);
  buffer_free(buf);
}

void do_kingdoms(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  ITERATOR *pIter;
  KINGDOM_DATA *kingdom;

  send_to_char(" #0<======> #GKingdom Name #0<========> #GLeader #0<===> #GPK's #0<==> #GPD's #0<====> #GRank #0<===>#n\n\n\r", ch);

  pIter = AllocIterator(kingdom_list);
  while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
  {
    cprintf(buf, " %-30s#n   %-12s %3d       %3d     %-12s\n\r",
      kingdom->longname, kingdom->leader, kingdom->pkills, kingdom->pdeaths, getKingdomRank(getKingdomMight(kingdom)));
    send_to_char(buf, ch);
  }
  send_to_char("\n\r #0<============================================================================>#n\n\r", ch);
}

KINGDOM_DATA *get_kingdom2(char *name)
{
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter, *pIter2;

  pIter = AllocIterator(kingdom_list);
  while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
  {
    MEMBER_DATA *member;

    pIter2 = AllocIterator(kingdom->members);
    while ((member = (MEMBER_DATA *) NextInList(pIter2)) != NULL)
    {
      if (!str_cmp(member->name, name))
        return kingdom;
    }
  }

  return NULL;
}

KINGDOM_DATA *get_kingdom(CHAR_DATA *ch)
{
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;

  if (IS_NPC(ch)) return NULL;

  pIter = AllocIterator(kingdom_list);
  while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
  {
    if (kingdom->kingid == ch->pcdata->kingdom)
      return kingdom;
  }

  return NULL;
}

void free_member(MEMBER_DATA *member)
{
  free_string(member->name);
  free_string(member->invited_by);

  PushStack(member, member_free);
}

MEMBER_DATA *alloc_member()
{
  MEMBER_DATA *member;

  if ((member = (MEMBER_DATA *) PopStack(member_free)) == NULL)
  {
    member = calloc(1, sizeof(*member));
  }

  member->name = str_dup("");
  member->invited_by = str_dup("");
  member->level = 1;
  member->flags = 0;
  member->might = 0;
  member->pd    = 0;
  member->pk    = 0;

  return member;
}

void update_kingdom_membership(CHAR_DATA *ch, bool invite)
{
  KINGDOM_DATA *kingdom;
  MEMBER_DATA *member;
  ITERATOR *pIter, *pIter2;
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
    return;

  /* reset players data */
  ch->pcdata->kingdom       =  0;
  ch->pcdata->kingdomflags  =  0;
  ch->pcdata->kingdomrank   =  0;

  /* check for any invites */
  if (invite)
  {
    pIter = AllocIterator(kingdom_list);
    while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
    {
      pIter2 = AllocIterator(kingdom->invited);
      while ((member = (MEMBER_DATA *) NextInList(pIter2)) != NULL)
      {
        if (!str_cmp(member->name, ch->name))
        {
          sprintf(buf, "You have been invited to join %s#n (%s) by %s.\n\r",
            kingdom->longname, kingdom->shortname, member->invited_by);
          send_to_char(buf, ch);
        }
      }
    }
  }

  /* find membership for this player, and update data */
  pIter = AllocIterator(kingdom_list);
  while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
  {
    pIter2 = AllocIterator(kingdom->members);
    while ((member = (MEMBER_DATA *) NextInList(pIter2)) != NULL)
    {
      if (!str_cmp(member->name, ch->name))
      {
        ch->pcdata->kingdom = kingdom->kingid;
        ch->pcdata->kingdomflags = member->flags;
        ch->pcdata->kingdomrank = member->level;
        return;
      }
    }
  }
}

void do_kdecline(CHAR_DATA *ch, char *argument)
{
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter, *pIter2;
  MEMBER_DATA *member;
  char arg[MAX_INPUT_LENGTH];
         
  one_argument(argument, arg);
            
  if (arg[0] == '\0')
  {
    send_to_char("Decline whose invitation?\n\r", ch);
    return;
  }

  pIter = AllocIterator(kingdom_list);
  while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
  {
    if (str_cmp(kingdom->shortname, arg)) continue;
          
    pIter2 = AllocIterator(kingdom->invited);
    while ((member = (MEMBER_DATA *) NextInList(pIter2)) != NULL)
    {
      if (!str_cmp(ch->name, member->name))
      {
        DetachAtIterator(pIter2);
       
        free_member(member);
        save_kingdom(kingdom);
        send_to_char("You have declined their invitation\n\r", ch);
        return;
      }
    } 
  }
 
  send_to_char("That kingdom have not invited you.\n\r", ch);
}

void load_building(FILE *fp, KINGDOM_DATA *kingdom)
{
  KINGDOM_STRUCTURE *building;

  building             =  calloc(1, sizeof(*building));
  building->type       =  fread_number(fp);
  building->vnum       =  fread_number(fp);
  building->values[0]  =  fread_number(fp);
  building->values[1]  =  fread_number(fp);
  building->values[2]  =  fread_number(fp);
  building->values[3]  =  fread_number(fp);

  AttachToList(building, kingdom->buildings);
}

bool in_kingdom_hall(CHAR_DATA *ch)
{
  KINGDOM_DATA *kingdom;

  if ((kingdom = get_kingdom(ch)) == NULL)
    return FALSE;

  if (ch->in_room == NULL)
    return FALSE;

  if (ch->in_room->vnum < kingdom->vnums || ch->in_room->vnum > kingdom->vnums + 99)
    return FALSE;

  return TRUE;
}

void do_kjoin(CHAR_DATA *ch, char *argument)
{
  KINGDOM_DATA *kingdom;
  MEMBER_DATA *member;
  ITERATOR *pIter, *pIter2;
  bool found = FALSE;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  one_argument(argument, arg);

  if (get_kingdom(ch) != NULL)
  {
    send_to_char("You are already in a kingdom.\n\r", ch);
    return;
  }

  pIter = AllocIterator(kingdom_list);
  while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
  {
    pIter2 = AllocIterator(kingdom->invited);
    while ((member = (MEMBER_DATA *) NextInList(pIter2)) != NULL)
    {
      if (!str_cmp(ch->name, member->name))
      {
        if (!str_cmp(arg, kingdom->shortname))
        {
          if (member_of_other_kingdom(ch, kingdom))
          {
            send_to_char("Your allegiance lies elsewhere.\n\r", ch);
            return;
          }

          DetachAtIterator(pIter2);
          AttachToList(member, kingdom->members);

          save_kingdom(kingdom);
          update_kingdom_membership(ch, FALSE);
          send_to_char("Ok. You have joined a kingdom.\n\r", ch);
          return;
        }
        else if (arg[0] == '\0')
        {
          found = TRUE;
          sprintf(buf, "You have been invited to join %s#n (%s) by %s.\n\r",
            kingdom->longname, kingdom->shortname, member->invited_by);
          send_to_char(buf, ch);
        }
      }
    }
  }

  if (arg[0] == '\0' && !found)
    send_to_char("You have not been invited to join any kingdom.\n\r", ch);
  else if (arg[0] == '\0')
    send_to_char("\n\rTo join a kingdom, simply type kjoin <kingdom name>.\n\r", ch);
  else
    send_to_char("You have not been invited to join that kingdom.\n\r", ch);
}

void do_kleave(CHAR_DATA *ch, char *argument)
{
  KINGDOM_DATA *kingdom;
  MEMBER_DATA *member;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];
   
  if (IS_NPC(ch) || ch->desc == NULL) return;
  if ((kingdom = get_kingdom(ch)) == NULL)
  {
    send_to_char("You are not a member of any kingdom.\n\r", ch);
    return;
  }
  if (!str_cmp(kingdom->leader, ch->name))
  {
    send_to_char("You are the leader of this kingdom.\n\r", ch);
    return;
  }
  if (argument[0] == '\0')
  {
    send_to_char("You must verify this with your password (kleave <password>).\n\r", ch);
    return;
  }
  if (str_cmp(crypt(argument, ch->desc->account->owner), ch->desc->account->password))
  {
    send_to_char("Incorrect Password.\n\r", ch);
    return;
  }
  pIter = AllocIterator(kingdom->members);
  while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(member->name, ch->name))
    {
      DetachAtIterator(pIter);

      free_member(member);
      break;
    }
  }
  save_kingdom(kingdom);
  update_kingdom_membership(ch, FALSE);

  send_to_char("You have left your kingdom.\n\r", ch);

  sprintf(buf, "%s has left the kingdom.\n\n\r"
               "We are sorry to inform this kingdom that the person known as %s\n\r"
               "has chosen to leave this kingdom and seek %s fortune elsewhere.\n\n\r"
               "Regards,\n\n\rThe Kingdom Code.\n\r", ch->name, ch->name,
                (ch->sex == SEX_FEMALE) ? "her" : "his");

  make_note("Personal", "Kingdom Code", kingdom->shortname, "Kingdom Info", 2, buf, 0);
}

void do_koutcast(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  MEMBER_DATA *member;
  ITERATOR *pIter;
  KINGDOM_DATA *kingdom;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) return;
  if ((kingdom = get_kingdom(ch)) == NULL)
  {
    send_to_char("You are not a member of any kingdom.\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->kingdomflags, KFLAG_CANOUTCAST) && str_cmp(ch->name, kingdom->leader))
  {
    send_to_char("You are not allowed to outcast members.\n\r", ch);
    return;
  }
  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Outcast whom from this kingdom.\n\r", ch);
    return;
  }

  pIter = AllocIterator(kingdom->members);
  while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(member->name, arg))
    {
      if (!str_cmp(member->name, ch->name))
      {
        send_to_char("Sorry, it's not possible to outcast yourself.\n\r", ch);
      }
      else if (member->level >= ch->pcdata->kingdomrank && str_cmp(ch->name, kingdom->leader))
      {
        send_to_char("Sorry, they have a much higher level than you.\n\r", ch);
      }
      else if (!str_cmp(member->name, kingdom->leader))
      {
        send_to_char("You cannot outcast the leader of the kingdom.\n\r", ch);
      }
      else
      {
        DetachAtIterator(pIter);

        free_member(member);

        save_kingdom(kingdom);
        if ((victim = get_char_world(ch, arg)) != NULL)
        {
          send_to_char("You have been outcasted from your kingdom.\n\r", victim);
          update_kingdom_membership(victim, FALSE);
        }
        send_to_char("Ok. Player outcasted.\n\r", ch);
      }
      return;
    }
  }

  send_to_char("There are no members by that name.\n\r", ch);
}

void do_kinfo(CHAR_DATA *ch, char *argument)
{   
  BUFFER *buf;
  KINGDOM_DATA *kingdom;
  ROOM_INDEX_DATA *pRoom;

  if (IS_NPC(ch)) return;
  if ((kingdom = get_kingdom(ch)) == NULL)
  {
    send_to_char("You are not a member of any kingdom.\n\r", ch);
    return;
  }
  buf = buffer_new(MAX_STRING_LENGTH);

  bprintf(buf, " %s\n\n\r", get_dystopia_banner("Kingdom Info", 48));
  bprintf(buf, "  #yLeader        #0: #R%s#n\n\r", kingdom->leader);
  bprintf(buf, "  #yName          #0: #R%s#n\n\r", kingdom->longname);
  bprintf(buf, "  #yTreasury      #0: #R%d gold#n\n\r", kingdom->treasury);
  bprintf(buf, "  #yTaxrate       #0: #R%d %%#n\n\r", kingdom->taxrate);
  bprintf(buf, "  #yUpkeep        #0: #R%d gold#n\n\r", get_kingdom_upkeep(kingdom));
  bprintf(buf, "  #yKing Active   #0: #R%d %%#n\n\r",
    (muddata.mudinfo[MUDINFO_UPDATED] / 2)
    ? (100 * kingdom->king_active / 15) / (muddata.mudinfo[MUDINFO_UPDATED] / 2)
    : 100);

  /* check kingdom entry point */
  if (kingdom->entry > 0 && (pRoom = get_room_index(kingdom->entry)) != NULL)
    bprintf(buf, "  #yEntry         #0: %s#n\n\r", pRoom->name);

  bprintf(buf, "  #yPkills        #0: #R%d kill%s#n\n\r",
    kingdom->pkills, (kingdom->pkills == 1) ? "" : "s");
  bprintf(buf, "  #yPdeaths       #0: #R%d death%s#n\n\r",
    kingdom->pdeaths, (kingdom->pdeaths == 1) ? "" : "s");

  bprintf(buf, "\n\r %s\n\r", get_dystopia_banner("", 48));

  send_to_char(buf->data, ch);
  buffer_free(buf);
}

void do_kdonate(CHAR_DATA *ch, char *argument)
{
  KINGDOM_DATA *kingdom;
  char arg[MAX_INPUT_LENGTH];
  int amount;

  if (IS_NPC(ch)) return;
  if ((kingdom = get_kingdom(ch)) == NULL)
  {
    send_to_char("You are not a member of any kingdom.\n\r", ch);
    return;
  }
  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("How much gold will you donate to your kingdom?\n\r", ch);
    return;
  }

  if ((amount = atoi(arg)) < 1 || amount > getGold(ch))
  {
    send_to_char("You cannot donate that many goldcrowns.\n\r", ch);
    return;
  }

  setGold(ch, -1 * amount);
  kingdom->treasury += amount;

  save_kingdom(kingdom);
  send_to_char("Thanks for the donation.\n\r", ch);
}

void do_klist(CHAR_DATA *ch, char *argument)
{
  BUFFER *buf;
  KINGDOM_DATA *kingdom;
  MEMBER_DATA *member;
  ITERATOR *pIter;
  bool found = FALSE;
  int count = 0;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) return;
  if ((kingdom = get_kingdom(ch)) == NULL)
  {
    send_to_char("You are not a member of any kingdom.\n\r", ch);
    return;
  }
  buf = buffer_new(MAX_STRING_LENGTH);
  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    bprintf(buf, "               #G[#yxxxxxxx#G]  #RKingdom Members #G[#yxxxxxxx#G]#n\n\n\r");
    pIter = AllocIterator(kingdom->members);
    while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
    {
      bprintf(buf, " %c%-17s%s",
        UPPER(member->name[0]), &member->name[1], (++count % 4 == 0) ? "\n\r" : "");
    }
    if (count % 4)
      bprintf(buf, "\n\r");

    count = 0;
    pIter = AllocIterator(kingdom->invited);
    while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
    {
      if (!found)
      {
        found = TRUE;
        bprintf(buf, "\n\r               #G[#yxxxxxxx#G]  #RInvited Players #G[#yxxxxxxx#G]#n\n\n\r");
      }
      bprintf(buf, " %c%-17s%s",
        UPPER(member->name[0]), &member->name[1], (++count % 4 == 0) ? "\n\r" : "");
    }
    if (count % 4)
      bprintf(buf, "\n\r");
    bprintf(buf, "\n\r To see the details of a player, type 'klist [playername]'.\n\r");
  }
  else
  {
    pIter = AllocIterator(kingdom->invited);
    while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
    {
      if (!str_cmp(member->name, arg))
      {
        bprintf(buf, " #G[#y++++++#G]  #RKingdom Invitation  #G[#y++++++#G]#n\n\r"
                     "  #yPlayer Name       #R%s#n\n\r" 
                     "  #yInvited by        #R%s#n\n\r",
          member->name, member->invited_by);
        found = TRUE;
        break;
      }
    }
    if (!found)
    {
      pIter = AllocIterator(kingdom->members);
      while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
      {
        if (!str_cmp(member->name, arg))
        {
          bprintf(buf, " #G[#y+++++#G] #RKingdom Membership Card #G[#y+++++#G]#n\n\n\r"
                       "  #yMember Name       #R%s#n\n\r"
                       "  #yInvited by        #R%s#n\n\r"
                       "  #yPkills            #R%d#n\n\r"
                       "  #yPdeaths           #R%d#n\n\r"
                       "  #yKingdom rank      #R%d#n\n\r"
                       "  #yKingdom flags     #R%s#n\n\r",
            member->name, member->invited_by, member->pk, member->pd, member->level,
            flag_string(kingdom_flags, member->flags));
          found = TRUE;
          break;
        }
      }
    }
    if (!found)
    {
      bprintf(buf, "There are no players in this kingdom by that name.\n\r");
    }
  }

  send_to_char(buf->data, ch);
  buffer_free(buf);
}

void do_kuninvite(CHAR_DATA *ch, char *argument)
{
  KINGDOM_DATA *kingdom;
  MEMBER_DATA *member;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) return;
  if ((kingdom = get_kingdom(ch)) == NULL)
  {
    send_to_char("You are not a member of any kingdom.\n\r", ch);
    return;
  }
  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Whom do you wish to remove from the invited list?\n\r", ch);
    return;
  }

  pIter = AllocIterator(kingdom->invited);
  while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(member->name, arg))
    {
      if (str_cmp(member->invited_by, ch->name) && str_cmp(kingdom->leader, ch->name))
      {
        send_to_char("You don't have the rights to uninvite this person.\n\r", ch);
        return;
      }
      DetachAtIterator(pIter);

      free_member(member);
      send_to_char("Invitation removed.\n\r", ch);
      save_kingdom(kingdom);
      return;
    }
  }
  send_to_char("There is noone by that name invited into this kingdom.\n\r", ch);
}

void do_kinvite(CHAR_DATA *ch, char *argument)
{
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;
  MEMBER_DATA *member;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) return;
  if ((kingdom = get_kingdom(ch)) == NULL)
  {
    send_to_char("You are not a member of any kingdom.\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->kingdomflags, KFLAG_CANINVITE) && str_cmp(ch->name, kingdom->leader))
  {
    send_to_char("You are not allowed to invite new members.\n\r", ch);
    return;
  }
  one_argument(argument, arg);  

  if (arg[0] == '\0')
  {  
    send_to_char("Who do you want to invite into your kingdom?\n\r", ch);
    return;
  }
  arg[0] = UPPER(arg[0]);
  if (!char_exists(arg))
  {
    send_to_char("That player does not exist.\n\r", ch);
    return;
  }

  pIter = AllocIterator(kingdom->invited);
  while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(member->name, arg))
    {
      send_to_char("That player has already been invited.\n\r", ch);
      return;
    }
  }
  pIter = AllocIterator(kingdom->members);
  while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(member->name, arg))
    {
      send_to_char("That player is already a member of this kingdom.\n\r", ch);   
      return;
    }
  }

  member = alloc_member();
  member->name = str_dup(arg);
  member->invited_by = str_dup(ch->name);

  AttachToList(member, kingdom->invited);

  save_kingdom(kingdom);
  send_to_char("Invitation send.\n\r", ch);

  /* tell them about their new kingdom */
  if ((victim = get_char_world(ch, arg)) != NULL)
    update_kingdom_membership(victim, TRUE);
}

void do_ktalk(CHAR_DATA *ch, char *argument)
{
  if (get_kingdom(ch) == NULL)
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  talk_channel(ch, argument, CHANNEL_KINGDOM, 0, "ktalk");
}

void do_kset(CHAR_DATA *ch, char *argument)
{
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;
  char field[MAX_STRING_LENGTH];
  char setting[MAX_STRING_LENGTH];

  if (IS_NPC(ch)) return;
  argument = get_token(argument, field);

  if (ch->pcdata->kingdom == 0)
  {
    send_to_char("You are not a member of any kingdom.\n\r", ch);
    return;
  }
  if ((kingdom = get_kingdom(ch)) == NULL)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }
  if (str_cmp(kingdom->leader, ch->name))
  {
    send_to_char("You need to be the king of your kingdom.\n\r", ch);
    return;
  }

  if (field[0] == '\0')
  {
    BUFFER *buf = buffer_new(MAX_STRING_LENGTH);

    bprintf(buf, "         #G[#y++++++#G] #RKingdom Settings #G[#y++++++#G]#n\n\n\r");
    bprintf(buf, " #yprefix#0/#ysuffix #0- #Rset the channel end/start symbols.#n\n\r");
    bprintf(buf, " #ytaxrate       #0- #Rset the taxrate for the kingdom.#n\n\r");
    bprintf(buf, " #ylongname      #0- #Rset the long name for the kingdom.#n\n\r");
    bprintf(buf, " #yshortname     #0- #Rset the short name for the kingdom.#n\n\r");
    bprintf(buf, " #ywhoname       #0- #Rset the who name for the kingdom.#n\n\r");
    bprintf(buf, " #ymember        #0- #Rset the membership information for members.#n\n\r");
    bprintf(buf, "                   #0- #y(#Rrank#y)#n values : 1-10\n\r");
    bprintf(buf, "                   #0- #y(#Rflag#y)#n values : caninvite, canoutcast, canbuy\n\r");
    bprintf(buf, "                     Syntax: kset member [name] [field] [value]\n\r");

    send_to_char(buf->data, ch);
    buffer_free(buf);
  }
  else if (!str_cmp(field, "member"))
  {
    char subfield[MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    MEMBER_DATA *member;
    CHAR_DATA *victim;

    argument = one_argument(argument, name);
    argument = one_argument(argument, subfield);

    if (name[0] == '\0')
    {
      send_to_char("What member do you wish to edit?\n\r", ch);
      return;
    }
    pIter = AllocIterator(kingdom->members);
    while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
    {
      if (!str_cmp(member->name, name))
        break;
    }
    if (!member)
    {
      send_to_char("Sorry, no member by that name.\n\r", ch);
      return;
    }

    if (subfield[0] == '\0')
    {
      send_to_char("What field do you wish to edit?\n\r", ch);
      return;
    }

    if (!str_cmp(subfield, "rank"))
    {
      int rank = atoi(argument);

      if (rank < 1 || rank > 10)
      {
        send_to_char("A players rank should be between 1 and 10.\n\r", ch);
        return;
      }
      member->level = rank;
    }
    else if (!str_cmp(subfield, "flag"))
    {
      char flag[MAX_INPUT_LENGTH];
      bool found = FALSE;
      int i;

      one_argument(argument, flag);
      if (flag[0] == '\0')
      {
        send_to_char("Which flag do you wish to set?\n\r", ch);
        return;
      }
      for (i = 0; kingdom_flags[i].name[0] != '\0'; i++)
      {
        if (!str_cmp(flag, kingdom_flags[i].name))
        {
          found = TRUE;
          TOGGLE_BIT(member->flags, kingdom_flags[i].bit);
        }
      }
      if (!found)
      {
        send_to_char("There is no such flag.\n\r", ch);
        return;
      }
    }
    else
    {
      send_to_char("There is no such field to edit.\n\r", ch);
      return;
    }

    send_to_char("Ok. Field set.\n\r", ch);
    save_kingdom(kingdom);
    if ((victim = get_char_world(ch, name)) != NULL)
      update_kingdom_membership(victim, FALSE);
  }
  else if (!str_cmp(field, "taxrate"))
  {
    int taxrate;

    get_token(argument, setting);
    if ((taxrate = atoi(setting)) < 1 || taxrate > 20)
    {
      send_to_char("Syntax: kset taxrate [1-20]\n\r", ch);
      return;
    }
    kingdom->taxrate = taxrate;
    save_kingdom(kingdom);
    send_to_char("Ok. Taxrate set.\n\r", ch);
  }
  else if (!str_cmp(field, "prefix"))
  {
    get_token(argument, setting);
    if (setting[0] == '\0')
    {
      send_to_char("Set the channel prefix to what?\n\r", ch);
      return;
    }
    smash_tilde(setting);
    if (collen(setting) < 1 || collen(setting) > 4)
    {
      send_to_char("The channel prefix should be between 1 and 4 characters.\n\r", ch);
      return;
    }
    free_string(kingdom->prefix);
    kingdom->prefix = str_dup(setting);
    save_kingdom(kingdom);
    send_to_char("Ok. Prefix set.\n\r", ch);
  }
  else if (!str_cmp(field, "suffix"))
  {
    get_token(argument, setting);
    if (setting[0] == '\0')
    {
      send_to_char("Set the channel suffix to what?\n\r", ch);
      return;
    }
    smash_tilde(setting);
    if (collen(setting) < 1 || collen(setting) > 4)
    {
      send_to_char("The channel suffix should be between 1 and 4 characters.\n\r", ch);
      return;
    }
    free_string(kingdom->suffix);
    kingdom->suffix = str_dup(setting);
    save_kingdom(kingdom);
    send_to_char("Ok. Suffix set.\n\r", ch);
  }
  else if (!str_cmp(field, "whoname"))
  {
    if (argument[0] == '\0')
    {
      send_to_char("Set the kingdoms whoname to what?\n\r", ch);
      return;
    }
    smash_tilde(argument);
    if (collen(argument) < 6 || collen(argument) > 20)
    {
      send_to_char("The whoname should be between 6 and 20 character.\n\r", ch);
      return;
    }
    free_string(kingdom->whoname);
    kingdom->whoname = str_dup(argument);
    save_kingdom(kingdom);
    send_to_char("Ok. Whoname set.\n\r", ch);
  }
  else if (!str_cmp(field, "longname"))
  {
    if (argument[0] == '\0')
    {
      send_to_char("Set the kingdoms longname to what?\n\r", ch);
      return;
    }
    smash_tilde(argument);
    if (collen(argument) < 10 || collen(argument) > 30)
    {
      send_to_char("The longname should be between 10 and 30 character.\n\r", ch);
      return;
    }
    free_string(kingdom->longname);
    kingdom->longname = str_dup(argument);
    save_kingdom(kingdom);
    send_to_char("Ok. Longname set.\n\r", ch);
  }
  else if (!str_cmp(field, "shortname"))
  {
    KINGDOM_DATA *king;

    get_token(argument, setting);
    if (setting[0] == '\0')
    {
      send_to_char("Set the kingdoms shortname to what?\n\r", ch);
      return;
    }
    smash_tilde(setting);

    if (strlen(setting) < 4 || strlen(setting) > 10)
    {
      send_to_char("The shortname should be between 4 and 10 character.\n\r", ch);
      return;
    }

    pIter = AllocIterator(kingdom_list);
    while ((king = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
    {
      if (king == kingdom)
        continue;

      if (!str_cmp(king->shortname, setting))
      {
        send_to_char("That is an invalid shortname.\n\r", ch);
        return;
      }
    }

    if (char_exists(setting))
    {
      send_to_char("That is an invalid shortname.\n\r", ch);
      return;
    }

    free_string(kingdom->shortname);
    kingdom->shortname = str_dup(setting);
    save_kingdom(kingdom);
    send_to_char("Ok. Shortname set.\n\r", ch);
  }
  else
  {
    do_kset(ch, "");
  }
}

/* This is a ripped one_argument, changed to suit
 * the needs of the kingdom functions.
 */
char *get_token(char *argument, char *token)
{
 char cEnd;

 while (isspace(*argument))
   argument++;

  cEnd = ' ';
  if (*argument == '\'' || *argument == '"')
    cEnd = *argument++;

  while (*argument != '\0')
  {
    if (*argument == cEnd)
    {
      argument++;
      break;
    }
    *token++ = *argument++;
  }
  *token = '\0';

  while (isspace(*argument))
    argument++;

  return argument;
}

bool is_valid_for_quest(CHAR_DATA *ch)
{
  if (get_kingdom(ch) != NULL)
  {
    return FALSE;
  }
  if (getMight(ch) < RANK_CADET)
  {
    return FALSE;
  }

  return TRUE;
}

KINGDOM_QUEST *get_kingdom_quest()
{
  return (the_kingdom_quest);
}

void clear_kingdom_quest()
{
  free_string(the_kingdom_quest->namelist);
  free_string(the_kingdom_quest->passphrase);
  free(the_kingdom_quest);
  the_kingdom_quest = NULL;
}

void do_kquest(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *gch;
  ITERATOR *pIter;
  KINGDOM_QUEST *kquest;
  char arg[MAX_STRING_LENGTH];

  argument = get_token(argument, arg);

  if (arg[0] == '\0')
  {
    kquest = get_kingdom_quest();
    if (kquest != NULL && is_full_name(ch->name, kquest->namelist))
    {
      do_kquest(ch, "info");
    }
    else
    {
      send_to_char("Type 'kquest gain' to begin a kingdom quest.\n\r", ch);
    }
    return;
  }
  else if (!str_cmp(arg, "gain"))
  {
    char playerlist[MAX_INPUT_LENGTH];
    int count = 1, j = 0, cost = 1000;
    int *temp;

    /* FIX ME - idea
     * We would like some sort of pack dogs to hunt down
     * and attack the players while they try to solve
     * this quest. Each time a packdog dies, it should
     * spawn a new dog, which will continue the hunt.
     *
     * should use the pathfinding system...
     */

    if ((kquest = get_kingdom_quest()) != NULL)
    {
      if (is_full_name(ch->name, kquest->namelist))
        send_to_char("You are in the middle of a kingdom quest already.\n\r", ch);
      else
        send_to_char("Somebody else is in the middle of a kingdom quest.\n\r", ch);
      return;
    }

    if (!is_valid_for_quest(ch))
    {
      send_to_char("You cannot join a kingdom quest.\n\r", ch);
      return;
    }
    if (ch->leader != NULL)
    {
      send_to_char("The leader of the group must initiate the kingdom quest.\n\r", ch);
      return;
    }
    if (getMight(ch) < RANK_HERO)
    {
      send_to_char("You must have at least the rank of HERO to initiate a quest.\n\r", ch);
      return;
    }
    sprintf(playerlist, "%s", ch->name);
    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (IS_NPC(gch)) continue;
      if (gch == ch) continue;
      if (!is_same_group(gch, ch)) continue;
      if (!is_valid_for_quest(gch)) continue;

      count++;
      strcat(playerlist, " ");
      strcat(playerlist, gch->name);
    }
    if (count < 3 || count > 5)
    {
      send_to_char("Your team must be between 3 and 5 valid players.\n\r", ch);
      return;
    }
    if (getGold(ch) < cost)
    {
      send_to_char("You do not have the 1000 goldcrowns needed.\n\r", ch);
      return;
    }
    setGold(ch, -1 * cost);

    kquest                = calloc(1, sizeof(*kquest));
    kquest->time_left     = 30;
    kquest->namelist      = str_dup(playerlist);
    kquest->current_quest = 0;
    kquest->player_count  = count;
    kquest->clues[0]      = '\0';
    the_kingdom_quest     = kquest;

    if (count == 3)
      kquest->passphrase = get_rand_qw6();
    else if (count == 4)
      kquest->passphrase = get_rand_qw7();
    else
      kquest->passphrase = get_rand_qw8();

    /* A dirty trick to create these items in a random order */
    temp = calloc(1, strlen(kquest->passphrase) * sizeof(int));
    while (1)
    {
      int x = number_range(0, strlen(kquest->passphrase) - 1);
      OBJ_DATA *obj;

      /* make sure we only pick each of these once */
      if (temp[x] != 0) continue;
      temp[x] = 1;

      obj = create_object(get_obj_index(OBJ_VNUM_PROTOPLASM), 50);
      obj->wear_flags  = ITEM_TAKE;
      obj->item_type   = ITEM_QUESTCLUE;
      obj->short_descr = str_dup("A questclue");   
      obj->name        = str_dup("questclue clue");
      obj->ownerid     = ch->pcdata->playerid;
      SET_BIT(obj->extra_flags, ITEM_NOLOCATE);
      sprintf(arg, "A questclue with the letter '%c' lies here.", UPPER(kquest->passphrase[x]));
      obj->description = str_dup(arg);
      object_decay(obj, 30 * 60);
      obj_to_room(obj, get_rand_room());

      if (++j >= (int) strlen(kquest->passphrase))
        break;
    }
    free(temp);

    delay_act("A disembodied voice says '#yYou must find all the questclues and discover the secret passphrase.#n'.", ch->in_room, 2);
    delay_act("A disembodied voice says '#yOnce you have discovered the passphrase, you must defeat The Avatar of Memnon.#n'.", ch->in_room, 6);

    return;
  }
  else if (!str_cmp(arg, "info"))
  {
    BUFFER *buf;

    kquest = get_kingdom_quest();
    if (kquest != NULL && is_full_name(ch->name, kquest->namelist))
    {
      buf = buffer_new(MAX_STRING_LENGTH);
      bprintf(buf, "%s\n\n\r", get_dystopia_banner("Kingdom Quest", 64));
      bprintf(buf, " Quest Participants   : %s\n\r", line_indent(kquest->namelist, 24, 64));
      bprintf(buf, " Quests Completed     : %d quests.\n\r", kquest->current_quest);
      bprintf(buf, " Clues Collected      : %s\n\r", kquest->clues);
      bprintf(buf, " Time Left            : %d minutes.\n\r", kquest->time_left);
      bprintf(buf, "\n\r%s\n\r", get_dystopia_banner("", 64));

      send_to_char(buf->data, ch);
      buffer_free(buf);
    }
    else
    {
      send_to_char("You are not undertaking any kingdom quests.\n\r", ch);
    }
    return;
  }
  else if (!str_cmp(arg, "passphrase"))
  {
    kquest = get_kingdom_quest();
    if (kquest == NULL || !is_full_name(ch->name, kquest->namelist))
    {
      send_to_char("You need to be a part of a quest to use clues.\n\r", ch);
      return;
    }
    if (kquest->current_quest < 5)
    {
      send_to_char("You have not found all the clues yet.\n\r", ch);
      return;
    }
    if (str_prefix(ch->name, kquest->namelist))
    {
      send_to_char("You are not the leader of this quest.\n\r", ch);
      return;
    }
    if (argument[0] == '\0')
    {
      send_to_char("What is your guess at the passphrase?\n\r", ch);
      return;
    }
    if (!questgroup_present(ch))
    {
      send_to_char("Your entire questgroup must be preset when uttering the passphrase.\n\r", ch);
      return;
    }
    if (!str_cmp(argument, kquest->passphrase))
    {
      ROOM_INDEX_DATA *pRoom;
      EVENT_DATA *event;

      if ((pRoom = get_room_index(ROOM_VNUM_KINGDOM)) == NULL)
      {
        send_to_char("You have encountered a bug, please report this.\n\r", ch);
        return;
      }
      if (SizeOfList(pRoom->people) > 0)
      {
        send_to_char("The Throne of Power cannot be accessed, please try later.\n\r", ch);
        return;
      }

      pIter = AllocIterator(ch->in_room->people);
      while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
      {
        if (IS_NPC(gch)) continue;
        if (!is_full_name(gch->name, kquest->namelist)) continue;

        act("$n vanishes in a puff of smoke.", gch, NULL, NULL, TO_ROOM);
        char_from_room(gch);
        char_to_room(gch, pRoom, TRUE);
        act("$n appears in a puff of smoke.", gch, NULL, NULL, TO_ROOM);
        do_look(gch, "");
      }

      /* add room event to pRoom which summons the avatar of memnon */
      event              =  alloc_event();
      event->fun         =  &event_room_kingdomquest;
      event->type        =  EVENT_ROOM_KINGDOMQUEST;
      add_event_room(event, pRoom, number_range(2, 3) * PULSE_PER_SECOND);

      delay_act("A disembodied voice says '#yPrepare to fight the great Memnon.#n'.", pRoom, 2);
    }
    else
    {
      delay_act("A disembodied voice says '#yIncorrect! You have failed the quest.#n'.", ch->in_room, 2);
      clear_kingdom_quest();
    }

    return;
  }
  else if (!str_cmp(arg, "useclue"))
  {
    OBJ_DATA *obj;
    char *ptr;

    kquest = get_kingdom_quest();
    if (kquest == NULL || !is_full_name(ch->name, kquest->namelist))
    {
      send_to_char("You need to be a part of a quest to use clues.\n\r", ch);
      return;
    }

    if (argument[0] == '\0')
    {
      send_to_char("What clue do you want to use?\n\r", ch);
      return;
    }
    else if ((obj = get_obj_carry(ch, argument)) == NULL)
    {
      send_to_char("You do not have that clue.\n\r", ch);
      return;
    }
    else if (obj->item_type != ITEM_QUESTCLUE)
    {
      send_to_char("That is not a questclue.\n\r", ch);
      return;
    }

    if ((ptr = strstr(obj->description, "letter")) != NULL)
    {
      int i = 0;

      /* scan to end of clues */
      while (kquest->clues[i] != '\0')
        i++;

      /* copy clue and terminate */
      kquest->clues[i++] = ptr[8];
      kquest->clues[i] = '\0';
    }
    else
    {
      bug("do_kquest: bad clue.", 0);
    }
    extract_obj(obj);

    if (++kquest->current_quest >= (int) strlen(kquest->passphrase))
    {
      delay_act("A disembodied voice says '#yYou have found all the questclues, what is the passphrase?#n'.", ch->in_room, 2);
    }
    else
    {
      delay_act("A disembodied voice says '#yThanks, I'll take that clue.#n'.", ch->in_room, 2);
    }
    return;
  }
  else
  {
    do_kquest(ch, "");
    return; 
  }
}

bool event_room_kingdomquest(EVENT_DATA *event)
{
  CHAR_DATA *leader = NULL;
  CHAR_DATA *gch;
  ROOM_INDEX_DATA *pRoom;
  ITERATOR *pIter;
  KINGDOM_QUEST *kquest;
  CHAR_DATA *pMob;
  MOB_INDEX_DATA *pMobIndex;
  int maxMight = 0;

  if ((pRoom = event->owner.room) == NULL)
  {
    bug("event_room_kingdomquest: no owner.", 0);
    return FALSE;
  }

  if ((kquest = get_kingdom_quest()) == NULL)
  {
    bug("event_room_kingdomquest: no kquest.", 0);
    return FALSE;
  }

  /* remove all non-questors and find the leader */
  pIter = AllocIterator(pRoom->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!is_full_name(gch->name, kquest->namelist))
    {
      char_from_room(gch);
      char_to_room(gch, get_room_index(ROOM_VNUM_CITYCENTER), TRUE);
      do_look(gch, "");
    }

    if (gch->leader == NULL)
      leader = gch;
  }

  /* the leader is missing for some reason */
  if (leader == NULL)
  {
    pIter = AllocIterator(pRoom->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      char_from_room(gch);   
      char_to_room(gch, get_room_index(ROOM_VNUM_CITYCENTER), TRUE);
      do_look(gch, "");
    }

    bug("event_room_kingdomquest: no leader.", 0);
    clear_kingdom_quest();
    return FALSE;
  }

  /* find the strongest player in the team */
  pIter = AllocIterator(pRoom->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    int newmight = getMight(gch);

    if (newmight > maxMight)
      maxMight = newmight;
  }

  /* load the avatar of memnon */
  if ((pMobIndex = get_mob_index(MOB_VNUM_MEMNON)) == NULL)
  {
    bug("event_room_kingdomquest: no Memnon.", 0);
    clear_kingdom_quest();
    return FALSE;
  }

  /* create and store info in prefix... nifty */
  pMob = create_mobile(pMobIndex);
  pMob->level  = 2 * maxMight + (kquest->player_count - 3) * 200;
  pMob->damroll = pMob->level;
  pMob->hitroll = pMob->level;

  char_to_room(pMob, pRoom, TRUE);
  do_say(pMob, "Prepare to die!");
  act("$n goes BERSERK!", pMob, NULL, NULL, TO_ROOM);
  
  pIter = AllocIterator(pRoom->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (pMob == gch)
      continue;

    if (can_see(pMob, gch))
    {
      damage(pMob, gch, NULL, number_range(gch->max_hit / 15, gch->max_hit / 8), TYPE_HIT);
      damage(pMob, gch, NULL, number_range(gch->max_hit / 15, gch->max_hit / 8), TYPE_HIT);
    }
  }

  return FALSE;
}

bool event_game_kingdomquest(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  ITERATOR *pIter;
  KINGDOM_QUEST *kquest;
  EVENT_DATA *newevent;

  /* init new event */
  newevent        = alloc_event();
  newevent->fun   = &event_game_kingdomquest;
  newevent->type  = EVENT_GAME_KINGDOMQUEST;
  add_event_world(newevent, 60 * PULSE_PER_SECOND);

  if ((kquest = get_kingdom_quest()) != NULL)
  {
    if (--kquest->time_left <= 0)
    {
      pIter = AllocIterator(char_list);
      while ((ch = (CHAR_DATA *) NextInList(pIter)) != NULL)
      {
        if (IS_NPC(ch)) continue;
        if (is_full_name(ch->name, kquest->namelist))
        {
          send_to_char("#RYou have failed your kingdom quest.#n\n\r", ch);
        }
      }

      clear_kingdom_quest();
    }
  }

  return FALSE;
}

bool questgroup_present(CHAR_DATA *ch)
{
  KINGDOM_QUEST *kquest;
  ITERATOR *pIter;
  CHAR_DATA *gch;
  int count = 0;

  if ((kquest = get_kingdom_quest()) == NULL)
    return FALSE;

  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (is_full_name(gch->name, kquest->namelist))
      count++;
  }

  return (count == kquest->player_count);
}

void delay_act(char *mess, ROOM_INDEX_DATA *pRoom, int pulses)
{
  EVENT_DATA *event;

  event             = alloc_event();
  event->type       = EVENT_ROOM_ACT;
  event->fun        = &event_room_act;
  event->argument   = str_dup(mess);
  add_event_room(event, pRoom, pulses);
}

bool event_room_act(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  ROOM_INDEX_DATA *pRoom;

  if ((pRoom = event->owner.room) == NULL)
  {
    bug("event_room_act: no owner.", 0);
    return FALSE;
  }

  if ((ch = (CHAR_DATA *) FirstInList(pRoom->people)) != NULL && event->argument != NULL)
  {
    act(event->argument, ch, NULL, NULL, TO_ALL);
  }

  return FALSE;
}

void do_kbuy(CHAR_DATA *ch, char *argument)
{
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) return;

  if (ch->pcdata->kingdom == 0)
  {
    send_to_char("You are not a member of any kingdom.\n\r", ch);
    return;
  }
  if ((kingdom = get_kingdom(ch)) == NULL)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->kingdomflags, KFLAG_CANBUY) && str_cmp(ch->name, kingdom->leader))
  {
    send_to_char("You are not allowed to buy anything for the kingdom.\n\r", ch);
    return;
  }

  if (argument[0] == '\0')
  {
    BUFFER *buf = buffer_new(MAX_STRING_LENGTH);

    bprintf(buf, " %s\n\n\r", get_dystopia_banner("Kingdom Shop", 64));

    if (kingdom->vnums == 0)
    {
      bprintf(buf, "  2000 gold   -  Kingdom Halls\n\r");
    }
    else
    {
      bprintf(buf, "  1000 gold   -  room [north|east|etc]\n\r");
      bprintf(buf, "   250 gold   -  exit [north|east|etc] [room]\n\r");
      bprintf(buf, "   100 gold   -  description\n\r");
      bprintf(buf, "   500 gold   -  entry (set the entry to the kingdom hall)\n\r");
      bprintf(buf, "       free   -  name [new room name]\n\r");
      bprintf(buf, "       free   -  clear [mobs|traps]\n\n\r");

      bprintf(buf, "  2000 gold   -  healer (casts spells)\n\r");
      bprintf(buf, "  2000 gold   -  wizard (casts spells)\n\r");
      bprintf(buf, "  2000 gold   -  guard  (protects the castle)\n\r");
      bprintf(buf, "  5000 gold   -  upgrade-healer (boost your healers)\n\r");
      bprintf(buf, "  5000 gold   -  upgrade-wizard (boost your wizards)\n\r");
      bprintf(buf, "  2000 gold   -  dummy (used for stance/weapon training)\n\r");
      bprintf(buf, "  5000 gold   -  cauldron (used for mixing rare items)\n\r");
      bprintf(buf, "  4000 gold   -  trap [blades|dispel] (buy a trap)\n\r");
      bprintf(buf, "  3500 gold   -  ballista (allows free attack)\n\r");
      bprintf(buf, "  4000 gold   -  teleport (random teleporter)\n\r");
      bprintf(buf, "  5000 gold   -  kingdom-alarm (warns of intruders)\n\r");
    }
    bprintf(buf, "\n\r You have #C%d#n goldcrowns in the treasury.\n\r", kingdom->treasury);
    bprintf(buf, "\n\r %s\n\r", get_dystopia_banner("", 64));

    send_to_char(buf->data, ch);
    buffer_free(buf);
    return;
  }

  if (!str_prefix(argument, "Kingdom Halls") && kingdom->vnums == 0)
  {
    int cost = 2000;

    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough goldcrowns to buy this.\n\r", ch);
      return;
    }

    if ((kingdom->vnums = get_next_hall()) == 0)
    {
      send_to_char("You have found a bug. Please report this.\n\r", ch);
      return;
    }
    kingdom->treasury -= cost;

    char_from_room(ch);
    char_to_room(ch, get_room_index(kingdom->vnums), TRUE);
    send_to_char("Welcome to your new kingdom halls.\n\r", ch);
    save_kingdom(kingdom);
    return;
  }
  else if (kingdom->vnums == 0)
  {
    do_kbuy(ch, "");
    return;
  }

  if (!str_cmp(argument, "entry"))
  {
    RESET_DATA *pReset;
    OBJ_INDEX_DATA *pObj;
    ROOM_INDEX_DATA *pRoom;
    OBJ_DATA *obj;
    int cost = 500;

    if (has_timer(ch))
      return;

    if (ch->in_room->vnum <= 1000 || ch->in_room->vnum >= ROOM_VNUM_KINGDOMHALLS)
    {
      send_to_char("You cannot create an entrypoint here.\n\r", ch);
      return;
    }

    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough goldcrowns to buy this.\n\r", ch);
      return;
    }

    /* remove the old entry - if it exists */
    if ((pRoom = get_room_index(kingdom->entry)) != NULL)
    {
      pIter = AllocIterator(pRoom->resets);
      while ((pReset = (RESET_DATA *) NextInList(pIter)) != NULL)
      {
        if (pReset->command == 'O' && pReset->arg1 == kingdom->vnums + KVNUM_PORTAL)
        {
          DetachAtIterator(pIter);
          free_reset_data(pReset);
          break;
        }
      }

      /* remove the actual portal */
      pIter = AllocIterator(pRoom->contents);
      while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
      {
        if (obj->pIndexData->vnum == kingdom->vnums + KVNUM_PORTAL)
          extract_obj(obj);
      }
    }

    /* pRoom should now point to the first entry in the kingdom halls */
    if ((pRoom = get_room_index(kingdom->vnums)) == NULL)
    {
      send_to_char("Your kingdom hall seems to be missing.\n\r", ch);
      return;
    }

    /* create an instance of the object */
    if ((pObj = get_obj_index(kingdom->vnums + KVNUM_PORTAL)) == NULL)
    {
      /* create and set default values */
      pObj = new_obj_index();
      pObj->vnum = kingdom->vnums + KVNUM_PORTAL;
      pObj->area = pRoom->area;
      pObj->value[0] = kingdom->vnums;
      pObj->item_type = ITEM_PORTAL;

      free_string(pObj->short_descr);
      free_string(pObj->name);
      free_string(pObj->description);
      pObj->short_descr = str_dup("a shimmering gateway");
      pObj->name = str_dup("gateway shimmering");
      pObj->description = str_dup("A shimmering gateway stands here.");

      if (kingdom->vnums + KVNUM_PORTAL > top_vnum_obj)
        top_vnum_obj = kingdom->vnums + KVNUM_PORTAL;

      /* add to the hash buckets */
      AttachToList(pObj, obj_index_hash[(kingdom->vnums + KVNUM_PORTAL) % MAX_KEY_HASH]);
    }

    /* create and put in room */
    obj = create_object(pObj, 50);
    obj_to_room(obj, ch->in_room);

    /* create the reset */
    pReset = new_reset_data();
    pReset->command = 'O';
    pReset->arg1 = kingdom->vnums + KVNUM_PORTAL;
    pReset->arg2 = 0;
    pReset->arg3 = ch->in_room->vnum;
    add_reset(ch->in_room, pReset, 0);

    /* flag both this area and kingdom area as changed */
    SET_BIT(pRoom->area->area_flags, AREA_CHANGED);
    SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);

    kingdom->entry = ch->in_room->vnum;
    send_to_char("Ok.\n\r", ch);

    /* take the money, and save the kingdom */
    kingdom->treasury -= cost;
    save_kingdom(kingdom);
    return;
  }

  /* from now on, the player should be in his kingdom. */
  if (!in_kingdom_hall(ch))
  {
    send_to_char("You need to be inside your kingdom to edit it.\n\r", ch);
    return;
  }
  argument = get_token(argument, arg);

  if (!str_cmp(arg, "room"))
  {
    int cost = 1000;
    ROOM_INDEX_DATA *pRoom;

    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough goldcrowns to buy this.\n\r", ch);
      return;
    }
    if (!str_cmp(argument, "north"))
    {
      if (ch->in_room->exit[DIR_NORTH] != NULL)
      {
        send_to_char("There is already a room in that direction.\n\r", ch);
        return;
      }
      pRoom = create_next_room(ch->in_room, DIR_NORTH);
    }
    else if (!str_cmp(argument, "east"))
    {
      if (ch->in_room->exit[DIR_EAST] != NULL)
      {
        send_to_char("There is already a room in that direction.\n\r", ch);
        return;
      }
      pRoom = create_next_room(ch->in_room, DIR_EAST);
    }
    else if (!str_cmp(argument, "south"))
    {
      if (ch->in_room->exit[DIR_SOUTH] != NULL)
      {
        send_to_char("There is already a room in that direction.\n\r", ch);
        return;
      }
      pRoom = create_next_room(ch->in_room, DIR_SOUTH);
    }
    else if (!str_cmp(argument, "west"))
    {
      if (ch->in_room->exit[DIR_WEST] != NULL)
      {
        send_to_char("There is already a room in that direction.\n\r", ch);
        return;
      }
      pRoom = create_next_room(ch->in_room, DIR_WEST);
    }
    else if (!str_cmp(argument, "up"))
    {
      if (ch->in_room->exit[DIR_UP] != NULL)
      {
        send_to_char("There is already a room in that direction.\n\r", ch);
        return;
      }
      pRoom = create_next_room(ch->in_room, DIR_UP);
    }
    else if (!str_cmp(argument, "down"))
    {
      if (ch->in_room->exit[DIR_DOWN] != NULL)
      {
        send_to_char("There is already a room in that direction.\n\r", ch);
        return;
      }
      pRoom = create_next_room(ch->in_room, DIR_DOWN);
    }
    else
    {
      send_to_char("That is not a valid direction.\n\r", ch);
      return;
    }
    if (pRoom == NULL)
    {
      send_to_char("Something went wrong under the construction.\n\r", ch);
      return;
    }
    kingdom->treasury -= cost;
    save_kingdom(kingdom);
    send_to_char("The room has been created.\n\r", ch);
    return;
  }
  else if (!str_cmp(arg, "trap"))
  {
    int cost = 4000;

    if (count_traps(kingdom) >= count_rooms(kingdom) / 2)
    {
      send_to_char("The kingdom cannot support any more traps.\n\r", ch);
      return;
    }

    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough gold to buy a trap.\n\r", ch);
      return;
    }

    one_argument(argument, arg);
    if (!str_cmp(arg, "blades"))
    {
      EVENT_DATA *event;

      if (IS_SET(ch->in_room->room_flags, ROOM_BLADE_BARRIER))
      {
        send_to_char("This room already has a blade barrier trap.\n\r", ch);
        return;
      }

      SET_BIT(ch->in_room->room_flags, ROOM_BLADE_BARRIER);

      event = alloc_event();
      event->type = EVENT_ROOM_BLADEBARRIER;
      event->fun = &event_room_bladebarrier;
      add_event_room(event, ch->in_room, 10 * PULSE_PER_SECOND);
    }
    else if (!str_cmp(arg, "dispel"))
    {
      EVENT_DATA *event;

      if (IS_SET(ch->in_room->room_flags, ROOM_DISPEL_MAGIC))
      {
        send_to_char("This room already has a dispel magic trap.\n\r", ch);
        return;
      }

      SET_BIT(ch->in_room->room_flags, ROOM_DISPEL_MAGIC);

      event = alloc_event();
      event->type = EVENT_ROOM_DISPEL_MAGIC;
      event->fun = &event_room_dispel_magic;
      add_event_room(event, ch->in_room, 10 * PULSE_PER_SECOND);
    }
    else
    {
      send_to_char("Syntax: kbuy trap [blades|dispel]\n\r", ch);
      return;
    }

    send_to_char("Trap purchased.\n\r", ch);
    SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);
    kingdom->treasury -= cost;
    save_kingdom(kingdom);
  }
  else if (!str_cmp(arg, "teleport"))
  {
    EXTRA_DESCR_DATA *extra;
    KINGDOM_STRUCTURE *building;
    const int cost = 4000;

    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough goldcrowns to buy this.\n\r", ch);
      return;
    }

    /* scan for old teleport room */
    pIter = AllocIterator(kingdom->buildings);
    while ((building = (KINGDOM_STRUCTURE *) NextInList(pIter)) != NULL)
    {
      if (building->vnum == ch->in_room->vnum && building->type == KSTRUCT_TELEPORT)
      {
        send_to_char("This room already has a teleport room.\n\r", ch);
        return;
      }
    }

    building = calloc(1, sizeof(*building));
    building->type = KSTRUCT_TELEPORT;
    building->vnum = ch->in_room->vnum;
    building->values[0] = 0;
    building->values[1] = 0;
    building->values[2] = 0;
    building->values[3] = 0;
    AttachToList(building, kingdom->buildings);

    extra = new_extra_descr();
    extra->keyword = str_dup("a teleport platform");
    extra->description = str_dup("a teleport platform stands on the floor.\n\r");
    extra->buffer1 = str_dup("You dematerialize.");
    extra->buffer2 = str_dup("$n dematerializes.");
    extra->type = ED_TYPE_TOUCH;
    extra->action = ED_ACTION_TELEPORT;
    extra->vnum = 0;

    AttachToList(extra, ch->in_room->extra_descr);

    SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);

    kingdom->treasury -= cost;
    save_kingdom(kingdom);
    send_to_char("A teleport platform has been purchased.\n\r", ch);
    return;
  }
  else if (!str_cmp(arg, "ballista"))
  {
    EXTRA_DESCR_DATA *extra;
    KINGDOM_STRUCTURE *building;
    const int cost = 3500;

    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough goldcrowns to buy this.\n\r", ch);
      return;
    }

    /* scan for old ballista yard */
    pIter = AllocIterator(kingdom->buildings);
    while ((building = (KINGDOM_STRUCTURE *) NextInList(pIter)) != NULL)
    {
      if (building->vnum == ch->in_room->vnum && building->type == KSTRUCT_BALLISTA)
        break;
    }

    /* create a new ballista yard if needed */
    if (!building)
    {
      building = calloc(1, sizeof(*building));
      building->type = KSTRUCT_BALLISTA;
      building->vnum = ch->in_room->vnum;
      building->values[0] = 1;
      building->values[1] = 1;
      building->values[2] = 0;
      building->values[3] = 0;
      AttachToList(building, kingdom->buildings);

      extra = new_extra_descr();
      extra->keyword = str_dup("ballista");
      extra->description = str_dup("a ballista stands on the floor.\n\r");
      extra->buffer1 = str_dup("");
      extra->buffer2 = str_dup("");
      extra->type = ED_TYPE_NONE;
      extra->action = ED_ACTION_NONE;
      extra->vnum = 0;
      AttachToList(extra, ch->in_room->extra_descr);
    }
    else
    {
      if (building->values[0] >= 3)
      {
        send_to_char("You cannot buy any more ballistas for this room.\n\r", ch);
        return;
      }
      building->values[0]++;
      building->values[1]++;

      pIter = AllocIterator(ch->in_room->extra_descr);
      while ((extra = (EXTRA_DESCR_DATA *) NextInList(pIter)) != NULL)
      {
        if (!str_cmp(extra->keyword, "ballista"))
          break;
      }

      if (extra != NULL)
      {
        free_string(extra->description);
        sprintf(arg, "%d ballistas stands on the floor.\n\r", building->values[0]);
        extra->description = str_dup(arg);
      }
      else
      {
        bug("kbuy ballista: missing extra descr in room vnum %d", ch->in_room->vnum);
      }
    }

    SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);

    kingdom->treasury -= cost;
    save_kingdom(kingdom);
    send_to_char("A ballista has been purchased.\n\r", ch);
    return;
  }
  else if (!str_cmp(arg, "kingdom-alarm"))
  {
    int cost = 5000;

    if (IS_SET(kingdom->flags, KINGFLAG_ALARM))
    {
      send_to_char("You already have a kingdom alarm.\n\r", ch);
      return;
    }

    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough goldcrowns to buy this.\n\r", ch);
      return;
    }
    kingdom->treasury -= cost;

    SET_BIT(kingdom->flags, KINGFLAG_ALARM);
    save_kingdom(kingdom);
    send_to_char("A kingdom alarm has been purchased.\n\r", ch);
    return;
  }
  else if (!str_cmp(arg, "cauldron"))
  {
    EXTRA_DESCR_DATA *extra;
    KINGDOM_STRUCTURE *building;
    const int cost = 5000;

    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough goldcrowns to buy this.\n\r", ch);
      return;
    }

    pIter = AllocIterator(kingdom->buildings);
    while ((building = (KINGDOM_STRUCTURE *) NextInList(pIter)) != NULL)
    {
      if (building->vnum == ch->in_room->vnum && building->type == KSTRUCT_CAULDRON)
      {
        send_to_char("You already have a cauldron in this room.\n\r", ch);
        return;
      }
    }

    building = calloc(1, sizeof(*building));
    building->type = KSTRUCT_CAULDRON;
    building->vnum = ch->in_room->vnum;
    building->values[0] = 0;
    building->values[1] = 0;
    building->values[2] = 0;
    building->values[3] = 0;
    AttachToList(building, kingdom->buildings);

    extra = new_extra_descr();
    extra->keyword = str_dup("cauldron");
    extra->description = str_dup("a black cauldron stands on the floor.\n\r");
    extra->buffer1 = str_dup("");
    extra->buffer2 = str_dup("");
    extra->type = ED_TYPE_NONE;
    extra->action = ED_ACTION_NONE;
    extra->vnum = 0;
    AttachToList(extra, ch->in_room->extra_descr);

    SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);

    kingdom->treasury -= cost;
    save_kingdom(kingdom);
    send_to_char("A cauldron has been purchased.\n\r", ch);
    return;
  }
  else if (!str_cmp(arg, "upgrade-healer"))
  {
    CHAR_DATA *pMob;
    MOB_INDEX_DATA *pMobIndex;

    const int cost = 5000;
    int vnum = 0;

    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough goldcrowns.\n\r", ch);
      return;
    }
    vnum += kingdom->vnums / 100;
    vnum *= 100;
    vnum += KVNUM_HEALER;

    if ((pMobIndex = get_mob_index(vnum)) == NULL)
    {
      send_to_char("Your kingdom does not have any healers.\n\r", ch);
      return;
    }

    if (pMobIndex->level > 2500)
    {
      send_to_char("You cannot boost the kingdom healers any more.", ch);
      return;
    }
    pMobIndex->level += 300;

    pIter = AllocIterator(char_list);
    while ((pMob = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (IS_NPC(pMob) && pMob->pIndexData == pMobIndex)
        pMob->level = pMobIndex->level;
    }

    SET_BIT(pMobIndex->area->area_flags, AREA_CHANGED);

    kingdom->treasury -= cost;
    save_kingdom(kingdom);
    send_to_char("Ok. All healers upgraded.\n\r", ch);
    return;
  }
  else if (!str_cmp(arg, "upgrade-wizard"))
  {
    CHAR_DATA *pMob;
    MOB_INDEX_DATA *pMobIndex;
    const int cost = 5000;
    int vnum = 0;

    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough goldcrowns.\n\r", ch);
      return;
    }
    vnum += kingdom->vnums / 100;
    vnum *= 100;
    vnum += KVNUM_WIZARD;

    if ((pMobIndex = get_mob_index(vnum)) == NULL)
    {
      send_to_char("Your kingdom does not have any wizards.\n\r", ch);
      return;
    }

    if (pMobIndex->level > 2500)
    {
      send_to_char("You cannot boost the kingdom wizards any more.", ch);
      return;
    }
    pMobIndex->level += 300;

    pIter = AllocIterator(char_list);
    while ((pMob = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (IS_NPC(pMob) && pMob->pIndexData == pMobIndex)
        pMob->level = pMobIndex->level;
    }

    SET_BIT(pMobIndex->area->area_flags, AREA_CHANGED);

    kingdom->treasury -= cost;
    save_kingdom(kingdom);
    send_to_char("Ok. All wizards upgraded.\n\r", ch);
    return;
  }
  else if (!str_cmp(arg, "guard"))
  {
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pMobIndex;
    const int cost = 2000;
    int count = 0;

    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough goldcrowns.\n\r", ch);
      return;
    }

    /* 1 free guard, and then 1 guard per 3 rooms is the max */
    if (count_mobs(kingdom, KVNUM_GUARD) >= count_rooms(kingdom) / 3 + 1)
    {
      send_to_char("The kingdom cannot support any more guards.\n\r"
                   "You need to build additonal rooms first.\n\r", ch);
      return;
    }

    pIter = AllocIterator(ch->in_room->resets);
    while ((pReset = (RESET_DATA *) NextInList(pIter)) != NULL)
    {
      if (pReset->command != 'M') continue;

      if (++count >= 5)
      {
        send_to_char("No more mobiles are allowed in this room.\n\r", ch);
        return;
      }
    }

    if ((pMobIndex = next_kingdom_mobile(kingdom, KVNUM_GUARD)) == NULL)
    {
      send_to_char("You have encountered a bug, please report this.\n\r", ch);
      return;
    }
    kingdom->treasury -= cost;
    save_kingdom(kingdom);    

    /* set the details of this mobile */
    if (pMobIndex->level == 1)
      pMobIndex->level = 50;
    pMobIndex->death_fun = death_lookup("deathspec_kingdom_guard");
    free_string(pMobIndex->player_name);
    free_string(pMobIndex->short_descr);
    free_string(pMobIndex->long_descr);
    pMobIndex->player_name = str_dup("guard");
    pMobIndex->short_descr = str_dup("the guard");
    pMobIndex->long_descr = str_dup("The ever watchful kingdom guard is standing here.\n\r");
    SET_BIT(pMobIndex->act, ACT_SEMIAGGRESSIVE);

    /* add the reset */
    pReset = new_reset_data();
    pReset->command = 'M';
    pReset->repop = TRUE;
    pReset->arg1 = pMobIndex->vnum;
    pReset->arg2 = 1;
    pReset->arg3 = ch->in_room->vnum;
    add_reset(ch->in_room, pReset, 0);

    /* flag the area as saved */
    SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);

    send_to_char("Ok.\n\r", ch);
    return;
  }
  else if (!str_cmp(arg, "healer"))
  {
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pMobIndex;
    const int cost = 2000;
    int count = 0;

    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough goldcrowns.\n\r", ch);
      return;
    }

    /* 3 free healers, and then 1 healer per room is the max */
    if (count_mobs(kingdom, KVNUM_HEALER) >= count_rooms(kingdom) + 3)
    {
      send_to_char("The kingdom cannot support any more healers.\n\r"
                   "You need to build additonal rooms first.\n\r", ch);
      return;
    }

    pIter = AllocIterator(ch->in_room->resets);
    while ((pReset = (RESET_DATA *) NextInList(pIter)) != NULL)
    {
      if (pReset->command != 'M') continue;

      if (++count >= 5)
      {
        send_to_char("No more mobiles are allowed in this room.\n\r", ch);
        return;
      }
    }

    if ((pMobIndex = next_kingdom_mobile(kingdom, KVNUM_HEALER)) == NULL)
    {
      send_to_char("You have encountered a bug, please report this.\n\r", ch);
      return;
    }
    kingdom->treasury -= cost;
    save_kingdom(kingdom);

    /* set the details of this mobile */
    if (pMobIndex->level == 1)
      pMobIndex->level = 1000;
    pMobIndex->spec_fun = spec_lookup("spec_kingdom_healer");
    free_string(pMobIndex->player_name);
    free_string(pMobIndex->short_descr);
    free_string(pMobIndex->long_descr);
    pMobIndex->player_name = str_dup("healer");
    pMobIndex->short_descr = str_dup("the healer");
    pMobIndex->long_descr = str_dup("The kingdom healer stands here, chanting spells.\n\r");

    /* add the reset */
    pReset = new_reset_data();
    pReset->command = 'M';
    pReset->repop = TRUE;
    pReset->arg1 = pMobIndex->vnum;
    pReset->arg2 = 1;
    pReset->arg3 = ch->in_room->vnum;
    add_reset(ch->in_room, pReset, 0);

    /* flag the area as saved */
    SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);

    send_to_char("Ok.\n\r", ch);
    return;
  }
  else if (!str_cmp(arg, "wizard"))
  {
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pMobIndex;
    const int cost = 2000;
    int count = 0;

    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough goldcrowns.\n\r", ch);
      return;
    }

    /* 3 free wizards, and then 1 wizard per room is the max */
    if (count_mobs(kingdom, KVNUM_WIZARD) >= count_rooms(kingdom) + 3)
    {
      send_to_char("The kingdom cannot support any more wizards.\n\r"
                   "You need to build additonal rooms first.\n\r", ch);
      return;
    }

    pIter = AllocIterator(ch->in_room->resets);
    while ((pReset = (RESET_DATA *) NextInList(pIter)) != NULL)
    {
      if (pReset->command != 'M') continue;
      
      if (++count >= 5)
      {
        send_to_char("No more mobiles are allowed in this room.\n\r", ch);
        return;
      }
    }

    if ((pMobIndex = next_kingdom_mobile(kingdom, KVNUM_WIZARD)) == NULL)
    {
      send_to_char("You have encountered a bug, please report this.\n\r", ch);
      return;
    }
    kingdom->treasury -= cost;
    save_kingdom(kingdom);

    /* set the details of this mobile */
    if (pMobIndex->level == 1)
      pMobIndex->level = 1000;
    pMobIndex->spec_fun = spec_lookup("spec_kingdom_wizard");
    free_string(pMobIndex->player_name);
    free_string(pMobIndex->short_descr);
    free_string(pMobIndex->long_descr);
    pMobIndex->player_name = str_dup("wizard");
    pMobIndex->short_descr = str_dup("the wizard");
    pMobIndex->long_descr = str_dup("The kingdom wizard stands here, chanting spells.\n\r");

    /* add the reset */
    pReset = new_reset_data();
    pReset->command = 'M';
    pReset->repop = TRUE;
    pReset->arg1 = pMobIndex->vnum;
    pReset->arg2 = 1;
    pReset->arg3 = ch->in_room->vnum;
    add_reset(ch->in_room, pReset, 0);

    /* flag the area as saved */
    SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);

    send_to_char("Ok.\n\r", ch);
    return;
  }
  else if (!str_cmp(arg, "dummy"))
  {
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pMobIndex;
    const int cost = 2000;
   
    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough goldcrowns.\n\r", ch);
      return;
    }
    if ((pMobIndex = next_kingdom_mobile(kingdom, KVNUM_DUMMY)) == NULL)
    {
      send_to_char("You have encountered a bug, please report this.\n\r", ch);
      return;
    }
    kingdom->treasury -= cost;
    save_kingdom(kingdom);

    /* set the details of this mobile */
    if (pMobIndex->level == 1)
      pMobIndex->level = 150;
    pMobIndex->toughness = 100;
    free_string(pMobIndex->player_name);
    free_string(pMobIndex->short_descr);
    free_string(pMobIndex->long_descr);
    pMobIndex->player_name = str_dup("practice dummy");
    pMobIndex->short_descr = str_dup("the dummy");
    pMobIndex->long_descr = str_dup("a practice dummy stands here.\n\r");

    /* add the reset */
    pReset = new_reset_data();
    pReset->command = 'M';
    pReset->repop = TRUE;
    pReset->arg1 = pMobIndex->vnum;
    pReset->arg2 = 1;
    pReset->arg3 = ch->in_room->vnum;
    add_reset(ch->in_room, pReset, 0);

    /* flag the area as saved */
    SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);

    send_to_char("Ok.\n\r", ch);
    return;
  }
  else if (!str_cmp(arg, "exit"))
  {
    ROOM_INDEX_DATA *pRoom;
    char buf[MAX_STRING_LENGTH];
    char path[MAX_INPUT_LENGTH];
    int dir, vnum, i;
    int cost = 250;

    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough goldcrowns to buy this.\n\r", ch);
      return;
    }

    argument = one_argument(argument, path);

    if (path[0] == '\0' || argument[0] == '\0' || (vnum = atoi(argument)) <= 0)
    {
      for (i = 0, vnum = ch->in_room->area->lvnum; i < 100; i++)
      {
        if ((pRoom = get_room_index(vnum + i)) == NULL) continue;

        sprintf(buf, " [%5d]  %s\n\r", pRoom->vnum, pRoom->name);
        send_to_char(buf, ch);
      }
      send_to_char("\n\rSyntax: kbuy exit [dir] [room number]\n\r", ch);
      return;
    }
    if (vnum == ch->in_room->vnum || vnum < ch->in_room->area->lvnum ||
        vnum > ch->in_room->area->uvnum || (pRoom = get_room_index(vnum)) == NULL)
    {
      send_to_char("You cannot create an exit to that room.\n\r", ch);
      return;
    }

    if (!str_cmp(path, "north"))
      dir = DIR_NORTH;
    else if (!str_cmp(path, "east"))
      dir = DIR_EAST;
    else if (!str_cmp(path, "south"))
      dir = DIR_SOUTH;
    else if (!str_cmp(path, "west"))
      dir = DIR_WEST;
    else if (!str_cmp(path, "up"))
      dir = DIR_UP;
    else if (!str_cmp(path, "down"))
      dir = DIR_DOWN;
    else
    {
      send_to_char("You can only make exits going south, north, east, west, up and down.\n\r", ch);
      return;
    }

    if (pRoom->exit[rev_dir[dir]] != NULL)
    {
      send_to_char("Target room already has an exit going that way.\n\r", ch);
      return;
    }

    if (ch->in_room->exit[dir] != NULL)
    {
      send_to_char("This room already has an exit going that way.\n\r", ch);
      return;
    }

    /* setup the exit from toRoom to pRoom */
    ch->in_room->exit[dir]             = new_exit();
    ch->in_room->exit[dir]->to_room    = pRoom;
  
    /* setup the exit from pRoom to toRoom */
    pRoom->exit[rev_dir[dir]]          = new_exit();
    pRoom->exit[rev_dir[dir]]->to_room = ch->in_room;

    /* flag the area as changed */
    SET_BIT(pRoom->area->area_flags, AREA_CHANGED);

    kingdom->treasury -= cost;

    send_to_char("Ok.\n\r", ch);
    return;
  }
  else if (!str_cmp(arg, "clear"))
  {
    one_argument(argument, arg);

    if (!str_cmp(arg, "traps"))
    {
      REMOVE_BIT(ch->in_room->room_flags, ROOM_BLADE_BARRIER);
      REMOVE_BIT(ch->in_room->room_flags, ROOM_DISPEL_MAGIC);

      send_to_char("All traps has been removed from this room.\n\r", ch);
    }
    else if (!str_cmp(arg, "mobs"))
    {
      CHAR_DATA *gch;
      RESET_DATA *pReset;

      pIter = AllocIterator(ch->in_room->resets);
      while ((pReset = (RESET_DATA *) NextInList(pIter)) != NULL)
      {
        if (pReset->command == 'M')
        {
          DetachAtIterator(pIter);
          free_reset_data(pReset);
        }
      }

      pIter = AllocIterator(ch->in_room->people);
      while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
      {
        if (IS_NPC(gch))
          extract_char(gch, TRUE);
      }

      send_to_char("All retainers in this room has been fired.\n\r", ch);
    }
    else
    {
      send_to_char("Syntax: kbuy clear [mobs|traps]\n\r", ch);
    }
    return;
  }
  else if (!str_cmp(arg, "name"))
  {
    if (strlen(argument) < 3 || strlen(argument) > 50)
    {
      send_to_char("Room name should be between 3 and 50 characters.\n\r", ch);
      return;
    }
    smash_tilde(argument);

    free_string(ch->in_room->name);
    ch->in_room->name = str_dup(argument);
    SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);

    send_to_char("Ok.\n\r", ch);
    return;
  }
  else if (!str_cmp(arg, "description"))
  {
    int cost = 100;

    if (kingdom->treasury < cost)
    {
      send_to_char("Your kingdom does not have enough goldcrowns to buy this.\n\r", ch);
      return;
    }
    kingdom->treasury -= cost;
    string_append(ch, &ch->in_room->description);
    SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);
    return;
  }
  else
  {
    do_kbuy(ch, "");
    return;
  }
}

int get_next_hall()
{
  EVENT_DATA *event;
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *pRoom;
  ITERATOR *pIter;
  KINGDOM_DATA *kingdom;
  int vnum, id, iHash, door;
  char buf[MAX_INPUT_LENGTH];
  bool found;

  /* find a free vnum range - we do note allow more than 50 kingdoms */
  for (vnum = ROOM_VNUM_KINGDOMHALLS; vnum < (ROOM_VNUM_KINGDOMHALLS + 5000); vnum += 100)
  {
    found = FALSE;

    pIter = AllocIterator(kingdom_list);
    while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
    {
      if (kingdom->vnums == vnum)
        found = TRUE;
    }

    if (!found)
      break;
  }

  /* sorry, we do not allow more kingdoms than this. */
  if (vnum == ROOM_VNUM_KINGDOMHALLS + 5000)
    return 0;

  /* first create and save an area with this range */
  if (aedit_create(NULL, "") == FALSE)
    return 0;

  id = (vnum - ROOM_VNUM_KINGDOMHALLS) / 100 + 1;

  pArea = (AREA_DATA *) LastInList(area_list);

  /* setup area data */
  free_string(pArea->filename);
  free_string(pArea->name);
  sprintf(buf, "king%d.are", id);
  pArea->filename = str_dup(buf);
  sprintf(buf, "Generic   Kingdom Halls %d", id);
  pArea->name = str_dup(buf);
  pArea->lvnum = vnum;
  pArea->uvnum = vnum + 99;
  pArea->cvnum = vnum;

  /* create the first room at vnum */
  pRoom = new_room_index();
  pRoom->area = pArea;
  pRoom->vnum = vnum;

  if (vnum > top_vnum_room)
    top_vnum_room = vnum;

  iHash = vnum % MAX_KEY_HASH;
  AttachToList(pRoom, room_index_hash[iHash]);

  /* reset all doors */
  for (door = 0; door <= 5; door++)
    pRoom->exit[door] = NULL;  

  /* set default values */
  free_string(pRoom->description);
  free_string(pRoom->name);
  pRoom->description = str_dup("The kindom halls are empty.\n");
  pRoom->name = str_dup("The Kingdom Halls");
  SET_BIT(pRoom->room_flags, ROOM_KINGDOM);

  event              =  alloc_event();
  event->fun         =  &event_area_reset;
  event->type        =  EVENT_AREA_RESET;
  add_event_area(event, pArea, number_range(1, 10));

  /* save all areas and area list */
  do_asave(NULL, "changed");

  return vnum;
}

ROOM_INDEX_DATA *create_next_room(ROOM_INDEX_DATA *toRoom, int dir)
{
  ROOM_INDEX_DATA *pRoom;
  int vnum, i, iHash, door;
      
  if (toRoom->exit[dir] != NULL)
  {
    bug("create_next_room: exit already exist at room vnum %d.", toRoom->vnum);
    return NULL;
  }
     
  /* first find a valid vnum for this room */
  vnum  = toRoom->vnum / 100;
  vnum *= 100;
  for (i = 0 ; i < 100; i++)
  {  
    if (get_room_index(vnum + i) == NULL)
      break;
  }
  if (i == 100) return NULL;
  vnum += i;
    
  /* create the room */
  pRoom = new_room_index();
  pRoom->area = toRoom->area;
  pRoom->vnum = vnum;
   
  if (vnum > top_vnum_room)
    top_vnum_room = vnum;
   
  iHash = vnum % MAX_KEY_HASH;
  AttachToList(pRoom, room_index_hash[iHash]);

  /* reset all doors */
  for (door = 0; door <= 5; door++)
    pRoom->exit[door] = NULL;
  
  /* set default values */   
  free_string(pRoom->description);
  free_string(pRoom->name);
  pRoom->description = str_dup("The kindom halls are empty.\n");
  pRoom->name = str_dup("The Kingdom Halls");
  SET_BIT(pRoom->room_flags, ROOM_KINGDOM);
  
  /* setup the exit from toRoom to pRoom */  
  toRoom->exit[dir]                  = new_exit();
  toRoom->exit[dir]->to_room         = pRoom;
  
  /* setup the exit from pRoom to toRoom */
  pRoom->exit[rev_dir[dir]]          = new_exit();
  pRoom->exit[rev_dir[dir]]->to_room = toRoom;
       
  /* flag the area as changed */
  SET_BIT(toRoom->area->area_flags, AREA_CHANGED);
  
  /* return pointer to room */
  return pRoom;
}

MOB_INDEX_DATA *next_kingdom_mobile(KINGDOM_DATA *kingdom, int vnum)
{
  ROOM_INDEX_DATA *pRoom;
  MOB_INDEX_DATA *pMobIndex;
  int mvnum = 0;

  if ((pRoom = get_room_index(kingdom->vnums)) == NULL)
    return NULL;

  mvnum += kingdom->vnums / 100;
  mvnum *= 100;
  mvnum += vnum;

  /* already exist, return a pointer */
  if ((pMobIndex = get_mob_index(mvnum)) != NULL)
    return pMobIndex;

  /* create mobile */
  pMobIndex = new_mob_index();
  pMobIndex->vnum = mvnum;
  pMobIndex->area = pRoom->area;

  /* update top_vnum_mob counter */
  if (mvnum > top_vnum_mob)
    top_vnum_mob = mvnum;

  /* attach to hash buckets */
  AttachToList(pMobIndex, mob_index_hash[mvnum % MAX_KEY_HASH]);

  /* set some default values */
  pMobIndex->act = ACT_IS_NPC + ACT_NOEXP + ACT_SENTINEL;
  pMobIndex->affected_by = AFF_DETECT_INVIS;
  pMobIndex->sex = SEX_MALE;
  pMobIndex->level = 1;

  return pMobIndex;
}
