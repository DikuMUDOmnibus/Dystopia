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

/* local functions */
void drop_bloodrage              ( CHAR_DATA *ch );
bool event_room_shadowevil       ( EVENT_DATA *event );
bool event_room_tendrils         ( EVENT_DATA *event );
bool event_mobile_confused       ( EVENT_DATA *event );
bool event_player_bloodtheft     ( EVENT_DATA *event );

const struct evolve_entry shadow_evolve_table[] =
{
  { "shadowplane", 0, 0,
    EVOLVE_1, SHADOW_EVOLVE_SHADOWPLANE, EVOLVE_1, SHADOW_EVOLVE_CONFUSION,
    20000, 15000, 25000, 50000000, 5000
  },

  { "confusion", 0, 0,
    EVOLVE_1, SHADOW_EVOLVE_CONFUSION, EVOLVE_1, SHADOW_EVOLVE_SHADOWPLANE,
    20000, 20000, 15000, 50000000, 5000
  },

  { "tendrils of shadows", 0, 0,
    EVOLVE_1, SHADOW_EVOLVE_TENDRILS, EVOLVE_1, SHADOW_EVOLVE_FUMES,
    25000, 15000, 15000, 50000000, 5000
  },

  { "concussion fumes", 0, 0,
    EVOLVE_1, SHADOW_EVOLVE_FUMES, EVOLVE_1, SHADOW_EVOLVE_TENDRILS,
    20000, 20000, 20000, 50000000, 5000
  },

  { "skullduggery", 0, 0,
    EVOLVE_1, SHADOW_EVOLVE_SKULLDUGGERY, EVOLVE_1, SHADOW_EVOLVE_ASSASSIN,
    15000, 15000, 25000, 50000000, 5000
  },

  { "assassin", 0, 0,
    EVOLVE_1, SHADOW_EVOLVE_ASSASSIN, EVOLVE_1, SHADOW_EVOLVE_SKULLDUGGERY,
    25000, 15000, 15000, 50000000, 5000
  },

  { "planegrab", EVOLVE_1, SHADOW_EVOLVE_SHADOWPLANE,
    EVOLVE_2, SHADOW_EVOLVE_PLANEGRAB, EVOLVE_2, SHADOW_EVOLVE_PLANESHRED,
    25000, 20000, 35000, 200000000, 10000
  },

  { "planeshred", EVOLVE_1, SHADOW_EVOLVE_SHADOWPLANE,
    EVOLVE_2, SHADOW_EVOLVE_PLANESHRED, EVOLVE_2, SHADOW_EVOLVE_PLANEGRAB,
    20000, 25000, 35000, 200000000, 10000
  },

  { "mindboost", EVOLVE_1, SHADOW_EVOLVE_CONFUSION,
    EVOLVE_2, SHADOW_EVOLVE_MINDBOOST, EVOLVE_2, SHADOW_EVOLVE_MINDBLANK,
    20000, 30000, 40000, 200000000, 10000
  },

  { "mindblank", EVOLVE_1, SHADOW_EVOLVE_CONFUSION,
    EVOLVE_2, SHADOW_EVOLVE_MINDBLANK, EVOLVE_2, SHADOW_EVOLVE_MINDBOOST,
    20000, 40000, 30000, 200000000, 10000
  },

  { "bloodtheft", EVOLVE_1, SHADOW_EVOLVE_SKULLDUGGERY,
    EVOLVE_2, SHADOW_EVOLVE_BLOODTHEFT, EVOLVE_2, SHADOW_EVOLVE_RAZORPUNCH,
    25000, 40000, 25000, 200000000, 10000
  },

  { "razorpunch", EVOLVE_1, SHADOW_EVOLVE_SKULLDUGGERY,
    EVOLVE_2, SHADOW_EVOLVE_RAZORPUNCH, EVOLVE_2, SHADOW_EVOLVE_BLOODTHEFT,
    25000, 25000, 40000, 200000000, 10000
  },

  { "garotte", EVOLVE_1, SHADOW_EVOLVE_ASSASSIN,
    EVOLVE_2, SHADOW_EVOLVE_GAROTTE, EVOLVE_2, SHADOW_EVOLVE_AWARENESS,
    40000, 25000, 25000, 200000000, 10000
  },

  { "awareness", EVOLVE_1, SHADOW_EVOLVE_ASSASSIN,
    EVOLVE_2, SHADOW_EVOLVE_AWARENESS, EVOLVE_2, SHADOW_EVOLVE_GAROTTE,
    30000, 30000, 30000, 200000000, 10000
  },

  { "acid tendrils", EVOLVE_1, SHADOW_EVOLVE_TENDRILS,
    EVOLVE_2, SHADOW_EVOLVE_ACIDTENDRILS, EVOLVE_2, SHADOW_EVOLVE_BLURTENDRILS,
    40000, 30000, 30000, 200000000, 10000
  },

  { "blur tendrils", EVOLVE_1, SHADOW_EVOLVE_TENDRILS,
    EVOLVE_2, SHADOW_EVOLVE_BLURTENDRILS, EVOLVE_2, SHADOW_EVOLVE_ACIDTENDRILS,
    30000, 30000, 40000, 200000000, 10000
  },

  { "frostblast", EVOLVE_1, SHADOW_EVOLVE_FUMES,
    EVOLVE_2, SHADOW_EVOLVE_FROSTBLAST, EVOLVE_2, SHADOW_EVOLVE_MIRROR,
    30000, 35000, 35000, 200000000, 10000
  },

  { "mirror image", EVOLVE_1, SHADOW_EVOLVE_FUMES,
    EVOLVE_2, SHADOW_EVOLVE_MIRROR, EVOLVE_2, SHADOW_EVOLVE_FROSTBLAST,
    30000, 30000, 40000, 200000000, 10000
  },

  { "shadow witness", EVOLVE_2, SHADOW_EVOLVE_PLANEGRAB,
    EVOLVE_3, SHADOW_EVOLVE_WITNESS, EVOLVE_3, SHADOW_EVOLVE_FEINTS,
    50000, 40000, 55000, 500000000, 15000
  },    

  { "shadow feints", EVOLVE_2, SHADOW_EVOLVE_PLANEGRAB,
    EVOLVE_3, SHADOW_EVOLVE_FEINTS, EVOLVE_3, SHADOW_EVOLVE_WITNESS,
    55000, 45000, 45000, 500000000, 15000
  },

  { "touch of darkness", EVOLVE_2, SHADOW_EVOLVE_PLANESHRED,
    EVOLVE_3, SHADOW_EVOLVE_DTOUCH, EVOLVE_3, SHADOW_EVOLVE_POWERSHRED,
    45000, 50000, 50000, 500000000, 15000
  },

  { "power shred", EVOLVE_2, SHADOW_EVOLVE_PLANESHRED,
    EVOLVE_3, SHADOW_EVOLVE_POWERSHRED, EVOLVE_3, SHADOW_EVOLVE_DTOUCH,
    50000, 50000, 45000, 500000000, 15000
  },

  /* end of table */
  { "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

void shadow_commands(CHAR_DATA *ch)
{
  BUFFER *buf = buffer_new(MAX_STRING_LENGTH);
  char shadowattack[MAX_STRING_LENGTH]; int attcount = 0;
  char shadowpowers[MAX_STRING_LENGTH]; int powcount = 0;
  char shadowevolve[MAX_STRING_LENGTH]; int evocount = 0;
  char generic[MAX_STRING_LENGTH]; int gencount = 0;
  int cmd;

  bprintf(buf, "%s\n\r", get_dystopia_banner("    Powers    ", 76));

  for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
  {
    if (cmd_table[cmd].race != CLASS_SHADOW)
      continue;

    /* check to see if the player has actually learned the power */
    if (!can_use_command(ch, cmd))
      continue;

    switch(cmd_table[cmd].powertype)
    {
      default:
        bug("shadow_commands: cmd %d unknown powertype.", cmd);
        break;
      case 0:
        if (gencount == 0)
          sprintf(generic, " %-15s :", "Shadow Powers");
        strcat(generic, " ");
        strcat(generic, cmd_table[cmd].name);
        gencount++;
        break;
      case SHADOW_ATTACK:
        if (attcount == 0)
          sprintf(shadowattack, " %-15s :", "Shadow Attacks");
        strcat(shadowattack, " ");
        strcat(shadowattack, cmd_table[cmd].name);
        attcount++;
        break;
      case SHADOW_POWERS:
        if (powcount == 0)
          sprintf(shadowpowers, " %-15s :", "Shadowlearned");
        strcat(shadowpowers, " ");
        strcat(shadowpowers, cmd_table[cmd].name);
        powcount++;
        break;
      case EVOLVE_1:
      case EVOLVE_2:
      case EVOLVE_3:
        if (evocount == 0)
          sprintf(shadowevolve, " %-15s :", "Evolutions");
        strcat(shadowevolve, " ");
        strcat(shadowevolve, cmd_table[cmd].name);
        evocount++;
        break;
    }
  }

  if (gencount > 0)
    bprintf(buf, "%19.19s%s\n\r", generic, line_indent(&generic[18], 19, 75));
  if (powcount > 0)
    bprintf(buf, "%19.19s%s\n\r", shadowpowers, line_indent(&shadowpowers[18], 19, 75));
  if (attcount > 0)
    bprintf(buf, "%19.19s%s\n\r", shadowattack, line_indent(&shadowattack[18], 19, 75));
  if (evocount > 0)
    bprintf(buf, "%19.19s%s\n\r", shadowevolve, line_indent(&shadowevolve[18], 19, 75));

  bprintf(buf, "%s\n\r", get_dystopia_banner("", 76));

  send_to_char(buf->data, ch);
  buffer_free(buf);
}

void show_shadow_evolves(CHAR_DATA *ch, int base, EVOLVE_DATA *evolve, bool recursive)
{
  char buf[MAX_STRING_LENGTH];
  int i;
  bool found = FALSE;

  if (base == -1)
  {
    sprintf(buf, " #9%-20s %8s %6s %6s %10s %6s#n<BR>", "Evolve", "Hps", "Mana", "Move", "Exp", "Gold");
    strcat(evolve->error, buf);
    strcat(evolve->error, " #0-------------------------------------------------------------#n<BR>");
  }

  for (i = 0; shadow_evolve_table[i].name[0] != '\0'; i++)
  {
    int *reqfield = &ch->pcdata->powers[shadow_evolve_table[i].req_field];
    int *evolvefield = &ch->pcdata->powers[shadow_evolve_table[i].evolve_field];
    int *opposefield = &ch->pcdata->powers[shadow_evolve_table[i].oppose_field];

    /* got the opposing evolve ? */
    if (shadow_evolve_table[i].oppose_bit != 0 && IS_SET(*opposefield, shadow_evolve_table[i].oppose_bit)) continue;

    if (base != -1)
    {
      if (shadow_evolve_table[i].req_bit != shadow_evolve_table[base].evolve_bit) continue;
      if (shadow_evolve_table[i].req_field != shadow_evolve_table[base].evolve_field) continue;
    }
    else
    {
      if (shadow_evolve_table[i].req_bit != 0 && !IS_SET(*reqfield, shadow_evolve_table[i].req_bit)) continue;
    }

    /* add this evolve, then do the recursion dance */
    if (!IS_SET(*evolvefield, shadow_evolve_table[i].evolve_bit))
    {
      if (recursive)
        strcat(evolve->error, "  #c");
      else
        strcat(evolve->error, "#C");

      /* add this evolve, then do the recursion dance */
      sprintf(buf, " <SEND href=\"help '%s'\">%-20.20s</SEND> %s %5d %6d %6d %10d %6d<BR>",
        shadow_evolve_table[i].name,
        shadow_evolve_table[i].name,
        (recursive) ? "" : "  ",
        shadow_evolve_table[i].hps,
        shadow_evolve_table[i].mana,
        shadow_evolve_table[i].move,
        shadow_evolve_table[i].exp,
        shadow_evolve_table[i].gold);
      strcat(evolve->error, buf);

      found = TRUE;

      if (!recursive)
        show_shadow_evolves(ch, i, evolve, TRUE);
    }
  }

  if (base == -1 && !found)
  {
    sprintf(evolve->error, "You are unable to evolve.<BR>");
  }
}

void shadow_evolve(CHAR_DATA *ch, char *argument, EVOLVE_DATA *evolve)
{
  int i;

  if (argument[0] == '\0')
  {
    show_shadow_evolves(ch, -1, evolve, FALSE);
    return;
  }

  for (i = 0; shadow_evolve_table[i].name[0] != '\0'; i++)
  {
    int *reqfield = &ch->pcdata->powers[shadow_evolve_table[i].req_field];
    int *evolvefield = &ch->pcdata->powers[shadow_evolve_table[i].evolve_field];
    int *opposefield = &ch->pcdata->powers[shadow_evolve_table[i].oppose_field];

    if (IS_SET(*evolvefield, shadow_evolve_table[i].evolve_bit)) continue;
    if (shadow_evolve_table[i].oppose_bit != 0 && IS_SET(*opposefield, shadow_evolve_table[i].oppose_bit)) continue;
    if (shadow_evolve_table[i].req_bit != 0 && !IS_SET(*reqfield, shadow_evolve_table[i].req_bit)) continue;

    if (!str_cmp(argument, shadow_evolve_table[i].name))
      break;
  }

  if (shadow_evolve_table[i].name[0] == '\0')
  {
    sprintf(evolve->error, "There is no evolve by that name.<BR>");
    return;
  }

  /* set the evolve data */
  evolve->hps   =  shadow_evolve_table[i].hps;
  evolve->mana  =  shadow_evolve_table[i].mana;
  evolve->move  =  shadow_evolve_table[i].move;
  evolve->exp   =  shadow_evolve_table[i].exp;
  evolve->gold  =  shadow_evolve_table[i].gold;
  evolve->field =  &ch->pcdata->powers[shadow_evolve_table[i].evolve_field];
  evolve->bit   =  shadow_evolve_table[i].evolve_bit;
  evolve->valid =  TRUE;
}

bool event_player_witnessgrab(EVENT_DATA *event)
{
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_witnessgrab: no owner.", 0);
    return FALSE;
  }

  send_to_char("You feel a pulling sensation, and hear a slight pop.\n\r", ch);
  act("$n is torn trough the planebarrier by a wicked looking wraith.", ch, NULL, NULL, TO_ROOM);

  SET_BIT(ch->newbits, NEW_SHADOWPLANE);

  return FALSE;
}

bool event_player_witness(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  int cost = 400;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_witness: no owner.", 0);
    return FALSE;
  }

  if (ch->move < cost)
  {
    send_to_char("You lose control over the shadow witness.\n\r", ch);
    return FALSE;
  }
  modify_move(ch, -1 * cost);

  event = alloc_event();
  event->fun = &event_player_witness;
  event->type = EVENT_PLAYER_WITNESS;
  add_event_char(event, ch, 3 * PULSE_PER_SECOND);

  return FALSE;
}

