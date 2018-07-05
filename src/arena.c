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

#define ARENA_LVNUM     101 /* lower vnum for the arena */
#define ARENA_HVNUM     134 /* upper vnum for the arena */
#define ARENA_PLAYERS     8 /* max players in the arena */
#define FORTRESS_LVNUM  151
#define FORTRESS_NONE   155
#define FORTRESS_HVNUM  169
#define FORTRESS_MAX      6

void  recursive_divide               ( bool reset, int optimal, int depth );
void  send_to_redteam                ( char *txt );
void  send_to_blueteam               ( char *txt );
bool  event_player_contest           ( EVENT_DATA *event );
ROOM_INDEX_DATA *get_next_team_room  ( bool reset );

int next_arena_room;
TEAMARENA_DATA arena;

void strip_arena_eq(CHAR_DATA *ch)
{
  OBJ_DATA *obj;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->carrying);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->pIndexData->vnum == OBJ_VNUM_PROTOPLASM && IS_OBJ_STAT(obj, ITEM_OLC))
      extract_obj(obj);
  }
}

void send_to_redteam(char *txt)
{
  CHAR_DATA *ch;
  ITERATOR *pIter;
  int i;

  pIter = AllocIterator(char_list);
  while ((ch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    for (i = 0; i < 2 * MAX_TEAM_SIZE; i++)
    {
      if (arena.redteam[i] == ch)
        send_to_char(txt, ch);
    }
  }
}

void send_to_blueteam(char *txt)
{
  CHAR_DATA *ch;
  ITERATOR *pIter; 
  int i;

  pIter = AllocIterator(char_list);
  while ((ch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    for (i = 0; i < 2 * MAX_TEAM_SIZE; i++)
    {
      if (arena.blueteam[i] == ch)
        send_to_char(txt, ch);
    }
  }
}

ROOM_INDEX_DATA *get_next_team_room(bool reset)
{
  static int count;
  int vnum;

  if (reset)
  {
    count = 0;
    return NULL;
  }

  vnum = FORTRESS_LVNUM + count;

  if (vnum == FORTRESS_NONE)
    vnum++;

  if (vnum < FORTRESS_LVNUM || vnum > FORTRESS_HVNUM)
  {
    bug("get_next_team_room: out of bound.", 0);
    return NULL;
  }
  count++;

  return (get_room_index(vnum));
}

void do_teamjoin(CHAR_DATA *ch, char *argument)
{
  ROOM_INDEX_DATA *pRoom;
  char buf[MAX_STRING_LENGTH];
  int i;

  if (IS_NPC(ch))
    return;

  if (!IS_SET(arena.status, ARENA_FORTRESS_CONTEST))
  {
    send_to_char("There is currently no team battle to join.\n\r", ch);
    return;
  }

  if (IS_SET(arena.status, ARENA_FORTRESS_INUSE))
  {
    send_to_char("The team battle has already started.\n\r", ch);
    return;
  }

  if (has_timer(ch))
    return;

  for (i = 0; i < 2 * MAX_TEAM_SIZE; i++)
  {
    if (arena.signup[i] != NULL) continue;

    if ((pRoom = get_next_team_room(FALSE)) == NULL)
    {
      send_to_char("I'm afraid we have run out of free rooms in the fortress.\n\r", ch);
      return;
    }

    char_from_room(ch);
    char_to_room(ch, pRoom, TRUE);
    restore_player(ch);
    do_look(ch, "");
    SET_BIT(ch->act, PLR_FREEZE);

    if (IS_SET(ch->act, PLR_AUTOSPECTATE))
      SET_BIT(ch->pcdata->tempflag, TEMP_SPECTATE);

    sprintf(buf, "%s enters the #CTeam Arena#n", ch->name);
    do_info(ch, buf);

    arena.signup[i] = ch;
    return;
  }

  send_to_char("There is no more room in the team arena.\n\r", ch);
}

void cancel_team_arena()
{
  CHAR_DATA *ch;
  int i;

  for (i = 0; i < 2 * MAX_TEAM_SIZE; i++)
  {
    if ((ch = arena.signup[i]) != NULL)
    {
      char_from_room(ch);
      char_to_room(ch, get_room_index(ROOM_VNUM_CITYSAFE), TRUE);
      send_to_char("The team battle was cancelled.\n\r", ch);
      REMOVE_BIT(ch->act, PLR_FREEZE);
    }

    arena.signup[i] = NULL;
  }
}

/* P = NP, anyone ??
 *
 * I'm afraid there is no smart algorithm to find the
 * best solution to this problem, so we do it the hard way.
 */
void recursive_divide(bool reset, int optimal, int depth)
{
  CHAR_DATA *ch;
  static int bestteam[2 * MAX_TEAM_SIZE];
  static int teamone[2 * MAX_TEAM_SIZE];
  static int mightrating[2 * MAX_TEAM_SIZE];
  static int besttry;
  int i, j, k, might, diff;

  /* should only reset on the first call */
  if (reset)
  {
    for (i = 0; i < 2 * MAX_TEAM_SIZE; i++)
    {
      bestteam[i] = 0;
      teamone[i] = 0;

      if (arena.signup[i] != NULL)
	mightrating[i] = getMight(arena.signup[i]);
      else
	mightrating[i] = 0;
    }

    besttry = optimal;
  }

  for (ch = arena.signup[depth], i = depth;
       i < 2 * MAX_TEAM_SIZE && ch != NULL;
       ch = arena.signup[++i])
  {
    /* add the current player to team one */
    teamone[i] = 1;

    /* count the might of the current team */
    for (j = 0, might = 0; j < 2 * MAX_TEAM_SIZE; j++)
    {
      if (teamone[j] == 1)
	might += mightrating[j];
    }

    /* check to see if we have improved our division */
    if ((might <= optimal && (diff = (optimal - might)) < besttry) ||
        (might > optimal && (diff = (might - optimal)) < besttry))
    {
      besttry = diff;

      for (j = 0; j < 2 * MAX_TEAM_SIZE; j++)
	bestteam[j] = teamone[j];
    }

    /* room for improvements */
    if (might < optimal)
      recursive_divide(FALSE, optimal, i + 1);

    /* remove the current player from team one */
    teamone[i] = 0;
  }

  /* back to toplevel, let's store the result */
  if (reset)
  {
    for (i = 0, j = 0, k = 0; i < 2 * MAX_TEAM_SIZE; i++)
    {
      if (arena.signup[i] == NULL)
        continue;

      if (bestteam[i] == 1)
        arena.redteam[j++] = arena.signup[i];
      else
        arena.blueteam[k++] = arena.signup[i];
    }

    /* fill out the rooster with NULL's */
    while (k < 2 * MAX_TEAM_SIZE)
      arena.blueteam[k++] = NULL;
    while (j < 2 * MAX_TEAM_SIZE)
      arena.redteam[j++] = NULL;
  }
}

void begin_team_arena()
{
  EVENT_DATA *event;
  CHAR_DATA *ch, *redleader = NULL, *blueleader = NULL;
  BUFFER *buf;
  ITERATOR *pIter;
  DESCRIPTOR_DATA *d;
  int i, count = 0, might = 0;

  for (i = 0; i < 2 * MAX_TEAM_SIZE; i++)
  {
    if (arena.signup[i] != NULL)
    {
      count++;
      might += getMight(arena.signup[i]);
    }
  }

  if (count <= 1)
  {
    do_info(NULL, "The #CTeam Arena#n was cancelled due to lack of players");
    cancel_team_arena();
    return;
  }

  /* divide the players into two teams */
  recursive_divide(TRUE, might / 2, 0);

  /* put the two teams into the loop of the game */
  for (i = 0; i < 2 * MAX_TEAM_SIZE; i++)
  {
    if ((ch = arena.redteam[i]) != NULL)
    {
      if (redleader == NULL)
	redleader = ch;
      else
      {
        ch->master = NULL;       /* follow noone    */
        ch->leader = redleader;  /* join this group */
      }

      event = alloc_event();
      event->fun = &event_player_contest;
      event->type = EVENT_PLAYER_CONTEST;
      event->argument = str_dup("1");
      add_event_char(event, ch, 2 * PULSE_PER_SECOND);
    }

    if ((ch = arena.blueteam[i]) != NULL)
    {
      if (blueleader == NULL)
	blueleader = ch;
      else
      {
        ch->master = NULL;        /* follow noone    */
        ch->leader = blueleader;  /* join this group */
      }

      event = alloc_event();
      event->fun = &event_player_contest;
      event->type = EVENT_PLAYER_CONTEST;
      event->argument = str_dup("1");
      add_event_char(event, ch, 2 * PULSE_PER_SECOND);
    }
  }

  /* create output */
  buf = buffer_new(MAX_STRING_LENGTH);
  bprintf(buf, "\n\r %s\n\n\r", get_dystopia_banner("Team Arena Contest", 56));
  bprintf(buf, "%13s#u#L%9s#n%13s#u#R%8s#n\n\r", " ", "BLUE TEAM", " ", "RED TEAM");
  for (i = 0; i < 2 * MAX_TEAM_SIZE; i++)
  {
    if (arena.redteam[i] == NULL && arena.blueteam[i] == NULL) continue;

    bprintf(buf, "%13s#l%-12s#n%10s#r%-12s#n\n\r",
      " ", (arena.blueteam[i]) ? arena.blueteam[i]->name : " ",
      " ", (arena.redteam[i]) ? arena.redteam[i]->name : " ");
  }
  bprintf(buf, "\n\r %s\n\r", get_dystopia_banner("Team Arena Contest", 56));

  /* send info to everyone online */
  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    CHAR_DATA *vch;

    if (d->connected == CON_PLAYING && (vch = d->character) != NULL)
    {
      send_to_char(buf->data, vch);
    }
  }

  /* clear the buffer */
  buffer_free(buf);

  /* flag the fortress */
  REMOVE_BIT(arena.status, ARENA_FORTRESS_DEATH);
  REMOVE_BIT(arena.status, ARENA_FORTRESS_1VS1);
  REMOVE_BIT(arena.status, ARENA_FORTRESS_READY);
  SET_BIT(arena.status, ARENA_FORTRESS_SPAR);
  SET_BIT(arena.status, ARENA_FORTRESS_TEAM);
  SET_BIT(arena.status, ARENA_FORTRESS_INUSE);
}

bool event_player_contest(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  AFFECT_DATA paf;
  AFFECT_DATA *af;
  OBJ_DATA *obj;
  ITERATOR *pIter;
  OBJ_INDEX_DATA *pIndexData;
  int count = (event->argument) ? atoi(event->argument) : 0;
  int i;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_fortress: no owner.", 0);
    return FALSE;
  }

  switch(count)
  {
    default:
      bug("event_player_contest: bad count %d.", count);
      return FALSE;
      break;
    case 1:
      pIter = AllocIterator(ch->carrying);
      while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
      {
        if (obj->wear_loc != WEAR_NONE && !IS_OBJ_STAT(obj, ITEM_MASTERY))
        {
          obj_from_char(obj);
          obj_to_char(obj, ch);
        }
      }
      send_to_char("All of your (non-mastery) equipment has been removed.\n\r", ch);
      break;
    case 2:
      pIter = AllocIterator(ch->affected);
      while ((af = (AFFECT_DATA *) NextInList(pIter)) != NULL)
        affect_remove(ch, af);
      send_to_char("All spells affecting you have been removed.\n\r", ch);
      break;
    case 3:
      if ((pIndexData = get_obj_index(OBJ_VNUM_PROTOPLASM)) == NULL)
      {
        bug("event_player_contest: no protoplasm.", 0);
      }
      else
      {
        if (ch->class == CLASS_FAE)
        {
          for (i = 0; i < 2; i++)
          {
            obj = create_object(pIndexData, 50);
            obj->item_type = ITEM_WEAPON;
            obj->wear_flags += ITEM_WIELD;
            obj->resistance = 10;
            obj->toughness = 100;
            object_decay(obj, 15 * 60);
            obj->ownerid = ch->pcdata->playerid;

            free_string(obj->name);
            free_string(obj->short_descr);
            free_string(obj->description);
            obj->name = str_dup("arena bowcaster");
            obj->short_descr = str_dup("An arena bowcaster");
            obj->description = str_dup("A piece of arena equipment lies here.");

            obj->value[1] = 60;                                                 
            obj->value[2] = 30;
            obj->value[3] = 6;

            paf.type       = 0;
            paf.duration   = -1;
            paf.location   = APPLY_HITROLL;
            paf.modifier   = 60;
            paf.bitvector  = 0;
            affect_to_obj(obj, &paf);

            paf.type       = 0;
            paf.duration   = -1;
            paf.location   = APPLY_DAMROLL;
            paf.modifier   = 60;
            paf.bitvector  = 0;
            affect_to_obj(obj, &paf);

            paf.type       = 0;
            paf.duration   = -1;
            paf.location   = APPLY_AC;
            paf.modifier   = -60;
            paf.bitvector  = 0;
            affect_to_obj(obj, &paf);

            SET_BIT(obj->extra_flags, ITEM_FAE_BLAST);
            SET_BIT(obj->extra_flags, ITEM_OLC);
            SET_BIT(obj->quest, QUEST_GIANTSTONE);

            obj_to_char(obj, ch);
            if (i == 0)
              equip_char(ch, obj, WEAR_WIELD);
            if (i == 1)
              equip_char(ch, obj, WEAR_HOLD);
          }
        }
        else
        {
          for (i = 0; i < 18; i++)
          {
            obj = create_object(pIndexData, 50);
            obj->resistance = 10;
            obj->toughness = 100;
            object_decay(obj, 15 * 60);
            obj->ownerid = ch->pcdata->playerid;

            free_string(obj->name);
            free_string(obj->short_descr);
            free_string(obj->description);
            obj->description = str_dup("A piece of arena equipment lies here.");

            paf.type       = 0;
            paf.duration   = -1;
            paf.location   = APPLY_HITROLL;
            paf.modifier   = 15;
            paf.bitvector  = 0;
            affect_to_obj(obj, &paf);

            paf.type       = 0;
            paf.duration   = -1;
            paf.location   = APPLY_DAMROLL;
            paf.modifier   = 15;
            paf.bitvector  = 0;
            affect_to_obj(obj, &paf);

            paf.type       = 0;
            paf.duration   = -1;
            paf.location   = APPLY_AC;
            paf.modifier   = -15;
            paf.bitvector  = 0;
            affect_to_obj(obj, &paf);

            SET_BIT(obj->extra_flags, ITEM_OLC);
            SET_BIT(obj->quest, QUEST_GIANTSTONE);

            switch(i)
            {
              default:
                obj->name = str_dup("bugged");
                obj->short_descr = str_dup("bugged");
                obj_to_char(obj, ch);
                break;
              case 0:
              case 1:
                obj->item_type = ITEM_WEAPON;
                obj->wear_flags += ITEM_WIELD;
                obj->name = str_dup("arena sword");
                obj->short_descr = str_dup("an arena sword");
                obj->value[1] = 60;
                obj->value[2] = 30;
                obj->value[3] = 3;
                obj_to_char(obj, ch);
                if (i == 0) equip_char(ch, obj, WEAR_WIELD);
                if (i == 1) equip_char(ch, obj, WEAR_HOLD);
                break;
              case 2:
              case 3:
                obj->item_type = ITEM_ARMOR;
                obj->wear_flags += ITEM_WEAR_FINGER;
                obj->name = str_dup("arena ring");
                obj->short_descr = str_dup("an arena ring");
                obj->value[0] = 15;
                obj->value[3] = 0;
                obj_to_char(obj, ch);
                if (i == 2) equip_char(ch, obj, WEAR_FINGER_L);
                if (i == 3) equip_char(ch, obj, WEAR_FINGER_R);
                break;
              case 4:
              case 5:
                obj->item_type = ITEM_ARMOR;
                obj->wear_flags += ITEM_WEAR_NECK;
                obj->name = str_dup("arena amulet");
                obj->short_descr = str_dup("an arena amulet");
                obj->value[0] = 15;
                obj->value[3] = 0;
                obj_to_char(obj, ch);
                if (i == 4) equip_char(ch, obj, WEAR_NECK_1);
                if (i == 5) equip_char(ch, obj, WEAR_NECK_2);
                break;
              case 6:
              case 7:
                obj->item_type = ITEM_ARMOR;
                obj->wear_flags += ITEM_WEAR_WRIST;           
                obj->name = str_dup("arena bracer");
                obj->short_descr = str_dup("an arena bracer");
                obj->value[0] = 15;                           
                obj->value[3] = 0; 
                obj_to_char(obj, ch);
                if (i == 6) equip_char(ch, obj, WEAR_WRIST_L);
                if (i == 7) equip_char(ch, obj, WEAR_WRIST_R);
                break;
              case 8:
                obj->item_type = ITEM_ARMOR;
                obj->wear_flags += ITEM_WEAR_BODY;
                obj->name = str_dup("arena mail");
                obj->short_descr = str_dup("an arena mail");
                obj->value[0] = 15;
                obj->value[3] = 0;
                obj_to_char(obj, ch);
                equip_char(ch, obj, WEAR_BODY);
                break;
              case 9:
                obj->item_type = ITEM_ARMOR;                             
                obj->wear_flags += ITEM_WEAR_HEAD;
                obj->name = str_dup("arena helmet");
                obj->short_descr = str_dup("an arena helmet");
                obj->value[0] = 15;
                obj->value[3] = 0;
                obj_to_char(obj, ch);
                equip_char(ch, obj, WEAR_HEAD);
                break;
              case 10:
                obj->item_type = ITEM_ARMOR;
                obj->wear_flags += ITEM_WEAR_LEGS;
                obj->name = str_dup("arena leggings");
                obj->short_descr = str_dup("a pair of arena leggings");
                obj->value[0] = 15;
                obj->value[3] = 0;
                obj_to_char(obj, ch);
                equip_char(ch, obj, WEAR_LEGS);
                break;
              case 11:
                obj->item_type = ITEM_ARMOR;
                obj->wear_flags += ITEM_WEAR_FEET;
                obj->name = str_dup("arena boots");
                obj->short_descr = str_dup("a pair of arena boots");
                obj->value[0] = 15;
                obj->value[3] = 0;
                obj_to_char(obj, ch);
                equip_char(ch, obj, WEAR_FEET);
                break;
              case 12:
                obj->item_type = ITEM_ARMOR;
                obj->wear_flags += ITEM_WEAR_HANDS;           
                obj->name = str_dup("arena gloves");
                obj->short_descr = str_dup("a pair of arena gloves");
                obj->value[0] = 15;                           
                obj->value[3] = 0; 
                obj_to_char(obj, ch);
                equip_char(ch, obj, WEAR_HANDS);
                break;
              case 13:
                obj->item_type = ITEM_ARMOR;
                obj->wear_flags += ITEM_WEAR_ARMS;
                obj->name = str_dup("arena sleeves");
                obj->short_descr = str_dup("a pair of arena sleeves");
                obj->value[0] = 15;
                obj->value[3] = 0;
                obj_to_char(obj, ch);
                equip_char(ch, obj, WEAR_ARMS);
                break;
              case 14:
                obj->item_type = ITEM_ARMOR;                             
                obj->wear_flags += ITEM_WEAR_ABOUT;
                obj->name = str_dup("arena cloak");
                obj->short_descr = str_dup("an arena cloak");
                obj->value[0] = 15;
                obj->value[3] = OBJECT_REGENERATE;
                obj_to_char(obj, ch);
                equip_char(ch, obj, WEAR_ABOUT);
                break;
              case 15:
                obj->item_type = ITEM_ARMOR;
                obj->wear_flags += ITEM_WEAR_WAIST;
                obj->name = str_dup("arena belt");
                obj->short_descr = str_dup("an arena belt");
                obj->value[0] = 15;
                obj->value[3] = OBJECT_FLYING;
                obj_to_char(obj, ch);
                equip_char(ch, obj, WEAR_WAIST);
                break;
              case 16:
                obj->item_type = ITEM_ARMOR;
                obj->wear_flags += ITEM_WEAR_FACE;
                obj->name = str_dup("arena visor");
                obj->short_descr = str_dup("an arena visor");
                obj->value[0] = 15;
                obj->value[3] = 0;
                obj_to_char(obj, ch);
                equip_char(ch, obj, WEAR_FACE);
                break;
              case 17:
                obj->item_type = ITEM_ARMOR;
                obj->wear_flags += ITEM_WEAR_SHIELD;          
                obj->name = str_dup("arena shield");
                obj->short_descr = str_dup("an arena shield");
                obj->value[0] = 15;                           
                obj->value[3] = OBJECT_SANCTUARY; 
                obj_to_char(obj, ch);
                equip_char(ch, obj, WEAR_SHIELD);
                break;
            }
          }
        }
      }
      do_equipment(ch, "");
      send_to_char("You will be using this equipment for the fight.\n\r", ch);
      break;
    case 4:
      ch->pcdata->safe_counter = 6;
      REMOVE_BIT(ch->act, PLR_FREEZE);
      send_to_char("You have 20 seconds to get ready to fight.\n\r", ch);
      break;
  }

  if (++count < 5)
  {
    char buf[MAX_INPUT_LENGTH];

    sprintf(buf, "%d", count);

    event = alloc_event();
    event->fun = &event_player_contest;
    event->type = EVENT_PLAYER_CONTEST;
    event->argument = str_dup(buf);
    add_event_char(event, ch, 2 * PULSE_PER_SECOND);
  }

  return FALSE;
}

