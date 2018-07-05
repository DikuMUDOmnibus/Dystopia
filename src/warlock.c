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

/* local procedures */
bool  event_area_earthmother     ( EVENT_DATA *event );
bool  event_area_plague          ( EVENT_DATA *event );
bool  event_area_milkandhoney    ( EVENT_DATA *event );
bool  event_room_pentagram       ( EVENT_DATA *event );
bool  event_room_vines           ( EVENT_DATA *event );
bool  event_room_callwild        ( EVENT_DATA *event );
bool  event_room_doombolt        ( EVENT_DATA *event );
bool  event_player_displace      ( EVENT_DATA *event );
bool  event_player_pviper_red    ( EVENT_DATA *event );
bool  event_player_pviper_green  ( EVENT_DATA *event );
bool  event_player_pviper_yellow ( EVENT_DATA *event );
bool  event_player_pviper_purple ( EVENT_DATA *event );
bool  event_player_pviper_blue   ( EVENT_DATA *event );
bool  event_player_crows         ( EVENT_DATA *event );
bool  event_player_leeches       ( EVENT_DATA *event );
bool  event_player_scrybirds     ( EVENT_DATA *event );
bool  event_player_timetrip      ( EVENT_DATA *event );
bool  event_object_implode       ( EVENT_DATA *event );
bool  event_mobile_sunset        ( EVENT_DATA *event );
int   get_train_cost             ( int clevel, int basecost );
int   get_warlock_power          ( CHAR_DATA *ch, char *power );
bool  save_warlock_spell         ( CHAR_DATA *ch );
void  spell_to_char              ( SPELL_DATA *spell, CHAR_DATA *ch );
void  discharge_chain            ( CHAR_DATA *ch, int power, int tg, int excl, bool spellchain );
void  spell_attack               ( CHAR_DATA *ch, CHAR_DATA *victim, int power );
void  steal_object               ( OBJ_DATA *obj, CHAR_DATA *victim, CHAR_DATA *ch );
int   get_cost                   ( CHAR_DATA *ch, int cost );
bool  spell_excludes_char        ( SPELL_DATA *spell, int excl, CHAR_DATA *ch, CHAR_DATA *victim,
                                         CHAR_DATA *targ_char, bool inherit_exclude );
void  spell_boost_char           ( CHAR_DATA *ch, CHAR_DATA *victim, int power );

SPELL_DATA  * alloc_spell             ( void );
CHAR_DATA   * get_char_homingdevice   ( CHAR_DATA *ch, char *argument );

/* local variables */
STACK *spell_free = NULL;
struct archmage_type archmage_list[7];

const struct warlock_affect waffect_table[] =
{
  {   "blind",                AFF_BLIND          },
  {   "invisible",            AFF_INVISIBLE      },
  {   "detect-evil",          AFF_DETECT_EVIL    },
  {   "detect-invis",         AFF_DETECT_INVIS   },
  {   "detect-magic",         AFF_DETECT_MAGIC   },
  {   "detect-hidden",        AFF_DETECT_HIDDEN  },
  {   "sanctuary",            AFF_SANCTUARY      },
  {   "infrared",             AFF_INFRARED       },
  {   "curse",                AFF_CURSE          },
  {   "flaming",              AFF_FLAMING        },
  {   "poison",               AFF_POISON         },
  {   "sneak",                AFF_SNEAK          },
  {   "flying",               AFF_FLYING         },
  {   "pass-door",            AFF_PASS_DOOR      },
  {   "shadowsight",          AFF_SHADOWSIGHT    },
  {   "web",                  AFF_WEBBED         },
  {   "",                     0                  }
};

const struct evolve_entry warlock_evolve_table[] =
{
  { "path of time", 0, 0,
    EVOLVE_1, WARLOCK_EVOLVE_TIME, EVOLVE_1, WARLOCK_EVOLVE_SPACE,
    10000, 20000, 25000, 50000000, 5000
  },

  { "path of space", 0, 0,
    EVOLVE_1, WARLOCK_EVOLVE_SPACE, EVOLVE_1, WARLOCK_EVOLVE_TIME,
    15000, 30000, 15000, 50000000, 5000
  },

  { "contingency chain", 0, 0,
    EVOLVE_1, WARLOCK_EVOLVE_CONTINGENCY, EVOLVE_1, WARLOCK_EVOLVE_HOMING,
    25000, 15000, 15000, 50000000, 5000
  },

  { "homing device", 0, 0,
    EVOLVE_1, WARLOCK_EVOLVE_HOMING, EVOLVE_1, WARLOCK_EVOLVE_CONTINGENCY,
    20000, 20000, 20000, 50000000, 5000
  },

  { "doombolt", 0, 0,
    EVOLVE_1, WARLOCK_EVOLVE_DOOMBOLT, EVOLVE_1, WARLOCK_EVOLVE_MOUNTAIN,
    15000, 25000, 15000, 50000000, 5000
  },

  { "mountain king", 0, 0,
    EVOLVE_1, WARLOCK_EVOLVE_MOUNTAIN, EVOLVE_1, WARLOCK_EVOLVE_DOOMBOLT,
    15000, 15000, 25000, 50000000, 5000
  },

  {
    "plague chain", EVOLVE_1, WARLOCK_EVOLVE_SPACE,
    EVOLVE_2, WARLOCK_EVOLVE_PLAGUE, EVOLVE_2, WARLOCK_EVOLVE_GLIMMER,
    30000, 30000, 30000, 200000000, 10000
  },

  {
    "glimmer chain", EVOLVE_1, WARLOCK_EVOLVE_SPACE,
    EVOLVE_2, WARLOCK_EVOLVE_GLIMMER, EVOLVE_2, WARLOCK_EVOLVE_PLAGUE,
    25000, 40000, 25000, 200000000, 10000
  },

  {
    "time travel", EVOLVE_1, WARLOCK_EVOLVE_TIME,
    EVOLVE_2, WARLOCK_EVOLVE_TIMETRAVEL, EVOLVE_2, WARLOCK_EVOLVE_OLDAGE,
    25000, 25000, 40000, 200000000, 10000
  },

  {
    "sunset age", EVOLVE_1, WARLOCK_EVOLVE_TIME,
    EVOLVE_2, WARLOCK_EVOLVE_OLDAGE, EVOLVE_2, WARLOCK_EVOLVE_TIMETRAVEL,
    40000, 25000, 25000, 200000000, 10000
  },

  {
    "backlash", EVOLVE_1, WARLOCK_EVOLVE_HOMING,
    EVOLVE_2, WARLOCK_EVOLVE_BACKLASH, EVOLVE_2, WARLOCK_EVOLVE_HUNTINGSTARS,
    35000, 30000, 30000, 200000000, 10000
  },

  {
    "hunting stars", EVOLVE_1, WARLOCK_EVOLVE_HOMING,
    EVOLVE_2, WARLOCK_EVOLVE_HUNTINGSTARS, EVOLVE_2, WARLOCK_EVOLVE_BACKLASH,
    30000, 35000, 25000, 200000000, 10000
  },

  {
    "seven stitches", EVOLVE_1, WARLOCK_EVOLVE_CONTINGENCY,
    EVOLVE_2, WARLOCK_EVOLVE_STITCHES, EVOLVE_2, WARLOCK_EVOLVE_PRECOGNITION,
    25000, 30000, 40000, 200000000, 10000
  },

  {
    "precognition", EVOLVE_1, WARLOCK_EVOLVE_CONTINGENCY,
    EVOLVE_2, WARLOCK_EVOLVE_PRECOGNITION, EVOLVE_2, WARLOCK_EVOLVE_STITCHES,
    35000, 25000, 35000, 200000000, 10000
  },

  {
    "pulse of the earth", EVOLVE_1, WARLOCK_EVOLVE_MOUNTAIN,
    EVOLVE_2, WARLOCK_EVOLVE_EARTHPULSE, EVOLVE_2, WARLOCK_EVOLVE_DISJUNCTION,
    35000, 35000, 25000, 200000000, 10000
  },

  {
    "disjunction", EVOLVE_1, WARLOCK_EVOLVE_MOUNTAIN,
    EVOLVE_2, WARLOCK_EVOLVE_DISJUNCTION, EVOLVE_2, WARLOCK_EVOLVE_EARTHPULSE,
    35000, 35000, 25000, 200000000, 10000
  },

  {
    "doomstorm", EVOLVE_1, WARLOCK_EVOLVE_DOOMBOLT,
    EVOLVE_2, WARLOCK_EVOLVE_DOOMSTORM, EVOLVE_2, WARLOCK_EVOLVE_DOOMCHARGE,
    35000, 35000, 25000, 200000000, 10000
  },

  {
    "doomcharge", EVOLVE_1, WARLOCK_EVOLVE_DOOMBOLT,
    EVOLVE_2, WARLOCK_EVOLVE_DOOMCHARGE, EVOLVE_2, WARLOCK_EVOLVE_DOOMSTORM,
    30000, 40000, 25000, 200000000, 10000
  },

  { 
    "chains of decay", EVOLVE_2, WARLOCK_EVOLVE_PLAGUE,
    EVOLVE_3, WARLOCK_EVOLVE_DECAYCHAIN, EVOLVE_3, WARLOCK_EVOLVE_ZOMBIEENDURANCE,
    50000, 45000, 50000, 500000000, 15000
  },

  {
    "zombie endurance", EVOLVE_2, WARLOCK_EVOLVE_PLAGUE,
    EVOLVE_3, WARLOCK_EVOLVE_ZOMBIEENDURANCE, EVOLVE_3, WARLOCK_EVOLVE_DECAYCHAIN,
    45000, 50000, 50000, 500000000, 15000
  },

  /* end of table */
  { "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

void show_warlock_evolves(CHAR_DATA *ch, int base, EVOLVE_DATA *evolve, bool recursive)
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

  for (i = 0; warlock_evolve_table[i].name[0] != '\0'; i++)
  {
    int *reqfield = &ch->pcdata->powers[warlock_evolve_table[i].req_field];
    int *evolvefield = &ch->pcdata->powers[warlock_evolve_table[i].evolve_field];
    int *opposefield = &ch->pcdata->powers[warlock_evolve_table[i].oppose_field];

    /* got the opposing evolve ? */
    if (warlock_evolve_table[i].oppose_bit != 0 && IS_SET(*opposefield, warlock_evolve_table[i].oppose_bit))
      continue;

    if (base != -1)
    {
      if (warlock_evolve_table[i].req_bit != warlock_evolve_table[base].evolve_bit) continue;
      if (warlock_evolve_table[i].req_field != warlock_evolve_table[base].evolve_field) continue;
    }
    else
    {
      if (warlock_evolve_table[i].req_bit != 0 && !IS_SET(*reqfield, warlock_evolve_table[i].req_bit))
        continue;
    }

    /* add this evolve, then do the recursion dance */
    if (!IS_SET(*evolvefield, warlock_evolve_table[i].evolve_bit))
    {
      if (recursive)
        strcat(evolve->error, "  #c");
      else
        strcat(evolve->error, "#C");

      /* add this evolve, then do the recursion dance */
      sprintf(buf, " <SEND href=\"help '%s'\">%-20.20s</SEND> %s %5d %6d %6d %10d %6d#n<BR>",
        warlock_evolve_table[i].name,
        warlock_evolve_table[i].name,
        (recursive) ? "" : "  ",
        warlock_evolve_table[i].hps,
        warlock_evolve_table[i].mana,
        warlock_evolve_table[i].move,
        warlock_evolve_table[i].exp,
        warlock_evolve_table[i].gold);
      strcat(evolve->error, buf);

      found = TRUE;

      if (!recursive) 
        show_warlock_evolves(ch, i, evolve, TRUE);
    }
  }

  if (base == -1 && !found)
  {
    sprintf(evolve->error, "You are unable to evolve.<BR>");
  }
}

void warlock_evolve(CHAR_DATA *ch, char *argument, EVOLVE_DATA *evolve)
{
  int i;

  if (argument[0] == '\0')
  {
    show_warlock_evolves(ch, -1, evolve, FALSE);
    return;
  }

  for (i = 0; warlock_evolve_table[i].name[0] != '\0'; i++)
  {
    int *reqfield = &ch->pcdata->powers[warlock_evolve_table[i].req_field];
    int *evolvefield = &ch->pcdata->powers[warlock_evolve_table[i].evolve_field];
    int *opposefield = &ch->pcdata->powers[warlock_evolve_table[i].oppose_field];

    if (IS_SET(*evolvefield, warlock_evolve_table[i].evolve_bit))
      continue;
    if (warlock_evolve_table[i].oppose_bit != 0 && IS_SET(*opposefield, warlock_evolve_table[i].oppose_bit))
      continue;
    if (warlock_evolve_table[i].req_bit != 0 && !IS_SET(*reqfield, warlock_evolve_table[i].req_bit))
      continue;

    if (!str_cmp(argument, warlock_evolve_table[i].name))
      break;
  }

  if (warlock_evolve_table[i].name[0] == '\0')
  {
    sprintf(evolve->error, "There is no evolve by that name.<BR>");
    return;
  }

  /* set the evolve data */
  evolve->hps   =  warlock_evolve_table[i].hps;
  evolve->mana  =  warlock_evolve_table[i].mana;
  evolve->move  =  warlock_evolve_table[i].move;
  evolve->exp   =  warlock_evolve_table[i].exp;
  evolve->gold  =  warlock_evolve_table[i].gold;
  evolve->field =  &ch->pcdata->powers[warlock_evolve_table[i].evolve_field];
  evolve->bit   =  warlock_evolve_table[i].evolve_bit;
  evolve->valid =  TRUE;
}

void warlock_commands(CHAR_DATA *ch)
{
  BUFFER *buf = buffer_new(MAX_STRING_LENGTH);
  char enchantment[MAX_STRING_LENGTH]; int enccount = 0;
  char invocation[MAX_STRING_LENGTH]; int invcount = 0;
  char summoning[MAX_STRING_LENGTH]; int sumcount = 0;
  char abjuration[MAX_STRING_LENGTH]; int abjcount = 0;
  char necromancy[MAX_STRING_LENGTH]; int neccount = 0;
  char divination[MAX_STRING_LENGTH]; int divcount = 0;
  char evolve[MAX_STRING_LENGTH]; int evocount = 0;
  char generic[MAX_STRING_LENGTH]; int gencount = 0;
  int cmd;

  bprintf(buf, "%s\n\r", get_dystopia_banner("    Powers    ", 76));

  for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
  {
    if (cmd_table[cmd].race != CLASS_WARLOCK)
      continue;

    /* check to see if the player has actually learned the power */
    if (!can_use_command(ch, cmd))
      continue;

    switch(cmd_table[cmd].powertype)
    {
      default:
        bug("warlock_commands: cmd %d unknown powertype.", cmd);
        break;
      case 0:
        if (gencount == 0)
          sprintf(generic, " %-15s :", "Magi Powers");
        strcat(generic, " ");
        strcat(generic, cmd_table[cmd].name);
        gencount++;
        break;
      case SPHERE_ENCHANTMENT:
        if (enccount == 0)
          sprintf(enchantment, " %-15s :", "Enchantments");
        strcat(enchantment, " ");
        strcat(enchantment, cmd_table[cmd].name);
        enccount++;
        break;
      case SPHERE_DIVINATION:
        if (divcount == 0)
          sprintf(divination, " %-15s :", "Divinations");
        strcat(divination, " ");
        strcat(divination, cmd_table[cmd].name);
        divcount++;
        break;
      case SPHERE_NECROMANCY:
        if (neccount == 0)
          sprintf(necromancy, " %-15s :", "Necromantics");
        strcat(necromancy, " ");
        strcat(necromancy, cmd_table[cmd].name);
        neccount++;
        break;
      case SPHERE_SUMMONING:
        if (sumcount == 0)
          sprintf(summoning, " %-15s :", "Summonings");
        strcat(summoning, " ");
        strcat(summoning, cmd_table[cmd].name);
        sumcount++;
        break;
      case SPHERE_INVOCATION:
        if (invcount == 0)
          sprintf(invocation, " %-15s :", "Invocations");
        strcat(invocation, " ");
        strcat(invocation, cmd_table[cmd].name);
        invcount++;
        break;
      case SPHERE_ABJURATION:
        if (abjcount == 0)
          sprintf(abjuration, " %-15s :", "Abjurations");
        strcat(abjuration, " ");
        strcat(abjuration, cmd_table[cmd].name);
        abjcount++;
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
  if (abjcount > 0)
    bprintf(buf, "%19.19s%s\n\r", abjuration, line_indent(&abjuration[18], 19, 75));
  if (divcount > 0)
    bprintf(buf, "%19.19s%s\n\r", divination, line_indent(&divination[18], 19, 75));
  if (sumcount > 0)
    bprintf(buf, "%19.19s%s\n\r", summoning, line_indent(&summoning[18], 19, 75));
  if (neccount > 0)
    bprintf(buf, "%19.19s%s\n\r", necromancy, line_indent(&necromancy[18], 19, 75));
  if (invcount > 0)
    bprintf(buf, "%19.19s%s\n\r", invocation, line_indent(&invocation[18], 19, 75));
  if (enccount > 0)
    bprintf(buf, "%19.19s%s\n\r", enchantment, line_indent(&enchantment[18], 19, 75));
  if (evocount > 0)
    bprintf(buf, "%19.19s%s\n\r", evolve, line_indent(&evolve[18], 19, 75));

  bprintf(buf, "%s\n\r", get_dystopia_banner("", 76));

  send_to_char(buf->data, ch);
  buffer_free(buf);
}

void do_doombolt(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK)
   || !IS_SET(ch->pcdata->powers[EVOLVE_1], WARLOCK_EVOLVE_DOOMBOLT))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (!str_cmp(arg, "off"))
  {
    if (!IS_SET(ch->pcdata->tempflag, TEMP_DOOMBOLT))
    {
      send_to_char("You haven't got an active doombolt anywhere.\n\r", ch);
      return;
    }

    REMOVE_BIT(ch->pcdata->tempflag, TEMP_DOOMBOLT);

    event = alloc_event();
    event->fun = &event_dummy;
    event->type = EVENT_PLAYER_DOOMBOLT;
    add_event_char(event, ch, 20 * PULSE_PER_SECOND);

    send_to_char("Ok. Doombolt deactived.\n\r", ch);
    return;
  }

  if (IS_SET(ch->pcdata->tempflag, TEMP_DOOMBOLT))
  {
    send_to_char("You already have one active doombolt.\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_DOOMBOLT)) != NULL)
  {
    printf_to_char(ch, "You cannot use this power for another %s.\n\r", event_time_left(event));
    return;
  }

  if (ch->in_room == NULL)
  {
    send_to_char("You can only use this power when you are standing in a room.\n\r", ch);
    return;
  }

  SET_BIT(ch->pcdata->tempflag, TEMP_DOOMBOLT);
  sprintf(arg, "%d %s", ch->pcdata->playerid, (IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_DOOMSTORM)) ? "storm" : "bolt");

  event = alloc_event();
  event->argument = str_dup(arg);
  event->fun = &event_room_doombolt;
  event->type = EVENT_ROOM_DOOMBOLT;
  add_event_room(event, ch->in_room, 3 * PULSE_PER_SECOND);

  act("$n calls down a bolt of pure chaos.", ch, NULL, NULL, TO_ROOM);
  act("You call down a bolt of pure chaos.", ch, NULL, NULL, TO_CHAR);
}

bool event_room_doombolt(EVENT_DATA *event)
{
  CHAR_DATA *ch, *gch;
  ITERATOR *pIter;
  ROOM_INDEX_DATA *pRoom;
  char buf[MAX_INPUT_LENGTH];
  char *ptr;
  int id = (event->argument) ? atoi(event->argument) : 0;
  int cost = 750;
  bool doomstorm = FALSE;

  if ((pRoom = event->owner.room) == NULL)
  {
    bug("event_room_doombolt: no owner.", 0);
    return FALSE;
  }

  /* check for doomstorm enhancement */
  ptr = one_argument(event->argument, buf);
  if (!str_cmp(ptr, "storm"))
    doomstorm = TRUE;

  if ((ch = get_online_player(id)) == NULL)
  {
    if ((gch = (CHAR_DATA *) FirstInList(pRoom->people)) != NULL)
      act("The doombolt dissipates from the room.", gch, NULL, NULL, TO_ALL);

    /* try to find the player anyway (linkdead) */
    pIter = AllocIterator(char_list);
    while ((ch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (!IS_NPC(ch) && ch->pcdata->playerid == id)
        break;
    }

    if (ch != NULL)
      REMOVE_BIT(ch->pcdata->tempflag, TEMP_DOOMBOLT);

    return FALSE;
  }

  if (!IS_SET(ch->pcdata->tempflag, TEMP_DOOMBOLT))
  {
    if ((gch = (CHAR_DATA *) FirstInList(pRoom->people)) != NULL)
      act("The doombolt dissipates from the room.", gch, NULL, NULL, TO_ALL);

    return FALSE;
  }

  if (ch->mana < cost)
  {
    if (ch->in_room != pRoom)
      send_to_char("Your doombolt fades from existance.\n\r", ch);

    if ((gch = (CHAR_DATA *) FirstInList(pRoom->people)) != NULL)
      act("The doombolt dissipates from the room.", gch, NULL, NULL, TO_ALL);

    REMOVE_BIT(ch->pcdata->tempflag, TEMP_DOOMBOLT);

    return FALSE;
  }
  modify_mana(ch, -1 * cost);

  if (ch->in_room == pRoom)
  {
    pIter = AllocIterator(pRoom->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (!IS_NPC(gch) || ch == gch) continue;

      if (number_percent() >= 25)
        one_hit(ch, gch, gsn_doombolt, 0);
      if (!gch->dead && number_percent() >= 75)
        one_hit(ch, gch, gsn_doombolt, 0);
      if (!gch->dead && doomstorm)
        one_hit(ch, gch, gsn_doombolt, 0);
    }
  }

  sprintf(buf, "%d %s", id, (doomstorm) ? "storm" : "bolt");
  event = alloc_event();
  event->argument = str_dup(buf);
  event->fun = &event_room_doombolt;
  event->type = EVENT_ROOM_DOOMBOLT;
  add_event_room(event, pRoom, number_range(3, 6) * PULSE_PER_SECOND);

  return FALSE;
}

bool event_object_doomcharge(EVENT_DATA *event)
{
  OBJ_DATA *obj;
  CHAR_DATA *ch;
  char buf[MAX_INPUT_LENGTH];

  if ((obj = event->owner.obj) == NULL)
  {
    bug("event_object_doomcharge: no owner.", 0);
    return FALSE;
  }

  if ((ch = obj->carried_by) != NULL && obj->wear_loc != WEAR_NONE)
  {
    if (ch->mana < ch->max_mana)
    {
      modify_mana(ch, 1000);
      act("$p pulses, and 1000 mana is channeled into your body.", ch, obj, NULL, TO_CHAR);
    }
  }

  if (event->argument && atoi(event->argument) >= 1000)
  {
    sprintf(buf, "%d", atoi(event->argument) - 1000);

    event = alloc_event();
    event->fun = &event_object_doomcharge;
    event->type = EVENT_OBJECT_DOOMCHARGE;
    event->argument = str_dup(buf);
    add_event_object(event, obj, PULSE_PER_SECOND);
  }

  return FALSE;
}

void do_doomcharge(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int delay;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK)
   || !IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_DOOMCHARGE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  argument = one_argument(argument, arg);
  if ((delay = atoi(argument)) < 1 || delay > 200 || arg[0] == '\0')
  {
    send_to_char("Syntax: doomcharge [item] [time]\n\rWhere [time] is between 1 and 200.\n\r", ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You are not carrying that item.\n\r", ch);
    return;
  }

  if (event_isset_object(obj, EVENT_OBJECT_DOOMCHARGE))
  {
    send_to_char("This item has already been charged with mystical energies.\n\r", ch);
    return;
  }

  if (ch->mana < 3000)
  {
    send_to_char("You do not have enough mana to make the spell work.\n\r", ch);
    return;
  }

  act("$n infuses $p with a portion of $s mystical energies.", ch, obj, NULL, TO_ROOM);
  act("You infuse $p with a portion of your mystical energies.", ch, obj, NULL, TO_CHAR);

  sprintf(arg, "%d", UMIN(ch->mana / 3, 20000));
  modify_mana(ch, -1 * (UMIN(ch->mana / 3, 20000)));

  event = alloc_event();
  event->fun = &event_object_doomcharge;
  event->type = EVENT_OBJECT_DOOMCHARGE;
  event->argument = str_dup(arg);
  add_event_object(event, obj, delay * PULSE_PER_SECOND);
}

void do_disjunction(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA *paf;
  EVENT_DATA *event;
  ITERATOR *pIter;
  int cost = 1500;
  bool found = FALSE;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK)
   || !IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_DISJUNCTION))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0' || strlen(arg) < 3)
  {
    send_to_char("Try to disjunct what spell or affect?\n\r", ch);
    send_to_char("Valid special affects are : bloodacid, confusion & heatmetal.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to cast disjunction.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  printf_to_char(ch, "You attempt to dispel all '%s' affects.\n\r", arg);
  act("$n attempts a disjunction spell.", ch, NULL, NULL, TO_ROOM);

  pIter = AllocIterator(ch->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_prefix(arg, skill_table[paf->type].name))
    {
      if (number_percent() >= 20)
      {
        found = TRUE;
        if (paf->type > 0 && paf->type < MAX_SKILL && skill_table[paf->type].msg_off)
        {
          send_to_char(skill_table[paf->type].msg_off, ch);
          send_to_char("\n\r", ch);
        }
        affect_remove(ch, paf);
      }
    }
  }

  pIter = AllocIterator(ch->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    switch(event->type)
    {
      default:
        break;
      case EVENT_PLAYER_HEATMETAL:
        if (!str_cmp(arg, "heatmetal") && number_percent() >= 20)
        {
          act("$n's equipment cools down.", ch, NULL, NULL, TO_ROOM);
          send_to_char("Your equipment starts cooling down.\n\r", ch);
          dequeue_event(event, TRUE);
          found = TRUE;
        }
        break;
      case EVENT_MOBILE_CONFUSED:
        if (!str_cmp(arg, "confusion") && number_percent() >= 20)
        {
          act("$n growls and clenches $s fists.", ch, NULL, NULL, TO_ROOM);
          send_to_char("You regain control of your own body.\n\r", ch);
          dequeue_event(event, TRUE);
          found = TRUE;
        }
        break;
      case EVENT_MOBILE_ACIDBLOOD:
        if (!str_cmp(arg, "bloodacid") && number_percent() >= 20)
        {
          act("$n expels the acid from $s blood.", ch, NULL, NULL, TO_ROOM);
          send_to_char("Your bloodstream is no longer floated with acid.\n\r", ch);
          dequeue_event(event, TRUE);
          found = TRUE;
        }
        break;
    }
  }

  if (!found)
    send_to_char("Nothing happens.\n\r", ch);

  WAIT_STATE(ch, 8);
}

void do_mking(CHAR_DATA *ch, char *argument)
{
  int cost = 1000;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK)
   || !IS_SET(ch->pcdata->powers[EVOLVE_1], WARLOCK_EVOLVE_MOUNTAIN))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  TOGGLE_BIT(ch->newbits, NEW_MOUNTAIN);

  if (IS_SET(ch->newbits, NEW_MOUNTAIN))
  {
    if (ch->move < cost)
    {
      REMOVE_BIT(ch->newbits, NEW_MOUNTAIN);
      send_to_char("You do not have enough movementpoints.\n\r", ch);
      return;
    }
    modify_move(ch, -1 * cost);

    act("$n's skin turns into a grey rocklike substance.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You are the King of the Mountain!!!\n\r", ch);
  }
  else
  {
    act("$n's skin returns to its normal colour.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You revert from your rockform into human.\n\r", ch);
  }
}

