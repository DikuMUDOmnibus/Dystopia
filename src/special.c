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

/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN( spec_memnon		);
DECLARE_SPEC_FUN( spec_skeleton		);
DECLARE_SPEC_FUN( spec_kingdom_healer	);
DECLARE_SPEC_FUN( spec_kingdom_wizard   );
DECLARE_SPEC_FUN( spec_dwarf_grimnir	);
DECLARE_SPEC_FUN( spec_dwarf_valaya	);
DECLARE_SPEC_FUN( spec_dwarf_gamesh	);
DECLARE_SPEC_FUN( spec_dwarf_moradin	);
DECLARE_SPEC_FUN( spec_dwarf_prisina	);
DECLARE_SPEC_FUN( spec_calim		);
DECLARE_SPEC_FUN( spec_atl_neptune	);
DECLARE_SPEC_FUN( spec_atl_general	);
DECLARE_SPEC_FUN( spec_atl_martinus	);
DECLARE_SPEC_FUN( spec_atl_princess	);
DECLARE_SPEC_FUN( spec_atl_marid	);
DECLARE_SPEC_FUN( spec_atl_bruce	);
DECLARE_SPEC_FUN( spec_atl_samuel	);
DECLARE_SPEC_FUN( spec_illthid_slaver   );
DECLARE_SPEC_FUN( spec_illthid_overlord );

/***************************************************
 * This is a prototype of a special program, feel  *
 * free to copy and edit this to suit your needs   *
 ***************************************************

void spec_prototype(CHAR_DATA *ch, char *argument)
{
  if (!str_cmp(argument, "update"))
  {
    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

     * This part should contain movement, spell-casting, etc,
     * this part is called once every 4 seconds, and is not
     * related to combat...
     *
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;
    
    if ((victim = ch->fighting) == NULL)
      return;

     * This part is the combat oriented part of the code,
     * and is called once every combat round. It should
     * contain special moves/powers the mob uses in combat.
     *
  }
}

****************************************************/

void spec_atl_neptune(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch;

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    if (ch->hit > 75 * ch->max_hit / 100)
      return;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch || gch->level < 3 || IS_NPC(gch)) continue;

      if (gch->position == POS_MORTAL && SizeOfList(gch->pcdata->feeders) == 0)
      {
        ch->hit = UMIN(ch->max_hit, ch->hit + gch->max_hit);
        act("$n drains the lifeforce from $N's defenceless body.", ch, NULL, gch, TO_ROOM);

        gch->hit = UMAX(1, gch->hit);
        gch->position = POS_STANDING;
        act("$n staggers to $s feet, gasping for air.", gch, NULL, NULL, TO_ROOM);
        act("You feel a part of your lifeforce being sucked from your body.", gch, NULL, NULL, TO_CHAR);
        return;
      }
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;

    if ((victim = ch->fighting) == NULL)
      return;
  }
}

void spec_illthid_slaver(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch;

    if (ch->fighting != NULL || ch->position != POS_STANDING)
      return;

    /* semi-aggressive monsters */
    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (IS_NPC(gch) || !can_see(ch, gch) || gch->level >= LEVEL_IMMORTAL)
        continue;

      one_hit(ch, gch, TYPE_UNDEFINED, 1);
      return;
    }

    /* heal move/mana if needed */
    if (ch->move < ch->max_move || ch->mana < ch->max_mana)
    {
      int gain = number_range(200, 500);

      if (ch->move < ch->max_move)
        modify_move(ch, gain);
      if (ch->mana < ch->max_mana)
        modify_mana(ch, gain);
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;

    if ((victim = ch->fighting) == NULL)
      return;

    switch(number_range(1, 3))
    {
      default:
        if (ch->mana >= 200)
        {
          act("$n strikes $N with a mindblast.", ch, NULL, victim, TO_NOTVICT);
          act("$n strikes you with a mindblast.", ch, NULL, victim, TO_VICT);
          modify_mana(ch, -200);
          modify_mana(victim, -1 * number_range(500, 2000));

          if (victim->mana < 0)
            victim->mana = 0;

          if (victim->mana == 0 || number_range(1, 4) == 2)
          {
            AFFECT_DATA af;
            int sn;

            if ((sn = skill_lookup("mind blank")) > 0)
            {
              af.type = sn;
              af.duration = 20;
              af.location = APPLY_AC;
              af.modifier = 10;
              af.bitvector = AFF_MINDBLANK;
              affect_to_char(victim, &af);
            }
            else
              bug("spec_illthid: no spell.", 0);
          }
        }
        break;
      case 1:
      case 2:
        if (ch->move >= 200)
        {
          act("$n strikes $N with a tentacle.", ch, NULL, victim, TO_NOTVICT);
          act("$n strikes you with a tentacle.", ch, NULL, victim, TO_VICT);
          modify_move(ch, -200);
          modify_move(victim, -1 * number_range(500, 2000));

          if (victim->move < 0)
            victim->move = 0;

          if (victim->move == 0 || number_range(1, 5) == 2)
          {
            AFFECT_DATA af;
            int sn;

            if ((sn = skill_lookup("mind wreck")) > 0)
            {
              af.type = sn;
              af.duration = 20;
              af.location = APPLY_AC;
              af.modifier = 20;
              af.bitvector = AFF_CURSE;
              affect_to_char(victim, &af);
            }
            else
              bug("spec_illthid: no spell.", 0);
          }
        }
        break;
    }
  }
}

