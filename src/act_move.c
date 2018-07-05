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

char *const dir_name[] = {
  "north", "east", "south", "west", "up", "down"
};

const sh_int rev_dir[] = {
  2, 3, 0, 1, 5, 4
};

const sh_int movement_loss[SECT_MAX] = {
  1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6
};

/*
 * Local functions.
 */
int    find_door     ( CHAR_DATA *ch, char *arg );
int    count_imm     ( CHAR_DATA *ch );
bool   has_key       ( CHAR_DATA *ch, int key );
void   add_tracks    ( CHAR_DATA *ch, ROOM_INDEX_DATA *from_room, int direction );

bool door_closed(CHAR_DATA *ch, EXIT_DATA *pexit)
{
  if (IS_SET(pexit->exit_info, EX_CLOSED) && (!IS_AFFECTED(ch, AFF_PASS_DOOR)
   || IS_SET(pexit->exit_info, EX_NOPASS)) && !IS_AFFECTED(ch, AFF_ETHEREAL))
  {
    act("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
    return TRUE;
  }

  return FALSE;
}

bool door_walled(CHAR_DATA *ch, EXIT_DATA *pexit)
{
  if (IS_SET(pexit->exit_info, EX_PRISMATIC_WALL) &&
      IS_SET(pexit->exit_info, EX_CLOSED))
  {
    send_to_char("The prismatic wall prevents movement through this closed exit.\n\r", ch);
    return TRUE;
  }

  if (IS_SET(pexit->exit_info, EX_SHADOW_WALL) && !IS_CLASS(ch, CLASS_SHADOW))
  {
    if (number_range(1, 4) != 2)
    {
      send_to_char("You get disoriented, and stumble back into the room you came from.\n\r", ch);
      return TRUE;
    }
  }

  if (IS_SET(pexit->exit_info, EX_MUSHROOM_WALL))
  {
    send_to_char("The mushrooms block your path.\n\r", ch);
    return TRUE;
  }

  if (IS_SET(pexit->exit_info, EX_FIRE_WALL) && ch->class == 0)
  {
    act("$n bursts through the wall of fire.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You jump through the flames and are unaffected.\n\r", ch);
  }
  else if (IS_SET(pexit->exit_info, EX_FIRE_WALL) && ch->class != 0 && ch->level > 2)
  {
    act("$n bursts through the wall of fire.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You jump through the flames.\n\r", ch);
    send_to_char("The flames sear your flesh.\n\r", ch);
    modify_hps(ch, -1 * dice(6, 50));
    update_pos(ch);
  }

  if (IS_SET(pexit->exit_info, EX_SWORD_WALL) && ch->class == 0)
  {
    act("$n bursts through the wall of swords.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You jump through the swords and are unaffected.\n\r", ch);
  }
  else if (IS_SET(pexit->exit_info, EX_SWORD_WALL) && ch->class != 0 && ch->level > 2)
  {
    act("$n jumps through the wall of swords.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You jump through the swords.\n\r", ch);
    send_to_char("Aaaaaaaaarghhhhhhh! That hurt!\n\r", ch);
    modify_hps(ch, -1 * dice(6, 70));
    update_pos(ch);
  }

  if (IS_SET(pexit->exit_info, EX_ASH_WALL))
  {
    send_to_char("You scream in agony as the wall of ash rips apart your life force.\n\r", ch);
    act("$n screams in agony as the wall of ash rips $s life force apart.", ch, NULL, NULL, TO_ROOM);
    modify_hps(ch, -1 * ch->hit / 2);
    modify_move(ch, -1 * ch->move / 2);
    if (ch->hit < 1) ch->hit = 1;
  }

  return FALSE;
}

bool room_private(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom)
{
  if (room_is_private(pRoom))
  {
    if (IS_NPC(ch) || ch->level < MAX_LEVEL)
    {
      send_to_char("That room is private right now.\n\r", ch);
      return TRUE;
    }
    else
    {
      send_to_char("That room is private (Access granted).\n\r", ch);
      return FALSE;
    }
  }

  return FALSE;
}

bool body_broken(CHAR_DATA *ch)
{
  if ((IS_LEG_L(ch, BROKEN_LEG) || IS_LEG_L(ch, LOST_LEG)) &&
      (IS_LEG_R(ch, BROKEN_LEG) || IS_LEG_R(ch, LOST_LEG)) &&
      (IS_ARM_L(ch, BROKEN_ARM) || IS_ARM_L(ch, LOST_ARM) ||
       get_eq_char(ch, WEAR_HOLD) != NULL) && (IS_ARM_R(ch, BROKEN_ARM)
     || IS_ARM_R(ch, LOST_ARM) || get_eq_char(ch, WEAR_WIELD) != NULL))
  {
    send_to_char("You need at least one free arm to drag yourself with.\n\r", ch);
    return TRUE;
  }
  else if (IS_BODY(ch, BROKEN_SPINE) &&
           (IS_ARM_L(ch, BROKEN_ARM) || IS_ARM_L(ch, LOST_ARM) ||
            get_eq_char(ch, WEAR_HOLD) != NULL) && (IS_ARM_R(ch, BROKEN_ARM)
          || IS_ARM_R(ch, LOST_ARM) || get_eq_char(ch, WEAR_WIELD) != NULL))
  {
    send_to_char("You cannot move with a broken spine.\n\r", ch);
    return TRUE;
  }

  return FALSE;
}

void enter_room_message(CHAR_DATA *ch, int door)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *mount;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];
  char poly[MAX_STRING_LENGTH];
  char mount2[MAX_INPUT_LENGTH];
  char leave[20];
  int revdoor;

  if (IS_AFFECTED(ch, AFF_SNEAK) || ch->in_room == NULL)
    return;

  if (!IS_NPC(ch) && IS_CLASS(ch, CLASS_FAE) && IS_SET(ch->newbits, NEW_CHAMELEON))
  {
    CHAR_DATA *mobs;

    pIter = AllocIterator(ch->in_room->people);
    while ((mobs = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (IS_NPC(mobs))
      {
        ch = mobs;
        break;
      }
    }

    /* 66% chance of being invis if not mobile */
    if (!IS_NPC(ch) && number_range(1, 3) != 2)
      return;
  }

  if ((mount = ch->mount) != NULL && ch->mounted == IS_RIDING)
  {
    if (IS_NPC(mount))
      sprintf(mount2, " on %s", mount->short_descr);
    else
      sprintf(mount2, " on %s", mount->name);
  }
  else 
  {
    mount = NULL;
    mount2[0] = '\0';
  }

  /* water dome check */
  if (event_isset_room(ch->in_room, EVENT_ROOM_WATERDOME))
    strcat(mount2, ", bursting through the dome of water.");
  else
    strcat(mount2, ".");

  if (door == 0)
  {
    revdoor = 2;
    sprintf(buf, "the south");
  }
  else if (door == 1)
  {
    revdoor = 3;
    sprintf(buf, "the west");
  }
  else if (door == 2)
  {
    revdoor = 0;
    sprintf(buf, "the north");
  }
  else if (door == 3)
  {
    revdoor = 1;
    sprintf(buf, "the east");
  }
  else if (door == 4)
  {
    revdoor = 5;
    sprintf(buf, "below");
  }   
  else
  {
    revdoor = 4;
    sprintf(buf, "above");
  }

  if (IS_AFFECTED(ch, AFF_ETHEREAL))
    sprintf(leave, "floats");
  else if (IS_SET(ch->newbits, NEW_MUDFORM))
    sprintf(leave, "slithers");
  else if (ch->in_room->sector_type == SECT_WATER_SWIM)
    sprintf(leave, "swims");
  else if (IS_BODY(ch, BROKEN_SPINE))
    sprintf(leave, "drags $mself");
  else if (IS_LEG_L(ch, LOST_LEG) && IS_LEG_R(ch, LOST_LEG))
    sprintf(leave, "drags $mself");
  else if ((IS_LEG_L(ch, BROKEN_LEG) || IS_LEG_L(ch, LOST_LEG) ||
            IS_LEG_L(ch, LOST_FOOT)) && (IS_LEG_R(ch, BROKEN_LEG) ||
            IS_LEG_R(ch, LOST_LEG) || IS_LEG_R(ch, LOST_FOOT)))
    sprintf(leave, "crawls");
  else if (ch->hit < (ch->max_hit / 4))
    sprintf(leave, "crawls");
  else if ((IS_LEG_R(ch, LOST_LEG) || IS_LEG_R(ch, LOST_FOOT)) &&
           (!IS_LEG_L(ch, BROKEN_LEG) && !IS_LEG_L(ch, LOST_LEG) &&
            !IS_LEG_L(ch, LOST_FOOT)))
    sprintf(leave, "hops");
  else if ((IS_LEG_L(ch, LOST_LEG) || IS_LEG_L(ch, LOST_FOOT)) &&
           (!IS_LEG_R(ch, BROKEN_LEG) && !IS_LEG_R(ch, LOST_LEG) &&
            !IS_LEG_R(ch, LOST_FOOT)))
    sprintf(leave, "hops");
  else if ((IS_LEG_L(ch, BROKEN_LEG) || IS_LEG_L(ch, LOST_FOOT)) &&
           (!IS_LEG_R(ch, BROKEN_LEG) && !IS_LEG_R(ch, LOST_LEG) &&
            !IS_LEG_R(ch, LOST_FOOT)))
    sprintf(leave, "limps");
  else if ((IS_LEG_R(ch, BROKEN_LEG) || IS_LEG_R(ch, LOST_FOOT)) &&
          (!IS_LEG_L(ch, BROKEN_LEG) && !IS_LEG_L(ch, LOST_LEG) &&
           !IS_LEG_L(ch, LOST_FOOT)))
    sprintf(leave, "limps");
  else if (ch->hit < (ch->max_hit / 3))
    sprintf(leave, "limps");
  else if (ch->hit < (ch->max_hit / 2))
    sprintf(leave, "staggers");
  else if (!IS_NPC(ch))
  {
    if (ch->pcdata->condition[COND_DRUNK] > 10)
      sprintf(leave, "staggers");
    else
      sprintf(leave, "walks");
  }
  else if (IS_SET(ch->act, ACT_FISH))
    sprintf(leave, "swims");
  else
    sprintf(leave, "walks");

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    CHAR_DATA *victim;

    if ((victim = d->character) == NULL)
      continue;

    if (ch->in_room == NULL || victim->in_room == NULL)
      continue;

    if (ch == victim || ch->in_room != victim->in_room)
      continue;

    if (d->connected != CON_PLAYING)
      continue;

    if (!can_see(victim, ch))
      continue;

    if ((mount && IS_AFFECTED(mount, AFF_FLYING)) || IS_AFFECTED(ch, AFF_FLYING))
      sprintf(poly, "$n flies in from %s%s", buf, mount2);
    else if (mount)
      sprintf(poly, "$n rides in from %s%s", buf, mount2);
    else
      sprintf(poly, "$n %s in from %s%s", leave, buf, mount2);

    act(poly, ch, NULL, victim, TO_VICT);
  }
}

