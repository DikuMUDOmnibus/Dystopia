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

int   calculate_dam              ( CHAR_DATA *ch, CHAR_DATA *victim, int type );
void  elemboost                  ( CHAR_DATA *ch, int level );
bool  event_mobile_pipemove      ( EVENT_DATA *event );
bool  event_player_sacrifice_fae ( EVENT_DATA *event );

const struct evolve_entry fae_evolve_table[] =
{
  { "arcane", 0, 0,
    EVOLVE_1, FAE_EVOLVE_ARCANE, EVOLVE_1, FAE_EVOLVE_NATURE,
    10000, 25000, 15000, 50000000, 5000
  },

  { "nature", 0, 0,
    EVOLVE_1, FAE_EVOLVE_NATURE, EVOLVE_1, FAE_EVOLVE_ARCANE,
    10000, 15000, 25000, 50000000, 5000
  },

  { "spirit combat", 0, 0,
    EVOLVE_1, FAE_EVOLVE_SPIRIT, EVOLVE_1, FAE_EVOLVE_DRAGON,
    25000, 15000, 15000, 50000000, 5000
  },

  { "dragons breath", 0, 0,
    EVOLVE_1, FAE_EVOLVE_DRAGON, EVOLVE_1, FAE_EVOLVE_SPIRIT,
    30000, 10000, 10000, 50000000, 5000
  },

  { "blood sacrifice", 0, 0,
    EVOLVE_1, FAE_EVOLVE_SACRIFICE, EVOLVE_1, FAE_EVOLVE_WALL,
    22000, 20000, 18000, 50000000, 5000
  },

  { "prismatic wall", 0, 0,
    EVOLVE_1, FAE_EVOLVE_WALL, EVOLVE_1, FAE_EVOLVE_SACRIFICE,
    18000, 20000, 22000, 50000000, 5000
  },

  { "chameleon skin", EVOLVE_1, FAE_EVOLVE_ARCANE,
    EVOLVE_2, FAE_EVOLVE_CHAMELEON, EVOLVE_2, FAE_EVOLVE_WARDING,
    30000, 40000, 25000, 200000000, 10000
  },

  { "warding glyphs", EVOLVE_1, FAE_EVOLVE_ARCANE,
    EVOLVE_2, FAE_EVOLVE_WARDING, EVOLVE_2, FAE_EVOLVE_CHAMELEON,
    25000, 40000, 30000, 200000000, 10000
  },

  { "absorbing shield", EVOLVE_1, FAE_EVOLVE_NATURE,
    EVOLVE_2, FAE_EVOLVE_ABSORB, EVOLVE_2, FAE_EVOLVE_DEFLECT,
    25000, 30000, 40000, 200000000, 10000
  },

  { "deflecting shield", EVOLVE_1, FAE_EVOLVE_NATURE,
    EVOLVE_2, FAE_EVOLVE_DEFLECT, EVOLVE_2, FAE_EVOLVE_ABSORB,
    30000, 25000, 40000, 200000000, 10000
  },

  { "haunting spirits", EVOLVE_1, FAE_EVOLVE_SPIRIT,
    EVOLVE_2, FAE_EVOLVE_HAUNTING, EVOLVE_2, FAE_EVOLVE_BLASTBEAMS,
    25000, 25000, 40000, 200000000, 10000
  },

  { "blastbeams", EVOLVE_1, FAE_EVOLVE_SPIRIT,
    EVOLVE_2, FAE_EVOLVE_BLASTBEAMS, EVOLVE_2, FAE_EVOLVE_HAUNTING,
    30000, 30000, 30000, 200000000, 10000
  },

  { "freeze ancient", EVOLVE_1, FAE_EVOLVE_DRAGON,
    EVOLVE_2, FAE_EVOLVE_FREEZE, EVOLVE_2, FAE_EVOLVE_HALO,
    28000, 35000, 28000, 200000000, 10000
  },

  { "halo", EVOLVE_1, FAE_EVOLVE_DRAGON,
    EVOLVE_2, FAE_EVOLVE_HALO, EVOLVE_2, FAE_EVOLVE_FREEZE,
    25000, 35000, 35000, 200000000, 10000
  },

  { "prismatic spray", EVOLVE_1, FAE_EVOLVE_WALL,
    EVOLVE_2, FAE_EVOLVE_PSPRAY, EVOLVE_2, FAE_EVOLVE_PBLAST,
    35000, 40000, 20000, 200000000, 10000
  },

  { "prismatic chainblast", EVOLVE_1, FAE_EVOLVE_WALL,
    EVOLVE_2, FAE_EVOLVE_PBLAST, EVOLVE_2, FAE_EVOLVE_PSPRAY,
    35000, 20000, 35000, 200000000, 10000
  },

  { "blood immunity", EVOLVE_1, FAE_EVOLVE_SACRIFICE,
    EVOLVE_2, FAE_EVOLVE_BLOODIMMUNE, EVOLVE_2, FAE_EVOLVE_ACIDBLOOD,
    40000, 20000, 30000, 200000000, 10000
  },

  { "blood to acid", EVOLVE_1, FAE_EVOLVE_SACRIFICE,
    EVOLVE_2, FAE_EVOLVE_ACIDBLOOD, EVOLVE_2, FAE_EVOLVE_BLOODIMMUNE,
    30000, 40000, 30000, 200000000, 10000
  },

  { "acid heart", EVOLVE_2, FAE_EVOLVE_ACIDBLOOD,
    EVOLVE_3, FAE_EVOLVE_ACIDHEART, EVOLVE_3, FAE_EVOLVE_BLOODTASTE,
    40000, 55000, 50000, 500000000, 15000
  },

  { "taste of blood", EVOLVE_2, FAE_EVOLVE_ACIDBLOOD,
    EVOLVE_3, FAE_EVOLVE_BLOODTASTE, EVOLVE_3, FAE_EVOLVE_ACIDHEART,
    50000, 55000, 40000, 500000000, 15000
  },

  /* end of table */
  { "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

void show_fae_evolves(CHAR_DATA *ch, int base, EVOLVE_DATA *evolve, bool recursive)
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

  for (i = 0; fae_evolve_table[i].name[0] != '\0'; i++)
  {
    int *reqfield = &ch->pcdata->powers[fae_evolve_table[i].req_field];
    int *evolvefield = &ch->pcdata->powers[fae_evolve_table[i].evolve_field];
    int *opposefield = &ch->pcdata->powers[fae_evolve_table[i].oppose_field];

    /* got the opposing evolve ? */
    if (fae_evolve_table[i].oppose_bit != 0 && IS_SET(*opposefield, fae_evolve_table[i].oppose_bit))
      continue;

    if (base != -1)
    {
      if (fae_evolve_table[i].req_bit != fae_evolve_table[base].evolve_bit) continue;
      if (fae_evolve_table[i].req_field != fae_evolve_table[base].evolve_field) continue;
    }
    else
    {
      if (fae_evolve_table[i].req_bit != 0 && !IS_SET(*reqfield, fae_evolve_table[i].req_bit))
        continue;
    }

    /* add this evolve, then do the recursion dance */
    if (!IS_SET(*evolvefield, fae_evolve_table[i].evolve_bit))
    {
      if (recursive)
        strcat(evolve->error, "  #c");
      else
        strcat(evolve->error, "#C");

      /* add this evolve, then do the recursion dance */
      sprintf(buf, " <SEND href=\"help '%s'\">%-20.20s</SEND> %s %5d %6d %6d %10d %6d#n<BR>",
        fae_evolve_table[i].name,
        fae_evolve_table[i].name,
        (recursive) ? "" : "  ",
        fae_evolve_table[i].hps,
        fae_evolve_table[i].mana,
        fae_evolve_table[i].move,
        fae_evolve_table[i].exp,
        fae_evolve_table[i].gold);
      strcat(evolve->error, buf);

      found = TRUE;
    
      if (!recursive)
        show_fae_evolves(ch, i, evolve, TRUE);
    }
  }

  if (base == -1 && !found)
  {
    sprintf(evolve->error, "You are unable to evolve.<BR>");
  }
}

void fae_evolve(CHAR_DATA *ch, char *argument, EVOLVE_DATA *evolve)
{
  int i;

  if (argument[0] == '\0')
  {
    show_fae_evolves(ch, -1, evolve, FALSE);
    return;
  }

  for (i = 0; fae_evolve_table[i].name[0] != '\0'; i++)
  {
    int *reqfield = &ch->pcdata->powers[fae_evolve_table[i].req_field];
    int *evolvefield = &ch->pcdata->powers[fae_evolve_table[i].evolve_field];
    int *opposefield = &ch->pcdata->powers[fae_evolve_table[i].oppose_field];

    if (IS_SET(*evolvefield, fae_evolve_table[i].evolve_bit)) continue;
    if (fae_evolve_table[i].oppose_bit != 0 && IS_SET(*opposefield, fae_evolve_table[i].oppose_bit)) continue;
    if (fae_evolve_table[i].req_bit != 0 && !IS_SET(*reqfield, fae_evolve_table[i].req_bit)) continue;

    if (!str_cmp(argument, fae_evolve_table[i].name))
      break;
  }

  if (fae_evolve_table[i].name[0] == '\0')
  {
    sprintf(evolve->error, "There is no evolve by that name.<BR>");
    return;
  }

  /* set the evolve data */
  evolve->hps   =  fae_evolve_table[i].hps;
  evolve->mana  =  fae_evolve_table[i].mana;
  evolve->move  =  fae_evolve_table[i].move;
  evolve->exp   =  fae_evolve_table[i].exp;
  evolve->gold  =  fae_evolve_table[i].gold;
  evolve->field =  &ch->pcdata->powers[fae_evolve_table[i].evolve_field];
  evolve->bit   =  fae_evolve_table[i].evolve_bit;
  evolve->valid =  TRUE;
}

void fae_commands(CHAR_DATA *ch)
{
  BUFFER *buf = buffer_new(MAX_STRING_LENGTH);
  char evolve[MAX_STRING_LENGTH]; int evocount = 0;
  char generic[MAX_STRING_LENGTH]; int gencount = 0;
  char arcane[MAX_STRING_LENGTH]; int arccount = 0;
  char nature[MAX_STRING_LENGTH]; int natcount = 0;
  int cmd;

  bprintf(buf, "%s\n\r", get_dystopia_banner("    Powers    ", 76));

  for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
  {
    if (cmd_table[cmd].race != CLASS_FAE)
      continue;

    /* check to see if the player has actually learned the power */
    if (!can_use_command(ch, cmd))
      continue;

    switch(cmd_table[cmd].powertype)
    {
      default:
        bug("fae_commands: cmd %d unknown powertype.", cmd);
        break;
      case 0:
      case FAE_WILL:
      case FAE_ENERGY:
      case FAE_MATTER:
      case FAE_PLASMA:
        if (gencount == 0)
          sprintf(generic, " %-15s :", "Fae Powers");
        strcat(generic, " ");
        strcat(generic, cmd_table[cmd].name);
        gencount++;
        break;
      case DISC_FAE_NATURE:
        if (natcount == 0)
          sprintf(nature, " %-15s :", "Nature");
        strcat(nature, " ");
        strcat(nature, cmd_table[cmd].name);
        natcount++;
        break;
      case DISC_FAE_ARCANE:
        if (arccount == 0)
          sprintf(arcane, " %-15s :", "Arcane");
        strcat(arcane, " ");
        strcat(arcane, cmd_table[cmd].name);
        arccount++;
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
  if (arccount > 0)
    bprintf(buf, "%19.19s%s\n\r", arcane, line_indent(&arcane[18], 19, 75));
  if (natcount > 0)
    bprintf(buf, "%19.19s%s\n\r", nature, line_indent(&nature[18], 19, 75));
  if (evocount > 0)
    bprintf(buf, "%19.19s%s\n\r", evolve, line_indent(&evolve[18], 19, 75));

  bprintf(buf, "%s\n\r", get_dystopia_banner("", 76));

  send_to_char(buf->data, ch);
  buffer_free(buf);
}

void do_hspirits(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim = NULL;
  char arg[MAX_INPUT_LENGTH];
  int cost = 1500;
  int delay;

  if (IS_NPC(ch) || ch->in_room == NULL) return;
  if (!IS_CLASS(ch, CLASS_FAE) || !IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_HAUNTING))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_HAUNTING)) != NULL)
  {
    printf_to_char(ch, "You cannot use this power for another %s.\n\r", event_time_left(event));
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0' && (victim = ch->fighting) != NULL)
  {
    send_to_char("Whom do you wish to prevent fleeing?\n\r", ch);
    return;
  }

  if (victim == NULL && (victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(victim, EVENT_MOBILE_FLEE)) == NULL)
  {
    send_to_char("They are not trying to flee.\n\r", ch);
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

  delay = event_pulses_left(event) + number_range(2, 4) * PULSE_PER_SECOND;
  dequeue_event(event, TRUE);

  event = alloc_event();
  event->fun = &event_mobile_flee;
  event->type = EVENT_MOBILE_FLEE;
  add_event_char(event, victim, delay);

  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_PLAYER_FAE_HAUNTING;
  add_event_char(event, ch, 6 * PULSE_PER_SECOND);

  act("$n spews forth a foggy substance, which starts clinging to $N.", ch, NULL, victim, TO_NOTVICT);
  act("You spew forth the haunting spirits, preventing $N from fleeing.", ch, NULL, victim, TO_CHAR);
  act("$n spews forth a foggy substance, which starts attaching itself to you.", ch, NULL, victim, TO_VICT);
}

void do_energize(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  char arg[MAX_STRING_LENGTH];

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Energize what fae token?\n\r", ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You are not carrying that token.\n\r", ch);
    return;
  }

  if (obj->item_type != ITEM_FAETOKEN)
  {
    send_to_char("That is not a faetoken.\n\r", ch);
    return;
  }

  act("You energize $p.", ch, obj, NULL, TO_CHAR);
  act("$n energizes $p.", ch, obj, NULL, TO_ROOM);

  obj_cast_spell(obj->value[0], 50, ch, ch, NULL);
  extract_obj(obj);
}