void spec_illthid_overlord(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch;

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    /* auto assist if needed */
    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (IS_NPC(gch) || !can_see(ch, gch) || gch->level >= LEVEL_IMMORTAL)
        continue;

      if (gch->fighting != NULL && IS_NPC(gch->fighting))
      {
        one_hit(ch, gch, TYPE_UNDEFINED, 1);
        return;
      }
    }

    /* heal move/mana if needed */
    if (ch->move < ch->max_move || ch->mana < ch->max_mana)
    {
      int gain = number_range(200, 500);

      if (ch->move < ch->max_move)
        modify_move(ch, gain);
      if (ch->mana < ch->max_mana)
        modify_mana(ch, gain);
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;

    if ((victim = ch->fighting) == NULL)
      return;

    switch(number_range(1, 3))
    {
      default:
        if (ch->mana >= 200)
        {
          act("$n strikes $N with a mindblast.", ch, NULL, victim, TO_NOTVICT);
          act("$n strikes you with a mindblast.", ch, NULL, victim, TO_VICT);
          modify_mana(ch, -200);
          modify_mana(victim, -1 * number_range(1000, 3000));

          if (victim->mana < 0)
            victim->mana = 0;

          if (victim->mana == 0 || number_range(1, 4) == 2)
          {
            AFFECT_DATA af;
            int sn;

            if ((sn = skill_lookup("mind blank")) > 0)
            {
              af.type = sn;
              af.duration = 20;
              af.location = APPLY_AC;
              af.modifier = 10;
              af.bitvector = AFF_MINDBLANK;
              affect_to_char(victim, &af);
            }
            else
              bug("spec_illthid: no spell.", 0);
          }
        }
        break;
      case 1:
      case 2:
        if (ch->move >= 200)
        {
          act("$n strikes $N with a tentacle.", ch, NULL, victim, TO_NOTVICT);
          act("$n strikes you with a tentacle.", ch, NULL, victim, TO_VICT);
          modify_move(ch, -200);
          modify_move(victim, -1 * number_range(1000, 3000));

          if (victim->move < 0)
            victim->move = 0;

          if (victim->move == 0 || number_range(1, 5) == 2)
          {
            AFFECT_DATA af;
            int sn;

            if ((sn = skill_lookup("mind wreck")) > 0)
            {
              af.type = sn;
              af.duration = 20;
              af.location = APPLY_AC;
              af.modifier = 20;
              af.bitvector = AFF_CURSE;
              affect_to_char(victim, &af);
            }
            else
              bug("spec_illthid: no spell.", 0);
          }
        }
        break;
    }
  }
}

void spec_atl_general(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch;
    char buf[MAX_STRING_LENGTH];

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    if (ch->hit > 75 * ch->max_hit / 100)
      return;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch || gch->level < 3 || IS_NPC(gch)) continue;

      if (gch->position == POS_MORTAL)
      {
        sprintf(buf, "%s was executed by %s.", gch->name, "The General Guard");
        do_info(ch, buf);
        behead(gch);
        gch->mdeath++;
        gch->level = 2;
        return;
      }
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;

    if ((victim = ch->fighting) == NULL)
      return;
  }
}

void spec_atl_martinus(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch;
    char buf[MAX_STRING_LENGTH];

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    if (ch->hit > 75 * ch->max_hit / 100)
      return;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch || gch->level < 3 || IS_NPC(gch)) continue;

      if (gch->position == POS_MORTAL)
      {
        sprintf(buf, "%s was sacrificed by %s.", gch->name, "The Priest Martinus");
        do_info(ch, buf);
        behead(gch);
        gch->mdeath++;
        gch->level = 2;

        return;
      }
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;

    if ((victim = ch->fighting) == NULL)
      return;
  }
}