void leave_room_message(CHAR_DATA *ch, int door)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *mount;
  ITERATOR *pIter;
  char poly[MAX_STRING_LENGTH];
  char mount2[MAX_INPUT_LENGTH];
  char leave[20];

  if (IS_AFFECTED(ch, AFF_SNEAK))
    return;

  if (!IS_NPC(ch) && IS_CLASS(ch, CLASS_FAE) && IS_SET(ch->newbits, NEW_CHAMELEON))
  {
    CHAR_DATA *mobs;

    pIter = AllocIterator(ch->in_room->people);
    while ((mobs = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (IS_NPC(mobs))
      {
        ch = mobs;
        break;
      }
    }

    /* 66% chance of being invis if not mobile */
    if (!IS_NPC(ch) && number_range(1, 3) != 2)
      return;
  }

  if ((mount = ch->mount) != NULL && ch->mounted == IS_RIDING)
  {
    if (IS_NPC(mount))
      sprintf(mount2, " on %s", mount->short_descr);
    else
      sprintf(mount2, " on %s", mount->name);
  }
  else 
  {
    mount = NULL;
    mount2[0] = '\0';
  }

  if (event_isset_room(ch->in_room, EVENT_ROOM_WATERDOME))
    strcat(mount2, ", bursting through the water dome.");
  else
    strcat(mount2, ".");

  if (IS_AFFECTED(ch, AFF_ETHEREAL))
    sprintf(leave, "floats");
  else if (IS_SET(ch->newbits, NEW_MUDFORM))
    sprintf(leave, "slithers");
  else if (ch->in_room->sector_type == SECT_WATER_SWIM)
    sprintf(leave, "swims");
  else if (IS_BODY(ch, BROKEN_SPINE))
    sprintf(leave, "drags $mself");
  else if (IS_LEG_L(ch, LOST_LEG) && IS_LEG_R(ch, LOST_LEG))
    sprintf(leave, "drags $mself");
  else if ((IS_LEG_L(ch, BROKEN_LEG) || IS_LEG_L(ch, LOST_LEG) ||
            IS_LEG_L(ch, LOST_FOOT)) && (IS_LEG_R(ch, BROKEN_LEG) ||
            IS_LEG_R(ch, LOST_LEG) || IS_LEG_R(ch, LOST_FOOT)))
    sprintf(leave, "crawls");
  else if (ch->hit < (ch->max_hit / 4))
    sprintf(leave, "crawls");
  else if ((IS_LEG_R(ch, LOST_LEG) || IS_LEG_R(ch, LOST_FOOT)) &&
           (!IS_LEG_L(ch, BROKEN_LEG) && !IS_LEG_L(ch, LOST_LEG) &&
            !IS_LEG_L(ch, LOST_FOOT)))
    sprintf(leave, "hops");
  else if ((IS_LEG_L(ch, LOST_LEG) || IS_LEG_L(ch, LOST_FOOT)) &&
           (!IS_LEG_R(ch, BROKEN_LEG) && !IS_LEG_R(ch, LOST_LEG) &&
            !IS_LEG_R(ch, LOST_FOOT)))
    sprintf(leave, "hops");
  else if ((IS_LEG_L(ch, BROKEN_LEG) || IS_LEG_L(ch, LOST_FOOT)) &&
           (!IS_LEG_R(ch, BROKEN_LEG) && !IS_LEG_R(ch, LOST_LEG) &&
            !IS_LEG_R(ch, LOST_FOOT)))
    sprintf(leave, "limps");
  else if ((IS_LEG_R(ch, BROKEN_LEG) || IS_LEG_R(ch, LOST_FOOT)) &&
          (!IS_LEG_L(ch, BROKEN_LEG) && !IS_LEG_L(ch, LOST_LEG) &&
           !IS_LEG_L(ch, LOST_FOOT)))
    sprintf(leave, "limps");
  else if (ch->hit < (ch->max_hit / 3))
    sprintf(leave, "limps");
  else if (ch->hit < (ch->max_hit / 2))
    sprintf(leave, "staggers");
  else if (!IS_NPC(ch))
  {
    if (ch->pcdata->condition[COND_DRUNK] > 10)
      sprintf(leave, "staggers");
    else
      sprintf(leave, "walks");
  }
  else if (IS_SET(ch->act, ACT_FISH))
    sprintf(leave, "swims");
  else
    sprintf(leave, "walks");

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    CHAR_DATA *victim;

    if ((victim = d->character) == NULL)
      continue;

    if (ch->in_room == NULL || victim->in_room == NULL)
      continue;

    if (ch == victim || ch->in_room != victim->in_room)
      continue;

    if (d->connected != CON_PLAYING)
      continue;

    if (!can_see(victim, ch))
      continue;

    if ((mount && IS_AFFECTED(mount, AFF_FLYING)) || IS_AFFECTED(ch, AFF_FLYING))
      sprintf(poly, "$n flies %s%s", dir_name[door], mount2);
    else if (mount)
      sprintf(poly, "$n rides %s%s", dir_name[door], mount2);
    else
      sprintf(poly, "$n %s %s%s", leave, dir_name[door], mount2);

    act(poly, ch, NULL, victim, TO_VICT);
  }
}

void move_char(CHAR_DATA * ch, int door)
{
  CHAR_DATA *fch, *mount;
  ROOM_INDEX_DATA *in_room, *to_room = NULL;
  EVENT_DATA *event;
  ITERATOR *pIter;
  bool redirect = FALSE;

  if (door < 0 || door > 5)
  {
    bug("Do_move: bad door %d.", door);
    return;
  }

  if ((in_room = ch->in_room) == NULL)
  {
    bug("Move_char : ch not in any room.", 0);
    return;
  }

  if (IS_AFFECTED(ch, AFF_WEBBED))
  {
    send_to_char("You are unable to move with all this sticky webbing on.\n\r", ch);
    return;
  }

  pIter = AllocIterator(in_room->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == EVENT_ROOM_MISDIRECT)
    {
      char temp[MAX_INPUT_LENGTH];
      char *ptr;
      int newdir;

      ptr = one_argument(event->argument, temp);
      newdir = atoi(temp);

      if ((to_room = get_room_index(atoi(ptr))) == NULL)
        continue;

      /* do we wish to redirect them ? */
      if (newdir == door)
      {
        redirect = TRUE;
        break;
      }
    }
  }

  if (!redirect)
  {
    EXIT_DATA *pexit;

    if ((pexit = in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
    {
      send_to_char("Alas, you cannot go that way.\n\r", ch);
      return;
    }

    if (door_closed(ch, pexit))
      return;

    if (door_walled(ch, pexit))
      return;
  }

  if (room_private(ch, to_room))
    return;

  if (body_broken(ch))
    return;

  if (IS_AFFECTED(ch, AFF_CHARM) && ch->master && in_room == ch->master->in_room)
  {
    send_to_char("What?  And leave your beloved master?\n\r", ch);
    return;
  }

  if (IS_NPC(ch) && (mount = ch->mount) != NULL && IS_SET(ch->mounted, IS_MOUNT))
  {
    send_to_char("You better wait for instructions from your rider.\n\r", ch);
    return;
  }

  if (!IS_NPC(ch))
  {
    if (in_room->sector_type == SECT_AIR || to_room->sector_type == SECT_AIR)
    {
      if (!IS_AFFECTED(ch, AFF_FLYING))
      {
        send_to_char("You can't fly.\n\r", ch);
        return;
      }
    }

    if (in_room->sector_type == SECT_WATER_NOSWIM || to_room->sector_type == SECT_WATER_NOSWIM)
    {
      OBJ_DATA *obj;
      bool found;

      /*
       * Look for a boat.
       */
      found = FALSE;

      if (IS_AFFECTED(ch, AFF_FLYING))
        found = TRUE;

      if (!found)
      {
        pIter = AllocIterator(ch->carrying);
        while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
        {
          if (obj->item_type == ITEM_BOAT)
          {
            found = TRUE;
            break;
          }
        }
        if (!found)
        {
          send_to_char("You need a boat to go there.\n\r", ch);
          return;
        }
      }
    }
  }

  if (!IS_NPC(ch) && ch->stance[0] != STANCE_NONE)
    do_stance(ch, "none");

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_WHIRLWIND)) != NULL)
  {
    act("The whirlwind holding $n fades away.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You leave the whirldwind behind.\n\r", ch);
    dequeue_event(event, TRUE);
  }

  leave_room_message(ch, door);
  char_from_room(ch);
  char_to_room(ch, to_room, TRUE);
  enter_room_message(ch, door);

  do_look(ch, "auto");

  pIter = AllocIterator(in_room->people);
  while ((fch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if ((mount = fch->mount) != NULL && mount == ch && IS_SET(fch->mounted, IS_MOUNT))
    {
      char_from_room(fch);
      char_to_room(fch, ch->in_room, TRUE);
      act("$N digs $S heels into you.", fch, NULL, ch, TO_CHAR);
    }

    if (fch->master == ch && fch->position == POS_STANDING && fch->in_room != ch->in_room)
    {
      printf_to_char(fch, "You follow %s.\n\r", PERS(ch, fch));
      move_char(fch, door);
    }
  }

  room_text(ch, ">enter<");
}

void do_flex(CHAR_DATA * ch, char *argument)
{
  act("You flex your bulging muscles.", ch, NULL, NULL, TO_CHAR);
  act("$n flexes $s bulging muscles.", ch, NULL, NULL, TO_ROOM);
  
  if (IS_NPC(ch))
    return;
    
  if (IS_EXTRA(ch, TIED_UP))
  {
    act("The ropes restraining you snap.", ch, NULL, NULL, TO_CHAR);
    act("The ropes restraining $n snap.", ch, NULL, NULL, TO_ROOM);
    REMOVE_BIT(ch->extra, TIED_UP);
  }
  if (IS_AFFECTED(ch, AFF_WEBBED))
  {
    act("The webbing around you breaks away.", ch, NULL, NULL, TO_CHAR);
    act("The webbing around $n breaks away.", ch, NULL, NULL, TO_ROOM);
    REMOVE_BIT(ch->affected_by, AFF_WEBBED);
  }

  if (IS_SET(ch->newbits, NEW_TENDRIL1))
  {
    act("$n breaks free from one of the dark tendrils covering $m.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You break free from one of the dark tendrils.\n\r", ch);

    if (IS_SET(ch->newbits, NEW_TENDRIL3))
      REMOVE_BIT(ch->newbits, NEW_TENDRIL3);
    else if (IS_SET(ch->newbits, NEW_TENDRIL2))
      REMOVE_BIT(ch->newbits, NEW_TENDRIL2);
    else
      REMOVE_BIT(ch->newbits, NEW_TENDRIL1);

    if (!IS_SET(ch->newbits, NEW_TENDRIL1))
    {
      strip_event_mobile(ch, EVENT_MOBILE_BLURTENDRILS);
      if (event_isset_mobile(ch, EVENT_MOBILE_ACIDTENDRILS))
      {
        act("$n screams in pain as $e breaks free from the acidtendrils.", ch, NULL, NULL, TO_ROOM);
        send_to_char("You scream in pain as you break free from the acidtendrils.\n\r", ch);
        ch->hit = UMAX(ch->hit - 500, 1);
        strip_event_mobile(ch, EVENT_MOBILE_ACIDTENDRILS);
      }
    }
  }

  if (IS_SET(ch->newbits, NEW_STITCHES))
  {
    REMOVE_BIT(ch->newbits, NEW_STITCHES);
    send_to_char("The 7 stitches holding your mouth shut breaks apart.\n\r", ch);
  }

  WAIT_STATE(ch, 12);
}

void do_north(CHAR_DATA *ch, char *argument)
{
  ROOM_INDEX_DATA *in_room = ch->in_room;

  move_char(ch, DIR_NORTH);

  if (!IS_NPC(ch) && ch->in_room != in_room)
  {
    add_tracks(ch, in_room, DIR_NORTH);
  }
}

void do_east(CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *in_room = ch->in_room;
   
  move_char(ch, DIR_EAST);

  if (!IS_NPC(ch) && ch->in_room != in_room)  
  {
    add_tracks(ch, in_room, DIR_EAST);
  }
}

void do_south(CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *in_room = ch->in_room;
   
  move_char(ch, DIR_SOUTH);

  if (!IS_NPC(ch) && ch->in_room != in_room)  
  {
    add_tracks(ch, in_room, DIR_SOUTH);
  }
}

void do_west(CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *in_room = ch->in_room;
   
  move_char(ch, DIR_WEST);

  if (!IS_NPC(ch) && ch->in_room != in_room)  
  {
    add_tracks(ch, in_room, DIR_WEST);
  }
}

void do_up(CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *in_room = ch->in_room;
   
  move_char(ch, DIR_UP);

  if (!IS_NPC(ch) && ch->in_room != in_room)  
  {
    add_tracks(ch, in_room, DIR_UP);
  }
}

void do_down(CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *in_room = ch->in_room;
   
  move_char(ch, DIR_DOWN);

  if (!IS_NPC(ch) && ch->in_room != in_room)  
  {
    add_tracks(ch, in_room, DIR_DOWN);
  }
}

int find_door(CHAR_DATA * ch, char *arg)
{
  EXIT_DATA *pexit;
  int door;

  if (!str_cmp(arg, "n") || !str_cmp(arg, "north"))
    door = 0;
  else if (!str_cmp(arg, "e") || !str_cmp(arg, "east"))
    door = 1;
  else if (!str_cmp(arg, "s") || !str_cmp(arg, "south"))
    door = 2;
  else if (!str_cmp(arg, "w") || !str_cmp(arg, "west"))
    door = 3;
  else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"))
    door = 4;
  else if (!str_cmp(arg, "d") || !str_cmp(arg, "down"))
    door = 5;
  else
  {
    for (door = 0; door <= 5; door++)
    {
      if ((pexit = ch->in_room->exit[door]) != NULL && IS_SET(pexit->exit_info, EX_ISDOOR) && pexit->keyword != NULL && is_name(arg, pexit->keyword))
        return door;
    }
    act("I see no $T here.", ch, NULL, arg, TO_CHAR);
    return -1;
  }

  if ((pexit = ch->in_room->exit[door]) == NULL)
  {
    act("I see no door $T here.", ch, NULL, arg, TO_CHAR);
    return -1;
  }

  if (!IS_SET(pexit->exit_info, EX_ISDOOR))
  {
    send_to_char("You can't do that.\n\r", ch);
    return -1;
  }

  return door;
}


ROOM_INDEX_DATA *get_rand_room()
{
  ROOM_INDEX_DATA *room;

  for (;;)
  {
    if ((room = get_room_index(number_range(1000, 32000))) != NULL)
    {
      if (!IS_SET(room->room_flags, ROOM_PRIVATE) &&
          !IS_SET(room->room_flags, ROOM_ASTRAL) &&
          !IS_SET(room->room_flags, ROOM_KINGDOM) &&
          !IS_SET(room->room_flags, ROOM_NO_HOME) &&
          !IS_SET(room->room_flags, ROOM_SAFE) &&
          !IS_SET(room->area->areabits, AREA_BIT_OLC))
        break;
    }
  }

  return room;
}