void do_warding(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  int cost = 1500;

  if (IS_NPC(ch) || ch->in_room == NULL) return;
  if (!IS_CLASS(ch, CLASS_FAE) || !IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_WARDING))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
  {
    send_to_char("You cannot write a glyph in this room.\n\r", ch);
    return;
  }

  if (event_isset_room(ch->in_room, EVENT_ROOM_WARDING))
  {
    send_to_char("A warding is already pulsing on the floor.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to create a glyph of warding.\n\r", (cost - ch->mana));
    return;
  }
  modify_mana(ch, -1 * cost);

  act("$n draws a glyph of warding on the floor.", ch, NULL, NULL, TO_ROOM);
  send_to_char("You draw a glyph of warding on the floor.\n\r", ch);

  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_ROOM_WARDING;
  add_event_room(event, ch->in_room, 30 * PULSE_PER_SECOND);
}

bool event_room_blastward(EVENT_DATA *event)
{
  CHAR_DATA *gch;
  ROOM_INDEX_DATA *pRoom;
  ITERATOR *pIter;

  if ((pRoom = event->owner.room) == NULL)
  {
    bug("event_room_blastward: no owner.", 0);
    return FALSE;
  }

  pIter = AllocIterator(pRoom->people);
  if ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    int pulses = event_pulses_left(event);
    int dam;

    act("A warning glyph flares!!", gch, NULL, NULL, TO_ALL);

    do {
      if ((dam = UMIN(gch->hit - 1, pulses * dice(6, 4))) <= 0) continue;

      if (!IS_NPC(gch))
        printf_to_char(gch, "The flames burn your body! [%d]\n\r", dam);
      modify_hps(gch, -1 * dam);
    } while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL);
  }

  return FALSE;
}

void do_chameleon(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE) || !IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_CHAMELEON))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  TOGGLE_BIT(ch->newbits, NEW_CHAMELEON);
  if (IS_SET(ch->newbits, NEW_CHAMELEON))
    send_to_char("Your fade into the background, becoming one with your environment.\n\r", ch);
  else
  {
    act("$n makes $s presence known.", ch, NULL, NULL, TO_ROOM);
    send_to_char("Your skin regains its normal hue.\n\r", ch);
  }
}

void update_fae(CHAR_DATA *ch)
{
  if (ch->pcdata->powers[DISC_FAE_ARCANE] > 8 && !IS_SET(ch->pcdata->powers[EVOLVE_1], FAE_EVOLVE_ARCANE))
    ch->pcdata->powers[DISC_FAE_ARCANE] = 8;
  if (ch->pcdata->powers[DISC_FAE_NATURE] > 8 && !IS_SET(ch->pcdata->powers[EVOLVE_1], FAE_EVOLVE_NATURE))
    ch->pcdata->powers[DISC_FAE_NATURE] = 8;

  if (IS_SET(ch->newbits, NEW_FAEHALO))
  {
    int cost = 500;

    if (ch->move < cost || ch->mana < cost)
    {
      REMOVE_BIT(ch->newbits, NEW_FAEHALO);

      act("Your halo flickers and fades away.", ch, NULL, NULL, TO_CHAR);
      act("The fearsome halo surrounding $n starts to slowly fade away.", ch, NULL, NULL, TO_ROOM);
    }
    else
    {
      modify_move(ch, -1 * cost);
      modify_mana(ch, -1 * cost);
    }
  }

  if (IS_SET(ch->newbits, NEW_PSPRAY))
  {
    int cost = 400;

    if (ch->mana < cost)
    {
      REMOVE_BIT(ch->newbits, NEW_PSPRAY);

      act("$n dispels $s prismatic spray.", ch, NULL, NULL, TO_ROOM);
      send_to_char("You where unable keep your prismatic spray active.\n\r", ch);
    }
    else
    {
      modify_mana(ch, -1 * cost);
    }
  }

  if (IS_SET(ch->pcdata->powers[FAE_BITS], FAE_ACIDHEART))
  {
    int cost = 250;

    if (ch->move < cost)
    {
      REMOVE_BIT(ch->pcdata->powers[FAE_BITS], FAE_ACIDHEART);
      send_to_char("Your heart stops pumping acid through your veins.\n\r", ch);
    }
    else
    {
      modify_move(ch, -1 * cost);
    }
  }

  if (IS_SET(ch->act, PLR_HIDE))
  {
    int cost = 250;

    ch->pcdata->powers[FAE_VANISH_STRESS]++;

    cost *= 1 + ch->pcdata->powers[FAE_VANISH_STRESS] / 10;

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
    if (ch->pcdata->powers[FAE_VANISH_STRESS] > 0)
      ch->pcdata->powers[FAE_VANISH_STRESS]--;
  }

  /* regenerate body and limbs */
  if (ch->hit != ch->max_hit)
    regen_hps(ch, 2);
  if (ch->mana != ch->max_mana && !IS_SET(ch->act, PLR_HIDE))
    regen_mana(ch, 2);
  if (ch->move != ch->max_move)
    regen_move(ch, 2);

  regen_limb(ch);
}

void do_dragonbreath(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *gch;
  ITERATOR *pIter;
  int cost = 750;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE) || !IS_SET(ch->pcdata->powers[EVOLVE_1], FAE_EVOLVE_DRAGON))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, NULL, cost, eMana);

  if (ch->mana < cost)
  {
    send_to_char("You do not have enough mana to use this power.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("$n breaths a cone of fire, engulfing the room in flames.", ch, NULL, NULL, TO_ROOM);
  send_to_char("You breath a fiery blast of flames, engulfing the room.\n\r", ch);

  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    int dam = number_range(UMAX(10, ch->hit) / 14, UMAX(10, ch->hit) / 7);

    if (ch == gch) continue; 
    if (!can_see(ch, gch)) continue;
    if (is_safe(ch, gch)) continue;
    if (!IS_NPC(gch) && IS_SET(gch->immune, IMM_HEAT)) continue;

    if (saves_spell(URANGE(10, ch->hit / 1000, 50), gch))
      dam /= 2;

    if (dam > ch->damcap[DAM_CAP])
      dam = number_range((ch->damcap[DAM_CAP] - 200), (ch->damcap[DAM_CAP] + 100)); 
    if (IS_AFFECTED(gch, AFF_SANCTUARY))
      dam /=2;

    damage(ch, gch, NULL, dam, gsn_firebreath);
  }

  WAIT_STATE(ch, 12);
}

void elemboost(CHAR_DATA *ch, int level)
{
  int boost = 30;

  boost += getRank(ch, 0) * 30;
  boost += UMIN(level, 8) * 10;

  /* nature evolve */
  if (level > 8)
    boost += (level - 8) * 35;

  ch->pcdata->mod_str += getRank(ch, 0) / 2;
  ch->pcdata->mod_dex += getRank(ch, 0) / 2;
  ch->pcdata->mod_con += getRank(ch, 0) / 2;
  ch->pcdata->mod_wis += getRank(ch, 0) / 2;
  ch->pcdata->mod_int += getRank(ch, 0) / 2;
  ch->hitroll += boost;
  ch->damroll += boost;
  ch->armor   -= 3 * boost / 2;
}

void do_reform(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_ARCANE] < 2)
  {
    send_to_char("You need level 2 discipline in arcane.\n\r", ch);
    return;
  }
  ch->loc_hp[0] = 0;
  ch->loc_hp[1] = 0;
  ch->loc_hp[2] = 0;
  ch->loc_hp[3] = 0;
  ch->loc_hp[4] = 0;
  ch->loc_hp[5] = 0;
  ch->loc_hp[6] = 0;

  /* strip all fae energy charge events */
  strip_event_mobile(ch, EVENT_PLAYER_FAE_PLASMA);
  strip_event_mobile(ch, EVENT_PLAYER_FAE_ENERGY);
  strip_event_mobile(ch, EVENT_PLAYER_FAE_MATTER);
  strip_event_mobile(ch, EVENT_PLAYER_FAE_WILL);

  /* drop the fae shield */
  ch->pcdata->powers[FAE_SHIELD] = 0;

  send_to_char("#pYou reform your body, regrowing lost limbs.#n\n\r",ch);
}

void do_bloodimmune(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  int cost = UMAX(15000, ch->max_hit / 3);

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE)
   || !IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_BLOODIMMUNE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_BLOODIMMUNE)) != NULL)
  {
    printf_to_char(ch, "You are immune for another %s.\n\r", event_time_left(event));
    return;
  }

  if (ch->hit <= cost)
  {
    printf_to_char(ch, "You need %d more hitpoints to use this power.\n\r", cost - ch->hit + 1);
    return;
  }
  modify_hps(ch, -1 * cost);

  act("$n shreds his mortallity (and a rather huge portion of lifeforce).", ch, NULL, NULL, TO_ROOM);
  act("You shred your mortallity, sacrificing life for power.", ch, NULL, NULL, TO_CHAR);

  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_PLAYER_BLOODIMMUNE;
  add_event_char(event, ch, 12 * PULSE_PER_SECOND);
}

bool event_mobile_acidblood(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  CHAR_DATA *fae = NULL;
  ITERATOR *pIter;
  int dam = number_range(200, 400);

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_acidblood: no owner.", 0);
    return FALSE;
  }

  dam = UMIN(dam, ch->hit - 1);

  if (dam <= 0 || ch->level < 3 || ch->hit < ch->max_hit / 4)
  {
    send_to_char("Your bloodstream is no longer floated with acid.\n\r", ch);
    return FALSE;
  }

  if (event->argument != NULL && event->argument[0] != '\0')
  {
    pIter = AllocIterator(ch->in_room->people);
    while ((fae = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (!str_cmp(fae->name, event->argument))
        break;
    }
  }

  modify_hps(ch, -1 * dam);

  act("$n screams in agony as $s bloodlines are torn appart by acid.", ch, NULL, NULL, TO_ROOM);
  act("You scream in agony as your bloodlines are torn appart by acid.", ch, NULL, NULL, TO_CHAR);

  if (fae != NULL && fae->hit < fae->max_hit)
  {
    modify_hps(ch, dam / 3);
    act("You drain a portion of $N's acidic blood and use it to heal yourself.", fae, NULL, ch, TO_CHAR);
  }

  if (!IS_NPC(ch) || number_percent() > 10)
  {
    event = alloc_event();
    event->fun = &event_mobile_acidblood;
    event->type = EVENT_MOBILE_ACIDBLOOD;
    if (fae != NULL)
      event->argument = str_dup(fae->name);
    add_event_char(event, ch, 3 * PULSE_PER_SECOND);
  }

  return FALSE;
}

void do_bloodacid(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int cost = 1500;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE)
   || !IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_ACIDBLOOD))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Turn whose blood to acid?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (event_isset_mobile(victim, EVENT_MOBILE_ACIDBLOOD))
  {
    send_to_char("Their blood has already been turned to acid.\n\r", ch);
    return;
  }

  if (ch == victim)
  {
    send_to_char("You really don't want to do this to yourself.\n\r", ch);
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

  act("$N screams in agony as $n turns $S blood to acid.", ch, NULL, victim, TO_NOTVICT);
  act("$N screams in agony as you turn $S blood to acid.", ch, NULL, victim, TO_CHAR);
  act("You scream in agony as $n turns your blood to acid.", ch, NULL, victim, TO_VICT);

  event = alloc_event();
  event->fun = &event_mobile_acidblood;
  event->type = EVENT_MOBILE_ACIDBLOOD;
  if (IS_SET(ch->pcdata->powers[EVOLVE_3], FAE_EVOLVE_ACIDHEART))
    event->argument = str_dup(ch->name);
  add_event_char(event, victim, 3 * PULSE_PER_SECOND);

  aggress(ch, victim);

  WAIT_STATE(ch, 6);
}

