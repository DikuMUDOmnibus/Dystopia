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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "dystopia.h"

STACK *affect_free      = NULL;
bool   NoMasteryExtract = TRUE;

/*
 * Local functions.
 */

/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust(CHAR_DATA * ch)
{
  if (ch->trust != 0)
    return ch->trust;

  if (IS_NPC(ch) && ch->level >= LEVEL_AVATAR)
    return LEVEL_AVATAR - 1;
  else
    return ch->level;
}

/*
 * Retrieve a character's age.
 */
int get_age(CHAR_DATA * ch)
{
  return 17 + (ch->played + (int) (current_time - ch->logon)) / 7200;
}

/*
 * Retrieve character's current strength.
 */
int get_curr_str(CHAR_DATA * ch)
{
  int max = 50;;

  if (IS_NPC(ch))
    return 13;

  return URANGE(3, ch->pcdata->perm_str + ch->pcdata->mod_str, max);
}

/*
 * Retrieve character's current intelligence.
 */
int get_curr_int(CHAR_DATA * ch)
{
  int max = 50;

  if (IS_NPC(ch))
    return 13;

  return URANGE(3, ch->pcdata->perm_int + ch->pcdata->mod_int, max);
}

/*
 * Retrieve character's current wisdom.
 */
int get_curr_wis(CHAR_DATA * ch)
{
  int max = 50;

  if (IS_NPC(ch))
    return 13;

  return URANGE(3, ch->pcdata->perm_wis + ch->pcdata->mod_wis, max);
}

/*
 * Retrieve character's current dexterity.
 */
int get_curr_dex(CHAR_DATA * ch)
{
  int max = 50;

  if (IS_NPC(ch))
    return 13;

  return URANGE(3, ch->pcdata->perm_dex + ch->pcdata->mod_dex, max);
}

/*
 * Retrieve character's current constitution.
 */
int get_curr_con(CHAR_DATA * ch)
{
  int max = 50;

  if (IS_NPC(ch))
    return 13;

  return URANGE(3, ch->pcdata->perm_con + ch->pcdata->mod_con, max);
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n(CHAR_DATA * ch)
{
  if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
    return 100000;

  return MAX_WEAR + 2 * get_curr_dex(ch) / 3;
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w(CHAR_DATA *ch)
{
  if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
    return 1000000;

  return ((40 * UMIN(25, get_curr_str(ch))));
}

HELP_DATA *get_help(CHAR_DATA *ch, char *argument)
{
  char argall[MAX_INPUT_LENGTH];
  char argone[MAX_INPUT_LENGTH];
  char argnew[MAX_INPUT_LENGTH];
  HELP_DATA *pHelp;
  ITERATOR *pIter;
  int lev;

  if (argument[0] == '\0')
    argument = "summary";

  if (isdigit(argument[0]))
  {
    lev = number_argument(argument, argnew);
    argument = argnew;
  }
  else
    lev = -2;

  /*
   * Tricky argument handling so 'help a b' doesn't match a.
   */
  argall[0] = '\0';
  while ( argument[0] != '\0' )
  {
    argument = one_argument(argument, argone);
    if (argall[0] != '\0')
      strcat( argall, " ");
    strcat(argall, argone);
  }

  pIter = AllocIterator(help_list);
  while ((pHelp = (HELP_DATA *) NextInList(pIter)) != NULL)
  {
    if (ch == NULL)
    {
      if (pHelp->level > LEVEL_AVATAR)
        continue;
    }
    else if (pHelp->level > get_trust(ch))
      continue;

    if (lev != -2 && pHelp->level != lev)
      continue;
    if (is_name(argall, pHelp->name))
      return pHelp;
  }

  return NULL;
}

/*
 * See if a string is one of the names of an object.
 */
bool is_name(char *str, char *namelist)
{
  char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
  char *list, *string;

  /* fix crash on NULL namelist */
  if (namelist == NULL || namelist[0] == '\0')
    return FALSE;

  /* fixed to prevent is_name on "" returning TRUE */
  if (str[0] == '\0')
    return FALSE;

  string = str;
  /* we need ALL parts of string to match part of namelist */
  for (;;)                      /* start parsing string */
  {
    str = one_argument(str, part);

    if (part[0] == '\0')
      return TRUE;

    /* check to see if this is part of namelist */
    list = namelist;
    for (;;)                    /* start parsing namelist */
    {
      list = one_argument(list, name);
      if (name[0] == '\0')      /* this name was not found */
        return FALSE;

      if (!str_prefix(string, name))
        return TRUE;            /* full pattern match */

      if (!str_prefix(part, name))
        break;
    }
  }
}

bool is_full_name(const char *str, char *namelist)
{
  char name[MAX_INPUT_LENGTH];

  for (;;)
  {
    namelist = one_argument(namelist, name);
    if (name[0] == '\0')
      return FALSE;
    if (!str_cmp(str, name))
      return TRUE;
  }
}

bool char_exists(char *argument)
{   
  FILE *fp;
  char buf[MAX_STRING_LENGTH];
  bool found = FALSE;

  sprintf(buf, "%swhois/%s.whois", PLAYER_DIR, capitalize(argument));
  if ((fp = fopen(buf, "r")) != NULL)
  { 
    found = TRUE;
    fclose(fp);
  }
  return found;
}   


/*
 * Apply or remove an affect to a character.
 */
void affect_modify(CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd)
{
  OBJ_DATA *wield;
  int mod;

  mod = paf->modifier;

  if (fAdd)
  {
    SET_BIT(ch->affected_by, paf->bitvector);
  }
  else
  {
    REMOVE_BIT(ch->affected_by, paf->bitvector);
    mod = 0 - mod;
  }

  if (IS_NPC(ch))
  {
    switch (paf->location)
    {
      default:
        break;
      case APPLY_NONE:
        break;
      case APPLY_MANA:
        ch->max_mana += mod;
        break;
      case APPLY_HIT:
        ch->max_hit += mod;
        break;
      case APPLY_MOVE:
        ch->max_move += mod;
        break;
      case APPLY_AC:
        ch->armor += mod;
        break;
      case APPLY_HITROLL:
        ch->hitroll += mod;
        break;
      case APPLY_DAMROLL:
        ch->damroll += mod;
        break;
      case APPLY_SAVING_PARA:
        ch->saving_throw += mod;
        break;
      case APPLY_SAVING_ROD:
        ch->saving_throw += mod;
        break;
      case APPLY_SAVING_PETRI:
        ch->saving_throw += mod;
        break;
      case APPLY_SAVING_BREATH:
        ch->saving_throw += mod;
        break;
      case APPLY_SAVING_SPELL:
        ch->saving_throw += mod;
        break;
    }
    return;
  }

  switch (paf->location)
  {
    default:
      bug("Affect_modify: unknown location %d.", paf->location);
      return;

    case APPLY_NONE:
      break;
    case APPLY_STR:
      ch->pcdata->mod_str += mod;
      break;
    case APPLY_DEX:
      ch->pcdata->mod_dex += mod;
      break;
    case APPLY_INT:
      ch->pcdata->mod_int += mod;
      break;
    case APPLY_WIS:
      ch->pcdata->mod_wis += mod;
      break;
    case APPLY_CON:
      ch->pcdata->mod_con += mod;
      break;
    case APPLY_SEX:
    case APPLY_CLASS:
    case APPLY_LEVEL:
    case APPLY_AGE:
    case APPLY_HEIGHT:
    case APPLY_WEIGHT:
      break;
    case APPLY_MANA:
      ch->max_mana += mod;
      break;
    case APPLY_HIT:
      ch->max_hit += mod;
      break;
    case APPLY_MOVE:
      ch->max_move += mod;
      break;
    case APPLY_GOLD:
      break;
    case APPLY_EXP:
      break;
    case APPLY_AC:
      ch->armor += mod;
      break;
    case APPLY_HITROLL:
      ch->hitroll += mod;
      break;
    case APPLY_DAMROLL:
      ch->damroll += mod;
      break;
    case APPLY_SAVING_PARA:
      ch->saving_throw += mod;
      break;
    case APPLY_SAVING_ROD:
      ch->saving_throw += mod;
      break;
    case APPLY_SAVING_PETRI:
      ch->saving_throw += mod;
      break;
    case APPLY_SAVING_BREATH:
      ch->saving_throw += mod;
      break;
    case APPLY_SAVING_SPELL:
      ch->saving_throw += mod;
      break;
  }

  /*
   * Check for weapon wielding.
   * Guard against recursion (for weapons with affects).
   */
  if ((wield = get_eq_char(ch, WEAR_WIELD)) != NULL && wield->item_type == ITEM_WEAPON && get_obj_weight(wield) > (5 * UMIN(25, get_curr_str(ch))) / 2)
  {
    static int depth;

    if (depth == 0)
    {
      depth++;
      act("You drop $p.", ch, wield, NULL, TO_CHAR);
      act("$n drops $p.", ch, wield, NULL, TO_ROOM);
      obj_from_char(wield);
      obj_to_room(wield, ch->in_room);
      depth--;
    }
  }

  return;
}

void affect_to_obj(OBJ_DATA * obj, AFFECT_DATA * paf)
{
  AFFECT_DATA *paf_new;

  if ((paf_new = (AFFECT_DATA *) PopStack(affect_free)) == NULL)
  {
    paf_new = malloc(sizeof(*paf_new));
  }

  *paf_new = *paf;
  AttachToList(paf_new, obj->affected);
}

/*
 * Give an affect to a char.
 */
void affect_to_char(CHAR_DATA * ch, AFFECT_DATA * paf)
{
  AFFECT_DATA *paf_new;

  if ((paf_new = (AFFECT_DATA *) PopStack(affect_free)) == NULL)
  {
    paf_new = malloc(sizeof(*paf_new));
  }

  *paf_new = *paf;

  AttachToList(paf_new, ch->affected);

  affect_modify(ch, paf_new, TRUE);
}

/*
 * Remove an affect from a char.
 */
void affect_remove(CHAR_DATA * ch, AFFECT_DATA * paf)
{
  if (SizeOfList(ch->affected) == 0)
  {
    bug("Affect_remove: no affect.", 0);
    return;
  }

  affect_modify(ch, paf, FALSE);

  DetachFromList(paf, ch->affected);
  PushStack(paf, affect_free);
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip(CHAR_DATA * ch, int sn)
{
  AFFECT_DATA *paf;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->type == sn)
      affect_remove(ch, paf);
  }
}

/*
 * Return true if a char is affected by a spell.
 */
bool is_affected(CHAR_DATA * ch, int sn)
{
  AFFECT_DATA *paf;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->type == sn)
      return TRUE;
  }

  return FALSE;
}

