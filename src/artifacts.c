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

bool  event_artifact_update         ( EVENT_DATA *event );
bool  event_artifact_generic        ( EVENT_DATA *event );
bool  event_artifact_dragonorb      ( EVENT_DATA *event );
bool  event_artifact_singingsword   ( EVENT_DATA *event );
bool  event_player_dragonorb        ( EVENT_DATA *event );
bool  event_artifact_earthring      ( EVENT_DATA *event );
bool  event_player_earthring        ( EVENT_DATA *event );
bool  event_artifact_dragonrod      ( EVENT_DATA *event );

/* our artifacts table */
LIST *artifact_table = NULL;

/* the table of artifact programs */
const struct arti_entry artifact_programs[] =
{
  { "generic",      event_artifact_generic,      EVENT_ARTIFACT_GENERIC,      60 * PULSE_PER_SECOND },
  { "dragonorb",    event_artifact_dragonorb,    EVENT_ARTIFACT_DRAGONORB,    10 * PULSE_PER_SECOND },
  { "singingsword", event_artifact_singingsword, EVENT_ARTIFACT_SINGINGSWORD,  5 * PULSE_PER_SECOND },
  { "earthring",    event_artifact_earthring,    EVENT_ARTIFACT_EARTHRING,    10 * PULSE_PER_SECOND },
  { "dragonrod",    event_artifact_dragonrod,    EVENT_ARTIFACT_DRAGONROD,    20 * PULSE_PER_SECOND },

  /* terminate */
  { "", NULL, 0, 0 }
};

void load_artifact_table()
{
  FILE *fp;
  char *word;

  log_string("Loading Artifacts");

  if ((fp = fopen("../txt/artifacts.txt", "r")) == NULL)
  {
    bug("Unable to open artifacts.txt", 0);
    abort();
  }

  word = fread_word(fp);
  while (str_cmp(word, END_MARKER))
  {
    struct arti_type *artifact = malloc(sizeof(struct arti_type));

    artifact->owner = str_dup(word);
    artifact->vnum = fread_number(fp);
    artifact->active = fread_number(fp);
    artifact->fun = fread_string(fp);

    AttachToList(artifact, artifact_table);

    word = fread_word(fp);
  }

  fclose(fp);
}

void save_artifact_table()
{
  struct arti_type *artifact;
  ITERATOR *pIter;
  FILE *fp;

  if ((fp = fopen("../txt/artifacts.txt", "w")) == NULL)
  {
    bug("Unable to write to artifacts.txt", 0);
    return;
  }

  pIter = AllocIterator(artifact_table);
  while ((artifact = (struct arti_type *) NextInList(pIter)) != NULL)
  {
    fprintf(fp, "%s %d %d %s~\n",  artifact->owner, artifact->vnum, artifact->active, artifact->fun);
  }

  fprintf(fp, "%s\n", END_MARKER);

  fclose(fp);
}

/* This should be called from the artifacts own update function,
 * and the *ch pointer should be the person carrying the item,
 * or NULL if not carried by anyone. Only call this if the item
 * is _not_ carried by anyone or the carrier is _active_, that is,
 * the carrier is not idling, afk or safe.
 */
void update_artifact_table(OBJ_DATA *obj, CHAR_DATA *ch, bool active)
{
  ITERATOR *pIter;
  struct arti_type *artifact;
  int vnum = (obj->pIndexData != NULL) ? obj->pIndexData->vnum : 0;

  if (!IS_SET(obj->quest, QUEST_ARTIFACT))
    return;

  pIter = AllocIterator(artifact_table);
  while ((artifact = (struct arti_type *) NextInList(pIter)) != NULL)
  {
    if (vnum == artifact->vnum)
    {
      if ((ch != NULL && str_cmp(artifact->owner, ch->name)) ||
          (ch == NULL && str_cmp(artifact->owner, "noone")))
      {
        free_string(artifact->owner);
        artifact->owner = str_dup((ch != NULL && !IS_NPC(ch)) ? ch->name : "noone");
      }

      if (active)
        artifact->active++;

      save_artifact_table();
      return;
    }
  }

  bug("update_artifact_table: item vnum %d not found in table.", vnum);
}

bool check_arti_ownership(CHAR_DATA *ch, OBJ_DATA *obj)
{
  struct arti_type *artifact;
  ITERATOR *pIter;

  pIter = AllocIterator(artifact_table);
  while ((artifact = (struct arti_type *) NextInList(pIter)) != NULL)
  {
    if (obj->pIndexData->vnum == artifact->vnum)
    {
      if (!str_cmp(artifact->owner, ch->name))
        return TRUE;
    }
  }

  return FALSE;
}