void do_pspray(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  EVENT_DATA *event;
  char arg[MAX_INPUT_LENGTH];
  int cost = 1000;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE)
   || !IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_PSPRAY))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    if (IS_SET(ch->newbits, NEW_PSPRAY))
    {
      REMOVE_BIT(ch->newbits, NEW_PSPRAY);
      act("$n dispels $s prismatic spray.", ch, NULL, NULL, TO_ROOM);
      send_to_char("You dispel your prismatic spray spell.\n\r", ch);
    }
    else
    {
      if (ch->mana < cost)
      {
        printf_to_char(ch, "You need %d more mana to conjure a prismatic spray.\n\r", cost - ch->mana);
        return;
      }
      modify_mana(ch, -1 * cost);

      SET_BIT(ch->newbits, NEW_PSPRAY);
      act("$n conjures forth a prismatic spray.", ch, NULL, NULL, TO_ROOM);
      send_to_char("You conjure forth a prismatic spray.\n\r", ch);
    }
    return;
  }

  if (!IS_SET(ch->newbits, NEW_PSPRAY) && !event_isset_room(ch->in_room, EVENT_ROOM_PWALL))
  {
    send_to_char("First you must conjure forth a prismatic spray.\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  act("$n sprays a burst of scintillating colours into $N's face.\n\r", ch, NULL, victim, TO_NOTVICT);
  act("$n sprays a burst of scintillating colours into your face.\n\r", ch, NULL, victim, TO_VICT);
  act("You spray a burst of scintillating colours into $N's face.\n\r", ch, NULL, victim, TO_CHAR);

  if (!IS_NPC(victim))
  {
    send_to_char("It doesn't seem to have any affect on you.\n\r", victim);
    act("$n shakes $s head to get the colours away.", victim, NULL, NULL, TO_ROOM);
    return;
  }

  if (!event_isset_mobile(victim, EVENT_MOBILE_PSPRAY))
  {
    switch(number_range(1, 3))
    {
      default:
        do_say(victim, "ooOOOooo colours, yeah man!");
        break;
      case 2:
        do_say(victim, "I'm dizzy, would someone fetch me a chair?");
        break;
      case 3:
        do_say(victim, "Huh? What's that?");
        break;
    }

    event = alloc_event();
    event->fun = &event_dummy;
    event->type = EVENT_MOBILE_PSPRAY;
    add_event_char(event, victim, number_range(4, 8) * PULSE_PER_SECOND);
  }
}

bool event_mobile_chainblast(EVENT_DATA *event)
{
  CHAR_DATA *ch, *victim;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];
  int dam;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_chainblast: no owner.", 0);
    return FALSE;
  }

  if (ch->in_room == NULL)
    return FALSE;

  act("The prismatic bolt leaps from $n.", ch, NULL, NULL, TO_ROOM);
  act("The prismatic bolt leaps from you.", ch, NULL, NULL, TO_CHAR);

  if (number_percent() >= 95)
  {
    act("The prismatic blast fades away.", ch, NULL, NULL, TO_ALL);
    return FALSE;
  }

  pIter = AllocIterator(ch->in_room->people);
  while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (ch == victim || IS_CLASS(victim, CLASS_FAE)) continue;

    dam = (IS_NPC(victim)) ? number_range(1500, 5000) : number_range(500, 1500);
    if (dam >= victim->hit)
      dam = victim->hit - 1;

    if (dam <= 0)
    {
      act("The prismatic blast fades as it strikes $n.", victim, NULL, NULL, TO_ROOM);
      return FALSE;
    }
    modify_hps(victim, -1 * dam);

    sprintf(buf, "The #Gp#Rr#ri#ys#om#Ca#Pt#0i#9c#n bolt strikes you [%d]", dam);
    act(buf, victim, NULL, NULL, TO_CHAR);
    sprintf(buf, "The #Gp#Rr#ri#ys#om#Ca#Pt#0i#9c#n bolt strikes $n [%d]", dam);
    act(buf, victim, NULL, NULL, TO_ROOM);

    event = alloc_event();
    event->fun = &event_mobile_chainblast;
    event->type = EVENT_MOBILE_CHAINBLAST;
    add_event_char(event, victim, 3 * PULSE_PER_SECOND);

    return FALSE;
  }

  if ((event = event_isset_room(ch->in_room, EVENT_ROOM_PWALL)) != NULL)
  {
    EVENT_DATA *newevent;
    int delay = event_pulses_left(event) + 16;

    act("The prismatic blast fuses with the prismatic wall.", ch, NULL, NULL, TO_ALL);

    newevent = alloc_event();
    newevent->fun = &event_dummy;
    newevent->type = EVENT_ROOM_PWALL;
    add_event_room(newevent, ch->in_room, delay);

    dequeue_event(event, TRUE);
  }
  else
  {
    act("The prismatic blast fades away.", ch, NULL, NULL, TO_ALL);
  }

  return FALSE;
}

void do_pchain(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int cost = 2000;
  int dam;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE)
   || !IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_PBLAST))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAECHAIN)) != NULL)
  {
    printf_to_char(ch, "You cannot use this power for another %s.\n\r", event_time_left(event));
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Cause a prismatic chainblast to strike whom?\n\r", ch);
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

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMove);

  if (ch->move < cost)
  {
    printf_to_char(ch, "You need another %d move to blast someone with this chain.\n\r", cost - ch->move);
    return;
  }
  modify_move(ch, -1 * cost);

  dam = number_range(500, 1500);
  if (dam >= victim->hit)
    dam = victim->hit - 1;

  if (dam <= 0)
  {
    act("The prismatic blast fades as it strikes $N.", ch, NULL, victim, TO_CHAR);
    return;
  }

  sprintf(arg, "$n blasts $N with a #Gp#Rr#ri#ys#om#Ca#Pt#0i#9c#n bolt [%d]", dam);
  act(arg, ch, NULL, victim, TO_NOTVICT);
  sprintf(arg, "$n blasts you with a #Gp#Rr#ri#ys#om#Ca#Pt#0i#9c#n bolt [%d]", dam);
  act(arg, ch, NULL, victim, TO_VICT);
  sprintf(arg, "You blast $N with a #Gp#Rr#ri#ys#om#Ca#Pt#0i#9c#n bolt [%d]", dam);
  act(arg, ch, NULL, victim, TO_CHAR);

  hurt_person(ch, victim, dam);

  event = alloc_event();
  event->fun = &event_mobile_chainblast;
  event->type = EVENT_MOBILE_CHAINBLAST;
  add_event_char(event, victim, 3 * PULSE_PER_SECOND);

  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_PLAYER_FAECHAIN;
  add_event_char(event, ch, 30 * PULSE_PER_SECOND);

  aggress(ch, victim);
}

void do_pwall(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  char buf[MAX_STRING_LENGTH];
  int amount = 0;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE)
   || !IS_SET(ch->pcdata->powers[EVOLVE_1], FAE_EVOLVE_WALL))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (ch->in_room == NULL || event_isset_room(ch->in_room, EVENT_ROOM_PWALL))
  {
    send_to_char("This room is already covered in a sheet of ancient energy.\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_MATTER)) != NULL)
  {
    one_argument(event->argument, buf);
    amount += atoi(buf);
    dequeue_event(event, TRUE);
  }
  if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_PLASMA)) != NULL)
  {
    one_argument(event->argument, buf);
    amount += atoi(buf); 
    dequeue_event(event, TRUE);
  }
  if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_WILL)) != NULL)
  {
    one_argument(event->argument, buf);
    amount += atoi(buf); 
    dequeue_event(event, TRUE);
  }
  if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_ENERGY)) != NULL)
  {
    one_argument(event->argument, buf);
    amount += atoi(buf); 
    dequeue_event(event, TRUE);
  }

  if (amount <= 0)
  {
    send_to_char("You don't have enough ancient energy channeled.\n\r", ch);
    return;
  }

  if (ch->mana < 1500)
  {
    printf_to_char(ch, "You need %d more mana.\n\r", 1500 - ch->mana);
    return;
  }
  modify_mana(ch, -1500);

  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_ROOM_PWALL;
  add_event_room(event, ch->in_room, amount * PULSE_PER_SECOND);

  act("$n summons a sheet of prismatic energy to cover the room.", ch, NULL, NULL, TO_ROOM);
  act("You transform your stored ancients into a sheet of energy.", ch, NULL, NULL, TO_CHAR);
}

void do_halo(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE)
   || !IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_HALO))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  TOGGLE_BIT(ch->newbits, NEW_FAEHALO);

  if (IS_SET(ch->newbits, NEW_FAEHALO))
  {
    act("A halo of fearsome proportions burst from your body.", ch, NULL, NULL, TO_CHAR);
    act("A halo of fearsome proportions bursts from $n's body.", ch, NULL, NULL, TO_ROOM);
  }
  else
  {
    act("Your halo flickers and fades away.", ch, NULL, NULL, TO_CHAR);
    act("The fearsome halo surrounding $n starts to slowly fade away.", ch, NULL, NULL, TO_ROOM);
  }
}

void do_freezeancients(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *plasma, *matter, *energy, *will, *event;
  int cost = 0, delay;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE)
   || !IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_FREEZE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((plasma = event_isset_mobile(ch, EVENT_PLAYER_FAE_PLASMA)) != NULL)
  {
    if (plasma->argument == NULL)
    {
      send_to_char("You have encountered a bug, please report this.\n\r", ch);
      return;
    }
    cost += atoi(plasma->argument);
  }
  if ((matter = event_isset_mobile(ch, EVENT_PLAYER_FAE_MATTER)) != NULL)
  { 
    if (matter->argument == NULL)
    {
      send_to_char("You have encountered a bug, please report this.\n\r", ch);
      return;
    }
    cost += atoi(matter->argument);
  }
  if ((energy = event_isset_mobile(ch, EVENT_PLAYER_FAE_ENERGY)) != NULL)
  {
    if (energy->argument == NULL)
    {
      send_to_char("You have encountered a bug, please report this.\n\r", ch);
      return;
    }
    cost += atoi(energy->argument);
  }
  if ((will = event_isset_mobile(ch, EVENT_PLAYER_FAE_WILL)) != NULL)
  {
    if (will->argument == NULL)
    {
      send_to_char("You have encountered a bug, please report this.\n\r", ch);
      return;
    }
    cost += atoi(will->argument);
  }

  if (plasma == NULL && matter == NULL && energy == NULL && will == NULL)
  {
    send_to_char("You are not channeling anything.\n\r", ch);
    return;
  }

  if (ch->mana < cost * 200)
  {
    printf_to_char(ch, "You need %d more mana to freeze your ancients.\n\r", cost * 200 - ch->mana);
    return;
  }
  modify_mana(ch, -200 * cost);

  if (plasma != NULL)
  {
    delay = event_pulses_left(plasma) + 3 * PULSE_PER_SECOND;

    event           = alloc_event();
    event->argument = str_dup(plasma->argument);
    event->fun      = &event_player_fae_plasma;
    event->type     = EVENT_PLAYER_FAE_PLASMA;
    add_event_char(event, ch, delay);

    send_to_char("You delay your next channel of plasma.\n\r", ch);
    dequeue_event(plasma, TRUE);
  }

  if (will != NULL)
  {
    delay = event_pulses_left(will) + 3 * PULSE_PER_SECOND;

    event           = alloc_event();
    event->argument = str_dup(will->argument);
    event->fun      = &event_player_fae_will;
    event->type     = EVENT_PLAYER_FAE_WILL;
    add_event_char(event, ch, delay);

    send_to_char("You delay your next channel of will.\n\r", ch);
    dequeue_event(will, TRUE);
  }

  if (energy != NULL)
  {
    delay = event_pulses_left(energy) + 3 * PULSE_PER_SECOND;

    event           = alloc_event();
    event->argument = str_dup(energy->argument);
    event->fun      = &event_player_fae_energy;
    event->type     = EVENT_PLAYER_FAE_ENERGY;
    add_event_char(event, ch, delay);

    send_to_char("You delay your next channel of energy.\n\r", ch);
    dequeue_event(energy, TRUE);
  }

  if (matter != NULL)
  {
    delay = event_pulses_left(matter) + 3 * PULSE_PER_SECOND;

    event           = alloc_event();
    event->argument = str_dup(matter->argument);
    event->fun      = &event_player_fae_matter;
    event->type     = EVENT_PLAYER_FAE_MATTER;
    add_event_char(event, ch, delay);

    send_to_char("You delay your next channel of matter.\n\r", ch);
    dequeue_event(matter, TRUE);
  }
}

void do_bloodsacrifice(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE)
   || !IS_SET(ch->pcdata->powers[EVOLVE_1], FAE_EVOLVE_SACRIFICE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_SACRIFICE_WAIT)) != NULL)
  {
    printf_to_char(ch, "You cannot use this power for another %s.\n\r", event_time_left(event));
    return;
  }

  if (ch->hit <= 2000)
  {
    send_to_char("You do not have enough health to use this power.\n\r", ch);
    return;
  }
  modify_hps(ch, -2000);

  /* player cannot use this power for another 10 seconds */
  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_PLAYER_SACRIFICE_WAIT;
  add_event_char(event, ch, 10 * PULSE_PER_SECOND);

  /* schedule the healing events (6 rounds) */
  event = alloc_event();
  event->fun = &event_player_sacrifice_fae;
  event->type = EVENT_PLAYER_SACRIFICE_FAE;
  event->argument = str_dup("6");
  add_event_char(event, ch, PULSE_PER_SECOND);

  act("$n spits forth a shower of blood.", ch, NULL, NULL, TO_ROOM);
  act("You spit forth a shower of blood.", ch, NULL, NULL, TO_CHAR);
}

bool event_player_sacrifice_fae(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  int counts = (event->argument) ? atoi(event->argument) : 0;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_sacrifice_fae: no owner.", 0);
    return FALSE;
  }

  if (ch->hit <= 0)
    return FALSE;

  /* 500 point heal */
  modify_hps(ch, 500);

  if (counts-- > 0)
  {
    char buf[MAX_INPUT_LENGTH];

    sprintf(buf, "%d", counts);

    event = alloc_event();
    event->fun = &event_player_sacrifice_fae;
    event->type = EVENT_PLAYER_SACRIFICE_FAE;
    event->argument = str_dup(buf);
    add_event_char(event, ch, PULSE_PER_SECOND);
  }

  return FALSE;
}

void do_ghostgauntlets(CHAR_DATA *ch, char *argument)
{
  int cost = 1500;
  int sn;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_ARCANE] < 10)
  {
    send_to_char("You need level 10 discipline in arcane.\n\r", ch);
    return;
  }

  if ((sn = skill_lookup("ghost gauntlets")) <= 0)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    send_to_char("You do not have enough mana to summon the ghost gauntlets.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * cost);

  do_say(ch, "Donu virtious enchantum!");

  (*skill_table[sn].spell_fun) (sn, 50, ch, ch);
}

