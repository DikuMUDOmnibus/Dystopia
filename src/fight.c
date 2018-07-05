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
#include <math.h>

#include "dystopia.h"


/* death programs defines and function pointers */
DECLARE_DEATH_FUN( deathspec_memnon		);
DECLARE_DEATH_FUN( deathspec_demonspawn         );
DECLARE_DEATH_FUN( deathspec_plasma		);
DECLARE_DEATH_FUN( deathspec_archangel		);
DECLARE_DEATH_FUN( deathspec_guarddog           );
DECLARE_DEATH_FUN( deathspec_duegar		);
DECLARE_DEATH_FUN( deathspec_dwarven		);
DECLARE_DEATH_FUN( deathspec_snotling		);
DECLARE_DEATH_FUN( deathspec_illthid            );
DECLARE_DEATH_FUN( deathspec_kingdom_guard      );

const struct death_type death_table [] =
{
  { "deathspec_memnon",         deathspec_memnon           },
  { "deathspec_demonspawn",     deathspec_demonspawn       },
  { "deathspec_plasma",         deathspec_plasma           },
  { "deathspec_archangel",      deathspec_archangel	   },
  { "deathspec_guarddog",       deathspec_guarddog         },
  { "deathspec_duegar",         deathspec_duegar           },
  { "deathspec_dwarven",        deathspec_dwarven          },
  { "deathspec_snotling",       deathspec_snotling         },
  { "deathspec_illthid",        deathspec_illthid          },
  { "deathspec_kingdom_guard",  deathspec_kingdom_guard    },

  /* end of table */
  { "", 0 }
};

/*
 * Local functions.
 */
void  autodrop            ( CHAR_DATA *ch, CHAR_DATA *victim );
bool  check_parry         ( CHAR_DATA *ch, CHAR_DATA *victim, int dt );
void  dam_message         ( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, int dam, int dt );
void  death_cry           ( CHAR_DATA *ch );
void  group_gain          ( CHAR_DATA *ch, CHAR_DATA *victim );
int   xp_compute          ( CHAR_DATA *victim );
bool  can_counter         ( CHAR_DATA *ch );
bool  can_bypass          ( CHAR_DATA *ch );
int   number_attacks      ( CHAR_DATA *ch, CHAR_DATA *victim );

/* this structure is specific to the damage_message() function */
struct message_type  
{
  int      max_dam;
  char   * message;
};

bool event_mobile_fighting(EVENT_DATA *event)
{
  CHAR_DATA *ch, *victim, *rch;
  ITERATOR *pIter, *pIter2;
  int turn;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_fighting: no owner.", 0);
    return FALSE;
  }

  if (event->argument == NULL || (turn = atoi(event->argument)) < 0)
  {
    bug("event_mobile_fighting: no valid argument for turn.", 0);
    return FALSE;
  }

  /* 1 <= turn <= 3 */
  turn = turn % 3 + 1;

  /* not fighting anymore, stop this event sequence */
  if ((victim = ch->fighting) == NULL || ch->in_room == NULL || ch->hit <= 0 || ch->position <= POS_STUNNED)
    return FALSE;

  if (is_safe(ch, victim))
  {
    stop_fighting(ch, FALSE);
    return FALSE;
  }

  /* banding monsters at almost full health will try to form a group with others */
  if (IS_NPC(ch) && IS_SET(ch->act, ACT_BANDING) && ch->hit >= 9 * ch->max_hit / 10)
  {
    pIter = AllocIterator(ch->in_room->people);
    while ((rch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (!IS_NPC(rch) || rch == ch) continue;

      if (rch->pIndexData->vnum == ch->pIndexData->vnum && rch->fighting == victim)
      {
        act("$n joins $N's group.", ch, NULL, rch, TO_NOTVICT);

        if (victim->fighting == ch)
          victim->fighting = rch;

        rch->gcount += ch->gcount;
        band_description(rch);
        extract_char(ch, TRUE);

        return TRUE;
      }
    }
  }

  /* update fighttimers */
  if (!IS_NPC(ch) && !IS_NPC(victim))
  {
    if (ch->fight_timer < 8)
      ch->fight_timer = 8;
    else if (ch->fight_timer < 20)
      ch->fight_timer += 2;
  }

  /* deal the damage */
  if (IS_AWAKE(ch) && IS_AWAKE(victim) && ch->in_room == victim->in_room)
    multi_hit(ch, victim, turn);
  else
    stop_fighting(ch, FALSE);

  /* we should check to see if ch->fighting was NULL'ed */
  if ((victim = ch->fighting) == NULL)
  {
    if (ch->dead)
      return TRUE;

    return FALSE;
  }

  pIter = AllocIterator(ch->in_room->people);
  while ((rch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_AWAKE(rch) && rch->fighting == NULL)
    {
      CHAR_DATA *mount;

      /*
       * mounts and group members autoassists
       */
      if ((mount = rch->mount) == ch)
      {
        multi_hit(rch, victim, 1);
      }
      else if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
      {
        if ((!IS_NPC(rch) || IS_AFFECTED(rch, AFF_CHARM)) && is_same_group(ch, rch))
          multi_hit(rch, victim, 1);
      }

      /* mobiles of same type might help the combat (1/8th chance) */
      else if (IS_NPC(ch) && IS_NPC(rch) && !IS_AFFECTED(rch, AFF_CHARM))
      {
        if (rch->pIndexData == ch->pIndexData || number_bits(3) == 0)
        {
          CHAR_DATA *vch;
          CHAR_DATA *target = NULL;
          int number = 0;

          pIter2 = AllocIterator(ch->in_room->people);
          while ((vch = (CHAR_DATA *) NextInList(pIter2)) != NULL)
          {
            if (can_see(rch, vch) && is_same_group(vch, victim) && number_range(0, number) == 0)
            {
              target = vch;
              number++;
            }
          }

          if (target != NULL)
            multi_hit(rch, target, 1);
        }
      }
    }
  }

  /* continue event sequence */
  if (ch->dead == FALSE && ch->fighting)
  {
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *gch = ch->fighting;
    int delay = 1;

    sprintf(buf, "%d", turn);

    /* gch <- fight -> ch and sinkhole affect */
    if (IS_NPC(ch) && !IS_NPC(gch) && gch->fighting == ch && IS_SET(gch->newbits, NEW_SINKHOLE))
      delay = 2;

    event = alloc_event();
    event->fun = &event_mobile_fighting;
    event->type = EVENT_MOBILE_FIGHTING;
    event->argument = str_dup(buf);
    add_event_char(event, ch, delay * PULSE_PER_SECOND);
  }     

  if (ch->dead)
    return TRUE;

  return FALSE;
}

/*
 * Do one group of attacks.
 */
void multi_hit(CHAR_DATA *ch, CHAR_DATA *victim, int turn)
{
  OBJ_DATA *wield = NULL;
  OBJ_DATA *wield1, *wield2, *wield3, *wield4;
  int maxcount, countup, wieldorig = 0;
  bool loop = TRUE;

  /* stunned, incapitated, mortally wounded or dead mobiles do not fight */
  if (ch->position < POS_SLEEPING)
    return;

  /* update feed data */
  update_feed(ch, victim);

  /* find all wielded weapons */
  if ((wield1 = get_eq_char(ch, WEAR_WIELD)) != NULL && wield1->item_type == ITEM_WEAPON)
    wieldorig += 1;
  if ((wield2 = get_eq_char(ch, WEAR_HOLD)) != NULL && wield2->item_type == ITEM_WEAPON)
    wieldorig += 2;
  if ((wield3 = get_eq_char(ch, WEAR_THIRD)) != NULL && wield3->item_type == ITEM_WEAPON)
    wieldorig += 4;
  if ((wield4 = get_eq_char(ch, WEAR_FOURTH)) != NULL && wield4->item_type == ITEM_WEAPON)
    wieldorig += 8;

  /* find wield weapon */
  while (loop && wieldorig != 0)
  {
    switch(number_range(1, 10))
    {
      case 1:
      case 2:
      case 3:
        if ((wield = wield1) != NULL)
          loop = FALSE;
        break;
      case 4:
      case 5:
      case 6:
        if ((wield = wield2) != NULL)
          loop = FALSE;
        break;
      case 7:
      case 8:
        if ((wield = wield3) != NULL)
          loop = FALSE;
        break;
      case 9:
      case 10:
        if ((wield = wield4) != NULL)
          loop = FALSE;
        break;
    }
  }

  /* check for special stance moves */
  if (!IS_NPC(ch) && ch->stance[0] > 0 && number_percent() == 5)
  {
    int stance = ch->stance[0];

    if (ch->stance[stance] >= 200)
    {
      special_move(ch, victim);
      return;
    }
  }

  /* Here follows all turn 1 commands and special turn 4 entry */
  if (turn == 1 || turn == 4)
  {
    one_hit(ch, victim, TYPE_UNDEFINED, 1);

    /* stop if victim is dead or stunned */
    if (victim->dead || victim->position != POS_FIGHTING || ch->fighting != victim)
      return;

    /* spell affect is cast from the wielded weapon */
    if (wield != NULL && wield->item_type == ITEM_WEAPON)
    {
      int sn;

      if (wield->value[0] >= 1)
      {
        if (wield->value[0] >= 1000)
          sn = wield->value[0] - ((wield->value[0] / 1000) * 1000);
        else
          sn = wield->value[0];

        if (victim->position == POS_FIGHTING && sn != 0)
        {
          if (sn > 0 && sn < MAX_SKILL)
            (*skill_table[sn].spell_fun) (sn, wield->level, ch, victim);
          else
            bug("Bad weapon with sn = %d.", sn);
        }

        if (victim->dead || victim->position != POS_FIGHTING || ch->fighting != victim)
          return;

        if (object_is_affected(wield, OAFF_FROSTBITE))
        {
          if ((sn = skill_lookup("frostbite")) > 0)
          {
            AFFECT_DATA paf;

            paf.type = sn;
            paf.duration = 15;
            paf.location = APPLY_HITROLL;
            paf.modifier = -10;
            paf.bitvector = 0;
            affect_to_char(victim, &paf);
          }
          else
            bug("multihit: frostbite doesn't exist.", 0);

          one_hit(ch, victim, gsn_frostbite, 0);
        }
      }
    }

    /* if victim is stunned or dead we return */
    if (victim->dead || victim->position != POS_FIGHTING || ch->fighting != victim)
      return;

    /* mobile midround attacks - defined in special.c */
    if (IS_NPC(ch) && ch->spec_fun)
      (*ch->spec_fun) (ch, "midround");

    /* if victim is stunned or dead we return */
    if (victim->dead || victim->position != POS_FIGHTING || ch->fighting != victim)
      return;
  }

  /* calculate amount of attacks */
  maxcount = number_attacks(ch, victim);

  /* players have a chance to gain an extra attack (at level 200 the chance is 100%) */
  if (!IS_NPC(ch))
  {
    int chance = 0;

    if (wield != NULL && wield->item_type == ITEM_WEAPON)
      chance = (ch->wpn[wield->value[3]]) * 0.5;
    else
      chance = (ch->wpn[0]) * 0.5;

    if (number_percent() <= chance)
      maxcount++;
  }

  /* divide the attacks on the three turns */
  {
    int residue = maxcount % 3;

    maxcount /= 3;
    if (turn == 3)
      maxcount += residue;
  }

  for (countup = 0; countup <= maxcount; countup++)
  {
    one_hit(ch, victim, TYPE_UNDEFINED, 1);

    if (victim->dead || victim->position != POS_FIGHTING || ch->fighting != victim)
      return;
  }

  /* update mobile stance every 3 seconds */
  if (IS_NPC(ch) && turn == 2)
    mob_outstance(ch);

  /* Here we add player autoattacks on turn 2 and turn 4 */
  if (!IS_NPC(ch) && (turn == 2 || turn == 4))
  {
    /* Shadow Attacks */
    if (IS_CLASS(ch, CLASS_SHADOW))
    {
      if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_KNIFESHIELD))
      {
        one_hit(ch, victim, gsn_knifespin, 1);
        one_hit(ch, victim, gsn_knifespin, 1);
        one_hit(ch, victim, gsn_knifespin, 1);
      }
      if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLOODRAGE))
      {
        one_hit(ch, victim, gsn_gutcutter, 1);
      }
    }
    else if (IS_CLASS(ch, CLASS_GIANT))
    {
      int x = 0, i;

      for (i = 1; i < ch->pcdata->powers[GIANT_RANK]; i++)
      {
        if (get_eq_char(ch, WEAR_MASTERY) == NULL)
          x = number_range(1, 3);
        else
          x = number_range(1, 4);

        if (x == 1) one_hit(ch, victim, gsn_thwack, 1);
        else if (x == 2) one_hit(ch, victim, gsn_smack, 1);
        else if (x == 3) one_hit(ch, victim, gsn_bash, 1);
        else if (x == 4) one_hit(ch, victim, gsn_crush, 1);
      }

      /* remember the last attack, for combos */
      ch->pcdata->powers[GIANT_ATTACK] = x;

      if (event_isset_mobile(ch, EVENT_PLAYER_EARTHFLUX))
      {
        if (number_percent() > 50)
          one_hit(ch, victim, gsn_lavaburst, 1);
        else
          one_hit(ch, victim, gsn_spikes, 1);
      }
    }
  }

  /* Here we add shield affects from victim */

  /* mage fireshield on turn 2 and turn 4 */
  if (IS_AFFECTED(victim, AFF_FIRESHIELD) && (turn == 2 || turn == 4))
  {
    int dam = victim->spl[RED_MAGIC] + victim->spl[BLUE_MAGIC] + victim->spl[YELLOW_MAGIC]
            + victim->spl[GREEN_MAGIC] + victim->spl[PURPLE_MAGIC] + victim->damroll;
    int dam2;

    dam = up_dam(victim, ch, dam);
    dam = cap_dam(victim, ch, dam);

    if ((dam2 = UMIN(ch->hit - 1, number_range(50 * dam / 100, dam))) > 10)
      damage(victim, ch, NULL, dam2, gsn_fireshield);

    if (!IS_NPC(ch))
    {
      if ((dam2 = UMIN(ch->hit - 1, number_range(50 * dam / 100, dam))) > 10)
        damage(victim, ch, NULL, dam2, gsn_fireshield);
      if ((dam2 = UMIN(ch->hit - 1, number_range(50 * dam / 100, dam))) > 10)
        damage(victim, ch, NULL, dam2, gsn_fireshield);
    }
  }

  /* chaos shields on turn 2 */
  if (IS_ITEMAFF(victim, ITEMA_CHAOSSHIELD) && ch->position == POS_FIGHTING && (turn == 2 || turn == 4))
  {
    int level = (IS_NPC(victim) || victim->spl[1] < 4) ? victim->level : victim->spl[1] * 0.25;
    int sn;

    if ((sn = skill_lookup("chaos blast")) > 0)
      (*skill_table[sn].spell_fun) (sn, level, victim, ch);
  }
}

int number_attacks(CHAR_DATA *ch, CHAR_DATA *victim)
{
  ITERATOR *pIter;
  int count = 2;

  if (IS_NPC(ch))
  {
    /* 1 for level 50, 100, 150, 200 and 250 */
    count += UMIN(5, ch->level / 50);

    /* 2 for level 501, 1001, 1501 and 2001 */
    count += UMIN(8, 2 * (ch->level - 1) / 500);

    /* from 0 to 15 bonus from extra_attacks */
    count += UMIN(15, ch->pIndexData->extra_attack);

    if (!IS_NPC(victim))
    {
      /* reduce attacks by 40% */
      if (IS_CLASS(victim, CLASS_GIANT) && IS_SET(victim->extra, EXTRA_WINDWALK))
      {
        count *= 3;
        count /= 5;
      }
    }

    /* groups can hit more often */
    count *= ch->gcount;

    return UMAX(2, count);
  }

  if (IS_NPC(victim))
  {
    /* stances */
    if (IS_STANCE(ch, STANCE_VIPER) && number_percent() < ch->stance[STANCE_VIPER] * 0.5)
      count += 1;
    else if (IS_STANCE(ch, STANCE_MANTIS) && number_percent() < ch->stance[STANCE_MANTIS] * 0.5)
      count += 1;
    else if (IS_STANCE(ch, STANCE_TIGER) && number_percent() < ch->stance[STANCE_TIGER] * 0.5)
      count += 1;
    else if (IS_STANCE(ch, STANCE_SPIRIT) && number_percent() < ch->stance[STANCE_SPIRIT] * 0.5)
      count += 2;

    /* items */
    if (IS_ITEMAFF(ch, ITEMA_SPEED))
      count += 2;

    /* class modifiers */
    if (!IS_NPC(ch))
    {
      if (IS_CLASS(ch, CLASS_SHADOW))
        count += ch->pcdata->powers[SHADOW_MARTIAL] / 5;
      else if (IS_CLASS(ch, CLASS_GIANT))
      {
        count += ch->pcdata->powers[GIANT_RANK] / 1.5;
        if (IS_SET(ch->pcdata->powers[EVOLVE_1], GIANT_EVOLVE_WARRIOR))
          count++;
      }
      else if (IS_CLASS(ch, CLASS_FAE))
      {
        count += ch->pcdata->powers[FAE_PLASMA] / 4;
        count += ch->pcdata->powers[FAE_MATTER] / 4;
        count += ch->pcdata->powers[FAE_ENERGY] / 4;
        count += ch->pcdata->powers[FAE_WILL] / 4;
      }
      else if (IS_CLASS(ch, CLASS_WARLOCK))
        count += ch->pcdata->powers[SPHERE_INVOCATION];
    }
  }
  else
  {
    /* stance modifiers */
    if (IS_STANCE(ch, STANCE_VIPER) && number_percent() < ch->stance[STANCE_VIPER] * 0.5)
      count += 1;
    else if (IS_STANCE(ch, STANCE_MANTIS) && number_percent() < ch->stance[STANCE_MANTIS] * 0.5)
      count += 1;
    else if (IS_STANCE(ch, STANCE_TIGER) && number_percent() < ch->stance[STANCE_TIGER] * 0.5)
      count += 1;

    /* items */
    if (IS_ITEMAFF(ch, ITEMA_SPEED))
      count += 1;

    /* class modifiers */
    if (!IS_NPC(ch))
    {
      if (IS_CLASS(ch, CLASS_SHADOW))
        count += ch->pcdata->powers[SHADOW_MARTIAL] / 5;
      else if (IS_CLASS(ch, CLASS_GIANT))
        count += ch->pcdata->powers[GIANT_RANK]/1.5;
      else if (IS_CLASS(ch, CLASS_FAE))
      {
        count += ch->pcdata->powers[FAE_PLASMA] / 4; 
        count += ch->pcdata->powers[FAE_MATTER] / 4; 
        count += ch->pcdata->powers[FAE_ENERGY] / 4; 
        count += ch->pcdata->powers[FAE_WILL] / 4; 

        /* fae rebalance */
        if (count >= 3) count--;
      }
      else if (IS_CLASS(ch, CLASS_WARLOCK))
        count += ch->pcdata->powers[SPHERE_INVOCATION];
    }

    /* some wicked randomness */
    if (number_range(1, 4) == 2)
      count -= 1;
  }

  {
    AFFECT_DATA *paf;
    int slow_sn = skill_lookup("slow spell");
    int haste_sn = skill_lookup("haste spell");

    pIter = AllocIterator(ch->affected);
    while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
    {
      if (paf->type == slow_sn)
        count--;
      else if (paf->type == haste_sn)
        count++;
    }
  }

  if (count < 1)
    count = 1;

  return count;
}

/*
 * Hit one guy once.
 */
void one_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt, int handtype)
{
  OBJ_DATA *wield;
  int victim_ac;
  int thac0;
  int dam;
  int diceroll;
  int level;
  int attack_modify;
  int right_hand;

  /*
   * Can't beat a dead char!
   * Guard against weird room-leavings.
   */
  if (victim->dead || victim->position == POS_DEAD || ch->in_room != victim->in_room)
    return;

  if (is_safe(ch, victim))
    return;

  /*
   * Figure out the type of 'damage message'.
   */
  if (handtype == 8)
  {
    wield = get_eq_char(ch, WEAR_FOURTH);
    right_hand = 8;
  }
  else if (handtype == 4)
  {
    wield = get_eq_char(ch, WEAR_THIRD);
    right_hand = 4;
  }
  else if (handtype == 2)
  {
    wield = get_eq_char(ch, WEAR_HOLD);
    right_hand = 2;
  }
  else if (handtype == 0)
  {
    wield = NULL;
    right_hand = 0;
  }
  else
  {
    if (IS_SET(ch->act, PLR_RIGHTHAND))
    {
      if (number_percent() <= 80)
      {
        wield = get_eq_char(ch, WEAR_WIELD);
        right_hand = 1;
      }
      else
      {
        wield = get_eq_char(ch, WEAR_HOLD);
        right_hand = 2;
      }
    }
    else if (IS_SET(ch->act, PLR_LEFTHAND))
    {
      if (number_percent() <= 80)
      {
        wield = get_eq_char(ch, WEAR_HOLD);
        right_hand = 2;
      }
      else
      {
        wield = get_eq_char(ch, WEAR_WIELD);
        right_hand = 1;
      }
    }
    else
    {
      if (number_percent() <= 50)
      {
        right_hand = 2;
        wield = get_eq_char(ch, WEAR_HOLD);
      }
      else
      {
        right_hand = 1;
        wield = get_eq_char(ch, WEAR_WIELD);
      }
    }
  }

  if (dt == TYPE_UNDEFINED)
  {
    dt = TYPE_HIT;

    if (!IS_NPC(ch) && IS_CLASS(ch, CLASS_FAE) &&
         IS_SET(ch->pcdata->powers[FAE_BITS], FAE_BLASTBEAMS))
      dt += 6; /* blast weapon */
    else if (wield != NULL && wield->item_type == ITEM_WEAPON)
      dt += wield->value[3];
    else if (IS_NPC(ch))
      dt += ch->pIndexData->natural_attack;
  }

  if (dt >= 1000 && dt <= 1012 && !IS_NPC(ch))
    level = URANGE(1, (ch->wpn[dt - 1000] / 5), 40);
  else
    level = 40;

  thac0     = (level * char_hitroll(ch)) / 500;
  victim_ac = URANGE(-100, char_ac(victim) / 10, -1);

  if (!can_see(ch, victim))
    victim_ac -= 4;

  if ((diceroll = number_percent()) == 100 || diceroll > UMAX(80, -40 * thac0 / victim_ac + 60))
  {
    damage(ch, victim, NULL, 0, dt);
    improve_wpn(ch, dt, right_hand);
    improve_stance(ch);
    return;
  }

  /*
   * Hit.
   * Calc damage.
   */
  if (IS_NPC(ch))
  {
    dam = number_range(ch->level / 2, ch->level * 3 / 2);

    if (wield != NULL)
    {
      dam *= 3;
      dam /= 2;
    }
  }
  else
  {
    if (IS_CLASS(ch, CLASS_FAE) && IS_SET(ch->pcdata->powers[FAE_BITS], FAE_BLASTBEAMS))
    {
      if (IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_BLASTBEAMS))
        dam = dice(40, 50);
      else
        dam = dice(2 * ch->pcdata->powers[DISC_FAE_ARCANE] + 1, 5 * ch->pcdata->powers[DISC_FAE_ARCANE] + 1);
    }
    else if (wield != NULL && wield->item_type == ITEM_WEAPON)
      dam = dice(wield->value[1], wield->value[2]);
    else
      dam = dice(4, 10);
  }

  /*
   * Bonuses.
   */
  dam += char_damroll(ch);

  if (!IS_AWAKE(victim))
  {
    if (!IS_NPC(victim) && IS_SET(victim->pcdata->powers[EVOLVE_2], SHADOW_EVOLVE_AWARENESS))
    {
      act("$n jumps to $s feet and readies $mself for combat.", victim, NULL, NULL, TO_ROOM);
      act("You wake from your slumber just before $N tries to attack you.", victim, NULL, ch, TO_CHAR);
      victim->position = POS_STANDING;
    }
    else
    {
      dam *= 2;
    }
  }

  if (!IS_NPC(ch) && dt >= TYPE_HIT)
    dam = dam + (dam * (UMIN(350, (ch->wpn[dt - 1000] + 1)) / 60));

  dam = up_dam(ch, victim, dam);

  if (dt == gsn_garotte)      dam *= number_range(2, 4);
  if (dt == gsn_backstab)     dam *= number_range(2, 4);
  if (dt == gsn_huntingstars) dam += dam/2;

  /* check immunity -- gives 5% toughness */
  if (((dt == DT_SLASH || dt == DT_SLICE)  && IS_IMMUNE(victim, IMM_SLASH)) ||
      ((dt == DT_STAB  || dt == DT_PIERCE) && IS_IMMUNE(victim, IMM_STAB)) ||
      ((dt == DT_BLAST || dt == DT_POUND)  && IS_IMMUNE(victim, IMM_SMASH)) ||
      ((dt == DT_CLAW  || dt == DT_BITE)   && IS_IMMUNE(victim, IMM_ANIMAL)) ||
      ((dt == DT_WHIP  || dt == DT_CRUSH)  && IS_IMMUNE(victim, IMM_MISC)))
  {
    dam *= 19;
    dam /= 20;
  }

  dam = cap_dam(ch, victim, dam);

  if (IS_NPC(ch))
  {
    int dam_mod = URANGE(-50, ch->pIndexData->dam_modifier, 200);

    dam *= (100 + dam_mod);
    dam /= 100;
  }

  /* randomize damage +/- 20% */
  attack_modify = number_range(0, 40);
  dam = dam * (attack_modify + 80) / 100;

  damage(ch, victim, wield, dam, dt);
  improve_wpn(ch, dt, right_hand);
  improve_stance(ch);
}