void do_callwitness(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  int cost = 500;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_3], SHADOW_EVOLVE_WITNESS))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_WITNESS)) != NULL)
  {
    dequeue_event(event, TRUE);
    send_to_char("You release the shadow witness from its service.\n\r", ch);
    act("The wraith hovering around $n slides into the ground.", ch, NULL, NULL, TO_ROOM);
    return;
  }

  if (ch->pcdata->powers[SHADOW_POWER] < cost)
  {
    printf_to_char(ch, "You need %d more shadowpoints to bind the witness.\n\r", cost - ch->pcdata->powers[SHADOW_POWER]);
    return;
  }
  ch->pcdata->powers[SHADOW_POWER] -= cost;

  send_to_char("You bind the shadow witness to your service.\n\r", ch);
  act("$n binds a wraithlike creature to $s essence.", ch, NULL, NULL, TO_ROOM);

  event = alloc_event();
  event->fun = &event_player_witness;
  event->type = EVENT_PLAYER_WITNESS;
  add_event_char(event, ch, 3 * PULSE_PER_SECOND);
}

void do_dullcut(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (!IS_SET(ch->pcdata->powers[EVOLVE_3], SHADOW_EVOLVE_FEINTS) &&
      !IS_SET(ch->pcdata->powers[EVOLVE_3], SHADOW_EVOLVE_DTOUCH))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (!IS_SET(ch->newbits, NEW_SHADOWPLANE))
  {
    send_to_char("You must be in the shadowplane to perform this attack.\n\r", ch);
    return;
  }

  /* get victim */
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    if ((victim = ch->fighting) == NULL)
    {
      send_to_char("But you are not fighting anyone.\n\r", ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  /* check for bad targets */
  if (is_safe(ch, victim)) return;
  if (ch == victim)
  {
    send_to_char("You don't want to attack yourself.\n\r", ch);
    return;
  }

  /* deal damage and drop bloodrage */
  drop_bloodrage(ch);
  one_hit(ch, victim, gsn_dullcut, 1);

  if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DIRTTHROW_1))
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DULLCUT);
    send_to_char("The shadows flicker.\n\r", ch);
  }
  else if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_HTHRUST))
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DULLCUT);
    send_to_char("The shadows flicker.\n\r", ch);
  }
  else if (ch->pcdata->powers[SHADOW_COMBO] > 0)
  {
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    send_to_char("You feel the shadows subside.\n\r", ch);
  }
  else
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DULLCUT);
    send_to_char("The shadows form around you.\n\r", ch);
  }
  WAIT_STATE(ch, 6);
}

void do_hthrust(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_3], SHADOW_EVOLVE_FEINTS))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (!IS_SET(ch->newbits, NEW_SHADOWPLANE))
  {
    send_to_char("You must be in the shadowplane to perform this attack.\n\r", ch);
    return;
  }

  /* get victim */
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    if ((victim = ch->fighting) == NULL)
    {
      send_to_char("But you are not fighting anyone.\n\r", ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  /* check for bad targets */
  if (is_safe(ch, victim)) return;
  if (ch == victim)
  {
    send_to_char("You don't want to attack yourself.\n\r", ch);
    return;
  }

  /* deal damage and drop bloodrage */
  drop_bloodrage(ch);
  one_hit(ch, victim, gsn_hthrust, 1);

  if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_MOONSTRIKE_1) &&
      IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_MOONSTRIKE_2) &&
      IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_SHADOWTHRUST_1))
  {
    int sn;

    if (!IS_NPC(victim))
    {
      act("$n causes an implosion of shadowmatter to strike $N.", ch, NULL, victim, TO_NOTVICT);
      act("You cause an implosion of shadowmatter to strike $N.", ch, NULL, victim, TO_CHAR);
      act("$n causes an implosion of shadowmatter to strike you.", ch, NULL, victim, TO_VICT);

      if ((sn = skill_lookup("dispel magic")) > 0)
        (*skill_table[sn].spell_fun) (sn, (ch->spl[skill_table[sn].target] * 0.5), ch, victim);
      else
        bug("do_hthrust: bad spell.", 0);
    }
    else
    {
      send_to_char("Nothing happens.\n\r", ch);
    }

    ch->pcdata->powers[SHADOW_COMBO] = 0;
  }
  else if (ch->pcdata->powers[SHADOW_COMBO] > 0)
  {
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    send_to_char("You feel the shadows subside.\n\r", ch);
  }
  else
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_HTHRUST);
    send_to_char("The shadows form around you.\n\r", ch);
  }
  WAIT_STATE(ch, 6);
}

void do_planegrab(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim = NULL;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];
  int cost = 1000;
  bool fAll = FALSE, found = FALSE;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_PLANEGRAB))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (!IS_SET(ch->newbits, NEW_SHADOWPLANE))
  {
    send_to_char("You need to be in the shadowplane to grab others.\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Whom do you wish to grab into the shadowplane?\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "all") && IS_SET(ch->pcdata->powers[EVOLVE_3], SHADOW_EVOLVE_WITNESS))
    fAll = TRUE;

  if (!fAll)
  {
    if ((victim = get_char_room(ch, arg)) == NULL)
    {
      send_to_char("They are not here.\n\r", ch);
      return;
    }

    if (IS_SET(victim->newbits, NEW_SHADOWPLANE))
    {
      send_to_char("They are already in the shadowplane.\n\r", ch);
      return;
    }

    if (victim->position == POS_FIGHTING)
    {
      send_to_char("You cannot pull a fighting person through the veil.\n\r", ch);
      return;
    }

    /* reduce cost due to int/wis/con */
    cost = reduce_cost(ch, victim, cost, eMove);
  }

  if (ch->move < cost)
  {
    send_to_char("You do not have the stamina to pull anyone through the veil.\n\r", ch);
    return;
  }
  modify_move(ch, -1 * cost);

  if (fAll)
  {
    REMOVE_BIT(ch->newbits, NEW_SHADOWPLANE);

    pIter = AllocIterator(ch->in_room->people);
    while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (ch == victim) continue;
      if (IS_SET(victim->newbits, NEW_SHADOWPLANE)) continue;
      if (!can_see(ch, victim)) continue;
      if (is_safe(ch, victim)) continue;

      if (!IS_NPC(victim))
        ch->fight_timer += 3;

      SET_BIT(victim->newbits, NEW_SHADOWPLANE);

      act("You pull $N through the veil and into the plane of shadows.", ch, NULL, victim, TO_CHAR);
      act("$n pulls $N through a rift in space.", ch, NULL, victim, TO_NOTVICT);
      act("$n pulls you through a rift in space, and drops you in shadows.", ch, NULL, victim, TO_VICT);
      do_look(victim, "");

      if (IS_NPC(victim) || !IS_CLASS(victim, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_1], SHADOW_EVOLVE_SHADOWPLANE))
      {
        event            = alloc_event();
        event->fun       = &event_mobile_shadowgrabbed;
        event->type      = EVENT_MOBILE_SHADOWGRABBED;
        add_event_char(event, victim, 6 * PULSE_PER_SECOND);
      }

      found = TRUE;
    }
    SET_BIT(ch->newbits, NEW_SHADOWPLANE);

    if (!found)
      send_to_char("You where unable to pull anyone into the shadowplane.\n\r", ch);
  }
  else
  {
    REMOVE_BIT(ch->newbits, NEW_SHADOWPLANE);
    if (is_safe(ch, victim))
    {
      SET_BIT(ch->newbits, NEW_SHADOWPLANE);
      return;
    }
    SET_BIT(ch->newbits, NEW_SHADOWPLANE);
    SET_BIT(victim->newbits, NEW_SHADOWPLANE);

    act("You pull $N through the veil and into the plane of shadows.", ch, NULL, victim, TO_CHAR);
    act("$n pulls $N through a rift in space.", ch, NULL, victim, TO_NOTVICT);
    act("$n pulls you through a rift in space, and drops you in shadows.", ch, NULL, victim, TO_VICT);
    do_look(victim, "");

    if (IS_NPC(victim) || !IS_CLASS(victim, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_1], SHADOW_EVOLVE_SHADOWPLANE))
    {
      event            = alloc_event();
      event->fun       = &event_mobile_shadowgrabbed;
      event->type      = EVENT_MOBILE_SHADOWGRABBED;
      add_event_char(event, victim, 6 * PULSE_PER_SECOND);
    }

    if (!IS_NPC(victim))
      ch->fight_timer += 3;
  }
}

void do_supkeep(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (IS_SET(ch->newbits, NEW_SUPKEEP2))
  {
    REMOVE_BIT(ch->newbits, NEW_SUPKEEP2);
    send_to_char("You will keep your shadow powers active for 25 seconds.\n\r", ch);
  }
  else if (IS_SET(ch->newbits, NEW_SUPKEEP1))
  {
    REMOVE_BIT(ch->newbits, NEW_SUPKEEP1);
    SET_BIT(ch->newbits, NEW_SUPKEEP2);
    send_to_char("You will keep your shadow powers active for 75 seconds.\n\r", ch);
  }
  else
  {
    SET_BIT(ch->newbits, NEW_SUPKEEP1);
    send_to_char("You will keep your shadow powers active for 50 seconds.\n\r", ch);
  }
}

void do_planerift(CHAR_DATA *ch, char *argument)
{
  ROOM_INDEX_DATA *pRoom;
  CHAR_DATA *victim;
  int cost = 2000;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_3], SHADOW_EVOLVE_POWERSHRED))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((victim = ch->fighting) == NULL)
  {
    send_to_char("You are not fighting anyone.\n\r", ch);
    return;
  }

  if (victim->fighting != ch)
  {
    send_to_char("The person you are fighting is not fighting you.\n\r", ch);
    return;
  }

  if (ch->pcdata->powers[SHADOW_POWER] < cost)
  {
    printf_to_char(ch, "You need %d more shadow points before you can cause a rift.\n\r", cost - ch->pcdata->powers[SHADOW_POWER]);
    return;
  }
  ch->pcdata->powers[SHADOW_POWER] -= cost;

  act("$n tears a rift in the shadowplane, causing a temporal vortex to appear.", ch, NULL, NULL, TO_ALL);
  act("You tear a rift in the shadowplane, causing a temporal vortex to appear.", ch, NULL, NULL, TO_CHAR);

  if ((pRoom = get_rand_room_area(ch->in_room->area)) == NULL)
  {
    send_to_char("You fail to open a temporal rift in the shadowplane.\n\r", ch);
    return;
  }

  act("$n and $N vanishes into the shadowplane.", ch, NULL, victim, TO_NOTVICT);
  char_from_room(ch);
  char_from_room(victim);
  char_to_room(ch, pRoom, TRUE);
  char_to_room(victim, pRoom, TRUE);
  act("$n and $N is hurled into the room, covered in thick layers of shadows.", ch, NULL, victim, TO_NOTVICT);

  WAIT_STATE(ch, 8);
}