void do_watchfuleye(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *pMob;
  MOB_INDEX_DATA *pMobIndex;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_ARCANE] < 9)
  {
    send_to_char("You need level 9 discipline in arcane.\n\r", ch);
    return;
  }

  if (ch->pcdata->familiar)
  {
    send_to_char("You already have a familiar.\n\r", ch);
    return;
  }

  if ((pMobIndex = get_mob_index(MOB_VNUM_WATCHEYE)) == NULL)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }

  pMob = create_mobile(pMobIndex);
  char_to_room(pMob, ch->in_room, TRUE);
  ch->pcdata->familiar = pMob;
  pMob->wizard = ch;

  send_to_char("You create a watchful eye.\n\r", ch);
  act("$n utters a single powerword, and a huge eyeball appears in the room.", ch, NULL, NULL, TO_ROOM);
}

void do_nibbleeye(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *pMob;
  EVENT_DATA *event;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];
  int sn;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_ARCANE] < 9) 
  {
    send_to_char("You need level 9 discipline in arcane.\n\r", ch);
    return;
  }

  if ((pMob = ch->pcdata->familiar) == NULL || pMob->pIndexData->vnum != MOB_VNUM_WATCHEYE)
  {
    send_to_char("You do not have a watchful eye.\n\r", ch);
    return;
  }

  pIter = AllocIterator(pMob->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == EVENT_MOBILE_CASTING)
    {
      send_to_char("Your watchful eye is already casting a spell.\n\r", ch);
      return;
    }
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Cast which what where?\n\r", ch);
    return;
  }

  if ((sn = skill_lookup(arg)) < 0 || ch->level < skill_table[sn].skill_level)
  {
    send_to_char("You do not know how to cast that spell.\n\r", ch);
    return;
  }

  sprintf(arg, "You start casting '%s'.", skill_table[sn].name);
  act(arg, pMob, NULL, NULL, TO_CHAR);

  sprintf(arg, "$n starts casting '%s'.", skill_table[sn].name);
  act(arg, pMob, NULL, NULL, TO_ROOM);

  event            =  alloc_event();
  event->argument  =  str_dup(argument);
  event->fun       =  &cast_spell;
  event->type      =  EVENT_MOBILE_CASTING;
  add_event_char(event, pMob, skill_table[sn].beats + 1);
}

void do_elementalform( CHAR_DATA *ch, char *argument )
{
  char buf [MAX_STRING_LENGTH];
  const int mana_cost = 2000, move_cost = 2000;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_NATURE] < 1)
  {
    send_to_char("You need level 1 discipline in nature.\n\r",ch);
    return;
  }
  if (IS_SET(ch->newbits, NEW_CUBEFORM))
  {
    send_to_char("You are already an elemental force.\n\r"
                 "Use the 'clearstats' command to revert to your human form.\n\r", ch);
    return;
  }
  if (ch->move < move_cost || ch->mana < mana_cost)
  {
    send_to_char("You need 2000 mana and 2000 move to change into elemental form.\n\r",ch);
    return;
  }

  act( "$n transforms into an elemental storm.", ch, NULL, NULL, TO_ROOM);
  send_to_char("You become one with the elements, letting your rage guide you.\n\r",ch);
  SET_BIT(ch->affected_by, AFF_PASS_DOOR);
  SET_BIT(ch->newbits, NEW_CUBEFORM);
  SET_BIT(ch->affected_by, AFF_POLYMORPH);
  sprintf(buf,"#oAn #CElemental #oStorm #0(#g%s#0)#n", ch->name);
  free_string(ch->morph);
  ch->morph = str_dup(buf);
  modify_move(ch, -1 * move_cost);
  modify_mana(ch, -1 * mana_cost);
  elemboost(ch, ch->pcdata->powers[DISC_FAE_NATURE]);
}

void do_channel(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char event_arg[MAX_STRING_LENGTH];

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    do_channels(ch, argument);
    return;
  }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (arg1[0] == '\0' || arg2[0] == '\0' )
  {
    send_to_char("Start channeling what, and at what speed?\n\r",ch);
    return;
  }
  if (!is_number(arg2) || (atoi(arg2) < 1 || atoi(arg2) > 3))
  {
    if (atoi(arg2) == 4 && IS_SET(ch->newbits, NEW_MASTERY))
    {
      if (get_eq_char(ch, WEAR_MASTERY) == NULL)
      {
        send_to_char("You need to wear your mastery item to channel at the speed of 4.\n\r", ch);
        return;
      }
      else if (!str_prefix(arg1, "matter") && ch->pcdata->powers[FAE_PATH] == FAE_MATTER)
        ;
      else if (!str_prefix(arg1, "plasma") && ch->pcdata->powers[FAE_PATH] == FAE_PLASMA)
        ;
      else if (!str_prefix(arg1, "will") && ch->pcdata->powers[FAE_PATH] == FAE_WILL)
        ;
      else if (!str_prefix(arg1, "energy") && ch->pcdata->powers[FAE_PATH] == FAE_ENERGY)
        ;
      else
      {
        send_to_char("You cannot channel that ancient at the speed of 4.\n\r", ch);
        return;
      }
    }
    else
    {
      if ((!str_prefix(arg1, "matter") && ch->pcdata->powers[FAE_PATH] == FAE_MATTER) ||
          (!str_prefix(arg1, "plasma") && ch->pcdata->powers[FAE_PATH] == FAE_PLASMA) ||
          (!str_prefix(arg1, "will") && ch->pcdata->powers[FAE_PATH] == FAE_WILL) ||
          (!str_prefix(arg1, "energy") && ch->pcdata->powers[FAE_PATH] == FAE_ENERGY))
        send_to_char("The speed should be a number between 1 and 4.\n\r", ch);
      else
        send_to_char("The speed should be a number between 1 and 3.\n\r", ch);
      return;
    }
  }
  if (!str_prefix(arg1,"matter"))
  {
    if (atoi(arg2) > ch->pcdata->powers[FAE_MATTER])
    {
      send_to_char("You can't channel faster than your current level.\n\r",ch);
      return;
    }

    if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_MATTER)) != NULL)
    {
      char old[MAX_INPUT_LENGTH];

      one_argument(event->argument, old);
      sprintf(event_arg, "%d %d", atoi(old), atoi(arg2));

      /* modify speed of old event */
      free_string(event->argument);
      event->argument = str_dup(event_arg);
    }
    else
    {
      sprintf(event_arg, "%d %d", atoi(arg2), atoi(arg2));

      /* make new event, and enqueue it */
      event              =  alloc_event();
      event->argument    =  str_dup(event_arg);
      event->fun         =  &event_player_fae_matter;
      event->type        =  EVENT_PLAYER_FAE_MATTER;
      add_event_char(event, ch, 3 * PULSE_PER_SECOND);
    }

    send_to_char("#pOk, you start channeling matter#n.\n\r",ch);
  }
  else if (!str_prefix(arg1,"will")) 
  {
    if (atoi(arg2) > ch->pcdata->powers[FAE_WILL])
    {
      send_to_char("You can't channel faster than your current level.\n\r",ch);
      return;
    }

    if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_WILL)) != NULL)
    {
      char old[MAX_INPUT_LENGTH];

      one_argument(event->argument, old);
      sprintf(event_arg, "%d %d", atoi(old), atoi(arg2));
      
      /* modify speed of old event */
      free_string(event->argument);
      event->argument = str_dup(event_arg);
    }
    else
    {
      sprintf(event_arg, "%d %d", atoi(arg2), atoi(arg2));

      /* make new event, and enqueue it */
      event              =  alloc_event();
      event->argument    =  str_dup(event_arg);
      event->fun         =  &event_player_fae_will;
      event->type        =  EVENT_PLAYER_FAE_WILL;
      add_event_char(event, ch, 3 * PULSE_PER_SECOND);
    }

    send_to_char("#pOk, you start concentrating your willpower#n.\n\r",ch);
  }
  else if (!str_prefix(arg1,"plasma"))
  {
    if (atoi(arg2) > ch->pcdata->powers[FAE_PLASMA])
    {
      send_to_char("You can't channel faster than your current level.\n\r",ch);
      return;
    }

    if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_PLASMA)) != NULL)
    {
      char old[MAX_INPUT_LENGTH];

      one_argument(event->argument, old);
      sprintf(event_arg, "%d %d", atoi(old), atoi(arg2));

      free_string(event->argument);
      event->argument = str_dup(event_arg);
    }
    else
    {
      sprintf(event_arg, "%d %d", atoi(arg2), atoi(arg2));

      /* make new event, and enqueue it */
      event              =  alloc_event();
      event->argument    =  str_dup(event_arg);
      event->fun         =  &event_player_fae_plasma;
      event->type        =  EVENT_PLAYER_FAE_PLASMA;
      add_event_char(event, ch, 3 * PULSE_PER_SECOND);
    }

    send_to_char("#pOk, you start channeling plasma.#n\n\r",ch);
  }
  else if (!str_prefix(arg1,"energy"))
  {
    if (atoi(arg2) > ch->pcdata->powers[FAE_ENERGY])
    {
      send_to_char("You can't channel faster than your current level.\n\r",ch);
      return;
    }

    if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_ENERGY)) != NULL)
    {
      char old[MAX_INPUT_LENGTH];

      one_argument(event->argument, old);
      sprintf(event_arg, "%d %d", atoi(old), atoi(arg2));

      free_string(event->argument);
      event->argument = str_dup(event_arg);
    }
    else
    {
      sprintf(event_arg, "%d %d", atoi(arg2), atoi(arg2));

      /* make new event, and enqueue it */
      event              =  alloc_event();
      event->argument    =  str_dup(event_arg);
      event->fun         =  &event_player_fae_energy;
      event->type        =  EVENT_PLAYER_FAE_ENERGY;
      add_event_char(event, ch, 3 * PULSE_PER_SECOND);
    }

    send_to_char("#pOk, you start channeling energy.#n\n\r",ch);
  }
  else send_to_char("You cannot channel that.\n\r",ch);
}

void do_infuse(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int dam, i, sn;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (arg1[0] == '\0')
  {
    send_to_char("Infuse what into whom?\n\r",ch);
    return;
  }
  if (!str_prefix(arg1,"matter"))
  {
    if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_MATTER)) == NULL)
    {
      send_to_char("Your not charging matter.\n\r",ch);
      return;
    }
    if (ch->pcdata->powers[FAE_SHIELD] > 0)
    {
      send_to_char("Your shield still exists.\n\r",ch);
      return;
    }
    send_to_char("You weave a shield of charged matter from the fabric of the arcane.\n\r",ch);
    act("$n mutters a series of arcane words and weaves a shimmering shield around $m.",  ch, NULL, NULL, TO_ROOM);

    /* turn on shield */
    one_argument(event->argument, arg1);
    ch->pcdata->powers[FAE_SHIELD] = atoi(arg1) * 7 / 3;

    /* remove event */
    dequeue_event(event, TRUE);

    WAIT_STATE(ch,12);
    return;
  }
  if (arg2[0] == '\0' && ch->fighting != NULL) victim = ch->fighting;
  else if ((victim = get_char_room(ch, arg2)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch );
    return;
  }
  if (is_safe(ch,victim)) return;
  if (!str_prefix(arg1, "energy"))
  {
    if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_ENERGY)) == NULL)
    {
      send_to_char("Your not charging energy.\n\r",ch);
      return;
    }
    act("#pYour infuse $N's body with static energy, causing great pain.#n" ,ch,NULL,victim,TO_CHAR);
    act("#p$n#p touches you and chilling energies flow through your body.#n",ch,NULL,victim,TO_VICT);   
    act("#p$n#p touches $N, sending sparks flying.#n" ,ch,NULL,victim,TO_NOTVICT);

    dam = calculate_dam(ch, victim, FAE_ENERGY);
    damage(ch, victim, NULL, dam, gsn_lightning);

    if (ch->pcdata->powers[FAE_PATH] == FAE_ENERGY && atoi(event->argument) >= 8)
    {
      dam = calculate_dam(ch, victim, FAE_ENERGY);
      damage(ch, victim, NULL, dam, gsn_lightning);

      if (atoi(event->argument) >= 12)
      {
        dam = calculate_dam(ch, victim, FAE_ENERGY);
        damage(ch, victim, NULL, dam, gsn_lightning);
      }
    }
    dequeue_event(event, TRUE);

    WAIT_STATE(ch,6);
  }
  else if (!str_prefix(arg1,"plasma"))
  {
    if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_PLASMA)) == NULL)
    {
      send_to_char("Your not charging plasma.\n\r",ch);
      return;
    }
    act("#pYour body pulsates as you channel pure plasma into $N.#n" ,ch,NULL,victim,TO_CHAR);
    act("#p$n#p's body pulsates as $e channels pure plasma into you.#n",ch,NULL,victim,TO_VICT);
    act("#p$n#p's body pulsates as $e channels pure plasma into $N.#n" ,ch,NULL,victim,TO_NOTVICT);
    dam = calculate_dam(ch, victim, FAE_PLASMA);

    sn = atoi(event->argument);
    for (i = 0; i < sn; i++)
    {
      damage(ch, victim, NULL, dam, gsn_plasma);
      dam *= 0.9;
    }
    dequeue_event(event, TRUE);

    WAIT_STATE(ch, 12);
  }
  else if (!str_prefix(arg1,"will"))
  {
    if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_WILL)) == NULL)
    {
      send_to_char("Your not trying to concentrate your will.\n\r",ch);
      return;
    }
    act("#pYou bend the laws of gravity with your will and slam $N#p with stones, following up with a few spells.#n" ,ch,NULL,victim,TO_CHAR);
    act("#p$n#p stares at you while muttering a few arcane words.#n",ch,NULL,victim,TO_VICT);
    act("#pStones fly from the ground and slams into you while\n\r$n#p continues with the spellcasting.#n",ch,NULL,victim,TO_VICT);
    act("#pstones fly from the gound and slams into $N while\n\r$n#p mutters some arcane phrases.#n" ,ch,NULL,victim,TO_NOTVICT);

    sn = atoi(event->argument);
    for (i = 0; i < sn; i += 2)
    {
      dam = calculate_dam(ch,victim, FAE_WILL);
      damage(ch, victim, NULL, dam, gsn_telekinetic);
    }
    dequeue_event(event, TRUE);

    if ((sn = skill_lookup( "curse" ) ) > 0) (*skill_table[sn].spell_fun) (sn,50,ch,victim);
    if ((sn = skill_lookup( "web" ) ) > 0) (*skill_table[sn].spell_fun) (sn,50,ch,victim);

    if (ch->pcdata->powers[FAE_PATH] == FAE_WILL)
    {
      WAIT_STATE(ch, 6);
    }
    else
    {
      WAIT_STATE(ch, 12);
    }
  }
  else
    send_to_char("You have no idea how to infuse that.\n\r",ch);
}

