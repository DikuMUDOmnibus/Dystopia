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

/***************************************************************************
 *  Dystopian Questcode copyright © 2001 & 2002 by Brian Graversen, users  *
 *  must follow the DIKU, Merc and Godwars license as well as the license  *
 *  distributed with Dystopia                                              *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dystopia.h"

int  get_rand_mob              ( int min, int max );
int  get_rand_item             ( void );
int  load_special_item         ( CHAR_DATA *ch );
void give_token                ( CHAR_DATA *questmaster, CHAR_DATA *ch, int value );
void load_durgaard_key         ( void );

STACK *quest_free = NULL;

DECLARE_QUEST_FUN( questspec_special_item	);
DECLARE_QUEST_FUN( questspec_rand_mob		);
DECLARE_QUEST_FUN( questspec_mob_and_item	);
DECLARE_QUEST_FUN( questspec_hard_mob		);
DECLARE_QUEST_FUN( questspec_pk                 );
DECLARE_QUEST_FUN( questspec_durgaard		);
DECLARE_QUEST_FUN( questspec_dwarvensage	);
DECLARE_QUEST_FUN( questspec_gemqueen           );

const struct quest_type quest_table [] =
{
  { "questspec_special_item",    questspec_special_item     },
  { "questspec_rand_mob",        questspec_rand_mob         },
  { "questspec_mob_and_item",    questspec_mob_and_item     },
  { "questspec_hard_mob",        questspec_hard_mob         },
  { "questspec_pk",              questspec_pk               },
  { "questspec_durgaard",        questspec_durgaard         },
  { "questspec_dwarvensage",     questspec_dwarvensage      },
  { "questspec_gemqueen",        questspec_gemqueen         },

  /* end of table */
  { "", 0 }
};

/*
 * Used for show_char_to_char_0()
 */
bool is_quest_target(CHAR_DATA *ch, CHAR_DATA *victim)
{
  QUEST_DATA *quest;
  ITERATOR *pIter;

  if (IS_NPC(ch) || !IS_NPC(victim)) return FALSE;

  pIter = AllocIterator(ch->pcdata->quests);
  while ((quest = (QUEST_DATA *) NextInList(pIter)) != NULL)
  {
    switch(quest->type)
    {
      case QT_MOB:
        if (victim->pIndexData->vnum == quest->vnums[0])
          return TRUE;
        break;
      case QT_MOB_AND_OBJ:
        if (victim->pIndexData->vnum == quest->vnums[0])
          return TRUE;
        break;
      default: continue;
    }
  }
  return FALSE;
}

/*
 * Returns the vnum of a random object that actually pops on the mud.
 * It's a bit ugly, but it works :)
 */
int get_rand_item()
{
  MOB_INDEX_DATA *lastMob;
  OBJ_INDEX_DATA *pObjIndex;
  ROOM_INDEX_DATA *pRoom;
  ITERATOR *pIter;
  RESET_DATA *pReset;
  AREA_DATA *area;
  int vnum, i;
  bool found = FALSE;

  while (!found)
  {
    vnum = number_range(3000, 10000);
    if ((pObjIndex = get_obj_index(vnum)) != NULL)
    {
      if (!CAN_WEAR(pObjIndex, ITEM_TAKE)) continue;
      if (pObjIndex->item_type == ITEM_MONEY) continue;
      if ((area = pObjIndex->area) != NULL && !IS_SET(area->areabits, AREA_BIT_OLC))
      {

        /* these areas we don't want to pop from */
        if (strstr(area->name, "The Newbie Zone") ||
            strstr(area->name, "Durgaard Halls") ||
            strstr(area->name, "The Catacombs"))
          continue;

        for (i = area->lvnum; i <= area->uvnum; i++)
        {
          if ((pRoom = get_room_index(i)) != NULL)
          {
            lastMob = NULL;

            pIter = AllocIterator(pRoom->resets);
            while ((pReset = (RESET_DATA *) NextInList(pIter)) != NULL)
            {
              switch (pReset->command)
              {
                default:
                  break;
                case 'O':
                  if ((pObjIndex = get_obj_index(pReset->arg1)) != NULL)
                  {
                    if (pObjIndex->vnum == vnum)
                      found = TRUE;
                  }
                  break;
                case 'G':
                  if (lastMob != NULL && lastMob->shop_fun != NULL)
                    break;
                  if ((pObjIndex = get_obj_index(pReset->arg1)) != NULL)
                  {
                    if (pObjIndex->vnum == vnum)
                      found = TRUE;
                  }
                  break;
                case 'M':
                  lastMob = get_mob_index(pReset->arg1);
                  break;
              }
            }
          }
        }
      }
    }
  }
  return vnum;
}

