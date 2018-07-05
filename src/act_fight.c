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


void do_kill(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument(argument, arg);
  if (IS_NPC(ch) && ch->desc == NULL)
    return;
  if (arg[0] == '\0')
  {
    send_to_char("Kill whom?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char("You cannot kill yourself!\n\r", ch);
    return;
  }
  if (is_safe(ch, victim))
    return;
  if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)
  {
    act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (ch->position == POS_FIGHTING)
  {
    send_to_char("You do the best you can!\n\r", ch);
    return;
  }
  WAIT_STATE(ch, 8);
  if (!IS_NPC(ch) && !IS_NPC(victim))
  {
    ch->fight_timer += 3;
    victim->fight_timer += 3;
  }

  if (victim->position == POS_STUNNED)
  {
    act("You scream out in hatred and attack $N.", ch, NULL, victim, TO_CHAR);
    act("$n screams out in hatred and attacks $N.", ch, NULL, victim, TO_NOTVICT);
    one_hit(ch, victim, TYPE_UNDEFINED, 1);
  }

  if (!victim->dead && victim->hit > 0)
    multi_hit(ch, victim, 1);
}

void do_backstab(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Backstab whom?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char("How can you sneak up on yourself?\n\r", ch);
    return;
  }
  if (is_safe(ch, victim))
    return;
  if (((obj = get_eq_char(ch, WEAR_WIELD)) == NULL || obj->value[3] != 11) && ((obj = get_eq_char(ch, WEAR_HOLD)) == NULL || obj->value[3] != 11))
  {
    send_to_char("You need to wield a piercing weapon.\n\r", ch);
    return;
  }
  if (victim->fighting != NULL)
  {
    send_to_char("You can't backstab a fighting person.\n\r", ch);
    return;
  }
  if (victim->hit < victim->max_hit)
  {
    act("$N is hurt and suspicious ... you can't sneak up.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if (!IS_NPC(victim) && IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_AWARENESS))
  {   
    act("You fail to sneak up on $N, $M catches you in the attempt.", ch, NULL, victim, TO_CHAR);
    act("$n tries to backstab you, but you catch $m in the attempt.", ch, NULL, victim, TO_VICT);
    return;
  }

  WAIT_STATE(ch, skill_table[gsn_backstab].beats);
  if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_BACKSTAB))
    damage(ch, victim, NULL, 0, gsn_backstab);
  else if (!IS_AWAKE(victim) || IS_NPC(ch) || number_percent() < ch->pcdata->learned[gsn_backstab])
  {
    one_hit(ch, victim, gsn_backstab, 1);
    multi_hit(ch, victim, 1);
  }
  else
    damage(ch, victim, NULL, 0, gsn_backstab);
}

void do_flee(CHAR_DATA * ch, char *argument)
{
  EVENT_DATA *event;

  if (ch->fighting == NULL)
  {
    if (ch->position == POS_FIGHTING)
      ch->position = POS_STANDING;

    send_to_char("You aren't fighting anyone.\n\r", ch);
    return;
  }

  if (event_isset_mobile(ch, EVENT_MOBILE_FLEE))
  {
    send_to_char("You are already trying to flee.\n\r", ch);
    return;
  }

  if (!IS_NPC(ch) && IS_CLASS(ch, CLASS_GIANT)) 
  {
    if (event_isset_mobile(ch, EVENT_PLAYER_DEATHFRENZY))
    {
      send_to_char("NO! You must stay and kill them stinky huuumans.\n\r", ch);
      return;
    }
  }

  event = alloc_event();
  event->fun = &event_mobile_flee;
  event->type = EVENT_MOBILE_FLEE;
  add_event_char(event, ch, 5);

  send_to_char("You move into a fleeing position.\n\r", ch);
  act("$n tries to escape from combat.", ch, NULL, NULL, TO_ROOM);

  if (ch->in_room && (event = event_isset_room(ch->in_room, EVENT_ROOM_PWALL)) != NULL)
  {
    int dam = event_pulses_left(event);

    if ((dam = dam * 30) > 0)
    {
      if (dam >= ch->hit)
        dam = ch->hit - 1;

      if (dam > 0)
      {
        modify_hps(ch, -1 * dam);

        act("$n is struck by the prismatic sheet for trying to flee.", ch, NULL, NULL, TO_ROOM);
        send_to_char("You are struck by the prismatic sheet of energy.\n\r", ch);
      }
    }
  }
}

void do_rescue(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *fch;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Rescue whom?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char("What about fleeing instead?\n\r", ch);
    return;
  }
  if (!IS_NPC(ch) && IS_NPC(victim))
  {
    send_to_char("Doesn't need your help!\n\r", ch);
    return;
  }
  if (ch->fighting == victim)
  {
    send_to_char("Too late.\n\r", ch);
    return;
  }
  if ((fch = victim->fighting) == NULL)
  {
    send_to_char("That person is not fighting right now.\n\r", ch);
    return;
  }
  if (is_safe(ch, fch) || is_safe(ch, victim))
    return;
  WAIT_STATE(ch, skill_table[gsn_rescue].beats);
  if (!IS_NPC(ch) && number_percent() > ch->pcdata->learned[gsn_rescue])
  {
    send_to_char("You fail the rescue.\n\r", ch);
    return;
  }
  act("You rescue $N!", ch, NULL, victim, TO_CHAR);
  act("$n rescues you!", ch, NULL, victim, TO_VICT);
  act("$n rescues $N!", ch, NULL, victim, TO_NOTVICT);
  stop_fighting(fch, FALSE);
  stop_fighting(victim, FALSE);

  aggress(ch, fch);
}

bool event_mobile_stance(EVENT_DATA *event)
{
  CHAR_DATA *ch; 
  int waitstate;
  char buf[MAX_INPUT_LENGTH];

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_stance: no owner.", 0);   
    return FALSE;
  }
   
  sprintf(buf, "%s", event->argument);
  dequeue_event(event, TRUE);
    
  waitstate = ch->wait;
  do_stance(ch, buf);  
  ch->wait = waitstate;
    
  return TRUE;
}

void do_kick(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int dam;
  int stance;

  if (!IS_NPC(ch) && ch->level < skill_table[gsn_kick].skill_level)
  {
    send_to_char("First you should learn to kick.\n\r", ch);
    return;
  }
  if ((victim = ch->fighting) == NULL)
  {
    send_to_char("You aren't fighting anyone.\n\r", ch);
    return;
  }

  one_argument(argument, arg);

  if (arg[0] != '\0')
  {
    if ((victim = get_char_room(ch, arg)) == NULL)
    {
      send_to_char("They are not here.\n\r", ch);
      return;
    }

    if (victim->fighting != ch)
    {
      send_to_char("They are not fighting you.\n\r", ch);
      return;
    }
  }

  WAIT_STATE(ch, skill_table[gsn_kick].beats);

  if ((IS_NPC(ch) && ch->level < 1800))
  {
    dam = 500;
    damage(ch, victim, NULL, dam, gsn_kick);
    return;
  }
  if ((IS_NPC(ch)) || number_percent() < ch->pcdata->learned[gsn_kick])
    dam = number_range(1, 4);
  else
  {
    dam = 0;
    damage(ch, victim, NULL, dam, gsn_kick);
    return;
  }
  dam += char_damroll(ch);
  if (dam == 0)
    dam = 1;
  if (!IS_AWAKE(victim))
    dam *= 2;
  if (IS_NPC(ch))
    dam *= 100;
  if (!IS_NPC(ch))
    dam = dam + (dam * ((ch->wpn[0] + 1) / 100));
  if (!IS_NPC(ch))
  {
    stance = ch->stance[0];
    dam = dambonus(ch, victim, dam, stance);
  }
  if (!IS_NPC(ch) && !IS_NPC(victim) && dam > 750)
    dam = 750;
  if (dam <= 0)
    dam = 2;
  dam = cap_dam(ch, victim, dam);
  if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_KICK))
    damage(ch, victim, NULL, 0, gsn_kick);
  else
    damage(ch, victim, NULL, dam, gsn_kick);
  return;
}

