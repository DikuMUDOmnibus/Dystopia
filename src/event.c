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
#include <unistd.h>
#include <time.h>

#include "dystopia.h"

LIST       *eventqueue[MAX_EVENT_HASH];
STACK      *event_free = NULL;
LIST       *global_event_list = NULL;
int         current_bucket = 0;

/* local procedures */
void free_event                  ( EVENT_DATA *event );
bool event_game_arena_warning    ( EVENT_DATA *event );

bool event_game_articheck(EVENT_DATA *event)
{
  EVENT_DATA *newevent;
  ITERATOR *pIter, *pIter2;
  struct arti_type *artifact;

  pIter = AllocIterator(artifact_table);
  while ((artifact = (struct arti_type *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(artifact->owner, "noone"))
    {
      OBJ_DATA *obj;
      bool found = FALSE;

      pIter2 = AllocIterator(object_list);
      while ((obj = (OBJ_DATA *) NextInList(pIter2)) != NULL)
      {
        if (obj->pIndexData->vnum == artifact->vnum)
        {
          found = TRUE;
          break;
        }
      }

      if (!found)
      {
        OBJ_INDEX_DATA *pIndexData = get_obj_index(artifact->vnum);

        if (pIndexData)
        {
          obj = create_object(pIndexData, 50);

          obj_to_room(obj, get_rand_room());
        }
        else
        {
          bug("event_game_articheck: vnum %d illegal.", artifact->vnum);
        }
      }
    }
  }

  /* schedule another 30 second event */
  newevent        =  alloc_event();
  newevent->fun   =  &event_game_articheck;
  newevent->type  =  EVENT_GAME_ARTICHECK;
  add_event_world(newevent, (5 * 60 * PULSE_PER_SECOND));

  return FALSE;
}

bool event_game_pulse30(EVENT_DATA *event)
{
  EVENT_DATA *newevent;

  /* call all our 30 second update functions */
  update_mudinfo();
  update_polls();
  update_auctions();
  recycle_dummys();

  /* schedule another 30 second event */
  newevent        =  alloc_event();
  newevent->fun   =  &event_game_pulse30;
  newevent->type  =  EVENT_GAME_PULSE30;
  add_event_world(newevent, 30 * PULSE_PER_SECOND);

  /* we didn't dequeue */
  return FALSE;
}

/* This dummy function is used by events that
 * doesn't do anything besides making sure
 * only one such event can be in effect at any
 * given time. This callback function will simply
 * return FALSE, such that the event is freed.
 */
bool event_dummy(EVENT_DATA *event)
{
  CHAR_DATA *ch;

  switch(event->type)
  {
    default:
      break;
    case EVENT_PLAYER_GUSTWIND_WAIT:
      if ((ch = event->owner.ch) != NULL)
        send_to_char("You can use gust of wind again.\n\r", ch);
      break;
    case EVENT_MOBILE_BLURTENDRILS:
      if ((ch = event->owner.ch) != NULL)
        act("$n breaks free from the blurring tendrils.", ch, NULL, NULL, TO_ROOM);
      break;
    case EVENT_MOBILE_PSPRAY:
      if ((ch = event->owner.ch) != NULL)
        act("$n breaks free from the effect of the prismatic spray.", ch, NULL, NULL, TO_ROOM);
      break;
    case EVENT_MOBILE_CANTRIP_SPIKES:
      if ((ch = event->owner.ch) != NULL)
      {
        send_to_char("The spikes retract back into your body.\n\r", ch);
        act("The spikes covering $n retracts back into $s body.", ch, NULL, NULL, TO_ROOM);
      }
      break;
    case EVENT_PLAYER_BLOODTASTE:
      if ((ch = event->owner.ch) != NULL)
        printf_to_char(ch, "You have lost the taste of %s's blood.\n\r", event->argument);
      break;
  }

  return FALSE;
}

void event_look(CHAR_DATA *ch)
{
  EVENT_DATA *event;
  ITERATOR *pIter;
  ROOM_INDEX_DATA *pRoom;

  if ((pRoom = ch->in_room) == NULL)
    return;

  pIter = AllocIterator(pRoom->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    switch(event->type)
    {
      default:
        break;
      case EVENT_ROOM_PENTAGRAM:
        send_to_char("A pentagram is drawn on the floor.\n\r", ch);
        break;
      case EVENT_ROOM_GEYSER:
        send_to_char("A small geyser on the floor spews forth water.\n\r", ch);
        break;
      case EVENT_ROOM_VINES:
        send_to_char("Thorny vines fills the floor.\n\r", ch);
        break;
      case EVENT_ROOM_SHADOWVEIL:
        send_to_char("A shadowy fog envelopes this room.\n\r", ch);
        break;
      case EVENT_ROOM_TENDRILS:
        send_to_char("Dark tendrils are spewing from the ground.\n\r", ch);
        break;
      case EVENT_ROOM_WARDING:
        send_to_char("A glyph has been enscribed on the floor.\n\r", ch);
        break;
      case EVENT_ROOM_DOOMBOLT:
        send_to_char("A bolt of pure chaos flitters around the room.\n\r", ch);
        break;
      case EVENT_ROOM_PWALL:
        send_to_char("A sheath of shimmering energy covers the floor.\n\r", ch);
        break;
      case EVENT_ROOM_WATERDOME:
        send_to_char("A huge dome of water protects this room.\n\r", ch);
        break;
      case EVENT_ROOM_CANTRIP:
        send_to_char(event->argument, ch);
        break;
    }
  }
}

bool event_game_arena(EVENT_DATA *event)
{
  EVENT_DATA *newevent;

  /* open the arena */
  open_arena();

  /* warning in 40 seconds */
  newevent        = alloc_event();
  newevent->fun   = &event_game_arena_warning;
  newevent->type  = EVENT_GAME_ARENA_WARNING;
  add_event_world(newevent, 40 * PULSE_PER_SECOND);

  /* closes in 45 seconds */
  newevent        = alloc_event();
  newevent->fun   = &event_game_arena_close;
  newevent->type  = EVENT_GAME_ARENA_CLOSE;
  add_event_world(newevent, 45 * PULSE_PER_SECOND);

  /* the arena system requeued */
  newevent        =  alloc_event();
  newevent->fun   =  &event_game_arena;
  newevent->type  =  EVENT_GAME_ARENA;
  add_event_world(newevent, number_range(60, 90) * 60 * PULSE_PER_SECOND);

  /* we didn't dequeue it */
  return FALSE;
}

bool event_game_arena_warning(EVENT_DATA *event)
{
  do_info(NULL, "The Arena closes in 5 seconds, type #Carenajoin#n if you wish to enter");

  return FALSE;
}

bool event_game_arena_close(EVENT_DATA *event)
{
  if (IS_SET(arena.status, ARENA_FORTRESS_CONTEST))
    begin_team_arena();
  else
    close_arena();

  /* we didn't dequeue it */
  return FALSE;
}

bool event_game_ragnarok(EVENT_DATA *event)
{
  EVENT_DATA *newevent;

  ragnarok_stop();

  newevent        =  alloc_event();
  newevent->fun   =  &event_game_ragnarok_reset;
  newevent->type  =  EVENT_GAME_RAGNAROK_RESET;
  add_event_world(newevent, 120 * 60 * PULSE_PER_SECOND);

  /* we didn't dequeue */
  return FALSE;
}

bool event_game_ragnarok_reset(EVENT_DATA *event)
{
  new_ragnarok = TRUE;

  /* we didn't dequeue */
  return FALSE;
}

bool event_player_legend(EVENT_DATA *event)
{
  CHAR_DATA *ch, *calim;
  ROOM_INDEX_DATA *pRoom = NULL;
  ITERATOR *pIter;

  bool move_around = FALSE;
  bool failure = FALSE;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_legend: no owner.", 0);
    return FALSE;
  }

  pIter = AllocIterator(char_list);
  while ((calim = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!IS_NPC(calim)) continue;
    if (calim->pIndexData->vnum != MOB_VNUM_CALIM) continue;

    pRoom = calim->in_room;

    if (ch->in_room != calim->in_room)
    {
      act("$n vanishes in a puff of green smoke.", calim, NULL, NULL, TO_ROOM);
      char_from_room(calim);
      move_around = TRUE;
    }

    break;
  }

  if (calim == NULL)
  {
    pRoom = get_room_index(ROOM_VNUM_CITYCENTER);
    calim = create_mobile(get_mob_index(MOB_VNUM_CALIM));

    if (ch->in_room->vnum != ROOM_VNUM_CITYCENTER)
      move_around = TRUE;
    else
    {
      char_to_room(calim, ch->in_room, TRUE);
      act("$n appears in a puff of green smoke.", calim, NULL, NULL, TO_ROOM);
    }
  }

  if (move_around)
  {
    char_to_room(calim, ch->in_room, TRUE);
    act("$n appears in a puff of green smoke.", calim, NULL, NULL, TO_ROOM);
  }

  if (ch->practice < (ch->pcdata->legend + 1) * 1000)
  {
    do_say(calim, "You do not have enough primal energy, so piss off!");
    failure = TRUE;
  }
  if (getGold(ch) < (ch->pcdata->legend + 1) * 12500)
  {
    do_say(calim, "I don't see any gold, piss off beggar!");
    failure = TRUE;
  }

  if (!failure)
  {
    ch->pcdata->legend++;
    ch->practice -= ch->pcdata->legend * 1000;
    setGold(ch, -12500 * ch->pcdata->legend);

    do_say(calim, "I grant your request mortal.");
  }

  if (move_around)
  {
    act("$n melts down into the floor.", calim, NULL, NULL, TO_ROOM);
    char_from_room(calim);
    char_to_room(calim, pRoom, TRUE);
    act("$n reappears in a flash of light.", calim, NULL, NULL, TO_ROOM);
  }

  return FALSE;
}