void do_planeshred(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int cost = 2000;
  int dam;
  bool toggleback = FALSE;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_PLANESHRED))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    if (!IS_SET(ch->newbits, NEW_SHADOWPLANE))
      send_to_char("Whom in the shadowplane do you wish to shred?\n\r", ch);
    else
      send_to_char("Whom do you wish to shred from the shadowplane?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (!IS_SET(ch->pcdata->powers[EVOLVE_3], SHADOW_EVOLVE_DTOUCH))
  {
    if (IS_SET(ch->newbits, NEW_SHADOWPLANE) && IS_SET(victim->newbits, NEW_SHADOWPLANE))
    {
      send_to_char("You cannot shred someone who is in the shadowplane.\n\r", ch);
      return;
    }
    if (!IS_SET(ch->newbits, NEW_SHADOWPLANE) && !IS_SET(victim->newbits, NEW_SHADOWPLANE))
    {
      send_to_char("You can only shred people in the shadowplane.\n\r", ch);
      return;
    }
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    send_to_char("You do not have the mystical might to penetrate the barrier of worlds.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * cost);

  dam = URANGE(500, victim->hit / 20, 1800);
  if (IS_SET(victim->affected_by, AFF_SANCTUARY))
    dam /= 2;

  if (IS_SET(ch->newbits, NEW_SHADOWPLANE))
    act("You tear a rift from the shadowplane and shred $N.", ch, NULL, victim, TO_CHAR);
  else
    act("You tear a rift into the shadowplane and shred $N.", ch, NULL, victim, TO_CHAR);

  if ((IS_SET(ch->newbits, NEW_SHADOWPLANE) && !IS_SET(victim->newbits, NEW_SHADOWPLANE)) ||
     (!IS_SET(ch->newbits, NEW_SHADOWPLANE) &&  IS_SET(victim->newbits, NEW_SHADOWPLANE)))
  {
    TOGGLE_BIT(ch->newbits, NEW_SHADOWPLANE);
    toggleback = TRUE;
  }

  if (is_safe(ch, victim))
    return;

  if (toggleback)
    TOGGLE_BIT(ch->newbits, NEW_SHADOWPLANE);

  act("$n tears a shred in the fabric of space and assaults you.", ch, NULL, victim, TO_VICT);
  act("$n tears a shred in the shadowplane and assaults $N.", ch, NULL, victim, TO_NOTVICT);

  if (!IS_SET(ch->pcdata->powers[EVOLVE_3], SHADOW_EVOLVE_DTOUCH))
    TOGGLE_BIT(ch->newbits, NEW_SHADOWPLANE);
  else
  {
    if (IS_SET(ch->newbits, NEW_SHADOWPLANE) && !IS_SET(victim->newbits, NEW_SHADOWPLANE))
    {
      if (number_percent() >= 25)
        SET_BIT(victim->newbits, NEW_SHADOWPLANE);
      else
        REMOVE_BIT(ch->newbits, NEW_SHADOWPLANE);
    }
    else if (!IS_SET(ch->newbits, NEW_SHADOWPLANE) && IS_SET(victim->newbits, NEW_SHADOWPLANE))
    {
      if (number_percent() >= 25)
        REMOVE_BIT(victim->newbits, NEW_SHADOWPLANE);
      else
        SET_BIT(ch->newbits, NEW_SHADOWPLANE);
    }
  }

  if (!IS_NPC(victim))
    ch->fight_timer += 5;

  if (victim->position != POS_FIGHTING && IS_SET(ch->pcdata->powers[EVOLVE_3], SHADOW_EVOLVE_POWERSHRED))
  {
    ROOM_INDEX_DATA *pRoom;

    if ((pRoom = get_rand_room_area(ch->in_room->area)) != NULL)
    {
      act("$n and $N vanishes into the shadowplane.", ch, NULL, victim, TO_NOTVICT);
      char_from_room(ch);
      char_to_room(ch, pRoom, TRUE);
      char_from_room(victim);
      char_to_room(victim, pRoom, TRUE);
      act("$n and $N is hurled into the room, covered in thick layers of shadows.", ch, NULL, victim, TO_NOTVICT);
    }
  }

  if (victim->dead == FALSE && victim->hit > 0)
    damage(ch, victim, NULL, dam, gsn_shadowthrust);
  if (victim->dead == FALSE && victim->hit > 0)
    damage(ch, victim, NULL, dam, gsn_shadowthrust);
  if (victim->dead == FALSE && victim->hit > 0)
    damage(ch, victim, NULL, dam, gsn_shadowthrust);
  if (victim->dead == FALSE && victim->hit > 0)
    damage(ch, victim, NULL, dam, gsn_shadowthrust);

  if (!IS_SET(ch->pcdata->powers[EVOLVE_3], SHADOW_EVOLVE_POWERSHRED))
    WAIT_STATE(ch, 8);
}

void do_shadowplane(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;
  int cost = 500;
  int sn;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_1], SHADOW_EVOLVE_SHADOWPLANE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((sn = skill_lookup("planebind")) > 0)
  {
    if (is_affected(ch, sn))
    {
      send_to_char("You cannot shift between the planes while planebound.\n\r", ch);
      return;
    }
  }

  if (ch->position == POS_FIGHTING && !IS_SET(ch->pcdata->powers[EVOLVE_3], SHADOW_EVOLVE_FEINTS))
  {
    send_to_char("No way!  You are still fighting!\n\r", ch);
    return;
  }

  if (ch->hit <= cost)
  {
    send_to_char("You are unable to cross the threshold.\n\r", ch);
    return;
  }
  modify_hps(ch, -1 * cost);

  if (!IS_SET(ch->newbits, NEW_SHADOWPLANE))
  {
    CHAR_DATA *gch;
    bool done = FALSE;

    if (ch->fight_timer > 0)
    {
      if (!IS_SET(ch->pcdata->powers[EVOLVE_3], SHADOW_EVOLVE_FEINTS))
      {
        send_to_char("You cannot shift into the shadowplane with a fighttimer.\n\r", ch);
        return;
      }
      if (ch->position != POS_FIGHTING)
      {
        send_to_char("You cannot shift into the shadowplane without fighting someone.\n\r", ch);
        return;
      }
      if (ch->fighting == NULL || IS_NPC(ch->fighting))
      {
        send_to_char("You need to be fighting another player to planeshift the people fighting.\n\r", ch);
        return;
      }
    }

    send_to_char("You fade into the plane of shadows.\n\r", ch);
    act("The shadows flicker and swallow up $n.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->newbits, NEW_SHADOWPLANE);
    do_look(ch, "auto");

    while (!done)
    {
      done = TRUE;

      pIter = AllocIterator(ch->in_room->people);
      while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
      {
        if (IS_SET(gch->newbits, NEW_SHADOWPLANE)) continue;
        if (gch->fighting == NULL) continue;

        if (IS_SET(gch->fighting->newbits, NEW_SHADOWPLANE))
        {
          SET_BIT(gch->newbits, NEW_SHADOWPLANE);
          send_to_char("You fade into the plane of shadows.\n\r", gch);
          act("The shadows flicker and swallow up $n.", gch, NULL, NULL, TO_ROOM);
          do_look(gch, "auto");
          done = FALSE;
        }
      }
    }

    return;
  }

  if (ch->position != POS_STANDING)
  {
    send_to_char("You cannot leave the shadowplane while fighting.\n\r", ch);
    return;
  }

  REMOVE_BIT(ch->newbits, NEW_SHADOWPLANE);
  send_to_char("You fade back into the real world.\n\r", ch);
  act("The shadows flicker and $n fades into existance.", ch ,NULL, NULL, TO_ROOM);
  do_look(ch, "auto");
}

void do_fumeblast(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int cost = 1000;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_1], SHADOW_EVOLVE_FUMES))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0' && ch->fighting == NULL)
  {
    send_to_char("Which mobile do you wish to strike with your fumes.\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    if ((victim = ch->fighting) == NULL)
    {
      send_to_char("You cannot locate that mobile.\n\r", ch);
      return;
    }
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eHit);

  if (ch->hit <= cost)
  {
    send_to_char("Your health is to poor.\n\r", ch);
    return;
  }
  modify_hps(ch, -1 * cost);

  act("You breath a cone of noxious fumes at $N.", ch, NULL, victim, TO_CHAR);
  act("$n breathes a cone of noxious fumes at you.", ch, NULL, victim, TO_VICT);
  act("$n breathes a cone of noxious fumes at $N.", ch, NULL, victim, TO_NOTVICT);

  if (IS_NPC(victim) && !IS_SET(victim->newbits, NEW_FUMES))
  {
    SET_BIT(victim->newbits, NEW_FUMES);
    act("$n gags and chokes.", victim, NULL, NULL, TO_ROOM);
    return;
  }

  act("$n seems unaffected.", victim, NULL, NULL, TO_ROOM);
  send_to_char("You are unaffected.\n\r", victim);
}

void do_mindboost(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int cost = 1000;
  int sn;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_MINDBOOST))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_MINDBOOST)) != NULL)
  {
    printf_to_char(ch, "You must wait %s before you can use this power again.\n\r", event_time_left(event));
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Whom do you wish to mindboost?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (ch->move < cost)
  {
    send_to_char("You do not have the stamina to do this.\n\r", ch);
    return;
  }
  modify_move(ch, -1 * cost);

  if ((sn = skill_lookup("mind boost")) <= 0)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }

  if (ch != victim)
  {
    act("$n waves $s hands at $N, mumbling an arcane phrase.", ch, NULL, victim, TO_NOTVICT);
    act("$n waves $s hands at you, mumbling an arcane phrase.", ch, NULL, victim, TO_VICT);
    act("You try to boost $N's mind.", ch, NULL, victim, TO_CHAR);
  }
  else
  {
    send_to_char("You try to boost your mind.\n\r", ch);
    act("$n mumbles something, eyes flaring.", ch, NULL, NULL, TO_ROOM);
  }

  (*skill_table[sn].spell_fun) (sn, 100, ch, victim);

  event             = alloc_event();
  event->fun        = &event_dummy;
  event->type       = EVENT_PLAYER_MINDBOOST;
  add_event_char(event, ch, 60 * PULSE_PER_SECOND);

  WAIT_STATE(ch, 8);
}

void do_bloodtheft(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  int cost = 500;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_BLOODTHEFT))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((victim = ch->fighting) == NULL)
  {
    send_to_char("You are not fighting anyone.\n\r", ch);
    return;
  }

  if (victim->hit <= 1000)
  {
    send_to_char("They do not have anything worth stealing.\n\r", ch);
    return;
  }

  if (event_isset_mobile(ch, EVENT_PLAYER_BLOODTHEFT))
  {
    send_to_char("You already have a stream of blood running through you.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need another %d mana before you can use this power.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("You open your mouth wide, and call for $N's blood.", ch, NULL, victim, TO_CHAR);
  act("Blood starts pouring from $S mouth into yours.", ch, NULL, victim, TO_CHAR);
  act("$n opens $s mouth wide, and gazes intently at $N.", ch, NULL, victim, TO_NOTVICT);
  act("A stream of blood bursts from $N's mouth and into $n's.", ch, NULL, victim, TO_NOTVICT);
  act("$n opens $s mouth wide, and gazes intenly at you.", ch, NULL, victim, TO_VICT);
  act("A stream of blood bursts from your mouth and into $n's.", ch, NULL, victim, TO_VICT);

  hurt_person(ch, victim, 1000);

  event = alloc_event();
  event->fun = &event_player_bloodtheft;
  event->type = EVENT_PLAYER_BLOODTHEFT;
  event->argument = str_dup("10");
  add_event_char(event, ch, 3 * PULSE_PER_SECOND / 4);

  WAIT_STATE(ch, 4);
}

bool event_player_bloodtheft(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  int count = (event->argument) ? atoi(event->argument) : 0;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_bloodtheft: no owner.", 0);
    return FALSE;
  }

  if (ch->hit <= 0)
    return FALSE;

  if (ch->hit >= ch->max_hit)
  {
    send_to_char("You have regained full health, the bloodstream leaves your body.\n\r", ch);
    return FALSE;
  }

  modify_hps(ch, 100);
  printf_to_char(ch, "You gain 100 hitpoints from the bloodstream inside you.\n\r");

  if (count-- > 0)
  {
    char buf[MAX_INPUT_LENGTH];

    sprintf(buf, "%d", count);

    event = alloc_event();
    event->fun = &event_player_bloodtheft;
    event->type = EVENT_PLAYER_BLOODTHEFT;
    event->argument = str_dup(buf);
    add_event_char(event, ch, 3 * PULSE_PER_SECOND / 4);
  }

  return FALSE;
}