/* This function might return NULL, so do not use this
 * unless you can handle NULL rooms in the calling function.
 */
ROOM_INDEX_DATA *get_rand_room_area(AREA_DATA *area)
{
  ROOM_INDEX_DATA *room;
  int i;

  for (i = 1; i < 500; i++)
  {
    if ((room = get_room_index(number_range(area->lvnum, area->uvnum))) != NULL)
    {
      if (!IS_SET(room->room_flags, ROOM_PRIVATE) &&
          !IS_SET(room->room_flags, ROOM_ASTRAL) &&
          !IS_SET(room->room_flags, ROOM_KINGDOM) &&
          !IS_SET(room->room_flags, ROOM_NO_RECALL) &&
          !IS_SET(room->room_flags, ROOM_SAFE))
        return room;
    }
  }

  return NULL;
}

/* Designed for the portal spell, but can also have other uses...KaVir
 * V0 = Where the portal will take you.
 * V1 = The access code.
 * V2 = if =! 0, cannot be entered.
 * V3 = The room the portal is currently in.
 */
void do_enter(CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *pRoomIndex;
  ROOM_INDEX_DATA *location;
  char arg[MAX_INPUT_LENGTH];
  char poly[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  OBJ_DATA *obj;
  OBJ_DATA *portal;
  CHAR_DATA *mount;
  CHAR_DATA *gch;
  bool found;

  argument = one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Enter what?\n\r", ch);
    return;
  }
  obj = get_obj_list(ch, arg, ch->in_room->contents);
  if (obj == NULL)
  {
    act("I see no $T here.", ch, NULL, arg, TO_CHAR);
    return;
  }
  if (obj->item_type != ITEM_PORTAL)
  {
    act("You cannot enter that.", ch, NULL, arg, TO_CHAR);
    return;
  }

  if (ch->in_room == NULL)
  {
    send_to_char("Your room does not seem to be connected to anyting.\n\r", ch);
    return;
  }

  if (obj->item_type == ITEM_PORTAL)
  {
    if (obj->value[2] != 0)
    {
      act("It seems to be closed.", ch, NULL, arg, TO_CHAR);
      return;
    }

    pRoomIndex = get_room_index(obj->value[0]);
    location = ch->in_room;

    if (pRoomIndex == NULL)
    {
      act("You are unable to enter.", ch, NULL, arg, TO_CHAR);
      return;
    }

    act("You step into $p.", ch, obj, NULL, TO_CHAR);
    if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_POLYMORPH))
      sprintf(poly, "%s steps into $p.", ch->morph);
    else
      sprintf(poly, "$n steps into $p.");
    act(poly, ch, obj, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, pRoomIndex, TRUE);
    if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_POLYMORPH))
      sprintf(poly, "%s steps out of $p.", ch->morph);
    else
      sprintf(poly, "$n steps out of $p.");
    act(poly, ch, obj, NULL, TO_ROOM);

    found = FALSE;
    pIter = AllocIterator(ch->in_room->contents);
    while ((portal = (OBJ_DATA *) NextInList(pIter)) != NULL)
    {
      if ((obj->value[0] == portal->value[3]) && (obj->value[3] == portal->value[0]))
      {
        found = TRUE;
        break;
      }
    }

    do_look(ch, "auto");
    if ((mount = ch->mount) != NULL)
    {
      char_from_room(mount);
      char_to_room(mount, ch->in_room, TRUE);
    }

    pIter = AllocIterator(location->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch->master == ch && gch->position == POS_STANDING && gch->in_room != ch->in_room)
      {
        act("You follow $N.", gch, NULL, ch, TO_CHAR);
        do_enter(gch, arg);
      }
    }
  }
}

void do_smother(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *inroom;

  one_argument(argument, arg);
  if (IS_NPC(ch))
    return;
  inroom = ch->in_room;
  if (arg[0] == '\0' && !IS_SET(inroom->room_flags, ROOM_FLAMING))
  {
    send_to_char("Smother whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (!IS_AFFECTED(victim, AFF_FLAMING))
  {
    send_to_char("But they are not on fire!\n\r", ch);
    return;
  }

  if (number_percent() > (ch->level * 10))
  {
    act("You try to smother the flames around $N but fail!", ch, NULL, victim, TO_CHAR);
    act("$n tries to smother the flames around you but fails!", ch, NULL, victim, TO_VICT);
    act("$n tries to smother the flames around $N but fails!", ch, NULL, victim, TO_NOTVICT);
    if (number_percent() > 98 && !IS_AFFECTED(ch, AFF_FLAMING))
    {
      act("A spark of flame from $N's body sets you on fire!", ch, NULL, victim, TO_CHAR);
      act("A spark of flame from your body sets $n on fire!", ch, NULL, victim, TO_VICT);
      act("A spark of flame from $N's body sets $n on fire!", ch, NULL, victim, TO_NOTVICT);
      SET_BIT(ch->affected_by, AFF_FLAMING);
    }
    return;
  }

  act("You manage to smother the flames around $M!", ch, NULL, victim, TO_CHAR);
  act("$n manages to smother the flames around you!", ch, NULL, victim, TO_VICT);
  act("$n manages to smother the flames around $N!", ch, NULL, victim, TO_NOTVICT);
  REMOVE_BIT(victim->affected_by, AFF_FLAMING);
}


void do_open(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  ITERATOR *pIter;
  int door;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Open what?\n\r", ch);
    return;
  }

  if ((obj = get_obj_here(ch, arg)) != NULL)
  {
    /* 'open object' */
    if (obj->item_type != ITEM_CONTAINER)
    {
      send_to_char("That's not a container.\n\r", ch);
      return;
    }
    if (!IS_SET(obj->value[1], CONT_CLOSED))
    {
      send_to_char("It's already open.\n\r", ch);
      return;
    }
    if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
    {
      send_to_char("You can't do that.\n\r", ch);
      return;
    }
    if (IS_SET(obj->value[1], CONT_LOCKED))
    {
      send_to_char("It's locked.\n\r", ch);
      return;
    }

    REMOVE_BIT(obj->value[1], CONT_CLOSED);
    send_to_char("Ok.\n\r", ch);
    act("$n opens $p.", ch, obj, NULL, TO_ROOM);
    return;
  }

  if ((door = find_door(ch, arg)) >= 0)
  {
    /* 'open door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit = ch->in_room->exit[door];
    if (!IS_SET(pexit->exit_info, EX_CLOSED))
    {
      send_to_char("It's already open.\n\r", ch);
      return;
    }
    if (IS_SET(pexit->exit_info, EX_LOCKED))
    {
      send_to_char("It's locked.\n\r", ch);
      return;
    }

    REMOVE_BIT(pexit->exit_info, EX_CLOSED);
    act("$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM);
    send_to_char("Ok.\n\r", ch);

    /* open the other side */
    if ((to_room = pexit->to_room) != NULL && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL && pexit_rev->to_room == ch->in_room)
    {
      CHAR_DATA *rch;

      REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);

      pIter = AllocIterator(to_room->people);
      while ((rch = (CHAR_DATA *) NextInList(pIter)) != NULL)
        act("The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR);
    }
  }
}

void do_close(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  ITERATOR *pIter;
  int door;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Close what?\n\r", ch);
    return;
  }

  if ((obj = get_obj_here(ch, arg)) != NULL)
  {
    /* 'close object' */
    if (obj->item_type != ITEM_CONTAINER)
    {
      send_to_char("That's not a container.\n\r", ch);
      return;
    }
    if (IS_SET(obj->value[1], CONT_CLOSED))
    {
      send_to_char("It's already closed.\n\r", ch);
      return;
    }
    if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
    {
      send_to_char("You can't do that.\n\r", ch);
      return;
    }

    SET_BIT(obj->value[1], CONT_CLOSED);
    send_to_char("Ok.\n\r", ch);
    act("$n closes $p.", ch, obj, NULL, TO_ROOM);
    return;
  }

  if ((door = find_door(ch, arg)) >= 0)
  {
    /* 'close door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit = ch->in_room->exit[door];
    if (IS_SET(pexit->exit_info, EX_CLOSED))
    {
      send_to_char("It's already closed.\n\r", ch);
      return;
    }

    SET_BIT(pexit->exit_info, EX_CLOSED);
    act("$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM);
    send_to_char("Ok.\n\r", ch);

    /* close the other side */
    if ((to_room = pexit->to_room) != NULL && (pexit_rev = to_room->exit[rev_dir[door]]) != 0 && pexit_rev->to_room == ch->in_room)
    {
      CHAR_DATA *rch;

      SET_BIT(pexit_rev->exit_info, EX_CLOSED);
      pIter = AllocIterator(to_room->people);
      while ((rch = (CHAR_DATA *) NextInList(pIter)) != NULL)
        act("The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR);
    }
  }
}

bool has_key(CHAR_DATA * ch, int key)
{
  OBJ_DATA *obj;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->carrying);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->pIndexData->vnum == key)
      return TRUE;
  }

  return FALSE;
}

void do_lock(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int door;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Lock what?\n\r", ch);
    return;
  }

  if ((obj = get_obj_here(ch, arg)) != NULL)
  {
    /* 'lock object' */
    if (obj->item_type != ITEM_CONTAINER)
    {
      send_to_char("That's not a container.\n\r", ch);
      return;
    }
    if (!IS_SET(obj->value[1], CONT_CLOSED))
    {
      send_to_char("It's not closed.\n\r", ch);
      return;
    }
    if (obj->value[2] < 0)
    {
      send_to_char("It can't be locked.\n\r", ch);
      return;
    }
    if (!has_key(ch, obj->value[2]))
    {
      send_to_char("You lack the key.\n\r", ch);
      return;
    }
    if (IS_SET(obj->value[1], CONT_LOCKED))
    {
      send_to_char("It's already locked.\n\r", ch);
      return;
    }

    SET_BIT(obj->value[1], CONT_LOCKED);
    send_to_char("*Click*\n\r", ch);
    act("$n locks $p.", ch, obj, NULL, TO_ROOM);
    return;
  }

  if ((door = find_door(ch, arg)) >= 0)
  {
    /* 'lock door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit = ch->in_room->exit[door];
    if (!IS_SET(pexit->exit_info, EX_CLOSED))
    {
      send_to_char("It's not closed.\n\r", ch);
      return;
    }
    if (pexit->key < 0)
    {
      send_to_char("It can't be locked.\n\r", ch);
      return;
    }
    if (!has_key(ch, pexit->key))
    {
      send_to_char("You lack the key.\n\r", ch);
      return;
    }
    if (IS_SET(pexit->exit_info, EX_LOCKED))
    {
      send_to_char("It's already locked.\n\r", ch);
      return;
    }

    SET_BIT(pexit->exit_info, EX_LOCKED);
    send_to_char("*Click*\n\r", ch);
    act("$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM);

    /* lock the other side */
    if ((to_room = pexit->to_room) != NULL && (pexit_rev = to_room->exit[rev_dir[door]]) != 0 && pexit_rev->to_room == ch->in_room)
    {
      SET_BIT(pexit_rev->exit_info, EX_LOCKED);
    }
  }
}

void do_unlock(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int door;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Unlock what?\n\r", ch);
    return;
  }

  if ((obj = get_obj_here(ch, arg)) != NULL)
  {
    /* 'unlock object' */
    if (obj->item_type != ITEM_CONTAINER)
    {
      send_to_char("That's not a container.\n\r", ch);
      return;
    }
    if (!IS_SET(obj->value[1], CONT_CLOSED))
    {
      send_to_char("It's not closed.\n\r", ch);
      return;
    }
    if (obj->value[2] < 0)
    {
      send_to_char("It can't be unlocked.\n\r", ch);
      return;
    }
    if (!has_key(ch, obj->value[2]))
    {
      send_to_char("You lack the key.\n\r", ch);
      return;
    }
    if (!IS_SET(obj->value[1], CONT_LOCKED))
    {
      send_to_char("It's already unlocked.\n\r", ch);
      return;
    }

    REMOVE_BIT(obj->value[1], CONT_LOCKED);
    send_to_char("*Click*\n\r", ch);
    act("$n unlocks $p.", ch, obj, NULL, TO_ROOM);
    return;
  }

  if ((door = find_door(ch, arg)) >= 0)
  {
    /* 'unlock door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit = ch->in_room->exit[door];
    if (!IS_SET(pexit->exit_info, EX_CLOSED))
    {
      send_to_char("It's not closed.\n\r", ch);
      return;
    }
    if (pexit->key < 0)
    {
      send_to_char("It can't be unlocked.\n\r", ch);
      return;
    }
    if (!has_key(ch, pexit->key))
    {
      send_to_char("You lack the key.\n\r", ch);
      return;
    }
    if (!IS_SET(pexit->exit_info, EX_LOCKED))
    {
      send_to_char("It's already unlocked.\n\r", ch);
      return;
    }

    REMOVE_BIT(pexit->exit_info, EX_LOCKED);
    send_to_char("*Click*\n\r", ch);
    act("$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM);

    /* unlock the other side */
    if ((to_room = pexit->to_room) != NULL && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL && pexit_rev->to_room == ch->in_room)
    {
      REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
    }
  }
}