bool event_mobile_extract(EVENT_DATA *event)
{
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_extract: no owner.", 0);
    return FALSE;
  }

  extract_char(ch, TRUE);

  /* mobile died */
  return TRUE;
}

bool event_player_message(EVENT_DATA *event)
{
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_message: no owner.", 0);
    return FALSE;
  }

  if (event->argument == NULL)
  {
    bug("event_player_message: no argument.", 0);
    return FALSE;
  }

  send_to_char(event->argument, ch);

  return FALSE;
}

void delay_message(char *message, CHAR_DATA *ch, int delay)
{
  EVENT_DATA *event;

  if (IS_NPC(ch))
    return;

  event = alloc_event();
  event->fun = &event_player_message;
  event->type = EVENT_PLAYER_MESSAGE;
  event->argument = str_dup(message);
  add_event_char(event, ch, delay);
}

bool event_object_decay(EVENT_DATA *event)
{
  OBJ_DATA *obj;
  CHAR_DATA *rch;
  char *message;

  if ((obj = event->owner.obj) == NULL)
  {
    bug("event_object_decay: no owner.", 0);
    return FALSE;
  }

  switch (obj->item_type)
  {
    default:
      message = "$p vanishes.";  
      break;
    case ITEM_FOUNTAIN:
      message = "$p dries up.";
      break;
    case ITEM_CORPSE_NPC:
      message = "$p decays into dust.";
      break;
    case ITEM_CORPSE_PC:
      message = "$p decays into dust.";
      break;
    case ITEM_FOOD:
      message = "$p decomposes.";
      break;
    case ITEM_TRASH:
      message = "$p crumbles into dust.";
      break;
    case ITEM_WEAPON:
      message = "$p turns to fine dust and blows away.";
      break;
    case ITEM_WALL:
      message = "$p flows back into the ground.";
      break;
  }
  if (obj->carried_by != NULL)
  {
    act(message, obj->carried_by, obj, NULL, TO_CHAR);
  }
  else if (obj->in_room != NULL)
  {
    if ((rch = (CHAR_DATA *) FirstInList(obj->in_room->people)) != NULL)
    {
      act(message, rch, obj, NULL, TO_ALL);
    }
  }

  /* all claimed objects as well as containers should be dumped out of the corpse */
  if (obj->item_type == ITEM_CORPSE_NPC)
  {
    ITERATOR *pIter;
    OBJ_DATA *t_obj;

    pIter = AllocIterator(obj->contains);
    while ((t_obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
    {
      if (t_obj->ownerid == 0 && t_obj->item_type != ITEM_CONTAINER)
        continue;

      obj_from_obj(t_obj);

      if (obj->in_obj)
        obj_to_obj(t_obj, obj->in_obj);
      else if (obj->carried_by)
        obj_to_char(t_obj, obj->carried_by);
      else if (obj->in_room == NULL)
        extract_obj(t_obj);
      else
        obj_to_room(t_obj, obj->in_room);
    }
  }

  /* get rid of the object */
  extract_obj(obj);

  /* the object was extracted, and thus also the event */
  return TRUE;
}

/* affects are counted down once every 20 seconds.
 */
bool event_object_affects(EVENT_DATA *event)
{
  AFFECT_DATA *paf;
  EVENT_DATA *newevent;
  CHAR_DATA *ch;
  OBJ_DATA *obj;
  ITERATOR *pIter;
  bool reschedule = FALSE;

  if ((obj = event->owner.obj) == NULL)
  {
    bug("event_object_affects: no owner.", 0);
    return FALSE;
  }

  pIter = AllocIterator(obj->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->duration > 0)
    {
      if (--paf->duration <= 0)
      {
        /* a dirty hack to remove special object spells,
         * since the type of spell on items is not used for
         * anything, we use it to store the special affect bit.
         */
        switch(paf->type)
        {
          default:
            break;
          case OAFF_FROSTBITE:
            REMOVE_BIT(obj->spellflags, OAFF_FROSTBITE);
            break;
          case OAFF_LIQUID:
            REMOVE_BIT(obj->spellflags, OAFF_LIQUID);
            break;
        }

        /* if worn, strip affects from player */
        if (obj->wear_loc != WEAR_NONE && (ch = obj->carried_by) != NULL)
        {
          affect_modify(ch, paf, FALSE);
        }

        DetachAtIterator(pIter);
        PushStack(paf, affect_free);
      }
    }
  }

  /* check once more, to see if we have any more */
  pIter = AllocIterator(obj->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->duration > 0)
      reschedule = TRUE;
  }

  if (reschedule)
  {
    newevent              =  alloc_event();
    newevent->fun         =  &event_object_affects;
    newevent->type        =  event->type;  /* since more than one type of affects can be updated through this */
    add_event_object(newevent, obj, 20 * PULSE_PER_SECOND);
  }

  return FALSE;
}

bool event_game_areasave(EVENT_DATA *event)
{
  EVENT_DATA *newevent;

  do_asave(NULL, "changed");

  newevent        =  alloc_event();
  newevent->fun   =  &event_game_areasave;
  newevent->type  =  EVENT_GAME_AREASAVE;
  add_event_world(newevent, 2 * 60 * 60 * PULSE_PER_SECOND);

  return FALSE;
}

bool event_game_crashsafe(EVENT_DATA *event)
{
  /* unlink temp file so the crash recovery system will work again */
  unlink(CRASH_TEMP_FILE);

  /* we didn't dequeue it */
  return FALSE;
}