/*
 * Add or enhance an affect.
 */
void affect_join(CHAR_DATA * ch, AFFECT_DATA * paf)
{
  AFFECT_DATA *paf_old;
  ITERATOR *pIter;
  bool found;

  found = FALSE;

  pIter = AllocIterator(ch->affected);
  while ((paf_old = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf_old->type == paf->type)
    {
      paf->duration += paf_old->duration;
      paf->modifier += paf_old->modifier;
      affect_remove(ch, paf_old);
      break;
    }
  }

  affect_to_char(ch, paf);
  return;
}

/*
 * Move a char out of a room.
 */
void char_from_room(CHAR_DATA * ch)
{
  OBJ_DATA *obj;

  if (ch->in_room == NULL)
  {
    bug("Char_from_room: NULL.", 0);
    return;
  }

  if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room->light > 0)
    --ch->in_room->light;

  DetachFromList(ch, ch->in_room->people);
  ch->in_room = NULL;
}

/*
 * Move a char into a room.
 */
void char_to_room(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex, bool trigger)
{
  OBJ_DATA *obj;
  EVENT_DATA *event;
  bool aggrocheck = FALSE;
  bool blastwarding = FALSE;
  bool blastdone = FALSE;

  if (!pRoomIndex)
  {
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "Char_to_room: %s -> NULL room!  Putting char in limbo (%d)", ch->name, ROOM_VNUM_LIMBO);
    bug(buf, 0);

    /* This used to just return, but there was a problem with crashing
       and I saw no reason not to just put the char in limbo. -Narn */
    pRoomIndex = get_room_index(ROOM_VNUM_LIMBO);
  }

  ch->in_room = pRoomIndex;
  AttachToList(ch, pRoomIndex->people);

  if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
    ++ch->in_room->light;

  if (ch->loc_hp[6] > 0 && ch->in_room->blood < 1000)
    ch->in_room->blood += 1;

  /* aggro check event */
  if (!IS_NPC(ch) && trigger)
  {
    ITERATOR *pIter;

    pIter = AllocIterator(pRoomIndex->events);
    while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
    {
      if (event->type == EVENT_ROOM_AGGROCHECK)
        aggrocheck = TRUE;

      else if (event->type == EVENT_ROOM_WARDING)
        blastwarding = TRUE;

      else if (event->type == EVENT_ROOM_BLASTWARD)
        blastdone = TRUE;

      else if (event->type == EVENT_ROOM_SHADOWGUARD)
      {
        int id = (event->argument != NULL) ? atoi(event->argument) : 0;
        CHAR_DATA *vch;

        if (id > 0 && (vch = get_online_player(id)) != NULL)
        {
          if (vch != ch)
          {
            act("A shadowguard steps from the shadows and report.", vch, NULL, NULL, TO_ALL);
            act("A shadowguard says '#y$N has entered '$t#y', sire.#n'.", vch, pRoomIndex->name, ch, TO_ALL);
            dequeue_event(event, TRUE);
          }
        }
        else
        {
          dequeue_event(event, TRUE);
        }
      }
    }
  }

  if (blastwarding && !blastdone && !IS_NPC(ch))
  {
    event              =  alloc_event();
    event->fun         =  &event_room_blastward;
    event->type        =  EVENT_ROOM_BLASTWARD;
    add_event_room(event, pRoomIndex, 1);
  }

  if (!aggrocheck && ch->level < 7 && !IS_NPC(ch))
  {
    event              =  alloc_event();
    event->fun         =  &event_room_aggrocheck;
    event->type        =  EVENT_ROOM_AGGROCHECK;
    add_event_room(event, pRoomIndex, 1);
  }

  if (!IS_NPC(ch) && IS_CLASS(ch, CLASS_FAE) && IS_SET(ch->pcdata->powers[FAE_BITS], FAE_BLOODHUNGER) && trigger)
    fae_hunger(ch);
}

ROOM_INDEX_DATA *locate_obj(OBJ_DATA *obj)
{
  if (obj->in_obj != NULL)
    return locate_obj(obj->in_obj);

  if (obj->carried_by != NULL)
    return obj->carried_by->in_room;

  return obj->in_room;
}


/*
 * Give an obj to a char.
 */
void obj_to_char(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (obj == NULL)
    return;

  AttachToList(obj, ch->carrying);

  obj->carried_by = ch;
  obj->in_room = NULL;
  obj->in_obj = NULL;

  ch->carry_number++;
  ch->carry_weight += get_obj_weight(obj);
}

void obj_to_char_end(OBJ_DATA *obj, CHAR_DATA *ch)
{
  if (obj == NULL)
    return;

  AttachToEndOfList(obj, ch->carrying);

  obj->carried_by = ch;
  obj->in_room = NULL;
  obj->in_obj = NULL;

  ch->carry_number++;
  ch->carry_weight += get_obj_weight(obj);
}

/*
 * Take an obj from its character.
 */
void obj_from_char(OBJ_DATA * obj)
{
  CHAR_DATA *ch;

  if (obj == NULL)
    return;

  if ((ch = obj->carried_by) == NULL)
  {
    bug("Obj_from_char: null ch.", 0);
    return;
  }

  if (obj->wear_loc != WEAR_NONE)
    unequip_char(ch, obj);

  DetachFromList(obj, ch->carrying);

  obj->carried_by = NULL;
  ch->carry_number--;
  ch->carry_weight -= get_obj_weight(obj);
}