int up_dam(CHAR_DATA * ch, CHAR_DATA * victim, int dam)
{
  if (!IS_NPC(ch))
  {
    if (IS_CLASS(ch, CLASS_SHADOW))
    {
      if (IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_DPACT))
      {
        dam *= 2;
      }
      if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_SHADOWFORM)) 
      {
        dam *= 10;
        dam /= 25;
      }
    }
    else if (IS_CLASS(ch, CLASS_GIANT))
    {
      dam *= (1 + (15 * ch->pcdata->powers[GIANT_RANK] / 10));
      dam /= 4;
    }
    else if (IS_CLASS(ch, CLASS_WARLOCK))
    {
      dam *= (1 + 2 * ch->pcdata->powers[SPHERE_INVOCATION]);
      dam /= 7;
    }
    else if (IS_CLASS(ch, CLASS_FAE))
    {
      if (ch->pcdata->powers[DISC_FAE_ARCANE] >= 8)
      {
        dam *= 14;
        dam /= 10;
      }
      else if (ch->pcdata->powers[DISC_FAE_ARCANE] >= 4)
      {
        dam *= 12;
        dam /= 10;
      }
    }
  }
  return dam;
}

int cap_dam(CHAR_DATA * ch, CHAR_DATA * victim, int dam)
{
  if (is_safe(ch, victim))
    return 0;

  if (!IS_NPC(ch) && !IS_NPC(victim))
  {
    int chMight = getMight(ch), vcMight = getMight(victim), maxMight = UMAX(chMight, vcMight);

    /* damage is scaled down for lower rank fights to make them last longer */
    if (maxMight < RANK_VETERAN)
      dam *= 0.25;
    else if (maxMight < RANK_ADVENTURER)
      dam *= 0.30;
    else if (maxMight < RANK_HERO)
      dam *= 0.35;
    else if (maxMight < RANK_LEGENDARY)
      dam *= 0.40;
    else if (maxMight < RANK_MASTER)
      dam *= 0.45;
    else
      dam *= 0.5;
  }

  if (IS_NPC(ch) && dam > 2000)
    dam = 2000 + (dam - 2000) / 2;

  if (IS_ITEMAFF(victim, ITEMA_RESISTANCE))
  {
    dam *= 75;
    dam /= 100;
  }
  if (IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch))
  {
    dam *= 75;
    dam /= 100;
  }
  if (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch))
  {
    dam *= 75;
    dam /= 100;
  }

  if (!IS_NPC(victim))
  {
    dam = dam + (dam / number_range(2, 5) + number_range(10, 50));
    dam *= (number_range(2, 4) * number_range(2, 3) / number_range(4, 6));

    if (IS_CLASS(victim, CLASS_SHADOW))
    {
      if (IS_SET(victim->pcdata->powers[SHADOW_POWERS], NSHADOWS_TPACT))
      {
        dam *= 40;
        dam /= 100;
      }
      if (IS_SET(victim->pcdata->powers[SHADOW_BITS], NPOWER_SHADOWFORM))
        dam /= 2;
    }
    else if (IS_CLASS(victim, CLASS_GIANT))
    {
      if (IS_SET(victim->pcdata->powers[GIANT_GIFTS], GGIFT_LEATHERSKIN)) dam *= 0.70;
      if (victim->pcdata->powers[GIANT_STANDFIRM] == 1) 
      {
        if (victim->pcdata->powers[GIANT_POINTS] < dam / 20)
        {
          do_standfirm(victim, "");
        }
        else
        {
          victim->pcdata->powers[GIANT_POINTS] -= dam / 20;
          dam *= 0.65;
        }
      }
    }
    else if (IS_CLASS(victim, CLASS_FAE))
    {
      if (IS_SET(victim->pcdata->powers[FAE_BITS], FAE_GASEOUS))
      {
        dam *= 40;
        dam /= 100;
      }
      if (IS_SET(victim->newbits, NEW_CHAMELEON))
      {
        dam *= 11;
        dam /= 10;
      }
    }
    else if (IS_CLASS(victim, CLASS_WARLOCK))
    {
      int toughness = 8 * victim->pcdata->powers[SPHERE_ENCHANTMENT];

      if (IS_AFFECTED(victim, AFF_MVEST))
        toughness += 20;

      dam *= (100 - toughness);
      dam /= 100;
    }
  }

  /* strength and constitution mods */
  if (!IS_NPC(ch) && IS_NPC(victim))
  {
    dam *= 50 + UMAX(0, get_curr_str(ch) - 25);
    dam /= 50;
  }

  if (!IS_NPC(victim) && IS_NPC(ch))
  {
    dam *= 50 - UMAX(0, get_curr_con(victim) - 25);
    dam /= 50;

    if (dam > 100 && event_isset_mobile(victim, EVENT_PLAYER_BLOODIMMUNE))
      dam = number_range(1, 100);
  }

  if (IS_NPC(victim) && !IS_NPC(ch))
  {
    if (victim->pIndexData->toughness > 100)
      victim->pIndexData->toughness = 99;
    dam = dam * (100 - victim->pIndexData->toughness) / 100;
  }

  /* fumes makes mobiles very weak */
  if (IS_NPC(ch) && IS_SET(ch->newbits, NEW_FUMES))
  {
    dam *= 2;
    dam /= 3;
  }

  /* damage is always from 1 to 30.000 */
  dam = URANGE(1, dam, 30000);

  /* damcap */
  if (dam > ch->damcap[DAM_CAP])
    dam = number_range((ch->damcap[DAM_CAP] - 200), (ch->damcap[DAM_CAP] + 100));

  /* sanctuary */
  if (IS_AFFECTED(victim, AFF_SANCTUARY))
    dam *= 0.5;

  return dam;
}

bool can_counter(CHAR_DATA *ch)
{
  if (IS_STANCE(ch, STANCE_MONKEY))
  {
    if (number_percent() < ch->stance[STANCE_MONKEY] / 2)
      return TRUE;
  }

  return FALSE;
}

bool can_bypass(CHAR_DATA * ch)
{
  if (IS_STANCE(ch, STANCE_VIPER))
  {
    if (number_percent() < ch->stance[STANCE_VIPER] / 2)
      return TRUE;
  }
  else if (IS_STANCE(ch, STANCE_MANTIS))
  {
    if (number_percent() < ch->stance[STANCE_MANTIS] / 2)
      return TRUE;
  }
  else if (IS_STANCE(ch, STANCE_TIGER))
  {
    if (number_percent() < ch->stance[STANCE_TIGER] / 2)
      return TRUE;
  }

  return FALSE;
}

void update_damcap(CHAR_DATA *ch, CHAR_DATA *victim)
{
  int max_dam = 1000;

  if (IS_NPC(ch))
  {
    if (ch->level < 100)
      max_dam = ch->level + 200;
    else if (ch->level < 400)
      max_dam = 3 * ch->level;
    else if (ch->level < 1000)
      max_dam = 2 * ch->level + 1000;
    else
      max_dam = UMAX(3 * ch->level, 5000);
  }

  /* Special flags which modify damcap */
  if (IS_SET(ch->newbits, NEW_TENDRIL2))
    max_dam -= 300;

  /* stance related damcap bonuses */
  if (!can_counter(victim))
  {
    if (ch->stance[0] == STANCE_BULL)
      max_dam += 200;
    else if (ch->stance[0] == STANCE_DRAGON)
      max_dam += 250;
    else if (ch->stance[0] == STANCE_TIGER)
      max_dam += 200;
    else if (ch->stance[0] == STANCE_SPIRIT && IS_NPC(victim) && !IS_NPC(ch))
    {
      if (IS_SET(ch->pcdata->powers[EVOLVE_1], FAE_EVOLVE_SPIRIT))
        max_dam += 500;
      else
        max_dam += 300;
    }
  }

  if (!can_counter(ch))
  {
    if (victim->stance[0] == STANCE_CRAB)
      max_dam -= 250;
    else if (victim->stance[0] == STANCE_DRAGON)
      max_dam -= 250;
    else if (victim->stance[0] == STANCE_SWALLOW)
      max_dam -= 250;
    else if (victim->stance[0] == STANCE_SPIRIT && IS_NPC(ch) && !IS_NPC(victim))
    {
      if (IS_SET(victim->pcdata->powers[EVOLVE_1], FAE_EVOLVE_SPIRIT))
        max_dam -= 500;
      else
        max_dam -= 300;
    }
  }

  /* class stuff */
  if (!IS_NPC(ch))
  {
    max_dam += UMAX(0, (ch->generation - 6) * 50);
    max_dam += ch->pcdata->status * 10;
    max_dam += ch->pcdata->legend * 100;

    if (IS_NPC(victim))
    {
      max_dam += 100 * ch->pcdata->evolveCount;
      max_dam += UMAX(0, get_curr_str(ch) - 25) * 20;
    }

    if (IS_SET(ch->pcdata->tempflag, TEMP_ARTIFACT))
      max_dam += 250;

    if (IS_CLASS(ch, CLASS_SHADOW))
    {
      max_dam += ch->pcdata->powers[SHADOW_MARTIAL] * 70;
    }
    else if (IS_CLASS(ch, CLASS_WARLOCK))
    {
      int i;

      for (i = SPHERE_NECROMANCY; i <= SPHERE_SUMMONING; i++)
        max_dam += 50 * ch->pcdata->powers[i];
    }
    else if (IS_CLASS(ch, CLASS_GIANT))
    {
      max_dam += 200;
      max_dam += ch->pcdata->powers[GIANT_RANK] * 315;
    }
    else if (IS_CLASS(ch, CLASS_FAE))
    {
      max_dam += ch->pcdata->powers[FAE_PLASMA] * 55;
      max_dam += ch->pcdata->powers[FAE_MATTER] * 55;
      max_dam += ch->pcdata->powers[FAE_ENERGY] * 55;
      max_dam += ch->pcdata->powers[FAE_WILL] * 55;
    }
  }

  ch->damcap[DAM_CAP] = max_dam;
  ch->damcap[DAM_CHANGE] = 0;
}

/*
 * Inflict damage from a hit.
 */
void damage(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, int dam, int dt)
{
  EVENT_DATA *event;

  if (victim->position == POS_DEAD)
    return;

  if (dam < 0)
    dam = 0;

  /* update feeding */
  update_feed(ch, victim);

  /* Stop up any residual loopholes */
  if (ch->damcap[DAM_CHANGE] == 1)
    update_damcap(ch, victim);
  if (dam > ch->damcap[0])
    dam = ch->damcap[0];

  /*
   * Certain attacks are forbidden.
   * Most other attacks are returned.
   */
  if (victim != ch)
  {
    if (is_safe(ch, victim))
      return;

    if (ch->position > POS_STUNNED)
    {
      if (ch->fighting == NULL)
        set_fighting(ch, victim);
    }

    if (victim->position > POS_STUNNED)
    {
      if (victim->fighting == NULL)
        set_fighting(victim, ch);
    }

    if (victim->master == ch)
      stop_follower(victim, FALSE);

    /*
     * Check for disarm, trip, parry, and dodge.
     */
    if (dt >= TYPE_HIT)
    {
      if (IS_NPC(ch) && number_percent() < ch->level * 0.5)
        disarm(ch, victim);

      if (IS_NPC(ch) && number_percent() < ch->level * 0.5)
        trip(ch, victim);

      if (check_dodge(ch, victim, dt))
        return;

      /* stances might give additional dodge changes */
      switch(victim->stance[0])
      {
        case STANCE_SPIRIT:
          if (!IS_NPC(ch))
            break;
        case STANCE_MONGOOSE:
        case STANCE_SWALLOW:
          if (number_percent() < victim->stance[victim->stance[0]] &&
             !can_counter(ch) && !can_bypass(ch) && check_dodge(ch, victim, dt))
            return;
          break;
        default:
          break;
      }

      /* check for parry */
      if (check_parry(ch, victim, dt))
        return;

      /* stances might give additional parry changes */
      switch(victim->stance[0])
      {
        case STANCE_SPIRIT:
          if (!IS_NPC(ch))
            break;
        case STANCE_CRANE:
        case STANCE_MANTIS:
          if (number_percent() < victim->stance[victim->stance[0]] &&
             !can_counter(ch) && !can_bypass(ch) && check_parry(ch, victim, dt))
            return;
          break;
        default:
          break;
      }
    }

    if (!IS_NPC(victim))
    {
      /* the warlock backlash ability */
      if (IS_NPC(ch) && IS_CLASS(victim, CLASS_WARLOCK) && IS_SET(victim->newbits, NEW_BACKLASH))
      {
        if (number_percent() >= 70)
        {
          if ((dam = cap_dam(victim, ch, dam)) > 0)
          {
            act("#G$n's energyfield causes $N's attack to backlash.#n", victim, NULL, ch, TO_ROOM);
            act("#GYour energyfield causes $N's attack to backlash.#n", victim, NULL, ch, TO_CHAR);
            damage(victim, ch, NULL, dam, dt);
          }
          return;
        }
      }
      else if (IS_SET(victim->newbits, NEW_IRONMIND) && victim->hit < victim->max_hit / 2)
      {  
        REMOVE_BIT(victim->newbits, NEW_IRONMIND);
        send_to_char("#RYou focus your full concentration on the attack!#n\n\r", victim);
        printf_to_char(victim, "#RYou absorb #G%d hps#R from the attack, healing yourself.#n\n\r", dam);
        modify_hps(victim, dam);
        dam = 0;
      }
      else if (IS_CLASS(victim, CLASS_SHADOW) && IS_SET(victim->pcdata->powers[SHADOW_BITS], NPOWER_BLUR))
      {
        int ddodge = number_range(1, 50);

        /* we use the ddodge variable to ensure a truly double chance, look below */
        if (ddodge == 5 || ddodge == 20 || ddodge == 35 || ddodge == 50)
        {
          send_to_char("#0You feel the strike pass through your shadow body#n.\n\r", victim);
          send_to_char("#0You hit nothing but shadows#n.\n\r", ch);
          dam = 0;
        }

        /* second chance - doubles the total chance */
        if (dam > 0 && event_isset_mobile(ch, EVENT_MOBILE_BLURTENDRILS))
        {
          if (ddodge == 10 || ddodge == 15 || ddodge == 30 || ddodge == 45)
          {
            send_to_char("#0You feel the strike pass through your shadow body#n.\n\r", victim);
            send_to_char("#0You hit nothing but shadows#n.\n\r", ch);
            dam = 0;
          }
        }
      }
      else if (IS_AFFECTED(victim, AFF_SHATTERSHIELD) && number_range(1, 5) == 2)
      {
        act("$N's shattershield deflects $n's attack.", ch, NULL, victim, TO_NOTVICT);
        act("Your shattershield deflects $n's attack.", ch, NULL, victim, TO_VICT);
        act("$N's shattershield deflects your attack.", ch, NULL, victim, TO_CHAR);

        if (number_range(1, 3) == 2)
        {
          act("$n's shattershield breaks into a thousand pieces.", victim, NULL, NULL, TO_ROOM);
          act("Your shattershield breaks into a thousand pieces.", victim, NULL, NULL, TO_CHAR);
          REMOVE_BIT(victim->affected_by, AFF_SHATTERSHIELD);
          WAIT_STATE(victim, 12);
        }
      }
    }
  }

  if (dam > 0)
  {
    dam_message(ch, victim, obj, dam, dt);

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

    hurt_person(ch, victim, dam);

    if (victim->dead || victim->in_room == NULL)
      return;

    if (IS_CLASS(victim, CLASS_WARLOCK) && number_percent() >= 60 &&
        IS_SET(victim->newbits, NEW_MOUNTAIN) && obj != NULL)
    {
      int odam = number_range(90, 125);

      act("$p takes a serious dent as it strikes $n's rocky skin.", victim, obj, NULL, TO_ROOM);
      act("$p takes a serious dent as it strikes your rocky skin.", victim, obj, NULL, TO_CHAR);

      damage_obj(ch, obj, odam);
    }

    if (number_range(1, 3) == 2 && (event = event_isset_mobile(victim, EVENT_PLAYER_CATALYST)) != NULL)
    {
      char buf[MAX_STRING_LENGTH];
      int gain = atoi(event->argument) + dam / 3;

      if (gain < victim->max_mana)
      {
        sprintf(buf, "%d", gain);

        free_string(event->argument);
        event->argument = str_dup(buf);
      }
      else
      {
        send_to_char("Your catalyst overloads and explodes.\n\r", ch);
        dequeue_event(event, TRUE);
      }
    }
  }

  dropinvis(ch);
  dropinvis(victim);
}

void hurt_person(CHAR_DATA *ch, CHAR_DATA *victim, int dam)
{
  EVENT_DATA *event;
  ITERATOR *pIter;

  victim->hit -= dam;

  /* the fae anti-attack shield */
  if (!IS_NPC(victim) && IS_CLASS(victim, CLASS_FAE))
    fae_shield(ch, victim, dam);

  if (!IS_NPC(victim) && victim->level >= 12 && victim->hit < 1)
    victim->hit = 1;

  update_pos(victim);

  switch (victim->position)
  {
    case POS_MORTAL:
      act("$n is mortally wounded, and spraying blood everywhere.", victim, NULL, NULL, TO_ROOM);
      send_to_char("You are mortally wounded, and spraying blood everywhere.\n\r", victim);

      if (!IS_NPC(victim) && IS_CLASS(victim, CLASS_SHADOW) && (event = event_isset_mobile(victim, EVENT_PLAYER_WITNESS)) != NULL)
      {
        dequeue_event(event, TRUE);

        event = alloc_event();
        event->fun = &event_player_witnessgrab;
        event->type = EVENT_PLAYER_WITNESSGRAB;
        add_event_char(event, victim, 1 * PULSE_PER_SECOND + 1);
      }
      break;
    case POS_INCAP:
      act("$n is incapacitated, and bleeding badly.", victim, NULL, NULL, TO_ROOM);
      send_to_char("You are incapacitated, and bleeding badly.\n\r", victim);
      break;
    case POS_STUNNED:
      act("$n is stunned, but will soon recover.", victim, NULL, NULL, TO_ROOM);
      send_to_char("You are stunned, but will soon recover.\n\r", victim);
      break;
    case POS_DEAD:
      act("$n is DEAD!!", victim, 0, 0, TO_ROOM);
      send_to_char("You have been KILLED!!\n\r\n\r", victim);
      break;
    default:
      if (dam > victim->max_hit / 4)
        send_to_char("That really did HURT!\n\r", victim);
      if (victim->hit < victim->max_hit / 4 && dam > 0)
          send_to_char("You sure are BLEEDING!\n\r", victim);
      break;
  }

  /*
   * Sleep spells and extremely wounded folks.
   */
  if (!IS_AWAKE(victim))
    stop_fighting(victim, FALSE);

  /*
   * Payoff for killing things.
   */
  if (victim->hit <= 0 && IS_NPC(victim))
  {
    group_gain(ch, victim);
    victim->position = POS_DEAD;

    /*
     * quest updating
     */
    if (!IS_NPC(ch))
    {
      QUEST_DATA *quest;

      pIter = AllocIterator(ch->pcdata->quests);
      while ((quest = (QUEST_DATA *) NextInList(pIter)) != NULL)
      {
        if (quest->type == QT_MOB || quest->type == QT_MOB_AND_OBJ)
        {
          if (quest->vnums[0] == victim->pIndexData->vnum)
          {
            quest->vnums[0] = -1;
            send_to_char("#GYou have fulfilled a quest.#n\n\r", ch);
            break;
          }
        }
      }
    }
  }

  if (victim->position == POS_DEAD)
  {
    if (IS_NPC(victim) && !IS_NPC(ch))
    {
      ch->mkill++;
      ch->pcdata->session->mkills++;
    }

    raw_kill(victim, ch);

    if (IS_SET(ch->act, PLR_AUTOLOOT))
      do_get(ch, "all corpse");
    else
      do_look(ch, "in corpse");

    if (!IS_NPC(ch) && IS_NPC(victim))
    {
      if (IS_SET(ch->act, PLR_AUTOSAC))
      {
        if (IS_CLASS(ch, CLASS_WARLOCK) && ch->pcdata->powers[SPHERE_NECROMANCY] >= 1)
          do_corpsedrain(ch, "");
        else
          do_sacrifice(ch, "corpse");
      }
    }

    return;
  }

  if (victim == ch)
    return;

  if (victim->hit < victim->max_hit / 4 && victim->hit > 0 && !IS_NPC(victim) && IS_CLASS(victim, CLASS_WARLOCK) && SizeOfList(victim->pcdata->contingency) > 0)
  {
    discharge_contingency(victim);
  }
}

bool is_safe(CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (!ch->in_room || ch->dead)
    return TRUE;

  if (!victim->in_room || victim->dead)
    return TRUE;

  if (!IS_NPC(victim) && !IS_NPC(ch))
  {
    if (ch->level < 3 || victim->level < 3)
    {
      send_to_char("Both players must be avatars to fight.\n\r", ch);
      return TRUE;
    }
    if (ch->pcdata->safe_counter > 0)
    {
      send_to_char("You still have a few safe-ticks left.\n\r", ch);
      return TRUE;
    }
    if (victim->pcdata->safe_counter > 0)
    {
      send_to_char("They still have a few safe-ticks left.\n\r", ch);
      return TRUE;
    }
  }
  if (victim->shop_fun || victim->quest_fun)
  {
    send_to_char("That mobile is protected by the gods.\n\r", ch);
    return TRUE;
  }
  if (IS_AFFECTED(ch, AFF_ETHEREAL) && !IS_AFFECTED(victim, AFF_ETHEREAL))
  {
    send_to_char("You cannot while ethereal.\n\r", ch);
    return TRUE;
  }
  if (IS_AFFECTED(victim, AFF_ETHEREAL) && !IS_AFFECTED(ch, AFF_ETHEREAL))
  {
    send_to_char("You cannot fight an ethereal person.\n\r", ch);
    return TRUE;
  }
  if (IS_SET(ch->newbits, NEW_SHADOWPLANE) && !IS_SET(victim->newbits, NEW_SHADOWPLANE))
  {
    send_to_char("You cannot attack across planes.\n\r", ch);
    return TRUE;
  }
  if (IS_SET(victim->newbits, NEW_SHADOWPLANE) && !IS_SET(ch->newbits, NEW_SHADOWPLANE))
  {
    send_to_char("You cannot attack across planes.\n\r", ch);
    return TRUE;
  }
  if (IS_SET(ch->newbits, NEW_MUDFORM))
  {
    send_to_char("Mud cannot hurt anything.\n\r", ch);
    return TRUE;
  }
  if (IS_SET(victim->newbits, NEW_MUDFORM))
  {
    send_to_char("Attacking a puddle of mud doesn't make sense.\n\r", ch);
    return TRUE;
  }
  if (IS_SET(victim->extra, EXTRA_AFK))
  {
    send_to_char("They are AFK!\n\r", ch);
    return TRUE;
  }
  if (!IS_NPC(victim) && victim->desc == NULL && victim->timer > 1 && victim->fight_timer == 0 && !in_fortress(ch))
  {
    /* Timer check to avoid people going ld in the first round. */
    send_to_char("Nooo, they are linkdead.\n\r", ch);
    return TRUE;
  }
  if (victim->fight_timer > 0)
    return FALSE;

  if (IS_SET(ch->in_room->room_flags, ROOM_SAFE) && !ragnarok)
  {
    send_to_char("You cannot fight in a safe room.\n\r", ch);
    return TRUE;
  }

  /* ragnarok pseudo safe mode */
  if (!IS_NPC(ch) && !IS_NPC(victim) && ragnarok)
  {
    if (getRank(victim, 0) < getRank(ch, 0) - 3)
    {
      send_to_char("(ragnarok) You cannot attack them.\n\r", ch);
      return TRUE;
    }
    if (getRank(ch, 0) < getRank(victim, 0) + 3)
    {
      send_to_char("(ragnarok) You cannot attack them.\n\r", ch);
      return TRUE;
    }
  }

  return FALSE;
}

/*
 * Check for parry.
 */