void init_teamarena()
{
  int i;

  for (i = 0; i < 2 * MAX_TEAM_SIZE; i++)
  {
    arena.blueteam [i] = 0;
    arena.redteam  [i] = 0;
  }

  arena.status = ARENA_FORTRESS_READY + ARENA_FORTRESS_1VS1 + ARENA_FORTRESS_SPAR + ARENA_ARENA_CLOSED;
}

void open_fortress()
{
  SET_BIT(arena.status, ARENA_FORTRESS_READY);
  REMOVE_BIT(arena.status, ARENA_FORTRESS_INUSE);
  REMOVE_BIT(arena.status, ARENA_FORTRESS_TEAM);
  REMOVE_BIT(arena.status, ARENA_FORTRESS_CONTEST);
}

void do_arenastats(CHAR_DATA *ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;
  CHAR_DATA *gch;

  if (IS_NPC(ch)) return;

  if (!IS_SET(arena.status, ARENA_ARENA_INUSE))
  {
    send_to_char("The arena is currently closed.\n\r", ch);
    do_timer(ch, "");
    return;
  }

  if (in_arena(ch))
  {
    send_to_char("You cannot use this command inside the arena.\n\r", ch);
    return;
  }

  send_to_char("#G            People in the arena#n\n\n\r", ch);
  send_to_char("#RName                Health   Stamina     Mana#n\n\r", ch);
  send_to_char("#0----------------------------------------------#n\n\r", ch);

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if ((gch = d->character) != NULL)
    {
      if (in_arena(gch))
      {
        printf_to_char(ch, "%-15s    %3d/100   %3d/100   %3d/100  %s\n\r",
          gch->name, 100 * gch->hit / UMAX(gch->max_hit, 1),
          100 * gch->move / UMAX(gch->max_move, 1), 100 * gch->mana / UMAX(gch->max_mana, 1),
          (IS_SET(d->character->pcdata->tempflag, TEMP_SPECTATE)) ? "(Allows spectating)" : "");
      }
    }
  }
}