void do_pick(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *gch;
  ITERATOR *pIter;
  OBJ_DATA *obj;
  int door;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Pick what?\n\r", ch);
    return;
  }

  WAIT_STATE(ch, skill_table[gsn_pick_lock].beats);

  /* look for guards */
  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level)
    {
      act("$N is standing too close to the lock.", ch, NULL, gch, TO_CHAR);
      return;
    }
  }

  if (!IS_NPC(ch) && number_percent() > ch->pcdata->learned[gsn_pick_lock])
  {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if ((obj = get_obj_here(ch, arg)) != NULL)
  {
    /* 'pick object' */
    if (obj->item_type != ITEM_CONTAINER)
    {
      send_to_char("That's not a container.\n\r", ch);
      return;
    }
    if (!IS_SET(obj->value[1], CONT_CLOSED))
    {
      send_to_char("It's not closed.\n\r", ch);
      return;
    }
    if (obj->value[2] < 0)
    {
      send_to_char("It can't be unlocked.\n\r", ch);
      return;
    }
    if (!IS_SET(obj->value[1], CONT_LOCKED))
    {
      send_to_char("It's already unlocked.\n\r", ch);
      return;
    }
    if (IS_SET(obj->value[1], CONT_PICKPROOF))
    {
      send_to_char("You failed.\n\r", ch);
      return;
    }

    REMOVE_BIT(obj->value[1], CONT_LOCKED);
    send_to_char("*Click*\n\r", ch);
    act("$n picks $p.", ch, obj, NULL, TO_ROOM);
    return;
  }

  if ((door = find_door(ch, arg)) >= 0)
  {
    /* 'pick door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit = ch->in_room->exit[door];
    if (!IS_SET(pexit->exit_info, EX_CLOSED))
    {
      send_to_char("It's not closed.\n\r", ch);
      return;
    }
    if (pexit->key < 0)
    {
      send_to_char("It can't be picked.\n\r", ch);
      return;
    }
    if (!IS_SET(pexit->exit_info, EX_LOCKED))
    {
      send_to_char("It's already unlocked.\n\r", ch);
      return;
    }
    if (IS_SET(pexit->exit_info, EX_PICKPROOF))
    {
      send_to_char("You failed.\n\r", ch);
      return;
    }

    REMOVE_BIT(pexit->exit_info, EX_LOCKED);
    send_to_char("*Click*\n\r", ch);
    act("$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM);

    /* pick the other side */
    if ((to_room = pexit->to_room) != NULL && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL && pexit_rev->to_room == ch->in_room)
    {
      REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
    }
  }
}

void do_stand(CHAR_DATA * ch, char *argument)
{
  switch (ch->position)
  {
    case POS_SLEEPING:
      send_to_char("You wake and stand up.\n\r", ch);
      act("$n wakes and stands up.", ch, NULL, NULL, TO_ROOM);
      ch->position = POS_STANDING;
      break;

    case POS_RESTING:
    case POS_SITTING:
      send_to_char("You stand up.\n\r", ch);
      act("$n stands up.", ch, NULL, NULL, TO_ROOM);
      ch->position = POS_STANDING;
      break;

    case POS_MEDITATING:
      send_to_char("You uncross your legs and stand up.\n\r", ch);
      act("$n uncrosses $s legs and stands up.", ch, NULL, NULL, TO_ROOM);
      ch->position = POS_STANDING;
      break;

    case POS_STANDING:
      send_to_char("You are already standing.\n\r", ch);
      return;
      break;

    case POS_FIGHTING:
      send_to_char("You are already fighting!\n\r", ch);
      return;
      break;
  }
}

void do_rest(CHAR_DATA *ch, char *argument)
{
  switch (ch->position)
  {
    case POS_SLEEPING:
      send_to_char("You are already sleeping.\n\r", ch);
      break;

    case POS_RESTING:
      send_to_char("You are already resting.\n\r", ch);
      break;

    case POS_MEDITATING:
    case POS_SITTING:
    case POS_STANDING:
      send_to_char("You rest.\n\r", ch);
      act("$n rests.", ch, NULL, NULL, TO_ROOM);
      ch->position = POS_RESTING;
      break;

    case POS_FIGHTING:
      send_to_char("You are fighting!\n\r", ch);
      break;
  }
}

void do_sit(CHAR_DATA * ch, char *argument)
{
  switch (ch->position)
  {
    case POS_SLEEPING:
      send_to_char("You are already sleeping.\n\r", ch);
      break;

    case POS_RESTING:
      send_to_char("You are already resting.\n\r", ch);
      break;

    case POS_MEDITATING:
      send_to_char("You are already meditating.\n\r", ch);
      break;

    case POS_SITTING:
      send_to_char("You are already sitting.\n\r", ch);
      break;

    case POS_STANDING:
      send_to_char("You sit down.\n\r", ch);
      act("$n sits down.", ch, NULL, NULL, TO_ROOM);
      ch->position = POS_SITTING;
      break;

    case POS_FIGHTING:
      send_to_char("You are fighting!\n\r", ch);
      break;
  }

  return;
}

void do_sleep(CHAR_DATA * ch, char *argument)
{
  switch (ch->position)
  {
    case POS_SLEEPING:
      send_to_char("You are already sleeping.\n\r", ch);
      break;

    case POS_SITTING:
    case POS_MEDITATING:
    case POS_RESTING:
    case POS_STANDING:
      send_to_char("You sleep.\n\r", ch);
      act("$n sleeps.", ch, NULL, NULL, TO_ROOM);
      ch->position = POS_SLEEPING;
      break;

    case POS_FIGHTING:
      send_to_char("You are fighting!\n\r", ch);
      break;
  }
}

void do_wake(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    do_stand(ch, argument);
    return;
  }

  if (!IS_AWAKE(ch))
  {
    send_to_char("You are asleep yourself!\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (IS_AWAKE(victim))
  {
    act("$N is already awake.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if (victim->position < POS_SLEEPING)
  {
    act("$E doesn't respond!", ch, NULL, victim, TO_CHAR);
    return;
  }

  act("You wake $M.", ch, NULL, victim, TO_CHAR);
  act("$n wakes you.", ch, NULL, victim, TO_VICT);
  victim->position = POS_STANDING;
}

void do_sneak(CHAR_DATA * ch, char *argument)
{
  AFFECT_DATA af;

  send_to_char("You attempt to move silently.\n\r", ch);
  affect_strip(ch, gsn_sneak);

  if (IS_NPC(ch) || number_percent() < ch->pcdata->learned[gsn_sneak])
  {
    af.type = gsn_sneak;
    af.duration = ch->level;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_SNEAK;
    affect_to_char(ch, &af);
  }
}

/*
 * Contributed by Alander.
 */
void do_visible(CHAR_DATA * ch, char *argument)
{
  affect_strip(ch, gsn_invis);
  affect_strip(ch, gsn_mass_invis);
  affect_strip(ch, gsn_sneak);
  REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
  REMOVE_BIT(ch->affected_by, AFF_SNEAK);
  send_to_char("Ok.\n\r", ch);
}

void do_recall(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *mount;
  ROOM_INDEX_DATA *location;
  int pick, door;

  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);
  if (arg[0] == '\0' || (pick = atoi(arg)) < 1 || pick > 3)
  {
    pick = 1;
    send_to_char("Defaulting to recall 1 (use recall [1|2|3]).\n\r", ch);
  }
  pick--;

  act("$n's body flickers with green energy.", ch, NULL, NULL, TO_ROOM);
  act("Your body flickers with green energy.", ch, NULL, NULL, TO_CHAR);

  if (event_isset_room(ch->in_room, EVENT_ROOM_WATERDOME))
  {
    send_to_char("You are unable to recall from this room.\n\r", ch);
    return;
  }

  if ((location = get_room_index(ch->home[pick])) == NULL)
  {
    send_to_char("You are completely lost.\n\r", ch);
    return;
  }

  if (ch->in_room == location)
    return;

  if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL))
  {
    send_to_char("It is not possible to recall from this room.\n\r", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_CURSE))
  {
    send_to_char("You cannot recall with this curse on you.\n\r", ch);
    return;
  }

  /* can only recall one room when having a fight_timer */
  if (ch->fight_timer > 0)
  {
    EXIT_DATA *pExit;
    int count = 0;

    /* first count the exits */
    for (door = 0; door < 6; door++)
    {
      if ((pExit = ch->in_room->exit[door]) == NULL || pExit->to_room == NULL || IS_SET(pExit->exit_info, EX_CLOSED))
        continue;

      count++;
    }

    if (count <= 0)
    {
      send_to_char("It is not possible to recall from this room with a fighttimer.\n\r", ch);
      return;
    }

    pick = number_range(0, count - 1);

    /* first count the exits */
    for (count = 0, door = 0; door < 6; door++)
    {
      if ((pExit = ch->in_room->exit[door]) == NULL || pExit->to_room == NULL || IS_SET(pExit->exit_info, EX_CLOSED))
        continue;

      if (count++ == pick)
        break;
    }

    if (pExit == NULL || (location = pExit->to_room) == NULL)
    {
      send_to_char("It is not possible to recall from this room with a fighttimer.\n\r", ch);
      return;
    }

    WAIT_STATE(ch, 4);
  }

  if ((victim = ch->fighting) != NULL)
  {
    if (number_bits(1) == 0)
    {
      WAIT_STATE(ch, 4);
      sprintf(buf, "You failed!\n\r");
      send_to_char(buf, ch);
      return;
    }

    sprintf(buf, "You recall from combat!\n\r");
    send_to_char(buf, ch);
    stop_fighting(ch, TRUE);
  }

  act("$n disappears.", ch, NULL, NULL, TO_ROOM);
  char_from_room(ch);

  char_to_room(ch, location, TRUE);
  act("$n appears in the room.", ch, NULL, NULL, TO_ROOM);
  do_look(ch, "auto");

  if ((mount = ch->mount) == NULL)
    return;

  char_from_room(mount);
  char_to_room(mount, ch->in_room, TRUE);
}

void do_home(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int pick, i;

  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);
  if (arg[0] == '\0' || (pick = atoi(arg)) < 1 || pick > 3)
  {
    send_to_char("Syntax: home [1|2|3]\n\r", ch);
    return;
  }
  pick--;

  for (i = 0; i < 3; i++)
  {
    if (ch->in_room->vnum == ch->home[i])
    {
      send_to_char("But this is already your home!\n\r", ch);
      return;
    }
  }

  if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) ||
      IS_SET(ch->in_room->room_flags, ROOM_NO_HOME) ||
      IS_SET(ch->in_room->area->areabits, AREA_BIT_NOHOME) ||
      IS_SET(ch->in_room->area->areabits, AREA_BIT_OLC) ||
      IS_SET(ch->in_room->room_flags, ROOM_SAFE) ||
      IS_SET(ch->in_room->room_flags, ROOM_KINGDOM))
  {
    send_to_char("You are unable to make this room your home.\n\r", ch);
    return;
  }

  ch->home[pick] = ch->in_room->vnum;
  send_to_char("This room is now your home.\n\r", ch);
}

void do_escape(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  ROOM_INDEX_DATA *location;
  int i;

  if (IS_NPC(ch) || !IS_HERO(ch))
    return;

  if (ch->position >= POS_SLEEPING)
  {
    send_to_char("You can only do this if you are dying.\n\r", ch);
    return;
  }

  if (in_arena(ch))
  {
    send_to_char("You cannot escape from the arena.\n\r", ch);
    return;
  }
  if (in_fortress(ch))
  {
    send_to_char("You cannot escape from the fortress.\n\r", ch);
    return;
  }

  if (ch->move < 500)
  {
    printf_to_char(ch, "You need %d more move before you can escape.\n\r", 500 - ch->move);
    return;
  }

  if (ch->fight_timer > 0)
  {
    EVENT_DATA *event;
    EXIT_DATA *pExit;
    int count = 0, pick, door;

    if ((event = event_isset_mobile(ch, EVENT_PLAYER_ESCAPE)) != NULL)
    {
      printf_to_char(ch, "You are unable to escape for another %s.\n\r", event_time_left(event));
      return;
    }

    /* make sure escaping can only happen once every 10 seconds */
    event = alloc_event();
    event->fun = &event_dummy;
    event->type = EVENT_PLAYER_ESCAPE;
    add_event_char(event, ch, 5 * PULSE_PER_SECOND);

    /* first count the exits */
    for (door = 0; door < 6; door++)
    {
      if ((pExit = ch->in_room->exit[door]) == NULL ||
           pExit->to_room == NULL ||
           IS_SET(pExit->exit_info, EX_CLOSED))
        continue;

      count++;
    }

    if (count <= 0)
    {
      send_to_char("It is not possible to escape from this room with a fighttimer.\n\r", ch);
      return;
    }

    pick = number_range(0, count - 1);

    /* then grab the right exit */
    for (count = 0, door = 0; door < 6; door++)
    {
      if ((pExit = ch->in_room->exit[door]) == NULL ||
           pExit->to_room == NULL ||
           IS_SET(pExit->exit_info, EX_CLOSED))
        continue;

      if (count++ == pick)
        break;
    }

    if (pExit == NULL || (location = pExit->to_room) == NULL)
    {
      send_to_char("It is not possible to escape from this room with a fighttimer.\n\r", ch);
      return;
    }
  }
  else
  {
    for (i = 0; i < 3; i++)
    {
      if (ch->in_room->vnum == ch->home[i])
        return;
    }

    if ((location = get_room_index(ch->home[number_range(0,2)])) == NULL)
    {
      send_to_char("You are completely lost.\n\r", ch);
      return;
    }
  }

  if (ch->in_room == location)
    return;

  modify_move(ch, -1 * ch->move);
  modify_mana(ch, -1 * ch->mana);
  act("$n fades out of existance.", ch, NULL, NULL, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, location, TRUE);
  act("$n fades into existance.", ch, NULL, NULL, TO_ROOM);
  do_look(ch, "auto");
  sprintf(buf, "%s has escaped #Gdefenceless#n, easy kill for the quick.", ch->name);
  do_info(ch, buf);
}

