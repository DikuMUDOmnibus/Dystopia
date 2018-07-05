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

void giantattack                ( CHAR_DATA *ch, int attacktype );
bool event_player_deathfrenzy   ( EVENT_DATA *event );
bool event_player_waterstream   ( EVENT_DATA *event );
bool event_player_heatmetal     ( EVENT_DATA *event );
bool event_player_chopattack    ( EVENT_DATA *event );
bool event_player_whirlwind     ( EVENT_DATA *event );
void dawnboost                  ( CHAR_DATA *ch, int level );

const struct evolve_entry giant_evolve_table[] =
{
  { "heat metal", 0, 0,
    EVOLVE_1, GIANT_EVOLVE_FIRE, EVOLVE_1, GIANT_EVOLVE_WATER,
    20000, 25000, 15000, 50000000, 5000
  },

  { "stream of water", 0, 0,
    EVOLVE_1, GIANT_EVOLVE_WATER, EVOLVE_1, GIANT_EVOLVE_FIRE,
    20000, 15000, 20000, 50000000, 5000
  },

  { "form of mud", 0, 0,
    EVOLVE_1, GIANT_EVOLVE_EARTH, EVOLVE_1, GIANT_EVOLVE_WIND,
    25000, 15000, 15000, 50000000, 5000
  },

  { "gust of wind", 0, 0,
    EVOLVE_1, GIANT_EVOLVE_WIND, EVOLVE_1, GIANT_EVOLVE_EARTH,
    20000, 20000, 20000, 50000000, 5000
  },

  { "shamans path", 0, 0,
    EVOLVE_1, GIANT_EVOLVE_SHAMAN, EVOLVE_1, GIANT_EVOLVE_WARRIOR,
    15000, 30000, 15000, 50000000, 5000
  },

  { "warriors path", 0, 0,
    EVOLVE_1, GIANT_EVOLVE_WARRIOR, EVOLVE_1, GIANT_EVOLVE_SHAMAN,
    30000, 15000, 15000, 50000000, 5000
  },

  { "slow spell", EVOLVE_1, GIANT_EVOLVE_SHAMAN,
    EVOLVE_2, GIANT_EVOLVE_SLOW, EVOLVE_2, GIANT_EVOLVE_HASTE,
    25000, 35000, 20000, 200000000, 10000
  },

  { "haste spell", EVOLVE_1, GIANT_EVOLVE_SHAMAN,
    EVOLVE_2, GIANT_EVOLVE_HASTE, EVOLVE_2, GIANT_EVOLVE_SLOW,
    20000, 35000, 25000, 200000000, 10000
  },

  { "chop attack", EVOLVE_1, GIANT_EVOLVE_WARRIOR,
    EVOLVE_2, GIANT_EVOLVE_CHOP, EVOLVE_2, GIANT_EVOLVE_DEATHFRENZY,
    40000, 25000, 20000, 200000000, 10000
  },

  { "deathfrenzy", EVOLVE_1, GIANT_EVOLVE_WARRIOR,
    EVOLVE_2, GIANT_EVOLVE_DEATHFRENZY, EVOLVE_2, GIANT_EVOLVE_CHOP,
    40000, 20000, 25000, 200000000, 10000
  },

  { "fiery mallet", EVOLVE_1, GIANT_EVOLVE_FIRE,
    EVOLVE_2, GIANT_EVOLVE_MALLET, EVOLVE_2, GIANT_EVOLVE_IGNITE,
    40000, 20000, 20000, 200000000, 10000
  },

  { "ignite metal", EVOLVE_1, GIANT_EVOLVE_FIRE,
    EVOLVE_2, GIANT_EVOLVE_IGNITE, EVOLVE_2, GIANT_EVOLVE_MALLET,
    30000, 30000, 30000, 200000000, 10000
  },

  { "wind walk", EVOLVE_1, GIANT_EVOLVE_WIND,
    EVOLVE_2, GIANT_EVOLVE_WINDWALK, EVOLVE_2, GIANT_EVOLVE_WHIRLWIND,
    28000, 28000, 38000, 200000000, 10000
  },

  { "whirlwind", EVOLVE_1, GIANT_EVOLVE_WIND,
    EVOLVE_2, GIANT_EVOLVE_WHIRLWIND, EVOLVE_2, GIANT_EVOLVE_WINDWALK,
    28000, 38000, 28000, 200000000, 10000
  },

  { "liquify", EVOLVE_1, GIANT_EVOLVE_WATER,
    EVOLVE_2, GIANT_EVOLVE_LIQUIFY, EVOLVE_2, GIANT_EVOLVE_WATERDOME,
    30000, 40000, 30000, 200000000, 10000
  },

  { "water dome", EVOLVE_1, GIANT_EVOLVE_WATER,
    EVOLVE_2, GIANT_EVOLVE_WATERDOME, EVOLVE_2, GIANT_EVOLVE_LIQUIFY,
    30000, 30000, 40000, 200000000, 10000
  },

  { "sinkhole", EVOLVE_1, GIANT_EVOLVE_EARTH,
    EVOLVE_2, GIANT_EVOLVE_SINKHOLE, EVOLVE_2, GIANT_EVOLVE_EARTHFLUX,
    40000, 30000, 20000, 200000000, 10000
  },

  { "earth flux", EVOLVE_1, GIANT_EVOLVE_EARTH,
    EVOLVE_2, GIANT_EVOLVE_EARTHFLUX, EVOLVE_2, GIANT_EVOLVE_SINKHOLE,
    40000, 20000, 30000, 200000000, 10000
  },

  { "Tortoise Wizardry", EVOLVE_2, GIANT_EVOLVE_SLOW,
    EVOLVE_3, GIANT_EVOLVE_TORTOISE, EVOLVE_3, GIANT_EVOLVE_KABALISTIC,
    50000, 50000, 40000, 500000000, 15000
  },

  { "Kabalistic Lore", EVOLVE_2, GIANT_EVOLVE_SLOW,
    EVOLVE_3, GIANT_EVOLVE_KABALISTIC, EVOLVE_3, GIANT_EVOLVE_TORTOISE,
    45000, 50000, 45000, 500000000, 15000
  },

  { "Spectral Spells", EVOLVE_2, GIANT_EVOLVE_HASTE,
    EVOLVE_3, GIANT_EVOLVE_SPECTRAL, EVOLVE_3, GIANT_EVOLVE_VOODOO,
    50000, 40000, 50000, 500000000, 15000
  },

  { "Accelerated Voodoo", EVOLVE_2, GIANT_EVOLVE_HASTE,
    EVOLVE_3, GIANT_EVOLVE_VOODOO, EVOLVE_3, GIANT_EVOLVE_SPECTRAL,
    45000, 45000, 50000, 500000000, 15000
  },

  /* end of table */
  { "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

void show_giant_evolves(CHAR_DATA *ch, int base, EVOLVE_DATA *evolve, bool recursive)
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

  for (i = 0; giant_evolve_table[i].name[0] != '\0'; i++)
  {
    int *reqfield = &ch->pcdata->powers[giant_evolve_table[i].req_field];
    int *evolvefield = &ch->pcdata->powers[giant_evolve_table[i].evolve_field];
    int *opposefield = &ch->pcdata->powers[giant_evolve_table[i].oppose_field];

    /* got the opposing evolve ? */
    if (giant_evolve_table[i].oppose_bit != 0 && IS_SET(*opposefield, giant_evolve_table[i].oppose_bit))
      continue;

    if (base != -1)
    {
      if (giant_evolve_table[i].req_bit != giant_evolve_table[base].evolve_bit) continue;
      if (giant_evolve_table[i].req_field != giant_evolve_table[base].evolve_field) continue;
    }
    else
    {
      if (giant_evolve_table[i].req_bit != 0 && !IS_SET(*reqfield, giant_evolve_table[i].req_bit))
        continue;
    }

    /* add this evolve, then do the recursion dance */
    if (!IS_SET(*evolvefield, giant_evolve_table[i].evolve_bit))
    {
      if (recursive)
        strcat(evolve->error, "  #c");
      else
        strcat(evolve->error, "#C");

      /* add this evolve, then do the recursion dance */
      sprintf(buf, " <SEND href=\"help '%s'\">%-20.20s</SEND> %s %5d %6d %6d %10d %6d#n<BR>",
        giant_evolve_table[i].name,
        giant_evolve_table[i].name,
        (recursive) ? "" : "  ",
        giant_evolve_table[i].hps,
        giant_evolve_table[i].mana,
        giant_evolve_table[i].move,
        giant_evolve_table[i].exp,
        giant_evolve_table[i].gold);
      strcat(evolve->error, buf);

      found = TRUE;

      if (!recursive) 
        show_giant_evolves(ch, i, evolve, TRUE);
    }
  }

  if (base == -1 && !found)
  {
    sprintf(evolve->error, "You are unable to evolve.<BR>");
  }
}

void giant_evolve(CHAR_DATA *ch, char *argument, EVOLVE_DATA *evolve)
{
  int i;

  if (argument[0] == '\0')
  {
    show_giant_evolves(ch, -1, evolve, FALSE);
    return;
  }

  for (i = 0; giant_evolve_table[i].name[0] != '\0'; i++)
  {
    int *reqfield = &ch->pcdata->powers[giant_evolve_table[i].req_field];
    int *evolvefield = &ch->pcdata->powers[giant_evolve_table[i].evolve_field];
    int *opposefield = &ch->pcdata->powers[giant_evolve_table[i].oppose_field];

    if (IS_SET(*evolvefield, giant_evolve_table[i].evolve_bit))
      continue;
    if (giant_evolve_table[i].oppose_bit != 0 && IS_SET(*opposefield, giant_evolve_table[i].oppose_bit))
      continue;
    if (giant_evolve_table[i].req_bit != 0 && !IS_SET(*reqfield, giant_evolve_table[i].req_bit))
      continue;

    if (!str_cmp(argument, giant_evolve_table[i].name))
      break;
  }

  if (giant_evolve_table[i].name[0] == '\0')
  {
    sprintf(evolve->error, "There is no evolve by that name.<BR>");
    return;
  }

  /* set the evolve data */
  evolve->hps   =  giant_evolve_table[i].hps;
  evolve->mana  =  giant_evolve_table[i].mana;
  evolve->move  =  giant_evolve_table[i].move;
  evolve->exp   =  giant_evolve_table[i].exp;
  evolve->gold  =  giant_evolve_table[i].gold;
  evolve->field =  &ch->pcdata->powers[giant_evolve_table[i].evolve_field];
  evolve->bit   =  giant_evolve_table[i].evolve_bit;
  evolve->valid =  TRUE;
}

void giant_commands(CHAR_DATA *ch)
{
  BUFFER *buf = buffer_new(MAX_STRING_LENGTH);
  char evolve[MAX_STRING_LENGTH]; int evocount = 0;
  char generic[MAX_STRING_LENGTH]; int gencount = 0;
  char rank[MAX_STRING_LENGTH]; int rancount = 0;
  char gifts[MAX_STRING_LENGTH]; int gifcount = 0;
  int cmd;

  bprintf(buf, "%s\n\r", get_dystopia_banner("    Powers    ", 76));

  for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
  {
    if (cmd_table[cmd].race != CLASS_GIANT)
      continue;

    /* check to see if the player has actually learned the power */
    if (!can_use_command(ch, cmd))
      continue;

    switch(cmd_table[cmd].powertype)
    {
      default:
        bug("giant_commands: cmd %d unknown powertype.", cmd);
        break;
      case 0:
        if (gencount == 0)
          sprintf(generic, " %-15s :", "Giant Powers");
        strcat(generic, " ");
        strcat(generic, cmd_table[cmd].name);
        gencount++;
        break;
      case GIANT_RANK:
        if (rancount == 0)
          sprintf(rank, " %-15s :", "Growths");
        strcat(rank, " ");
        strcat(rank, cmd_table[cmd].name);
        rancount++;
        break;
      case GIANT_GIFTS:
        if (gifcount == 0)
          sprintf(gifts, " %-15s :", "Giant Gifts");
        strcat(gifts, " ");
        strcat(gifts, cmd_table[cmd].name);
        gifcount++;
        break;
      case EVOLVE_1:
      case EVOLVE_2:
      case EVOLVE_3:
        if (evocount == 0)
          sprintf(evolve, " %-15s :", "Evolutions");
        strcat(evolve, " ");
        strcat(evolve, cmd_table[cmd].name);
        evocount++;
        break;
    }
  }

  if (gencount > 0)
    bprintf(buf, "%19.19s%s\n\r", generic, line_indent(&generic[18], 19, 75));
  if (rancount > 0)
    bprintf(buf, "%19.19s%s\n\r", rank, line_indent(&rank[18], 19, 75));
  if (gifcount > 0)
    bprintf(buf, "%19.19s%s\n\r", gifts, line_indent(&gifts[18], 19, 75));
  if (evocount > 0)
    bprintf(buf, "%19.19s%s\n\r", evolve, line_indent(&evolve[18], 19, 75));

  bprintf(buf, "%s\n\r", get_dystopia_banner("", 76));

  send_to_char(buf->data, ch);
  buffer_free(buf);
}

void do_whirlwind(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];
  int cost = 2500;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT)
  || !IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_WHIRLWIND))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Conjure up a whirlwind to attack whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL && (victim = ch->fighting) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (ch == victim)
  {
    send_to_char("You cannot conjure whirlwinds to attack yourself.\n\r", ch);
    return;
  }

  if (is_safe(ch, victim))
    return;

  if (event_isset_mobile(victim, EVENT_PLAYER_WHIRLWIND))
  {
    send_to_char("A whirlwind is already attacking that player.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to conjure forth a whirlwind.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("$n conjures forth a whirlwind and sets it on $N.", ch, NULL, victim, TO_NOTVICT);
  act("You conjure forth a whirlwind and sets it on $N.", ch, NULL, victim, TO_CHAR);
  act("$n conjures forth a whirlwind and sets it on you.", ch, NULL, victim, TO_VICT);

  WAIT_STATE(ch, 8);

  event = alloc_event();
  event->fun = &event_player_whirlwind;
  event->type = EVENT_PLAYER_WHIRLWIND;
  add_event_char(event, victim, 2 * PULSE_PER_SECOND);

  aggress(ch, victim);
}

bool event_player_whirlwind(EVENT_DATA *event)
{
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_whirldwind: no owner.", 0);
    return FALSE;
  }

  if (ch->hit < 1)
  {
    act("The whirlwind attacking $n fades away.", ch, NULL, NULL, TO_ROOM);
    send_to_char("The whirlwind attacking you fades away.\n\r", ch);
    return FALSE;
  }

  switch(number_range(1, 4))
  {
    default:
      break;
    case 2:
      act("Lightning crackles up and down $n, courtesy of the whirlwind.", ch, NULL, NULL, TO_ROOM);
      send_to_char("Lightning crackles toward you from the whirlwind.\n\r", ch);
      modify_hps(ch, -1 * number_range(400, 800));
      update_pos(ch);
      break;
    case 3:
      act("A huge blast of wind steals the breath from $n, courtesy of the whirlwind.", ch, NULL, NULL, TO_ROOM);
      send_to_char("A huge blast of wind from the whirlwind steals your breath.\n\r", ch);
      modify_move(ch, -1 * number_range(600, 1200));
      break;
    case 4:
      act("The whirlwind seems to syphon the magical lifeforce from $n.", ch, NULL, NULL, TO_ROOM);
      send_to_char("The whirlwind drains a portion of your magical lifeforce.\n\r", ch);
      modify_mana(ch, -1 * number_range(600, 1200));
      break;
  }

  if (number_percent() >= 85)
  {
    event = alloc_event();
    event->fun = &event_player_whirlwind;
    event->type = EVENT_PLAYER_WHIRLWIND;
    add_event_char(event, ch, 2 * PULSE_PER_SECOND);
  }
  else
  {
    act("The whirlwind attacking $n fades away.", ch, NULL, NULL, TO_ROOM);
    send_to_char("The whirlwind attacking you fades away.\n\r", ch);
  }

  return FALSE;
}

