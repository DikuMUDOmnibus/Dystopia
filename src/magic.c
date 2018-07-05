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
void  say_spell     ( CHAR_DATA * ch, int sn );
void  improve_spl   ( CHAR_DATA * ch, int dtype, int sn );

/*
 * Improve ability at a certain spell type.  KaVir.
 */
void improve_spl(CHAR_DATA * ch, int dtype, int sn)
{
  char buf[MAX_INPUT_LENGTH];
  char bufskill[MAX_INPUT_LENGTH];
  char buftype[MAX_INPUT_LENGTH];
  int dice1;
  int dice2;
  int max_level = 200;

  dice1 = number_percent();
  dice2 = number_percent();

  if (IS_NPC(ch))
    return;

  if (dtype == 0)
    sprintf(buftype, "purple");
  else if (dtype == 1)
    sprintf(buftype, "red");
  else if (dtype == 2)
    sprintf(buftype, "blue");
  else if (dtype == 3)
    sprintf(buftype, "green");
  else if (dtype == 4)
    sprintf(buftype, "yellow");
  else
    return;

  if (IS_CLASS(ch, CLASS_WARLOCK))
  {
    if (ch->pcdata->powers[WARLOCK_RANK] == WLCK_RNK_WARLOCK)
      max_level = 220;
    else if (ch->pcdata->powers[WARLOCK_RANK] == WLCK_RNK_ARCHMAGE)
      max_level = 240;
  }

  if (IS_CLASS(ch, CLASS_GIANT))
  {
    if (dtype == YELLOW_MAGIC)
    {
      if (IS_SET(ch->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_TORTOISE) ||
          IS_SET(ch->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_KABALISTIC) ||
          IS_SET(ch->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_SPECTRAL))
        max_level = 220;
    }
    else if (dtype == RED_MAGIC)
    {
      if (IS_SET(ch->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_TORTOISE) ||
          IS_SET(ch->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_KABALISTIC) ||
          IS_SET(ch->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_VOODOO))
        max_level = 220;
    }
    else if (dtype == BLUE_MAGIC)
    {
      if (IS_SET(ch->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_TORTOISE) ||
          IS_SET(ch->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_SPECTRAL) ||
          IS_SET(ch->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_VOODOO))
        max_level = 220;
    }
  }

  if (ch->generation == 2)
    max_level += max_level * 0.06;
  else if (ch->generation == 1)
    max_level += max_level * 0.12;

  if (ch->spl[dtype] >= max_level)
    return;

  if ((dice1 > ch->spl[dtype] || dice2 > ch->spl[dtype]) || (dice1 == 100 || dice2 == 100))
    ch->spl[dtype] += 1;
  else
    return;

  if (ch->spl[dtype] == 1)
    sprintf(bufskill, "an apprentice of");
  else if (ch->spl[dtype] == 26)
    sprintf(bufskill, "a student at");
  else if (ch->spl[dtype] == 51)
    sprintf(bufskill, "a scholar at");
  else if (ch->spl[dtype] == 76)
    sprintf(bufskill, "a magus at");
  else if (ch->spl[dtype] == 101)
    sprintf(bufskill, "an adept at");
  else if (ch->spl[dtype] == 126)
    sprintf(bufskill, "a mage at");
  else if (ch->spl[dtype] == 151)
    sprintf(bufskill, "a wizard at");
  else if (ch->spl[dtype] == 176)
    sprintf(bufskill, "a master wizard at");
  else if (ch->spl[dtype] == 200)
    sprintf(bufskill, "a grand sorcerer at");
  else if (ch->spl[dtype] == 220)
    sprintf(bufskill, "a warlock at");
  else if (ch->spl[dtype] == 240)
    sprintf(bufskill, "an archmage at");
  else
    return;
  sprintf(buf, "#GYou are now %s %s magic.#n\n\r", bufskill, buftype);
  send_to_char(buf, ch);
}

/*
 * Lookup a skill by name.
 */
int skill_lookup(const char *name)
{
  int sn;

  for (sn = 0; sn < MAX_SKILL; sn++)
  {
    if (skill_table[sn].name == NULL)
      break;
    if (LOWER(name[0]) == LOWER(skill_table[sn].name[0]) && !str_prefix(name, skill_table[sn].name))
      return sn;
  }

  return -1;
}

/*
 * Utter mystical words for an sn.
 */
void say_spell(CHAR_DATA * ch, int sn)
{
  if (skill_table[sn].target == 0)
  {
    act("#p$n's eyes glow bright purple for a moment.#n", ch, NULL, NULL, TO_ROOM);
    act("#pYour eyes glow bright purple for a moment.#n", ch, NULL, NULL, TO_CHAR);
  }
  else if (skill_table[sn].target == 1)
  {
    act("#R$n's eyes glow bright red for a moment.#n", ch, NULL, NULL, TO_ROOM);
    act("#RYour eyes glow bright red for a moment.#n", ch, NULL, NULL, TO_CHAR);
  }
  else if (skill_table[sn].target == 2)
  {
    act("#L$n's eyes glow bright blue for a moment.#n", ch, NULL, NULL, TO_ROOM);
    act("#LYour eyes glow bright blue for a moment.#n", ch, NULL, NULL, TO_CHAR);
  }
  else if (skill_table[sn].target == 3)
  {
    act("#G$n's eyes glow bright green for a moment.#n", ch, NULL, NULL, TO_ROOM);
    act("#GYour eyes glow bright green for a moment.#n", ch, NULL, NULL, TO_CHAR);
  }
  else if (skill_table[sn].target == 4)
  {
    act("#y$n's eyes glow bright yellow for a moment.#n", ch, NULL, NULL, TO_ROOM);
    act("#yYour eyes glow bright yellow for a moment.#n", ch, NULL, NULL, TO_CHAR);
  }
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell(int level, CHAR_DATA *victim)
{
  int vcsave = (victim->spl[0] + victim->spl[1] + victim->spl[2] + victim->spl[3] + victim->spl[4]) / 20;
  int save = 50 - level / 2 + vcsave;

  if (!IS_NPC(victim))
  {
    save += victim->saving_throw;
  }
  else
  {
    save += victim->level / 10;
  }

  save = URANGE(20, save, 80);

  return number_percent() < save;
}

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_cast(CHAR_DATA * ch, char *argument)
{
  EVENT_DATA *event;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];
  int mana, sn, count = 0, delay = 1;

  /*
   * Switched NPC's can cast spells, but others can't.
   */
  if (IS_NPC(ch) && ch->desc == NULL)
    return;

  if (!IS_NPC(ch) && IS_SET(ch->newbits, NEW_STITCHES))
  {
    do_say(ch, "Umgh gmhu umhf!");
    return;
  }

  if (!IS_NPC(ch) && event_isset_mobile(ch, EVENT_MOBILE_ACIDBLOOD))
  {
    send_to_char("You are unable to focus on spellcasting while agonized like this.\n\r", ch);
    return;
  }

  one_argument(argument, arg);

  pIter = AllocIterator(ch->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == EVENT_MOBILE_CASTING)
    {
      delay = UMAX(delay, event_pulses_left(event) + 1);
      count++;
    }
  }

  if (count >= 5)
  {
    send_to_char("You cannot stack any more spells.\n\r", ch);
    return;
  }

  if (arg[0] == '\0')
  {
    send_to_char("Cast which what where?\n\r", ch);
    return;
  }

  if ((sn = skill_lookup(arg)) < 0 || (!IS_NPC(ch) && ch->level < skill_table[sn].skill_level))
  {
    send_to_char("You can't do that.\n\r", ch);
    return;
  }

  if (!can_use_skill(ch, sn))
  {
    send_to_char("You can't do that.\n\r", ch);
    return;
  }

  if (ch->position < skill_table[sn].minimum_position)
  {
    if (ch->move < 50)
    {
      send_to_char("You can't concentrate enough.\n\r", ch);
      return;
    }
    ch->move = ch->move - 50;
  }

  mana = IS_NPC(ch) ? 0 : UMAX(skill_table[sn].min_mana, 100 / (2 + (ch->level * 12) - skill_table[sn].skill_level));

  if (!IS_NPC(ch) && ch->mana < mana)
  {
    send_to_char("You don't have enough mana.\n\r", ch);
    return;
  }
  modify_mana(ch, -1 * mana);

  if (count == 0)
  {
    sprintf(arg, "You start casting '%s'.", skill_table[sn].name);
    act(arg, ch, NULL, NULL, TO_CHAR);

    sprintf(arg, "$n starts casting '%s'.", skill_table[sn].name);
    act(arg, ch, NULL, NULL, TO_ROOM);
  }
  else
  {
    sprintf(arg, "You prepare casting '%s'.", skill_table[sn].name);
    act(arg, ch, NULL, NULL, TO_CHAR);

    sprintf(arg, "$n prepares casting '%s'.", skill_table[sn].name);
    act(arg, ch, NULL, NULL, TO_ROOM);
  }

  if (IS_CLASS(ch, CLASS_WARLOCK) && !IS_NPC(ch) && IS_SET(ch->pcdata->powers[EVOLVE_1], WARLOCK_EVOLVE_TIME))
    delay += UMAX(skill_table[sn].beats / 2, skill_table[sn].beats - 12);
  else if (IS_CLASS(ch, CLASS_GIANT) && !IS_NPC(ch) && IS_SET(ch->pcdata->powers[EVOLVE_1], GIANT_EVOLVE_SHAMAN))
    delay += UMAX(4 * skill_table[sn].beats / 7, skill_table[sn].beats - 8);
  else
    delay += skill_table[sn].beats;

  /* dragonorb artifact */
  if (!IS_NPC(ch) && IS_SET(ch->pcdata->tempflag, TEMP_DRAGONORB))
  {
    delay = UMAX(1, delay - 2);
  }

  if (IS_IMMORTAL(ch))
    delay = 1;

  event            =  alloc_event();
  event->argument  =  str_dup(argument);
  event->fun       =  &cast_spell;
  event->type      =  EVENT_MOBILE_CASTING;
  add_event_char(event, ch, delay);
}

bool cast_spell(EVENT_DATA *event)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  AFFECT_DATA *paf;
  CHAR_DATA *ch, *victim = NULL;
  ITERATOR *pIter;
  OBJ_DATA *obj = NULL;
  bool deflect_spell = FALSE, absorb_spell = FALSE;
  void *vo = NULL;
  int sn, sn2;

  target_name = one_argument(event->argument, arg1);
  one_argument(target_name, arg2);

  if ((ch = event->owner.ch) == NULL)
  {
    bug("cast_spell: no owner.", 0);
    return FALSE;
  }

  if (ch->position < POS_FIGHTING)
  {
    send_to_char("You are unable to cast any spells in your current position.\n\r", ch);
    return FALSE;
  }

  if (event->argument)
  {
    target_name = one_argument(event->argument, arg1);
    one_argument(target_name, arg2);
  }
  else
  {
    target_name[0] = '\0';
    arg1[0] = '\0';
    arg2[0] = '\0';
  }

  if ((sn = skill_lookup(arg1)) < 0)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return FALSE;
  }

  switch (skill_table[sn].target)
  {
    default:
      bug("Do_cast: bad target for sn %d.", sn);
      return FALSE;

    case TAR_IGNORE:
      break;

    case TAR_CHAR_OFFENSIVE:
      if (arg2[0] == '\0')
      {
        if ((victim = ch->fighting) == NULL)
        {
          send_to_char("Cast the spell on whom?\n\r", ch);
          return FALSE;
        }
      }
      else
      {
        if ((victim = get_char_room(ch, arg2)) == NULL)
        {
          send_to_char("They aren't here.\n\r", ch);
          return FALSE;
        }
      }

      if (ch != victim && is_safe(ch, victim))
        return FALSE;

      if (ch == victim)
        send_to_char("Cast this on yourself? Ok...\n\r", ch);

      sn2 = skill_lookup("spellguard");

      pIter = AllocIterator(victim->affected);
      while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
      {
        if (paf->type == sn2)
        {
          affect_remove(ch, paf);
          if (ch == victim)
            send_to_char("Your spellguard blocks your own spell.\n\r", ch);
          else
          {
            act("$N's spellguard blocks your spell.", ch, NULL, victim, TO_CHAR);
            act("$n's spell is blocked by your spellguard.", ch, NULL, victim, TO_VICT);
          }
          return FALSE;
        }
      }

      if (!IS_NPC(ch))
      {
        if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)
        {
          send_to_char("You can't do that on your own follower.\n\r", ch);
          return FALSE;
        }
      }

      /* absorbing and deflecting shields */
      if (ch != victim && !IS_NPC(victim) && IS_CLASS(victim, CLASS_FAE))
      {
        if (IS_SET(victim->pcdata->powers[EVOLVE_2], FAE_EVOLVE_ABSORB) && number_range(1, 3) != 2)
          absorb_spell = TRUE;
        else if (IS_SET(victim->pcdata->powers[EVOLVE_2], FAE_EVOLVE_DEFLECT) && number_range(1, 3) != 2)
        {
          /* counter spell cannot be deflected */
          if (sn != skill_lookup("counter spell"))
            deflect_spell = TRUE;
        }
      }

      update_feed(ch, victim);

      vo = (void *) victim;
      break;

    case TAR_CHAR_DEFENSIVE:
      if (arg2[0] == '\0')
        victim = ch;
      else
      {
        if ((victim = get_char_room(ch, arg2)) == NULL)
        {
          send_to_char("They aren't here.\n\r", ch);
          return FALSE;
        }
      }

      if ((IS_SET(victim->newbits, NEW_SHADOWPLANE) && !IS_SET(ch->newbits, NEW_SHADOWPLANE)) ||
          (IS_SET(ch->newbits, NEW_SHADOWPLANE) && !IS_SET(victim->newbits, NEW_SHADOWPLANE)))
      {
        send_to_char("You cannot cast spells across the planes.\n\r", ch);
        return FALSE;
      }

      vo = (void *) victim;
      break;

    case TAR_CHAR_SELF:
      if (arg2[0] != '\0' && !is_name(arg2, ch->name))
      {
        send_to_char("You cannot cast this spell on another.\n\r", ch);
        return FALSE;
      }

      vo = (void *) ch;
      break;

    case TAR_OBJ_INV:
      if (arg2[0] == '\0')
      {
        send_to_char("What should the spell be cast upon?\n\r", ch);
        return FALSE;
      }

      if ((obj = get_obj_carry(ch, arg2)) == NULL)
      {
        send_to_char("You are not carrying that.\n\r", ch);
        return FALSE;
      }

      vo = (void *) obj;
      break;
  }

  say_spell(ch, sn);

  if (!IS_NPC(ch) && number_percent() > ch->pcdata->learned[sn])
  {
    send_to_char("You lost your concentration.\n\r", ch);
    improve_spl(ch, skill_table[sn].target, sn);
  }
  else
  {
    if (absorb_spell && victim != NULL)
    {
      act("$n's spell is absorbed by $N!", ch, NULL, victim, TO_NOTVICT);
      act("Your spell is absorbed by $N!", ch, NULL, victim, TO_CHAR);
      act("You absorb $n's spell!", ch, NULL, victim, TO_VICT);
      modify_mana(victim, skill_table[sn].min_mana);
      modify_hps(victim, UMIN(25, skill_table[sn].min_mana));
    }
    else if (deflect_spell && victim != NULL)
    {
      act("$n's spell is turned back at $m!", ch, NULL, NULL, TO_ROOM);
      send_to_char("Your spell is turned back at you!\n\r", ch);
      (*skill_table[sn].spell_fun) (sn, 50, victim, (void *) ch);
    }
    else if (IS_NPC(ch))
    {
      (*skill_table[sn].spell_fun) (sn, ch->level, ch, vo);
    }
    else
    {
      (*skill_table[sn].spell_fun) (sn, (ch->spl[skill_table[sn].target] * 0.5), ch, vo);
      improve_spl(ch, skill_table[sn].target, sn);
    }
  }

  if (skill_table[sn].target == TAR_CHAR_OFFENSIVE && victim != ch && victim->master != ch)
  {
    CHAR_DATA *vch;

    pIter = AllocIterator(ch->in_room->people);
    while ((vch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (victim == vch && victim->fighting == NULL)
      {
        multi_hit(victim, ch, 1);
        break;
      }
    }
  }

  /* if casting this spell caused the char to die, we return TRUE */
  if (ch->dead)
    return TRUE;

  return FALSE;
}