void do_razorpunch(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int cost = 1500;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_RAZORPUNCH))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((victim = ch->fighting) == NULL)
  {
    send_to_char("You are not fighting anyone.\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_RAZORHANDS)) != NULL)
  {
    printf_to_char(ch, "You cannot use this power for another %s.\n\r", event_time_left(event));
    return;
  }

  if (get_eq_char(ch, WEAR_WIELD) && get_eq_char(ch, WEAR_HOLD))
  {
    send_to_char("You need a free hand to use this power.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMove);

  if (ch->move < cost)
  {
    printf_to_char(ch, "You need %d more move before you can use this power.\n\r", cost - ch->move);
    return;
  }
  modify_move(ch, -1 * cost);

  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_PLAYER_RAZORHANDS;
  add_event_char(event, ch, 20 * PULSE_PER_SECOND);

  act("$n shapes $s fist into a razorsharp object, and throws a punch at $N.", ch, NULL, victim, TO_NOTVICT);
  act("You shape your fist into a razorsharp object, and throws a punch at $N.", ch, NULL, victim, TO_CHAR);
  act("$n shapes $s fist into a razorsharp object, and throws a punch at you.", ch, NULL, victim, TO_VICT);

  if ((obj = get_eq_char(victim, number_range(0, MAX_WEAR - 1))) != NULL)
  {
    obj_from_char(obj);
    obj_to_char(obj, ch);
    act("$p is torn from $N by $n.", ch, obj, victim, TO_NOTVICT);
    act("$p is torn from you by $n.", ch, obj, victim, TO_VICT);
    act("You tear $p from $N.", ch, obj, victim, TO_CHAR);
  }
  else
  {
    act("Nothing happens.", ch, NULL, NULL, TO_ALL);
  }
}

void do_mindblank(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int cost = 1000;
  int sn;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_MINDBLANK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_MINDBLANK)) != NULL)
  {
    printf_to_char(ch, "You must wait %s before you can use this power again.\n\r", event_time_left(event));
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Whom do you wish to mindblank?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (ch == victim)
  {
    send_to_char("That's hardly a good idea.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana <  cost)
  {
    send_to_char("You do not have the mystical might to accomplish this.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("$n waves $s hands at $N, mumbling an arcane phrase.", ch, NULL, victim, TO_NOTVICT);
  act("$n waves $s hands at you, mumbling an arcane phrase.", ch, NULL, victim, TO_VICT);
  act("You try to blank $N's mind.", ch, NULL, victim, TO_CHAR);

  if ((sn = skill_lookup("mind blank")) <= 0)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }

  (*skill_table[sn].spell_fun) (sn, 100, ch, victim);

  event             = alloc_event();
  event->fun        = &event_dummy;
  event->type       = EVENT_PLAYER_MINDBLANK;
  add_event_char(event, ch, 30 * PULSE_PER_SECOND);

  WAIT_STATE(ch, 6);
}

void do_confusion(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim = NULL;
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];
  int cost = 1500;  

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_1], SHADOW_EVOLVE_CONFUSION))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0' && (victim = ch->fighting) == NULL)
  {
    send_to_char("Whom do you wish to confuse?\n\r", ch);
    return;
  }

  if (victim == NULL && (victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (IS_NPC(victim))
  {
    if (victim->quest_fun || victim->shop_fun || victim->level > 1500 ||
        victim->death_fun || IS_SET(victim->act, ACT_SENTINEL))
    {
      send_to_char("That mobile is immune to confusion.\n\r", ch);
      return;
    }
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_CONFUSE_WAIT)) != NULL)
  {
    printf_to_char(ch, "You cannot use this power for another %s.\n\r", event_time_left(event));
    return;
  }

  if (event_isset_mobile(victim, EVENT_MOBILE_CONFUSED))
  {
    send_to_char("They are already confused.\n\r", ch);
    return;
  }

  if (is_safe(ch, victim)) return;

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to cast confusion.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_PLAYER_CONFUSE_WAIT;
  add_event_char(event, ch, 15 * PULSE_PER_SECOND);

  if (number_range(1, 3) == 2 && ch != victim)
  {
    act("You stare deeply into $N's eyes, but nothing happens.", ch, NULL, victim, TO_CHAR);
    act("$n stares deeply into your eyes.", ch, NULL, victim, TO_VICT);
    act("$n stares deeply into $N's eyes, but nothing seems to happen.", ch, NULL, victim, TO_NOTVICT);

    WAIT_STATE(ch, 6);
    return;
  }

  event             = alloc_event();
  event->fun        = &event_mobile_confused;
  event->type       = EVENT_MOBILE_CONFUSED;
  add_event_char(event, victim, 5 * PULSE_PER_SECOND);

  if (ch != victim)
  {
    act("You stare deeply into $N's eyes, leaving behind a deeper madness.", ch, NULL, victim, TO_CHAR);
    act("$n stares deeply into your eyes - you feel slightly dual.", ch, NULL, victim, TO_VICT);
    act("$n stares deeply into $N's eyes, who seems to grow slightly pale.", ch, NULL, victim, TO_NOTVICT);
  }
  else
  {
    send_to_char("You feel funny.\n\r", ch);
    act("$n crosses $s eyes.", ch, NULL, NULL, TO_ROOM);
  }

  WAIT_STATE(ch, 8);
}

bool event_mobile_acidtendrils(EVENT_DATA *event)
{
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_acidtendrils: no owner.", 0);
    return FALSE;
  }

  /* tendrils gone? */
  if (!IS_SET(ch->newbits, NEW_TENDRIL1) || ch->hit < 1)
    return FALSE;

  /* they never really flex, so we'd better do it like this */
  if (IS_NPC(ch) && number_percent() > 90)
    return FALSE;

  act("$n screams in pain as the oozing tendrils covering $m burns through $s skin.", ch, NULL, NULL, TO_ROOM);
  send_to_char("The oozing tendrils covering you burns through your skin.\n\r", ch);

  modify_hps(ch, -1 * number_range(200, 500));
  update_pos(ch);

  event = alloc_event();
  event->fun = &event_mobile_acidtendrils;
  event->type = EVENT_MOBILE_ACIDTENDRILS;
  add_event_char(event, ch, 3 * PULSE_PER_SECOND);

  return FALSE;
}

void do_acidtendrils(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];
  int cost = 1500;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_ACIDTENDRILS))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Whom do you wish to use acid tendrils on?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (event_isset_mobile(victim, EVENT_MOBILE_ACIDTENDRILS))
  {
    send_to_char("They are already covered in oozing acid tendrils.\n\r", ch);
    return;
  }

  if (!IS_SET(victim->newbits, NEW_TENDRIL1))
  {
    send_to_char("They are not covered in any tendrils.\n\r", ch);
    return;
  }

  if (is_safe(ch, victim))
    return;

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to use this power.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("You turn the tendrils trapping $N into acid with a wave of your hand.", ch, NULL, victim, TO_CHAR);
  act("$n waves $s hand, and the tendrils covering you suddenly starts burning.", ch, NULL, victim, TO_VICT);
  act("$n waves $s hand, turning the tendrils trapping $N into acid.", ch, NULL, victim, TO_NOTVICT);

  event = alloc_event();
  event->fun = &event_mobile_acidtendrils;
  event->type = EVENT_MOBILE_ACIDTENDRILS;
  add_event_char(event, victim, 2 * PULSE_PER_SECOND);

  aggress(ch, victim);
}

void do_mirrorimage(CHAR_DATA *ch, char *argument)
{
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *victim, *mirror;
  EVENT_DATA *event;
  int cost = 2500;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_MIRROR))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_MIRRORIMAGE)) != NULL)
  {
    printf_to_char(ch, "You cannot use this power for another %s.\n\r", event_time_left(event));
    return;
  }

  if ((victim = ch->fighting) == NULL)
  {
    send_to_char("You are not fighting anyone.\n\r", ch);
    return;
  }

  if (victim->fighting != ch)
  {
    send_to_char("They are not looking at you.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to cast the mirror image spell.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  if ((pMobIndex = get_mob_index(MOB_VNUM_PROTOMOBILE)) == NULL)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }

  act("$n breaths a cone of mind altering fumes at $N.", ch, NULL, victim, TO_NOTVICT);
  act("You breath a cone of mind altering fumes at $N.", ch, NULL, victim, TO_CHAR);
  act("$n breaths a cone of mind altering fumes at you.", ch, NULL, victim, TO_VICT);

  mirror = create_mobile(pMobIndex);
  free_string(mirror->name);
  free_string(mirror->short_descr);
  free_string(mirror->long_descr);
  mirror->name = str_dup(victim->name);
  mirror->short_descr = (IS_NPC(victim)) ? str_dup(victim->short_descr) : str_dup(victim->name);
  mirror->long_descr = str_dup("A mirror image is standing here.\n\r");
  mirror->level = 100;
  mirror->max_hit = UMIN(8000, victim->hit);
  mirror->hit = mirror->max_hit;

  char_to_room(mirror, ch->in_room, TRUE);

  victim->fighting = mirror;
  set_fighting(mirror, victim);

  event = alloc_event();
  event->fun = &event_mobile_extract;
  event->type = EVENT_MOBILE_EXTRACT;
  add_event_char(event, mirror, 60 * PULSE_PER_SECOND);

  /* make sure it's not overused */
  event = alloc_event();
  event->type = EVENT_PLAYER_MIRRORIMAGE;
  event->fun = &event_dummy;
  add_event_char(event, ch, 60 * PULSE_PER_SECOND);

  WAIT_STATE(ch, 6);
}

void do_frostblast(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj = NULL;
  CHAR_DATA *victim = NULL;
  AFFECT_DATA paf;
  char arg[MAX_INPUT_LENGTH];
  int cost = 1000;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_FROSTBLAST))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    if (ch->position == POS_FIGHTING)
      send_to_char("Frostblast whom?\n\r", ch);
    else
      send_to_char("Frostblast what weapon?\n\r", ch);
    return;
  }

  if (ch->position == POS_FIGHTING)
  {
    if ((victim = get_char_room(ch, arg)) == NULL)
    {
      send_to_char("They are not here.\n\r", ch);
      return;
    }
  }
  else
  {
    if ((obj = get_obj_carry(ch, arg)) == NULL)
    {
      send_to_char("You do not have that weapon.\n\r", ch);
      return;
    }
  }

  if (ch->position == POS_FIGHTING)
  {
    if (is_safe(ch, victim))
      return;
  }
  else
  {
    if (obj->item_type != ITEM_WEAPON)
    {
      act("$p is not a weapon.", ch, obj, NULL, TO_CHAR);
      return;
    }
    if (object_is_affected(obj, OAFF_FROSTBITE))
    {
      act("$p is already frostbitten.", ch, obj, NULL, TO_CHAR);
      return;
    }
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to use this power.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  if (ch->position == POS_FIGHTING)
  {
    int sn;

    if ((sn = skill_lookup("frostbite")) > 0)
    {
      paf.type = sn;
      paf.duration = 15;
      paf.location = APPLY_HITROLL;
      paf.modifier = -10;
      paf.bitvector = 0;
      affect_to_char(victim, &paf);
    }
    else
      bug("do_frostbite: frostbite doesn't exist.", 0);

    one_hit(ch, victim, gsn_frostbite, 1);
  }
  else
  {
    act("$n breaths a cone of frost, covering $p in an icy layer.", ch, obj, NULL, TO_ROOM);
    act("You breath a cone of frost, covering $p in an icy layer.", ch, obj, NULL, TO_CHAR);

    paf.type = OAFF_FROSTBITE;
    paf.duration = 15;
    paf.location = APPLY_NONE;
    paf.modifier = 0;
    paf.bitvector = 0;
    affect_to_obj(obj, &paf);

    if (!event_isset_object(obj, EVENT_OBJECT_AFFECTS))
    {
      EVENT_DATA *event;

      event = alloc_event();
      event->fun = &event_object_affects;
      event->type = EVENT_OBJECT_AFFECTS;
      add_event_object(event, obj, 20 * PULSE_PER_SECOND);
    }
  }

  WAIT_STATE(ch, 12);
}

void do_blurtendrils(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];
  int cost = 600;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_BLURTENDRILS))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("What monster do you wish to use blur tendrils on?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (event_isset_mobile(victim, EVENT_MOBILE_BLURTENDRILS))
  {
    send_to_char("They are already covered in blurring tendrils.\n\r", ch);
    return;
  }

  if (!IS_SET(victim->newbits, NEW_TENDRIL1))
  { 
    send_to_char("They are not covered in any tendrils.\n\r", ch);
    return;
  }

  if (is_safe(ch, victim))
    return;

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eHit);

  if (ch->hit <= cost)
  {
    send_to_char("You cannot afford this sacrifice.\n\r", ch);
    return;
  }
  modify_hps(ch, -1 * cost);

  act("$n waves a hand at $N, causing the tendrils trapping $M to shift and blur.", ch, NULL, victim, TO_ROOM);
  act("You wave a hand at $N, causing the tendrils trapping $M to shift and blur.", ch, NULL, victim, TO_CHAR);

  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_MOBILE_BLURTENDRILS;
  add_event_char(event, victim, 15 * PULSE_PER_SECOND);

  aggress(ch, victim);
}

void do_tendrils(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  ROOM_INDEX_DATA *pRoom;
  int cost = 750;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || !IS_SET(ch->pcdata->powers[EVOLVE_1], SHADOW_EVOLVE_TENDRILS))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((pRoom = ch->in_room) == NULL)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }

  if (IS_SET(pRoom->room_flags, ROOM_SAFE))
  {
    send_to_char("You cannot summon the tendrils in this room.\n\r", ch);
    return;
  }

  if (event_isset_room(pRoom, EVENT_ROOM_TENDRILS))
  {
    send_to_char("This room is already covered in dark tendrils.\n\r", ch);
    return;
  }

  if (ch->hit <= cost)
  {
    send_to_char("You cannot afford this sacrifice.\n\r", ch);
    return;
  }
  modify_hps(ch, -1 * cost);

  act("$n cuts $s wrists, and pours blood on the floor.", ch, NULL, NULL, TO_ROOM);
  send_to_char("You cut your wrists, bleeding on the floor.\n\r", ch);

  event              = alloc_event();
  event->type        = EVENT_ROOM_TENDRILS;
  event->fun         = &event_room_tendrils;
  event->argument    = str_dup("1");
  add_event_room(event, pRoom, 3 * PULSE_PER_SECOND);
}

bool event_room_tendrils(EVENT_DATA *event)
{
  CHAR_DATA *gch;
  ROOM_INDEX_DATA *pRoom;
  ITERATOR *pIter;
  char buf[MAX_INPUT_LENGTH];
  int i;

  if ((pRoom = event->owner.room) == NULL)
  {
    bug("event_room_tendrils", 0);
    return FALSE;
  }

  if (event->argument == NULL || event->argument[0] == '\0')
  {
    bug("event_room_tendrils: no argument.", 0);
    return FALSE;
  }

  if ((i = atoi(event->argument)) <= 0 || i > 8)
  {
    if ((gch = (CHAR_DATA *) FirstInList(pRoom->people)) != NULL)
      act("The dark tendrils whither and die.", gch, NULL, NULL, TO_ALL);

    return FALSE;
  }

  if (i++ == 1)
  {
    if ((gch = (CHAR_DATA *) FirstInList(pRoom->people)) != NULL)
      act("Dark and shadowy tentdrils spew from the ground, entangling everyone.", gch, NULL, NULL, TO_ALL);
  }

  pIter = AllocIterator(pRoom->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_CLASS(gch, CLASS_SHADOW)) continue;
    if (IS_SET(gch->newbits, NEW_TENDRIL3)) continue;

    if (!IS_NPC(gch) && (gch->level < 3 || gch->level > 6)) continue;
    if (number_range(1, 3) != 2) continue;

    if (IS_SET(gch->newbits, NEW_TENDRIL1) && IS_SET(gch->newbits, NEW_TENDRIL2))
      SET_BIT(gch->newbits, NEW_TENDRIL3);
    else if (IS_SET(gch->newbits, NEW_TENDRIL1))
    {
      SET_BIT(gch->newbits, NEW_TENDRIL2);
      if (gch->fighting)
        update_damcap(gch, gch->fighting);
    }
    else
      SET_BIT(gch->newbits, NEW_TENDRIL1);

    act("$n is entangled in some shadow tendrils.", gch, NULL, NULL, TO_ROOM);
    send_to_char("You become intangled in the shadow tendrils.\n\r", gch);
  }

  sprintf(buf, "%d", i);

  event              = alloc_event();
  event->type        = EVENT_ROOM_TENDRILS;
  event->fun         = &event_room_tendrils;
  event->argument    = str_dup(buf);
  add_event_room(event, pRoom, 3 * PULSE_PER_SECOND);

  return FALSE;
}