/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac(OBJ_DATA * obj, int iWear)
{
  if (obj->item_type != ITEM_ARMOR)
    return 0;

  switch (iWear)
  {
    case WEAR_BODY:
      return 3 * obj->value[0];
    case WEAR_HEAD:
      return 2 * obj->value[0];
    case WEAR_LEGS:
      return 2 * obj->value[0];
    case WEAR_FEET:
      return obj->value[0];
    case WEAR_HANDS:
      return obj->value[0];
    case WEAR_ARMS:
      return obj->value[0];
    case WEAR_SHIELD:
      return obj->value[0];
    case WEAR_FINGER_L:
      return obj->value[0];
    case WEAR_FINGER_R:
      return obj->value[0];
    case WEAR_NECK_1:
      return obj->value[0];
    case WEAR_NECK_2:
      return obj->value[0];
    case WEAR_ABOUT:
      return 2 * obj->value[0];
    case WEAR_WAIST:
      return obj->value[0];
    case WEAR_WRIST_L:
      return obj->value[0];
    case WEAR_WRIST_R:
      return obj->value[0];
    case WEAR_HOLD:
      return obj->value[0];
    case WEAR_FACE:
      return obj->value[0];
    case WEAR_SCABBARD_L:
      return 0;
    case WEAR_SCABBARD_R:
      return 0;
  }

  return 0;
}

/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char(CHAR_DATA * ch, int iWear)
{
  OBJ_DATA *obj;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->carrying);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->wear_loc == iWear)
      return obj;
  }

  return NULL;
}

/*
 * Equip a char with an obj.
 */