bool check_parry(CHAR_DATA *ch, CHAR_DATA* victim, int dt)
{
  OBJ_DATA *obj = NULL;
  int chance = 0;

  /* you cannot parry while sleeping */
  if (!IS_AWAKE(victim))
    return FALSE;

  /* can only parry actual weapon attacks */
  if (dt < DT_UNARMED || dt > DT_SUCK)
    return FALSE;

  /* find the weapon used for parry'ing (and take care of mobs and blastbeams) */
  if (IS_NPC(victim))
    obj = NULL;
  else if (!IS_NPC(victim) && IS_CLASS(victim, CLASS_FAE) &&
            IS_SET(victim->pcdata->powers[FAE_BITS], FAE_BLASTBEAMS))
    obj = NULL;
  else
  {
    if ((obj = get_eq_char(victim, WEAR_WIELD)) == NULL || obj->item_type != ITEM_WEAPON)
    {
      if ((obj = get_eq_char(victim, WEAR_HOLD)) == NULL || obj->item_type != ITEM_WEAPON)
        return FALSE;
    }
  }

  /* get base anti-parry from attacker */
  if (!IS_NPC(ch))
  {
    chance -= (ch->wpn[dt - 1000] * 0.1);
  }
  else
  {
    if (IS_SET(ch->newbits, NEW_FUMES))
      chance -= (ch->level * 0.17);
    else if (ch->precognition == victim)  /* warlock precognition */
      chance -= (ch->level * 0.17);
    else if (IS_CLASS(victim, CLASS_FAE) && IS_SET(victim->newbits, NEW_FAEHALO))
      chance -= (ch->level * 0.17);
    else if (event_isset_mobile(ch, EVENT_MOBILE_PSPRAY))
      chance -= (ch->level * 0.17);
    else
      chance -= (ch->level * 0.20);

    chance -= ch->pIndexData->extra_parry;

    if (ch->gcount > 1)
    {
      chance *= 10 + ch->gcount - 1;
      chance /= 10;
    }
  }

  /* get base parry from victim */
  if (!IS_NPC(victim))
  {
    chance += (victim->wpn[dt - 1000] * 0.5);
  }
  else
  {
    chance += UMIN(100, victim->level) + UMAX(0, (victim->level - 100) / 10);
    chance += victim->pIndexData->extra_parry;
  }

  /* crane, mantis and spirit stance gives modifiers to parry */
  if (IS_STANCE(victim, STANCE_CRANE) && !can_counter(ch) && !can_bypass(ch))
    chance += (victim->stance[STANCE_CRANE] * 0.25);
  else if (IS_STANCE(victim, STANCE_MANTIS) && !can_counter(ch) && !can_bypass(ch))
    chance += (victim->stance[STANCE_MANTIS] * 0.25);
  else if (!IS_NPC(victim) && IS_STANCE(victim, STANCE_SPIRIT) && IS_NPC(ch) && !can_counter(ch) && !can_bypass(ch))
  {
    if (IS_SET(victim->pcdata->powers[EVOLVE_1], FAE_EVOLVE_SPIRIT))
      chance += victim->stance[STANCE_SPIRIT] / 5;
    else
      chance += victim->stance[STANCE_SPIRIT] / 8;
  }

  /* get class modifiers from attacker */
  if (!IS_NPC(ch))
  {
    if (IS_CLASS(ch, CLASS_SHADOW))
    {
      chance -= ch->pcdata->powers[SHADOW_MARTIAL] * 2;
    }
    else if (IS_CLASS(ch, CLASS_GIANT))
    {
      chance -= (40 * ch->pcdata->powers[GIANT_ATT]) / 10;
      if (event_isset_mobile(ch, EVENT_PLAYER_DEATHFRENZY))
        chance -= 20;
    }
    else if (IS_CLASS(ch, CLASS_FAE))
    {
      chance -= 15;
      chance -= ch->pcdata->powers[FAE_PLASMA];
      chance -= ch->pcdata->powers[FAE_MATTER];
      chance -= ch->pcdata->powers[FAE_ENERGY];
      chance -= ch->pcdata->powers[FAE_WILL];
    }
    else if (IS_CLASS(ch, CLASS_WARLOCK))
    {
      chance -= ch->pcdata->powers[SPHERE_INVOCATION] * 8;
    }
    else if (in_newbiezone(ch))
    {
      chance -= 20;
    }
  }

  /* get class modifiers from victim */
  if (!IS_NPC(victim))
  {
    if (IS_CLASS(victim, CLASS_SHADOW))
    {
      chance += victim->pcdata->powers[SHADOW_MARTIAL] * 2;
    }
    else if (IS_CLASS(victim, CLASS_GIANT))
    {
      chance += (35 * victim->pcdata->powers[GIANT_DEF]) / 10;
    }
    else if (IS_CLASS(victim, CLASS_FAE))
    {
      chance += 15;
      chance += victim->pcdata->powers[FAE_PLASMA];
      chance += victim->pcdata->powers[FAE_MATTER];
      chance += victim->pcdata->powers[FAE_ENERGY];
      chance += victim->pcdata->powers[FAE_WILL];
    }
    else if (IS_CLASS(victim, CLASS_WARLOCK))
    {
      chance += victim->pcdata->powers[SPHERE_ABJURATION] * 5;

      if (IS_AFFECTED(victim, AFF_WALLSWORDS))
        chance += 15;
    }
    else if (in_newbiezone(victim))
    {
      chance += 20;
    }
  }

  /* giant warrior nerve pinch */
  if (is_affected(victim, skill_lookup("nerve pinch")))
    chance -= 20;

  /* add dex bonus to victim */
  if (!IS_NPC(victim) && IS_NPC(ch))
  {
    chance += UMAX(0, get_curr_dex(victim) - 25);
  }

  /* fix the actual chance to make a parry */
  chance = URANGE(20, chance, 80);

  /* check to see if the parry fails */
  if (number_percent() > chance)
    return FALSE;

  if (!IS_NPC(victim) && obj != NULL && obj->item_type == ITEM_WEAPON && obj->value[3] >= 0 && obj->value[3] <= 12)
  {
    if (!victim->pcdata->brief[BRIEF_3])
    {
      if (IS_AFFECTED(victim, AFF_WALLSWORDS))
        act("You parry $n's blow with your wall of swords.", ch, NULL, victim, TO_VICT);
      else
        act("You parry $n's blow with $p.", ch, obj, victim, TO_VICT);
    }
    if (!IS_NPC(ch) && !ch->pcdata->brief[BRIEF_3])
    {
      if (IS_AFFECTED(victim, AFF_WALLSWORDS))
        act("$N parries your blow with a wall of swords.", ch, NULL, victim, TO_CHAR);
      else
        act("$N parries your blow with $p.", ch, obj, victim, TO_CHAR);
    }
    return TRUE;
  }

  if (IS_NPC(victim) || !victim->pcdata->brief[BRIEF_3])
  {
    if (IS_AFFECTED(victim, AFF_WALLSWORDS))
      act("You parry $n's blow with your wall of swords.", ch, NULL, victim, TO_VICT);
    else
      act("You parry $n's attack.", ch, NULL, victim, TO_VICT);
  }
  if (IS_NPC(ch) || !ch->pcdata->brief[BRIEF_3])
  {
    if (IS_AFFECTED(victim, AFF_WALLSWORDS))
      act("$N parries your blow with a wall of swords.", ch, NULL, victim, TO_CHAR);
    else
      act("$N parries your attack.", ch, NULL, victim, TO_CHAR);
  }

  return TRUE;
}

/*
 * Check for dodge.
 */
bool check_dodge(CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
  ITERATOR *pIter;
  int chance = 0, sn;

  /* cannot dodge while sleeping */
  if (!IS_AWAKE(victim))
    return FALSE;

  /* attacker base bonus */
  if (!IS_NPC(ch))
  {
    chance -= (ch->wpn[dt - 1000] * 0.1);
  }
  else
  {
    if (IS_SET(ch->newbits, NEW_FUMES))
      chance -= (ch->level * 0.17);
    else if (ch->precognition == victim)  /* warlock precognition */
      chance -= (ch->level * 0.17);
    else if (IS_CLASS(victim, CLASS_FAE) && IS_SET(victim->newbits, NEW_FAEHALO))
      chance -= (ch->level * 0.17);
    else if (event_isset_mobile(ch, EVENT_MOBILE_PSPRAY))
      chance -= (ch->level * 0.17);
    else
      chance -= (ch->level * 0.20);

    chance -= ch->pIndexData->extra_dodge;

    if (ch->gcount > 1)
    {
      chance *= 10 + ch->gcount - 1;
      chance /= 10;
    }
  }

  /* defender base bonus */
  if (!IS_NPC(victim))
  {
    chance += (victim->wpn[0] * 0.5);
  }
  else
  {
    chance += UMIN(80, victim->level) + UMAX(0, (victim->level - 80) / 10);
    chance += victim->pIndexData->extra_dodge;
  }

  /* mongoose, swallow and spirit gives a bonus to dodge */
  if (IS_STANCE(victim, STANCE_MONGOOSE) && !can_counter(ch) && !can_bypass(ch))
    chance += victim->stance[STANCE_MONGOOSE] / 4;
  else if (IS_STANCE(victim, STANCE_SWALLOW) && !can_counter(ch) && !can_bypass(ch))
    chance += victim->stance[STANCE_SWALLOW] / 4;
  else if (!IS_NPC(victim) && IS_STANCE(victim, STANCE_SPIRIT) && IS_NPC(ch) && !can_counter(ch) && !can_bypass(ch))
  {
    if (IS_SET(victim->pcdata->powers[EVOLVE_1], FAE_EVOLVE_SPIRIT))
      chance += victim->stance[STANCE_SPIRIT] / 5;
    else
      chance += victim->stance[STANCE_SPIRIT] / 8;
  }

  /* victim tendril'ed makes it harder to dodge */
  if (IS_SET(victim->newbits, NEW_TENDRIL3))
    chance -= 25;

  /* special mindflayer spell, mind wreck */
  if ((sn = skill_lookup("mind wreck")) > 0)
  {
    AFFECT_DATA *paf;
    int penalty = 0;

    pIter = AllocIterator(victim->affected);
    while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
    {
      if (paf->type == sn)
        penalty += 4;
    }

    chance -= penalty;
  }
  else
  {
    /* then remove this entire block of code */
    bug("check_dodge: mind wreck spell no longer exists.", 0);
  }

  /* class stuff */
  if (!IS_NPC(ch))
  {
    if (IS_CLASS(ch, CLASS_SHADOW))
    {
      chance -= ch->pcdata->powers[SHADOW_MARTIAL] * 2;
    }
    else if (IS_CLASS(ch, CLASS_GIANT))
    {
      chance -= (40 * ch->pcdata->powers[GIANT_ATT]) / 10;
      if (event_isset_mobile(ch, EVENT_PLAYER_DEATHFRENZY))
        chance -= 20;
    }
    else if (IS_CLASS(ch, CLASS_FAE))
    {
      chance -= 15;
      chance -= ch->pcdata->powers[FAE_PLASMA];
      chance -= ch->pcdata->powers[FAE_MATTER];  
      chance -= ch->pcdata->powers[FAE_ENERGY];
      chance -= ch->pcdata->powers[FAE_WILL];
    }
    else if (IS_CLASS(ch, CLASS_WARLOCK))
    {
      chance -= ch->pcdata->powers[SPHERE_INVOCATION] * 8;
    }
    else if (in_newbiezone(ch))
    {
      chance -= 20;
    }
  }

  if (!IS_NPC(victim))
  {
    if (IS_CLASS(victim, CLASS_SHADOW))
    {
      chance += victim->pcdata->powers[SHADOW_MARTIAL] * 2;
    }
    else if (IS_CLASS(victim, CLASS_GIANT))
    {
      chance += (35 * victim->pcdata->powers[GIANT_DEF]) / 10;

      if (IS_NPC(ch))
      {
        if (IS_SET(victim->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_KABALISTIC) ||
            IS_SET(victim->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_TORTOISE) ||
            IS_SET(victim->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_SPECTRAL) ||
            IS_SET(victim->pcdata->powers[EVOLVE_3], GIANT_EVOLVE_VOODOO))
          chance += 15;
      }
    }
    else if (IS_CLASS(victim, CLASS_FAE))
    {
      chance += 15;
      chance += victim->pcdata->powers[FAE_PLASMA];
      chance += victim->pcdata->powers[FAE_MATTER];
      chance += victim->pcdata->powers[FAE_ENERGY];
      chance += victim->pcdata->powers[FAE_WILL];
    }
    else if (IS_CLASS(victim, CLASS_WARLOCK))
    {
      chance += victim->pcdata->powers[SPHERE_ABJURATION] * 8;

      if (IS_NPC(ch) && victim->pcdata->powers[SPHERE_ABJURATION] > 5)
        chance += 8;
    }
    else if (in_newbiezone(victim))
    {
      chance += 20;
    }
  }

  /* giant warrior nerve pinch */
  if (is_affected(victim, skill_lookup("nerve pinch")))
    chance -= 20;

  /* trying to flee */
  if (event_isset_mobile(victim, EVENT_MOBILE_FLEE))
    chance -= 5;

  /* add dex bonus to victim */
  if (!IS_NPC(victim) && IS_NPC(ch))
  {
    chance += UMAX(0, get_curr_dex(victim) - 25);
  }

  /* cutting corners */
  chance = URANGE(20, chance, 80);

  if (number_percent() >= chance)
    return FALSE;

  if (IS_NPC(victim) || !victim->pcdata->brief[BRIEF_3])
    act("You dodge $n's attack.", ch, NULL, victim, TO_VICT);
  if (IS_NPC(ch) || !ch->pcdata->brief[BRIEF_3])
    act("$N dodges your attack.", ch, NULL, victim, TO_CHAR);

  return TRUE;
}

/*
 * Set position of a victim.
 */
void update_pos(CHAR_DATA *victim)
{
  CHAR_DATA *mount;

  if (victim->hit > 0)
  {
    if (victim->position <= POS_STUNNED)
    {
      victim->position = POS_STANDING;

      if (IS_NPC(victim) || victim->max_hit * 0.25 > victim->hit)
      {
        act("$n clambers back to $s feet.", victim, NULL, NULL, TO_ROOM);
        act("You clamber back to your feet.", victim, NULL, NULL, TO_CHAR);
      }
      else
      {
        act("$n flips back up to $s feet.", victim, NULL, NULL, TO_ROOM);
        act("You flip back up to your feet.", victim, NULL, NULL, TO_CHAR);
      }

      /* this is a dirty hack to clear the spell queue */
      strip_event_mobile(victim, EVENT_MOBILE_CASTING);
    }
    return;
  }
  else if ((mount = victim->mount) != NULL)
  {
    if (victim->mounted == IS_MOUNT)
    {
      act("$n rolls off $N.", mount, NULL, victim, TO_ROOM);
      act("You roll off $N.", mount, NULL, victim, TO_CHAR);
    }
    else if (victim->mounted == IS_RIDING)
    {
      act("$n falls off $N.", victim, NULL, mount, TO_ROOM);
      act("You fall off $N.", victim, NULL, mount, TO_CHAR);
    }
    mount->mount = NULL;
    victim->mount = NULL;
    mount->mounted = IS_ON_FOOT;
    victim->mounted = IS_ON_FOOT;
  }

  if (!IS_NPC(victim) && victim->hit < -10)
  {
    victim->hit = -10;

    if (victim->position == POS_FIGHTING)
      stop_fighting(victim, TRUE);
    return;
  }

  if (IS_NPC(victim) && victim->hit < -6)
  {
    victim->position = POS_DEAD;
    return;
  }

  if (victim->hit <= -6)
    victim->position = POS_MORTAL;
  else if (victim->hit <= -3)
    victim->position = POS_INCAP;
  else
    victim->position = POS_STUNNED;

  return;
}

/*
 * Start fights.
 */
void set_fighting(CHAR_DATA *ch, CHAR_DATA *victim)
{
  EVENT_DATA *event;

  if (!event_isset_mobile(victim, EVENT_MOBILE_FIGHTING))
  {
    event = alloc_event();
    event->fun = &event_mobile_fighting;
    event->type = EVENT_MOBILE_FIGHTING;
    event->argument = str_dup("0");
    add_event_char(event, victim, PULSE_PER_SECOND);
  }
  if (!event_isset_mobile(ch, EVENT_MOBILE_FIGHTING))
  {
    event = alloc_event();
    event->fun = &event_mobile_fighting;
    event->type = EVENT_MOBILE_FIGHTING;
    event->argument = str_dup("0");
    add_event_char(event, ch, PULSE_PER_SECOND);
  }

  autodrop(victim, ch);
  dropinvis(victim);

  if (ch->fighting == NULL)
  {
    ch->fighting = victim;
    ch->position = POS_FIGHTING;
    ch->damcap[DAM_CHANGE] = 1;
    autodrop(ch, victim);
    dropinvis(ch);
  }
}

void stop_fighting(CHAR_DATA * ch, bool fBoth)
{
  CHAR_DATA *fch;
  ITERATOR *pIter;

  pIter = AllocIterator(char_list);
  while ((fch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (fch == ch || (fBoth && fch->fighting == ch))
    {
      fch->fighting = NULL;
      fch->position = POS_STANDING;
      update_pos(fch);
    }
  }
}

/*
 * Make a corpse out of a character.
 */
void make_corpse(CHAR_DATA *ch, CHAR_DATA *killer)
{
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *corpse;
  OBJ_DATA *obj;
  ITERATOR *pIter;
  char *name;

  /* error checking */
  if (!ch || !ch->in_room) return;

  if (IS_NPC(ch))
  {
    if (IS_SET(ch->newbits, NEW_BANDED))
      name = ch->pIndexData->short_descr;
    else
      name = ch->short_descr;

    corpse = create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
    object_decay(corpse, number_range(15, 25));
    corpse->value[2] = ch->pIndexData->vnum;

    /* try and pop random equipment goodies - none kingdom mobiles only */
    if (!IS_NPC(killer) && ch->pIndexData->vnum < ROOM_VNUM_KINGDOMHALLS)
    {
      if (ch->level >= 200)
      {
        if ((obj = pop_rand_equipment(getMobMight(ch->pIndexData), FALSE)) != NULL)
        {
          obj_to_obj(obj, corpse);

          if (getMobMight(ch->pIndexData) > 400 && ((IS_CLASS(killer, CLASS_FAE) && number_range(1, 4) == 2) || number_range(1, 10) == 4))
          {
            char *spellname;
            int sn;

            switch(number_range(1, 5))
            {
              default:
                spellname = "heroism";
                sn = skill_lookup("heroism");
                break;
              case 2:
                spellname = "nimbleness";
                sn = skill_lookup("nimbleness");
                break;
              case 3:
                spellname = "endurance";
                sn = skill_lookup("endurance");
                break;
              case 4:
                spellname = "omniscience";
                sn = skill_lookup("omniscience");
                break;
              case 5:
                spellname = "brilliance";
                sn = skill_lookup("brilliance");
                break;
            }

            if (sn <= 0)
            {
              bug("make_corpse: bad spell.", 0);
            }
            else
            {
              obj = create_object(get_obj_index(OBJ_VNUM_PROTOPLASM), 50);
              obj->weight = 1;
              obj->item_type = ITEM_FAETOKEN;
              free_string(obj->name);
              free_string(obj->short_descr);
              free_string(obj->description);
              sprintf(buf, "faetoken token %s", spellname);
              obj->name = str_dup(buf);
              sprintf(buf, "a faetoken of %s", spellname);
              obj->short_descr = str_dup(buf);
              obj->description = str_dup("A tiny faetoken lies here.");
              obj->value[0] = sn;
              obj_to_obj(obj, corpse);
            }
          }
        }
      }
      else if (ch->level >= 50)
      {
        if ((obj = pop_rand_loweq()) != NULL)
          obj_to_obj(obj, corpse);
      }
    }
  }
  else
  {
    name = ch->name;
    corpse = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
    object_decay(corpse, number_range(120, 180));
  }
  sprintf(buf, corpse->short_descr, name);
  free_string(corpse->short_descr);
  corpse->short_descr = str_dup(buf);
  sprintf(buf, corpse->description, name);
  free_string(corpse->description);
  corpse->description = str_dup(buf);

  pIter = AllocIterator(ch->carrying);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    obj_from_char(obj);
    if (IS_SET(obj->extra_flags, ITEM_VANISH))
      extract_obj(obj);
    else
    {
      obj_to_obj(obj, corpse);
    }
  }
  obj_to_room(corpse, ch->in_room);
}