bool event_mobile_shadowgrabbed(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  int sn = skill_lookup("planebind");
  bool bound = FALSE;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_shadowgrabbed: no owner.", 0);
    return FALSE;
  }

  /* no longer any reason to update this player */
  if (!IS_SET(ch->newbits, NEW_SHADOWPLANE))
    return FALSE;

  if (sn > 0 && is_affected(ch, sn))
    bound = TRUE;

  /* drain mana from the player - escape if possible */
  if (ch->fighting == NULL)
  {
    /* mobiles should have a chance to escape auto */
    if (IS_NPC(ch))
    {
      if (number_range(1, 5) == 3)
      {
        act("$n bursts free from the plane of shadows.", ch, NULL, NULL, TO_ROOM);
        REMOVE_BIT(ch->newbits, NEW_SHADOWPLANE);
        return FALSE;
      }
    }

    if (!bound && ch->position == POS_SLEEPING && number_range(1, 3) == 2)
    {
      ch->position = POS_STANDING;
      act("You wake from a horrible nightmare, finding yourself in the real world.", ch, NULL, NULL, TO_CHAR);
      act("$n fades into the real world and stands up.", ch, NULL, NULL, TO_ROOM);
      REMOVE_BIT(ch->newbits, NEW_SHADOWPLANE);
      return FALSE;
    }

    modify_mana(ch, -1 * (UMAX(ch->max_mana / 15, 1000)));
    if (ch->mana < 10) ch->mana = 10;
  }

  event            = alloc_event();
  event->fun       = &event_mobile_shadowgrabbed;
  event->type      = EVENT_MOBILE_SHADOWGRABBED;
  add_event_char(event, ch, 6 * PULSE_PER_SECOND);

  return FALSE;
}

bool event_mobile_confused(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  int sn, waitstate;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_confused: no owner.", 0);
    return FALSE;
  }

  switch(number_range(0, 7))
  {
    default:
      act("$n growls and clenches $s fists.", ch, NULL, NULL, TO_ROOM);
      send_to_char("You regain control of your own body.\n\r", ch);
      return FALSE;
      break;
    case 1:
      if (ch->stance[0] > 0)
      {
        switch(number_range(1, 5))
        {
          default:
            if (ch->stance[STANCE_TIGER] > 50)
              do_stance(ch, "tiger");
            else
              do_stance(ch, "viper");
            break;
          case 2:
            if (ch->stance[STANCE_MANTIS] > 50)
              do_stance(ch, "mantis");
            else
              do_stance(ch, "crane");
            break;
          case 3:
            if (ch->stance[STANCE_SWALLOW] > 50)
              do_stance(ch, "swallow");
            else
              do_stance(ch, "mongoose");
            break;
          case 4:
            if (ch->stance[STANCE_DRAGON] > 50)
              do_stance(ch, "dragon");
            else
              do_stance(ch, "bull");
            break;
          case 5:
            if (ch->stance[STANCE_MONKEY] > 50)
              do_stance(ch, "monkey");
            else
              do_stance(ch, "crab");
            break;
        }
      }
      break;
    case 2:
      act("$n stares blankly.", ch, NULL, NULL, TO_ROOM);
      send_to_char("You ponder your very existance for a moment.\n\r", ch);
      WAIT_STATE(ch, 12);
      break;
    case 3:
      do_say(ch, "GrrroOOooooWl!!!");
      waitstate = ch->wait;
      do_berserk(ch, "");
      ch->wait = UMAX(12, waitstate);
      break;
    case 4:
      if (get_eq_char(ch, WEAR_WIELD) == NULL && get_eq_char(ch, WEAR_HOLD) == NULL)
        break;

      do_say(ch, "I'd better put these weapons away before I hurt someone.");

      if (get_eq_char(ch, WEAR_SCABBARD_R) == NULL || get_eq_char(ch, WEAR_SCABBARD_L) == NULL)
      {
        do_sheath(ch, "both");
      }
      else
      {
        OBJ_DATA *obj;

        if ((obj = get_eq_char(ch, WEAR_WIELD)) != NULL)
        {
          obj_from_char(obj);
          obj_to_room(obj, ch->in_room);
          act("You drop $p on the ground.", ch, obj, NULL, TO_CHAR);
          act("$n drops $p on the ground.", ch, obj, NULL, TO_ROOM);
        }

        if ((obj = get_eq_char(ch, WEAR_HOLD)) != NULL)
        {
          obj_from_char(obj);
          obj_to_room(obj, ch->in_room);
          act("You drop $p on the ground.", ch, obj, NULL, TO_CHAR);
          act("$n drops $p on the ground.", ch, obj, NULL, TO_ROOM);
        }
      }

      break;
    case 5:
      if (ch->fighting)
        do_flee(ch, "");
      else if ((sn = skill_lookup("imp teleport")) > 0)
      {
        do_say(ch, "Time to explore the world.");
        (*skill_table[sn].spell_fun) (sn, 100, ch, ch);
      }
      break;
    case 6:
    case 7:
      break;
  }

  /* reusing function arguments - that's hardly a good practice */
  event             = alloc_event();
  event->fun        = &event_mobile_confused;
  event->type       = EVENT_MOBILE_CONFUSED;
  add_event_char(event, ch, 5 * PULSE_PER_SECOND);

  return FALSE;
}

bool event_room_shadowveil(EVENT_DATA *event)
{
  CHAR_DATA *gch;
  ROOM_INDEX_DATA *pRoom;
  EXIT_DATA *pExit;
  int door;

  if ((pRoom = event->owner.room) == NULL)
  {
    bug("event_room_shadowveil: no owner.", 0);
    return FALSE;
  }

  for (door = 0; door < 6; door++)
  {
    if ((pExit = pRoom->exit[door]) != NULL)
    {
      REMOVE_BIT(pExit->exit_info, EX_SHADOW_WALL);
    }
  }

  if ((gch = (CHAR_DATA *) FirstInList(pRoom->people)) != NULL)
    act("The veil of shadows fades.", gch, NULL, NULL, TO_ALL);

  return FALSE;
}

void do_moonstrike(CHAR_DATA *ch, char *argument) 
{
  OBJ_DATA *obj;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int i;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_ATTACK], NATTACK_MOONSTRIKE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    if ((victim = ch->fighting) == NULL)
    {
      send_to_char("But you are not fighting anyone.\n\r", ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (is_safe(ch, victim)) return;

  if (ch == victim)
  {
    send_to_char("You don't want to attack yourself.\n\r", ch);
    return;
  }

  drop_bloodrage(ch);
  one_hit(ch, victim, gsn_moonstrike, 1);
  if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DIRTTHROW_1) &&
      IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_SOULREAPER_1))
  {
    act("You summon a #9**#0shadow#9**#n storm to shred the life from $N.", ch, NULL, victim, TO_CHAR);
    act("$n conjures forth a #9**#0shadow#9**#n vortex, and you are caught in its shredding embrace.", ch, NULL, victim, TO_VICT);
    act("$n conjures forth a #9**#0shadow#9**#n storm to shred $N.", ch, NULL, victim, TO_NOTVICT);
    for (i = 0; i < MAX_WEAR; i++)
    {
      if ((obj = get_eq_char(victim, i)) != NULL)
      {
        if (number_range(1, 7) == 2) take_item(victim, obj);
      }
    }
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    WAIT_STATE(ch, 18);
    return;
  }
  else if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_WHIRL_1) &&
           IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DIRTTHROW_1) &&
           IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_WHIRL_2))
  {
    AFFECT_DATA af;

    one_hit(ch, victim, gsn_moonstrike, 1);
    one_hit(ch, victim, gsn_moonstrike, 1);
    do_say(ch, "Shadows heed my call, shroud this beast!");

    REMOVE_BIT(victim->act, PLR_HOLYLIGHT);
    af.type      = skill_lookup("blindness");
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;
    af.duration  = 60;
    af.bitvector = AFF_BLIND;
    affect_to_char(victim, &af);

    ch->pcdata->powers[SHADOW_COMBO] = 0;
    WAIT_STATE(ch, 18);
    return;
  }
  else if (IS_SET(ch->newbits, NEW_SHADOWPLANE) &&
           IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DIRTTHROW_1) &&
           IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DULLCUT))
  {
    act("Your moonstrike delivers a stunning blow to $N's head.", ch, NULL, victim, TO_CHAR);
    act("$n's moonstrike delivers a stunning blow to your head.", ch, NULL, victim, TO_VICT);
    act("$n's moonstrike delivers a stunning blow to $N's head.", ch, NULL, victim, TO_NOTVICT);
    SET_BIT(victim->newbits, NEW_FUMBLE);
    ch->pcdata->powers[SHADOW_COMBO] = 0;
  }
  else if (IS_SET(ch->newbits, NEW_SHADOWPLANE) &&
           IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_MOONSTRIKE_1) &&
           IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_SHADOWTHRUST_1))
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_MOONSTRIKE_2);
    send_to_char("The shadows flicker.\n\r", ch);
  }
  else if (ch->pcdata->powers[SHADOW_COMBO] > 0)
  {
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    send_to_char("You feel the shadows subside.\n\r", ch);
  }
  else
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_MOONSTRIKE_1);
    send_to_char("The shadows form around you.\n\r", ch);
  }
  WAIT_STATE(ch, 6);
}

void do_shadowthrust(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_ATTACK], NATTACK_SHADOWTHRUST))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    if ((victim = ch->fighting) == NULL)
    {
      send_to_char("But you are not fighting anyone.\n\r", ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (is_safe(ch, victim)) return;

  if (ch == victim)
  {
    send_to_char("You don't want to attack yourself.\n\r", ch);
    return;
  }

  drop_bloodrage(ch);
  one_hit(ch, victim, gsn_shadowthrust, 1);
  if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_CALTROPS_1) &&
      IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DIRTTHROW_1))
  {
    if (!IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_SHADOWFORM))
    {
      act("You become one with the shadows, transforming your body into shadow matter.", ch, NULL, NULL, TO_CHAR);
      act("$n becomes one with the shadows, transforming $s body into shadow matter.", ch, NULL, NULL, TO_ROOM);
      SET_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_SHADOWFORM);
    }
    else
    {
      act("You reform your body, and leave the shadow existance.", ch, NULL, NULL, TO_CHAR);
      act("$n reforms $s body, returning from the shadows.", ch, NULL, NULL, TO_ROOM);
      REMOVE_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_SHADOWFORM);
    }
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    WAIT_STATE(ch, 12);
    return;
  }
  else if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DIRTTHROW_1) &&
      IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_KNIFESPIN_1) &&
      IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_KNIFESPIN_2))
  {
    do_say(ch, "By the power of the shadows, you must die!");
    one_hit(ch, victim, gsn_shadowthrust, 1);
    one_hit(ch, victim, gsn_shadowthrust, 1);
    one_hit(ch, victim, gsn_shadowthrust, 1);
    one_hit(ch, victim, gsn_shadowthrust, 1);
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    WAIT_STATE(ch, 12); 
    return;
  }
  else if (IS_SET(ch->newbits, NEW_SHADOWPLANE) &&
           IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_MOONSTRIKE_1))
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_SHADOWTHRUST_1);
    send_to_char("The shadows flicker.\n\r", ch);
  }
  else if (ch->pcdata->powers[SHADOW_COMBO] > 0)
  {
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    send_to_char("You feel the shadows subside.\n\r", ch);
  }
  else
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_SHADOWTHRUST_1);
    send_to_char("The shadows form around you.\n\r", ch);
  }
  WAIT_STATE(ch, 6);
}

void do_dirtthrow(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
     
  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_ATTACK], NATTACK_DIRTTHROW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    if ((victim = ch->fighting) == NULL)
    {
      send_to_char("But you are not fighting anyone.\n\r", ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch, arg)) == NULL)
  { 
    send_to_char("They are not here.\n\r", ch); 
    return;
  }
  if (is_safe(ch, victim)) return;

  if (ch == victim)
  {
    send_to_char("You don't want to attack yourself.\n\r", ch);
    return;
  }

  drop_bloodrage(ch);
  one_hit(ch, victim, gsn_dirtthrow, 1);
  if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_CALTROPS_1))
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DIRTTHROW_1);
    send_to_char("The shadows flicker.\n\r", ch);
  }
  else if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_WHIRL_1))
  {   
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DIRTTHROW_1);
    send_to_char("The shadows flicker.\n\r", ch);
  }
  else if (ch->pcdata->powers[SHADOW_COMBO] > 0)
  {
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    send_to_char("You feel the shadows subside.\n\r", ch);
  }
  else
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DIRTTHROW_1);
    send_to_char("The shadows form around you.\n\r", ch);
  }
  WAIT_STATE(ch, 6);
}