void do_artiwiz(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  struct arti_type *artifact;
  char cmd[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  ITERATOR *pIter, *pIter2;
  OBJ_DATA *obj;
  bool found = FALSE;
  int i;

  argument = one_argument(argument, cmd);
  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (cmd[0] == '\0')
  {
    send_to_char("Syntax: artiwiz [add|remove|list] [object] [program].\n\r", ch);
    return;
  }

  if (!str_cmp(cmd, "add"))
  {
    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
      do_artiwiz(ch, "");
      return;
    }

    if ((obj = get_obj_carry(ch, arg1)) == NULL)
    {
      send_to_char("You do not have that item.\n\r", ch);
      return;
    }
    if (!IS_SET(obj->quest, QUEST_ARTIFACT))
    {
      send_to_char("That is not an artifact.\n\r", ch);
      return;
    }

    for (i = 0; artifact_programs[i].name[0] != '\0'; i++)
    {
      if (!str_cmp(arg2, artifact_programs[i].name))
      {
        found = TRUE;
        break;
      }
    }

    if (!found)
    {
      send_to_char("Valid programs are:\n\n\r", ch);

      for (i = 0; artifact_programs[i].name[0] != '\0'; i++)
      {
        sprintf(arg1, " o %s\n\r", artifact_programs[i].name);
        send_to_char(arg1, ch);
      }

      return;
    }

    pIter = AllocIterator(artifact_table);
    while ((artifact = (struct arti_type *) NextInList(pIter)) != NULL)
    {
      if (artifact->vnum == obj->pIndexData->vnum)
      {
        send_to_char("Artifact already in table.\n\r", ch);
        return;
      }
    }

    artifact = malloc(sizeof(struct arti_type));
    artifact->owner = str_dup("noone");
    artifact->vnum = obj->pIndexData->vnum;
    artifact->active = 0;
    artifact->fun = str_dup(artifact_programs[i].name);

    AttachToList(artifact, artifact_table);

    save_artifact_table();

    event = alloc_event();
    event->type = artifact_programs[i].type;
    event->fun = artifact_programs[i].fun;
    add_event_object(event, obj, artifact_programs[i].delay);
  }
  else if (!str_cmp(cmd, "remove"))
  {
    int vnum;

    if (arg1[0] == '\0')
    {
      send_to_char("Remove what vnum?\n\r", ch);
      return;
    }

    vnum = atoi(arg1);

    pIter = AllocIterator(artifact_table);
    while ((artifact = (struct arti_type *) NextInList(pIter)) != NULL)
    {
      if (artifact->vnum == vnum)
      {
        DetachAtIterator(pIter);

        /* fix all loaded artifacts with this vnum */
        pIter2 = AllocIterator(object_list);
        while ((obj = (OBJ_DATA *) NextInList(pIter2)) != NULL)
        {
          if (obj->pIndexData->vnum == artifact->vnum)
          {
            for (i = 0; artifact_programs[i].name[0] != '\0'; i++)
            {
              if (!str_cmp(artifact->fun, artifact_programs[i].name))
              {
                strip_event_object(obj, artifact_programs[i].type);
                strip_event_object(obj, EVENT_ARTIFACT_UPDATE);
              }
            }
          }
        }

        free_string(artifact->owner);
        free_string(artifact->fun);
        free(artifact);

        save_artifact_table();

        found = TRUE;
        break;
      }
    }

    if (!found)
    {
      send_to_char("There is no artifact by that vnum.\n\r", ch);
      return;
    }
  }
  else if (!str_cmp(cmd, "list"))
  {
    pIter = AllocIterator(artifact_table);
    while ((artifact = (struct arti_type *) NextInList(pIter)) != NULL)
    {
      sprintf(buf, " %3d  %-12s  %s\n\r", artifact->vnum, artifact->owner, artifact->fun);
      send_to_char(buf, ch);
    }
    return;
  }
  else
  {
    do_artiwiz(ch, "");
    return;
  }

  send_to_char("Ok.\n\r", ch);
}