void equip_char(CHAR_DATA *ch, OBJ_DATA *obj, int iWear)
{
  bool pro_vs_blah = TRUE;
  AFFECT_DATA *paf;
  ITERATOR *pIter;
  int sn;

  if (obj->item_type == ITEM_ARMOR)
    sn = obj->value[3];
  else
    sn = obj->value[0] / 1000;

  if (obj->condition < 1)
  {
    act("$p breaks apart and falls to the ground.", ch, obj, NULL, TO_CHAR);
    act("$n drops $p as it falls apart.", ch, obj, NULL, TO_ROOM);
    obj_from_char(obj);
    obj_to_room(obj, ch->in_room);
    return;
  }

  /* only faes can wear faeblasted equipment */
  if (IS_SET(obj->extra_flags, ITEM_FAE_BLAST) && !IS_CLASS(ch, CLASS_FAE))
  {
    act("You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR);
    act("$n is zapped by $p and drops it.", ch, obj, NULL, TO_ROOM);
    obj_from_char(obj);
    obj_to_room(obj, ch->in_room);
    return;
  }

  /* we are being a bit overprotective aren't we */
  if ((IS_OBJ_STAT(obj, ITEM_RARE) || IS_OBJ_STAT(obj, ITEM_SENTIENT)) && !IS_NPC(ch))
  {
    if (getMight(ch) < get_rare_rank_value(obj->cost))
    {
      obj_say(obj, "Stop touching me you freak.", "snarls");
      act("$p leaps from $n's hands.", ch, obj, NULL, TO_ROOM);
      act("$p leaps from your hands.", ch, obj, NULL, TO_CHAR);
      obj_from_char(obj);
      obj_to_room(obj, ch->in_room);
      return;
    }
  }

  /* obj->sentient_points = obj->cost - 100 (ie. ranging from 35 to 180)
   * and (getMight(ch) - 100) / 2 ranges is roughly 500 at almighty,
   * so 4-5 really powerful sentient pieces can be worn at that level.
   */
  if (IS_OBJ_STAT(obj, ITEM_SENTIENT) && !IS_NPC(ch))
  {
    OBJ_DATA *wObj;
    int sent = 0;
    int iWear2;

    for (iWear2 = 0; iWear2 < MAX_WEAR; iWear2++)
    {
      if ((wObj = get_eq_char(ch, iWear2)) != NULL)
      {
        if (IS_OBJ_STAT(wObj, ITEM_SENTIENT))
          sent += wObj->sentient_points;
      }
    }
    if (obj->sentient_points + sent > (getMight(ch) - 100) / 2)
    {
      act("You are zapped by the sentient $p and drop it.", ch, obj, NULL, TO_CHAR);
      act("$n is zapped by the sentient $p and drops it.", ch, obj, NULL, TO_ROOM);
      obj_from_char(obj);
      obj_to_room(obj, ch->in_room);
      return;
    }
  }

  /* can only gain protection if right align */
  if ((sn == OBJECT_PROTECT && !IS_GOOD(ch)) || (sn == OBJECT_PROTECTGOOD && !IS_EVIL(ch)))
  {
    pro_vs_blah = FALSE;
  }

  if (IS_SET(obj->extra_flags, ITEM_MASTERY) && !IS_NPC(ch))
  {
    if (obj->ownerid != ch->pcdata->playerid || !IS_SET(ch->newbits, NEW_MASTERY))
    {
      act("$p leaps from $n to the ground.", ch, obj, NULL, TO_ROOM);
      act("$p leaps from you, and onto the ground.", ch, obj, NULL, TO_CHAR);
      obj_from_char(obj);
      obj_to_room(obj, ch->in_room);
      return;
    }
  }

  if (!IS_NPC(ch) && IS_OBJ_STAT(obj, ITEM_LOYAL))
  {
    if (obj->questowner != NULL && str_cmp(ch->name, obj->questowner) && strlen(obj->questowner) > 1)
    {
      act("$p leaps from $n to the ground.", ch, obj, NULL, TO_ROOM);
      act("$p leaps from you, and onto the ground.", ch, obj, NULL, TO_CHAR);
      obj_from_char(obj);
      obj_to_room(obj, ch->in_room);
      return;
    }
  }

  if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) || (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) || (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)))
  {
    /*
     * Thanks to Morgenes for the bug fix here!
     */
    act("You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR);
    act("$n is zapped by $p and drops it.", ch, obj, NULL, TO_ROOM);
    obj_from_char(obj);
    obj_to_room(obj, ch->in_room);
    return;
  }

  /* zap mobiles if they try to wear relics */
  if (IS_NPC(ch) && IS_SET(obj->quest, QUEST_RELIC))
  {
    act("You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR);
    act("$n is zapped by $p and drops it.", ch, obj, NULL, TO_ROOM);
    obj_from_char(obj);
    obj_to_room(obj, ch->in_room);
    return;
  }

  if (iWear == WEAR_SCABBARD_L || iWear == WEAR_SCABBARD_R)
  {
    obj->wear_loc = iWear;
    return;
  }

  ch->armor -= apply_ac(obj, iWear);
  obj->wear_loc = iWear;

  pIter = AllocIterator(obj->pIndexData->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
    affect_modify(ch, paf, TRUE);

  pIter = AllocIterator(obj->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
    affect_modify(ch, paf, TRUE);

  if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room != NULL)
    ++ch->in_room->light;

  if (obj->wear_loc == WEAR_NONE)
  {
    return;
  }
  if (((obj->item_type == ITEM_ARMOR) && (obj->value[3] >= 1))
      || ((obj->item_type == ITEM_WEAPON) && (obj->value[0] >= 1000)))
  {
    if ((sn == OBJECT_BLIND) && (IS_AFFECTED(ch, AFF_BLIND)))
      return;
    else if ((sn == OBJECT_DETECTINVIS) && (IS_AFFECTED(ch, AFF_DETECT_INVIS)))
      return;
    else if ((sn == OBJECT_FLYING) && (IS_AFFECTED(ch, AFF_FLYING)))
      return;
    else if ((sn == OBJECT_INFRARED) && (IS_AFFECTED(ch, AFF_INFRARED)))
      return;
    else if ((sn == OBJECT_INVISIBLE) && (IS_AFFECTED(ch, AFF_INVISIBLE)))
      return;
    else if ((sn == OBJECT_PASSDOOR) && (IS_AFFECTED(ch, AFF_PASS_DOOR)))
      return;
    else if ((sn == OBJECT_PROTECT) && (IS_AFFECTED(ch, AFF_PROTECT) || !pro_vs_blah))
      return;
    else if ((sn == OBJECT_PROTECTGOOD) && (IS_AFFECTED(ch, AFF_PROTECT_GOOD) || !pro_vs_blah))
      return;
    else if ((sn == OBJECT_SANCTUARY) && (IS_AFFECTED(ch, AFF_SANCTUARY)))
      return;
    else if ((sn == OBJECT_DETECTHIDDEN) && (IS_AFFECTED(ch, AFF_DETECT_HIDDEN)))
      return;
    else if ((sn == OBJECT_SNEAK) && (IS_AFFECTED(ch, AFF_SNEAK)))
      return;
    else if ((sn == OBJECT_CHAOSSHIELD) && (IS_ITEMAFF(ch, ITEMA_CHAOSSHIELD)))
      return;
    else if ((sn == OBJECT_REGENERATE) && (IS_ITEMAFF(ch, ITEMA_REGENERATE)))
      return;
    else if ((sn == OBJECT_SPEED) && (IS_ITEMAFF(ch, ITEMA_SPEED)))
      return;
    else if ((sn == OBJECT_RESISTANCE) && (IS_ITEMAFF(ch, ITEMA_RESISTANCE)))
      return;
    else if ((sn == OBJECT_VISION) && (IS_ITEMAFF(ch, ITEMA_VISION)))
      return;
    else if ((sn == OBJECT_STALKER) && (IS_ITEMAFF(ch, ITEMA_STALKER)))
      return;
    else if ((sn == OBJECT_VANISH) && (IS_ITEMAFF(ch, ITEMA_VANISH)))
      return;

    if (sn == OBJECT_BLIND)
    {
      SET_BIT(ch->affected_by, AFF_BLIND);
      send_to_char("You cannot see a thing!\n\r", ch);
      act("$n seems to be blinded!", ch, NULL, NULL, TO_ROOM);
    }
    else if (sn == OBJECT_DETECTINVIS)
    {
      SET_BIT(ch->affected_by, AFF_DETECT_INVIS);
      send_to_char("Your eyes tingle.\n\r", ch);
      act("$n's eyes flicker with light.", ch, NULL, NULL, TO_ROOM);
    }
    else if (sn == OBJECT_FLYING)
    {
      SET_BIT(ch->affected_by, AFF_FLYING);
      send_to_char("Your feet rise off the ground.\n\r", ch);
      act("$n's feet rise off the ground.", ch, NULL, NULL, TO_ROOM);
    }
    else if (sn == OBJECT_INFRARED)
    {
      SET_BIT(ch->affected_by, AFF_INFRARED);
      send_to_char("Your eyes glow red.\n\r", ch);
      act("$n's eyes glow red.", ch, NULL, NULL, TO_ROOM);
    }
    else if (sn == OBJECT_INVISIBLE)
    {
      SET_BIT(ch->affected_by, AFF_INVISIBLE);
      send_to_char("You fade out of existance.\n\r", ch);
      act("$n fades out of existance.", ch, NULL, NULL, TO_ROOM);
    }
    else if (sn == OBJECT_PASSDOOR)
    {
      SET_BIT(ch->affected_by, AFF_PASS_DOOR);
      send_to_char("You turn translucent.\n\r", ch);
      act("$n turns translucent.", ch, NULL, NULL, TO_ROOM);
    }
    else if (sn == OBJECT_PROTECT)
    {
      SET_BIT(ch->affected_by, AFF_PROTECT);
      send_to_char("You are surrounded by a divine aura.\n\r", ch);
      act("$n is surrounded by a divine aura.", ch, NULL, NULL, TO_ROOM);
    }
    else if (sn == OBJECT_PROTECTGOOD)
    {
      SET_BIT(ch->affected_by, AFF_PROTECT_GOOD);
      send_to_char("You are surrounded by an unholy aura.\n\r", ch);
      act("$n is surrounded by an unholy aura.", ch, NULL, NULL, TO_ROOM);
    }
    else if (sn == OBJECT_SANCTUARY)
    {
      SET_BIT(ch->affected_by, AFF_SANCTUARY);
      send_to_char("You are surrounded by a white aura.\n\r", ch);
      act("$n is surrounded by a white aura.", ch, NULL, NULL, TO_ROOM);
    }
    else if (sn == OBJECT_DETECTHIDDEN)
    {
      SET_BIT(ch->affected_by, AFF_DETECT_HIDDEN);
      send_to_char("You awarenes improves.\n\r", ch);
      act("$n eyes tingle.", ch, NULL, NULL, TO_ROOM);
    }
    else if (sn == OBJECT_SNEAK)
    {
      SET_BIT(ch->affected_by, AFF_SNEAK);
      send_to_char("Your footsteps stop making any sound.\n\r", ch);
      act("$n's footsteps stop making any sound.", ch, NULL, NULL, TO_ROOM);
    }
    else if (sn == OBJECT_CHAOSSHIELD)
    {
      SET_BIT(ch->itemaffect, ITEMA_CHAOSSHIELD);
      send_to_char("You are surrounded by a swirling shield of chaos.\n\r", ch);
      act("$n is surrounded by a swirling shield of chaos.", ch, NULL, NULL, TO_ROOM);
    }
    else if (sn == OBJECT_REGENERATE)
    {
      send_to_char("You feel revived.\n\r", ch);
      SET_BIT(ch->itemaffect, ITEMA_REGENERATE);
    }
    else if (sn == OBJECT_SPEED)
    {
      SET_BIT(ch->itemaffect, ITEMA_SPEED);
      send_to_char("You start moving faster than the eye can follow.\n\r", ch);
      act("$n starts moving faster than the eye can follow.", ch, NULL, NULL, TO_ROOM);
    }
    else if (sn == OBJECT_RESISTANCE)
    {
      SET_BIT(ch->itemaffect, ITEMA_RESISTANCE);
    }
    else if (sn == OBJECT_VISION)
    {
      SET_BIT(ch->itemaffect, ITEMA_VISION);
      send_to_char("Your eyes begin to glow bright white.\n\r", ch);
      act("$n's eyes begin to glow bright white.", ch, NULL, NULL, TO_ROOM);
    }
    else if (sn == OBJECT_STALKER)
    {
      SET_BIT(ch->itemaffect, ITEMA_STALKER);
    }
    else if (sn == OBJECT_VANISH)
    {
      SET_BIT(ch->itemaffect, ITEMA_VANISH);
      send_to_char("You blend into the shadows.\n\r", ch);
      act("$n gradually fades into the shadows.", ch, NULL, NULL, TO_ROOM);
    }
  }
}

/*
 * Unequip a char with an obj.
 */
void unequip_char(CHAR_DATA * ch, OBJ_DATA * obj)
{
  CHAR_DATA *chch;
  OBJ_DATA *cObj;
  ITERATOR *pIter;
  AFFECT_DATA *paf;
  int sn, sn2, i;
  bool spell_remove = TRUE;

  if (obj->wear_loc == WEAR_NONE)
  {
    bug("Unequip_char: already unequipped.", 0);
    return;
  }

  if (obj->wear_loc == WEAR_SCABBARD_L || obj->wear_loc == WEAR_SCABBARD_R)
  {
    obj->wear_loc = -1;
    return;
  }

  ch->armor += apply_ac(obj, obj->wear_loc);
  obj->wear_loc = -1;

  pIter = AllocIterator(obj->pIndexData->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
    affect_modify(ch, paf, FALSE);

  pIter = AllocIterator(obj->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
    affect_modify(ch, paf, FALSE);

  if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room != NULL && ch->in_room->light > 0)
    --ch->in_room->light;

  if ((chch = get_char_world(ch, ch->name)) == NULL)
    return;
  if (chch->desc != ch->desc)
    return;

  if (!IS_NPC(ch) && (ch->desc != NULL && ch->desc->connected != CON_PLAYING))
    return;

  if (((obj->item_type == ITEM_ARMOR) && (obj->value[3] >= 1))
      || ((obj->item_type == ITEM_WEAPON) && (obj->value[0] >= 1000))
      || IS_SET(obj->quest, QUEST_ARTIFACT))
  {
    /* take the arti bonus */
    if (IS_SET(obj->quest, QUEST_ARTIFACT) && !IS_NPC(ch))
      REMOVE_BIT(ch->pcdata->tempflag, TEMP_ARTIFACT);

    if (obj->item_type == ITEM_ARMOR)
      sn = obj->value[3];
    else
      sn = obj->value[0] / 1000;

    /* checking for affects already on other pieces */
    for (i = 0; i < MAX_WEAR; i++)
    {
      if ((cObj = get_eq_char(ch, i)) != NULL) 
      {
        if (cObj == obj) continue;
        
        if (cObj->item_type == ITEM_ARMOR)
          sn2 = cObj->value[3];
        else sn2 = cObj->value[0] / 1000;
         
        if (sn == sn2) spell_remove = FALSE;
      }
    }

    if (spell_remove)
    {
      if (IS_AFFECTED(ch, AFF_BLIND) && sn == OBJECT_BLIND)
      {
        REMOVE_BIT(ch->affected_by, AFF_BLIND);
        send_to_char("You can see again.\n\r", ch);
        act("$n seems to be able to see again.", ch, NULL, NULL, TO_ROOM);
      }
      else if (IS_AFFECTED(ch, AFF_DETECT_INVIS) && sn == OBJECT_DETECTINVIS)
      {
        REMOVE_BIT(ch->affected_by, AFF_DETECT_INVIS);
        send_to_char("Your eyes stop tingling.\n\r", ch);
        act("$n's eyes stop flickering with light.", ch, NULL, NULL, TO_ROOM);
      }
      else if (IS_AFFECTED(ch, AFF_FLYING) && sn == OBJECT_FLYING)
      {
        REMOVE_BIT(ch->affected_by, AFF_FLYING);
        send_to_char("You slowly float to the ground.\n\r", ch);
        act("$n slowly floats to the ground.", ch, NULL, NULL, TO_ROOM);
      }
      else if (IS_AFFECTED(ch, AFF_INFRARED) && sn == OBJECT_INFRARED)
      {
        REMOVE_BIT(ch->affected_by, AFF_INFRARED);
        send_to_char("Your eyes stop glowing red.\n\r", ch);
        act("$n's eyes stop glowing red.", ch, NULL, NULL, TO_ROOM);
      }
      else if (IS_AFFECTED(ch, AFF_INVISIBLE) && sn == OBJECT_INVISIBLE)
      {
        REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
        send_to_char("You fade into existance.\n\r", ch);
        act("$n fades into existance.", ch, NULL, NULL, TO_ROOM);
      }
      else if (IS_AFFECTED(ch, AFF_PASS_DOOR) && sn == OBJECT_PASSDOOR)
      {
        REMOVE_BIT(ch->affected_by, AFF_PASS_DOOR);
        send_to_char("You feel solid again.\n\r", ch);
        act("$n is no longer translucent.", ch, NULL, NULL, TO_ROOM);
      }
      else if (IS_AFFECTED(ch, AFF_PROTECT) && sn == OBJECT_PROTECT)
      {
        REMOVE_BIT(ch->affected_by, AFF_PROTECT);
        send_to_char("The divine aura around you fades.\n\r", ch);
        act("The divine aura around $n fades.", ch, NULL, NULL, TO_ROOM);
      }
      else if (IS_AFFECTED(ch, AFF_PROTECT_GOOD) && sn == OBJECT_PROTECTGOOD)
      {
        REMOVE_BIT(ch->affected_by, AFF_PROTECT_GOOD);
        send_to_char("The unholy aura around you fades.\n\r", ch);
        act("The unholy aura around $n fades.", ch, NULL, NULL, TO_ROOM);
      }
      else if (IS_AFFECTED(ch, AFF_SANCTUARY) && sn == OBJECT_SANCTUARY)
      {
        REMOVE_BIT(ch->affected_by, AFF_SANCTUARY);
        send_to_char("The white aura around your body fades.\n\r", ch);
        act("The white aura about $n's body fades.", ch, NULL, NULL, TO_ROOM);
      }
      else if (IS_AFFECTED(ch, AFF_DETECT_HIDDEN) && sn == OBJECT_DETECTHIDDEN)
      {
        REMOVE_BIT(ch->affected_by, AFF_DETECT_HIDDEN);
        send_to_char("You feel less aware of your surrondings.\n\r", ch);
        act("$n eyes tingle.", ch, NULL, NULL, TO_ROOM);
      }
      else if (IS_AFFECTED(ch, AFF_SNEAK) && sn == OBJECT_SNEAK)
      {
        REMOVE_BIT(ch->affected_by, AFF_SNEAK);
        send_to_char("You are no longer moving so quietly.\n\r", ch);
        act("$n is no longer moving so quietly.", ch, NULL, NULL, TO_ROOM);
      }
      else if (IS_ITEMAFF(ch, ITEMA_CHAOSSHIELD) && sn == OBJECT_CHAOSSHIELD)
      {
        REMOVE_BIT(ch->itemaffect, ITEMA_CHAOSSHIELD);
        send_to_char("The swirling shield of chaos around you fades.\n\r", ch);
        act("The swirling shield of chaos around $n fades.", ch, NULL, NULL, TO_ROOM);
      }
      else if (IS_ITEMAFF(ch, ITEMA_REGENERATE) && sn == OBJECT_REGENERATE)
      {
        REMOVE_BIT(ch->itemaffect, ITEMA_REGENERATE);
      }
      else if (IS_ITEMAFF(ch, ITEMA_SPEED) && sn == OBJECT_SPEED)
      {
        REMOVE_BIT(ch->itemaffect, ITEMA_SPEED);
        send_to_char("Your actions slow down to normal speed.\n\r", ch);
        act("$n stops moving at supernatural speed.", ch, NULL, NULL, TO_ROOM);
      }
      else if (IS_ITEMAFF(ch, ITEMA_RESISTANCE) && sn == OBJECT_RESISTANCE)
      {
        REMOVE_BIT(ch->itemaffect, ITEMA_RESISTANCE);
      }
      else if (IS_ITEMAFF(ch, ITEMA_VISION) && sn == OBJECT_VISION)
      {
        REMOVE_BIT(ch->itemaffect, ITEMA_VISION);
        send_to_char("Your eyes stop glowing bright white.\n\r", ch);
        act("$n's eyes stop glowing bright white.", ch, NULL, NULL, TO_ROOM);
      }
      else if (IS_ITEMAFF(ch, ITEMA_STALKER) && sn == OBJECT_STALKER)
      {
        REMOVE_BIT(ch->itemaffect, ITEMA_STALKER);
      }
      else if (IS_ITEMAFF(ch, ITEMA_VANISH) && sn == OBJECT_VANISH)
      {
        REMOVE_BIT(ch->itemaffect, ITEMA_VANISH);
        send_to_char("You emerge from the shadows.\n\r", ch);
        act("$n gradually fades out of the shadows.", ch, NULL, NULL, TO_ROOM);
      }
    }
  }
}

/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list(OBJ_INDEX_DATA * pObjIndex, LIST * list)
{
  OBJ_DATA *obj;
  ITERATOR *pIter;
  int nMatch = 0;

  pIter = AllocIterator(list);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->pIndexData == pObjIndex)
      nMatch++;
  }

  return nMatch;
}

/*
 * Move an obj out of a room.
 */
void obj_from_room(OBJ_DATA * obj)
{
  ROOM_INDEX_DATA *in_room;

  if (obj == NULL)
    return;

  if ((in_room = obj->in_room) == NULL)
  {
    bug("obj_from_room: NULL.", 0);
    return;
  }

  DetachFromList(obj, in_room->contents);

  obj->in_room = NULL;
}

/*
 * Move an obj into a room.
 */
void obj_to_room(OBJ_DATA * obj, ROOM_INDEX_DATA * pRoomIndex)
{
  if (obj == NULL)
    return;
  if (pRoomIndex == NULL)
    return;

  AttachToList(obj, pRoomIndex->contents);

  obj->in_room = pRoomIndex;
  obj->carried_by = NULL;
  obj->in_obj = NULL;
}

/*
 * Move an object into an object.
 */
void obj_to_obj(OBJ_DATA * obj, OBJ_DATA * obj_to)
{
  if (obj == NULL)
    return;

  AttachToList(obj, obj_to->contains);

  obj->in_obj = obj_to;
  obj->in_room = NULL;
  obj->carried_by = NULL;

  for (; obj_to != NULL; obj_to = obj_to->in_obj)
  {
    if (obj_to->carried_by != NULL)
      obj_to->carried_by->carry_weight += get_obj_weight(obj);
  }
}

void obj_to_obj_end(OBJ_DATA * obj, OBJ_DATA * obj_to)
{
  if (obj == NULL)
    return;

  AttachToEndOfList(obj, obj_to->contains);

  obj->in_obj = obj_to;
  obj->in_room = NULL;
  obj->carried_by = NULL;

  for (; obj_to != NULL; obj_to = obj_to->in_obj)
  {
    if (obj_to->carried_by != NULL)
      obj_to->carried_by->carry_weight += get_obj_weight(obj);
  }
}

/*
 * Move an object out of an object.
 */
void obj_from_obj(OBJ_DATA * obj)
{
  OBJ_DATA *obj_from;

  if (obj == NULL)
    return;
  if ((obj_from = obj->in_obj) == NULL)
  {
    bug("Obj_from_obj: null obj_from.", 0);
    return;
  }

  DetachFromList(obj, obj_from->contains);

  obj->in_obj = NULL;

  for (; obj_from != NULL; obj_from = obj_from->in_obj)
  {
    if (obj_from->carried_by != NULL)
      obj_from->carried_by->carry_weight -= get_obj_weight(obj);
  }
}

/*
 * Extract an obj from the world.
 */
void extract_obj(OBJ_DATA *obj)
{
  OBJ_DATA *obj_content;
  AFFECT_DATA *paf;
  EXTRA_DESCR_DATA *ed;
  EVENT_DATA *event;
  ITERATOR *pIter;

  if (obj == NULL)
    return;

  if (obj->item_type == ITEM_CORPSE_PC && SizeOfList(obj->contains) > 0)
  {
    OBJ_DATA *t_obj;

    pIter = AllocIterator(obj->contains);
    while ((t_obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
    {
      obj_from_obj(t_obj);

      if (obj->in_obj)
        obj_to_obj(t_obj, obj->in_obj);
      else if (obj->carried_by)
        obj_to_char(t_obj, obj->carried_by);
      else if (obj->in_room == NULL)
        extract_obj(t_obj);
      else
        obj_to_room(t_obj, obj->in_room);
    }
  }

  if (obj->in_room != NULL)
    obj_from_room(obj);
  else if (obj->carried_by != NULL)
    obj_from_char(obj);
  else if (obj->in_obj != NULL)
    obj_from_obj(obj);

  /* let's not sacrifice mastery items... */
  if (IS_OBJ_STAT(obj, ITEM_MASTERY) && NoMasteryExtract)
  {
    int ownerid;

    if ((ownerid = obj->ownerid) != 0)
    {
      CHAR_DATA *ch;

      if ((ch = get_online_player(ownerid)) != NULL)
      {
        obj_to_char(obj, ch);
        return;
      }
      else
      {
        bug("extract_obj: mastery item owned by playerid %d extracted.", ownerid);
      }
    }
  }

  /* let's not sacrifice other peoples items */
  if (sacrificer != NULL && obj->ownerid != 0 && obj->ownerid != sacrificer->pcdata->playerid)
  {
    CHAR_DATA *ch;
    ROOM_INDEX_DATA *pRoom = get_room_index(1);

    if ((ch = get_online_player(obj->ownerid)) != NULL)
    {
      obj_to_char(obj, ch);
      return;
    }
    else if (pRoom != NULL)
    {
      obj_to_room(obj, pRoom);
      return;
    }
  }

  DetachFromList(obj, object_list);

  pIter = AllocIterator(obj->contains);
  while ((obj_content = (OBJ_DATA *) NextInList(pIter)) != NULL)
    extract_obj(obj_content);

  pIter = AllocIterator(obj->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    DetachAtIterator(pIter);
    PushStack(paf, affect_free);
  }

  pIter = AllocIterator(obj->extra_descr);
  while ((ed = (EXTRA_DESCR_DATA *) NextInList(pIter)) != NULL)
  {
    free_string(ed->description);
    free_string(ed->keyword);

    DetachAtIterator(pIter);
    PushStack(ed, extra_descr_free);
  }

  pIter = AllocIterator(obj->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
    dequeue_event(event, TRUE);

  obj->pIndexData->count--;
  free_obj(obj);
}

void free_obj(OBJ_DATA *obj)
{
  free_string(obj->name);
  free_string(obj->description);
  free_string(obj->short_descr);
  free_string(obj->questowner);

  FreeList(obj->contains);
  FreeList(obj->events);
  FreeList(obj->affected);
  FreeList(obj->extra_descr);

  PushStack(obj, obj_free);
}

/*
 * Extract a char from the world.
 */
void extract_char(CHAR_DATA * ch, bool fPull)
{
  CHAR_DATA *wch;
  CHAR_DATA *familiar;
  CHAR_DATA *wizard;
  OBJ_DATA *obj;
  ITERATOR *pIter;

  if (ch == NULL)
    return;

  /* do we practice respawning ? */
  if (ch->respawn)
    ch->respawn->repop = TRUE;

  if (ch->in_room == NULL)
  {
    bug("Extract_char: NULL room.", 0);
    return;
  }

  if (fPull)
    die_follower(ch);

  stop_fighting(ch, TRUE);

  NoMasteryExtract = FALSE;
  pIter = AllocIterator(ch->carrying);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    extract_obj(obj);
  }
  NoMasteryExtract = TRUE;

  if (IS_NPC(ch))
    --ch->pIndexData->count;

  if (!fPull)
  {
    bool newbie = in_newbiezone(ch);

    char_from_room(ch);
    if (newbie)
    {
      char_to_room(ch, get_room_index(ROOM_VNUM_SCHOOL), TRUE);
      restore_player(ch);
    }
    else
    {
      KINGDOM_DATA *kingdom;

      if ((kingdom = get_kingdom(ch)) != NULL && kingdom->vnums != 0)
      {
        ROOM_INDEX_DATA *pRoom;

        if ((pRoom = get_room_index(kingdom->vnums)) != NULL)
        {
          char_to_room(ch, pRoom, TRUE);
          return;
        }
      }

      char_to_room(ch, get_room_index(ROOM_VNUM_CITYSAFE), TRUE);
    }

    return;
  }
  else
  {
    char_from_room(ch);
  }

  pIter = AllocIterator(char_list);
  while ((wch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_NPC(wch)) continue;

    if (wch->pcdata->reply == ch)
      wch->pcdata->reply = NULL;
  }

  DetachFromList(ch, char_list);

  if (ch->desc)
    ch->desc->character = NULL;

  if ((wizard = ch->wizard) != NULL)
  {
    if (!IS_NPC(wizard)) wizard->pcdata->familiar = NULL;
    ch->wizard = NULL;
  }

  if (!IS_NPC(ch))
  {
    if ((familiar = ch->pcdata->familiar) != NULL)
    {
      familiar->wizard = NULL;
      ch->pcdata->familiar = NULL;
      if (IS_NPC(familiar))
      {
        act("$n slowly fades away to nothing.", familiar, NULL, NULL, TO_ROOM);
        extract_char(familiar, TRUE);
      }
    }
    if ((familiar = ch->pcdata->partner) != NULL)
      ch->pcdata->partner = NULL;
    if ((familiar = ch->pcdata->propose) != NULL)
      ch->pcdata->propose = NULL;

    pIter = AllocIterator(char_list);
    while ((familiar = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (!IS_NPC(familiar) && familiar->pcdata->propose != NULL && familiar->pcdata->propose == ch)
        familiar->pcdata->propose = NULL;
      if (!IS_NPC(familiar) && familiar->pcdata->partner != NULL && familiar->pcdata->partner == ch)
        familiar->pcdata->partner = NULL;
    }
  }

  free_char(ch);
}

/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *rch;
  ITERATOR *pIter, *pIter2;
  int number;
  int count;

  number = number_argument(argument, arg);
  count = 0;

  if (!str_cmp(arg, "self"))
    return ch;

  pIter = AllocIterator(ch->in_room->people);
  while ((rch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!can_see(ch, rch) || (!is_name(arg, rch->name) && (IS_NPC(rch) || !is_name(arg, rch->name)) && (IS_NPC(rch) || !is_name(arg, rch->morph))))
      continue;

    if (++count == number)
    {
      if (event_isset_mobile(rch, EVENT_PLAYER_DISPLACE) && number_range(1, 4) == 2)
      {
        CHAR_DATA *gch;

        pIter2 = AllocIterator(rch->in_room->people);
        while ((gch = (CHAR_DATA *) NextInList(pIter2)) != NULL)
        {
          if (gch != ch && gch != rch)
            return gch;
        }

        return NULL;
      }
      else
        return rch;
    }
  }

  return NULL;
}

CHAR_DATA *get_char_area(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *wch;
  ITERATOR *pIter;
  int number;
  int count;

  if ((wch = get_char_room(ch, argument)) != NULL)
    return wch;

  number = number_argument(argument, arg);
  count = 0;

  pIter = AllocIterator(char_list);
  while ((wch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (wch->in_room == NULL)
      continue;
    else if (ch->in_room && ch->in_room->area != wch->in_room->area)
      continue;
    else if (!can_see(ch, wch) || !is_name(arg, wch->name))
      continue;

    if (++count == number)
      return wch;
  }

  return NULL;
}

/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  CHAR_DATA *wch;
  int number;
  int count;

  if ((wch = get_char_room(ch, argument)) != NULL)
    return wch;

  number = number_argument(argument, arg);
  count = 0;

  pIter = AllocIterator(char_list);
  while ((wch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (wch->in_room == NULL)
      continue;
    else if (!can_see(ch, wch) || !is_name(arg, wch->name))
      continue;

    if (++count == number)
      return wch;
  }

  return NULL;
}

/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type(OBJ_INDEX_DATA * pObjIndex)
{
  OBJ_DATA *obj;
  ITERATOR *pIter;

  pIter = AllocIterator(object_list);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->pIndexData == pObjIndex)
      return obj;
  }

  return NULL;
}

/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list(CHAR_DATA * ch, char *argument, LIST* list)
{
  char arg[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  OBJ_DATA *obj;
  int number;
  int count;

  number = number_argument(argument, arg);
  count = 0;

  pIter = AllocIterator(list);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (can_see_obj(ch, obj) && is_name(arg, obj->name))
    {
      if (++count == number)
        return obj;
    }
  }

  return NULL;
}

/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  ITERATOR *pIter;
  int number;
  int count;

  number = number_argument(argument, arg);
  count = 0;
  pIter = AllocIterator(ch->carrying);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj) && is_name(arg, obj->name))
    {
      if (++count == number)
        return obj;
    }
  }

  return NULL;
}