/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell(int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj)
{
  bool deflect_spell = FALSE;
  bool absorb_spell = FALSE;
  void *vo;

  if (sn <= 0)
    return;

  if (IS_NPC(ch))
    return;

  if (sn >= MAX_SKILL || skill_table[sn].spell_fun == 0)
  {
    bug("Obj_cast_spell: bad sn %d.", sn);
    return;
  }

  switch (skill_table[sn].target)
  {
    default:
      bug("Obj_cast_spell: bad target for sn %d.", sn);
      return;

    case TAR_IGNORE:
      vo = NULL;
      break;

    case TAR_CHAR_OFFENSIVE:
      if (victim == NULL)
        victim = ch->fighting;

      if (victim == NULL)
      {
        send_to_char("You can't do that.\n\r", ch);
        return;
      }

      if (ch != victim && is_safe(ch, victim))
        return;

      update_feed(ch, victim);

      /* absorbing and deflecting shields */
      if (ch != victim && !IS_NPC(victim) && IS_CLASS(victim, CLASS_FAE))
      {
        if (IS_SET(victim->pcdata->powers[EVOLVE_2], FAE_EVOLVE_ABSORB) && number_range(1, 3) != 2)
          absorb_spell = TRUE;
        else if (IS_SET(victim->pcdata->powers[EVOLVE_2], FAE_EVOLVE_DEFLECT) && number_range(1, 3) != 2)
          deflect_spell = TRUE;
      }

      vo = (void *) victim;
      break;

    case TAR_CHAR_DEFENSIVE:
      if (victim == NULL)
        victim = ch;
      vo = (void *) victim;
      break;

    case TAR_CHAR_SELF:
      vo = (void *) ch;
      break;

    case TAR_OBJ_INV:
      if (obj == NULL)
      {
        send_to_char("You can't do that.\n\r", ch);
        return;
      }
      vo = (void *) obj;
      break;
  }

  target_name = "";

  if (absorb_spell && victim != NULL)
  {
    act("$n's spell is absorbed by $N!", ch, NULL, victim, TO_NOTVICT);
    act("Your spell is absorbed by $N!", ch, NULL, victim, TO_CHAR);
    act("You absorb $n's spell!", ch, NULL, victim, TO_VICT);
    modify_mana(victim, skill_table[sn].min_mana);
    modify_hps(victim, UMIN(25, skill_table[sn].min_mana));
  }
  else if (deflect_spell && victim != NULL)
  {
    act("$n's spell is turned back at $m!", ch, NULL, NULL, TO_ROOM);
    send_to_char("Your spell is turned back at you!\n\r", ch);
    (*skill_table[sn].spell_fun) (sn, 50, victim, (void *) ch);
  }   
  else
  {
    (*skill_table[sn].spell_fun) (sn, level, ch, vo);
  }

  if (skill_table[sn].target == TAR_CHAR_OFFENSIVE && victim != ch && victim->master != ch)
  {
    CHAR_DATA *vch;
    ITERATOR *pIter;

    pIter = AllocIterator(ch->in_room->people);
    while ((vch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (victim == vch && victim->fighting == NULL)
      {
        multi_hit(victim, ch, 1);
        break;
      }
    }
  }

  return;
}

/*
 * Spell functions.
 */

void spell_armor(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, sn))
    return;

  af.type = sn;
  af.duration = 24;
  af.modifier = -20;
  af.location = APPLY_AC;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  act("$n is shrouded in a suit of translucent glowing armor.", victim, NULL, NULL, TO_ROOM);
  send_to_char("You are shrouded in a suit of translucent glowing armor.\n\r", victim);

  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
}

void spell_godbless(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if (is_affected(victim, sn))
    return;

  af.type = sn;
  af.duration = 150;
  af.location = APPLY_HITROLL;
  af.modifier = 150;
  af.bitvector = 0;
  affect_to_char(victim, &af);
  
  af.location = APPLY_SAVING_SPELL;
  af.modifier = UMIN(10, level / 8);
  affect_to_char(victim, &af);

  af.location = APPLY_DAMROLL;
  af.modifier = 150;
  affect_to_char(victim, &af);

  act("$n is filled with God's Blessing.", victim, NULL, NULL, TO_ROOM);
  send_to_char("You are filled with God's Blessing.\n\r", victim);
}

void spell_ghostgauntlets(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, sn))
    return;

  af.type = sn;
  af.duration = 25;
  af.location = APPLY_HITROLL;
  af.modifier = 75;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  af.location = APPLY_SAVING_SPELL;
  af.modifier = 5;
  affect_to_char(victim, &af);

  af.location = APPLY_DAMROLL;
  af.modifier = 75;
  affect_to_char(victim, &af);

  act("A pair of translucent gauntlets appear in the air beside $n.", victim, NULL, NULL, TO_ROOM);
  send_to_char("A pair of translucent gauntlets appear in the air beside you.\n\r", victim);
}

void spell_bless(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, sn))
    return;

  af.type = sn;
  af.duration = 6 + level;
  af.location = APPLY_HITROLL;
  af.modifier = level / 8;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  af.location = APPLY_SAVING_SPELL;
  af.modifier = UMIN(10, level / 8);
  affect_to_char(victim, &af);

  act("$n is blessed.", victim, NULL, NULL, TO_ROOM);
  send_to_char("You feel righteous.\n\r", victim);

  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
}

void spell_mind_boost(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if (IS_AFFECTED(victim, AFF_MINDBOOST))
    return;

  af.type = sn;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.duration = level / 2;
  af.bitvector = AFF_MINDBOOST;
  affect_to_char(victim, &af);

  act("$n eyes flare with an inner fire.", victim, NULL, NULL, TO_ROOM);
  send_to_char("Your mind feels sharp and ready.\n\r", victim);
}

void spell_mind_blank(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED(victim, AFF_MINDBLANK))
  {
    if (ch != victim)
      send_to_char("They are already affected.\n\r", ch);
    else
      send_to_char("You are already affected.\n\r", ch);
    return;
  }

  if (saves_spell(level, victim) && saves_spell(level, victim))
  {
    if (ch != victim)
      send_to_char("They resist the spell.\n\r", ch);
    else
      send_to_char("Nothing happens.\n\r", ch);
    return;
  }

  af.type = sn;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.duration = level / 2;
  af.bitvector = AFF_MINDBLANK;
  affect_to_char(victim, &af);

  act("$n starts to drool.", victim, NULL, NULL, TO_ROOM);
  send_to_char("Your mind feels fuzzy and warm.\n\r", victim);

  update_feed(ch, victim);

  /* attack back */
  {
    CHAR_DATA *vch;
    ITERATOR *pIter;

    pIter = AllocIterator(ch->in_room->people);
    while ((vch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (victim == vch && victim->fighting == NULL)
      {
        multi_hit(victim, ch, 1);
        break;
      }
    }
  }
}

void spell_blindness(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED(victim, AFF_BLIND) || saves_spell(level, victim))
    return;

  af.type = sn;
  af.location = APPLY_HITROLL;
  af.modifier = -4;
  af.duration = 1 + level;
  af.bitvector = AFF_BLIND;
  affect_to_char(victim, &af);
  send_to_char("You are blinded!\n\r", victim);

  act("$n is blinded!", victim, NULL, NULL, TO_ROOM);
}

void spell_charm_person(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (victim == ch)
  {
    send_to_char("You like yourself even better!\n\r", ch);
    return;
  }

  if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_CHARM))
  {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  /* I don't want people charming ghosts and stuff - KaVir */
  if (IS_NPC(victim) && IS_AFFECTED(victim, AFF_ETHEREAL))
  {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if (IS_IMMORTAL(victim))
  {
    send_to_char("You cannot cast puny mortal magic on immortals!\n\r", ch);
    return;
  }

  if (IS_AFFECTED(victim, AFF_CHARM) ||
      IS_AFFECTED(ch, AFF_CHARM) ||
      level < victim->level ||
      saves_spell(level, victim))
  {
    send_to_char("Nothing happens.\n\r", ch);
    return;
  }

  if (victim->master)
    stop_follower(victim, FALSE);
  add_follower(victim, ch);

  af.type = sn;
  af.duration = number_fuzzy(level / 4);
  af.location = 0;
  af.modifier = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(victim, &af);

  act("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);

  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
}

void spell_chill_touch(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const sh_int dam_each[] = {
    9,
    10, 10, 10, 11, 11, 12, 12, 13, 13, 13,
    14, 14, 14, 15, 15, 15, 16, 16, 16, 17,
    17, 17, 18, 18, 18, 19, 19, 19, 20, 20,
    20, 21, 21, 21, 22, 22, 22, 23, 23, 23,
    24, 24, 24, 25, 25, 25, 26, 26, 26, 27,
    30, 40, 50, 60, 70, 80, 90, 100, 110, 120
  };
  AFFECT_DATA af;
  int dam;

  level = URANGE(0, level, (int) (sizeof(dam_each) / sizeof(dam_each[0])) - 1);
  dam = number_range(dam_each[level] / 2, dam_each[level] * 2);

  if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_COLD))
    dam = 0;

  if (dam > 0 && !is_affected(victim, sn) && (!saves_spell(level, victim) || IS_NPC(victim)))
  {
    af.type = sn;
    af.duration = 6;
    af.location = APPLY_STR;
    af.modifier = -1;
    af.bitvector = 0;
    affect_join(victim, &af);
  }
  else
  {
    dam /= 2;
  }

  damage(ch, victim, NULL, dam, sn);
}

void spell_cure_blindness(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (is_affected(victim, gsn_blindness))
  {
    affect_strip(victim, gsn_blindness);
    send_to_char("Your vision returns!\n\r", victim);
    if (ch != victim)
      send_to_char("Ok.\n\r", ch);
  }
}

void spell_cure_poison(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (is_affected(victim, gsn_poison))
  {
    affect_strip(victim, gsn_poison);
    act("$N looks better.", ch, NULL, victim, TO_NOTVICT);
    send_to_char("A warm feeling runs through your body.\n\r", victim);
    if (ch != victim)
      send_to_char("Ok.\n\r", ch);
  }
}

void spell_curse(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED(victim, AFF_CURSE))
  {
    act("$N is already cursed.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if (saves_spell(level, victim))
  {
    act("$N resists your curse spell.", ch, NULL, victim, TO_CHAR);
    return;
  }

  af.type = sn;
  af.duration = 4 * level;
  af.location = APPLY_HITROLL;
  af.modifier = -1;
  af.bitvector = AFF_CURSE;
  affect_to_char(victim, &af);

  af.location = APPLY_SAVING_SPELL;
  af.modifier = -5;
  affect_to_char(victim, &af);

  send_to_char("You feel unclean.\n\r", victim);

  if (ch != victim)
    send_to_char("You give them the evil eye.\n\r", ch);
}

void spell_detect_evil(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED(victim, AFF_DETECT_EVIL))
    return;

  af.type = sn;
  af.duration = level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_EVIL;
  affect_to_char(victim, &af);
  send_to_char("Your eyes tingle.\n\r", victim);

  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
}

void spell_detect_hidden(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED(victim, AFF_DETECT_HIDDEN))
    return;

  af.type = sn;
  af.duration = level;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_DETECT_HIDDEN;
  affect_to_char(victim, &af);

  send_to_char("Your awareness improves.\n\r", victim);
  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
}

void spell_detect_invis(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED(victim, AFF_DETECT_INVIS))
    return;

  af.type = sn;
  af.duration = level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_INVIS;
  affect_to_char(victim, &af);

  send_to_char("Your eyes tingle.\n\r", victim);
  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
}

void spell_detect_magic(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED(victim, AFF_DETECT_MAGIC))
    return;

  af.type = sn;
  af.duration = level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_MAGIC;
  affect_to_char(victim, &af);
  send_to_char("Your eyes tingle.\n\r", victim);

  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
}

void spell_detect_poison(int sn, int level, CHAR_DATA * ch, void *vo)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;

  if (obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD)
  {
    if (obj->value[3] != 0)
      send_to_char("You smell poisonous fumes.\n\r", ch);
    else
      send_to_char("It looks very delicious.\n\r", ch);
  }
  else
  {
    send_to_char("It doesn't look poisoned.\n\r", ch);
  }
}

void spell_counter_spell(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  EVENT_DATA *event, *event_dispel = NULL;
  ITERATOR *pIter;
  char spell[MAX_INPUT_LENGTH];
  int tl = 999, i, sn2;

  if (ch == victim)
  {
    send_to_char("You cannot counter your own spells.\n\r", ch);
    return;
  }

  /* check to see if victim is casting */
  pIter = AllocIterator(victim->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == EVENT_MOBILE_CASTING)
    {
      if ((i = event_pulses_left(event)) < tl)
      {
        event_dispel = event;
        tl = i;
      }
    }
  }

  if (event_dispel == NULL)
  {
    send_to_char("They are not casting any spells.\n\r", ch);
    return;
  }

  if (saves_spell(level, victim))
  {
    send_to_char("Your counter spell failed.\n\r", ch);
    return;
  }

  one_argument(event_dispel->argument, spell);
  if ((sn2 = skill_lookup(spell)) < 0)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }

  dequeue_event(event_dispel, TRUE);

  sprintf(spell, "$n counters your '%s' spell.", skill_table[sn2].name);
  act(spell, ch, NULL, victim, TO_VICT);
  sprintf(spell, "You counter $N's '%s' spell.", skill_table[sn2].name);
  act(spell, ch, NULL, victim, TO_CHAR);
  sprintf(spell, "$n counters $N's '%s' spell.", skill_table[sn2].name);
  act(spell, ch, NULL, victim, TO_NOTVICT);
}