void do_artifact(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  ITERATOR *pIter;
  OBJ_DATA *in_obj;
  bool found = FALSE;

  one_argument(argument, arg);
  if (!str_cmp(arg, "all"))
  {
    struct arti_type *artifact;

    pIter = AllocIterator(artifact_table);
    while ((artifact = (struct arti_type *) NextInList(pIter)) != NULL)
    {
      int active;
      OBJ_INDEX_DATA *pIndexData = get_obj_index(artifact->vnum);

      if (pIndexData == NULL)
      {
        bug("Unknown artifact in artifact_table : vnum %d", artifact->vnum);
        continue;
      }

      active = 100 * artifact->active / (muddata.mudinfo[MUDINFO_UPDATED] * 3 + 1);

      found = TRUE;
      sprintf(buf, "[%3d%%] %s is owned by %s.\n\r",
        active, pIndexData->short_descr, artifact->owner);
      send_to_char(buf, ch);
    }

    if (!found)
      send_to_char("There are no artifacts in the game. Type 'artifact all'.\n\r", ch);

    return;
  }

  pIter = AllocIterator(object_list);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (!IS_SET(obj->quest, QUEST_ARTIFACT))
      continue;

    found = TRUE;

    for (in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj)
      ;

    if (in_obj->carried_by != NULL)
      sprintf(buf, "%s carried by %s.\n\r", obj->short_descr, PERS(in_obj->carried_by, ch));
    else
      sprintf(buf, "%s in %s.\n\r", obj->short_descr, in_obj->in_room == NULL ? "somewhere" : in_obj->in_room->name);

    buf[0] = UPPER(buf[0]);
    send_to_char(buf, ch);
  }

  if (!found)
    send_to_char("There are no artifacts in the game.\n\r", ch);
}

bool event_artifact_update(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  OBJ_DATA *obj, *obj2;
  ITERATOR *pIter;
  bool active = FALSE;

  if ((obj = event->owner.obj) == NULL)
  {
    bug("event_artifact_update: no owner.", 0);
    return FALSE;                               
  }

  if ((ch = obj->carried_by) != NULL && !IS_NPC(ch))
  {
    if (ch->fight_timer <= 0 &&
       ((getMight(ch) < RANK_CADET)
     || (IS_SET(ch->extra, EXTRA_AFK) && ch->desc &&
       !(ch->desc->connected >= CON_NOTE_TO && ch->desc->connected <= CON_NOTE_FINISH))
     || (ch->level < 3)
     || !IS_SET(ch->extra, EXTRA_PKREADY)
     || (ch->pcdata->sit_safe >= 100)))
    {
      if (--obj->love <= -10)
      {
        CHAR_DATA *gch;

        switch(number_range(1, 3))
        {
          default:
            obj_say(obj, "This is getting boring, I'm out of here.", NULL);
            break;
          case 2:
            obj_say(obj, "Time to explore the world.", NULL);
            break;
          case 3:
            obj_say(obj, "I'll just go buy some cigarettes, be right back.", NULL);
            break;
        }
        act("$p vanishes in a puff of smoke.", ch, obj, NULL, TO_ALL);
        obj_from_char(obj);
        obj_to_room(obj, get_rand_room());

        if ((gch = (CHAR_DATA *) FirstInList(obj->in_room->people)) != NULL)
          act("$p appears in a puff of smoke.", gch, obj, NULL, TO_ALL);
      }
    }
    else
    {
      if (obj->love < 2)
        obj->love++;

      active = TRUE;
    }

    /* it should add the TEMP_ARTIFACT flag */
    if (!IS_NPC(ch))
      SET_BIT(ch->pcdata->tempflag, TEMP_ARTIFACT);
  }
  else if (obj->in_room != NULL && number_percent() <= 20)
  {
    if ((ch = (CHAR_DATA *) FirstInList(obj->in_room->people)) != NULL)
      act("$p vanishes in a puff of smoke.", ch, obj, NULL, TO_ALL);

    obj_from_room(obj);
    obj_to_room(obj, get_rand_room());

    if ((ch = (CHAR_DATA *) FirstInList(obj->in_room->people)) != NULL)
      act("$p appears in a puff of smoke.", ch, obj, NULL, TO_ALL);

    active = TRUE;
  }

  /* still being carried? */
  if ((ch = obj->carried_by) != NULL)
  {
    pIter = AllocIterator(ch->carrying);
    while ((obj2 = (OBJ_DATA *) NextInList(pIter)) != NULL)
    {
      if (obj == obj2) continue;

      if (IS_SET(obj2->quest, QUEST_ARTIFACT))
      {
        switch(number_range(1, 3))
        {
          default:
            obj_say(obj, "I'd rather be somewhere else, I don't like you!", NULL);
            break;
          case 2:
            obj_say(obj, "You don't love me anymore!", "exclaims");
            break;
          case 3:
            obj_say(obj, "I'm not enough for you? Bah!", "exclaims");
            break;
        }
        act("$p vanishes in a puff of smoke.", ch, obj, NULL, TO_ALL);
        obj_from_char(obj);
        obj_to_room(obj, get_rand_room());  

        if ((ch = (CHAR_DATA *) FirstInList(obj->in_room->people)) != NULL)
          act("$p appears in a puff of smoke.", ch, obj, NULL, TO_ALL);

        break;
      }
    }
  }

  /* update the counter */
  update_artifact_table(obj, obj->carried_by, active);

  /* requeue update */
  event = alloc_event();
  event->fun = &event_artifact_update;
  event->type = EVENT_ARTIFACT_UPDATE;
  add_event_object(event, obj, 10 * PULSE_PER_SECOND);

  return FALSE;
}