void do_gutcutter(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
 
  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_ATTACK], NATTACK_GUTCUTTER))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }      
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    if ((victim = ch->fighting) == NULL)
    {
      send_to_char("But you are not fighting anyone.\n\r", ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (is_safe(ch, victim)) return;

  if (ch == victim)
  {
    send_to_char("You don't want to attack yourself.\n\r", ch);
    return;
  }

  drop_bloodrage(ch);
  one_hit(ch, victim, gsn_gutcutter, 1);
  if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_SHADOWTHRUST_1))
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_GUTCUTTER_1);
    send_to_char("The shadows form closer around you.\n\r", ch);
  }
  else if (ch->pcdata->powers[SHADOW_COMBO] > 0)
  {
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    send_to_char("You feel the shadows subside.\n\r", ch);
  }
  else
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DIRTTHROW_1);
    send_to_char("The shadows form around you.\n\r", ch);
  }
  WAIT_STATE(ch, 6);
  return;
}

void do_soulreaper(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_ATTACK], NATTACK_SOULREAPER))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    if ((victim = ch->fighting) == NULL)
    {
      send_to_char("But you are not fighting anyone.\n\r", ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (is_safe(ch, victim)) return;

  if (ch == victim)
  {
    send_to_char("You don't want to attack yourself.\n\r", ch);
    return;
  }

  drop_bloodrage(ch);
  one_hit(ch, victim, gsn_soulreaper, 1);
  if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_SHADOWTHRUST_1) &&
      IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_GUTCUTTER_1))
  {
    if (!IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLUR))
    {
      act("You shroud yourself in a blanket of shadows.", ch, NULL, NULL, TO_CHAR);
      act("$n shrouds $mself in a blanket of shadows.", ch, NULL, NULL, TO_ROOM);
      SET_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLUR);
    }
    else
      send_to_char("You are already blurred in shadows.\n\r", ch);
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    WAIT_STATE(ch, 12);
    return;
  }
  else if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DIRTTHROW_1))
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_SOULREAPER_1);
    send_to_char("The shadows form around you, ready to strike.\n\r", ch);
  }
  else if (ch->pcdata->powers[SHADOW_COMBO] > 0)
  {
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    send_to_char("You feel the shadows subside.\n\r", ch);
  }
  else
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_SOULREAPER_1);
    send_to_char("The shadows form around you.\n\r", ch);
  }
  WAIT_STATE(ch, 6);
}

void do_knifespin(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
 
  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_ATTACK], NATTACK_KNIFESPIN))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    if ((victim = ch->fighting) == NULL)
    { 
      send_to_char("But you are not fighting anyone.\n\r", ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (is_safe(ch, victim)) return;

  if (ch == victim)
  {
    send_to_char("You don't want to attack yourself.\n\r", ch);
    return;
  }

  drop_bloodrage(ch);
  one_hit(ch, victim, gsn_knifespin, 1);
  if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_CALTROPS_1))
  {
    if (!IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_KNIFESHIELD))
    {
      act("You throw the knife into the air and a barrier of knives form around you.", ch, NULL, NULL, TO_CHAR);
      act("$n throws a knife into the air, and a barrier of knives surround $m.", ch, NULL, NULL, TO_ROOM);
      SET_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_KNIFESHIELD);
    }
    else
      send_to_char("You already have a set of flying knives around you.\n\r", ch);
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    WAIT_STATE(ch, 6);
    return;
  }
  else if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_WAKASASHISLICE_1) &&
           IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_WAKASASHISLICE_2))
  {
    act("You throw back your head and howl in glee as you enter the bloodrage.", ch, NULL, NULL, TO_CHAR);
    act("$n throws back $s head and howl in glee as $e enters a bloodrage.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLOODRAGE);
    if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_SHADOWFORM))
    {
      REMOVE_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_SHADOWFORM);
      send_to_char("You return from the shadows.\n\r", ch);
    }
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    WAIT_STATE(ch, 12);
    return;
  }
  else if (IS_SET(ch->newbits, NEW_SHADOWPLANE) &&
           IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DULLCUT) &&
           IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_HTHRUST))
  {
    act("$n tears a rift in the shadowplane, showering $N with knifes.", ch, NULL, victim, TO_NOTVICT);
    act("You tear a rift in the shadowplane, showering $N with knifes.", ch, NULL, victim, TO_CHAR);
    act("$n tears a rift in the shadowplane, showering you with knifes.", ch, NULL, victim, TO_VICT);
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    one_hit(ch, victim, gsn_knifespin, 1);
    one_hit(ch, victim, gsn_knifespin, 1);
  }
  else if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DIRTTHROW_1) &&
           IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_KNIFESPIN_1))
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_KNIFESPIN_2);
    send_to_char("The shadows grow more substantial.\n\r", ch);
  }
  else if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DIRTTHROW_1))
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_KNIFESPIN_1);
    send_to_char("The shadows grow more substantial.\n\r", ch);
  }
  else if (ch->pcdata->powers[SHADOW_COMBO] > 0)
  {
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    send_to_char("You feel the shadows subside.\n\r", ch);
  }
  else
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_KNIFESPIN_1);
    send_to_char("The shadows form around you.\n\r", ch);
  }
  WAIT_STATE(ch, 6);
}

void do_whirl(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (get_eq_char(ch, WEAR_MASTERY) == NULL)
  {
    send_to_char("You need to wear your mastery blades.\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    if ((victim = ch->fighting) == NULL)
    {
      send_to_char("But you are not fighting anyone.\n\r", ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (is_safe(ch, victim)) return;

  if (ch == victim)
  {
    send_to_char("You don't want to attack yourself.\n\r", ch);
    return;
  }

  drop_bloodrage(ch);
  one_hit(ch, victim, gsn_whirl, 1);
  if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_DIRTTHROW_1) &&
      IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_WHIRL_1))
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_WHIRL_2);
    send_to_char("The shadows grow more substantial.\n\r", ch);
  }
  else if (ch->pcdata->powers[SHADOW_COMBO] > 0)
  {
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    send_to_char("You feel the shadows subside.\n\r", ch);
  }
  else
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_WHIRL_1);
    send_to_char("The shadows form around you.\n\r", ch);
  }
  WAIT_STATE(ch, 6);
}

void do_wakasashislice(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
    
  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_ATTACK], NATTACK_WAKASASHISLICE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    if ((victim = ch->fighting) == NULL)
    {
      send_to_char("But you are not fighting anyone.\n\r", ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (is_safe(ch, victim)) return;

  if (ch == victim)
  {
    send_to_char("You don't want to attack yourself.\n\r", ch);
    return;
  }

  drop_bloodrage(ch);
  one_hit(ch, victim, gsn_wakasashislice, 1);
  if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_KNIFESPIN_1))
  {
    do_say(ch, "Wakasashi!!");
    one_hit(ch, victim, gsn_wakasashislice, 1);
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    WAIT_STATE(ch, 6); 
    return;
  }
  else if (IS_SET(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_WAKASASHISLICE_1))
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_WAKASASHISLICE_2);
    send_to_char("The shadows form around you, ready to strike.\n\r", ch);
  }
  else if (ch->pcdata->powers[SHADOW_COMBO] > 0)
  {
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    send_to_char("You feel the shadows subside.\n\r", ch);
  }
  else
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_WAKASASHISLICE_1);
    send_to_char("The shadows form around you.\n\r", ch);
  }
  WAIT_STATE(ch, 6);
}

void do_caltrops(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_ATTACK], NATTACK_CALTROPS))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    if ((victim = ch->fighting) == NULL)
    {
      send_to_char("But you are not fighting anyone.\n\r", ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (ch == victim)
  {
    send_to_char("You don't want to attack yourself.\n\r", ch);
    return;
  }

  if (is_safe(ch, victim))
    return;

  drop_bloodrage(ch);
  one_hit(ch, victim, gsn_caltrops, 1);
  if (ch->pcdata->powers[SHADOW_COMBO] > 0)
  {
    ch->pcdata->powers[SHADOW_COMBO] = 0;
    send_to_char("You feel the shadows subside.\n\r", ch);
  }
  else
  {
    SET_BIT(ch->pcdata->powers[SHADOW_COMBO], NCOMBO_CALTROPS_1);
    send_to_char("The shadows form around you.\n\r", ch);
  }
  WAIT_STATE(ch, 6);
}

void do_shadowlearn(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  BUFFER *buf;
  int i, cost, power;
  bool experience = TRUE;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  buf = buffer_new(MAX_STRING_LENGTH);
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    bprintf(buf, "                    #R{}#y***#R{} #0Shadow Powers #R{}#y***#R{}#n\n\n\r");

    bprintf(buf, "              #yMartial Skill #0[#R");
    for (i = 0; i < ch->pcdata->powers[SHADOW_MARTIAL]; i++)
      bprintf(buf, "*");
    if (i < 25)
      bprintf(buf, "#n");
    for ( ; i < 25; i++)
      bprintf(buf, "*");

    bprintf(buf, "#0]#n\n\r\n\r");
    bprintf(buf, "   #RSilentwalk #0[#y%5d#0]   #RVanish       #0[#y%5d#0]   #RAura Sight     #0[#y%5d#0]#n\n\r",
      (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_SILENTWALK)) ? 0 : 10000,
      (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_HIDE)) ? 0 : 25000,
      (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_AURA)) ? 0 : 12500);
    bprintf(buf, "   #RTruesight  #0[#y%5d#0]   #RDemonic Pact #0[#y%5d#0]   #RSpirit Soaring #0[#y%5d#0]#n\n\r",
      (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_SIGHT)) ? 0 : 10000,
      (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_TPACT)) ? 0 : 50000,
      (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_SPIRIT)) ? 0 : 25000);
    bprintf(buf, "   #RNight Pact #0[#y%5d#0]   #RScry         #0[#y%5d#0]   #RShield         #0[#y%5d#0]#n\n\r",
      (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_DPACT)) ? 0 : 50000,
      (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_SCRY)) ? 0 : 25000,
      (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_SHIELD)) ? 0 : 25000);
    bprintf(buf, "   #RSoulseeker #0[#y%5d#0]   #RAssassinate  #0[#y%5d#0]   #RBloodenhance   #0[#y%5d#0]#n\n\r",
      (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_SOULSEEKERS)) ? 0 : 25000,
      (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_ASSASSINATE)) ? 0 : 25000,
      (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_BLOOD)) ? 0 : 15000);
    bprintf(buf, "   #RShadowedge #0[#y%5d#0]   #RShadowportal #0[#y%5d#0]   #RShadowveil     #0[#y%5d#0]#n\n\n\r",
      (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_EDGE)) ? 0 : 15000,
      (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_PORTAL)) ? 0 : 50000,
      (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_VEIL)) ? 0 : 40000);
    bprintf(buf, "                #0Shadow Attacks (500K exp to learn)#n\n\r\n\r");
    bprintf(buf, "      #RMoonstrike    Shadowthrust    Dirtthrow        Gutcutter#n\n\r");
    bprintf(buf, "      #RSoulreaper    Knifespin       Wakasashislice   Caltrops#n\n\n\r");
    bprintf(buf, "    #RShadow Points   #y%d#n\n\r", ch->pcdata->powers[SHADOW_POWER]);
    send_to_char(buf->data, ch);
    buffer_free(buf);
    return;
  }
  if (!str_cmp(arg, "martial"))
  {
    if (ch->pcdata->powers[SHADOW_MARTIAL] >= 25)
    {
      send_to_char("You are already a master of martial combat.\n\r", ch);
      buffer_free(buf);
      return;
    }
    if (ch->practice < (ch->pcdata->powers[SHADOW_MARTIAL] + 1) * 15)
    {
      bprintf(buf, "You need %d more primal to gain this martial level.\n\r",
        (ch->pcdata->powers[SHADOW_MARTIAL] + 1) * 15 - ch->practice);
      send_to_char(buf->data, ch);
      buffer_free(buf);
      return;
    }
    ch->pcdata->powers[SHADOW_MARTIAL]++;
    ch->practice -= ch->pcdata->powers[SHADOW_MARTIAL] * 15;
    send_to_char("Ok.\n\r", ch);
    buffer_free(buf);
    return;
  }
  else if (!str_cmp(arg, "silentwalk"))
  {
    power = NSHADOWS_SILENTWALK;
    cost = 10000;
    experience = FALSE;
  }
  else if (!str_cmp(arg, "shadowportal"))
  {
    power = NSHADOWS_PORTAL;
    cost = 50000;
    experience = FALSE;
  }
  else if (!str_cmp(arg, "shadowveil"))
  {
    power = NSHADOWS_VEIL;
    cost = 40000;
    experience = FALSE;
  }
  else if (!str_cmp(arg, "spirit") || !str_cmp(arg, "soaring"))
  {
    power = NSHADOWS_SPIRIT;
    cost = 25000;
    experience = FALSE;
  }
  else if (!str_cmp(arg, "aura") || !str_cmp(arg, "sight"))
  {
    power = NSHADOWS_AURA;
    cost = 12500;
    experience = FALSE;
  }
  else if (!str_cmp(arg, "vanish"))
  {
    power = NSHADOWS_HIDE;
    cost = 25000;
    experience = FALSE;
  }
  else if (!str_cmp(arg, "truesight"))
  {
    power = NSHADOWS_SIGHT;
    cost = 10000;
    experience = FALSE;
  }
  else if (!str_cmp(arg, "demonic"))
  {
    power = NSHADOWS_TPACT;
    cost = 50000;
    experience = FALSE;
  }
  else if (!str_cmp(arg, "shadowedge"))
  {
    power = NSHADOWS_EDGE;
    cost = 15000;
    experience = FALSE;
  }
  else if (!str_cmp(arg, "bloodenhance"))
  {
    power = NSHADOWS_BLOOD;
    cost = 15000;
    experience = FALSE;
  }
  else if (!str_cmp(arg, "night"))
  {
    power = NSHADOWS_DPACT;
    cost = 50000;
    experience = FALSE;
  }
  else if (!str_cmp(arg, "scry"))
  {
    power = NSHADOWS_SCRY;
    cost = 25000;
    experience = FALSE;
  }
  else if (!str_cmp(arg, "shield"))
  {
    power = NSHADOWS_SHIELD;
    cost = 25000;
    experience = FALSE;
  }
  else if (!str_cmp(arg, "soulseeker"))
  {
    power = NSHADOWS_SOULSEEKERS;
    cost = 25000;
    experience = FALSE;
  }
  else if (!str_cmp(arg, "assassinate"))
  {
    power = NSHADOWS_ASSASSINATE;
    cost = 25000;
    experience = FALSE;
  }
  else if (!str_cmp(arg, "moonstrike"))
  {
    power = NATTACK_MOONSTRIKE;
    cost = 500000;
  }
  else if (!str_cmp(arg, "shadowthrust"))
  {
    power = NATTACK_SHADOWTHRUST;
    cost = 500000;
  }
  else if (!str_cmp(arg, "dirtthrow"))
  {
    power = NATTACK_DIRTTHROW;
    cost = 500000;
  }
  else if (!str_cmp(arg, "gutcutter"))
  {
    power = NATTACK_GUTCUTTER;
    cost = 500000;
  }
  else if (!str_cmp(arg, "soulreaper"))
  {
    power = NATTACK_SOULREAPER;
    cost = 500000;
  }
  else if (!str_cmp(arg, "knifespin"))
  {
    power = NATTACK_KNIFESPIN;
    cost = 500000;
  }
  else if (!str_cmp(arg, "wakasashislice"))
  {
    power = NATTACK_WAKASASHISLICE;
    cost = 500000;
  }
  else if (!str_cmp(arg, "caltrops"))
  {
    power = NATTACK_CALTROPS;
    cost = 500000;
  }
  else
  {
    do_shadowlearn(ch, "");
    buffer_free(buf);
    return;
  }

  /* take the cost and give the skill */
  if (!experience)
  {
    if (IS_SET(ch->pcdata->powers[SHADOW_POWERS], power))
    {
      send_to_char("You already have that power.\n\r", ch);
      buffer_free(buf);
      return;
    }
    if (ch->pcdata->powers[SHADOW_POWER] < cost)
    {
      send_to_char("You don't have enough shadow points to buy this power.\n\r", ch);
      buffer_free(buf);
      return;
    }
    ch->pcdata->powers[SHADOW_POWER] -= cost;
    SET_BIT(ch->pcdata->powers[SHADOW_POWERS], power);
  }
  else
  {
    if (IS_SET(ch->pcdata->powers[SHADOW_ATTACK], power))
    {
      send_to_char("You already know that attack style.\n\r", ch);
      buffer_free(buf);
      return;
    }
    if (ch->exp < cost)
    {
      bprintf(buf, "You don't have the %d exp.\n\r", cost);
      send_to_char(buf->data, ch);
      buffer_free(buf);
      return;
    }
    ch->exp -= cost;
    SET_BIT(ch->pcdata->powers[SHADOW_ATTACK], power);
  }
  send_to_char("Ok.\n\r", ch);
  buffer_free(buf);
}

