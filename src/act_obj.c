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

/*
 * Local functions.
 */
#define CD CHAR_DATA

void get_obj     ( CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container );
void sheath      ( CHAR_DATA * ch, bool right );
void draw        ( CHAR_DATA * ch, bool right );

#undef	CD

void do_call(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  CHAR_DATA *gch;
  CHAR_DATA *victim = NULL;
  ROOM_INDEX_DATA *chroom;
  ROOM_INDEX_DATA *objroom;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("What object do you wish to call?\n\r", ch);
    return;
  }
  if (IS_NPC(ch))
    return;

  act("Your eyes flicker with yellow energy.", ch, NULL, NULL, TO_CHAR);
  act("$n's eyes flicker with yellow energy.", ch, NULL, NULL, TO_ROOM);

  WAIT_STATE(ch, 6);

  if (!str_cmp(arg, "all"))
  {
    call_all(ch);
    return;
  }
  if ((obj = get_obj_world(ch, arg)) == NULL)
  {
    send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
    return;
  }
  if (obj->ownerid == 0 || obj->ownerid != ch->pcdata->playerid)
  {
    send_to_char("Nothing happens.\n\r", ch);
    return;
  }
  if (IS_OBJ_STAT(obj, ITEM_NOLOCATE))
  {
    send_to_char("Nothing happens.\n\r", ch);
    return;
  }

  for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);

  if (in_obj->carried_by != NULL)
  {
    if (in_obj->carried_by == ch)
      return;
    if ((gch = in_obj->carried_by) != NULL)
    {
      if (gch->desc && gch->desc->connected != CON_PLAYING)
        return;
    }
  }
  if (obj->carried_by != NULL && obj->carried_by != ch)
  {
    victim = obj->carried_by;
    if (!IS_NPC(victim) && victim->desc != NULL && victim->desc->connected != CON_PLAYING)
      return;
    act("$p suddenly vanishes from your hands!", victim, obj, NULL, TO_CHAR);
    act("$p suddenly vanishes from $n's hands!", victim, obj, NULL, TO_ROOM);
    obj_from_char(obj);
  }
  else if (obj->in_room != NULL)
  {
    chroom = ch->in_room;
    objroom = obj->in_room;
    char_from_room(ch);
    char_to_room(ch, objroom, FALSE);
    act("$p vanishes from the ground!", ch, obj, NULL, TO_ROOM);
    if (chroom == objroom)
      act("$p vanishes from the ground!", ch, obj, NULL, TO_CHAR);
    char_from_room(ch);
    char_to_room(ch, chroom, FALSE);
    obj_from_room(obj);
  }
  else if (obj->in_obj != NULL)
    obj_from_obj(obj);
  else
  {
    send_to_char("Nothing happens.\n\r", ch);
    return;
  }
  obj_to_char(obj, ch);
  act("$p materializes in your hands.", ch, obj, NULL, TO_CHAR);
  act("$p materializes in $n's hands.", ch, obj, NULL, TO_ROOM);
}

void call_all(CHAR_DATA * ch)
{
  CHAR_DATA *gch;
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  ITERATOR *pIter;
  CHAR_DATA *victim = NULL;
  DESCRIPTOR_DATA *d;
  ROOM_INDEX_DATA *chroom;
  ROOM_INDEX_DATA *objroom;
  bool found = FALSE;

  if (IS_NPC(ch))
    return;

  pIter = AllocIterator(object_list);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    bool container_called = FALSE;

    if (obj->ownerid == 0 || obj->ownerid != ch->pcdata->playerid)
      continue;

    if (IS_OBJ_STAT(obj, ITEM_NOLOCATE))
      continue;

    /* find the outermost object (container) */
    for (in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj)
    {
      /* check to see if any of the containers on the way out is
       * owned by the calling player (in which case we don't need
       * to actually call this item)
       */
      if (in_obj != obj && in_obj->ownerid == ch->pcdata->playerid)
        container_called = TRUE;
    }

    if (in_obj->carried_by != NULL)
    {
      if (in_obj->carried_by == ch)
        continue;

      if ((gch = in_obj->carried_by) != NULL)
      {
        if (gch->desc && gch->desc->connected != CON_PLAYING)
          continue;
      }
    }

    /* object is inside an object owned by the calling player,
     * so this object will be called later.
     */
    if (container_called || (in_obj != obj && in_obj->ownerid == ch->pcdata->playerid))
      continue;

    if (obj->carried_by != NULL && obj->carried_by != ch)
    {
      if (obj->carried_by == ch || obj->carried_by->desc == NULL || obj->carried_by->desc->connected != CON_PLAYING)
      {
        if (!IS_NPC(obj->carried_by))
          return;
      }
      found = TRUE;
      act("$p suddenly vanishes from your hands!", obj->carried_by, obj, NULL, TO_CHAR);
      act("$p suddenly vanishes from $n's hands!", obj->carried_by, obj, NULL, TO_ROOM);
      SET_BIT(obj->carried_by->extra, EXTRA_CALL_ALL);
      obj_from_char(obj);
    }
    else if (obj->in_room != NULL)
    {
      chroom = ch->in_room;
      objroom = obj->in_room;
      char_from_room(ch);
      char_to_room(ch, objroom, FALSE);
      act("$p vanishes from the ground!", ch, obj, NULL, TO_ROOM);
      if (chroom == objroom)
        act("$p vanishes from the ground!", ch, obj, NULL, TO_CHAR);
      char_from_room(ch);
      char_to_room(ch, chroom, FALSE);
      obj_from_room(obj);
      found = TRUE;
    }
    else if (obj->in_obj != NULL)
    {
      found = TRUE;
      obj_from_obj(obj);
    }
    else continue;

    found = TRUE;
    obj_to_char(obj, ch);
    act("$p materializes in your hands.", ch, obj, NULL, TO_CHAR);
    act("$p materializes in $n's hands.", ch, obj, NULL, TO_ROOM);
  }

  if (!found)
    send_to_char("Nothing happens.\n\r", ch);

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected != CON_PLAYING)
      continue;
    if ((victim = d->character) == NULL)
      continue;
    if (IS_NPC(victim))
      continue;
    if (ch != victim && !IS_EXTRA(victim, EXTRA_CALL_ALL))
      continue;
    REMOVE_BIT(victim->extra, EXTRA_CALL_ALL);
  }
  return;
}

void get_obj(CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container)
{
  if (!CAN_WEAR(obj, ITEM_TAKE))
  {
    send_to_char("You can't take that.\n\r", ch);
    return;
  }

  if (object_is_affected(obj, OAFF_LIQUID))
  {
    if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT) || !IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_LIQUIFY))
    {
      act("Your hands pass right through $p.", ch, obj, NULL, TO_CHAR);
      return;
    }
  }

  /* only warlocks can handle homing devices */
  if (obj->item_type == ITEM_HOMING && !IS_CLASS(ch, CLASS_WARLOCK))
  {
    send_to_char("You can't take that.\n\r", ch);
    return;
  }

  if (ch->carry_number + 1 > can_carry_n(ch))
  {
    if (ch->sex == SEX_FEMALE)
      sound_to_char("armsfull-f.wav", ch);
    else
      sound_to_char("armsfull-m.wav", ch);
    act("$d: you can't carry that many items.", ch, NULL, obj->name, TO_CHAR);
    return;
  }

  if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch))
  {
    if (ch->sex == SEX_FEMALE)
      sound_to_char("armsfull-f.wav", ch);
    else
      sound_to_char("armsfull-m.wav", ch);
    act("$d: you can't carry that much weight.", ch, NULL, obj->name, TO_CHAR);
    return;
  }

  if (container != NULL)
  {
    act("You get $p from $P.", ch, obj, container, TO_CHAR);
    act("$n gets $p from $P.", ch, obj, container, TO_ROOM);
    obj_from_obj(obj);
  }
  else
  {
    act("You pick up $p.", ch, obj, container, TO_CHAR);
    act("$n picks $p up.", ch, obj, container, TO_ROOM);

    if (obj != NULL)
      obj_from_room(obj);
  }

  if (obj->item_type == ITEM_MONEY)
    extract_obj(obj);
  else
    obj_to_char(obj, ch);
}

