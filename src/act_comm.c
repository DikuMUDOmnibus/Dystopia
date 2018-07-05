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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dystopia.h"


STACK * history_free = NULL;
STACK * ignore_free  = NULL;

void attach_history  ( CHAR_DATA *ch, char *message, char *argument );
bool is_in           ( char *, char * );
bool all_in          ( char *, char * );
char *socialc        ( CHAR_DATA * ch, char *argument, char *you, char *them );
char *socialv        ( CHAR_DATA * ch, char *argument, char *you, char *them );
char *socialn        ( CHAR_DATA * ch, char *argument, char *you, char *them );

/* Trace's Bounty code */
void do_bounty(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (IS_NPC(ch))
    return;

  if (arg1[0] == '\0' || arg2[0] == '\0')
  {
    send_to_char("Place a bounty on who's head?\n\r"
                 "Syntax:  Bounty <victim> <amount>\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg1)) == NULL)
  {
    send_to_char("They are currently not logged in!\n\r", ch);
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("You cannot put a bounty on NPCs!\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char("Not on yourself.\n\r", ch);
    return;
  }

  if (victim->level >= 7)
  {
    send_to_char("You can't put a bounty on an immortal.\n\r", ch);
    return;
  }

  if (is_number(arg2))
  {
    int amount;

    amount = atoi(arg2);
    if (amount < 100)
    {
      send_to_char("Needs to be at least 100 Gold, less than that you gotta be kidding.\n\r", ch);
      return;
    }
    if (getGold(ch) < amount)
    {
      send_to_char("You don't have that many goldcrowns!\n\r", ch);
      return;
    }
    setGold(ch, -1 * amount);
    victim->pcdata->bounty += amount;
    sprintf(buf, "%s puts %d goldcrowns on %s's head, who now have a %d gc bounty.",
      ch->name, amount, victim->name, victim->pcdata->bounty);
    do_info(ch, buf);
  }
}

void room_message(ROOM_INDEX_DATA * room, char *message)
{
  CHAR_DATA *rch;

  if ((rch = (CHAR_DATA *) FirstInList(room->people)) == NULL)
    return;

  act(message, rch, NULL, NULL, TO_ROOM);
  act(message, rch, NULL, NULL, TO_CHAR);
}

void do_chat(CHAR_DATA * ch, char *argument)
{
  talk_channel(ch, argument, CHANNEL_CHAT, 0, "chat");
}

void do_flame(CHAR_DATA * ch, char *argument)
{
  talk_channel(ch, argument, CHANNEL_FLAME, 0, "bitch");
}

void do_sing(CHAR_DATA * ch, char *argument)
{
  talk_channel(ch, argument, CHANNEL_MUSIC, 0, "sing");
}

char *drunktalk(char *argument)
{
  static char buf[MAX_STRING_LENGTH];
  char *ptr;
  int i, length;

  struct spk_type
  {
    char * old;
    char * new;
  };

  static const struct spk_type spk_table[] =  
  {
    { " ",  " "   },
    { "is", "ish" },
    { "a", "a" }, { "b", "b" }, { "c", "c" }, { "d", "d" },
    { "e", "e" }, { "f", "f" }, { "g", "g" }, { "h", "h" },
    { "i", "i" }, { "j", "j" }, { "k", "k" }, { "l", "l" },
    { "m", "m" }, { "n", "n" }, { "o", "o" }, { "p", "p" },
    { "q", "q" }, { "r", "r" }, { "s", "s" }, { "t", "t" },
    { "u", "u" }, { "v", "v" }, { "w", "w" }, { "x", "x" },
    { "y", "y" }, { "z", "z" }, { ",", "," }, { ".", "." },
    { ";", ";" }, { ":", ":" }, { "(", "(" }, { ")", ")" },
    { ")", ")" }, { "-", "-" }, { "!", "!" }, { "?", "?" },
    { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" },
    { "5", "5" }, { "6", "6" }, { "7", "7" }, { "8", "8" },
    { "9", "9" }, { "0", "0" }, { "%", "%" }, {  "",  "" }
  };

  buf[0] = '\0';

  for (ptr = argument; *ptr != '\0'; ptr += length)
  {
    for (i = 0; spk_table[i].old[0] != '\0'; i++)
    {
      if (!str_prefix(spk_table[i].old, ptr))
      {
        strcat(buf, spk_table[i].new);

        if (number_range(1, 5) == 1 && str_cmp(spk_table[i].new, " "))
          strcat(buf, spk_table[i].new);
        else if (!str_cmp(spk_table[i].new, " "))
        {
          if (number_range(1, 5) == 1 && strlen(buf) < MAX_INPUT_LENGTH)
            strcat(buf, "*hic* ");
        }
        break;
      }
    }

    length = UMAX(1, strlen(spk_table[i].old));
  }

  buf[0] = UPPER(buf[0]);

  for (i = 1; buf[i] != '\0'; i++)
  {
    if (number_range(1, 3) == 1)
      buf[i] = UPPER(buf[i]);
  }

  return buf;
}

void do_yell(CHAR_DATA * ch, char *argument)
{
  talk_channel(ch, argument, CHANNEL_YELL, 0, "yell");
}

void do_immtalk(CHAR_DATA * ch, char *argument)
{
  talk_channel(ch, argument, CHANNEL_IMMTALK, 0, "immtalk");
}

void do_say(CHAR_DATA * ch, char *argument)
{
  char poly[MAX_STRING_LENGTH];
  char speak[10];
  char speaks[10];
  char endbit[2];
  char secbit[2];

  if (!IS_NPC(ch) && IS_SET(ch->newbits, NEW_STITCHES))
  {
    send_to_char("You are having problems talking.\n\r", ch);
    argument = "Umgh gmhu umhf!";
  }

  if (IS_HEAD(ch, LOST_TONGUE))
  {
    send_to_char("You can't speak without a tongue!\n\r", ch);
    return;
  }
  if (IS_EXTRA(ch, GAGGED))
  {
    send_to_char("You can't speak with a gag on!\n\r", ch);
    return;
  }

  if (strlen(argument) > MAX_INPUT_LENGTH)
  {
    send_to_char("Line too long.\n\r", ch);
    return;
  }

  if (argument[0] == '\0')
  {
    send_to_char("Say what?\n\r", ch);
    return;
  }

  endbit[0] = argument[strlen(argument) - 1];
  endbit[1] = '\0';

  if (strlen(argument) > 1)
    secbit[0] = argument[strlen(argument) - 2];
  else
    secbit[0] = '\0';
  secbit[1] = '\0';

  if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
  {
    argument = drunktalk(argument);
    sprintf(speak, "slur");
    sprintf(speaks, "slurs");
  }
  else if (IS_BODY(ch, CUT_THROAT))
  {
    sprintf(speak, "rasp");
    sprintf(speaks, "rasps");
  }
  else if (!str_cmp(endbit, "!"))
  {
    sprintf(speak, "exclaim");
    sprintf(speaks, "exclaims");
  }
  else if (!str_cmp(endbit, "?"))
  {
    sprintf(speak, "ask");
    sprintf(speaks, "asks");
  }
  else if (secbit[0] != '\0' && str_cmp(secbit, ".") && !str_cmp(endbit, "."))
  {
    sprintf(speak, "state");
    sprintf(speaks, "states");
  }
  else if (secbit[0] != '\0' && !str_cmp(secbit, ".") && !str_cmp(endbit, "."))
  {
    sprintf(speak, "mutter");
    sprintf(speaks, "mutters");
  }
  else
  {
    sprintf(speak, "say");
    sprintf(speaks, "says");
  }
  sprintf(poly, "You %s '#y$T#n'.", speak);

  act(poly, ch, NULL, argument, TO_CHAR);

  if (is_silenced(ch))
    return;

  sprintf(poly, "$n %s '#y$T#n'.", speaks);

  act(poly, ch, NULL, argument, TO_ROOM);
  room_text(ch, strlower(argument));
}

void room_text(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *vch;
  CHAR_DATA *mob;
  OBJ_DATA *obj;
  ROOMTEXT_DATA *rt;
  ITERATOR *pIter, *pIter2;
  bool mobfound;
  bool hop;

  pIter2 = AllocIterator(ch->in_room->roomtext);
  while ((rt = (ROOMTEXT_DATA *) NextInList(pIter2)) != NULL)
  {
    if (!strcmp(argument, rt->input) || is_in(argument, rt->input) || all_in(argument, rt->input))
    {
      if (rt->name != NULL && rt->name != '\0' && str_cmp(rt->name, "all") && str_cmp(rt->name, "|all*"))
      {
        if (!is_in(ch->name, rt->name))
          continue;
      }
      mobfound = TRUE;
      if (rt->mob != 0)
      {
        mobfound = FALSE;
        pIter = AllocIterator(ch->in_room->people);
        while ((vch = (CHAR_DATA *) NextInList(pIter)) != NULL)
        {
          if (!IS_NPC(vch))
            continue;
          if (vch->pIndexData->vnum == rt->mob)
          {
            mobfound = TRUE;
            break;
          }
        }
      }
      if (!mobfound)
        continue;
      hop = FALSE;
      switch (rt->type % RT_RETURN)
      {
        case RT_SAY:
          break;
        case RT_LIGHTS:
          do_changelight(ch, "");
          break;
        case RT_LIGHT:
          REMOVE_BIT(ch->in_room->room_flags, ROOM_DARK);
          break;
        case RT_DARK:
          SET_BIT(ch->in_room->room_flags, ROOM_DARK);
          break;
        case RT_OBJECT:
          if (get_obj_index(rt->power) == NULL)
            return;
          obj = create_object(get_obj_index(rt->power), ch->level);
          if (IS_SET(rt->type, RT_TIMER))
            object_decay(obj, 8);
          if (CAN_WEAR(obj, ITEM_TAKE))
            obj_to_char(obj, ch);
          else
            obj_to_room(obj, ch->in_room);
          if (!str_cmp(rt->choutput, "copy"))
            act(rt->output, ch, obj, NULL, TO_CHAR);
          else
            act(rt->choutput, ch, obj, NULL, TO_CHAR);
          if (!IS_SET(rt->type, RT_PERSONAL))
            act(rt->output, ch, obj, NULL, TO_ROOM);
          hop = TRUE;
          break;
        case RT_MOBILE:
          if (get_mob_index(rt->power) == NULL)
            return;
          mob = create_mobile(get_mob_index(rt->power));
          char_to_room(mob, ch->in_room, TRUE);
          if (!str_cmp(rt->choutput, "copy"))
            act(rt->output, ch, NULL, mob, TO_CHAR);
          else
            act(rt->choutput, ch, NULL, mob, TO_CHAR);
          if (!IS_SET(rt->type, RT_PERSONAL))
            act(rt->output, ch, NULL, mob, TO_ROOM);
          hop = TRUE;
          break;
        case RT_SPELL:
          (*skill_table[rt->power].spell_fun) (rt->power, number_range(40, 50), ch, ch);
          break;
        case RT_PORTAL:
          if (get_obj_index(OBJ_VNUM_PORTAL) == NULL)
            return;
          obj = create_object(get_obj_index(OBJ_VNUM_PORTAL), 0);
          object_decay(obj, 8);
          obj->value[0] = rt->power;
          obj->value[1] = 1;
          obj_to_room(obj, ch->in_room);
          break;
        case RT_TELEPORT:
          if (get_room_index(rt->power) == NULL)
            return;
          if (!str_cmp(rt->choutput, "copy"))
            act(rt->output, ch, NULL, NULL, TO_CHAR);
          else
            act(rt->choutput, ch, NULL, NULL, TO_CHAR);
          if (!IS_SET(rt->type, RT_PERSONAL))
            act(rt->output, ch, NULL, NULL, TO_ROOM);
          char_from_room(ch);
          char_to_room(ch, get_room_index(rt->power), TRUE);
          act("$n appears in the room.", ch, NULL, NULL, TO_ROOM);
          do_look(ch, "auto");
          hop = TRUE;
          break;
        case RT_ACTION:
          pIter = AllocIterator(ch->in_room->people);
          while ((mob = (CHAR_DATA *) NextInList(pIter)) != NULL)
          {
            char xbuf[MAX_INPUT_LENGTH];

            if (is_contained("%s", rt->output))
              sprintf(xbuf, rt->output, ch->name);
            else
              strcpy(xbuf, rt->output);

            if (!IS_NPC(mob) || mob == ch)
              continue;
            if (mob->pIndexData->vnum == rt->mob)
              interpret(mob, xbuf);
          }
          break;
        case RT_TEXT:
          send_to_char(rt->output, ch);
          break;
        case RT_OPEN_LIFT:
          open_lift(ch);
          break;
        case RT_CLOSE_LIFT:
          close_lift(ch);
          break;
        case RT_MOVE_LIFT:
          move_lift(ch, rt->power);
          break;
        default:
          break;
      }
      if (hop && IS_SET(rt->type, RT_RETURN))
        return;
      else if (hop)
        continue;
      if (!str_cmp(rt->choutput, "copy") && !IS_SET(rt->type, RT_ACTION))
        act(rt->output, ch, NULL, NULL, TO_CHAR);
      else if (!IS_SET(rt->type, RT_ACTION))
        act(rt->choutput, ch, NULL, NULL, TO_CHAR);
      if (!IS_SET(rt->type, RT_PERSONAL) && !IS_SET(rt->type, RT_ACTION))
        act(rt->output, ch, NULL, NULL, TO_ROOM);
      if (IS_SET(rt->type, RT_RETURN))
        return;
    }
  }
}

char *strlower(char *ip)
{
  static char buffer[MAX_INPUT_LENGTH];
  int pos;

  for (pos = 0; pos < (MAX_INPUT_LENGTH - 1) && ip[pos] != '\0'; pos++)
  {
    buffer[pos] = tolower(ip[pos]);
  }
  buffer[pos] = '\0';

  return buffer;
}

bool is_in(char *arg, char *ip)
{
  char *lo_arg;
  char cmp[MAX_INPUT_LENGTH];
  int fitted;

  if (ip[0] != '|')
    return FALSE;

  cmp[0] = '\0';
  lo_arg = strlower(arg);

  do
  {
    ip += strlen(cmp) + 1;
    fitted = sscanf(ip, "%[^*]", cmp);
    if (strstr(lo_arg, cmp) != NULL)
    {
      return TRUE;
    }
  }
  while (fitted > 0);

  return FALSE;
}

bool all_in(char *arg, char *ip)
{
  char *lo_arg;
  char cmp[MAX_INPUT_LENGTH];
  int fitted;

  if (ip[0] != '&')
    return FALSE;

  cmp[0] = '\0';
  lo_arg = strlower(arg);

  do
  {
    ip += strlen(cmp) + 1;
    fitted = sscanf(ip, "%[^*]", cmp);
    if (strstr(lo_arg, cmp) == NULL)
    {
      return FALSE;
    }
  }
  while (fitted > 0);

  return TRUE;
}

void do_tell(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char poly[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int position;

  if (IS_EXTRA(ch, GAGGED))
  {
    send_to_char("Your message didn't get through.\n\r", ch);
    return;
  }

  argument = one_argument(argument, arg);

  if (arg[0] == '\0' || argument[0] == '\0')
  {
    send_to_char("Tell whom what?\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg)) == NULL || IS_NPC(victim))
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (!IS_IMMORTAL(ch) && !IS_AWAKE(victim))
  {
    act("$E can't hear you.", ch, 0, victim, TO_CHAR);
    return;
  }

  if (!IS_NPC(victim) && victim->desc == NULL)
  {
    act("$E is currently link dead.", ch, 0, victim, TO_CHAR);
    return;
  }

  if (IS_SET(victim->deaf, CHANNEL_TELL) && !IS_IMMORTAL(ch))
  {
    if (IS_NPC(victim) || IS_NPC(ch) || strlen(victim->pcdata->marriage) < 2 || str_cmp(ch->name, victim->pcdata->marriage))
    {
      act("$E can't hear you.", ch, 0, victim, TO_CHAR);
      return;
    }
  }

  if (check_ignore(ch, victim))
  {
    send_to_char("They are ignoring you.\n\r", ch);
    return;
  }

  sprintf(poly, "You tell $N '#C$t#n'.");
  act(poly, ch, argument, victim, TO_CHAR);

  if (!IS_NPC(ch) && !IS_NPC(victim))
    update_history(ch, victim, poly, argument, -1);

  if (is_silenced(ch))
    return;

  position = victim->position;
  victim->position = POS_STANDING;

  sprintf(poly, "$n tells you '#C$t#n'.");
  act(poly, ch, argument, victim, TO_VICT);

  if (!IS_NPC(ch) && !IS_NPC(victim))
    update_history(victim, ch, poly, argument, 1);

  victim->position = position;
  victim->pcdata->reply = ch;
}

void do_whisper(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  if (IS_EXTRA(ch, GAGGED))
  {
    send_to_char("Not with a gag on!\n\r", ch);
    return;
  }

  argument = one_argument(argument, arg);

  if (arg[0] == '\0' || argument[0] == '\0')
  {
    send_to_char("Syntax: whisper <person> <message>\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg)) == NULL || (victim->in_room != ch->in_room))
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (!IS_AWAKE(victim))
  {
    act("$E cannot hear you.", ch, 0, victim, TO_CHAR);
    return;
  }

  if (!IS_NPC(victim) && victim->desc == NULL)
  {
    act("$E is currently link dead.", ch, 0, victim, TO_CHAR);
    return;
  }

  act("You whisper to $N '$t'.", ch, argument, victim, TO_CHAR);

  if (is_silenced(ch)) return;

  act("$n whispers to you '$t'.", ch, argument, victim, TO_VICT);
  act("$n whispers something to $N.", ch, NULL, victim, TO_NOTVICT);
}

void do_reply(CHAR_DATA * ch, char *argument)
{
  char poly[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int position;

  if (IS_NPC(ch))
    return;

  if (IS_EXTRA(ch, GAGGED))
  {
    send_to_char("Your message didn't get through.\n\r", ch);
    return;
  }

  if ((victim = ch->pcdata->reply) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (!IS_IMMORTAL(ch) && !IS_AWAKE(victim))
  {
    act("$E can't hear you.", ch, 0, victim, TO_CHAR);
    return;
  }

  if (!IS_NPC(victim) && victim->desc == NULL)
  {
    act("$E is currently link dead.", ch, 0, victim, TO_CHAR);
    return;
  }

  if (check_ignore(ch, victim))
  {
    send_to_char("They are ignoring you.\n\r", ch);
    return;
  }

  sprintf(poly, "You reply to $N '#C$t#n'.");
  act(poly, ch, argument, victim, TO_CHAR);

  if (!IS_NPC(ch) && !IS_NPC(victim))
    update_history(ch, victim, poly, argument, -1);

  if (is_silenced(ch))
    return;

  position = victim->position;
  victim->position = POS_STANDING;

  sprintf(poly, "$n replies to you '#C$t#n'.");
  act(poly, ch, argument, victim, TO_VICT);

  if (!IS_NPC(ch) && !IS_NPC(victim))
  {
    update_history(victim, ch, poly, argument, 1);
  }

  victim->position = position;
  victim->pcdata->reply = ch;
}

void do_emote(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char *plast;

  if (IS_HEAD(ch, LOST_TONGUE) || IS_EXTRA(ch, GAGGED))
  {
    send_to_char("You can't show your emotions.\n\r", ch);
    return;
  }

  if (argument[0] == '\0')
  {
    send_to_char("Pose what?\n\r", ch);
    return;
  }

  for (plast = argument; *plast != '\0'; plast++)
    ;

  strcpy(buf, argument);
  if (isalpha(plast[-1]))
    strcat(buf, ".");

  act("You $T", ch, NULL, buf, TO_CHAR);

  if (is_silenced(ch))
    return;

  act("$n $T", ch, NULL, buf, TO_ROOM);
}

void do_xemote(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char oldarg[MAX_STRING_LENGTH];
  char *plast;
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  char you[80];
  char them[80];

  argument = one_argument(argument, arg);

  if (IS_HEAD(ch, LOST_TONGUE) || IS_EXTRA(ch, GAGGED))
  {
    send_to_char("You can't show your emotions.\n\r", ch);
    return;
  }

  if (strlen(argument) > MAX_INPUT_LENGTH)
  {
    send_to_char("Line too long.\n\r", ch);
    return;
  }

  if (argument[0] == '\0' || arg[0] == '\0')
  {
    send_to_char("Syntax: emote <person> <sentence>\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }
  if (IS_NPC(ch))
  {
    if (ch->short_descr != NULL)
      strcpy(you, ch->short_descr);
    else
      return;
  }
  else
  {
    if (ch->name != NULL)
      strcpy(you, ch->name);
    else
      return;
  }
  if (IS_NPC(victim))
  {
    if (victim->short_descr != NULL)
      strcpy(you, victim->short_descr);
    else
      return;
  }
  else
  {
    if (victim->name != NULL)
      strcpy(you, victim->name);
    else
      return;
  }

  strcpy(oldarg, argument);
  strcpy(buf, argument);
  for (plast = argument; *plast != '\0'; plast++)
    ;

  if (isalpha(plast[-1]))
    strcat(buf, ".");
  argument = socialc(ch, buf, you, them);

  strcpy(buf, argument);
  strcpy(buf2, "You ");
  buf[0] = LOWER(buf[0]);
  strcat(buf2, buf);
  capitalize(buf2);
  act(buf2, ch, NULL, victim, TO_CHAR);

  if (is_silenced(ch)) return;

  strcpy(buf, oldarg);
  for (plast = argument; *plast != '\0'; plast++)
    ;
  if (isalpha(plast[-1]))
    strcat(buf, ".");

  argument = socialn(ch, buf, you, them);

  strcpy(buf, argument);
  strcpy(buf2, "$n ");
  buf[0] = LOWER(buf[0]);
  strcat(buf2, buf);
  capitalize(buf2);
  act(buf2, ch, NULL, victim, TO_NOTVICT);

  strcpy(buf, oldarg);
  for (plast = argument; *plast != '\0'; plast++)
    ;
  if (isalpha(plast[-1]))
    strcat(buf, ".");

  argument = socialv(ch, buf, you, them);

  strcpy(buf, argument);
  strcpy(buf2, "$n ");
  buf[0] = LOWER(buf[0]);
  strcat(buf2, buf);
  capitalize(buf2);
  act(buf2, ch, NULL, victim, TO_VICT);
}

void do_idea(CHAR_DATA * ch, char *argument)
{
  send_to_char("Please use board 2 for ideas.\n\r", ch);
}

void do_typo(CHAR_DATA * ch, char *argument)
{
  send_to_char("Please post a note on the general board about any typos.\n\r", ch);
}

void do_qui(CHAR_DATA * ch, char *argument)
{
  send_to_char("If you want to QUIT, you have to spell it out.\n\r", ch);
}

void do_quit(CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d = ch->desc;
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *mount;
  CHAR_DATA *gch;
  ITERATOR *pIter;
  bool silent = FALSE;

  if (argument[0] != '\0' && argument[1] != '\0' &&
      argument[0] ==  27  && argument[1] ==  27)
    silent = TRUE;

  if (IS_NPC(ch))
    return;

  if (ch->position == POS_FIGHTING)
  {
    send_to_char("No way! You are fighting.\n\r", ch);
    return;
  }

  if (ch->fight_timer > 0)
  {
    send_to_char("Not until your fight timer expires.\n\r", ch);
    return;
  }

  if (in_arena(ch) || in_fortress(ch))
  {
    send_to_char("You cannot quit in the arena or in the fortress.\n\r", ch);
    return;
  }

  if (ch->position != POS_STANDING)
  {
    ch->position = POS_STANDING;
  }

  if (ch->in_room != NULL)
  {
    /* remove player from others kingdoms */
    if (IS_SET(ch->in_room->room_flags, ROOM_KINGDOM) && !in_kingdom_hall(ch))
    {
      ROOM_INDEX_DATA *pRoom = get_room_index(ROOM_VNUM_CITYSAFE);

      if (pRoom != NULL)
      {
        char_from_room(ch);
        char_to_room(ch, pRoom, TRUE);
      }
    }
  }

  call_all(ch);                 /* wippy-doodle-doo */

  pIter = AllocIterator(char_list);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_NPC(gch))
      continue;
    if (gch->desc)
    {
      if (gch->desc->connected != CON_PLAYING && !(gch->desc->connected >= CON_NOTE_TO && gch->desc->connected <= CON_NOTE_FINISH))
        continue;
    }
    if (gch->challenger == ch)
    {
      gch->challenger = NULL;
      send_to_char("Your victim leaves the game.\n\r", gch);
    }
  }

  if (ch->level > 6);           /* do nothing */
  else if (!silent && IS_SET(ch->pcdata->jflags, JFLAG_SETLOGOUT))
    logout_message(ch);
  else if (!silent)
  {
    sprintf(buf, "#R%s #9has fled from #RCalim's Cradle#9.#n", ch->name);
    leave_info(buf);
  }

  if (ch->challenger != NULL)
    ch->challenger = NULL;
  if ((mount = ch->mount) != NULL)
    do_dismount(ch, "");

  if (ch->pcdata->in_progress)
    free_note(ch->pcdata->in_progress);

  save_char_obj(ch);
  act("$n has left the game.", ch, NULL, NULL, TO_ROOM);

  if (ch->in_room != NULL)
    char_from_room(ch);
  char_to_room(ch, get_room_index(ROOM_VNUM_DISCONNECTION), TRUE);

  if (!silent)
  {
    log_string("%s has quit.", ch->name);
  }

  /*
   * After extract_char the ch is no longer valid!
   */
  extract_char(ch, TRUE);

  if (d != NULL)
  {
    EVENT_DATA *event;

    d->connected = CON_PICK_PLAYER;
    show_options(d);

    event = alloc_event();
    event->fun = &event_socket_idle;
    event->type = EVENT_SOCKET_IDLE;
    add_event_desc(event, d, 3 * 60 * PULSE_PER_SECOND);
  }
}

void do_save(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (ch->level < 2)
  {
    send_to_char("You must kill at least 5 mobs before you can save.\n\r", ch);
    return;
  }

  /* We no longer save per request, it's done automagically */
  /* save_char_obj(ch);  */

  send_to_char("Saved.\n\r", ch);
}

void do_follow(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Follow whom?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }
  if (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL)
  {
    act("But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR);
    return;
  }
  if (victim == ch)
  {
    if (ch->master == NULL)
    {
      send_to_char("You already follow yourself.\n\r", ch);
      return;
    }
    stop_follower(ch, FALSE);
    return;
  }

  if (ch->master != NULL)
    stop_follower(ch, FALSE);

  add_follower(ch, victim);
  return;
}

void add_follower(CHAR_DATA * ch, CHAR_DATA * master)
{
  if (ch->master != NULL)
  {
    bug("Add_follower: non-null master.", 0);
    return;
  }

  ch->master = master;
  ch->leader = NULL;

  if (can_see(master, ch))
    act("$n now follows you.", ch, NULL, master, TO_VICT);

  act("You now follow $N.", ch, NULL, master, TO_CHAR);
}

void stop_follower(CHAR_DATA * ch, bool isDead)
{
  if (ch->master == NULL)
  {
    bug("Stop_follower: null master.", 0);
    return;
  }
  if (IS_AFFECTED(ch, AFF_CHARM))
  {
    REMOVE_BIT(ch->affected_by, AFF_CHARM);
    affect_strip(ch, gsn_charm_person);
  }
  if (can_see(ch->master, ch))
    act("$n stops following you.", ch, NULL, ch->master, TO_VICT);
  act("You stop following $N.", ch, NULL, ch->master, TO_CHAR);

  ch->master = NULL;
  ch->leader = NULL;
}

void die_follower(CHAR_DATA * ch)
{
  CHAR_DATA *fch;
  ITERATOR *pIter;

  if (ch->simulacrum)
  {
    CHAR_DATA *simulacrum = ch->simulacrum;

    simulacrum->simulacrum = NULL;
    send_to_char("You feel the death of your simulacrum.\n\r", simulacrum);
    ch->simulacrum = NULL;
  }

  if (ch->master != NULL)
    stop_follower(ch, TRUE);

  ch->leader = NULL;

  pIter = AllocIterator(char_list);
  while ((fch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (fch->leader == ch)
      fch->leader = fch;
    if (fch->master == ch)
      stop_follower(fch, FALSE);
  }
}

void do_order(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *och;
  ITERATOR *pIter;
  bool found;
  bool fAll;

  argument = one_argument(argument, arg);

  if (arg[0] == '\0' || argument[0] == '\0')
  {
    send_to_char("Order whom to do what?\n\r", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM))
  {
    send_to_char("You feel like taking, not giving, orders.\n\r", ch);
    return;
  }

  if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
  {
    send_to_char("You can't order things around here.\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "all"))
  {
    send_to_char("Ordering 'all' has been disabled.\n\r", ch);
    return;
  }
  else
  {
    fAll = FALSE;
    if ((victim = get_char_room(ch, arg)) == NULL)
    {
      send_to_char("They aren't here.\n\r", ch);
      return;
    }

    if (victim == ch)
    {
      send_to_char("Aye aye, right away!\n\r", ch);
      return;
    }

    if ((!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch))

    {
      send_to_char("Do it yourself!\n\r", ch);
      return;
    }

  }

  found = FALSE;
  pIter = AllocIterator(ch->in_room->people);
  while ((och = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (och == ch)
      continue;

    if ((IS_AFFECTED(och, AFF_CHARM) && och->master == ch && (fAll || och == victim)))
    {
      found = TRUE;
      act("$n orders you to '$t'.", ch, argument, och, TO_VICT);
      interpret(och, argument);
    }
  }

  if (found)
  {
    send_to_char("Ok.\n\r", ch);
    if (!IS_NPC(victim))
      ch->wait = victim->wait;
  }
  else
    send_to_char("You have no followers here.\n\r", ch);
  WAIT_STATE(ch, 12);
}

void do_group(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim, *gch;
  ITERATOR *pIter;

  if (IS_NPC(ch)) return;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    CHAR_DATA *leader;

    leader = (ch->leader != NULL) ? ch->leader : ch;
    sprintf(buf, "%s's group:\n\r", PERS(leader, ch));
    send_to_char(buf, ch);

    pIter = AllocIterator(char_list);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (is_same_group(gch, ch))
      {
        sprintf(buf, "[%-16s] %4d/%4d hp %4d/%4d mana %4d/%4d mv %5d xp\n\r",
          gch->name, gch->hit, gch->max_hit, gch->mana, gch->max_mana, gch->move, gch->max_move, gch->exp);
        send_to_char(buf, ch);
      }
    }
    return;
  }

  if (ch->master != NULL || (ch->leader != NULL && ch->leader != ch))
  {
    send_to_char("But you are following someone else!\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "all"))
  {
    bool found = FALSE;

    pIter = AllocIterator(ch->in_room->people);
    while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (gch == ch) continue;

      if (gch->master == ch && !is_same_group(ch, gch))
      {
        gch->leader = ch;
        act("$N joins $n's group.", ch, NULL, gch, TO_NOTVICT);
        act("You join $n's group.", ch, NULL, gch, TO_VICT);
        act("$N joins your group.", ch, NULL, gch, TO_CHAR);
        found = TRUE;
      }
    }

    if (!found)
      send_to_char("Noone is following you.\n\r", ch);

    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (victim->master != ch && ch != victim)
  {
    act("$N isn't following you.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if (is_same_group(victim, ch) && ch != victim)
  {
    victim->leader = NULL;
    act("$n removes $N from $s group.", ch, NULL, victim, TO_NOTVICT);
    act("$n removes you from $s group.", ch, NULL, victim, TO_VICT);
    act("You remove $N from your group.", ch, NULL, victim, TO_CHAR);
    return;
  }

  victim->leader = ch;
  act("$N joins $n's group.", ch, NULL, victim, TO_NOTVICT);
  act("You join $n's group.", ch, NULL, victim, TO_VICT);
  act("$N joins your group.", ch, NULL, victim, TO_CHAR);
}

void do_gtell(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  ITERATOR *pIter;
  DESCRIPTOR_DATA *d;
  CHAR_DATA *gch;

  if (argument[0] == '\0')
  {
    send_to_char("Tell your group what?\n\r", ch);
    return;
  }

  /*
   * Note use of send_to_char, so gtell works on sleepers.
   */
  sprintf(buf, "#G%s tells the group #R'#G%s#R'\n\r#n", ch->name, argument);
  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->character != NULL)
      gch = d->character;
    else
      continue;

    if (is_same_group(gch, ch))
      send_to_char(buf, gch);
  }
}

/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group(CHAR_DATA * ach, CHAR_DATA * bch)
{
  if (ach->leader != NULL)
    ach = ach->leader;
  if (bch->leader != NULL)
    bch = bch->leader;

  return ach == bch;
}

void do_changelight(CHAR_DATA * ch, char *argument)
{
  if (IS_SET(ch->in_room->room_flags, ROOM_DARK))
  {
    REMOVE_BIT(ch->in_room->room_flags, ROOM_DARK);
    act("The room is suddenly filled with light!", ch, NULL, NULL, TO_CHAR);
    act("The room is suddenly filled with light!", ch, NULL, NULL, TO_ROOM);
    return;
  }
  SET_BIT(ch->in_room->room_flags, ROOM_DARK);
  act("The lights in the room suddenly go out!", ch, NULL, NULL, TO_CHAR);
  act("The lights in the room suddenly go out!", ch, NULL, NULL, TO_ROOM);
}

void open_lift(CHAR_DATA * ch)
{
  ROOM_INDEX_DATA *location;
  int in_room;

  in_room = ch->in_room->vnum;
  location = get_room_index(in_room);

  if (is_open(ch))
    return;

  act("The doors open.", ch, NULL, NULL, TO_CHAR);
  act("The doors open.", ch, NULL, NULL, TO_ROOM);
  move_door(ch);
  if (is_open(ch))
    act("The doors close.", ch, NULL, NULL, TO_ROOM);
  if (!same_floor(ch, in_room))
    act("The lift judders suddenly.", ch, NULL, NULL, TO_ROOM);
  if (is_open(ch))
    act("The doors open.", ch, NULL, NULL, TO_ROOM);
  move_door(ch);
  open_door(ch, FALSE);
  char_from_room(ch);
  char_to_room(ch, location, TRUE);
  open_door(ch, TRUE);
  move_door(ch);
  open_door(ch, TRUE);
  thru_door(ch, in_room);
  char_from_room(ch);
  char_to_room(ch, location, TRUE);
}

void close_lift(CHAR_DATA * ch)
{
  ROOM_INDEX_DATA *location;
  int in_room;

  in_room = ch->in_room->vnum;
  location = get_room_index(in_room);

  if (!is_open(ch))
    return;

  act("The doors close.", ch, NULL, NULL, TO_CHAR);
  act("The doors close.", ch, NULL, NULL, TO_ROOM);
  open_door(ch, FALSE);
  move_door(ch);
  open_door(ch, FALSE);
  char_from_room(ch);
  char_to_room(ch, location, TRUE);
}

void move_lift(CHAR_DATA * ch, int to_room)
{
  ROOM_INDEX_DATA *location;
  int in_room;

  in_room = ch->in_room->vnum;
  location = get_room_index(in_room);

  if (is_open(ch))
    act("The doors close.", ch, NULL, NULL, TO_CHAR);
  if (is_open(ch))
    act("The doors close.", ch, NULL, NULL, TO_ROOM);
  if (!same_floor(ch, to_room))
    act("The lift judders suddenly.", ch, NULL, NULL, TO_CHAR);
  if (!same_floor(ch, to_room))
    act("The lift judders suddenly.", ch, NULL, NULL, TO_ROOM);
  move_door(ch);
  open_door(ch, FALSE);
  char_from_room(ch);
  char_to_room(ch, location, TRUE);
  open_door(ch, FALSE);
  thru_door(ch, to_room);
}

bool same_floor(CHAR_DATA * ch, int cmp_room)
{
  OBJ_DATA *obj;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->in_room->contents);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->item_type != ITEM_PORTAL)
      continue;
    if (obj->value[0] == cmp_room)
      return TRUE;
    else
      return FALSE;
  }

  return FALSE;
}

bool is_open(CHAR_DATA * ch)
{
  OBJ_DATA *obj;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->in_room->contents);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->item_type != ITEM_PORTAL)
      continue;
    if (obj->value[2] == 0)
      return TRUE;
    else
      return FALSE;
  }

  return FALSE;
}

void move_door(CHAR_DATA * ch)
{
  OBJ_DATA *obj;
  ROOM_INDEX_DATA *pRoomIndex;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->in_room->contents);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->item_type != ITEM_PORTAL)
      continue;

    pRoomIndex = get_room_index(obj->value[0]);
    char_from_room(ch);
    char_to_room(ch, pRoomIndex, TRUE);
    return;
  }
}

void thru_door(CHAR_DATA * ch, int doorexit)
{
  OBJ_DATA *obj;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->in_room->contents);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->item_type != ITEM_PORTAL)
      continue;

    obj->value[0] = doorexit;
    return;
  }
}

void open_door(CHAR_DATA * ch, bool be_open)
{
  OBJ_DATA *obj;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->in_room->contents);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (obj->item_type != ITEM_PORTAL)
      continue;
    if (obj->value[2] == 0 && !be_open)
      obj->value[2] = 3;
    else if (obj->value[2] == 3 && be_open)
      obj->value[2] = 0;

    return;
  }
}

char *socialc(CHAR_DATA * ch, char *argument, char *you, char *them)
{
  char buf[MAX_STRING_LENGTH];
  char *pName;
  int iSyl;
  int length;

  struct spk_type
  {
    char *old;
    char *new;
  };

  static const struct spk_type spk_table[] = {
    {" ", " "},
    {"you are", "$E is"},
    {"you.", "$M."},
    {"you,", "$M,"},
    {"you ", "$M "},
    {" you", " $M"},
    {"your ", "$S "},
    {" your", " $S"},
    {"yours.", "theirs."},
    {"yours,", "theirs,"},
    {"yours ", "theirs "},
    {" yours", " theirs"},
    {"begins", "begin"},
    {"caresses", "caress"},
    {"gives", "give"},
    {"glares", "glare"},
    {"grins", "grin"},
    {"licks", "lick"},
    {"looks", "look"},
    {"loves", "love"},
    {"plunges", "plunge"},
    {"presses", "press"},
    {"pulls", "pull"},
    {"runs", "run"},
    {"slaps", "slap"},
    {"slides", "slide"},
    {"smashes", "smash"},
    {"squeezes", "squeeze"},
    {"stares", "stare"},
    {"sticks", "stick"},
    {"strokes", "stroke"},
    {"tugs", "tug"},
    {"thinks", "think"},
    {"thrusts", "thrust"},
    {"whistles", "whistle"},
    {"wraps", "wrap"},
    {"winks", "wink"},
    {"wishes", "wish"},
    {" winks", " wink"},
    {" his", " your"},
    {"his ", "your "},
    {" her", " your"},
    {"her ", "your "},
    {" him", " your"},
    {"him ", "your "},
    {"the", "the"},
    {" he", " you"},
    {"he ", "you "},
    {" she", " you"},
    {"she ", "you "},
    {"a", "a"}, {"b", "b"}, {"c", "c"}, {"d", "d"},
    {"e", "e"}, {"f", "f"}, {"g", "g"}, {"h", "h"},
    {"i", "i"}, {"j", "j"}, {"k", "k"}, {"l", "l"},
    {"m", "m"}, {"n", "n"}, {"o", "o"}, {"p", "p"},
    {"q", "q"}, {"r", "r"}, {"s", "s"}, {"t", "t"},
    {"u", "u"}, {"v", "v"}, {"w", "w"}, {"x", "x"},
    {"y", "y"}, {"z", "z"}, {",", ","}, {".", "."},
    {";", ";"}, {":", ":"}, {"(", "("}, {")", ")"},
    {")", ")"}, {"-", "-"}, {"!", "!"}, {"?", "?"},
    {"1", "1"}, {"2", "2"}, {"3", "3"}, {"4", "4"},
    {"5", "5"}, {"6", "6"}, {"7", "7"}, {"8", "8"},
    {"9", "9"}, {"0", "0"}, {"%", "%"}, {"", ""}
  };

  buf[0] = '\0';

  if (argument[0] == '\0')
    return argument;

  for (pName = argument; *pName != '\0'; pName += length)
  {
    for (iSyl = 0; (length = strlen(spk_table[iSyl].old)) != 0; iSyl++)
    {
      if (!str_prefix(spk_table[iSyl].old, pName))
      {
        strcat(buf, spk_table[iSyl].new);
        break;
      }
    }

    if (length == 0)
      length = 1;
  }

  argument[0] = '\0';
  strcpy(argument, buf);
  argument[0] = UPPER(argument[0]);

  return argument;
}

char *socialv(CHAR_DATA * ch, char *argument, char *you, char *them)
{
  char buf[MAX_STRING_LENGTH];
  char *pName;
  int iSyl;
  int length;

  struct spk_type
  {
    char *old;
    char *new;
  };

  static const struct spk_type spk_table[] = {
    {" ", " "},
    {" his", " $s"},
    {"his ", "$s "},
    {" her", " $s"},
    {"her ", "$s "},
    {" him", " $m"},
    {"him ", "$m "},
    {" he", " $e"},
    {"he ", "$e "},
    {" she", " $e"},
    {"she ", "$e "},
    {"a", "a"}, {"b", "b"}, {"c", "c"}, {"d", "d"},
    {"e", "e"}, {"f", "f"}, {"g", "g"}, {"h", "h"},
    {"i", "i"}, {"j", "j"}, {"k", "k"}, {"l", "l"},
    {"m", "m"}, {"n", "n"}, {"o", "o"}, {"p", "p"},
    {"q", "q"}, {"r", "r"}, {"s", "s"}, {"t", "t"},
    {"u", "u"}, {"v", "v"}, {"w", "w"}, {"x", "x"},
    {"y", "y"}, {"z", "z"}, {",", ","}, {".", "."},
    {";", ";"}, {":", ":"}, {"(", "("}, {")", ")"},
    {")", ")"}, {"-", "-"}, {"!", "!"}, {"?", "?"},
    {"1", "1"}, {"2", "2"}, {"3", "3"}, {"4", "4"},
    {"5", "5"}, {"6", "6"}, {"7", "7"}, {"8", "8"},
    {"9", "9"}, {"0", "0"}, {"%", "%"}, {"", ""}
  };

  buf[0] = '\0';

  if (argument[0] == '\0')
    return argument;

  for (pName = argument; *pName != '\0'; pName += length)
  {
    for (iSyl = 0; (length = strlen(spk_table[iSyl].old)) != 0; iSyl++)
    {
      if (!str_prefix(spk_table[iSyl].old, pName))
      {
        strcat(buf, spk_table[iSyl].new);
        break;
      }
    }

    if (length == 0)
      length = 1;
  }

  argument[0] = '\0';
  strcpy(argument, buf);
  argument[0] = UPPER(argument[0]);

  return argument;
}

char *socialn(CHAR_DATA * ch, char *argument, char *you, char *them)
{
  char buf[MAX_STRING_LENGTH];
  char *pName;
  int iSyl;
  int length;

  struct spk_type
  {
    char *old;
    char *new;
  };

  static const struct spk_type spk_table[] = {
    {" ", " "},
    {"you are", "$N is"},
    {"you.", "$N."},
    {"you,", "$N,"},
    {"you ", "$N "},
    {" you", " $N"},
    {"your.", "$N's."},
    {"your,", "$N's,"},
    {"your ", "$N's "},
    {" your", " $N's"},
    {"yourself", "$Mself"},
    {" his", " $s"},
    {"his ", "$s "},
    {" her", " $s"},
    {"her ", "$s "},
    {" him", " $m"},
    {"him ", "$m "},
    {" he", " $e"},
    {"he ", "$e "},
    {" she", " $e"},
    {"she ", "$e "},
    {"a", "a"}, {"b", "b"}, {"c", "c"}, {"d", "d"},
    {"e", "e"}, {"f", "f"}, {"g", "g"}, {"h", "h"},
    {"i", "i"}, {"j", "j"}, {"k", "k"}, {"l", "l"},
    {"m", "m"}, {"n", "n"}, {"o", "o"}, {"p", "p"},
    {"q", "q"}, {"r", "r"}, {"s", "s"}, {"t", "t"},
    {"u", "u"}, {"v", "v"}, {"w", "w"}, {"x", "x"},
    {"y", "y"}, {"z", "z"}, {",", ","}, {".", "."},
    {";", ";"}, {":", ":"}, {"(", "("}, {")", ")"},
    {")", ")"}, {"-", "-"}, {"!", "!"}, {"?", "?"},
    {"1", "1"}, {"2", "2"}, {"3", "3"}, {"4", "4"},
    {"5", "5"}, {"6", "6"}, {"7", "7"}, {"8", "8"},
    {"9", "9"}, {"0", "0"}, {"%", "%"}, {"", ""}
  };

  buf[0] = '\0';

  if (argument[0] == '\0')
    return argument;

  for (pName = argument; *pName != '\0'; pName += length)
  {
    for (iSyl = 0; (length = strlen(spk_table[iSyl].old)) != 0; iSyl++)
    {
      if (!str_prefix(spk_table[iSyl].old, pName))
      {
        strcat(buf, spk_table[iSyl].new);
        break;
      }
    }

    if (length == 0)
      length = 1;
  }

  argument[0] = '\0';
  strcpy(argument, buf);
  argument[0] = UPPER(argument[0]);

  return argument;
}

bool check_ignore(CHAR_DATA *ch, CHAR_DATA *victim)
{
  ITERATOR *pIter;

  if (!IS_NPC(ch) && !IS_NPC(victim) && ch->desc && ch->desc->account)
  {
    IGNORE_DATA *ignore;

    pIter = AllocIterator(victim->pcdata->ignores);
    while ((ignore = (IGNORE_DATA *) NextInList(pIter)) != NULL)
    {
      if (!str_cmp(ignore->account, ch->desc->account->owner))
        return TRUE;
    }
  }

  return FALSE;
}

IGNORE_DATA *alloc_ignore()
{
  IGNORE_DATA *ignore;

  if ((ignore = (IGNORE_DATA *) PopStack(ignore_free)) == NULL)
  {
    ignore = malloc(sizeof(*ignore));
  }

  ignore->player = str_dup("");
  ignore->account = str_dup("");

  return ignore;
}

void free_ignore(IGNORE_DATA *ignore, CHAR_DATA *ch)
{
  DetachFromList(ignore, ch->pcdata->ignores);

  free_string(ignore->player);
  free_string(ignore->account);

  PushStack(ignore, ignore_free);
}

/*
 * should probably make an mxp_act() as well, so we could have
 * mxp stuff happen on those global channels.
 */
void talk_channel(CHAR_DATA *ch, char *argument, int channel, int sub_channel, const char *verb)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  ITERATOR *pIter;
  KINGDOM_DATA *kingdom = NULL;
  DESCRIPTOR_DATA *d;

  buf3[0] = '\0';
  strcpy(buf2, " ");
  argument = replace_letter_with_word(argument, 'u', "you");

  if (!IS_NPC(ch) && (getMight(ch) < RANK_CADET || !IS_SET(ch->extra, EXTRA_PKREADY))
    && ch->level < 6 && channel == CHANNEL_CHAT)
  {
    /* Newbies are restricted to use the newbie channel */
    channel = CHANNEL_NEWBIE;
  }
  if (!IS_NPC(ch) && (get_age(ch) - 17) < 2 && channel == CHANNEL_FLAME)
  {
    send_to_char("You must be at least 4 hours old to use this channel.\n\r", ch);
    return;
  }
  if (argument[0] == '\0')
  {
    sprintf(buf, "%s what?\n\r", verb);
    buf[0] = UPPER(buf[0]);
    send_to_char(buf, ch);
    return;
  }
  if (IS_HEAD(ch, LOST_TONGUE))
  {
    sprintf(buf, "You can't %s without a tongue!\n\r", verb);
    send_to_char(buf, ch);
    return;
  }
  if (IS_EXTRA(ch, GAGGED))
  {
    sprintf(buf, "You can't %s with a gag on!\n\r", verb);
    send_to_char(buf, ch);
    return;
  }
  REMOVE_BIT(ch->deaf, channel);

  if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    argument = drunktalk(argument);

  switch (channel)
  {
    default:
      if (channel == CHANNEL_FLAME)
      {
        sprintf(buf, "You %s '#G%s#n'.\n\r", verb, argument);
        mxp_to_char(buf, ch, MXP_SAFE);
        sprintf(buf, "%s %ses '#G$t#n'.", ch->name, verb);
        sprintf(buf2, "#0[#GFl#0] #C%s", ch->name);
      }
      else if (channel == CHANNEL_CHAT)
      {
        sprintf(buf, "You %s '#R%s#n'.\n\r", verb, argument);
        mxp_to_char(buf, ch, MXP_SAFE);
        if (ch->trust > 6)
          sprintf(buf, "#0-^- #G%s #0-^-#n '#R$t#n'.", ch->name);
        else if (IS_NPC(ch))
          sprintf(buf, "%s chats '#R$t#n'.", ch->short_descr);
        else  
          sprintf(buf, "%s %ss '#R$t#n'.", ch->name, verb);
        sprintf(buf2, "#0[#GCh#0] #C%s", ch->name);

        if (!IS_NPC(ch) && ch->desc && IS_SET(ch->desc->account->flags, ACCOUNT_FLAG_CHANNELLOG))
          sprintf(buf3, "[CL] %s chats '%s'.", ch->name, argument);
      }
      else
      {
        sprintf(buf, "You %s '#R%s#n'.\n\r", verb, argument);
        mxp_to_char(buf, ch, MXP_SAFE);
        sprintf(buf, "%s %ss '#R$t#n'.", ch->name, verb);
        sprintf(buf2, "#0[#GCh#0] #C%s", ch->name);
      }
      break;
    case CHANNEL_IMMTALK:
      sprintf(buf, "#y.:#P%s#y:.#C $t.#n", ch->name);
      act(buf, ch, argument, NULL, TO_CHAR);
      sprintf(buf2, "#y.:#P%s#y:.", ch->name);
      break;
    case CHANNEL_CLASS:
      switch(sub_channel)
      {
        case CC_SHADOW:
          sprintf(buf, "#y-*(#0%s#y)*- #C'$t'.#n", ch->name);
          sprintf(buf2, "#y-*(#0%s#y)*-", ch->name);
          act(buf, ch, argument, NULL, TO_CHAR);
          break;
        case CC_WARLOCK:
          sprintf(buf, "#y/#R<#G-#P%s#G-#R>#y\\ #C'$t'.#n", ch->name);
          sprintf(buf2, "#y/#R<#G-#P%s#G-#R>#y\\", ch->name);
          act(buf, ch, argument, NULL, TO_CHAR);
          break;
        case CC_FAE:
          sprintf(buf, "#G>>#R(#y%s#R)#G<<#C '$t'.#n",ch->name);
          sprintf(buf2, "#G>>#R(#y%s#R)#G<<", ch->name);
          act(buf, ch, argument, NULL, TO_CHAR);
          break;
        case CC_GIANT:
          sprintf(buf, "#G<:>#o%s#G<:>#C '$t'.#n",ch->name);
          sprintf(buf2, "#G<:>#o%s#G<:>", ch->name);
          act(buf, ch, argument, NULL, TO_CHAR);
          break;
      }

      if (!IS_NPC(ch) && ch->desc && IS_SET(ch->desc->account->flags, ACCOUNT_FLAG_CHANNELLOG))
        sprintf(buf3, "[CL] %s classtalks '%s'.", ch->name, argument);

      break;      
    case CHANNEL_NEWBIE:
      sprintf(buf, "You %s '#R%s#n'.\n\r", verb, argument);
      mxp_to_char(buf, ch, MXP_SAFE);
      sprintf(buf, "%s the newbie chats #9'#R$t#9'.#n", ch->name);
      sprintf(buf2, "#0[#GCh#0] #C%s", ch->name);

      if (!IS_NPC(ch) && ch->desc && IS_SET(ch->desc->account->flags, ACCOUNT_FLAG_CHANNELLOG))
        sprintf(buf3, "[CL] %s chats '%s'.", ch->name, argument);

      break;
    case CHANNEL_KINGDOM:
      if ((kingdom = get_kingdom(ch)) == NULL)
      {
        send_to_char("You are not a member of any kingdom.\n\r", ch);
        return;
      }
      sprintf(buf, "%s%s%s #n'#C$t#n'.", kingdom->prefix, ch->name, kingdom->suffix);
      sprintf(buf2, "%s%s%s", kingdom->prefix, ch->name, kingdom->suffix);
      act(buf, ch, argument, NULL, TO_CHAR);

      if (!IS_NPC(ch) && ch->desc && IS_SET(ch->desc->account->flags, ACCOUNT_FLAG_CHANNELLOG))
        sprintf(buf3, "[CL] %s kingdomtalks '%s'.", ch->name, argument);

      break;
  }

  if (!IS_NPC(ch))
    update_history(ch, ch, buf2, argument, 0);

  /* silenced, and they don't know it :) */
  if (is_silenced(ch))
    return;

  /* anti spamming */
  if (!IS_NPC(ch) && (channel == CHANNEL_CHAT || channel == CHANNEL_NEWBIE))
  {
    if (!str_cmp(ch->pcdata->last_global, argument))
      return;

    free_string(ch->pcdata->last_global);
    ch->pcdata->last_global = str_dup(argument);
  }

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    CHAR_DATA *gch;
    bool note = FALSE;

    if (d->connected != CON_PLAYING)
    {
      if (d->connected >= CON_NOTE_TO && d->connected <= CON_NOTE_FINISH)
        note = TRUE;
      else
        continue;
    }
    if ((gch = d->character) == NULL) continue;
    if (gch == ch) continue;
    if (IS_SET(gch->deaf, channel)) continue;

    if (channel == CHANNEL_IMMTALK && !IS_IMMORTAL(gch)) continue;
    if (channel == CHANNEL_CLASS && !IS_IMMORTAL(gch) && (gch->class != ch->class || gch->level < 3))
      continue;
    if (channel == CHANNEL_YELL && gch->in_room && gch->in_room->area != ch->in_room->area)
      continue;
    if (channel == CHANNEL_KINGDOM && kingdom != get_kingdom(gch) && gch->level < 12)
      continue;
    if (check_ignore(ch, gch))
      continue;

    if (!IS_NPC(gch) && !IS_NPC(ch))
    {
      update_history(gch, ch, buf2, argument, 0);
    }

    if (!note)
    {
      int pos = gch->position;

      gch->position = POS_STANDING;
      act(buf, ch, argument, gch, TO_VICT);
      gch->position = pos;
    }
  }

  if (buf3[0] != '\0')
    log_string2("%s", buf3);
}