/* here follows the artifact programs */

bool event_artifact_dragonrod(EVENT_DATA *event)
{
  CHAR_DATA *ch, *gch;
  OBJ_DATA *obj;
  ITERATOR *pIter;
  bool found = FALSE;
  char buf[MAX_STRING_LENGTH];
  int i;

  if ((obj = event->owner.obj) == NULL)
  {
    bug("event_artifact_dragonrod: no owner.", 0);
    return FALSE;
  }

  /* if the update event isn't set, set it */
  if (!event_isset_object(obj, EVENT_ARTIFACT_UPDATE))
  {
    EVENT_DATA *newevent;

    newevent = alloc_event();
    newevent->fun = &event_artifact_update;
    newevent->type = EVENT_ARTIFACT_UPDATE;
    add_event_object(newevent, obj, 10 * PULSE_PER_SECOND);
  }

  if ((ch = obj->carried_by) != NULL && !IS_NPC(ch))
  {
    if (ch->fighting && obj->wear_loc != WEAR_NONE)
    {
      act("$p breaths a cone of fire, engulfing the room in flames.", ch, obj, NULL, TO_ALL);

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
    }
    else if (obj->wear_loc != WEAR_NONE)
    {
      switch(number_range(1, 10))
      {
        default:
          act("$p licks its lips.", ch, obj, NULL, TO_ALL);
          break;
        case 1:
        case 2:
          obj_say(obj, "Let's find something to kill, I'm sooo hungry.", "purrs");
          break;
        case 3:
        case 4:
          pIter = AllocIterator(ch->in_room->people);
          while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
          {
            if (ch == gch || !can_see(ch, gch)) continue;

            sprintf(buf, "Why don't you attack %s, I think he would taste just fine.", PERS(gch, ch));
            obj_say(obj, buf, "purrs");
            break;
          }
          break;
        case 5:
        case 6:
          obj_say(obj, "GrroooOOoWl!! I need fresh flesh.", "growls");
          break;
        case 7:
        case 8:
          pIter = AllocIterator(ch->in_room->people);
          while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
          {
            if (ch == gch || !can_see(ch, gch)) continue;

            sprintf(buf, "Let's barbecue %s.", PERS(gch, ch));
            obj_say(obj, buf, "purrs");
            break;
          }
          break;
      }
    }
  }

  /* Find the programs type, and make sure to requeue it
   * once more, or report a BUG if the type is unknown.
   */
  for (i = 0; artifact_programs[i].name[0] != '\0'; i++)
  {
    if (artifact_programs[i].type == event->type)
    {
      found = TRUE;
      break;
    }
  }
  if (found == TRUE)
  {
    event = alloc_event();
    event->type = EVENT_ARTIFACT_DRAGONROD;
    event->fun = &event_artifact_dragonrod;
    add_event_object(event, obj, artifact_programs[i].delay);
  }
  else
  {
    bug("artifact event type %d not found in table.", event->type);
  }

  return FALSE;
}

bool event_artifact_generic(EVENT_DATA *event)
{
  OBJ_DATA *obj;
  bool found = FALSE;
  int i;

  if ((obj = event->owner.obj) == NULL)
  {
    bug("event_artifact_generic: no owner.", 0);
    return FALSE;
  }

  /* if the update event isn't set, set it */
  if (!event_isset_object(obj, EVENT_ARTIFACT_UPDATE))
  {
    EVENT_DATA *newevent;

    newevent = alloc_event();
    newevent->fun = &event_artifact_update;
    newevent->type = EVENT_ARTIFACT_UPDATE;
    add_event_object(newevent, obj, 10 * PULSE_PER_SECOND);
  }

  /* Find the programs type, and make sure to requeue it
   * once more, or report a BUG if the type is unknown.
   */
  for (i = 0; artifact_programs[i].name[0] != '\0'; i++)
  {
    if (artifact_programs[i].type == event->type)
    {
      found = TRUE;
      break;
    }
  }
  if (found == TRUE)
  {
    event = alloc_event();
    event->type = EVENT_ARTIFACT_GENERIC;
    event->fun = &event_artifact_generic;
    add_event_object(event, obj, artifact_programs[i].delay);
  }
  else
  {
    bug("artifact event type %d not found in table.", event->type);
  }

  return FALSE;
}

bool event_player_dragonorb(EVENT_DATA *event)
{
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_artifact: no owner", 0);
    return FALSE;
  }

  if (!IS_NPC(ch) && IS_SET(ch->pcdata->tempflag, TEMP_DRAGONORB))
  {
    send_to_char("You feel the energy of the dragon orb leaving your body.\n\r", ch);
    REMOVE_BIT(ch->pcdata->tempflag, TEMP_DRAGONORB);
  }

  return FALSE;
}