void do_ancients(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int new_power;
  int cost = 2000000; /* 2 million pr. rank */

  argument = one_argument( argument, arg );

  if (IS_NPC(ch)) return;
  if( !IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("What?\n\r",ch);
    return;
  }
  if (arg[0] == '\0')
  {
    sprintf(buf,"#pPlasma  #G[#y%d#G]   #pMatter    #G[#y%d#G]   #pEnergy   #G[#y%d#G]   #pWill   #G[#y%d#G]#n\n\r",
      ch->pcdata->powers[FAE_PLASMA], ch->pcdata->powers[FAE_MATTER],
      ch->pcdata->powers[FAE_ENERGY], ch->pcdata->powers[FAE_WILL]);
    send_to_char(buf,ch);
    send_to_char("\n\r#p        Which ancient would you like to improve?#n\n\r",ch);
    return;
  }
  if (!str_cmp(arg,"plasma")) new_power = FAE_PLASMA;
  else if (!str_cmp(arg,"will")) new_power = FAE_WILL;
  else if (!str_cmp(arg,"energy")) new_power = FAE_ENERGY;
  else if (!str_cmp(arg,"matter")) new_power = FAE_MATTER;
  else
  {
    send_to_char("No such ancients.\n\r",ch);
    return;
  }
  if (ch->pcdata->powers[new_power] > 7)
  {
    send_to_char("You know it all, your the champ.\n\r",ch);
    return;
  }
  if (ch->wpn[6] < (ch->pcdata->powers[new_power]) * 75)
  {
    send_to_char("Your blasting powers are not good enough.\n\r",ch);
    return;
  }
  cost = cost * (ch->pcdata->powers[new_power] + 1);
  if (ch->exp < cost)
  {
    sprintf(buf,"You need %d more exp to gain that ancient.\n\r", cost - ch->exp);
    send_to_char(buf, ch);
    return;
  }
  ch->exp -= cost;
  ch->pcdata->powers[new_power]++;
  send_to_char("Ok.\n\r",ch);
  return;
}

void do_will(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (ch->pcdata->powers[FAE_WILL] < 8)
  {
    send_to_char("You have not mastered your own will yet.\n\r",ch);
    return;
  }
  if (IS_SET(ch->affected_by, AFF_FLYING))
  {
    REMOVE_BIT(ch->affected_by, AFF_FLYING);
    send_to_char("You release your control and float slowly to the ground.\n\r",ch);
  }
  else
  {
    SET_BIT(ch->affected_by, AFF_FLYING);
    send_to_char("You concentrate on the forces of gravity, and slowly float into the air.\n\r",ch);
  }
  WAIT_STATE(ch, 8);
}

void do_matter(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (ch->pcdata->powers[FAE_MATTER] < 8)
  {
    send_to_char("Your control over matter is to weak.\n\r",ch);
    return;
  }
  if (IS_AFFECTED(ch, AFF_SANCTUARY))
  {
    REMOVE_BIT(ch->affected_by, AFF_SANCTUARY);
    send_to_char("No longer needing the shield, you disband it.\n\r",ch);
  }
  else
  {
    SET_BIT(ch->affected_by, AFF_SANCTUARY);
    send_to_char("You bend the fabric of space to form a shimmering shield around you.\n\r",ch);
  }
  WAIT_STATE(ch,8);
}  

void do_energy(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (ch->pcdata->powers[FAE_ENERGY] < 8)
  {
    send_to_char("You cannot harness the energy yet.\n\r",ch);
    return;
  }
  if (IS_SET(ch->itemaffect, ITEMA_CHAOSSHIELD))
  {
    REMOVE_BIT(ch->itemaffect, ITEMA_CHAOSSHIELD);
    send_to_char("Your shields flicker and die.\n\r",ch);
  }
  else 
  {
    SET_BIT(ch->itemaffect, ITEMA_CHAOSSHIELD);
    send_to_char("You form a chaotic shield of different colors,\n\reach representing one of the different energies in the universe.\n\r",ch);
  }
  WAIT_STATE(ch, 8);
}  

void do_plasma(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (ch->pcdata->powers[FAE_PLASMA] < 8)
  {
    send_to_char("The plasma refuse to do your bidding.\n\r",ch);
    return;
  }
  if (IS_AFFECTED(ch, AFF_PROTECT) || IS_AFFECTED(ch, AFF_PROTECT_GOOD))
  {
    if (IS_SET(ch->affected_by, AFF_PROTECT)) REMOVE_BIT(ch->affected_by, AFF_PROTECT);
    if (IS_SET(ch->affected_by, AFF_PROTECT_GOOD)) REMOVE_BIT(ch->affected_by, AFF_PROTECT_GOOD);
    send_to_char("You call off your shield of protection.\n\r",ch);
  }
  else
  {
    if (IS_GOOD(ch))
    {
      SET_BIT(ch->affected_by, AFF_PROTECT);
      send_to_char("You summon a mystical force to protect you from evil.\n\r",ch);
    }
    else if (IS_EVIL(ch))
    {
      SET_BIT(ch->affected_by, AFF_PROTECT_GOOD);
      send_to_char("You summon a mystical force to protect you from good.\n\r",ch);
    }
    else send_to_char("Nothing happens.\n\r",ch);
  }
  WAIT_STATE(ch, 8);
}  

int calculate_dam(CHAR_DATA *ch, CHAR_DATA *victim, int type)
{
  EVENT_DATA *event;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];
  int dam = UMIN(225, char_damroll(ch) / 4);
  int level;

  pIter = AllocIterator(ch->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (type == FAE_PLASMA && event->type == EVENT_PLAYER_FAE_PLASMA)
      break;
    if (type == FAE_MATTER && event->type == EVENT_PLAYER_FAE_MATTER)
      break;
    if (type == FAE_ENERGY && event->type == EVENT_PLAYER_FAE_ENERGY)
      break;
    if (type == FAE_WILL && event->type == EVENT_PLAYER_FAE_WILL)
      break;
  }
  if (event == NULL)
  {
    bug("Calculate_dam: bad energy type '%d'.", type);
    return 0;
  }

  /* calculate level - caps at 10 */
  one_argument(event->argument, arg);
  level = atoi(arg);

  if (level > 10)
    level = 10;

  if (type == FAE_PLASMA)
  {
    dam *= 1 + ch->pcdata->powers[FAE_PLASMA] / 4;
    dam *= (10 * level / 13);
  }
  else if (type == FAE_ENERGY)
  {
    dam *= 1 + ch->pcdata->powers[FAE_ENERGY] / 2;
    dam *= (15 * level / 10);
  }
  else if (type == FAE_WILL)
  {
    dam *= 1 + ch->pcdata->powers[FAE_WILL] / 4;
    dam *= (10 * level / 18);
  }

  /* random from 70% to 130% of damage */
  dam = number_range(7 * dam / 10, 13 * dam / 10);

  dam = up_dam(ch, victim, dam);
  dam = cap_dam(ch, victim, dam);

  return dam;
}

void do_faetalk( CHAR_DATA *ch, char *argument )
{
  int class = ch->class;

  if (IS_NPC(ch) || (!IS_IMMORTAL(ch) && !IS_CLASS(ch, CLASS_FAE)))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  ch->class = CLASS_FAE;
  talk_channel(ch, argument, CHANNEL_CLASS, CC_FAE, "faetalk");
  ch->class = class;
}

void fae_shield(CHAR_DATA *ch, CHAR_DATA *victim, int dam)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int new_dam;

  /* doesn't work on faes or monsters */
  if (IS_NPC(victim) || !IS_CLASS(victim, CLASS_FAE))
    return;

  /* 30% chance to happen */
  if (number_percent() >= 30)
    return;

  if (victim->pcdata->powers[FAE_SHIELD] > 0)
  {
    if (--victim->pcdata->powers[FAE_SHIELD] == 0)
      send_to_char("#pYour shield of matter flickers and dies.#n\n\r",victim);

    new_dam = number_range(3 * dam / 5, 3 * dam / 4);
    if (new_dam > 1000)
      new_dam = number_range(800, 1100);
    if (new_dam >= victim->hit)
      new_dam = victim->hit - 1;
    if (new_dam <= 0)
      return;

    if (victim->pcdata->powers[FAE_PATH] == FAE_MATTER)
    {
      dam *= 11;
      dam /= 10;
    }

    /* 5% chance for double damage (10% for mastered faes) */
    if (number_percent() >= (victim->pcdata->powers[FAE_PATH] == FAE_MATTER) ? 90 : 95)
      dam *= 2;

    /* some more interesting messages */
    switch(number_range(0, 3))
    {
      default:
        sprintf(buf, "A #ystriking serpent#n appears from thin air and attacks you #9[#G%d#9]#n", new_dam);
        sprintf(buf2, "Your mattershield summons a #ystriking serpent#n to attack %s #9[#G%d#9]#n", PERS(ch, victim), new_dam);
        break;
      case 1:
        sprintf(buf, "An #yelectric charge#n shocks you #9[#G%d#9]#n", new_dam);
        sprintf(buf2, "Your mattershield discharges an #yelectric shock#n at %s #9[#G%d#9]#n", PERS(ch, victim), new_dam);
        break;
      case 2:
        sprintf(buf, "A #ywhirling vortex#n springs from %s's body and strikes you #9[#G%d#9]#n", PERS(victim, ch), new_dam);
        sprintf(buf2, "Your mattershield summons a #ywhirling vortex#n to strike %s #9[#G%d#9]#n", PERS(ch, victim), new_dam);
        break;
      case 3:
        sprintf(buf, "A flock of #ytiny fireballs#n springs from %s's eyes and hits you #9[#G%d#9]#n", PERS(victim, ch), new_dam);
        sprintf(buf2, "Your mattershield summons a flock of #ytiny fireballs#n, hitting %s #9[#G%d#9]#n", PERS(ch, victim), new_dam);
        break;
    }

    if (!IS_NPC(ch) && ch->pcdata->brief[BRIEF_5])
    {
      ch->pcdata->brief5data[BRIEF5_AMOUNT_DEALT] += dam;
      ch->pcdata->brief5data[BRIEF5_NUM_DEALT]++;
    }  
    if (!IS_NPC(victim) && victim->pcdata->brief[BRIEF_5])
    {
      victim->pcdata->brief5data[BRIEF5_AMOUNT_RECEIVED] += dam;
      victim->pcdata->brief5data[BRIEF5_NUM_RECEIVED]++;
    }

    if (!IS_NPC(ch) && !ch->pcdata->brief[BRIEF_5])
      act(buf, victim, NULL, ch, TO_VICT);
    if (!IS_NPC(victim) && !victim->pcdata->brief[BRIEF_5])
      act(buf2, victim, NULL, ch, TO_CHAR);

    hurt_person(victim, ch, new_dam);
  }
}

void do_chaossigil(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int cost = 2000;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_ARCANE] < 7)
  {
    send_to_char("You need level 7 discipline in arcane.\n\r",ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Draw a sigil of chaos upon whom?\n\r", ch);
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
  if (event_isset_mobile(victim, EVENT_PLAYER_FAE_SIGIL))
  {
    send_to_char("They already have a sigil carved upon their forehead.\n\r", ch);
    return;
  }
  if (is_safe(ch, victim)) return;

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMana);

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to use this power.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("You draw a sigil of chaos upon $N's forehead.", ch, NULL, victim, TO_CHAR);
  act("$n draws a sigil of chaos upon your forehead.", ch, NULL, victim, TO_VICT);
  act("$n draws a sigil of chaos upon $N's forehead.", ch, NULL, victim, TO_NOTVICT);

  event            =  alloc_event();
  event->fun       =  &event_player_fae_sigil;
  event->type      =  EVENT_PLAYER_FAE_SIGIL;
  add_event_char(event, victim, 3 * PULSE_PER_SECOND);

  aggress(ch, victim);
}