bool event_game_weather(EVENT_DATA *event)
{
  EVENT_DATA *newevent;
  char buf[MAX_STRING_LENGTH] = { '\0' };
  ITERATOR *pIter;
  DESCRIPTOR_DATA *d;
  CHAR_DATA *ch = NULL;
  int diff;
  bool char_up;

  switch (++time_info.hour)
  {
    case 5:
      weather_info.sunlight = SUN_LIGHT;
      strcat(buf, "Another day has begun.\n\r");
      break;

    case 6:
      weather_info.sunlight = SUN_RISE;
      strcat(buf, "The black orb rises in the sky.\n\r");
      break;
   
    case 19:
      weather_info.sunlight = SUN_SET;
      strcat(buf, "The sun slowly disappears in the west.\n\r");
      break;

    case 20:
      weather_info.sunlight = SUN_DARK;
      strcat(buf, "The night has begun.\n\r");
      break;

    case 24:
      time_info.hour = 0;  
      time_info.day++;
   
      pIter = AllocIterator(descriptor_list);
      while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
      {
        char_up = FALSE;

        if (d->connected == CON_PLAYING && (ch = d->character) != NULL && !IS_NPC(ch))
        {
          send_to_char("You hear a clock in the distance strike midnight.\n\r", ch);
          if (ch->pcdata->followers > 0)
            ch->pcdata->followers = 0;
 
          if (IS_SET(ch->in_room->room_flags, ROOM_SILENCE))
          {
            act("The silence leaves the room.", ch, NULL, NULL, TO_ALL);
            REMOVE_BIT(ch->in_room->room_flags, ROOM_SILENCE);  
          }
  
          if (IS_SET(ch->in_room->room_flags, ROOM_FLAMING))
          {
            act("The flames in the room die down.", ch, NULL, NULL, TO_ALL);
            REMOVE_BIT(ch->in_room->room_flags, ROOM_FLAMING);
          }
        }
      }
      break;
  }
      
  if (time_info.day >= 35)
  {
    time_info.day = 0;
    time_info.month++;
  }

  if (time_info.month >= 17)
  {
    time_info.month = 0;
    time_info.year++;
  }
    
  /*
   * Weather change.  
   */
  if (time_info.month >= 9 && time_info.month <= 16)   
    diff = weather_info.mmhg > 985 ? -2 : 2;
  else
    diff = weather_info.mmhg > 1015 ? -2 : 2;

  weather_info.change += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
  weather_info.change = UMAX(weather_info.change, -12);
  weather_info.change = UMIN(weather_info.change, 12);

  weather_info.mmhg += weather_info.change;
  weather_info.mmhg = UMAX(weather_info.mmhg, 960);
  weather_info.mmhg = UMIN(weather_info.mmhg, 1040);

  switch (weather_info.sky)
  {
    default:
      bug("Weather_update: bad sky %d.", weather_info.sky);
      weather_info.sky = SKY_CLOUDLESS;
      break;

    case SKY_CLOUDLESS:
      if (weather_info.mmhg < 990 || (weather_info.mmhg < 1010 && number_bits(2) == 0))
      {
        strcat(buf, "The sky is getting cloudy.\n\r");
        weather_info.sky = SKY_CLOUDY;
      }
      break;

    case SKY_CLOUDY:
      if (weather_info.mmhg < 970 || (weather_info.mmhg < 990 && number_bits(2) == 0))
      {
        strcat(buf, "It starts to rain.\n\r");
        weather_info.sky = SKY_RAINING;
      }

      if (weather_info.mmhg > 1030 && number_bits(2) == 0)
      {
        strcat(buf, "The clouds disappear.\n\r");
        weather_info.sky = SKY_CLOUDLESS;
      }
      break;

    case SKY_RAINING: 
      if (weather_info.mmhg < 970 && number_bits(2) == 0)
      {
        strcat(buf, "Lightning flashes in the sky.\n\r");
        weather_info.sky = SKY_LIGHTNING;
      }  
  
      if (weather_info.mmhg > 1030 || (weather_info.mmhg > 1010 && number_bits(2) == 0))
      {
        strcat(buf, "The rain stopped.\n\r");
        weather_info.sky = SKY_CLOUDY;
      }
      break;

    case SKY_LIGHTNING:
      if (weather_info.mmhg > 1010 || (weather_info.mmhg > 990 && number_bits(2) == 0))
      {
        strcat(buf, "The lightning has stopped.\n\r");
        weather_info.sky = SKY_RAINING;
        break;
      }     
      break;
  }

  if (buf[0] != '\0')
  {
    pIter = AllocIterator(descriptor_list);
    while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
    {
      if (d->connected == CON_PLAYING && IS_OUTSIDE(d->character) && IS_AWAKE(d->character))
      {
        send_to_char(buf, d->character);
      }
    }
  }

  /* make a new weather update event */
  newevent        =  alloc_event();
  newevent->fun   =  &event_game_weather;
  newevent->type  =  EVENT_GAME_WEATHER;
  add_event_world(newevent, number_range(PULSE_TICK / 2, 3 * PULSE_TICK / 2));

  /* we didn't dequeue it */
  return FALSE;
}

bool event_socket_idle(EVENT_DATA *event)
{
  DESCRIPTOR_DATA *d;

  if ((d = event->owner.desc) == NULL)
  {
    bug("event_socket_idle: no owner.", 0);
    return FALSE;
  }

  write_to_buffer(d, "\n\n\rYour connection has timed out, please reconnect...\n\n\r", 0);

  /* and out you go */
  close_socket(d);

  /* we closed the socket, so it has alrady been unlinked */
  return TRUE;
}

/* Updates a mobile/player every 20-40 seconds, this
 * function is slightly performance sensative - do
 * not add to heavy calculations if it's not needed.
 */
bool event_char_update(EVENT_DATA *event)
{
  EVENT_DATA *newevent;
  AFFECT_DATA *paf, *paf_next;
  QUEST_DATA *quest;
  ITERATOR *pIter;
  CHAR_DATA *ch;
  char buf[MAX_STRING_LENGTH];

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_char_update: no owner.", 0);
    return FALSE;
  }

  /*
   * This part only affects PC's
   */
  if (!IS_NPC(ch))
  {
    /*
     * update active quests
     */
    pIter = AllocIterator(ch->pcdata->quests);
    while ((quest = (QUEST_DATA *) NextInList(pIter)) != NULL)
    {
      if (quest->expire > current_time)
        continue;

      quest_from_char(ch, quest);
      send_to_char("You have failed to complete a quest.\n\r", ch);
    }

    /* the time bonus goes up or down */
    if (!IS_SET(ch->extra, EXTRA_AFK))
      update_active_counters(ch);

    /* do a popup once every minute */
    if (ch->desc)
      char_popup(ch);
  }

  /*
   * updating spells on all mobs and players
   */
  pIter = AllocIterator(ch->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->duration > 0)
      paf->duration--;
    else if (paf->duration == 0)
    {
      /* bit of a dirty hack - but almost always spells of the same type
       * with the same duration will be stacked just after eachother...
       */
      paf_next = (AFFECT_DATA *) PeekNextInList(pIter);
      if (paf_next == NULL || paf_next->type != paf->type || paf_next->duration > 0)
      {
        if (paf->type > 0 && paf->type < MAX_SKILL && skill_table[paf->type].msg_off)
        {
          act(skill_table[paf->type].msg_off, ch, NULL, NULL, TO_CHAR);
          act(skill_table[paf->type].msg_off_others, ch, NULL, NULL, TO_ROOM);
        }
        else
        {
          sprintf(buf, "Update Bug : Bad affect on %s.", ch->name);
          bug(buf, 0);
        }
      }

      affect_remove(ch, paf);
    }
  }
      
  /* 
   * Updating current position / minor healing
   */
  if (ch->position <= POS_STUNNED)
  {
    ch->hit = ch->hit + number_range(2, 4);

    update_pos(ch);
  }

  /*
   * Dealing damage due to missing limbs, etc.
   */
  if (ch->loc_hp[6] > 0 && ch->in_room != NULL && ch->hit > 0)
  { 
    int dam = 0;
     
    if (IS_BLEEDING(ch, BLEEDING_HEAD))
    {
      act("A spray of blood shoots from the stump of $n's neck.", ch, NULL, NULL, TO_ROOM);
      send_to_char("A spray of blood shoots from the stump of your neck.\n\r", ch);
      dam += number_range(20, 50);
    }
    if (IS_BLEEDING(ch, BLEEDING_THROAT))
    {
      act("Blood pours from the slash in $n's throat.", ch, NULL, NULL, TO_ROOM);
      send_to_char("Blood pours from the slash in your throat.\n\r", ch);
      dam += number_range(10, 20);
    }
    if (IS_BLEEDING(ch, BLEEDING_ARM_L))
    {
      act("A spray of blood shoots from the stump of $n's left arm.", ch, NULL, NULL, TO_ROOM);
      send_to_char("A spray of blood shoots from the stump of your left arm.\n\r", ch);
      dam += number_range(10, 20);
    }
    else if (IS_BLEEDING(ch, BLEEDING_HAND_L))
    {
      act("A spray of blood shoots from the stump of $n's left wrist.", ch, NULL, NULL, TO_ROOM);
      send_to_char("A spray of blood shoots from the stump of your left wrist.\n\r", ch);
      dam += number_range(5, 10);
    }
    if (IS_BLEEDING(ch, BLEEDING_ARM_R))
    {
      act("A spray of blood shoots from the stump of $n's right arm.", ch, NULL, NULL, TO_ROOM);
      send_to_char("A spray of blood shoots from the stump of your right arm.\n\r", ch);
      dam += number_range(10, 20);
    }
    else if (IS_BLEEDING(ch, BLEEDING_HAND_R))
    {
      act("A spray of blood shoots from the stump of $n's right wrist.", ch, NULL, NULL, TO_ROOM);
      send_to_char("A spray of blood shoots from the stump of your right wrist.\n\r", ch);
      dam += number_range(5, 10);
    }
    if (IS_BLEEDING(ch, BLEEDING_LEG_L))
    {
      act("A spray of blood shoots from the stump of $n's left leg.", ch, NULL, NULL, TO_ROOM);
      send_to_char("A spray of blood shoots from the stump of your left leg.\n\r", ch);
      dam += number_range(10, 20);
    }
    else if (IS_BLEEDING(ch, BLEEDING_FOOT_L))
    {
      act("A spray of blood shoots from the stump of $n's left ankle.", ch, NULL, NULL, TO_ROOM);
      send_to_char("A spray of blood shoots from the stump of your left ankle.\n\r", ch);
      dam += number_range(5, 10);
    }
    if (IS_BLEEDING(ch, BLEEDING_LEG_R))
    {
      act("A spray of blood shoots from the stump of $n's right leg.", ch, NULL, NULL, TO_ROOM);
      send_to_char("A spray of blood shoots from the stump of your right leg.\n\r", ch);
      dam += number_range(10, 20);
    }
    else if (IS_BLEEDING(ch, BLEEDING_FOOT_R))
    {
      act("A spray of blood shoots from the stump of $n's right ankle.", ch, NULL, NULL, TO_ROOM);
      send_to_char("A spray of blood shoots from the stump of your right ankle.\n\r", ch);
      dam += number_range(5, 10);
    }

    ch->hit = UMAX(1, ch->hit - dam);
    update_pos(ch);

    ch->in_room->blood += dam;
    if (ch->in_room->blood > 1000)
      ch->in_room->blood = 1000; 
  }

  if (IS_AFFECTED(ch, AFF_FLAMING) && ch->hit > 0)
  {
    int dam;

    act("$n's flesh burns and crisps.", ch, NULL, NULL, TO_ROOM);
    send_to_char("Your flesh burns and crisps.\n\r", ch);
    dam = number_range(250, 300);

    ch->hit = UMAX(1, ch->hit - dam);
    update_pos(ch);
  }
      
  /* 
   * More damage stuff
   */
  if (IS_AFFECTED(ch, AFF_POISON) && ch->hit > 0)
  {  
    act("$n shivers and suffers.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You shiver and suffer.\n\r", ch);

    ch->hit = UMAX(1, ch->hit - number_range(100, 200));

    if (number_range(1, 4) == 1)
    {
      REMOVE_BIT(ch->affected_by, AFF_POISON);
      send_to_char("You feel the poison leave your system.\n\r", ch);
    }
  }

  /* new update event for this mobile */
  newevent              =  alloc_event();
  newevent->fun         =  &event_char_update;
  newevent->type        =  EVENT_CHAR_UPDATE;
  add_event_char(newevent, ch, number_range(20 * PULSE_PER_SECOND, 40 * PULSE_PER_SECOND));

  /* we didn't dequeue */
  return FALSE;
}