CHAR_DATA *get_char_homingdevice(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *wch;
  ITERATOR *pIter, *pIter2;
  OBJ_DATA *obj;
  int number, count = 0;

  if ((wch = get_char_room(ch, argument)) != NULL)
    return wch;

  number = number_argument(argument, arg);

  /* only PC's for now */
  if (IS_NPC(ch)) return NULL;

  pIter = AllocIterator(char_list);
  while ((wch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    bool homer = FALSE;

    if (wch->in_room == NULL) continue;
    if (!can_see(ch, wch) || !is_name(arg, wch->name)) continue;

    pIter2 = AllocIterator(wch->in_room->contents);
    while ((obj = (OBJ_DATA *) NextInList(pIter2)) != NULL)
    {
      if (obj->item_type == ITEM_HOMING && obj->ownerid == ch->pcdata->playerid)
        homer = TRUE;
    }

    if (!homer) continue;

    if (++count == number)
      return wch;
  }

  return NULL;
}

void do_sunset(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];
  int cost = 3500;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_OLDAGE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Whom do you wish to age?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (ch == victim)
  {
    send_to_char("You do not want to age yourself.\n\r", ch);
    return;
  }

  if (event_isset_mobile(victim, EVENT_MOBILE_SUNSET))
  {
    send_to_char("They are already ravaged by the forces of time.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    send_to_char("You do not have enough mana.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * cost);

  event = alloc_event();
  event->fun = &event_mobile_sunset;
  event->type = EVENT_MOBILE_SUNSET;
  event->argument = str_dup("1");
  add_event_char(event, victim, PULSE_PER_SECOND);

  act("You cast your spell of age on $N.", ch, NULL, victim, TO_CHAR);
  act("$n waves $s fingers at $N, emitting sparks in the air.", ch, NULL, victim, TO_NOTVICT);
  act("$n waves $s fingers at you, and you feel suddenly strange.", ch, NULL, victim, TO_VICT);
}

bool event_mobile_sunset(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  ITERATOR *pIter;
  char buf[MAX_INPUT_LENGTH];
  int i;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_sunset: no owner.", 0);
    return FALSE;
  }

  if (event->argument == NULL || event->argument[0] == '\0')
  {
    bug("event_mobile_sunset: no argument.", 0);
    return FALSE;
  }

  i = atoi(event->argument);

  if (i++ <= 4)
  {
    AFFECT_DATA *af;
    AFFECT_DATA paf;
    int sn;

    if ((sn = skill_lookup("sunset age")) > 0)
    {
      paf.type      = sn;
      paf.duration  = 40;
      paf.location  = APPLY_DAMROLL;
      paf.modifier  = -1 * number_range(15, 40);
      paf.bitvector = 0;
      affect_to_char(ch, &paf);
    }
    else
    {
      bug("event_mobile_sunset: no sunset age spell.", 0);
      return FALSE;
    }

    /* age all spells on this player */
    pIter = AllocIterator(ch->affected);
    while ((af = (AFFECT_DATA *) NextInList(pIter)) != NULL)
    {
      if (af->type != sn)
      {
        af->duration = UMAX(0, af->duration - number_range(10, 20));
      }
    }

    send_to_char("You grow older and older.\n\r", ch);
    act("$n's hair grows more white and $s skin becomes more wrinkled.", ch, NULL, NULL, TO_ROOM);

    sprintf(buf, "%d", i);
    event = alloc_event();
    event->fun = &event_mobile_sunset;
    event->type = EVENT_MOBILE_SUNSET;
    event->argument = str_dup(buf);
    add_event_char(event, ch, 3 * PULSE_PER_SECOND);
  }

  return FALSE;
}

void do_timetrip(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_TIMETRAVEL))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (event_isset_mobile(ch, EVENT_PLAYER_TIMETRIP_WAIT))
  {
    if (event_isset_mobile(ch, EVENT_PLAYER_TIMETRIP))
      send_to_char("You are already tripping through time.\n\r", ch);
    else
      send_to_char("You are still recovering from your last timetrip.\n\r", ch);
    return;
  }

  sprintf(buf, "%d %d %d", ch->hit, ch->move, ch->mana);

  /* set delay */
  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_PLAYER_TIMETRIP_WAIT;
  add_event_char(event, ch, 60 * PULSE_PER_SECOND);

  /* set timetrip event */
  event = alloc_event();
  event->fun = &event_player_timetrip;
  event->type = EVENT_PLAYER_TIMETRIP;
  event->argument = str_dup(buf);
  add_event_char(event, ch, 13);

  send_to_char("You fork the river of time.\n\r", ch);
  act("$n wavers for an instance as $s timeline is forked.", ch, NULL, NULL, TO_ROOM);
}

bool event_player_timetrip(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  char *ptr = event->argument;
  char buf[MAX_INPUT_LENGTH];
  int i;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_timetrip: no owner.", 0);
    return FALSE;
  }

  if (event->argument == NULL || event->argument[0] == '\0')
  {
    bug("event_player_timetrip: no argument.", 0);
    return FALSE;
  }

  if (ch->hit < 1)
  {
    send_to_char("Your timeline clashes with reality.\n\r", ch);
    return FALSE;
  }

  ptr = one_argument(ptr, buf);
  i = atoi(buf);
  ch->hit = (92 * i) / 100;

  ptr = one_argument(ptr, buf);
  i = atoi(buf);
  ch->move = (92 * i) / 100;

  ptr = one_argument(ptr, buf);
  i = atoi(buf);
  ch->mana = (92 * i) / 100;

  send_to_char("Your timelines become intertwined.\n\r", ch);

  return FALSE;
}

void do_meditate(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  send_to_char("You sit down, cross your legs and start meditating.\n\r", ch);
  act("$n sits down, crosses $s legs and starts meditating.", ch, NULL, NULL, TO_ROOM);
  ch->position = POS_MEDITATING;
}

void update_archmage(CHAR_DATA *ch)
{
  if (IS_NPC(ch) || ch->class != CLASS_WARLOCK || ch->level > 6)
    return;

  /* fix archmages that has lost their titles */
  if (ch->pcdata->powers[WARLOCK_RANK] == WLCK_RNK_ARCHMAGE)
  {
    if (str_cmp(archmage_list[ch->pcdata->powers[WARLOCK_PATH]].player, ch->name))
    {
      ch->pcdata->powers[WARLOCK_RANK] = WLCK_RNK_WARLOCK;
    }
  }

  if (ch->pcdata->powers[WARLOCK_RANK] == WLCK_RNK_WARLOCK)
  {
    if (!str_cmp(archmage_list[ch->pcdata->powers[WARLOCK_PATH]].player, "noone"))
    {
      free_string(archmage_list[ch->pcdata->powers[WARLOCK_PATH]].player);
      archmage_list[ch->pcdata->powers[WARLOCK_PATH]].player = str_dup(ch->name);
      ch->pcdata->powers[WARLOCK_RANK] = WLCK_RNK_ARCHMAGE;
      save_archmages();
    }
  }
}

void load_archmages()
{
  FILE *fp;

  if ((fp = fopen("../txt/archmages.txt", "r")) == NULL)
  {
    bug("load_archmages: cannot open archmages.txt", 0);
    abort();
  }

  archmage_list[0].player = fread_string(fp);
  archmage_list[PATH_NECROMANCY].player = fread_string(fp);
  archmage_list[PATH_NECROMANCY].active = fread_number(fp);
  archmage_list[PATH_ABJURATION].player = fread_string(fp);
  archmage_list[PATH_ABJURATION].active = fread_number(fp);
  archmage_list[PATH_INVOCATION].player = fread_string(fp);
  archmage_list[PATH_INVOCATION].active = fread_number(fp);
  archmage_list[PATH_DIVINATION].player = fread_string(fp);
  archmage_list[PATH_DIVINATION].active = fread_number(fp);
  archmage_list[PATH_ENCHANTMENT].player = fread_string(fp);
  archmage_list[PATH_ENCHANTMENT].active = fread_number(fp);
  archmage_list[PATH_SUMMONING].player = fread_string(fp);
  archmage_list[PATH_SUMMONING].active = fread_number(fp);

  fclose(fp);
}

void save_archmages()
{
  FILE *fp;

  if ((fp = fopen("../txt/archmages.txt", "w")) == NULL)
  {
    bug("save_archmages: cannot open archmages.txt", 0);
    return;
  }

  fprintf(fp, "ARCHMAGES~\n");
  fprintf(fp, "%s~ %d\n", archmage_list[PATH_NECROMANCY].player, archmage_list[PATH_NECROMANCY].active);
  fprintf(fp, "%s~ %d\n", archmage_list[PATH_ABJURATION].player, archmage_list[PATH_ABJURATION].active);
  fprintf(fp, "%s~ %d\n", archmage_list[PATH_INVOCATION].player, archmage_list[PATH_INVOCATION].active);
  fprintf(fp, "%s~ %d\n", archmage_list[PATH_DIVINATION].player, archmage_list[PATH_DIVINATION].active);
  fprintf(fp, "%s~ %d\n", archmage_list[PATH_ENCHANTMENT].player, archmage_list[PATH_ENCHANTMENT].active);
  fprintf(fp, "%s~ %d\n", archmage_list[PATH_SUMMONING].player, archmage_list[PATH_SUMMONING].active);

  fclose(fp);
}

void do_archmages(CHAR_DATA *ch, char *argument)
{
  BUFFER *buf;

  if (IS_NPC(ch) || (!IS_CLASS(ch, CLASS_WARLOCK) && ch->level < 7))
  { 
    do_huh(ch, "");  
    return;
  }

  buf = buffer_new(MAX_STRING_LENGTH);

  bprintf(buf, " %s\n\n\r", get_dystopia_banner("Archmages", 40));

  bprintf(buf, "   [%3d%%] Necromancy        %s\n\r",
    40 * archmage_list[PATH_NECROMANCY].active / (muddata.mudinfo[MUDINFO_UPDATED] * 3 + 1),
    archmage_list[PATH_NECROMANCY].player);
  bprintf(buf, "   [%3d%%] Abjuration        %s\n\r",
    40 * archmage_list[PATH_ABJURATION].active / (muddata.mudinfo[MUDINFO_UPDATED] * 3 + 1),
    archmage_list[PATH_ABJURATION].player);
  bprintf(buf, "   [%3d%%] Invocation        %s\n\r",
    40 * archmage_list[PATH_INVOCATION].active / (muddata.mudinfo[MUDINFO_UPDATED] * 3 + 1),
    archmage_list[PATH_INVOCATION].player);
  bprintf(buf, "   [%3d%%] Divination        %s\n\r",
    40 * archmage_list[PATH_DIVINATION].active / (muddata.mudinfo[MUDINFO_UPDATED] * 3 + 1),
    archmage_list[PATH_DIVINATION].player);
  bprintf(buf, "   [%3d%%] Enchantment       %s\n\r",
    40 * archmage_list[PATH_ENCHANTMENT].active / (muddata.mudinfo[MUDINFO_UPDATED] * 3 + 1),
    archmage_list[PATH_ENCHANTMENT].player);
  bprintf(buf, "   [%3d%%] Summoning         %s\n\r",
    40 * archmage_list[PATH_SUMMONING].active / (muddata.mudinfo[MUDINFO_UPDATED] * 3 + 1),
    archmage_list[PATH_SUMMONING].player);

  bprintf(buf, "\n\r %s\n\r", get_dystopia_banner("", 40));

  send_to_char(buf->data, ch);
  buffer_free(buf);
}

void do_earthmother(CHAR_DATA *ch, char *argument)
{
  AREA_AFFECT af;
  AREA_DATA *pArea;
  const int cost = 1500;
  EVENT_DATA *event;
  char buf[MAX_STRING_LENGTH];

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }
  
  /* check for correct enchantment level */
  if (ch->pcdata->powers[SPHERE_ENCHANTMENT] < 2)
  {
    send_to_char("Your level of expertise in enchantments is to low.\n\r", ch);
    return;
  }

  if (!ch->in_room || event_isset_area((pArea = ch->in_room->area), EVENT_AREA_EARTHMOTHER))
  {
    send_to_char("This area already has an earthmother enchantment laid out.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    send_to_char("You do not have enough mana.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * cost);

  send_to_char("You lay the earthmother enchantment on this area.\n\r", ch);
  act("$n draws a circle on the ground, and invoke the name of the earthmother.", ch, NULL, NULL, TO_ROOM);

  sprintf(buf, "%d", ch->pcdata->playerid);
  event = alloc_event();
  event->argument = str_dup(buf);
  event->type = EVENT_AREA_EARTHMOTHER;
  event->fun = &event_area_earthmother;
  add_event_area(event, pArea, 8 * PULSE_PER_SECOND);

  af.type      = AREA_AFF_EARTHMOTHER;
  af.duration  = number_range(15, 25);
  af.level     = ch->pcdata->powers[SPHERE_ENCHANTMENT];
  af.owner     = ch->pcdata->playerid;
  affect_to_area(pArea, &af);
}