void make_part(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int vnum;

  argument = one_argument(argument, arg);
  vnum = 0;

  if (arg[0] == '\0')
    return;
  if (ch->in_room == NULL)
    return;

  if (!str_cmp(arg, "head"))
    vnum = OBJ_VNUM_SEVERED_HEAD;
  else if (!str_cmp(arg, "arm"))
    vnum = OBJ_VNUM_SLICED_ARM;
  else if (!str_cmp(arg, "leg"))
    vnum = OBJ_VNUM_SLICED_LEG;
  else if (!str_cmp(arg, "heart"))
    vnum = OBJ_VNUM_TORN_HEART;
  else if (!str_cmp(arg, "turd"))
    vnum = OBJ_VNUM_TORN_HEART;
  else if (!str_cmp(arg, "entrails"))
    vnum = OBJ_VNUM_SPILLED_ENTRAILS;
  else if (!str_cmp(arg, "brain"))
    vnum = OBJ_VNUM_QUIVERING_BRAIN;
  else if (!str_cmp(arg, "eyeball"))
    vnum = OBJ_VNUM_SQUIDGY_EYEBALL;
  else if (!str_cmp(arg, "blood"))
    vnum = OBJ_VNUM_SPILT_BLOOD;
  else if (!str_cmp(arg, "face"))
    vnum = OBJ_VNUM_RIPPED_FACE;
  else if (!str_cmp(arg, "windpipe"))
    vnum = OBJ_VNUM_TORN_WINDPIPE;
  else if (!str_cmp(arg, "cracked_head"))
    vnum = OBJ_VNUM_CRACKED_HEAD;
  else if (!str_cmp(arg, "ear"))
    vnum = OBJ_VNUM_SLICED_EAR;
  else if (!str_cmp(arg, "nose"))
    vnum = OBJ_VNUM_SLICED_NOSE;
  else if (!str_cmp(arg, "tooth"))
    vnum = OBJ_VNUM_KNOCKED_TOOTH;
  else if (!str_cmp(arg, "tongue"))
    vnum = OBJ_VNUM_TORN_TONGUE;
  else if (!str_cmp(arg, "hand"))
    vnum = OBJ_VNUM_SEVERED_HAND;
  else if (!str_cmp(arg, "foot"))
    vnum = OBJ_VNUM_SEVERED_FOOT;
  else if (!str_cmp(arg, "thumb"))
    vnum = OBJ_VNUM_SEVERED_THUMB;
  else if (!str_cmp(arg, "index"))
    vnum = OBJ_VNUM_SEVERED_INDEX;
  else if (!str_cmp(arg, "middle"))
    vnum = OBJ_VNUM_SEVERED_MIDDLE;
  else if (!str_cmp(arg, "ring"))
    vnum = OBJ_VNUM_SEVERED_RING;
  else if (!str_cmp(arg, "little"))
    vnum = OBJ_VNUM_SEVERED_LITTLE;
  else if (!str_cmp(arg, "toe"))
    vnum = OBJ_VNUM_SEVERED_TOE;

  if (vnum != 0)
  {
    OBJ_DATA *obj;
    char *name;

    name = IS_NPC(ch) ? ch->short_descr : ch->name;
    obj = create_object(get_obj_index(vnum), 0);

    if (str_cmp(arg, "heart") || IS_NPC(ch))
      object_decay(obj, number_range(10, 20));

    if (!str_cmp(arg, "head") && IS_NPC(ch))
      obj->value[1] = ch->pIndexData->vnum;

    if (!IS_NPC(ch))
    {
      sprintf(buf, obj->name, name);
      free_string(obj->name);
      obj->name = str_dup(buf);
    }
    else
    {
      sprintf(buf, obj->name, "mob");
      free_string(obj->name);
      obj->name = str_dup(buf);
    }
    sprintf(buf, obj->short_descr, name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(buf);
    sprintf(buf, obj->description, name);
    free_string(obj->description);
    obj->description = str_dup(buf);

    obj_to_room(obj, ch->in_room);
  }
  else
  {
    sprintf(buf, "make_part on unknown part '%s'.", arg);
    bug(buf, 0);
  }
}

void raw_kill(CHAR_DATA *victim, CHAR_DATA *killer)
{
  CHAR_DATA *mount;
  ITERATOR *pIter;
  AFFECT_DATA *paf;

  if (victim->gcount <= 1 || !IS_NPC(victim))
    stop_fighting(victim, TRUE);

  make_corpse(victim, killer);

  if ((mount = victim->mount) != NULL)
  {
    if (victim->mounted == IS_MOUNT)
    {
      act("$n rolls off the corpse of $N.", mount, NULL, victim, TO_ROOM);
      act("You roll off the corpse of $N.", mount, NULL, victim, TO_CHAR);
    }
    else if (victim->mounted == IS_RIDING)
    {
      act("$n falls off $N.", victim, NULL, mount, TO_ROOM);
      act("You fall off $N.", victim, NULL, mount, TO_CHAR);
    }
    mount->mount = NULL;
    victim->mount = NULL;
    mount->mounted = IS_ON_FOOT;
    victim->mounted = IS_ON_FOOT;
  }

  if (IS_NPC(victim))
  {
    if (victim->death_fun && killer && killer != victim)
    {
      (*victim->death_fun)(victim, killer);
    }
    victim->pIndexData->killed++;
    kill_table[URANGE(0, victim->level, MAX_LEVEL - 1)].killed++;

    if (victim->gcount > 1)
    {
      victim->gcount--;
      victim->hit = victim->max_hit;
      victim->position = POS_STANDING;

      band_description(victim);
    }
    else
    {
      extract_char(victim, TRUE);
    }

    return;
  }

  extract_char(victim, FALSE);

  pIter = AllocIterator(victim->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
    affect_remove(victim, paf);

  if (IS_AFFECTED(victim, AFF_POLYMORPH) && IS_AFFECTED(victim, AFF_ETHEREAL))
  {
    victim->affected_by = AFF_POLYMORPH + AFF_ETHEREAL;
  }
  else if (IS_AFFECTED(victim, AFF_POLYMORPH))
    victim->affected_by = AFF_POLYMORPH;
  else if (IS_AFFECTED(victim, AFF_ETHEREAL))
    victim->affected_by = AFF_ETHEREAL;
  else
    victim->affected_by = 0;
  REMOVE_BIT(victim->extra, TIED_UP);
  REMOVE_BIT(victim->extra, GAGGED);
  REMOVE_BIT(victim->extra, BLINDFOLDED);
  victim->itemaffect = 0;
  victim->loc_hp[0] = 0;
  victim->loc_hp[1] = 0;
  victim->loc_hp[2] = 0;
  victim->loc_hp[3] = 0;
  victim->loc_hp[4] = 0;
  victim->loc_hp[5] = 0;
  victim->loc_hp[6] = 0;
  victim->armor = 100;
  victim->position = POS_RESTING;
  victim->hit = UMAX(1, victim->hit);
  victim->mana = UMAX(1, victim->mana);
  victim->move = UMAX(1, victim->move);
  victim->hitroll = 0;
  victim->damroll = 0;
  victim->saving_throw = 0;
  victim->carry_weight = 0;
  victim->carry_number = 0;
  call_all(victim);
  save_char_obj(victim);
}

void behead(CHAR_DATA *victim)
{
  if (IS_NPC(victim))
    return;

  stop_fighting(victim, TRUE);
  make_corpse(victim, NULL);
  powerdown(victim);
  victim->fight_timer = 0;

  victim->position = POS_STANDING;
  victim->hit = 1;
  victim->mana = UMAX(1, victim->mana);
  victim->move = UMAX(1, victim->move);

  make_part(victim, "head");
  make_part(victim, "heart");
  extract_char(victim, FALSE);

  if (victim == NULL)
  {
    bug("Behead: Victim no longer exists.", 0);
    return;
  }

  call_all(victim);
  save_char_obj(victim);
}

void group_gain(CHAR_DATA *ch, CHAR_DATA *victim)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  EVENT_DATA *event;
  ITERATOR *pIter;
  CHAR_DATA *gch;
  int xp, temp, powerlevel;
  int classbase = number_range(3 * getMobMight(victim->pIndexData) / 2, 5 * getMobMight(victim->pIndexData) / 2);

  /* this would be silly */
  if (victim == ch)
    return;

  /* this is the base powerlevel */
  powerlevel = getMight(ch);

  /* count group members */
  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (ch == gch) continue;

    if (is_same_group(gch, ch))
      powerlevel += getMight(gch);
  }

  /* calculate base experience */
  xp = xp_compute(victim);
  xp *= muddata.ccenter[CCENTER_EXP_LEVEL];
  xp /= 100;

  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    int xp_modifier = 100, gexp = xp, classpoints = classbase;

    /* NPC's and non-group members are not rewarded */
    if (IS_NPC(gch) || !is_same_group(gch, ch))
      continue;

    sprintf(buf2, "#RExp modifiers  #G:");

    if ((IS_EVIL(gch) && IS_GOOD(victim)) || (IS_GOOD(gch) && IS_EVIL(victim)))
    {
      xp_modifier += 25;
      strcat(buf2, " #Calignment");
    }
    if ((IS_EVIL(gch) && IS_EVIL(victim)) || (IS_GOOD(gch) && IS_GOOD(victim)))
    {
      xp_modifier -= 25;
      strcat(buf2, " #palignment");
    }
    if (gch != NULL && gch->desc != NULL)
    {
      if (gch->desc->out_compress)
      {
        xp_modifier += 25;
        strcat(buf2, " #Cmccp");
      }
    }
    if ((get_age(gch) - 17) < 2)  /* 4 hours worth of newbie exp. */
    {
      xp_modifier += 200;
      strcat(buf2, " #Cnewbie");
    }
    if ((temp = getRank(gch, 0)) > 0)
    {
      xp_modifier += temp * 5;
      strcat(buf2, " #Csize");
    }
    if (!IS_NPC(gch) && gch->pcdata->status > 0)
    {
      xp_modifier += gch->pcdata->status * 2;
      strcat(buf2, " #Cstatus");
    }
    if (!IS_NPC(gch) && gch->pcdata->time_tick > 49)
    {
      xp_modifier += gch->pcdata->time_tick / 5;
      strcat(buf2, " #Ctime");
    }
    if (!IS_NPC(gch) && IS_SET(gch->pcdata->tempflag, TEMP_EDGE))
    {
      xp_modifier += 15;
      strcat(buf2," #Cedge");
    }
    if (gch->in_room && event_isset_area(gch->in_room->area, EVENT_AREA_MILKANDHONEY))
    {
      if (number_percent() > 75)
      {
        xp_modifier += 100;
        strcat(buf2, " #Cmilk-and-honey");
      }
    }
    strcat(buf2, "#n\n\r");

    /* calculate exp for this player */
    temp = UMIN(100, 150 * getMight(gch) / powerlevel);
    gexp = gexp * temp / 100;
    gexp = gexp * xp_modifier / 100;

    /* calculate class points for this player */
    classpoints = classpoints * (UMIN(100, 150 * getMight(gch) / powerlevel)) / 100;

    /* send exp information to player */
    if (!IS_NPC(gch) && !gch->pcdata->brief[BRIEF_4])
    {
      send_to_char(buf2, gch);
      sprintf(buf2, "#RTotal modifier #G:#n %d percent bonus\n\r", xp_modifier - 100);
      send_to_char(buf2, gch);
    }

    /* anti mortal exp bot thingie */
    if ((get_age(gch) - 17) > 2 && ch->level < 3)
      gexp /= 10;

    /* cap the exp if to much is gained */
    if (gexp > muddata.ccenter[CCENTER_MAX_EXP])
      gexp = number_range(muddata.ccenter[CCENTER_MAX_EXP] * 0.9, 
                          muddata.ccenter[CCENTER_MAX_EXP] * 1.1);

    /* avoid overflow */
    if (gch->exp + gexp > 2147483647 || gch->exp + gexp < 0)
      gexp = 0;

    if (!IS_SET(gch->extra, EXTRA_PKREADY) && (gch->exp + gexp) > 200000000)
    {
      send_to_char("#LYou cannot store more than #G200 million exp#L if you are not pkready.#n\n\r", gch);
      gexp = 0;
    }

    /* player must accept game policy before 4 hours has passed */
    if ((get_age(gch) - 17) >= 2 && !IS_SET(gch->pcdata->jflags, JFLAG_POLICY))
    {
      gexp = 0;
      send_to_char("#LYou must accept the policy before you can gain more experience [#9HELP POLICY#L]#n\n\r", gch);
    }

    if ((event = event_isset_mobile(gch, EVENT_PLAYER_ALIGNMENT)) != NULL)
    {
      int align = atoi(event->argument);

      align -= victim->alignment;
      sprintf(buf, "%d", align);

      free_string(event->argument);
      event->argument = str_dup(buf);
    }

    if ((event = event_isset_mobile(gch, EVENT_PLAYER_STUDY)) == NULL)
    {
      printf_to_char(gch, "You receive %s experience points.\n\r", dot_it_up(gexp));
      gch->exp += gexp;

      if (!IS_NPC(gch))
        gch->pcdata->session->exp += gexp;
    }
    else
    {
      char arg1[MAX_INPUT_LENGTH];
      char arg2[MAX_INPUT_LENGTH];
      int exp_needed;
      char *argument;

      argument = one_argument(event->argument, arg1);
      one_argument(argument, arg2);
      exp_needed = atoi(arg2);
      exp_needed -= gexp;
      free_string(event->argument);
      sprintf(arg2, "%s %d", arg1, UMAX(0, exp_needed));
      event->argument = str_dup(arg2);

      sprintf(buf, "You gather %d study points.\n\r", gexp);
      send_to_char(buf, gch);

      if (exp_needed < 1)
        event->passes = 0;
    }

    if (IS_CLASS(gch, CLASS_SHADOW) || IS_CLASS(gch, CLASS_GIANT))
    {
      if (IS_CLASS(gch, CLASS_SHADOW))
        gch->pcdata->powers[SHADOW_POWER] += classpoints;
      else if (IS_CLASS(gch, CLASS_GIANT))
        gch->pcdata->powers[GIANT_POINTS] += classpoints;

      sprintf(buf,"You gain #y(#C%d#y)#n class points.\n\r", classpoints);

      if (!gch->pcdata->brief[BRIEF_4])
        send_to_char(buf, gch);
    }
  }
}

int xp_compute(CHAR_DATA *victim)
{
  double xp, level = getMobMight(victim->pIndexData);

  if (IS_NPC(victim) && (IS_SET(victim->act, ACT_NOEXP)))
    return 0;

  /*
   * Calculate the experience for this mobile...
   */
  xp = level * log(level) * URANGE(50, log(level) * sqrt(level), 150);
  xp = number_range((6 * xp) / 10, (14 * xp) / 10);

  if (xp < muddata.ccenter[CCENTER_MIN_EXP])
    xp = number_range(muddata.ccenter[CCENTER_MIN_EXP] * 0.9, 
                      muddata.ccenter[CCENTER_MIN_EXP] * 1.1);

  return (int) xp;
}

void dam_message(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, int dam, int dt)
{
  char buf1[512], buf2[512], buf3[512], dmbuf_a[128], dmbuf_v[128];
  const char *attack, *attack2, *dm = NULL;
  int damp = (IS_NPC(victim)) ? 0 : -10;
  int bodyloc, i;
  bool critical = FALSE;
  static char *attack_table[] = {
    "hit", "slice", "stab", "slash", "whip", "claw",
    "blast", "pound", "crush", "grep", "bite", "pierce", "suck"
  };
  static char *attack_table2[] = {
    "hits", "slices", "stabs", "slashes", "whips", "claws",
    "blasts", "pounds", "crushes", "greps", "bites", "pierces", "sucks"
  };
  static struct message_type message_table[] = {
    {     0,  "misses"                                    },
    {     1,  "#Clightly#n"                               },
    {    51,  "#yhard#n"                                  },
    {   251,  "#Lvery hard#n"                             },
    {   501,  "#Gextremely hard#n"                        },
    {   751,  "#9incredibly hard#n"                       },
    {  1251,  "#rshredding flesh#n"                       },
    {  2001,  "#osplintering bone#n"                      },
    {  3001,  "#Rspraying blood like a fine red mist#n"   },
    {  4501,  "#yso hard it hurts just to see#n"          },
    {  6001,  "#G<#y*#L{#R*#L}#y*#G> #Gextracting organs #G<#y*#L{#R*#L}#y*#G>#n" },
    {    -1,  "" }
  };

  if (dam < 0)
    dam = 0;

  for (i = 0; message_table[i].message[0] != '\0'; i++)
  {
    if (dam >= message_table[i].max_dam)
      dm = message_table[i].message;
  }

  if (!IS_NPC(victim) && IS_SET(victim->newbits, NEW_ENH_COMBAT))
    sprintf(dmbuf_v, "#9[#%c%d#9]#n", victim->pcdata->enh_combat[3], dam);
  else
    sprintf(dmbuf_v, "#9[#R%d#9]#n", dam);

  if (!IS_NPC(ch) && IS_SET(ch->newbits, NEW_ENH_COMBAT))
    sprintf(dmbuf_a, "#9[#%c%d#9]#n", ch->pcdata->enh_combat[1], dam);
  else
    sprintf(dmbuf_a, "#9[#G%d#9]#n", dam);

  if ((victim->hit - dam > damp) || (dt >= 0 && dt < MAX_SKILL) || (IS_NPC(victim) && IS_SET(victim->act, ACT_NOPARTS)))
  {
    if (dt == TYPE_HIT && !IS_NPC(ch))
    {
      if (dam == 0)
      {
        sprintf(buf1, "$n misses $N");
        sprintf(buf2, "You miss $N");
        sprintf(buf3, "$n misses you");
      }
      else
      {
        sprintf(buf1, "$n hits $N %s", dm);
        sprintf(buf2, "You hit $N %s %s", dm, dmbuf_a);
        sprintf(buf3, "$n hits you %s %s", dm, dmbuf_v);

        critical = TRUE;
      }
    }
    else
    {
      if (dt >= 0 && dt < MAX_SKILL)
      {
        attack = skill_table[dt].noun_damage;
        attack2 = skill_table[dt].noun_damage;
      }
      else if (dt >= TYPE_HIT && dt < (int) (TYPE_HIT + sizeof(attack_table) / sizeof(attack_table[0])))
      {
        attack = attack_table[dt - TYPE_HIT];
        attack2 = attack_table2[dt - TYPE_HIT];
      }
      else
      {
        dt = TYPE_HIT;
        attack = attack_table[0];
        attack2 = attack_table2[0];
      }
      if (dam == 0)
      {
        sprintf(buf1, "$n's %s misses $N", attack);
        sprintf(buf2, "Your %s miss $N", attack);
        sprintf(buf3, "$n's %s misses you", attack);
      }
      else
      {
        if (dt >= 0 && dt < MAX_SKILL)
        {
          sprintf(buf1, "$n's %s strikes $N %s", attack2, dm);
          sprintf(buf2, "Your %s strikes $N %s %s", attack, dm, dmbuf_a);
          sprintf(buf3, "$n's %s strikes you %s %s", attack2, dm, dmbuf_v);
        }
        else
        {
          sprintf(buf1, "$n %s $N %s", attack2, dm);
          sprintf(buf2, "You %s $N %s %s", attack, dm, dmbuf_a);
          sprintf(buf3, "$n %s you %s %s", attack2, dm, dmbuf_v);

          critical = TRUE;
        }
      }
    }

    /*
     * Message to room.
     */
    act_brief(buf1, ch, NULL, victim, TO_NOTVICT, BRIEF_6);

    /*
     * Message to attacker
     */
    if (!IS_NPC(ch) && (!ch->pcdata->brief[BRIEF_5] ||
       (IS_CLASS(ch, CLASS_GIANT) && (dt == gsn_smack || dt == gsn_bash || dt == gsn_thwack || dt == gsn_crush))))
    {
      if (!ch->pcdata->brief[BRIEF_2] || dam > 0 ||
         (IS_CLASS(ch, CLASS_GIANT) && (dt == gsn_smack || dt == gsn_bash || dt == gsn_thwack || dt == gsn_crush)))
      {
        act(buf2, ch, NULL, victim, TO_CHAR);
      }
    }

    /*
     * Message to victim
     */
    if (!IS_NPC(victim) && !victim->pcdata->brief[BRIEF_5])
    {
      if (!(victim->pcdata->brief[BRIEF_2] && dam == 0))
      {
        act(buf3, ch, NULL, victim, TO_VICT);
      }
    }

    if (critical)
      critical_hit(ch, victim, obj, dt, dam);

    return;
  }
  if (dt == TYPE_HIT)
  {
    damp = number_range(1, 5);
    if (damp == 1)
    {
      act("You ram your fingers into $N's eye sockets and rip $S face off.", ch, NULL, victim, TO_CHAR);
      act("$n rams $s fingers into $N's eye sockets and rips $S face off.", ch, NULL, victim, TO_NOTVICT);
      act("$n rams $s fingers into your eye sockets and rips your face off.", ch, NULL, victim, TO_VICT);
      make_part(victim, "face");
    }
    else if (damp == 2)
    {
      act("You grab $N by the throat and tear $S windpipe out.", ch, NULL, victim, TO_CHAR);
      act("$n grabs $N by the throat and tears $S windpipe out.", ch, NULL, victim, TO_NOTVICT);
      act("$n grabs you by the throat and tears your windpipe out.", ch, NULL, victim, TO_VICT);
      make_part(victim, "windpipe");
    }
    else if (damp == 3)
    {
      act("You punch your fist through $N's stomach and rip out $S entrails.", ch, NULL, victim, TO_CHAR);
      act("$n punches $s fist through $N's stomach and rips out $S entrails.", ch, NULL, victim, TO_NOTVICT);
      act("$n punches $s fist through your stomach and rips out your entrails.", ch, NULL, victim, TO_VICT);
      make_part(victim, "entrails");
    }
    else if (damp == 4)
    {
      if (!IS_BODY(victim, BROKEN_SPINE))
        SET_BIT(victim->loc_hp[1], BROKEN_SPINE);
      act("You hoist $N above your head and slam $M down upon your knee.\n\rThere is a loud cracking sound as $N's spine snaps.", ch, NULL, victim, TO_CHAR);
      act("$n hoists $N above $s head and slams $M down upon $s knee.\n\rThere is a loud cracking sound as $N's spine snaps.", ch, NULL, victim, TO_NOTVICT);
      act("$n hoists you above $s head and slams you down upon $s knee.\n\rThere is a loud cracking sound as your spine snaps.", ch, NULL, victim, TO_VICT);
    }
    else if (damp == 5)
    {
      act("You lock your arm around $N's head, and give it a vicious twist.", ch, NULL, victim, TO_CHAR);
      act("$n locks $s arm around $N's head, and gives it a vicious twist.", ch, NULL, victim, TO_NOTVICT);
      act("$n locks $s arm around your head, and gives it a vicious twist.", ch, NULL, victim, TO_VICT);
      if (!IS_BODY(victim, BROKEN_NECK))
      {
        act("There is a loud snapping noise as your neck breaks.", victim, NULL, NULL, TO_CHAR);
        act("There is a loud snapping noise as $n's neck breaks.", victim, NULL, NULL, TO_ROOM);
        SET_BIT(victim->loc_hp[1], BROKEN_NECK);
      }
    }
    return;
  }
  if (dt >= 0 && dt < MAX_SKILL)
    attack = skill_table[dt].noun_damage;
  else if (dt >= TYPE_HIT && dt < TYPE_HIT + (int) (sizeof(attack_table) / sizeof(attack_table[0])))
    attack = attack_table[dt - TYPE_HIT];
  else
  {
    dt = TYPE_HIT;
    attack = attack_table[0];
  }
  if (!str_cmp(attack, "slash") || !str_cmp(attack, "slice"))
  {
    damp = number_range(1, 8);
    if (damp == 1)
    {
      act("You swing your blade in a low arc, rupturing $N's abdominal cavity.\n\r$S entrails spray out over a wide area.", ch, NULL, victim, TO_CHAR);
      act("$n swings $s blade in a low arc, rupturing $N's abdominal cavity.\n\r$S entrails spray out over a wide area.", ch, NULL, victim, TO_NOTVICT);
      act("$n swings $s blade in a low arc, rupturing your abdominal cavity.\n\rYour entrails spray out over a wide area.", ch, NULL, victim, TO_VICT);
      make_part(victim, "entrails");
    }
    else if (damp == 2)
    {
      act("You thrust your blade into $N's mouth and twist it viciously.\n\rThe end of your blade bursts through the back of $S head.", ch, NULL, victim, TO_CHAR);
      act("$n thrusts $s blade into $N's mouth and twists it viciously.\n\rThe end of the blade bursts through the back of $N's head.", ch, NULL, victim, TO_NOTVICT);
      act("$n thrusts $s blade into your mouth and twists it viciously.\n\rYou feel the end of the blade burst through the back of your head.", ch, NULL, victim, TO_VICT);
    }
    else if (damp == 3)
    {
      if (!IS_BODY(victim, CUT_THROAT))
        SET_BIT(victim->loc_hp[1], CUT_THROAT);
      if (!IS_BLEEDING(victim, BLEEDING_THROAT))
        SET_BIT(victim->loc_hp[6], BLEEDING_THROAT);
      act("Your blow slices open $N's carotid artery, spraying blood everywhere.", ch, NULL, victim, TO_CHAR);
      act("$n's blow slices open $N's carotid artery, spraying blood everywhere.", ch, NULL, victim, TO_NOTVICT);
      act("$n's blow slices open your carotid artery, spraying blood everywhere.", ch, NULL, victim, TO_VICT);
      make_part(victim, "blood");
    }
    else if (damp == 4)
    {
      if (!IS_BODY(victim, CUT_THROAT))
        SET_BIT(victim->loc_hp[1], CUT_THROAT);
      if (!IS_BLEEDING(victim, BLEEDING_THROAT))
        SET_BIT(victim->loc_hp[6], BLEEDING_THROAT);
      act("You swing your blade across $N's throat, showering the area with blood.", ch, NULL, victim, TO_CHAR);
      act("$n swings $s blade across $N's throat, showering the area with blood.", ch, NULL, victim, TO_NOTVICT);
      act("$n swings $s blade across your throat, showering the area with blood.", ch, NULL, victim, TO_VICT);
      make_part(victim, "blood");
    }
    else if (damp == 5)
    {
      if (!IS_HEAD(victim, BROKEN_SKULL))
      {
        act("You swing your blade down upon $N's head, splitting it open.\n\r$N's brains pour out of $S forehead.", ch, NULL, victim, TO_CHAR);
        act("$n swings $s blade down upon $N's head, splitting it open.\n\r$N's brains pour out of $S forehead.", ch, NULL, victim, TO_NOTVICT);
        act("$n swings $s blade down upon your head, splitting it open.\n\rYour brains pour out of your forehead.", ch, NULL, victim, TO_VICT);
        make_part(victim, "brain");
        SET_BIT(victim->loc_hp[0], BROKEN_SKULL);
      }
      else
      {
        act("You plunge your blade deep into $N's chest.", ch, NULL, victim, TO_CHAR);
        act("$n plunges $s blade deep into $N's chest.", ch, NULL, victim, TO_NOTVICT);
        act("$n plunges $s blade deep into your chest.", ch, NULL, victim, TO_VICT);
      }
    }
    else if (damp == 6)
    {
      act("You swing your blade between $N's legs, nearly splitting $M in half.", ch, NULL, victim, TO_CHAR);
      act("$n swings $s blade between $N's legs, nearly splitting $M in half.", ch, NULL, victim, TO_NOTVICT);
      act("$n swings $s blade between your legs, nearly splitting you in half.", ch, NULL, victim, TO_VICT);
    }
    else if (damp == 7)
    {
      if (!IS_ARM_L(victim, LOST_ARM))
      {
        act("You swing your blade in a wide arc, slicing off $N's arm.", ch, NULL, victim, TO_CHAR);
        act("$n swings $s blade in a wide arc, slicing off $N's arm.", ch, NULL, victim, TO_NOTVICT);
        act("$n swings $s blade in a wide arc, slicing off your arm.", ch, NULL, victim, TO_VICT);
        make_part(victim, "arm");
        SET_BIT(victim->loc_hp[2], LOST_ARM);
        if (!IS_BLEEDING(victim, BLEEDING_ARM_L))
          SET_BIT(victim->loc_hp[6], BLEEDING_ARM_L);
        if (IS_BLEEDING(victim, BLEEDING_HAND_L))
          REMOVE_BIT(victim->loc_hp[6], BLEEDING_HAND_L);
      }
      else if (!IS_ARM_R(victim, LOST_ARM))
      {
        act("You swing your blade in a wide arc, slicing off $N's arm.", ch, NULL, victim, TO_CHAR);
        act("$n swings $s blade in a wide arc, slicing off $N's arm.", ch, NULL, victim, TO_NOTVICT);
        act("$n swings $s blade in a wide arc, slicing off your arm.", ch, NULL, victim, TO_VICT);
        make_part(victim, "arm");
        SET_BIT(victim->loc_hp[3], LOST_ARM);
        if (!IS_BLEEDING(victim, BLEEDING_ARM_R))
          SET_BIT(victim->loc_hp[6], BLEEDING_ARM_R);
        if (IS_BLEEDING(victim, BLEEDING_HAND_R))
          REMOVE_BIT(victim->loc_hp[6], BLEEDING_HAND_R);
      }
      else
      {
        act("You plunge your blade deep into $N's chest.", ch, NULL, victim, TO_CHAR);
        act("$n plunges $s blade deep into $N's chest.", ch, NULL, victim, TO_NOTVICT);
        act("$n plunges $s blade deep into your chest.", ch, NULL, victim, TO_VICT);
      }
    }
    else if (damp == 8)
    {
      if (!IS_LEG_L(victim, LOST_LEG))
      {
        act("You swing your blade in a low arc, slicing off $N's leg at the hip.", ch, NULL, victim, TO_CHAR);
        act("$n swings $s blade in a low arc, slicing off $N's leg at the hip.", ch, NULL, victim, TO_NOTVICT);
        act("$n swings $s blade in a wide arc, slicing off your leg at the hip.", ch, NULL, victim, TO_VICT);
        make_part(victim, "leg");
        SET_BIT(victim->loc_hp[4], LOST_LEG);
        if (!IS_BLEEDING(victim, BLEEDING_LEG_L))
          SET_BIT(victim->loc_hp[6], BLEEDING_LEG_L);
        if (IS_BLEEDING(victim, BLEEDING_FOOT_L))
          REMOVE_BIT(victim->loc_hp[6], BLEEDING_FOOT_L);
      }
      else if (!IS_LEG_R(victim, LOST_LEG))
      {
        act("You swing your blade in a low arc, slicing off $N's leg at the hip.", ch, NULL, victim, TO_CHAR);
        act("$n swings $s blade in a low arc, slicing off $N's leg at the hip.", ch, NULL, victim, TO_NOTVICT);
        act("$n swings $s blade in a wide arc, slicing off your leg at the hip.", ch, NULL, victim, TO_VICT);
        make_part(victim, "leg");
        SET_BIT(victim->loc_hp[5], LOST_LEG);
        if (!IS_BLEEDING(victim, BLEEDING_LEG_R))
          SET_BIT(victim->loc_hp[6], BLEEDING_LEG_R);
        if (IS_BLEEDING(victim, BLEEDING_FOOT_R))
          REMOVE_BIT(victim->loc_hp[6], BLEEDING_FOOT_R);
      }
      else
      {
        act("You plunge your blade deep into $N's chest.", ch, NULL, victim, TO_CHAR);
        act("$n plunges $s blade deep into $N's chest.", ch, NULL, victim, TO_NOTVICT);
        act("$n plunges $s blade deep into your chest.", ch, NULL, victim, TO_VICT);
      }
    }
  }
  else if (!str_cmp(attack, "stab") || !str_cmp(attack, "pierce"))
  {
    damp = number_range(1, 5);
    if (damp == 1)
    {
      act("You defty invert your weapon and plunge it point first into $N's chest.\n\rA shower of blood sprays from the wound, showering the area.", ch, NULL, victim, TO_CHAR);
      act("$n defty inverts $s weapon and plunge it point first into $N's chest.\n\rA shower of blood sprays from the wound, showering the area.", ch, NULL, victim, TO_NOTVICT);
      act("$n defty inverts $s weapon and plunge it point first into your chest.\n\rA shower of blood sprays from the wound, showering the area.", ch, NULL, victim, TO_VICT);
      make_part(victim, "blood");
    }
    else if (damp == 2)
    {
      act("You thrust your blade into $N's mouth and twist it viciously.\n\rThe end of your blade bursts through the back of $S head.", ch, NULL, victim, TO_CHAR);
      act("$n thrusts $s blade into $N's mouth and twists it viciously.\n\rThe end of the blade bursts through the back of $N's head.", ch, NULL, victim, TO_NOTVICT);
      act("$n thrusts $s blade into your mouth and twists it viciously.\n\rYou feel the end of the blade burst through the back of your head.", ch, NULL, victim, TO_VICT);
    }
    else if (damp == 3)
    {
      act("You thrust your weapon up under $N's jaw and through $S head.", ch, NULL, victim, TO_CHAR);
      act("$n thrusts $s weapon up under $N's jaw and through $S head.", ch, NULL, victim, TO_NOTVICT);
      act("$n thrusts $s weapon up under your jaw and through your head.", ch, NULL, victim, TO_VICT);
    }
    else if (damp == 4)
    {
      act("You ram your weapon through $N's body, pinning $M to the ground.", ch, NULL, victim, TO_CHAR);
      act("$n rams $s weapon through $N's body, pinning $M to the ground.", ch, NULL, victim, TO_NOTVICT);
      act("$n rams $s weapon through your body, pinning you to the ground.", ch, NULL, victim, TO_VICT);
    }
    else if (damp == 5)
    {
      act("You stab your weapon into $N's eye and out the back of $S head.", ch, NULL, victim, TO_CHAR);
      act("$n stabs $s weapon into $N's eye and out the back of $S head.", ch, NULL, victim, TO_NOTVICT);
      act("$n stabs $s weapon into your eye and out the back of your head.", ch, NULL, victim, TO_VICT);
      if (!IS_HEAD(victim, LOST_EYE_L) && number_percent() < 50)
        SET_BIT(victim->loc_hp[0], LOST_EYE_L);
      else if (!IS_HEAD(victim, LOST_EYE_R))
        SET_BIT(victim->loc_hp[0], LOST_EYE_R);
      else if (!IS_HEAD(victim, LOST_EYE_L))
        SET_BIT(victim->loc_hp[0], LOST_EYE_L);
    }
  }
  else if (!str_cmp(attack, "blast") || !str_cmp(attack, "pound") || !str_cmp(attack, "crush"))
  {
    damp = number_range(1, 3);
    bodyloc = 0;
    if (damp == 1)
    {
      act("Your blow smashes through $N's chest, caving in half $S ribcage.", ch, NULL, victim, TO_CHAR);
      act("$n's blow smashes through $N's chest, caving in half $S ribcage.", ch, NULL, victim, TO_NOTVICT);
      act("$n's blow smashes through your chest, caving in half your ribcage.", ch, NULL, victim, TO_VICT);
      if (IS_BODY(victim, BROKEN_RIBS_1))
      {
        bodyloc += 1;
        REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_1);
      }
      if (IS_BODY(victim, BROKEN_RIBS_2))
      {
        bodyloc += 2;
        REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_2);
      }
      if (IS_BODY(victim, BROKEN_RIBS_4))
      {
        bodyloc += 4;
        REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_4);
      }
      if (IS_BODY(victim, BROKEN_RIBS_8))
      {
        bodyloc += 8;
        REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_8);
      }
      if (IS_BODY(victim, BROKEN_RIBS_16))
      {
        bodyloc += 16;
        REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_16);
      }
      bodyloc += number_range(1, 3);
      if (bodyloc > 24)
        bodyloc = 24;
      if (bodyloc >= 16)
      {
        bodyloc -= 16;
        SET_BIT(victim->loc_hp[1], BROKEN_RIBS_16);
      }
      if (bodyloc >= 8)
      {
        bodyloc -= 8;
        SET_BIT(victim->loc_hp[1], BROKEN_RIBS_8);
      }
      if (bodyloc >= 4)
      {
        bodyloc -= 4;
        SET_BIT(victim->loc_hp[1], BROKEN_RIBS_4);
      }
      if (bodyloc >= 2)
      {
        bodyloc -= 2;
        SET_BIT(victim->loc_hp[1], BROKEN_RIBS_2);
      }
      if (bodyloc >= 1)
      {
        bodyloc -= 1;
        SET_BIT(victim->loc_hp[1], BROKEN_RIBS_1);
      }
    }
    else if (damp == 2)
    {
      act("Your blow smashes $N's spine, shattering it in several places.", ch, NULL, victim, TO_CHAR);
      act("$n's blow smashes $N's spine, shattering it in several places.", ch, NULL, victim, TO_NOTVICT);
      act("$n's blow smashes your spine, shattering it in several places.", ch, NULL, victim, TO_VICT);
      if (!IS_BODY(victim, BROKEN_SPINE))
        SET_BIT(victim->loc_hp[1], BROKEN_SPINE);
    }
    else if (damp == 3)
    {
      if (!IS_HEAD(victim, BROKEN_SKULL))
      {
        act("You swing your weapon down upon $N's head.\n\r$N's head cracks open like an overripe melon, leaking out brains.", ch, NULL, victim, TO_CHAR);
        act("$n swings $s weapon down upon $N's head.\n\r$N's head cracks open like an overripe melon, covering you with brains.", ch, NULL, victim, TO_NOTVICT);
        act("$n swings $s weapon down upon your head.\n\rYour head cracks open like an overripe melon, spilling your brains everywhere.", ch, NULL, victim, TO_VICT);
        make_part(victim, "brain");
        SET_BIT(victim->loc_hp[0], BROKEN_SKULL);
      }
      else
      {
        act("You hammer your weapon into $N's side, crushing bone.", ch, NULL, victim, TO_CHAR);
        act("$n hammers $s weapon into $N's side, crushing bone.", ch, NULL, victim, TO_NOTVICT);
        act("$n hammers $s weapon into your side, crushing bone.", ch, NULL, victim, TO_VICT);
      }
    }
  }
  else if (!str_cmp(attack, "bite"))
  {
    act("You sink your teeth into $N's throat and tear out $S jugular vein.\n\rYou wipe the blood from your chin with one hand.", ch, NULL, victim, TO_CHAR);
    act("$n sink $s teeth into $N's throat and tears out $S jugular vein.\n\r$n wipes the blood from $s chin with one hand.", ch, NULL, victim, TO_NOTVICT);
    act("$n sink $s teeth into your throat and tears out your jugular vein.\n\r$n wipes the blood from $s chin with one hand.", ch, NULL, victim, TO_VICT);
    make_part(victim, "blood");
    if (!IS_BODY(victim, CUT_THROAT))
      SET_BIT(victim->loc_hp[1], CUT_THROAT);
    if (!IS_BLEEDING(victim, BLEEDING_THROAT))
      SET_BIT(victim->loc_hp[6], BLEEDING_THROAT);
  }
  else if (!str_cmp(attack, "claw"))
  {
    damp = number_range(1, 2);
    if (damp == 1)
    {
      act("You tear out $N's throat, showering the area with blood.", ch, NULL, victim, TO_CHAR);
      act("$n tears out $N's throat, showering the area with blood.", ch, NULL, victim, TO_NOTVICT);
      act("$n tears out your throat, showering the area with blood.", ch, NULL, victim, TO_VICT);
      make_part(victim, "blood");
      if (!IS_BODY(victim, CUT_THROAT))
        SET_BIT(victim->loc_hp[1], CUT_THROAT);
      if (!IS_BLEEDING(victim, BLEEDING_THROAT))
        SET_BIT(victim->loc_hp[6], BLEEDING_THROAT);
    }
    if (damp == 2)
    {
      if (!IS_HEAD(victim, LOST_EYE_L) && number_percent() < 50)
      {
        act("You rip an eyeball from $N's face.", ch, NULL, victim, TO_CHAR);
        act("$n rips an eyeball from $N's face.", ch, NULL, victim, TO_NOTVICT);
        act("$n rips an eyeball from your face.", ch, NULL, victim, TO_VICT);
        make_part(victim, "eyeball");
        SET_BIT(victim->loc_hp[0], LOST_EYE_L);
      }
      else if (!IS_HEAD(victim, LOST_EYE_R))
      {
        act("You rip an eyeball from $N's face.", ch, NULL, victim, TO_CHAR);
        act("$n rips an eyeball from $N's face.", ch, NULL, victim, TO_NOTVICT);
        act("$n rips an eyeball from your face.", ch, NULL, victim, TO_VICT);
        make_part(victim, "eyeball");
        SET_BIT(victim->loc_hp[0], LOST_EYE_R);
      }
      else if (!IS_HEAD(victim, LOST_EYE_L))
      {
        act("You rip an eyeball from $N's face.", ch, NULL, victim, TO_CHAR);
        act("$n rips an eyeball from $N's face.", ch, NULL, victim, TO_NOTVICT);
        act("$n rips an eyeball from your face.", ch, NULL, victim, TO_VICT);
        make_part(victim, "eyeball");
        SET_BIT(victim->loc_hp[0], LOST_EYE_L);
      }
      else
      {
        act("You claw open $N's chest.", ch, NULL, victim, TO_CHAR);
        act("$n claws open $N's chest.", ch, NULL, victim, TO_NOTVICT);
        act("$n claws open $N's chest.", ch, NULL, victim, TO_VICT);
      }
    }
  }
  else if (!str_cmp(attack, "whip"))
  {
    act("You entangle $N around the neck, and squeeze out $S life.", ch, NULL, victim, TO_CHAR);
    act("$n entangles $N around the neck, and squeezes out $S life.", ch, NULL, victim, TO_NOTVICT);
    act("$n entangles you around the neck, and squeezes the life out of you.", ch, NULL, victim, TO_VICT);
    if (!IS_BODY(victim, BROKEN_NECK))
      SET_BIT(victim->loc_hp[1], BROKEN_NECK);
  }
  else if (!str_cmp(attack, "suck") || !str_cmp(attack, "grep"))
  {
    act("You place your weapon on $N's head and suck out $S brains.", ch, NULL, victim, TO_CHAR);
    act("$n places $s weapon on $N's head and suck out $S brains.", ch, NULL, victim, TO_NOTVICT);
    act("$n places $s weapon on your head and suck out your brains.", ch, NULL, victim, TO_VICT);
  }
  else
  {
    bug("dam_message: bad dt %d.", dt);
  }
}