void spec_atl_chelly(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch;

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch || gch->level < 3 || IS_NPC(gch)) continue;

      if (gch->position == POS_MORTAL && SizeOfList(gch->pcdata->feeders) == 0)
      {
        ch->hit = UMIN(ch->max_hit, ch->hit + gch->max_hit);
        act("$n drains the lifeforce from $N's defenceless body.", ch, NULL, gch, TO_ROOM);

        gch->hit = UMAX(1, gch->hit);
        gch->position = POS_STANDING;
        act("$n staggers to $s feet, gasping for air.", gch, NULL, NULL, TO_ROOM);
        act("You feel a part of your lifeforce being sucked from your body.", gch, NULL, NULL, TO_CHAR);
        return;
      }
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;

    if ((victim = ch->fighting) == NULL)
      return;
  }
}

void spec_atl_marid(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch;

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    if (ch->hit > 75 * ch->max_hit / 100)
      return;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch || gch->level < 3 || IS_NPC(gch) || gch->hit <= 1000) continue;

      modify_hps(ch, 1000);
      hurt_person(ch, gch, 1000);
      act("$n waves a hand, and lifeforce spurts from $N into $m.", ch, NULL, gch, TO_NOTVICT);
      act("$n waves a hand, and lifeforce spurts from you into $m.", ch, NULL, gch, TO_VICT);
      return;
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;

    if ((victim = ch->fighting) == NULL)
      return;
  }
}

void spec_atl_bruce(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch;

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    if (ch->hit > 75 * ch->max_hit / 100)
      return;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch || gch->level < 3 || IS_NPC(gch)) continue;

      if (gch->position == POS_MORTAL)
      {
        act("$n puts $N on a huge fishing hook, and throws $M into the water.", ch, NULL, gch, TO_ROOM);
        char_from_room(gch);
        char_to_room(gch, get_rand_room(), TRUE);
        act("$n dumps down from the sky, landing facedown.", gch, NULL, NULL, TO_ROOM);
        return;
      }
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;

    if ((victim = ch->fighting) == NULL)
      return;
  }
}

void spec_atl_samuel(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch;
    char buf[MAX_STRING_LENGTH];

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    if (ch->hit > 75 * ch->max_hit / 100)
      return;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch || gch->level < 3 || IS_NPC(gch)) continue;

      if (gch->position == POS_MORTAL)
      {
        sprintf(buf, "%s was beheaded by %s.", gch->name, "Samuel the Woodcutter");
        do_info(ch, buf);
        behead(gch);
        gch->mdeath++;
        gch->level = 2;
        return;
      }
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;

    if ((victim = ch->fighting) == NULL)
      return;
  }
}

void spec_calim(CHAR_DATA *ch, char *argument)
{
  ROOM_INDEX_DATA *pRoom;
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch;
    char buf[MAX_STRING_LENGTH];

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    /* Calim only does stuff if he is hurt */
    if (ch->hit < 75 * ch->max_hit / 100)
    {
      pIter = AllocIterator(ch->in_room->people);
      while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
      {
        if (gch == ch || gch->level < 3 || IS_NPC(gch)) continue;

        if (gch->position == POS_MORTAL)
        {
          sprintf(buf, "%s was executed by %s.", gch->name, "The Great Calim");
          do_info(ch, buf);
          behead(gch);
          gch->mdeath++;
          gch->level = 2;

          return;
        }
      }
    }

    /* move calim somewhere else */
    switch(ch->in_room->vnum)
    {
      default:
        if ((pRoom = get_room_index(7803)) != NULL)
        {
          char_from_room(ch);
          char_to_room(ch, pRoom, TRUE);
        }
        break;
      case 7803:
        do_east(ch, "");
        break;
      case 7808:
        do_north(ch, "");
        break;
      case 7804:
        do_north(ch, "");
        break;
      case 7806:
        do_west(ch, "");
        break;
      case 7802:
        do_west(ch, "");
        break;
      case 7807:
        do_south(ch, "");
        break;
      case 7805:
        do_south(ch, "");
        break;
      case 7809:
        do_east(ch, "");
        break;
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;

    if ((victim = ch->fighting) == NULL)
      return;
  }
}