void do_milkandhoney(CHAR_DATA *ch, char *argument)
{
  AREA_AFFECT af;  
  AREA_DATA *pArea;
  const int cost = 1500;
  EVENT_DATA *event;
  char buf[MAX_STRING_LENGTH];

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct enchantment level */
  if (ch->pcdata->powers[SPHERE_ENCHANTMENT] < 4)
  {
    send_to_char("Your level of expertise in enchantments is to low.\n\r", ch);
    return;
  }

  if (!ch->in_room || event_isset_area((pArea = ch->in_room->area), EVENT_AREA_MILKANDHONEY))
  {
    send_to_char("This area already has a milk and honey enchantment laid out.\n\r", ch);
    return;
  }

  if (in_arena(ch))
  {
    send_to_char("You cannot cast milk and honey in the arena.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    send_to_char("You do not have enough mana.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * cost);

  send_to_char("You lay the land of milk and honey enchantment.\n\r", ch);
  act("$n calls the blessing of the goddess of luck upon this place.", ch, NULL, NULL, TO_ROOM);

  sprintf(buf, "%d", ch->pcdata->playerid);
  event = alloc_event();
  event->argument = str_dup(buf);
  event->type = EVENT_AREA_MILKANDHONEY;
  event->fun = &event_area_milkandhoney;
  add_event_area(event, pArea, 8 * PULSE_PER_SECOND);

  af.type      = AREA_AFF_MILKANDHONEY;
  af.duration  = number_range(15, 25);
  af.level     = ch->pcdata->powers[SPHERE_ENCHANTMENT];
  af.owner     = ch->pcdata->playerid;
  affect_to_area(pArea, &af);
}

void do_ironmind(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  /* check for correct invocation level */
  if (ch->pcdata->powers[SPHERE_ABJURATION] < 6)
  {
    send_to_char("Your level of expertise in abjuration is to low.\n\r", ch);
    return;
  }
  if (IS_SET(ch->newbits, NEW_IRONMIND))
  {
    send_to_char("Your already focusing your mental powers.\n\r", ch);
    return;
  }
  SET_BIT(ch->newbits, NEW_IRONMIND);
  send_to_char("You focus your concentration on absorbing damage.\n\r", ch);
  WAIT_STATE(ch, 6);
}

void do_plague(CHAR_DATA *ch, char *argument)
{
  AREA_AFFECT af;
  AREA_DATA *pArea;
  const int cost = 1500;
  EVENT_DATA *event;
  char buf[MAX_STRING_LENGTH];

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct enchantment level */
  if (ch->pcdata->powers[SPHERE_ENCHANTMENT] < 3)
  {
    send_to_char("Your level of expertise in enchantments is to low.\n\r", ch);
    return;
  }

  if (!ch->in_room || event_isset_area((pArea = ch->in_room->area), EVENT_AREA_PLAGUE))
  {
    send_to_char("This area already has an plague enchantment laid out.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    send_to_char("You do not have enough mana.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * cost);

  send_to_char("You lay an enchantment of plague on this area.\n\r", ch);
  act("$n emits a dark haze from $s body, while uttering gutteral words.", ch, NULL, NULL, TO_ROOM);

  sprintf(buf, "%d", ch->pcdata->playerid);
  event = alloc_event();
  event->argument = str_dup(buf);
  event->type = EVENT_AREA_PLAGUE;
  event->fun = &event_area_plague;
  add_event_area(event, pArea, 8 * PULSE_PER_SECOND);

  af.type      = AREA_AFF_PLAGUE;
  af.duration  = number_range(15, 25);
  af.level     = ch->pcdata->powers[SPHERE_ENCHANTMENT];
  af.owner     = ch->pcdata->playerid;
  affect_to_area(pArea, &af);
}

void do_steelfists(CHAR_DATA *ch, char *argument)
{
  int sn, level;
  const int mana_cost = 2500;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct enchantment level */
  if (ch->pcdata->powers[SPHERE_ENCHANTMENT] < 1)
  {
    send_to_char("Your level of expertise in enchantments is to low.\n\r", ch);
    return;
  }

  if (ch->mana < mana_cost)
  {
    printf_to_char(ch, "You need %d more mana to cast fists of steel.\n\r", mana_cost - ch->mana);
    return;
  }

  if ((sn = skill_lookup("steel fists")) <= 0)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }

  level = ch->spl[0] + ch->spl[1] + ch->spl[2] + ch->spl[3] + ch->spl[4];
  level = UMAX(level, 150);

  (*skill_table[sn].spell_fun) (sn, level, ch, ch);
  WAIT_STATE(ch, 6);
  modify_mana(ch, -1 * mana_cost);
}

void do_displacement(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  const int cost = 2500;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct enchantment level */
  if (ch->pcdata->powers[SPHERE_ENCHANTMENT] < 5)
  {
    send_to_char("Your level of expertise in enchantments is to low.\n\r", ch);
    return;
  }
  if (event_isset_mobile(ch, EVENT_PLAYER_DISPLACE))
  {
    strip_event_mobile(ch, EVENT_PLAYER_DISPLACE);
    send_to_char("You dispel your displacement charm.\n\r", ch);
    return;
  }
  if (ch->mana < cost)
  {
    send_to_char("You do not have enough mana.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * cost);

  send_to_char("You cast the displacement charm.\n\r", ch);
  act("$n fades out of focus.", ch, NULL, NULL, TO_ROOM);

  event = alloc_event();
  event->fun = &event_player_displace;
  event->type = EVENT_PLAYER_DISPLACE;
  add_event_char(event, ch, 5 * PULSE_PER_SECOND);
}

bool event_player_displace(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  EVENT_DATA *newevent;
  const int cost = 650;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_displace: no owner.", 0);
    return FALSE;
  }

  if (ch->mana < cost)
  {
    send_to_char("Your displacement charm fades due to lack of mana.\n\r", ch);
    return FALSE;
  }
  modify_mana(ch, -1 * cost);

  newevent = alloc_event();
  newevent->fun = &event_player_displace;
  newevent->type = EVENT_PLAYER_DISPLACE;
  add_event_char(newevent, ch, 5 * PULSE_PER_SECOND);

  return FALSE;
}

void do_readaura(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  BUFFER *buf;
  char arg[MAX_INPUT_LENGTH];
  int mana_cost = 250;

  /* check for warlock PC class */
  if (IS_NPC(ch) || (!IS_CLASS(ch, CLASS_WARLOCK) && !IS_CLASS(ch, CLASS_FAE)))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct divination level */
  if (IS_CLASS(ch, CLASS_WARLOCK) && ch->pcdata->powers[SPHERE_DIVINATION] < 2)
  {
    send_to_char("Your level of expertise in divination is to low.\n\r", ch);
    return;
  }

  /* check for arcane level */
  if (IS_CLASS(ch, CLASS_FAE) && ch->pcdata->powers[DISC_FAE_ARCANE] < 3)
  {
    send_to_char("You need level 3 discipline in arcane.\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Who's aura do you wish to read?\n\r", ch);
    return;
  }

  /* get the victim */
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (IS_IMMUNE(victim, IMM_SHIELDED))
  {
    send_to_char("Something is blocking your attempt to read their aura.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  mana_cost = reduce_cost(ch, victim, mana_cost, eMana);

  /* check mana cost */
  if (ch->mana <= mana_cost)
  {
    send_to_char("You do not have enough mana to read their aura.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * mana_cost);

  buf = buffer_new(MAX_STRING_LENGTH);
  bprintf(buf, " %s\n\r", get_dystopia_banner("Aura Reading", 60));
  bprintf(buf, " Name: %s\n\n\r",
    (IS_NPC(victim)) ? victim->short_descr : victim->name);
  bprintf(buf, " Hit: %d/%d Mana: %d/%d Move: %d/%d\n\r",
    victim->hit, victim->max_hit, victim->mana, victim->max_mana, victim->move, victim->max_move);
  bprintf(buf, " Damroll: %d  Hitroll: %d  Armor: %d  Damcap: %d\n\r",
    char_damroll(victim), char_hitroll(victim), char_ac(victim), victim->damcap[DAM_CAP]);
  bprintf(buf, " %s\n\r", get_dystopia_banner("", 60));

  send_to_char(buf->data, ch);
  buffer_free(buf);
}

void do_corpsedrain(CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj;
  ITERATOR *pIter;
  bool found = FALSE;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct necromancy level */
  if (ch->pcdata->powers[SPHERE_NECROMANCY] < 1)
  {
    send_to_char("Your level of expertise in necromancy is to low.\n\r", ch);
    return;
  }

  pIter = AllocIterator(ch->in_room->contents);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
      continue;

    if (obj->item_type == ITEM_CORPSE_NPC)
      modify_mana(ch, number_range(50, 100));
    else
      modify_mana(ch, number_range(500, 1000));

    act("You drain the last lifeforce from $p.", ch, obj, NULL, TO_CHAR);
    act("$n drains the last lifeforce from $p.", ch, obj, NULL, TO_ROOM);

    found = TRUE;
    extract_obj(obj);
  }

  if (!found)
  {
    send_to_char("You are unable to find any suitable corpses in this room.\n\r", ch);
  }
  else
  {
    WAIT_STATE(ch, 12);
  }
}

bool event_player_catalyst(EVENT_DATA *event)
{
  EVENT_DATA *newevent;
  CHAR_DATA *ch;
  const int cost = 200;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_catalyst: no owner.", 0);
    return FALSE;
  }
  if (ch->mana < cost)
  {
    send_to_char("Your catalyst fades.\n\r", ch);
    return FALSE;
  }
  modify_mana(ch, -1 * cost);

  newevent = alloc_event();
  newevent->argument = str_dup(event->argument);
  newevent->type = EVENT_PLAYER_CATALYST;
  newevent->fun = &event_player_catalyst;
  add_event_char(newevent, ch, 3 * PULSE_PER_SECOND);

  return FALSE;
}

void do_flamberge(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *gch;
  ITERATOR *pIter;
  const int mana_cost = 750;
  int sn;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct invocation level */
  if (ch->pcdata->powers[SPHERE_INVOCATION] < 1)
  {
    send_to_char("Your level of expertise in invocation is to low.\n\r", ch);
    return;
  }

  if (ch->mana < mana_cost)
  {
    send_to_char("You do not have enough mana to cast flamberge.\n\r", ch);
    return;
  }

  if ((sn = skill_lookup("flamberge")) < 0)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }

  modify_mana(ch, -1 * mana_cost);

  act("You spread your fingers and engulf the room in flames.", ch, NULL, NULL, TO_CHAR);
  act("$n spreads $s fingers and engulf the room in flames.", ch, NULL, NULL, TO_ROOM);

  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    int dam;

    /* only attack those that can be attacked */
    if (gch == ch || is_safe(ch, gch))
      continue;

    /* calculate damage and fire away */
    dam = 100 * ch->pcdata->powers[SPHERE_INVOCATION];
    dam = number_range(3 * dam, 7 * dam);
    dam = cap_dam(ch, gch, dam);

    damage(ch, gch, NULL, dam, sn);
  }

  WAIT_STATE(ch, 8);
}

void do_warlocktalk(CHAR_DATA * ch, char *argument)
{
  int class = ch->class;

  if (IS_NPC(ch) || (!IS_IMMORTAL(ch) && !IS_CLASS(ch, CLASS_WARLOCK)))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  ch->class = CLASS_WARLOCK;
  talk_channel(ch, argument, CHANNEL_CLASS, CC_WARLOCK, "warlock talk");
  ch->class = class;
}

void do_truthtell(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *tmp;
  const int cost = 1200;
  char arg[MAX_INPUT_LENGTH];

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct divination level */
  if (ch->pcdata->powers[SPHERE_DIVINATION] < 5)
  {
    send_to_char("Your level of expertise in divination is to low.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana.\n\r", cost - ch->mana);
    return;
  }

  argument = one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Truthtell whom?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("Not on mobiles.\n\r", ch);
    return;
  }
  if (is_safe(ch, victim)) return;
  if (!str_cmp(argument, "score"))
  {
    modify_mana(ch, -1 * cost);

    if (save_warlock_spell(victim))
    {
      act("$n tries to invade your mind.", ch, NULL, victim, TO_VICT);
      send_to_char("You fail.\n\r", ch);

      if (victim->position == POS_STANDING && victim->fighting == NULL)
      {
        update_feed(ch, victim);
        multi_hit(victim, ch, 1);
      }

      return;
    }
    tmp = victim->desc;
    victim->desc = ch->desc;
    do_score(victim, "");
    victim->desc = tmp;
    act("You feel $n's intrusion in your mind.", ch, NULL, victim, TO_VICT);
  }
  else if (!str_cmp(argument, "level"))
  {
    modify_mana(ch, -1 * cost);

    if (save_warlock_spell(victim))
    {
      act("$n tries to invade your mind.", ch, NULL, victim, TO_VICT);
      send_to_char("You fail.\n\r", ch);
      if (victim->position == POS_STANDING && victim->fighting == NULL)
      {
        update_feed(ch, victim);
        multi_hit(victim, ch, 1);
      }
      return;
    }
    tmp = victim->desc;
    victim->desc = ch->desc;
    do_level(victim, "");
    victim->desc = tmp;
    act("You feel $n's intrusion in your mind.", ch, NULL, victim, TO_VICT);
  }
  else
  {
    send_to_char("Get their score or level?\n\r", ch);
  }
}

void do_paradisebirds(CHAR_DATA *ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  const int cost = 1000;
  CHAR_DATA *gch;
  ITERATOR *pIter;
  EVENT_DATA *event;
  BUFFER *buf;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct divination level */
  if (ch->pcdata->powers[SPHERE_DIVINATION] < 4)
  {
    send_to_char("Your level of expertise in divination is to low.\n\r", ch);
    return;
  }

  if (event_isset_mobile(ch, EVENT_PLAYER_SCRYBIRDS))
  {
    send_to_char("The birds of paradise are already flocking.\n\r", ch);
    return;
  }

  if (ch->mana <= cost)
  {
    send_to_char("You do not have enough mana to summon the birds of paradise.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("$n emits a low whistle.", ch, NULL, NULL, TO_ROOM);
  act("You whistle in the arcane language of the birds of paradise.", ch, NULL, NULL, TO_CHAR);

  buf = buffer_new(MAX_STRING_LENGTH);
  bprintf(buf, "Name         Health  Timer  Location\n\r");
  bprintf(buf, "===========================================================\n\r");

  /* create the output */
  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected != CON_PLAYING || (gch = d->character) == NULL)
      continue;
    if (gch->level > 6)
      continue;
    if (!can_see(ch, gch))
      continue;

    if (event_isset_mobile(gch, EVENT_PLAYER_WITNESS))
    {
      send_to_char("The shadow witness whispers something in your ear.\n\r", gch);
      printf_to_char(gch, "  '%s's cursed birds have spotted you sire.'.\n\r", ch->name);
    }

    bprintf(buf, "%-12s (%3d%%)    %2d   %s#n\n\r",
      gch->name, 100 * gch->hit / UMAX(1, gch->max_hit), gch->fight_timer,
     (gch->in_room) ? gch->in_room->name : "(somewhere)");
  }

  event = alloc_event();
  event->argument = str_dup(buf->data);
  event->type = EVENT_PLAYER_SCRYBIRDS;
  event->fun = &event_player_scrybirds;
  add_event_char(event, ch, number_range(2, 4) * PULSE_PER_SECOND);

  buffer_free(buf);
}

bool event_player_scrybirds(EVENT_DATA *event)
{
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_scrybirds: no owner.", 0);
    return FALSE;
  }

  act("A flock of birds lands on $n, and starts chittering to $m.", ch, NULL, NULL, TO_ROOM);
  send_to_char("The birds of paradise report back to you.\n\n\r", ch);
  send_to_char(event->argument, ch);

  return FALSE;
}

bool event_mobile_mummyrot(EVENT_DATA *event)
{
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_mummyrot: no owner.", 0);
    return FALSE;
  }

  /* 20% chance of dropping a limb */
  if (number_percent() >= 80)
  {
    /* FIX ME - should rot the limb - add code for that here... */
  }

  if (number_percent() >= 10)
  {
    event = alloc_event();
    event->fun = &event_mobile_mummyrot;
    event->type = EVENT_MOBILE_MUMMYROT;
    add_event_char(event, ch, 2 * PULSE_PER_SECOND);
  }
  else
  {
    send_to_char("You are cured from the horrible pox.\n\r", ch);
    act("$n seems to be cured from the horrible pox $e was suffering from.", ch, NULL, NULL, TO_ROOM);
  }

  return FALSE;
}