void do_get(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  OBJ_DATA *obj;
  OBJ_DATA *container;
  bool found;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (IS_AFFECTED(ch, AFF_ETHEREAL))
  {
    send_to_char("You cannot pick things up while ethereal.\n\r", ch);
    return;
  }

  /* Get type. */
  if (arg1[0] == '\0')
  {
    send_to_char("Get what?\n\r", ch);
    return;
  }

  if (arg2[0] == '\0')
  {
    if (str_cmp(arg1, "all") && str_prefix("all.", arg1))
    {
      /* 'get obj' */
      obj = get_obj_list(ch, arg1, ch->in_room->contents);
      if (obj == NULL)
      {
        act("I see no $T here.", ch, NULL, arg1, TO_CHAR);
        return;
      }
      if (IS_NPC(ch) && IS_SET(obj->quest, QUEST_ARTIFACT))
      {
        send_to_char("You can't pick that up.\n\r", ch);
        return;
      }

      get_obj(ch, obj, NULL);
    }
    else
    {
      /* 'get all' or 'get all.obj' */
      found = FALSE;

      pIter = AllocIterator(ch->in_room->contents);
      while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
      {
        if ((arg1[3] == '\0' || is_name(&arg1[4], obj->name)) && can_see_obj(ch, obj))
        {
          found = TRUE;
          get_obj(ch, obj, NULL);
        }
      }

      if (!found)
      {
        if (arg1[3] == '\0')
          send_to_char("I see nothing here.\n\r", ch);
        else
          act("I see no $T here.", ch, NULL, &arg1[4], TO_CHAR);
      }
    }
  }
  else
  {
    /* 'get ... container' */
    if (!str_cmp(arg2, "all") || !str_prefix("all.", arg2))
    {
      send_to_char("You can't do that.\n\r", ch);
      return;
    }

    if ((container = get_obj_here(ch, arg2)) == NULL)
    {
      act("I see no $T here.", ch, NULL, arg2, TO_CHAR);
      return;
    }

    switch (container->item_type)
    {
      default:
        send_to_char("That's not a container.\n\r", ch);
        return;

      case ITEM_CONTAINER:
      case ITEM_CORPSE_NPC:
      case ITEM_CORPSE_PC:
        break;
    }

    if (IS_SET(container->value[1], CONT_CLOSED))
    {
      act("The $d is closed.", ch, NULL, container->name, TO_CHAR);
      return;
    }

    if (str_cmp(arg1, "all") && str_prefix("all.", arg1))
    {
      /* 'get obj container' */
      obj = get_obj_list(ch, arg1, container->contains);
      if (obj == NULL)
      {
        act("I see nothing like that in the $T.", ch, NULL, arg2, TO_CHAR);
        return;
      }
      get_obj(ch, obj, container);
    }
    else
    {
      /* 'get all container' or 'get all.obj container' */
      found = FALSE;

      pIter = AllocIterator(container->contains);
      while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
      {
        if ((arg1[3] == '\0' || is_name(&arg1[4], obj->name)) && can_see_obj(ch, obj))
        {
          found = TRUE;
          get_obj(ch, obj, container);
        }
      }

      if (!found)
      {
        if (arg1[3] == '\0')
          act("I see nothing in the $T.", ch, NULL, arg2, TO_CHAR);
        else
          act("I see nothing like that in the $T.", ch, NULL, arg2, TO_CHAR);
      }
    }
  }
}