void spec_dwarf_grimnir(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch, *assist = NULL;
    char buf[MAX_STRING_LENGTH];

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch || gch->level < 3 || IS_NPC(gch)) continue;

      if (gch->position == POS_MORTAL)
      {
        sprintf(buf, "%s was executed by %s.", gch->name, ch->short_descr);
        do_info(ch, buf);
        behead(gch);
        gch->mdeath++;
        gch->level = 2;

        return;
      }

      /* fighting an NPC ? */
      if (gch->fighting != NULL && gch->fighting != ch && IS_NPC(gch->fighting))
        assist = gch;
    }

    if (assist)
    {
      multi_hit(ch, assist, 1);
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;
    int count = 0;

    if ((victim = ch->fighting) == NULL)
      return;

    if (number_percent() >= 50)
      one_hit(ch, victim, TYPE_UNDEFINED, 1);

    pIter = AllocIterator(ch->in_room->people);
    while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (!IS_NPC(victim)) continue;

      if (victim->pIndexData->vnum == MOB_VNUM_DWARF_WARRIOR)
      {
        count++;

        if (victim->fighting == NULL && ch->fighting)
          multi_hit(victim, ch->fighting, 1);
      }
    }

    if (count < 3)
    {
      MOB_INDEX_DATA *pMobIndex;

      do_say(ch, "To me warriors, to me at once!");

      if ((pMobIndex = get_mob_index(MOB_VNUM_DWARF_WARRIOR)) == NULL)
      {
        bug("spec_dwarf_grimnir: No Dwarf Warriors!", 0);
        return;
      }
 
      victim = create_mobile(pMobIndex);
      char_to_room(victim, ch->in_room, TRUE);

      act("$n appears in the room.", victim, NULL, NULL, TO_ROOM);

      if (ch->fighting)
      {
        multi_hit(victim, ch->fighting, 1);
      }
    }
  }
}

void spec_dwarf_valaya(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch, *assist = NULL;
    char buf[MAX_STRING_LENGTH];

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch || gch->level < 3 || IS_NPC(gch)) continue;

      if (gch->position == POS_MORTAL)
      {
        sprintf(buf, "%s was executed by %s.", gch->name, ch->short_descr);
        do_info(ch, buf);
        behead(gch);
        gch->mdeath++;
        gch->level = 2;

        return;
      }

      /* fighting an NPC ? */
      if (gch->fighting != NULL && gch->fighting != ch && IS_NPC(gch->fighting))
        assist = gch;
    }

    if (assist)
    {
      multi_hit(ch, assist, 1);
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;
    int sn;

    if ((victim = ch->fighting) == NULL)
      return;

    switch(number_range(1, 3))
    {
      default:
        if ((sn = skill_lookup("fireball")) <= 0)
        {
          bug("spec_dwarf_valaya: fireball spell.", 0);
          return;
        }
        break;
      case 2:
        if ((sn = skill_lookup("magic missile")) <= 0)
        {
          bug("spec_dwarf_valaya: magic missile spell.", 0);
          return;
        }
        break;
      case 3:
        if ((sn = skill_lookup("poison")) <= 0)
        {
          bug("spec_dwarf_valaya: poison spell.", 0);
          return;
        }
        break;
    }

    /* cast the spell */
    (*skill_table[sn].spell_fun) (sn, 50, ch, victim);

    if (ch->fighting == NULL)
      return;

    if (number_percent() >= 50)
      one_hit(ch, victim, TYPE_UNDEFINED, 1);
  }
}

void spec_dwarf_gamesh(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch, *assist = NULL;
    char buf[MAX_STRING_LENGTH];

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch || gch->level < 3 || IS_NPC(gch)) continue;

      if (gch->position == POS_MORTAL)
      {
        sprintf(buf, "%s was executed by %s.", gch->name, ch->short_descr);
        do_info(ch, buf);
        behead(gch);
        gch->mdeath++;
        gch->level = 2;

        return;
      }

      /* fighting an NPC ? */
      if (gch->fighting != NULL && gch->fighting != ch && IS_NPC(gch->fighting))
        assist = gch;
    }

    if (assist)
    {
      multi_hit(ch, assist, 1);
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;
    int count = 0;

    if ((victim = ch->fighting) == NULL)
      return;

    if (number_percent() >= 50)
      one_hit(ch, victim, TYPE_UNDEFINED, 1);

    pIter = AllocIterator(ch->in_room->people);
    while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (!IS_NPC(victim)) continue;

      if (victim->pIndexData->vnum == MOB_VNUM_DUEGAR)
      {
        if (victim->fighting == NULL && ch->fighting)
          multi_hit(victim, ch->fighting, 1);

        count++;
      }
    }

    if (count < 3)
    {
      MOB_INDEX_DATA *pMobIndex;

      do_say(ch, "Help, Help Me!");

      if ((pMobIndex = get_mob_index(MOB_VNUM_DUEGAR)) == NULL)
      {
        bug("spec_dwarf_gamesh: No Duegar!", 0);
        return;
      }

      victim = create_mobile(pMobIndex);
      char_to_room(victim, ch->in_room, TRUE);

      act("$n appears in the room.", victim, NULL, NULL, TO_ROOM);

      if (ch->fighting)
      {
        multi_hit(victim, ch->fighting, 1);
      }
    }
  }
}