void do_mummyrot(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim = NULL;
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];
  int cost = 4000;
  int level = ch->spl[0] + ch->spl[1] + ch->spl[2] + ch->spl[3] + ch->spl[4];

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK) ||
     !IS_SET(ch->pcdata->powers[EVOLVE_3], WARLOCK_EVOLVE_DECAYCHAIN))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_MUMMYWAIT)) != NULL)
  {
    printf_to_char(ch, "You cannot use this power for another %s.\n\r", event_time_left(event));
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0' && (victim = ch->fighting) == NULL)
  {
    send_to_char("Inflict whom with mummyrot?\n\r", ch);
    return;
  }
  if (victim == NULL && (victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (event_isset_mobile(victim, EVENT_MOBILE_MUMMYROT))
  {
    send_to_char("They are already affected by mummy rot.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need another %d mana to use this power.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("$n touches you with a small piece of rotten bone.", ch, NULL, victim, TO_VICT);
  act("$n touches $N with a small piece of rotten bone.", ch, NULL, victim, TO_NOTVICT);
  act("You touch $N with a small piece of rotten bone.", ch, NULL, victim, TO_CHAR);

  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_PLAYER_MUMMYWAIT;
  add_event_char(event, ch, 10 * PULSE_PER_SECOND);
  WAIT_STATE(ch, 4);

  if (saves_spell(level, victim) && saves_spell(level, victim))
  {
    act("You resist $n's spell.", ch, NULL, victim, TO_VICT);
    act("$N resists your spell.", ch, NULL, victim, TO_CHAR);
    return;
  }

  act("A pox quickly spreads over $n's skin.", victim, NULL, NULL, TO_ROOM);
  send_to_char("Excruciating pain wrecks your body as a pox spreads over your skin.\n\r", victim);

  event = alloc_event();
  event->fun = &event_mobile_mummyrot;
  event->type = EVENT_MOBILE_MUMMYROT;
  add_event_char(event, victim, 2 * PULSE_PER_SECOND);
}

void do_warscry(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int mana_cost = 500;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct divination level */
  if (ch->pcdata->powers[SPHERE_DIVINATION] < 3)
  {
    send_to_char("Your level of expertise in divination is to low.\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Whom do you wish to scry on?\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("You where unable to locate them.\n\r", ch);
    return;
  }

  if (IS_IMMUNE(victim, IMM_SHIELDED))
  {
    send_to_char("They are shielded from scrying.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  mana_cost = reduce_cost(ch, victim, mana_cost, eMana);

  if (ch->mana < mana_cost)
  {
    send_to_char("You do not have enough mana to summon a scrying sphere.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * mana_cost);

  if (event_isset_mobile(victim, EVENT_PLAYER_WITNESS))
  {
    send_to_char("The shadow witness whispers something in your ear.\n\r", victim);
    printf_to_char(victim, "  '%s is scrying on you sire.'.\n\r", ch->name);
  }

  /* create a floating sphere, and remember to CLOSE IT */
  obj = create_object(get_obj_index(OBJ_VNUM_PROTOPLASM), 50);
  object_decay(obj, number_range(15, 20));
  obj->item_type = ITEM_PORTAL;
  obj->value[0] = victim->in_room->vnum;
  obj->value[2] = 1;  /* this will close it */
  REMOVE_BIT(obj->wear_flags, ITEM_TAKE);
  free_string(obj->short_descr);
  free_string(obj->description);
  free_string(obj->name);
  obj->short_descr = str_dup("A floating sphere");
  obj->description = str_dup("A floating sphere hovers here.");
  obj->name = str_dup("sphere");
  obj_to_room(obj, ch->in_room);

  act("You mumble a few incantations and $p appears.", ch, obj, NULL, TO_CHAR);
  act("$n mumbles a few incantations and $p appears.", ch, obj, NULL, TO_ROOM);
}

void do_pentagram(CHAR_DATA * ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  ITERATOR *pIter;
  char arg[MAX_STRING_LENGTH];
  int mana_cost = 2000;
   
  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct summoning level */
  if (ch->pcdata->powers[SPHERE_SUMMONING] < 2)
  {
    send_to_char("Your level of expertise in summoning is to low.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  mana_cost = reduce_cost(ch, NULL, mana_cost, eMana);

  /* check for mana required */
  if (ch->mana < mana_cost)
  {  
    send_to_char("You do not have enough mana to use this power.\n\r", ch);
    return;
  }
      
  /* check for a valid target, if any ? */
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Whom do you wish to summon with your pentagram?\n\r", ch);
    return;
  }
      
  /* send summoning messages and take away the mana cost */
  act("You draw a pentagram on the floor and start chanting.", ch, NULL, NULL, TO_CHAR);
  act("$n draws a pentagram on the floor and starts chanting.", ch, NULL, NULL, TO_ROOM);
  modify_mana(ch, -1 * mana_cost);
      
  /* try and summon something */
  if (ch->pcdata->powers[SPHERE_SUMMONING] < 6)
  {
    if ((victim = get_char_area(ch, arg)) == NULL
        || victim == ch
        || victim->in_room == NULL
        || IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
        || IS_SET(ch->in_room->room_flags, ROOM_SAFE)
        || IS_SET(victim->in_room->room_flags, ROOM_SAFE)
        || IS_SET(victim->in_room->area->areabits, AREA_BIT_NOPENTA)
        || IS_SET(victim->in_room->area->areabits, AREA_BIT_OLC)
        || victim->in_room->area != ch->in_room->area
        || victim->in_room == ch->in_room)
    {
      send_to_char("You failed.\n\r", ch);
      return;
    }

    /* setup some variables and send messages to target room */
    sprintf(arg, "%d", ch->in_room->vnum);
    act("A shimmering pentagram forms under your feet.", victim, NULL, NULL, TO_ALL);

    /* enqueue pentagram in victims room */
    event = alloc_event();
    event->argument = str_dup(arg);
    event->fun = &event_room_pentagram;
    event->type = EVENT_ROOM_PENTAGRAM;
    add_event_room(event, victim->in_room, number_range(2, 2 * PULSE_PER_SECOND));
  }
  else
  {
    char buf[MAX_STRING_LENGTH];
    int found = 0; 

    if (IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
     || IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
      send_to_char("You cannot cast the pentagram spell in this room.\n\r", ch);
      return;
    }
   
    sprintf(buf, "%d", ch->in_room->vnum);

    pIter = AllocIterator(char_list);
    while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (!is_name(arg, victim->name)) continue;
      if (!can_see(ch, victim)) continue;
      if (victim == ch) continue;
      if (victim->in_room == ch->in_room) continue;
      if (victim->in_room == NULL) continue;
      if (victim->in_room->area != ch->in_room->area) continue;
      if (IS_SET(victim->in_room->area->areabits, AREA_BIT_OLC)) continue;
      if (IS_SET(victim->in_room->area->areabits, AREA_BIT_NOPENTA)) continue;
      if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)) continue;
      if (event_isset_room(victim->in_room, EVENT_ROOM_PENTAGRAM)) continue;
   
      if (found++ >= 5) continue;
   
      act("A shimmering pentagram forms under your feet.", victim, NULL, NULL, TO_ALL);

      /* enqueue pentagram in victims room */
      event = alloc_event();
      event->argument = str_dup(buf);
      event->fun = &event_room_pentagram;
      event->type = EVENT_ROOM_PENTAGRAM;
      add_event_room(event, victim->in_room, number_range(2, 2 * PULSE_PER_SECOND));
    }

    if (found == 0)
      send_to_char("Nothing happens, the spell failed.\n\r", ch);
  }
}

void do_study(CHAR_DATA * ch, char *argument)
{
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  const int basecost = 3000000; /* 3 million exp */
  const int max_power = 5;
  int cost, level = 0;
  bool canstudy = TRUE;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for active study events */
  if ((event = event_isset_mobile(ch, EVENT_PLAYER_STUDY)) != NULL)
  {
    char *ptr;
    int session;

    if (!str_cmp(argument, "cancel"))
    {
      send_to_char("You cancel your study session.\n\r", ch);
      dequeue_event(event, TRUE);
      return;
    }

    canstudy = FALSE;
    ptr = one_argument(event->argument, arg2);
    one_argument(ptr, arg);

    /* find the cost of training this study */
    cost = get_warlock_power(ch, arg2);
    cost = get_train_cost(cost, basecost);
    session = atoi(arg);

    level = 25 * (cost - session) / cost;
  }

  one_argument(argument, arg);
  if ((str_cmp(arg, "summoning") && str_cmp(arg, "necromancy") && str_cmp(arg, "abjuration")
    && str_cmp(arg, "invocation") && str_cmp(arg, "enchantment") && str_cmp(arg, "divination"))
    || !canstudy)
  {
    BUFFER *buf = buffer_new(MAX_STRING_LENGTH);

    bprintf(buf, " #uYour current levels of study in the magic fields are#n\n\n\r");
    bprintf(buf, "   Summoning  [%d]   Necromancy  [%d]   Abjuration [%d]\n\r",
      ch->pcdata->powers[SPHERE_SUMMONING],
      ch->pcdata->powers[SPHERE_NECROMANCY],
      ch->pcdata->powers[SPHERE_ABJURATION]);
    bprintf(buf, "   Invocation [%d]   Enchantment [%d]   Divination [%d]\n\r",
      ch->pcdata->powers[SPHERE_INVOCATION],
      ch->pcdata->powers[SPHERE_ENCHANTMENT],
      ch->pcdata->powers[SPHERE_DIVINATION]);

    if (canstudy)
      bprintf(buf, "\n\r Syntax: study <field>\n\r");
    else
    {
      int i;

      bprintf(buf, "\n\r %c%s :: [#C", UPPER(arg2[0]), &arg2[1]);
      for (i = 0; i < 25; i++)
      {
        if (level == i)
          bprintf(buf, "#n*");
        else
          bprintf(buf, "*");
      }
      bprintf(buf, "#n]\n\n\r");
      bprintf(buf, " Type '#Cstudy cancel#n' if you wish to cancel your study session.\n\r");
    }

    send_to_char(buf->data, ch);
    buffer_free(buf);
    return;
  }

  if ((cost = get_warlock_power(ch, arg)) >= max_power)
  {
    send_to_char("But you have already mastered that sphere.\n\r", ch);
    return;
  }
  cost = get_train_cost(cost, basecost);

  sprintf(arg2, "You have 30 minutes to gather %d experience points.\n\r", cost);
  send_to_char(arg2, ch);

  /* set study session variables */
  sprintf(arg2, "%s %d", arg, cost);

  /* enqueue study event */
  event = alloc_event();
  event->argument = str_dup(arg2);
  event->fun = &event_player_study;
  event->type = EVENT_PLAYER_STUDY;
  add_event_char(event, ch, 30 * 60 * PULSE_PER_SECOND);
}

void do_crows(CHAR_DATA * ch, char *argument)
{
  EVENT_DATA *event;
  int cost = 750;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct summoning level */
  if (ch->pcdata->powers[SPHERE_SUMMONING] < 4)
  {
    send_to_char("Your level of expertise in summoning is to low.\n\r", ch);
    return;
  }
  if (event_isset_mobile(ch, EVENT_PLAYER_CROWS))
  {
    send_to_char("You have already summoned a murder of crows.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to summon a murder of crows.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("$n lets out a high pitched scream, calling for someone or something.", ch, NULL, NULL, TO_ROOM);
  send_to_char("You send out a call for a murder of crows.\n\r", ch);

  event = alloc_event();
  event->fun = &event_player_crows;
  event->type = EVENT_PLAYER_CROWS;
  add_event_char(event, ch, number_range(1, 3) * PULSE_PER_SECOND);
}

void do_leeches(CHAR_DATA * ch, char *argument)
{
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int cost = 750;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct summoning level */
  if (ch->pcdata->powers[SPHERE_SUMMONING] < 3)
  {
    send_to_char("Your level of expertise in summoning is to low.\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Send the metal leeches to anoy whom?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (event_isset_mobile(victim, EVENT_PLAYER_LEECHES))
  {
    send_to_char("They are already covered in leeches.\n\r", ch);
    return;
  }
  if (ch == victim)
  {
    send_to_char("That would be stupid.\n\r", ch);
    return;
  }

  if (is_safe(ch, victim))
    return;

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to summon the metal leeches.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("You call upon some metal leeches to infest $N's equipment.", ch, NULL, victim, TO_CHAR);
  act("$n calls upon some metal leeches to infest $N's equipment.", ch, NULL, victim, TO_NOTVICT);
  act("$n calls upon some metal leeches to infest your equipment.", ch, NULL, victim, TO_VICT);

  event             =  alloc_event();
  event->argument   =  str_dup("init");
  event->type       =  EVENT_PLAYER_LEECHES;
  event->fun        =  &event_player_leeches;
  add_event_char(event, victim, 3 * PULSE_PER_SECOND);

  aggress(ch, victim);
}

void do_callwild(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  ROOM_INDEX_DATA *pRoom;
  const int cost = 2000;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct summoning level */
  if (ch->pcdata->powers[SPHERE_SUMMONING] < 5)
  {
    send_to_char("Your level of expertise in summoning is to low.\n\r", ch);
    return;
  }
  if ((pRoom = ch->in_room) == NULL)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }
  if (event_isset_room(pRoom, EVENT_ROOM_CALLWILD))
  {
    send_to_char("A tempest storm is already rising here.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("$n stretches $s hands to the sky and calls upon the thunder god.", ch, NULL, NULL, TO_ROOM);
  act("You stretch your hands to the sky and call upon the thunder god.", ch, NULL, NULL, TO_CHAR);

  event = alloc_event();
  event->type = EVENT_ROOM_CALLWILD;
  event->fun = &event_room_callwild;
  event->argument = str_dup(ch->name);
  add_event_room(event, pRoom, number_range(3, 6) * PULSE_PER_SECOND);
}

bool event_room_callwild(EVENT_DATA *event)
{
  ROOM_INDEX_DATA *pRoom;
  CHAR_DATA *gch, *first;
  ITERATOR *pIter;
  EXIT_DATA *pExit;
  int door;

  if ((pRoom = event->owner.room) == NULL)
  {
    bug("event_room_callwild: no owner.", 0);
    return FALSE;
  }

  if ((first = (CHAR_DATA *) FirstInList(pRoom->people)) != NULL)
  {
    if (IS_SET(pRoom->room_flags, ROOM_SAFE))
    {
      act("A tempest storm rages through the room, but harms noone.", first, NULL, NULL, TO_ALL);
      return FALSE;
    }

    act("A tempest storm rages through the room.", first, NULL, NULL, TO_ALL);

    pIter = AllocIterator(pRoom->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      int excount = 0;

      if (!str_cmp(event->argument, gch->name)) continue;
      if (gch->level < 2) continue;
      if (gch->hit <= 1) continue;

      send_to_char("You are struck by the tempest storm.\n\r", gch);

      switch(number_range(1, 3))
      {
        default:
          act("$n is covered in a shower of lightning bolts.", gch, NULL, NULL, TO_ROOM);
          modify_hps(gch, -1 * number_range(500, 1500));
          update_pos(gch);
          break;
        case 1:
          act("$n is struck by a column of flames.", gch, NULL, NULL, TO_ROOM);
          send_to_char("You burst into flames.\n\r", gch);
          SET_BIT(gch->affected_by, AFF_FLAMING);
          modify_hps(gch, -1 * number_range(350, 1000));
          update_pos(gch);
          break;
        case 2:
          act("$n is swept away by the storm.", gch, NULL, NULL, TO_ROOM);
          send_to_char("You are hurled into the air.\n\r", gch);
          modify_hps(gch, -1 * number_range(250, 500));
          update_pos(gch);
          for (door = 0; door < 6; door++)
          {
            if ((pExit = pRoom->exit[door]) != NULL)
              excount++;
          }
          if (excount > 0)
          {
            int i = number_range(1, excount);

            for (door = 0; door < 6; door++)
            {
              if ((pExit = pRoom->exit[door]) != NULL)
              {
                if (--i <= 0 && pExit->to_room)
                {
                  door = 6; /* dirty hack to leave loop */

                  if (gch->fighting)
                    stop_fighting(gch, TRUE);
                  char_from_room(gch);
                  char_to_room(gch, pExit->to_room, TRUE);
                  do_look(gch, "");
                }
              }
            }
          }
          break;
      }
    }
  }

  return FALSE;
}

void do_bindingvines(CHAR_DATA * ch, char *argument)
{
  EVENT_DATA *event;
  ROOM_INDEX_DATA *pRoom = ch->in_room;
  const int cost = 750;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct summoning level */
  if (ch->pcdata->powers[SPHERE_SUMMONING] < 1)
  {
    send_to_char("Your level of expertise in summoning is to low.\n\r", ch);
    return;
  }

  if (pRoom == NULL)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }
  if (event_isset_room(pRoom, EVENT_ROOM_VINES))
  {
    send_to_char("This room is already covered in binding vines.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to summon the binding vines.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("$n calls forth binding vines from the ground.", ch, NULL, NULL, TO_ROOM);
  act("You call forth binding vines from the ground.", ch, NULL, NULL, TO_CHAR);

  /* enqueue pentagram in victims room */
  event              =  alloc_event();
  event->argument    =  str_dup(ch->name);
  event->fun         =  &event_room_vines;
  event->type        =  EVENT_ROOM_VINES;
  add_event_room(event, pRoom, 3 * PULSE_PER_SECOND);
}

bool event_room_vines(EVENT_DATA *event)
{
  ROOM_INDEX_DATA *pRoom;
  ITERATOR *pIter;
  EVENT_DATA *newevent;
  CHAR_DATA *gch;

  if ((pRoom = event->owner.room) == NULL)
  {
    bug("event_room_vines: no owner.", 0);
    return FALSE;
  }

  gch = (CHAR_DATA *) FirstInList(pRoom->people);

  /* 25% chance of going away */
  if (number_percent() < 75)
  {
    newevent              =  alloc_event();
    newevent->argument    =  str_dup(event->argument);
    newevent->fun         =  &event_room_vines;
    newevent->type        =  EVENT_ROOM_VINES;
    add_event_room(newevent, pRoom, 3 * PULSE_PER_SECOND);
  }
  else if (gch != NULL)
  {
    act("The binding vines crumbles to dust and fades away.", gch, NULL, NULL, TO_ALL);
  }

  pIter = AllocIterator(pRoom->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (save_warlock_spell(gch)) continue;
    if (!str_cmp(event->argument, gch->name)) continue;
    if (IS_SET(gch->affected_by, AFF_WEBBED)) continue;

    SET_BIT(gch->affected_by, AFF_WEBBED);
    act("$n is trapped in some weblike vines spewing from the ground.", gch, NULL, NULL, TO_ROOM);
    send_to_char("You are trapped in some weblike vines spewing from the ground.\n\r", gch);
  }

  return FALSE;
}

bool event_room_pentagram(EVENT_DATA * event)
{
  CHAR_DATA *gch;
  ROOM_INDEX_DATA *pRoom;
  ROOM_INDEX_DATA *pRoom2;
  ITERATOR *pIter, *pIter2;
  OBJ_DATA *obj;
  bool found = FALSE, has_key;
  const int max_people = 30;
  int pcounter = 0;

  if ((pRoom = event->owner.room) == NULL)
  {
    bug("event_room_pentagram: no owner.", 0);
    return FALSE;
  }

  /* find target room */
  if ((pRoom2 = get_room_index(atoi(event->argument))) == NULL)
  {
    bug("event_room_pentagram: no target.", 0);
    return FALSE;
  }

  /* count people in target room */
  pcounter += SizeOfList(pRoom2->people);

  pIter = AllocIterator(pRoom->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    has_key = FALSE;

    if (!IS_NPC(gch) && !IS_IMMUNE(gch, IMM_SUMMON))
      continue;
    if (IS_NPC(gch) && save_warlock_spell(gch))
      continue;
    if (IS_NPC(gch) && (gch->shop_fun != NULL || gch->quest_fun != NULL))
      continue;
    if (gch->position == POS_FIGHTING)
      continue;
    if (pcounter++ >= max_people)
      continue;

    pIter2 = AllocIterator(gch->carrying);
    while ((obj = (OBJ_DATA *) NextInList(pIter2)) != NULL)
    {
      if (obj->item_type == ITEM_KEY)
        has_key = TRUE;
    }

    if (has_key)
      continue;

    found = TRUE;

    /* move mobile and send messages */
    act("The pentagram flashes, and the world starts spinning.", gch, NULL, NULL, TO_CHAR);
    act("The pentagram flashes, and $n vanishes", gch, NULL, NULL, TO_ROOM);
    char_from_room(gch);
    char_to_room(gch, pRoom2, TRUE);
    act("The world stops spinning and you find yourself somewhere else.", gch, NULL, NULL, TO_CHAR);
    act("$n appears in the pentagram.", gch, NULL, NULL, TO_ROOM);
    do_look(gch, "");
  }

  /* send failure message to target room */
  if (!found && (gch = (CHAR_DATA *) FirstInList(pRoom2->people)) != NULL)
  {
    act("The pentagram fades away, nothing happens.", gch, NULL, NULL, TO_ALL);
  }

  /* we didn't dequeue */
  return FALSE;
}

bool event_player_study(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char *argument;
  int exp_needed;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_study: no owner.", 0);
    return FALSE;
  }

  /* get event variables */
  argument = one_argument(event->argument, arg1);
  argument = one_argument(argument, arg2);
  exp_needed = atoi(arg2);

  if (exp_needed > 0)
  {
    send_to_char("#RYou failed your study session.#n\n\r", ch);
    return FALSE;
  }

  send_to_char("#GYou succeeded in your study session.#n\n\r", ch);
  if (!str_cmp(arg1, "summoning"))
    ch->pcdata->powers[SPHERE_SUMMONING]++;
  else if (!str_cmp(arg1, "necromancy"))
    ch->pcdata->powers[SPHERE_NECROMANCY]++;
  else if (!str_cmp(arg1, "abjuration"))
    ch->pcdata->powers[SPHERE_ABJURATION]++;
  else if (!str_cmp(arg1, "divination"))
    ch->pcdata->powers[SPHERE_DIVINATION]++;
  else if (!str_cmp(arg1, "enchantment"))
    ch->pcdata->powers[SPHERE_ENCHANTMENT]++;
  else if (!str_cmp(arg1, "invocation"))
    ch->pcdata->powers[SPHERE_INVOCATION]++;
  else
    send_to_char("Unknown field of study. Please report this to an immortal.\n\r", ch);

  /* we didn't dequeue */
  return FALSE;
}

/* Current 1.8 setting will result in the following
 * costs for the 5 levels in each sphere. Roughly.
 *
 * 3, 6, 10, 18, 32 million exp
 */
int get_train_cost(int clevel, int basecost)
{
  int i, cost = 1;

  cost *= basecost;
  for (i = 0; i < clevel; i++)
  {
    cost *= 18;
    cost /= 10;
  }

  return UMAX(basecost, cost);
}

int get_warlock_power(CHAR_DATA * ch, char *power)
{
  if (IS_NPC(ch))
    return -1;

  if (!str_cmp(power, "summoning"))
    return ((ch->pcdata->powers[WARLOCK_PATH] == PATH_SUMMONING)
       ? ch->pcdata->powers[SPHERE_SUMMONING] - 1
       : ch->pcdata->powers[SPHERE_SUMMONING]);
  if (!str_cmp(power, "necromancy"))
    return ((ch->pcdata->powers[WARLOCK_PATH] == PATH_NECROMANCY)
       ? ch->pcdata->powers[SPHERE_NECROMANCY] - 1
       : ch->pcdata->powers[SPHERE_NECROMANCY]);
  if (!str_cmp(power, "abjuration"))
    return ((ch->pcdata->powers[WARLOCK_PATH] == PATH_ABJURATION)
       ? ch->pcdata->powers[SPHERE_ABJURATION] - 1
       : ch->pcdata->powers[SPHERE_ABJURATION]);
  if (!str_cmp(power, "invocation"))
    return ((ch->pcdata->powers[WARLOCK_PATH] == PATH_INVOCATION)
       ? ch->pcdata->powers[SPHERE_INVOCATION] - 1
       : ch->pcdata->powers[SPHERE_INVOCATION]);
  if (!str_cmp(power, "enchantment"))
    return ((ch->pcdata->powers[WARLOCK_PATH] == PATH_ENCHANTMENT)
       ? ch->pcdata->powers[SPHERE_ENCHANTMENT] - 1
       : ch->pcdata->powers[SPHERE_ENCHANTMENT]);
  if (!str_cmp(power, "divination"))
    return ((ch->pcdata->powers[WARLOCK_PATH] == PATH_DIVINATION)
       ? ch->pcdata->powers[SPHERE_DIVINATION] - 1
       : ch->pcdata->powers[SPHERE_DIVINATION]);

  return -1;
}

bool save_warlock_spell(CHAR_DATA * ch)
{
  int chance;

  if (IS_NPC(ch))
  {
    chance = ch->level / 15;
  }
  else
  {
    chance = ch->spl[0] + ch->spl[1] + ch->spl[2] + ch->spl[3] + ch->spl[4];
    chance /= 25;
  }

  chance = URANGE(25, chance, 75);

  return (number_percent() < chance);
}

void spell_noxious_fumes(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  af.type = sn;
  af.duration = number_range(20, 30);
  af.location = APPLY_HITROLL;
  af.modifier = -1 * number_range(level, 2 * level);
  af.bitvector = 0;
  affect_to_char(victim, &af);
  af.location = APPLY_DAMROLL;
  affect_to_char(victim, &af);
}

void spell_steelfists(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, sn))
  {
    send_to_char("Your fists are already made of steel.\n\r", ch);
    return;
  }

  af.type = sn;
  af.duration = number_range(20, 30);
  af.location = APPLY_HITROLL;
  af.modifier = level / 10;
  af.bitvector = 0;
  affect_to_char(victim, &af);
  af.location = APPLY_DAMROLL;
  affect_to_char(victim, &af);
  af.location = APPLY_AC;
  af.modifier = -1 * level / 5;
  affect_to_char(victim, &af);

  act("$n mutters an incantation and $s hands turns to solid steel.", victim, NULL, NULL, TO_ROOM);
  send_to_char("You mutter an incantation and your hands turn to steel\n\r", victim);
}

SPELL_DATA *alloc_spell()
{
  SPELL_DATA *spell;

  if ((spell = (SPELL_DATA *) PopStack(spell_free)) == NULL)
    spell = malloc(sizeof(*spell));

  /* reset spell data */
  spell->flags = 0;
  spell->power = 0;
  spell->type = 0;
  spell->argument = str_dup("");

  return spell;
}

void free_spell(SPELL_DATA * spell)
{
  free_string(spell->argument);
  PushStack(spell, spell_free);
}

void spell_from_char(SPELL_DATA *spell, CHAR_DATA *ch, bool spellchain)
{
  LIST *list;

  if (IS_NPC(ch) || ch->pcdata == NULL)
  {
    bug("Spell_from_char on NPC", 0);
    return;
  }

  list = (spellchain) ? ch->pcdata->spells : ch->pcdata->contingency;
  DetachFromList(spell, list);

  free_spell(spell);
}

void spell_to_char(SPELL_DATA * spell, CHAR_DATA * ch)
{
  if (IS_NPC(ch) || ch->pcdata == NULL)
  {
    bug("Spell_to_char on NPC", 0);
    return;
  }
  AttachToList(spell, ch->pcdata->spells);
}

void do_wcancel(CHAR_DATA * ch, char *argument)
{
  ITERATOR *pIter;
  SPELL_DATA *spell;

  if (IS_NPC(ch))
    return;

  if (!IS_CLASS(ch, CLASS_WARLOCK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (SizeOfList(ch->pcdata->spells) == 0)
  {
    send_to_char("But you are not casting a spell.\n\r", ch);
    return;
  }

  pIter = AllocIterator(ch->pcdata->spells);
  while ((spell = (SPELL_DATA *) NextInList(pIter)) != NULL)
    spell_from_char(spell, ch, TRUE);

  send_to_char("You discharge all your chained spells.\n\r", ch);
}

void do_winit(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  SPELL_DATA *spell;
  const int cost = 100;
  int residue;

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_WARLOCK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (SizeOfList(ch->pcdata->spells) > 0)
  {
    send_to_char("But you are already casting a spell.\n\r", ch);
    return;
  }
  if ((residue = get_cost(ch, cost)) != 0)
  {
    sprintf(buf, "You need %d more mana.\n\r", residue);
    send_to_char(buf, ch);
    return;
  }

  spell = alloc_spell();
  spell_to_char(spell, ch);

  act_brief("$n raises a finger, calling for the magic.", ch, NULL, NULL, TO_ROOM, BRIEF_7);
  act("You raise a finger and beckon for the magic to come.", ch, NULL, NULL, TO_CHAR);
}

void do_wpower(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  SPELL_DATA *spell;
  const int cost = 100;
  int level, residue;

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_WARLOCK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((spell = (SPELL_DATA *) FirstInList(ch->pcdata->spells)) == NULL)
  {
    send_to_char("But you are not casting a spell.\n\r", ch);
    return;
  }
  if (argument[0] == '\0')
  {
    send_to_char("What powerlevel should the spell be set at?\n\r", ch);
    return;
  }
  if ((level = atoi(argument)) < 1 || level > 100)
  {
    send_to_char("The power level should be set between 1 and 100.\n\r", ch);
    return;
  }
  if ((residue = get_cost(ch, cost)) != 0)
  {          
    sprintf(buf, "You need %d more mana.\n\r", residue);
    send_to_char(buf, ch);
    return;  
  }

  spell->power = level;

  act_brief("$n clenches $s fist.", ch, NULL, NULL, TO_ROOM, BRIEF_7);
  act("You clench your fist, drawing in power.", ch, NULL, NULL, TO_CHAR);
}

void do_wchain(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  SPELL_DATA *spell;
  const int cost = 100;
  const int max_chain = 5;
  int count = 0, residue;

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_WARLOCK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  /* we do a quick count of all the players spells */
  count += SizeOfList(ch->pcdata->spells);

  if ((spell = (SPELL_DATA *) FirstInList(ch->pcdata->spells)) == NULL)
  {
    send_to_char("But you are not casting a spell.\n\r", ch);
    return;
  }
  if (count >= max_chain)
  {
    send_to_char("You cannot chain any more spells, you must cast this chain.\n\r", ch);
    return;
  }
  if (spell->argument[0] == '\0')
  {
    send_to_char("The current spell has no target, you cannot begin a new chain.\n\r", ch);
    return;
  }
  if (spell->type == 0)
  {
    send_to_char("The current spell has no type, you cannot begin a new chain.\n\r", ch);
    return;
  }
  if ((residue = get_cost(ch, cost)) != 0)
  {
    sprintf(buf, "You need %d more mana.\n\r", residue);
    send_to_char(buf, ch);
    return;
  }

  spell = alloc_spell();
  spell_to_char(spell, ch);

  act_brief("$n draws a circle in the air, trailing residue magic.", ch, NULL, NULL, TO_ROOM, BRIEF_7);
  act("You draw a circle in the air, symbolising the end and the begining.", ch, NULL, NULL, TO_CHAR);
}

void do_homingdevice(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  OBJ_INDEX_DATA *pObjIndex;
  int cost = 2500;

  if (IS_NPC(ch))
    return;

  if (!IS_CLASS(ch, CLASS_WARLOCK) || !IS_SET(ch->pcdata->powers[EVOLVE_1], WARLOCK_EVOLVE_HOMING))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    send_to_char("You do not have enough mana to create a homing device.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * cost);

  if ((pObjIndex = get_obj_index(OBJ_VNUM_PROTOPLASM)) == NULL)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }

  obj = create_object(pObjIndex, 50);
  obj->item_type = ITEM_HOMING;
  obj->ownerid = ch->pcdata->playerid;
  free_string(obj->short_descr);
  free_string(obj->name);
  free_string(obj->description);
  obj->short_descr = str_dup("a homing device");
  obj->name = str_dup("homing device");
  obj->description = str_dup("a homing device lies here.");

  /* spell lasts 5 minutes */
  object_decay(obj, 5 * 60 );

  obj_to_char(obj, ch);
  send_to_char("A homing device appears in your hands.\n\r", ch);
  act("A homing device appears in $n's hands.", ch, NULL, NULL, TO_ROOM);
}

void do_foldspace(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];
  int dir, cost = 4000;

  if (IS_NPC(ch))
    return;
 
  if (!IS_CLASS(ch, CLASS_WARLOCK) || !IS_SET(ch->pcdata->powers[EVOLVE_1], WARLOCK_EVOLVE_SPACE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (get_room_index(ch->pcdata->wlck_vnum) == NULL)
  {
    send_to_char("First you must lock your power onto a room.\n\r", ch);
    return;
  }

  one_argument(argument, arg);

  if (arg[0] == '\0' || ch->in_room == NULL)
  {
    send_to_char("Which way should the exit lead: north, south, east or west?\n\r", ch);
    return;
  }

  if (!str_prefix(arg, "south"))
    dir = DIR_SOUTH;
  else if (!str_prefix(arg, "north"))
    dir = DIR_NORTH;
  else if (!str_prefix(arg, "east"))
    dir = DIR_EAST;
  else if (!str_prefix(arg, "west"))
    dir = DIR_WEST;
  else
  {
    do_foldspace(ch, "");
    return;
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to use this power.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("Reality distorts for a second while $n modifies the world.", ch, NULL, NULL, TO_ROOM);
  send_to_char("You create a magical pathway.\n\r", ch);

  sprintf(arg, "%d %d", dir, ch->pcdata->wlck_vnum);

  event              =  alloc_event();
  event->fun         =  &event_dummy;
  event->type        =  EVENT_ROOM_MISDIRECT;
  event->argument    =  str_dup(arg);
  add_event_room(event, ch->in_room, 3 * 60 * PULSE_PER_SECOND);
}

void do_lockroom(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (!IS_CLASS(ch, CLASS_WARLOCK) || !IS_SET(ch->pcdata->powers[EVOLVE_1], WARLOCK_EVOLVE_SPACE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (ch->in_room && IS_SET(ch->in_room->room_flags, ROOM_KINGDOM))
  {
    send_to_char("You cannot lock onto kingdom rooms.\n\r", ch);
    return;
  }

  ch->pcdata->wlck_vnum = (ch->in_room) ? ch->in_room->vnum : 0;

  send_to_char("You lock your spacefolding power onto this room.\n\r", ch);
  act("$n mumbles an arcane phrase and the floor glows slightly for an instance.", ch, NULL, NULL, TO_ROOM);
}

void do_contingency(CHAR_DATA *ch, char *argument)
{
  SPELL_DATA *spell;
  ITERATOR *pIter;
  LIST *list;

  if (IS_NPC(ch))
    return;

  if (!IS_CLASS(ch, CLASS_WARLOCK) || !IS_SET(ch->pcdata->powers[EVOLVE_1], WARLOCK_EVOLVE_CONTINGENCY))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if ((spell = (SPELL_DATA *) FirstInList(ch->pcdata->spells)) == NULL)
  {
    send_to_char("But you are not casting a spell.\n\r", ch);
    return;
  }
  if (spell->argument[0] == '\0')
  {
    send_to_char("The current spell has no target, you cannot begin a new chain.\n\r", ch);
    return;
  }
  if (spell->type == 0)
  {
    send_to_char("The current spell has no type, you cannot begin a new chain.\n\r", ch);
    return;
  }
  if (SizeOfList(ch->pcdata->contingency) > 0 && str_cmp(argument, "override"))
  {
    send_to_char("You already have a contingency spell in place.\n\r"
                 "If you wish to override type 'contingency override'\n\r", ch);
    return;
  }

  pIter = AllocIterator(ch->pcdata->contingency);
  while ((spell = (SPELL_DATA *) NextInList(pIter)) != NULL)
    spell_from_char(spell, ch, FALSE);

  list = ch->pcdata->contingency;
  ch->pcdata->contingency = ch->pcdata->spells;
  ch->pcdata->spells = list;

  send_to_char("Contingency spell set.\n\r", ch);
}

void do_backlash(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_WARLOCK) || !IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_BACKLASH))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  TOGGLE_BIT(ch->newbits, NEW_BACKLASH);

  if (IS_SET(ch->newbits, NEW_BACKLASH))
  {
    act("A shimmering forcefields pops into existance around $n.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You create a shimmering forcefield.\n\r", ch);
  }
  else
  {
    act("The forcefield around $n flickers and dies.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You dispel your forcefield.\n\r", ch);
  }
}

bool event_player_huntingstars(EVENT_DATA *event)
{
  CHAR_DATA *ch, *victim;
  ROOM_INDEX_DATA *chroom;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_huntingstars: no owner.", 0);
    return FALSE;
  }

  if (event->argument == NULL || event->argument[0] == '\0')
  {
    bug("event_player_huntingstars: no target.", 0);
    return FALSE;
  }

  if ((victim = get_online_player(atoi(event->argument))) == NULL || victim->in_room == NULL)
  {
    send_to_char("One of your hunting stars misses its target.\n\r", ch);
    return FALSE;
  }

  chroom = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, victim->in_room, FALSE);
  one_hit(ch, victim, gsn_huntingstars, 0);
  char_from_room(ch);
  char_to_room(ch, chroom, FALSE);

  /* cannot happen, but I'll do it anyway */
  if (ch->dead)
    return TRUE;

  return FALSE;
}

void do_stitches(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int cost = 900;

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_WARLOCK) || !IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_STITCHES))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Cast 'seven stitches' on whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("This spell has no affect on monsters.\n\r", ch);
    return;
  }

  if (is_safe(ch, victim))
    return;

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to cast this spell.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  if (IS_SET(victim->newbits, NEW_STITCHES))
  {
    send_to_char("Your spell fails, they are already stitched.\n\r", ch);
    return;
  }

  act("$n summons a large needle.", ch, NULL, NULL, TO_ROOM);
  act("You summon a large needle.", ch, NULL, NULL, TO_CHAR);
  act("The large needle quickly sews $n's mouth together with 7 stitches.", victim, NULL, NULL, TO_ROOM);
  act("The large needle quickly sews your mouth together with 7 stitches.", victim, NULL, NULL, TO_CHAR);

  SET_BIT(victim->newbits, NEW_STITCHES);
}

void do_precognition(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int cost = 500;

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_WARLOCK) || !IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_PRECOGNITION))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Precognize whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (!IS_NPC(victim))
  {
    send_to_char("This spell has no affect on players.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eHit);

  if (ch->hit <= cost)
  {
    send_to_char("Your health cannot tolerate this.\n\r", ch);
    return;
  }
  modify_hps(ch, -1 * cost);

  act("You mindmeld with $N.", ch, NULL, victim, TO_CHAR);
  victim->precognition = ch;
}

void do_hstars(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_WARLOCK) || !IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_HUNTINGSTARS))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  TOGGLE_BIT(ch->newbits, NEW_HSTARS);

  if (IS_SET(ch->newbits, NEW_HSTARS))
  {
    act("$n flicks $s finger and 5 floating red stars pops into existance.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You create 5 floating hunting stars.\n\r", ch);
    WAIT_STATE(ch, 12);
  }
  else
  {
    act("The red stars around $n pops out of existance.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You dispel your hunting stars.\n\r", ch);
  }
}

void do_wglimmer(CHAR_DATA *ch, char *argument)
{
  SPELL_DATA *spell;
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_WARLOCK) || !IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_GLIMMER))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if ((spell = (SPELL_DATA *) FirstInList(ch->pcdata->spells)) == NULL)
  {
    send_to_char("But you are not casting a spell.\n\r", ch);
    return;
  }
  if (spell->argument[0] == '\0')
  {
    send_to_char("You must first set a focus for this spell.\n\r", ch);
    return;
  }

  if (argument[0] == '\0')
  {
    int i = 0;

    send_to_char("                         [=====] Valid Glimmers [=====]\n\n\r", ch);
    while (waffect_table[i].name[0] != '\0')
    {
      sprintf(buf, " %-18.18s", waffect_table[i++].name);
      send_to_char(buf, ch);
      if (i % 4 == 0) send_to_char("\n\r", ch);
    }
    if (i % 4 != 0) send_to_char("\n\r", ch);

    return;
  }

  sprintf(buf, "%s %s", spell->argument, argument);
  free_string(spell->argument);
  spell->argument = str_dup(buf);

  act_brief("$n traces a glowing sigil in the air.", ch, NULL, NULL, TO_ROOM, BRIEF_7);
  act("You trace a targeting sigil in the air.", ch, NULL, NULL, TO_CHAR);
}

