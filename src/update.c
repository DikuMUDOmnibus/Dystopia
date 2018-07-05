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
#include <unistd.h>

#include "dystopia.h"

void reg_mend(CHAR_DATA * ch)
{
  int ribs = 0;
  int teeth = 0;
 
  if (IS_BODY(ch, BROKEN_RIBS_1))
    ribs += 1;
  if (IS_BODY(ch, BROKEN_RIBS_2))
    ribs += 2;
  if (IS_BODY(ch, BROKEN_RIBS_4))
    ribs += 4;
  if (IS_BODY(ch, BROKEN_RIBS_8))
    ribs += 8;
  if (IS_BODY(ch, BROKEN_RIBS_16))
    ribs += 16;
  if (IS_HEAD(ch, LOST_TOOTH_1))
    teeth += 1;
  if (IS_HEAD(ch, LOST_TOOTH_2))
    teeth += 2;
  if (IS_HEAD(ch, LOST_TOOTH_4))
    teeth += 4;
  if (IS_HEAD(ch, LOST_TOOTH_8))
    teeth += 8;
  if (IS_HEAD(ch, LOST_TOOTH_16))
    teeth += 16;

  if (ribs > 0)
  {
    if (IS_BODY(ch, BROKEN_RIBS_1))
      REMOVE_BIT(ch->loc_hp[1], BROKEN_RIBS_1);
    if (IS_BODY(ch, BROKEN_RIBS_2))
      REMOVE_BIT(ch->loc_hp[1], BROKEN_RIBS_2);
    if (IS_BODY(ch, BROKEN_RIBS_4))
      REMOVE_BIT(ch->loc_hp[1], BROKEN_RIBS_4);
    if (IS_BODY(ch, BROKEN_RIBS_8))
      REMOVE_BIT(ch->loc_hp[1], BROKEN_RIBS_8);
    if (IS_BODY(ch, BROKEN_RIBS_16))
      REMOVE_BIT(ch->loc_hp[1], BROKEN_RIBS_16);
    ribs -= 1;
    if (ribs >= 16)
    {
      ribs -= 16;
      SET_BIT(ch->loc_hp[1], BROKEN_RIBS_16);
    }
    if (ribs >= 8)
    {
      ribs -= 8;
      SET_BIT(ch->loc_hp[1], BROKEN_RIBS_8);
    }
    if (ribs >= 4)
    {
      ribs -= 4;
      SET_BIT(ch->loc_hp[1], BROKEN_RIBS_4);
    }
    if (ribs >= 2)
    {
      ribs -= 2;
      SET_BIT(ch->loc_hp[1], BROKEN_RIBS_2);
    }
    if (ribs >= 1)
    {
      ribs -= 1;
      SET_BIT(ch->loc_hp[1], BROKEN_RIBS_1);
    }
    act("One of $n's ribs snap back into place.", ch, NULL, NULL, TO_ROOM);
    act("One of your ribs snap back into place.", ch, NULL, NULL, TO_CHAR);
  }
  else if (IS_HEAD(ch, LOST_EYE_L))
  {
    act("An eyeball appears in $n's left eye socket.", ch, NULL, NULL, TO_ROOM);
    act("An eyeball appears in your left eye socket.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_HEAD], LOST_EYE_L);
  }
  else if (IS_HEAD(ch, LOST_EYE_R))
  {
    act("An eyeball appears in $n's right eye socket.", ch, NULL, NULL, TO_ROOM);
    act("An eyeball appears in your right eye socket.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_HEAD], LOST_EYE_R);
  }
  else if (IS_HEAD(ch, LOST_EAR_L))
  {
    act("An ear grows on the left side of $n's head.", ch, NULL, NULL, TO_ROOM);
    act("An ear grows on the left side of your head.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_HEAD], LOST_EAR_L);
  }
  else if (IS_HEAD(ch, LOST_EAR_R))
  {
    act("An ear grows on the right side of $n's head.", ch, NULL, NULL, TO_ROOM);
    act("An ear grows on the right side of your head.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_HEAD], LOST_EAR_R);
  }
  else if (IS_HEAD(ch, LOST_NOSE))
  {
    act("A nose grows on the front of $n's face.", ch, NULL, NULL, TO_ROOM);
    act("A nose grows on the front of your face.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_HEAD], LOST_NOSE);
    REMOVE_BIT(ch->loc_hp[LOC_HEAD], BROKEN_NOSE);
  }  
  else if (teeth > 0)
  {  
    if (IS_HEAD(ch, LOST_TOOTH_1))
      REMOVE_BIT(ch->loc_hp[LOC_HEAD], LOST_TOOTH_1);
    if (IS_HEAD(ch, LOST_TOOTH_2))
      REMOVE_BIT(ch->loc_hp[LOC_HEAD], LOST_TOOTH_2);
    if (IS_HEAD(ch, LOST_TOOTH_4))
      REMOVE_BIT(ch->loc_hp[LOC_HEAD], LOST_TOOTH_4);
    if (IS_HEAD(ch, LOST_TOOTH_8))
      REMOVE_BIT(ch->loc_hp[LOC_HEAD], LOST_TOOTH_8);
    if (IS_HEAD(ch, LOST_TOOTH_16))
      REMOVE_BIT(ch->loc_hp[LOC_HEAD], LOST_TOOTH_16);
    teeth -= 1; 
    if (teeth >= 16)
    {
      teeth -= 16;
      SET_BIT(ch->loc_hp[LOC_HEAD], LOST_TOOTH_16);
    }
    if (teeth >= 8)
    {
      teeth -= 8;
      SET_BIT(ch->loc_hp[LOC_HEAD], LOST_TOOTH_8);
    }
    if (teeth >= 4)
    {
      teeth -= 4;
      SET_BIT(ch->loc_hp[LOC_HEAD], LOST_TOOTH_4);
    }
    if (teeth >= 2)
    {
      teeth -= 2;
      SET_BIT(ch->loc_hp[LOC_HEAD], LOST_TOOTH_2);
    }
    if (teeth >= 1)
    {
      teeth -= 1;
      SET_BIT(ch->loc_hp[LOC_HEAD], LOST_TOOTH_1);
    }
    act("A missing tooth grows in your mouth.", ch, NULL, NULL, TO_CHAR);
    act("A missing tooth grows in $n's mouth.", ch, NULL, NULL, TO_ROOM);
  }
  else if (IS_HEAD(ch, BROKEN_NOSE))
  {
    act("$n's nose snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your nose snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_HEAD], BROKEN_NOSE);
  }
  else if (IS_HEAD(ch, BROKEN_JAW))
  {
    act("$n's jaw snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your jaw snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_HEAD], BROKEN_JAW);
  }
  else if (IS_HEAD(ch, BROKEN_SKULL))
  {
    act("$n's skull knits itself back together.", ch, NULL, NULL, TO_ROOM);
    act("Your skull knits itself back together.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_HEAD], BROKEN_SKULL);  
  }
  else if (IS_BODY(ch, BROKEN_SPINE))
  {
    act("$n's spine knits itself back together.", ch, NULL, NULL, TO_ROOM);
    act("Your spine knits itself back together.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_BODY], BROKEN_SPINE);  
  }
  else if (IS_BODY(ch, BROKEN_NECK))
  {
    act("$n's neck snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your neck snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_BODY], BROKEN_NECK);
  }
  else if (IS_ARM_L(ch, LOST_ARM))
  {
    act("An arm grows from the stump of $n's left shoulder.", ch, NULL, NULL, TO_ROOM);
    act("An arm grows from the stump of your left shoulder.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], LOST_ARM);  
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], BROKEN_ARM);
    SET_BIT(ch->loc_hp[LOC_ARM_L], LOST_HAND);
  }  
  else if (IS_ARM_R(ch, LOST_ARM))
  {
    act("An arm grows from the stump of $n's right shoulder.", ch, NULL, NULL, TO_ROOM);
    act("An arm grows from the stump of your right shoulder.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], LOST_ARM);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], BROKEN_ARM);
    SET_BIT(ch->loc_hp[LOC_ARM_R], LOST_HAND);
  }  
  else if (IS_LEG_L(ch, LOST_LEG))
  {  
    act("A leg grows from the stump of $n's left hip.", ch, NULL, NULL, TO_ROOM);
    act("A leg grows from the stump of your left hip.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_LEG_L], LOST_LEG);
    REMOVE_BIT(ch->loc_hp[LOC_LEG_L], BROKEN_LEG);
    SET_BIT(ch->loc_hp[LOC_LEG_L], LOST_FOOT);
  }
  else if (IS_LEG_R(ch, LOST_LEG))  
  {
    act("A leg grows from the stump of $n's right hip.", ch, NULL, NULL, TO_ROOM);
    act("A leg grows from the stump of your right hip.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_LEG_R], LOST_LEG);  
    REMOVE_BIT(ch->loc_hp[LOC_LEG_R], BROKEN_LEG);
    SET_BIT(ch->loc_hp[LOC_LEG_R], LOST_FOOT);
  }
  else if (IS_ARM_L(ch, BROKEN_ARM))
  {
    act("$n's left arm snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your left arm snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], BROKEN_ARM);
  }
  else if (IS_ARM_R(ch, BROKEN_ARM))
  {
    act("$n's right arm snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your right arm snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], BROKEN_ARM);
  }
  else if (IS_LEG_L(ch, BROKEN_LEG))
  {
    act("$n's left leg snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your left leg snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_LEG_L], BROKEN_LEG);
  }
  else if (IS_LEG_R(ch, BROKEN_LEG))
  {
    act("$n's right leg snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your right leg snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_LEG_R], BROKEN_LEG);
  }
  else if (IS_ARM_L(ch, LOST_HAND))
  {
    act("A hand grows from the stump of $n's left wrist.", ch, NULL, NULL, TO_ROOM);
    act("A hand grows from the stump of your left wrist.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], LOST_HAND);
    SET_BIT(ch->loc_hp[LOC_ARM_L], LOST_THUMB);
    SET_BIT(ch->loc_hp[LOC_ARM_L], LOST_FINGER_I);
    SET_BIT(ch->loc_hp[LOC_ARM_L], LOST_FINGER_M);
    SET_BIT(ch->loc_hp[LOC_ARM_L], LOST_FINGER_R);
    SET_BIT(ch->loc_hp[LOC_ARM_L], LOST_FINGER_L);
  }
  else if (IS_ARM_R(ch, LOST_HAND))
  {
    act("A hand grows from the stump of $n's right wrist.", ch, NULL, NULL, TO_ROOM);
    act("A hand grows from the stump of your right wrist.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], LOST_HAND);
    SET_BIT(ch->loc_hp[LOC_ARM_R], LOST_THUMB);
    SET_BIT(ch->loc_hp[LOC_ARM_R], LOST_FINGER_I);
    SET_BIT(ch->loc_hp[LOC_ARM_R], LOST_FINGER_M);
    SET_BIT(ch->loc_hp[LOC_ARM_R], LOST_FINGER_R);
    SET_BIT(ch->loc_hp[LOC_ARM_R], LOST_FINGER_L);
  }
  else if (IS_LEG_L(ch, LOST_FOOT)) 
  {
    act("A foot grows from the stump of $n's left ankle.", ch, NULL, NULL, TO_ROOM);
    act("A foot grows from the stump of your left ankle.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_LEG_L], LOST_FOOT); 
  }
  else if (IS_LEG_R(ch, LOST_FOOT))
  {
    act("A foot grows from the stump of $n's right ankle.", ch, NULL, NULL, TO_ROOM);
    act("A foot grows from the stump of your right ankle.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_LEG_R], LOST_FOOT);
  }
  else if (IS_ARM_L(ch, LOST_THUMB))
  {
    act("A thumb slides out of $n's left hand.", ch, NULL, NULL, TO_ROOM);
    act("A thumb slides out of your left hand.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], LOST_THUMB);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], BROKEN_THUMB);
  }
  else if (IS_ARM_L(ch, BROKEN_THUMB))
  {
    act("$n's left thumb snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your left thumb snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], BROKEN_THUMB);
  }
  else if (IS_ARM_L(ch, LOST_FINGER_I))
  {
    act("An index finger slides out of $n's left hand.", ch, NULL, NULL, TO_ROOM);
    act("An index finger slides out of your left hand.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], LOST_FINGER_I);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], BROKEN_FINGER_I);
  }
  else if (IS_ARM_L(ch, BROKEN_FINGER_I))
  {
    act("$n's left index finger snaps back into place.", ch, NULL, NULL, TO_ROOM);  
    act("Your left index finger snaps back into place.", ch, NULL, NULL, TO_CHAR);  
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], BROKEN_FINGER_I);
  }
  else if (IS_ARM_L(ch, LOST_FINGER_M))
  {
    act("A middle finger slides out of $n's left hand.", ch, NULL, NULL, TO_ROOM);
    act("A middle finger slides out of your left hand.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], LOST_FINGER_M);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], BROKEN_FINGER_M);
  }
  else if (IS_ARM_L(ch, BROKEN_FINGER_M))
  {
    act("$n's left middle finger snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your left middle finger snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], BROKEN_FINGER_M);
  }
  else if (IS_ARM_L(ch, LOST_FINGER_R))
  {
    act("A ring finger slides out of $n's left hand.", ch, NULL, NULL, TO_ROOM);
    act("A ring finger slides out of your left hand.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], LOST_FINGER_R);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], BROKEN_FINGER_R);
  }
  else if (IS_ARM_L(ch, BROKEN_FINGER_R))
  {
    act("$n's left ring finger snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your left ring finger snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], BROKEN_FINGER_R);
  }
  else if (IS_ARM_L(ch, LOST_FINGER_L))
  {
    act("A little finger slides out of $n's left hand.", ch, NULL, NULL, TO_ROOM);
    act("A little finger slides out of your left hand.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], LOST_FINGER_L);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], BROKEN_FINGER_L);
  }
  else if (IS_ARM_L(ch, BROKEN_FINGER_L))
  {
    act("$n's left little finger snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your left little finger snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_L], BROKEN_FINGER_L);
  }
  else if (IS_ARM_R(ch, LOST_THUMB))
  {
    act("A thumb slides out of $n's right hand.", ch, NULL, NULL, TO_ROOM);
    act("A thumb slides out of your right hand.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], LOST_THUMB);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], BROKEN_THUMB);
  }
  else if (IS_ARM_R(ch, BROKEN_THUMB))
  {
    act("$n's right thumb snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your right thumb snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], BROKEN_THUMB);
  }
  else if (IS_ARM_R(ch, LOST_FINGER_I))
  {
    act("An index finger slides out of $n's right hand.", ch, NULL, NULL, TO_ROOM);
    act("An index finger slides out of your right hand.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], LOST_FINGER_I);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], BROKEN_FINGER_I);
  }
  else if (IS_ARM_R(ch, BROKEN_FINGER_I))
  {
    act("$n's right index finger snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your right index finger snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], BROKEN_FINGER_I);
  }
  else if (IS_ARM_R(ch, LOST_FINGER_M))
  {
    act("A middle finger slides out of $n's right hand.", ch, NULL, NULL, TO_ROOM);
    act("A middle finger slides out of your right hand.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], LOST_FINGER_M);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], BROKEN_FINGER_M);
  }
  else if (IS_ARM_R(ch, BROKEN_FINGER_M))
  {
    act("$n's right middle finger snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your right middle finger snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], BROKEN_FINGER_M);
  }
  else if (IS_ARM_R(ch, LOST_FINGER_R))
  {
    act("A ring finger slides out of $n's right hand.", ch, NULL, NULL, TO_ROOM);
    act("A ring finger slides out of your right hand.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], LOST_FINGER_R);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], BROKEN_FINGER_R);
  }
  else if (IS_ARM_R(ch, BROKEN_FINGER_R))
  {
    act("$n's right ring finger snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your right ring finger snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], BROKEN_FINGER_R);
  }
  else if (IS_ARM_R(ch, LOST_FINGER_L))
  {
    act("A little finger slides out of $n's right hand.", ch, NULL, NULL, TO_ROOM);
    act("A little finger slides out of your right hand.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], LOST_FINGER_L);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], BROKEN_FINGER_L);
  }
  else if (IS_ARM_R(ch, BROKEN_FINGER_L))
  {
    act("$n's right little finger snaps back into place.", ch, NULL, NULL, TO_ROOM);
    act("Your right little finger snaps back into place.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_ARM_R], BROKEN_FINGER_L);
  }
  else if (IS_BODY(ch, CUT_THROAT))
  {
    if (IS_SET(ch->loc_hp[6], BLEEDING_THROAT))
      return;
    act("The wound in $n's throat closes up.", ch, NULL, NULL, TO_ROOM);
    act("The wound in your throat closes up.", ch, NULL, NULL, TO_CHAR);
    REMOVE_BIT(ch->loc_hp[LOC_BODY], CUT_THROAT);
  }
  return;
}

void update_morted_timer(CHAR_DATA *ch)
{
  if (ch->hit > 0 && ch->pcdata->mortal > 0)
    ch->pcdata->mortal = 0;
  else
  {
    ch->pcdata->mortal++;

    if (ch->pcdata->mortal > 10 && !in_fortress(ch) && !in_arena(ch))
    {
      ch->hit = 100;
      update_pos(ch);
      char_from_room(ch);
      char_to_room(ch, get_room_index(ROOM_VNUM_CITYSAFE), TRUE);
      send_to_char("You receive a heal from the gods.\n\r", ch);
      ch->pcdata->mortal = 0;
      ch->fight_timer = 0;
    }
  }
}

void update_sit_safe_counter(CHAR_DATA * ch)
{
  if (IS_NPC(ch))
    return;

  if (ch->pcdata->safe_counter > 0)
  {
    ch->pcdata->safe_counter--;

    if (ch->pcdata->safe_counter == 0)
    {
      if (ch->level >= 3 && IS_SET(ch->extra, EXTRA_PKREADY))
        send_to_char("You can now pk again.\n\r", ch);
    }
  }
  if (IS_SET(ch->in_room->room_flags, ROOM_SAFE) && ch->level > 2 && ch->level < 7 && getMight(ch) >= RANK_CADET)
  {
    if (!IS_SET(ch->extra, EXTRA_AFK))
      ch->pcdata->sit_safe += number_range(5, 10);

    if ((ch->pcdata->sit_safe > 200 || ch->fight_timer != 0) && !ragnarok && !ch->fighting)
    {
      send_to_char("\nThe gods are tired of granting you refuge.\n\r", ch);
      char_from_room(ch);
      char_to_room(ch, get_room_index(ROOM_VNUM_CITYCENTER), TRUE);
    }
  }
  else
  {
    if (ch->pcdata->sit_safe > 0)
      ch->pcdata->sit_safe -= 10;
    else
      ch->pcdata->sit_safe = 0;
  }
}

void update_drunks(CHAR_DATA * ch)
{
  if (ch->pcdata->condition[COND_DRUNK] > 10 && number_range(1, 10) == 1)
  {
    send_to_char("You hiccup loudly.\n\r", ch);
    act("$n hiccups.", ch, NULL, NULL, TO_ROOM);
  }
  if (ch->pcdata->condition[COND_DRUNK] > 0 && number_range(1, 10) == 5)
  {
    ch->pcdata->condition[COND_DRUNK]--;
    if (ch->pcdata->condition[COND_DRUNK] == 0)
      send_to_char("You feel sober again.\n\r", ch);
  }
}

void sex_update(CHAR_DATA * ch)
{
  if (ch->pcdata->stage[0] > 0 || ch->pcdata->stage[2] > 0)
  {
    CHAR_DATA *vch;

    if (ch->pcdata->stage[1] > 0 && ch->pcdata->stage[2] >= 225)
    {
      ch->pcdata->stage[2] += 1;
      if ((vch = ch->pcdata->partner) != NULL &&
          !IS_NPC(vch) && vch->pcdata->partner == ch && ((vch->pcdata->stage[2] >= 200 && vch->sex == SEX_FEMALE) || (ch->pcdata->stage[2] >= 200 && ch->sex == SEX_FEMALE)))
      {
        if (ch->in_room != vch->in_room)
          return;
        if (vch->pcdata->stage[2] >= 225 && ch->pcdata->stage[2] >= 225 && vch->pcdata->stage[2] < 240 && ch->pcdata->stage[2] < 240)
        {
          ch->pcdata->stage[2] = 240;
          vch->pcdata->stage[2] = 240;
        }
        if (ch->sex == SEX_MALE && vch->pcdata->stage[2] >= 240)
        {
          act("You thrust deeply between $N's warm, damp thighs.", ch, NULL, vch, TO_CHAR);
          act("$n thrusts deeply between your warm, damp thighs.", ch, NULL, vch, TO_VICT);
          act("$n thrusts deeply between $N's warm, damp thighs.", ch, NULL, vch, TO_NOTVICT);
          if (vch->pcdata->stage[2] > ch->pcdata->stage[2])
            ch->pcdata->stage[2] = vch->pcdata->stage[2];
        }
        else if (ch->sex == SEX_FEMALE && vch->pcdata->stage[2] >= 240)
        {
          act("You squeeze your legs tightly around $N, moaning loudly.", ch, NULL, vch, TO_CHAR);
          act("$n squeezes $s legs tightly around you, moaning loudly.", ch, NULL, vch, TO_VICT);
          act("$n squeezes $s legs tightly around $N, moaning loudly.", ch, NULL, vch, TO_NOTVICT);
          if (vch->pcdata->stage[2] > ch->pcdata->stage[2])
            ch->pcdata->stage[2] = vch->pcdata->stage[2];
        }
      }
      if (ch->pcdata->stage[2] >= 250)
      {
        if ((vch = ch->pcdata->partner) != NULL && !IS_NPC(vch) && vch->pcdata->partner == ch && ch->in_room == vch->in_room)
        {
          vch->pcdata->stage[2] = 250;
          if (ch->sex == SEX_MALE)
          {
            stage_update(ch, vch, 2, "xm-thrust");
            stage_update(vch, ch, 2, "xf-squeeze");
          }
          else
          {
            stage_update(vch, ch, 2, "xm-thrust");
            stage_update(ch, vch, 2, "xf-squeeze");
          }

          ch->pcdata->stage[0] = 0;
          vch->pcdata->stage[0] = 0;

          if (!IS_EXTRA(ch, EXTRA_EXP))
          {
            send_to_char("Congratulations on achieving a simultanious orgasm!  Receive 100000 exp!\n\r", ch);
            SET_BIT(ch->extra, EXTRA_EXP);
            ch->exp += 100000;
          }
          if (!IS_EXTRA(vch, EXTRA_EXP))
          {
            send_to_char("Congratulations on achieving a simultanious orgasm!  Receive 100000 exp!\n\r", vch);
            SET_BIT(vch->extra, EXTRA_EXP);
            vch->exp += 100000;
          }
        }
      }
    }
    else
    {
      if (ch->pcdata->stage[0] > 0 && ch->pcdata->stage[2] < 1 && ch->position != POS_RESTING)
      {
        if (ch->pcdata->stage[0] > 1)
          ch->pcdata->stage[0] -= 1;
        else
          ch->pcdata->stage[0] = 0;
      }
      else if (ch->pcdata->stage[2] > 0 && ch->pcdata->stage[0] < 1)
      {
        if (ch->pcdata->stage[2] > 10)
          ch->pcdata->stage[2] -= 10;
        else
          ch->pcdata->stage[2] = 0;
        if (ch->sex == SEX_MALE && ch->pcdata->stage[2] == 0)
          send_to_char("You feel fully recovered.\n\r", ch);
      }
    }
  }
}

void regen_limb(CHAR_DATA * ch)
{
  if (ch->loc_hp[6] > 0)
  {
    int sn = skill_lookup("clot");

    (*skill_table[sn].spell_fun) (sn, ch->level, ch, ch);
  }
  else if ((ch->loc_hp[0] + ch->loc_hp[1] + ch->loc_hp[2] + ch->loc_hp[3] + ch->loc_hp[4] + ch->loc_hp[5]) != 0)
    reg_mend(ch);
}

void update_arti_regen(CHAR_DATA * ch)
{
  if (ch->hit < ch->max_hit)
    regen_hps(ch, 2);
  if (ch->mana < ch->max_mana)
    regen_mana(ch, 2);
  if (ch->move < ch->max_move)
    regen_move(ch, 2);

  regen_limb(ch);
}

void regen_hps(CHAR_DATA *ch, int multiplier)
{
  ITERATOR *pIter;
  FEED_DATA *feed;
  int min = 80;
  int max = 180;
  int gain = number_range(min, max);

  if (event_isset_mobile(ch, EVENT_MOBILE_MUMMYROT))
    return;

  if (ch->position == POS_SLEEPING)
    gain *= 2;
 
  if (ch->in_room && ch->in_room->area)
  {
    if (ch->fight_timer == 0 && event_isset_area(ch->in_room->area, EVENT_AREA_EARTHMOTHER))
      gain = 3 * gain / 2;
    if (ch->fight_timer > 0 && event_isset_area(ch->in_room->area, EVENT_AREA_PLAGUE))
      gain = 0;
  }

  ch->hit = UMIN(ch->hit + (gain * multiplier), ch->max_hit);

  /* remove all feed data once a player is fully healed */
  if (!IS_NPC(ch) && ch->hit >= ch->max_hit)
  {
    pIter = AllocIterator(ch->pcdata->feeders);
    while ((feed = (FEED_DATA *) NextInList(pIter)) != NULL)
      free_feed(ch, feed);
  }

  update_pos(ch);
}

void regen_mana(CHAR_DATA *ch, int multiplier)
{
  int min = 80;
  int max = 180;
  int gain = number_range(min, max);

  if (ch->position == POS_MEDITATING)
    gain *= 2;
 
  if (ch->in_room && ch->in_room->area)
  {
    if (ch->fight_timer == 0 && event_isset_area(ch->in_room->area, EVENT_AREA_EARTHMOTHER))
      gain = 3 * gain / 2;
    if (ch->fight_timer > 0 && event_isset_area(ch->in_room->area, EVENT_AREA_PLAGUE))
      gain = 0;
  }

  ch->mana = UMIN(ch->mana + (gain * multiplier), ch->max_mana);
}

void regen_move(CHAR_DATA *ch, int multiplier)
{
  int min = 80;
  int max = 180;
  int gain = number_range(min, max); 

  if (ch->position == POS_SLEEPING)
    gain *= 2;

  if (ch->in_room && ch->in_room->area)
  {
    if (ch->fight_timer == 0 && event_isset_area(ch->in_room->area, EVENT_AREA_EARTHMOTHER))
      gain = 3 * gain / 2;
    if (ch->fight_timer > 0 && event_isset_area(ch->in_room->area, EVENT_AREA_PLAGUE))
      gain = 0;
  }

  ch->move = UMIN(ch->move + (gain * multiplier), ch->max_move);
}

void update_active_counters(CHAR_DATA *ch)
{
  /* we do not update idle players */
  if (ch->timer >= 5) return;

  if (ch->pcdata->time_tick < 250)
    ch->pcdata->time_tick++;
}

bool update_player_idle(CHAR_DATA *ch)
{
  ch->timer++;

  /* after 6 minutes we void the player , after 8 we autoquit him/her */
  if (ch->timer >= 120)
  {
    if (!in_fortress(ch))
    {
      do_quit(ch, "");
      return TRUE;
    }
  }
  else if (ch->timer >= 90)
  {
    if (!in_fortress(ch) && !in_arena(ch))
    {
      if (ch->was_in_room == NULL && ch->in_room != NULL)
      {
        ch->was_in_room = ch->in_room;
        if (ch->fighting != NULL)
          stop_fighting(ch, TRUE);
        act("$n disappears into the void.", ch, NULL, NULL, TO_ROOM);
        send_to_char("You disappear into the void.\n\r", ch);
        char_from_room(ch);
        char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO), TRUE);
      }
    }
  }

  return FALSE;
}

void update_midi(CHAR_DATA *ch)
{
  if (ch->pcdata->prev_area && ch->in_room)
  {
    if (ch->pcdata->prev_area != ch->in_room->area)
    {
      ch->pcdata->prev_area = ch->in_room->area;
      update_music(ch);
    }
  }
  else if (ch->in_room)
  {
    ch->pcdata->prev_area = ch->in_room->area;
    update_music(ch);
  }
}

void update_edge(CHAR_DATA *ch)
{
  CHAR_DATA *gch;
  ITERATOR *pIter;

  pIter = AllocIterator(char_list);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_NPC(gch)) continue;
    REMOVE_BIT(gch->pcdata->tempflag, TEMP_EDGE);
  }

  SET_BIT(ch->pcdata->tempflag, TEMP_EDGE);
}