void do_windwalk(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT)
  || !IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_WINDWALK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  TOGGLE_BIT(ch->extra, EXTRA_WINDWALK);

  if (IS_SET(ch->extra, EXTRA_WINDWALK))
  {
    act("A violent storm erupts from your body, lifting you into the wind.", ch, NULL, NULL, TO_CHAR);
    act("A violent storm erupts from $n's body, and lifts $m into the wind.", ch, NULL, NULL, TO_ROOM);
  }
  else
  {
    act("The storm calms down, and you return to your feet.", ch, NULL, NULL, TO_CHAR);
    act("The violent storm keeping $n in the air calms down.", ch, NULL, NULL, TO_ROOM);
  }
}

void do_slowspell(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int sn, move_cost = 2000, mana_cost = 2000;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT)
  || !IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_SLOW))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (event_isset_mobile(ch, EVENT_PLAYER_SLOWSPELL))
  {
    send_to_char("You can only cast this spell once every 30 seconds.\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Whom do you wish to target with your slow spell?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  mana_cost = reduce_cost(ch, victim, mana_cost, eMana);
  move_cost = reduce_cost(ch, victim, move_cost, eMove);

  if (ch->move < move_cost || ch->mana < mana_cost)
  {
    printf_to_char(ch, "You need %d more move and %d more mana.\n\r",
      (move_cost - ch->move > 0) ? move_cost - ch->move : 0,
      (mana_cost - ch->mana > 0) ? mana_cost - ch->mana : 0);
    return;
  }

  if ((sn = skill_lookup("slow spell")) <= 0)
  {
    send_to_char("You have found a bug, please report this.\n\r", ch);
    return;
  }

  if (is_safe(ch, victim)) return;

  modify_move(ch, -1 * move_cost);
  modify_mana(ch, -1 * mana_cost);

  do_say(ch, "Me cast mighty magic on you! Uga Buga!");

  (*skill_table[sn].spell_fun) (sn, 50, ch, victim);

  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_PLAYER_SLOWSPELL;
  add_event_char(event, ch, 30 * PULSE_PER_SECOND);
}

bool event_room_waterdome(EVENT_DATA *event)
{
  ROOM_INDEX_DATA *pRoom;
  CHAR_DATA *ch;
  char buf[MAX_STRING_LENGTH];
  int playerid = (event->argument) ? atoi(event->argument) : 0;
  int cost = 750;

  if ((pRoom = event->owner.room) == NULL)
  {
    bug("event_room_waterdome: no owner.", 0);
    return FALSE;
  }

  if ((ch = get_online_player(playerid)) == NULL)
  {
    if ((ch = (CHAR_DATA *) FirstInList(pRoom->people)) != NULL)
      act("The water dome falls apart in a shower of water.", ch, NULL, NULL, TO_ALL);

    return FALSE;
  }

  if (ch->mana < cost)
  {
    if ((ch = (CHAR_DATA *) FirstInList(pRoom->people)) != NULL)
      act("The water dome falls apart in a shower of water.", ch, NULL, NULL, TO_ALL);

    return FALSE;
  }
  modify_mana(ch, -1 * cost);

  sprintf(buf, "%d", playerid);

  event = alloc_event();
  event->fun = &event_room_waterdome;
  event->type = EVENT_ROOM_WATERDOME;
  event->argument = str_dup(buf);
  add_event_room(event, pRoom, 3 * PULSE_PER_SECOND);

  return FALSE;
}

void do_waterdome(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT)
  || !IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_WATERDOME))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (ch->in_room == NULL)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }

  if (!str_cmp(argument, "off"))
  {
    CHAR_DATA *gch;
    ROOM_INDEX_DATA *pRoom;
    ITERATOR *pIter;
    bool found = FALSE;
    int i;

    for (i = 0; i < MAX_EVENT_HASH; i++)
    {
      pIter = AllocIterator(eventqueue[i]);
      while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
      {
        if (event->type == EVENT_ROOM_WATERDOME && (pRoom = event->owner.room) != NULL && event->argument)
        {
          if (atoi(event->argument) == ch->pcdata->playerid)
          {
            if ((gch = (CHAR_DATA *) FirstInList(pRoom->people)) != NULL)
              act("The water dome falls apart in a shower of water.", gch, NULL, NULL, TO_ALL);

            if (ch->in_room != pRoom)
              send_to_char("A water dome is dispelled.\n\r", ch);

            found = TRUE;
            dequeue_event(event, TRUE);
          }
        }
      }
    }

    if (!found)
      send_to_char("You have no water domes running active in the world.\n\r", ch);

    return;
  }

  if (has_timer(ch))
    return;

  if ((event = event_isset_room(ch->in_room, EVENT_ROOM_WATERDOME)) != NULL)
  {
    send_to_char("This room is already covered in a water dome.\n\r", ch);
    return;
  }

  send_to_char("You raise a huge water dome, which grows to protect the entire room.\n\r", ch);
  act("$n raises a huge water dome, which grows to fill the entire room.", ch, NULL, NULL, TO_ROOM);

  sprintf(buf, "%d", ch->pcdata->playerid);

  event = alloc_event();
  event->fun = &event_room_waterdome;
  event->type = EVENT_ROOM_WATERDOME;
  event->argument = str_dup(buf);
  add_event_room(event, ch->in_room, 3 * PULSE_PER_SECOND);
}