/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  ITERATOR *pIter;
  int number;
  int count;

  number = number_argument(argument, arg);
  count = 0;

  pIter = AllocIterator(ch->carrying);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->wear_loc != WEAR_NONE && can_see_obj(ch, obj) && is_name(arg, obj->name))
    {
      if (++count == number)
        return obj;
    }
  }

  return NULL;
}

/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here(CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj;

  obj = get_obj_list(ch, argument, ch->in_room->contents);
  if (obj != NULL)
    return obj;

  if ((obj = get_obj_carry(ch, argument)) != NULL)
    return obj;

  if ((obj = get_obj_wear(ch, argument)) != NULL)
    return obj;

  return NULL;
}

/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_room(CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj;

  obj = get_obj_list(ch, argument, ch->in_room->contents);
  if (obj != NULL)
    return obj;

  return NULL;
}

/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  OBJ_DATA *obj;
  int number;
  int count;

  if ((obj = get_obj_here(ch, argument)) != NULL)
    return obj;

  number = number_argument(argument, arg);
  count = 0;
  pIter = AllocIterator(object_list);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (can_see_obj(ch, obj) && is_name(arg, obj->name))
    {
      if (++count == number)
        return obj;
    }
  }

  return NULL;
}

/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight(OBJ_DATA *obj)
{
  ITERATOR *pIter;
  int weight = obj->weight;

  pIter = AllocIterator(obj->contains);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
    weight += get_obj_weight(obj);

  return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark(ROOM_INDEX_DATA * pRoomIndex)
{
  if (!pRoomIndex)
  {
    bug("Room_is_dark: No room", 0);
    return TRUE;
  }

  if (pRoomIndex->light > 0)
    return FALSE;

  if (IS_SET(pRoomIndex->room_flags, ROOM_LIGHT))
    return FALSE;

  if (IS_SET(pRoomIndex->room_flags, ROOM_DARK))
    return TRUE;

  if (pRoomIndex->sector_type == SECT_INSIDE || pRoomIndex->sector_type == SECT_CITY)
    return FALSE;

  if (weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK)
    return TRUE;

  return FALSE;
}

/*
 * True if room is private.
 */
bool room_is_private(ROOM_INDEX_DATA * pRoomIndex)
{
  CHAR_DATA *rch;
  ITERATOR *pIter;
  int count;

  count = 0;
  pIter = AllocIterator(pRoomIndex->people);
  while ((rch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!IS_NPC(rch))
      count++;
  }

  if (IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE) && count >= 2)
    return TRUE;

  return FALSE;
}