void spell_dispel_evil(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if (!IS_NPC(ch) && IS_EVIL(ch))
    victim = ch;

  if (IS_GOOD(victim))
  {
    act("God protects $N.", ch, NULL, victim, TO_ROOM);
    return;
  }

  if (IS_NEUTRAL(victim))
  {
    act("$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);
    return;
  }

  dam = dice(level, 8);

  if (saves_spell(level, victim))
    dam /= 2;

  damage(ch, victim, NULL, dam, sn);
}

void spell_dispel_magic(int sn, int level, CHAR_DATA * ch, void *vo)
{
  AFFECT_DATA *paf;
  EVENT_DATA *event;
  ITERATOR *pIter;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  bool found = FALSE;

  if (ch == victim)
  {
    act("$n attempts to dispel all magical affects on $mself.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You attempt to dispel all magical affects on yourself.\n\r", ch);
  }
  else
  {
    act("$n attempts to dispel all magical affects on $N.", ch, NULL, victim, TO_NOTVICT);
    act("$n attempts to dispel all magical affects on you.", ch, NULL, victim, TO_VICT);
    act("You attempt to dispel all magical affects on $N.", ch, NULL, victim, TO_CHAR);
  }

  pIter = AllocIterator(ch->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    switch(event->type)
    {
      default:
        break;
      case EVENT_PLAYER_HEATMETAL:
        if (victim == ch || !saves_spell(level, victim))
        {
          found = TRUE;
          dequeue_event(event, TRUE);
          act("$n's equipment stops burning.", victim, NULL, NULL, TO_ROOM);
          send_to_char("Your equipment stops burning.\n\r", victim);
        }
        break;
      case EVENT_MOBILE_CONFUSED:
        if (victim == ch || !saves_spell(level, victim))
        {
          found = TRUE;
          dequeue_event(event, TRUE);
          act("$n seems to regain focus.", victim, NULL, NULL, TO_ROOM);
          send_to_char("You regain focus.\n\r", victim);
        }
        break;
      case EVENT_MOBILE_ACIDBLOOD:
        if (victim == ch || !saves_spell(level, victim))
        {
          found = TRUE;
          dequeue_event(event, TRUE);
          act("$n no longer seems as painwrecked as before.", victim, NULL, NULL, TO_ROOM);
          send_to_char("Your blood looses its acid properties.\n\r", victim);
        }
        break;
    }
  }

  pIter = AllocIterator(victim->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (victim == ch || !saves_spell(level, victim))
    {
      if (paf->type > 0 && paf->type < MAX_SKILL && skill_table[paf->type].msg_off)
      {
        act(skill_table[paf->type].msg_off, victim, NULL, NULL, TO_CHAR);
        act(skill_table[paf->type].msg_off_others, victim, NULL, NULL, TO_ROOM);
      }
      found = TRUE;
      affect_remove(victim, paf);
    }
  }

  if (!found)
    send_to_char("Nothing happens.\n\r", ch);
  else if (ch != victim)
    send_to_char("Your spell seems to have some affect.\n\r", ch);
}

void spell_earthquake(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *vch;
  ITERATOR *pIter;
  DESCRIPTOR_DATA *d;

  if (ch->in_room == NULL) return;

  send_to_char("The earth trembles beneath your feet!\n\r", ch);
  act("$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);

  pIter = AllocIterator(ch->in_room->people);
  while ((vch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!can_see(ch, vch) || IS_AFFECTED(vch, AFF_FLYING))
      continue;

    if (vch != ch)
      damage(ch, vch, NULL, dice(level, 5), sn);
  }

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->character && d->connected == CON_PLAYING && d->character->in_room &&
        d->character->in_room->area == ch->in_room->area &&
        d->character->in_room != ch->in_room)
    {
      send_to_char("The earth shivers and trembles.\n\r", d->character);
    }
  }
}

void spell_enchant_weapon(int sn, int level, CHAR_DATA * ch, void *vo)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA *paf;

  if (obj->item_type != ITEM_WEAPON || IS_SET(obj->quest, QUEST_ENCHANTED) || IS_SET(obj->quest, QUEST_ARTIFACT))
  {
    send_to_char("You are unable to enchant this weapon.\n\r", ch);
    return;
  }

  if ((paf = (AFFECT_DATA *) PopStack(affect_free)) == NULL)
    paf = malloc(sizeof(*paf));

  paf->type = sn;
  paf->duration = -1;
  paf->location = APPLY_HITROLL;
  paf->modifier = level / 5;
  paf->bitvector = 0;
  AttachToList(paf, obj->affected);

  if ((paf = (AFFECT_DATA *) PopStack(affect_free)) == NULL)
    paf = malloc(sizeof(*paf));

  paf->type = sn;
  paf->duration = -1;
  paf->location = APPLY_DAMROLL;
  paf->modifier = level / 10;
  paf->bitvector = 0;
  AttachToList(paf, obj->affected);

  if (IS_GOOD(ch))
  {
    SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
    REMOVE_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);
    REMOVE_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
    SET_BIT(obj->quest, QUEST_ENCHANTED);
    act("$p glows blue.", ch, obj, NULL, TO_CHAR);
    act("$p glows blue.", ch, obj, NULL, TO_ROOM);
  }
  else if (IS_EVIL(ch))
  {
    SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
    REMOVE_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);
    REMOVE_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
    SET_BIT(obj->quest, QUEST_ENCHANTED);
    act("$p glows red.", ch, obj, NULL, TO_CHAR);
    act("$p glows red.", ch, obj, NULL, TO_ROOM);
  }
  else
  {
    SET_BIT(obj->quest, QUEST_ENCHANTED);
    act("$p glows yellow.", ch, obj, NULL, TO_CHAR);
    act("$p glows yellow.", ch, obj, NULL, TO_ROOM);
  }
}

/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_energy_drain(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if (saves_spell(level, victim))
    return;

  dam = dice(10, level);

  if (IS_IMMUNE(victim, IMM_DRAIN))
    dam = 0;
  else
  {
    modify_mana(victim, victim->mana / 2);
    modify_move(victim, victim->move / 2);
    modify_hps(ch, dam);
  }

  damage(ch, victim, NULL, dam, sn);
}

void spell_desanct(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  send_to_char("#GDESANCT!!!! #yMuhahahaha.\n\r#n", ch);

  if (IS_AFFECTED(victim, AFF_SANCTUARY))
  {
    REMOVE_BIT(victim->affected_by, AFF_SANCTUARY);

    act("The white aura around $n fades.", victim, NULL, NULL, TO_ROOM);
    send_to_char("The white aura around your body fades.\n\r", victim);
  }
}

void spell_imp_heal(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int irandom = number_range(1000, 1500);

  modify_hps(victim, irandom);
  update_pos(victim);

  send_to_char("A warm feeling fills your body.\n\r", victim);

  if (ch == victim)
    act("$n heals $mself.", ch, NULL, NULL, TO_ROOM);
  else
    act("$n heals $N.", ch, NULL, victim, TO_NOTVICT);

  if (ch != victim)
    send_to_char("#GSUPER HEAL!!!.\n\r#n", ch);
}

void spell_imp_fireball(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = number_range(1500, 3000);

  if (saves_spell(level, victim))
    dam /= 2;

  if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_HEAT))
    dam /= 2;

  /* it is rank restricted in PK */
  if (!IS_NPC(ch) && !IS_NPC(victim))
  {
    int maxdam;

    if (victim->max_hit >= 40000)
      maxdam = 1500;
    else if (victim->max_hit >= 35000)
      maxdam = 1250;
    else if (victim->max_hit >= 30000)
      maxdam = 1000;
    else if (victim->max_hit >= 25000)
      maxdam = 750;
    else
      maxdam = 500;

    if (dam > maxdam * 1.1)
      dam = number_range(0.9 * maxdam, 1.1 * maxdam);
  }

  damage(ch, victim, NULL, dam, sn);
}

void spell_imp_teleport(int sn, int level, CHAR_DATA * ch, void *vo)
{
  ROOM_INDEX_DATA *pRoom;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *mount;

  if (victim->in_room == NULL
      || IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
      || IS_SET(victim->in_room->room_flags, ROOM_SAFE)
      || (victim != ch && (saves_spell(level, victim)
      || saves_spell(level, victim))))
  {
    send_to_char("You failed the #GTELEPORT#n spell.\n\r", ch);
    return;
  }

  if ((pRoom = get_rand_room_area(victim->in_room->area)) == NULL)
  {
    send_to_char("You failed the #GTELEPORT#n spell.\n\r", ch);
    return;
  }

  act("$n slowly fades out of existence.", victim, NULL, NULL, TO_ROOM);
  char_from_room(victim);

  char_to_room(victim, pRoom, TRUE);
  act("$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM);

  do_look(victim, "auto");

  if ((mount = ch->mount) == NULL)
    return;

  char_from_room(mount);
  char_to_room(mount, victim->in_room, TRUE);
  do_look(mount, "auto");
}

void spell_gas_blast(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *vch;
  int count = 0;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->in_room->people);
  while ((vch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!can_see(ch, vch)) continue;
    if (is_safe(ch, vch)) continue;

    if (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch))
    {
      int dam = number_range(UMAX(10, ch->hit) / 14, UMAX(10, ch->hit) / 7);

      if (saves_spell(level, vch))
        dam /= 2;

      if (dam > ch->damcap[DAM_CAP])
        dam = number_range((ch->damcap[DAM_CAP] - 200), (ch->damcap[DAM_CAP] + 100));

      if (IS_AFFECTED(vch, AFF_SANCTUARY))
        dam /= 2;

      damage( ch, vch, NULL, dam, sn );

      if (++count >= 5)
        return;
    }
  }
}

void spell_haste(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;   
  AFFECT_DATA af;

  if (is_affected(victim, sn))
  {
    send_to_char("Nothing happens.\n\r", ch);

    if (victim != ch)
      send_to_char("Nothing happens.\n\r", victim);
    return;
  }

  af.type      = sn;
  af.duration  = 15;
  af.location  = APPLY_HITROLL;
  af.modifier  = 4;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  if (!IS_NPC(ch) && IS_CLASS(ch, CLASS_GIANT) &&
     (IS_SET(ch->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_SPECTRAL) ||
      IS_SET(ch->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_VOODOO)))
    affect_to_char(victim, &af);

  if (ch != victim)
    send_to_char("You cast your spell on them.\n\r", ch);

  act("$n starts moving very fast.", victim, NULL, NULL, TO_ROOM);
  send_to_char("Your actions starts to speed up.\n\r", victim);
}

void spell_slow(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if (is_affected(victim, sn))
  {
    send_to_char("Nothing happens.\n\r", ch);
    if (victim != ch)
      send_to_char("Nothing happens.\n\r", victim);
    return;
  }

  af.type      = sn;
  af.duration  = 15;
  af.location  = APPLY_HITROLL;
  af.modifier  = -4;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  if (!IS_NPC(ch) && IS_CLASS(ch, CLASS_GIANT) &&
     (IS_SET(ch->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_TORTOISE) ||
      IS_SET(ch->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_KABALISTIC)))
    affect_to_char(victim, &af);

  if (ch != victim)
    send_to_char("You cast your spell on them.\n\r", ch);

  act("$n slows down.", victim, NULL, NULL, TO_ROOM);
  send_to_char("Your actions starts to slow down.\n\r", victim);
}

void spell_nerve_pinch(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;  

  if (is_affected(victim, sn)) return;

  af.type      = sn;
  af.duration  = 15;
  af.location  = APPLY_HITROLL;
  af.modifier  = -4;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  send_to_char("You strike their central nerve system.\n\r", ch);

  act("$n chokes and spasms.", victim, NULL, NULL, TO_ROOM);
  act("As $n hits you, you start to choke and spasm.", ch, NULL, victim, TO_VICT);
}

void spell_spellguard(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, sn))
  {
    if (ch == victim)
      send_to_char("You are already spellguarded.\n\r", ch);
    else
      send_to_char("They are already spellguarded.\n\r", ch);

    return;
  }

  af.type      = sn;
  af.duration  = 15;
  af.location  = APPLY_SAVING_SPELL;
  af.modifier  = 4;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  act("$n is spellguarded.", victim, NULL, NULL, TO_ROOM);
  send_to_char("You are guarded from spells.\n\r", victim);

  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
}

bool event_mobile_cantrip_heal(EVENT_DATA *event) 
{
  CHAR_DATA *ch;
  char buf[MAX_INPUT_LENGTH];
  int num;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_cantrip_heal: no owner.", 0);
    return FALSE;
  }

  switch(number_range(1, 3))
  {
    default:
      if (ch->hit < ch->max_hit)
      {
        ch->hit = ch->hit + number_range(50, 150);
        if (ch->hit > ch->max_hit) ch->hit = ch->max_hit;
        act("A #Lblue aura#n flashes around $n.", ch, NULL, NULL, TO_ROOM);
        send_to_char("A #Lblue aura#n flashes around you.\n\r", ch);
      }
      break;
    case 2:
      if (ch->move < ch->max_move)
      {
        ch->move = ch->move + number_range(150, 2150);
        if (ch->move > ch->max_move) ch->move = ch->max_move;
        act("A #Ggreen aura#n flashes around $n.", ch, NULL, NULL, TO_ROOM);
        send_to_char("A #Ggreen aura#n flashes around you.\n\r", ch);
      }
      break;
    case 3:
      if (ch->mana < ch->max_mana)
      {
        ch->mana = ch->mana + number_range(150, 250);
        if (ch->mana > ch->max_mana) ch->mana = ch->max_mana;
        act("A #Rred aura#n flashes around $n.", ch, NULL, NULL, TO_ROOM);
        send_to_char("A #Rred aura#n flashes around you.\n\r", ch);
      }
      break;
  }

  if ((num = (event->argument != NULL) ? atoi(event->argument) : 0) > 0)
  {
    sprintf(buf, "%d", num - 1);

    event = alloc_event();
    event->fun = &event_mobile_cantrip_heal;
    event->type = EVENT_MOBILE_CANTRIP_HEAL;
    event->argument = str_dup(buf);
    add_event_char(event, ch, 1 * PULSE_PER_SECOND);
  }

  return FALSE; 
}