void spec_dwarf_moradin(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch, *assist = NULL;
    char buf[MAX_STRING_LENGTH];

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch || gch->level < 3 || IS_NPC(gch)) continue;

      if (gch->position == POS_MORTAL)
      {
        sprintf(buf, "%s was executed by %s.", gch->name, ch->short_descr);
        do_info(ch, buf);
        behead(gch);
        gch->mdeath++;
        gch->level = 2;

        return;
      }

      /* fighting an NPC ? */
      if (gch->fighting != NULL && gch->fighting != ch && IS_NPC(gch->fighting))
        assist = gch;
    }

    if (assist)
    {
      multi_hit(ch, assist, 1);
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;

    if ((victim = ch->fighting) == NULL)
      return;

    if (number_percent() >= 50)
    {
      one_hit(ch, victim, TYPE_UNDEFINED, 1);
      one_hit(ch, victim, TYPE_UNDEFINED, 1);
    }
  }
}

void spec_dwarf_prisina(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch, *assist = NULL;
    char buf[MAX_STRING_LENGTH];

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch || gch->level < 3 || IS_NPC(gch)) continue;

      if (gch->position == POS_MORTAL)
      {
        sprintf(buf, "%s was executed by %s.", gch->name, ch->short_descr);
        do_info(ch, buf);
        behead(gch);
        gch->mdeath++;
        gch->level = 2;

        return;
      }

      /* fighting an NPC ? */
      if (gch->fighting != NULL && gch->fighting != ch && IS_NPC(gch->fighting))
        assist = gch;
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;

    if ((victim = ch->fighting) == NULL)
      return;

    switch(number_range(1, 5))
    {
      default:
        break;
      case 3:
        if (!IS_SET(victim->affected_by, AFF_WEBBED))
        {
          act("$n throws a shimmering web at you.", ch, NULL, victim, TO_VICT);
          act("$n throws a shimmering web at $N.", ch, NULL, victim, TO_NOTVICT);

          if (number_percent() >= 25)
          {
            SET_BIT(victim->affected_by, AFF_WEBBED);
            act("$n is caught in the shimmering web.", victim, NULL, NULL, TO_ROOM);
            send_to_char("You are caught in the shimmering web.\n\r", ch);
          }
        }
        break;
      case 4:
      case 5:
        if (IS_SET(victim->affected_by, AFF_WEBBED))
        {
          one_hit(ch, victim, TYPE_UNDEFINED, 1);
          one_hit(ch, victim, TYPE_UNDEFINED, 1);
          one_hit(ch, victim, TYPE_UNDEFINED, 1);
          return;
        }
        break;
    }

    if (number_percent() >= 50)
      one_hit(ch, victim, TYPE_UNDEFINED, 1);
  }
}