/* Update special programs for mobile - all programs are
 * updated every 3-5 seconds, so please don't add anything
 * heavy to this. (Performance sensitive).
 */
bool event_mobile_spec(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  EVENT_DATA *newevent;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_spec: no owner.", 0);
    return FALSE;
  }

  /* only mobiles */
  if (!IS_NPC(ch)) return FALSE;

  /* check for special program */
  if (ch->spec_fun != 0)
  {
    (*ch->spec_fun) (ch, "update");

    /* mobile died ? */
    if (ch->dead)
      return TRUE;
  }
  else
  {
    bug("event_mobile_spec: no special program.", 0);
    return FALSE;
  }

  /* new update event for this mobile */
  newevent              =  alloc_event();
  newevent->fun         =  &event_mobile_spec;
  newevent->type        =  EVENT_MOBILE_SPEC; 
  add_event_char(newevent, ch, number_range(3, 5) * PULSE_PER_SECOND);

  return FALSE;
}

/* Scavengers are updated every 5-8 seconds.
 * Do not add anything heavy to this function,
 * since it is performance sensitive.
 */
bool event_mobile_scavenge(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  EVENT_DATA *newevent;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_scavenge: no owner.", 0);
    return FALSE;
  }

  /* only mobiles */
  if (!IS_NPC(ch)) return FALSE;

  if (SizeOfList(ch->in_room->contents) > 0 && number_bits(2) == 0)
  {
    OBJ_DATA *obj;
    OBJ_DATA *obj_best = NULL;
    int max = 1;
    ITERATOR *pIter;

    pIter = AllocIterator(ch->in_room->contents);
    while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
    {
      if (!can_see_obj(ch, obj))
        continue;
      if (IS_SET(obj->quest, QUEST_ARTIFACT))
        continue;
      if (CAN_WEAR(obj, ITEM_TAKE) && obj->cost > max)
      {
        obj_best = obj;
        max = obj->cost;
      }
    }
    if (obj_best)
    {
      obj_from_room(obj_best);
      obj_to_char(obj_best, ch);
      act("$n picks $p up.", ch, obj_best, NULL, TO_ROOM);
    }
  }

  /* new update event for this mobile */
  newevent              =  alloc_event();
  newevent->fun         =  &event_mobile_scavenge;
  newevent->type        =  EVENT_MOBILE_SCAVENGE; 
  add_event_char(newevent, ch, number_range(5, 8) * PULSE_PER_SECOND);

  return FALSE;
}

/* Updates a mobile every 16-24 seconds, this function
 * is performance sensative - do not add to heavy
 * calculations to this if it's not needed. All mobiles
 * are updated through this function.
 */
bool event_mobile_heal(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  EVENT_DATA *newevent;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_heal: no owner.", 0);
    return FALSE;
  }

  /* only mobiles */
  if (!IS_NPC(ch)) return FALSE;

  /* mobiles heal 10% of their total hps every 16-24 seconds */
  if (ch->hit < ch->max_hit && IS_SET(ch->extra, EXTRA_HEAL))
  {
    REMOVE_BIT(ch->extra, EXTRA_HEAL);
    ch->hit = UMIN(ch->hit + ch->max_hit / 10, ch->max_hit);
  }

  if (IS_SET(ch->newbits, NEW_FUMES) && number_percent() >= 90)
  {
    REMOVE_BIT(ch->newbits, NEW_FUMES);
    act("$n moves clear of the noxious fumes.", ch, NULL, NULL, TO_ROOM);
  }

  if (ch->precognition != NULL && number_percent() >= 90)
  {
    ch->precognition = NULL;
    act("$n seems more sure of $mself.", ch, NULL, NULL, TO_ROOM);
  }

  /* mobiles heal 10% of their total hps every 16-24 seconds */
  if (ch->hit < ch->max_hit && ch->position == POS_STANDING)
  {
    ch->hit = UMIN(ch->hit + ch->max_hit / 10, ch->max_hit);
  }

  /* relax from fighting stance */
  if (ch->position == POS_STANDING && ch->stance[0] > 0)
  {
    ch->stance[0] = 0;
    act("$n relaxes from $s fighting stance.", ch, NULL, NULL, TO_ROOM);
  }

  /* new update event for this mobile */
  newevent              =  alloc_event();
  newevent->fun         =  &event_mobile_heal;
  newevent->type        =  EVENT_MOBILE_HEAL;
  add_event_char(newevent, ch, number_range(16, 24) * PULSE_PER_SECOND);

  return FALSE;
}

/* Move mobiles around - called every 4-6 seconds, and only
 * on mobiles that don't have the SENTINEL flag. This is
 * performance sensitive, so please don't add anything heavy.
 */
bool event_mobile_move(EVENT_DATA *event)
{
  CHAR_DATA *ch;
  EVENT_DATA *newevent;
  EXIT_DATA *pexit;
  int door;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_mobile_move: no owner.", 0);
    return FALSE;
  }

  if (!IS_NPC(ch)) return FALSE;

  /* mobiles heal 2.5% of their total hps every 4-6 seconds */
  if (ch->hit < ch->max_hit && ch->position == POS_STANDING)
  { 
    ch->hit = UMIN(ch->hit + 25 * ch->max_hit / 1000, ch->max_hit);
  }

  do
  {
    /* prone mobile should stand up in this pulse */
    if (ch->position != POS_STANDING)
    {
      do_stand(ch, "");
      break;
    }

    /* try to move this mobile */
    if ((door = number_bits(5)) <= 5
      && (pexit = ch->in_room->exit[door]) != NULL
      && pexit->to_room
      && !IS_SET(pexit->exit_info, EX_CLOSED)
      && !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
      && !(IS_SET(ch->act, ACT_FISH) && pexit->to_room->sector_type != SECT_WATER_SWIM && pexit->to_room->sector_type != SECT_WATER_NOSWIM)
      && !(IS_SET(ch->act, ACT_NOSWIM) && (pexit->to_room->sector_type == SECT_WATER_SWIM || pexit->to_room->sector_type == SECT_WATER_NOSWIM))
      && pexit->to_room->area == ch->in_room->area)
    {
      move_char(ch, door);
    }

  } while (FALSE);

  /* new update event for this mobile */
  newevent              =  alloc_event();
  newevent->fun         =  &event_mobile_move;
  newevent->type        =  EVENT_MOBILE_MOVE;
  add_event_char(newevent, ch, number_range(4, 6) * PULSE_PER_SECOND);

  /* we didn't dequeue the event */
  return FALSE;
}