void do_wcast(CHAR_DATA *ch, char *argument)
{
  SPELL_DATA *spell;

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_WARLOCK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if ((spell = (SPELL_DATA *) FirstInList(ch->pcdata->spells)) == NULL)
  {
    send_to_char("But you are not casting a spell.\n\r", ch);
    return;
  }
  if (spell->argument[0] == '\0')
  {
    send_to_char("The current spell has no target, you cannot cast this chain.\n\r", ch);
    return;
  }
  if (spell->type == 0)
  {
    send_to_char("The current spell has no type, you cannot cast this chain.\n\r", ch);
    return;
  }

  discharge_chain(ch, 100, SPELL_TARGET_PERSON + SPELL_TARGET_LOCAL, 0, TRUE);
}

void discharge_contingency(CHAR_DATA *ch)
{
  SPELL_DATA *spell;

  if ((spell = (SPELL_DATA *) FirstInList(ch->pcdata->contingency)) == NULL)
  {
    bug("Discharge_contingency: null spell", 0);
    return;
  }

  discharge_chain(ch, 100, SPELL_TARGET_PERSON + SPELL_TARGET_LOCAL, 0, FALSE);
}

void discharge_chain(CHAR_DATA * ch, int power, int tg, int excl, bool spellchain)
{
  OBJ_DATA *obj = NULL;
  CHAR_DATA *victim = NULL;
  LIST *list;
  SPELL_DATA *spell;
  ITERATOR *pIter;
  char target[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char *arguments;
  bool inherit_target = FALSE, inherit_exclude = FALSE;
  bool obj_target = FALSE, failed = FALSE;
  int residue;

  /* track down the last spell in the chain */
  spell = (SPELL_DATA *) ((spellchain) ? LastInList(ch->pcdata->spells) : LastInList(ch->pcdata->contingency));

  /* set chain power if any */
  if (spell->power != 0)
    power = spell->power;

  /* copy the target */
  arguments = one_argument(spell->argument, target);

  /* If this chain does not have a target set,
   * we will inherit from the previous chain.
   */
  if (!IS_SET(spell->flags, SPELL_TARGET_ROOM) &&
      !IS_SET(spell->flags, SPELL_TARGET_PERSON) &&
      !IS_SET(spell->flags, SPELL_TARGET_OBJECT) &&
      !IS_SET(spell->flags, SPELL_TARGET_LOCAL) &&
      !IS_SET(spell->flags, SPELL_TARGET_GLOBAL))
  {
    inherit_target = TRUE;
  }
  else
  {
    tg = 0;

    /* defaults to SPELL_TARGET_PERSON */
    if (IS_SET(spell->flags, SPELL_TARGET_ROOM))
      tg += SPELL_TARGET_ROOM;
    else if (IS_SET(spell->flags, SPELL_TARGET_OBJECT))
      tg += SPELL_TARGET_OBJECT;
    else
      tg += SPELL_TARGET_PERSON;

    /* defaults to SPELL_TARGET_LOCAL */
    if (IS_SET(spell->flags, SPELL_TARGET_GLOBAL))
      tg += SPELL_TARGET_GLOBAL;
    else
      tg += SPELL_TARGET_LOCAL;
  }

  /* If this chain does not have any excludes set,
   * we will inherit the excludes from the previous
   * chain.
   */
  if (!IS_SET(spell->flags, SPELL_EXCLUDE_TARGET) &&
      !IS_SET(spell->flags, SPELL_EXCLUDE_MOBILES) &&
      !IS_SET(spell->flags, SPELL_EXCLUDE_PLAYERS) &&
      !IS_SET(spell->flags, SPELL_EXCLUDE_GROUP) &&
      !IS_SET(spell->flags, SPELL_EXCLUDE_NONGROUP))
  {
    inherit_exclude = TRUE;
  }
  else
  {
    excl = 0;

    if (IS_SET(spell->flags, SPELL_EXCLUDE_TARGET))
      excl += SPELL_EXCLUDE_TARGET;
    if (IS_SET(spell->flags, SPELL_EXCLUDE_MOBILES))
      excl += SPELL_EXCLUDE_MOBILES;
    if (IS_SET(spell->flags, SPELL_EXCLUDE_PLAYERS))
      excl += SPELL_EXCLUDE_PLAYERS;
    if (IS_SET(spell->flags, SPELL_EXCLUDE_GROUP))
      excl += SPELL_EXCLUDE_GROUP;
    if (IS_SET(spell->flags, SPELL_EXCLUDE_NONGROUP))
      excl += SPELL_EXCLUDE_NONGROUP;
  }

  /* here we grab the target of this chain,
   * if this chain fails to find a target, we
   * will continue with the following chain.
   */
  if (IS_SET(spell->flags, SPELL_TARGET_GLOBAL) || (inherit_target && IS_SET(tg, SPELL_TARGET_GLOBAL)))
  {
    if (IS_SET(spell->flags, SPELL_TARGET_OBJECT) || (inherit_target && IS_SET(tg, SPELL_TARGET_OBJECT)))
    {
      if ((obj = get_obj_carry(ch, target)) != NULL)
      {
        obj_target = TRUE;
      }
      else if (!str_cmp(target, "auto") && ch->fighting != NULL)
      {
        victim = ch->fighting;
      }
      else if ((victim = get_char_room(ch, target)) == NULL)
      {
        send_to_char("The spell fizzles, local target was not found.\n\r", ch);
        failed = TRUE;
      }
    }
    /* FIX ME - the auto part does not exit in the above section... that's why
     *
     * I think I should have commented this FIX ME a bit better, because know
     * that I read this comment a few years later, I have no idea what I'm talking
     * about. Seems there is something to fix here, just not sure what or why :)
     */
    else if (!str_cmp(target, "auto") && ch->fighting != NULL)
    {
      victim = ch->fighting;
    }
    else if ((victim = get_char_world(ch, target)) == NULL)
    {
      send_to_char("The spell fizzles, global target was not found.\n\r", ch);
      failed = TRUE;
    }

    /* you can only chainbomb someone who is standing in the same area as yourself,
     * even if you are doing a global chant.
     */
    if (ch->in_room != NULL && victim != NULL && victim->in_room != NULL && victim->in_room->area != ch->in_room->area)
    {
      send_to_char("The spell fizzles, global spells cannot cross area boundaries.\n\r", ch);
      failed = TRUE;
    }
  }
  else
  {
    if (IS_SET(spell->flags, SPELL_TARGET_OBJECT) || (inherit_target && IS_SET(tg, SPELL_TARGET_OBJECT)))
    {
      if ((obj = get_obj_carry(ch, target)) != NULL)
      {
        obj_target = TRUE;
      }
      else
      {
        send_to_char("The spell fizzles, object was not found.\n\r", ch);
        failed = TRUE;
      }
    }
    else if (!str_cmp(target, "auto") && ch->fighting != NULL)
    {  
      victim = ch->fighting;
    }
    else if ((victim = get_char_homingdevice(ch, target)) == NULL)
    {
      send_to_char("The spell fizzles, local target was not found.\n\r", ch);
      failed = TRUE;
    }
  }

  /* spell didn't fail, and we found a victim */
  if (failed == FALSE && victim != NULL)
  {
    if (victim->in_room != NULL && ch->in_room != NULL)
    {
      if ((IS_SET(victim->in_room->room_flags, ROOM_ASTRAL) && !IS_SET(ch->in_room->room_flags, ROOM_ASTRAL)) ||
          (IS_SET(!victim->in_room->room_flags, ROOM_ASTRAL) && IS_SET(ch->in_room->room_flags, ROOM_ASTRAL)))
      {
        send_to_char("You cannot cast spells across the astral sphere.\n\r", ch);
        failed = TRUE;
      }
      if ((in_arena(ch) && !in_arena(victim)) || (!in_arena(ch) && in_arena(victim)))
      {
        send_to_char("You cannot cast spells into or out of the arena.\n\r", ch);
        failed = TRUE;
      }
      if ((in_fortress(ch) && !in_fortress(victim)) || (!in_fortress(ch) && in_fortress(victim)))
      {
        send_to_char("You cannot cast spells into or out of the fortress.\n\r", ch);
        failed = TRUE;
      }
    }
    else
    {
      send_to_char("You or your victim is bugged - report this.\n\r", ch);
      bug("discharge_chain: target is not in any room", 0);
      failed = TRUE;
    }
  }

  /* if we have a spell to dischage, do it */
  if (failed == FALSE)
  {
    switch (spell->type)
    {
      default:
        send_to_char("You have found a bug, please report this.\n\r", ch);
        return;
      case SPELL_TYPE_HEAL:
        if (IS_SET(spell->flags, SPELL_TARGET_ROOM) || (inherit_target && IS_SET(tg, SPELL_TARGET_ROOM)))
        {
          CHAR_DATA *targ_char = victim;
          int cost = 15 * power;

          if (ch->in_room != victim->in_room)
            cost *= 2;

          if ((residue = get_cost(ch, cost)) != 0)
          {
            sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
            send_to_char(buf, ch);
            break;
          }

          pIter = AllocIterator(victim->in_room->people);
          while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
          {
            if (spell_excludes_char(spell, excl, ch, victim, targ_char, inherit_exclude))
              continue;

            modify_hps(victim, 8 * power);

            if (victim != ch)
            {
              act("You are healed by $n's healing spell.", ch, NULL, victim, TO_VICT);
              act("Your healing spell strikes $N.", ch, NULL, victim, TO_CHAR);
            }
            else
              act("You heal yourself.", ch, NULL, NULL, TO_CHAR);
          }
        }
        else if (IS_SET(spell->flags, SPELL_TARGET_OBJECT) || (inherit_target && IS_SET(tg, SPELL_TARGET_OBJECT)))
        {
          int cost = 7 * power;

          if (IS_SET(spell->flags, SPELL_TARGET_GLOBAL) || (inherit_target && IS_SET(tg, SPELL_TARGET_GLOBAL)))
          {  
            cost *= 2;

            if ((residue = get_cost(ch, cost)) != 0)
            {
              sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
              send_to_char(buf, ch);
              break;
            }

            /* find the first object in the list we want to affect */
            if (obj_target)
            {
              if ((victim = obj->carried_by) == NULL)
              {
                send_to_char("You have encountered a bug, please report this.", ch);
                break;
              }
              else
              {
                list = obj->carried_by->carrying;
              }
            }
            else
            {
              list = victim->carrying;
              if (SizeOfList(list) == 0)
              {
                send_to_char("They are not carrying any items.\n\r", ch);
                break;
              }
            }

            /* spell message */
            act("$n waves $s fingers and all of $N's equipment shimmers briefly.", ch, NULL, victim, TO_NOTVICT);
            act("You heal all of $N's equipment, making it slightly better.", ch, NULL, victim, TO_CHAR);
            act("$n waves $s fingers at you, and your equipment shimmers briefly.", ch, NULL, victim, TO_VICT);

            pIter = AllocIterator(list);
            while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
            {
              if (IS_OBJ_STAT(obj, ITEM_NOREPAIR))
                continue;

              obj->condition = 100;
              obj->toughness = 100;
              obj->resistance = 10;
            }
          }
          else
          {
            if ((residue = get_cost(ch, cost)) != 0)
            {
              sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
              send_to_char(buf, ch);
              break;
            }

            /* find the first object in the list we want to affect */
            if (obj_target)
            {
              if ((victim = obj->carried_by) == NULL)
              {
                send_to_char("You have encountered a bug, please report this.", ch);
                break;
              }
            }
            else /* pick the first item the person is carrying */
            {
              if (SizeOfList(victim->carrying) == 0)
              {
                send_to_char("They are not carrying any items.\n\r", ch);
                break;
              }

              obj = (OBJ_DATA *) FirstInList(victim->carrying);
            }

            /* spell message */
            act("$n waves $s fingers at $N.", ch, NULL, victim, TO_NOTVICT);
            act("You heal some of $N's equipment.", ch, NULL, victim, TO_CHAR);
            act("$n waves $s fingers at you.", ch, NULL, victim, TO_VICT);
            act("$p shimmers briefly.", victim, obj, NULL, TO_ALL);

            if (!IS_OBJ_STAT(obj, ITEM_NOREPAIR))
            {
              obj->condition = 100;
              obj->toughness = 100;
              obj->resistance = 10;
            }
          }
        }
        else /* spell targets one person */
        {
          int cost = 7 * power;
          int multi = 8;

          if (victim->in_room != ch->in_room)
            cost *= 2;

          if ((residue = get_cost(ch, cost)) != 0)
          {
            sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
            send_to_char(buf, ch);
            break;
          }

          /* in pk heals are scaled down for lowrank players */
          if (victim == ch && ch->fight_timer > 0 && ch->fighting != NULL && !IS_NPC(ch->fighting))
          {
            int chMight = getMight(ch), vcMight = getMight(ch->fighting), maxMight = UMAX(chMight, vcMight);

            if (maxMight < RANK_VETERAN)
              multi = 5;
            else if (maxMight < RANK_HERO)
              multi = 6;
            else if (maxMight < RANK_MASTER)
              multi = 7;
          }

          modify_hps(victim, power * multi);

          if (victim != ch)
          {
            act("You are healed by $n's healing spell.", ch, NULL, victim, TO_VICT);
            act("Your healing spell strikes $N.", ch, NULL, victim, TO_CHAR);
          }
          else
            act("You heal yourself.", ch, NULL, NULL, TO_CHAR);
        }
        break;
      case SPELL_TYPE_DAMAGE:
        if (IS_SET(spell->flags, SPELL_TARGET_ROOM) || (inherit_target && IS_SET(tg, SPELL_TARGET_ROOM)))
        {
          CHAR_DATA *targ_char = victim;
          int cost = 10 * power;

          if (ch->in_room != victim->in_room)
            cost *= 2;

          if ((residue = get_cost(ch, cost)) != 0)
          {
            sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
            send_to_char(buf, ch);
            break;
          }

          pIter = AllocIterator(victim->in_room->people);
          while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
          {
            /* damage spell never hurts yourself */
            if (ch == victim)
              continue;

            if (spell_excludes_char(spell, excl, ch, victim, targ_char, inherit_exclude))
              continue;

            /* can we attack? */
            if (is_safe(ch, victim))
              continue;

            spell_attack(ch, victim, power);
          }
        }
        else if (IS_SET(spell->flags, SPELL_TARGET_OBJECT) || (inherit_target && IS_SET(tg, SPELL_TARGET_OBJECT)))
        {
          int cost = 7 * power;

          if (IS_SET(spell->flags, SPELL_TARGET_GLOBAL) || (inherit_target && IS_SET(tg, SPELL_TARGET_GLOBAL)))
          {
            cost *= 2;

            if ((residue = get_cost(ch, cost)) != 0)
            {
              sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
              send_to_char(buf, ch);
              break;
            }

            /* find the first object in the list we want to affect */
            if (obj_target)
            {
              if ((victim = obj->carried_by) == NULL)
              {
                send_to_char("You have encountered a bug, please report this.", ch);
                break;
              }
              else
              {
                list = victim->carrying;
              }
            }
            else
            {
              list = victim->carrying;
              if (SizeOfList(list) == 0)
              {
                send_to_char("They are not carrying any items.\n\r", ch);
                break;
              }
            }
            if (victim == ch)
            {
              send_to_char("You really don't want to do that to yourself.\n\r", ch);
              break;
            }

            if (is_safe(ch, victim))
              break;

            /* spell message */
            act("$n unleashes a lightning storm upon $N.", ch, NULL, victim, TO_NOTVICT);
            act("You unleash a lightning storm upon $N.", ch, NULL, victim, TO_CHAR);
            act("$n unleashes a lightning storm upon you.", ch, NULL, victim, TO_VICT);

            pIter = AllocIterator(list);
            while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
            {
              damage_obj(ch, obj, number_range(power, 120));
            }
          }
          else
          {
            if ((residue = get_cost(ch, cost)) != 0)
            {
              sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
              send_to_char(buf, ch);
              break;
            }

            /* find the first object in the list we want to affect */
            if (obj_target)
            {
              send_to_char("You really don't want to destroy your own equipment.\n\r", ch);
              break;
            }
            else /* pick the first item the person is carrying */
            {
              list = victim->carrying;
              if (SizeOfList(list) == 0)
              {
                send_to_char("They are not carrying any items.\n\r", ch);
                break;
              }

              obj = (OBJ_DATA *) FirstInList(list);
            }
            if (victim == ch)
            {
              send_to_char("You really don't want to do that to yourself.\n\r", ch);
              break;
            }

            if (is_safe(ch, victim))
              break;

            /* spell message */
            act("$n unleashes a lightning blast at $N.", ch, NULL, victim, TO_NOTVICT);
            act("You unleash a lightning blast at $N.", ch, NULL, victim, TO_CHAR);  
            act("$n unleashes a lightning blast at you.", ch, NULL, victim, TO_VICT);

            damage_obj(ch, obj, number_range(power, 150));
          }

          aggress(ch, victim);
        }
        else /* spell targets one person */
        {
          int cost = 5 * power;

          if (victim->in_room != ch->in_room)
            cost *= 2;

          if (ch == victim)
          {
            send_to_char("You do not want to harm yourself.\n\r", ch);
            return;
          }

          if (is_safe(ch, victim))
            break;

          if ((residue = get_cost(ch, cost)) != 0)
          {
            sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
            send_to_char(buf, ch);
            break;
          }

          spell_attack(ch, victim, power);
        }
        break;
      case SPELL_TYPE_AFFECT:
        if (IS_SET(spell->flags, SPELL_TARGET_ROOM) || (inherit_target && IS_SET(tg, SPELL_TARGET_ROOM)))
        {
          CHAR_DATA *targ_char = victim;
          int cost = 25 * power;

          if ((residue = get_cost(ch, cost)) != 0)
          {
            sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
            send_to_char(buf, ch);
            break;
          }

          pIter = AllocIterator(victim->in_room->people);
          while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
          {
            if (spell_excludes_char(spell, excl, ch, victim, targ_char, inherit_exclude))
              continue;

            spell_boost_char(ch, victim, power);

            if (ch != victim)
            {
              act("$n's shimmergloom spell strikes $N.", ch, NULL, victim, TO_NOTVICT);
              act("$n's shimmergloom spell strikes you.", ch, NULL, victim, TO_VICT);
              act("Your shimmergloom spell strikes $N.", ch, NULL, victim, TO_CHAR);
            }
            else
            {
              act("$n casts a shimmergloom spell about $mself.", ch, NULL, victim, TO_ROOM);
              act("You cast a shimmergloom spell on yourself.", ch, NULL, victim, TO_CHAR);
            }
          }
        }
        else if (IS_SET(spell->flags, SPELL_TARGET_OBJECT) || (inherit_target && IS_SET(tg, SPELL_TARGET_OBJECT)))
        {
          EVENT_DATA *event;
          AFFECT_DATA paf;
          int cost = 7 * power;

          if (IS_SET(spell->flags, SPELL_TARGET_GLOBAL) || (inherit_target && IS_SET(tg, SPELL_TARGET_GLOBAL)))
          {
            cost *= 2;

            if ((residue = get_cost(ch, cost)) != 0)
            {
              sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
              send_to_char(buf, ch);
              break;
            }

            /* find the first object in the list we want to affect */
            if (obj_target)
            {
              if ((victim = obj->carried_by) == NULL)
              {
                send_to_char("You have encountered a bug, please report this.", ch);
                break;
              }
              else
              {
                list = victim->carrying;
              }
            }
            else
            {
              list = victim->carrying;
              if (SizeOfList(list) == 0)
              {
                send_to_char("They are not carrying any items.\n\r", ch);
                break;
              }
            }

            /* spell message */
            act("$n lays a powerful enchantment on $N's equipment.", ch, NULL, victim, TO_NOTVICT);
            act("You lay a powerful enchantment on $N's equipment.", ch, NULL, victim, TO_CHAR);
            act("$n lays a powerful enchantment on your equipment.", ch, NULL, victim, TO_VICT);

            pIter = AllocIterator(list);
            while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
            {
              /* only affect worn and un-enchanted objects */
              if (obj->wear_loc != WEAR_NONE && !event_isset_object(obj, EVENT_OBJECT_AFFECTS))
              {
                /* enhanced damroll and hitroll */
                paf.type           = 0;
                paf.duration       = power * number_range(12, 24) / 100;
                paf.location       = APPLY_DAMROLL;
                paf.modifier       = number_range(3, 5);
                paf.bitvector      = 0;
                affect_to_obj(obj, &paf);
                if (obj->carried_by)
                  affect_modify(obj->carried_by, &paf, TRUE);
                paf.location       = APPLY_HITROLL;
                affect_to_obj(obj, &paf);
                if (obj->carried_by)
                  affect_modify(obj->carried_by, &paf, TRUE);

                event              =  alloc_event();
                event->fun         =  &event_object_affects;
                event->type        =  EVENT_OBJECT_AFFECTS;
                add_event_object(event, obj, 20 * PULSE_PER_SECOND);
              }
            }
          }
          else
          { 
            if ((residue = get_cost(ch, cost)) != 0)
            {
              sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
              send_to_char(buf, ch);
              break;
            }

            /* find the first object in the list we want to affect */
            if (obj_target)
            {
              if ((victim = obj->carried_by) == NULL)
              {
                send_to_char("You have encountered a bug, please report this.", ch);
                break;
              }
            }
            else /* pick the first item the person is carrying */
            {
              list = victim->carrying;
              if (SizeOfList(list) == 0)
              {
                send_to_char("They are not carrying any items.\n\r", ch);
                break;
              }

              obj = (OBJ_DATA *) FirstInList(list);
            }

            /* spell message */
            act("$n lays a powerful enchantment on some of $N's equipment.", ch, NULL, victim, TO_NOTVICT);
            act("You lay a powerful enchantment on some of $N's equipment.", ch, NULL, victim, TO_CHAR);
            act("$n lays a powerful enchantment on some of your equipment.", ch, NULL, victim, TO_VICT);

            /* only affect worn and un-enchanted objects */
            if (!event_isset_object(obj, EVENT_OBJECT_AFFECTS))
            {
              /* enhanced damroll and hitroll */
              paf.type           = 0;
              paf.duration       = power * number_range(12, 24) / 100;
              paf.location       = APPLY_DAMROLL;
              paf.modifier       = number_range(3, 5);
              paf.bitvector      = 0;
              affect_to_obj(obj, &paf);
              if (obj->carried_by && obj->wear_loc != WEAR_NONE)
                affect_modify(obj->carried_by, &paf, TRUE);
              paf.location       = APPLY_HITROLL;
              affect_to_obj(obj, &paf);
              if (obj->carried_by && obj->wear_loc != WEAR_NONE)
                affect_modify(obj->carried_by, &paf, TRUE);

              event              =  alloc_event();
              event->fun         =  &event_object_affects;
              event->type        =  EVENT_OBJECT_AFFECTS;
              add_event_object(event, obj, 20 * PULSE_PER_SECOND);

              act("$p shimmers for a moment", victim, obj, NULL, TO_ALL);
            }
            else
            {
              act("The spell fails.", ch, NULL, NULL, TO_ALL);
            }
          }
        }
        else /* spell targets one person */
        {
          int cost = 15 * power;

          if ((residue = get_cost(ch, cost)) != 0)
          {
            sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
            send_to_char(buf, ch);
            break;
          }

          spell_boost_char(ch, victim, power);

          if (ch != victim)
          {
            act("$n's shimmergloom spell strikes $N.", ch, NULL, victim, TO_NOTVICT);
            act("$n's shimmergloom spell strikes you.", ch, NULL, victim, TO_VICT);
            act("Your shimmergloom spell strikes $N.", ch, NULL, victim, TO_CHAR);
          }
          else
          {
            act("$n casts a shimmergloom spell about $mself.", ch, NULL, victim, TO_ROOM);
            act("You cast a shimmergloom spell on yourself.", ch, NULL, victim, TO_CHAR);
          }
        }
        break;
      case SPELL_TYPE_TRAIN:
        if (IS_SET(spell->flags, SPELL_TARGET_ROOM) || (inherit_target && IS_SET(tg, SPELL_TARGET_ROOM)))
        {
          CHAR_DATA *targ_char = victim;
          AFFECT_DATA paf;
          int sn, cost = 35 * power;

          if ((residue = get_cost(ch, cost)) != 0)
          {
            sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
            send_to_char(buf, ch);
            break;
          }

          pIter = AllocIterator(victim->in_room->people);
          while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
          {
            if (spell_excludes_char(spell, excl, ch, victim, targ_char, inherit_exclude))
              continue;

            if ((sn = skill_lookup("orange sorcery")) > 0)
            {
              if (!is_affected(victim, sn))
              {
                paf.type = sn;
                paf.duration = 2 + power * number_range(20, 30) / 100;
                paf.location = APPLY_HIT;
                paf.modifier = -1 * (power * number_range(500, 2500)) / 100;
                paf.bitvector = 0;
                affect_to_char(victim, &paf);
                paf.location = APPLY_MANA;
                affect_to_char(victim, &paf);
                paf.location = APPLY_MOVE;
                affect_to_char(victim, &paf);
              }
            }

            if (ch != victim)
            {
              act("$n's plague spell strikes $N.", ch, NULL, victim, TO_NOTVICT);
              act("$n's plague spell strikes you.", ch, NULL, victim, TO_VICT);
              act("Your plague spell strikes $N.", ch, NULL, victim, TO_CHAR);
            }
            else
            {
              act("$n casts a plague spell on $mself.", ch, NULL, victim, TO_ROOM);
              act("You cast a plague spell on yourself.", ch, NULL, victim, TO_CHAR);
            }
          }
        }
        else if (IS_SET(spell->flags, SPELL_TARGET_OBJECT) || (inherit_target && IS_SET(tg, SPELL_TARGET_OBJECT)))
        {
          EVENT_DATA *event;
          AFFECT_DATA paf;
          int cost = 10 * power;

          if (IS_SET(spell->flags, SPELL_TARGET_GLOBAL) || (inherit_target && IS_SET(tg, SPELL_TARGET_GLOBAL)))
          {
            cost *= 3;

            if ((residue = get_cost(ch, cost)) != 0)
            {
              sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
              send_to_char(buf, ch);
              break;
            }

            /* find the first object in the list we want to affect */
            if (obj_target)
            {
              if ((victim = obj->carried_by) == NULL)
              {
                send_to_char("You have encountered a bug, please report this.", ch);
                break;
              }
              else
              {
                list = victim->carrying;
              }
            }
            else
            {
              list = victim->carrying;
              if (SizeOfList(list) == 0)
              {
                act("$N is not carrying any items.", ch, NULL, victim, TO_CHAR);
                break;
              }
            }
             
            /* spell message */
            act("$n lays a powerful enchantment on $N's equipment.", ch, NULL, victim, TO_NOTVICT);
            act("You lay a powerful enchantment on $N's equipment.", ch, NULL, victim, TO_CHAR);
            act("$n lays a powerful enchantment on your equipment.", ch, NULL, victim, TO_VICT);

            pIter = AllocIterator(list);
            while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
            {
              /* only affect worn and un-enchanted objects */
              if (obj->wear_loc != WEAR_NONE && !event_isset_object(obj, EVENT_OBJECT_PLAGUE))
              {
                /* enchant the item */
                paf.type           = 0;
                paf.duration       = power * number_range(12, 24) / 100;
                paf.location       = APPLY_HIT;
                paf.modifier       = -1 * number_range(5, 25);
                paf.bitvector      = 0;
                affect_to_obj(obj, &paf);
                if (obj->carried_by)
                  affect_modify(obj->carried_by, &paf, TRUE);
                paf.location       = APPLY_MOVE;
                affect_to_obj(obj, &paf);
                if (obj->carried_by)
                  affect_modify(obj->carried_by, &paf, TRUE);
                paf.location       = APPLY_MANA;   
                affect_to_obj(obj, &paf);
                if (obj->carried_by)
                  affect_modify(obj->carried_by, &paf, TRUE);

                event              =  alloc_event();
                event->fun         =  &event_object_affects;
                event->type        =  EVENT_OBJECT_PLAGUE;
                add_event_object(event, obj, 20 * PULSE_PER_SECOND);
              }
            }
          }
          else
          {  
            if ((residue = get_cost(ch, cost)) != 0)
            {
              sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
              send_to_char(buf, ch);
              break;
            }

            /* find the first object in the list we want to affect */
            if (obj_target)
            {
              if ((victim = obj->carried_by) == NULL)
              {
                send_to_char("You have encountered a bug, please report this.", ch);
                break;
              }
            }   
            else /* pick the first item the person is carrying */
            {
              list = victim->carrying;
              if (SizeOfList(list) == 0)
              {
                send_to_char("They are not carrying any items.\n\r", ch);
                break;
              }

              obj = (OBJ_DATA *) FirstInList(list);
            }

            /* spell message */
            act("$n lays a powerful enchantment on some of $N's equipment.", ch, NULL, victim, TO_NOTVICT);
            act("You lay a powerful enchantment on some of $N's equipment.", ch, NULL, victim, TO_CHAR);
            act("$n lays a powerful enchantment on some of your equipment.", ch, NULL, victim, TO_VICT);

            /* only affect worn and un-enchanted objects */
            if (!event_isset_object(obj, EVENT_OBJECT_PLAGUE))
            {
              /* enchant the item */
              paf.type           = 0;
              paf.duration       = power * number_range(12, 24) / 100;
              paf.location       = APPLY_HIT;
              paf.modifier       = -1 * number_range(5, 25);
              paf.bitvector      = 0;
              affect_to_obj(obj, &paf);
              if (obj->carried_by && obj->wear_loc != WEAR_NONE)
                affect_modify(obj->carried_by, &paf, TRUE);
              paf.location       = APPLY_MOVE;
              affect_to_obj(obj, &paf);
              if (obj->carried_by && obj->wear_loc != WEAR_NONE)
                affect_modify(obj->carried_by, &paf, TRUE);
              paf.location       = APPLY_MANA;
              affect_to_obj(obj, &paf);
              if (obj->carried_by && obj->wear_loc != WEAR_NONE)
                affect_modify(obj->carried_by, &paf, TRUE);

              event              =  alloc_event();
              event->fun         =  &event_object_affects;
              event->type        =  EVENT_OBJECT_PLAGUE;
              add_event_object(event, obj, 20 * PULSE_PER_SECOND);

              act("$p shimmers for a moment", victim, obj, NULL, TO_ALL);
            }
            else
            {
              act("The spell fails.", ch, NULL, NULL, TO_ALL);
            }
          }
        }
        else /* spell targets one person */
        {
          AFFECT_DATA paf;
          int sn, cost = 20 * power;

          if ((residue = get_cost(ch, cost)) != 0)
          {
            sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
            send_to_char(buf, ch);
            break;
          }

          if ((sn = skill_lookup("orange sorcery")) > 0)
          {
            if (!is_affected(victim, sn))
            {
              paf.type = sn;
              paf.duration = 1 + power * number_range(20, 30) / 100;
              paf.location = APPLY_HIT;
              paf.modifier = -1 * (power * number_range(500, 2500)) / 100;
              paf.bitvector = 0;
              affect_to_char(victim, &paf);
              paf.location = APPLY_MANA;
              affect_to_char(victim, &paf);
              paf.location = APPLY_MOVE;
              affect_to_char(victim, &paf);
            }
          }

          if (ch != victim)
          {
            act("$n's plague spell strikes $N.", ch, NULL, victim, TO_NOTVICT);
            act("$n's plague spell strikes you.", ch, NULL, victim, TO_VICT);
            act("Your plague spell strikes $N.", ch, NULL, victim, TO_CHAR);
          }
          else
          {
            act("$n casts a plague spell on $mself.", ch, NULL, victim, TO_ROOM);
            act("You cast a plague spell on yourself.", ch, NULL, victim, TO_CHAR);
          }
        }
        break;
      case SPELL_TYPE_GLIMMER:
        if (IS_SET(spell->flags, SPELL_TARGET_ROOM) || (inherit_target && IS_SET(tg, SPELL_TARGET_ROOM)))
        {
          AFFECT_DATA paf;
          CHAR_DATA *targ_char = victim;
          int sn, bit = 0, cost = 35 * power;
          char ptr[MAX_INPUT_LENGTH];

          if ((residue = get_cost(ch, cost)) != 0)
          {
            sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
            send_to_char(buf, ch);
            break;
          }

          arguments = one_argument(arguments, ptr);
          while (ptr[0] != '\0')
          {
            int i;

            for (i = 0; waffect_table[i].name[0] != '\0'; i++) 
            {
              if (!str_cmp(waffect_table[i].name, ptr))
                bit += waffect_table[i].bit;
            }

            arguments = one_argument(arguments, ptr);
          }

          if (bit == 0)
          {
            send_to_char("This spell contains no valid affects.\n\r", ch);
            break;
          }

          pIter = AllocIterator(victim->in_room->people);
          while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
          {
            if (spell_excludes_char(spell, excl, ch, victim, targ_char, inherit_exclude))
              continue;

            if ((sn = skill_lookup("orange sorcery")) > 0)
            {
              if (!is_affected(victim, sn))
              { 
                paf.type = sn;
                paf.duration = UMAX(1, power * number_range(20, 30) / 100);
                paf.location = APPLY_NONE;
                paf.modifier = 0;
                paf.bitvector = bit;
                affect_to_char(victim, &paf);
              }
            }  

            if (ch != victim)
            {
              act("$n's glimmer spell strikes $N.", ch, NULL, victim, TO_NOTVICT);   
              act("$n's glimmer spell strikes you.", ch, NULL, victim, TO_VICT);
              act("Your glimmer spell strikes $N.", ch, NULL, victim, TO_CHAR);
            }
            else
            {
              act("$n casts a glimmer spell on $mself.", ch, NULL, victim, TO_ROOM);
              act("You cast a glimmer spell on yourself.", ch, NULL, victim, TO_CHAR);
            }
          }
        }
        else if (IS_SET(spell->flags, SPELL_TARGET_PERSON) || (inherit_target && IS_SET(tg, SPELL_TARGET_PERSON)))
        {
          AFFECT_DATA paf;
          int sn, bit = 0, cost = 20 * power;
          char ptr[MAX_INPUT_LENGTH];

          if ((residue = get_cost(ch, cost)) != 0)
          {
            sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
            send_to_char(buf, ch);
            break;
          }

          arguments = one_argument(arguments, ptr);
          while (ptr[0] != '\0')
          {   
            int i;    
    
            for (i = 0; waffect_table[i].name[0] != '\0'; i++)
            {
              if (!str_cmp(waffect_table[i].name, ptr))
                bit += waffect_table[i].bit;
            }
            
            arguments = one_argument(arguments, ptr);
          }

          if (bit == 0)
          {
            send_to_char("This spell contains no valid affects.\n\r", ch);
            break;
          }

          if ((sn = skill_lookup("orange sorcery")) > 0)
          {
            if (!is_affected(victim, sn))
            {
              paf.type = sn;
              paf.duration = UMAX(3, power * number_range(20, 30) / 100);
              paf.location = APPLY_NONE;
              paf.modifier = 0;
              paf.bitvector = bit;
              affect_to_char(victim, &paf);
            }
          }

          if (ch != victim)
          {
            act("$n's glimmer spell strikes $N.", ch, NULL, victim, TO_NOTVICT);
            act("$n's glimmer spell strikes you.", ch, NULL, victim, TO_VICT);
            act("Your glimmer spell strikes $N.", ch, NULL, victim, TO_CHAR);
          }
          else
          {
            act("$n casts a glimmer spell on $mself.", ch, NULL, victim, TO_ROOM);
            act("You cast a glimmer spell on yourself.", ch, NULL, victim, TO_CHAR);
          }
        }
        else
        {
          send_to_char("Glimmer spells can only target living beings.\n\r", ch);
        }
        break;
    }
  }

  /* remove this spell from the chain */
  spell_from_char(spell, ch, spellchain);

  /* discharge next spell in chain, if any */
  if ((spellchain && SizeOfList(ch->pcdata->spells) > 0) || (!spellchain && SizeOfList(ch->pcdata->contingency) > 0))
    discharge_chain(ch, power, tg, excl, spellchain);
}

void do_wtarget(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  SPELL_DATA *spell;
  const int cost = 100;
  bool valid = FALSE;
  int residue;

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_WARLOCK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if ((spell = (SPELL_DATA *) FirstInList(ch->pcdata->spells)) == NULL)
  {
    send_to_char("But you are not casting a spell.\n\r", ch);
    return;
  }
  if (argument[0] == '\0')
  {
    send_to_char("What is the scale of this spells target?\n\r", ch);
    return;
  }
  if ((residue = get_cost(ch, cost)) != 0)
  {
    sprintf(buf, "You need %d more mana.\n\r", residue);
    send_to_char(buf, ch);
    return;
  }

  if (is_full_name("person", argument))
  {
    REMOVE_BIT(spell->flags, SPELL_TARGET_ROOM);
    REMOVE_BIT(spell->flags, SPELL_TARGET_OBJECT);
    SET_BIT(spell->flags, SPELL_TARGET_PERSON);
    valid = TRUE;
  }
  else if (is_full_name("room", argument))
  {
    REMOVE_BIT(spell->flags, SPELL_TARGET_OBJECT);
    REMOVE_BIT(spell->flags, SPELL_TARGET_PERSON);
    SET_BIT(spell->flags, SPELL_TARGET_ROOM);
    valid = TRUE;
  }
  else if (is_full_name("object", argument))
  {
    REMOVE_BIT(spell->flags, SPELL_TARGET_ROOM);
    REMOVE_BIT(spell->flags, SPELL_TARGET_PERSON);
    SET_BIT(spell->flags, SPELL_TARGET_OBJECT);
    valid = TRUE;
  }

  if (!valid)
  {
    send_to_char("You need to pick a valid target for this spell.\n\r", ch);
    return;
  }

  if (is_full_name("local", argument))
  {
    REMOVE_BIT(spell->flags, SPELL_TARGET_GLOBAL);
    SET_BIT(spell->flags, SPELL_TARGET_LOCAL);
  }
  else if (is_full_name("global", argument))
  {
    SET_BIT(spell->flags, SPELL_TARGET_GLOBAL);
    REMOVE_BIT(spell->flags, SPELL_TARGET_LOCAL);
  }

  act_brief("$n traces a humming sigil in the air.", ch, NULL, NULL, TO_ROOM, BRIEF_7);
  act("You trace a humming targeting sigil in the air.", ch, NULL, NULL, TO_CHAR);
}

void do_wexclude(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  SPELL_DATA *spell;
  const int cost = 100;
  bool valid = FALSE;
  int residue;

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_WARLOCK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if ((spell = (SPELL_DATA *) FirstInList(ch->pcdata->spells)) == NULL)
  {
    send_to_char("But you are not casting a spell.\n\r", ch);
    return;
  }
  if (argument[0] == '\0')
  {
    send_to_char("Whom should this spell not affect?\n\r", ch);
    return;
  }
  if ((residue = get_cost(ch, cost)) != 0)
  {
    sprintf(buf, "You need %d more mana.\n\r", residue);
    send_to_char(buf, ch);
    return;
  }

  if (is_full_name("target", argument))
  {
    SET_BIT(spell->flags, SPELL_EXCLUDE_TARGET);
    valid = TRUE;
  }
  if (is_full_name("mobiles", argument))
  {
    SET_BIT(spell->flags, SPELL_EXCLUDE_MOBILES);
    valid = TRUE;
  }
  if (is_full_name("players", argument))
  {
    SET_BIT(spell->flags, SPELL_EXCLUDE_PLAYERS);
    valid = TRUE;
  }
  if (is_full_name("group", argument))
  {
    SET_BIT(spell->flags, SPELL_EXCLUDE_GROUP);
    valid = TRUE;
  }
  if (is_full_name("nongroup", argument))
  {
    SET_BIT(spell->flags, SPELL_EXCLUDE_NONGROUP);
    valid = TRUE;
  }

  if (!valid)
  {
    send_to_char("Whom do you wish to exclude from this spell?\n\r", ch);
    return;
  }

  act_brief("$n waves his hand, trailing magical sparks.", ch, NULL, NULL, TO_ROOM, BRIEF_7);
  act("You wave your hand, exluding a targetgroup.", ch, NULL, NULL, TO_CHAR);
}

void do_wtype(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  SPELL_DATA *spell;
  const int cost = 100;
  int residue;

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_WARLOCK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if ((spell = (SPELL_DATA *) FirstInList(ch->pcdata->spells)) == NULL)
  {
    send_to_char("But you are not casting a spell.\n\r", ch);
    return;
  }
  if (argument[0] == '\0')
  {
    send_to_char("What kind of spell is this?\n\r", ch);
    return;
  }
  if ((residue = get_cost(ch, cost)) != 0)
  {
    sprintf(buf, "You need %d more mana.\n\r", residue);
    send_to_char(buf, ch);
    return;
  }

  if (!str_cmp(argument, "heal"))
  {
    spell->type = SPELL_TYPE_HEAL;
    act_brief("$n traces a Mulharandi rune in the air.", ch, NULL, NULL, TO_ROOM, BRIEF_7);
    act("You trace a Mulharandi healing rune in the air.", ch, NULL, NULL, TO_CHAR);
  }
  else if (!str_cmp(argument, "damage"))
  {
    spell->type = SPELL_TYPE_DAMAGE;
    act_brief("$n traces a Haruunic glyph in the air.", ch, NULL, NULL, TO_ROOM, BRIEF_7);
    act("You trace a Haruunic explosive glyph in the air.", ch, NULL, NULL, TO_CHAR);
  }
  else if (!str_cmp(argument, "affect"))
  {
    spell->type = SPELL_TYPE_AFFECT;
    act_brief("$n traces a Soltac sigil in the air.", ch, NULL, NULL, TO_ROOM, BRIEF_7);
    act("You trace a Soltac sigil of strength in the air.", ch, NULL, NULL, TO_CHAR);
  }
  else if (!str_cmp(argument, "plague") && IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_PLAGUE))
  {
    spell->type = SPELL_TYPE_TRAIN;
    act_brief("$n traces a Dorican symbol of disease in the air.", ch, NULL, NULL, TO_ROOM, BRIEF_7);
    act("You trace a Dorican symbol of disease in the air.", ch, NULL, NULL, TO_CHAR);
  }
  else if (!str_cmp(argument, "glimmer") && IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_GLIMMER))
  {
    spell->type = SPELL_TYPE_GLIMMER;
    act_brief("$n traces a K'tryan line of change in the air.", ch, NULL, NULL, TO_ROOM, BRIEF_7);
    act("You trace a K'tryan line of change in the air.", ch, NULL, NULL, TO_CHAR);
  }
  else
  {
    send_to_char("You don't know any spells of that type.\n\r", ch);
    return;
  }
}