void spec_kingdom_wizard(CHAR_DATA *ch, char *argument)
{
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch;
    int count = 0, pick, sn = 0, level;

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    if ((kingdom = vnum_kingdom(ch->in_room->vnum)) != NULL)
    {
      if (IS_SET(kingdom->flags, KINGFLAG_STRIKE))
        return;
    }

    /* doesn't always cast spells */
    if (number_range(1, 3) != 2) return;

    if (ch->level > 10)
      level = (int) 2 * ch->level / log(ch->level);
    else
      level = 10;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (!in_kingdom_hall(gch)) continue;
      if (gch->fighting != NULL || gch->position == POS_FIGHTING) continue;

      count++;
    }
    if (count == 0) return;

    pick = number_range(1, count);
    count = 0;
    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (!in_kingdom_hall(gch)) continue;
      if (gch->fighting != NULL || gch->position == POS_FIGHTING) continue;

      if (++count == pick) break;
    }

    if (gch == NULL || gch == ch)
    {
      bug("spec_kingdom_wizard: bad code.", 0);
      return;
    }

    switch(number_range(1, 5))
    {
      case 1:
        if ((sn = skill_lookup("bless")) <= 0)
        {
          bug("spec_kingdom_wizard: spell 'bless' not a spell.", 0);
          return;
        }
        if (!is_affected(gch, sn))
        {
          do_say(ch, "abra unso sabru");
          (*skill_table[sn].spell_fun) (sn, level, ch, gch);
        }
        break;
      case 2:
        if ((sn = skill_lookup("darkblessing")) <= 0)
        {
          bug("spec_kingdom_wizard: spell 'darkblessing' not a spell.", 0);
          return;
        }
        if (!is_affected(gch, sn))
        {
          do_say(ch, "tect tri perning");
          (*skill_table[sn].spell_fun) (sn, level, ch, gch);
        }
        break;
      case 3:
        if ((sn = skill_lookup("frenzy")) <= 0)
        {
          bug("spec_kingdom_wizard: spell 'frenzy' not a spell.", 0);
          return;
        }
        if (!is_affected(gch, sn))
        {
          do_say(ch, "duda illaven");
          (*skill_table[sn].spell_fun) (sn, level, ch, gch);
        }
        break;
      case 4:
        if ((sn = skill_lookup("shield")) <= 0)  
        {
          bug("spec_kingdom_wizard: spell 'shield' not a spell.", 0);  
          return;
        }
        if (!is_affected(gch, sn))
        {
          do_say(ch, "nofoper ra ra re");
          (*skill_table[sn].spell_fun) (sn, level, ch, gch);
        }
        break;
      case 5:
        if ((sn = skill_lookup("stone skin")) <= 0)
        {
          bug("spec_kingdom_wizard: spell 'stone skin' not a spell.", 0);
          return;
        }
        if (!is_affected(gch, sn))
        {
          do_say(ch, "nuglar ratum ra re");
          (*skill_table[sn].spell_fun) (sn, level, ch, gch);
        }
        break;
    }
        
    return;
  }
  else if (!str_cmp(argument, "midround"))
  {
    return;   
  }
}

void spec_kingdom_healer(CHAR_DATA *ch, char *argument)
{
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    CHAR_DATA *gch;
    int count = 0, pick, amount, level = UMAX(0, (ch->level - 1000) / 1000);

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    if ((kingdom = vnum_kingdom(ch->in_room->vnum)) != NULL)
    {
      if (IS_SET(kingdom->flags, KINGFLAG_STRIKE))
        return;
    }

    /* doesn't always cast spells */
    if (number_range(1, 3) != 2) return;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (!in_kingdom_hall(gch)) continue;
      if (gch->fighting != NULL || gch->position == POS_FIGHTING) continue;

      count++;
    }
    if (count == 0) return;

    pick = number_range(1, count);
    count = 0;
    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (!in_kingdom_hall(gch)) continue;
      if (gch->fighting != NULL || gch->position == POS_FIGHTING) continue;

      if (++count == pick) break;
    }

    if (gch == NULL || gch == ch)
    {
      bug("spec_kingdom_healer: bad code.", 0);
      return;
    }

    amount = number_range(1000 + 300 * level, 1500 + 600 * level);

    switch(number_range(1, 4))
    {
      case 1:
      case 2:
        if (gch->hit < gch->max_hit)
        {
          do_say(ch, "en oculo lacri bur");
          send_to_char("A warm feeling fills your body.\n\r", gch);
          act("$n heals $N.", ch, NULL, gch, TO_NOTVICT);

          modify_hps(gch, amount);
          update_pos(gch);
        }
        break;
      case 3:
        if (gch->mana < gch->max_mana)
        {
          do_say(ch, "abra unso mylar");
          act("$n draws in energy and channels it into $N.", ch, NULL, gch, TO_NOTVICT);
          act("$n draws in energy and channels it into you.", ch, NULL, gch, TO_VICT);

          modify_mana(gch, amount);
        }
        break;
      case 4:
        if (gch->move < gch->max_move)
        {
          do_say(ch, "tect oculo gar");
          act("$n looks less tired.", gch, NULL, NULL, TO_ROOM);
          send_to_char("You feel less tired.\n\r", gch);

          modify_move(gch, amount);
        }
        break;
    }
    return;
  }
  else if (!str_cmp(argument, "midround"))
  {
    return;
  }
}

void spec_skeleton(CHAR_DATA *ch, char *argument)
{
  if (!str_cmp(argument, "update"))
  {
    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    /* without a leader, skeletons die */
    if (!ch->leader)
    {
      act("$n crumbles to dust.", ch, NULL, NULL, TO_ROOM);
      extract_char(ch, TRUE);
      return;
    }
  }
  else if (!str_cmp(argument, "midround"))
  {
    CHAR_DATA *victim;

    if ((victim = ch->fighting) == NULL)
      return;


  }
}