bool event_area_reset(EVENT_DATA *event)
{
  AREA_DATA *pArea;
  EVENT_DATA *newevent;
  int newtime;

  if ((pArea = event->owner.area) == NULL)
  {
    bug("event_area_reset: no owner.", 0);
    return FALSE;
  }

  /* reset the area */
  reset_area(pArea);

  /* dirty hack to make the newbie zone repop faster */
  if (!str_cmp(pArea->name, "Jobo     The Newbie Zone"))
    newtime = number_range(30, 60) * PULSE_PER_SECOND;
  else
    newtime = number_range(180, 300) * PULSE_PER_SECOND;

  /* requeue in 3-5 minutes (180-300 seconds) */
  newevent              =  alloc_event();
  newevent->fun         =  &event_area_reset;
  newevent->type        =  EVENT_AREA_RESET;
  add_event_area(newevent, pArea, newtime);

  /* we didn't dequeue */
  return FALSE;
}

bool event_room_aggrocheck(EVENT_DATA *event)
{
  ROOM_INDEX_DATA *pRoom;
  CHAR_DATA *wch, *victim;
  ITERATOR *pIter, *pIter2;
  bool found = FALSE;

  if ((pRoom = event->owner.room) == NULL)
  {
    bug("event_room_aggrocheck: no owner.", 0);
    return FALSE;
  }

  /* safe room */
  if (IS_SET(pRoom->room_flags, ROOM_SAFE))
    return FALSE;

  pIter = AllocIterator(pRoom->people);
  while ((wch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    /* check for aggressive mobiles */
    if (!IS_NPC(wch) ||
       (!IS_SET(wch->act, ACT_AGGRESSIVE) && !IS_SET(wch->act, ACT_SEMIAGGRESSIVE))||
         wch->fighting != NULL ||
         IS_AFFECTED(wch, AFF_CHARM) ||
        !IS_AWAKE(wch))
    {
      continue;
    }

    found = FALSE;

    /* find a victim */
    pIter2 = AllocIterator(pRoom->people);
    while ((victim = (CHAR_DATA *) NextInList(pIter2)) != NULL)
    {
      if (IS_NPC(victim)) continue;
      if (IS_SET(wch->act, ACT_AGGRESSIVE) && !can_see(wch, victim)) continue;
      if (IS_SET(wch->act, ACT_SEMIAGGRESSIVE) && in_kingdom_hall(victim)) continue;

      /* 75% chance we attack */
      if (number_percent() < 25) continue;

      act("$n screams and attacks!", wch, NULL, victim, TO_VICT);
      multi_hit(wch, victim, 1);
      found = TRUE;
    }
  }

  /* we didn't dequeue */
  return FALSE;
}

bool event_room_dispel_magic(EVENT_DATA *event)
{
  ROOM_INDEX_DATA *pRoom;
  CHAR_DATA *victim;
  AFFECT_DATA *paf;
  ITERATOR *pIter, *pIter2;

  if ((pRoom = event->owner.room) == NULL)
  {
    bug("event_room_dispel_magic: no owner.", 0);
    return FALSE;
  }

  pIter = AllocIterator(pRoom->people);
  while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_NPC(victim))
      continue;

    /* 50% chance to hit your own trap */
    if (in_kingdom_hall(victim) && number_percent() > 50)
      continue;

    act("A whirling vortex of crackling blue energy strikes you.", victim, NULL, NULL, TO_CHAR);
    act("A whirling vortex of crackling blue energy strikes $n.", victim, NULL, NULL, TO_ROOM);

    pIter2 = AllocIterator(victim->affected);
    while ((paf = (AFFECT_DATA *) NextInList(pIter2)) != NULL)
    {
      if (!saves_spell(85, victim))
      {
        if (paf->type > 0 && paf->type < MAX_SKILL && skill_table[paf->type].msg_off)
        {
          act(skill_table[paf->type].msg_off, victim, NULL, NULL, TO_CHAR);
          act(skill_table[paf->type].msg_off_others, victim, NULL, NULL, TO_ROOM);
        }
        affect_remove(victim, paf);
      }
    }
  }

  /* enqueue new dispel magic event in this room */
  if (IS_SET(pRoom->room_flags, ROOM_DISPEL_MAGIC))
  {
    event              =  alloc_event();
    event->fun         =  &event_room_dispel_magic;
    event->type        =  EVENT_ROOM_DISPEL_MAGIC;
    add_event_room(event, pRoom, number_range(4, 8) * PULSE_PER_SECOND);
  }

  /* we didn't dequeue */
  return FALSE;
}

bool event_room_bladebarrier(EVENT_DATA *event)
{
  ROOM_INDEX_DATA *pRoom;
  CHAR_DATA *victim, *vfirst = NULL;
  ITERATOR *pIter;
  int dam;

  if ((pRoom = event->owner.room) == NULL)
  {
    bug("event_room_bladebarrier: no owner.", 0);
    return FALSE;
  }

  pIter = AllocIterator(pRoom->people);
  while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_NPC(victim) || victim->hit < 2)
      continue;

    if (vfirst == NULL)
      vfirst = victim;

    act("The scattered blades on the ground fly up into the air ripping into you.", victim, NULL, NULL, TO_CHAR);
    act("The scattered blades on the ground fly up into the air ripping into $n.", victim, NULL, NULL, TO_ROOM);

    dam = number_range(500, 2500);
    modify_hps(victim, -1 * dam);
    update_pos(victim);
  }

  if (vfirst)
  {
    act("The blades drop to the ground inert.", vfirst, NULL, NULL, TO_ALL);
  }

  /* enqueue new blade barrier event in this room */
  if (IS_SET(pRoom->room_flags, ROOM_BLADE_BARRIER))
  {
    event              =  alloc_event();
    event->fun         =  &event_room_bladebarrier;
    event->type        =  EVENT_ROOM_BLADEBARRIER;
    add_event_room(event, pRoom, number_range(7, 13) * PULSE_PER_SECOND);
  }

  /* we didn't dequeue */
  return FALSE;
}

bool event_player_arena_madness(EVENT_DATA *event)
{
  CHAR_DATA *ch, *gch;
  ITERATOR *pIter;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_arena_madness: no owner.", 0);
    return FALSE;
  }

  pIter = AllocIterator(ch->in_room->people);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (gch == ch) continue;
    if (IS_NPC(gch)) continue;
    if (!can_see(ch, gch)) continue;
    if (str_cmp(gch->name, event->argument)) continue;

    if (ch->position != POS_STANDING)
    {
      ch->position = POS_STANDING;
      send_to_char("Sensing an enemy, you jump to your feet.\n\r", ch);
      act("$n jumps to $s feet, ready for battle.", ch, NULL, NULL, TO_ROOM);
    }

    act("$n screams out in hatred and attacks you.", ch, NULL, gch, TO_VICT);
    act("You scream out in hatred and attack $N.", ch, NULL, gch, TO_CHAR);  

    one_hit(ch, gch, TYPE_UNDEFINED, 1);

    return FALSE;
  }

  return FALSE;
}

/* This event retriggers itself. Even though it may
 * seem smarter to only queue this event if needed,
 * it's not worth the time to add all the codechecks
 * where needed. There simply aren't enough players.
 */