bool event_artifact_earthring(EVENT_DATA *event)
{
  OBJ_DATA *obj;
  CHAR_DATA *ch;
  bool found = FALSE;
  int i;

  if ((obj = event->owner.obj) == NULL)
  {
    bug("event_artifact_earthring: no owner.", 0);
    return FALSE;
  }

  /* if the update event isn't set, set it */
  if (!event_isset_object(obj, EVENT_ARTIFACT_UPDATE))
  {
    EVENT_DATA *newevent;

    newevent = alloc_event();
    newevent->fun = &event_artifact_update;
    newevent->type = EVENT_ARTIFACT_UPDATE;
    add_event_object(newevent, obj, 10 * PULSE_PER_SECOND);
  }

  if ((ch = obj->carried_by) != NULL && ch->fighting != NULL && obj->wear_loc != WEAR_NONE)
  {
    /* if the wearer has less than 25% health, the ring might help */
    if (ch->hit < ch->max_hit / 4 && number_percent() >= 75)
    {
      act("$p explodes in a shower of flames.", ch, obj, NULL, TO_ALL);

      event = alloc_event();
      event->fun = &event_player_earthring;
      event->type = EVENT_PLAYER_EARTHRING;
      event->argument = str_dup("6");
      add_event_char(event, ch, PULSE_PER_SECOND);

      /* remove the ownership */
      update_artifact_table(obj, NULL, FALSE);
   
      /* extract the artifact */
      extract_obj(obj);
      
      /* return TRUE because we extracted the artifact */
      return TRUE;
    }
  }

  /* Find the programs type, and make sure to requeue it
   * once more, or report a BUG if the type is unknown.
   */
  for (i = 0; artifact_programs[i].name[0] != '\0'; i++)
  {
    if (artifact_programs[i].type == event->type)
    {
      found = TRUE;
      break;
    }
  }
  if (found == TRUE)
  {
    event = alloc_event();
    event->type = EVENT_ARTIFACT_EARTHRING;
    event->fun = &event_artifact_earthring;
    add_event_object(event, obj, artifact_programs[i].delay);
  }
  else
  {
    bug("artifact event type %d not found in table.", event->type);
  }

  return FALSE;
}

bool event_player_earthring(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  int counts = (event->argument) ? atoi(event->argument) : 0;
  int heal = number_range(100, 300);

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_earthring: no owner.", 0);
    return FALSE;
  }

  if (ch->hit <= 0)
    return FALSE;

  printf_to_char(ch, "The earthly elements restore %d hitpoints.\n\r", heal);
  ch->hit = UMIN(ch->max_hit, ch->hit + heal);

  if (counts-- > 0)
  {
    char buf[MAX_INPUT_LENGTH];

    sprintf(buf, "%d", counts);

    event = alloc_event();
    event->fun = &event_player_earthring;
    event->type = EVENT_PLAYER_EARTHRING;
    event->argument = str_dup(buf);
    add_event_char(event, ch, PULSE_PER_SECOND);
  }

  return FALSE;
}