/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm(CHAR_DATA * ch, CHAR_DATA * victim)
{
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch) && victim->level > 2 && number_percent() > 10)
    return;
  if (!IS_IMMUNE(ch, IMM_DISARM))
    return;
  if (((obj = get_eq_char(victim, WEAR_WIELD)) == NULL) || obj->item_type != ITEM_WEAPON)
  {
    if (((obj = get_eq_char(victim, WEAR_HOLD)) == NULL) || obj->item_type != ITEM_WEAPON)
      return;
  }
  sprintf(buf, "#9$n disarms you!#n");
  act(buf, ch, NULL, victim, TO_VICT);
  sprintf(buf, "#9You disarm $N!#n");
  act(buf, ch, NULL, victim, TO_CHAR);
  sprintf(buf, "#9$n disarms $N!#n");
  act(buf, ch, NULL, victim, TO_NOTVICT);
  obj_from_char(obj);
  if (IS_SET(obj->extra_flags, ITEM_LOYAL) && (!IS_NPC(victim)))
  {
    act("$p leaps back into your hand!", victim, obj, NULL, TO_CHAR);
    act("$p leaps back into $n's hand!", victim, obj, NULL, TO_ROOM);
    obj_to_char(obj, victim);
    do_wear(victim, obj->name);
  }
  else if (IS_NPC(victim))
    obj_to_char(obj, victim);
  else
    obj_to_room(obj, victim->in_room);
  return;
}

/*
 * Trip a creature.
 * Caller must check for successful attack.
 */
void trip(CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (IS_AFFECTED(victim, AFF_FLYING))
    return;
  if (IS_NPC(ch) && victim->level > 2 && number_percent() > 5)
    return;
  if (victim->wait == 0)
  {
    act("#9$n trips you and you go down!#n", ch, NULL, victim, TO_VICT);
    act("#9You trip $N and $E goes down!#n", ch, NULL, victim, TO_CHAR);
    act("#9$n trips $N and $E goes down!#n", ch, NULL, victim, TO_NOTVICT);
    WAIT_STATE(victim, 18);
    victim->position = POS_RESTING;
  }
}

bool event_mobile_flee(EVENT_DATA *event)
{
  ROOM_INDEX_DATA *was_in;
  ROOM_INDEX_DATA *now_in;
  CHAR_DATA *victim;
  CHAR_DATA *ch;
  char buf[MAX_STRING_LENGTH];
  int attempt;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_flee: no owner.", 0);
    return FALSE;
  }

  if ((victim = ch->fighting) == NULL || ch->position != POS_FIGHTING || ch->hit <= 0)
  {
    send_to_char("You are no longer fighting anyone.\n\r", ch);
    return FALSE;
  }

  if (IS_AFFECTED(ch, AFF_WEBBED))
  {
    send_to_char("Not with all this sticky webbing around you.\n\r", ch);
    act("$n struggles with the webbing surrounding $m.", ch, NULL, NULL, TO_ROOM);
    return FALSE;
  }

  if (IS_SET(ch->newbits, NEW_TENDRIL1))
  {
    send_to_char("You cannot flee with all these tendrils covering you.\n\r", ch);
    act("$n struggles with the tendrils holding $m.", ch, NULL, NULL, TO_ROOM);
    return FALSE;
  }

  if (!IS_NPC(ch) && IS_CLASS(ch, CLASS_GIANT))
  {
    if (event_isset_mobile(ch, EVENT_PLAYER_DEATHFRENZY))
    {
      send_to_char("NO! You must stay and kill them stinky huuumans.\n\r", ch);
      return FALSE;
    }
  }

  if (!IS_NPC(victim) && IS_CLASS(victim, CLASS_GIANT))
  {
    if (IS_SET(victim->pcdata->powers[GIANT_GIFTS], GGIFT_LONGLEGS) && number_percent( ) > 30)
    {
      sprintf(buf,"%s steps down in front of you, blocking your path!\n\r", victim->name);
      send_to_char(buf, ch);
      act("$n cant escape with $N blocking the path.", ch, NULL, victim, TO_ROOM);
      return FALSE;
    }
  }

  was_in = ch->in_room;
  for (attempt = 0; attempt < 6; attempt++)
  { 
    EXIT_DATA *pexit;
    int door = number_door();

    if ((pexit = was_in->exit[door]) == NULL || pexit->to_room == NULL || IS_SET(pexit->exit_info, EX_CLOSED))
      continue;

    move_char(ch, door);

    if ((now_in = ch->in_room) == was_in)
      continue;

    ch->in_room = was_in;
    act("$n has fled!", ch, NULL, NULL, TO_ROOM);
    ch->in_room = now_in;

    if (!IS_NPC(ch))
      send_to_char("You flee from combat!  Coward!\n\r", ch);

    if (!IS_NPC(ch) && !IS_NPC(victim) && IS_CLASS(victim, CLASS_WARLOCK) && IS_SET(victim->newbits, NEW_HSTARS))
    {
      int i;

      sprintf(buf, "%d", ch->pcdata->playerid);

      for (i = 1; i <=  5; i++)
      {
        event = alloc_event();
        event->fun = &event_player_huntingstars;
        event->type = EVENT_PLAYER_HUNTINGSTARS;
        event->argument = str_dup(buf);
        add_event_char(event, victim, 2 * i);
      }
    }

    stop_fighting(ch, TRUE);
    return FALSE;
  }

  send_to_char("You were unable to escape!\n\r", ch);
  act("$n failed $s escape attempt.", ch, NULL, NULL, TO_ROOM);
  return FALSE;
}

int dambonus(CHAR_DATA *ch, CHAR_DATA *victim, int dam, int stance)
{
  if (dam < 1)
    return 0;

  if (stance < 1)
    return dam;

  if (!can_counter(victim))
  {
    if (IS_STANCE(ch, STANCE_MONKEY))
    {
      int mindam = dam * 0.25;

      dam *= (ch->stance[STANCE_MONKEY] + 1) / 200;

      if (dam < mindam)
        dam = mindam;
    }
    else if (IS_STANCE(ch, STANCE_BULL) && number_percent() < ch->stance[STANCE_BULL] / 2)
      dam += dam * (ch->stance[STANCE_BULL] / 100);
    else if (IS_STANCE(ch, STANCE_DRAGON) && number_percent() < ch->stance[STANCE_DRAGON] / 2)
      dam += dam * (ch->stance[STANCE_DRAGON] / 100);
    else if (IS_STANCE(ch, STANCE_TIGER) && number_percent() < ch->stance[STANCE_TIGER] / 2)
      dam += dam * (ch->stance[STANCE_TIGER] / 100);
    else if (IS_STANCE(ch, STANCE_SPIRIT) && number_percent() < ch->stance[STANCE_SPIRIT] / 2 && IS_NPC(victim) && !IS_NPC(ch))
    {
      if (IS_SET(ch->pcdata->powers[EVOLVE_1], FAE_EVOLVE_SPIRIT))
        dam += dam * (ch->stance[STANCE_SPIRIT] / 85);
      else
        dam += dam * (ch->stance[STANCE_SPIRIT] / 95);
    }
    else if (ch->stance[0] > 0 && number_percent() > ch->stance[stance] / 2)
      dam *= 0.5;
  }

  if (!can_counter(ch))
  {
    if (IS_STANCE(victim, STANCE_CRAB) && number_percent() < victim->stance[STANCE_CRAB] / 2)
      dam /= UMAX(1, victim->stance[STANCE_CRAB] / 100);
    else if (IS_STANCE(victim, STANCE_DRAGON) && number_percent() < victim->stance[STANCE_DRAGON] / 2)
      dam /= UMAX(1, victim->stance[STANCE_DRAGON] / 100);
    else if (IS_STANCE(victim, STANCE_SWALLOW) && number_percent() < victim->stance[STANCE_SWALLOW] / 2)
      dam /= UMAX(1, victim->stance[STANCE_SWALLOW] / 100);
    else if (IS_STANCE(victim, STANCE_SPIRIT) && number_percent() < victim->stance[STANCE_SPIRIT] / 2 && IS_NPC(ch) && !IS_NPC(victim))
    {
      if (IS_SET(victim->pcdata->powers[EVOLVE_1], FAE_EVOLVE_SPIRIT))
        dam /= UMAX(1, victim->stance[STANCE_SPIRIT] / 90);
      else
        dam /= UMAX(1, victim->stance[STANCE_SPIRIT] / 100);
    }
  }

  return dam;
}

void decap_message(CHAR_DATA * ch, CHAR_DATA * victim)
{
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];
  bool unarmed = FALSE;
  int primary, secondary;

  if (IS_NPC(ch) || IS_NPC(victim))
    return;

  if (IS_SET(ch->act, PLR_LEFTHAND))
  {
    primary = WEAR_HOLD;
    secondary = WEAR_WIELD;
  }
  else if (IS_SET(ch->act, PLR_RIGHTHAND))
  {
    primary = WEAR_WIELD;
    secondary = WEAR_HOLD;
  }
  else
  {
    if (number_percent() >= 50)
    {
      primary = WEAR_WIELD;
      secondary = WEAR_HOLD;
    }
    else
    {
      primary = WEAR_HOLD;
      secondary = WEAR_WIELD;
    }
  }

  if ((obj = get_eq_char(ch, primary)) == NULL)
  {
    if ((obj = get_eq_char(ch, secondary)) == NULL)
    {
      unarmed = TRUE;
    }
  }

  if ((obj != NULL && obj->item_type != ITEM_WEAPON))
    unarmed = TRUE;

  /* The players own decap message */
  if (IS_SET(ch->pcdata->jflags, JFLAG_SETDECAP))
  {
    special_decap_message(ch, victim);
    return;
  }

  if (unarmed)
  {
    if (victim->sex == SEX_MALE)
      sprintf(buf, "#P%s #ygot his head torn off by #R%s#n", victim->name, ch->name);
    else if (victim->sex == SEX_FEMALE)
      sprintf(buf, "#P%s #ygot her head torn off by #R%s#n", victim->name, ch->name);
    else
      sprintf(buf, "#P%s #ygot its head torn off by #R%s#n", victim->name, ch->name);
  }
  else if (obj->value[3] == 1)
  {
    if (victim->sex == SEX_MALE)
      sprintf(buf, "#P%s #ygot his head sliced off by #R%s#n", victim->name, ch->name);
    else if (victim->sex == SEX_FEMALE)
      sprintf(buf, "#P%s #ygot her head sliced off by #R%s#n", victim->name, ch->name);
    else
      sprintf(buf, "#P%s #ygot its head sliced off by #R%s#n", victim->name, ch->name);
  }
  else if (obj->value[3] == 2)
  {
    if (victim->sex == SEX_MALE)
      sprintf(buf, "#P%s #ygot his heart stabbed through by #R%s#n", victim->name, ch->name);
    else if (victim->sex == SEX_FEMALE)
      sprintf(buf, "#P%s #ygot her heart stabbed through by #R%s#n", victim->name, ch->name);
    else
      sprintf(buf, "#P%s #ygot its heart stabbed through by #R%s#n", victim->name, ch->name);
  }
  else if (obj->value[3] == 3)
  {
    if (victim->sex == SEX_MALE)
      sprintf(buf, "#P%s #ygot his head slashed off by #R%s#n", victim->name, ch->name);
    else if (victim->sex == SEX_FEMALE)
      sprintf(buf, "#P%s #ygot her head slashed off by #R%s#n", victim->name, ch->name);
    else
      sprintf(buf, "#P%s #ygot its head slashed off by #R%s#n", victim->name, ch->name);
  }
  else if (obj->value[3] == 4)
    sprintf(buf, "#P%s #ygot strangled by #R%s", victim->name, ch->name);
  else if (obj->value[3] == 5)
    sprintf(buf, "#R%s #yruns a clawed hand through #P%s #yand pulls out the heart#n", ch->name, victim->name);
  else if (obj->value[3] == 6)
    sprintf(buf, "#R%s #yshoots #P%s #yseveral times and spits on the corpse#n", ch->name, victim->name);
  else if (obj->value[3] == 7)
    sprintf(buf, "#R%s #ypounds #P%s #yon the head and the skull caves in#n", ch->name, victim->name);
  else if (obj->value[3] == 8)
    sprintf(buf, "#R%s #ycrushes #P%s #yto a bloody pulp#n", ch->name, victim->name);
  else if (obj->value[3] == 9)
    sprintf(buf, "#P%s #yhas been grepped by #R%s#y, that's just mean!#n", victim->name, ch->name);
  else if (obj->value[3] == 10)
    sprintf(buf, "#P%s #ywas bitten to death by #R%s#n", victim->name, ch->name);
  else if (obj->value[3] == 11)
    sprintf(buf, "#R%s #yhas punctured the lungs of #P%s#y, what a meanie!#n", ch->name, victim->name);
  else if (obj->value[3] == 12)
    sprintf(buf, "#R%s #ygrabs #P%s #yby the head and sucks the brain out#n", ch->name, victim->name);
  else
  {
    if (victim->sex == SEX_MALE)
      sprintf(buf, "#P%s #ygot his head sliced off by #R%s#n", victim->name, ch->name);
    else if (victim->sex == SEX_FEMALE)
      sprintf(buf, "#P%s #ygot her head sliced off by #R%s#n", victim->name, ch->name);
    else
      sprintf(buf, "#P%s #ygot its head sliced off by #R%s#n", victim->name, ch->name);
  }
  death_info(buf);
  return;
}

bool has_timer(CHAR_DATA * ch)
{
  if (ch->fight_timer > 0 && !IS_NPC(ch))
  {
    send_to_char("Not until your fight timer runs out!\n\r", ch);
    return TRUE;
  }
  return FALSE;
}

void autodrop(CHAR_DATA *ch, CHAR_DATA *victim)
{
  char buf[MAX_INPUT_LENGTH];
  char stancename[20];
  int pick = (IS_NPC(victim)) ? STANCE_MOBSTANCE : STANCE_PKSTANCE;

  if (ch->stance[pick] == STANCE_NONE)
    return;

  if (ch->stance[pick] == STANCE_VIPER)
    sprintf(stancename, "viper");
  else if (ch->stance[pick] == STANCE_CRANE)
    sprintf(stancename, "crane");
  else if (ch->stance[pick] == STANCE_CRAB)
    sprintf(stancename, "crab");
  else if (ch->stance[pick] == STANCE_MONGOOSE)
    sprintf(stancename, "mongoose");
  else if (ch->stance[pick] == STANCE_BULL)
    sprintf(stancename, "bull");
  else if (ch->stance[pick] == STANCE_MANTIS)
    sprintf(stancename, "mantis");
  else if (ch->stance[pick] == STANCE_DRAGON)
    sprintf(stancename, "dragon");
  else if (ch->stance[pick] == STANCE_TIGER)
    sprintf(stancename, "tiger");
  else if (ch->stance[pick] == STANCE_MONKEY)
    sprintf(stancename, "monkey");
  else if (ch->stance[pick] == STANCE_SWALLOW)
    sprintf(stancename, "swallow");
  else if (ch->stance[pick] == STANCE_SPIRIT)
    sprintf(stancename, "spirit");
  else
    return;

  if (ch->stance[0] < 1)
  {
    ch->stance[0] = ch->stance[pick];
    sprintf(buf, "#9You autodrop into the #y%s#9 stance.", stancename);
    act(buf, ch, NULL, NULL, TO_CHAR);
    sprintf(buf, "#9$n autodrops into the #y%s#9 stance.", stancename);
    act(buf, ch, NULL, NULL, TO_ROOM);
  }
}