bool event_player_heal(EVENT_DATA *event)
{
  FEED_DATA *feed;
  EVENT_DATA *newevent;
  ITERATOR *pIter, *pIter2;
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_heal: no owner.", 0);
    return FALSE;
  }
  
  if (IS_NPC(ch))
    return FALSE;

  if (ch->in_room == NULL)
  {
    bug("event_player_heal: player not in any room", 0);
    return FALSE;
  }

  /* automatic pkready after 24 hours */
  if (!IS_SET(ch->extra, EXTRA_PKREADY) && (get_age(ch) - 17) >= 12)
    SET_BIT(ch->extra, EXTRA_PKREADY);

  if (IS_CLASS(ch, CLASS_FAE))
    ch->form = ITEM_WEAR_FAE;
  else
    ch->form = ITEM_WEAR_ALL;

  /* update all feeding data on this player */
  pIter = AllocIterator(ch->pcdata->feeders);
  while ((feed = (FEED_DATA *) NextInList(pIter)) != NULL)
  {
    /* if the player is below 25% max hitpoint, and the feed_data
     * is recent, we no longer reduce it's time - this is done
     * to avoid perm-mort-waiting to remove all the feed_data's.
     */
    if (ch->hit < 25 * ch->max_hit / 100 && feed->time >= 4)
      continue;

    if (--feed->time <= 0)
      free_feed(ch, feed);
  }

  if (in_arena(ch) && ch->position != POS_FIGHTING && ch->position > POS_STUNNED)
  {
    CHAR_DATA *gch;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch) continue;
      if (IS_NPC(gch)) continue;
      if (!can_see(ch, gch)) continue;

      newevent = alloc_event();
      newevent->fun = &event_player_arena_madness;
      newevent->argument = str_dup(gch->name);
      newevent->type = EVENT_PLAYER_ARENA_MADNESS;
      add_event_char(newevent, ch, 3 * PULSE_PER_SECOND);

      break;
    }
  }

  /* kingdom alarm if needed */
  if (ch->in_room && ch->in_room->vnum >= ROOM_VNUM_KINGDOMHALLS)
  {
    KINGDOM_DATA *kingdom;

    pIter = AllocIterator(kingdom_list);
    while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
    {
      if (!IS_SET(kingdom->flags, KINGFLAG_ALARM))
        continue;

      if (ch->in_room->vnum >= kingdom->vnums && ch->in_room->vnum <= kingdom->vnums + 99)
      {
        if (get_kingdom(ch) != kingdom)
        {
          DESCRIPTOR_DATA *d;
          char buf[MAX_STRING_LENGTH];

          pIter2 = AllocIterator(descriptor_list);
          while ((d = (DESCRIPTOR_DATA *) NextInList(pIter2)) != NULL)
          {
            CHAR_DATA *gch = d->character;

            if (d->connected != CON_PLAYING || gch == NULL)
              continue;

            if (gch->pcdata->kingdom != kingdom->kingid)
              continue;

            if (event_isset_mobile(gch, EVENT_PLAYER_KALARM))
              continue;

            sprintf(buf, " #G>( #C%s is #Cinvading your kingdom #G)<#n\n\r", PERS(ch, gch));
            send_to_char(buf, gch);

            newevent = alloc_event();
            newevent->fun = &event_dummy;
            newevent->type = EVENT_PLAYER_KALARM;
            add_event_char(newevent, gch, 10 * PULSE_PER_SECOND);
          }
        }
      }
    }
  }

  do
  {
    if (ch->fight_timer > 0)
      ch->fight_timer--;

    /* check for autoquit due to idle */
    if ((ch->level < LEVEL_IMMORTAL || !ch->desc) && !IS_SET(ch->extra, EXTRA_AFK))
    {
      if (update_player_idle(ch))
      {
        return TRUE;
      }
    }

    /* call some update stuff on this player */
    update_morted_timer(ch);
    update_sit_safe_counter(ch);
    update_drunks(ch);
    sex_update(ch);

    if (IS_SET(ch->newbits, NEW_STITCHES) && number_percent() >= 95)
    {
      REMOVE_BIT(ch->newbits, NEW_STITCHES);
      send_to_char("The 7 stitches holding your mouth shut breaks apart.\n\r", ch);
    }

    /* update MSP */
    if (IS_SET(ch->act, PLR_MUSIC))
      update_midi(ch);

    if (IS_HERO(ch) && ch->hit > 0 && !IS_SET(ch->extra, EXTRA_AFK))
    {
      KINGDOM_DATA *kingdom;

      if (IS_CLASS(ch, CLASS_SHADOW))
        update_shadow(ch);
      else if (IS_CLASS(ch, CLASS_WARLOCK))
        update_warlock(ch);
      else if (IS_CLASS(ch, CLASS_GIANT))
        update_giant(ch);
      else if (IS_CLASS(ch, CLASS_FAE))
        update_fae(ch);
      else if (ch->class == 0 && IS_ITEMAFF(ch, ITEMA_REGENERATE))
        update_arti_regen(ch);

      /* update kings */
      if ((kingdom = get_kingdom(ch)) != NULL && !str_cmp(kingdom->leader, ch->name))
        kingdom->king_active++;
    }
    else if (!IS_EXTRA(ch, EXTRA_AFK))
    {
      if ((IS_ITEMAFF(ch, ITEMA_REGENERATE)) && ch->hit > 0)
      {
        update_arti_regen(ch);
      }
      else if (ch->level < 3 && get_age(ch) < 2)  /* allows newbie mortals to regen */
      {
        update_arti_regen(ch);
      }
      else
      {
        modify_hps(ch, number_range(1, 5));
        update_pos(ch);
      }
    }
  } while (FALSE);

  /* schedule new heal/update event */
  newevent              =  alloc_event();
  newevent->fun         =  &event_player_heal;
  newevent->type        =  EVENT_PLAYER_HEAL;
  add_event_char(newevent, ch, 4 * PULSE_PER_SECOND);

  /* we did not dequeue the event */
  return FALSE;
}

/* Event_player_save
 *
 * saves the player, and enqueues another save event
 * to trigger in 2 minutes. Used for autosaving
 * player pfiles.
 */
bool event_player_save(EVENT_DATA *event)
{
  EVENT_DATA *newevent;
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_save: no owner.", 0);
    return FALSE;
  }

  if (IS_NPC(ch)) return FALSE;

  /* save the player */
  save_char_obj(ch);

  /* schedule new save event */
  newevent              =  alloc_event();
  newevent->fun         =  &event_player_save;
  newevent->type        =  EVENT_PLAYER_SAVE;
  add_event_char(newevent, ch, 2 * 60 * PULSE_PER_SECOND);

  /* we didn't dequeue this event, so return FALSE */
  return FALSE;
}


/***********************************************************
 * Below you'll find all the support functions for events. *
 * Browse and modify as you find fun - learning is vital.  *
 ***********************************************************/

bool enqueue_event(EVENT_DATA *event, int game_pulses)
{
  int bucket, passes;

  if (event->ownertype == EVENT_UNOWNED)
  {
    bug("enqueue_event: event type %d with no owner.", event->type);
    return FALSE;
  }

  /* When loading events, this can sometimes become
   * strictly less than 0 - which causes problems.
   */
  if (game_pulses < 1)
    game_pulses = 1;

  /*
   * calculate which bucket to put the event in,
   * and how many passes the event must stay in
   * the queue.
   */
  bucket = (game_pulses + current_bucket) % MAX_EVENT_HASH;
  passes = game_pulses / MAX_EVENT_HASH;

  /*
   * attach the event in the queue.
   */
  AttachToList(event, eventqueue[bucket]);

  /* modify the event with this knowledge */
  event->passes = passes;
  event->bucket = bucket;

  muddata.events_queued++;

  /* success */
  return TRUE;
}

void dequeue_event(EVENT_DATA *event, bool dequeue_global)
{
  if (dequeue_global)
    DetachFromList(event, eventqueue[event->bucket]);

  switch(event->ownertype)
  {
    default:
      bug("dequeue_event: event type %d has no owner.", event->type);
      break;
    case EVENT_OWNER_GAME:
      DetachFromList(event, global_event_list);
      break;
    case EVENT_OWNER_CHAR:
      DetachFromList(event, event->owner.ch->events);
      break;
    case EVENT_OWNER_DESC:
      DetachFromList(event, event->owner.desc->events);
      break;
    case EVENT_OWNER_OBJ:
      DetachFromList(event, event->owner.obj->events);
      break;
    case EVENT_OWNER_ROOM:
      DetachFromList(event, event->owner.room->events);
      break;
    case EVENT_OWNER_AREA:
      DetachFromList(event, event->owner.area->events);
      break;
  }

  muddata.events_queued--;

  /* and finally we free the data, and recycle it */
  free_event(event);
}

void free_event(EVENT_DATA *event)
{
  /* free memory */
  free_string(event->argument);

  /* attach to free list */
  PushStack(event, event_free);
}

EVENT_DATA *alloc_event()
{
  static EVENT_DATA event_empty;
  EVENT_DATA *event;

  if ((event = (EVENT_DATA *) PopStack(event_free)) == NULL)
  {
    event = calloc(1, sizeof(*event));
    muddata.events_allocated++;
  }

  /* clear the event */
  *event                     =  event_empty;
  event->fun                 =  NULL;
  event->argument            =  str_dup("");
  event->owner.ch            =  NULL;
  event->passes              =  0;
  event->ownertype           =  EVENT_UNOWNED;
  event->type                =  EVENT_NONE;

  return event;
}