void spell_cantrip(int sn, int level, CHAR_DATA *ch, void *vo)
{
  EVENT_DATA *event;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  bool found = FALSE;

  switch(number_range(1, 6))
  {
    default:
      send_to_char("Your cantrip spell fizzles.\n\r", ch);
      break;
    case 1:
      if (event_isset_mobile(victim, EVENT_MOBILE_CANTRIP_HEAL))
      {
        if (ch != victim)
          act("$N is already affected by this cantrip effect.", ch, NULL, victim, TO_CHAR);
        else
          send_to_char("You are already affected by this cantrip effect.\n\r", ch);
        return;
      }

      event = alloc_event();
      event->fun = &event_mobile_cantrip_heal;
      event->type = EVENT_MOBILE_CANTRIP_HEAL;
      event->argument = str_dup("5");
      add_event_char(event, victim, 1 * PULSE_PER_SECOND);

      if (ch != victim)
      {
        act("$N is affected by your cantrip spell.", ch, NULL, victim, TO_CHAR);
        act("$n casts some sort of spell on $N.", ch, NULL, victim, TO_NOTVICT);
        act("$n casts some sort of spell on you.", ch, NULL, victim, TO_VICT);
      }
      else
      {
        send_to_char("Your cantrip spell takes affect.\n\r", ch);
        act("$n casts some sort of spell on $mself.", ch, NULL, NULL, TO_ROOM);
      }
      break;
    case 2:
      if (is_affected(victim, sn))
      {
        if (ch == victim)
          send_to_char("You are already affected by a cantrip spell.\n\r", ch);
        else
          act("$N is already affected by a cantrip spell.", ch, NULL, victim, TO_CHAR);
        return;
      }

      switch(number_range(1, 5))
      {
        default:
          af.location = APPLY_HITROLL;
          break;
        case 3:
        case 4:
          af.location = APPLY_DAMROLL;
          break;
        case 5:
          af.location = APPLY_SAVING_SPELL;
          break;
      }

      af.type = sn;
      af.duration = number_range(2, 4);
      af.modifier = number_range(10, 20);
      af.bitvector = 0;
      affect_to_char(victim, &af);

      if (ch == victim)
      {
        act("$n casts some sort of spell on $mself.", ch, NULL, NULL, TO_ROOM);
        act("You cast a cantrip spell on yourself.", ch, NULL, NULL, TO_CHAR);
      }
      else
      {
        act("You cast a cantrip spell on $N.", ch, NULL, victim, TO_CHAR);
        act("$n casts some sort of spell on $N.", ch, NULL, victim, TO_NOTVICT);
        act("$n casts some sort of spell on you.", ch, NULL, victim, TO_VICT);
      }
      break;
    case 3:
      if (event_isset_mobile(victim, EVENT_MOBILE_CANTRIP_SPIKES))
      {
        if (ch == victim)
          send_to_char("You are already covered in spikes.\n\r", ch);
        else
          act("$N is already covered in spikes.", ch, NULL, victim, TO_CHAR);
        return;
      }

      event = alloc_event();
      event->fun = &event_dummy;
      event->type = EVENT_MOBILE_CANTRIP_SPIKES;
      add_event_char(event, victim, 6 * PULSE_PER_SECOND);

      act("Large, dangerous looking spikes grows from $n's body.", victim, NULL, NULL, TO_ROOM);
      send_to_char("Large, dangerous looking spikes grows from your body.\n\r", victim);
      break;
    case 4:
      if (!IS_SET(victim->affected_by, AFF_SANCTUARY))
      {
        SET_BIT(victim->affected_by, AFF_SANCTUARY);
        act("$n is affected by a sanctuary spell.", victim, NULL, NULL, TO_ROOM);
        send_to_char("You are affected by a sanctuary spell.\n\r", victim);
        found = TRUE;
      }
      if (!IS_SET(victim->affected_by, AFF_FLYING))
      {
        SET_BIT(victim->affected_by, AFF_FLYING);
        act("$n is affected by a fly spell.", victim, NULL, NULL, TO_ROOM);
        send_to_char("You are affected by a fly spell.\n\r", victim);
        found = TRUE;
      }
      if (!IS_SET(victim->act, PLR_HOLYLIGHT))
      {
        SET_BIT(victim->act, PLR_HOLYLIGHT);
        act("$n is granted superior vision.", victim, NULL, NULL, TO_ROOM);
        send_to_char("You are granted superior vision.\n\r", victim);
        found = TRUE;
      }
      if (IS_GOOD(victim) && !IS_SET(victim->affected_by, AFF_PROTECT))
      {
        SET_BIT(victim->affected_by, AFF_PROTECT);
        act("$n is protected from evil.", victim, NULL, NULL, TO_ROOM);
        send_to_char("You are protected from evil.\n\r", victim);
        found = TRUE;
      }
      if (IS_EVIL(victim) && !IS_SET(victim->affected_by, AFF_PROTECT_GOOD))
      {
        SET_BIT(victim->affected_by, AFF_PROTECT_GOOD);
        act("$n is protected from good.", victim, NULL, NULL, TO_ROOM);
        send_to_char("You are protected from good.\n\r", victim);
        found = TRUE;
      }

      if (!found)
        send_to_char("Nothing happens.\n\r", ch);

      break;
    case 5:
      if (ch->in_room == NULL)
        return;

      event = alloc_event();
      event->fun = &event_dummy;
      event->type = EVENT_ROOM_CANTRIP;
      switch(number_range(1,3))
      {
        default:
          event->argument = str_dup("The room is littered with dead frogs.\n\r");
          act("As $n completes $s spell, a shower of frogs falls from thin air.", ch, NULL, NULL, TO_ROOM);
          send_to_char("Your cantrip causes a shower of frogs to appear from thin air.\n\r", ch);
          break;
        case 2:
          event->argument = str_dup("Several large butterflies are hovering in the air.\n\r");
          act("As $n completes $s spell, several large butterflies spews from $s fingers.", ch, NULL, NULL, TO_ROOM);
          send_to_char("Your cantrip causes several large butterflies to spew from your fingers.\n\r", ch);
          break;
        case 3:
          event->argument = str_dup("The floor is covered in waisthigh grass.\n\r");
          act("As $n completes $s spell, grass grows from the floor, all the way to your waist.", ch, NULL, NULL, TO_ROOM);
          send_to_char("Your cantrip causes grass to grow from the floor to your waist.\n\r", ch);
          break;
      }
      add_event_room(event, ch->in_room, 6 * PULSE_PER_SECOND);
      break;
  }
}

bool event_mobile_rupture(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  AFFECT_DATA *paf;
  ITERATOR *pIter;
  char buf[MAX_INPUT_LENGTH];
  char *ptr;
  int level, num;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_rupture: no owner.", 0);
    return FALSE;
  }

  ptr = one_argument(event->argument, buf);
  num = atoi(buf);
  one_argument(ptr, buf);
  level = atoi(buf);

  pIter = AllocIterator(ch->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (!saves_spell(level, ch))
    {
      if (paf->type > 0 && paf->type < MAX_SKILL && skill_table[paf->type].msg_off)
      {
        act(skill_table[paf->type].msg_off, ch, NULL, NULL, TO_CHAR);
        act(skill_table[paf->type].msg_off_others, ch, NULL, NULL, TO_ROOM);
      }
      affect_remove(ch, paf);
    }
  }

  if (num > 0)
  {
    sprintf(buf, "%d %d", num - 1, level);
    event = alloc_event();
    event->fun = &event_mobile_rupture;
    event->type = EVENT_MOBILE_RUPTURE;
    event->argument = str_dup(buf);
    add_event_char(event, ch, 2 * PULSE_PER_SECOND);
  }

  return FALSE;
}

void spell_rupture(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  EVENT_DATA *event;
  char buf[MAX_INPUT_LENGTH];

  if (event_isset_mobile(victim, EVENT_MOBILE_RUPTURE))
  {
    if (ch != victim)
      act("$N is already affected by a rupture.", ch, NULL, victim, TO_CHAR);
    else
      send_to_char("You are already affected by a rupture.\n\r", ch);

    return;
  }

  if (ch != victim && saves_spell(level, victim))
  {
    act("$N resists your rupture spell.", ch, NULL, victim, TO_CHAR);
    act("You resist $n's spell.", ch, NULL, victim, TO_VICT);
    return;
  }

  sprintf(buf, "%d %d", 3, level);

  event = alloc_event();
  event->fun = &event_mobile_rupture;
  event->type = EVENT_MOBILE_RUPTURE;
  event->argument = str_dup(buf);
  add_event_char(event, victim, 1 * PULSE_PER_SECOND);

  if (ch != victim)
  {
    act("$n affects you with a rupture spell.", ch, NULL, victim, TO_VICT);
    act("Your rupture spell takes hold of $N.", ch, NULL, victim, TO_CHAR);
  }
  else
  {
    send_to_char("You affect yourself with a rupture.\n\r", ch);
    act("$n affects $mself with some sort of enchantment.", ch, NULL, NULL, TO_ROOM);
  }
}

void spell_spelltrap(int sn, int level, CHAR_DATA *ch, void *vo)
{
  EVENT_DATA *event, *event_dispel = NULL;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  ITERATOR *pIter;
  int tl = 999, i;

  if (ch == victim || saves_spell(level, victim))
  {
    send_to_char("They resist your spelltrap spell.\n\r", ch);
    return;
  }

  /* check to see if victim is casting */ 
  pIter = AllocIterator(victim->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == EVENT_MOBILE_CASTING)
    {
      if ((i = event_pulses_left(event)) < tl)
      {
        event_dispel = event;
        tl = i;
      }
    }
  }

  if (event_dispel == NULL)
  {
    act("$N is not casting any spells, your spelltrap fails.", ch, NULL, victim, TO_CHAR);
    return;
  }

  event = alloc_event();
  event->fun = &cast_spell;
  event->argument = str_dup(event_dispel->argument);
  event->type = EVENT_MOBILE_CASTING;
  add_event_char(event, victim, tl + 12);

  dequeue_event(event_dispel, TRUE);

  act("You trap $N's spell, forcing it into a spell-loop.", ch, NULL, victim, TO_CHAR);
  act("$n traps your spell, forcing it into a spell-loop.", ch, NULL, victim, TO_VICT);
}

void spell_golden_gate(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *victim, *gch;
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  ITERATOR *pIter;
  AFFECT_DATA af;

  if (!IS_OBJ_STAT(obj, ITEM_RARE) && !IS_OBJ_STAT(obj, ITEM_SENTIENT))
  {
    send_to_char("That item is not even rare - The Golden Gate spell fails.\n\r", ch);
    return;
  }

  if (obj->ownerid != 0)
  {
    send_to_char("That item is claimed - The Golden Gate spell fails.\n\r", ch);
    return;
  }

  if ((victim = ch->fighting) != NULL)
  {
    act("$n points $p at you and three beams of golden light flies from $s hand.", ch, obj, victim, TO_VICT);
    act("$n points $p at $N and three beams of golden light flies from $s hand.", ch, obj, victim, TO_NOTVICT);
    act("You point $p at $N and three beams of golden light flies from your hand.", ch, obj, victim, TO_CHAR);
  }
  else
  {
    act("$n mutters a series of arcane phrases over $p.", ch, obj, NULL, TO_ROOM);
    act("You mutter a series of arcane phrases over $p.", ch, obj, NULL, TO_CHAR);
    act("Three golden beams flies from $p, covering the room.", ch, obj, NULL, TO_ALL);
  }

  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (gch == ch)
      continue;

    if (saves_spell(level, gch) && (gch != victim || saves_spell(level, gch)))
    {
      act("$n dodges the golden beams.", gch, NULL, NULL, TO_ROOM);
      act("You dodge the golden beams.", gch, NULL, NULL, TO_CHAR);
      continue;
    }

    act("$n is struck by the golden beams and is pinned down.", gch, NULL, NULL, TO_ROOM);
    act("You are struck by the golden beams and become pinned down.", gch, NULL, NULL, TO_CHAR);

    af.type = sn;
    af.location = APPLY_AC;
    af.modifier = 200;
    af.duration = number_range(10, 20);
    af.bitvector = AFF_WEBBED;
    affect_to_char(gch, &af);
  }

  act("$p crumbles to dust.", ch, obj, NULL, TO_ALL);
  extract_obj(obj);
}


void spell_fireball(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const sh_int dam_each[] = {
    20,
    20, 20, 20, 20, 20, 25, 25, 25, 25, 25,
    30, 30, 30, 30, 30, 35, 40, 45, 50, 55,
    60, 65, 70, 75, 80, 82, 84, 86, 88, 90,
    92, 94, 96, 98, 100, 102, 104, 106, 108, 110,
    112, 114, 116, 118, 120, 122, 124, 126, 128, 130,
    150, 200, 250, 300, 400, 500, 650, 750, 850, 1000
  };
  int dam;

  level = URANGE(0, level, (int) (sizeof(dam_each) / sizeof(dam_each[0])) - 1);
  dam = number_range(dam_each[level] / 2, dam_each[level] * 2);

  if (saves_spell(level, victim))
    dam /= 2;

  if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_HEAT))
    dam = 0;

  damage(ch, victim, NULL, dam, sn);
}

void spell_faerie_fog(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *ich;
  ITERATOR *pIter;

  act("$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM);
  send_to_char("You conjure a cloud of purple smoke.\n\r", ch);

  pIter = AllocIterator(ch->in_room->people);
  while ((ich = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (ich == ch || (!IS_NPC(ich) && ich->level > 6))
      continue;

    affect_strip(ich, gsn_invis);
    affect_strip(ich, gsn_mass_invis);
    affect_strip(ich, gsn_sneak);

    REMOVE_BIT(ich->affected_by, AFF_INVISIBLE);
    REMOVE_BIT(ich->affected_by, AFF_SNEAK);
    REMOVE_BIT(ich->act, PLR_HIDE);

    if (IS_SET(ich->affected_by, AFF_ETHEREAL))
    {
      /* not fighting, or both participants fail their save */
      if (!saves_spell(level, ich) && (ich->fighting == NULL || !saves_spell(level, ich->fighting)))
      {
        REMOVE_BIT(ich->affected_by, AFF_ETHEREAL);

        if (ich->fighting)
          REMOVE_BIT(ich->fighting->affected_by, AFF_ETHEREAL);
      }
    }

    if (IS_SET(ich->newbits, NEW_SHADOWPLANE))
    {
      /* not fighting, or both participants fail their save */
      if (!saves_spell(level, ich) && (ich->fighting == NULL || !saves_spell(level, ich->fighting)))
      {
        REMOVE_BIT(ich->newbits, NEW_SHADOWPLANE);

        if (ich->fighting)
          REMOVE_BIT(ich->fighting->newbits, NEW_SHADOWPLANE);
      }
    }

    act("$n is revealed!", ich, NULL, NULL, TO_ROOM);
    send_to_char("You are revealed!\n\r", ich);
  }

  REMOVE_BIT(ch->in_room->room_flags, ROOM_TOTAL_DARKNESS);
}

void spell_fly(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED(victim, AFF_FLYING))
    return;

  af.type = sn;
  af.duration = level + 3;
  af.location = 0;
  af.modifier = 0;
  af.bitvector = AFF_FLYING;
  affect_to_char(victim, &af);

  send_to_char("You rise up off the ground.\n\r", victim);
  act("$n rises up off the ground.", victim, NULL, NULL, TO_ROOM);
}

void spell_endurance(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af, *paf;
  ITERATOR *pIter;

  pIter = AllocIterator(victim->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->type == sn)
      break;
  }

  if (paf != NULL)
  {
    affect_modify(victim, paf, FALSE);
    paf->duration = UMAX(paf->duration, 50);
    paf->modifier += number_range(2, 5);
    affect_modify(victim, paf, TRUE);
  }
  else
  {
    af.type = sn;
    af.duration = 50;
    af.location = APPLY_CON;
    af.modifier = number_range(2, 5);
    af.bitvector = 0;
    affect_to_char(victim, &af);
  }

  act("$n's muscles bulge with enhanced endurance.", victim, NULL, NULL, TO_ROOM);
  act("Your muscles bulge with enhanced endurance.", victim, NULL, NULL, TO_CHAR);
}

void spell_heroism(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af, *paf;
  ITERATOR *pIter;

  pIter = AllocIterator(victim->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->type == sn)
      break;
  }

  if (paf != NULL)
  {
    affect_modify(victim, paf, FALSE);
    paf->duration = UMAX(paf->duration, 50);
    paf->modifier += number_range(2, 5);
    affect_modify(victim, paf, TRUE);
  }
  else
  {
    af.type = sn;
    af.duration = 50;
    af.location = APPLY_STR;
    af.modifier = number_range(2, 5);
    af.bitvector = 0;
    affect_to_char(victim, &af);
  }

  act("$n emits and aura of heroic proportions.", victim, NULL, NULL, TO_ROOM);
  act("You feel suddenly heroic beyond normal.", victim, NULL, NULL, TO_CHAR);
}