void spec_memnon(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *gch;
  char buf[MAX_STRING_LENGTH];
  KINGDOM_QUEST *kquest;
  ITERATOR *pIter;

  if (!str_cmp(argument, "update"))
  {
    int count = 0;

    if (ch->fighting || ch->position <= POS_STUNNED)
      return;

    /* no quest, puff goes Memnon... */
    if ((kquest = get_kingdom_quest()) == NULL)
    {
      extract_char(ch, TRUE);
      return;
    }

    if (!IS_SET(ch->affected_by, AFF_SANCTUARY))
    {
      do_say(ch, "Kalo Karatu Zonu!");
      SET_BIT(ch->affected_by, AFF_SANCTUARY);
      act("$n is surrounded in a white aura.", ch, NULL, NULL, TO_ROOM);    
      return;
    }

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch) continue;

      if (gch->position == POS_MORTAL)
      {
        sprintf(buf, "%s was executed by The Avatar of Memnon.", gch->name);
        do_info(ch, buf);
        behead(gch);
        gch->mdeath++;
        gch->level = 2;

        return;
      }

      if (IS_SET(gch->newbits, NEW_SHADOWPLANE))
      {
        REMOVE_BIT(gch->newbits, NEW_SHADOWPLANE);
        act("$n breaths a cone of purple gas at $N.", ch, NULL, gch, TO_NOTVICT);
        act("$n breaths a cone of purple gas at you.", ch, NULL, gch, TO_VICT);
        send_to_char("You fade into the real world.\n\r", gch);
        return;
      }

      if (IS_SET(gch->newbits, NEW_MUDFORM))
      {
        REMOVE_BIT(gch->newbits, NEW_MUDFORM);
        act("$n breaths a cone of purple gas at $N.", ch, NULL, gch, TO_NOTVICT);
        act("$n breaths a cone of purple gas at you.", ch, NULL, gch, TO_VICT);
        send_to_char("You form becomes solid again.\n\r", gch);
        return;
      }

      if (IS_SET(gch->act, PLR_HIDE))
      {
        REMOVE_BIT(gch->act, PLR_HIDE);
        act("$n breaths a cone of purple gas at $N.", ch, NULL, gch, TO_NOTVICT);
        act("$n breaths a cone of purple gas at you.", ch, NULL, gch, TO_VICT);
        send_to_char("You become very visible.\n\r", gch);
        return;
      }

      if (IS_SET(gch->affected_by, AFF_ETHEREAL))
      {
        REMOVE_BIT(gch->affected_by, AFF_ETHEREAL);
        act("$n breaths a cone of purple gas at $N.", ch, NULL, gch, TO_NOTVICT);
        act("$n breaths a cone of purple gas at you.", ch, NULL, gch, TO_VICT);
        send_to_char("You become solid again.\n\r", gch);
        return;
      }
    }

    act("$n goes BERSERK!", ch, NULL, NULL, TO_ROOM);

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (ch == gch)
        continue;   

      count++;

      if (can_see(ch, gch))
      {
        damage(ch, gch, NULL, number_range(gch->max_hit / 15, gch->max_hit / 8), TYPE_HIT);
        damage(ch, gch, NULL, number_range(gch->max_hit / 15, gch->max_hit / 8), TYPE_HIT);
      }
    }

    /* noone in the room, then they have failed the quest */
    if (count == 0)
    {
      /* tell them that they have failed their kingdom quest */
      pIter = AllocIterator(char_list);
      while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
      {
        if (is_full_name(gch->name, kquest->namelist))
        {
          send_to_char("\n\n\r #RYOU HAVE FAILED A KINGDOM QUEST...#n  \n\n\r", gch);

          char_from_room(gch);
          char_to_room(gch, get_room_index(ROOM_VNUM_CITYCENTER), TRUE);
        }
      }

      clear_kingdom_quest();

      /* extract this mobile */
      extract_char(ch, TRUE);
    }

    return;
  }
  else if (!str_cmp(argument, "midround"))
  {
    int maxMight = 0;
    int dam;

    if (!ch->fighting)
      return;

    /* calculate maxMight */
    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch) continue;
      if (getMight(gch) > maxMight)
        maxMight = getMight(gch);
    }

    if (maxMight < RANK_HERO)
      dam = number_range(1000, 2000);
    else if (maxMight < RANK_CAPTAIN)
      dam = number_range(1500, 3000);
    else if (maxMight < RANK_DUKE)
      dam = number_range(2000, 4000);
    else
      dam = number_range(2500, 5000);

    if (!IS_SET(ch->affected_by, AFF_SANCTUARY) && number_range(1, 3) == 2)
    {
      do_say(ch, "Kalo Karatu Zonu!");
      SET_BIT(ch->affected_by, AFF_SANCTUARY);
      act("$n is surrounded in a white aura.", ch, NULL, NULL, TO_ROOM);
      return;
    }

    switch(number_range(0, 5))
    {
      default:
        damage(ch, ch->fighting, NULL, dam, TYPE_HIT);
        break;
      case 1:
        act("$n taps on $N's lifeforce.", ch, NULL, ch->fighting, TO_NOTVICT);
        act("$n taps on your lifeforce.", ch, NULL, ch->fighting, TO_VICT);
        dam = ch->fighting->move / 2 + ch->fighting->mana / 2;
        modify_hps(ch, dam);
        modify_move(ch->fighting, ch->fighting->move / 2);
        modify_mana(ch->fighting, ch->fighting->mana / 2);
        break;
      case 2:
        gch = ch->fighting;

        act("$n knocks $N unconscious!", ch, NULL, gch, TO_NOTVICT);
        act("$n knocks you unconscious!", ch, NULL, gch, TO_VICT);
        stop_fighting(gch, TRUE);
        gch->position = POS_STUNNED;

        do_say(ch, "Pitiful mortals, I will feast on your measly souls tonight!");
        break;
      case 3:
        act("$n breaths a cone of frost on $N.", ch, NULL, ch->fighting, TO_NOTVICT);
        act("$n breaths a cone of frost on you. You cannot move!!!", ch, NULL, ch->fighting, TO_VICT);
        WAIT_STATE(ch->fighting, 36);
        break;
    }
  }
}