void do_relearn(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_STRING_LENGTH];

  argument = one_argument(argument, arg1);

  if (IS_NPC(ch))
    return;

  if (!str_cmp(arg1, "slash") || !str_cmp(arg1, "slice"))
  {
    if (IS_IMMUNE(ch, IMM_SLASH))
    {
      REMOVE_BIT(ch->immune, IMM_SLASH);
      send_to_char("You forget Slash & Slice resistances.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "stab") || !str_cmp(arg1, "pierce"))
  {
    if (IS_IMMUNE(ch, IMM_STAB))
    {
      REMOVE_BIT(ch->immune, IMM_STAB);
      send_to_char("You forget Stab & Pierce resistances.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "smash") || !str_cmp(arg1, "pound") || !str_cmp(arg1, "blast") || !str_cmp(arg1, "crush"))
  {
    if (IS_IMMUNE(ch, IMM_SMASH))
    {
      REMOVE_BIT(ch->immune, IMM_SMASH);
      send_to_char("You forget Pound, Blast & Crush resistances.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "beast") || !str_cmp(arg1, "claw") || !str_cmp(arg1, "bite"))
  {
    if (IS_IMMUNE(ch, IMM_ANIMAL))
    {
      REMOVE_BIT(ch->immune, IMM_ANIMAL);
      send_to_char("You forget Claw & Bite resistances.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "grab") || !str_cmp(arg1, "grep") || !str_cmp(arg1, "whip") || !str_cmp(arg1, "suck"))
  {
    if (IS_IMMUNE(ch, IMM_MISC))
    {
      REMOVE_BIT(ch->immune, IMM_MISC);
      send_to_char("You forget Grep, Whip & Suck resistances.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "charm"))
  {
    if (IS_IMMUNE(ch, IMM_CHARM))
    {
      REMOVE_BIT(ch->immune, IMM_CHARM);
      send_to_char("You forget Charm immunity.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "heat"))
  {
    if (IS_IMMUNE(ch, IMM_HEAT))
    {
      REMOVE_BIT(ch->immune, IMM_HEAT);
      send_to_char("You forget Heat immunity.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "cold"))
  {
    if (IS_IMMUNE(ch, IMM_COLD))
    {
      REMOVE_BIT(ch->immune, IMM_COLD);
      send_to_char("You forget Cold immunity.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "lightning"))
  {
    if (IS_IMMUNE(ch, IMM_LIGHTNING))
    {
      REMOVE_BIT(ch->immune, IMM_LIGHTNING);
      send_to_char("You forget Lightning immunity.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "acid"))
  {
    if (IS_IMMUNE(ch, IMM_ACID))
    {
      REMOVE_BIT(ch->immune, IMM_ACID);
      send_to_char("You forget Acid immunity.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "cold"))
  {
    if (IS_IMMUNE(ch, IMM_COLD))
    {
      REMOVE_BIT(ch->immune, IMM_COLD);
      send_to_char("You forget Cold immunity.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "drain"))
  {
    if (IS_IMMUNE(ch, IMM_DRAIN))
    {
      REMOVE_BIT(ch->immune, IMM_DRAIN);
      send_to_char("You forget Drain immunity.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "hurl"))
  {
    if (IS_IMMUNE(ch, IMM_HURL))
    {
      REMOVE_BIT(ch->immune, IMM_HURL);
      send_to_char("You forget Hurl immunity.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "backstab"))
  {
    if (IS_IMMUNE(ch, IMM_BACKSTAB))
    {
      REMOVE_BIT(ch->immune, IMM_BACKSTAB);
      send_to_char("You forget Backstab immunity.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "kick"))
  {
    if (IS_IMMUNE(ch, IMM_KICK))
    {
      REMOVE_BIT(ch->immune, IMM_KICK);
      send_to_char("You forget Kick immunity.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "disarm"))
  {
    if (IS_IMMUNE(ch, IMM_DISARM))
    {
      REMOVE_BIT(ch->immune, IMM_DISARM);
      send_to_char("You forget Disarm immunity.\n\r", ch);
      return;
    }
  }
  if (!strcmp(arg1, "steal"))
  {
    if (IS_IMMUNE(ch, IMM_STEAL))
    {
      REMOVE_BIT(ch->immune, IMM_STEAL);
      send_to_char("You forget Steal immunity.\n\r", ch);
      return;
    }
  }
  else
  {
    send_to_char("Command: relearn an already learned immunity/resistance.\n\r", ch);
    return;
  }
}

int count_imm(CHAR_DATA *ch)
{
  int immune_counter = 0;

  if (IS_IMMUNE(ch, IMM_SLASH))
    immune_counter += 1;
  if (IS_IMMUNE(ch, IMM_STAB))
    immune_counter += 1;
  if (IS_IMMUNE(ch, IMM_SMASH))
    immune_counter += 1;
  if (IS_IMMUNE(ch, IMM_ANIMAL))
    immune_counter += 1;
  if (IS_IMMUNE(ch, IMM_MISC))
    immune_counter += 1;
  if (IS_IMMUNE(ch, IMM_CHARM))
    immune_counter += 1;
  if (IS_IMMUNE(ch, IMM_HEAT))
    immune_counter += 1;
  if (IS_IMMUNE(ch, IMM_COLD))
    immune_counter += 1;
  if (IS_IMMUNE(ch, IMM_LIGHTNING))
    immune_counter += 1;
  if (IS_IMMUNE(ch, IMM_ACID))
    immune_counter += 1;
  if (IS_IMMUNE(ch, IMM_DRAIN))
    immune_counter += 1;
  if (IS_IMMUNE(ch, IMM_HURL))
    immune_counter += 1;
  if (IS_IMMUNE(ch, IMM_BACKSTAB))
    immune_counter += 1;
  if (IS_IMMUNE(ch, IMM_DISARM))
    immune_counter += 1;
  if (IS_IMMUNE(ch, IMM_STEAL))
    immune_counter += 1;
  if (IS_IMMUNE(ch, IMM_KICK))
    immune_counter += 1;

  return immune_counter;
}

void do_train(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int amount = 0, i = 0, j = 0, cost;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (IS_NPC(ch)) return;

  if (arg1[0] == '\0')
  {
    BUFFER *buf2 = buffer_new(MAX_STRING_LENGTH);
    int count = count_imm(ch);

    sprintf(arg1, " %s\n\n\r", get_dystopia_banner("Train Screen", 56));
    send_to_char(arg1, ch);

    bprintf(buf2, "  You currently have #C%d#n exp and #C%d#n primal.<BR><BR>",
      ch->exp, ch->practice);

    if (ch->level == 2 && ch->hit >= 2000)
    {
      int gm = 0, gms = 0; 
      bool gmblast = FALSE;

      for (i = 0; i < 13; i++)
      {
        if (ch->wpn[i] >= 200)
          gm++;
        if (i == 6 && ch->wpn[i] >= 200)
          gmblast = TRUE;
      }
      for (i = 6; i < 11; i++)
      {
        if (ch->stance[i] >= 200)
          gms++;
      }

      switch(ch->class)
      {
        case CLASS_FAE:
          if (ch->max_move >= 3500 && ch->max_mana >= 3500 && ch->max_hit >= 5000 && gm >= 3 && gmblast && gms >= 2)
            bprintf(buf2, "  Train Avatar                   :  #CFree#n<BR><BR>");
          break;
        case CLASS_GIANT:
          if (ch->max_move >= 2500)
            bprintf(buf2, "  Train Avatar                   :  #CFree#n<BR><BR>");
          break;
        case CLASS_SHADOW:
          if (ch->max_hit >= 2500)
            bprintf(buf2, "  Train Avatar                   :  #CFree#n<BR><BR>");
          break;
        case CLASS_WARLOCK:
          if (ch->max_mana >= 2500)
            bprintf(buf2, "  Train Avatar                   :  #CFree#n<BR><BR>");
          break;
      }
    }
    else if (ch->level == 3 && !IS_SET(ch->newbits, NEW_MASTERY))
    {
      bprintf(buf2, "  Train Mortal                   :  #C%7d#n experience<BR><BR>", 5000000);
    }

    bprintf(buf2, "  Hitpoints                      :  #C%7d#n exp/point.<BR>",
      ch->max_hit + 1);
    bprintf(buf2, "  Movement                       :  #C%7d#n exp/point.<BR>",
      ch->max_move + 1);
    bprintf(buf2, "  Mana                           :  #C%7d#n exp/point.<BR>",
      ch->max_mana + 1);
    bprintf(buf2, "  Primal                         :  #C%7d#n exp/point.<BR>",
      (ch->practice + 1) * 500);
    bprintf(buf2, "  Stats (dex/str/con/wis/int)    :  #C%7d#n exp/point.<BR><BR>",
      100);

    if (getRank(ch, 1) >= RANK_ALMIGHTY)
    {
      bprintf(buf2, "  Legend                         :  #C%7d#n primal.<BR>"
                    "                                    #C%7d#n gold.<BR><BR>",
        (ch->pcdata->legend + 1) * 1000, (ch->pcdata->legend + 1) * 12500);
    }

    bprintf(buf2, "  #uImmunities#n                     :  #C%7d#n experience.<BR><BR>",
      (count < 10) ? (count + 1) * 10000 : 0);
    bprintf(buf2, "  [%s] <SEND href=\"%s slash\">slash</SEND>     "
                  "[%s] <SEND href=\"%s stab\">stab</SEND>     "
                  "[%s] <SEND href=\"%s smash\">smash</SEND>    "
                  "[%s] <SEND href=\"%s beast\">beast</SEND><BR>",
      (IS_IMMUNE(ch, IMM_SLASH)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_SLASH)) ? "relearn" : "train",
      (IS_IMMUNE(ch, IMM_STAB)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_STAB)) ? "relearn" : "train",
      (IS_IMMUNE(ch, IMM_SMASH)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_SMASH)) ? "relearn" : "train",
      (IS_IMMUNE(ch, IMM_ANIMAL)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_ANIMAL)) ? "relearn" : "train");
    bprintf(buf2, "  [%s] <SEND href=\"%s hurl\">hurl</SEND>      "
                  "[%s] <SEND href=\"%s acid\">acid</SEND>     "
                  "[%s] <SEND href=\"%s kick\">kick</SEND>     "
                  "[%s] <SEND href=\"%s disarm\">disarm</SEND><BR>",
      (IS_IMMUNE(ch, IMM_HURL)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_HURL)) ? "relearn" : "train",
      (IS_IMMUNE(ch, IMM_ACID)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_ACID)) ? "relearn" : "train",
      (IS_IMMUNE(ch, IMM_KICK)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_KICK)) ? "relearn" : "train",
      (IS_IMMUNE(ch, IMM_DISARM)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_DISARM)) ? "relearn" : "train");
    bprintf(buf2, "  [%s] <SEND href=\"%s steal\">steal</SEND>     "
                  "[%s] <SEND href=\"%s charm\">charm</SEND>    "
                  "[%s] <SEND href=\"%s heat\">heat</SEND>     "
                  "[%s] <SEND href=\"%s backstab\">backstab</SEND><BR>",
      (IS_IMMUNE(ch, IMM_STEAL)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_STEAL)) ? "relearn" : "train",
      (IS_IMMUNE(ch, IMM_CHARM)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_CHARM)) ? "relearn" : "train",
      (IS_IMMUNE(ch, IMM_HEAT)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_HEAT)) ? "relearn" : "train",
      (IS_IMMUNE(ch, IMM_BACKSTAB)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_BACKSTAB)) ? "relearn" : "train");
    bprintf(buf2, "  [%s] <SEND href=\"%s lightning\">lightning</SEND> "
                  "[%s] <SEND href=\"%s grab\">grab</SEND>     "
                  "[%s] <SEND href=\"%s cold\">cold</SEND>     "
                  "[%s] <SEND href=\"%s drain\">drain</SEND><BR>",
      (IS_IMMUNE(ch, IMM_LIGHTNING)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_LIGHTNING)) ? "relearn" : "train",
      (IS_IMMUNE(ch, IMM_MISC)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_MISC)) ? "relearn" : "train",
      (IS_IMMUNE(ch, IMM_COLD)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_COLD)) ? "relearn" : "train",
      (IS_IMMUNE(ch, IMM_DRAIN)) ? "o" : " ",
      (IS_IMMUNE(ch, IMM_DRAIN)) ? "relearn" : "train");

    mxp_to_char(buf2->data, ch, MXP_ALL);
    buffer_free(buf2);

    sprintf(arg1, "\n\r %s\n\n\r", get_dystopia_banner("", 56));
    send_to_char(arg1, ch);
  }
  else if (!str_cmp(arg1, "dex") || !str_cmp(arg1, "str") || !str_cmp(arg1, "con") ||
           !str_cmp(arg1, "wis") || !str_cmp(arg1, "int"))
  {
    int max_stat = 25, gain = 1;
    char stat[100];

    if (!str_cmp(arg1, "dex") && IS_CLASS(ch, CLASS_SHADOW) &&
        IS_SET(ch->pcdata->powers[EVOLVE_1], SHADOW_EVOLVE_SKULLDUGGERY))
      max_stat = 30;

    if ((!str_cmp(arg1, "dex") && ch->pcdata->perm_dex >= max_stat) ||
        (!str_cmp(arg1, "str") && ch->pcdata->perm_str >= max_stat) ||
        (!str_cmp(arg1, "con") && ch->pcdata->perm_con >= max_stat) ||
        (!str_cmp(arg1, "wis") && ch->pcdata->perm_wis >= max_stat) ||
        (!str_cmp(arg1, "int") && ch->pcdata->perm_int >= max_stat))
    {
      send_to_char("You have already reached the maximum for this statistic.\n\r", ch);
      return;
    }

    if (!str_cmp(arg1, "dex"))
    {
      if (!str_cmp(arg2, "all"))
        gain = max_stat - ch->pcdata->perm_dex;

      if (ch->exp < gain * 100)
      {
        send_to_char("You do not have enough experience.\n\r", ch);
        return;
      }
      ch->pcdata->perm_dex += gain;
      sprintf(stat, "dexterity");
    }
    else if (!str_cmp(arg1, "str"))
    {
      if (!str_cmp(arg2, "all"))
        gain = max_stat - ch->pcdata->perm_str;

      if (ch->exp < gain * 100)
      {
        send_to_char("You do not have enough experience.\n\r", ch);
        return;
      }
      ch->pcdata->perm_str += gain;
      sprintf(stat, "strength");
    }
    else if (!str_cmp(arg1, "con"))
    {
      if (!str_cmp(arg2, "all"))
        gain = max_stat - ch->pcdata->perm_con;

      if (ch->exp < gain * 100)
      {
        send_to_char("You do not have enough experience.\n\r", ch);
        return;
      }
      ch->pcdata->perm_con += gain;
      sprintf(stat, "constitution");
    }
    else if (!str_cmp(arg1, "wis"))
    {
      if (!str_cmp(arg2, "all"))
        gain = max_stat - ch->pcdata->perm_wis;

      if (ch->exp < gain * 100)
      {
        send_to_char("You do not have enough experience.\n\r", ch);
        return;
      }
      ch->pcdata->perm_wis += gain;
      sprintf(stat, "wisdom");
    }
    else if (!str_cmp(arg1, "int"))
    {
      if (!str_cmp(arg2, "all"))
        gain = max_stat - ch->pcdata->perm_int;

      if (ch->exp < gain * 100)
      {
        send_to_char("You do not have enough experience.\n\r", ch);
        return;
      }
      ch->pcdata->perm_int += gain;
      sprintf(stat, "intelligence");
    }

    ch->exp -= gain * 100;
    sprintf(buf, "Your %s increases %d point%s.\n\r",
      stat, gain, (gain == 1) ? "" : "s");
    send_to_char(buf, ch);
  }
  else if (!str_cmp(arg1, "mortal"))
  {
    if (IS_SET(ch->newbits, NEW_MASTERY))
    {
      send_to_char("You can no longer use this option.\n\r", ch);
      return;
    }
    if (ch->level != 3)
    {
      send_to_char("You need to be avatar to train mortal.\n\r", ch);
      return;
    }
    if (has_timer(ch)) return;
    if (in_arena(ch) || in_fortress(ch))
    {
      send_to_char("You cannot train mortal here.\n\r", ch);
      return;
    }
    if (ch->exp < 5000000)
    {
      send_to_char("You do not have the 5 million exp.\n\r", ch);
      return;
    }
    ch->exp -= 5000000;
    clearstats(ch);
    ch->level = 2;
    sprintf(buf, "%s forfeits %s avatar.",
      ch->name, (ch->sex == SEX_MALE) ? "his" : "her");
    do_info(ch, buf);
  }
  else if (!str_cmp(arg1, "primal"))
  {
    if (is_number(arg2))
    {
      if ((amount = atoi(arg2)) < 1 || amount > 1000)
      {
        send_to_char("Please enter a value between 1 and 1000.\n\r", ch);
        return;
      }

      cost = ((2 * ch->practice + amount) * (amount + 1) / 2 - ch->practice) * 500;

      if (ch->exp < cost)
      {
        printf_to_char(ch, "You need #C%s#n more exp to train that much primal.\n\r", dot_it_up(cost - ch->exp));
        send_to_char("If you wish to train as much as possible, use 'train primal all'.\n\r", ch);
      }
      else
      {
        ch->exp -= cost;
        ch->practice += amount;
        printf_to_char(ch, "You gain %d primal.\n\r", amount);
      }
    }
    else if (!str_cmp(arg2, "all"))
    {
      amount = 1000;

      for (i = 0; i < amount && ch->exp >= (ch->practice + 1) * 500; i++)
      {
        ch->practice++;
        ch->exp -= ch->practice * 500;
        j++;
      }

      if (i == 0)
        send_to_char("You need more exp to gain any primal.\n\r", ch);
      else
        printf_to_char(ch, "You gain %d primal.\n\r", i);
    }
    else
    {
      send_to_char("Please enter a numeric value between 1 and 1000 or use 'all'.\n\r", ch);
      return;
    }
  }
  else if (!str_cmp(arg1, "hp") || !str_cmp(arg1, "hitpoints"))
  {
    int statcap = 40000 + ch->pcdata->evolveCount * 5000;

    if (!IS_SET(ch->extra, EXTRA_PKREADY))
      statcap = 10000;

    if (ch->max_hit >= statcap)
    {
      send_to_char("You've reached the statcap.\n\r", ch);
      return;
    }

    if (is_number(arg2))
    {
      if ((amount = atoi(arg2)) < 1 || amount > 4000)
      {
        send_to_char("Please enter a value between 1 and 4000.\n\r", ch);
        return;
      }

      cost = (2 * ch->max_hit + amount) * (amount + 1) / 2 - ch->max_hit;

      if (ch->exp < cost)
      {
        printf_to_char(ch, "You need #C%s#n more exp to train that much hps.\n\r", dot_it_up(cost - ch->exp));
        send_to_char("If you wish to train as much as possible, use 'train hp all'.\n\r", ch);
      }
      else
      {
        ch->exp -= cost;
        ch->max_hit += amount;
        ch->pcdata->session->hit += amount;
        printf_to_char(ch, "You gain %d hp%s.\n\r", amount, (amount == 1) ? "" : "s");
      }
    }
    else if (!str_cmp(arg2, "all"))
    {
      for (i = 0; i < statcap; i++)
      {
        if (ch->exp >= UMAX((ch->max_hit + 1), 1) && ch->max_hit < statcap)
        {
          ch->pcdata->session->hit++;
          ch->max_hit++;
          ch->exp -= UMAX(ch->max_hit, 1);
        }
        else
          break;
      }

      if (i == 0)
        send_to_char("You need more exp to gain any hps.\n\r", ch);
      else
        printf_to_char(ch, "You gain %d hp%s.\n\r", i, (i == 1) ? "" : "s");
    }
    else
    {
      send_to_char("Please enter a numeric value between 1 and 4000 or use 'all'.\n\r", ch);
      return;
    }
  }
  else if (!str_cmp(arg1, "move") ||!str_cmp(arg1, "movement"))
  {
    int statcap = 40000 + ch->pcdata->evolveCount * 5000;

    if (!IS_SET(ch->extra, EXTRA_PKREADY))
      statcap = 10000;

    if (ch->max_move >= statcap)
    {
      send_to_char("You've reached the statcap.\n\r", ch);
      return;
    }

    if (is_number(arg2))
    {
      if ((amount = atoi(arg2)) < 1 || amount > 4000)
      {
        send_to_char("Please enter a value between 1 and 4000.\n\r", ch);
        return;
      }

      cost = (2 * ch->max_move + amount) * (amount + 1) / 2 - ch->max_move;

      if (ch->exp < cost)
      {
        printf_to_char(ch, "You need #C%s#n more exp to train that much movement.\n\r", dot_it_up(cost - ch->exp));
        send_to_char("If you wish to train as much as possible, use 'train move all'.\n\r", ch);
      }
      else
      {
        ch->exp -= cost;
        ch->max_move += amount;
        ch->pcdata->session->move += amount;
        printf_to_char(ch, "You gain %d move.\n\r", amount);
      }
    }
    else if (!str_cmp(arg2, "all"))
    {
      for (i = 0; i < statcap; i++)
      {
        if (ch->exp >= UMAX((ch->max_move + 1), 1) && ch->max_move < statcap)
        {
          ch->pcdata->session->move++;
          ch->max_move++;
          ch->exp -= UMAX(ch->max_move, 1);
        }
        else
          break;
      }

      if (i == 0)
        send_to_char("You need more exp to gain any movement.\n\r", ch);
      else
        printf_to_char(ch, "You gain %d move.\n\r", i);
    }
    else
    {
      send_to_char("Please enter a numeric value between 1 and 4000 or use 'all'.\n\r", ch);
      return;
    }
  }
  else if (!str_cmp(arg1, "mana"))
  {
    int statcap = 40000 + ch->pcdata->evolveCount * 5000;

    if (!IS_SET(ch->extra, EXTRA_PKREADY))
      statcap = 10000;

    if (ch->max_mana >= statcap)
    {
      send_to_char("You've reached the statcap.\n\r", ch);
      return;
    }

    if (is_number(arg2))
    {
      if ((amount = atoi(arg2)) < 1 || amount > 4000)
      {
        send_to_char("Please enter a value between 1 and 4000.\n\r", ch);
        return;
      }

      cost = (2 * ch->max_mana + amount) * (amount + 1) / 2 - ch->max_mana;

      if (ch->exp < cost)
      {
        printf_to_char(ch, "You need #C%s#n more exp to train that much mana.\n\r", dot_it_up(cost - ch->exp));
        send_to_char("If you wish to train as much as possible, use 'train mana all'.\n\r", ch);
      }
      else
      {
        ch->exp -= cost;
        ch->max_mana += amount;
        ch->pcdata->session->mana += amount;
        printf_to_char(ch, "You gain %d mana.\n\r", amount);
      }
    }
    else if (!str_cmp(arg2, "all"))
    {
      for (i = 0; i < statcap; i++)
      {
        if (ch->exp >= UMAX((ch->max_mana + 1), 1) && ch->max_mana < statcap)
        {
          ch->pcdata->session->mana++;
          ch->max_mana++;
          ch->exp -= UMAX(ch->max_mana, 1);
        }
        else
          break;
      }

      if (i == 0)
        send_to_char("You need more exp to gain any mana.\n\r", ch);
      else
        printf_to_char(ch, "You gain %d mana.\n\r", i);
    }
    else
    {
      send_to_char("Please enter a numeric value between 1 and 4000 or use 'all'.\n\r", ch);
      return;
    }
  }
  else if (!str_cmp(arg1, "legend") && getRank(ch, 1) >= RANK_ALMIGHTY)
  {
    EVENT_DATA *event;

    if (event_isset_mobile(ch, EVENT_PLAYER_LEGEND))
    {
      send_to_char("You must wait for Calim to grant your request.\n\r", ch);
      return;
    }

    if (ch->practice < (ch->pcdata->legend + 1) * 1000)
    {
      printf_to_char(ch, "You need %d more primal to gain this legend.\n\r", (ch->pcdata->legend + 1) * 1000 - ch->practice);
      return;
    }
    if (getGold(ch) < 12500 * (ch->pcdata->legend + 1))
    {
      printf_to_char(ch, "You are short %d goldcrowns.\n\r", (ch->pcdata->legend + 1) * 12500 - getGold(ch));
      return;
    }

    act("$n calls for the great Calim.", ch, NULL, NULL, TO_ROOM);
    act("You beseech the great Calim.", ch, NULL, NULL, TO_CHAR);

    event = alloc_event();
    event->fun = &event_player_legend;
    event->type = EVENT_PLAYER_LEGEND;
    add_event_char(event, ch, 2 * PULSE_PER_SECOND);
    return;
  }
  else if (!str_cmp(arg1, "slash") || !str_cmp(arg1, "stab") ||
           !str_cmp(arg1, "smash") || !str_cmp(arg1, "beast") ||
           !str_cmp(arg1, "grab") || !str_cmp(arg1, "charm") ||
           !str_cmp(arg1, "heat") || !str_cmp(arg1, "cold") ||
           !str_cmp(arg1, "lightning") || !str_cmp(arg1, "acid") ||
           !str_cmp(arg1, "drain") || !str_cmp(arg1, "hurl") ||
           !str_cmp(arg1, "backstab") || !str_cmp(arg1, "kick") ||
           !str_cmp(arg1, "disarm") || !str_cmp(arg1, "steal"))
  {
    int immune_counter = count_imm(ch);

    if (immune_counter >= 10)
    {
      send_to_char("You already have 10 immunities/resistances.\n\r", ch);
      send_to_char("Use relearn to remove an immunity/resistance.\n\r", ch);
    }
    else if (ch->exp < (immune_counter + 1) * 10000)
    {
      send_to_char("You do not have enough experience to buy this immunity.\n\r", ch);
    }
    else if (!str_cmp(arg1, "slash") && !IS_IMMUNE(ch, IMM_SLASH))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_SLASH);
      send_to_char("You are now more resistant to slashing and slicing weapons.\n\r", ch);
    }
    else if (!str_cmp(arg1, "stab") && !IS_IMMUNE(ch, IMM_STAB))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_STAB);
      send_to_char("You are now more resistant to stabbing and piercing weapons.\n\r", ch);
    }
    else if (!str_cmp(arg1, "smash") && !IS_IMMUNE(ch, IMM_SMASH))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_SMASH);
      send_to_char("You are now more resistant to blasting, pounding and crushing weapons.\n\r", ch);
    }
    else if (!str_cmp(arg1, "beast") && !IS_IMMUNE(ch, IMM_ANIMAL))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_ANIMAL);
      send_to_char("You are now more resistant to claw and bite attacks.\n\r", ch);
    }
    else if (!str_cmp(arg1, "grab") && !IS_IMMUNE(ch, IMM_MISC))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_MISC);
      send_to_char("You are now more resistant to grepping, sucking and whipping weapons.\n\r", ch);
    }
    else if (!str_cmp(arg1, "charm") && !IS_IMMUNE(ch, IMM_CHARM))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_CHARM);
      send_to_char("You are now immune to charm spells.\n\r", ch);
    }
    else if (!str_cmp(arg1, "heat") && !IS_IMMUNE(ch, IMM_HEAT))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_HEAT);
      send_to_char("You are now immune to heat and fire spells.\n\r", ch);
    }
    else if (!str_cmp(arg1, "cold") && !IS_IMMUNE(ch, IMM_COLD))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_COLD);
      send_to_char("You are now immune to cold spells.\n\r", ch);
    }
    else if (!str_cmp(arg1, "lightning") && !IS_IMMUNE(ch, IMM_LIGHTNING))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_LIGHTNING);
      send_to_char("You are now immune to lightning and electrical spells.\n\r", ch);
    }
    else if (!str_cmp(arg1, "acid") && !IS_IMMUNE(ch, IMM_ACID))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_ACID);
      send_to_char("You are now immune to acid spells.\n\r", ch);
    }
    else if (!str_cmp(arg1, "drain") && !IS_IMMUNE(ch, IMM_DRAIN))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_DRAIN);
      send_to_char("You are now immune to the energy drain spell.\n\r", ch);
    }
    else if (!str_cmp(arg1, "hurl") && !IS_IMMUNE(ch, IMM_HURL))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_HURL);
      send_to_char("You are now immune to being hurled.\n\r", ch);
    }
    else if (!str_cmp(arg1, "backstab") && !IS_IMMUNE(ch, IMM_BACKSTAB))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_BACKSTAB);
      send_to_char("You are now immune to being backstabbed.\n\r", ch);
    }
    else if (!str_cmp(arg1, "kick") && !IS_IMMUNE(ch, IMM_KICK))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_KICK);
      send_to_char("You are now immune to being kicked.\n\r", ch);
    }
    else if (!str_cmp(arg1, "disarm") && !IS_IMMUNE(ch, IMM_DISARM))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_DISARM);
      send_to_char("You are now immune to being disarmed.\n\r", ch);
    }
    else if (!str_cmp(arg1, "steal") && !IS_IMMUNE(ch, IMM_STEAL))
    {
      ch->exp -= (immune_counter + 1) * 10000;
      SET_BIT(ch->immune, IMM_STEAL);
      send_to_char("You are now immune to being stolen from.\n\r", ch);
    }
    else
    {
      send_to_char("You are already immune to that.\n\r", ch);
    }
  }
  else if (!str_cmp(arg1, "avatar") && ch->level == 2)
  {
    int gm = 0, gms = 0;
    bool gmblast = FALSE, canavatar = FALSE;

    clearstats(ch);
    if (ch->max_hit < 2000)
    {
      send_to_char("You need at least 2000 hp to train avatar.\n\r", ch);
      return;
    }

    for (i = 0; i < 13; i++)
    {
      if (ch->wpn[i] >= 200)
        gm++;
      if (i == 6 && ch->wpn[i] >= 200)
        gmblast = TRUE;
    }
    for (i = 6; i < 11; i++)
    {
      if (ch->stance[i] >= 200)
        gms++;
    }

    switch(ch->class)
    {
      default:
        send_to_char("Your class cannot train avatar.\n\r", ch);
        return;
        break;
      case CLASS_FAE:
        if (ch->max_move >= 3500 && ch->max_mana >= 3500 && ch->max_hit >= 5000 && gm >= 3 && gmblast && gms >= 2)
          canavatar = TRUE;
        break;
      case CLASS_GIANT:
        if (ch->max_move >= 2500)
          canavatar = TRUE;
        break;
      case CLASS_SHADOW:
        if (ch->max_hit >= 2500)
          canavatar = TRUE;
        break;
      case CLASS_WARLOCK:   
        if (ch->max_mana >= 2500)
          canavatar = TRUE;
        break;
    }

    if (!canavatar)
    {
      send_to_char("You do not meet the requirements to train avatar.\n\r", ch);
      send_to_char("Please read HELP REQUIREMENT.\n\r", ch);
      return;
    }

    ch->level = 3;

    if (!ragnarok)
      ch->pcdata->safe_counter = 10;
    else
      ch->pcdata->safe_counter = 3;

    send_to_char("You become an avatar!\n\r", ch);

    if (IS_CLASS(ch, CLASS_FAE))
      ch->form = ITEM_WEAR_FAE;
    else
      ch->form = ITEM_WEAR_ALL;

    if (IS_SET(ch->pcdata->jflags, JFLAG_SETAVATAR))
    {
      avatar_message(ch);
    }
    else
    {
      sprintf(buf, "%s has become an avatar!", ch->name);
      avatar_info(buf);
    }
  }
  else
  {
    do_train(ch, "");
  }
}