void do_liquify(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  AFFECT_DATA paf;
  char arg[MAX_INPUT_LENGTH];
  int gold_cost = 200, mana_cost = 500;
  int duration = 20;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT)
  || !IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_LIQUIFY))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  argument = one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("What item do you wish to liquify?\n\r", ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You do not have that item.\n\r", ch);
    return;
  }

  if (obj->ownerid != ch->pcdata->playerid)
  {
    send_to_char("You do not own this item.\n\r", ch);
    return;
  }

  if (object_is_affected(obj, OAFF_LIQUID))
  {
    send_to_char("This item is already liquified.\n\r", ch);
    return;
  }

  if (ch->mana < mana_cost)
  {
    printf_to_char(ch, "You need %d more mana to use liquify on this item.\n\r", mana_cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * mana_cost);

  if (!str_cmp(argument, "permanent"))
  {
    if (getGold(ch) < gold_cost)
    {
      printf_to_char(ch, "You need %d more gold to make this spell permanent.\n\r", gold_cost - getGold(ch));
      return;
    }
    setGold(ch, -1 * gold_cost);
    duration = -1;
  }

  paf.type = OAFF_LIQUID;
  paf.duration = duration;
  paf.location = APPLY_NONE;
  paf.modifier = 0;
  paf.bitvector = 0;
  affect_to_obj(obj, &paf);

  if (duration >= 0 && !event_isset_object(obj, EVENT_OBJECT_AFFECTS))
  {
    EVENT_DATA *event;

    event              =  alloc_event();
    event->fun         =  &event_object_affects;
    event->type        =  EVENT_OBJECT_AFFECTS;
    add_event_object(event, obj, 20 * PULSE_PER_SECOND);
  }

  act("$n spits on $p, and it turns into a watery substance.", ch, obj, NULL, TO_ROOM);
  act("You spit on $p, and it turns into a watery substance.", ch, obj, NULL, TO_CHAR);
}

void do_waterflux(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  int cost = 2000;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT)
  || !IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_LIQUIFY))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (event_isset_mobile(ch, EVENT_PLAYER_WATERFLUX))
  {
    send_to_char("You are still recovering from your last waterflux.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana before you can do a waterflux.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("A shower of water bursts from $n's body, purifying $m.", ch, NULL, NULL, TO_ROOM);
  send_to_char("You purify your body with a shower of water.\n\r", ch);

  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_PLAYER_WATERFLUX;
  add_event_char(event, ch, 10 * PULSE_PER_SECOND);

  /* strip bad affects */
  strip_event_mobile(ch, EVENT_PLAYER_LEECHES);
  strip_event_mobile(ch, EVENT_PLAYER_HEATMETAL);
}

void do_sinkhole(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT)
  || !IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_SINKHOLE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  TOGGLE_BIT(ch->newbits, NEW_SINKHOLE);

  if (IS_SET(ch->newbits, NEW_SINKHOLE))
  {
    act("A sinkhole appears infront of $n.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You create a sinkhole in front of you.\n\r", ch);
  }
  else
  {
    act("The sinkhole in front of $n vanishes in a puff of smoke.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You dispel your sinkhole.\n\r", ch);
  }
}