void do_wfocus(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  SPELL_DATA *spell;
  char arg[MAX_STRING_LENGTH];
  const int cost = 100;
  int residue;

  if (IS_NPC(ch))
    return;
  if (!IS_CLASS(ch, CLASS_WARLOCK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if ((spell = (SPELL_DATA *) FirstInList(ch->pcdata->spells)) == NULL)
  {
    send_to_char("But you are not casting a spell.\n\r", ch);
    return;
  }
  if (argument[0] == '\0')
  {
    send_to_char("Whom or what should this spell be directed at?\n\r", ch);
    return;
  }
  if ((residue = get_cost(ch, cost)) != 0)
  {
    sprintf(buf, "Spell fizzled, you need %d more mana.\n\r", residue);
    send_to_char(buf, ch);
    return;
  }

  one_argument(argument, arg);
  free_string(spell->argument);
  spell->argument = str_dup(arg);

  act_brief("$n traces a glowing sigil in the air.", ch, NULL, NULL, TO_ROOM, BRIEF_7);
  act("You trace a targeting sigil in the air.", ch, NULL, NULL, TO_CHAR);
}

void spell_attack(CHAR_DATA *ch, CHAR_DATA *victim, int power)
{
  int dam = power * 10 + char_damroll(ch);

  if ((IS_SET(ch->in_room->room_flags, ROOM_PRIVATE) || IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)) &&
        ch->in_room != victim->in_room)
  {
    send_to_char("Your attack fizzles, you cast global spells to or from private rooms.\n\r", ch);
    return;
  }
  if ((IS_SET(ch->in_room->room_flags, ROOM_ASTRAL) || IS_SET(victim->in_room->room_flags, ROOM_ASTRAL)) &&
       ch->in_room != victim->in_room)
  {
    send_to_char("Your attack fizzles, they are not connected to the same astral sphere as you.\n\r", ch);
    return;
  }
  if ((IS_SET(ch->in_room->room_flags, ROOM_SAFE) && ch->fight_timer == 0) ||
      (IS_SET(victim->in_room->room_flags, ROOM_SAFE) && victim->fight_timer == 0))
  {
    send_to_char("Your attack fizzles, they are safe from attacks.\n\r", ch);
    return;
  }

  /* modify damage */
  dam = number_range(3 * dam / 4, 5 * dam / 4);
  dam = up_dam(ch, victim, dam);
  dam = cap_dam(ch, victim, dam);

  damage(ch, victim, NULL, dam, gsn_chantspell);

  if (ch->fight_timer < 10 && !IS_NPC(victim))
    ch->fight_timer += 3;

  if (ch->fighting && ch->in_room != ch->fighting->in_room)
    stop_fighting(ch, TRUE);

  /* global chanting huge monsters to death should be difficult */
  if (IS_NPC(victim) && !victim->dead && victim->position != POS_FIGHTING)
    SET_BIT(victim->extra, EXTRA_HEAL);
}

void do_lifedrain(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int drain;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct necromancy level */
  if (ch->pcdata->powers[SPHERE_NECROMANCY] < 2)
  {
    send_to_char("Your level of expertise in necromancy is to low.\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Lifedrain whom?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (is_safe(ch, victim)) return;
  if (ch == victim)
  {
    send_to_char("You really don't want to do that.\n\r", ch);
    return;
  }
  if (100 * victim->hit / victim->max_hit < 75)
  {
    send_to_char("They are not healthy enough.\n\r", ch);
    return;
  }
  drain = UMIN(2500, 10 * victim->hit / 100);

  act("You drain the lifeforce from $N.", ch, NULL, victim, TO_CHAR);
  act("$n drains the lifeforce from $N.", ch, NULL, victim, TO_NOTVICT);
  act("$n drains the lifeforce from your body.", ch, NULL, victim, TO_VICT);

  aggress(ch, victim);

  hurt_person(ch, victim, drain);
  modify_mana(ch, drain);
  WAIT_STATE(ch, 18);
}

void do_tarukeye(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA af;
  CHAR_DATA *victim;
  EVENT_DATA *event;
  int cost = 2000;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct necromancy level */
  if (ch->pcdata->powers[SPHERE_NECROMANCY] < 3)
  {
    send_to_char("Your level of expertise in necromancy is to low.\n\r", ch);
    return;
  }
  if ((event = event_isset_mobile(ch, EVENT_PLAYER_TARUK)) != NULL)
  {
    printf_to_char(ch, "You must wait %s before you can use this power again.\n\r", event_time_left(event));
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0' && ch->fighting == NULL)
  {
    send_to_char("Curse whom with the eye of Taruk?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    if ((victim = ch->fighting) == NULL)
    {
      send_to_char("They are not here.\n\r", ch);
      return;
    }
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to cast this spell.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("You curse $N with the eye of Taruk, stealing $S vision.", ch, NULL, victim, TO_CHAR);
  act("$n utters an incantation, and your senses dwindle.", ch, NULL, victim, TO_VICT);
  act("$n utters an incantation, and $N's eyes goes blank.", ch, NULL, victim, TO_NOTVICT);

  SET_BIT(ch->act, PLR_HOLYLIGHT);
  if (is_affected(ch, gsn_blindness))
    affect_strip(ch, gsn_blindness);

  REMOVE_BIT(victim->act, PLR_HOLYLIGHT);
  af.type      = skill_lookup("blindness");
  af.location  = APPLY_HITROLL;
  af.modifier  = -4;
  af.duration  = 60;
  af.bitvector = AFF_BLIND;
  affect_to_char(victim, &af);

  event = alloc_event();
  event->type = EVENT_PLAYER_TARUK;
  event->fun = &event_dummy;
  add_event_char(event, ch, 3 * 60 * PULSE_PER_SECOND);

  WAIT_STATE(ch, 12);
}

void do_chillbolt(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  EVENT_DATA *event;
  CHAR_DATA *victim;
  int cost = 750;
  int dam;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct necromancy level */
  if (ch->pcdata->powers[SPHERE_NECROMANCY] < 4)
  {
    send_to_char("Your level of expertise in necromancy is to low.\n\r", ch);
    return;
  }
  if ((event = event_isset_mobile(ch, EVENT_PLAYER_CHILLBOLT)) != NULL)
  {
    sprintf(buf, "You cannot cast a chillbolt for another %d seconds",
      event_pulses_left(event) / PULSE_PER_SECOND);
    send_to_char(buf, ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Blast whom with a chillbolt?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  dam = number_range(ch->pcdata->powers[SPHERE_NECROMANCY] * 150,
                     ch->pcdata->powers[SPHERE_NECROMANCY] * 250);
  dam = up_dam(ch, victim, dam);
  dam = cap_dam(ch, victim, dam);
  damage(ch, victim, NULL, dam, gsn_chillbolt);

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to cast a chillbolt.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  WAIT_STATE(victim, ch->pcdata->powers[SPHERE_NECROMANCY] * 3);
  WAIT_STATE(ch, 6);

  event             = alloc_event();
  event->fun        = &event_dummy;
  event->type       = EVENT_PLAYER_CHILLBOLT;
  add_event_char(event, ch, 20 * PULSE_PER_SECOND);
}

void do_deathspell(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *gch;
  ITERATOR *pIter;
  int cost = 850;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct necromancy level */
  if (ch->pcdata->powers[SPHERE_NECROMANCY] < 5)
  {
    send_to_char("Your level of expertise in necromancy is to low.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, NULL, cost, eMana);

  if (ch->mana < cost)
  {
    send_to_char("You do not have enough mana to cast the deathspell.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("Black lightning blasts from your hands, engulfing the room,", ch, NULL, NULL, TO_CHAR);
  act("Black lightning blasts from $n's hands, engulfing the room,", ch, NULL, NULL, TO_ROOM);

  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    int dam = number_range(200 * ch->pcdata->powers[SPHERE_NECROMANCY],
                           600 * ch->pcdata->powers[SPHERE_NECROMANCY]);

    if (ch == gch || !IS_NPC(gch)) continue;
    if (is_safe(ch, gch)) continue;

    if (gch->level < 75)
    {
      act("$N screams like a wretched little creature, as death comes crawling.", ch, NULL, gch, TO_ALL);
      stop_fighting(gch, FALSE);
      gch->position = POS_STUNNED;
    }

    damage(ch, gch, NULL, dam, gsn_deathspell);
  }

  WAIT_STATE(ch, 8);
}

void do_pvipers(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];
  int cost = 750;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct invocation level */
  if (ch->pcdata->powers[SPHERE_INVOCATION] < 3)
  {
    send_to_char("Your level of expertise in invocation is to low.\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Whom do you want to engulf in prismatic vipers?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (ch == victim)
  {
    send_to_char("You really don't want to do that to yourself.\n\r", ch);
    return;
  }

  if (is_safe(ch, victim))
    return;

  if (event_isset_mobile(victim, EVENT_PLAYER_PVIPER_RED) ||
      event_isset_mobile(victim, EVENT_PLAYER_PVIPER_GREEN) ||
      event_isset_mobile(victim, EVENT_PLAYER_PVIPER_BLUE) ||
      event_isset_mobile(victim, EVENT_PLAYER_PVIPER_YELLOW) ||
      event_isset_mobile(victim, EVENT_PLAYER_PVIPER_PURPLE))
  {
    send_to_char("They already have an active viper on them.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to summon the prismatic vipers.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  /* messages to player, victim and room */
  act("$n calls forth five prismatic vipers, ordering them to attack $N.", ch, NULL, victim, TO_NOTVICT);
  act("You call forth five prismatic vipers and order them to attack $N.", ch, NULL, victim, TO_CHAR);
  act("$n calls forth five prismatic vipers and orders them to attack you.", ch, NULL, victim, TO_VICT);

  sprintf(arg, "%d", ch->spl[RED_MAGIC]);
  event             = alloc_event();
  event->argument   = str_dup(arg);
  event->type       = EVENT_PLAYER_PVIPER_RED;
  event->fun        = &event_player_pviper_red;
  add_event_char(event, victim, 3 * PULSE_PER_SECOND);

  sprintf(arg, "%d", ch->spl[PURPLE_MAGIC]);
  event             = alloc_event();
  event->argument   = str_dup(arg);
  event->type       = EVENT_PLAYER_PVIPER_PURPLE;
  event->fun        = &event_player_pviper_purple;
  add_event_char(event, victim, 3 * PULSE_PER_SECOND);

  sprintf(arg, "%d", ch->spl[YELLOW_MAGIC]);
  event             = alloc_event();
  event->argument   = str_dup(arg);
  event->type       = EVENT_PLAYER_PVIPER_YELLOW;
  event->fun        = &event_player_pviper_yellow;
  add_event_char(event, victim, 3 * PULSE_PER_SECOND);

  sprintf(arg, "%d", ch->spl[GREEN_MAGIC]);
  event             = alloc_event();
  event->argument   = str_dup(arg);
  event->type       = EVENT_PLAYER_PVIPER_GREEN;
  event->fun        = &event_player_pviper_green;
  add_event_char(event, victim, 3 * PULSE_PER_SECOND);

  sprintf(arg, "%d", ch->spl[BLUE_MAGIC]);
  event             = alloc_event();
  event->argument   = str_dup(arg);
  event->type       = EVENT_PLAYER_PVIPER_BLUE;
  event->fun        = &event_player_pviper_blue;
  add_event_char(event, victim, 3 * PULSE_PER_SECOND);

  aggress(ch, victim);
}

void do_flamestorm(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *gch;
  ITERATOR *pIter;
  int mana_cost = 1500;
  int sn;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct invocation level */
  if (ch->pcdata->powers[SPHERE_INVOCATION] < 4)
  {
    send_to_char("Your level of expertise in invocation is to low.\n\r", ch);
    return;
  }

  if (ch->mana < mana_cost)
  {
    send_to_char("You do not have enough mana to cast flamestorm.\n\r", ch);
    return;
  }

  if ((sn = skill_lookup("flamestorm")) < 0)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }

  modify_mana(ch, -1 * mana_cost);

  act("You spread your fingers and engulf the room in flames.", ch, NULL, NULL, TO_CHAR);
  act("$n spreads $s fingers and engulf the room in flames.", ch, NULL, NULL, TO_ROOM);

  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    int dam;

    /* only attack those that can be attacked */
    if (is_full_name("players", argument) && !IS_NPC(gch))
      continue;
    else if (is_full_name("mobiles", argument) && IS_NPC(gch))
      continue;
    else if (is_full_name("group", argument) && is_same_group(gch, ch))
      continue;
    else if (is_full_name("nongroup", argument) && !is_same_group(gch, ch))
      continue;
    else if (gch == ch || is_safe(ch, gch))
      continue;

    /* calculate damage and fire away */
    dam = 150 * ch->pcdata->powers[SPHERE_INVOCATION];
    dam = number_range(4 * dam, 8 * dam);
    dam = cap_dam(ch, gch, dam);

    damage(ch, gch, NULL, dam, sn);
  }

  WAIT_STATE(ch, 12);
}

void do_meteorstrike(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int cost = 500;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct invocation level */
  if (ch->pcdata->powers[SPHERE_INVOCATION] < 2)
  {
    send_to_char("Your level of expertise in invocation is to low.\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Strike whom with a single meteor?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to cast a meteor strike.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  one_hit(ch, victim, gsn_meteor, 1);

  WAIT_STATE(ch, 8);
}

void do_meteorswarm(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int cost = 1000;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct invocation level */
  if (ch->pcdata->powers[SPHERE_INVOCATION] < 5)
  {
    send_to_char("Your level of expertise in invocation is to low.\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Strike whom with a swarm of meteors?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to cast a meteor swarm.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  one_hit(ch, victim, gsn_meteor, 1);
  one_hit(ch, victim, gsn_meteor, 1);
  one_hit(ch, victim, gsn_meteor, 1);

  WAIT_STATE(ch, 12);
}

void do_magicvest(CHAR_DATA *ch, char *argument)
{
  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct invocation level */
  if (ch->pcdata->powers[SPHERE_ABJURATION] < 1)
  {
    send_to_char("Your level of expertise in abjuration is to low.\n\r", ch);
    return;
  }

  TOGGLE_BIT(ch->affected_by, AFF_MVEST);
  if (IS_SET(ch->affected_by, AFF_MVEST))
  {
    act("$n cloaks $mself in a magical vestment.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You cloak yourself in a magical vestment.\n\r", ch);
  }
  else
  {
    act("$n banishes $s magical vestment.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You banish your magical vestment.\n\r", ch);
  }
}

void do_catalyze(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  char buf[MAX_STRING_LENGTH];
  int gain;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }
  
  /* check for correct invocation level */
  if (ch->pcdata->powers[SPHERE_ABJURATION] < 5)
  {
    send_to_char("Your level of expertise in abjuration is to low.\n\r", ch);
    return;
  }
  if ((event = event_isset_mobile(ch, EVENT_PLAYER_CATALYST)) == NULL)
  {
    send_to_char("You do not have a catalyst running.\n\r", ch);
    return;
  }
  gain = atoi(event->argument);
  modify_mana(ch, gain);
  sprintf(buf, "You gain %d mana from the catalyst.\n\r", gain);
  send_to_char(buf, ch);
  act("$n's eyes flashes briefly.", ch, NULL, NULL, TO_ROOM);
  dequeue_event(event, TRUE);
}

void do_catalyst(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct invocation level */
  if (ch->pcdata->powers[SPHERE_ABJURATION] < 5)
  {
    send_to_char("Your level of expertise in abjuration is to low.\n\r", ch);
    return;
  }
  if (event_isset_mobile(ch, EVENT_PLAYER_CATALYST))
  {
    send_to_char("You already have a catalyst running.\n\r", ch);
    return;
  }

  act("$n's eyes flashes briefly.", ch, NULL, NULL, TO_ROOM);
  send_to_char("You invoke the catalyst spell.\n\r", ch);

  event = alloc_event();
  event->argument = str_dup("0");
  event->type = EVENT_PLAYER_CATALYST;
  event->fun = &event_player_catalyst;
  add_event_char(event, ch, 3 * PULSE_PER_SECOND);
}

void do_fireshield(CHAR_DATA *ch, char *argument)
{
  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct abjuration level */
  if (ch->pcdata->powers[SPHERE_ABJURATION] < 3)
  {
    send_to_char("Your level of expertise in abjuration is to low.\n\r", ch);
    return;
  }

  TOGGLE_BIT(ch->affected_by, AFF_FIRESHIELD);
  if (IS_SET(ch->affected_by, AFF_FIRESHIELD))
  {
    act("$n conjures forth a fiery shield of flames.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You conjure fiery shield of flames.\n\r", ch);
  }
  else
  {
    act("$n banishes $s fiery shield of flames.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You banish your fiery shield of flames.\n\r", ch);
  }
}

void do_wallofswords(CHAR_DATA *ch, char *argument)
{
  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct abjuration level */
  if (ch->pcdata->powers[SPHERE_ABJURATION] < 2)
  {
    send_to_char("Your level of expertise in abjuration is to low.\n\r", ch);
    return;
  }

  TOGGLE_BIT(ch->affected_by, AFF_WALLSWORDS);
  if (IS_SET(ch->affected_by, AFF_WALLSWORDS))
  {
    act("$n conjures forth a wall of razorsharp swords.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You conjure forth a wall of razorsharp swords.\n\r", ch);
  }
  else
  {
    act("$n banishes $s wall of swords.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You banish your wall of swords.\n\r", ch);
  }
}

void do_shattershield(CHAR_DATA *ch, char *argument)
{
  int cost = 2000;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct invocation level */
  if (ch->pcdata->powers[SPHERE_ABJURATION] < 4)
  {
    send_to_char("Your level of expertise in abjuration is to low.\n\r", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_SHATTERSHIELD))
  {
    send_to_char("You already have a shattershield up and humming.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to summon a shattershield.\n\r", cost  - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  SET_BIT(ch->affected_by, AFF_SHATTERSHIELD);
  act("$n summons a glassy, flickering shield around $m.", ch, NULL, NULL, TO_ROOM);
  send_to_char("You invoke a shattershield enchantment.\n\r", ch);
  WAIT_STATE(ch, 6);
}

bool event_player_pviper_red(EVENT_DATA *event)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *ch;
  int level;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_pviper_red: no owner.", 0);
    return FALSE;
  }
  level = atoi(event->argument);

  if (number_percent() <= 75)
  {
    EVENT_DATA *newevent;

    act("The crimson red viper drinks deeply of your stamina.", ch, NULL, NULL, TO_CHAR);
    act("A crimson red viper drinks deeply from $n.", ch, NULL, NULL, TO_ROOM);

    modify_hps(ch, -1 * number_range(2 * level, 5 * level));
    update_pos(ch);

    sprintf(buf, "%d", level);
    newevent             = alloc_event();
    newevent->argument   = str_dup(buf);
    newevent->type       = EVENT_PLAYER_PVIPER_RED;
    newevent->fun        = &event_player_pviper_red;
    add_event_char(newevent, ch, 3 * PULSE_PER_SECOND);
  }
  else
  {
    act("The crimson red viper around $n fades away.", ch, NULL, NULL, TO_ROOM);
    send_to_char("The crimson red viper around you fades away.\n\r", ch);
  }

  return FALSE;
}

bool event_player_pviper_green(EVENT_DATA *event)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *ch;
  int level;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_pviper_green: no owner.", 0);
    return FALSE;
  }
  level = atoi(event->argument);

  if (number_percent() <= 75)
  {
    EVENT_DATA *newevent;
    int sn;

    if ((sn = skill_lookup("noxious fumes")) <= 0)
    {
      bug("Event_players_pvipers_green: no spell.", 0);
      return FALSE;
    }

    act("The scaly green viper spews noxious fumes in your head.", ch, NULL, NULL, TO_CHAR);
    act("A scaly green viper spews noxious fumes at $n's head.", ch, NULL, NULL, TO_ROOM);

    (*skill_table[sn].spell_fun) (sn, level / 50 + 1, ch, ch);

    sprintf(buf, "%d", level);
    newevent             = alloc_event();
    newevent->argument   = str_dup(buf);
    newevent->type       = EVENT_PLAYER_PVIPER_GREEN;
    newevent->fun        = &event_player_pviper_green;
    add_event_char(newevent, ch, 3 * PULSE_PER_SECOND);
  }
  else
  {
    act("The scaly green viper around $n fades away.", ch, NULL, NULL, TO_ROOM);
    send_to_char("The scaly green viper around you fades away.\n\r", ch);
  }

  return FALSE;
}

bool event_player_pviper_yellow(EVENT_DATA *event)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *ch;
  int level;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_pviper_yellow: no owner.", 0);
    return FALSE;
  }
  level = atoi(event->argument);

  if (number_percent() <= 75)
  {
    EVENT_DATA *newevent;

    act("The hissing yellow viper drinks deeply of your magical reserves.", ch, NULL, NULL, TO_CHAR);
    act("A hissing yellow viper drinks deeply from $n.", ch, NULL, NULL, TO_ROOM);

    if (ch->mana > 1)
      modify_mana(ch, -1 * number_range(2 * level, 5 * level));

    sprintf(buf, "%d", level);
    newevent             = alloc_event();
    newevent->argument   = str_dup(buf);
    newevent->type       = EVENT_PLAYER_PVIPER_YELLOW;
    newevent->fun        = &event_player_pviper_yellow;
    add_event_char(newevent, ch, 3 * PULSE_PER_SECOND);
  }
  else
  {
    act("The hissing yellow viper around $n fades away.", ch, NULL, NULL, TO_ROOM);
    send_to_char("The hissing yellow viper around you fades away.\n\r", ch);
  }

  return FALSE;
}

bool event_player_pviper_purple(EVENT_DATA *event)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *ch;
  int level;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_pviper_purple: no owner.", 0);
    return FALSE;
  }
  level = atoi(event->argument);

  if (number_percent() <= 75)
  {
    EVENT_DATA *newevent;

    act("The purple bloated viper drinks deeply of your physical energies.", ch, NULL, NULL, TO_CHAR);
    act("A purple bloated viper drinks deeply from $n.", ch, NULL, NULL, TO_ROOM);

    if (ch->move > 1)
      modify_move(ch, -1 * number_range(2 * level, 5 * level));

    sprintf(buf, "%d", level);
    newevent             = alloc_event();
    newevent->argument   = str_dup(buf);
    newevent->type       = EVENT_PLAYER_PVIPER_PURPLE;
    newevent->fun        = &event_player_pviper_purple;
    add_event_char(newevent, ch, 3 * PULSE_PER_SECOND);
  }
  else
  {
    act("The purple bloated viper around $n fades away.", ch, NULL, NULL, TO_ROOM);
    send_to_char("The purple bloated viper around you fades away.\n\r", ch);
  }

  return FALSE;
}

bool event_player_pviper_blue(EVENT_DATA *event)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *ch;
  int level;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_pviper_blue: no owner.", 0);
    return FALSE;
  }
  level = atoi(event->argument);

  if (number_percent() <= 75)
  {
    EVENT_DATA *newevent;
    int delay = number_range(3, 6);

    act("The lightning blue viper stuns you with a lightning blast.", ch, NULL, NULL, TO_CHAR);
    act("A lightning blue viper stuns $n with a lightning blast.", ch, NULL, NULL, TO_ROOM);

    WAIT_STATE(ch, delay);

    sprintf(buf, "%d", level);
    newevent             = alloc_event();
    newevent->argument   = str_dup(buf);
    newevent->type       = EVENT_PLAYER_PVIPER_BLUE;
    newevent->fun        = &event_player_pviper_blue;
    add_event_char(newevent, ch, 3 * PULSE_PER_SECOND);
  }
  else
  {
    act("The lightning blue viper around $n fades away.", ch, NULL, NULL, TO_ROOM);
    send_to_char("The lightning blue viper around you fades away.\n\r", ch);
  }

  return FALSE;
}

bool event_player_crows(EVENT_DATA *event)
{
  CHAR_DATA *ch, *gch;
  ITERATOR *pIter;
  OBJ_DATA *obj;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_crows: no owner.", 0);
    return FALSE;
  }

  act("A murder of crows swoops down from above.", ch, NULL, NULL, TO_ALL);

  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (gch->fighting != ch) continue;

    /* stealing bitching crows */
    if (number_range(1, 4) == 2 && (obj = get_eq_char(gch, WEAR_FINGER_L)) != NULL)
      steal_object(obj, gch, ch);
    if (number_range(1, 4) == 2 && (obj = get_eq_char(gch, WEAR_FINGER_R)) != NULL)
      steal_object(obj, gch, ch);
    if (number_range(1, 4) == 2 && (obj = get_eq_char(gch, WEAR_FINGER_L)) != NULL)
      steal_object(obj, gch, ch);
    if (number_range(1, 4) == 2 && (obj = get_eq_char(gch, WEAR_FINGER_R)) != NULL)
      steal_object(obj, gch, ch);
    if (number_range(1, 4) == 2 && (obj = get_eq_char(gch, WEAR_FLOAT)) != NULL)
      steal_object(obj, gch, ch);
    if (number_range(1, 4) == 2 && (obj = get_eq_char(gch, WEAR_MEDAL)) != NULL)
      steal_object(obj, gch, ch);
  }

  act("The murder of crows fly away.", ch, NULL, NULL, TO_ALL);

  /* dequeue */
  return FALSE;
}

void steal_object(OBJ_DATA *obj, CHAR_DATA *victim, CHAR_DATA *ch)
{
  act("The murder of crows steals $p from $N and gives it to $n.", ch, obj, victim, TO_ROOM);
  act("The murder of crows steals $p from $N and gives it to you.", ch, obj, victim, TO_CHAR);
  act("The murder of crows steals $p from you and gives it to $n.", ch, obj, victim, TO_VICT);
  obj_from_char(obj);
  obj_to_char(obj, ch);
}

bool event_player_leeches(EVENT_DATA *event)
{
  OBJ_DATA *obj;
  ITERATOR *pIter;
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_leeches: no owner.", 0);
    return FALSE;
  }

  pIter = AllocIterator(ch->carrying);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (object_is_affected(obj, OAFF_LIQUID))
      continue;

    if (obj->wear_loc != WEAR_NONE)
    {
      act_brief("The metal leeches gnaw and eat at $p.", ch, obj, NULL, TO_ALL, BRIEF_7);

      damage_obj(ch, obj, number_range(105, 115));
    }
  }

  if (number_percent() > 25 || (event->argument && !str_cmp(event->argument, "init")))
  {
    EVENT_DATA *newevent;

    newevent             =  alloc_event();
    newevent->type       =  EVENT_PLAYER_LEECHES;
    newevent->fun        =  &event_player_leeches;
    add_event_char(newevent, ch, 3 * PULSE_PER_SECOND);
  }
  else
  {
    act("The metal leeches crawling on $n crumbles to dust.", ch, NULL, NULL, TO_ROOM);
    act("The metal leeches crawling on you crumbles to dust.", ch, NULL, NULL, TO_CHAR);
  }

  return FALSE;
}

bool event_area_earthmother(EVENT_DATA *event)
{
  EVENT_DATA *newevent;
  AREA_DATA *pArea;
  AREA_AFFECT *paf;
  int owner = atoi(event->argument);
  const int cost = 500;

  if ((pArea = event->owner.area) == NULL)
  {
    bug("event_area_earthmother: no owner.", 0);
    return FALSE;
  }

  if ((paf = has_area_affect(pArea, AREA_AFF_EARTHMOTHER, owner)) != NULL)
  {
    CHAR_DATA *ch = NULL;

    if ((ch = get_online_player(paf->owner)) == NULL || --paf->duration <= 0)
    {
      if (ch)
        send_to_char("Your earthmother enchantment fades.\n\r", ch);
      affect_from_area(pArea, paf);
      return FALSE;
    }
    if (ch->mana < cost)
    {
      send_to_char("Your earthmother enchantment fades.\n\r", ch);
      affect_from_area(pArea, paf);
      return FALSE;
    }
    modify_mana(ch, -1 * cost);

    newevent = alloc_event();
    newevent->argument = str_dup(event->argument);
    newevent->type = EVENT_AREA_EARTHMOTHER;
    newevent->fun = &event_area_earthmother;
    add_event_area(newevent, pArea, 8 * PULSE_PER_SECOND);
  }
  else
  {
    bug("event_area_earthmother: missing affect.", 0);
  }

  return FALSE;
}

bool event_area_milkandhoney(EVENT_DATA *event)
{
  EVENT_DATA *newevent;
  AREA_DATA *pArea;
  AREA_AFFECT *paf;
  int owner = atoi(event->argument);
  const int cost = 600;
  int sn;

  if ((pArea = event->owner.area) == NULL)
  {
    bug("event_area_plague: no owner.", 0);
    return FALSE;
  }
  if ((paf = has_area_affect(pArea, AREA_AFF_MILKANDHONEY, owner)) != NULL)
  {
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch = NULL, *gch;
    ITERATOR *pIter;

    pIter = AllocIterator(descriptor_list);
    while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
    {
      if ((gch = d->character) == NULL || d->connected != CON_PLAYING) continue;
      if (gch->in_room == NULL || gch->in_room->area != pArea) continue;

      /* add some affect, with a 33% chance */
      switch(number_range(1, 12))
      {
        default:
          break;
        case 1:
          if (gch->fight_timer == 0)
          {
            send_to_char("You reap the benefits of living in the land of milk and honey.\n\r", gch);
            restore_player(gch);
          }
          break;
        case 2:
          if ((sn = skill_lookup("godbless")) <= 0)
          {
            bug("event_area_milkandhoney: spell does not exist.", 0);
          }
          else
          {
            (*skill_table[sn].spell_fun) (sn, 50, gch, gch);
          }
          break;
        case 3:
          if ((sn = skill_lookup("spirit kiss")) <= 0)
          {
            bug("event_area_milkandhoney: spell does not exist.", 0);
          }
          else
          {
            (*skill_table[sn].spell_fun) (sn, 50, gch, gch);
          }
          break;
        case 4:
          gch->wait = 0;
          gch->hit += 500; /* yes, can go over max */
          send_to_char("You feel full of vitality.\n\r", gch);
          break;
      }
    }

    if ((ch = get_online_player(paf->owner)) == NULL || --paf->duration <= 0)
    {
      if (ch)
        send_to_char("Your milk and honey enchantment fades.\n\r", ch);
      affect_from_area(pArea, paf);
      return FALSE;
    }
    if (ch->mana < cost)
    {
      send_to_char("Your milk and honey enchantment fades.\n\r", ch);
      affect_from_area(pArea, paf);
      return FALSE;
    }
    modify_mana(ch, -1 * cost);

    newevent = alloc_event();
    newevent->argument = str_dup(event->argument);
    newevent->type = EVENT_AREA_MILKANDHONEY;
    newevent->fun = &event_area_milkandhoney;
    add_event_area(newevent, pArea, 8 * PULSE_PER_SECOND);
  }
  else
  {
    bug("event_area_milkandhoney: missing affect.", 0);
  }

  return FALSE;
}

bool event_area_plague(EVENT_DATA *event)
{
  EVENT_DATA *newevent;
  AREA_DATA *pArea;
  AREA_AFFECT *paf;
  int owner = atoi(event->argument);
  const int cost = 400;

  if ((pArea = event->owner.area) == NULL)
  {
    bug("event_area_plague: no owner.", 0);
    return FALSE;
  }

  if ((paf = has_area_affect(pArea, AREA_AFF_PLAGUE, owner)) != NULL)
  {
    CHAR_DATA *ch = NULL;

    if ((ch = get_online_player(paf->owner)) == NULL || --paf->duration <= 0)
    {
      if (ch)
        send_to_char("Your enchantment of plague fades.\n\r", ch);
      affect_from_area(pArea, paf);
      return FALSE;
    }
    if (ch->mana < cost)
    {
      send_to_char("Your enchantment of plague fades.\n\r", ch);
      affect_from_area(pArea, paf);
      return FALSE;
    }
    modify_mana(ch, -1 * cost);

    newevent = alloc_event();
    newevent->argument = str_dup(event->argument);
    newevent->type = EVENT_AREA_PLAGUE;
    newevent->fun = &event_area_plague;
    add_event_area(newevent, pArea, 8 * PULSE_PER_SECOND);
  }
  else
  {
    bug("event_area_plague: missing affect.", 0);
  }

  return FALSE;
}

int get_cost(CHAR_DATA *ch, int cost)
{
  if (IS_SET(ch->affected_by, AFF_BEACON))
  {
    cost *= 7;
    cost /= 11;
  }

  if (ch->mana < cost)
  {
    return (cost - ch->mana);
  }
  modify_mana(ch, -1 * cost);

  return 0;
}

void update_warlock(CHAR_DATA *ch)
{
  /* update the active counter for this archmage */
  if (ch->pcdata->powers[WARLOCK_RANK] == WLCK_RNK_ARCHMAGE)
  {
    archmage_list[ch->pcdata->powers[WARLOCK_PATH]].active++;
  }

  if (IS_SET(ch->newbits, NEW_MOUNTAIN))
  {
    int cost = 3000;

    if (ch->move < cost)
    {
      REMOVE_BIT(ch->newbits, NEW_MOUNTAIN);
      act("$n's skin returns to its normal colour.", ch, NULL, NULL, TO_ROOM);
      send_to_char("You revert from your rockform into human.\n\r", ch);
    }
    else
    {
      modify_move(ch, -1 * cost);
    }
  }

  if (IS_SET(ch->newbits, NEW_BACKLASH))
  {
    int cost = 400;

    if (ch->mana < cost)
    {
      REMOVE_BIT(ch->newbits, NEW_BACKLASH);
      act("$n's forcefield flickers and dies.", ch, NULL, NULL, TO_ROOM);
      send_to_char("Your forcefield flickers and dies.\n\r", ch);
    }
    else
    {
      modify_mana(ch, -1 * cost);
    }
  }

  if (IS_SET(ch->newbits, NEW_HSTARS))
  {
    int cost = 500;

    if (ch->mana < cost)
    {
      REMOVE_BIT(ch->newbits, NEW_HSTARS);
      act("The red stars floating around $n vanishes.", ch, NULL, NULL, TO_ROOM);
      send_to_char("Your hunting stars vanishes with a loud POP!\n\r", ch);
    }
    else
    {
      modify_mana(ch, -1 * cost);
    }
  }

  if (!IS_ITEMAFF(ch, ITEMA_REGENERATE))
    return;

  /* regenerate body and limbs */
  if (ch->hit != ch->max_hit)
    regen_hps(ch, 2);

  if (!IS_SET(ch->affected_by, AFF_BEACON))
  {
    if (ch->mana != ch->max_mana)
    {
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_EARTHPULSE) &&
          ch->fight_timer <= 0 && ch->position != POS_FIGHTING)
        regen_mana(ch, 4);
      else
        regen_mana(ch, 2);
    }
  }
  else
  {
    if (ch->mana != ch->max_mana)
      regen_mana(ch, 1);
  }

  if (!IS_SET(ch->newbits, NEW_MOUNTAIN))
  {
    if (ch->move != ch->max_move)
      regen_move(ch, 2);
  }
  else
  {
    if (ch->move != ch->max_move)
      regen_move(ch, 1);
  }

  regen_limb(ch);
}

void do_beacon(CHAR_DATA *ch, char *argument)
{
  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct abjuration level */
  if (ch->pcdata->powers[SPHERE_ENCHANTMENT] < 6)
  {
    send_to_char("Your level of expertise in enchantment is to low.\n\r", ch);
    return;
  }

  TOGGLE_BIT(ch->affected_by, AFF_BEACON);
  if (IS_SET(ch->affected_by, AFF_BEACON))
  {
    act("$n starts glowing with a pale green light.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You submerge yourself in the weave.\n\r", ch);
  }
  else
  {
    act("The pale green glow around $n fades away.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You leave the powers of the weave behind.\n\r", ch);
  }

  WAIT_STATE(ch, 6);
}

void do_simulacrum(CHAR_DATA *ch, char *argument)
{
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *pMob;
  const int cost = 2000;
  char buf[MAX_STRING_LENGTH];

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct necromancy level */
  if (ch->pcdata->powers[SPHERE_NECROMANCY] < 6)
  {
    send_to_char("Your level of expertise in necromancy is to low.\n\r", ch);
    return;
  }
  if (ch->mana < cost)
  {
    send_to_char("You do not have enough mana to create a simulacrum.\n\r", ch);
    return;
  }
  if (ch->simulacrum)
  {
    send_to_char("You already have a simulacrum.\n\r", ch);
    return;
  }
  if (in_arena(ch) || in_fortress(ch))
  {
    send_to_char("You cannot use this power in the arena or in the fortress.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * cost);

  if ((pMobIndex = get_mob_index(MOB_VNUM_PROTOMOBILE)) == NULL)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }
  pMob = create_mobile(pMobIndex);
  pMob->sex = ch->sex;
  free_string(pMob->name);
  free_string(pMob->short_descr);
  free_string(pMob->long_descr);

  sprintf(buf, "%s simulacrum", ch->name);
  pMob->name = str_dup(buf);

  sprintf(buf, "the simulacrum of %s", ch->name);
  pMob->short_descr = str_dup(buf);

  sprintf(buf, "%s is standing here.\n\r", ch->name);
  pMob->long_descr = str_dup(buf);

  pMob->simulacrum = ch;
  ch->simulacrum = pMob;

  char_to_room(pMob, ch->in_room, TRUE);

  act("$n breaths life into $N.", ch, NULL, pMob, TO_ROOM);
  act("You breath life info $N.", ch, NULL, pMob, TO_CHAR);
}

void do_phylactery(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *gch;
  CHAR_DATA *simulacrum;
  ITERATOR *pIter;
  ROOM_INDEX_DATA *pRoom;
  const int cost = 1500;
  int failure;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }

  /* check for correct necromancy level */
  if (ch->pcdata->powers[SPHERE_NECROMANCY] < 6)
  {
    send_to_char("Your level of expertise in necromancy is to low.\n\r", ch);
    return;
  }
  if (ch->mana < cost)
  {
    send_to_char("You do not have enough mana to change places with your simulacrum.\n\r", ch);
    return;
  }
  if ((simulacrum = ch->simulacrum) == NULL)
  {
    send_to_char("You have no simulacrum.\n\r", ch);
    return;
  }
  if (in_arena(ch) || in_fortress(ch))
  {
    send_to_char("You cannot use this power in the arena or in the fortress.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * cost);

  if (simulacrum->dead || simulacrum->in_room == NULL)
  {
    ch->simulacrum = NULL;
    send_to_char("Your simulacrum is bugged, please report this.\n\r", ch);
    return;
  }
  if (in_arena(simulacrum) || in_fortress(simulacrum) || IS_SET(simulacrum->in_room->room_flags, ROOM_KINGDOM))
  {
    send_to_char("Your simulacrum does not respond to your summons.\n\r", ch);
    return;
  }

  if (ch->fighting != NULL || ch->fight_timer > 0)
  {
    if (simulacrum->in_room->area != ch->in_room->area)
      failure = 90;
    else
      failure = 40;

    if (IS_AFFECTED(ch, AFF_CURSE))
      failure += 10;

    if (number_percent() <= failure)
    {
      extract_char(simulacrum, TRUE);
      send_to_char("You feel the sudden death of your simulacrum.\n\r", ch);
      return;
    }
  }

  send_to_char("You switch places with your simulacrum.\n\r", ch);
  act("Something strange seems to have happened to $n.", ch, NULL, NULL, TO_ROOM);

  pRoom = simulacrum->in_room;
  char_from_room(simulacrum);
  char_to_room(simulacrum, ch->in_room, TRUE);

  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (gch->fighting == ch)
      gch->fighting = simulacrum;
  }

  simulacrum->fighting = ch->fighting;
  ch->fighting = NULL;
  simulacrum->position = ch->position;
  ch->position = (ch->position == POS_FIGHTING) ? POS_STANDING : ch->position;

  char_from_room(ch);
  char_to_room(ch, pRoom, TRUE);

  ch->simulacrum = NULL;
  simulacrum->simulacrum = NULL;

  ch->move = UMIN(5000, ch->move);
  ch->mana = UMIN(5000, ch->mana);
  ch->hit  = UMIN(5000, ch->hit);
}

void do_implode(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  OBJ_DATA *obj;
  char arg[MAX_INPUT_LENGTH];
  const int cost = 750;

  /* check for warlock PC class */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    do_huh(ch, "");
    return;
  }
  
  /* check for correct invocation level */
  if (ch->pcdata->powers[SPHERE_INVOCATION] < 6)
  {
    send_to_char("Your level of expertise in invocation is to low.\n\r", ch);
    return;
  }
  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Which object do you wish to implode?\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    send_to_char("You do not have enough mana to cast this enchantment.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * cost);

  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You are not carrying that object.\n\r", ch);
    return;
  }

  if (obj->ownerid != 0 && obj->ownerid != ch->pcdata->playerid)
  {
    send_to_char("You cannot implode items owned by other players.\n\r", ch);
    return;
  }

  if (obj->item_type == ITEM_CONTAINER)
  {
    send_to_char("You cannot implode containers.\n\r", ch);
    return;
  }

  event = alloc_event();
  event->type = EVENT_OBJECT_IMPLODE;
  event->fun = &event_object_implode;
  event->argument = str_dup(ch->name);
  add_event_object(event, obj, 4 * PULSE_PER_SECOND);

  act("$n mutters an arcane phrase over $p, and it starts humming.", ch, obj, NULL, TO_ROOM);
  act("You mutter an arcane phrase over $p, and it starts humming.", ch, obj, NULL, TO_CHAR);

  WAIT_STATE(ch, 6);
}

bool event_object_implode(EVENT_DATA *event)
{
  OBJ_DATA *obj;
  CHAR_DATA *ch, *gch;
  ROOM_INDEX_DATA *pRoom;
  ITERATOR *pIter;

  if ((obj = event->owner.obj) == NULL)
  {
    bug("event_obj_implode: no owner.", 0);
    return FALSE;
  }

  pIter = AllocIterator(char_list);
  while ((ch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(event->argument, ch->name))
      break;
  }

  /* if the owner of the bomb is not online, it doesn't work. */
  if (ch == NULL)
  {
    return FALSE;
  }

  if ((pRoom = locate_obj(obj)) != NULL)
  {
    pIter = AllocIterator(pRoom->people);
    if ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
      act("$p explodes in a shower of flames!", gch, obj, NULL, TO_ALL);

    do {
      int dam = number_range(1500, 3000);

      if (gch == ch) continue;
      dam = up_dam(ch, gch, dam);
      dam = cap_dam(ch, gch, dam);
      damage(ch, gch, NULL, dam, gsn_fireball);
    } while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL);
  }
  extract_obj(obj);

  return TRUE;
}