int load_special_item(CHAR_DATA *ch)
{
  OBJ_DATA *obj;
  OBJ_INDEX_DATA *pObjIndex;
  int vnum;

  vnum = number_range(6800, 6805);
  if ((pObjIndex = get_obj_index(vnum)) == NULL)
  {
    bug("Load_special_item : %d", vnum);
    return 3;  /* Just to be on the safe side */
  }
  obj = create_object(pObjIndex, 50);
  obj->ownerid = ch->pcdata->playerid;
  SET_BIT(obj->extra_flags, ITEM_NOLOCATE);
  object_decay(obj, 12 * 60);
  obj_to_room(obj, get_rand_room());
  return vnum;
}

int get_rand_mob(int min, int max)
{
  AREA_DATA *area;
  MOB_INDEX_DATA *pMob;
  ROOM_INDEX_DATA *pRoom;
  ITERATOR *pIter;
  RESET_DATA *pReset;
  int vnum, i = 0, k;

  while (i++ < 800)
  {
    vnum = number_range(3000, 10000);

    if ((pMob = get_mob_index(vnum)) != NULL)
    {
      if (pMob->shop_fun != NULL || pMob->quest_fun != NULL)
        continue;

      if (pMob->level >= min && pMob->level <= max)
      {
        if ((area = pMob->area) != NULL && !IS_SET(area->areabits, AREA_BIT_OLC))
        {

          /* these areas we don't want to pop from */
          if (strstr(area->name, "The Newbie Zone") ||
              strstr(area->name, "Durgaard Halls") ||
              strstr(area->name, "The Catacombs"))
            continue;

          for (k = area->lvnum; k <= area->uvnum; k++)
          {
            if ((pRoom = get_room_index(k)) != NULL)
            {
              pIter = AllocIterator(pRoom->resets);
              while ((pReset = (RESET_DATA *) NextInList(pIter)) != NULL)
              {
                switch (pReset->command)
                {
                  default:
                    break;
                  case 'M':
                    if ((pMob = get_mob_index(pReset->arg1)) != NULL)
                    {
                      if (pMob->vnum == vnum)
                        return vnum;
                    }
                    break;
                }
              }
            }
          }
        }
      }
    }
  }

  return -1;
}