void spell_nimbleness(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af, *paf;
  ITERATOR *pIter;

  pIter = AllocIterator(victim->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->type == sn)
      break;
  }

  if (paf != NULL)
  {
    affect_modify(victim, paf, FALSE);
    paf->duration = UMAX(paf->duration, 50);
    paf->modifier += number_range(2, 5);
    affect_modify(victim, paf, TRUE);
  }
  else
  {
    af.type = sn;
    af.duration = 50;
    af.location = APPLY_DEX;
    af.modifier = number_range(2, 5);
    af.bitvector = 0;
    affect_to_char(victim, &af);
  }

  act("$n starts slithering around like a snake.", victim, NULL, NULL, TO_ROOM);
  act("You feel extremely nimble and dexterious.", victim, NULL, NULL, TO_CHAR);
}

void spell_omniscience(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af, *paf;
  ITERATOR *pIter;

  pIter = AllocIterator(victim->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->type == sn)
      break;
  }

  if (paf != NULL)
  {
    affect_modify(victim, paf, FALSE);
    paf->duration = UMAX(paf->duration, 50);
    paf->modifier += number_range(2, 5);
    affect_modify(victim, paf, TRUE);
  }
  else
  {
    af.type = sn;
    af.duration = 50;
    af.location = APPLY_WIS;
    af.modifier = number_range(2, 5);
    af.bitvector = 0;
    affect_to_char(victim, &af);
  }

  act("$n starts emitting immense wisdom.", victim, NULL, NULL, TO_ROOM);
  act("You feel wise beyond your age.", victim, NULL, NULL, TO_CHAR);
}

void spell_brilliance(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af, *paf;
  ITERATOR *pIter;

  pIter = AllocIterator(victim->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->type == sn)
      break;
  }

  if (paf != NULL)
  {
    affect_modify(victim, paf, FALSE);
    paf->duration = UMAX(paf->duration, 50);
    paf->modifier += number_range(2, 5);
    affect_modify(victim, paf, TRUE);
  }
  else
  {
    af.type = sn;
    af.duration = 50;
    af.location = APPLY_INT;
    af.modifier = number_range(2, 5);
    af.bitvector = 0;
    affect_to_char(victim, &af);
  }

  act("$n's eyes start shining with an inner light.", victim, NULL, NULL, TO_ROOM);
  act("You feel like solving some equations.", victim, NULL, NULL, TO_CHAR);
}

void spell_giant_strength(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, sn))
    return;

  af.type = sn;
  af.duration = level;
  af.location = APPLY_STR;
  af.modifier = 1 + (level >= 18) + (level >= 25);
  af.bitvector = 0;
  affect_to_char(victim, &af);

  send_to_char("You feel stronger.\n\r", victim);

  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
}

void spell_harm(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = UMAX(20, victim->hit - dice(1, 4));

  if (saves_spell(level, victim))
    dam = UMIN(50, dam / 4);

  dam = UMIN(100, dam);

  damage(ch, victim, NULL, dam, sn);
}

void spell_group_heal(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *ich;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->in_room->people);
  while ((ich = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (is_same_group(ich, ch))
    {
      modify_hps(ich, number_range(150, 250));
      send_to_char("You feel healed.\n\r", ich);
    }
  }
}

void spell_heal(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (!IS_NPC(victim) && ch->level < 3 && victim->level > 2)
  {
    send_to_char("You don't want to do that.\n\r", ch);
    return;
  }

  victim->hit = UMIN(victim->hit + 100, victim->max_hit);

  if (IS_NPC(victim) && victim->hit >= victim->max_hit)
    victim->hit = victim->max_hit - 100;

  update_pos(victim);
  send_to_char("A warm feeling fills your body.\n\r", victim);

  if (ch == victim)
    act("$n heals $mself.", ch, NULL, NULL, TO_ROOM);
  else
    act("$n heals $N.", ch, NULL, victim, TO_NOTVICT);
}

void spell_newbie(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (getMight(victim) >= RANK_CADET)
  {
    send_to_char("This spell will only work for newbies.\n\r", ch);
    return;
  }
  act("$n fades away.", victim, NULL, NULL, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, get_room_index(ROOM_VNUM_SCHOOL), TRUE);
  act("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
  send_to_char("You zap back to the newbie area.\n\r", victim);
}

void spell_identify(int sn, int level, CHAR_DATA * ch, void *vo)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  BUFFER *buf, *buf2;

  buf = identify_obj(obj);

  buf2 = box_text(buf->data, "Identify");

  send_to_char("\n\r", ch);
  send_to_char(buf2->data, ch);

  buffer_free(buf);
}

void spell_infravision(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED(victim, AFF_INFRARED))
    return;
  act("$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM);
  af.type = sn;
  af.duration = 2 * level;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_INFRARED;
  affect_to_char(victim, &af);
  send_to_char("Your eyes glow red.\n\r", victim);
  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
  return;
}

void spell_invis(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED(victim, AFF_INVISIBLE))
    return;

  act("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);
  af.type = sn;
  af.duration = 24;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_INVISIBLE;
  affect_to_char(victim, &af);
  send_to_char("You fade out of existence.\n\r", victim);
  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
  return;
}

void spell_know_alignment(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  char *msg;
  int ap;

  ap = victim->alignment;

  if (ap > 700)
    msg = "$N has an aura as white as the driven snow.";
  else if (ap > 350)
    msg = "$N is of excellent moral character.";
  else if (ap > 100)
    msg = "$N is often kind and thoughtful.";
  else if (ap > -100)
    msg = "$N doesn't have a firm moral commitment.";
  else if (ap > -350)
    msg = "$N lies to $S friends.";
  else if (ap > -700)
    msg = "$N's slash DISEMBOWELS you!";
  else
    msg = "I'd rather just not say anything at all about $N.";

  act(msg, ch, NULL, victim, TO_CHAR);
  return;
}

void spell_planebind(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, sn))
    return;

  if (saves_spell(level, victim))
  {
    act("$N resist the spell.", ch, NULL, victim, TO_CHAR);
    act("You resist the spell.", ch, NULL, victim, TO_VICT);
    return;
  }

  af.type = sn;
  af.duration = 1 + level;
  af.location = APPLY_AC;
  af.modifier = 50;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  if (ch != victim)
    act("$N is trapped in an airy set of shadow-bindings.", ch, NULL, victim, TO_CHAR);

  act("You are trapped in an airy set of shadow-bindings.", ch, NULL, victim, TO_VICT);
}

void spell_shadow_guard(int sn, int level, CHAR_DATA *ch, void *vo)
{
  EVENT_DATA *event;
  ROOM_INDEX_DATA *pRoom;
  char buf[MAX_INPUT_LENGTH];
  int cost = 500;

  if ((pRoom = ch->in_room) == NULL || IS_NPC(ch))
    return;

  if (event_isset_room(pRoom, EVENT_ROOM_SHADOWGUARD))
  {
    send_to_char("A shadow is already guarding this place.\n\r", ch);
    return;
  }

  if (ch->pcdata->powers[SHADOW_POWER] < cost)
  {
    printf_to_char(ch, "You need %d more shadowpoints to summon the shadowguard.\n\r", cost - ch->pcdata->powers[SHADOW_POWER]);
    return;
  }
  ch->pcdata->powers[SHADOW_POWER] -= cost;

  sprintf(buf, "%d", ch->pcdata->playerid);

  event = alloc_event();
  event->fun = &event_dummy;
  event->type = EVENT_ROOM_SHADOWGUARD;
  event->argument = str_dup(buf);
  add_event_room(event, pRoom, 5 * level * PULSE_PER_SECOND);

  act("$n summons a shadowguard to patrol this location.", ch, NULL, NULL, TO_ROOM);
  act("You summon a shadowguard to patrol this location.", ch, NULL, NULL, TO_CHAR);
}

void spell_locate_object(int sn, int level, CHAR_DATA *ch, void *vo)
{
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  ITERATOR *pIter;
  bool found = FALSE;
  int count = 1;
  int start_number;

  target_name = one_argument(target_name, arg1);
  one_argument(target_name, arg2);

  start_number = UMAX(1, atoi(arg2));

  pIter = AllocIterator(object_list);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (!can_see_obj(ch, obj) || !is_name(arg1, obj->name))
      continue;

    /* kingdom related items do not show */
    if (obj->pIndexData->vnum >= ROOM_VNUM_KINGDOMHALLS)
      continue;

    /* invis items are not shown */
    if (IS_OBJ_STAT(obj, ITEM_NOSHOW))
      continue;

    if (count < start_number)
    {
      count++;
      continue;
    }

    found = TRUE;

    for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);
    if (in_obj->carried_by != NULL)
      sprintf(buf, "%2d. %s carried by %s.\n\r", count, obj->short_descr, PERS(in_obj->carried_by, ch));
    else
      sprintf(buf, "%2d. %s in %s.\n\r", count, obj->short_descr, in_obj->in_room == NULL ? "somewhere" : in_obj->in_room->name);

    buf[0] = UPPER(buf[0]);
    send_to_char(buf, ch);

    if (count - start_number > 42)
      break;
    else
      count++;
  }
  if (!found)
    send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
  return;
}

void spell_magic_missile(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const sh_int dam_each[] = {
    0,
    3, 3, 4, 4, 5, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 8, 8, 8, 8, 8,
    9, 9, 9, 9, 9, 10, 10, 10, 10, 10,
    11, 11, 11, 11, 11, 12, 12, 12, 12, 12,
    13, 13, 13, 13, 13, 14, 14, 14, 14, 14,
    15, 20, 25, 30, 35, 40, 45, 55, 65, 75
  };
  int dam;

  level = URANGE(0, level, (int) (sizeof(dam_each) / sizeof(dam_each[0])) - 1);
  dam = number_range(dam_each[level] / 2, dam_each[level] * 2);

  if (saves_spell(level, victim))
    dam /= 2;
  damage(ch, victim, NULL, dam, sn);
  if (number_range(1, 3) != 2)
    (*skill_table[sn].spell_fun) (sn, level, ch, victim);
  return;
}

void spell_mass_invis(int sn, int level, CHAR_DATA * ch, void *vo)
{
  AFFECT_DATA af;
  CHAR_DATA *gch;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!is_same_group(gch, ch) || IS_AFFECTED(gch, AFF_INVISIBLE))
      continue;
    act("$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM);
    send_to_char("You slowly fade out of existence.\n\r", gch);
    af.type = sn;
    af.duration = 24;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char(gch, &af);
  }
  send_to_char("Ok.\n\r", ch);
}

void spell_null(int sn, int level, CHAR_DATA * ch, void *vo)
{
  send_to_char("That's not a spell!\n\r", ch);
  return;
}

void spell_pass_door(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED(victim, AFF_PASS_DOOR))
    return;
  af.type = sn;
  af.duration = number_fuzzy(level / 4);
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_PASS_DOOR;
  affect_to_char(victim, &af);
  act("$n turns translucent.", victim, NULL, NULL, TO_ROOM);
  send_to_char("You turn translucent.\n\r", victim);
  return;
}

void spell_poison(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  char buf[MAX_INPUT_LENGTH];

  /* Ghosts cannot be poisoned - KaVir */
  if (IS_NPC(victim) && IS_AFFECTED(victim, AFF_ETHEREAL))
    return;

  if (saves_spell(level, victim))
    return;
  af.type = sn;
  af.duration = level;
  af.location = APPLY_STR;
  af.modifier = 0 - number_range(1, 3);
  af.bitvector = AFF_POISON;
  affect_join(victim, &af);
  send_to_char("You feel very sick.\n\r", victim);
  if (ch == victim)
    return;
  if (!IS_NPC(victim))
    sprintf(buf, "%s looks very sick as your poison takes affect.\n\r", victim->name);
  else
    sprintf(buf, "%s looks very sick as your poison takes affect.\n\r", victim->short_descr);
  send_to_char(buf, ch);
  return;
}

void spell_readaura(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  char buf[MAX_INPUT_LENGTH];

  act("$n examines $N intently.", ch, NULL, victim, TO_NOTVICT);
  act("$n examines you intently.", ch, NULL, victim, TO_VICT);
  if (IS_NPC(victim))
    sprintf(buf, "%s is an NPC.\n\r", victim->short_descr);
  else
  {
    if (victim->level == 12)
      sprintf(buf, "%s is an Implementor.\n\r", victim->name);
    else if (victim->level == 11)
      sprintf(buf, "%s is a High Judge.\n\r", victim->name);
    else if (victim->level == 10)
      sprintf(buf, "%s is a Judge.\n\r", victim->name);
    else if (victim->level == 9)
      sprintf(buf, "%s is an Enforcer.\n\r", victim->name);
    else if (victim->level == 8)
      sprintf(buf, "%s is a Quest Maker.\n\r", victim->name);
    else if (victim->level == 7)
      sprintf(buf, "%s is a Builder.\n\r", victim->name);
    else if (victim->level >= 3)
      sprintf(buf, "%s is an Avatar.\n\r", victim->name);
    else
      sprintf(buf, "%s is a Mortal.\n\r", victim->name);
  }
  send_to_char(buf, ch);
  if (!IS_NPC(victim))
  {
    sprintf(buf, "Str:%d, Int:%d, Wis:%d, Dex:%d, Con:%d.\n\r", get_curr_str(victim), get_curr_int(victim), get_curr_wis(victim), get_curr_dex(victim), get_curr_con(victim));
    send_to_char(buf, ch);
  }
  sprintf(buf, "Hp:%d/%d, Mana:%d/%d, Move:%d/%d.\n\r", victim->hit, victim->max_hit, victim->mana, victim->max_mana, victim->move, victim->max_move);
  send_to_char(buf, ch);
  if (!IS_NPC(victim))
    sprintf(buf, "Hitroll:%d, Damroll:%d, AC:%d.\n\r", char_hitroll(victim), char_damroll(victim), char_ac(victim));
  else
    sprintf(buf, "AC:%d.\n\r", char_ac(victim));
  send_to_char(buf, ch);
  sprintf(buf, "Alignment:%d.\n\r", victim->alignment);
  send_to_char(buf, ch);
  if (!IS_NPC(victim) && IS_EXTRA(victim, EXTRA_PREGNANT) && ch->sex == SEX_FEMALE)
    act("$N is pregnant.", ch, NULL, victim, TO_CHAR);
}

void spell_protection_vs_good(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED(victim, AFF_PROTECT_GOOD))
    return;
  if (!IS_EVIL(victim))
  {
    send_to_char("The spell fails.\n\r", ch);
    return;
  }
  af.type = sn;
  af.duration = 24;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_PROTECT_GOOD;
  affect_to_char(victim, &af);
  send_to_char("You feel protected.\n\r", victim);
  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
}

void spell_protection(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED(victim, AFF_PROTECT))
    return;
  if (!IS_GOOD(victim))
  {
    send_to_char("The spell fails.\n\r", ch);
    return;
  }
  af.type = sn;
  af.duration = 24;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_PROTECT;
  affect_to_char(victim, &af);
  send_to_char("You feel protected.\n\r", victim);
  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
  return;
}

void spell_refresh(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  victim->move = UMIN(victim->move + level * 3, victim->max_move);
  act("$n looks less tired.", victim, NULL, NULL, TO_ROOM);
  send_to_char("You feel less tired.\n\r", victim);
  if (!IS_NPC(victim) && victim->sex == SEX_MALE && victim->pcdata->stage[0] < 1 && victim->pcdata->stage[2] > 0)
    victim->pcdata->stage[2] = 0;
  return;
}