bool event_player_earthflux(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  int cost = 600;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_earthflux: no owner.", 0);
    return FALSE;
  }

  if (ch->hit <= cost)
  {
    send_to_char("The hot molten lava in your bloodstreams cools down.\n\r", ch);
    send_to_char("The spikes extruding from your body falls of and crumbles to dust.\n\r", ch);
    send_to_char("The killing madness leaves your mind.\n\r", ch);

    act("$n's body stops emitting bursts of lava", ch, NULL, NULL, TO_ROOM);
    act("The spikes extruding from $n falls of and crumbles to dust.", ch, NULL, NULL, TO_ROOM);
    return FALSE;
  }
  modify_hps(ch, -1 * cost);

  /* requeue */
  event = alloc_event();
  event->type = EVENT_PLAYER_EARTHFLUX;
  event->fun = &event_player_earthflux;
  add_event_char(event, ch, 3 * PULSE_PER_SECOND);

  return FALSE;
}

void do_earthflux(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT)
  || !IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_EARTHFLUX))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_EARTHFLUX)) != NULL)
  {
    dequeue_event(event, TRUE);

    send_to_char("The hot molten lava in your bloodstreams cools down.\n\r", ch);
    send_to_char("The spikes extruding from your body falls of and crumbles to dust.\n\r", ch);
    send_to_char("The killing madness leaves your mind.\n\r", ch);

    act("$n's body stops emitting bursts of lava", ch, NULL, NULL, TO_ROOM);
    act("The spikes extruding from $n falls of and crumbles to dust.", ch, NULL, NULL, TO_ROOM);

    modify_hps(ch, -500);
    update_pos(ch);
  }
  else
  {
    int cost = 500;

    if (ch->hit <= cost)
    {
      printf_to_char(ch, "You need another %d hitpoints to use this power.\n\r", cost + 1 - ch->hit);
      return;
    }
    modify_hps(ch, -1 * cost);

    event = alloc_event();
    event->type = EVENT_PLAYER_EARTHFLUX;
    event->fun = &event_player_earthflux;
    add_event_char(event, ch, 3 * PULSE_PER_SECOND);

    send_to_char("Your bloodstream turns to hot molten lava.\n\r", ch);
    send_to_char("Long, sharp spikes extrude from your body, causing great pain.\n\r", ch);
    send_to_char("The pain forces you into a murdering rampage.\n\r", ch);

    act("$n's body starts emitting bursts of hot molten lava.", ch, NULL, NULL, TO_ROOM);
    act("Long, sharp spikes extrude from $n's body.", ch, NULL, NULL, TO_ROOM);
  }
}

void do_mallet(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int cost = 1000;
  int sn, protect;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT)
  || !IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_MALLET))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (!IS_GOOD(ch) && !IS_EVIL(ch))
  {
    send_to_char("You must either be good or evil to use this power.\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("What item do you wish to transform into a fiery mallet?\n\r", ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You cannot find that weapon.\n\r", ch);
    return;
  }

  if (obj->ownerid != ch->pcdata->playerid)
  {
    send_to_char("You do not own this weapon.\n\r", ch);
    return;
  }

  if (obj->item_type != ITEM_WEAPON)
  {
    send_to_char("That is not a weapon.\n\r", ch);
    return;
  }

  if (!IS_OBJ_STAT(obj, ITEM_RARE))
  {
    send_to_char("That is not a rare weapon.\n\r", ch);
    return;
  }

  if (obj->cost < 70)
  {
    send_to_char("The weapon must have a value of 70 or more.\n\r", ch);
    return;
  }

  if (obj->value[0] > 0)
  {
    send_to_char("This item already has a spell on it.\n\r", ch);
    return;
  }

  if (obj->value[1] > 40 || obj->value[2] > 45)
  {
    send_to_char("This item is to powerful to enchant.\n\r", ch);
    return;
  }

  if (getGold(ch) < cost)
  {
    printf_to_char(ch, "You need %d more gold before you can create a fiery mallet.\n\r", cost - getGold(ch));
    return;
  }
  setGold(ch, -1 * cost);

  sn = UMAX(0, skill_lookup("imp fireball"));

  if (IS_GOOD(ch))
    protect = OBJECT_PROTECT;
  else
    protect = OBJECT_PROTECTGOOD;

  obj->value[0] = 1000 * protect + sn;
  obj->value[1] = 40;
  obj->value[2] = 45;
  obj->value[3] = 7;
  obj->extra_flags = ITEM_GLOW + ITEM_LOYAL + ((IS_GOOD(ch)) ? ITEM_ANTI_EVIL : ITEM_ANTI_GOOD);

  act("$n breaths a cone of fire on $p.", ch, obj, NULL, TO_ROOM);
  act("You breath a cone of fire on $p.", ch, obj, NULL, TO_CHAR);

  free_string(obj->name);
  free_string(obj->short_descr);
  free_string(obj->description);
  obj->name = str_dup("fiery mallet");
  obj->short_descr = str_dup("a fiery mallet");
  obj->description = str_dup("a fiery mallet lies here.");
}

void do_burnmetal(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event; 
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int cost = 1000;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT)
  || !IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_IGNITE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Cast burn metal on whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (ch == victim)
  {
    send_to_char("That would be stupid.\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(victim, EVENT_PLAYER_HEATMETAL)) == NULL)
  {
    send_to_char("First you must cast heat metal on them.\n\r", ch);
    return;
  }

  if (event->argument == NULL)
  {
    send_to_char("You have found a bug, please report this.\n\r", ch);
    return;
  }

  if (strstr(event->argument, "ignite") == NULL)
  {
    send_to_char("First you must cast ignite metal on them.\n\r", ch);
    return;
  }

  if (strstr(event->argument, "burn") != NULL)
  {
    send_to_char("Their metal is already burning.", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need another %d mana to use this power.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  sprintf(buf, "%s burn", event->argument);
  free_string(event->argument);
  event->argument = str_dup(buf);

  act("You cast a 'burn metal' spell on $N.", ch, NULL, victim, TO_CHAR);
  act("$n casts a 'burn metal' spell on you.", ch, NULL, victim, TO_VICT);
  act("$n casts a 'burn metal' spell on $N.", ch, NULL, victim, TO_NOTVICT);

  WAIT_STATE(ch, 6);
}

void do_ignitemetal(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int cost = 1000;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT)
  || !IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_IGNITE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Cast ignite metal on whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (ch == victim)
  {
    send_to_char("That would be stupid.\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(victim, EVENT_PLAYER_HEATMETAL)) == NULL)
  {
    send_to_char("First you must cast heat metal on them.\n\r", ch);
    return;
  }

  if (event->argument == NULL)
  {
    send_to_char("You have found a bug, please report this.\n\r", ch);
    return;
  }

  if (strstr(event->argument, "ignite") != NULL)
  {
    send_to_char("They are already ignited.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need another %d mana to use this power.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  sprintf(buf, "%s ignite", event->argument);
  free_string(event->argument);
  event->argument = str_dup(buf);

  act("You cast a 'ignite metal' spell on $N.", ch, NULL, victim, TO_CHAR);
  act("$n casts a 'ignite metal' spell on you.", ch, NULL, victim, TO_VICT);
  act("$n casts a 'ignite metal' spell on $N.", ch, NULL, victim, TO_NOTVICT);

  WAIT_STATE(ch, 6);
}

void do_hastespell(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int sn, move_cost = 2000, mana_cost = 2000;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT)
  || !IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_HASTE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (event_isset_mobile(ch, EVENT_PLAYER_HASTESPELL))
  {
    send_to_char("You can only cast this spell once every 30 seconds.\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Whom do you wish to target with your haste spell?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  mana_cost = reduce_cost(ch, victim, mana_cost, eMana);
  move_cost = reduce_cost(ch, victim, move_cost, eMove);

  if (ch->move < move_cost || ch->mana < mana_cost)
  {
    printf_to_char(ch, "You need %d more move and %d more mana.\n\r",
      (move_cost - ch->move > 0) ? move_cost - ch->move : 0,
      (mana_cost - ch->mana > 0) ? mana_cost - ch->mana : 0);
    return;
  }

  if ((sn = skill_lookup("haste spell")) <= 0)
  {
    send_to_char("You have found a bug, please report this.\n\r", ch);
    return;
  }

  modify_move(ch, -1 * move_cost);
  modify_mana(ch, -1 * mana_cost);

  do_say(ch, "Me cast mighty magic on you! Uga Buga!");

  (*skill_table[sn].spell_fun) (sn, 50, ch, victim);

  event = alloc_event();
  event->fun = &event_dummy;  
  event->type = EVENT_PLAYER_HASTESPELL;
  add_event_char(event, ch, 30 * PULSE_PER_SECOND);
}

void do_chopattack(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  EVENT_DATA *event;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT)
  || !IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_CHOP))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_CHOPATTACK_WAIT)) != NULL)
  {
    printf_to_char(ch, "You must wait %s before you can use this power again.\n\r", event_time_left(event));
    return;
  }

  if ((victim = ch->fighting) == NULL)
  {
    send_to_char("But your not fighting anyone.\n\r", ch);
    return;
  }

  do_say(ch, "Me like you skull, me take!");

  one_hit(ch, victim, gsn_bash, 1);

  event = alloc_event();
  event->fun = &event_player_chopattack;
  event->type = EVENT_PLAYER_CHOPATTACK;
  event->argument = str_dup("1");
  add_event_char(event, ch, 6);

  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_PLAYER_CHOPATTACK_WAIT;
  add_event_char(event, ch, 60 * PULSE_PER_SECOND);

  WAIT_STATE(ch, 24);
}

bool event_player_chopattack(EVENT_DATA *event)
{
  CHAR_DATA *ch, *victim;
  char buf[MAX_STRING_LENGTH];
  int i = 0;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_chopattack: no owner.", 0);
    return FALSE;
  }

  /* still fighting ? */
  if ((victim = ch->fighting) == NULL)
    return FALSE;

  if (event->argument && (i = atoi(event->argument)) > 0)
  {
    switch(i)
    {
      default:
      {
        int sn;

        if ((sn = skill_lookup("nerve pinch")) <= 0)
        {
          bug("event_player_chopattack: no nerve pinch.", 0);
          return FALSE;
        }

        one_hit(ch, victim, gsn_bash, 1);
        (*skill_table[sn].spell_fun) (sn, 50, ch, victim);

        return FALSE;
        break;
      }
      case 1:
        one_hit(ch, victim, gsn_smack, 1);
        break;
      case 2:
        one_hit(ch, victim, gsn_thwack, 1);
        break;
    }
  }

  sprintf(buf, "%d", (i+1));

  event = alloc_event();
  event->fun = &event_player_chopattack;
  event->type = EVENT_PLAYER_CHOPATTACK;
  event->argument = str_dup(buf);
  add_event_char(event, ch, 6);

  return FALSE;
}