void do_showquest(CHAR_DATA *ch, char *argument)
{
  QUEST_DATA *quest;
  OBJ_INDEX_DATA *pObjIndex;
  MOB_INDEX_DATA *pMobIndex1;
  MOB_INDEX_DATA *pMobIndex2;
  CHAR_DATA *gch;
  CHAR_DATA *ich = NULL;
  ITERATOR *pIter, *pIter2;
  char buf[MAX_STRING_LENGTH];
  bool found = FALSE;
  bool found2 = FALSE;

  if (IS_NPC(ch)) return;

  pIter2 = AllocIterator(ch->pcdata->quests);
  while ((quest = (QUEST_DATA *) NextInList(pIter2)) != NULL)
  {
    found = TRUE;
    switch(quest->type)
    {
      default:
        sprintf(buf, "Do_showquests: %s has bad quest type %d.", ch->name, quest->type);
        bug(buf, 0);
        break;
      case QT_MOB:
        if (quest->vnums[0] != -1 && (pMobIndex1 = get_mob_index(quest->vnums[0])) == NULL)
        {
          sprintf(buf, "Do_showquests: %s has bad quest mob %d.", ch->name, quest->vnums[0]);
          bug(buf, 0);
          break;
        }
        if ((pMobIndex2 = get_mob_index(quest->giver)) == NULL)
        {
          sprintf(buf, "Do_showquests: %s has bad quest mob %d.", ch->name, quest->giver);
          bug(buf, 0);
          break; 
        }
        sprintf(buf, "You have been charged by #G%s#n to complete the following quest\n\r",
          pMobIndex2->short_descr);
        send_to_char(buf, ch);
        if (quest->vnums[0] != -1)
        {
          sprintf(buf, " * Find and slay #R%s#n.\n\r", pMobIndex1->short_descr);
          send_to_char(buf, ch);
          sprintf(buf, " * You have #C%s#n to complete the quest.\n\r", get_time_left(quest->expire - current_time));
          send_to_char(buf, ch);
        }
        else
        {
          send_to_char(" * This quest has been #Ccompleted#n.\n\r", ch);
          sprintf(buf, " * You have #C%s#n to return to the questgiver.\n\r", get_time_left(quest->expire - current_time));
          send_to_char(buf, ch);
        }
        break;
      case QT_PK:
        if ((pMobIndex2 = get_mob_index(quest->giver)) == NULL)
        {
          sprintf(buf, "Do_showquests: %s has bad quest mob %d.", ch->name, quest->giver);
          bug(buf, 0);
          break;
        }

        pIter = AllocIterator(char_list);
        while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
        {
          if (IS_NPC(gch)) continue;
          if (gch->pcdata->playerid == quest->vnums[0])
          {
            ich = gch;
            found2 = TRUE;
          }
        }
        sprintf(buf, "You have been charged by #G%s#n to complete the following quest\n\r",
          pMobIndex2->short_descr);
        send_to_char(buf, ch);
        if (quest->vnums[0] != -1)
        {
          sprintf(buf, " * Hunt and slay #R%s#n.\n\r",
            found2 ? ich->name : "someone");
          send_to_char(buf, ch);
          sprintf(buf, " * You have #C%s#n to complete the quest.\n\r", get_time_left(quest->expire - current_time));
          send_to_char(buf, ch);
        }
        else
        {
          send_to_char(" * This quest has been #Ccompleted#n.\n\r", ch);
          sprintf(buf, " * You have #C%s#n to return to the questgiver.\n\r", get_time_left(quest->expire - current_time));
          send_to_char(buf, ch);
        }
        break;
      case QT_OBJ:
        if ((pObjIndex = get_obj_index(quest->vnums[0])) == NULL)
        {
          sprintf(buf, "Do_showquests: %s has bad quest item %d.", ch->name, quest->vnums[0]);
          bug(buf, 0);
          break;
        }
        if ((pMobIndex2 = get_mob_index(quest->giver)) == NULL)
        {
          sprintf(buf, "Do_showquests: %s has bad quest mob %d.", ch->name, quest->giver);
          bug(buf, 0);
          break;
        }
        sprintf(buf, "You have been charged by #G%s#n to complete the following quest\n\r",
          pMobIndex2->short_descr);
        send_to_char(buf, ch);
        sprintf(buf, " * Find and return #R%s#n.\n\r", pObjIndex->short_descr);
        send_to_char(buf, ch);
        sprintf(buf, " * You have #C%s#n to complete the quest.\n\r", get_time_left(quest->expire - current_time));
        send_to_char(buf, ch);
        break;
      case QT_MOB_AND_OBJ:
        if (quest->vnums[0] != -1 && (pMobIndex1 = get_mob_index(quest->vnums[0])) == NULL)
        {
          sprintf(buf, "Do_showquests: %s has bad quest mob %d.", ch->name, quest->vnums[0]); 
          bug(buf, 0);
          break;
        }
        if ((pObjIndex = get_obj_index(quest->vnums[1])) == NULL)
        {
          sprintf(buf, "Do_showquests: %s has bad quest item %d.", ch->name, quest->vnums[1]);
          bug(buf, 0);
          break;
        }
        if ((pMobIndex2 = get_mob_index(quest->giver)) == NULL)
        {
          sprintf(buf, "Do_showquests: %s has bad quest mob %d.", ch->name, quest->giver);
          bug(buf, 0);
          break;
        }
        sprintf(buf, "You have been charged by #G%s#n to complete the following quest\n\r",
          pMobIndex2->short_descr);
        send_to_char(buf, ch);
        if (quest->vnums[0] != -1)
        {
          sprintf(buf, " * Find and slay #R%s#n.\n\r", pMobIndex1->short_descr);
          send_to_char(buf, ch);
        }
        sprintf(buf, " * Find and return #R%s#n.\n\r", pObjIndex->short_descr);
        send_to_char(buf, ch);
        sprintf(buf, " * You have #C%s#n to complete the quest.\n\r", get_time_left(quest->expire - current_time));
        send_to_char(buf, ch);
        break;
    }
  }
  if (!found) send_to_char("You are currently undertaking no quests.\n\r", ch);
}