void spell_remove_curse(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  char arg[MAX_INPUT_LENGTH];

  one_argument(target_name, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Remove curse on what?\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, target_name)) != NULL)
  {
    if (is_affected(victim, gsn_curse))
    {
      affect_strip(victim, gsn_curse);
      send_to_char("You feel better.\n\r", victim);
      if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    }
    return;
  }
  if ((obj = get_obj_carry(ch, arg)) != NULL)
  {
    if (IS_SET(obj->extra_flags, ITEM_NOREMOVE))
    {
      REMOVE_BIT(obj->extra_flags, ITEM_NOREMOVE);
      act("$p flickers with energy.", ch, obj, NULL, TO_CHAR);
    }
    else if (IS_SET(obj->extra_flags, ITEM_NODROP))
    {
      REMOVE_BIT(obj->extra_flags, ITEM_NODROP);
      act("$p flickers with energy.", ch, obj, NULL, TO_CHAR);
    }
    return;
  }
  send_to_char("No such creature or object to remove curse on.\n\r", ch);
  return;
}

void spell_sanctuary(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED(victim, AFF_SANCTUARY))
    return;
  af.type = sn;
  af.duration = number_fuzzy(level / 4);
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_SANCTUARY;
  affect_to_char(victim, &af);
  act("$n is surrounded in a white aura.", victim, NULL, NULL, TO_ROOM);
  send_to_char("You are surrounded by a white aura!\n\r", victim);
  return;
}

void spell_shield(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, sn))
    return;
  af.type = sn;
  af.duration = 8 + level;
  af.location = APPLY_AC;
  af.modifier = -20;
  af.bitvector = 0;
  affect_to_char(victim, &af);
  act("$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM);
  send_to_char("You are surrounded by a force shield.\n\r", victim);
  return;
}

void spell_stone_skin(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, sn))
    return;

  af.type = sn;
  af.duration = level / 10;
  af.location = APPLY_AC;
  af.modifier = -40;
  af.bitvector = 0;
  affect_to_char(victim, &af);
  act("$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM);
  send_to_char("Your skin turns to stone.\n\r", victim);
  return;
}

void spell_summon(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim;
  ITERATOR *pIter;
  CHAR_DATA *mount;
  OBJ_DATA *obj;
  bool has_key = FALSE;

  if ((victim = get_char_area(ch, target_name)) == NULL
      || victim == ch
      || IS_SET(victim->in_room->room_flags, ROOM_SAFE)
      || IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
      || IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
      || victim->level >= level + 3
      || victim->fighting != NULL
      || (!IS_NPC(victim) && !IS_IMMUNE(victim, IMM_SUMMON))
      || (IS_NPC(victim) && IS_AFFECTED(victim, AFF_ETHEREAL))
      || (IS_NPC(victim) && saves_spell(level, victim)))
  {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  pIter = AllocIterator(victim->carrying);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->item_type == ITEM_KEY)
      has_key = TRUE;
  }
  if (has_key)
  {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  act("$n disappears suddenly.", victim, NULL, NULL, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, ch->in_room, TRUE);
  act("$n arrives suddenly.", victim, NULL, NULL, TO_ROOM);
  act("$N has summoned you!", victim, NULL, ch, TO_CHAR);
  do_look(victim, "auto");
  if ((mount = victim->mount) == NULL)
    return;
  char_from_room(mount);
  char_to_room(mount, get_room_index(victim->in_room->vnum), TRUE);
  do_look(mount, "auto");
}

void spell_weaken(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, sn) || saves_spell(level, victim))
    return;
  af.type = sn;
  af.duration = level / 2;
  af.location = APPLY_STR;
  af.modifier = -2;
  af.bitvector = 0;
  affect_to_char(victim, &af);
  send_to_char("You feel weaker.\n\r", victim);
  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
  return;
}

/*
 * This is for muds that _want_ scrolls of recall.
 * Ick.
 */
void spell_word_of_recall(int sn, int level, CHAR_DATA * ch, void *vo)
{
  do_recall((CHAR_DATA *) vo, "");
  return;
}

void spell_fire_breath(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  int hpch;

  hpch = UMAX(10, ch->hit);
  dam = number_range(hpch / 16 + 1, hpch / 8);
  if (saves_spell(level, victim))
    dam /= 2;
  dam = cap_dam(ch, victim, dam);
  if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_HEAT))
    dam = 0;
  damage(ch, victim, NULL, dam, sn);
  return;
}

void spell_soulblade(int sn, int level, CHAR_DATA * ch, void *vo)
{
  const char * weapon_table[] =
  {
    "hit", "slice", "stab", "slash", "whip", "claw", "blast",
    "pound", "crush", "grep", "bite", "pierce", "suck"
  };
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  char buf[MAX_STRING_LENGTH];
  char wpnname[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int weapontype = 0, i;

  one_argument(target_name, arg);
  if (arg[0] == '\0')
  {
    BUFFER *bufs = buffer_new(MAX_STRING_LENGTH);
    int count = 0;

    bprintf(bufs, "Syntax : cast soulblade [type]\n\rTypes  :");
    for (i = 1; i < 12; i++)
    {
      if (i == 9) continue;

      if (ch->wpn[i] >= 200)
      {
        count++;

        bprintf(bufs, " %s", weapon_table[i]);
      }
    }
    if (count == 0)
      bprintf(bufs, " none\n\r");
    else
      bprintf(bufs, "\n\r");

    send_to_char(bufs->data, ch);
    buffer_free(bufs);
    return;
  }

  if (!str_cmp(arg, "slice"))
  {
    if (ch->wpn[1] < 200)
    {
      send_to_char("You have not grandmastered that weapontype.\n\r", ch);
      return;
    }
    weapontype = 1;
    sprintf(wpnname, "blade");
  }
  if (!str_cmp(arg, "stab"))
  {
    if (ch->wpn[2] < 200)
    {
      send_to_char("You have not grandmastered that weapontype.\n\r", ch);
      return;
    }
    weapontype = 2;
    sprintf(wpnname, "blade");
  }
  if (!str_cmp(arg, "slash"))
  {
    if (ch->wpn[3] < 200)
    {
      send_to_char("You have not grandmastered that weapontype.\n\r", ch);
      return;
    }
    weapontype = 3;
    sprintf(wpnname, "blade");
  }
  if (!str_cmp(arg, "whip"))
  {
    if (ch->wpn[4] < 200)
    {
      send_to_char("You have not grandmastered that weapontype.\n\r", ch);
      return;
    }
    weapontype = 4;
    sprintf(wpnname, "whip");
  }
  if (!str_cmp(arg, "claw"))
  {
    if (ch->wpn[5] < 200)
    {
      send_to_char("You have not grandmastered that weapontype.\n\r", ch);
      return;
    }
    weapontype = 5;
    sprintf(wpnname, "claw");
  }
  if (!str_cmp(arg, "blast"))
  {
    if (ch->wpn[6] < 200)
    {
      send_to_char("You have not grandmastered that weapontype.\n\r", ch);
      return;
    }
    weapontype = 6;
    sprintf(wpnname, "blaster");
  }
  if (!str_cmp(arg, "pound"))
  {
    if (ch->wpn[7] < 200)
    {
      send_to_char("You have not grandmastered that weapontype.\n\r", ch);
      return;
    }
    weapontype = 7;
    sprintf(wpnname, "mace");
  }
  if (!str_cmp(arg, "crush"))
  {
    if (ch->wpn[8] < 200)
    {
      send_to_char("You have not grandmastered that weapontype.\n\r", ch);
      return;
    }
    weapontype = 8;
    sprintf(wpnname, "mace");
  }
  if (!str_cmp(arg, "bite"))
  {
    if (ch->wpn[10] < 200)
    {
      send_to_char("You have not grandmastered that weapontype.\n\r", ch);
      return;
    }
    weapontype = 10;
    sprintf(wpnname, "fang");
  }
  if (!str_cmp(arg, "pierce"))
  {
    if (ch->wpn[11] < 200)
    {
      send_to_char("You have not grandmastered that weapontype.\n\r", ch);
      return;
    }
    weapontype = 11;
    sprintf(wpnname, "blade");
  }

  if (weapontype == 0)
  {
    send_to_char("That is not a valid weapontype.\n\r", ch);
    return;
  }

  obj = create_object(get_obj_index(OBJ_VNUM_SOULBLADE), 0);

  free_string(obj->name);
  sprintf(buf, "%s soul %s", ch->name, wpnname);
  obj->name = str_dup(buf);

  free_string(obj->short_descr);
  sprintf(buf, "%s's soul %s", ch->name, wpnname);
  buf[0] = UPPER(buf[0]);
  obj->short_descr = str_dup(buf);

  free_string(obj->description);
  sprintf(buf, "%s's soul %s is lying here.", ch->name, wpnname);
  buf[0] = UPPER(buf[0]);
  obj->description = str_dup(buf);

  if (ch->spl[2] > 4)
    obj->level = ch->spl[2] / 4;
  else
    obj->level = 1;
  if (obj->level > 60)
    obj->level = 60;
  obj->value[0] = 13034;
  obj->value[1] = 10;
  obj->value[2] = 20;
  obj->value[3] = weapontype;
  free_string(obj->questowner);
  obj->questowner = str_dup(ch->name);
  obj->ownerid = ch->pcdata->playerid;
  obj_to_char(obj, ch);

  act("$p fades into existance in your hand.", ch, obj, NULL, TO_CHAR);
  act("$p fades into existance in $n's hand.", ch, obj, NULL, TO_ROOM);
}

void spell_mana(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (ch->move < 50)
  {
    send_to_char("You are too exhausted to do that.\n\r", ch);
    return;
  }
  ch->move = ch->move - 50;
  victim->mana = UMIN(victim->mana + level + 10, victim->max_mana);
  update_pos(ch);
  update_pos(victim);
  if (ch == victim)
  {
    send_to_char("You draw in energy from your surrounding area.\n\r", ch);
    act("$n draws in energy from $s surrounding area.", ch, NULL, NULL, TO_ROOM);
    return;
  }
  act("You draw in energy from around you and channel it into $N.", ch, NULL, victim, TO_CHAR);
  act("$n draws in energy and channels it into $N.", ch, NULL, victim, TO_NOTVICT);
  act("$n draws in energy and channels it into you.", ch, NULL, victim, TO_VICT);
  return;
}

void spell_frenzy(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, sn))
    return;
  af.type = sn;
  af.duration = 1 + level / 10;
  af.location = APPLY_HITROLL;
  af.modifier = level / 10;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  af.location = APPLY_DAMROLL;
  af.modifier = level / 10;
  affect_to_char(victim, &af);

  af.location = APPLY_AC;
  af.modifier = level / 5;
  affect_to_char(victim, &af);
  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
  act("$n is consumed with rage.", victim, NULL, NULL, TO_ROOM);
  send_to_char("You are consumed with rage!\n\r", victim);
}

void spell_darkblessing(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, sn))
    return;

  af.type = sn;
  af.duration = level / 2;
  af.location = APPLY_HITROLL;
  af.modifier = 1 + level / 14;
  af.bitvector = 0;
  affect_to_char(victim, &af);
  af.location = APPLY_DAMROLL;
  af.modifier = 1 + level / 14;
  affect_to_char(victim, &af);

  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
  act("$n looks wicked.", victim, NULL, NULL, TO_ROOM);
  send_to_char("You feel wicked.\n\r", victim);
}

void spell_permanency(int sn, int level, CHAR_DATA *ch, void *vo)
{
  AFFECT_DATA *paf;
  EVENT_DATA *event;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];
  int cost = 5;
  bool backfire = FALSE, found = FALSE;

  one_argument(target_name, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Which spell do you wish to make permanent?\n\r", ch);
    return;
  }

  if (event_isset_mobile(ch, EVENT_PLAYER_PERMANENCY))
  {
    send_to_char("The mystical energies refuse to do your bidding.\n\r", ch);
    return;
  }

  if (ch->practice < cost)
  {
    printf_to_char(ch, "You need %d more primal before you can cast this spell.\n\r", cost - ch->practice);
    return;
  }
  ch->practice -= cost;

  if (number_percent() <= 5)
    backfire = TRUE;

  pIter = AllocIterator(ch->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_prefix(arg, skill_table[paf->type].name))
    {
      found = TRUE;

      if (backfire)
      {
        paf->duration = 1;
        printf_to_char(ch, "You #Rfail#n to enhance the duration of the '%s' spell.\n\r", skill_table[paf->type].name);
      }
      else
      {
        if (paf->duration < 50)
          paf->duration += 25;
        printf_to_char(ch, "You enhance the duration of the '%s' spell.\n\r", skill_table[paf->type].name);
      }
    }
  }

  if (!found)
  {
    send_to_char("You are not affected by any such spell.\n\r", ch);
  }
  else if (backfire)
  {
    event = alloc_event();
    event->type = EVENT_PLAYER_PERMANENCY;
    event->fun = &event_dummy;
    add_event_char(event, ch, 60 * PULSE_PER_SECOND);

    send_to_char("The permanency spell backfires.\n\r", ch);
  }
}

/* This spell is designed for potions */
void spell_energyflux(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  victim->mana = UMIN(victim->mana + 50, victim->max_mana);
  update_pos(victim);
  send_to_char("You feel mana channel into your body.\n\r", victim);
}

void spell_transport(int sn, int level, CHAR_DATA * ch, void *vo)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  CHAR_DATA *victim;

  target_name = one_argument(target_name, arg1);
  target_name = one_argument(target_name, arg2);

  if (arg1[0] == '\0')
  {
    send_to_char("Transport which object?\n\r", ch);
    return;
  }

  if (arg2[0] == '\0')
  {
    send_to_char("Transport who whom?\n\r", ch);
    return;
  }
  if ((victim = get_char_world(ch, arg2)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg1)) == NULL)
  {
    send_to_char("You are not carrying that item.\n\r", ch);
    return;
  }

  if (IS_SET(obj->quest, QUEST_ARTIFACT))
  {
    send_to_char("You can't transport that.\n\r", ch);
    return;
  }

  if (victim->shop_fun)
  {
    send_to_char("They don't want it.\n\r", ch);
    return;
  }

  if (IS_SET(victim->in_room->room_flags, ROOM_ASTRAL))
  {
    send_to_char("You can't find its room.\n\r", ch);
    return;
  }
  if (IS_SET(ch->in_room->room_flags, ROOM_ASTRAL))
  {
    send_to_char("Your room is not connected to the astral plane.\n\r", ch);
    return;
  }

  if (!IS_NPC(victim) && !IS_IMMUNE(victim, IMM_TRANSPORT))
  {
    send_to_char("You are unable to transport anything to them.\n\r", ch);
    return;
  }

  act("$p vanishes from your hands in an swirl of smoke.", ch, obj, NULL, TO_CHAR);
  act("$p vanishes from $n's hands in a swirl of smoke.", ch, obj, NULL, TO_ROOM);
  obj_from_char(obj);
  obj_to_char(obj, victim);
  act("$p appears in your hands in an swirl of smoke.", victim, obj, NULL, TO_CHAR);
  act("$p appears in $n's hands in an swirl of smoke.", victim, obj, NULL, TO_ROOM);
}