void do_heatmetal(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int cost = 1500;

  if (IS_NPC(ch) ||
     !IS_CLASS(ch, CLASS_GIANT) ||
     !IS_SET(ch->pcdata->powers[EVOLVE_1], GIANT_EVOLVE_FIRE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Whom do you wish to cast 'heat metal' on?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (ch == victim)
  {
    send_to_char("That would be stupid.\n\r", ch);
    return;
  }
  if (is_safe(ch, victim)) return;

  if (event_isset_mobile(victim, EVENT_PLAYER_HEATMETAL))
  {
    send_to_char("They are already affected.\n\r", ch);
    return;
  }

  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need another %d mana to use this power.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("You cast a 'heat metal' spell on $N.", ch, NULL, victim, TO_CHAR);
  act("$n casts a 'heat metal' spell on you.", ch, NULL, victim, TO_VICT);
  act("$n casts a 'heat metal' spell on $N.", ch, NULL, victim, TO_NOTVICT);

  event             =  alloc_event();
  event->argument   =  str_dup("1");
  event->type       =  EVENT_PLAYER_HEATMETAL;
  event->fun        =  &event_player_heatmetal;
  add_event_char(event, victim, 3 * PULSE_PER_SECOND);

  aggress(ch, victim);
}

bool event_player_heatmetal(EVENT_DATA *event)
{
  OBJ_DATA *obj;
  CHAR_DATA *ch;
  ITERATOR *pIter;
  int i = 10;
  bool ignite = FALSE;
  bool burn = FALSE;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("Event_player_heatmetal: no owner.", 0);
    return FALSE;
  }

  if (ch->position <= POS_STUNNED || ch->hit < 1)
    return FALSE;

  if (event->argument != NULL)
  {
    if (strstr(event->argument, "burn"))
      burn = TRUE;
    if (strstr(event->argument, "ignite"))
      ignite = TRUE;
  }

  pIter = AllocIterator(ch->carrying);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (object_is_affected(obj, OAFF_LIQUID))
      continue;

    if (obj->wear_loc != WEAR_NONE && number_range(1, 5) == 2)
    {
      if (burn)
      {
        act_brief("$n screams in pain as $p (on fire) burns through $s skin.", ch, obj, NULL, TO_ROOM, BRIEF_7);
        act_brief("$p (on fire) is burning through your skin.", ch, obj, NULL, TO_CHAR, BRIEF_7);
      }
      else if (ignite)
      {
        act_brief("$n screams in pain as $p (ignited) burns through $s skin.", ch, obj, NULL, TO_ROOM, BRIEF_7);
        act_brief("$p (ignited) is burning through your skin.", ch, obj, NULL, TO_CHAR, BRIEF_7);
      }
      else
      {
        act_brief("$n screams in pain as $p burns through $s skin.", ch, obj, NULL, TO_ROOM, BRIEF_7);
        act_brief("$p is burning through your skin.", ch, obj, NULL, TO_CHAR, BRIEF_7);
      }

      modify_hps(ch, -1 * number_range(100, 200));
      if (ignite)
        modify_hps(ch, -1 * number_range(100, 200));

      if (ch->hit < 1) ch->hit = 1;

      obj->condition -= number_range(2, 5);
      if (burn)
        obj->condition -= number_range(2, 5);

      if (obj->condition < 1)
        break_obj(ch, obj);
    }
  }

  if (event->argument != NULL)
    i = atoi(event->argument);

  if (i < 4)
  {
    EVENT_DATA *newevent;
    char buf[MAX_INPUT_LENGTH];

    sprintf(buf, "%d%s%s", ++i, (ignite) ? " ignite" : "", (burn) ? " burn" : "");

    newevent             =  alloc_event();
    newevent->argument   =  str_dup(buf);
    newevent->type       =  EVENT_PLAYER_HEATMETAL;
    newevent->fun        =  &event_player_heatmetal;
    add_event_char(newevent, ch, 3 * PULSE_PER_SECOND);
  }
  else
  {
    act("$n's equipment stops burning $s skin.", ch, NULL, NULL, TO_ROOM);
    act("Your equipment returns to normal.", ch, NULL, NULL, TO_CHAR);
  }

  return FALSE;
}

void do_gustwind(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  ITERATOR *pIter;
  EVENT_DATA *event;
  EXIT_DATA *pExit;
  ROOM_INDEX_DATA *to_room;
  int dir;

  if (IS_NPC(ch) ||
     !IS_CLASS(ch, CLASS_GIANT) ||
     !IS_SET(ch->pcdata->powers[EVOLVE_1], GIANT_EVOLVE_WIND))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_GUSTWIND_WAIT)) != NULL)
  {
    printf_to_char(ch, "You must wait %s before you can use this power again.\n\r", event_time_left(event));
    return;
  }

  act("You call upon the mountain winds to sweep these fields of vermin.", ch, NULL, NULL, TO_CHAR);
  act("$n reaches to the clouds and pulls down a thunder cloud.", ch, NULL, NULL, TO_ROOM);

  pIter = AllocIterator(ch->in_room->people);
  while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!IS_NPC(victim) && IS_CLASS(victim, CLASS_GIANT) &&
        victim->pcdata->powers[GIANT_STANDFIRM] == 1) continue;

    switch(number_range(1, 3))
    {
      default:
        break;
      case 2:
        act("$n is struck by lightning.", victim, NULL, NULL, TO_ROOM);
        send_to_char("You are struck by lightning.\n\r", victim);
        stop_fighting(victim, TRUE);
        modify_hps(victim, -1 * number_range(500, 1000));
        update_pos(victim);
        victim->position = UMIN(POS_STUNNED, ch->position);
        break;
      case 3:
        dir = number_range(0, 3);
        if ((pExit = victim->in_room->exit[dir]) != NULL && (to_room = pExit->to_room) != NULL)
        {
          char buf[MAX_INPUT_LENGTH];

          stop_fighting(victim, TRUE);

          act("$n is lifted up by the storm and carried away.", victim, NULL, NULL, TO_ROOM);
          act("You are lifted up by the storm and carried away.", victim, NULL, NULL, TO_CHAR);

          char_from_room(victim);
          char_to_room(victim, to_room, TRUE);

          sprintf(buf, "$n comes crashing into the room from the %s.", dir_name[rev_dir[dir]]);
          act(buf, victim, NULL, NULL, TO_ROOM);
          act("You crash into the ground.", victim, NULL, NULL, TO_CHAR);

          modify_hps(victim, -1 * number_range(500, 1000));
          update_pos(victim);
          victim->position = UMIN(POS_STUNNED, victim->position);
        }
        else
        {
          act("$n if lifted up by the storm and thrown into the wall.", victim, NULL, NULL, TO_ROOM);
          send_to_char("You are lifted by the storm and thrown into the wall.\n\r", victim);
          stop_fighting(victim, TRUE);
          modify_hps(victim, -1 * number_range(500, 1000));
          update_pos(victim);
          victim->position = UMIN(POS_STUNNED, ch->position);
        }
        break;
    }
  }

  /* enqueue delayd wait */
  event              =  alloc_event();
  event->fun         =  &event_dummy;
  event->type        =  EVENT_PLAYER_GUSTWIND_WAIT;
  add_event_char(event, ch, 60 * PULSE_PER_SECOND);
}