void dropinvis(CHAR_DATA * ch)
{
  if (ch->level < 7 && IS_SET(ch->act, PLR_HIDE))
    REMOVE_BIT(ch->act, PLR_HIDE);
}


void improve_wpn(CHAR_DATA *ch, int dtype, int right_hand)
{
  OBJ_DATA *wield;
  char bufskill[20];
  char buf[MAX_INPUT_LENGTH];
  int dice1, dice2, trapper, max_skl = 200;

  if (IS_NPC(ch))
    return;

  dice1 = number_percent();
  dice2 = number_percent();

  if (right_hand == 1)
    wield = get_eq_char(ch, WEAR_WIELD);
  else if (right_hand == 2)
    wield = get_eq_char(ch, WEAR_HOLD);
  else if (right_hand == 4)
    wield = get_eq_char(ch, WEAR_THIRD);
  else
    wield = get_eq_char(ch, WEAR_FOURTH);

  if (wield == NULL)
    dtype = TYPE_HIT;

  if (dtype == TYPE_UNDEFINED)
  {
    dtype = TYPE_HIT;
    if (wield != NULL && wield->item_type == ITEM_WEAPON)
      dtype += wield->value[3];
  }

  if (dtype == TYPE_HIT && IS_CLASS(ch, CLASS_FAE) && IS_SET(ch->pcdata->powers[FAE_BITS], FAE_BLASTBEAMS))
    dtype += 6; /* blast weapon */

  if (dtype < 1000 || dtype > 1012)
    return;

  dtype -= 1000;

  if (IS_CLASS(ch, CLASS_FAE) && dtype == 6)
    max_skl = 600;
  else if (IS_CLASS(ch, CLASS_FAE))
    max_skl = 225;
  else if (IS_CLASS(ch, CLASS_SHADOW))
    max_skl = 350;
  else if (IS_CLASS(ch, CLASS_WARLOCK))
    max_skl = 250;
  else if (IS_CLASS(ch, CLASS_GIANT))
    max_skl = 300;

  if (ch->generation == 2)
    max_skl += max_skl * 0.1;
  else if (ch->generation == 1)
    max_skl += max_skl * 0.2;

  if (ch->wpn[dtype] >= max_skl)
    return;

  trapper = ch->wpn[dtype];

  if ((dice1 > ch->wpn[dtype] || dice2 > ch->wpn[dtype]) || (dice1 >= 99 || dice2 >= 99))
    ch->wpn[dtype]++;
  else
    return;

  if (trapper == ch->wpn[dtype])
    return;

  if (ch->wpn[dtype] == 1)
    sprintf(bufskill, "slightly skilled");
  else if (ch->wpn[dtype] == 26)
    sprintf(bufskill, "reasonable");
  else if (ch->wpn[dtype] == 51)
    sprintf(bufskill, "fairly competent");
  else if (ch->wpn[dtype] == 76)
    sprintf(bufskill, "highly skilled");
  else if (ch->wpn[dtype] == 101)
    sprintf(bufskill, "very dangerous");
  else if (ch->wpn[dtype] == 126)
    sprintf(bufskill, "extremely deadly");
  else if (ch->wpn[dtype] == 151)
    sprintf(bufskill, "an expert");
  else if (ch->wpn[dtype] == 176)
    sprintf(bufskill, "a master");
  else if (ch->wpn[dtype] == 200)
    sprintf(bufskill, "a grand master");
  else if (ch->wpn[dtype] == 201)
    sprintf(bufskill, "supremely skilled");
  else if (ch->wpn[dtype] == 1000)
    sprintf(bufskill, "divinely skilled");
  else
    return;

  if (wield == NULL || dtype == 0)
  {
    if (IS_CLASS(ch, CLASS_FAE) && IS_SET(ch->pcdata->powers[FAE_BITS], FAE_BLASTBEAMS))
      sprintf(buf, "#GYou are now %s at blasting weapons.#n\n\r", bufskill);
    else
      sprintf(buf, "#GYou are now %s at unarmed combat.#n\n\r", bufskill);
  }
  else
    sprintf(buf, "#GYou are now %s with %s.#n\n\r", bufskill, wield->short_descr);

  send_to_char(buf, ch);
}

void improve_stance(CHAR_DATA * ch)
{
  char buf[MAX_INPUT_LENGTH];
  char bufskill[25];
  char stancename[10];
  int dice1;
  int dice2;
  int stance;
  int max_stance;

  dice1 = number_percent();
  dice2 = number_percent();

  if (IS_NPC(ch))
    return;

  stance = ch->stance[0];
  if (stance < 1 || stance > 17)
    return;

  if (ch->class == CLASS_FAE)
  {
    if (stance == STANCE_SPIRIT && IS_SET(ch->pcdata->powers[EVOLVE_1], FAE_EVOLVE_SPIRIT))
      max_stance = 250;
    else
      max_stance = 200;
  }
  else
  {
    max_stance = 200;
  }

  if (ch->stance[stance] >= max_stance)
  {
    ch->stance[stance] = max_stance;
    return;
  }

  if ((dice1 > ch->stance[stance] && dice2 > ch->stance[stance]) || (dice1 >= 98 || dice2 >= 99))
    ch->stance[stance] += 1;
  else
    return;
  if (stance == ch->stance[stance])
    return;

  if (ch->stance[stance] == 1)
    sprintf(bufskill, "an apprentice of");
  else if (ch->stance[stance] == 26)
    sprintf(bufskill, "a trainee of");
  else if (ch->stance[stance] == 51)
    sprintf(bufskill, "a student of");
  else if (ch->stance[stance] == 76)
    sprintf(bufskill, "fairly experienced in");
  else if (ch->stance[stance] == 101)
    sprintf(bufskill, "well trained in");
  else if (ch->stance[stance] == 126)
    sprintf(bufskill, "highly skilled in");
  else if (ch->stance[stance] == 151)
    sprintf(bufskill, "an expert of");
  else if (ch->stance[stance] == 176)
    sprintf(bufskill, "a master of");
  else if (ch->stance[stance] == 200)
    sprintf(bufskill, "a grand master of");
  else if (ch->stance[stance] == 250)
    sprintf(bufskill, "a guru of");
  else
    return;

  if (stance == STANCE_VIPER)
    sprintf(stancename, "viper");
  else if (stance == STANCE_CRANE)
    sprintf(stancename, "crane");
  else if (stance == STANCE_CRAB)
    sprintf(stancename, "crab");
  else if (stance == STANCE_MONGOOSE)
    sprintf(stancename, "mongoose");
  else if (stance == STANCE_BULL)
    sprintf(stancename, "bull");
  else if (stance == STANCE_MANTIS)
    sprintf(stancename, "mantis");
  else if (stance == STANCE_DRAGON)
    sprintf(stancename, "dragon");
  else if (stance == STANCE_TIGER)
    sprintf(stancename, "tiger");
  else if (stance == STANCE_MONKEY)
    sprintf(stancename, "monkey");
  else if (stance == STANCE_SWALLOW)
    sprintf(stancename, "swallow");
  else if (stance == STANCE_SPIRIT)
    sprintf(stancename, "spirit");
  else
    return;
  sprintf(buf, "#GYou are now %s the %s stance#n.\n\r", bufskill, stancename);
  send_to_char(buf, ch);
}