void spell_regenerate(int sn, int level, CHAR_DATA * ch, void *vo)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  CHAR_DATA *victim;
  int teeth = 0;

  target_name = one_argument(target_name, arg1);
  target_name = one_argument(target_name, arg2);

  if (arg1[0] == '\0')
  {
    send_to_char("Which body part?\n\r", ch);
    return;
  }

  if (arg2[0] == '\0')
  {
    send_to_char("Regenerate which person?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg2)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (victim->loc_hp[6] > 0)
  {
    send_to_char("You cannot regenerate someone who is still bleeding.\n\r", ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg1)) == NULL)
  {
    send_to_char("You are not carrying that item.\n\r", ch);
    return;
  }

  if (IS_HEAD(victim, LOST_TOOTH_1))
    teeth += 1;
  if (IS_HEAD(victim, LOST_TOOTH_2))
    teeth += 2;
  if (IS_HEAD(victim, LOST_TOOTH_4))
    teeth += 4;
  if (IS_HEAD(victim, LOST_TOOTH_8))
    teeth += 8;
  if (IS_HEAD(victim, LOST_TOOTH_16))
    teeth += 16;

  if (obj->pIndexData->vnum == OBJ_VNUM_SLICED_ARM)
  {
    if (!IS_ARM_L(victim, LOST_ARM) && !IS_ARM_R(victim, LOST_ARM))
    {
      send_to_char("They don't need an arm.\n\r", ch);
      return;
    }
    if (IS_ARM_L(victim, LOST_ARM))
      REMOVE_BIT(victim->loc_hp[LOC_ARM_L], LOST_ARM);
    else if (IS_ARM_R(victim, LOST_ARM))
      REMOVE_BIT(victim->loc_hp[LOC_ARM_R], LOST_ARM);
    act("You press $p onto the stump of $N's shoulder.", ch, obj, victim, TO_CHAR);
    act("$n presses $p onto the stump of $N's shoulder.", ch, obj, victim, TO_NOTVICT);
    act("$n presses $p onto the stump of your shoulder.", ch, obj, victim, TO_VICT);
    extract_obj(obj);
    return;
  }
  else if (obj->pIndexData->vnum == OBJ_VNUM_SLICED_LEG)
  {
    if (!IS_LEG_L(victim, LOST_LEG) && !IS_LEG_R(victim, LOST_LEG))
    {
      send_to_char("They don't need a leg.\n\r", ch);
      return;
    }
    if (IS_LEG_L(victim, LOST_LEG))
      REMOVE_BIT(victim->loc_hp[LOC_LEG_L], LOST_LEG);
    else if (IS_LEG_R(victim, LOST_LEG))
      REMOVE_BIT(victim->loc_hp[LOC_LEG_R], LOST_LEG);
    act("You press $p onto the stump of $N's hip.", ch, obj, victim, TO_CHAR);
    act("$n presses $p onto the stump of $N's hip.", ch, obj, victim, TO_NOTVICT);
    act("$n presses $p onto the stump of your hip.", ch, obj, victim, TO_VICT);
    extract_obj(obj);
    return;
  }
  else if (obj->pIndexData->vnum == OBJ_VNUM_SQUIDGY_EYEBALL)
  {
    if (!IS_HEAD(victim, LOST_EYE_L) && !IS_HEAD(victim, LOST_EYE_R))
    {
      send_to_char("They don't need an eye.\n\r", ch);
      return;
    }
    if (IS_HEAD(victim, LOST_EYE_L))
      REMOVE_BIT(victim->loc_hp[LOC_HEAD], LOST_EYE_L);
    else if (IS_HEAD(victim, LOST_EYE_R))
      REMOVE_BIT(victim->loc_hp[LOC_HEAD], LOST_EYE_R);
    act("You press $p into $N's empty eye socket.", ch, obj, victim, TO_CHAR);
    act("$n presses $p into $N's empty eye socket.", ch, obj, victim, TO_NOTVICT);
    act("$n presses $p into your empty eye socket.", ch, obj, victim, TO_VICT);
    extract_obj(obj);
    return;
  }
  else if (obj->pIndexData->vnum == OBJ_VNUM_SLICED_EAR)
  {
    if (!IS_HEAD(victim, LOST_EAR_L) && !IS_HEAD(victim, LOST_EAR_R))
    {
      send_to_char("They don't need an ear.\n\r", ch);
      return;
    }
    if (IS_HEAD(victim, LOST_EAR_L))
      REMOVE_BIT(victim->loc_hp[LOC_HEAD], LOST_EAR_L);
    else if (IS_HEAD(victim, LOST_EAR_R))
      REMOVE_BIT(victim->loc_hp[LOC_HEAD], LOST_EAR_R);
    act("You press $p onto the side of $N's head.", ch, obj, victim, TO_CHAR);
    act("$n presses $p onto the side of $N's head.", ch, obj, victim, TO_NOTVICT);
    act("$n presses $p onto the side of your head.", ch, obj, victim, TO_VICT);
    extract_obj(obj);
    return;
  }
  else if (obj->pIndexData->vnum == OBJ_VNUM_SLICED_NOSE)
  {
    if (!IS_HEAD(victim, LOST_NOSE))
    {
      send_to_char("They don't need a nose.\n\r", ch);
      return;
    }
    REMOVE_BIT(victim->loc_hp[LOC_HEAD], LOST_NOSE);
    act("You press $p onto the front of $N's face.", ch, obj, victim, TO_CHAR);
    act("$n presses $p onto the front of $N's face.", ch, obj, victim, TO_NOTVICT);
    act("$n presses $p onto the front of your face.", ch, obj, victim, TO_VICT);
    extract_obj(obj);
    return;
  }
  else if (obj->pIndexData->vnum == OBJ_VNUM_SEVERED_HAND)
  {
    if (!IS_ARM_L(victim, LOST_ARM) && IS_ARM_L(victim, LOST_HAND))
      REMOVE_BIT(victim->loc_hp[LOC_ARM_L], LOST_HAND);
    else if (!IS_ARM_R(victim, LOST_ARM) && IS_ARM_R(victim, LOST_HAND))
      REMOVE_BIT(victim->loc_hp[LOC_ARM_R], LOST_HAND);
    else
    {
      send_to_char("They don't need a hand.\n\r", ch);
      return;
    }
    act("You press $p onto the stump of $N's wrist.", ch, obj, victim, TO_CHAR);
    act("$n presses $p onto the stump of $N's wrist.", ch, obj, victim, TO_NOTVICT);
    act("$n presses $p onto the stump of your wrist.", ch, obj, victim, TO_VICT);
    extract_obj(obj);
    return;
  }
  else if (obj->pIndexData->vnum == OBJ_VNUM_SEVERED_FOOT)
  {
    if (!IS_LEG_L(victim, LOST_LEG) && IS_LEG_L(victim, LOST_FOOT))
      REMOVE_BIT(victim->loc_hp[LOC_LEG_L], LOST_FOOT);
    else if (!IS_LEG_R(victim, LOST_LEG) && IS_LEG_R(victim, LOST_FOOT))
      REMOVE_BIT(victim->loc_hp[LOC_LEG_R], LOST_FOOT);
    else
    {
      send_to_char("They don't need a foot.\n\r", ch);
      return;
    }
    act("You press $p onto the stump of $N's ankle.", ch, obj, victim, TO_CHAR);
    act("$n presses $p onto the stump of $N's ankle.", ch, obj, victim, TO_NOTVICT);
    act("$n presses $p onto the stump of your ankle.", ch, obj, victim, TO_VICT);
    extract_obj(obj);
    return;
  }
  else if (obj->pIndexData->vnum == OBJ_VNUM_SEVERED_THUMB)
  {
    if (!IS_ARM_L(victim, LOST_ARM) && !IS_ARM_L(victim, LOST_HAND) && IS_ARM_L(victim, LOST_THUMB))
      REMOVE_BIT(victim->loc_hp[LOC_ARM_L], LOST_THUMB);
    else if (!IS_ARM_R(victim, LOST_ARM) && !IS_ARM_R(victim, LOST_HAND) && IS_ARM_R(victim, LOST_THUMB))
      REMOVE_BIT(victim->loc_hp[LOC_ARM_R], LOST_THUMB);
    else
    {
      send_to_char("They don't need a thumb.\n\r", ch);
      return;
    }
    act("You press $p onto $N's hand.", ch, obj, victim, TO_CHAR);
    act("$n presses $p onto $N's hand.", ch, obj, victim, TO_NOTVICT);
    act("$n presses $p onto your hand.", ch, obj, victim, TO_VICT);
    extract_obj(obj);
    return;
  }
  else if (obj->pIndexData->vnum == OBJ_VNUM_SEVERED_INDEX)
  {
    if (!IS_ARM_L(victim, LOST_ARM) && !IS_ARM_L(victim, LOST_HAND) && IS_ARM_L(victim, LOST_FINGER_I))
      REMOVE_BIT(victim->loc_hp[LOC_ARM_L], LOST_FINGER_I);
    else if (!IS_ARM_R(victim, LOST_ARM) && !IS_ARM_R(victim, LOST_HAND) && IS_ARM_R(victim, LOST_FINGER_I))
      REMOVE_BIT(victim->loc_hp[LOC_ARM_R], LOST_FINGER_I);
    else
    {
      send_to_char("They don't need an index finger.\n\r", ch);
      return;
    }
    act("You press $p onto $N's hand.", ch, obj, victim, TO_CHAR);
    act("$n presses $p onto $N's hand.", ch, obj, victim, TO_NOTVICT);
    act("$n presses $p onto your hand.", ch, obj, victim, TO_VICT);
    extract_obj(obj);
    return;
  }
  else if (obj->pIndexData->vnum == OBJ_VNUM_SEVERED_MIDDLE)
  {
    if (!IS_ARM_L(victim, LOST_ARM) && !IS_ARM_L(victim, LOST_HAND) && IS_ARM_L(victim, LOST_FINGER_M))
      REMOVE_BIT(victim->loc_hp[LOC_ARM_L], LOST_FINGER_M);
    else if (!IS_ARM_R(victim, LOST_ARM) && !IS_ARM_R(victim, LOST_HAND) && IS_ARM_R(victim, LOST_FINGER_M))
      REMOVE_BIT(victim->loc_hp[LOC_ARM_R], LOST_FINGER_M);
    else
    {
      send_to_char("They don't need a middle finger.\n\r", ch);
      return;
    }
    act("You press $p onto $N's hand.", ch, obj, victim, TO_CHAR);
    act("$n presses $p onto $N's hand.", ch, obj, victim, TO_NOTVICT);
    act("$n presses $p onto your hand.", ch, obj, victim, TO_VICT);
    extract_obj(obj);
    return;
  }
  else if (obj->pIndexData->vnum == OBJ_VNUM_SEVERED_RING)
  {
    if (!IS_ARM_L(victim, LOST_ARM) && !IS_ARM_L(victim, LOST_HAND) && IS_ARM_L(victim, LOST_FINGER_R))
      REMOVE_BIT(victim->loc_hp[LOC_ARM_L], LOST_FINGER_R);
    else if (!IS_ARM_R(victim, LOST_ARM) && !IS_ARM_R(victim, LOST_HAND) && IS_ARM_R(victim, LOST_FINGER_R))
      REMOVE_BIT(victim->loc_hp[LOC_ARM_R], LOST_FINGER_R);
    else
    {
      send_to_char("They don't need a ring finger.\n\r", ch);
      return;
    }
    act("You press $p onto $N's hand.", ch, obj, victim, TO_CHAR);
    act("$n presses $p onto $N's hand.", ch, obj, victim, TO_NOTVICT);
    act("$n presses $p onto your hand.", ch, obj, victim, TO_VICT);
    extract_obj(obj);
    return;
  }
  else if (obj->pIndexData->vnum == OBJ_VNUM_SEVERED_LITTLE)
  {
    if (!IS_ARM_L(victim, LOST_ARM) && !IS_ARM_L(victim, LOST_HAND) && IS_ARM_L(victim, LOST_FINGER_L))
      REMOVE_BIT(victim->loc_hp[LOC_ARM_L], LOST_FINGER_L);
    else if (!IS_ARM_R(victim, LOST_ARM) && !IS_ARM_R(victim, LOST_HAND) && IS_ARM_R(victim, LOST_FINGER_L))
      REMOVE_BIT(victim->loc_hp[LOC_ARM_R], LOST_FINGER_L);
    else
    {
      send_to_char("They don't need a little finger.\n\r", ch);
      return;
    }
    act("You press $p onto $N's hand.", ch, obj, victim, TO_CHAR);
    act("$n presses $p onto $N's hand.", ch, obj, victim, TO_NOTVICT);
    act("$n presses $p onto your hand.", ch, obj, victim, TO_VICT);
    extract_obj(obj);
    return;
  }
  else if (teeth > 0)
  {
    if (IS_HEAD(victim, LOST_TOOTH_1))
      REMOVE_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_1);
    if (IS_HEAD(victim, LOST_TOOTH_2))
      REMOVE_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_2);
    if (IS_HEAD(victim, LOST_TOOTH_4))
      REMOVE_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_4);
    if (IS_HEAD(victim, LOST_TOOTH_8))
      REMOVE_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_8);
    if (IS_HEAD(victim, LOST_TOOTH_16))
      REMOVE_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_16);
    teeth -= 1;
    if (teeth >= 16)
    {
      teeth -= 16;
      SET_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_16);
    }
    if (teeth >= 8)
    {
      teeth -= 8;
      SET_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_8);
    }
    if (teeth >= 4)
    {
      teeth -= 4;
      SET_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_4);
    }
    if (teeth >= 2)
    {
      teeth -= 2;
      SET_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_2);
    }
    if (teeth >= 1)
    {
      teeth -= 1;
      SET_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_1);
    }
    act("You press $p into $N's mouth.", ch, obj, victim, TO_CHAR);
    act("$n presses $p into $N's mouth.", ch, obj, victim, TO_NOTVICT);
    act("$n presses $p into your mouth.", ch, obj, victim, TO_VICT);
    extract_obj(obj);
  }
  else
  {
    act("There is nowhere to stick $p on $N.", ch, obj, victim, TO_CHAR);
    return;
  }
  return;
}