void open_arena()
{
  ITERATOR *pIter;
  DESCRIPTOR_DATA *d;
  int pcount = 0, vetcount = 0, legcount = 0;

  SET_BIT(arena.status, ARENA_ARENA_OPEN);
  REMOVE_BIT(arena.status, ARENA_ARENA_CLOSED);

  next_arena_room = ARENA_LVNUM; /* first person to join will be put in this room. */

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected == CON_PLAYING && d->character && d->character->level < 7)
    {
      int might = getMight(d->character);

      /* only count cadet and above */
      if (might < RANK_CADET)
        continue;

      /* count all players of cadet or better rank */
      pcount++;

      if (might < RANK_ADVENTURER)  /* cadet, private and veteran players */
        vetcount++;
      if (might < RANK_MASTER)      /* cadet, private, etc..., legendary players */
        legcount++;
    }
  }

  if (number_percent() > 50 && (vetcount >= 5 || legcount >= 5 || (pcount >= 10 && !IS_SET(arena.status, ARENA_FORTRESS_INUSE))))
  {
    if (pcount >= 10 && !IS_SET(arena.status, ARENA_FORTRESS_INUSE))
    {
      do_info(NULL, "The Fortress is now open for #CTeam Arena#n (type teamjoin)");
      SET_BIT(arena.status, ARENA_FORTRESS_CONTEST);
      REMOVE_BIT(arena.status, ARENA_ARENA_OPEN);
      SET_BIT(arena.status, ARENA_ARENA_CLOSED);
      get_next_team_room(TRUE);
    }
    else if (vetcount >= 5)
    {
      do_info(NULL, "The Arena is now open for #rVeterans#n and lower rank (type arenajoin)");
      SET_BIT(arena.status, ARENA_ARENA_VETRANK);
    }
    else /* legcount >=5 */
    {
      do_info(NULL, "The Arena is now open for #yLegendaries#n and lower rank (type arenajoin)");
      SET_BIT(arena.status, ARENA_ARENA_LEGRANK);
    }
  }
  else
  {
    REMOVE_BIT(arena.status, ARENA_ARENA_VETRANK);
    REMOVE_BIT(arena.status, ARENA_ARENA_LEGRANK);
    do_info(NULL,"The Arena is now open for #CEVERYONE#n (type arenajoin)");
  }
}