void do_mount(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Mount what?\n\r", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_POLYMORPH))
  {
    send_to_char("You cannot ride in this form.\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (ch == victim)
  {
    send_to_char("You cannot ride on your own back!\n\r", ch);
    return;
  }

  if (ch->mounted > 0)
  {
    send_to_char("You are already riding.\n\r", ch);
    return;
  }

  if (!IS_NPC(victim) && !IS_IMMORTAL(ch))
  {
    send_to_char("You cannot mount them.\n\r", ch);
    return;
  }
  if (victim->mounted > 0)
  {
    send_to_char("You cannot mount them.\n\r", ch);
    return;
  }
  if (IS_NPC(victim) && !IS_SET(victim->act, ACT_MOUNT) && !IS_IMMORTAL(ch))
  {
    send_to_char("You cannot mount them.\n\r", ch);
    return;
  }

  if (victim->position < POS_STANDING)
  {
    if (victim->position < POS_SLEEPING)
      act("$N is too badly hurt for that.", ch, NULL, victim, TO_CHAR);
    else if (victim->position == POS_SLEEPING)
      act("First you better wake $m up.", ch, NULL, victim, TO_CHAR);
    else if (victim->position == POS_RESTING)
      act("First $e better stand up.", ch, NULL, victim, TO_CHAR);
    else if (victim->position == POS_MEDITATING)
      act("First $e better stand up.", ch, NULL, victim, TO_CHAR);
    else if (victim->position == POS_SITTING)
      act("First $e better stand up.", ch, NULL, victim, TO_CHAR);
    else if (victim->position == POS_SLEEPING)
      act("First you better wake $m up.", ch, NULL, victim, TO_CHAR);
    else if (victim->position == POS_FIGHTING)
      act("Not while $e's fighting.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if (!IS_NPC(ch) && ch->stance[0] != -1)
    do_stance(ch, "none");

  ch->mounted = IS_RIDING;
  victim->mounted = IS_MOUNT;
  ch->mount = victim;
  victim->mount = ch;

  act("You clamber onto $N's back.", ch, NULL, victim, TO_CHAR);
  act("$n clambers onto $N's back.", ch, NULL, victim, TO_ROOM);
  return;
}

void do_dismount(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;

  if (IS_SET(ch->mounted, IS_RIDING))
  {
    if ((victim = ch->mount) == NULL)
    {
      ch->mounted = 0;
      send_to_char("You stop riding the air.\n\r", ch);
      return;
    }
  }
  if (ch->mounted == 0)
  {
    send_to_char("But you are not riding!\n\r", ch);
    return;
  }

  if ((victim = ch->mount) == NULL)
  {
    send_to_char("But you are not riding!\n\r", ch);
    return;
  }

  act("You clamber off $N's back.", ch, NULL, victim, TO_CHAR);
  act("$n clambers off $N's back.", ch, NULL, victim, TO_ROOM);

  ch->mounted = IS_ON_FOOT;
  victim->mounted = IS_ON_FOOT;

  ch->mount = NULL;
  victim->mount = NULL;

  return;
}

void do_tie(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  bool found = FALSE;

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, arg);
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("You cannot tie a mob up!\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char("You cannot tie yourself up!\n\r", ch);
    return;
  }
  if (IS_EXTRA(victim, TIED_UP))
  {
    send_to_char("But they are already tied up!\n\r", ch);
    return;
  }
  if (victim->position > POS_STUNNED || victim->hit > 0)
  {
    send_to_char("You can only tie up a defenceless person.\n\r", ch);
    return;
  }
  if (in_arena(victim))
  {
    sprintf(buf, "#C%s #ohas been vanquished from the #Rarena#o by #C%s#n", victim->name, ch->name);
    do_info(ch, buf);
    victim->pcdata->alosses++;

    if ((location = get_room_index(ROOM_VNUM_CITYSAFE)) == NULL)
    {
      bug("do_tie: no altar.", 0);
      return;
    }

    char_from_room(victim);
    char_to_room(victim, location, TRUE);

    /* stop spectating */
    stop_spectating(victim);

    victim->fight_timer = 0;
    restore_player(victim);
    call_all(victim);

    /* update arena kill counter */
    muddata.pk_count_now[1]++;

    /*  Check for winner  */
    found = FALSE;

    pIter = AllocIterator(char_list);
    while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (IS_NPC(victim))
        continue;

      /* any other players in the arena */
      if (in_arena(victim) && victim != ch)
        found = TRUE;
    }

    if (!found)
    {
      sprintf(buf, "#C%s #oemerges victorious from the #Rarena#n", ch->name);
      ch->pcdata->awins++;
      do_info(ch, buf);

      char_from_room(ch);
      char_to_room(ch, location, TRUE);
      ch->fight_timer = 0;
      restore_player(ch);
      win_arena(ch);

      /* stop spectating */
      stop_spectating(ch);

      REMOVE_BIT(arena.status, ARENA_ARENA_INUSE);
      SET_BIT(arena.status, ARENA_ARENA_CLOSED);
    }

    return;
  }

  act("You quickly tie up $N.", ch, NULL, victim, TO_CHAR);
  act("$n quickly ties up $N.", ch, NULL, victim, TO_ROOM);
  send_to_char("You have been tied up!\n\r", victim);
  SET_BIT(victim->extra, TIED_UP);

  if (IS_SET(ch->pcdata->jflags, JFLAG_SETTIE))
  {
    tie_message(ch, victim);
  }
  else
  {
    sprintf(buf, "#P%s #yhas been tied up by #R%s#n", victim->name, ch->name);
    do_info(ch, buf);
  }
}

void do_untie(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument(argument, arg);
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }
  if (!IS_EXTRA(victim, TIED_UP))
  {
    send_to_char("But they are not tied up!\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char("You cannot untie yourself!\n\r", ch);
    return;
  }
  act("You quickly untie $N.", ch, NULL, victim, TO_CHAR);
  act("$n quickly unties $N.", ch, NULL, victim, TO_NOTVICT);
  act("$n quickly unties you.", ch, NULL, victim, TO_VICT);
  REMOVE_BIT(victim->extra, TIED_UP);
  return;
}

void do_gag(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument(argument, arg);
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch && !IS_EXTRA(victim, GAGGED) && IS_EXTRA(victim, TIED_UP))
  {
    send_to_char("You cannot gag yourself!\n\r", ch);
    return;
  }
  if (!IS_EXTRA(victim, TIED_UP) && !IS_EXTRA(victim, GAGGED))
  {
    send_to_char("You can only gag someone who is tied up!\n\r", ch);
    return;
  }
  if (!IS_EXTRA(victim, GAGGED))
  {
    act("You place a gag over $N's mouth.", ch, NULL, victim, TO_CHAR);
    act("$n places a gag over $N's mouth.", ch, NULL, victim, TO_NOTVICT);
    act("$n places a gag over your mouth.", ch, NULL, victim, TO_VICT);
    SET_BIT(victim->extra, GAGGED);
    return;
  }
  if (ch == victim)
  {
    act("You remove the gag from your mouth.", ch, NULL, victim, TO_CHAR);
    act("$n removes the gag from $s mouth.", ch, NULL, victim, TO_ROOM);
    REMOVE_BIT(victim->extra, GAGGED);
    return;
  }
  act("You remove the gag from $N's mouth.", ch, NULL, victim, TO_CHAR);
  act("$n removes the gag from $N's mouth.", ch, NULL, victim, TO_NOTVICT);
  act("$n removes the gag from your mouth.", ch, NULL, victim, TO_VICT);
  REMOVE_BIT(victim->extra, GAGGED);
  return;
}

void do_blindfold(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument(argument, arg);
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch && !IS_EXTRA(victim, BLINDFOLDED) && IS_EXTRA(victim, TIED_UP))
  {
    send_to_char("You cannot blindfold yourself!\n\r", ch);
    return;
  }
  if (!IS_EXTRA(victim, TIED_UP) && !IS_EXTRA(victim, BLINDFOLDED))
  {
    send_to_char("You can only blindfold someone who is tied up!\n\r", ch);
    return;
  }
  if (!IS_EXTRA(victim, BLINDFOLDED))
  {
    act("You place a blindfold over $N's eyes.", ch, NULL, victim, TO_CHAR);
    act("$n places a blindfold over $N's eyes.", ch, NULL, victim, TO_NOTVICT);
    act("$n places a blindfold over your eyes.", ch, NULL, victim, TO_VICT);
    SET_BIT(victim->extra, BLINDFOLDED);
    return;
  }
  if (ch == victim)
  {
    act("You remove the blindfold from your eyes.", ch, NULL, victim, TO_CHAR);
    act("$n removes the blindfold from $s eyes.", ch, NULL, victim, TO_ROOM);
    REMOVE_BIT(victim->extra, BLINDFOLDED);
    return;
  }
  act("You remove the blindfold from $N's eyes.", ch, NULL, victim, TO_CHAR);
  act("$n removes the blindfold from $N's eyes.", ch, NULL, victim, TO_NOTVICT);
  act("$n removes the blindfold from your eyes.", ch, NULL, victim, TO_VICT);
  REMOVE_BIT(victim->extra, BLINDFOLDED);
}