void spell_clot(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  if (IS_BLEEDING(victim, BLEEDING_HEAD))
  {
    act("$n's head stops bleeding.", victim, NULL, NULL, TO_ROOM);
    act("Your head stops bleeding.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[6], BLEEDING_HEAD);
  }
  else if (IS_BLEEDING(victim, BLEEDING_THROAT))
  {
    act("$n's throat stops bleeding.", victim, NULL, NULL, TO_ROOM);
    act("Your throat stops bleeding.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[6], BLEEDING_THROAT);
  }
  else if (IS_BLEEDING(victim, BLEEDING_ARM_L))
  {
    act("The stump of $n's left arm stops bleeding.", victim, NULL, NULL, TO_ROOM);
    act("The stump of your left arm stops bleeding.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[6], BLEEDING_ARM_L);
  }
  else if (IS_BLEEDING(victim, BLEEDING_ARM_R))
  {
    act("The stump of $n's right arm stops bleeding.", victim, NULL, NULL, TO_ROOM);
    act("The stump of your right arm stops bleeding.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[6], BLEEDING_ARM_R);
  }
  else if (IS_BLEEDING(victim, BLEEDING_LEG_L))
  {
    act("The stump of $n's left leg stops bleeding.", victim, NULL, NULL, TO_ROOM);
    act("The stump of your left leg stops bleeding.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[6], BLEEDING_LEG_L);
  }
  else if (IS_BLEEDING(victim, BLEEDING_LEG_R))
  {
    act("The stump of $n's right leg stops bleeding.", victim, NULL, NULL, TO_ROOM);
    act("The stump of your right leg stops bleeding.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[6], BLEEDING_LEG_R);
  }
  else if (IS_BLEEDING(victim, BLEEDING_HAND_L))
  {
    act("The stump of $n's left wrist stops bleeding.", victim, NULL, NULL, TO_ROOM);
    act("The stump of your left wrist stops bleeding.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[6], BLEEDING_HAND_L);
  }
  else if (IS_BLEEDING(victim, BLEEDING_HAND_R))
  {
    act("The stump of $n's right wrist stops bleeding.", victim, NULL, NULL, TO_ROOM);
    act("The stump of your right wrist stops bleeding.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[6], BLEEDING_HAND_R);
  }
  else if (IS_BLEEDING(victim, BLEEDING_FOOT_L))
  {
    act("The stump of $n's left ankle stops bleeding.", victim, NULL, NULL, TO_ROOM);
    act("The stump of your left ankle stops bleeding.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[6], BLEEDING_FOOT_L);
  }
  else if (IS_BLEEDING(victim, BLEEDING_FOOT_R))
  {
    act("The stump of $n's right ankle stops bleeding.", victim, NULL, NULL, TO_ROOM);
    act("The stump of your right ankle stops bleeding.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[6], BLEEDING_FOOT_R);
  }
  else
    send_to_char("They have no wounds to clot.\n\r", ch);
  return;
}

void spell_mend(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int ribs = 0;

  if (IS_BODY(victim, BROKEN_RIBS_1))
    ribs += 1;
  if (IS_BODY(victim, BROKEN_RIBS_2))
    ribs += 2;
  if (IS_BODY(victim, BROKEN_RIBS_4))
    ribs += 4;
  if (IS_BODY(victim, BROKEN_RIBS_8))
    ribs += 8;
  if (IS_BODY(victim, BROKEN_RIBS_16))
    ribs += 16;

  if (ribs > 0)
  {
    if (IS_BODY(victim, BROKEN_RIBS_1))
      REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_1);
    if (IS_BODY(victim, BROKEN_RIBS_2))
      REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_2);
    if (IS_BODY(victim, BROKEN_RIBS_4))
      REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_4);
    if (IS_BODY(victim, BROKEN_RIBS_8))
      REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_8);
    if (IS_BODY(victim, BROKEN_RIBS_16))
      REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_16);
    ribs -= 1;
    if (ribs >= 16)
    {
      ribs -= 16;
      SET_BIT(victim->loc_hp[1], BROKEN_RIBS_16);
    }
    if (ribs >= 8)
    {
      ribs -= 8;
      SET_BIT(victim->loc_hp[1], BROKEN_RIBS_8);
    }
    if (ribs >= 4)
    {
      ribs -= 4;
      SET_BIT(victim->loc_hp[1], BROKEN_RIBS_4);
    }
    if (ribs >= 2)
    {
      ribs -= 2;
      SET_BIT(victim->loc_hp[1], BROKEN_RIBS_2);
    }
    if (ribs >= 1)
    {
      ribs -= 1;
      SET_BIT(victim->loc_hp[1], BROKEN_RIBS_1);
    }
    act("One of $n's ribs snap back into place.", victim, NULL, NULL, TO_ROOM);
    act("One of your ribs snap back into place.", victim, NULL, NULL, TO_CHAR);
  }
  else if (IS_HEAD(victim, BROKEN_NOSE) && !IS_HEAD(victim, LOST_NOSE))
  {
    act("$n's nose snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your nose snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_HEAD], BROKEN_NOSE);
  }
  else if (IS_HEAD(victim, BROKEN_JAW))
  {
    act("$n's jaw snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your jaw snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_HEAD], BROKEN_JAW);
  }
  else if (IS_HEAD(victim, BROKEN_SKULL))
  {
    act("$n's skull knits itself back together.", victim, NULL, NULL, TO_ROOM);
    act("Your skull knits itself back together.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_HEAD], BROKEN_SKULL);
  }
  else if (IS_BODY(victim, BROKEN_SPINE))
  {
    act("$n's spine knits itself back together.", victim, NULL, NULL, TO_ROOM);
    act("Your spine knits itself back together.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_BODY], BROKEN_SPINE);
  }
  else if (IS_BODY(victim, BROKEN_NECK))
  {
    act("$n's neck snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your neck snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_BODY], BROKEN_NECK);
  }
  else if (IS_ARM_L(victim, BROKEN_ARM) && !IS_ARM_L(victim, LOST_ARM))
  {
    act("$n's left arm snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your left arm snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_ARM_L], BROKEN_ARM);
  }
  else if (IS_ARM_R(victim, BROKEN_ARM) && !IS_ARM_R(victim, LOST_ARM))
  {
    act("$n's right arm snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your right arm snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_ARM_R], BROKEN_ARM);
  }
  else if (IS_LEG_L(victim, BROKEN_LEG) && !IS_LEG_L(victim, LOST_LEG))
  {
    act("$n's left leg snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your left leg snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_LEG_L], BROKEN_LEG);
  }
  else if (IS_LEG_R(victim, BROKEN_LEG) && !IS_LEG_R(victim, LOST_LEG))
  {
    act("$n's right leg snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your right leg snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_LEG_R], BROKEN_LEG);
  }
  else if (IS_ARM_L(victim, BROKEN_THUMB) && !IS_ARM_L(victim, LOST_ARM) && !IS_ARM_L(victim, LOST_HAND) && !IS_ARM_L(victim, LOST_THUMB))
  {
    act("$n's left thumb snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your left thumb snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_ARM_L], BROKEN_THUMB);
  }
  else if (IS_ARM_L(victim, BROKEN_FINGER_I) && !IS_ARM_L(victim, LOST_ARM) && !IS_ARM_L(victim, LOST_HAND) && !IS_ARM_L(victim, LOST_FINGER_I))
  {
    act("$n's left index finger snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your left index finger snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_ARM_L], BROKEN_FINGER_I);
  }
  else if (IS_ARM_L(victim, BROKEN_FINGER_M) && !IS_ARM_L(victim, LOST_ARM) && !IS_ARM_L(victim, LOST_HAND) && !IS_ARM_L(victim, LOST_FINGER_M))
  {
    act("$n's left middle finger snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your left middle finger snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_ARM_L], BROKEN_FINGER_M);
  }
  else if (IS_ARM_L(victim, BROKEN_FINGER_R) && !IS_ARM_L(victim, LOST_ARM) && !IS_ARM_L(victim, LOST_HAND) && !IS_ARM_L(victim, LOST_FINGER_R))
  {
    act("$n's left ring finger snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your left ring finger snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_ARM_L], BROKEN_FINGER_R);
  }
  else if (IS_ARM_L(victim, BROKEN_FINGER_L) && !IS_ARM_L(victim, LOST_ARM) && !IS_ARM_L(victim, LOST_HAND) && !IS_ARM_L(victim, LOST_FINGER_L))
  {
    act("$n's left little finger snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your left little finger snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_ARM_L], BROKEN_FINGER_L);
  }
  else if (IS_ARM_R(victim, BROKEN_THUMB) && !IS_ARM_R(victim, LOST_ARM) && !IS_ARM_R(victim, LOST_HAND) && !IS_ARM_R(victim, LOST_THUMB))
  {
    act("$n's right thumb snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your right thumb snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_ARM_R], BROKEN_THUMB);
  }
  else if (IS_ARM_R(victim, BROKEN_FINGER_I) && !IS_ARM_R(victim, LOST_ARM) && !IS_ARM_R(victim, LOST_HAND) && !IS_ARM_R(victim, LOST_FINGER_I))
  {
    act("$n's right index finger snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your right index finger snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_ARM_R], BROKEN_FINGER_I);
  }
  else if (IS_ARM_R(victim, BROKEN_FINGER_M) && !IS_ARM_R(victim, LOST_ARM) && !IS_ARM_R(victim, LOST_HAND) && !IS_ARM_R(victim, LOST_FINGER_M))
  {
    act("$n's right middle finger snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your right middle finger snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_ARM_R], BROKEN_FINGER_M);
  }
  else if (IS_ARM_R(victim, BROKEN_FINGER_R) && !IS_ARM_R(victim, LOST_ARM) && !IS_ARM_R(victim, LOST_HAND) && !IS_ARM_R(victim, LOST_FINGER_R))
  {
    act("$n's right ring finger snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your right ring finger snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_ARM_R], BROKEN_FINGER_R);
  }
  else if (IS_ARM_R(victim, BROKEN_FINGER_L) && !IS_ARM_R(victim, LOST_ARM) && !IS_ARM_R(victim, LOST_HAND) && !IS_ARM_R(victim, LOST_FINGER_L))
  {
    act("$n's right little finger snaps back into place.", victim, NULL, NULL, TO_ROOM);
    act("Your right little finger snaps back into place.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_ARM_R], BROKEN_FINGER_L);
  }
  else if (IS_BODY(victim, CUT_THROAT))
  {
    if (IS_SET(victim->loc_hp[6], BLEEDING_THROAT))
    {
      send_to_char("But their throat is still bleeding!\n\r", ch);
      return;
    }
    act("The wound in $n's throat closes up.", victim, NULL, NULL, TO_ROOM);
    act("The wound in your throat closes up.", victim, NULL, NULL, TO_CHAR);
    REMOVE_BIT(victim->loc_hp[LOC_BODY], CUT_THROAT);
  }
  else
    send_to_char("They have no bones to mend.\n\r", ch);
  return;
}

void spell_mount(int sn, int level, CHAR_DATA * ch, void *vo)
{
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  if (IS_NPC(ch))
    return;

  if (ch->pcdata->followers > 4)
  {
    send_to_char("Nothing happens.\n\r", ch);
    return;
  }
  ch->pcdata->followers++;

  victim = create_mobile(get_mob_index(MOB_VNUM_MOUNT));
  victim->level = level + 1;
  victim->armor = 0 - (2 * level);
  victim->hitroll = level;
  victim->damroll = level;
  victim->hit = 100 * level + 1;
  victim->max_hit = 100 * level + 1;
  SET_BIT(victim->affected_by, AFF_FLYING);
  SET_BIT(victim->act, ACT_NOEXP);
  if (IS_GOOD(ch))
  {
    free_string(victim->name);
    victim->name = str_dup("mount white horse pegasus");
    sprintf(buf, "%s's white pegasus", ch->name);
    free_string(victim->short_descr);
    victim->short_descr = str_dup(buf);
    free_string(victim->long_descr);
    victim->long_descr = str_dup("A beautiful white pegasus stands here.\n\r");
  }
  else if (IS_NEUTRAL(ch))
  {
    free_string(victim->name);
    victim->name = str_dup("mount griffin");
    sprintf(buf, "%s's griffin", ch->name);
    free_string(victim->short_descr);
    victim->short_descr = str_dup(buf);
    free_string(victim->long_descr);
    victim->long_descr = str_dup("A vicious looking griffin stands here.\n\r");
  }
  else
  {
    free_string(victim->name);
    victim->name = str_dup("mount black horse nightmare");
    sprintf(buf, "%s's black nightmare", ch->name);
    free_string(victim->short_descr);
    victim->short_descr = str_dup(buf);
    free_string(victim->long_descr);
    victim->long_descr = str_dup("A large black demonic horse stands here.\n\r");
  }
  char_to_room(victim, ch->in_room, TRUE);
  act("$N fades into existance.", ch, NULL, victim, TO_CHAR);
  act("$N fades into existance.", ch, NULL, victim, TO_ROOM);
  return;
}

void spell_repair(int sn, int level, CHAR_DATA * ch, void *vo)
{
  OBJ_DATA *obj;
  ITERATOR *pIter;
  bool found = FALSE;

  pIter = AllocIterator(ch->carrying);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_OBJ_STAT(obj, ITEM_NOREPAIR))
      continue;

    if (obj->condition < 100 && can_see_obj(ch, obj))
    {
      found = TRUE;
      obj->condition = 100;
      act("$p magically repairs itself.", ch, obj, NULL, TO_ALL);
    }
  }
  if (!found)
  {
    send_to_char("None of your equipment needs repairing.\n\r", ch);
    return;
  }
  return;
}

void spell_spellproof(int sn, int level, CHAR_DATA * ch, void *vo)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;

  if (IS_SET(obj->quest, QUEST_SPELLPROOF))
  {
    send_to_char("That item is already resistance to spells.\n\r", ch);
    return;
  }

  SET_BIT(obj->quest, QUEST_SPELLPROOF);
  act("$p shimmers for a moment.", ch, obj, NULL, TO_CHAR);
  act("$p shimmers for a moment.", ch, obj, NULL, TO_ROOM);
  return;
}

void spell_chaos_blast(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if (IS_ITEMAFF(victim, ITEMA_CHAOSSHIELD))
    return;

  dam = dice(level, 150);

  if (saves_spell(level, victim))
    dam /= 2;

  if (IS_AFFECTED(ch, AFF_SANCTUARY))
    dam /= 2;

  if (IS_ITEMAFF(ch, ITEMA_CHAOSSHIELD))
  {
    dam *= 3;
    dam /= 4;
  }

  if (!IS_NPC(ch))
    dam /= 2;

  if (IS_NPC(victim) && dam >= victim->hit)
    dam = victim->hit - 1;

  if (dam == 0)
    return;

  damage(ch, victim, NULL, dam, sn);
}

void spell_resistance(int sn, int level, CHAR_DATA * ch, void *vo)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;

  if (IS_SET(obj->quest, QUEST_ARTIFACT))
  {
    send_to_char("Not on artifacts.\n\r", ch);
    return;
  }

  if (IS_OBJ_STAT(obj, ITEM_NOREPAIR))
  {
    send_to_char("This item cannot be targetted by this spell.\n\r", ch);
    return;
  }

  if (obj->resistance <= 10 && obj->toughness >= 100)
  {
    send_to_char("You cannot make that item any more resistant.\n\r", ch);
    return;
  }
  obj->resistance = 10;
  obj->toughness = 100;
  act("$p sparkles for a moment.", ch, obj, NULL, TO_CHAR);
  act("$p sparkles for a moment.", ch, obj, NULL, TO_ROOM);
}

void spell_spiritkiss(int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (victim == NULL)
    victim = ch;

  /* damned good at casting these spells */
  if (IS_CLASS(ch, CLASS_FAE))
    level *= 2;

  if (is_affected(victim, sn))
    return;
  af.type = sn;
  af.duration = 2 * level;
  af.location = APPLY_HITROLL;
  af.modifier = level;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  af.type = sn;
  af.duration = 2 * level;
  af.location = APPLY_DAMROLL;
  af.modifier = level;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  af.location = APPLY_SAVING_SPELL;
  af.modifier = UMIN(10, level / 8);
  affect_to_char(victim, &af);
  act("$n is filled with spiritual power.", victim, NULL, NULL, TO_ROOM);
  send_to_char("You are blessed by the spirits.\n\r", victim);
  if (ch != victim)
    send_to_char("Ok.\n\r", ch);
}