void close_arena()
{
  CHAR_DATA *gch = NULL;
  CHAR_DATA *vch;
  ITERATOR *pIter;
  int arenaplayers = 0;

  SET_BIT(arena.status, ARENA_ARENA_INUSE);
  REMOVE_BIT(arena.status, ARENA_ARENA_OPEN);

  pIter = AllocIterator(char_list);
  while ((vch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_NPC(vch)) continue;

    if (in_arena(vch))
    {
      REMOVE_BIT(vch->act, PLR_FREEZE);
      gch = vch;
      arenaplayers++;
    }
  }

  /* if there was only one player, remove him */
  if (arenaplayers <= 1)
  {
    if (arenaplayers)
    {
      char_from_room(gch);
      char_to_room(gch, get_room_index(ROOM_VNUM_CITYSAFE), TRUE);
    }

    REMOVE_BIT(arena.status, ARENA_ARENA_INUSE);
    SET_BIT(arena.status, ARENA_ARENA_CLOSED);

    do_info(NULL, "The Arena fight was cancelled due to lack of players!");
    return;
  }
  else
  {
    do_info(NULL, "The Arena is now closed, let the games begin!");
  }
}

void do_arenajoin(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  ITERATOR *pIter;
  int arenapeople = 0;
  DESCRIPTOR_DATA *d;

  if (IS_NPC(ch))
    return;

  if (!IS_SET(arena.status, ARENA_ARENA_OPEN))
  {
    send_to_char("The arena is closed.\n\r", ch);
    return;
  }

  if (ch->fight_timer > 0)
  {
    send_to_char("You have a fighttimer.\n\r", ch);
    return;
  }

  if ((IS_SET(arena.status, ARENA_ARENA_VETRANK) && getMight(ch) >= RANK_ADVENTURER) ||
      (IS_SET(arena.status, ARENA_ARENA_LEGRANK) && getMight(ch) >= RANK_MASTER))
  {
    send_to_char("Your a bit to big for this arena.\n\r", ch);
    return;
  }

  if (in_arena(ch) || in_fortress(ch))
  {
    send_to_char("You cannot join the arena from there.\n\r", ch);
    return;
  }

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->character != NULL)
    {
      if (!d->connected == CON_PLAYING)
        continue;

      if (in_arena(d->character))
        arenapeople++;
    }
  }

  if (arenapeople > ARENA_PLAYERS)
  {
    send_to_char("The arena is crowded atm.\n\r",ch);
    return;
  }

  char_from_room(ch);
  char_to_room(ch, get_room_index(next_arena_room), TRUE);
  next_arena_room += (ARENA_HVNUM - ARENA_LVNUM) / ARENA_PLAYERS;
  sprintf(buf,"%s has joined the Arena!",ch->name);
  do_info(ch, buf);
  restore_player(ch);
  SET_BIT(ch->act, PLR_FREEZE);

  if (IS_SET(ch->act, PLR_AUTOSPECTATE))
    SET_BIT(ch->pcdata->tempflag, TEMP_SPECTATE);
}