/*
 * True if char can see victim. (we allow ch == NULL)
 */
bool can_see(CHAR_DATA *ch, CHAR_DATA *victim)
{
  if (ch == victim)
    return TRUE;

  if (ch && get_trust(ch) > 6)
    return TRUE;

  /* invis immortals cannot be seen by players */
  if (!IS_NPC(victim) && IS_SET(victim->act, PLR_HIDE) && victim->level > 6)
    return FALSE;

  if (ch && IS_ITEMAFF(ch, ITEMA_VISION))
    return TRUE;
  if (!IS_NPC(victim) && IS_SET(victim->act, PLR_HIDE))
    return FALSE;

  if (ch && ch->in_room != NULL)
  {
    if (IS_SET(ch->in_room->room_flags, ROOM_TOTAL_DARKNESS))
    {
      if (!IS_IMMORTAL(ch))
        return FALSE;
      else
        return TRUE;
    }
  }

  if (ch && IS_EXTRA(ch, BLINDFOLDED))
    return FALSE;

  if (ch && !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
    return TRUE;

  if (ch && IS_HEAD(ch, LOST_EYE_L) && IS_HEAD(ch, LOST_EYE_R))
    return FALSE;

  if (ch && IS_AFFECTED(ch, AFF_BLIND) && !IS_AFFECTED(ch, AFF_SHADOWSIGHT))
    return FALSE;

  if (ch && room_is_dark(ch->in_room) && !IS_AFFECTED(ch, AFF_INFRARED))
    return FALSE;

  if (ch && IS_AFFECTED(victim, AFF_INVISIBLE) && !IS_AFFECTED(ch, AFF_DETECT_INVIS))
    return FALSE;

  return TRUE;
}

/*
 * True if char can see obj.
 */
bool can_see_obj(CHAR_DATA * ch, OBJ_DATA * obj)
{
  CHAR_DATA *gch;

  if ((gch = obj->carried_by) != NULL)
  {
    if (gch->desc != NULL)
    {
      if (gch->desc->connected != CON_PLAYING)
        return FALSE;
    }
  }

  /* for quests */
  if (IS_OBJ_STAT(obj, ITEM_NOLOCATE))
  {
    CHAR_DATA *leader = ch->leader;

    if (IS_NPC(ch))
      return FALSE;
    if (ch->pcdata->playerid != obj->ownerid && !(leader != NULL && leader->pcdata->playerid == obj->ownerid))
      return FALSE;
  }

  if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
    return TRUE;

  if (IS_ITEMAFF(ch, ITEMA_VISION))
    return TRUE;

  if (IS_OBJ_STAT(obj, ITEM_GLOW))
    return TRUE;

  if (obj->item_type == ITEM_POTION)
    return TRUE;

  if (IS_HEAD(ch, LOST_EYE_L) && IS_HEAD(ch, LOST_EYE_R))
    return FALSE;

  if (IS_EXTRA(ch, BLINDFOLDED))
    return FALSE;

  if (IS_AFFECTED(ch, AFF_BLIND) && !IS_AFFECTED(ch, AFF_SHADOWSIGHT))
    return FALSE;

  if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
    return TRUE;

  if (room_is_dark(ch->in_room) && !IS_AFFECTED(ch, AFF_INFRARED))
    return FALSE;

  if (IS_SET(obj->extra_flags, ITEM_INVIS) && !IS_AFFECTED(ch, AFF_DETECT_INVIS))
    return FALSE;

  return TRUE;
}

/*
 * True if char can drop obj.
 */
bool can_drop_obj(CHAR_DATA * ch, OBJ_DATA * obj)
{
  if (!IS_SET(obj->extra_flags, ITEM_NODROP))
    return TRUE;

  if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
    return TRUE;

  return FALSE;
}

/*
 * Return ascii name of an item type.
 */
char *item_type_name(OBJ_DATA * obj)
{
  switch (obj->item_type)
  {
    case ITEM_LIGHT:
      return "light";
    case ITEM_SCROLL:
      return "scroll";
    case ITEM_WAND:
      return "wand";
    case ITEM_STAFF:
      return "staff";
    case ITEM_WEAPON:
      return "weapon";
    case ITEM_TREASURE:
      return "treasure";
    case ITEM_ARMOR:
      return "armor";
    case ITEM_POTION:
      return "potion";
    case ITEM_FURNITURE:
      return "furniture";
    case ITEM_TRASH:
      return "trash";
    case ITEM_CONTAINER:
      return "container";
    case ITEM_DRINK_CON:
      return "drink container";
    case ITEM_KEY:
      return "key";
    case ITEM_FOOD:
      return "food";
    case ITEM_MONEY:
      return "money";
    case ITEM_BOAT:
      return "boat";
    case ITEM_CORPSE_NPC:
      return "npc corpse";
    case ITEM_CORPSE_PC:
      return "pc corpse";
    case ITEM_FOUNTAIN:
      return "fountain";
    case ITEM_PILL:
      return "pill";
    case ITEM_PORTAL:
      return "portal";
    case ITEM_QUEST:
      return "gold token";
    case ITEM_HEAD:
      return "head";
    case ITEM_QUESTCLUE:
      return "quest clue";
    case ITEM_HOMING:
      return "homing device";
    case ITEM_FAETOKEN:
      return "fae token";
  }

  bug("Item_type_name: unknown type %d.", obj->item_type);
  return "(unknown)";
}

/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name(int location)
{
  switch (location)
  {
    case APPLY_NONE:
      return "none";
    case APPLY_STR:
      return "strength";
    case APPLY_DEX:
      return "dexterity";
    case APPLY_INT:
      return "intelligence";
    case APPLY_WIS:
      return "wisdom";
    case APPLY_CON:
      return "constitution";
    case APPLY_SEX:
      return "sex";
    case APPLY_CLASS:
      return "class";
    case APPLY_LEVEL:
      return "level";
    case APPLY_AGE:
      return "age";
    case APPLY_MANA:
      return "mana";
    case APPLY_HIT:
      return "hp";
    case APPLY_MOVE:
      return "moves";
    case APPLY_GOLD:
      return "gold";
    case APPLY_EXP:
      return "experience";
    case APPLY_AC:
      return "armor class";
    case APPLY_HITROLL:
      return "hit roll";
    case APPLY_DAMROLL:
      return "damage roll";
    case APPLY_SAVING_PARA:
      return "save vs paralysis";
    case APPLY_SAVING_ROD:
      return "save vs rod";
    case APPLY_SAVING_PETRI:
      return "save vs petrification";
    case APPLY_SAVING_BREATH:
      return "save vs breath";
    case APPLY_SAVING_SPELL:
      return "save vs spell";
  }

  bug("Affect_location_name: unknown location %d.", location);
  return "(unknown)";
}

/*
 * Return ascii name of an affect bit vector.
 */
char *affect_bit_name(int vector)
{
  static char buf[512];

  buf[0] = '\0';
  if (vector & AFF_BLIND)
    strcat(buf, " blind");
  if (vector & AFF_INVISIBLE)
    strcat(buf, " invisible");
  if (vector & AFF_DETECT_EVIL)
    strcat(buf, " detect_evil");
  if (vector & AFF_DETECT_INVIS)
    strcat(buf, " detect_invis");
  if (vector & AFF_DETECT_MAGIC)
    strcat(buf, " detect_magic");
  if (vector & AFF_DETECT_HIDDEN)
    strcat(buf, " detect_hidden");
  if (vector & AFF_SANCTUARY)
    strcat(buf, " sanctuary");
  if (vector & AFF_FIRESHIELD)
    strcat(buf, " fire_shield");
  if (vector & AFF_INFRARED)
    strcat(buf, " infrared");
  if (vector & AFF_CURSE)
    strcat(buf, " curse");
  if (vector & AFF_FLAMING)
    strcat(buf, " flaming");
  if (vector & AFF_POISON)
    strcat(buf, " poison");
  if (vector & AFF_PROTECT)
    strcat(buf, " protect");
  if (vector & AFF_ETHEREAL)
    strcat(buf, " ethereal");
  if (vector & AFF_SNEAK)
    strcat(buf, " sneak");
  if (vector & AFF_CHARM)
    strcat(buf, " charm");
  if (vector & AFF_FLYING)
    strcat(buf, " flying");
  if (vector & AFF_PASS_DOOR)
    strcat(buf, " pass_door");
  if (vector & AFF_POLYMORPH)
    strcat(buf, " polymorph");
  if (vector & AFF_SHADOWSIGHT)
    strcat(buf, " shadowsight");
  if (vector & AFF_MINDBLANK)
    strcat(buf, " mindblank");
  if (vector & AFF_MINDBOOST)
    strcat(buf, " mindboost");

  return (buf[0] != '\0') ? buf + 1 : "none";
}

/*
 * Return ascii name of extra flags vector.
 */
char *extra_bit_name(int extraflags)
{
  static char buf[512];

  buf[0] = '\0';
  if (extraflags & ITEM_GLOW)
    strcat(buf, " glow");
  if (extraflags & ITEM_HUM)
    strcat(buf, " hum");
  if (extraflags & ITEM_INVIS)
    strcat(buf, " invis");
  if (extraflags & ITEM_MAGIC)
    strcat(buf, " magic");
  if (extraflags & ITEM_NODROP)
    strcat(buf, " nodrop");
  if (extraflags & ITEM_ANTI_GOOD)
    strcat(buf, " anti-good");
  if (extraflags & ITEM_ANTI_EVIL)
    strcat(buf, " anti-evil");
  if (extraflags & ITEM_ANTI_NEUTRAL)
    strcat(buf, " anti-neutral");
  if (extraflags & ITEM_BLESS)
    strcat(buf, " bless");
  if (extraflags & ITEM_NOREMOVE)
    strcat(buf, " noremove");
  if (extraflags & ITEM_INVENTORY)
    strcat(buf, " inventory");
  if (extraflags & ITEM_LOYAL)
    strcat(buf, " loyal");
  if (extraflags & ITEM_NOSHOW)
    strcat(buf, " noshow");
  if (extraflags & ITEM_KEEP)
    strcat(buf, " keep");
  if (extraflags & ITEM_RARE)
    strcat(buf, " rare");
  if (extraflags & ITEM_GEMMED)
    strcat(buf, " gemmed");
  if (extraflags & ITEM_FAE_BLAST)
     strcat(buf, " fae-blasted");
  if (extraflags & ITEM_SENTIENT)
    strcat(buf, " sentient");
  if (extraflags & ITEM_VANISH)
    strcat(buf, " vanish");
  if (extraflags & ITEM_NOCLAIM)
    strcat(buf, " noclaim");
  if (extraflags & ITEM_NOLOCATE)
    strcat(buf, " nolocate");
  if (extraflags & ITEM_UNBREAKABLE)
    strcat(buf, " unbreakable");
  if (extraflags & ITEM_NOREPAIR)
    strcat(buf, " norepair");
  return (buf[0] != '\0') ? buf + 1 : "none";
}