void do_put(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *container;
  OBJ_DATA *obj;
  ITERATOR *pIter;
  const int max_contain = 50;
  int count = 0;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0' || arg2[0] == '\0')
  {
    send_to_char("Put what in what?\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "all") || !str_prefix("all.", arg2))
  {
    send_to_char("You can't do that.\n\r", ch);
    return;
  }

  /* find the container first */
  container = get_obj_carry(ch, arg2); 
  if (container == NULL)
    container = get_obj_wear(ch, arg2);
  if (container == NULL)
    container = get_obj_list(ch, arg2, ch->in_room->contents);

  if (container == NULL)
  {
    act("I see no $T here.", ch, NULL, arg2, TO_CHAR);
    return;
  }

  if (container->item_type != ITEM_CONTAINER)
  {
    send_to_char("That's not a container.\n\r", ch);
    return;
  }

  if (IS_SET(container->value[1], CONT_CLOSED))
  {
    act("The $d is closed.", ch, NULL, container->name, TO_CHAR);
    return;
  }

  /* count the amount of items already in the container */
  count += SizeOfList(container->contains);

  if (str_cmp(arg1, "all") && str_prefix("all.", arg1))
  {
    /* 'put obj container' */
    if ((obj = get_obj_carry(ch, arg1)) == NULL)
    {
      send_to_char("You do not have that item.\n\r", ch);
      return;
    }

    if (obj == container)
    {
      send_to_char("You can't fold it into itself.\n\r", ch);
      return;
    }

    if (obj->item_type == ITEM_CONTAINER && obj->value[0] >= container->value[0])
    {
      send_to_char("You cannot put large containers into small containers.\n\r", ch);
      return;
    }

    if (IS_SET(obj->quest, QUEST_ARTIFACT))
    {
      send_to_char("You cannot put artifacts in a container.\n\r", ch);
      return;
    }

    if (IS_OBJ_STAT(obj, ITEM_OLC))
    {
      send_to_char("You cannot put this item into a container.\n\r", ch);
      return;
    }

    if (IS_OBJ_STAT(obj, ITEM_NOCLAIM))
    {
      send_to_char("This item cannot be put in containers.\n\r", ch);
      return;
    }

    if (!can_drop_obj(ch, obj))
    {
      send_to_char("You can't let go of it.\n\r", ch);
      return;
    }

    if (get_obj_weight(obj) + get_obj_weight(container) > container->value[0])
    {
      send_to_char("It won't fit.\n\r", ch);
      return;
    }

    if (count >= max_contain)
    {
      act("$p is already full, you cannot find place for anything else.", ch, container, NULL, TO_CHAR);
      return;
    }

    obj_from_char(obj);
    obj_to_obj(obj, container);
    act("$n puts $p in $P.", ch, obj, container, TO_ROOM);
    act("You put $p in $P.", ch, obj, container, TO_CHAR);
  }
  else
  {
    /* 'put all container' or 'put all.obj container' */
    pIter = AllocIterator(ch->carrying);
    while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
    {
      if ((arg1[3] == '\0' || is_name(&arg1[4], obj->name))
          && can_see_obj(ch, obj)
          && obj->wear_loc == WEAR_NONE
          && obj != container
          && !IS_SET(obj->quest, QUEST_ARTIFACT)
          && !IS_OBJ_STAT(obj, ITEM_NOCLAIM)
          && !IS_OBJ_STAT(obj, ITEM_OLC)
          && can_drop_obj(ch, obj)
          && obj->item_type != ITEM_CONTAINER
          && get_obj_weight(obj) + get_obj_weight(container) <= container->value[0])
      {
        if (count >= max_contain)
        {
          act("$p is full, you cannot find place for anything else.", ch, container, NULL, TO_CHAR);
          return;
        }

        count++;

        obj_from_char(obj);
        obj_to_obj(obj, container);

        act("$n puts $p in $P.", ch, obj, container, TO_ROOM);
        act("You put $p in $P.", ch, obj, container, TO_CHAR);
      }
    }
  }
}

void do_drop(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  ITERATOR *pIter;
  bool found;

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Drop what?\n\r", ch);
    return;
  }

  if (str_cmp(arg, "all") && str_prefix("all.", arg))
  {
    /* 'drop obj' */
    if ((obj = get_obj_carry(ch, arg)) == NULL)
    {
      send_to_char("You do not have that item.\n\r", ch);
      return;
    }

    if (!can_drop_obj(ch, obj))
    {
      send_to_char("You can't let go of it.\n\r", ch);
      return;
    }

    obj_from_char(obj);
    obj_to_room(obj, ch->in_room);
    act("$n drops $p.", ch, obj, NULL, TO_ROOM);
    act("You drop $p.", ch, obj, NULL, TO_CHAR);
  }
  else
  {
    /* 'drop all' or 'drop all.obj' */
    found = FALSE;

    pIter = AllocIterator(ch->carrying);
    while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
    {
      if ((arg[3] == '\0' || is_name(&arg[4], obj->name)) && can_see_obj(ch, obj) && obj->wear_loc == WEAR_NONE && can_drop_obj(ch, obj))
      {
        found = TRUE;
        obj_from_char(obj);
        obj_to_room(obj, ch->in_room);
        act("$n drops $p.", ch, obj, NULL, TO_ROOM);
        act("You drop $p.", ch, obj, NULL, TO_CHAR);
      }
    }

    if (!found)
    {
      if (arg[3] == '\0')
        act("You are not carrying anything.", ch, NULL, arg, TO_CHAR);
      else
        act("You are not carrying any $T.", ch, NULL, &arg[4], TO_CHAR);
    }
  }
}

void do_give(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0' || arg2[0] == '\0')
  {
    send_to_char("Give what to whom?\n\r", ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg1)) == NULL)
  {
    send_to_char("You do not have that item.\n\r", ch);
    return;
  }

  if (obj->wear_loc != WEAR_NONE)
  {
    send_to_char("You must remove it first.\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg2)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (IS_SET(obj->quest, QUEST_ARTIFACT))
  {
    obj_say(obj, "Please don't give me away, I love you.", "whines");
    return;
  }

  if (!can_drop_obj(ch, obj))
  {
    send_to_char("You can't let go of it.\n\r", ch);
    return;
  }

  if (IS_AFFECTED(victim, AFF_ETHEREAL))
  {
    send_to_char("You cannot give things to ethereal people.\n\r", ch);
    return;
  }

  if (victim->carry_number + 1 > can_carry_n(victim))
  {
    act("$N has $S hands full.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if (victim->shop_fun)
  {
    send_to_char("They don't want it.\n\r", ch);
    return;
  }

  if (victim->carry_weight + get_obj_weight(obj) > can_carry_w(victim))
  {
    act("$N can't carry that much weight.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if (!can_see_obj(victim, obj))
  {
    act("$N can't see it.", ch, NULL, victim, TO_CHAR);
    return;
  }

  obj_from_char(obj);
  obj_to_char(obj, victim);
  act("$n gives $p to $N.", ch, obj, victim, TO_NOTVICT);
  act("$n gives you $p.", ch, obj, victim, TO_VICT);
  act("You give $p to $N.", ch, obj, victim, TO_CHAR);
}

void do_fill(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *fountain;
  ITERATOR *pIter;
  bool found;
  int liquid;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Fill what?\n\r", ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You do not have that item.\n\r", ch);
    return;
  }

  found = FALSE;
  pIter = AllocIterator(ch->in_room->contents);
  while ((fountain = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (fountain->item_type == ITEM_FOUNTAIN)
    {
      found = TRUE;
      break;
    }
  }

  if (!found)
  {
    send_to_char("There is no fountain here!\n\r", ch);
    return;
  }

  else if (IS_AFFECTED(ch, AFF_ETHEREAL))
  {
    send_to_char("You cannot fill containers while ethereal.\n\r", ch);
    return;
  }

  if (obj->item_type != ITEM_DRINK_CON)
  {
    send_to_char("You can't fill that.\n\r", ch);
    return;
  }

  if (obj->value[1] >= obj->value[0])
  {
    send_to_char("Your container is already full.\n\r", ch);
    return;
  }

  if ((obj->value[2] != fountain->value[2]) && obj->value[1] > 0)
  {
    send_to_char("You cannot mix two different liquids.\n\r", ch);
    return;
  }

  act("$n dips $p into $P.", ch, obj, fountain, TO_ROOM);
  act("You dip $p into $P.", ch, obj, fountain, TO_CHAR);
  obj->value[2] = fountain->value[2];
  obj->value[1] = obj->value[0];
  liquid = obj->value[2];
  act("$n fills $p with $T.", ch, obj, liq_table[liquid].liq_name, TO_ROOM);
  act("You fill $p with $T.", ch, obj, liq_table[liquid].liq_name, TO_CHAR);
  return;
}

void do_drink(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  ITERATOR *pIter;
  int amount;
  int liquid;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    pIter = AllocIterator(ch->in_room->contents);
    while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
    {
      if (obj->item_type == ITEM_FOUNTAIN)
        break;
    }

    if (obj == NULL)
    {
      send_to_char("Drink what?\n\r", ch);
      return;
    }
  }
  else
  {
    if ((obj = get_obj_here(ch, arg)) == NULL)
    {
      send_to_char("You can't find it.\n\r", ch);
      return;
    }
  }

  if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
  {
    send_to_char("You fail to reach your mouth.  *Hic*\n\r", ch);
    return;
  }

  switch (obj->item_type)
  {
    default:
      send_to_char("You can't drink from that.\n\r", ch);
      break;

    case ITEM_POTION:
      do_quaff(ch, obj->name);
      return;
    case ITEM_FOUNTAIN:
      if ((liquid = obj->value[2]) >= LIQ_MAX)
      {
        bug("Do_drink: bad liquid number %d.", liquid);
        liquid = obj->value[2] = 0;
      }

      else if (IS_AFFECTED(ch, AFF_ETHEREAL))
      {
        send_to_char("You can only drink from things you are carrying while ethereal.\n\r", ch);
        return;
      }

      act("$n drinks $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_ROOM);
      act("You drink $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_CHAR);

      amount = number_range(25, 50);
      amount = UMIN(amount, obj->value[1]);

      if (!IS_NPC(ch))
      {
        ch->pcdata->condition[COND_DRUNK] += amount * liq_table[liquid].liq_affect[COND_DRUNK];

        if (ch->pcdata->condition[COND_DRUNK] > 10)
          send_to_char("You feel drunk.\n\r", ch);
      }

      if (obj->value[3] != 0 && !IS_NPC(ch))
      {
        /* The shit was poisoned ! */
        AFFECT_DATA af;

        act("$n chokes and gags.", ch, NULL, NULL, TO_ROOM);
        send_to_char("You choke and gag.\n\r", ch);
        af.type = gsn_poison;
        af.duration = 3 * amount;
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = AFF_POISON;
        affect_join(ch, &af);
      }
      break;

    case ITEM_DRINK_CON:
      if (obj->value[1] <= 0)
      {
        send_to_char("It is already empty.\n\r", ch);
        return;
      }

      if ((liquid = obj->value[2]) >= LIQ_MAX)
      {
        bug("Do_drink: bad liquid number %d.", liquid);
        liquid = obj->value[2] = 0;
      }

      act("$n drinks $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_ROOM);
      act("You drink $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_CHAR);

      amount = number_range(25, 50);
      amount = UMIN(amount, obj->value[1]);

      if (!IS_NPC(ch))
      {
        ch->pcdata->condition[COND_DRUNK] += amount * liq_table[liquid].liq_affect[COND_DRUNK];

        if (ch->pcdata->condition[COND_DRUNK] > 10)
          send_to_char("You feel drunk.\n\r", ch);
      }

      if (obj->value[3] != 0 && !IS_NPC(ch))
      {
        /* The shit was poisoned ! */
        AFFECT_DATA af;

        act("$n chokes and gags.", ch, NULL, NULL, TO_ROOM);
        send_to_char("You choke and gag.\n\r", ch);
        af.type = gsn_poison;
        af.duration = 3 * amount;
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = AFF_POISON;
        affect_join(ch, &af);
      }

      obj->value[1] -= amount;
      if (obj->value[1] <= 0)
      {
        obj->value[1] = 0;
      }
      break;
  }

  return;
}

void do_empty(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int liquid;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Empty what?\n\r", ch);
    return;
  }

  if ((obj = get_obj_here(ch, arg)) == NULL)
  {
    send_to_char("You can't find it.\n\r", ch);
    return;
  }

  switch (obj->item_type)
  {
    default:
      send_to_char("You cannot empty that.\n\r", ch);
      break;

    case ITEM_DRINK_CON:
      if (obj->value[1] <= 0)
      {
        send_to_char("It is already empty.\n\r", ch);
        return;
      }

      if ((liquid = obj->value[2]) >= LIQ_MAX)
      {
        bug("Do_drink: bad liquid number %d.", liquid);
        liquid = obj->value[2] = 0;
      }

      act("$n empties $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_ROOM);
      act("You empty $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_CHAR);

      obj->value[1] = 0;
      break;
  }

  return;
}

void do_eat(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int level;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Eat what?\n\r", ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You do not have that item.\n\r", ch);
    return;
  }

  if (!IS_IMMORTAL(ch))
  {
    if (obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL && obj->item_type != ITEM_QUEST)
    {
      send_to_char("That's not edible.\n\r", ch);
      return;
    }

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 50 && obj->item_type != ITEM_TRASH && obj->item_type != ITEM_QUEST && obj->item_type != ITEM_PILL)
    {
      send_to_char("You are too full to eat more.\n\r", ch);
      return;
    }
    if (!IS_NPC(ch) && obj->item_type != ITEM_QUEST && obj->item_type != ITEM_PILL && obj->item_type != ITEM_FOOD)
    {
      send_to_char("You can't eat that.\n\r", ch);
      return;
    }
  }

  if (obj->item_type != ITEM_QUEST)
  {
    act("$n eats $p.", ch, obj, NULL, TO_ROOM);
    act("You eat $p.", ch, obj, NULL, TO_CHAR);
  }

  switch (obj->item_type)
  {
    default:
      break;

    case ITEM_FOOD:
      if (obj->value[3] != 0)
      {
        /* The shit was poisoned! */
        AFFECT_DATA af;

        act("$n chokes and gags.", ch, NULL, NULL, TO_ROOM);
        send_to_char("You choke and gag.\n\r", ch);

        af.type = gsn_poison;
        af.duration = 2 * obj->value[0];
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = AFF_POISON;
        affect_join(ch, &af);
      }
      break;

    case ITEM_PILL:
      level = obj->value[0];
      if (level < 1)
        level = 1;
      if (level > MAX_SPELL)
        level = MAX_SPELL;

      obj_cast_spell(obj->value[1], level, ch, ch, NULL);
      obj_cast_spell(obj->value[2], level, ch, ch, NULL);
      obj_cast_spell(obj->value[3], level, ch, ch, NULL);
      if (ch->position == POS_FIGHTING)
      {
        WAIT_STATE(ch, 6);
      }
      break;

    case ITEM_QUEST:
      if (!IS_NPC(ch))
      {
        deposit(ch, obj);
        return;
      }
      break;
  }

  if (obj != NULL)
  {
    REMOVE_BIT(obj->extra_flags, ITEM_MASTERY);
    extract_obj(obj);
  }
}

/*
 * Remove an object.
 */
bool remove_obj(CHAR_DATA * ch, int iWear, bool fReplace)
{
  OBJ_DATA *obj;

  if ((obj = get_eq_char(ch, iWear)) == NULL)
    return TRUE;

  if (!fReplace)
    return FALSE;

  if (IS_SET(obj->extra_flags, ITEM_NOREMOVE))
  {
    act("You can't remove $p.", ch, obj, NULL, TO_CHAR);
    return FALSE;
  }

  unequip_char(ch, obj);
  act("$n stops using $p.", ch, obj, NULL, TO_ROOM);
  act("You stop using $p.", ch, obj, NULL, TO_CHAR);
  return TRUE;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
bool wear_obj(CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace)
{
  bool wolf_ok = FALSE;

  /* special fortress contest check */
  if (in_fortress(ch) && IS_SET(arena.status, ARENA_FORTRESS_CONTEST))
  {
    /* only OLC items (ie. arena gear) and mastery items are allowed */
    if (!IS_OBJ_STAT(obj, ITEM_OLC) && !IS_OBJ_STAT(obj, ITEM_MASTERY))
    {
      send_to_char("You are unable to use it.\n\r", ch);
      return TRUE;
    }
  }

  if (object_is_affected(obj, OAFF_LIQUID))
  {
    if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT) || !IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_LIQUIFY))
    {
      act("Your hands pass right through $p.", ch, obj, NULL, TO_CHAR);
      return TRUE;
    }
  }

  if (CAN_WEAR(obj, ITEM_WIELD) || CAN_WEAR(obj, ITEM_HOLD))
  {
    if (get_eq_char(ch, WEAR_WIELD) != NULL
        && get_eq_char(ch, WEAR_HOLD) != NULL
        && !remove_obj(ch, WEAR_WIELD, fReplace)
        && !remove_obj(ch, WEAR_HOLD, fReplace))
      return FALSE;

    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WIELD))
    {
      send_to_char("You are unable to use it.\n\r", ch);
      return TRUE;
    }

    if (get_eq_char(ch, WEAR_WIELD) == NULL && is_ok_to_wear(ch, wolf_ok, "right_hand"))
    {
      act("$n clutches $p in $s right hand.", ch, obj, NULL, TO_ROOM);
      act("You clutch $p in your right hand.", ch, obj, NULL, TO_CHAR);

      if (obj->item_type == ITEM_WEAPON)
      {
        if (!IS_NPC(ch) && IS_OBJ_STAT(obj, ITEM_LOYAL))
        {
          if (obj->ownerid > 0 && ch->pcdata->playerid != obj->ownerid)
          {
            act("$p leaps out of $n's hand.", ch, obj, NULL, TO_ROOM);
            act("$p leaps out of your hand.", ch, obj, NULL, TO_CHAR);
            obj_from_char(obj);
            obj_to_room(obj, ch->in_room);
            return TRUE;
          }
        }

        equip_char(ch, obj, WEAR_WIELD);
        if (!IS_NPC(ch))
          do_skill(ch, ch->name);
        return TRUE;
      }
      equip_char(ch, obj, WEAR_WIELD);
      return TRUE;
    }
    else if (get_eq_char(ch, WEAR_HOLD) == NULL && is_ok_to_wear(ch, wolf_ok, "left_hand"))
    {
      act("$n clutches $p in $s left hand.", ch, obj, NULL, TO_ROOM);
      act("You clutch $p in your left hand.", ch, obj, NULL, TO_CHAR);

      if (obj->item_type == ITEM_WEAPON)
      {
        if (!IS_NPC(ch) && IS_OBJ_STAT(obj, ITEM_LOYAL))
        {
          if (obj->ownerid > 0 && ch->pcdata->playerid != obj->ownerid)
          {
            act("$p leaps out of $n's hand.", ch, obj, NULL, TO_ROOM);
            act("$p leaps out of your hand.", ch, obj, NULL, TO_CHAR);
            obj_from_char(obj);
            obj_to_room(obj, ch->in_room);
            return TRUE;
          }
        }

        equip_char(ch, obj, WEAR_HOLD);
        if (!IS_NPC(ch))
          do_skill(ch, ch->name);
        return TRUE;
      }
      equip_char(ch, obj, WEAR_HOLD);
      return TRUE;
    }
    else if (get_eq_char(ch, WEAR_THIRD) == NULL && is_ok_to_wear(ch, wolf_ok, "third_hand"))
    {
      act("$n clutches $p in $s third hand.", ch, obj, NULL, TO_ROOM);
      act("You clutch $p in your third hand.", ch, obj, NULL, TO_CHAR);

      if (obj->item_type == ITEM_WEAPON)
      {
        if (!IS_NPC(ch) && IS_OBJ_STAT(obj, ITEM_LOYAL))
        {
          if (obj->ownerid > 0 && ch->pcdata->playerid != obj->ownerid)
          {
            act("$p leaps out of $n's hand.", ch, obj, NULL, TO_ROOM);
            act("$p leaps out of your hand.", ch, obj, NULL, TO_CHAR);
            obj_from_char(obj);
            obj_to_room(obj, ch->in_room);
            return TRUE;
          }
        }
        equip_char(ch, obj, WEAR_THIRD);
        if (!IS_NPC(ch))
          do_skill(ch, ch->name);
        return TRUE;
      }

      if (!IS_NPC(ch))
        do_skill(ch, ch->name);
      equip_char(ch, obj, WEAR_THIRD);
      return TRUE;
    }
    else if (get_eq_char(ch, WEAR_FOURTH) == NULL && is_ok_to_wear(ch, wolf_ok, "fourth_hand"))
    {
      act("$n clutches $p in $s fourth hand.", ch, obj, NULL, TO_ROOM);
      act("You clutch $p in your fourth hand.", ch, obj, NULL, TO_CHAR);

      if (obj->item_type == ITEM_WEAPON)
      {
        if (!IS_NPC(ch) && IS_OBJ_STAT(obj, ITEM_LOYAL))
        {
          if (obj->ownerid > 0 && ch->pcdata->playerid != obj->ownerid)
          {
            act("$p leaps out of $n's hand.", ch, obj, NULL, TO_ROOM);
            act("$p leaps out of your hand.", ch, obj, NULL, TO_CHAR);
            obj_from_char(obj);
            obj_to_room(obj, ch->in_room);
            return TRUE;
          }
        }
        equip_char(ch, obj, WEAR_FOURTH);
        if (!IS_NPC(ch))
          do_skill(ch, ch->name);
        return TRUE;
      }
      equip_char(ch, obj, WEAR_FOURTH);
      return TRUE;
    }
    if (!is_ok_to_wear(ch, wolf_ok, "left_hand") && !is_ok_to_wear(ch, wolf_ok, "right_hand"))
      send_to_char("You cannot use anything in your hands.\n\r", ch);
    else
      send_to_char("You have no free hands.\n\r", ch);
    return TRUE;
  }
  if (obj->item_type == ITEM_LIGHT)
  {
    if (!remove_obj(ch, WEAR_LIGHT, fReplace))
      return FALSE;
    act("$n lights $p and holds it.", ch, obj, NULL, TO_ROOM);
    act("You light $p and hold it.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_LIGHT);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_FINGER))
  {
    if (get_eq_char(ch, WEAR_FINGER_L) != NULL && get_eq_char(ch, WEAR_FINGER_R) != NULL && !remove_obj(ch, WEAR_FINGER_L, fReplace) && !remove_obj(ch, WEAR_FINGER_R, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_FINGER))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }

    if (get_eq_char(ch, WEAR_FINGER_L) == NULL && is_ok_to_wear(ch, wolf_ok, "left_finger"))
    {
      act("$n wears $p on $s left finger.", ch, obj, NULL, TO_ROOM);
      act("You wear $p on your left finger.", ch, obj, NULL, TO_CHAR);
      equip_char(ch, obj, WEAR_FINGER_L);
      return TRUE;
    }
    else if (get_eq_char(ch, WEAR_FINGER_R) == NULL && is_ok_to_wear(ch, wolf_ok, "right_finger"))
    {
      act("$n wears $p on $s right finger.", ch, obj, NULL, TO_ROOM);
      act("You wear $p on your right finger.", ch, obj, NULL, TO_CHAR);
      equip_char(ch, obj, WEAR_FINGER_R);
      return TRUE;
    }
    if (!is_ok_to_wear(ch, wolf_ok, "left_finger") && !is_ok_to_wear(ch, wolf_ok, "right_finger"))
      send_to_char("You cannot wear any rings.\n\r", ch);
    else
      send_to_char("You cannot wear any more rings.\n\r", ch);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_NECK))
  {
    if (get_eq_char(ch, WEAR_NECK_1) != NULL && get_eq_char(ch, WEAR_NECK_2) != NULL && !remove_obj(ch, WEAR_NECK_1, fReplace) && !remove_obj(ch, WEAR_NECK_2, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_NECK))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }

    if (get_eq_char(ch, WEAR_NECK_1) == NULL)
    {
      act("$n slips $p around $s neck.", ch, obj, NULL, TO_ROOM);
      act("You slip $p around your neck.", ch, obj, NULL, TO_CHAR);
      equip_char(ch, obj, WEAR_NECK_1);
      return TRUE;
    }

    if (get_eq_char(ch, WEAR_NECK_2) == NULL)
    {
      act("$n slips $p around $s neck.", ch, obj, NULL, TO_ROOM);
      act("You slip $p around your neck.", ch, obj, NULL, TO_CHAR);
      equip_char(ch, obj, WEAR_NECK_2);
      return TRUE;
    }
    bug("Wear_obj: no free neck.", 0);
    send_to_char("You are already wearing two things around your neck.\n\r", ch);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_BODY))
  {
    if (!remove_obj(ch, WEAR_BODY, fReplace))
      return FALSE;

    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_BODY))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }
    act("$n fits $p on $s body.", ch, obj, NULL, TO_ROOM);
    act("You fit $p on your body.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_BODY);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_HEAD))
  {
    if (!remove_obj(ch, WEAR_HEAD, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_HEAD))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }
    if (!is_ok_to_wear(ch, wolf_ok, "head"))
    {
      send_to_char("You have no head to wear it on.\n\r", ch);
      return TRUE;
    }
    act("$n places $p on $s head.", ch, obj, NULL, TO_ROOM);
    act("You place $p on your head.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_HEAD);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_FACE))
  {
    if (!remove_obj(ch, WEAR_FACE, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_HEAD))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }
    if (!is_ok_to_wear(ch, wolf_ok, "face"))
    {
      send_to_char("You have no face to wear it on.\n\r", ch);
      return TRUE;
    }
    act("$n places $p on $s face.", ch, obj, NULL, TO_ROOM);
    act("You place $p on your face.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_FACE);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_LEGS))
  {
    if (!remove_obj(ch, WEAR_LEGS, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_LEGS))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }
    if (!is_ok_to_wear(ch, wolf_ok, "legs"))
    {
      send_to_char("You have no legs to wear them on.\n\r", ch);
      return TRUE;
    }
    act("$n slips $s legs into $p.", ch, obj, NULL, TO_ROOM);
    act("You slip your legs into $p.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_LEGS);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_FEET))
  {
    if (!remove_obj(ch, WEAR_FEET, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_FEET))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }
    if (!is_ok_to_wear(ch, wolf_ok, "feet"))
    {
      send_to_char("You have no feet to wear them on.\n\r", ch);
      return TRUE;
    }
    act("$n slips $s feet into $p.", ch, obj, NULL, TO_ROOM);
    act("You slip your feet into $p.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_FEET);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_HANDS))
  {
    if (!remove_obj(ch, WEAR_HANDS, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_HANDS))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }
    if (!is_ok_to_wear(ch, wolf_ok, "hands"))
    {
      send_to_char("You have no hands to wear them on.\n\r", ch);
      return TRUE;
    }
    act("$n pulls $p onto $s hands.", ch, obj, NULL, TO_ROOM);
    act("You pull $p onto your hands.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_HANDS);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_ARMS))
  {
    if (!remove_obj(ch, WEAR_ARMS, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_ARMS))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }
    if (!is_ok_to_wear(ch, wolf_ok, "arms"))
    {
      send_to_char("You have no arms to wear them on.\n\r", ch);
      return TRUE;
    }
    act("$n slides $s arms into $p.", ch, obj, NULL, TO_ROOM);
    act("You slide your arms into $p.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_ARMS);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_ABOUT))
  {
    if (!remove_obj(ch, WEAR_ABOUT, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_ABOUT))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }
    act("$n pulls $p about $s body.", ch, obj, NULL, TO_ROOM);
    act("You pull $p about your body.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_ABOUT);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_WAIST))
  {
    if (!remove_obj(ch, WEAR_WAIST, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_WAIST))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }
    act("$n ties $p around $s waist.", ch, obj, NULL, TO_ROOM);
    act("You tie $p around your waist.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_WAIST);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_MASTERY))
  {
    if (!remove_obj(ch, WEAR_MASTERY, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_MASTERY))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }
    act("$n holds $p in $s hands.", ch, obj, NULL, TO_ROOM);
    act("You hold onto $p.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_MASTERY);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_FLOAT))
  {
    if (!remove_obj(ch, WEAR_FLOAT, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_FLOAT))
    {
      send_to_char("You are unable to wear it, perhaps you suck\n\r", ch);
      return TRUE;
    }
    act("$n throws $p into the air and it starts flying about.", ch, obj, NULL, TO_ROOM);
    act("You throw $p into the air and it starts circling around.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_FLOAT);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_MEDAL))
  {
    if (!remove_obj(ch, WEAR_MEDAL, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_MEDAL))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }
    act("$n puts $p on $s uniform.", ch, obj, NULL, TO_ROOM);
    act("You put $p on your uniform.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_MEDAL);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_BODYART))
  {
    if (!remove_obj(ch, WEAR_BODYART, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_BODYART))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }
    act("$n screams in agony as $p bores into $s body.", ch, obj, NULL, TO_ROOM);
    act("You scream in agony as $p bores into your body.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_BODYART);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_WRIST))
  {
    if (get_eq_char(ch, WEAR_WRIST_L) != NULL && get_eq_char(ch, WEAR_WRIST_R) != NULL && !remove_obj(ch, WEAR_WRIST_L, fReplace) && !remove_obj(ch, WEAR_WRIST_R, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_WRIST))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }

    if (get_eq_char(ch, WEAR_WRIST_L) == NULL && is_ok_to_wear(ch, wolf_ok, "right_wrist"))
    {
      act("$n slides $s left wrist into $p.", ch, obj, NULL, TO_ROOM);
      act("You slide your left wrist into $p.", ch, obj, NULL, TO_CHAR);
      equip_char(ch, obj, WEAR_WRIST_L);
      return TRUE;
    }
    else if (get_eq_char(ch, WEAR_WRIST_R) == NULL && is_ok_to_wear(ch, wolf_ok, "left_wrist"))
    {
      act("$n slides $s left wrist into $p.", ch, obj, NULL, TO_ROOM);
      act("You slide your right wrist into $p.", ch, obj, NULL, TO_CHAR);
      equip_char(ch, obj, WEAR_WRIST_R);
      return TRUE;
    }
    if (!is_ok_to_wear(ch, wolf_ok, "left_wrist") && !is_ok_to_wear(ch, wolf_ok, "right_wrist"))
      send_to_char("You cannot wear anything on your wrists.\n\r", ch);
    else
      send_to_char("You cannot wear any more on your wrists.\n\r", ch);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WEAR_SHIELD))
  {
    if (!remove_obj(ch, WEAR_SHIELD, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WEAR_SHIELD))
    {
      send_to_char("You are unable to wear it.\n\r", ch);
      return TRUE;
    }
    act("$n straps $p onto $s shield arm.", ch, obj, NULL, TO_ROOM);
    act("You strap $p onto your shield arm.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_SHIELD);
    return TRUE;
  }

  if (CAN_WEAR(obj, ITEM_WIELD))
  {
    if (!remove_obj(ch, WEAR_WIELD, fReplace))
      return FALSE;
    if (!IS_NPC(ch) && !IS_FORM(ch, ITEM_WIELD))
    {
      send_to_char("You are unable to wield it.\n\r", ch);
      return TRUE;
    }

    if (get_obj_weight(obj) > (5 * UMIN(25, get_curr_str(ch))) / 2)
    {
      send_to_char("It is too heavy for you to wield.\n\r", ch);
      return TRUE;
    }

    act("$n wields $p.", ch, obj, NULL, TO_ROOM);
    act("You wield $p.", ch, obj, NULL, TO_CHAR);

    if (!IS_NPC(ch) && IS_OBJ_STAT(obj, ITEM_LOYAL))
    {
      if (obj->ownerid > 0 && ch->pcdata->playerid != obj->ownerid)
      {
        act("$p leaps out of $n's hand.", ch, obj, NULL, TO_ROOM);
        act("$p leaps out of your hand.", ch, obj, NULL, TO_CHAR);
        obj_from_char(obj);
        obj_to_room(obj, ch->in_room);
        return TRUE;
      }
    }
    equip_char(ch, obj, WEAR_WIELD);
    if (!IS_NPC(ch))
      do_skill(ch, ch->name);
    return TRUE;
  }

  if (fReplace)
    send_to_char("You can't wear, wield or hold that.\n\r", ch);

  return FALSE;
}

void do_wear(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  one_argument(argument, arg);

  if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_POLYMORPH) && IS_CLASS(ch, CLASS_GIANT) &&
      !event_isset_mobile(ch, EVENT_PLAYER_WATERFLUX) &&
      !event_isset_mobile(ch, EVENT_PLAYER_EARTHFLUX))
  {
    send_to_char("You cannot wear anything while dawnstrength is enabled.\n\r", ch);
    return;
  }

  if (arg[0] == '\0')
  {
    send_to_char("Wear, wield, or hold what?\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "all"))
  {
    bool found = FALSE;
    bool noitems = TRUE;
    ITERATOR *pIter;

    pIter = AllocIterator(ch->carrying);
    while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
    {
      if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj))
      {
        noitems = FALSE;

        if (wear_obj(ch, obj, FALSE))
          found = TRUE;
      }
    }

    if (noitems)
      send_to_char("You have no more items to wear.\n\r", ch);
    else if (!found)
      send_to_char("You cannot wear anything else.\n\r", ch);

    return;
  }
  else
  {
    if ((obj = get_obj_carry(ch, arg)) == NULL)
    {
      send_to_char("You do not have that item.\n\r", ch);
      return;
    }

    wear_obj(ch, obj, TRUE);
  }
}

void do_remove(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Remove what?\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "all"))
  {
    ITERATOR *pIter;

    pIter = AllocIterator(ch->carrying);
    while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
    {
      if (obj->wear_loc != WEAR_NONE && can_see_obj(ch, obj))
      {
        remove_obj(ch, obj->wear_loc, TRUE);
      }
    }
    return;
  }
  if ((obj = get_obj_wear(ch, arg)) == NULL)
  {
    send_to_char("You do not have that item.\n\r", ch);
    return;
  }
  remove_obj(ch, obj->wear_loc, TRUE);
  return;
}

void do_sacrifice(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  ITERATOR *pIter;
  int i = 0;

  argument = one_argument(argument, arg);

  if (IS_NPC(ch))
    return;

  if (arg[0] == '\0')
  {
    send_to_char("Sacrifice what?\n\r", ch);
    return;
  }
  if (IS_AFFECTED(ch, AFF_CHARM))
  {
    send_to_char("Not while charmed.\n\r", ch);
    return;
  }
  if (!str_cmp(arg, "all") || !str_prefix("all.", arg))
  {
    /* set the global sacrificer variable */
    if (!IS_NPC(ch)) sacrificer = ch;

    pIter = AllocIterator(ch->in_room->contents);
    while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
    {
      if (arg[3] != '\0' && !is_name(&arg[4], obj->name))
        continue;
      if (++i > 35)
        break;
      if (!CAN_WEAR(obj, ITEM_TAKE) || obj->item_type == ITEM_QUEST || IS_SET(obj->quest, QUEST_ARTIFACT)
          || (obj->questowner != NULL && strlen(obj->questowner) > 1 && str_cmp(ch->name, obj->questowner)))
      {
        act("You are unable to drain any energy from $p.", ch, obj, NULL, TO_CHAR);
        continue;
      }

      if (obj->ownerid != 0 && str_cmp(argument, "claimed"))
      {
        act("$p is claimed, and you haven't typed '#gsacrifice all claimed#n'.", ch, obj, NULL, TO_CHAR);
        continue;
      }

      act("$p disintegrates into a fine powder.", ch, obj, NULL, TO_CHAR);
      act("$p disintegrates into a fine powder.", ch, obj, NULL, TO_ROOM);
      extract_obj(obj);
    }
    if (i == 0)
      send_to_char("Nothing found.\n\r", ch);
    else
      act("$n destroys most of the items in the room.", ch, NULL, NULL, TO_ROOM);

    sacrificer = NULL;

    return;
  }
  obj = get_obj_list(ch, arg, ch->in_room->contents);
  if (obj == NULL)
  {
    send_to_char("You can't find it.\n\r", ch);
    return;
  }
  if (!CAN_WEAR(obj, ITEM_TAKE) || obj->item_type == ITEM_QUEST ||
      obj->item_type == ITEM_MONEY || obj->item_type == ITEM_TREASURE ||
      IS_SET(obj->quest, QUEST_ARTIFACT) || (obj->questowner != NULL && strlen(obj->questowner) > 1 && str_cmp(ch->name, obj->questowner)))
  {
    act("You are unable to drain any energy from $p.", ch, obj, 0, TO_CHAR);
    return;
  }
  act("$p disintegrates into a fine powder.", ch, obj, NULL, TO_CHAR);
  act("$n drains the energy from $p.", ch, obj, NULL, TO_ROOM);
  act("$p disintegrates into a fine powder.", ch, obj, NULL, TO_ROOM);

  if (!IS_NPC(ch))
    sacrificer = ch;

  extract_obj(obj);

  sacrificer = NULL;
}

void do_quaff(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int level;

  one_argument(argument, arg);

  if (IS_NPC(ch))
    return;

  if (arg[0] == '\0')
  {
    send_to_char("Quaff what?\n\r", ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You do not have that potion.\n\r", ch);
    return;
  }

  if (obj->item_type != ITEM_POTION)
  {
    send_to_char("You can quaff only potions.\n\r", ch);
    return;
  }
  if (IS_NPC(ch))
    return;

  act("$n quaffs $p.", ch, obj, NULL, TO_ROOM);
  act("You quaff $p.", ch, obj, NULL, TO_CHAR);

  level = obj->value[0];
  if (level < 1)
    level = 1;
  if (level > MAX_SPELL)
    level = MAX_SPELL;

  obj_cast_spell(obj->value[1], level, ch, ch, NULL);
  obj_cast_spell(obj->value[2], level, ch, ch, NULL);
  obj_cast_spell(obj->value[3], level, ch, ch, NULL);

  extract_obj(obj);

  if (ch->position == POS_FIGHTING)
   WAIT_STATE(ch, 6);
}

void do_recite(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *scroll;
  OBJ_DATA *obj;
  int level;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (IS_NPC(ch))
    return;

  if ((scroll = get_obj_carry(ch, arg1)) == NULL)
  {
    send_to_char("You do not have that scroll.\n\r", ch);
    return;
  }

  if (scroll->item_type != ITEM_SCROLL)
  {
    send_to_char("You can recite only scrolls.\n\r", ch);
    return;
  }

  obj = NULL;
  if (arg2[0] == '\0')
  {
    victim = ch;
  }
  else
  {
    if ((victim = get_char_room(ch, arg2)) == NULL && (obj = get_obj_here(ch, arg2)) == NULL)
    {
      send_to_char("You can't find it.\n\r", ch);
      return;
    }
  }
  if (IS_NPC(ch))
    return;

  act("$n recites $p.", ch, scroll, NULL, TO_ROOM);
  act("You recite $p.", ch, scroll, NULL, TO_CHAR);

  level = scroll->value[0];
  if (level < 1)
    level = 1;
  if (level > MAX_SPELL)
    level = MAX_SPELL;

  obj_cast_spell(scroll->value[1], level, ch, victim, obj);
  obj_cast_spell(scroll->value[2], level, ch, victim, obj);
  obj_cast_spell(scroll->value[3], level, ch, victim, obj);

  extract_obj(scroll);

  if (ch->position == POS_FIGHTING)
    WAIT_STATE(ch, 6);
}

void do_brandish(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *vch;
  ITERATOR *pIter;
  OBJ_DATA *staff;
  char arg[MAX_INPUT_LENGTH];
  int sn, level;

  if (IS_NPC(ch))
    return;
  one_argument(argument, arg);

  if ((staff = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You do not have that staff.\n\r", ch);
    return;
  }
  if (staff->item_type != ITEM_STAFF)
  {
    send_to_char("You can brandish only with a staff.\n\r", ch);
    return;
  }
  if ((sn = staff->value[3]) < 0 || sn >= MAX_SKILL || skill_table[sn].spell_fun == 0)
  {
    bug("Do_brandish: Bad Staff [obj vnum %d].", staff->pIndexData->vnum);
    send_to_char("Something is wrong with this staff.\n\r", ch);
    return;
  }
  if (staff->value[2] > 0)
  {
    act("$n brandishes $p.", ch, staff, NULL, TO_ROOM);
    act("You brandish $p.", ch, staff, NULL, TO_CHAR);

    pIter = AllocIterator(ch->in_room->people);
    while ((vch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      switch (skill_table[sn].target)
      {
        default:
          bug("Do_brandish: bad spelltype for vnum %d.", staff->pIndexData->vnum);
          return;
        case TAR_CHAR_OFFENSIVE:
        case TAR_CHAR_DEFENSIVE:
          break;
      }
      level = staff->value[0];
      if (level < 1)
        level = 1;
      if (level > MAX_SPELL)
        level = MAX_SPELL;
      obj_cast_spell(staff->value[3], level, ch, vch, NULL);
    }
  }
  if (--staff->value[2] <= 0)
  {
    act("$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM);
    act("Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR);
    extract_obj(staff);
  }

  WAIT_STATE(ch, 18);
}

void do_zap(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *wand;
  OBJ_DATA *obj;
  int level;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg);

  if (IS_NPC(ch))
    return;

  if (arg[0] == '\0' || arg1[0] == '\0')
  {
    send_to_char("Zap whom with what?\n\r", ch);
    return;
  }

  if ((wand = get_obj_carry(ch, arg1)) == NULL)
  {
    send_to_char("You do not have that wand.\n\r", ch);
    return;
  }
  if (wand->item_type != ITEM_WAND)
  {
    send_to_char("You can zap only with a wand.\n\r", ch);
    return;
  }

  obj = NULL;
  if ((victim = get_char_room(ch, arg)) == NULL && (obj = get_obj_here(ch, arg)) == NULL)
  {
    send_to_char("You can't find it.\n\r", ch);
    return;
  }

  WAIT_STATE(ch, 12);

  if (wand->value[2] > 0)
  {
    if (victim != NULL)
    {
      act("$n zaps $N with $p.", ch, wand, victim, TO_ROOM);
      act("You zap $N with $p.", ch, wand, victim, TO_CHAR);
    }
    else
    {
      act("$n zaps $P with $p.", ch, wand, obj, TO_ROOM);
      act("You zap $P with $p.", ch, wand, obj, TO_CHAR);
    }

    level = wand->value[0];
    if (level < 1)
      level = 1;
    if (level > MAX_SPELL)
      level = MAX_SPELL;

    obj_cast_spell(wand->value[3], level, ch, victim, obj);
  }

  if (--wand->value[2] <= 0)
  {
    act("$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM);
    act("Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR);
    extract_obj(wand);
  }
}

void do_steal(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int percent;
  bool skullduggery = FALSE;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0' || arg2[0] == '\0')
  {
    send_to_char("Steal what from whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg2)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (victim == ch)
  {
    send_to_char("That's pointless.\n\r", ch);
    return;
  }

  if (IS_IMMORTAL(victim))
  {
    send_to_char("Steal from an immortal are you crasy!\n\r", ch);
    return;
  }

  if (is_safe(ch, victim))
    return;

  if (!IS_IMMORTAL(ch))
    WAIT_STATE(ch, skill_table[gsn_steal].beats);

  if (!IS_NPC(ch) && IS_CLASS(ch, CLASS_SHADOW) && IS_SET(ch->pcdata->powers[EVOLVE_1], SHADOW_EVOLVE_SKULLDUGGERY))
    skullduggery = TRUE;

  percent = number_percent() + (IS_AWAKE(victim) ? 10 : -50);

  if ((ch->level + number_range(1, 20) < victim->level)
      || (!IS_NPC(ch) && !IS_NPC(victim) && ch->level < 3)
      || (!IS_NPC(ch) && !IS_NPC(victim) && victim->level < 3)
      || (victim->position == POS_FIGHTING && !skullduggery)
      || (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_STEAL) && !skullduggery)
      || (!IS_NPC(victim) && IS_IMMORTAL(victim))
      || (!IS_NPC(ch) && percent > ch->pcdata->learned[gsn_steal]))
  {
    /*
     * Failure.
     */
    send_to_char("Oops.\n\r", ch);
    act("$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT);
    act("$n tried to steal from $N.\n\r", ch, NULL, victim, TO_NOTVICT);
    sprintf(buf, "%s is a bloody thief!", ch->name);
    do_yell(victim, buf);

    if (!IS_NPC(ch) && IS_NPC(victim))
      multi_hit(victim, ch, 1);

    return;
  }

  if ((obj = get_obj_carry(victim, arg1)) == NULL)
  {
    send_to_char("You can't find it.\n\r", ch);
    return;
  }

  if (!can_drop_obj(ch, obj) ||
      IS_SET(obj->extra_flags, ITEM_LOYAL) ||
      obj->item_type == ITEM_CONTAINER ||
      IS_SET(obj->extra_flags, ITEM_INVENTORY))
  {
    send_to_char("You can't pry it away.\n\r", ch);
    return;
  }

  if (ch->carry_number + 1 > can_carry_n(ch))
  {
    send_to_char("You have your hands full.\n\r", ch);
    return;
  }

  if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch))
  {
    if (ch->sex == SEX_FEMALE)
      sound_to_char("armsfull-f.wav", ch);
    else
      sound_to_char("armsfull-m.wav", ch);
    send_to_char("You can't carry that much weight.\n\r", ch);
    return;
  }

  sprintf(buf, "You notice that %s is missing.\n\r", obj->short_descr);
  delay_message(buf, victim, 3 * PULSE_PER_SECOND);

  obj_from_char(obj);
  obj_to_char(obj, ch);
  send_to_char("You got it!\n\r", ch);
}

bool is_ok_to_wear(CHAR_DATA *ch, bool wolf_ok, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int count;

  argument = one_argument(argument, arg);

  if (!str_cmp(arg, "left_hand"))
  {
    if (!IS_NPC(ch) && IS_CLASS(ch, CLASS_FAE) &&
         IS_SET(ch->pcdata->powers[FAE_BITS], FAE_BLASTBEAMS))
      return FALSE;
    else if (IS_ARM_L(ch, LOST_ARM))
      return FALSE;
    else if (IS_ARM_L(ch, BROKEN_ARM))
      return FALSE;
    else if (IS_ARM_L(ch, LOST_HAND))
      return FALSE;
    else if (IS_ARM_L(ch, BROKEN_THUMB))
      return FALSE;
    else if (IS_ARM_L(ch, LOST_THUMB))
      return FALSE;
    count = 0;
    if (IS_ARM_L(ch, LOST_FINGER_I) || IS_ARM_L(ch, BROKEN_FINGER_I))
      count += 1;
    if (IS_ARM_L(ch, LOST_FINGER_M) || IS_ARM_L(ch, BROKEN_FINGER_M))
      count += 1;
    if (IS_ARM_L(ch, LOST_FINGER_R) || IS_ARM_L(ch, BROKEN_FINGER_R))
      count += 1;
    if (IS_ARM_L(ch, LOST_FINGER_L) || IS_ARM_L(ch, BROKEN_FINGER_L))
      count += 1;
    if (count > 2)
      return FALSE;
  }
  else if (!str_cmp(arg, "right_hand"))
  {
    if (!IS_NPC(ch) && IS_CLASS(ch, CLASS_FAE) &&
         IS_SET(ch->pcdata->powers[FAE_BITS], FAE_BLASTBEAMS))
      return FALSE;
    else if (IS_ARM_R(ch, LOST_ARM))
      return FALSE;
    else if (IS_ARM_R(ch, BROKEN_ARM))
      return FALSE;
    else if (IS_ARM_R(ch, LOST_HAND))
      return FALSE;
    else if (IS_ARM_R(ch, BROKEN_THUMB))
      return FALSE;
    else if (IS_ARM_R(ch, LOST_THUMB))
      return FALSE;
    count = 0;
    if (IS_ARM_R(ch, LOST_FINGER_I) || IS_ARM_R(ch, BROKEN_FINGER_I))
      count += 1;
    if (IS_ARM_R(ch, LOST_FINGER_M) || IS_ARM_R(ch, BROKEN_FINGER_M))
      count += 1;
    if (IS_ARM_R(ch, LOST_FINGER_R) || IS_ARM_R(ch, BROKEN_FINGER_R))
      count += 1;
    if (IS_ARM_R(ch, LOST_FINGER_L) || IS_ARM_R(ch, BROKEN_FINGER_L))
      count += 1;
    if (count > 2)
      return FALSE;
  }
  else if (!str_cmp(arg, "third_hand"))
  {
    if (!IS_SET(ch->newbits, THIRD_HAND))
      return FALSE;
  }
  else if (!str_cmp(arg, "fourth_hand"))
  {
    if (!IS_SET(ch->newbits, FOURTH_HAND))
      return FALSE;
  }
  else if (!str_cmp(arg, "left_wrist"))
  {
    if (IS_ARM_L(ch, LOST_ARM))
      return FALSE;
    else if (IS_ARM_L(ch, LOST_HAND))
      return FALSE;
  }
  else if (!str_cmp(arg, "right_wrist"))
  {
    if (IS_ARM_R(ch, LOST_ARM))
      return FALSE;
    else if (IS_ARM_R(ch, LOST_HAND))
      return FALSE;
  }
  else if (!str_cmp(arg, "left_finger"))
  {
    if (IS_ARM_L(ch, LOST_ARM))
      return FALSE;
    else if (IS_ARM_L(ch, LOST_HAND))
      return FALSE;
    else if (IS_ARM_L(ch, LOST_FINGER_R))
      return FALSE;
  }
  else if (!str_cmp(arg, "right_finger"))
  {
    if (IS_ARM_R(ch, LOST_ARM))
      return FALSE;
    else if (IS_ARM_R(ch, LOST_HAND))
      return FALSE;
    else if (IS_ARM_R(ch, LOST_FINGER_R))
      return FALSE;
  }
  else if (!str_cmp(arg, "arms"))
  {
    if (IS_ARM_L(ch, LOST_ARM) && IS_ARM_R(ch, LOST_ARM))
      return FALSE;
  }
  else if (!str_cmp(arg, "hands"))
  {
    if (IS_ARM_L(ch, LOST_ARM) && IS_ARM_R(ch, LOST_ARM))
      return FALSE;
    if (IS_ARM_L(ch, LOST_HAND) || IS_ARM_R(ch, LOST_HAND))
      return FALSE;
  }
  else if (!str_cmp(arg, "legs"))
  {
    if (IS_LEG_L(ch, LOST_LEG) && IS_LEG_R(ch, LOST_LEG))
      return FALSE;
  }
  else if (!str_cmp(arg, "feet"))
  {
    if (IS_LEG_L(ch, LOST_LEG) && IS_LEG_R(ch, LOST_LEG))
      return FALSE;
    if (IS_LEG_L(ch, LOST_FOOT) || IS_LEG_R(ch, LOST_FOOT))
      return FALSE;
  }
  return TRUE;
}

void do_sheath(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  if (arg[0] == '\0')
    send_to_char("Which hand, left or right?\n\r", ch);
  else if (!str_cmp(arg, "all") || !str_cmp(arg, "both"))
  {
    sheath(ch, TRUE);
    sheath(ch, FALSE);
  }
  else if (!str_cmp(arg, "l") || !str_cmp(arg, "left"))
    sheath(ch, FALSE);
  else if (!str_cmp(arg, "r") || !str_cmp(arg, "right"))
    sheath(ch, TRUE);
  else
    send_to_char("Which hand, left or right?\n\r", ch);
  return;
}

void do_draw(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  if (arg[0] == '\0')
    send_to_char("Which hand, left or right?\n\r", ch);
  else if (!str_cmp(arg, "all") || !str_cmp(arg, "both"))
  {
    if (IS_ARM_L(ch, LOST_ARM) || IS_ARM_R(ch, LOST_ARM))
    {
      send_to_char("You don't have both of your arms.\n\r", ch);
      return;
    }
    draw(ch, TRUE);
    draw(ch, FALSE);
  }
  else if (!str_cmp(arg, "l") || !str_cmp(arg, "left"))
  {
    if (IS_ARM_L(ch, LOST_ARM))
    {
      send_to_char("You have lost your left arm.\n\r", ch);
      return;
    }
    draw(ch, FALSE);
  }
  else if (!str_cmp(arg, "r") || !str_cmp(arg, "right"))
  {
    if (IS_ARM_R(ch, LOST_ARM))
    {
      send_to_char("You have lost your right arm.\n\r", ch);
      return;
    }
    draw(ch, TRUE);
  }
  else
    send_to_char("Which hand, left or right?\n\r", ch);
  return;
}

void sheath(CHAR_DATA * ch, bool right)
{
  OBJ_DATA *obj;
  OBJ_DATA *obj2;
  int scabbard;

  if (right)
  {
    scabbard = WEAR_SCABBARD_R;
    if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL)
    {
      send_to_char("You are not holding anything in your right hand.\n\r", ch);
      return;
    }
    else if ((obj2 = get_eq_char(ch, scabbard)) != NULL)
    {
      act("You already have $p in your right scabbard.", ch, obj2, NULL, TO_CHAR);
      return;
    }
    act("You slide $p into your right scabbard.", ch, obj, NULL, TO_CHAR);
    act("$n slides $p into $s right scabbard.", ch, obj, NULL, TO_ROOM);
  }
  else
  {
    scabbard = WEAR_SCABBARD_L;
    if ((obj = get_eq_char(ch, WEAR_HOLD)) == NULL)
    {
      send_to_char("You are not holding anything in your left hand.\n\r", ch);
      return;
    }
    else if ((obj2 = get_eq_char(ch, scabbard)) != NULL)
    {
      act("You already have $p in your left scabbard.", ch, obj2, NULL, TO_CHAR);
      return;
    }
    act("You slide $p into your left scabbard.", ch, obj, NULL, TO_CHAR);
    act("$n slides $p into $s left scabbard.", ch, obj, NULL, TO_ROOM);
  }
  if (obj->item_type != ITEM_WEAPON)
  {
    act("$p is not a weapon.", ch, obj, NULL, TO_CHAR);
    return;
  }
  unequip_char(ch, obj);
  obj->wear_loc = scabbard;
  return;
}

void draw(CHAR_DATA * ch, bool right)
{
  OBJ_DATA *obj;
  OBJ_DATA *obj2;
  int scabbard;
  int worn;

  if (right)
  {
    scabbard = WEAR_SCABBARD_R;
    worn = WEAR_WIELD;
    if ((obj = get_eq_char(ch, scabbard)) == NULL)
    {
      send_to_char("Your right scabbard is empty.\n\r", ch);
      return;
    }
    else if ((obj2 = get_eq_char(ch, WEAR_WIELD)) != NULL)
    {
      act("You already have $p in your right hand.", ch, obj2, NULL, TO_CHAR);
      return;
    }
    act("You draw $p from your right scabbard.", ch, obj, NULL, TO_CHAR);
    act("$n draws $p from $s right scabbard.", ch, obj, NULL, TO_ROOM);
  }
  else
  {
    scabbard = WEAR_SCABBARD_L;
    worn = WEAR_HOLD;
    if ((obj = get_eq_char(ch, scabbard)) == NULL)
    {
      send_to_char("Your left scabbard is empty.\n\r", ch);
      return;
    }
    else if ((obj2 = get_eq_char(ch, WEAR_HOLD)) != NULL)
    {
      act("You already have $p in your left hand.", ch, obj2, NULL, TO_CHAR);
      return;
    }
    act("You draw $p from your left scabbard.", ch, obj, NULL, TO_CHAR);
    act("$n draws $p from $s left scabbard.", ch, obj, NULL, TO_ROOM);
  }
  obj->wear_loc = -1;
  equip_char(ch, obj, worn);
  return;
}