bool event_artifact_singingsword(EVENT_DATA *event)
{
  CHAR_DATA *ch, *victim, *gch;
  ITERATOR *pIter;
  OBJ_DATA *obj;
  int i, count = 0;
  bool found = FALSE;
  static int song;
  static int line;

  if ((obj = event->owner.obj) == NULL)
  {
    bug("event_artifact_singingsword: no owner.", 0);
    return FALSE;
  }

  if ((ch = obj->carried_by) != NULL && (victim = ch->fighting) != NULL && obj->wear_loc != WEAR_NONE)
  {
    /* dirty trick : each song must have the same amount of lines,
     * but we will termiate the song when we reach a NUL'ed line, so
     * just fill out the songs with empty lines
     */
    char * song_table[MAX_SING_SONG][MAX_SING_LINE] =
    {
      { "Blowzabelle my bouncing Doxie",
        "Come let's trudge it to Kirkham Fair",
        "There's stout Liquor enough to Fox me"
      },

      { "And young Cullies to buy thy Ware",
        "Mind your Matters ye Sot without medling",
        "How I manage the sale of my Toys"
      },

      { "Get by Piping as I do by Pedling",
        "You need never want me for supplies",
        "God-a-mercy my Sweeting, I find thou think'st fitting"
      },

      { "To hint by this twitting, I owe thee a Crown",
        "Tho' for that I've been staying, a greater Debt's paying",
        "Your rate of delaying will never Compound"
      },

      { "I'll come home when my Pouch is full",
        "And soundly pay thee all old Arrears",
        "You'll forget it your Pate's so dull"
      },

      { "As by drowzy Neglect appears",
        "May the Drone of my Bag never hum",
        "If I fail to remember my Blowse"
      },

      { "May my Buttocks be ev'ry ones Drum",
        "If I think thou wilt pay me a Souse",
        "Squeakham, Squeakham, Bag-pipe will make 'em"
      },

      { "Whisking, Frisking, Money brings in",
        "Smoaking, Toping, Landlady groping",
        "Whores and Scores will spend it again"
      },

      { "By the best as I guess in the Town",
        "I swear thou shalt have e'ery Groat",
        "By the worst that a Woman e'er found"
      },

      { "If I have it will signify nought",
        "If good Nature works no better",
        ""
      },

      { "Blowzabella I'd have you to know",
        "Though you fancy my Stock is so low", 
        ""
      },

      { "I've more Rhino than always I show",
        "For some good Reasons of State that I know",
        "Since your Cheating I always knew"
      },

      { "For my Ware I got something too",
        "I've more Sence than to tell to you",
        "Singly then let's imploy Wit"
      },

      { "I'll use Pipe as my gain does hit",
        "And If I a new Chapman get",
        "You'll be easy too"
      },  

      { "Easy as any worn out Shoo",
        "Free and Frolick we'll Couple Gratis",
        "Thus we'll show all the Human Race"
      },

      { "That the best of the Marriage State is",
        "Blowzabella's and Collin's Case",
        ""
      }
    };
    bool endofsong = FALSE;

    /* premature end of song */
    if (song_table[song][line][0] == '\0')
    {
      song = (song + 1) % MAX_SING_SONG;
      line = 0;
      endofsong = TRUE;
    }

    obj_say(obj, song_table[song][line], "sings");

    /* update pointer */
    line = (line + 1) % MAX_SING_LINE;
    if (line == 0)
    {
      song = (song + 1) % MAX_SING_SONG;
      endofsong = TRUE;
    }

    /* cast a spell at end of song */
    if (endofsong)
    {
      int sn;

      switch(song)
      {
        default:
          break;
        case 0:
          act("$p flashes bright #Rred#n for a second.", ch, obj, NULL, TO_ALL);
          if ((sn = skill_lookup("dispel magic")) > 0)
            (*skill_table[sn].spell_fun) (sn, 50, ch, victim);
          else
            bug("event_artifact_singingsword: bad spell.", 0);
          break;
        case 1:
          act("$p flashes bright #Rred#n for a second.", ch, obj, NULL, TO_ALL);
          if ((sn = skill_lookup("curse")) > 0)
            (*skill_table[sn].spell_fun) (sn, 50, ch, victim);
          else
            bug("event_artifact_singingsword: bad spell.", 0);
          break;
        case 2:
          act("$p flashes bright #Rred#n for a second.", ch, obj, NULL, TO_ALL);
          if ((sn = skill_lookup("poison")) > 0)
            (*skill_table[sn].spell_fun) (sn, 50, ch, victim);
          else
            bug("event_artifact_singingsword: bad spell.", 0);
          break;
        case 3:
          act("$p flashes bright #Rred#n for a second.", ch, obj, NULL, TO_ALL);
          if ((sn = skill_lookup("magic missile")) > 0)
            (*skill_table[sn].spell_fun) (sn, 50, ch, victim);
          else
            bug("event_artifact_singingsword: bad spell.", 0);
          break;
        case 4:
          if (event_isset_mobile(victim, EVENT_MOBILE_CASTING))
          {
            act("$p flashes bright #Rred#n for a second.", ch, obj, NULL, TO_ALL);
            if ((sn = skill_lookup("counter spell")) > 0)
              (*skill_table[sn].spell_fun) (sn, 50, ch, victim);
            else
              bug("event_artifact_singingsword: bad spell.", 0);
          }
          break;
        case 5:
          act("$p flashes bright #Lblue#n for a second.", ch, obj, NULL, TO_ALL);
          if ((sn = skill_lookup("darkblessing")) > 0)
            (*skill_table[sn].spell_fun) (sn, 50, ch, ch);
          else
            bug("event_artifact_singingsword: bad spell.", 0);
          break;
        case 6:
          act("$p flashes bright #Lblue#n for a second.", ch, obj, NULL, TO_ALL);
          if ((sn = skill_lookup("bless")) > 0)
            (*skill_table[sn].spell_fun) (sn, 50, ch, ch);
          else
            bug("event_artifact_singingsword: bad spell.", 0);
          break;
        case 7:
          act("$p flashes bright #Lblue#n for a second.", ch, obj, NULL, TO_ALL);
          if ((sn = skill_lookup("frenzy")) > 0)
            (*skill_table[sn].spell_fun) (sn, 50, ch, ch);    
          else
            bug("event_artifact_singingsword: bad spell.", 0);
          break;
        case 8:
          send_to_char("You feel revived.\n\r", ch);
          if (ch->hit < ch->max_hit)
            ch->hit = UMIN(ch->max_hit, ch->hit + number_range(500, 1000));
          break;
        case 9:
          send_to_char("You feel revived.\n\r", ch);
          if (ch->move < ch->max_move)
            ch->move = UMIN(ch->max_move, ch->move + number_range(500, 1000));
          break;
        case 10:
          send_to_char("You feel revived.\n\r", ch);
          if (ch->mana < ch->max_mana)
            ch->mana = UMIN(ch->max_mana, ch->mana + number_range(500, 1000));
          break;
        case 11:
          act("$n becomes mad with bloodlust.", victim, NULL, NULL, TO_ROOM);
          send_to_char("You can no longer control your bloodlust.\n\r", victim);
          act("You go BERSERK!", victim, NULL, NULL, TO_CHAR);
          act("$n goes BERSERK!", victim, NULL, NULL, TO_ROOM);
          sound_to_room("berserk.wav", ch);

          pIter = AllocIterator(ch->in_room->people);
          while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
          {
            if (victim->dead || victim->in_room == NULL)
              break;

            if (victim == gch || !can_see(victim, gch) || is_safe(victim, gch) || count++ >= 4)
              continue;

            multi_hit(victim, gch, 4);
          }
          WAIT_STATE(victim, 24);
          break;
        case 12:
          if (!saves_spell(100, ch))
          {
            act("$n becomes mad with bloodlust.", ch, NULL, NULL, TO_ROOM);
            send_to_char("You can no longer control your bloodlust.\n\r", ch);
            act("You go BERSERK!", ch, NULL, NULL, TO_CHAR);
            act("$n goes BERSERK!", ch, NULL, NULL, TO_ROOM);
            sound_to_room("berserk.wav", ch);

            pIter = AllocIterator(ch->in_room->people);
            while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
            {
              if (ch->dead || ch->in_room == NULL)
                break;

              if (ch == gch || !can_see(ch, gch) || is_safe(ch, gch) || count++ >= 4)
                continue;

              multi_hit(ch, gch, 4);
            }
            WAIT_STATE(ch, 24);
          }
          else
            send_to_char("You resist Kroll's berserker rage.\n\r", ch);
          break;
        case 13:
          act("$p makes an eerie anguished scream.", ch, obj, NULL, TO_ALL);
          do_flee(victim, "");
          break;
        case 14:
          act("$p makes an eerie anguished scream.", ch, obj, NULL, TO_ALL);
          if (!saves_spell(100, ch))
            do_flee(ch, "");
          break;
        case MAX_SING_SONG - 1:
          act("$p belches out a ball of fire.", ch, obj, NULL, TO_ALL);
          one_hit(ch, victim, gsn_firebreath, 1);
          break;
      }
    }
  }

  /* if the update event isn't set, set it */
  if (!event_isset_object(obj, EVENT_ARTIFACT_UPDATE))
  {
    EVENT_DATA *newevent;

    newevent = alloc_event();
    newevent->fun = &event_artifact_update;
    newevent->type = EVENT_ARTIFACT_UPDATE;
    add_event_object(newevent, obj, 10 * PULSE_PER_SECOND);
  }

  /* Find the programs type, and make sure to requeue it
   * once more, or report a BUG if the type is unknown.
   */
  for (i = 0; artifact_programs[i].name[0] != '\0'; i++)
  {
    if (artifact_programs[i].type == event->type)
    {
      found = TRUE;
      break;
    }
  }
  if (found == TRUE)
  {
    event = alloc_event();
    event->type = EVENT_ARTIFACT_SINGINGSWORD;
    event->fun = &event_artifact_singingsword;
    add_event_object(event, obj, artifact_programs[i].delay);
  }
  else
  {
    bug("artifact event type %d not found in table.", event->type);
  }

  return FALSE;
}