void critical_hit(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, int dt, int dam)
{
  OBJ_DATA *damaged;
  char buf[MAX_STRING_LENGTH];
  char buf2[100];
  bool vorpal = FALSE;
  int dtype = dt - 1000;
  int critical = 0;
  int count, count2;

  /* the fae class is immune to critical hits */
  if (IS_CLASS(victim, CLASS_FAE))
    return;

  /* only weapon attacks can be critical */
  if (dtype < 0 || dtype > 12)
    return;

  /* increase chance */
  if (IS_NPC(ch))
    critical += ((ch->level + 1) / 5);
  else
    critical += ((ch->wpn[dtype] + 1) / 10);

  /* decrease chance */
  if (IS_NPC(victim))
  {
    critical -= ((victim->level + 1) / 5);
  }
  else
  {
    OBJ_DATA *weap1 = get_eq_char(victim, WEAR_WIELD);
    OBJ_DATA *weap2 = get_eq_char(victim, WEAR_HOLD);
    int wpn1 = 0, wpn2 = 0;

    if (weap1 && weap1->item_type == ITEM_WEAPON)
      wpn1 = URANGE(0, weap1->value[3], 12);
    if (weap2 && weap2->item_type == ITEM_WEAPON)
      wpn2 = URANGE(0, weap2->value[3], 12);  

    if (victim->wpn[wpn1] > victim->wpn[wpn2])
      critical -= ((victim->wpn[wpn1] + 1) / 10);
    else
      critical -= ((victim->wpn[wpn2] + 1) / 10);
  }

  /* cap chance */
  if (critical < 1)
    critical = 1;
  else if (IS_NPC(ch) && critical > 5)
    critical = 5;

  /* check chance */
  if (number_percent() > critical)
    return;

  /* check for vorpal weapon */
  if (obj && obj->item_type == ITEM_WEAPON)
  {
    if (obj->value[0] / 1000 == OBJECT_VORPAL)
      vorpal = TRUE;
  }

  critical = number_range(1, 23);
  if (critical == 1)
  {
    if (IS_HEAD(victim, LOST_EYE_L) && IS_HEAD(victim, LOST_EYE_R))
      return;
    if ((damaged = get_eq_char(victim, WEAR_FACE)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevents you from loosing an eye.", victim, damaged, NULL, TO_CHAR);
      act("$p prevents $n from loosing an eye.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }

    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_HEAD(victim, LOST_EYE_L) && number_percent() < 50)
      SET_BIT(victim->loc_hp[0], LOST_EYE_L);
    else if (!IS_HEAD(victim, LOST_EYE_R))
      SET_BIT(victim->loc_hp[0], LOST_EYE_R);
    else if (!IS_HEAD(victim, LOST_EYE_L))
      SET_BIT(victim->loc_hp[0], LOST_EYE_L);
    else
      return;
    act("Your skillful blow takes out $N's eye!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow takes out $N's eye!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow takes out your eye!", ch, NULL, victim, TO_VICT);
    make_part(victim, "eyeball");
  }
  else if (critical == 2)
  {
    if (IS_HEAD(victim, LOST_EAR_L) && IS_HEAD(victim, LOST_EAR_R))
      return;
    if ((damaged = get_eq_char(victim, WEAR_HEAD)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevents you from loosing an ear.", victim, damaged, NULL, TO_CHAR);
      act("$p prevents $n from loosing an ear.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_HEAD(victim, LOST_EAR_L) && number_percent() < 50)
      SET_BIT(victim->loc_hp[0], LOST_EAR_L);
    else if (!IS_HEAD(victim, LOST_EAR_R))
      SET_BIT(victim->loc_hp[0], LOST_EAR_R);
    else if (!IS_HEAD(victim, LOST_EAR_L))
      SET_BIT(victim->loc_hp[0], LOST_EAR_L);
    else
      return;
    act("Your skillful blow cuts off $N's ear!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow cuts off $N's ear!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow cuts off your ear!", ch, NULL, victim, TO_VICT);
    make_part(victim, "ear");
  }
  else if (critical == 3)
  {
    if (IS_HEAD(victim, LOST_NOSE))
      return;
    if ((damaged = get_eq_char(victim, WEAR_FACE)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevents you from loosing your nose.", victim, damaged, NULL, TO_CHAR);
      act("$p prevents $n from loosing $s nose.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    SET_BIT(victim->loc_hp[0], LOST_NOSE);
    act("Your skillful blow cuts off $N's nose!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow cuts off $N's nose!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow cuts off your nose!", ch, NULL, victim, TO_VICT);
    make_part(victim, "nose");
  }
  else if (critical == 4)
  {
    if (IS_HEAD(victim, LOST_NOSE) || IS_HEAD(victim, BROKEN_NOSE))
      return;
    if ((damaged = get_eq_char(victim, WEAR_FACE)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevents you from breaking your nose.", victim, damaged, NULL, TO_CHAR);
      act("$p prevents $n from breaking $s nose.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_HEAD(victim, LOST_NOSE) && !IS_HEAD(victim, BROKEN_NOSE))
      SET_BIT(victim->loc_hp[0], BROKEN_NOSE);
    else
      return;
    act("Your skillful blow breaks $N's nose!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow breaks $N's nose!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow breaks your nose!", ch, NULL, victim, TO_VICT);
  }
  else if (critical == 5)
  {
    if (IS_HEAD(victim, BROKEN_JAW))
      return;
    if ((damaged = get_eq_char(victim, WEAR_FACE)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevents you from breaking your jaw.", victim, damaged, NULL, TO_CHAR);
      act("$p prevents $n from breaking $s jaw.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_HEAD(victim, BROKEN_JAW))
      SET_BIT(victim->loc_hp[0], BROKEN_JAW);
    else
      return;
    act("Your skillful blow breaks $N's jaw!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow breaks $N's jaw!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow breaks your jaw!", ch, NULL, victim, TO_VICT);
  }
  else if (critical == 6)
  {
    if (IS_ARM_L(victim, LOST_ARM))
      return;
    if ((damaged = get_eq_char(victim, WEAR_ARMS)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from loosing your left arm.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from loosing $s left arm.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }

    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_ARM_L(victim, LOST_ARM))
      SET_BIT(victim->loc_hp[2], LOST_ARM);
    else
      return;
    if (!IS_BLEEDING(victim, BLEEDING_ARM_L))
      SET_BIT(victim->loc_hp[6], BLEEDING_ARM_L);
    if (IS_BLEEDING(victim, BLEEDING_HAND_L))
      REMOVE_BIT(victim->loc_hp[6], BLEEDING_HAND_L);
    act("Your skillful blow cuts off $N's left arm!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow cuts off $N's left arm!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow cuts off your left arm!", ch, NULL, victim, TO_VICT);
    make_part(victim, "arm");
    if (IS_ARM_L(victim, LOST_ARM) && IS_ARM_R(victim, LOST_ARM))
    {
      if ((obj = get_eq_char(victim, WEAR_ARMS)) != NULL)
        take_item(victim, obj);
    }
    if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
      take_item(victim, obj);
    if ((obj = get_eq_char(victim, WEAR_HANDS)) != NULL)
      take_item(victim, obj);
    if ((obj = get_eq_char(victim, WEAR_WRIST_L)) != NULL)
      take_item(victim, obj);
    if ((obj = get_eq_char(victim, WEAR_FINGER_L)) != NULL)
      take_item(victim, obj);
  }
  else if (critical == 7)
  {
    if (IS_ARM_R(victim, LOST_ARM))
      return;
    if ((damaged = get_eq_char(victim, WEAR_ARMS)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from loosing your right arm.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from loosing $s right arm.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }

    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_ARM_R(victim, LOST_ARM))
      SET_BIT(victim->loc_hp[3], LOST_ARM);
    else
      return;
    if (!IS_BLEEDING(victim, BLEEDING_ARM_R))
      SET_BIT(victim->loc_hp[6], BLEEDING_ARM_R);
    if (IS_BLEEDING(victim, BLEEDING_HAND_R))
      REMOVE_BIT(victim->loc_hp[6], BLEEDING_HAND_R);
    act("Your skillful blow cuts off $N's right arm!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow cuts off $N's right arm!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow cuts off your right arm!", ch, NULL, victim, TO_VICT);
    make_part(victim, "arm");
    if (IS_ARM_L(victim, LOST_ARM) && IS_ARM_R(victim, LOST_ARM))
    {
      if ((obj = get_eq_char(victim, WEAR_ARMS)) != NULL)
        take_item(victim, obj);
    }
    if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
      take_item(victim, obj);
    if ((obj = get_eq_char(victim, WEAR_HANDS)) != NULL)
      take_item(victim, obj);
    if ((obj = get_eq_char(victim, WEAR_WRIST_R)) != NULL)
      take_item(victim, obj);
    if ((obj = get_eq_char(victim, WEAR_FINGER_R)) != NULL)
      take_item(victim, obj);
  }
  else if (critical == 8)
  {
    if (IS_ARM_L(victim, LOST_ARM) || IS_ARM_L(victim, BROKEN_ARM))
      return;
    if ((damaged = get_eq_char(victim, WEAR_ARMS)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from breaking your left arm.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from breaking $s left arm.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }

    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_ARM_L(victim, BROKEN_ARM) && !IS_ARM_L(victim, LOST_ARM))
      SET_BIT(victim->loc_hp[2], BROKEN_ARM);
    else
      return;
    act("Your skillful blow breaks $N's left arm!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow breaks $N's left arm!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow breaks your left arm!", ch, NULL, victim, TO_VICT);
    if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
      take_item(victim, obj);
  }
  else if (critical == 9)
  {
    if (IS_ARM_R(victim, LOST_ARM) || IS_ARM_R(victim, BROKEN_ARM))
      return;
    if ((damaged = get_eq_char(victim, WEAR_ARMS)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from breaking your right arm.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from breaking $s right arm.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }

    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_ARM_R(victim, BROKEN_ARM) && !IS_ARM_R(victim, LOST_ARM))
      SET_BIT(victim->loc_hp[3], BROKEN_ARM);
    else
      return;
    act("Your skillful blow breaks $N's right arm!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow breaks $N's right arm!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow breaks your right arm!", ch, NULL, victim, TO_VICT);
    if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
      take_item(victim, obj);
  }
  else if (critical == 10)
  {
    if (IS_ARM_L(victim, LOST_HAND) || IS_ARM_L(victim, LOST_ARM))
      return;
    if ((damaged = get_eq_char(victim, WEAR_HANDS)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from loosing your left hand.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from loosing $s left hand.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }

    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_ARM_L(victim, LOST_HAND) && !IS_ARM_L(victim, LOST_ARM))
      SET_BIT(victim->loc_hp[2], LOST_HAND);
    else
      return;
    if (IS_BLEEDING(victim, BLEEDING_ARM_L))
      REMOVE_BIT(victim->loc_hp[6], BLEEDING_ARM_L);
    if (!IS_BLEEDING(victim, BLEEDING_HAND_L))
      SET_BIT(victim->loc_hp[6], BLEEDING_HAND_L);
    act("Your skillful blow cuts off $N's left hand!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow cuts off $N's left hand!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow cuts off your left hand!", ch, NULL, victim, TO_VICT);
    make_part(victim, "hand");
    if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
      take_item(victim, obj);
    if ((obj = get_eq_char(victim, WEAR_HANDS)) != NULL)
      take_item(victim, obj);
    if ((obj = get_eq_char(victim, WEAR_WRIST_L)) != NULL)
      take_item(victim, obj);
    if ((obj = get_eq_char(victim, WEAR_FINGER_L)) != NULL)
      take_item(victim, obj);
  }
  else if (critical == 11)
  {
    if (IS_ARM_R(victim, LOST_HAND) || IS_ARM_R(victim, LOST_ARM))
      return;
    if ((damaged = get_eq_char(victim, WEAR_HANDS)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from loosing your right hand.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from loosing $s right hand.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }

    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_ARM_R(victim, LOST_HAND) && !IS_ARM_R(victim, LOST_ARM))
      SET_BIT(victim->loc_hp[3], LOST_HAND);
    else
      return;
    if (IS_BLEEDING(victim, BLEEDING_ARM_R))
      REMOVE_BIT(victim->loc_hp[6], BLEEDING_ARM_R);
    if (!IS_BLEEDING(victim, BLEEDING_HAND_R))
      SET_BIT(victim->loc_hp[6], BLEEDING_HAND_R);
    act("Your skillful blow cuts off $N's right hand!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow cuts off $N's right hand!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow cuts off your right hand!", ch, NULL, victim, TO_VICT);
    make_part(victim, "hand");
    if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
      take_item(victim, obj);
    if ((obj = get_eq_char(victim, WEAR_HANDS)) != NULL)
      take_item(victim, obj);
    if ((obj = get_eq_char(victim, WEAR_WRIST_R)) != NULL)
      take_item(victim, obj);
    if ((obj = get_eq_char(victim, WEAR_FINGER_R)) != NULL)
      take_item(victim, obj);
  }
  else if (critical == 12)
  {
    if (IS_ARM_L(victim, LOST_ARM))
      return;
    if (IS_ARM_L(victim, LOST_HAND))
      return;
    if (IS_ARM_L(victim, LOST_THUMB) && IS_ARM_L(victim, LOST_FINGER_I) && IS_ARM_L(victim, LOST_FINGER_M) && IS_ARM_L(victim, LOST_FINGER_R) && IS_ARM_L(victim, LOST_FINGER_L))
      return;
    if ((damaged = get_eq_char(victim, WEAR_HANDS)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from loosing some fingers from your left hand.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from loosing some fingers from $s left hand.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    count = 0;
    count2 = 0;
    if (!IS_ARM_L(victim, LOST_THUMB) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[2], LOST_THUMB);
      count2 += 1;
      make_part(victim, "thumb");
    }
    if (!IS_ARM_L(victim, LOST_FINGER_I) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[2], LOST_FINGER_I);
      count += 1;
      make_part(victim, "index");
    }
    if (!IS_ARM_L(victim, LOST_FINGER_M) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[2], LOST_FINGER_M);
      count += 1;
      make_part(victim, "middle");
    }
    if (!IS_ARM_L(victim, LOST_FINGER_R) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[2], LOST_FINGER_R);
      count += 1;
      make_part(victim, "ring");
      if ((obj = get_eq_char(victim, WEAR_FINGER_L)) != NULL)
        take_item(victim, obj);
    }
    if (!IS_ARM_L(victim, LOST_FINGER_L) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[2], LOST_FINGER_L);
      count += 1;
      make_part(victim, "little");
    }
    if (count == 1)
      sprintf(buf2, "finger");
    else
      sprintf(buf2, "fingers");
    if (count > 0 && count2 > 0)
    {
      sprintf(buf, "Your skillful blow cuts off %d %s and the thumb from $N's left hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "$n's skillful blow cuts off %d %s and the thumb from $N's left hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_NOTVICT);
      sprintf(buf, "$n's skillful blow cuts off %d %s and the thumb from your left hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_VICT);

      if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
        take_item(victim, obj);
      return;
    }
    else if (count > 0)
    {
      sprintf(buf, "Your skillful blow cuts off %d %s from $N's left hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "$n's skillful blow cuts off %d %s from $N's left hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_NOTVICT);
      sprintf(buf, "$n's skillful blow cuts off %d %s from your left hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_VICT);

      if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
        take_item(victim, obj);
      return;
    }
    else if (count2 > 0)
    {
      sprintf(buf, "Your skillful blow cuts off the thumb from $N's left hand.");
      act(buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "$n's skillful blow cuts off the thumb from $N's left hand.");
      act(buf, ch, NULL, victim, TO_NOTVICT);
      sprintf(buf, "$n's skillful blow cuts off the thumb from your left hand.");
      act(buf, ch, NULL, victim, TO_VICT);

      if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
        take_item(victim, obj);
      return;
    }
  }
  else if (critical == 13)
  {
    if (IS_ARM_L(victim, LOST_ARM))
      return;
    if (IS_ARM_L(victim, LOST_HAND))
      return;
    if ((IS_ARM_L(victim, LOST_THUMB) || IS_ARM_L(victim, BROKEN_THUMB)) &&
        (IS_ARM_L(victim, LOST_FINGER_I) || IS_ARM_L(victim, BROKEN_FINGER_I)) &&
        (IS_ARM_L(victim, LOST_FINGER_M) || IS_ARM_L(victim, BROKEN_FINGER_M)) &&
        (IS_ARM_L(victim, LOST_FINGER_R) || IS_ARM_L(victim, BROKEN_FINGER_R)) && (IS_ARM_L(victim, LOST_FINGER_L) || IS_ARM_L(victim, BROKEN_FINGER_L)))
      return;
    if ((damaged = get_eq_char(victim, WEAR_HANDS)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from breaking some fingers on your left hand.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from breaking some fingers on $s left hand.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    count = 0;
    count2 = 0;
    if (IS_ARM_L(victim, LOST_ARM))
      return;
    if (IS_ARM_L(victim, LOST_HAND))
      return;

    if (!IS_ARM_L(victim, BROKEN_THUMB) && !IS_ARM_L(victim, LOST_THUMB) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[2], BROKEN_THUMB);
      count2 += 1;
    }
    if (!IS_ARM_L(victim, BROKEN_FINGER_I) && !IS_ARM_L(victim, LOST_FINGER_I) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[2], BROKEN_FINGER_I);
      count += 1;
    }
    if (!IS_ARM_L(victim, BROKEN_FINGER_M) && !IS_ARM_L(victim, LOST_FINGER_M) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[2], BROKEN_FINGER_M);
      count += 1;
    }
    if (!IS_ARM_L(victim, BROKEN_FINGER_R) && !IS_ARM_L(victim, LOST_FINGER_R) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[2], BROKEN_FINGER_R);
      count += 1;
    }
    if (!IS_ARM_L(victim, BROKEN_FINGER_L) && !IS_ARM_L(victim, LOST_FINGER_L) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[2], BROKEN_FINGER_L);
      count += 1;
    }
    if (count == 1)
      sprintf(buf2, "finger");
    else
      sprintf(buf2, "fingers");
    if (count > 0 && count2 > 0)
    {
      sprintf(buf, "Your skillful breaks %d %s and the thumb on $N's left hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "$n's skillful blow breaks %d %s and the thumb on $N's left hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_NOTVICT);
      sprintf(buf, "$n's skillful blow breaks %d %s and the thumb on your left hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_VICT);

      if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
        take_item(victim, obj);
      return;
    }
    else if (count > 0)
    {
      sprintf(buf, "Your skillful blow breaks %d %s on $N's left hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "$n's skillful blow breaks %d %s on $N's left hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_NOTVICT);
      sprintf(buf, "$n's skillful blow breaks %d %s on your left hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_VICT);

      if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
        take_item(victim, obj);
      return;
    }
    else if (count2 > 0)
    {
      sprintf(buf, "Your skillful blow breaks the thumb on $N's left hand.");
      act(buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "$n's skillful blow breaks the thumb on $N's left hand.");
      act(buf, ch, NULL, victim, TO_NOTVICT);
      sprintf(buf, "$n's skillful blow breaks the thumb on your left hand.");
      act(buf, ch, NULL, victim, TO_VICT);

      if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
        take_item(victim, obj);
      return;
    }
  }
  else if (critical == 14)
  {
    if (IS_ARM_R(victim, LOST_ARM))
      return;
    if (IS_ARM_R(victim, LOST_HAND))
      return;
    if (IS_ARM_R(victim, LOST_THUMB) && IS_ARM_R(victim, LOST_FINGER_I) && IS_ARM_R(victim, LOST_FINGER_M) && IS_ARM_R(victim, LOST_FINGER_R) && IS_ARM_R(victim, LOST_FINGER_L))
      return;
    if ((damaged = get_eq_char(victim, WEAR_HANDS)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from loosing some fingers from your right hand.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from loosing some fingers from $s right hand.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    count = 0;
    count2 = 0;
    if (IS_ARM_R(victim, LOST_ARM))
      return;
    if (IS_ARM_R(victim, LOST_HAND))
      return;

    if (!IS_ARM_R(victim, LOST_THUMB) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[3], LOST_THUMB);
      count2 += 1;
      make_part(victim, "thumb");
    }
    if (!IS_ARM_R(victim, LOST_FINGER_I) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[3], LOST_FINGER_I);
      count += 1;
      make_part(victim, "index");
    }
    if (!IS_ARM_R(victim, LOST_FINGER_M) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[3], LOST_FINGER_M);
      count += 1;
      make_part(victim, "middle");
    }
    if (!IS_ARM_R(victim, LOST_FINGER_R) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[3], LOST_FINGER_R);
      count += 1;
      make_part(victim, "ring");
      if ((obj = get_eq_char(victim, WEAR_FINGER_R)) != NULL)
        take_item(victim, obj);
    }
    if (!IS_ARM_R(victim, LOST_FINGER_L) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[3], LOST_FINGER_L);
      count += 1;
      make_part(victim, "little");
    }
    if (count == 1)
      sprintf(buf2, "finger");
    else
      sprintf(buf2, "fingers");
    if (count > 0 && count2 > 0)
    {
      sprintf(buf, "Your skillful blow cuts off %d %s and the thumb from $N's right hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "$n's skillful blow cuts off %d %s and the thumb from $N's right hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_NOTVICT);
      sprintf(buf, "$n's skillful blow cuts off %d %s and the thumb from your right hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_VICT);

      if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
        take_item(victim, obj);
      return;
    }
    else if (count > 0)
    {
      sprintf(buf, "Your skillful blow cuts off %d %s from $N's right hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "$n's skillful blow cuts off %d %s from $N's right hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_NOTVICT);
      sprintf(buf, "$n's skillful blow cuts off %d %s from your right hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_VICT);

      if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
        take_item(victim, obj);
      return;
    }
    else if (count2 > 0)
    {
      sprintf(buf, "Your skillful blow cuts off the thumb from $N's right hand.");
      act(buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "$n's skillful blow cuts off the thumb from $N's right hand.");
      act(buf, ch, NULL, victim, TO_NOTVICT);
      sprintf(buf, "$n's skillful blow cuts off the thumb from your right hand.");
      act(buf, ch, NULL, victim, TO_VICT);

      if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
        take_item(victim, obj);
      return;
    }
  }
  else if (critical == 15)
  {
    if (IS_ARM_R(victim, LOST_ARM))
      return;
    if (IS_ARM_R(victim, LOST_HAND))
      return;
    if ((IS_ARM_R(victim, LOST_THUMB) || IS_ARM_R(victim, BROKEN_THUMB)) &&
        (IS_ARM_R(victim, LOST_FINGER_I) || IS_ARM_R(victim, BROKEN_FINGER_I)) &&
        (IS_ARM_R(victim, LOST_FINGER_M) || IS_ARM_R(victim, BROKEN_FINGER_M)) &&
        (IS_ARM_R(victim, LOST_FINGER_R) || IS_ARM_R(victim, BROKEN_FINGER_R)) && (IS_ARM_R(victim, LOST_FINGER_L) || IS_ARM_R(victim, BROKEN_FINGER_L)))
      return;
    if ((damaged = get_eq_char(victim, WEAR_HANDS)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from breaking some fingers on your right hand.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from breaking some fingers on $s right hand.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    count = 0;
    count2 = 0;
    if (IS_ARM_R(victim, LOST_ARM))
      return;
    if (IS_ARM_R(victim, LOST_HAND))
      return;

    if (!IS_ARM_R(victim, BROKEN_THUMB) && !IS_ARM_R(victim, LOST_THUMB) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[3], BROKEN_THUMB);
      count2 += 1;
    }
    if (!IS_ARM_R(victim, BROKEN_FINGER_I) && !IS_ARM_R(victim, LOST_FINGER_I) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[3], BROKEN_FINGER_I);
      count += 1;
    }
    if (!IS_ARM_R(victim, BROKEN_FINGER_M) && !IS_ARM_R(victim, LOST_FINGER_M) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[3], BROKEN_FINGER_M);
      count += 1;
    }
    if (!IS_ARM_R(victim, BROKEN_FINGER_R) && !IS_ARM_R(victim, LOST_FINGER_R) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[3], BROKEN_FINGER_R);
      count += 1;
    }
    if (!IS_ARM_R(victim, BROKEN_FINGER_L) && !IS_ARM_R(victim, LOST_FINGER_L) && number_percent() < 25)
    {
      SET_BIT(victim->loc_hp[3], BROKEN_FINGER_L);
      count += 1;
    }
    if (count == 1)
      sprintf(buf2, "finger");
    else
      sprintf(buf2, "fingers");
    if (count > 0 && count2 > 0)
    {
      sprintf(buf, "Your skillful breaks %d %s and the thumb on $N's right hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "$n's skillful blow breaks %d %s and the thumb on $N's right hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_NOTVICT);
      sprintf(buf, "$n's skillful blow breaks %d %s and the thumb on your right hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_VICT);

      if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
        take_item(victim, obj);
      return;
    }
    else if (count > 0)
    {
      sprintf(buf, "Your skillful blow breaks %d %s on $N's right hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "$n's skillful blow breaks %d %s on $N's right hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_NOTVICT);
      sprintf(buf, "$n's skillful blow breaks %d %s on your right hand.", count, buf2);
      act(buf, ch, NULL, victim, TO_VICT);

      if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
        take_item(victim, obj);
      return;
    }
    else if (count2 > 0)
    {
      sprintf(buf, "Your skillful blow breaks the thumb on $N's right hand.");
      act(buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "$n's skillful blow breaks the thumb on $N's right hand.");
      act(buf, ch, NULL, victim, TO_NOTVICT);
      sprintf(buf, "$n's skillful blow breaks the thumb on your right hand.");
      act(buf, ch, NULL, victim, TO_VICT);

      if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
        take_item(victim, obj);
      return;
    }
  }
  else if (critical == 16)
  {
    if (IS_LEG_L(victim, LOST_LEG))
      return;
    if ((damaged = get_eq_char(victim, WEAR_LEGS)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from loosing your left leg.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from loosing $s left leg.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_LEG_L(victim, LOST_LEG))
      SET_BIT(victim->loc_hp[4], LOST_LEG);
    else
      return;
    if (!IS_BLEEDING(victim, BLEEDING_LEG_L))
      SET_BIT(victim->loc_hp[6], BLEEDING_LEG_L);
    if (IS_BLEEDING(victim, BLEEDING_FOOT_L))
      REMOVE_BIT(victim->loc_hp[6], BLEEDING_FOOT_L);
    act("Your skillful blow cuts off $N's left leg!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow cuts off $N's left leg!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow cuts off your left leg!", ch, NULL, victim, TO_VICT);
    make_part(victim, "leg");

    if (IS_LEG_L(victim, LOST_LEG) && IS_LEG_R(victim, LOST_LEG))
    {
      if ((obj = get_eq_char(victim, WEAR_LEGS)) != NULL)
        take_item(victim, obj);
    }
    if ((obj = get_eq_char(victim, WEAR_FEET)) != NULL)
      take_item(victim, obj);
  }
  else if (critical == 17)
  {
    if (IS_LEG_R(victim, LOST_LEG))
      return;
    if ((damaged = get_eq_char(victim, WEAR_LEGS)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from loosing your right leg.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from loosing $s right leg.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_LEG_R(victim, LOST_LEG))
      SET_BIT(victim->loc_hp[5], LOST_LEG);
    else
      return;
    if (!IS_BLEEDING(victim, BLEEDING_LEG_R))
      SET_BIT(victim->loc_hp[6], BLEEDING_LEG_R);
    if (IS_BLEEDING(victim, BLEEDING_FOOT_R))
      REMOVE_BIT(victim->loc_hp[6], BLEEDING_FOOT_R);
    act("Your skillful blow cuts off $N's right leg!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow cuts off $N's right leg!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow cuts off your right leg!", ch, NULL, victim, TO_VICT);
    make_part(victim, "leg");

    if (IS_LEG_L(victim, LOST_LEG) && IS_LEG_R(victim, LOST_LEG))
    {
      if ((obj = get_eq_char(victim, WEAR_LEGS)) != NULL)
        take_item(victim, obj);
    }
    if ((obj = get_eq_char(victim, WEAR_FEET)) != NULL)
      take_item(victim, obj);
  }
  else if (critical == 18)
  {
    if (IS_LEG_L(victim, BROKEN_LEG) || IS_LEG_L(victim, LOST_LEG))
      return;
    if ((damaged = get_eq_char(victim, WEAR_LEGS)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from breaking your left leg.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from breaking $s left leg.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_LEG_L(victim, BROKEN_LEG) && !IS_LEG_L(victim, LOST_LEG))
      SET_BIT(victim->loc_hp[4], BROKEN_LEG);
    else
      return;
    act("Your skillful blow breaks $N's left leg!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow breaks $N's left leg!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow breaks your left leg!", ch, NULL, victim, TO_VICT);
  }
  else if (critical == 19)
  {
    if (IS_LEG_R(victim, BROKEN_LEG) || IS_LEG_R(victim, LOST_LEG))
      return;
    if ((damaged = get_eq_char(victim, WEAR_LEGS)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from breaking your right leg.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from breaking $s right leg.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_LEG_R(victim, BROKEN_LEG) && !IS_LEG_R(victim, LOST_LEG))
      SET_BIT(victim->loc_hp[5], BROKEN_LEG);
    else
      return;
    act("Your skillful blow breaks $N's right leg!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow breaks $N's right leg!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow breaks your right leg!", ch, NULL, victim, TO_VICT);
  }
  else if (critical == 20)
  {
    if (IS_LEG_L(victim, LOST_LEG) || IS_LEG_L(victim, LOST_FOOT))
      return;
    if ((damaged = get_eq_char(victim, WEAR_FEET)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from loosing your left foot.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from loosing $s left foot.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_LEG_L(victim, LOST_LEG) && !IS_LEG_L(victim, LOST_FOOT))
      SET_BIT(victim->loc_hp[4], LOST_FOOT);
    else
      return;
    if (IS_BLEEDING(victim, BLEEDING_LEG_L))
      REMOVE_BIT(victim->loc_hp[6], BLEEDING_LEG_L);
    if (!IS_BLEEDING(victim, BLEEDING_FOOT_L))
      SET_BIT(victim->loc_hp[6], BLEEDING_FOOT_L);
    act("Your skillful blow cuts off $N's left foot!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow cuts off $N's left foot!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow cuts off your left foot!", ch, NULL, victim, TO_VICT);
    make_part(victim, "foot");
    if ((obj = get_eq_char(victim, WEAR_FEET)) != NULL)
      take_item(victim, obj);
  }
  else if (critical == 21)
  {
    if (IS_LEG_R(victim, LOST_LEG) || IS_LEG_R(victim, LOST_FOOT))
      return;
    if ((damaged = get_eq_char(victim, WEAR_FEET)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevent you from loosing your right foot.", victim, damaged, NULL, TO_CHAR);
      act("$p prevent $n from loosing $s right foot.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*    
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

    if (!IS_LEG_R(victim, LOST_LEG) && !IS_LEG_R(victim, LOST_FOOT))
      SET_BIT(victim->loc_hp[5], LOST_FOOT);
    else
      return;
    if (IS_BLEEDING(victim, BLEEDING_LEG_R))
      REMOVE_BIT(victim->loc_hp[6], BLEEDING_LEG_R);
    if (!IS_BLEEDING(victim, BLEEDING_FOOT_R))
      SET_BIT(victim->loc_hp[6], BLEEDING_FOOT_R);
    act("Your skillful blow cuts off $N's right foot!", ch, NULL, victim, TO_CHAR);
    act("$n's skillful blow cuts off $N's right foot!", ch, NULL, victim, TO_NOTVICT);
    act("$n's skillful blow cuts off your right foot!", ch, NULL, victim, TO_VICT);
    make_part(victim, "foot");
    if ((obj = get_eq_char(victim, WEAR_FEET)) != NULL)
      take_item(victim, obj);
  }
  else if (critical == 22)
  {
    int bodyloc = 0;
    int broken = number_range(1, 3);

    if (IS_BODY(victim, BROKEN_RIBS_1))
      bodyloc += 1;
    if (IS_BODY(victim, BROKEN_RIBS_2))
      bodyloc += 2;
    if (IS_BODY(victim, BROKEN_RIBS_4))
      bodyloc += 4;
    if (IS_BODY(victim, BROKEN_RIBS_8))
      bodyloc += 8;
    if (IS_BODY(victim, BROKEN_RIBS_16))
      bodyloc += 16;
    if (bodyloc >= 24)
      return;

    if ((damaged = get_eq_char(victim, WEAR_BODY)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevents you from breaking some ribs.", victim, damaged, NULL, TO_CHAR);
      act("$p prevents $n from breaking some ribs.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*    
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

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
    if (bodyloc + broken > 24)
      broken -= 1;
    if (bodyloc + broken > 24)
      broken -= 1;
    bodyloc += broken;
    if (bodyloc >= 16)
    {
      bodyloc -= 16;
      SET_BIT(victim->loc_hp[1], BROKEN_RIBS_16);
    }
    if (bodyloc >= 8)
    {
      bodyloc -= 8;
      SET_BIT(victim->loc_hp[1], BROKEN_RIBS_8);
    }
    if (bodyloc >= 4)
    {
      bodyloc -= 4;
      SET_BIT(victim->loc_hp[1], BROKEN_RIBS_4);
    }
    if (bodyloc >= 2)
    {
      bodyloc -= 2;
      SET_BIT(victim->loc_hp[1], BROKEN_RIBS_2);
    }
    if (bodyloc >= 1)
    {
      bodyloc -= 1;
      SET_BIT(victim->loc_hp[1], BROKEN_RIBS_1);
    }
    sprintf(buf, "Your skillful blow breaks %d of $N's ribs!", broken);
    act(buf, ch, NULL, victim, TO_CHAR);
    sprintf(buf, "$n's skillful blow breaks %d of $N's ribs!", broken);
    act(buf, ch, NULL, victim, TO_NOTVICT);
    sprintf(buf, "$n's skillful blow breaks %d of your ribs!", broken);
    act(buf, ch, NULL, victim, TO_VICT);
  }
  else if (critical == 23)
  {
    int bodyloc = 0;
    int broken = number_range(1, 3);

    if (IS_HEAD(victim, LOST_TOOTH_1))
      bodyloc += 1;
    if (IS_HEAD(victim, LOST_TOOTH_2))
      bodyloc += 2;
    if (IS_HEAD(victim, LOST_TOOTH_4))
      bodyloc += 4;
    if (IS_HEAD(victim, LOST_TOOTH_8))
      bodyloc += 8;
    if (IS_HEAD(victim, LOST_TOOTH_16))
      bodyloc += 16;
    if (bodyloc >= 28)
      return;

    if ((damaged = get_eq_char(victim, WEAR_FACE)) != NULL && damaged->toughness > 0 && !vorpal)
    {
      act("$p prevents you from loosing some teeth.", victim, damaged, NULL, TO_CHAR);
      act("$p prevents $n from loosing some teeth.", victim, damaged, NULL, TO_ROOM);
      damage_obj(ch, damaged, dam);
      return;
    }
    /*
     * Stone skin will prevent any damage done to arms/legs.
     */
    if (is_affected(victim, skill_lookup("stone skin")))
      return;

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
    if (bodyloc + broken > 28)
      broken -= 1;
    if (bodyloc + broken > 28)
      broken -= 1;
    bodyloc += broken;
    if (bodyloc >= 16)
    {
      bodyloc -= 16;
      SET_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_16);
    }
    if (bodyloc >= 8)
    {
      bodyloc -= 8;
      SET_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_8);
    }
    if (bodyloc >= 4)
    {
      bodyloc -= 4;
      SET_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_4);
    }
    if (bodyloc >= 2)
    {
      bodyloc -= 2;
      SET_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_2);
    }
    if (bodyloc >= 1)
    {
      bodyloc -= 1;
      SET_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_1);
    }
    sprintf(buf, "Your skillful blow knocks out %d of $N's teeth!", broken);
    act(buf, ch, NULL, victim, TO_CHAR);
    sprintf(buf, "$n's skillful blow knocks out %d of $N's teeth!", broken);
    act(buf, ch, NULL, victim, TO_NOTVICT);
    sprintf(buf, "$n's skillful blow knocks out %d of your teeth!", broken);
    act(buf, ch, NULL, victim, TO_VICT);
    if (broken >= 1)
      make_part(victim, "tooth");
    if (broken >= 2)
      make_part(victim, "tooth");
    if (broken >= 3)
      make_part(victim, "tooth");
    return;
  }
}

void special_move(CHAR_DATA * ch, CHAR_DATA * victim)
{
  int dam = number_range(5, 10) + char_damroll(ch);

  if (dam < 10)
    dam = 10;

  switch (number_range(1, 7))
  {
    default:
      return;
    case 1:
      act("You pull your hands into your waist then snap them into $N's stomach.", ch, NULL, victim, TO_CHAR);
      act("$n pulls $s hands into $s waist then snaps them into your stomach.", ch, NULL, victim, TO_VICT);
      act("$n pulls $s hands into $s waist then snaps them into $N's stomach.", ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, NULL, dam, gsn_punch);
      if (victim == NULL || victim->position == POS_DEAD)
        return;
      act("You double over in agony, and fall to the ground gasping for breath.", victim, NULL, NULL, TO_CHAR);
      act("$n doubles over in agony, and falls to the ground gasping for breath.", victim, NULL, NULL, TO_ROOM);
      stop_fighting(victim, TRUE);
      victim->position = POS_STUNNED;
      break;
    case 2:
      act("You spin in a low circle, catching $N behind $S ankle.", ch, NULL, victim, TO_CHAR);
      act("$n spins in a low circle, catching you behind your ankle.", ch, NULL, victim, TO_VICT);
      act("$n spins in a low circle, catching $N behind $S ankle.", ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, NULL, dam, gsn_sweep);
      if (victim == NULL || victim->position == POS_DEAD)
        return;
      if (number_percent() <= 25 && !IS_LEG_L(victim, BROKEN_LEG) && !IS_LEG_L(victim, LOST_LEG))
      {
        act("Your left leg shatters under the impact of the blow!", victim, NULL, NULL, TO_CHAR);
        act("$n's left leg shatters under the impact of the blow!", victim, NULL, NULL, TO_ROOM);
        SET_BIT(victim->loc_hp[LOC_LEG_L], BROKEN_LEG);
      }
      else if (number_percent() <= 25 && !IS_LEG_R(victim, BROKEN_LEG) && !IS_LEG_R(victim, LOST_LEG))
      {
        act("Your right leg shatters under the impact of the blow!", victim, NULL, NULL, TO_CHAR);
        act("$n's right leg shatters under the impact of the blow!", victim, NULL, NULL, TO_ROOM);
        SET_BIT(victim->loc_hp[LOC_LEG_R], BROKEN_LEG);
      }
      act("You crash to the ground, stunned.", victim, NULL, NULL, TO_CHAR);
      act("$n crashes to the ground, stunned.", victim, NULL, NULL, TO_ROOM);
      stop_fighting(victim, TRUE);
      victim->position = POS_STUNNED;
      break;
    case 3:
      act("You roll between $N's legs and flip to your feet.", ch, NULL, victim, TO_CHAR);
      act("$n rolls between your legs and flips to $s feet.", ch, NULL, victim, TO_VICT);
      act("$n rolls between $N's legs and flips to $s feet.", ch, NULL, victim, TO_NOTVICT);
      act("You spin around and smash your elbow into the back of $N's head.", ch, NULL, victim, TO_CHAR);
      act("$n spins around and smashes $s elbow into the back of your head.", ch, NULL, victim, TO_VICT);
      act("$n spins around and smashes $s elbow into the back of $N's head.", ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, NULL, dam, gsn_elbow);
      if (victim == NULL || victim->position == POS_DEAD)
        return;
      act("You fall to the ground, stunned.", victim, NULL, NULL, TO_CHAR);
      act("$n falls to the ground, stunned.", victim, NULL, NULL, TO_ROOM);
      stop_fighting(victim, TRUE);
      victim->position = POS_STUNNED;
      break;
    case 4:
      act("You somersault over $N's head and land lightly on your toes.", ch, NULL, victim, TO_CHAR);
      act("$n somersaults over your head and lands lightly on $s toes.", ch, NULL, victim, TO_VICT);
      act("$n somersaults over $N's head and lands lightly on $s toes.", ch, NULL, victim, TO_NOTVICT);
      act("You roll back onto your shoulders and kick both feet into $N's back.", ch, NULL, victim, TO_CHAR);
      act("$n rolls back onto $s shoulders and kicks both feet into your back.", ch, NULL, victim, TO_VICT);
      act("$n rolls back onto $s shoulders and kicks both feet into $N's back.", ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, NULL, dam, gsn_kick);
      if (victim == NULL || victim->position == POS_DEAD)
        return;
      if (number_percent() <= 25 && !IS_BODY(victim, BROKEN_SPINE))
      {
        act("Your spine shatters under the impact of the blow!", victim, NULL, NULL, TO_CHAR);
        act("$n's spine shatters under the impact of the blow!", victim, NULL, NULL, TO_ROOM);
        SET_BIT(victim->loc_hp[LOC_BODY], BROKEN_SPINE);
      }
      act("You fall to the ground, stunned.", victim, NULL, NULL, TO_CHAR);
      act("$n falls to the ground, stunned.", victim, NULL, NULL, TO_ROOM);
      act("You flip back up to your feet.", ch, NULL, NULL, TO_CHAR);
      act("$n flips back up to $s feet.", ch, NULL, NULL, TO_ROOM);
      stop_fighting(victim, TRUE);
      victim->position = POS_STUNNED;
      break;
    case 5:
      act("You grab $N by the neck and slam your head into $S face.", ch, NULL, victim, TO_CHAR);
      act("$n grabs $N by the neck and slams $s head into $S face.", ch, NULL, victim, TO_NOTVICT);
      act("$n grabs you by the neck and slams $s head into your face.", ch, NULL, victim, TO_VICT);
      damage(ch, victim, NULL, dam, gsn_headbutt);
      if (victim == NULL || victim->position == POS_DEAD)
        return;
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
      else if (number_percent() <= 25 && !IS_BODY(victim, BROKEN_NECK))
      {
        act("Your neck shatters under the impact of the blow!", victim, NULL, NULL, TO_CHAR);
        act("$n's neck shatters under the impact of the blow!", victim, NULL, NULL, TO_ROOM);
        SET_BIT(victim->loc_hp[LOC_BODY], BROKEN_NECK);
      }
      act("You grab $N by the waist and hoist $M above your head.", ch, NULL, victim, TO_CHAR);
      act("$n grabs $N by the waist and hoists $M above $s head.", ch, NULL, victim, TO_NOTVICT);
      act("$n grabs you by the waist and hoists you above $s head.", ch, NULL, victim, TO_VICT);
      if (victim == NULL || victim->position == POS_DEAD)
        return;
      special_hurl(ch, victim);
      if (victim == NULL || victim->position == POS_DEAD)
        return;
      act("You crash to the ground, stunned.", victim, NULL, NULL, TO_CHAR);
      act("$n crashes to the ground, stunned.", victim, NULL, NULL, TO_ROOM);
      stop_fighting(victim, TRUE);
      victim->position = POS_STUNNED;
      break;
    case 6:
      act("You slam your fist into $N's stomach, who doubles over in agony.", ch, NULL, victim, TO_CHAR);
      act("$n slams $s fist into your stomach, and you double over in agony.", ch, NULL, victim, TO_VICT);
      act("$n slams $s fist into $N's stomach, who doubles over in agony.", ch, NULL, victim, TO_NOTVICT);
      act("You grab $N by the head and slam $S face into your knee.", ch, NULL, victim, TO_CHAR);
      act("$n grabs you by the head and slams your face into $s knee.", ch, NULL, victim, TO_VICT);
      act("$n grabs $N by the head and slams $S face into $s knee.", ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, NULL, dam, gsn_knee);
      if (victim == NULL || victim->position == POS_DEAD)
        return;
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
      else if (number_percent() <= 25 && !IS_BODY(victim, BROKEN_NECK))
      {
        act("Your neck shatters under the impact of the blow!", victim, NULL, NULL, TO_CHAR);
        act("$n's neck shatters under the impact of the blow!", victim, NULL, NULL, TO_ROOM);
        SET_BIT(victim->loc_hp[LOC_BODY], BROKEN_NECK);
      }
      act("You roll onto your back and smash your feet into $N's chest.", ch, NULL, victim, TO_CHAR);
      act("$n rolls onto $s back and smashes $s feet into your chest.", ch, NULL, victim, TO_VICT);
      act("$n rolls onto $s back and smashes $s feet into $N's chest.", ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, NULL, dam, gsn_kick);
      if (victim == NULL || victim->position == POS_DEAD)
        return;
      act("You crash to the ground, stunned.", victim, NULL, NULL, TO_CHAR);
      act("$n crashes to the ground, stunned.", victim, NULL, NULL, TO_ROOM);
      act("You flip back up to your feet.", ch, NULL, NULL, TO_CHAR);
      act("$n flips back up to $s feet.", ch, NULL, NULL, TO_ROOM);
      stop_fighting(victim, TRUE);
      victim->position = POS_STUNNED;
      break;
    case 7:
      act("You duck under $N's attack and pound your fist into $S stomach.", ch, NULL, victim, TO_CHAR);
      act("$n ducks under your attack and pounds $s fist into your stomach.", ch, NULL, victim, TO_VICT);
      act("$n ducks under $N's attack and pounds $s fist into $N's stomach.", ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, NULL, dam, gsn_punch);
      if (victim == NULL || victim->position == POS_DEAD)
        return;
      act("You double over in agony.", victim, NULL, NULL, TO_CHAR);
      act("$n doubles over in agony.", victim, NULL, NULL, TO_ROOM);

      act("You grab $M by the head and smash your knee into $S face.", ch, NULL, victim, TO_CHAR);
      act("$n grabs you by the head and smashes $s knee into your face.", ch, NULL, victim, TO_VICT);
      act("$n grabs $M by the head and smashes $s knee into $N's face.", ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, NULL, dam, gsn_knee);
      if (victim == NULL || victim->position == POS_DEAD)
        return;
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
      else if (number_percent() <= 25 && !IS_BODY(victim, BROKEN_NECK))
      {
        act("Your neck shatters under the impact of the blow!", victim, NULL, NULL, TO_CHAR);
        act("$n's neck shatters under the impact of the blow!", victim, NULL, NULL, TO_ROOM);
        SET_BIT(victim->loc_hp[LOC_BODY], BROKEN_NECK);
      }

      act("You stamp on the back of $N's leg, forcing $M to drop to one knee.", ch, NULL, victim, TO_CHAR);
      act("$n stamps on the back of your leg, forcing you to drop to one knee.", ch, NULL, victim, TO_VICT);
      act("$n stamps on the back of $N's leg, forcing $M to drop to one knee.", ch, NULL, victim, TO_NOTVICT);

      act("You grab $N by the hair and yank $S head back.", ch, NULL, victim, TO_CHAR);
      act("$n grabs you by the hair and yank your head back.", ch, NULL, victim, TO_VICT);
      act("$n grabs $N by the hair and yank $S head back.", ch, NULL, victim, TO_NOTVICT);

      act("You hammer your elbow down into $N's face.", ch, NULL, victim, TO_CHAR);
      act("$n hammers $s elbow down into your face.", ch, NULL, victim, TO_VICT);
      act("$n hammers $s elbow down into $N's face.", ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, NULL, dam, gsn_elbow);
      if (victim == NULL || victim->position == POS_DEAD)
        return;
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
      else if (number_percent() <= 25 && !IS_BODY(victim, BROKEN_NECK))
      {
        act("Your neck shatters under the impact of the blow!", victim, NULL, NULL, TO_CHAR);
        act("$n's neck shatters under the impact of the blow!", victim, NULL, NULL, TO_ROOM);
        SET_BIT(victim->loc_hp[LOC_BODY], BROKEN_NECK);
      }
      act("You crash to the ground, stunned.", victim, NULL, NULL, TO_CHAR);
      act("$n crashes to the ground, stunned.", victim, NULL, NULL, TO_ROOM);
      stop_fighting(victim, TRUE);
      victim->position = POS_STUNNED;
      break;
  }
  return;
}

void special_hurl(CHAR_DATA * ch, CHAR_DATA * victim)
{
  ROOM_INDEX_DATA *to_room;
  EXIT_DATA *pexit;
  EXIT_DATA *pexit_rev;
  char buf[MAX_INPUT_LENGTH];
  char direction[MAX_INPUT_LENGTH];
  int door;
  int revdir;
  int dam;

  if (victim->in_room == NULL)
    return;

  revdir = 0;

  door = number_range(0, 3);

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

  if (victim->in_room)
    pexit = victim->in_room->exit[door];
  else
    return;
  if (IS_SET(pexit->exit_info, EX_CLOSED) && !IS_AFFECTED(victim, AFF_PASS_DOOR) && !IS_AFFECTED(victim, AFF_ETHEREAL))
  {
    if (IS_SET(pexit->exit_info, EX_LOCKED))
      REMOVE_BIT(pexit->exit_info, EX_LOCKED);
    if (IS_SET(pexit->exit_info, EX_CLOSED))
      REMOVE_BIT(pexit->exit_info, EX_CLOSED);
    sprintf(buf, "$n hurls $N %s.", direction);
    act(buf, ch, NULL, victim, TO_NOTVICT);
    sprintf(buf, "You hurl $N %s.", direction);
    act(buf, ch, NULL, victim, TO_CHAR);
    sprintf(buf, "$n hurls you %s, smashing you through the %s.", direction, pexit->keyword);
    act(buf, ch, NULL, victim, TO_VICT);
    sprintf(buf, "There is a loud crash as $n smashes through the $d.");
    act(buf, victim, NULL, pexit->keyword, TO_ROOM);

    if ((to_room = pexit->to_room) != NULL && (pexit_rev = to_room->exit[revdir]) != NULL && pexit_rev->to_room == ch->in_room && pexit_rev->keyword != NULL)
    {
      if (IS_SET(pexit_rev->exit_info, EX_LOCKED))
        REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
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


DEATH_FUN *death_lookup(const char *name)
{
  int cmd;

  for (cmd = 0; *death_table[cmd].death_name; cmd++)
    if (!str_cmp(name, death_table[cmd].death_name))
      return death_table[cmd].death_fun;

  return NULL;
}

bool event_mobile_illthid(EVENT_DATA *event)
{
  ROOM_INDEX_DATA *pRoom;
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_illthid: no owner.", 0);
    return FALSE;
  }

  if (ch->position != POS_STANDING)
    return FALSE;

  if ((pRoom = get_room_index(ROOM_VNUM_ILLTHID)) == NULL)
  {
    bug("event_mobile_illthid: no room.", 0);
    return FALSE;
  }

  act("$n vanishes in a loud POP!", ch, NULL, NULL, TO_ROOM);
  act("You hear a loud POP!", ch, NULL, NULL, TO_CHAR);
  char_from_room(ch);
  char_to_room(ch, pRoom, TRUE);
  do_look(ch, "auto");

  return FALSE;
}

bool event_mobile_snotlingsleep(EVENT_DATA *event)
{
  ROOM_INDEX_DATA *pRoom;
  CHAR_DATA *ch;
  int vnum;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_snotlingsleep: no owner.", 0);
    return FALSE;
  }

  if (ch->position < POS_SLEEPING)
    return FALSE;

  for (;;)
  {
    vnum = number_range(3501, 3540);

    if ((pRoom = get_room_index(vnum)) != NULL)
    {
      act("$n drops comatose to the ground.", ch, NULL, NULL, TO_ROOM);
      act("You drop comatose to the ground.", ch, NULL, NULL, TO_CHAR);

      ch->position = POS_SLEEPING;

      act("A pair of snotling slaves picks up $n and runs of with $m.", ch, NULL, NULL, TO_ROOM);
      char_from_room(ch);
      char_to_room(ch, pRoom, TRUE);
      act("$n is dumped on the ground by a pair of snotling slaves.", ch, NULL, NULL, TO_ROOM);

      return FALSE;
    }
  }

  return FALSE;
}

bool event_mobile_kguard(EVENT_DATA *event)
{
  ROOM_INDEX_DATA *pRoom;
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_kguard: no owner.", 0);
    return FALSE;
  }

  if (ch->hit < 0)
    return FALSE;

  pRoom = get_rand_room();

  act("$n vanishes in a flash of smoke.", ch, NULL, NULL, TO_ROOM);
  send_to_char("The kingdom guards teleport spell triggers.\n\r", ch);
  char_from_room(ch);
  char_to_room(ch, pRoom, TRUE);
  act("$n appears in a flash of smoke.", ch, NULL, NULL, TO_ROOM);
  do_look(ch, "auto");

  return FALSE;
}

void deathspec_kingdom_guard(CHAR_DATA *ch, CHAR_DATA *killer)
{
  KINGDOM_DATA *kingdom;
  EVENT_DATA *event;

  if ((kingdom = vnum_kingdom(ch->in_room->vnum)) != NULL)
  {
    if (IS_SET(kingdom->flags, KINGFLAG_STRIKE))
      return;
  }

  if (event_isset_mobile(killer, EVENT_MOBILE_KGUARD))
    return;

  if (in_kingdom_hall(killer))
    return;

  event = alloc_event();
  event->fun = &event_mobile_kguard;
  event->type = EVENT_MOBILE_KGUARD;
  add_event_char(event, killer, 4 * PULSE_PER_SECOND);
}

void deathspec_illthid(CHAR_DATA *ch, CHAR_DATA *killer)
{
  EVENT_DATA *event;

  if (event_isset_mobile(killer, EVENT_MOBILE_ILLTHID))
    return;

  send_to_char("The illthid strikes you with a mindblast befire it dies.\n\r", killer); 

  event = alloc_event();
  event->type = EVENT_MOBILE_ILLTHID;
  event->fun = &event_mobile_illthid;
  add_event_char(event, killer, 3 * PULSE_PER_SECOND);
}

void deathspec_snotling(CHAR_DATA *ch, CHAR_DATA *killer)
{
  EVENT_DATA *event;

  if (event_isset_mobile(killer, EVENT_MOBILE_SNOTLINGSLEEP))
    return;

  printf_to_char(killer, "A gas explosion erupts for the corpse of %s.\n\r", ch->short_descr);

  event = alloc_event();
  event->type = EVENT_MOBILE_SNOTLINGSLEEP;
  event->fun = &event_mobile_snotlingsleep;
  add_event_char(event, killer, number_range(3, 6) * PULSE_PER_SECOND);
}

void deathspec_duegar(CHAR_DATA *ch, CHAR_DATA *killer)
{
  AFFECT_DATA af;
  int sn;

  if (ch->in_room == NULL)
  {
    bug("deathspec_duegar: not in any room.", 0);
    return;
  }

  if ((sn = skill_lookup("curse")) <= 0)
  {
    bug("deathspec_duegar: no curse spell.", 0);
    return;
  }

  af.type = sn;
  af.duration = 25;
  af.modifier = -50;
  af.location = APPLY_HITROLL;
  af.bitvector = AFF_CURSE;
  affect_to_char(killer, &af);
}

void deathspec_dwarven(CHAR_DATA *ch, CHAR_DATA *killer)
{
  AFFECT_DATA af;
  int sn;

  if (ch->in_room == NULL)
  {
    bug("deathspec_dwarven: not in any room.", 0);
    return;
  }

  if ((sn = skill_lookup("curse")) <= 0)
  {
    bug("deathspec_dwarven: no curse spell.", 0);
    return;
  }

  af.type = sn;
  af.duration = 25;
  af.modifier = -50;
  af.location = APPLY_DAMROLL;
  af.bitvector = AFF_CURSE;
  affect_to_char(killer, &af);
}

void deathspec_plasma(CHAR_DATA *ch, CHAR_DATA *killer)
{
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *gch;
  ITERATOR *pIter;
  int count = number_range(2, 4);
  int i = 0;

  if ((pMobIndex = get_mob_index(MOB_VNUM_PLASMASPAWN)) == NULL)
  {
    bug("deathspec_plasma: the demons does not exist.", 0);
    return;
  }
  if (ch->in_room == NULL)
  {
    bug("deathspec_plasma: not in any room.", 0);
    return;
  }

  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!IS_NPC(gch)) continue;

    if (gch->pIndexData->vnum == MOB_VNUM_PLASMASPAWN)
      i++;
  }

  /* we don't want to fill up the room to much */
  if (i >= 10) return;

  for (i = 0; i < count; i++)
  {
    gch = create_mobile(pMobIndex);

    char_to_room(gch, ch->in_room, TRUE);
    act("A bit of plasma spews to life!!!", ch, NULL, NULL, TO_ROOM);
  }
}

bool event_player_archangel(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  OBJ_DATA *obj;
  AFFECT_DATA af;
  char buf[MAX_STRING_LENGTH];
  int value = number_range(5, 15), sn;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_archangel: no owner.", 0);
    return FALSE;
  }

  if ((sn = skill_lookup("godbless")) < 0)
  {
    bug("event_player_archangel: no godbless spell.", 0);
    return FALSE;
  }

  act("The ghost of an archangel rises from the ground.", ch, NULL, NULL, TO_ALL);

  switch(number_range(0, 3))
  {
    default:
      act("The Archangel says '#yYou have defeated me in fair combat $n, for this I salute you.#n'.", ch, NULL, NULL, TO_ALL);
      restore_player(ch);
      break;
    case 1:
      act("The Archangel says '#y$n, I will haunt you for the rest of your days for killing me.#n'.", ch, NULL, NULL, TO_ALL);
      send_to_char("You know despair.\n\r", ch);

      af.type = sn;
      af.duration = 25;
      af.modifier = -50;
      af.location = APPLY_HITROLL;
      af.bitvector = 0;
      affect_to_char(ch, &af);
      af.location = APPLY_DAMROLL;
      affect_to_char(ch, &af);

      break;
    case 2:
      act("The Archangel says '#yUntil we meet again $n.#n'.", ch, NULL, NULL, TO_ALL);
      send_to_char("You feel blessed by the gods.\n\r", ch);

      af.type = sn;
      af.duration = 25;
      af.modifier = 50;
      af.location = APPLY_HITROLL;
      af.bitvector = 0;
      affect_to_char(ch, &af);
      af.location = APPLY_DAMROLL;
      affect_to_char(ch, &af);

      break;
    case 3:
      act("The Archangel says '#yYou have freed me from this hell $n, take this token as a reward.#n'.", ch, NULL, NULL, TO_ALL);
      obj = create_object(get_obj_index(OBJ_VNUM_PROTOPLASM), 50);
      obj->value[0] = value;
      obj->level = value;
      obj->cost = 1000;
      obj->item_type = ITEM_QUEST;
      obj_to_char(obj, ch);
      free_string(obj->name);
      obj->name = str_dup("gold token");
      free_string(obj->short_descr);
      sprintf(buf, "a %d point gold token", value);
      obj->short_descr = str_dup(buf);
      free_string(obj->description);
      sprintf(buf, "A %d point gold token lies on the floor.", value);
      obj->description = str_dup(buf);
      break;
  }

  act("The ghost of the archangel vanishes from sight.", ch, NULL, NULL, TO_ALL);

  return FALSE;
}

bool event_player_guarddog(EVENT_DATA *event)
{
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_guarddog: no owner.", 0);
    return FALSE;
  }

  if (ch->in_room && ch->in_room->vnum >= 4895 && ch->in_room->vnum <= 4930)
    act("A voice says '#yFind the emerald key and travel to the sewers (read HELP AREAS).#n'.", ch, NULL, NULL, TO_CHAR);
  else
    act("A voice says '#yYou must travel to the sewers (read HELP AREAS).#n'.", ch, NULL, NULL, TO_CHAR);

  return FALSE;
}

void deathspec_guarddog(CHAR_DATA *ch, CHAR_DATA *killer)
{
  EVENT_DATA *event;

  /* attach a visit from the grave */
  event           = alloc_event();
  event->type     = EVENT_PLAYER_GUARDDOG;
  event->fun      = &event_player_guarddog;
  add_event_char(event, killer, 3 * PULSE_PER_SECOND);
}

void deathspec_archangel(CHAR_DATA *ch, CHAR_DATA *killer)
{
  EVENT_DATA *event;

  /* attach a visit from the grave */
  event           = alloc_event();
  event->type     = EVENT_PLAYER_ARCHANGEL;
  event->fun      = &event_player_archangel;
  add_event_char(event, killer, number_range(4, 6) * PULSE_PER_SECOND);
}

void deathspec_demonspawn(CHAR_DATA *ch, CHAR_DATA *killer)
{
  MOB_INDEX_DATA *pMobIndex;
  int count = number_range(2, 4);
  int i;

  if ((pMobIndex = get_mob_index(MOB_VNUM_DEMONSPAWN)) == NULL)
  {
    bug("deathspec_demonspawn: the demons does not exist.", 0);
    return;
  }
  if (ch->in_room == NULL)
  {
    bug("deathspec_demonspawn: not in any room.", 0);
    return;
  }

  for (i = 0; i < count; i++)
  {
    CHAR_DATA *gch = create_mobile(pMobIndex);

    char_to_room(gch, ch->in_room, TRUE);
  }

  act("Green demonspawn spew forth from $n's corpse", ch, NULL, NULL, TO_ROOM);
}

void deathspec_memnon(CHAR_DATA *ch, CHAR_DATA *killer)
{
  FILE *fp;
  CHAR_DATA *gch;
  KINGDOM_DATA *kingdom;
  KINGDOM_QUEST *kquest;
  char strsave[MAX_INPUT_LENGTH];
  char name[MAX_INPUT_LENGTH];
  char leader[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  int i = 0;

  /* first find a fitting name */
  while(1)
  {
    i++;
    sprintf(strsave, "../kingdoms/king%d.kd", i);
    if ((fp = fopen(strsave, "r")) != NULL)
    {
      fclose(fp);
    }
    else
    {
      sprintf(name, "king%d", i);
      break;
    }
  }

  if ((kquest = get_kingdom_quest()) == NULL)
  {
    bug("deathspec_memnon: no kingdom quest.", 0);
    return;
  }

  one_argument(kquest->namelist, leader);
  leader[0] = UPPER(leader[0]);

  /* allocate and clear data - should put this into an alloc_kingdom call */
  kingdom = calloc(1, sizeof(*kingdom));
  clear_kingdom(kingdom);

  /* set default values */
  kingdom->kingid      =  i;
  kingdom->file        =  str_dup(name);
  kingdom->longname    =  str_dup(name);
  kingdom->whoname     =  str_dup(name);
  kingdom->leader      =  str_dup(leader);
  kingdom->prefix      =  str_dup("[");
  kingdom->suffix      =  str_dup("]");
  kingdom->shortname   =  str_dup(name);
  kingdom->king_active =  muddata.mudinfo[MUDINFO_UPDATED] * 7;

  pIter = AllocIterator(char_list);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (is_full_name(gch->name, kquest->namelist))
    {
      MEMBER_DATA *member;

      send_to_char("\n\n\r #CYOU HAVE COMPLETED A KINGDOM QUEST...#n  \n\n\r", gch);

      if (!member_of_other_kingdom(ch, kingdom) || !str_cmp(gch->name, leader))
      {
        member = alloc_member();
        member->name = str_dup(gch->name);
        member->invited_by = str_dup("Original Member");

        AttachToList(member, kingdom->members);
      }

      char_from_room(gch);
      char_to_room(gch, get_room_index(ROOM_VNUM_CITYCENTER), TRUE);
    }
  }

  AttachToList(kingdom, kingdom_list);
  save_kingdom(kingdom);

  pIter = AllocIterator(char_list);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (is_full_name(gch->name, kquest->namelist))
    {
      update_kingdom_membership(gch, FALSE);
    }
  }

  fp = fopen("../txt/kingdom.lst", "w");
  pIter = AllocIterator(kingdom_list);
  while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
  {
    fprintf(fp, "../kingdoms/%s.kd\n", kingdom->file);
  }
  fprintf(fp, "END\n");
  fclose(fp);

  clear_kingdom_quest();
}

char *death_string(DEATH_FUN *fun)
{
  int cmd;

  for (cmd = 0; *death_table[cmd].death_fun; cmd++)
    if (fun == death_table[cmd].death_fun)
      return death_table[cmd].death_name;

  return NULL;
}

void ragnarokdecap(CHAR_DATA *ch, CHAR_DATA *victim)
{
  char buf[MAX_STRING_LENGTH];

  if (getRank(victim, 0) < getRank(ch, 0) - 3)
  {
    send_to_char("You cannot decapitate someone of this rank.\n\r", ch);
    return;
  }

  char_from_room(victim);
  char_to_room(victim,get_room_index(ROOM_VNUM_CITYSAFE), TRUE);
  clearstats(victim);
  victim->level = 2;
  dropinvis(ch);
  sprintf(buf,"%s was beheaded by %s, the ragnarok continues",victim->name, ch->name);     
  do_info(ch,buf);
  send_to_char("YOU HAVE BEEN KILLED!!!!\n\r",victim);
  call_all(victim);
  do_train(victim,"avatar");
  restore_player(victim);
}

void fortresskill(CHAR_DATA *ch, CHAR_DATA *victim)
{
  char buf[MAX_STRING_LENGTH];
  ROOM_INDEX_DATA *location;
  ITERATOR *pIter;

  if (IS_NPC(ch) || IS_NPC(victim))
  {
    bug("Fortress failure", 0);
    return;
  }

  /*
   * cleanup
   */
  ch->fight_timer = 0;
  victim->fight_timer = 0;
  free_string(victim->morph);
  victim->morph = str_dup("");
  REMOVE_BIT(victim->extra, TIED_UP);
  REMOVE_BIT(victim->extra, GAGGED);
  REMOVE_BIT(victim->extra, BLINDFOLDED);

  /*
   * Tally the score
   */
  ch->pcdata->awins++;
  victim->pcdata->alosses++;  

  /* update arena kill counter */
  muddata.pk_count_now[1]++;

  /*
   * Information
   */
  sprintf(buf, "%s was beaten in The Forbidden Fortress by %s.", victim->name, ch->name);
  do_info(ch, buf);
   
  /*
   * Out they go.
   */
  if ((location = get_room_index(ROOM_VNUM_CITYSAFE)) == NULL)
    return;

  char_from_room(victim);
  char_to_room(victim, location, TRUE);
  call_all(victim);   
  clearstats(victim);
  victim->level = 2;
  victim->fight_timer = 0;
  restore_player(victim);
  strip_arena_eq(victim);

  /* stop spectating */
  stop_spectating(victim);

  /* remove from red/blue group */
  if (IS_SET(arena.status, ARENA_FORTRESS_TEAM))
  {
    victim->leader = NULL;
  }

  if (!IS_SET(arena.status, ARENA_FORTRESS_TEAM) || !ch->in_room)
  {
    char_from_room(ch);
    char_to_room(ch, location, TRUE);
    call_all(ch);
    restore_player(ch);
    open_fortress();
    ch->fight_timer = 0;
    stop_spectating(ch);
  }
  else
  {
    CHAR_DATA *gch, *teammate[2 * MAX_TEAM_SIZE];
    bool redteam = TRUE;
    int teamcount = 0;
    int i;

    for (i = 0; i < 2 * MAX_TEAM_SIZE; i++)
    {
      teammate[i] = NULL;

      if (arena.blueteam[i] == ch)
        redteam = FALSE;
    }

    pIter = AllocIterator(char_list);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      bool found = FALSE;

      if (gch == ch || IS_NPC(gch))
        continue;
      if (!gch->in_room || gch->in_room->area != ch->in_room->area)
        continue;

      /* teammates ? */
      for (i = 0; i < 2 * MAX_TEAM_SIZE; i++)
      {
        if (redteam && arena.redteam[i] == gch)
        {
          teammate[teamcount++] = gch;
          found = TRUE;
        }
        if (!redteam && arena.blueteam[i] == gch)
        {
          teammate[teamcount++] = gch;
          found = TRUE;
        }

        /* someone of the opposing team still in there ? */
        if (!redteam && arena.redteam[i] == gch)
          return;
        if (redteam && arena.blueteam[i] == gch)
          return;
      }

      if (!found)
        return;
    }

    if (redteam)
      do_info(ch, "The Red Team emerges as the winners");
    else
      do_info(ch, "The Blue Team emerges as the winners");

    char_from_room(ch);
    char_to_room(ch, location, TRUE);
    call_all(ch);
    restore_player(ch);
    ch->leader = NULL;
    ch->fight_timer = 0;
    strip_arena_eq(ch);

    stop_spectating(ch);

    for (i = 0; i < 2 * MAX_TEAM_SIZE; i++)
    {
      if (teammate[i] != NULL)
      {
        char_from_room(teammate[i]);
        char_to_room(teammate[i], location, TRUE);
        call_all(teammate[i]);
        restore_player(teammate[i]);
        teammate[i]->leader = NULL;
        teammate[i]->fight_timer = 0;
        strip_arena_eq(teammate[i]);

        stop_spectating(teammate[i]);
      }
    }

    open_fortress();
  }
}