void do_history(CHAR_DATA *ch, char *argument)
{
  HISTORY_DATA *history;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];
  char fmt[MAX_INPUT_LENGTH];
  int pSize = 0, i = 1;
  bool found = FALSE;

  if (IS_NPC(ch)) return;

  if (ch->pcdata->history == NULL)
  {
    send_to_char("No history.\n\r", ch);
    return;
  }

  /* get max playersize */
  pIter = AllocIterator(ch->pcdata->history);
  while ((history = (HISTORY_DATA *) NextInList(pIter)) != NULL)
  {
    pSize = UMAX(pSize, collen(history->player));
  }

  /* create the format for the output */
  sprintf(fmt, " %%-%ds #0: %%s%%s#n\n\r", pSize);

  pIter = AllocIterator(ch->pcdata->history);
  while ((history = (HISTORY_DATA *) NextInList(pIter)) != NULL)
  {
    if (strlen(history->player) > 3)
    {
      found = TRUE;
      cprintf(buf, fmt, history->player, (i++ % 2) ? "#R" : "#C", line_indent(history->message, pSize + 4, 80));
      send_to_char(buf, ch);
    }
  }

  if (!found)
    send_to_char("No history.\n\r", ch);
}

void update_history(CHAR_DATA *ch, CHAR_DATA *talker, char *player, char *argument, int mode)
{
  char message[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) return;

  if (mode == -1)
    sprintf(message, "#0[#G->#0] #C%s", talker->name);
  else if (mode == 1)
    sprintf(message, "#0[#G<-#0] #C%s", talker->name);
  else
    strcpy(message, player);

  attach_history(ch, message, argument);
}