void do_mudform(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_GIANT) ||
     !IS_SET(ch->pcdata->powers[EVOLVE_1], GIANT_EVOLVE_EARTH))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (has_timer(ch))
    return;

  if (!IS_SET(ch->newbits, NEW_MUDFORM))
  {
    send_to_char("You body slowly turns to mud, leaving you as a small puddle on the floor.\n\r", ch);
    act("$n melts into a small puddle of mud.", ch, NULL, NULL, TO_ROOM);

    SET_BIT(ch->newbits, NEW_MUDFORM);
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    sprintf(buf,"A puddle of mud");
    free_string(ch->morph);
    ch->morph = str_dup(buf);
  }
  else
  {
    REMOVE_BIT(ch->newbits, NEW_MUDFORM);

    if (!IS_SET(ch->newbits, NEW_CUBEFORM))
    {
      REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
      free_string(ch->morph);
      ch->morph=str_dup("");
    }
    else
    {
      sprintf(buf, "%s the giant", ch->name);
      free_string(ch->morph);
      ch->morph = str_dup(buf);
    }

    if (IS_SET(ch->newbits, NEW_CUBEFORM))
      send_to_char("You grow from a puddle of mud into a huge giant.\n\r", ch);
    else
      send_to_char("You grow from a puddle of mud into your normal form.\n\r", ch);

    act("$n slowly forms from a puddle of mud on the floor.", ch, NULL, NULL, TO_ROOM);
  }
}

void do_waterstream(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch) ||
     !IS_CLASS(ch, CLASS_GIANT) ||
     !IS_SET(ch->pcdata->powers[EVOLVE_1], GIANT_EVOLVE_WATER))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (IS_SET(ch->newbits, NEW_MUDFORM))
  {
    send_to_char("Not in this form.\n\r", ch);
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
  }
  else
  {
    victim = ch;
  }

  if (event_isset_mobile(victim, EVENT_PLAYER_WATERSTREAM))
  {
    if (ch != victim)
      send_to_char("Their body is already glowing with this healing power.\n\r", ch);
    else
      send_to_char("Your body is not ready for you to use this power again.\n\r", ch);
    return;
  }

  if (victim != ch)
  {
    if (ch->move < 500)
    {
      send_to_char("You do not have enough move to use this power.\n\r", ch);
      return;
    }
    modify_move(ch, -500);
  }

  if (ch == victim)
  {
    act("You let purifying waters stream through your body.", ch, NULL, victim, TO_CHAR);
  }
  else
  {
    act("You let purifying waters stream through $N's body.", ch, NULL, victim, TO_CHAR);
    act("$n touches you briefly, leaving a soft glow on your body.", ch, NULL, victim, TO_VICT);
  }
  act("$n touches $N briefly, leaving a soft glow on $S body.", ch, NULL, victim, TO_NOTVICT);

  event            =  alloc_event();
  event->fun       =  &event_player_waterstream;
  event->type      =  EVENT_PLAYER_WATERSTREAM;
  event->argument  =  str_dup("1");
  add_event_char(event, victim, PULSE_PER_SECOND);

  WAIT_STATE(ch, 4);
}

bool event_player_waterstream(EVENT_DATA *event)
{
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("Event_player_waterstream: no owner.", 0);
    return FALSE;
  }

  /* reattach new event */
  if (event->argument != NULL)
  {
    EVENT_DATA *newevent;
    char buf[MAX_STRING_LENGTH];
    int i = atoi(event->argument);

    if (++i > 5 && number_range(1, 3) != 2)
    {
      send_to_char("The purifying waters have done their work.\n\r", ch);
      return FALSE;
    }

    sprintf(buf, "%d", i);

    newevent            =  alloc_event();
    newevent->fun       =  &event_player_waterstream;
    newevent->type      =  EVENT_PLAYER_WATERSTREAM;
    newevent->argument  =  str_dup(buf);
    add_event_char(newevent, ch, PULSE_PER_SECOND);
  }

  /* do the healing */
  if (ch->position > POS_STUNNED)
  {
    int gain = number_range(100, 200);

    modify_hps(ch, gain);
    modify_move(ch, gain);
    modify_mana(ch, gain);
  }

  return FALSE;
}

void dawnboost(CHAR_DATA *ch, int level)
{
  ch->damroll += level * 65;
  ch->hitroll += level * 65;
  ch->armor   -= level * 80;
}

void update_giant(CHAR_DATA *ch)
{
  if (IS_SET(ch->extra, EXTRA_WINDWALK))
  {
    int cost = 600;

    if (ch->move < cost)
    {
      REMOVE_BIT(ch->extra, EXTRA_WINDWALK);
      act("The storm calms down, and you return to your feet.", ch, NULL, NULL, TO_CHAR);
      act("The violent storm keeping $n in the air calms down.", ch, NULL, NULL, TO_ROOM);
    }
    else
    {
      modify_move(ch, -1 * cost);
    }
  }

  if (IS_SET(ch->newbits, NEW_SINKHOLE))
  {
    int cost = 600;

    if (ch->mana < cost)
    {
      REMOVE_BIT(ch->newbits, NEW_SINKHOLE);
      act("The sinkhole in front of $n vanishes in a puff of smoke.", ch, NULL, NULL, TO_ROOM);
      send_to_char("You dispel your sinkhole.\n\r", ch);
    }
    else
    {
      modify_mana(ch, -1 * cost);
    }
  }

  if (IS_SET(ch->newbits, NEW_MUDFORM))
  {
    int cost = 350;

    if (ch->move < cost)
    {
      REMOVE_BIT(ch->newbits, NEW_MUDFORM);
      send_to_char("You grow from a puddle of mud into a huge giant.\n\r", ch);
      act("$n slowly forms from a puddle of mud on the floor.", ch, NULL, NULL, TO_ROOM);

      REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
      free_string(ch->morph);
      ch->morph=str_dup("");  
    }
    else
    {
      modify_move(ch, -1 * cost);
    }
  }

  /* regenerate body and limbs */
  if (ch->hit != ch->max_hit && IS_ITEMAFF(ch, ITEMA_REGENERATE))
    regen_hps(ch, 2);
  if (ch->mana != ch->max_mana && IS_ITEMAFF(ch, ITEMA_REGENERATE))
    regen_mana(ch, 2);
  if (ch->move != ch->max_move && IS_ITEMAFF(ch, ITEMA_REGENERATE) && !IS_SET(ch->newbits, NEW_MUDFORM))
    regen_move(ch, 2);

  if (IS_ITEMAFF(ch, ITEMA_REGENERATE))
    regen_limb(ch);
}