void do_timewarp(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  int cost = 2500;
  char *action;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_ARCANE] < 6)
  {
    send_to_char("You need level 6 discipline in arcane.\n\r",ch);
    return;
  }
  if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_TIMEWARP)) != NULL)
  {
    printf_to_char(ch, "You must wait %s before you can use this power again.\n\r", event_time_left(event));
    return;
  }
  if ((victim = ch->fighting) == NULL)
  {
    send_to_char("You are not fighting anyone.\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("Not on mobiles.\n\r", ch);
    return;
  }
  if (!victim->desc)
  {
    send_to_char("Not on linkdead players.\n\r", ch);
    return;
  }
  if (!victim->desc->inlast || strlen(victim->desc->inlast) < 2)
  {
    send_to_char("But they haven't done anything yet that they can relive.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to use this power.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("$n waves $s fingers and utters a string of gutteral words at $N.", ch, NULL, victim, TO_ROOM);
  act("You wave your fingers at $N, uttering the phrase of ages.", ch, NULL, victim, TO_CHAR);
  act("$n waves $s fingers at you and utters a string of gutteral words.", ch, NULL, victim, TO_VICT);

  action = str_dup(victim->desc->inlast);
  interpret(victim, action);
  free_string(action);

  /* enqueue wait state for power */
  event            =  alloc_event();     
  event->fun       =  &event_player_fae_timewarp;
  event->type      =  EVENT_PLAYER_FAE_TIMEWARP;
  add_event_char(event, ch, 60 * PULSE_PER_SECOND);

  WAIT_STATE(ch, 6);
}

bool event_player_fae_sigil(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  EVENT_DATA *newevent;
  int dam;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_fae_sigil: no owner.", 0);
    return FALSE;
  }

  /* calculate damage */
  dam = number_range(300, 600);
  dam = UMIN(dam, ch->hit - 1);

  if (dam <= 0 || ch->level < 3)
    return FALSE;

  modify_hps(ch, -1 * dam);

  act("You scream in pain as the chaos sigil bores deeper into your forehead.", ch, NULL, NULL, TO_CHAR);
  act("$n screams in pain as the sigil on $s forehead glows a pale blue.", ch, NULL, NULL, TO_ROOM);

  if (number_percent() > 10)
  {
    newevent            =  alloc_event();
    newevent->fun       =  &event_player_fae_sigil;
    newevent->type      =  EVENT_PLAYER_FAE_SIGIL;
    add_event_char(newevent, ch, 3 * PULSE_PER_SECOND);
  }

  return FALSE;
}

bool event_player_fae_plasma(EVENT_DATA *event)
{
  EVENT_DATA *newevent;
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char event_arg[MAX_STRING_LENGTH];
  CHAR_DATA *ch;
  char *cPtr;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_fae_plasma: no owner.", 0);
    return FALSE;
  }

  /* perhaps it went mortal or something */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE))
  {
    return FALSE;
  }

  cPtr = one_argument(event->argument, arg1);
  one_argument(cPtr, arg2);

  /* should check for backfire */
  if (atoi(arg1) > ch->pcdata->powers[FAE_PLASMA])
  {
    modify_hps(ch, -1 * atoi(arg1) * number_range(200, 400));
    send_to_char("#oYou scream in pain as you lose control over the forces of plasma.#n\n\r", ch);
    act("$n screams in pain, strange plasma flows from $s body.", ch, NULL, NULL, TO_ROOM);

    update_pos(ch);

    return FALSE;
  }
  send_to_char("#yYou channel more #Gplasma#y from your surroundings into your body.#n\n\r", ch);
  act("$n draws more plasma into $s body.", ch, NULL, NULL, TO_ROOM);

  /* make new event, and enqueue it */
  sprintf(event_arg, "%d %d", atoi(arg1) + atoi(arg2), atoi(arg2));
  newevent            =  alloc_event();
  newevent->argument  =  str_dup(event_arg);
  newevent->fun       =  &event_player_fae_plasma;
  newevent->type      =  EVENT_PLAYER_FAE_PLASMA;
  add_event_char(newevent, ch, 3 * PULSE_PER_SECOND);

  /* make sure old event is dequeued */
  return FALSE;
}

bool event_player_fae_matter(EVENT_DATA *event)
{
  EVENT_DATA *newevent;
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char event_arg[MAX_STRING_LENGTH];
  CHAR_DATA *ch;
  char *cPtr;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_fae_matter: no owner.", 0);
    return FALSE;
  }

  /* perhaps it went mortal or something */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE))
  {
    return FALSE;
  }

  cPtr = one_argument(event->argument, arg1);
  one_argument(cPtr, arg2);

  /* should check for backfire */
  if (atoi(arg1) > ch->pcdata->powers[FAE_MATTER])
  {
    modify_hps(ch, -1 * atoi(arg1) * number_range(200, 400));
    send_to_char("#oYou scream in pain as you lose control over the forces of matter.#n\n\r", ch);
    act("$n screams in pain, strange matter flows from $s body.", ch, NULL, NULL, TO_ROOM);

    update_pos(ch);

    return FALSE;
  }
  send_to_char("#yYou channel more #Gmatter#y from your homeplane into existance.#n\n\r", ch);
  act("$n draws more matter into $s body.", ch, NULL, NULL, TO_ROOM);

  /* make new event, and enqueue it */
  sprintf(event_arg, "%d %d", atoi(arg1) + atoi(arg2), atoi(arg2));
  newevent            =  alloc_event();
  newevent->argument  =  str_dup(event_arg);
  newevent->fun       =  &event_player_fae_matter;
  newevent->type      =  EVENT_PLAYER_FAE_MATTER;
  add_event_char(newevent, ch, 3 * PULSE_PER_SECOND);

  /* make sure old event is dequeued */
  return FALSE;
}

bool event_player_fae_energy(EVENT_DATA *event)
{
  EVENT_DATA *newevent;
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char event_arg[MAX_STRING_LENGTH];
  CHAR_DATA *ch;
  char *cPtr;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_fae_energy: no owner.", 0);
    return FALSE;
  }

  /* perhaps it went mortal or something */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE))
  {
    return FALSE;
  }

  cPtr = one_argument(event->argument, arg1);
  one_argument(cPtr, arg2);

  /* should check for backfire */
  if (atoi(arg1) > ch->pcdata->powers[FAE_ENERGY])
  {
    modify_hps(ch, -1 * atoi(arg1) * number_range(200, 400));
    send_to_char("#oYou scream in pain as you lose control over the forces of energy.#n\n\r", ch);
    act("$n screams in pain, strange energies flows from $s body.", ch, NULL, NULL, TO_ROOM);

    update_pos(ch);

    return FALSE;
  }
  send_to_char("#yYou channel more #Genergy#y from the arcane into your mind.#n\n\r", ch);
  act("$n draws more energy into $s body.", ch, NULL, NULL, TO_ROOM);

  /* make new event, and enqueue it */
  sprintf(event_arg, "%d %d", atoi(arg1) + atoi(arg2), atoi(arg2));
  newevent            =  alloc_event();
  newevent->argument  =  str_dup(event_arg);
  newevent->fun       =  &event_player_fae_energy;
  newevent->type      =  EVENT_PLAYER_FAE_ENERGY;
  add_event_char(newevent, ch, 3 * PULSE_PER_SECOND);

  /* make sure old event is dequeued */
  return FALSE;
}

bool event_player_fae_will(EVENT_DATA *event)
{
  EVENT_DATA *newevent;
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char event_arg[MAX_STRING_LENGTH];
  CHAR_DATA *ch;
  char *cPtr;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_fae_will: no owner.", 0);
    return FALSE;
  }

  /* perhaps it went mortal or something */
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE))
  {
    return FALSE;
  }

  cPtr = one_argument(event->argument, arg1);
  one_argument(cPtr, arg2);

  /* should check for backfire */
  if (atoi(arg1) > ch->pcdata->powers[FAE_WILL])
  {
    modify_hps(ch, -1 * atoi(arg1) * number_range(200, 400));
    send_to_char("#oYou scream in pain as you lose concentration and your willpower crumbles.#n\n\r", ch);
    act("$n screams in pain, and clutches $s head.", ch, NULL, NULL, TO_ROOM);

    update_pos(ch);

    return FALSE;
  }
  send_to_char("#yYou concentrate on your #Gwillpower#y, focusing your thoughts.#n\n\r", ch);
  act("$n concentrates on $s willpower.", ch, NULL, NULL, TO_ROOM);

  /* make new event, and enqueue it */
  sprintf(event_arg, "%d %d", atoi(arg1) + atoi(arg2), atoi(arg2));
  newevent            =  alloc_event();
  newevent->argument  =  str_dup(event_arg);
  newevent->fun       =  &event_player_fae_will;
  newevent->type      =  EVENT_PLAYER_FAE_WILL;
  add_event_char(newevent, ch, 3 * PULSE_PER_SECOND);

  /* make sure old event is dequeued */
  return FALSE;
}

void do_spidercall(CHAR_DATA *ch, char *argument)   
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  int cost = 1500;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_NATURE] < 6)
  {
    send_to_char("You need level 6 discipline in nature.\n\r",ch);
    return;
  }
  if ((victim = ch->fighting) == NULL)
  {
    send_to_char("You are not fighting anyone.\n\r", ch);
    return;
  }
  if (event_isset_mobile(victim, EVENT_PLAYER_SPIDERS))
  {
    send_to_char("Spiders are already crawling all over this one.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, victim, cost, eMove);

  if (ch->move < cost)
  {
    printf_to_char(ch, "You need %d more move to use this power.\n\r", cost - ch->move);
    return;
  }
  modify_move(ch, -1 * cost);

  /* messages */
  act("$n lets out a primal, high pitched scream, staring at $N.", ch, NULL, victim, TO_NOTVICT);
  act("You call out to the spiders, ordering them to cocoon $N.", ch, NULL, victim, TO_CHAR);
  act("$n lets out a primal, high pitched scream, staring at you.", ch, NULL, victim, TO_VICT);
  act("Spiders of all sizes crawl towards $n.", victim, NULL, NULL, TO_ROOM);
  act("Spiders of all sizes crawl towards you.", victim, NULL, NULL, TO_CHAR);

  /* create event and attach */
  event            =  alloc_event();
  event->argument  =  str_dup("first");
  event->fun       =  &event_player_spiders;
  event->type      =  EVENT_PLAYER_SPIDERS;
  add_event_char(event, victim, number_range(2, 4) * PULSE_PER_SECOND);
}

bool event_player_spiders(EVENT_DATA *event)
{
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_spiders: no owner.", 0);
    return FALSE;
  }
  if (!str_cmp(event->argument, "first"))
  {
    if (number_percent() > 80)
    {
      act("$n franticly brushes the spiders of $m.", ch, NULL, NULL, TO_ROOM);
      send_to_char("You where able to brush all the spiders of you.\n\r", ch);
    }
    else
    {
      EVENT_DATA *newevent;

      SET_BIT(ch->affected_by, AFF_WEBBED);
      act("$n is cocooned in a glob of spiderweb, spiders scurry about $m.", ch, NULL, NULL, TO_ROOM);
      send_to_char("You are cocooned in a glob of spiderweb by the spiders crawling all over you.\n\r", ch);

      /* create event and attach */
      newevent            =  alloc_event();
      newevent->argument  =  str_dup("second");
      newevent->fun       =  &event_player_spiders;
      newevent->type      =  EVENT_PLAYER_SPIDERS;
      add_event_char(newevent, ch, number_range(6, 10) * PULSE_PER_SECOND);
    }

    return FALSE;
  }

  if (!IS_SET(ch->affected_by, AFF_WEBBED))
  {
    act("$n is cocooned in a glob of spiderweb, spiders scurries away.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You are cocooned in a glob of spiderweb, and the spiders crawl away.\n\r", ch);

    SET_BIT(ch->affected_by, AFF_WEBBED);
  }

  return FALSE;
}