void do_eyespy(CHAR_DATA *ch, char *argument)
{
  MOB_INDEX_DATA *eyeIndex;
  CHAR_DATA *eye;
  OBJ_DATA *sphere = NULL;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_WARLOCK))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  /* check for correct divination level */
  if (ch->pcdata->powers[SPHERE_DIVINATION] < 6)
  {
    send_to_char("Your level of expertise in divination is to low.\n\r", ch);
    return;
  }

  argument = one_argument(argument, arg);
  if (!str_cmp(arg, "sphere") && (sphere = get_obj_room(ch, arg)) != NULL)
  {
    if (sphere->item_type != ITEM_PORTAL || sphere->value[0] == 0)
    {
      send_to_char("That is not a scrying sphere.", ch);
      return;
    }
  }

  if (ch->pcdata->familiar)
  {
    send_to_char("You already have a familiar.\n\r", ch);
    return;
  }

  if (IS_HEAD(ch, LOST_EYE_L) && IS_HEAD(ch, LOST_EYE_R))
  {
    send_to_char("You do not have any eyes to use.\n\r", ch);
    return;
  }
  if (!IS_HEAD(ch, LOST_EYE_L))
  {
    if (sphere)
    {
      act("You rip out your left eye and drop it into $p.", ch, sphere, NULL, TO_CHAR);
      act("$n rips out $s left eye and drops it into $p.", ch, sphere, NULL, TO_ROOM);
    }
    else
    {
      act("You rip out your left eye and drop it on the ground.", ch, NULL, NULL, TO_CHAR);
      act("$n rips out $s left eye and drops it on the ground.", ch, NULL, NULL, TO_ROOM);
    }
    SET_BIT(ch->loc_hp[0], LOST_EYE_L);
  }
  else
  {
    if (sphere)
    {
      act("You rip out your right eye and drop it into $p.", ch, sphere, NULL, TO_CHAR);
      act("$n rips out $s right eye and drops it into $p.", ch, sphere, NULL, TO_ROOM);
    }
    else
    {
      act("You rip out your right eye and drop it on the ground.", ch, NULL, NULL, TO_CHAR);
      act("$n rips out $s right eye and drops it on the ground.", ch, NULL, NULL, TO_ROOM);
    }
    SET_BIT(ch->loc_hp[0], LOST_EYE_R);
  }

  if ((eyeIndex = get_mob_index(MOB_VNUM_EYE)) == NULL)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }
  eye = create_mobile(eyeIndex);

  if (sphere)
  {
    ROOM_INDEX_DATA *pRoom;

    if ((pRoom = get_room_index(sphere->value[0])) != NULL)
    {
      char_to_room(eye, pRoom, TRUE);
    }
    else
    {
      char_to_room(eye, ch->in_room, TRUE);
      act("$n explodes in a shower of flames.", ch, NULL, NULL, TO_ALL);
      extract_char(eye, TRUE);
    }
  }
  else
  {
    char_to_room(eye, ch->in_room, TRUE);
  }

  ch->pcdata->familiar = eye;
  eye->wizard = ch;
}