void quest_to_char(CHAR_DATA *ch, QUEST_DATA *quest)
{
  QUEST_DATA *quest_new;

  if (IS_NPC(ch))
  {
    bug("Quest_to_char: on npc", 0);
    return;
  }

  if ((quest_new = (QUEST_DATA *) PopStack(quest_free)) == NULL)
    quest_new = malloc(sizeof(*quest_new));

 *quest_new = *quest;
  AttachToList(quest_new, ch->pcdata->quests);
}

/*
 * Remove an affect from a char.
 */
void quest_from_char(CHAR_DATA *ch, QUEST_DATA *quest)
{
  if (IS_NPC(ch))
  {
    bug("Quest_from_char: on npc", 0);
    return;
  }
  DetachFromList(quest, ch->pcdata->quests);
  PushStack(quest, quest_free);
}

QUEST_FUN *quest_lookup(const char *name)
{
  int cmd;

  for (cmd = 0; *quest_table[cmd].quest_name; cmd++)
    if (!str_cmp(name, quest_table[cmd].quest_name))
      return quest_table[cmd].quest_fun;
  return 0;
}

char *quest_string(QUEST_FUN *fun)
{
  int cmd;

  for (cmd = 0; *quest_table[cmd].quest_fun; cmd++)
    if (fun == quest_table[cmd].quest_fun)
      return quest_table[cmd].quest_name;

  return 0;
}

void give_token(CHAR_DATA *questmaster, CHAR_DATA *ch, int value)
{
  OBJ_DATA *obj;
  KINGDOM_DATA *kingdom;
  char buf[MAX_STRING_LENGTH];

  value *= muddata.ccenter[CCENTER_QPS_LEVEL];
  value /= 100;

  value *= 100 + ch->pcdata->status * 2;
  value /= 100;

  if (IS_SET(ch->pcdata->tempflag, TEMP_EDGE))
  {
    value *= 11;
    value /= 10;
  }
  if (ch->pcdata->time_tick > 49)
  {
    value *= 100 + ch->pcdata->time_tick/10;
    value /= 100;
  }

  if ((kingdom = get_kingdom(ch)) != NULL)
  {
    int tax;

    tax = (kingdom->taxrate * value) / 100;
    value -= tax;
    kingdom->treasury += tax;
    save_kingdom(kingdom);
  }

  obj = create_object(get_obj_index(OBJ_VNUM_PROTOPLASM), 50);
  obj->value[0] = value;
  obj->level = value; 
  obj->cost = 1000;
  obj->item_type = ITEM_QUEST;
  obj_to_char(obj, ch);
  free_string(obj->name);
  obj->name = str_dup("goldcrowns bag");
  sprintf(buf, "a bag of %d goldcrowns", value);
  free_string(obj->short_descr);
  obj->short_descr = str_dup(buf);
  sprintf(buf, "A bag with %d goldcrowns lies on the floor.", value);
  free_string(obj->description);
  obj->description = str_dup(buf);
  act("You receive $p from $N.", ch, obj, questmaster, TO_CHAR);
  act("$n receives $p from $N.", ch, obj, questmaster, TO_ROOM);

  ch->pcdata->session->quests++;
  ch->pcdata->session->gold += value;

  ch->pcdata->questsrun++;
  setGoldTotal(ch, value);
}