void do_disctrain(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_STRING_LENGTH];
  int chance = ch->practice * 25;

  if (IS_NPC(ch)) return;  
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char(" #9#uFaerie Discipline Training System#n\n\r", ch);
    send_to_char(" You can use this command to train both the arcane and nature disciplines\n\r", ch);
    send_to_char(" of the fae class. The more primal you have when you try to gain another level\n\r", ch);
    send_to_char(" the higher the chance of gaining it. Whether you fail or succeded, you will\n\r", ch);
    send_to_char(" lose all the primal you currently have. A good rule is to spend at least\n\r", ch);
    send_to_char(" 20 primal for each level (so spend at least 100 primal if you want to gain\n\r", ch);
    send_to_char(" level 5 in a discipline). The syntax is simply the following :\n\n\r", ch);
    send_to_char(" disctrain <nature|arcane>\n\n\r", ch);
    sprintf(arg, " Current levels :  arcane [%d]  nature [%d]\n\r",
      ch->pcdata->powers[DISC_FAE_ARCANE], 
      ch->pcdata->powers[DISC_FAE_NATURE]);
    send_to_char(arg, ch);
    return;
  }
  if (!str_cmp(arg, "nature"))
  {
    if (IS_SET(ch->pcdata->powers[EVOLVE_1], FAE_EVOLVE_NATURE))
    {
      if (ch->pcdata->powers[DISC_FAE_NATURE] >= 10)
      {
        send_to_char("You have already mastered the arts of nature.\n\r", ch);
        return;
      }
    }
    else if (ch->pcdata->powers[DISC_FAE_NATURE] >= 8)
    {
      send_to_char("You have already mastered the arts of nature.\n\r", ch);
      return;
    }

    chance /= (10 * (ch->pcdata->powers[DISC_FAE_NATURE] + 1));
    if (ch->practice < (ch->pcdata->powers[DISC_FAE_NATURE] + 1) * 15)
      chance = 0;
    if (chance > 80)
      chance = 80;
    ch->practice = 0;
    if (number_percent() < chance)
    {
      send_to_char("The powers of the faerie smile upon you.\n\r", ch);
      ch->pcdata->powers[DISC_FAE_NATURE]++;
    }
    else
    {
      send_to_char("The powers of the faerie escapes your grasp.\n\r", ch);
    }
  }
  else if (!str_cmp(arg, "arcane"))
  {
    if (IS_SET(ch->pcdata->powers[EVOLVE_1], FAE_EVOLVE_ARCANE))
    {
      if (ch->pcdata->powers[DISC_FAE_ARCANE] >= 10)
      {
        send_to_char("You have already mastered the arcane arts.\n\r", ch);
        return;
      }
    }
    else if (ch->pcdata->powers[DISC_FAE_ARCANE] >= 8)
    {
      send_to_char("You have already mastered the arcane arts.\n\r", ch);
      return;
    }

    chance /= (10 * (ch->pcdata->powers[DISC_FAE_ARCANE] + 1));
    if (ch->practice < (ch->pcdata->powers[DISC_FAE_ARCANE] + 1) * 15)
      chance = 0;
    if (chance > 80)
      chance = 80;
    ch->practice = 0;

    if (number_percent() < chance)
    {
      send_to_char("The powers of the faerie smile upon you.\n\r", ch);
      ch->pcdata->powers[DISC_FAE_ARCANE]++;
    }
    else
    {   
      send_to_char("The powers of the faerie escapes your grasp.\n\r", ch);
    }
  }
  else
  {
    do_disctrain(ch, "");
  }
}

void do_spiritkiss(CHAR_DATA *ch, char *argument)
{
  int sn, cost = 500;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_NATURE] < 5)
  {
    send_to_char("You need level 5 discipline in nature.\n\r", ch);
    return;
  }
  if ((sn = skill_lookup("spirit kiss")) <= 0)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to use this power.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  send_to_char("You call upon the blessing of the ancient faerie spirits.\n\r", ch);
  (*skill_table[sn].spell_fun) (sn, ch->pcdata->powers[DISC_FAE_NATURE] * 10, ch, ch);
}

void do_bloodtrack(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE) ||
     !IS_SET(ch->pcdata->powers[EVOLVE_3], FAE_EVOLVE_BLOODTASTE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (event_isset_mobile(ch, EVENT_PLAYER_BLOODTASTE))
  {
    send_to_char("You are already tracking someone's taste of blood.\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Start tracking whom?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (ch == victim)
  {
    send_to_char("You cannot track yourself.\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("You can only track players, not monsters.\n\r", ch);
    return;
  }
  if (victim->hit >= victim->max_hit)
  {
    act("$N is not bleeding, you cannot $S their blood.", ch, NULL, victim, TO_CHAR);
    return;
  }

  act("$n licks you, stealing a few drops of your blood.", ch, NULL, victim, TO_VICT);
  act("You lick $N, stealing a few drops of $S blood.", ch, NULL, victim, TO_CHAR);
  act("$n licks $N, stealing a few drops of $S blood.", ch, NULL, victim, TO_NOTVICT);

  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_PLAYER_BLOODTASTE;
  event->argument = str_dup(victim->name);
  add_event_char(event, ch, 4 * 60 * PULSE_PER_SECOND);
}

void fae_hunger(CHAR_DATA *ch)
{
  CHAR_DATA *victim;
  ITERATOR *pIter;
  EVENT_DATA *event;

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_BLOODTASTE)) == NULL)
    return;

  if (ch->fighting != NULL)
    return;

  pIter = AllocIterator(ch->in_room->people);
  while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (victim == ch || IS_NPC(ch))
      continue;

    if (!str_cmp(victim->name, event->argument))
      break;
  }

  if (victim != NULL)
  {
    act("$n screams out in hatred, attacking $N.", ch, NULL, victim, TO_NOTVICT);
    act("You scream out in hatred and attack $N.", ch, NULL, victim, TO_CHAR);
    act("$n screams out in hatred and attack you.", ch, NULL, victim, TO_VICT);

    aggress(ch, victim);
  }
}

void do_acidheart(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE) ||
     !IS_SET(ch->pcdata->powers[EVOLVE_3], FAE_EVOLVE_ACIDHEART))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (!str_cmp(argument, "on"))
  {
    if (!IS_SET(ch->pcdata->powers[FAE_BITS], FAE_ACIDHEART))
    {
      int cost = 1000;

      if (ch->mana < cost)
      {
        printf_to_char(ch, "You need %d more mana to do this.\n\r", cost - ch->mana);
        return;
      }
      modify_mana(ch, -1 * cost);

      SET_BIT(ch->pcdata->powers[FAE_BITS], FAE_ACIDHEART);
      send_to_char("Your heart starts pumping acid through your veins.\n\r", ch);
    }
    else
    {
      send_to_char("Your heart is already pumping acid through your veins.\n\r", ch);
    }
  }
  else if (!str_cmp(argument, "off"))
  {
    if (IS_SET(ch->pcdata->powers[FAE_BITS], FAE_ACIDHEART))
    {
      REMOVE_BIT(ch->pcdata->powers[FAE_BITS], FAE_ACIDHEART);
      send_to_char("Your heart stops pumping acid through your veins.\n\r", ch);
    }
    else
    {
      send_to_char("Your heart is not pumping acid through your veins.\n\r", ch);
    }
  }
  else
  {
    send_to_char("Syntax: acidheart [on|off]\n\r", ch);
  }
}

void do_bloodhunger(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch) || !IS_CLASS(ch, CLASS_FAE) ||
     !IS_SET(ch->pcdata->powers[EVOLVE_3], FAE_EVOLVE_BLOODTASTE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (!str_cmp(argument, "on"))
  {
    if (!IS_SET(ch->pcdata->powers[FAE_BITS], FAE_BLOODHUNGER))
    {
      SET_BIT(ch->pcdata->powers[FAE_BITS], FAE_BLOODHUNGER);
      send_to_char("You hunger for the taste of blood.\n\r", ch);
    }
    else
    {
      send_to_char("You already hunger for the taste of blood.\n\r", ch);
    }
  }
  else if (!str_cmp(argument, "off"))
  {
    if (IS_SET(ch->pcdata->powers[FAE_BITS], FAE_BLOODHUNGER))
    {
      REMOVE_BIT(ch->pcdata->powers[FAE_BITS], FAE_BLOODHUNGER);
      send_to_char("Your quell your hunger for the taste of blood.\n\r", ch);
    }
    else
    {
      send_to_char("Your taste for blood is already quelled.\n\r", ch);
    }
  }
  else
  {
    send_to_char("Syntax: bloodhunger [on|off]\n\r", ch);
  }
}

void do_gaseous(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_NATURE] < 3)
  {
    send_to_char("You need level 3 discipline in nature.\n\r", ch);
    return;
  }
  TOGGLE_BIT(ch->pcdata->powers[FAE_BITS], FAE_GASEOUS);
  if (IS_SET(ch->pcdata->powers[FAE_BITS], FAE_GASEOUS))
  {
    send_to_char("You body becomes a gaseous mass.\n\r", ch);
  }
  else
  {
    send_to_char("Your become more substantial.\n\r", ch);
  }
}

void do_blastbeams(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }  
  if (ch->pcdata->powers[DISC_FAE_ARCANE] < 1)
  {
    send_to_char("You need level 1 discipline in arcane.\n\r",ch);
    return;
  } 
  TOGGLE_BIT(ch->pcdata->powers[FAE_BITS], FAE_BLASTBEAMS);
  if (IS_SET(ch->pcdata->powers[FAE_BITS], FAE_BLASTBEAMS))
  {
    if ((obj = get_eq_char(ch, WEAR_HOLD)) != NULL)
      take_item(ch, obj);
    if ((obj = get_eq_char(ch, WEAR_WIELD)) != NULL)
      take_item(ch, obj);

    send_to_char("You turn your hands into beams of energy.\n\r", ch);
  }
  else
  {
    send_to_char("Your hands return to normal.\n\r", ch);
  }
}

void do_faetune(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  char arg[MAX_INPUT_LENGTH];
  const int cost = 1000;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_NATURE] < 9)
  {
    send_to_char("You need level 9 discipline in nature.\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("What weapon do you wish to faetune?\n\r", ch);
    return;
  }
  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You are not carrying that item.\n\r", ch);
    return;
  }
  if (obj->item_type != ITEM_WEAPON)
  {
    send_to_char("You can only faetune weapons.\n\r", ch);
    return;
  }
  if (IS_SET(obj->quest, QUEST_ARTIFACT))
  {
    send_to_char("Not on artifacts.\n\r", ch);
    return;
  }
  if (obj->ownerid != ch->pcdata->playerid)
  {
    send_to_char("You can only faetune weapons that you own.\n\r", ch);
    return;
  }
  if (IS_SET(obj->extra_flags, ITEM_FAE_BLAST))
  {
    send_to_char("That weapon has already been tuned or blasted.\n\r", ch);
    return;
  }
  if (getGold(ch) < cost)
  {
    send_to_char("You don't have 1.000 goldcrowns.\n\r", ch);   
    return;
  }
  SET_BIT(obj->extra_flags, ITEM_FAE_BLAST);
  obj->value[3] = 6;
  setGold(ch, -1 * cost);

  act("$n blasts $p with a beam of pure energy.", ch, obj, NULL, TO_ROOM);
  act("You blast $p with a beam of pure energy.", ch, obj, NULL, TO_CHAR);
}

void do_faeblast(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  AFFECT_DATA *af;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];
  const int cost = 1000;
  int damroll = 0, hitroll = 0, armor = 0;
  bool bDam = FALSE, bHit = FALSE, bArm = FALSE;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_ARCANE] < 8)
  {
    send_to_char("You need level 8 discipline in arcane.\n\r",ch);
    return;
  }
  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("What weapon do you wish to faeblast?\n\r", ch);
    return;
  }
  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You are not carrying that item.\n\r", ch);
    return;
  }
  if (obj->item_type != ITEM_WEAPON)
  {
    send_to_char("You can only faeblast weapons.\n\r", ch);
    return;
  }
  if (IS_SET(obj->quest, QUEST_ARTIFACT))
  {
    send_to_char("Not on artifacts.\n\r", ch);
    return;
  }
  if (obj->value[3] != 6)
  {
    send_to_char("You can only faeblast blast weapons.\n\r", ch);
    return;
  }
  if (obj->ownerid != ch->pcdata->playerid)
  {
    send_to_char("You can only faeblast weapons that you own.\n\r", ch);
    return;
  }
  if (IS_SET(obj->extra_flags, ITEM_FAE_BLAST))
  {
    send_to_char("That weapon has already been blasted or tuned.\n\r", ch);
    return;
  }
  if (getGold(ch) < cost)
  {
    send_to_char("You don't have 1.000 goldcrowns.\n\r", ch);
    return;
  }

  pIter = AllocIterator(obj->pIndexData->affected);
  while ((af = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (af->location == APPLY_HITROLL)
      hitroll += af->modifier;
    else if (af->location == APPLY_DAMROLL)
      damroll += af->modifier;
    else if (af->location == APPLY_AC)
      armor   += af->modifier;
  }

  pIter = AllocIterator(obj->affected);
  while ((af = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (af->location == APPLY_HITROLL)
    {
      af->modifier = 2 * af->modifier + hitroll;
      bHit = TRUE;
    }
    else if (af->location == APPLY_DAMROLL)
    {
      af->modifier = 2 * af->modifier + damroll;
      bDam = TRUE;
    }
    else if (af->location == APPLY_AC)
    {
      af->modifier = 2 * af->modifier + armor;
      bArm = TRUE;
    }
  }
  if (!bHit && hitroll != 0)
  {
    AFFECT_DATA paf;
  
    paf.type           = 0;
    paf.duration       = -1;
    paf.location       = APPLY_HITROLL;
    paf.modifier       = hitroll;
    paf.bitvector      = 0;
    affect_to_obj(obj, &paf);
  }
  if (!bDam && damroll != 0)
  {
    AFFECT_DATA paf;

    paf.type           = 0;
    paf.duration       = -1;
    paf.location       = APPLY_DAMROLL;
    paf.modifier       = damroll;
    paf.bitvector      = 0;
    affect_to_obj(obj, &paf);
  }
  if (!bArm && armor != 0)
  {
    AFFECT_DATA paf;

    paf.type           = 0;
    paf.duration       = -1;
    paf.location       = APPLY_AC;
    paf.modifier       = armor;
    paf.bitvector      = 0;
    affect_to_obj(obj, &paf);
  }
  SET_BIT(obj->extra_flags, ITEM_FAE_BLAST);
  setGold(ch, -1 * cost);

  act("$n blasts $p with a beam of pure energy.", ch, obj, NULL, TO_ROOM);
  act("You blast $p with a beam of pure energy.", ch, obj, NULL, TO_CHAR);
}

void do_geyser(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  int cost = 2500;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (!ch->in_room)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_NATURE] < 7)
  {
    send_to_char("You need level 7 discipline in nature.\n\r",ch);
    return;
  }
  if ((event = event_isset_mobile(ch, EVENT_PLAYER_GEYSER_WAIT)) != NULL)
  {
    printf_to_char(ch, "You must wait %s before you can use this power again.\n\r", event_time_left(event));
    return;
  }

  if (event_isset_room(ch->in_room, EVENT_ROOM_GEYSER))
  {
    send_to_char("There is already a healing geyser in this room.\n\r", ch);
    return;
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to summon a geyser.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("$n raises a steaming geyser from the ground.", ch, NULL, NULL, TO_ROOM);
  act("You raise a healing geyser from the ground.", ch, NULL, NULL, TO_CHAR);

  /* room event */
  event              =  alloc_event();
  event->argument    =  str_dup("first");
  event->fun         =  &event_room_geyser;
  event->type        =  EVENT_ROOM_GEYSER;
  add_event_room(event, ch->in_room, 2 * PULSE_PER_SECOND);

  /* player event */
  event              =  alloc_event();
  event->fun         =  &event_dummy;
  event->type        =  EVENT_PLAYER_GEYSER_WAIT;
  add_event_char(event, ch, 6 * PULSE_PER_SECOND);
}