void do_track(CHAR_DATA * ch, char *argument)
{
  EVENT_DATA *event;
  ITERATOR *pIter;
  char name[MAX_INPUT_LENGTH];
  char direction[MAX_INPUT_LENGTH];
  char *ptr;
  bool found = FALSE;

  act("$n carefully examines the ground for tracks.", ch, NULL, NULL, TO_ROOM);
  act("You carefully examine the ground for tracks.\n\r", ch, NULL, NULL, TO_CHAR);

  if (!IS_NPC(ch) && number_percent() > ch->pcdata->learned[gsn_track])
  {
    send_to_char("You cannot sense any trails from this room.\n\r", ch);
    return;
  }

  pIter = AllocIterator(ch->in_room->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == EVENT_ROOM_TRACKS)
    {
      ptr = one_argument(event->argument, name);
      one_argument(ptr, direction);

      if (event_pulses_left(event) < PULSE_TRACK / 2)
      {
        if (str_cmp(name, ch->name))
          printf_to_char(ch, "o You find some of #C%s#n's old tracks heading #C%s#n.\n\r", name, direction);
        else
          printf_to_char(ch, "o You find some of #Gyour own#n old tracks heading #C%s#n.\n\r", direction);
      }
      else
      {
        if (str_cmp(name, ch->name))
          printf_to_char(ch, "o You find #C%s#n's tracks heading #C%s#n.\n\r", name, direction);
        else
          printf_to_char(ch, "o You find #Gyour own#n tracks heading #C%s#n.\n\r", direction);
      }

      found = TRUE;
    }
  }

  if (!found)
    send_to_char("You cannot find any tracks in this room.\n\r", ch);
}