/*
 * Special Functions Table.     OLC
 */
const   struct  spec_type       spec_table      [ ] =
{
  /*
   * Special function commands.
   */
  { "spec_memnon",                spec_memnon            },
  { "spec_skeleton",              spec_skeleton          },
  { "spec_kingdom_healer",        spec_kingdom_healer    },
  { "spec_kingdom_wizard",        spec_kingdom_wizard    },
  { "spec_dwarf_grimnir",         spec_dwarf_grimnir     },
  { "spec_dwarf_valaya",          spec_dwarf_valaya      },
  { "spec_dwarf_gamesh",          spec_dwarf_gamesh      },
  { "spec_dwarf_moradin",         spec_dwarf_moradin     },
  { "spec_dwarf_prisina",         spec_dwarf_prisina     },
  { "spec_calim",                 spec_calim             },
  { "spec_atl_neptune",           spec_atl_neptune       },
  { "spec_atl_general",           spec_atl_general       },
  { "spec_atl_martinus",          spec_atl_martinus      },
  { "spec_atl_chelly",            spec_atl_chelly        },
  { "spec_atl_marid",             spec_atl_marid         },
  { "spec_atl_bruce",             spec_atl_bruce         },
  { "spec_atl_samuel",            spec_atl_samuel        },
  { "spec_illthid_slaver",        spec_illthid_slaver    },
  { "spec_illthid_overlord",      spec_illthid_overlord  },

  /*
   * End of list.
   */
  { "", 0 }
};


/*****************************************************************************
 Name:          spec_lookup
 Purpose:       Given a name, return the appropriate spec fun.
 Called by:     do_mset(act_wiz.c) load_specials,reset_area(db.c)
 ****************************************************************************/
SPEC_FUN *spec_lookup( const char *name )       /* OLC */
{   
    int cmd;

    for ( cmd = 0; *spec_table[cmd].spec_name; cmd++ )  /* OLC 1.1b */
        if ( !str_cmp( name, spec_table[cmd].spec_name ) )
            return spec_table[cmd].spec_fun;
 
    return 0;
}

/*****************************************************************************
 Name:          spec_string
 Purpose:       Given a function, return the appropriate name.
 Called by:     <???>
 ****************************************************************************/
char *spec_string( SPEC_FUN *fun )      /* OLC */
{
    int cmd;

    for ( cmd = 0; *spec_table[cmd].spec_fun; cmd++ )   /* OLC 1.1b */
        if ( fun == spec_table[cmd].spec_fun )
            return spec_table[cmd].spec_name;
    
    return 0;
}
 