void do_faepipes(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *gch;
  EVENT_DATA *event;
  ITERATOR *pIter;
  char *ptr;
  int cost = 2500;
  int success = 0;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (!ch->in_room)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_NATURE] < 10)
  {
    send_to_char("You need level 10 discipline in nature.\n\r",ch);
    return;
  }

  if (event_isset_mobile(ch, EVENT_PLAYER_FAEPIPES))
  {
    send_to_char("You are still piping on your pipes.\n\r", ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  cost = reduce_cost(ch, NULL, cost, eMove);
   
  if (ch->move < cost)
  {
    printf_to_char(ch, "You need %d more move to use this power.\n\r", cost - ch->move);
    return;
  }
  modify_move(ch, -1 * cost);

  /* enqueue pipe event, preventing players from spamming this command */
  event            = alloc_event();
  event->owner.ch  = ch;
  event->fun       = &event_dummy;
  event->type      = EVENT_PLAYER_FAEPIPES;
  add_event_char(event, ch, 60 * PULSE_PER_SECOND);

  act("$n starts playing a haunting melody on $s pipes.", ch, NULL, NULL, TO_ROOM);
  send_to_char("You pull out your pipes and start playing.\n\r", ch);

  /* find all mobiles in this area, and tell them to move */
  pIter = AllocIterator(char_list);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (ch->position != POS_STANDING) continue;
    if (IS_SET(gch->act, ACT_SENTINEL) && saves_spell(40, gch)) continue;
    if (event_isset_mobile(gch, EVENT_MOBILE_PIPEMOVE)) continue;
    if (gch->in_room == NULL || gch->in_room == ch->in_room) continue;
    if (gch->in_room->area != ch->in_room->area) continue;
    if (gch->level > 1000 && saves_spell(40, gch)) continue;
    if (gch->spec_fun || gch->shop_fun || gch->quest_fun) continue;
    if (saves_spell(40, gch)) continue;

    if (ch->in_room != gch->in_room && !IS_NPC(gch))
    {
      send_to_char("An eerie haunting sound is heard in the distance.\n\r", gch);
      continue;
    }

    if ((ptr = pathfind(gch, ch)) != NULL)
    {
      event            = alloc_event();
      event->argument  = str_dup(ptr);
      event->fun       = &event_mobile_pipemove;
      event->type      = EVENT_MOBILE_PIPEMOVE;
      add_event_char(event, gch, 3);
    }
    else continue;

    if (++success >= 8) break;
  }
}

bool event_mobile_pipemove(EVENT_DATA *event)
{
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_pipemove: no owner.", 0);
    return FALSE;
  }

  /* ready to walk or not? */
  if (ch->position != POS_STANDING)
    return FALSE;

  /* follow the path, if possible */
  if (event->argument && event->argument[0] != '\0')
  {
    ROOM_INDEX_DATA *in_room = ch->in_room;

    switch(event->argument[0])
    {
      default:
        bug("event_mobile_pipemove: bad dir '%d'", event->argument[0]);
        return FALSE;
        break;
      case 'n':
        do_north(ch, "");
        break;
      case 's':
        do_south(ch, "");
        break;
      case 'e':
        do_east(ch, "");
        break;
      case 'w':
        do_west(ch, "");
        break;
      case 'd':
        do_down(ch, "");
        break;
      case 'u': 
        do_up(ch, "");
        break;
    }

    /* failed to move */
    if (in_room == ch->in_room)
      return FALSE;
  }
  else
  {
    bug("event_mobile_pipemove: no argument", 0);
    return FALSE;
  }

  if (event->argument[1] != '\0')
  {
    EVENT_DATA *newevent;

    newevent           = alloc_event();
    newevent->fun      = &event_mobile_pipemove;
    newevent->type     = EVENT_MOBILE_PIPEMOVE;
    newevent->argument = str_dup(&event->argument[1]);
    add_event_char(newevent, ch, 3);
  }

  return FALSE;
}

void do_unleashed(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *gch;
  EVENT_DATA *event;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];
  int move_cost = 500, mana_cost = 500, plasma = 0, matter = 0, will = 0, energy = 0;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))  
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (!ch->in_room)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_NATURE] < 8)
  {
    send_to_char("You need level 8 discipline in nature.\n\r",ch);
    return;
  }

  /* reduce cost due to int/wis/con */
  mana_cost = reduce_cost(ch, NULL, mana_cost, eMana);
  move_cost = reduce_cost(ch, NULL, move_cost, eMove);

  if (ch->move < move_cost || ch->mana < mana_cost)
  {
    printf_to_char(ch, "You need %d more move and %d more mana.\n\r",
      (move_cost - ch->move > 0) ? move_cost - ch->move : 0,
      (mana_cost - ch->mana > 0) ? mana_cost - ch->mana : 0);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_PLASMA)) != NULL)
  {
    one_argument(event->argument, arg);
    plasma = atoi(arg);
    dequeue_event(event, TRUE);
  }
  if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_WILL)) != NULL)
  {
    one_argument(event->argument, arg);
    will = atoi(arg);
    dequeue_event(event, TRUE);
  }
  if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_ENERGY)) != NULL)
  {
    one_argument(event->argument, arg);
    energy = atoi(arg);
    dequeue_event(event, TRUE);
  }
  if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_MATTER)) != NULL)
  {
    one_argument(event->argument, arg);
    matter = atoi(arg);
    dequeue_event(event, TRUE);
  }
  if (matter < 2 && will < 2 && energy < 2 && plasma < 2)
  {
    send_to_char("Nothing happens.\n\r", ch);
    return;
  }

  act("$n unleashes all $s stored energies.", ch, NULL, NULL, TO_ROOM);
  act("You unleash all your stored ancients.", ch, NULL, NULL, TO_CHAR);

  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (gch->fighting != ch || !IS_NPC(gch)) continue;
    if (is_safe(ch, gch)) continue;

    if (matter >= 2)
    {
      matter -= 2;
      one_hit(ch, gch, gsn_matter, 1);
    }
    if (plasma >= 2 && !gch->dead)
    {
      plasma -= 2;
      one_hit(ch, gch, gsn_plasma, 1);
    }
    if (will >= 2 && !gch->dead)
    {
      will -= 2;
      one_hit(ch, gch, gsn_telekinetic, 1);
    }
    if (energy >= 2 && !gch->dead)
    {
      energy -= 2;
      one_hit(ch, gch, gsn_lightning, 1);
    }
  }

  modify_move(ch, -1 * move_cost);
  modify_mana(ch, -1 * mana_cost);
  WAIT_STATE(ch, 12);
}

bool event_room_geyser(EVENT_DATA *event)
{
  CHAR_DATA *gch;
  ITERATOR *pIter;
  ROOM_INDEX_DATA *pRoom;

  if ((pRoom = event->owner.room) == NULL)
  {
    bug("event_room_geyser: no owner.", 0);
    return FALSE;
  }
  if (!str_cmp(event->argument, "first") ||
      !str_cmp(event->argument, "second") ||
      !str_cmp(event->argument, "third"))
  {
    EVENT_DATA *newevent;

    newevent              =  alloc_event();
    if (!str_cmp(event->argument, "first"))
      newevent->argument  =  str_dup("second");
    else if (!str_cmp(event->argument, "second"))
      newevent->argument  =  str_dup("third");
    else
      newevent->argument  =  str_dup("fourth");
    newevent->fun         =  &event_room_geyser;
    newevent->type        =  EVENT_ROOM_GEYSER;
    add_event_room(newevent, pRoom, 3 * PULSE_PER_SECOND);
  }

  pIter = AllocIterator(pRoom->people);
  if ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    act("A geyser spews forth a burst of healing waters.", gch, NULL, NULL, TO_ALL);

    do {
      modify_mana(gch, number_range(1000, 2000));
      modify_move(gch, number_range(1000, 2000));
      modify_hps(gch, number_range(1000, 2000));
      update_pos(gch);

      send_to_char("You feel healing energies wash over you.\n\r", gch);
    } while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL);
  }

  return FALSE;
}

bool event_player_fae_timewarp(EVENT_DATA *event)
{
  /* This isn't supposed to do anything, besides
   * preventing the player from using timewarp
   */
  return FALSE;
}

void do_martyr(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int move, mana, hit;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_NATURE] < 4)
  {
    send_to_char("You need level 4 discipline in nature.\n\r", ch);
    return;
  }
  one_argument(argument, arg);
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char("Not on yourself.\n\r", ch);
    return;
  }
  if (victim->hit  == victim->max_hit &&
      victim->move == victim->max_move &&
      victim->mana == victim->max_mana)
  {
    send_to_char("They have sustained no wounds.\n\r", ch);
    return;
  }
  move = victim->max_move - victim->move;
  hit  = victim->max_hit  - victim->hit;
  mana = victim->max_mana - victim->mana;

  move = UMIN(ch->move / 2, move / 2);
  hit  = UMIN(ch->hit  / 2, hit  / 2);
  mana = UMIN(ch->mana / 2, mana / 2);

  if (move == 0 && hit == 0 && mana == 0)
  {
    send_to_char("You are unable to transfer their wounds.\n\r", ch);
    return;
  }

  act("$n lays a shimmering hand on $N, transfering lifeforce into $M.", ch, NULL, victim, TO_NOTVICT);
  act("You lay a shimmering hand on $N, and transfers lifeforce into $M.", ch, NULL, victim, TO_CHAR);
  act("$n lays a shimmering hand on you, and tranfers lifeforce into you.", ch, NULL, victim, TO_VICT);

  modify_mana(ch, -1 * mana);
  modify_move(ch, -1 * move);
  modify_hps(ch, -1 * hit);
  modify_mana(victim, mana);
  modify_move(victim, move);
  modify_hps(victim, hit);

  WAIT_STATE(ch, 24);
}

void do_phantom(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  CHAR_DATA *gch;
  ITERATOR *pIter;
  bool found = FALSE;
  const int cost = 2000;

  if (IS_NPC(ch)) return;
  if (!IS_CLASS(ch, CLASS_FAE))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (ch->pcdata->powers[DISC_FAE_ARCANE] < 5)
  {
    send_to_char("You need level 5 discipline in arcane.\n\r", ch);
    return;
  }

  if ((event = event_isset_mobile(ch, EVENT_PLAYER_PHANTOM_WAIT)) != NULL)
  {
    printf_to_char(ch, "You cannot use this power for another %s.\n\r", event_time_left(event));
    return;
  }

  if (ch->mana < cost)
  {
    printf_to_char(ch, "You need %d more mana to cast this illusion.\n\r", cost - ch->mana);
    return;
  }
  modify_mana(ch, -1 * cost);

  act("You weave a powerful illusion of nightmarish proportions.", ch, NULL, NULL, TO_CHAR);
  act("$n's body spews forth horrible creatures of nightmarish proportions.", ch, NULL, NULL, TO_ROOM);

  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (event_isset_mobile(gch, EVENT_PLAYER_PHANTOM))
      continue;

    if (gch->fighting == ch)
    {
      event             =  alloc_event();
      event->argument   =  str_dup("first");
      event->fun        =  &event_player_phantom;
      event->type       =  EVENT_PLAYER_PHANTOM;
      add_event_char(event, gch, 2 * PULSE_PER_SECOND);

      send_to_char("You are panic stricken.\n\r", gch);
      act("$N is panic struck.", ch, NULL, gch, TO_CHAR);
      found = TRUE;
    }
  }

  if (!found)
  {
    send_to_char("Noone seems to be affected.\n\r", ch);
  }

  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_PLAYER_PHANTOM_WAIT;
  add_event_char(event, ch, 8 * PULSE_PER_SECOND);

  WAIT_STATE(ch, 6);
}

bool event_player_phantom(EVENT_DATA *event)
{
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_phantom: no owner.", 0);
    return FALSE;
  }
  if (!str_cmp(event->argument, "first"))
  {
    if (number_percent() > 80)
    {
      act("$n seems to calm down.", ch, NULL, NULL, TO_ROOM);
      send_to_char("You calm yourself down.\n\r", ch);
    }
    else
    {
      EVENT_DATA *newevent;

      /* create event and attach */
      newevent            =  alloc_event();
      newevent->argument  =  str_dup("second");
      newevent->fun       =  &event_player_phantom;
      newevent->type      =  EVENT_PLAYER_PHANTOM;
      add_event_char(newevent, ch, 4 * PULSE_PER_SECOND);
    }
  }

  if (ch->position == POS_FIGHTING)
    do_flee(ch, "");

  return FALSE;
}