void add_tracks(CHAR_DATA *ch, ROOM_INDEX_DATA *from_room, int direction)
{
  EVENT_DATA *event;
  ITERATOR *pIter;
  char buf[MAX_INPUT_LENGTH];

  /* NPC's don't leave tracks */
  if (IS_NPC(ch))
    return;

  /* The special stalker items removes all tracks */
  if (IS_ITEMAFF(ch, ITEMA_STALKER))
    return;

  /* Shadowlords with the silentwalk power doesn't leave tracks */
  if (IS_CLASS(ch, CLASS_SHADOW) && IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_SILENTWALK))
    return;

  /* store name and direction in buffer */
  switch(direction)
  {
    default:
      sprintf(buf, "%s 'in some unknown direction'", ch->name);
      break;
    case DIR_NORTH:
      sprintf(buf, "%s 'north'", ch->name);
      break;
    case DIR_SOUTH:
      sprintf(buf, "%s 'south'", ch->name);
      break;
    case DIR_WEST:
      sprintf(buf, "%s 'west'", ch->name);
      break;
    case DIR_EAST:
      sprintf(buf, "%s 'east'", ch->name);
      break;
    case DIR_UP:
      sprintf(buf, "%s 'up'", ch->name);
      break;
    case DIR_DOWN:
      sprintf(buf, "%s 'down'", ch->name);
      break;
  }

  /* Check for previous tracks */
  pIter = AllocIterator(from_room->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == EVENT_ROOM_TRACKS && !str_cmp(buf, event->argument))
      break;
  }

  /* Remove old tracks */
  if (event != NULL)
    dequeue_event(event, TRUE);

  /* Add new tracks */
  event = alloc_event();
  event->type = EVENT_ROOM_TRACKS;
  event->fun = &event_dummy;
  event->argument = str_dup(buf);
  add_event_room(event, from_room, PULSE_TRACK);
}

void do_scry(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim = NULL;
  ROOM_INDEX_DATA *chroom;
  ROOM_INDEX_DATA *victimroom;
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument(argument, arg);

  if (IS_NPC(ch)) return;

  /* can the player use this power */
  if (!IS_ITEMAFF(ch, ITEMA_VISION) && !ragnarok)
  {
    if (!IS_CLASS(ch, CLASS_SHADOW) && !IS_CLASS(ch, CLASS_GIANT) && !IS_CLASS(ch, CLASS_FAE))
    {
      send_to_char("Huh?\n\r",ch);
      return;
    }

    if (IS_CLASS(ch, CLASS_GIANT) && ch->pcdata->powers[GIANT_RANK] < 4)
    {
      send_to_char("You are not tall enough.\n\r", ch);
      return;
    }

    if (IS_CLASS(ch, CLASS_FAE) && ch->pcdata->powers[DISC_FAE_NATURE] < 2)
    {
      send_to_char("You need level 2 discipline in nature.\n\r",ch);
      return;
    }

    if (IS_CLASS(ch, CLASS_SHADOW) && !IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_SCRY))
    {
      send_to_char("You don't have that power.\n\r", ch);
      return;
    }
  }

  if (arg[0] == '\0')
  {
    send_to_char("Scry whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("You are unable to locate them.\n\r", ch);
    return;
  }

  if (event_isset_mobile(victim, EVENT_PLAYER_DISPLACE))
  {
    send_to_char("Something blocks your scrying attempt.\n\r", ch);
    return;
  }

  /* check for shields */
  if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_SHIELDED) && !ragnarok)
  {
    send_to_char("You cannot find them.\n\r", ch);
    return;
  }

  if (event_isset_room(victim->in_room, EVENT_ROOM_WATERDOME))
  {
    send_to_char("All you can see is a great mass of blue.\n\r", ch);
    return;
  }

  if (event_isset_mobile(victim, EVENT_PLAYER_WITNESS))
  {
    send_to_char("The shadow witness whispers something in your ear.\n\r", victim);
    printf_to_char(victim, "  '%s is scrying on you sire.'.\n\r", ch->name);
  }

  if (!IS_NPC(victim) && IS_CLASS(victim, CLASS_FAE) && ch != victim &&
       IS_SET(victim->pcdata->powers[FAE_BITS], FAE_ACIDHEART))
  {
    EVENT_DATA *event;

    if (!event_isset_mobile(ch, EVENT_MOBILE_ACIDBLOOD))
    {
      event = alloc_event();
      event->fun = &event_mobile_acidblood;
      event->type = EVENT_MOBILE_ACIDBLOOD;
      event->argument = str_dup(victim->name);
      add_event_char(event, ch, 3 * PULSE_PER_SECOND);
    }
  }

  chroom = ch->in_room;
  victimroom = victim->in_room;
  char_from_room(ch);
  char_to_room(ch,victimroom, FALSE);
  do_look(ch, "scry");
  char_from_room(ch);
  char_to_room(ch, chroom, FALSE);

  if (!IS_NPC(victim))
    ch->fight_timer += 5;
}

bool event_player_alignment(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  OBJ_DATA *obj;
  ITERATOR *pIter;
  int align = atoi(event->argument) / 10;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_alignment: no owner.", 0);
    return FALSE;
  }

  ch->alignment = URANGE(-1000, align, 1000);

  /* now drop all anti-you items */
  pIter = AllocIterator(ch->carrying);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->wear_loc == WEAR_NONE) continue;

    if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) || (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)))
    {
      act("You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR);
      act("$n is zapped by $p and drops it.", ch, obj, NULL, TO_ROOM);
      obj_from_char(obj);
      obj_to_room(obj, ch->in_room);
    }
  }

  if (IS_GOOD(ch))
  {
    REMOVE_BIT(ch->affected_by, AFF_PROTECT_GOOD);
    send_to_char("Alignment is now angelic.\n\r", ch);
  }
  else if (IS_EVIL(ch))
  {
    REMOVE_BIT(ch->affected_by, AFF_PROTECT);
    send_to_char("Alignment is now satanic.\n\r", ch);
  }
  else
  {
    REMOVE_BIT(ch->affected_by, AFF_PROTECT);
    REMOVE_BIT(ch->affected_by, AFF_PROTECT_GOOD); 
    send_to_char("Alignment is now neutral.\n\r", ch);
  }

  return FALSE;
}

void do_alignment(CHAR_DATA * ch, char *argument)
{
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_ALIGNMENT)) != NULL)
  {
    int align = atoi(event->argument) / 10, i;
    BUFFER *buf = buffer_new(MAX_STRING_LENGTH);

    bprintf(buf, " %s\n\n\r", get_dystopia_banner("Alignment Quest", 40));
    bprintf(buf, "               Evil - Neutral - Good\n\r");
    bprintf(buf, "  Alignment : [");

    for (i = -1000; i <= 1000; i += 100)
    {
      if (align >= i)
        bprintf(buf, "*");
      else
        bprintf(buf, " ");
    }

    bprintf(buf, "]\n\n\r");
    bprintf(buf, "  Time Left :  %s\n\r", event_time_left(event));
    bprintf(buf, "\n\r %s\n\r", get_dystopia_banner("Alignment Quest", 40));

    send_to_char(buf->data, ch);
    buffer_free(buf);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0' || str_cmp(arg, "change"))
  {
    char alignment[MAX_INPUT_LENGTH];

    if (IS_GOOD(ch))
      sprintf(alignment, "good");
    else if (IS_EVIL(ch))
      sprintf(alignment, "evil");
    else
      sprintf(alignment, "neutral");

    printf_to_char(ch, "Your current alignment is #C%s#n (%d).\n\r", alignment, ch->alignment);
    send_to_char("Syntax: alignment change.\n\r", ch);
    return;
  }

  if (ch->generation < 2)
  {
    send_to_char("You cannot change your alignment (Generation 1).\n\r", ch);
    return;
  }

  sprintf(arg, "%d", ch->alignment * 10);
  event = alloc_event();
  event->fun = &event_player_alignment;
  event->type = EVENT_PLAYER_ALIGNMENT;
  event->argument = str_dup(arg);
  add_event_char(event, ch, 5 * 60 * PULSE_PER_SECOND);

  send_to_char("You have 5 minutes to act like your wanted alignment.\n\r", ch);
}