void do_battletrain(CHAR_DATA *ch, char *argument)
{
  const int basecost = 2000000;
  int cost;
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_GIANT))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (argument[0] == '\0')
  {
    sprintf(buf, "Attack [%d] Defence [%d]\n\n\r",
      ch->pcdata->powers[GIANT_ATT],
      ch->pcdata->powers[GIANT_DEF]);
    send_to_char(buf, ch);
    send_to_char("Battletrain defence or attack?\n\r", ch);
    return;
  }
  if (!str_prefix(argument, "attack"))
  {
    cost = (ch->pcdata->powers[GIANT_ATT] + 1) * basecost;

    if (ch->pcdata->powers[GIANT_ATT] >= 10)
    {
      send_to_char("You cannot train your attack skill any higher.\n\r", ch);
      return;
    }
    if (ch->exp < cost)
    {
      printf_to_char(ch, "You need %d more exp to gain the next level.\n\r", cost - ch->exp);
      return;
    }
    ch->exp -= cost;
    ch->pcdata->powers[GIANT_ATT]++;
    send_to_char("You train your attack skill.\n\r", ch);
  }
  else if (!str_prefix(argument, "defence"))
  {
    cost = (ch->pcdata->powers[GIANT_DEF] + 1) * basecost;

    if (ch->pcdata->powers[GIANT_DEF] >= 10)
    {
      send_to_char("You cannot train your defence skill any higher.\n\r", ch);
      return;
    }
    if (ch->exp < cost)
    {
      printf_to_char(ch, "You need %d more exp to gain the next level.\n\r", cost - ch->exp);
      return;
    }
    ch->exp -= cost;
    ch->pcdata->powers[GIANT_DEF]++;
    send_to_char("You train your defence skill.\n\r", ch);
  }
  else
  {
    do_battletrain(ch, "");
  }
}

void do_grow(CHAR_DATA *ch, char *argument)
{
  int cost = 10000000;          /* 10 million */

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_GIANT))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[GIANT_RANK] >= FOOT_30)
  {
    send_to_char("Your quite a tall giant already.\n\r", ch);
    return;
  }
  if (ch->exp < cost * (ch->pcdata->powers[GIANT_RANK] + 1))
  {
    printf_to_char(ch, "You need %d more exp to grow.\n\r", cost * (ch->pcdata->powers[GIANT_RANK] + 1) - ch->exp);
    return;
  }
  ch->pcdata->powers[GIANT_RANK]++;

  /* boost the dawnstrength if enabled */
  if (IS_SET(ch->newbits, NEW_CUBEFORM))
  {
    dawnboost(ch, 1);
  }

  ch->exp -= ch->pcdata->powers[GIANT_RANK] * cost;
  send_to_char("You grow and grow, and can almost touch the stars.\n\r", ch);
}

void do_thwack(CHAR_DATA * ch, char *argument)
{
  giantattack(ch, 1);
}

void do_smack(CHAR_DATA * ch, char *argument)
{
  giantattack(ch, 2);
}

void do_crush(CHAR_DATA * ch, char *argument)
{
  if (!IS_CLASS(ch, CLASS_GIANT) || IS_NPC(ch))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (get_eq_char(ch, WEAR_MASTERY) == NULL)
  {
    send_to_char("You need to wear your mastery pendant.\n\r", ch);
    return;
  }
  giantattack(ch, 4);
}

void do_bash(CHAR_DATA * ch, char *argument)
{
  giantattack(ch, 3);
}

void giantattack(CHAR_DATA * ch, int attacktype)
{
  CHAR_DATA *victim;
  int cost = 1000;

  if (IS_NPC(ch))
    return;

  if (!IS_CLASS(ch, CLASS_GIANT))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((victim = ch->fighting) == NULL)
  {
    send_to_char("You aren't fighting anyone.\n\r", ch);
    return;
  }

  cost = reduce_cost(ch, victim, cost, eMove);
  if (ch->move < cost)
  {
    send_to_char("You're not up to it, you ain't got the stamina in ya.\n\r", ch);
    return;
  }
  modify_move(ch, -1 * cost);

  if (attacktype == ch->pcdata->powers[GIANT_ATTACK])
  {
    do_say(ch, "Oi Oi, I smell a tiny humaaaan, thwack, smack, bash you skull, humaaan.");
    one_hit(ch, victim, gsn_thwack, 1);
    one_hit(ch, victim, gsn_smack, 1);
    one_hit(ch, victim, gsn_bash, 1);
    if (get_eq_char(ch, WEAR_MASTERY) != NULL)
      one_hit(ch, victim, gsn_crush, 1);
  }
  else
  {
    if (attacktype == 1)
      one_hit(ch, victim, gsn_thwack, 1);
    if (attacktype == 2)
      one_hit(ch, victim, gsn_smack, 1);
    if (attacktype == 3)
      one_hit(ch, victim, gsn_bash, 1);
    if (attacktype == 4)
      one_hit(ch, victim, gsn_crush, 1);
  }
  WAIT_STATE(ch, 12);
}

void do_dawnstrength(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_GIANT))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[GIANT_RANK] == 0)
  {
    send_to_char("You're too short, piss off.\n\r", ch);
    return;
  }
  if (IS_SET(ch->newbits, NEW_CUBEFORM))
  {
    REMOVE_BIT(ch->newbits, NEW_CUBEFORM);
    act("$n lets the strength of old fade away.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You let the strength of old fade away.\n\r", ch);
    dawnboost(ch, -1 * ch->pcdata->powers[GIANT_RANK]);

    if (!IS_SET(ch->newbits, NEW_MUDFORM))
    {
      free_string(ch->morph);
      ch->morph = str_dup("");
      REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
    }
  }
  else
  {
    act("$n grows to enormous heights.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You call upon the strength of old to guide you.\n\r", ch);
    SET_BIT(ch->newbits, NEW_CUBEFORM);
    SET_BIT(ch->affected_by, AFF_POLYMORPH);

    if (!IS_SET(ch->newbits, NEW_MUDFORM))
    {
      sprintf(buf, "%s the giant", ch->name);
      free_string(ch->morph);
      ch->morph = str_dup(buf);
    }
    dawnboost(ch, ch->pcdata->powers[GIANT_RANK]);
  }
}

void do_giantgift(CHAR_DATA * ch, char *argument)
{
  int cost = 60000;   /* 60.000 class points */
  int max_gifts = 6;

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_GIANT))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[GGIFTS_GAINED] >= max_gifts)
  {
    send_to_char("You already have all giant gifts.\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[GIANT_POINTS] < cost)
  {
    printf_to_char(ch, "You still need %d class points for the next gift.\n\r", cost - ch->pcdata->powers[GIANT_POINTS]);
    return;
  }
  ch->pcdata->powers[GIANT_POINTS] -= cost;
  if (ch->pcdata->powers[GGIFTS_GAINED] < 1)
  {
    SET_BIT(ch->pcdata->powers[GIANT_GIFTS], GGIFT_LEATHERSKIN);
    send_to_char("You are gifted with leather skin, preventing damage taken.\n\r", ch);
  }
  else if (ch->pcdata->powers[GGIFTS_GAINED] < 2)
  {
    SET_BIT(ch->pcdata->powers[GIANT_GIFTS], GGIFT_STONESHAPE);
    send_to_char("You are gifted with the ability to shape stone onto tools of war.\n\r", ch);
  }
  else if (ch->pcdata->powers[GGIFTS_GAINED] < 3)
  {
    SET_BIT(ch->pcdata->powers[GIANT_GIFTS], GGIFT_REVIVAL);
    send_to_char("You are gifted the power of revival, the healing of nature.\n\r", ch);
  }
  else if (ch->pcdata->powers[GGIFTS_GAINED] < 4)
  {
    SET_BIT(ch->pcdata->powers[GIANT_GIFTS], GGIFT_EARTHPUNCH);
    send_to_char("You are granted the power to strike the earth with a mighty punch, shaking everyone in the room.\n\r", ch);
  }
  else if (ch->pcdata->powers[GGIFTS_GAINED] < 5)
  {
    SET_BIT(ch->pcdata->powers[GIANT_GIFTS], GGIFT_STANDFIRM);
    send_to_char("You are granted the power to stand firm, thus taking less damage in combat.\n\r", ch);
  }
  else
  {
    SET_BIT(ch->pcdata->powers[GIANT_GIFTS], GGIFT_LONGLEGS);
    send_to_char("You are granted the gift of long legs, thus being able to step in front of a fleeing opponent. (auto)\n\r", ch);
  }
  ch->pcdata->powers[GGIFTS_GAINED]++;
}

void do_gsweep(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *location;

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_GIANT))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[GIANT_RANK] < 2)
  {
    send_to_char("You are not tall enough.\n\r", ch);
    return;
  }
  if ((victim = ch->fighting) == NULL)
  {
    send_to_char("You're not fighting anyone you can sweep.\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("Not on mobiles.\n\r", ch);
    return;
  }
  do_say(ch, "Oi Oi, Im gonna crush you little skull, Humaaan.");
  one_hit(ch, victim, gsn_backfist, 0);
  one_hit(ch, victim, gsn_headbutt, 0);
  special_hurl(ch, victim);
  if (victim->in_room == NULL || ch->in_room == NULL)
    return;
  if (ch->in_room != victim->in_room)
  {
    stop_fighting(ch, TRUE);
    stop_fighting(victim, TRUE);
    act("$n walks after $N.", ch, victim, NULL, TO_ROOM);
    send_to_char("You follow the tiny humaaan.\n\r", ch);
    send_to_char("The Giant walks after you.....\n\r", victim);
    location = victim->in_room;
    char_from_room(ch);
    char_to_room(ch, location, TRUE);
    do_look(ch, "auto");
  }
  victim->position = POS_STANDING;
  one_hit(ch, victim, gsn_bash, 1);
  WAIT_STATE(ch, 24);
}