void do_qcomplete(CHAR_DATA *ch, char *argument)  
{
  CHAR_DATA *questmaster;
  QUEST_DATA *quest;
  ITERATOR *pIter;
  OBJ_DATA *obj;
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  bool found = FALSE;

  if (IS_NPC(ch)) return;
  argument = one_argument(argument, arg);
  one_argument(argument, arg2);
  if ((questmaster = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("You cannot seem to find that questmaster.\n\r", ch);
    return;
  }
  if (!IS_NPC(questmaster))   
  {
    send_to_char("Players cannot give quests.\n\r", ch);
    return;
  }
  if (questmaster->quest_fun != 0)
  {
    pIter = AllocIterator(ch->pcdata->quests);
    while ((quest = (QUEST_DATA *) NextInList(pIter)) != NULL && !found)
    {
      if (quest->giver != questmaster->pIndexData->vnum) continue;
      found = TRUE;

      /* Let's check if the quest is actually completed */
      switch (quest->type)
      {
        default:
          bug("Quest_complete: Bad Quest Type", 0);
          return;
        case QT_MOB:
          if (quest->vnums[0] != -1)
          {
            send_to_char("You have not completed that quest yet.\n\r", ch);
            return;
          }
          break;
        case QT_PK:
          if (quest->vnums[0] != -1)
          {
            send_to_char("You have not completed that quest yet.\n\r", ch);
            return;
          }
          break;
        case QT_OBJ:
          if ((obj = get_obj_carry(ch, arg2)) == NULL)
          {
            send_to_char("What object do you wish to return?\n\r", ch);
            return;
          }
          if (obj->pIndexData->vnum != quest->vnums[0])
          {
            send_to_char("That is not the object of the quest.\n\r", ch);
            return;
          }
          extract_obj(obj);
          break;
        case QT_MOB_AND_OBJ:
          if (quest->vnums[0] != -1)
          {
            send_to_char("You have not completed that quest yet.\n\r", ch);
            return;
          }
          if ((obj = get_obj_carry(ch, arg2)) == NULL)
          {
            send_to_char("What object do you wish to return?\n\r", ch);
            return;   
          }
          if (obj->pIndexData->vnum != quest->vnums[1])
          {
            send_to_char("That is not the object of the quest.\n\r", ch);
            return;
          }
          extract_obj(obj);
          break;
      }
      (*questmaster->quest_fun)(questmaster, ch, "complete");
      quest_from_char(ch, quest);
    }
    if (!found) send_to_char("You are not questing for this questmaster.\n\r", ch);
  }
  else
    send_to_char("Doesn't seem like that's a questmaster.\n\r", ch);
  return;
}

void do_qgain(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *questmaster;
  QUEST_DATA *quest;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) return;
  one_argument(argument, arg);
  if ((questmaster = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("You cannot seem to find that questmaster.\n\r", ch);
    return;
  }
  if (!IS_NPC(questmaster))
  {
    send_to_char("Players cannot give quests.\n\r", ch);
    return;
  }
  if (questmaster->quest_fun != 0)
  {
    if (!IS_SET(ch->extra, EXTRA_PKREADY) && ch->pcdata->questsrun >= 100)
    {
      send_to_char("If you wish to solve any more quests, you must become pkready.\n\r", ch);
      return;
    }

    pIter = AllocIterator(ch->pcdata->quests);
    while ((quest = (QUEST_DATA *) NextInList(pIter)) != NULL)
    {
      if (quest->giver == questmaster->pIndexData->vnum)
      {
        send_to_char("You have already been given a quest from this questmaster.\n\r", ch);
        return;
      }
    }

    /* ROCK THEM! */
    (*questmaster->quest_fun)(questmaster, ch, "gain");  
  }
  else
    send_to_char("Doesn't seem like that's a questmaster.\n\r", ch);
}

void questspec_rand_mob(CHAR_DATA *questmaster, CHAR_DATA *ch, char *argument)
{
  if (!str_cmp(argument, "gain"))
  {
    QUEST_DATA new_quest;

    new_quest.type      = QT_MOB;
    new_quest.expire    = current_time + (7 * 60);
    new_quest.giver     = questmaster->pIndexData->vnum;
    new_quest.vnums[0]  = get_rand_mob(100, 350);
    new_quest.vnums[1]  = 0;
    new_quest.vnums[2]  = 0;
    new_quest.vnums[3]  = 0;

    if (new_quest.vnums[0] == -1)
    {
      do_say(questmaster, "I'm afraid I have no jobs for you at the moment.");
      return;
    }

    do_say(questmaster, "I charge you to find and kill this monster!");
    quest_to_char(ch, &new_quest);
    return;
  } 
  else if (!str_cmp(argument, "complete"))
  {
    int value = 40;

    give_token(questmaster, ch, value);
    do_say(questmaster, "Thanks for solving my quest, come back again if you want.");
    return;
  }
}

void questspec_hard_mob(CHAR_DATA *questmaster, CHAR_DATA *ch, char *argument)
{
  if (!str_cmp(argument, "gain"))
  {
    QUEST_DATA new_quest;

    new_quest.type      = QT_MOB;
    new_quest.expire    = current_time + (7 * 60);
    new_quest.giver     = questmaster->pIndexData->vnum;

    if (getMight(ch) >= RANK_GENERAL)
      new_quest.vnums[0]  = get_rand_mob(500, 850);
    else
      new_quest.vnums[0]  = get_rand_mob(350, 650);

    new_quest.vnums[1]  = 0;
    new_quest.vnums[2]  = 0;
    new_quest.vnums[3]  = 0;

    if (new_quest.vnums[0] == -1)
    {
      do_say(questmaster, "I'm afraid I have no jobs for you at the moment.");
      return;
    }

    do_say(questmaster, "I charge you to find and kill this horrible monster!");
    quest_to_char(ch, &new_quest);
    return;
  }
  else if (!str_cmp(argument, "complete"))
  {
    int value = 80;

    give_token(questmaster, ch, value);
    do_say(questmaster, "Thanks for solving my quest, come back again if you want.");
    return;
  } 
}  

void questspec_special_item(CHAR_DATA *questmaster, CHAR_DATA *ch, char *argument)
{
  if (!str_cmp(argument, "gain"))
  {
    QUEST_DATA new_quest;

    do_say(questmaster, "I charge you to retrive this ancient artifact of mine!");
    new_quest.type      = QT_OBJ;   
    new_quest.expire    = current_time + (7 * 60);
    new_quest.giver     = questmaster->pIndexData->vnum;
    new_quest.vnums[0]  = load_special_item(ch);
    new_quest.vnums[1]  = 0;
    new_quest.vnums[2]  = 0;
    new_quest.vnums[3]  = 0;
    quest_to_char(ch, &new_quest);
    return;
  }
  else if (!str_cmp(argument, "complete"))
  {
    int value = 100;

    give_token(questmaster, ch, value);
    do_say(questmaster, "Thanks for solving my quest, come back again if you want.");
    return;
  }
}

void questspec_mob_and_item(CHAR_DATA *questmaster, CHAR_DATA *ch, char *argument)
{
  if (!str_cmp(argument, "gain"))
  {
    QUEST_DATA new_quest;   

    do_say(questmaster, "Please slay this monster and retrive this item for me!");
    new_quest.type      = QT_MOB_AND_OBJ;
    new_quest.expire    = current_time + (7 * 60);
    new_quest.giver     = questmaster->pIndexData->vnum;
    new_quest.vnums[0]  = get_rand_mob(100, 400);
    new_quest.vnums[1]  = get_rand_item();
    new_quest.vnums[2]  = 0;
    new_quest.vnums[3]  = 0;
    quest_to_char(ch, &new_quest);
    return;
  }
  else if (!str_cmp(argument, "complete"))
  {
    int value = 90;

    give_token(questmaster, ch, value);
    do_say(questmaster, "Thanks for solving my quest, come back again if you want.");
    return;
  }
}

void load_durgaard_key()
{
  MOB_INDEX_DATA *pMob = get_mob_index(MOB_VNUM_DWARF_WARRIOR);
  OBJ_INDEX_DATA *pObj = get_obj_index(OBJ_VNUM_DURGAARDKEY);
  CHAR_DATA *ch;
  OBJ_DATA *obj;
  ROOM_INDEX_DATA *pRoom;
  EVENT_DATA *event;

  if (pMob == NULL)
  {
    bug("load_durgaard_key: no such monster.", 0);
    return;
  }
  if (pObj == NULL)
  {
    bug("load_durgaard_key: no such item.", 0);
    return;
  }

  ch = create_mobile(pMob);
  REMOVE_BIT(ch->act, ACT_AGGRESSIVE);

  do
  {
    pRoom = get_room_index(number_range(3604, 3692));
  } while (pRoom == NULL);

  char_to_room(ch, pRoom, TRUE);
  obj = create_object(pObj, 50);
  object_decay(obj, 12 * 60);
  obj_to_char(obj, ch);

  event = alloc_event();
  event->type = EVENT_MOBILE_EXTRACT;
  event->fun = &event_mobile_extract;
  add_event_char(event, ch, 10 * 60 * PULSE_PER_SECOND);
}

void questspec_gemqueen(CHAR_DATA *questmaster, CHAR_DATA *ch, char *argument)
{
  if (!str_cmp(argument, "gain"))
  {
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *pRoom;
    OBJ_INDEX_DATA *pObjIndexData;
    QUEST_DATA new_quest;

    if ((pObjIndexData = get_obj_index(OBJ_VNUM_GEMSTONEEGG)) == NULL)
    {
      do_say(questmaster, "I'm broken, go tell an admin.");
      return;
    }

    for (;;)
    {
      if ((pRoom = get_room_index(number_range(3604, 3692))) != NULL)
        break;
    }

    do_say(questmaster, "The wicked dwarves has stolen one of our eggs, go fetch it.");

    new_quest.type      = QT_OBJ;
    new_quest.expire    = current_time + (6 * 60);
    new_quest.giver     = questmaster->pIndexData->vnum;
    new_quest.vnums[0]  = OBJ_VNUM_GEMSTONEEGG;
    new_quest.vnums[1]  = 0;
    new_quest.vnums[2]  = 0;
    new_quest.vnums[3]  = 0;

    obj = create_object(pObjIndexData, 50);
    obj_to_room(obj, pRoom);
    object_decay(obj, 480);

    quest_to_char(ch, &new_quest);
  }
  else if (!str_cmp(argument, "complete"))
  {
    OBJ_INDEX_DATA *pObj;
    OBJ_DATA *obj;

    if ((pObj = get_obj_index(OBJ_VNUM_GEMSTONEPASS)) == NULL)
    {
      do_say(questmaster, "Hmm, I'm all out of gemstone passes, please tell an admin.");
      return;
    }

    obj = create_object(pObj, 50);
    obj_to_char(obj, ch);

    do_say(questmaster, "Take this gemstone pass as a reward for your actions.");
    act("$N hands $p to $n.", ch, obj, questmaster, TO_ROOM);
    act("$N hands $p to you.", ch, obj, questmaster, TO_CHAR);
  }
}

void questspec_dwarvensage(CHAR_DATA *questmaster, CHAR_DATA *ch, char *argument)
{
  if (!str_cmp(argument, "gain"))
  {
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *pRoom;
    OBJ_INDEX_DATA *pObjIndexData;
    QUEST_DATA new_quest;

    if ((pObjIndexData = get_obj_index(OBJ_VNUM_CRACKEDEGG)) == NULL ||
        (pRoom = get_room_index(ROOM_VNUM_EGGCHAMBER)) == NULL)
    {
      do_say(questmaster, "I'm broken, go tell an admin.");
      return;
    }

    do_say(questmaster, "Find me a cracked gemstone egg, and I'll lead you to the 3rd layer.");

    new_quest.type      = QT_OBJ;
    new_quest.expire    = current_time + (6 * 60);
    new_quest.giver     = questmaster->pIndexData->vnum;
    new_quest.vnums[0]  = OBJ_VNUM_CRACKEDEGG;
    new_quest.vnums[1]  = 0;
    new_quest.vnums[2]  = 0;
    new_quest.vnums[3]  = 0;

    obj = create_object(pObjIndexData, 50);
    obj_to_room(obj, pRoom);
    object_decay(obj, 480);

    quest_to_char(ch, &new_quest);
  }
  else if (!str_cmp(argument, "complete"))
  {
    OBJ_DATA *obj;

    obj = create_object(get_obj_index(OBJ_VNUM_PORTAL), 0);
    obj->value[0] = ROOM_VNUM_PILLAR;
    obj->value[3] = ch->in_room->vnum;
    object_decay(obj, 10);
    obj_to_room(obj, ch->in_room);

    do_say(questmaster, "Enter the portal to the 3rd layer brave warrior.");

    act("$p appears in front of $n.", ch, obj, NULL, TO_ROOM);
    act("$p appears in front of you.", ch, obj, NULL, TO_CHAR);
  }
}

void questspec_durgaard(CHAR_DATA *questmaster, CHAR_DATA *ch, char *argument)
{
  if (!str_cmp(argument, "gain"))
  {
    QUEST_DATA new_quest;

    do_say(questmaster, "You must find and return the Durgaard Castle Key.");

    new_quest.type      = QT_OBJ;
    new_quest.expire    = current_time + (6 * 60);
    new_quest.giver     = questmaster->pIndexData->vnum;
    new_quest.vnums[0]  = OBJ_VNUM_DURGAARDKEY;
    new_quest.vnums[1]  = 0;
    new_quest.vnums[2]  = 0;
    new_quest.vnums[3]  = 0;

    load_durgaard_key();

    quest_to_char(ch, &new_quest);
  }
  else if (!str_cmp(argument, "complete"))
  {
    OBJ_DATA *obj;

    obj = create_object(get_obj_index(OBJ_VNUM_PORTAL), 0);
    obj->value[0] = ROOM_VNUM_DURGAARD;
    obj->value[3] = ch->in_room->vnum;
    object_decay(obj, 10);
    obj_to_room(obj, ch->in_room);

    do_say(questmaster, "Enter the portal to Durgaard brave warrior.");

    act("$p appears in front of $n.", ch, obj, NULL, TO_ROOM);
    act("$p appears in front of you.", ch, obj, NULL, TO_CHAR);
  }
}

void questspec_pk(CHAR_DATA *questmaster, CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter;

  if (!str_cmp(argument, "gain"))
  {
    QUEST_DATA new_quest;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *gch = NULL;
    bool found = FALSE;

    /* find a red target */
    pIter = AllocIterator(descriptor_list);
    while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
    {
      if (d->connected == CON_PLAYING)
      {
        if ((gch = d->character) == NULL) continue;
        if (IS_IMMORTAL(gch)) continue;
        if (fair_fight(ch, gch) && fair_fight(gch, ch)) found = TRUE;
      }
    }

    /* find a yellow target */
    if (!found)
    {
      pIter = AllocIterator(descriptor_list);
      while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
      {
        if (d->connected == CON_PLAYING)
        {
          if ((gch = d->character) == NULL) continue;
          if (IS_IMMORTAL(gch)) continue;
          if (fair_fight(ch, gch)) found = TRUE;
        }
      }
    }

    if (!found)
    {
      do_say(questmaster, "Sorry, I have no quest for you at this time.");
      return;
    }

    do_say(questmaster, "I charge you to find and destroy this player!");
    new_quest.type      = QT_PK;
    new_quest.expire    = current_time + (9 * 60);
    new_quest.giver     = questmaster->pIndexData->vnum;
    new_quest.vnums[0]  = gch->pcdata->playerid;
    new_quest.vnums[1]  = 0;
    new_quest.vnums[2]  = 0;
    new_quest.vnums[3]  = 0;

    quest_to_char(ch, &new_quest);
    return;
  }
  else if (!str_cmp(argument, "complete"))
  {
    int value = 200;

    give_token(questmaster, ch, value);
    do_say(questmaster, "Thanks for solving my quest, come back again if you want.");
    return;
  }
}