void do_punch(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int dam;

  one_argument(argument, arg);
  if (IS_NPC(ch))
    return;
  if (ch->level < skill_table[gsn_punch].skill_level)
  {
    send_to_char("First you should learn to punch.\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (ch == victim)
  {
    send_to_char("You cannot punch yourself!\n\r", ch);
    return;
  }
  if (is_safe(ch, victim))
    return;
  if (victim->hit < victim->max_hit)
  {
    send_to_char("They are hurt and suspicious.\n\r", ch);
    return;
  }
  if (victim->position < POS_FIGHTING)
  {
    send_to_char("You can only punch someone who is standing.\n\r", ch);
    return;
  }
  act("You draw your fist back and aim a punch at $N.", ch, NULL, victim, TO_CHAR);
  act("$n draws $s fist back and aims a punch at you.", ch, NULL, victim, TO_VICT);
  act("$n draws $s fist back and aims a punch at $N.", ch, NULL, victim, TO_NOTVICT);
  WAIT_STATE(ch, skill_table[gsn_punch].beats);
  if (IS_NPC(ch) || number_percent() < ch->pcdata->learned[gsn_punch])
    dam = number_range(1, 4);
  else
  {
    dam = 0;
    damage(ch, victim, NULL, dam, gsn_punch);
    return;
  }
  dam += char_damroll(ch);
  if (dam == 0)
    dam = 1;
  if (!IS_AWAKE(victim))
    dam *= 2;
  if (!IS_NPC(ch))
    dam = dam + (dam * (ch->wpn[0] / 100));
  if (dam <= 0)
    dam = 1;
  if (dam > 1000)
    dam = 1000;

  sound_to_room("punch.wav", ch);

  damage(ch, victim, NULL, dam, gsn_punch);
  if (victim == NULL || victim->position == POS_DEAD || dam < 1)
    return;
  if (victim->position == POS_FIGHTING)
    stop_fighting(victim, TRUE);
  if (number_percent() <= 25 && !IS_HEAD(victim, BROKEN_NOSE) && !IS_HEAD(victim, LOST_NOSE))
  {
    act("Your nose shatters under the impact of the blow!", victim, NULL, NULL, TO_CHAR);
    act("$n's nose shatters under the impact of the blow!", victim, NULL, NULL, TO_ROOM);
    SET_BIT(victim->loc_hp[LOC_HEAD], BROKEN_NOSE);
  }
  else if (number_percent() <= 25 && !IS_HEAD(victim, BROKEN_JAW))
  {
    act("Your jaw shatters under the impact of the blow!", victim, NULL, NULL, TO_CHAR);
    act("$n's jaw shatters under the impact of the blow!", victim, NULL, NULL, TO_ROOM);
    SET_BIT(victim->loc_hp[LOC_HEAD], BROKEN_JAW);
  }
  act("You fall to the ground stunned!", victim, NULL, NULL, TO_CHAR);
  act("$n falls to the ground stunned!", victim, NULL, NULL, TO_ROOM);
  victim->position = POS_STUNNED;
  if (dam > 1000)
    dam = 1000;
  return;
}

void do_berserk(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *vch;
  ITERATOR *pIter;
  CHAR_DATA *mount;
  int number_hit = 0;

  argument = one_argument(argument, arg);
  if (IS_NPC(ch))
    return;
  if (ch->level < skill_table[gsn_berserk].skill_level)
  {
    send_to_char("You are not wild enough to go berserk.\n\r", ch);
    return;
  }
  WAIT_STATE(ch, 24);
  if (number_percent() > ch->pcdata->learned[gsn_berserk])
  {
    act("You rant and rave, but nothing much happens.", ch, NULL, NULL, TO_CHAR);
    act("$n gets a wild look in $s eyes, but nothing much happens.", ch, NULL, NULL, TO_ROOM);
    return;
  }
  act("You go BERSERK!", ch, NULL, NULL, TO_CHAR);
  act("$n goes BERSERK!", ch, NULL, NULL, TO_ROOM);
  sound_to_room("berserk.wav", ch);

  pIter = AllocIterator(ch->in_room->people);
  while ((vch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (number_hit > 4)
      continue;
    if (!IS_NPC(vch))
      continue;
    if (ch == vch)
      continue;

    if ((mount = ch->mount) != NULL)
      if (mount == vch)
        continue;
    if (can_see(ch, vch))
    {
      multi_hit(ch, vch, 4);
      number_hit++;
    }
  }
}

/* Hurl skill by KaVir */
void do_hurl(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  CHAR_DATA *mount;
  ROOM_INDEX_DATA *to_room;
  EXIT_DATA *pexit;
  EXIT_DATA *pexit_rev;
  char buf[MAX_INPUT_LENGTH];
  char direction[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int door;
  int revdir;
  int dam;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  if (!IS_NPC(ch) && ch->pcdata->learned[gsn_hurl] < 1)
  {
    send_to_char("Maybe you should learn the skill first?\n\r", ch);
    return;
  }
  if (arg1[0] == '\0')
  {
    send_to_char("Who do you wish to hurl?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg1)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char("How can you hurl yourself?\n\r", ch);
    return;
  }
  if (!IS_NPC(victim) && is_safe(ch, victim))
    return;
  if ((mount = victim->mount) != NULL && victim->mounted == IS_MOUNT)
  {
    send_to_char("But they have someone on their back!\n\r", ch);
    return;
  }
  else if ((mount = victim->mount) != NULL && victim->mounted == IS_RIDING)
  {
    send_to_char("But they are riding!\n\r", ch);
    return;
  }
  if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_HURL))
  {
    send_to_char("You are unable to get their feet off the ground.\n\r", ch);
    return;
  }
  if (IS_NPC(victim) && victim->level > 900)
  {
    send_to_char("You are unable to get their feet off the ground.\n\r", ch);
    return;
  }
  if ((victim->hit < victim->max_hit) || (victim->position == POS_FIGHTING && victim->fighting != ch))
  {
    act("$N is hurt and suspicious, and you are unable to approach $M.", ch, NULL, victim, TO_CHAR);
    return;
  }
  WAIT_STATE(ch, skill_table[gsn_hurl].beats);
  if (!IS_NPC(ch) && number_percent() > ch->pcdata->learned[gsn_hurl])
  {
    send_to_char("You are unable to get their feet off the ground.\n\r", ch);
    multi_hit(victim, ch, 1);
    return;
  }
  revdir = 0;
  if (arg2[0] == '\0')
    door = number_range(0, 3);
  else
  {
    if (!str_cmp(arg2, "n") || !str_cmp(arg2, "north"))
      door = 0;
    else if (!str_cmp(arg2, "e") || !str_cmp(arg2, "east"))
      door = 1;
    else if (!str_cmp(arg2, "s") || !str_cmp(arg2, "south"))
      door = 2;
    else if (!str_cmp(arg2, "w") || !str_cmp(arg2, "west"))
      door = 3;
    else
    {
      send_to_char("You can only hurl people north, south, east or west.\n\r", ch);
      return;
    }
  }
  if (door == 0)
  {
    sprintf(direction, "north");
    revdir = 2;
  }
  if (door == 1)
  {
    sprintf(direction, "east");
    revdir = 3;
  }
  if (door == 2)
  {
    sprintf(direction, "south");
    revdir = 0;
  }
  if (door == 3)
  {
    sprintf(direction, "west");
    revdir = 1;
  }
  if ((pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
  {
    sprintf(buf, "$n hurls $N into the %s wall.", direction);
    act(buf, ch, NULL, victim, TO_NOTVICT);
    sprintf(buf, "You hurl $N into the %s wall.", direction);
    act(buf, ch, NULL, victim, TO_CHAR);
    sprintf(buf, "$n hurls you into the %s wall.", direction);
    act(buf, ch, NULL, victim, TO_VICT);
    dam = number_range(ch->level, (ch->level * 4));
    victim->hit = victim->hit - dam;
    update_pos(victim);
    if (IS_NPC(victim) && !IS_NPC(ch))
      ch->mkill = ch->mkill + 1;
    if (!IS_NPC(victim) && IS_NPC(ch))
      victim->mdeath = victim->mdeath + 1;
    if (victim->position == POS_DEAD)
    {
      raw_kill(victim, ch);
      return;
    }
    return;
  }

  pexit = victim->in_room->exit[door];

  if (IS_SET(pexit->exit_info, EX_PRISMATIC_WALL))
  {
    sprintf(buf, "$n hurls $N into the %s wall.", direction);
    act(buf, ch, NULL, victim, TO_NOTVICT);
    sprintf(buf, "You hurl $N into the %s wall.", direction);
    act(buf, ch, NULL, victim, TO_CHAR);
    sprintf(buf, "$n hurls you into the %s wall.", direction);
    act(buf, ch, NULL, victim, TO_VICT);
    dam = number_range(ch->level, (ch->level * 4));
    victim->hit = victim->hit - dam;
    update_pos(victim);

    if (IS_NPC(victim) && !IS_NPC(ch))
      ch->mkill = ch->mkill + 1;
    if (!IS_NPC(victim) && IS_NPC(ch))
      victim->mdeath = victim->mdeath + 1;
    if (victim->position == POS_DEAD)
    {
      raw_kill(victim, ch);
      return;
    }
    return;
  }

  if (IS_SET(pexit->exit_info, EX_CLOSED) && !IS_AFFECTED(victim, AFF_PASS_DOOR) && !IS_AFFECTED(victim, AFF_ETHEREAL))
  {
    if (IS_SET(pexit->exit_info, EX_LOCKED))
      REMOVE_BIT(pexit->exit_info, EX_LOCKED);
    if (IS_SET(pexit->exit_info, EX_CLOSED))
      REMOVE_BIT(pexit->exit_info, EX_CLOSED);
    sprintf(buf, "$n hoists $N in the air and hurls $M %s.", direction);
    act(buf, ch, NULL, victim, TO_NOTVICT);
    sprintf(buf, "You hoist $N in the air and hurl $M %s.", direction);
    act(buf, ch, NULL, victim, TO_CHAR);

    if (!IS_SET(pexit->exit_info, EX_LOCKED))
    {
      sprintf(buf, "$n hurls you %s, smashing you through the %s.", direction, pexit->keyword);
      act(buf, ch, NULL, victim, TO_VICT);
      sprintf(buf, "There is a loud crash as $n smashes through the $d.");
      act(buf, victim, NULL, pexit->keyword, TO_ROOM);
    
      if ((to_room = pexit->to_room) != NULL && (pexit_rev = to_room->exit[revdir]) != NULL && pexit_rev->to_room == ch->in_room && pexit_rev->keyword != NULL)
      {
        if (IS_SET(pexit_rev->exit_info, EX_CLOSED))
          REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
        if (door == 0)
          sprintf(direction, "south");
        if (door == 1)
          sprintf(direction, "west");
        if (door == 2)
          sprintf(direction, "north");
        if (door == 3)
          sprintf(direction, "east");
        char_from_room(victim);
        char_to_room(victim, to_room, TRUE);
        sprintf(buf, "$n comes smashing in through the %s $d.", direction);
        act(buf, victim, NULL, pexit->keyword, TO_ROOM);
      }

      dam = number_range(ch->level, (ch->level * 6));
      victim->hit = victim->hit - dam;
      update_pos(victim);
      if (IS_NPC(victim) && !IS_NPC(ch))
        ch->mkill = ch->mkill + 1;
      if (!IS_NPC(victim) && IS_NPC(ch))
        victim->mdeath = victim->mdeath + 1;
      if (victim->position == POS_DEAD)
      {
        raw_kill(victim, ch);
        return;
      }
    }
  }
  else
  {
    sprintf(buf, "$n hurls $N %s.", direction);
    act(buf, ch, NULL, victim, TO_NOTVICT);
    sprintf(buf, "You hurl $N %s.", direction);
    act(buf, ch, NULL, victim, TO_CHAR);
    sprintf(buf, "$n hurls you %s.", direction);
    act(buf, ch, NULL, victim, TO_VICT);
    if (door == 0)
      sprintf(direction, "south");
    if (door == 1)
      sprintf(direction, "west");
    if (door == 2)
      sprintf(direction, "north");
    if (door == 3)
      sprintf(direction, "east");
    char_from_room(victim);
    char_to_room(victim, to_room, TRUE);
    sprintf(buf, "$n comes flying in from the %s.", direction);
    act(buf, victim, NULL, NULL, TO_ROOM);
    dam = number_range(ch->level, (ch->level * 2));
    victim->hit = victim->hit - dam;
    update_pos(victim);
    if (IS_NPC(victim) && !IS_NPC(ch))
      ch->mkill = ch->mkill + 1;
    if (!IS_NPC(victim) && IS_NPC(ch))
      victim->mdeath = victim->mdeath + 1;
    if (victim->position == POS_DEAD)
    {
      raw_kill(victim, ch);
      return;
    }
  }
}

void do_disarm(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int percent;

  if (IS_NPC(ch)) return;

  if (ch->level < skill_table[gsn_disarm].skill_level)
  {
    send_to_char("You don't know how to disarm opponents.\n\r", ch);
    return;
  }

  if ((get_eq_char(ch, WEAR_WIELD) == NULL) && (get_eq_char(ch, WEAR_HOLD) == NULL)
    && !(IS_CLASS(ch, CLASS_FAE) && IS_SET(ch->pcdata->powers[FAE_BITS], FAE_BLASTBEAMS)))
  {
    send_to_char("You must wield a weapon to disarm.\n\r", ch);
    return;
  }

  if ((victim = ch->fighting) == NULL)
  {
    send_to_char("You aren't fighting anyone.\n\r", ch);
    return;
  }

  if (((obj = get_eq_char(victim, WEAR_WIELD)) == NULL) && ((obj = get_eq_char(victim, WEAR_HOLD)) == NULL))
  {
    send_to_char("Your opponent is not wielding a weapon.\n\r", ch);
    return;
  }

  WAIT_STATE(ch, skill_table[gsn_disarm].beats);
  percent = number_percent() + victim->level - ch->level;
  if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_DISARM))
    send_to_char("You failed.\n\r", ch);
  else if (IS_NPC(ch) || percent < ch->pcdata->learned[gsn_disarm] * 2 / 3)
    disarm(ch, victim);
  else
    send_to_char("You failed.\n\r", ch);
  return;
}

/* For decapitating players - KaVir */
void do_decapitate(CHAR_DATA * ch, char *argument)
{
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;
  QUEST_DATA *quest;
  CHAR_DATA *victim;
  EVENT_DATA *event;
  ROOM_INDEX_DATA *location;
  bool can_decap = TRUE;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  bool deathmatch = FALSE;
  bool archmage = FALSE;
  bool wasfed = FALSE;
  int gexp = 0, i;

  if (IS_NPC(ch)) return;
  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Decapitate whom?\n\r", ch);
    return;
  }
  if (in_arena(ch)) 
  {
    send_to_char("You're in the arena.\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }
  if (ch == victim)
  {
    send_to_char("That might be a bit tricky...\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("You can only decapitate other players.\n\r", ch);
    return;
  }
  if (victim->position > POS_MORTAL)
  {
    send_to_char("You can only do this to mortally wounded players.\n\r", ch);
    return;
  }
  if (is_safe(ch, victim)) return;

  if (victim->level > 6)
  {
    send_to_char("Be nice and don't decap the admins.\n\r", ch);
    return;
  }

  if (!IS_SET(ch->extra, EXTRA_PKREADY))
  {
    send_to_char("Before you start killing, you must become pkready.\n\r", ch);
    return;
  }

  if (!IS_SET(victim->extra, EXTRA_PKREADY))
  {
    send_to_char("They are not pkready.\n\r", ch);
    return;
  }

  /* check for feeding */
  wasfed = check_feed(ch, victim);

  if (in_fortress(ch) && IS_SET(arena.status, ARENA_FORTRESS_SPAR))
  {
    fortresskill(ch, victim);
    return;
  }
  else if (in_fortress(ch) && IS_SET(arena.status, ARENA_FORTRESS_DEATH))
  {
    deathmatch = TRUE;

    if ((location = get_room_index(ROOM_VNUM_VICTORY)) == NULL)
    {
      bug("Do_decap: No Victory Room.", 0);
      return;
    }

    open_fortress();

    char_from_room(victim);
    char_to_room(victim, location, TRUE);
    char_from_room(ch);
    char_to_room(ch, location, TRUE);
    ch->fight_timer = 0;

    /* cancel spectating on both players */
    stop_spectating(ch);
    stop_spectating(victim);
  }
  else if (ragnarok)
  {
    ragnarokdecap(ch, victim);
    return;
  }

  /* only players with an actual rank can pk */
  if (getMight(ch) < RANK_CADET)
  {
    send_to_char("If you wish to pk, then you should move into the pk-range.\n\r", ch);
    return;
  }

  /* archmage status update */
  if (IS_CLASS(ch, CLASS_WARLOCK) && IS_CLASS(victim, CLASS_WARLOCK) && !wasfed)
  {
    if (ch->pcdata->powers[WARLOCK_PATH] == victim->pcdata->powers[WARLOCK_PATH] &&
        ch->pcdata->powers[WARLOCK_RANK] == WLCK_RNK_WARLOCK && victim->pcdata->powers[WARLOCK_RANK] == WLCK_RNK_ARCHMAGE)
    {
      free_string(archmage_list[victim->pcdata->powers[WARLOCK_PATH]].player);
      archmage_list[victim->pcdata->powers[WARLOCK_PATH]].player = str_dup(ch->name);
      ch->pcdata->powers[WARLOCK_RANK] = WLCK_RNK_ARCHMAGE;
      victim->pcdata->powers[WARLOCK_RANK] = WLCK_RNK_WARLOCK;
      save_archmages();

      for (i = 0; i < 5; i++)
      {
        victim->spl[i] = UMIN(220, victim->spl[i]);
      }

      archmage = TRUE;
    }
  }

  if (!fair_fight(ch, victim))
    can_decap = FALSE;
  if (ch->level > 6)
    can_decap = TRUE;

  if (!can_decap)
  {
    if (archmage)
    {
      behead(victim);   
      victim->level = 2;
      dropinvis(ch);
      sprintf(buf, "#P%s #owas torn to pieces by #R%s #c(#0Archmage Kill#c)#n", victim->name, ch->name);
      death_info(buf);
      muddata.pk_count_now[0]++;
      return;
    }

    /* here we add pseudo-retaliate for highlevel vs lowlevel */
    if (can_kill_lowbie(ch, victim))
    {
      behead(victim);
      victim->level = 2;
      dropinvis(ch);
      sprintf(buf, "#P%s #owas torn to pieces by #R%s #c(#0Retaliation#c)#n", victim->name, ch->name);
      death_info(buf);
      log_string("%s retaliates against %s.", ch->name, victim->name);
      muddata.pk_count_now[0]++;
      victim->pdeath++;
      return;
    }

    /* artifact kill? */
    if (IS_SET(victim->pcdata->tempflag, TEMP_ARTIFACT))
    {
      behead(victim);
      victim->level = 2;
      dropinvis(ch);
      sprintf(buf, "#P%s #owas torn to pieces by #R%s #c(#0Artifact#c)#n", victim->name, ch->name);
      death_info(buf);
      log_string("%s arti-kills %s.", ch->name, victim->name);
      muddata.pk_count_now[0]++;
      return;
    }

    if (IS_SET(ch->act, PLR_PARADOX))
    {
      send_to_char("You would get a paradox counter if you where to do this...\n\r"
                   "You must enable paradox kills by typing 'config paradox'.\n\r", ch);
      return;
    }

    behead(victim);
    victim->level = 2;
    dropinvis(ch);
    ch->pcdata->mean_paradox_counter++;
    ch->pcdata->bounty += number_range(60, 120);
    sprintf(buf, "#P%s #owas torn to pieces by #R%s #c(#0Paradox Counter#c)#n", victim->name, ch->name);
    death_info(buf);
    if (ch->pcdata->mean_paradox_counter > 2)
    {
      ch->pcdata->mean_paradox_counter = 0;
      do_paradox(ch, "self");
    }

    return;
  }

  if (victim->pcdata->bounty > 0)
  {
    sprintf(buf, "You receive a %d QP bounty, for killing %s.\n\r", victim->pcdata->bounty, victim->name);
    send_to_char(buf, ch);
    setGold(ch, victim->pcdata->bounty);
    victim->pcdata->bounty = 0;
  }

  /* update quest status */
  pIter = AllocIterator(ch->pcdata->quests);
  while ((quest = (QUEST_DATA *) NextInList(pIter)) != NULL)
  {
    if (quest->type == QT_PK)
    {
      if (quest->vnums[0] == victim->pcdata->playerid)
      {
        quest->vnums[0] = -1;
        send_to_char("#GYou have fulfilled a quest.#n\n\r", ch);
        break;
      }
    }
  }

  sound_to_char("deathcry1.wav", victim);

  if (!wasfed)
  {
    update_edge(ch);
    gexp = victim->exp / 3;
    victim->exp -= victim->exp / 3;
  }

  /* update players status */
  if (!wasfed)
  {
    if ((victim->pcdata->status == 0 && ch->pcdata->status == 0) || victim->pcdata->status > 0)
    {
      ch->pcdata->status++;
      send_to_char("You gain a status point.\n\r", ch);

      if (victim->pcdata->status > 0)
      {
        victim->pcdata->status--;
        send_to_char("You lose a status point.\n\r", victim);
      }
    }
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_STUDY)) == NULL && gexp > 0)
  {
    sprintf(buf, "You receive %s experience points.\n\r", dot_it_up(gexp));
    send_to_char(buf, ch);
    ch->exp += gexp;
    ch->pcdata->session->exp += gexp;
  }
  else if (gexp > 0)
  {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int exp_needed;
    char *marg;

    marg = one_argument(event->argument, arg1);
    one_argument(marg, arg2);
    exp_needed = atoi(arg2);
    exp_needed -= gexp;
    free_string(event->argument);
    sprintf(arg2, "%s %d", arg1, UMAX(0, exp_needed));
    event->argument = str_dup(arg2);

    sprintf(buf, "You gather %d study points.\n\r", gexp);
    send_to_char(buf, ch);

    if (exp_needed < 1)
      event->passes = 0;
  }

  /*
   * hitpoints reward for killing
   */
  modify_hps(ch, victim->max_hit * 0.1);

  /*
   * Update the last decaps to prevent spamcapping.
   */
  free_string(ch->pcdata->last_decap[1]);
  ch->pcdata->last_decap[1] = str_dup(ch->pcdata->last_decap[0]);
  free_string(ch->pcdata->last_decap[0]);
  ch->pcdata->last_decap[0] = str_dup(victim->name);

  /*
   * Retaliation update
   */
  free_string(victim->pcdata->retaliation[1]);
  victim->pcdata->retaliation[1] = str_dup(victim->pcdata->retaliation[0]);
  free_string(victim->pcdata->retaliation[0]);
  victim->pcdata->retaliation[0] = str_dup(ch->name);

  /* clear retaliation */
  if (!str_cmp(ch->pcdata->retaliation[0], victim->name))
  {
    free_string(ch->pcdata->retaliation[0]);
    ch->pcdata->retaliation[0] = str_dup("Noone");
  }
  if (!str_cmp(ch->pcdata->retaliation[1], victim->name))
  {
    free_string(ch->pcdata->retaliation[1]);
    ch->pcdata->retaliation[1] = str_dup("Noone");
  }

  /* update kingdom pkills and pdeaths */
  if (!wasfed && (kingdom = get_kingdom(ch)) != NULL)
  {
    MEMBER_DATA *member;

    pIter = AllocIterator(kingdom->members);
    while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
    {
      if (!str_cmp(member->name, ch->name))
        member->pk++;
    }

    kingdom->pkills++;
    save_kingdom(kingdom);
  }
  if (!wasfed && (kingdom = get_kingdom(victim)) != NULL)
  {
    MEMBER_DATA *member;

    pIter = AllocIterator(kingdom->members);
    while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
    {
      if (!str_cmp(member->name, victim->name))
        member->pd++;
    }

    kingdom->pdeaths++;
    save_kingdom(kingdom);
  }

  muddata.pk_count_now[0]++;

  act("A misty white vapour pours from $N's corpse into your body.", ch, NULL, victim, TO_CHAR);
  act("A misty white vapour pours from $N's corpse into $n's body.", ch, NULL, victim, TO_NOTVICT);
  act("You double over in agony as raw energy pulses through your veins.", ch, NULL, NULL, TO_CHAR);
  act("$n doubles over in agony as sparks of energy crackle around $m.", ch, NULL, NULL, TO_NOTVICT);
  if (!deathmatch)
    ch->fight_timer += 10;
  behead(victim);
  dropinvis(ch);

  /* auto sacrifice heads */
  if (IS_SET(ch->act, PLR_AUTOHEAD))
  {
    do_sacrifice(ch, "head");
  }

  if (ch->pcdata->mean_paradox_counter > 0)
    ch->pcdata->mean_paradox_counter--;

  ch->pkill++;
  ch->pcdata->session->pkills++;

  victim->pdeath++;
  victim->level = 2;
  decap_message(ch, victim);

  if (!wasfed)
  {
    log_string("%s decapitated by %s at %d.", victim->name, ch->name, victim->in_room->vnum);
  }
  else
  {
    log_string("%s was fed to %s at %d.", victim->name, ch->name, victim->in_room->vnum);
  }

  ch->pcdata->bounty += number_range(30, 80);
}

void do_crack(CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj;
  OBJ_DATA *right;
  OBJ_DATA *left;

  right = get_eq_char(ch, WEAR_WIELD);
  left = get_eq_char(ch, WEAR_HOLD);
  if (right != NULL && right->pIndexData->vnum == 12)
    obj = right;
  else if (left != NULL && left->pIndexData->vnum == 12)
    obj = left;
  else
  {
    send_to_char("You are not holding any heads.\n\r", ch);
    return;
  }
  act("You hurl $p at the floor.", ch, obj, NULL, TO_CHAR);
  act("$n hurls $p at the floor.", ch, obj, NULL, TO_ROOM);
  act("$p cracks open, leaking brains out across the floor.", ch, obj, NULL, TO_CHAR);
  act("$p cracks open, leaking brains out across the floor.", ch, obj, NULL, TO_ROOM);
  extract_obj(obj);
}

void do_autostance(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char stance[MAX_INPUT_LENGTH];
  int pick;

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, stance);
  argument = one_argument(argument, arg);

  if (stance[0] == '\0' || arg[0] == '\0')
  {
    send_to_char("Syntax: autostance [monster|pk] [stance]\n\r", ch);
    return;
  }

  if (!str_cmp(stance, "monster"))
  {
    send_to_char("Trying to set monster stance.\n\r", ch);
    pick = STANCE_MOBSTANCE;
  }
  else if (!str_cmp(stance, "pk"))
  {
    send_to_char("Trying to set pk stance.\n\r", ch);
    pick = STANCE_PKSTANCE;
  }
  else
  {
    do_autostance(ch, "");
    return;
  }

  if (!str_cmp(arg, "none"))
  {
    send_to_char("You no longer autostance.\n\r", ch);
    ch->stance[pick] = STANCE_NONE;
  }
  else if (!str_cmp(arg, "crane"))
  {
    send_to_char("You now autostance into the crane stance.\n\r", ch);
    ch->stance[pick] = STANCE_CRANE;
  }
  else if (!str_cmp(arg, "crab"))
  {
    send_to_char("You now autostance into the crab stance.\n\r", ch);
    ch->stance[pick] = STANCE_CRAB;
  }
  else if (!str_cmp(arg, "bull"))
  {
    send_to_char("You now autostance into the bull stance.\n\r", ch);
    ch->stance[pick] = STANCE_BULL;
  }
  else if (!str_cmp(arg, "viper"))
  {
    send_to_char("You now autostance into the viper stance.\n\r", ch);
    ch->stance[pick] = STANCE_VIPER;
  }
  else if (!str_cmp(arg, "mongoose"))
  {
    send_to_char("You now autostance into the mongoose stance.\n\r", ch);
    ch->stance[pick] = STANCE_MONGOOSE;
  }
  else if (!str_cmp(arg, "mantis") && ch->stance[STANCE_CRANE] >= 200 && ch->stance[STANCE_VIPER] >= 200)
  {
    send_to_char("You now autostance into the mantis stance.\n\r", ch);
    ch->stance[pick] = STANCE_MANTIS;
  }
  else if (!str_cmp(arg, "monkey") && ch->stance[STANCE_CRANE] >= 200 && ch->stance[STANCE_MONGOOSE] >= 200)
  {
    send_to_char("You now autostance into the monkey stance.\n\r", ch);
    ch->stance[pick] = STANCE_MONKEY;
  }
  else if (!str_cmp(arg, "swallow") && ch->stance[STANCE_CRAB] >= 200 && ch->stance[STANCE_MONGOOSE] >= 200)
  {
    send_to_char("You now autostance into the swallow stance.\n\r", ch);
    ch->stance[pick] = STANCE_SWALLOW;
  }
  else if (!str_cmp(arg, "tiger") && ch->stance[STANCE_BULL] >= 200 && ch->stance[STANCE_VIPER] >= 200)
  {
    send_to_char("You now autostance into the tiger stance.\n\r", ch);
    ch->stance[pick] = STANCE_TIGER;
  }
  else if (!str_cmp(arg, "dragon") && ch->stance[STANCE_CRAB] >= 200 && ch->stance[STANCE_BULL] >= 200)
  {
    send_to_char("You now autostance into the dragon stance.\n\r", ch);
    ch->stance[pick] = STANCE_DRAGON;
  }
  else if (!str_cmp(arg, "spirit") && IS_CLASS(ch, CLASS_FAE)
        && ch->stance[STANCE_DRAGON] >= 200 && ch->stance[STANCE_TIGER] >= 200
        && ch->stance[STANCE_SWALLOW] >= 200 && ch->stance[STANCE_MANTIS] >= 200
        && ch->stance[STANCE_MONKEY] >= 200)
  {
    send_to_char("You now autostance into the spirit stance.\n\r", ch);
    ch->stance[pick] = STANCE_SPIRIT;
  }
  else
    send_to_char("You can't set your autostance to that!\n\r", ch);
}

void do_skill(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char bufskill[25];
  char bufskill2[25];
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *wield;
  OBJ_DATA *wield2;
  int dtype;
  int dtype2;

  one_argument(argument, arg);

  if (IS_NPC(ch))
    return;

  if (arg[0] == '\0')
    sprintf(arg, "self");

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  wield = get_eq_char(victim, WEAR_WIELD);
  wield2 = get_eq_char(victim, WEAR_HOLD);

  dtype = TYPE_HIT;
  dtype2 = TYPE_HIT;
  if (wield != NULL && wield->item_type == ITEM_WEAPON)
    dtype += wield->value[3];
  if (wield2 != NULL && wield2->item_type == ITEM_WEAPON)
    dtype2 += wield2->value[3];
  dtype -= 1000;
  dtype2 -= 1000;

  if (victim->wpn[dtype] == 0)
    sprintf(bufskill, "totally unskilled");
  else if (victim->wpn[dtype] <= 25)
    sprintf(bufskill, "slightly skilled");
  else if (victim->wpn[dtype] <= 50)
    sprintf(bufskill, "reasonable");
  else if (victim->wpn[dtype] <= 75)
    sprintf(bufskill, "fairly competent");
  else if (victim->wpn[dtype] <= 100)
    sprintf(bufskill, "highly skilled");
  else if (victim->wpn[dtype] <= 125)
    sprintf(bufskill, "very dangerous");
  else if (victim->wpn[dtype] <= 150)
    sprintf(bufskill, "extremely deadly");
  else if (victim->wpn[dtype] <= 175)
    sprintf(bufskill, "an expert");
  else if (victim->wpn[dtype] <= 199)
    sprintf(bufskill, "a master");
  else if (victim->wpn[dtype] == 200)
    sprintf(bufskill, "a grand master");
  else if (victim->wpn[dtype] <= 999)
    sprintf(bufskill, "supremely skilled");
  else if (victim->wpn[dtype] == 1000)
    sprintf(bufskill, "divinely skilled");
  else
    return;

  if (victim->wpn[dtype2] == 0)
    sprintf(bufskill2, "totally unskilled");
  else if (victim->wpn[dtype2] <= 25)
    sprintf(bufskill2, "slightly skilled");
  else if (victim->wpn[dtype2] <= 50)
    sprintf(bufskill2, "reasonable");
  else if (victim->wpn[dtype2] <= 75)
    sprintf(bufskill2, "fairly competent");
  else if (victim->wpn[dtype2] <= 100)
    sprintf(bufskill2, "highly skilled");
  else if (victim->wpn[dtype2] <= 125)
    sprintf(bufskill2, "very dangerous");
  else if (victim->wpn[dtype2] <= 150)
    sprintf(bufskill2, "extremely deadly");
  else if (victim->wpn[dtype2] <= 175)
    sprintf(bufskill2, "an expert");
  else if (victim->wpn[dtype2] <= 199)
    sprintf(bufskill2, "a master");
  else if (victim->wpn[dtype2] == 200)
    sprintf(bufskill2, "a grand master");
  else if (victim->wpn[dtype2] <= 999)
    sprintf(bufskill2, "supremely skilled");
  else if (victim->wpn[dtype2] == 1000)
    sprintf(bufskill2, "divinely skilled");
  else
    return;

  if (ch == victim)
  {
    if (dtype == 0 && dtype2 == 0)
      sprintf(buf, "You are %s at unarmed combat.\n\r", bufskill);
    else
    {
      if (dtype != 0)
        sprintf(buf, "You are %s with %s.\n\r", bufskill, wield->short_descr);
      if (dtype2 != 0)
        sprintf(buf2, "You are %s with %s.\n\r", bufskill2, wield2->short_descr);
    }
  }
  else
  {
    if (dtype == 0 && dtype2 == 0)
      sprintf(buf, "%s is %s at unarmed combat.\n\r", victim->name, bufskill);
    else
    {
      if (dtype != 0)
        sprintf(buf, "%s is %s with %s.\n\r", victim->name, bufskill, wield->short_descr);
      if (dtype2 != 0)
        sprintf(buf2, "%s is %s with %s.\n\r", victim->name, bufskill2, wield2->short_descr);
    }
  }
  if (!(dtype == 0 && dtype2 != 0))
    send_to_char(buf, ch);
  if (dtype2 != 0)
    send_to_char(buf2, ch);
  skillstance(ch, victim);
}

void do_throw(CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location;
  ROOM_INDEX_DATA *to_room;
  EXIT_DATA *pexit;
  CHAR_DATA *victim = NULL;
  OBJ_DATA *obj;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  char revdoor[MAX_INPUT_LENGTH];
  int door;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL)
  {
    if ((obj = get_eq_char(ch, WEAR_HOLD)) == NULL)
    {
      send_to_char("You are not holding anything to throw.\n\r", ch);
      return;
    }
  }

  if (arg1[0] == '\0')
  {
    send_to_char("Which direction do you wish to throw?\n\r", ch);
    return;
  }

  if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north"))
  {
    door = 0;
    sprintf(arg1, "north");
  }
  else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east"))
  {
    door = 1;
    sprintf(arg1, "east");
  }
  else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south"))
  {
    door = 2;
    sprintf(arg1, "south");
  }
  else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west"))
  {
    door = 3;
    sprintf(arg1, "west");
  }
  else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up"))
  {
    door = 4;
    sprintf(arg1, "up");
  }
  else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down"))
  {
    door = 5;
    sprintf(arg1, "down");
  }
  else
  {
    send_to_char("You can only throw north, south, east, west, up or down.\n\r", ch);
    return;
  }

  if (!str_cmp(arg1, "north"))
  {
    door = 0;
    sprintf(revdoor, "south");
  }
  else if (!str_cmp(arg1, "east"))
  {
    door = 1;
    sprintf(revdoor, "west");
  }
  else if (!str_cmp(arg1, "south"))
  {
    door = 2;
    sprintf(revdoor, "north");
  }
  else if (!str_cmp(arg1, "west"))
  {
    door = 3;
    sprintf(revdoor, "east");
  }
  else if (!str_cmp(arg1, "up"))
  {
    door = 4;
    sprintf(revdoor, "down");
  }
  else if (!str_cmp(arg1, "down"))
  {
    door = 5;
    sprintf(revdoor, "up");
  }
  else
    return;

  location = ch->in_room;

  sprintf(buf, "You hurl $p %s.", arg1);
  act(buf, ch, obj, NULL, TO_CHAR);
  sprintf(buf, "$n hurls $p %s.", arg1);
  act(buf, ch, obj, NULL, TO_ROOM);
  /* First room */
  if ((pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
  {
    sprintf(buf, "$p bounces off the %s wall.", arg1);
    act(buf, ch, obj, NULL, TO_ROOM);
    act(buf, ch, obj, NULL, TO_CHAR);
    obj_from_char(obj);
    obj_to_room(obj, ch->in_room);
    return;
  }
  pexit = ch->in_room->exit[door];
  if (IS_SET(pexit->exit_info, EX_CLOSED))
  {
    sprintf(buf, "$p bounces off the %s door.", arg1);
    act(buf, ch, obj, NULL, TO_ROOM);
    act(buf, ch, obj, NULL, TO_CHAR);
    obj_from_char(obj);
    obj_to_room(obj, ch->in_room);
    return;
  }
  char_from_room(ch);
  char_to_room(ch, to_room, TRUE);
  if ((victim = get_char_room(ch, arg2)) != NULL)
  {
    sprintf(buf, "$p comes flying in from the %s and lands in $N's hands.", revdoor);
    act(buf, ch, obj, victim, TO_NOTVICT);
    sprintf(buf, "$p comes flying in from the %s and lands in your hands.", revdoor);
    act(buf, ch, obj, victim, TO_VICT);
    obj_from_char(obj);
    obj_to_char(obj, victim);
    char_from_room(ch);
    char_to_room(ch, location, TRUE);
    return;
  }

  /* Second room */
  if ((pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
  {
    sprintf(buf, "$p comes flying in from the %s and strikes %s wall.", revdoor, arg1);
    act(buf, ch, obj, NULL, TO_ROOM);
    obj_from_char(obj);
    obj_to_room(obj, ch->in_room);
    char_from_room(ch);
    char_to_room(ch, location, TRUE);
    return;
  }
  pexit = ch->in_room->exit[door];
  if (IS_SET(pexit->exit_info, EX_CLOSED))
  {
    sprintf(buf, "$p comes flying in from the %s and strikes the %s door.", revdoor, arg1);
    act(buf, ch, obj, NULL, TO_ROOM);
    obj_from_char(obj);
    obj_to_room(obj, ch->in_room);
    char_from_room(ch);
    char_to_room(ch, location, TRUE);
    return;
  }
  sprintf(buf, "$p comes flying in from the %s and carries on %s.", revdoor, arg1);
  act(buf, ch, obj, NULL, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, to_room, TRUE);
  if ((victim = get_char_room(ch, arg2)) != NULL)
  {
    sprintf(buf, "$p comes flying in from the %s and lands in $N's hands.", revdoor);
    act(buf, ch, obj, victim, TO_NOTVICT);
    sprintf(buf, "$p comes flying in from the %s and lands in your hands.", revdoor);
    act(buf, ch, obj, victim, TO_VICT);
    obj_from_char(obj);
    obj_to_char(obj, victim);
    char_from_room(ch);
    char_to_room(ch, location, TRUE);
    return;
  }

  /* Third room */
  if ((pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
  {
    sprintf(buf, "$p comes flying in from the %s and strikes %s wall.", revdoor, arg1);
    act(buf, ch, obj, NULL, TO_ROOM);
    obj_from_char(obj);
    obj_to_room(obj, ch->in_room);
    char_from_room(ch);
    char_to_room(ch, location, TRUE);
    return;
  }
  pexit = ch->in_room->exit[door];
  if (IS_SET(pexit->exit_info, EX_CLOSED))
  {
    sprintf(buf, "$p comes flying in from the %s and strikes the %s door.", revdoor, arg1);
    act(buf, ch, obj, NULL, TO_ROOM);
    obj_from_char(obj);
    obj_to_room(obj, ch->in_room);
    char_from_room(ch);
    char_to_room(ch, location, TRUE);
    return;
  }
  sprintf(buf, "$p comes flying in from the %s and carries on %s.", revdoor, arg1);
  act(buf, ch, obj, NULL, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, to_room, TRUE);
  if ((victim = get_char_room(ch, arg2)) != NULL)
  {
    sprintf(buf, "$p comes flying in from the %s and lands in $N's hands.", revdoor);
    act(buf, ch, obj, victim, TO_NOTVICT);
    sprintf(buf, "$p comes flying in from the %s and lands in your hands.", revdoor);
    act(buf, ch, obj, victim, TO_VICT);
    obj_from_char(obj);
    obj_to_char(obj, victim);
    char_from_room(ch);
    char_to_room(ch, location, TRUE);
    return;
  }

  sprintf(buf, "$p comes flying in from the %s and drops at your feet.", revdoor);
  act(buf, ch, obj, NULL, TO_ROOM);
  obj_from_char(obj);
  obj_to_room(obj, ch->in_room);

  /* Move them back */
  char_from_room(ch);
  char_to_room(ch, location, TRUE);
}

void do_stance(CHAR_DATA * ch, char *argument)
{
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];
  char *stance;
  int selection;

  if (event_isset_mobile(ch, EVENT_MOBILE_STANCING))
  {
    send_to_char("You are already trying to change your fighting stance.\n\r", ch);
    return;
  }

  argument = one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    if (ch->stance[0] > 0)
    {
      switch(ch->stance[0])
      {
        default:
          stance = "bugged";
          break;
        case STANCE_BULL:
          stance = "bull";
          break;
        case STANCE_MANTIS:
          stance = "mantis";
          break;
        case STANCE_DRAGON:
          stance = "dragon";
          break;
        case STANCE_TIGER:
          stance = "tiger";
          break;
        case STANCE_MONKEY:
          stance = "monkey";
          break;
        case STANCE_SWALLOW:
          stance = "swallow";
          break;
        case STANCE_CRAB:
          stance = "crab";
          break;
        case STANCE_CRANE:
          stance = "crane";
          break;
        case STANCE_VIPER:
          stance = "viper";
          break;
        case STANCE_MONGOOSE:
          stance = "mongoose";
          break;
        case STANCE_SPIRIT:
          stance = "spirit";
          break;
      }
      printf_to_char(ch, "You are currently in the #C%s#n stance.\n\r", stance);
    }
    send_to_char("Change into what stance : none, bull, crane, viper, crab, mongoose\n\r", ch);
    if ((ch->stance[STANCE_CRANE] >= 200 && ch->stance[STANCE_VIPER] >= 200) ||
        (ch->stance[STANCE_BULL] >= 200 && ch->stance[STANCE_CRAB] >= 200) ||
        (ch->stance[STANCE_BULL] >= 200 && ch->stance[STANCE_VIPER] >= 200) ||
        (ch->stance[STANCE_CRANE] >= 200 && ch->stance[STANCE_MONGOOSE] >= 200) ||
        (ch->stance[STANCE_CRAB] >= 200 && ch->stance[STANCE_MONGOOSE] >= 200))
    {
      bool found = FALSE;

      send_to_char("                          ", ch);
      if (ch->stance[STANCE_CRANE] >= 200 && ch->stance[STANCE_VIPER] >= 200)
      {
        send_to_char("mantis", ch);
        found = TRUE;
      }
      if (ch->stance[STANCE_BULL] >= 200 && ch->stance[STANCE_CRAB] >= 200)
      {
        if (found)
          send_to_char(", dragon", ch);
        else
        {
          send_to_char("dragon", ch);
          found = TRUE;
        }
      }
      if (ch->stance[STANCE_BULL] >= 200 && ch->stance[STANCE_VIPER] >= 200)
      {
        if (found)
          send_to_char(", tiger", ch);
        else
        {
          send_to_char("tiger", ch);
          found = TRUE;
        }
      }
      if (ch->stance[STANCE_CRANE] >= 200 && ch->stance[STANCE_MONGOOSE] >= 200)
      {
        if (found)
          send_to_char(", monkey", ch);
        else
        {
          send_to_char("monkey", ch);
          found = TRUE;
        }
      }
      if (ch->stance[STANCE_CRAB] >= 200 && ch->stance[STANCE_MONGOOSE] >= 200)
      {
        if (found)
          send_to_char(", swallow", ch);
        else
        {
          send_to_char("swallow", ch);
          found = TRUE;
        }
      }
      send_to_char("\n\r", ch);
    }
    return;
  }

  if (!str_cmp(arg, "none"))
    selection = STANCE_NONE;
  else if (!str_cmp(arg, "viper"))
    selection = STANCE_VIPER;
  else if (!str_cmp(arg, "crane"))
    selection = STANCE_CRANE;
  else if (!str_cmp(arg, "crab"))
    selection = STANCE_CRAB;
  else if (!str_cmp(arg, "mongoose"))
    selection = STANCE_MONGOOSE;
  else if (!str_cmp(arg, "bull"))
    selection = STANCE_BULL;
  else if (!str_cmp(arg, "mantis"))
    selection = STANCE_MANTIS;
  else if (!str_cmp(arg, "swallow"))
    selection = STANCE_SWALLOW;
  else if (!str_cmp(arg, "tiger"))
    selection = STANCE_TIGER;
  else if (!str_cmp(arg, "dragon"))
    selection = STANCE_DRAGON;
  else if (!str_cmp(arg, "monkey"))
    selection = STANCE_MONKEY;
  else if (!str_cmp(arg, "spirit"))
    selection = STANCE_SPIRIT;
  else
  {
    do_stance(ch, "");
    return;
  }

  if (ch->stance[0] == selection)
  {
    if (selection == STANCE_NONE)
      send_to_char("You are already unstanced.\n\r", ch);
    else
      send_to_char("You are already in that fighting stance.\n\r", ch);
    return;
  }

  WAIT_STATE(ch, 6);

  if (ch->stance[0] > 0)
  {
    send_to_char("You relax from your fighting stance.\n\r", ch);
    act("$n relaxes from $s fighting stance.", ch, NULL, NULL, TO_ROOM);
    ch->stance[0] = STANCE_NONE;

    if (selection != STANCE_NONE)
    {
      event = alloc_event();
      event->fun = &event_mobile_stance;
      event->type = EVENT_MOBILE_STANCING;
      event->argument = str_dup(arg);
      add_event_char(event, ch, 1 * PULSE_PER_SECOND);
    }

    return;
  }

  if (selection == STANCE_VIPER)
  {
    send_to_char("You arch your body into the #yviper#n fighting stance.\n\r", ch);
    act("$n arches $s body into the #yviper#n fighting stance.", ch, NULL, NULL, TO_ROOM);
  }
  else if (selection == STANCE_CRANE)
  {
    send_to_char("You swing your body into the #ycrane#n fighting stance.\n\r", ch);
    act("$n swings $s body into the #ycrane#n fighting stance.", ch, NULL, NULL, TO_ROOM);
  }
  else if (selection == STANCE_CRAB)
  {
    send_to_char("You squat down into the #ycrab#n fighting stance.\n\r", ch);
    act("$n squats down into the #ycrab#n fighting stance. ", ch, NULL, NULL, TO_ROOM);
  }
  else if (selection == STANCE_MONGOOSE)
  {
    send_to_char("You twist into the #ymongoose#n fighting stance.\n\r", ch);
    act("$n twists into the #ymongoose#n fighting stance. ", ch, NULL, NULL, TO_ROOM);
  }
  else if (selection == STANCE_BULL)
  {
    send_to_char("You hunch down into the #ybull#n fighting stance.\n\r", ch);
    act("$n hunches down into the #ybull#n fighting stance. ", ch, NULL, NULL, TO_ROOM);
  }
  else if (selection == STANCE_MANTIS)
  {
    if (ch->stance[STANCE_CRANE] < 200 || ch->stance[STANCE_VIPER] < 200)
    {
      send_to_char("First you must grandmaster #CCrane#n and #CViper#n.\n\r", ch);
      return;
    }
    send_to_char("You spin your body into the #ymantis#n fighting stance.\n\r", ch);
    act("$n spins $s body into the #ymantis#n fighting stance.", ch, NULL, NULL, TO_ROOM);
  }
  else if (selection == STANCE_DRAGON)
  {
    if (ch->stance[STANCE_BULL] < 200 || ch->stance[STANCE_CRAB] < 200)
    {
      send_to_char("First you must grandmaster #CBull#n and #CCrab#n.\n\r", ch);
      return;
    }
    send_to_char("You coil your body into the #ydragon#n fighting stance.\n\r", ch);
    act("$n coils $s body into the #ydragon#n fighting stance.", ch, NULL, NULL, TO_ROOM);
  }
  else if (selection == STANCE_TIGER)
  {
    if (ch->stance[STANCE_BULL] < 200 || ch->stance[STANCE_VIPER] < 200)
    {
      send_to_char("First you must grandmaster #CBull#n and #CViper#n.\n\r", ch);
      return;
    }
    send_to_char("You lunge into the #ytiger#n fighting stance.\n\r", ch);
    act("$n lunges into the #ytiger#n fighting stance.", ch, NULL, NULL, TO_ROOM);
  }
  else if (selection == STANCE_MONKEY)
  {
    if (ch->stance[STANCE_CRANE] < 200 || ch->stance[STANCE_MONGOOSE] < 200)
    {
      send_to_char("First you must grandmaster #CCrane#n and #CMongoose#n.\n\r", ch);
      return;
    }
    send_to_char("You rotate your body into the #ymonkey#n fighting stance.\n\r", ch);
    act("$n rotates $s body into the #ymonkey#n fighting stance.", ch, NULL, NULL, TO_ROOM);
  }
  else if (selection == STANCE_SWALLOW)
  {
    if (ch->stance[STANCE_CRAB] < 200 || ch->stance[STANCE_MONGOOSE] < 200)
    {
      send_to_char("First you must grandmaster #CCrab#n and #CMongoose#n.\n\r", ch);
      return;
    }
    send_to_char("You slide into the #yswallow#n fighting stance.\n\r", ch);
    act("$n slides into the #yswallow#n fighting stance.", ch, NULL, NULL, TO_ROOM);
  }
  else if (selection == STANCE_SPIRIT)
  {
    if (!IS_CLASS(ch, CLASS_FAE))
    {
      send_to_char("Only the faerie kin can use the spirit stance.\n\r", ch);
      return;
    }
    if (ch->stance[STANCE_DRAGON] < 200 || ch->stance[STANCE_SWALLOW] < 200 ||
        ch->stance[STANCE_MANTIS] < 200 || ch->stance[STANCE_TIGER] < 200 ||
        ch->stance[STANCE_MONKEY] < 200)
    {
      send_to_char("First you must grandmaster all your other stances.\n\r", ch);
      return;
    }
    send_to_char("You shift into the #yspirit#n fighting stance.\n\r", ch);
    act("$n shifts into the #yspirit#n fighting stance.", ch, NULL, NULL, TO_ROOM);
  }
  else
  {
    send_to_char("Something is wrong, please report this.\n\r", ch);
    return;
  }

  ch->stance[0] = selection;
  if (ch->fighting)
  {
    update_damcap(ch, ch->fighting);

    if (ch->fighting->fighting)
      update_damcap(ch->fighting, ch->fighting->fighting);
  }
}

void do_tackle(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Tackle whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (ch == victim)
  {
    send_to_char("You cannot tackle yourself.\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(victim, EVENT_MOBILE_FLEE)) == NULL)
  {
    send_to_char("You can only attempt to tackle someone who is fleeing.\n\r", ch);
    return;
  }

  act("$n runs over and tries to tackle $N.", ch, NULL, victim, TO_NOTVICT);
  act("$n runs over and tries to tackle you.", ch, NULL, victim, TO_VICT);
  act("You run over and try to tackle $N.", ch, NULL, victim, TO_CHAR);

  if (!check_dodge(ch, victim, DT_UNARMED))
  {
    dequeue_event(event, TRUE);
    one_hit(ch, victim, gsn_tackle, 0);
  }
  else
  {
    printf_to_char(ch, "You fail to tackle %s.\n\r", PERS(victim, ch));
    printf_to_char(ch, "%s fails to tackle you.\n\r", PERS(ch, victim));
  }

  WAIT_STATE(ch, 10);
}

void do_gensteal(CHAR_DATA *ch, char *argument)   
{
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *location;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  bool fortresssteal = FALSE;

  if (IS_NPC(ch))
    return;

  if (in_arena(ch))
  {
    send_to_char("You cannot steal generation while in the arena.\n\r",ch);
    return;
  }

  if (ragnarok)
  {
    send_to_char("Not while ragnarok is running.\n\r",ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Gensteal whom?\n\r",ch);
    return;
  }
  if ((victim=get_char_room(ch,arg))==NULL)
  {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }
  if (ch == victim)
  {
    send_to_char("You cannot gensteal yourself.\n\r",ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("You cannot gensteal monsters.\n\r",ch);
    return;
  }
  if (ch->class != victim->class)
  {
    send_to_char("You can only steal generation from players of your own class.\n\r",ch);
    return;
  }
  if (getMight(ch) < RANK_CADET)
  {
    send_to_char("You need to be a cadet or better.\n\r", ch);
    return;
  }
  if (getMight(victim) < RANK_CADET)
  {
    send_to_char("You can only gensteal Cadets or better.\n\r", ch);
    return;
  }
  if (!IS_SET(ch->extra, EXTRA_PKREADY))
  {
    send_to_char("You must be pkready before you can steal generation.\n\r", ch);
    return;
  }
  if (!IS_SET(victim->extra, EXTRA_PKREADY))
  {
    send_to_char("You cannot steal generation from players that are not pkready.\n\r", ch);
    return;
  }
  if (victim->position > 1)
  {
    send_to_char("He resists your attempt to gensteal.\n\r",ch);
    return;
  }
  if (ch->generation < victim->generation)
  {
    send_to_char("Sorry, you are already a higher generation than they are.\n\r",ch);
    return;
  }
  if (victim->generation > 7)
  {
    send_to_char("They are worthless.\n\r",ch);
    return;
  }
  if (ch->generation <= 1)
  {
    send_to_char("You are the highest possible generation.\n\r",ch);
    return;
  }

  if (is_safe(ch,victim))
    return;

  if (check_feed(ch, victim))
  {
    send_to_char("You can only gensteal from players that you have beaten yourself.\n\r", ch);
    return;
  }

  if (in_fortress(ch) && IS_SET(arena.status, ARENA_FORTRESS_DEATH))
  {
    fortresssteal = TRUE;

    open_fortress();
    if ((location = get_room_index(ROOM_VNUM_VICTORY)) == NULL) return;
    char_from_room(victim);
    char_to_room(victim, location, TRUE);
    char_from_room(ch);
    char_to_room(ch, location, TRUE);
  }
  else if (in_fortress(ch))
  {
    send_to_char("You're in The Forbidden Fortress.\n\r", ch);
    return;
  }
  act("Lightning flows through your body as you steal the powers of $N.\n\r", ch, NULL, victim, TO_CHAR);
  act("$n puts his hands on $N's head and lightning covers his body.\n\r", ch, NULL, victim, TO_NOTVICT);
  send_to_char("Your generation has been stolen.\n\r",victim);
  sprintf(buf, "#G%s #ohas beaten #L%s #oin combat, and has stolen their generation!#n",ch->name,
    victim->name);
  do_info(ch,buf);

  victim->generation++;
  victim->hit = 1;
  victim->fight_timer = 0;
  ch->generation--;

  /* after a fortress steal, we set fighttimer to 0 */
  if (!fortresssteal)
    ch->fight_timer += 5;
  else
    ch->fight_timer = 0;

  fix_weaponskill(ch, TRUE);
  fix_weaponskill(victim, TRUE);

  fix_magicskill(ch, TRUE);
  fix_magicskill(victim, TRUE);

  powerdown(victim);
  victim->level = 2;

  update_pos(victim);
  WAIT_STATE(ch, 12);

  muddata.pk_count_now[2]++;

  if ((location = get_room_index(ROOM_VNUM_CITYSAFE)) == NULL)
    return;

  char_from_room(victim);  
  char_to_room(victim, location, TRUE);
  victim->fight_timer = 0;

  REMOVE_BIT(victim->affected_by, AFF_WEBBED);
  REMOVE_BIT(victim->extra, TIED_UP);
  REMOVE_BIT(victim->extra, BLINDFOLDED);
  REMOVE_BIT(victim->extra, GAGGED);

  log_string("%s was genstolen by %s", victim->name, ch->name);
}

void do_combatswitch( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  
  argument = one_argument(argument,arg);
  if (IS_NPC(ch)) return;
  
  if (ch->fighting == NULL)
  {
    send_to_char("Ah, but your not really fighting anyone!\n\r",ch);
    return;
  }
  if ((victim=get_char_room(ch,arg))==NULL)
  {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }
  if (ch == victim)
  {
    send_to_char("How stupid are you?!?\n\r",ch);
    return;
  }
  if (ch->fighting == victim)
  {    
    send_to_char("Maybe you should just continue fighting them.\n\r",ch);
    return;
  }
  if (victim->fighting != ch)
  {
    send_to_char("You failed.\n\r",ch);
    return;
  }
  if (number_range(1,3) == 1)
  {      
    send_to_char("You failed.\n\r",ch);
    WAIT_STATE(ch, 12);
    return;
  }
  ch->fighting = victim;

  update_damcap(ch, victim);

  send_to_char("Hehe, bet they didn't expect that to happen...\n\r",ch);
  WAIT_STATE(ch,6);
}