void do_revival(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_GIANT))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[GIANT_GIFTS], GGIFT_REVIVAL))
  {
    send_to_char("You don't have that gift.\n\r", ch);
    return;
  }
  modify_hps(ch, number_range(300, 600));
  send_to_char("You feel revived.\n\r", ch);
  WAIT_STATE(ch, 12);
}

void do_earthpunch(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *gch;
  ITERATOR *pIter;

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_GIANT))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[GIANT_GIFTS], GGIFT_EARTHPUNCH))
  {
    send_to_char("You don't have that gift.\n\r", ch);
    return;
  }
  send_to_char("You slam your fist into the ground creating a minor earthquake.\n\r", ch);
  act("$n slams $m fist into the ground, creating a minor earthquake.", ch, NULL, NULL, TO_ROOM);

  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (ch == gch || is_safe(ch, gch))
      continue;

    if (gch->fighting != NULL)
    {
      one_hit(ch, gch, gsn_bash, 0);
      send_to_char("You lose your balance.\n\r", gch);
      do_stance(gch, "none");
    }
    else
    {
      send_to_char("You drop to the ground, stunned.\n\r", gch);
      stop_fighting(gch, TRUE);
      gch->position = POS_STUNNED;
    }
  }

  WAIT_STATE(ch, 18);
}

void do_standfirm(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_GIANT))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[GIANT_GIFTS], GGIFT_STANDFIRM))
  {
    send_to_char("You don't have that gift.\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[GIANT_STANDFIRM] == 0)
  {
    send_to_char("You slam your feet hard into the ground, and refuse to budge.\n\r", ch);
    ch->pcdata->powers[GIANT_STANDFIRM] = 1;
  }
  else
  {
    send_to_char("You relax from your standfirm.\n\r", ch);
    ch->pcdata->powers[GIANT_STANDFIRM] = 0;
  }
}

void do_stoneshape(CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj;
  char arg[MAX_INPUT_LENGTH];
  int cost = 200;

  one_argument(argument, arg);

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_GIANT))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[GIANT_GIFTS], GGIFT_STONESHAPE))
  {
    send_to_char("You don't have that gift.\n\r", ch);
    return;
  }
  if (arg[0] == '\0')
  {
    send_to_char("Stoneshape what?\n\r", ch);
    return;
  }
  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You dont have that item.\n\r", ch);
    return;
  }
  if ((obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR) || IS_SET(obj->quest, QUEST_GIANTSTONE))
  {
    send_to_char("You are unable to stoneshape this item.\n\r", ch);
    return;
  }
  if (getGold(ch) < cost)
  {
    printf_to_char(ch, "It costs %d goldcrowns to use this power, and you don't have that much.\n\r", cost);
    return;
  }
  if (obj->item_type == ITEM_WEAPON)
  {
    if (obj->value[1] < 40)
      obj->value[1] += number_range(1, 3);
    if (obj->value[2] < 70)
      obj->value[2] += number_range(1, 3);
  }
  forge_affect(obj, number_range(2, 4));
  SET_BIT(obj->quest, QUEST_GIANTSTONE);
  setGold(ch, -1 * cost);
  send_to_char("You shape the ancient stone of the world onto the item, making it more powerful.\n\r", ch);
}

void do_deathfrenzy(CHAR_DATA * ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_GIANT))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[GIANT_RANK] < 5)
  {
    send_to_char("You are not tall enough.\n\r", ch);
    return;
  }
  if (IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_DEATHFRENZY) && ch->hit > ch->max_hit * 0.5)
  {
    send_to_char("Your still in good health.\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_DEATHFRENZY) && ch->hit > ch->max_hit * 0.25)
  {
    send_to_char("Your still in good health.\n\r", ch);
    return;
  }
  if (event_isset_mobile(ch, EVENT_PLAYER_DEATHFRENZY))
  {
    send_to_char("Your already raging.\n\r", ch);
    return;
  }
  if ((victim = ch->fighting) == NULL)
  {
    send_to_char("But, your not fighting anyone, who in the world do you plan on getting pissed at?\n\r", ch);
    return;
  }
  send_to_char("You enter your deathfrenzy.\n\r", ch);

  /* create and attach delayed event */
  event = alloc_event();
  event->fun = &event_player_deathfrenzy;
  event->type = EVENT_PLAYER_DEATHFRENZY;
  add_event_char(event, ch, number_range(30 * PULSE_PER_SECOND, 60 * PULSE_PER_SECOND));

  do_say(ch, "You Humaaans think you so smart, me bash little humaaan skull, you not so smart no more!");
  multi_hit(ch, victim, 4);
}

void do_rumble(CHAR_DATA * ch, char *argument)
{
  int class = ch->class;

  if (IS_NPC(ch) || (!IS_IMMORTAL(ch) && !IS_CLASS(ch, CLASS_GIANT)))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  ch->class = CLASS_GIANT;
  talk_channel(ch, argument, CHANNEL_CLASS, CC_GIANT, "rumble");
  ch->class = class;
}

bool event_player_deathfrenzy(EVENT_DATA * event)
{
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("Event_player_deathfrenzy: no owner.", 0);
    return FALSE;
  }

  send_to_char("You can enter the deathfrenzy again.\n\r", ch);

  /* we didn't dequeue it */
  return FALSE;
}

void do_earthswallow(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  CHAR_DATA *mount;
  ROOM_INDEX_DATA *pRoom;
  char arg[MAX_INPUT_LENGTH];
  int cost = 1500;

  one_argument(argument, arg);

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_GIANT))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[GIANT_RANK] < 3)
  {
    send_to_char("You are not tall enough.\n\r", ch);
    return;
  }

  if (has_timer(ch))
    return;

  if (arg[0] == '\0')
  {
    send_to_char("Earthswallow whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("You cannot earthswallow monsters.\n\r", ch);
    return;
  }

  if (is_safe(ch, victim))
    return;

  if (victim == ch)
  {
    send_to_char("That doesn't seem like a good idea.\n\r", ch);
    return;
  }

  if (ch->move < cost)
  {
    printf_to_char(ch, "You need %d more move to use this power.\n\r", cost - ch->move);
    return;
  }
  modify_move(ch, -1 * cost);

  act("You hammer the earth, which opens and swallows $N.", ch, NULL, victim, TO_CHAR);
  act("$n hammers the earth, which opens and swallows you.", ch, NULL, victim, TO_VICT);
  act("$n hammers the earth, which opens and swallows $N.", ch, NULL, victim, TO_NOTVICT);

  WAIT_STATE(ch, 8);

  if (victim->in_room == NULL || victim->in_room->area == NULL)
    return;

  if ((pRoom = get_rand_room_area(victim->in_room->area)) == NULL)
  {
    act("Nothing happens.", ch, NULL, NULL, TO_ALL);
    return;
  }

  char_from_room(victim);
  char_to_room(victim, pRoom, TRUE);

  act("$n is flung out of a vulcano.", victim, NULL, NULL, TO_ROOM);
  do_look(victim, "auto");

  if ((mount = victim->mount) == NULL)
    return;

  char_from_room(mount);
  char_to_room(mount, victim->in_room, TRUE);
  do_look(mount, "auto");
}