void do_soulseek(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *chroom;
  ROOM_INDEX_DATA *victimroom;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_SOULSEEKERS))
  {
    send_to_char("You don't have the soulseeker power yet.\n\r", ch);
    return;
  }
  if (strlen(ch->pcdata->soultarget) < 3)
  {
    send_to_char("But your not targetting anyone.\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[SHADOW_SOULAMMO] < 1)
  {
    send_to_char("You have lost your lock on their soul.\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[SHADOW_POWER] < 500)
  {
    send_to_char("You don't have enough class points.\n\r", ch);
    return;
  }
  if ((victim = get_char_world(ch, ch->pcdata->soultarget)) == NULL)
  {
    send_to_char("You cannot find them.\n\r", ch);
    return;
  }
  if ((victimroom = victim->in_room) == NULL)
  {
    send_to_char("They are hiding from you.\n\r", ch);
    return;
  }

  if (victimroom->area != ch->in_room->area)
  {
    send_to_char("They have left the area.\n\r", ch);
    return;
  }

  chroom = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, victimroom, FALSE);
  one_hit(ch, victim, gsn_soulseeker, 1);
  one_hit(ch, victim, gsn_soulseeker, 1);
  one_hit(ch, victim, gsn_soulseeker, 1);
  char_from_room(ch);
  char_to_room(ch, chroom, FALSE);
  ch->pcdata->powers[SHADOW_POWER] -= 500;
  WAIT_STATE(ch, 12);
}

void do_soultarget(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  
  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_SOULSEEKERS))
  {  
    send_to_char("You don't have the soulseeker power yet.\n\r", ch);
    return;
  }
  if ((victim = ch->fighting) == NULL)
  {
    send_to_char("You are not fighting anyone.\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("NPC's have no soul to target.\n\r", ch);
    return;
  }
  free_string(ch->pcdata->soultarget);
  ch->pcdata->soultarget = str_dup(victim->name);
  ch->pcdata->powers[SHADOW_SOULAMMO] = 5;
  send_to_char("You get a lock on their soul, they cannot escape your wrath now.\n\r", ch);
  send_to_char("You feel a dark presence in the very core of your being.\n\r", victim);
  WAIT_STATE(ch, 6);
}

void drop_bloodrage(CHAR_DATA *ch)
{
  if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLOODRAGE))
  {
    send_to_char("Your bloodrage subsides.\n\r", ch);
    REMOVE_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLOODRAGE);
  }
}

void do_shadowtalk(CHAR_DATA *ch, char *argument)
{
  int class = ch->class;

  if (IS_NPC(ch) || (!IS_IMMORTAL(ch) && !IS_CLASS(ch, CLASS_SHADOW)))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  ch->class = CLASS_SHADOW;
  talk_channel(ch, argument, CHANNEL_CLASS, CC_SHADOW, "shadow sign");
  ch->class = class;
}

void do_spirits(CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;
  int sn;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_SPIRIT))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if ((sn = skill_lookup("soaring spirit")) <= 0)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }
  if (is_affected(ch, sn))
  {
    send_to_char("Your spirits are already soaring.\n\r", ch);
    return;
  }

  af.type      = sn;
  af.duration  = number_range(20, 40);
  af.location  = APPLY_HITROLL;
  af.modifier  = ch->pcdata->powers[SHADOW_MARTIAL] * 8;
  af.bitvector = 0;
  affect_to_char(ch, &af);
  af.location  = APPLY_DAMROLL;
  affect_to_char(ch, &af);

  act("$n's eyes shimmers for an instance.", ch, NULL, NULL, TO_ROOM);
  send_to_char("You are possessed by a soaring spirit.\n\r", ch);
}

void do_vanish(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) && !IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (IS_CLASS(ch, CLASS_SHADOW) && !IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_HIDE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (IS_CLASS(ch, CLASS_FAE) && ch->pcdata->powers[DISC_FAE_ARCANE] < 4)
  {
    send_to_char("You need level 4 discipline in arcane.\n\r", ch);
    return;
  }

  if (IS_SET(ch->act, PLR_HIDE))
  {
    REMOVE_BIT(ch->act, PLR_HIDE);
    send_to_char("You slowly fade into existance.\n\r", ch);
    act("$n slowly fades into existance.", ch, NULL, NULL, TO_ROOM);
  }
  else
  {
    if (has_timer(ch)) return;
    send_to_char("You slowly fade out of existance.\n\r", ch);
    act("$n slowly fades out of existance.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->act, PLR_HIDE);
  }
}

void do_assassinate(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance = 5, maxlvl = 1000;
  bool assassin = FALSE;
  bool shadowplane = FALSE;

  argument = one_argument( argument, arg );

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_ASSASSINATE))
  {
    send_to_char("You don't have that power.\n\r", ch);
    return;
  }
  if (arg[0] == '\0')
  {
    send_to_char("Assassinate whom?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }
  if (victim->position == POS_SLEEPING) chance *= 2;
  if (victim == ch)
  {
    send_to_char("How can you assassinate yourself?\n\r", ch);
    return;
  }

  if (!IS_NPC(victim) && IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_AWARENESS))
  {
    act("You fail to sneak up on $N, $M catches you in the attempt.", ch, NULL, victim, TO_CHAR);
    act("$n tries to assassinate you, but you catch $m in the attempt.", ch, NULL, victim, TO_VICT);
    return;
  }

  if (IS_SET(ch->newbits, NEW_SHADOWPLANE) && !IS_SET(victim->newbits, NEW_SHADOWPLANE))
  {
    shadowplane = TRUE;
    REMOVE_BIT(ch->newbits, NEW_SHADOWPLANE);
  }

  if (is_safe(ch,victim))
  {
    if (shadowplane)
      SET_BIT(ch->newbits, NEW_SHADOWPLANE);
    return;
  }
  if (victim->fighting)
  {
    send_to_char("You can't assassinate a fighting person.\n\r", ch);
    if (shadowplane)
      SET_BIT(ch->newbits, NEW_SHADOWPLANE);
    return;
  }

  /* chance to assassinate a hurt victim is 0% */
  if (victim->hit < victim->max_hit)
  {
    chance = 0;
  }

  if (IS_SET(ch->pcdata->powers[EVOLVE_1], SHADOW_EVOLVE_ASSASSIN))
  {
    assassin = TRUE;
    maxlvl += 125;
  }

  if (victim->level > maxlvl)
  {
    send_to_char("They are to powerful for you.\n\r", ch);
    if (shadowplane)
      SET_BIT(ch->newbits, NEW_SHADOWPLANE);
    return;
  }
  if (!IS_AFFECTED(ch, AFF_SNEAK) && !shadowplane)
  {
    send_to_char("You must sneak (or enter shadowplane) before you can assassinate!\n\r", ch);
    return;
  }

  WAIT_STATE(ch, (assassin) ? 8 : 12);

  if (assassin)
  {
    chance *= 3;
    chance /= 2;
  }

  if (IS_SET(ch->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_GAROTTE))
  {
    act("Someone lays a garotte around your neck and squeezes hard.", ch, NULL, victim, TO_VICT);
    act("Someone lays a garotte around $N's neck and squeezes hard.", ch, NULL, victim, TO_NOTVICT);
    act("You lay a garotte around $N's neck and squeezes hard.", ch, NULL, victim, TO_CHAR);

    one_hit(ch, victim, gsn_garotte, 0);
    one_hit(ch, victim, gsn_garotte, 0);

    if (victim->dead)
      return;
  }

  if (shadowplane)
  {
    act("$n jumps from the shadows and tries to assassinate you.", ch, NULL, victim, TO_VICT);
    act("$n jumps from the shadows and tries to assassinate $N.", ch, NULL, victim, TO_NOTVICT);
    act("You jump from the shadows and try to assassinate $N.", ch, NULL, victim, TO_CHAR);
  }
  else
  {
    act("$n appears from nowhere and tries to assassinate you.", ch, NULL, victim, TO_VICT);
    act("$n appears from nowhere and tries to assassinate $N.", ch, NULL, victim, TO_NOTVICT);
    act("You try to assassinate $N.", ch, NULL, victim, TO_CHAR);
  }

  if (number_percent() < chance)
  {
    if (!IS_NPC(victim))
    {
      one_hit(ch, victim, gsn_backstab, 1);
      one_hit(ch, victim, gsn_backstab, 1);
      one_hit(ch, victim, gsn_backstab, 1);
      one_hit(ch, victim, gsn_backstab, 1);
      one_hit(ch, victim, gsn_backstab, 1);
    }
    else
    {
      victim->hit = 1;
      one_hit(ch, victim, gsn_backstab, 1);
      return;
    }
  }
  else
  {
    one_hit(ch, victim, gsn_backstab, 1);
    one_hit(ch, victim, gsn_backstab, 1);
  }

  if (!IS_NPC(victim))
    ch->fight_timer += 3;

  if (assassin && victim->dead == FALSE && victim->hit > 0)
    multi_hit(ch, victim, 1);
}

void do_aurasight(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_AURA))
  {
    send_to_char("You don't have the aura sight power yet.\n\r", ch);
    return;
  }

  TOGGLE_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_AURASIGHT);
  if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_AURASIGHT))
  {
    send_to_char("You enable your aura sight.\n\r", ch);
  }
  else
  {
    send_to_char("You disable your aura sight.\n\r", ch);
  }
}