void do_resign(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  CHAR_DATA *gch = NULL;
  ITERATOR *pIter;
  int found = 0;
  ROOM_INDEX_DATA *location;
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch)) return;

  if (!in_arena(ch))
  {
    send_to_char("Your not in the arena.\n\r",ch);
    return;
  }

  sprintf(buf,"%s resigns from the Arena", ch->name);
  do_info(ch, buf);

  if ((location = get_room_index(ROOM_VNUM_CITYSAFE)) == NULL)
  {
    bug("do_resignt: no altar.", 0);
    return;
  }

  /* move player out of the arena and restore */
  char_from_room(ch);
  char_to_room(ch, location, TRUE);
  ch->fight_timer = 0;
  restore_player(ch);

  /* stop spectating */
  stop_spectating(ch);

  call_all(ch);
  ch->pcdata->alosses++;

  pIter = AllocIterator(char_list);
  while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_NPC(victim))
      continue;

    if (in_arena(victim))
    {
      gch = victim;
      found++;
    }
  }

  if (found == 1)
  {
    sprintf(buf,"#C%s #oemerges victorious from the #Rarena#n", gch->name);
    do_info(gch, buf);

    char_from_room(gch);
    char_to_room(gch, location, TRUE);
    gch->fight_timer = 0;
    restore_player(gch);
    stop_spectating(gch);
    call_all(gch);
    gch->pcdata->awins++;
    win_arena(gch);

    REMOVE_BIT(arena.status, ARENA_ARENA_INUSE);
    SET_BIT(arena.status, ARENA_ARENA_CLOSED);
  }
}