void attach_history(CHAR_DATA *ch, char *message, char *argument)
{
  HISTORY_DATA *history;

  if ((history = (HISTORY_DATA *) PopStack(history_free)) == NULL)
    history = malloc(sizeof(*history));

  history->player = str_dup(message);
  history->message = str_dup(strip_ansi(argument));

  AttachToList(history, ch->pcdata->history);

  if (SizeOfList(ch->pcdata->history) >= MAX_HISTORY)
  {
    if ((history = (HISTORY_DATA *) LastInList(ch->pcdata->history)) != NULL)
    {
      free_string(history->message);
      free_string(history->player);
      DetachFromList(history, ch->pcdata->history);
      PushStack(history, history_free);
    }
  }
}

/*
 * Will remove the last line in the current note,
 * if there is a note, and it has a line to remove.
 */
void delete_last_line_in_note(CHAR_DATA *ch)
{
  char buf[4 * MAX_STRING_LENGTH];
  char *ptr;
  bool found = FALSE;
  int nCount = 0;

  buf[0] = '\0';

  if (IS_NPC(ch)) return;
  if (ch->pcdata->in_progress->text == NULL)
  {
    send_to_char("No note to delete lines in.\n\r", ch);
    return;
  }
  if (strlen(ch->pcdata->in_progress->text) < 1)
  {
    send_to_char("Empty note, nothing to delete.\n\r", ch);
    return;
  }
  strcpy(buf, ch->pcdata->in_progress->text);
  ptr = buf;
  while (*ptr != '\0')
  {
    if (*ptr == '\n') nCount++;
    ptr++;
  }
  if (nCount == 1)
  {
    free_string(ch->pcdata->in_progress->text);
    ch->pcdata->in_progress->text = NULL;
    send_to_char("Entire note deleted.\n\r", ch);
    return;
  }
  else
  {
    while (*ptr != '\n' || !found)
    {
      if (*ptr == '\n') found = TRUE;
      ptr--;
    }
  }
  ptr++;
  *ptr = '\0';
  free_string(ch->pcdata->in_progress->text);
  ch->pcdata->in_progress->text = str_dup(buf);
  send_to_char("Line deleted.\n\r", ch);
}