void do_shadowedge(CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA paf;
  OBJ_DATA *obj;
  char arg[MAX_INPUT_LENGTH];
  int cost = 50;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_EDGE))
  {
    send_to_char("You don't have that power.\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Which weapon's edge do you wish to coat with a layer of shadows?\n\r", ch);
    return;
  }
  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You don't have that weapon.\n\r", ch);
    return;
  }
  if (obj->item_type != ITEM_WEAPON)
  {
    send_to_char("That is not a weapon.\n\r", ch);
    return;
  }
  if (obj->value[0] / 1000 > 0)
  {
    send_to_char("That weapon already has an affect.\n\r", ch);
    return;
  }
  if (getGold(ch) < cost)
  {
    printf_to_char(ch, "You need %d more goldcrowns to use this power.\n\r", cost - getGold(ch));
    return;
  }
  setGold(ch, -1 * cost);

  act("$n coats $p's edge in a layer of shadows.", ch, obj, NULL, TO_ROOM);
  act("You coat $p's edge in a layer of shadows.", ch, obj, NULL, TO_CHAR);
  SET_BIT(obj->extra_flags, ITEM_SHADOWEDGE);

  /* enhanced damroll due to sharpness */
  paf.type           = 0;
  paf.duration       = -1;
  paf.location       = APPLY_DAMROLL;
  paf.modifier       = number_range(5, 15);
  paf.bitvector      = 0;
  affect_to_obj(obj, &paf);

  /* much lower dam-range due to more fragile weapon */
  obj->value[1] = UMAX(1, (19 * obj->value[1]) / 20);
  obj->value[2] = UMAX(1, (19 * obj->value[2]) / 20);

  /* set vorpal flag */
  if (obj->value[0] >= 0)
    obj->value[0] += 15000;
  else
    obj->value[0] = 15000;
}

void do_bloodenhance(CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA paf;
  OBJ_DATA *obj;
  char arg[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  AFFECT_DATA *af;
  int damroll = 0;
  bool bHit = FALSE;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_BLOOD))
  {
    send_to_char("You don't have that power.\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Which weapon's edge do you wish to coat with a layer of shadows?\n\r", ch);
    return;
  }
  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You don't have that weapon.\n\r", ch);
    return;
  }
  if (obj->item_type != ITEM_WEAPON)
  {
    send_to_char("That is not a weapon.\n\r", ch);
    return;
  }
  if (IS_SET(obj->extra_flags, ITEM_BLOODENHANCE))
  {
    send_to_char("This weapon has already been coated with blood.\n\r", ch);
    return;
  }

  pIter = AllocIterator(obj->pIndexData->affected);
  while ((af = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (af->location == APPLY_HITROLL)
    {
      damroll += af->modifier;
    }
  }

  pIter = AllocIterator(obj->affected);
  while ((af = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (af->location == APPLY_HITROLL)
    {
      damroll += af->modifier;
      if (!bHit) af->modifier = af->modifier - 10;
      bHit = TRUE;
    }
  }
  if (!bHit)
  {
    paf.type           = 0;
    paf.duration       = -1;
    paf.location       = APPLY_HITROLL;
    paf.modifier       = -10;
    paf.bitvector      = 0;
    affect_to_obj(obj, &paf);
  }
  damroll = UMIN(2 * damroll, 20);

  paf.type           = 0;
  paf.duration       = -1;
  paf.location       = APPLY_DAMROLL;
  paf.modifier       = damroll;
  paf.bitvector      = 0;
  affect_to_obj(obj, &paf);

  /* lower average a tad */
  obj->value[1]--;
  obj->value[2]--;
  SET_BIT(obj->extra_flags, ITEM_BLOODENHANCE);

  act("$n coats $p in a layer of blood.", ch, obj, NULL, TO_ROOM);
  act("You coat $p in a layer of blood.", ch, obj, NULL, TO_CHAR);
}

void do_shadowportal(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  const int cost = 1500;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_PORTAL))
  {
    send_to_char("You don't have that power.\n\r", ch);
    return;
  }
  if (event_isset_mobile(ch, EVENT_PLAYER_SHADOWPORTAL_WAIT))
  {
    send_to_char("You can only use this power once every minute.\n\r", ch);
    return;
  }
  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to call a shadowportal.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("$n opens a vortex of whirling shadows.", ch, NULL, NULL, TO_ROOM);
  act("You open a vortex of whirling shadows.", ch, NULL, NULL, TO_CHAR);

  /* enqueue delayed shadow portal affect */
  event              =  alloc_event();
  event->fun         =  &event_player_shadowportal;
  event->type        =  EVENT_PLAYER_SHADOWPORTAL;
  add_event_char(event, ch, 2 * PULSE_PER_SECOND);

  /* enqueue delayd wait */
  event              =  alloc_event();
  event->fun         =  &event_dummy;
  event->type        =  EVENT_PLAYER_SHADOWPORTAL_WAIT;
  add_event_char(event, ch, 60 * PULSE_PER_SECOND);
}

bool event_player_shadowportal(EVENT_DATA *event)
{
  ROOM_INDEX_DATA *pRoom;
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_shadowportal: no owner.", 0);
    return FALSE;
  }

  /* only 66% chance when fighting */
  if (ch->fighting && number_range(1, 3) == 2)
  {
    send_to_char("The pull of the shadow vortex is not strong enough.\n\r", ch);
    act("The whirling vortex of shadows fades away.", ch, NULL, NULL, TO_ALL);
    return FALSE;
  }

  if (IS_AFFECTED(ch, AFF_CURSE) && number_range(1, 3) == 2)
  {
    send_to_char("The pull of the shadow vortex is not strong enough.\n\r", ch);
    act("The whirling vortex of shadows fades away.", ch, NULL, NULL, TO_ALL);
    return FALSE;
  }

  /* will not work when webbed */
  if (IS_AFFECTED(ch, AFF_WEBBED))
  {
    send_to_char("The shadow vortex is not strong enough to pull you from the webbing.\n\r", ch);
    act("The whirling vortex of shadows fades away.", ch, NULL, NULL, TO_ALL);
    return FALSE;
  }

  if (IS_SET(ch->newbits, NEW_TENDRIL1))
  {
    send_to_char("The shadow vortex is not strong enough to pull you from the tendrils.\n\r", ch);
    act("The whirling vortex of shadows fades away.", ch, NULL, NULL, TO_ALL);
    return FALSE;
  }

  if (ch->in_room == NULL || ch->in_room->area == NULL)
    return FALSE;

  if ((pRoom = get_rand_room_area(ch->in_room->area)) == NULL)
  {
    act("The whirling vortex of shadows fades away.", ch, NULL, NULL, TO_ALL);
    return FALSE;
  }

  /* fade out messages */
  act("You are sucked into the shadow vortex.", ch, NULL, NULL, TO_CHAR);
  act("$n is sucked into the shadow vortex, which closes behind $m.", ch, NULL, NULL, TO_ROOM);

  /* move character */
  if (ch->fighting)
    stop_fighting(ch, TRUE);
  char_from_room(ch);
  char_to_room(ch, pRoom, TRUE);

  /* fade in messages */
  act("You are thrown from the shadows.", ch, NULL, NULL, TO_CHAR);
  act("A whirling vortex of shadows spews forth $n.", ch, NULL, NULL, TO_ROOM);

  do_look(ch, "auto");
  WAIT_STATE(ch, 18);

  /* make sure the event is dequeued */
  return FALSE;
}

void do_shadowveil(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  EXIT_DATA *pExit;
  int cost = 2500;
  int door;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW) || ch->in_room == NULL)
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_VEIL))
  {
    send_to_char("You don't have that power.\n\r", ch);
    return;
  }
  if (ch->move < cost)
  {
    printf_to_char(ch, "You need %d more move to create a veil of shadows.\n\r", cost - ch->move);
    return;
  }
  modify_move(ch, -1 * cost);

  act("You create a veil of shadows, covering this room.", ch, NULL, NULL, TO_CHAR);
  act("$n creates a veil of shadows, covering the entire room.", ch, NULL, NULL, TO_ROOM);

  for (door = 0; door < 6; door++)
  {
    if ((pExit = ch->in_room->exit[door]) != NULL)
    {
      SET_BIT(pExit->exit_info, EX_SHADOW_WALL);
    }
  }

  /* make an event to remove these exit flags */
  event               =  alloc_event();
  event->type         =  EVENT_ROOM_SHADOWVEIL;
  event->fun          =  &event_room_shadowveil;
  add_event_room(event, ch->in_room, 30 * PULSE_PER_SECOND);

  WAIT_STATE(ch, 12);
}

void update_shadow(CHAR_DATA *ch)
{
  if (ch->position != POS_FIGHTING)
  {
    if (!IS_SET(ch->act, PLR_FREEZE) /* arena fix */
     && ((IS_SET(ch->newbits, NEW_SUPKEEP2) && ++ch->pcdata->powers[SHADOW_COMBATTICK] > 18)
     || (IS_SET(ch->newbits, NEW_SUPKEEP1) && ++ch->pcdata->powers[SHADOW_COMBATTICK] > 12)
     || (!IS_SET(ch->newbits, NEW_SUPKEEP1) && !IS_SET(ch->newbits, NEW_SUPKEEP2) && ++ch->pcdata->powers[SHADOW_COMBATTICK] > 6)))
    {
      /* reset counters and any active combos */
      ch->pcdata->powers[SHADOW_COMBO] = 0;
      ch->pcdata->powers[SHADOW_COMBATTICK] = 0;

      /* let any active powers fade */
      if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_KNIFESHIELD))
      {
        REMOVE_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_KNIFESHIELD);
        send_to_char("Your knife barrier fades from lack of use.\n\r", ch);
      }
      if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLUR))
      {
        REMOVE_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLUR);
        send_to_char("The shroud of shadows fade for lack of combat.\n\r", ch);
      }
      if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLOODRAGE))
      {
        REMOVE_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLOODRAGE);
        send_to_char("Your bloodrage subsides.\n\r", ch);
      }
      if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_SHADOWFORM))
      {
        REMOVE_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_SHADOWFORM);
        send_to_char("You regain your normal form.\n\r", ch);
      }
    }
  }
  else
  {
    /* reset non-fighting counter */
    ch->pcdata->powers[SHADOW_COMBATTICK] = 0;

    /* pay upkeep cost for all powers */
    if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLUR))
    {
      if (ch->pcdata->powers[SHADOW_POWER] < 75)
      {
        send_to_char("Your blurring magic fades due to lack of shadow points.\n\r", ch);
        REMOVE_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLUR);
      }
      else ch->pcdata->powers[SHADOW_POWER] -= 75;
    }
    if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLOODRAGE))
    {
      if (ch->pcdata->powers[SHADOW_POWER] < 150)
      {
        send_to_char("Your bloodrage fades due to lack of shadow points.\n\r", ch);  
        REMOVE_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLOODRAGE);   
      }  
      else ch->pcdata->powers[SHADOW_POWER] -= 150;
    }
    if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_SHADOWFORM))
    {
      if (ch->pcdata->powers[SHADOW_POWER] < 150)
      {
        send_to_char("Your shadowform fades due to lack of shadow points.\n\r", ch);
        REMOVE_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_SHADOWFORM);
      }
      else ch->pcdata->powers[SHADOW_POWER] -= 150;
    }
    if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_KNIFESHIELD))
    {
      if (ch->pcdata->powers[SHADOW_POWER] < 50)
      {
        send_to_char("Your knifeshield fades due to lack of shadow points.\n\r", ch);
        REMOVE_BIT(ch->pcdata->powers[SHADOW_BITS], NPOWER_KNIFESHIELD);
      }
      else ch->pcdata->powers[SHADOW_POWER] -= 50;
    }
  }
      
  /* count down on any soulammo (for soultarget) */
  if (ch->pcdata->powers[SHADOW_SOULAMMO] > 0) ch->pcdata->powers[SHADOW_SOULAMMO]--;
        
  /* regenerate body and limbs */
  if (ch->hit != ch->max_hit)
    regen_hps(ch, 2);
  if (ch->mana != ch->max_mana && !IS_SET(ch->act, PLR_HIDE))
    regen_mana(ch, 2);
  if (ch->move != ch->max_move && !IS_SET(ch->newbits, NEW_SHADOWPLANE))
    regen_move(ch, 2);

  if (IS_SET(ch->act, PLR_HIDE))
  {
    int cost = 250;

    ch->pcdata->powers[SHADOW_VANISH_STRESS]++;

    cost *= 1 + ch->pcdata->powers[SHADOW_VANISH_STRESS] / 10;

    if (ch->mana < cost)
    {
      REMOVE_BIT(ch->act, PLR_HIDE);
      act("$n slowly fades into existance.", ch, NULL, NULL, TO_ROOM);
      send_to_char("You slowly fade into existance.\n\r", ch);
    }
    else
    {
      modify_mana(ch, -1 * cost);
    }
  }
  else
  {
    if (ch->pcdata->powers[SHADOW_VANISH_STRESS] > 0)
      ch->pcdata->powers[SHADOW_VANISH_STRESS]--;
  }

  if (IS_SET(ch->newbits, NEW_SHADOWPLANE))
  {
    int cost = 250;

    if (ch->move < cost)
    {
      REMOVE_BIT(ch->newbits, NEW_SHADOWPLANE);
      send_to_char("You fade back into the real world.\n\r", ch);
      act("The shadows flicker and $n fades into existance.", ch ,NULL, NULL, TO_ROOM);
    }
    else
    {
      modify_move(ch, -1 * cost);
    }
  }

  regen_limb(ch);
}

void do_shield(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  
  argument = one_argument(argument, arg);

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_SHADOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (IS_CLASS(ch, CLASS_SHADOW) && !IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_SHIELD))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_IMMUNE(ch,IMM_SHIELDED) )
  {
    send_to_char("You shield your aura from those around you.\n\r",ch);
    SET_BIT(ch->immune, IMM_SHIELDED);
    return;
  }
  send_to_char("You stop shielding your aura.\n\r",ch);
  REMOVE_BIT(ch->immune, IMM_SHIELDED);
}