void win_arena(CHAR_DATA *ch)
{
  OBJ_DATA *obj;

  if ((obj = pop_rand_equipment(500 + (150 * ARENA_PLAYERS * (next_arena_room - ARENA_LVNUM))/(ARENA_HVNUM - ARENA_LVNUM), TRUE)) != NULL)
    obj_to_char(obj, ch);
}

/*
 * The challenge system, uses specific vnums of rooms, so don't remove those.
 */
void do_challenge(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  
  argument = one_argument(argument, arg);
  one_argument(argument, arg2);

  if (IS_NPC(ch)) return;
  if (arg[0] == '\0' || arg2[0] == '\0')
  {
    send_to_char("Syntax : Challenge <person> <death/spar>\n\r", ch);
    return;
  }

  if (!IS_SET(ch->extra, EXTRA_PKREADY))
  {
    send_to_char("You must be pkready before you can challenge anyone.\n\r", ch);
    return;
  }

  if (event_isset_mobile(ch, EVENT_PLAYER_ARENA_WAIT))
  {
    send_to_char("You can only use challenge once every 2 minutes.\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("You can't challenge monsters.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char("Ehm, no.\n\r", ch);
    return;
  }
  if (victim->level != 3)
  {
    send_to_char("You can only challenge avatars.\n\r", ch);
    return;
  }
  if (victim == ch->challenger)
  {
    send_to_char("You have already challenged them.\n\r", ch);
    return;
  }

  if (check_ignore(ch, victim) || check_ignore(victim, ch))
  {
    send_to_char("You cannot challenge someone that you are ignoring or are being ignored by.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "death")) ch->deathmatch = TRUE;
  else if (!str_cmp(arg2, "spar")) ch->deathmatch = FALSE;
  else
  {
    do_challenge(ch, "");
    return;
  }

  if (ch->deathmatch)
  {
    bool canDecap, canKillYou;

    canDecap = fair_fight(ch, victim);
    canKillYou = fair_fight(victim, ch);
  
    if (!canKillYou || !canDecap)
    {
      send_to_char("You cannot challenge someone in a deathmatch if you cannot kill eachother.\n\r", ch);
      return;
    }
  }

  ch->challenger = victim;
  send_to_char("You challenge them.\n\r", ch);

  /* set the delay event */
  event            =  alloc_event();
  event->fun       =  &event_dummy;
  event->type      =  EVENT_PLAYER_ARENA_WAIT;
  add_event_char(event, ch, 2 * 60 * PULSE_PER_SECOND);

  if (ch->deathmatch) sprintf(buf, "You have been challenged to a #Gdeathmatch#n by %s. Type agree %s to start the fight.\n\r", ch->name, ch->name);
  else sprintf(buf, "You have been challenged to a #Gspar#n by %s. Type agree %s to start the fight.\n\r", ch->name, ch->name);
  send_to_char(buf, victim);
  WAIT_STATE(ch, 8);
}

void do_decline(CHAR_DATA *ch, char *argument)
{    
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  if (IS_NPC(ch)) return;
  if (arg[0] == '\0')
  {
    send_to_char("Decline whom?\n\r", ch);
    return;
  }
  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("You can't decline a monster, since it can't challenge you.\n\r", ch);
    return;
  }
  if (victim->challenger != ch)
  {
    send_to_char("They aren't challenging you. (they may have cancelled the challenge)\n\r", ch);
    return;
  }
  victim->challenger = NULL;
  send_to_char("You decline their challenge.\n\r", ch);
  send_to_char("Your challenge has been declined.\n\r", victim);
}

void do_accept2(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  bool canDecap, canKillYou;
  ROOM_INDEX_DATA *location;

  one_argument(argument, arg);

  if (IS_NPC(ch)) return;
  if (IS_SET(arena.status, ARENA_FORTRESS_INUSE) || IS_SET(arena.status, ARENA_FORTRESS_CONTEST))
  {
    send_to_char("The Forbidden Fortress is currently being used by someone else.\n\r", ch);
    return;
  }
  if (arg[0] == '\0')
  {
    send_to_char("Accept whoms challenge?\n\r", ch);
    return;
  }
  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("You cannot accept a challenge from a monster.\n\r", ch);
    return;
  }
  if (victim->challenger != ch)
  {
    send_to_char("You haven't been challenged by them.\n\r", ch);
    return;
  }
  if (victim->level != 3)
  {
    send_to_char("They have to be avatar.\n\r", ch);
    return;
  }
  if (victim->fight_timer > 0)
  {
    send_to_char("They have a fighttimer currently, you'll have to wait.\n\r", ch);
    return;
  }
  if (ch->fight_timer > 0)
  {
    send_to_char("Not with a fight timer.\n\r", ch);
    return;
  }
  if (IS_SET(victim->extra, EXTRA_AFK))
  {
    send_to_char("They are AFK!\n\r", ch);
    return;
  }
  if (!IS_SET(ch->extra, EXTRA_PKREADY))
  {
    send_to_char("You must be pkready before you can accept a challenge.\n\r", ch);
    return;
  }
  if (victim->position != POS_STANDING)
  {
    send_to_char("They are not standing, you'll have to wait.\n\r", ch);
    return;
  }
  if (in_arena(ch))
  {
    send_to_char("You cannot accept a challenge in the arena.\n\r", ch);
    return;
  }
  if (in_arena(victim))
  {
    send_to_char("They are currently busy in the arena.\n\r", ch);
    return;
  }

  if (victim->deathmatch)
  {
    canDecap = fair_fight(ch, victim);
    canKillYou = fair_fight(victim, ch);

    if (!canKillYou || !canDecap)
    {
      send_to_char("You cannot accept a deathmatch if you cannot kill eachother.\n\r", ch);
      return;
    }
  }

  if ((location = get_room_index(ROOM_VNUM_FORTRESS1)) == NULL)
  {
    bug("Fortress Missing.", 0);
    return;
  }
  char_from_room(ch);
  char_to_room(ch, location, TRUE);

  if (IS_SET(ch->act, PLR_AUTOSPECTATE))
    SET_BIT(ch->pcdata->tempflag, TEMP_SPECTATE);

  if ((location = get_room_index(ROOM_VNUM_FORTRESS2)) == NULL)
  {
    bug("Fortress Missing.", 0);
    return;
  }
  char_from_room(victim);
  char_to_room(victim, location, TRUE);

  if (IS_SET(victim->act, PLR_AUTOSPECTATE))
    SET_BIT(victim->pcdata->tempflag, TEMP_SPECTATE);

  restore_player(victim);
  restore_player(ch);

  if (!victim->deathmatch)
    sprintf(buf, "%s and %s enter #CThe Forbidden Fortress#n to test their skills", ch->name, victim->name);
  else
    sprintf(buf, "%s and %s enter #CThe Forbidden Fortress#n to duel for their lives", ch->name, victim->name);
  do_info(ch, buf);

  SET_BIT(arena.status, ARENA_FORTRESS_INUSE);
  REMOVE_BIT(arena.status, ARENA_FORTRESS_READY);

  if (victim->deathmatch)
  {
    REMOVE_BIT(arena.status, ARENA_FORTRESS_SPAR);
    SET_BIT(arena.status, ARENA_FORTRESS_DEATH);
  }
  else
  {
    REMOVE_BIT(arena.status, ARENA_FORTRESS_DEATH);
    SET_BIT(arena.status, ARENA_FORTRESS_SPAR);
  }

  victim->challenger = NULL;
}

void do_fortressstats(CHAR_DATA *ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;
  CHAR_DATA *gch;

  if (IS_NPC(ch)) return;

  if (IS_SET(arena.status, ARENA_FORTRESS_READY))
  {
    send_to_char("The fortress is currenly inactive.\n\r", ch);
    return;
  }

  if (in_fortress(ch))
  {
    send_to_char("You cannot use this command inside the fortress.\n\r", ch);
    return;
  }

  send_to_char("#G            Forbidden Fortress#n\n\r\n\r", ch);
  send_to_char("#RName                Health   Stamina     Mana#n\n\r", ch);
  send_to_char("#0----------------------------------------------#n\n\r", ch);

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if ((gch = d->character) != NULL)
    {
      if (in_fortress(gch))
      {
        printf_to_char(ch, "%-15s    %3d/100   %3d/100   %3d/100  %s\n\r",
          gch->name, 100 * gch->hit / UMAX(gch->max_hit, 1),
          100 * gch->move / UMAX(gch->max_move, 1), 100 * gch->mana / UMAX(gch->max_mana, 1),   
          (IS_SET(d->character->pcdata->tempflag, TEMP_SPECTATE)) ? "(Allows spectating)" : "");
      }
    }  
  }
}