bool event_artifact_dragonorb(EVENT_DATA *event)
{
  EVENT_DATA *newevent;
  CHAR_DATA *ch;
  OBJ_DATA *obj;
  int i;
  bool found = FALSE;

  if ((obj = event->owner.obj) == NULL)
  {
    bug("event_artifact_dragonorb: no owner.", 0);
    return FALSE;
  }

  if ((ch = obj->carried_by) != NULL && !IS_NPC(ch))
  {
    if (!IS_SET(ch->pcdata->tempflag, TEMP_DRAGONORB))
    {
      SET_BIT(ch->pcdata->tempflag, TEMP_DRAGONORB);

      act("$p flashes bright #Rred#n for a second.", ch, obj, NULL, TO_ALL);
      send_to_char("You feel energized.\n\r", ch);
    }
    if ((newevent = event_isset_mobile(ch, EVENT_PLAYER_DRAGONORB)) != NULL)
    {
      dequeue_event(newevent, TRUE);
    }

    /* enqueue wait event */
    newevent = alloc_event();
    newevent->fun = &event_player_dragonorb;
    newevent->type = EVENT_PLAYER_DRAGONORB;
    add_event_char(newevent, ch, 60 * PULSE_PER_SECOND);

    /* 1% chance it will turn into a dragon */
    if (ch->fighting != NULL && !IS_NPC(ch->fighting) && number_percent() == 50)
    {
      MOB_INDEX_DATA *pMobIndex;
      CHAR_DATA *gch;

      /* check to see if we have a mobile of this type */
      if ((pMobIndex = get_mob_index(MOB_VNUM_DRAGONORB)) == NULL)
      {
        bug("event_artifact_dragonorb: no dragon mobile.", 0);
        return FALSE;
      }

      /* create mobile */
      gch = create_mobile(pMobIndex);

      /* put the mobile in the room */
      char_to_room(gch, ch->in_room, TRUE);

      /* tell everyone about it */
      act("$p explodes in a shower of flames as $N bursts from it.", ch, obj, gch, TO_ALL);

      /* do a few quick attacks */
      one_hit(gch, ch, gsn_flamestorm, 0);
      one_hit(gch, ch, gsn_flamestorm, 0);

      /* remove the ownership */
      update_artifact_table(obj, NULL, FALSE);

      /* extract the artifact */
      extract_obj(obj);

      /* return TRUE because we extracted the artifact */
      return TRUE;
    }

    /* boost the owner with a bit of mana */
    if (ch->mana < ch->max_mana && number_percent() <= 20)
    {
      act("$p flashes bright #Lblue#n for a second.", ch, obj, NULL, TO_ROOM);
      send_to_char("You feel refreshed.\n\r", ch);
      ch->mana = UMIN(ch->max_mana, ch->mana + number_range(600, 1200));
    }

    /* socials */
    if (number_percent() <= 20)
    {
      switch(number_range(1, 8))
      {
        default:
          break;
        case 1:
          act("$p winks suggestively at $n.", ch, obj, NULL, TO_ROOM);
          act("$p winks suggestively at you.", ch, obj, NULL, TO_CHAR);
          break;
        case 2:
          act("$p bounces up and down, ready for some action.", ch, obj, NULL, TO_ALL);
          break;
        case 3:
          act("$p tries to conceal a small burp.", ch, obj, NULL, TO_ALL);
          break;
        case 4:
          act("$p chuckles at something $n said earlier.", ch, obj, NULL, TO_ROOM);
          act("$p chuckles at something you said earlier.", ch, obj, NULL, TO_CHAR);
          break;
        case 5:
          act("$p beams a smile at $n.", ch, obj, NULL, TO_ROOM);
          act("$p beams a smile at you.", ch, obj, NULL, TO_CHAR);
          break;
        case 6:
          act("$p cuddles up close to $n.", ch, obj, NULL, TO_ROOM);
          act("$p cuddles up close to you.", ch, obj, NULL, TO_CHAR);
          break;
        case 7:
          act("$p yawns, it must be tired.", ch, obj, NULL, TO_ALL);
          break;
        case 8:
          if (ch->hit < ch->max_hit / 2)
          {
            act("$p whimpers in fright - it seems concerned for $n.", ch, obj, NULL, TO_ROOM);
            act("$p whimpers in fright - it seems concerned for you.", ch, obj, NULL, TO_CHAR);
          }
          break;
      }
    }
  }

  /* if the update event isn't set, set it */
  if (!event_isset_object(obj, EVENT_ARTIFACT_UPDATE))
  {
    newevent = alloc_event();
    newevent->fun = &event_artifact_update;
    newevent->type = EVENT_ARTIFACT_UPDATE;
    add_event_object(newevent, obj, 10 * PULSE_PER_SECOND);
  }


  /* Find the programs type, and make sure to requeue it
   * once more, or report a BUG if the type is unknown.
   */
  for (i = 0; artifact_programs[i].name[0] != '\0'; i++)
  {
    if (artifact_programs[i].type == event->type)
    {
      found = TRUE;
      break;
    }
  }
  if (found == TRUE)
  {
    event = alloc_event();
    event->type = EVENT_ARTIFACT_DRAGONORB;
    event->fun = &event_artifact_dragonorb;
    add_event_object(event, obj, artifact_programs[i].delay);
  }
  else
  {
    bug("artifact event type %d not found in table.", event->type);
  }

  return FALSE;
}