void init_event_queue(int section)
{
  ITERATOR *pIter;

  if (section == 1)
  {
    int i;

    /* clear the queue */
    for (i = 0; i < MAX_EVENT_HASH; i++)
    {
      eventqueue[i] = AllocList();
    }

    global_event_list = AllocList();
    event_free = AllocStack();
  }
  else if (section == 2)
  {
    EVENT_DATA *event;
    AREA_DATA *pArea;

    /* the weather update function */
    event        =  alloc_event();
    event->fun   =  &event_game_weather;
    event->type  =  EVENT_GAME_WEATHER;
    add_event_world(event, number_range(1, MAX_EVENT_HASH));

    /* the crash recovery fix */
    event        =  alloc_event();
    event->fun   =  &event_game_crashsafe;
    event->type  =  EVENT_GAME_CRASHSAFE;
    add_event_world(event, 15 * PULSE_PER_SECOND);

    /* saving areas */
    event        =  alloc_event();
    event->fun   =  &event_game_areasave;
    event->type  =  EVENT_GAME_AREASAVE;
    add_event_world(event, 2 * 60 * 60 * PULSE_PER_SECOND);

    /* the arena system */
    event        =  alloc_event();
    event->fun   =  &event_game_arena;
    event->type  =  EVENT_GAME_ARENA;
    add_event_world(event, number_range(30, 90) * 60 * PULSE_PER_SECOND);

    /* 30 second events */
    event        = alloc_event();
    event->fun   = &event_game_pulse30;
    event->type  = EVENT_GAME_PULSE30;
    add_event_world(event, 30 * PULSE_PER_SECOND);

    /* artifact check event */
    event        = alloc_event();
    event->fun   = &event_game_articheck;
    event->type  = EVENT_GAME_ARTICHECK;
    add_event_world(event, (5 * 60 * PULSE_PER_SECOND));

    /* add the kingdom quest update event */
    event        = alloc_event();
    event->fun   = &event_game_kingdomquest;
    event->type  = EVENT_GAME_KINGDOMQUEST;
    add_event_world(event, 60 * PULSE_PER_SECOND);

    pIter = AllocIterator(area_list);
    while ((pArea = (AREA_DATA *) NextInList(pIter)) != NULL)
    {
      event              =  alloc_event();
      event->fun         =  &event_area_reset;
      event->type        =  EVENT_AREA_RESET;
      add_event_area(event, pArea, number_range(1, 10));
    }
  }

  /* that's all for now */
}

void heartbeat()
{
  EVENT_DATA *event;
  ITERATOR *pIter;

  /*
   * current_bucket should be global, it is also used in enqueue_event
   * to figure out what bucket to place the new event in.
   */
  current_bucket = (current_bucket + 1) % MAX_EVENT_HASH;

  pIter = AllocIterator(eventqueue[current_bucket]);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    /*
     * Here we use the event->passes integer, to keep track of
     * how many times we have ignored this event.
     */
    if (event->passes-- > 0) continue;

    /*
     * execute event and extract if needed. We assume that all
     * event functions are of the following prototype
     *
     * bool event_function   args (( EVENT_DATA *event ));
     */
    if (!((*event->fun)(event)))
    {
      DetachAtIterator(pIter);
      dequeue_event(event, FALSE);
    }
  }
}

void init_events_object(OBJ_DATA *obj)
{
  EVENT_DATA *event;
  AFFECT_DATA *paf;
  ITERATOR *pIter;
  bool done = FALSE;

  /* if an object has temporary effects, we will
   * initiate an event to make sure such affects
   * are eventually removed
   */
  pIter = AllocIterator(obj->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->duration > 0)
    {
      event              =  alloc_event();
      event->fun         =  &event_object_affects;
      event->type        =  EVENT_OBJECT_AFFECTS;
      add_event_object(event, obj, number_range(1, MAX_EVENT_HASH));

      done = TRUE;
    }
  }

  /* initialize artifact program */
  if (IS_SET(obj->quest, QUEST_ARTIFACT))
  {
    struct arti_type *artifact;
    int i;

    pIter = AllocIterator(artifact_table);
    while ((artifact = (struct arti_type *) NextInList(pIter)) != NULL)
    {
      if (artifact->vnum == obj->pIndexData->vnum)
      {
        for (i = 0; artifact_programs[i].name[0] != '\0'; i++)
        {
          if (!str_cmp(artifact->fun, artifact_programs[i].name))
            break;
        }

        if (artifact_programs[i].name[0] != '\0')
        {
          event = alloc_event();
          event->type = artifact_programs[i].type;
          event->fun = artifact_programs[i].fun;
          add_event_object(event, obj, artifact_programs[i].delay);
        }

        break;
      }
    }
  }
}

/*
 * Could take as much as 32 seconds before the first
 * update takes place - I'm not sure if I should change
 * this. Atm I don't think it matters.
 */
void init_events_player(CHAR_DATA *ch)
{
  EVENT_DATA *event;

  if (IS_NPC(ch)) return;

  /* start autosave of character */
  event              =  alloc_event();
  event->fun         =  &event_player_save;
  event->type        =  EVENT_PLAYER_SAVE;
  add_event_char(event, ch, number_range(1, MAX_EVENT_HASH));

  /* start autoheal/update of player */
  event              =  alloc_event();
  event->fun         =  &event_player_heal;
  event->type        =  EVENT_PLAYER_HEAL;
  add_event_char(event, ch, number_range(1, MAX_EVENT_HASH));

  /* status autoupdate of player (spells, quests, etc) */
  event              =  alloc_event();
  event->fun         =  &event_char_update;
  event->type        =  EVENT_CHAR_UPDATE;
  add_event_char(event, ch, number_range(1, MAX_EVENT_HASH));

  /* that's all the init events for players */
}

void init_events_mobile(CHAR_DATA *ch)
{
  EVENT_DATA *event;

  if (!IS_NPC(ch)) return;

  /* sentinel mobiles is just given a heal action
   * every 16-24 seconds, healing 10% of their hps,
   * where moving mobiles are updated every 4-6 secs,
   * where they move around and heal 2.5% of their hps.
   */
  if (!IS_SET(ch->act, ACT_SENTINEL))
  {
    event            =  alloc_event();
    event->fun       =  &event_mobile_move;
    event->type      =  EVENT_MOBILE_MOVE;
    add_event_char(event, ch, number_range(1, MAX_EVENT_HASH));
  }
  else
  {
    event            =  alloc_event();
    event->fun       =  &event_mobile_heal;
    event->type      =  EVENT_MOBILE_HEAL;
    add_event_char(event, ch, number_range(1, MAX_EVENT_HASH));
  }

  /* add scavenge update to mobiles */
  if (IS_SET(ch->act, ACT_SCAVENGER))
  {
    event            =  alloc_event();
    event->fun       =  &event_mobile_scavenge;
    event->type      =  EVENT_MOBILE_SCAVENGE;
    add_event_char(event, ch, number_range(1, MAX_EVENT_HASH));
  }

  /* add special program update to mobiles */
  if (ch->spec_fun != 0)
  {
    event            =  alloc_event();
    event->fun       =  &event_mobile_spec;
    event->type      =  EVENT_MOBILE_SPEC;
    add_event_char(event, ch, number_range(1, MAX_EVENT_HASH));
  }

  /* status autoupdate of player (spells, poison, etc) */
  event              =  alloc_event();
  event->fun         =  &event_char_update;
  event->type        =  EVENT_CHAR_UPDATE;
  add_event_char(event, ch, number_range(1, MAX_EVENT_HASH));

  /* that's all the init events for mobiles */
}

void init_events_room(ROOM_INDEX_DATA *pRoom)
{
  EVENT_DATA *event;

  if (IS_SET(pRoom->room_flags, ROOM_BLADE_BARRIER))
  {
    event              =  alloc_event();
    event->fun         =  &event_room_bladebarrier;
    event->type        =  EVENT_ROOM_BLADEBARRIER;
    add_event_room(event, pRoom, number_range(7, 13) * PULSE_PER_SECOND);
  }

  if (IS_SET(pRoom->room_flags, ROOM_DISPEL_MAGIC))
  {
    event              =  alloc_event();
    event->fun         =  &event_room_dispel_magic;
    event->type        =  EVENT_ROOM_DISPEL_MAGIC;
    add_event_room(event, pRoom, number_range(10, 20) * PULSE_PER_SECOND);
  }

  /* that's all the init events for rooms */
}