/* check to see if a given spell excludes victim */
bool spell_excludes_char(SPELL_DATA *spell, int excl, CHAR_DATA *ch, CHAR_DATA *victim,
                         CHAR_DATA *targ_char, bool inherit_exclude)
{
  /* exclude players */
  if (!IS_NPC(victim) && (IS_SET(spell->flags, SPELL_EXCLUDE_PLAYERS) ||
     (inherit_exclude && IS_SET(excl, SPELL_EXCLUDE_PLAYERS))))         
    return TRUE;

  /* exclude mobiles */
  if (IS_NPC(victim) && (IS_SET(spell->flags, SPELL_EXCLUDE_MOBILES) ||
     (inherit_exclude && IS_SET(excl, SPELL_EXCLUDE_MOBILES))))        
    return TRUE;

  /* exclude target */
  if (targ_char == victim && (IS_SET(spell->flags, SPELL_EXCLUDE_TARGET) ||
     (inherit_exclude && IS_SET(excl, SPELL_EXCLUDE_TARGET))))             
    return TRUE;

  /* exclude group */
  if (is_same_group(victim, ch) && (IS_SET(spell->flags, SPELL_EXCLUDE_GROUP) ||
     (inherit_exclude && IS_SET(excl, SPELL_EXCLUDE_GROUP))))                   
    return TRUE;

  /* exclude non-group */
  if (!is_same_group(victim, ch) && (IS_SET(spell->flags, SPELL_EXCLUDE_NONGROUP) ||
     (inherit_exclude && IS_SET(excl, SPELL_EXCLUDE_NONGROUP))))                    
    return TRUE;

  return FALSE;
}

void spell_boost_char(CHAR_DATA *ch, CHAR_DATA *victim, int power)
{
  AFFECT_DATA paf;
  int sn;

  if ((sn = skill_lookup("purple sorcery")) > 0)
  {
    if (!is_affected(victim, sn))
    {
      paf.type = sn;
      paf.duration = power * (10 + ch->spl[PURPLE_MAGIC] / 5) / 100;
      paf.location = APPLY_HITROLL;
      paf.modifier = UMAX(5, ch->spl[PURPLE_MAGIC] / 10);
      paf.bitvector = 0;
      affect_to_char(victim, &paf);
      paf.location = APPLY_DAMROLL;
      affect_to_char(victim, &paf);
      paf.location = APPLY_AC;
      paf.modifier = UMIN(-10, ch->spl[PURPLE_MAGIC] / 5);
      affect_to_char(victim, &paf);                     
    }
  }
  if ((sn = skill_lookup("yellow sorcery")) > 0)
  {
    if (!is_affected(victim, sn))
    {
      paf.type = sn;
      paf.duration = power * (10 + ch->spl[YELLOW_MAGIC] / 5) / 100;
      paf.location = APPLY_HITROLL;
      paf.modifier = UMAX(5, ch->spl[YELLOW_MAGIC] / 10);
      paf.bitvector = 0;
      affect_to_char(victim, &paf);
      paf.location = APPLY_DAMROLL;
      affect_to_char(victim, &paf);
      paf.location = APPLY_AC;
      paf.modifier = UMIN(-10, ch->spl[YELLOW_MAGIC] / 5);
      affect_to_char(victim, &paf);
    }
  }
  if ((sn = skill_lookup("green sorcery")) > 0)
  {
    if (!is_affected(victim, sn))
    {
      paf.type = sn;
      paf.duration = power * (10 + ch->spl[GREEN_MAGIC] / 5) / 100;
      paf.location = APPLY_HITROLL;
      paf.modifier = UMAX(5, ch->spl[GREEN_MAGIC] / 10);
      paf.bitvector = 0;
      affect_to_char(victim, &paf);
      paf.location = APPLY_DAMROLL;
      affect_to_char(victim, &paf);
      paf.location = APPLY_AC;     
      paf.modifier = UMIN(-10, ch->spl[GREEN_MAGIC] / 5);
      affect_to_char(victim, &paf);                      
    }
  }
  if ((sn = skill_lookup("blue sorcery")) > 0)
  {
    if (!is_affected(victim, sn))
    {
      paf.type = sn;
      paf.duration = power * (10 + ch->spl[BLUE_MAGIC] / 5) / 100;
      paf.location = APPLY_HITROLL;
      paf.modifier = UMAX(5, ch->spl[BLUE_MAGIC] / 10);
      paf.bitvector = 0;
      affect_to_char(victim, &paf);
      paf.location = APPLY_DAMROLL;
      affect_to_char(victim, &paf);
      paf.location = APPLY_AC;
      paf.modifier = UMIN(-10, ch->spl[BLUE_MAGIC] / 5);
      affect_to_char(victim, &paf);
    }
  }
  if ((sn = skill_lookup("red sorcery")) > 0)
  {
    if (!is_affected(victim, sn))
    {
      paf.type = sn;
      paf.duration = power * (10 + ch->spl[RED_MAGIC] / 5) / 100;
      paf.location = APPLY_HITROLL;
      paf.modifier = UMAX(5, ch->spl[RED_MAGIC] / 10);
      paf.bitvector = 0;
      affect_to_char(victim, &paf);
      paf.location = APPLY_DAMROLL;
      affect_to_char(victim, &paf);
      paf.location = APPLY_AC;
      paf.modifier = UMIN(-10, ch->spl[RED_MAGIC] / 5);
      affect_to_char(victim, &paf);
    }
  }
}