void count_events(int *a, int *b, int *c, int *d)
{
  int total = 0, max = 0, min = -1, average = 0;
  int i, bound, current;

  if ((bound = (MAX_EVENT_HASH - current_bucket)) < 10)
  {
    bound = 10 - bound;
  }
  else
  {
    bound = 0;
  }

  for (i = 0; i < MAX_EVENT_HASH; i++)
  {
    current = SizeOfList(eventqueue[i]);

    /* update total count */
    total += current;

    /* update max bucket */
    max = UMAX(max, current);

    /* update min bucket */
    if (min < 0)
    {
      min = current;
    }
    else
    {
      min = UMIN(min, current);
    }

    /* add to average if within range */
    if ((i > current_bucket && i <= current_bucket + 10) || i < bound) 
      average += current;
  }

  /* store values */
  *a = total;
  *b = max;
  *c = min;
  *d = average / 10;
}

void object_decay(OBJ_DATA *obj, int delay)
{
  EVENT_DATA *event;

  event             =  alloc_event();
  event->fun        =  &event_object_decay;
  event->type       =  EVENT_OBJECT_DECAY;
  add_event_object(event, obj, delay * PULSE_PER_SECOND);
}

void add_event_world(EVENT_DATA *event, int delay)
{
  if (event->fun == NULL)
  {
    bug("add_event_world no fun.", 0);
    return;
  }
  if (event->type == EVENT_NONE)
  {
    bug("add_event_world: no type.", 0);
    return;
  }

  event->ownertype = EVENT_OWNER_GAME;

  AttachToList(event, global_event_list);
  enqueue_event(event, delay);
}

void add_event_char(EVENT_DATA *event, CHAR_DATA *ch, int delay)
{
  if (event->fun == NULL)
  {
    bug("add_event_char: no fun.", 0);
    return;
  }
  if (event->type == EVENT_NONE)
  {
    bug("add_event_char: no type.", 0);
    return;
  }

  event->ownertype = EVENT_OWNER_CHAR;
  event->owner.ch = ch;

  AttachToList(event, ch->events);
  enqueue_event(event, delay);
}

void add_event_room(EVENT_DATA *event, ROOM_INDEX_DATA *pRoom, int delay)
{
  if (event->fun == NULL)
  {
    bug("add_event_room: no fun.", 0);
    return;
  }
  if (event->type == EVENT_NONE)
  {
    bug("add_event_room: no type.", 0);
    return;
  }

  event->ownertype = EVENT_OWNER_ROOM;
  event->owner.room = pRoom;

  AttachToList(event, pRoom->events);
  enqueue_event(event, delay);
}

void add_event_object(EVENT_DATA *event, OBJ_DATA *obj, int delay)
{
  if (event->fun == NULL)
  {
    bug("add_event_object: no fun.", 0);
    return;
  }
  if (event->type == EVENT_NONE)
  {
    bug("add_event_object: no type.", 0);
    return;
  }

  event->ownertype = EVENT_OWNER_OBJ;
  event->owner.obj = obj;

  AttachToList(event, obj->events);
  enqueue_event(event, delay);
}

void add_event_desc(EVENT_DATA *event, DESCRIPTOR_DATA *d, int delay)
{
  if (event->fun == NULL)
  {
    bug("add_event_desc: no fun.", 0);
    return;
  }
  if (event->type == EVENT_NONE)
  {
    bug("add_event_desc: no type.", 0);
    return;
  }

  event->ownertype = EVENT_OWNER_DESC;
  event->owner.desc = d;

  AttachToList(event, d->events);
  enqueue_event(event, delay);
}

void add_event_area(EVENT_DATA *event, AREA_DATA *pArea, int delay)
{
  if (event->fun == NULL)
  {
    bug("add_event_area: no fun.", 0);
    return;
  }
  if (event->type == EVENT_NONE)
  {
    bug("add_event_area: no type.", 0);
    return;
  }

  event->ownertype = EVENT_OWNER_AREA;
  event->owner.area = pArea;

  AttachToList(event, pArea->events);
  enqueue_event(event, delay);
}

void strip_event_object(OBJ_DATA *obj, int type)
{
  EVENT_DATA *event;
  ITERATOR *pIter;

  pIter = AllocIterator(obj->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == type)
      dequeue_event(event, TRUE);
  }
}

EVENT_DATA *event_isset_socket(DESCRIPTOR_DATA *d, int type)
{
  EVENT_DATA *event;
  ITERATOR *pIter;

  pIter = AllocIterator(d->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == type)
      return event;
  }
  
  return NULL;
}

EVENT_DATA *event_isset_area(AREA_DATA *pArea, int type)
{
  EVENT_DATA *event;
   ITERATOR *pIter;

  pIter = AllocIterator(pArea->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == type)
      return event;
  }

  return NULL;
}

EVENT_DATA *event_isset_room(ROOM_INDEX_DATA *pRoom, int type)
{
  EVENT_DATA *event;
  ITERATOR *pIter;

  pIter = AllocIterator(pRoom->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == type)
      return event;
  }
 
  return NULL;
}

EVENT_DATA *event_isset_object(OBJ_DATA *obj, int type)
{
  EVENT_DATA *event;
  ITERATOR *pIter;

  pIter = AllocIterator(obj->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == type)
      return event;
  }

  return NULL;
}

void strip_event_socket(DESCRIPTOR_DATA *d, int type)
{
  EVENT_DATA *event;
  ITERATOR *pIter;

  pIter = AllocIterator(d->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == type)
      dequeue_event(event, TRUE);
  }
}

void strip_event_world(int type)
{
  EVENT_DATA *event;
  ITERATOR *pIter;

  pIter = AllocIterator(global_event_list);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == type)
      dequeue_event(event, TRUE);
  }
}

void strip_event_mobile(CHAR_DATA *ch, int type)
{
  EVENT_DATA *event;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == type)
      dequeue_event(event, TRUE);
  }
}

EVENT_DATA *event_isset_mobile(CHAR_DATA *ch, int type)
{
  EVENT_DATA *event;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == type)
      return event;
  }
  
  return NULL;
}  

char *event_time_left(EVENT_DATA *event)
{
  static char buf[MAX_STRING_LENGTH];
  char temp[MAX_STRING_LENGTH];
  int secs = event_pulses_left(event) / PULSE_PER_SECOND;
  int mins, hours;

  buf[0] = '\0';

  hours = secs / 3600;
  secs = secs % 3600;
  mins = secs / 60;
  secs = secs % 60;

  if (hours)
  {
    sprintf(temp, "%d hour%s", hours, (hours != 1) ? "s" : "");
    strcat(buf, temp);
  }
  if (mins)
  {
    sprintf(temp, "%s%d minute%s", (hours != 0) ? " " : "", mins, (mins != 1) ? "s" : "");
    strcat(buf, temp);
  }
  if (secs)
  {
    sprintf(temp, "%s%d second%s", (hours || mins) ? " " : "", secs, (secs != 1) ? "s" : "");
    strcat(buf, temp);
  }

  /* make sure it contains something */
  if (!hours && !mins && !secs)
  {
    sprintf(buf, "1 second");
  }

  return buf;
}

int event_pulses_left(EVENT_DATA *event)
{
  int passes = event->passes;
  int bucket = event->bucket;
  int pulses;

  if (current_bucket < bucket)
  {
    pulses = bucket - current_bucket + MAX_EVENT_HASH * passes;
  }
  else
  {
    pulses = (passes + 1) * MAX_EVENT_HASH + bucket - current_bucket;
  }

  return pulses;
}

/* Save all the players events.
 */
void save_player_events(CHAR_DATA *ch, FILE *fp)
{
  EVENT_DATA *event;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    switch(event->type)
    {
      default:
        break;
      case EVENT_PLAYER_STUDY:
      case EVENT_MOBILE_SHADOWGRABBED:
        fprintf(fp, "Event        %d %d %s~\n",
          event->type, event_pulses_left(event),
         (event->argument == NULL) ? "(null)" : event->argument);
        break;
    }
  }
}

/* Loads a single event from the file fp and
 * attaches it to the player and the event_queue.
 */
void load_player_event(CHAR_DATA *ch, FILE *fp)
{
  EVENT_DATA *event;
  int pulses;

  event             =  alloc_event();
  event->type       =  fread_number(fp);
  pulses            =  fread_number(fp);
  event->argument   =  fread_string(fp);

  switch(event->type)
  {
    default:
      bug("load_player_event: bad type %d", event->type);
      free_event(event);
      break;
    case EVENT_PLAYER_STUDY:
      event->fun = &event_player_study;
      add_event_char(event, ch, pulses);
      break;
    case EVENT_MOBILE_SHADOWGRABBED:
      event->fun = &event_mobile_shadowgrabbed;
      add_event_char(event, ch, pulses);
     break;
  }
}
