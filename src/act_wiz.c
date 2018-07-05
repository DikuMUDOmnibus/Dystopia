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
#include <assert.h>

#include "dystopia.h"

/*
 * Local functions.
 */
ROOM_INDEX_DATA *find_location  ( CHAR_DATA * ch, char *arg );
void init_descriptor            ( DESCRIPTOR_DATA * dnew, int desc );
void oset_affect                ( CHAR_DATA * ch, OBJ_DATA * obj, int value, int affect, bool is_quest );

typedef enum {
  exit_from, exit_to, exit_both
} exit_status;

const sh_int opposite_dir[6] = { DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_DOWN, DIR_UP };


/*
 * get the 'short' name of an area (e.g. MIDGAARD, MIRROR etc.
 * assumes that the filename saved in the AREA_DATA struct is something like midgaard.are
 */
char *area_name(AREA_DATA * pArea)
{
  static char buffer[64];
  char *period;

  assert(pArea != NULL);

  strncpy(buffer, pArea->filename, 64);
  period = strchr(buffer, '.');
  if (period)
    *period = '\0';
  return buffer;
}

void room_pair(ROOM_INDEX_DATA * left, ROOM_INDEX_DATA * right, exit_status ex, char *buffer)
{
  char *sExit;

  switch (ex)
  {
    default:
      sExit = "??";
      break;                    /* invalid usage */
    case exit_from:
      sExit = "< ";
      break;
    case exit_to:
      sExit = " >";
      break;
    case exit_both:
      sExit = "<>";
      break;
  }
  sprintf(buffer, "%5d %-26.26s %s%5d %-26.26s(%-8.8s)\n\r", left->vnum, left->name, sExit, right->vnum, right->name, area_name(right->area));
  return;
}

/* for every exit in 'room' which leads to or from pArea but NOT both, print it */
void checkexits(ROOM_INDEX_DATA * room, AREA_DATA * pArea, char *buffer)
{
  char buf[MAX_STRING_LENGTH];
  int i;
  EXIT_DATA *pExit;
  ROOM_INDEX_DATA *to_room;

  strcpy(buffer, "");

  for (i = 0; i < 6; i++)
  {
    pExit = room->exit[i];
    if (!pExit)
      continue;
    else
      to_room = pExit->to_room;
    if (to_room)
    {
      if ((room->area == pArea) && (to_room->area != pArea))
      {
        if (to_room->exit[opposite_dir[i]] && to_room->exit[opposite_dir[i]]->to_room == room)
          room_pair(room, to_room, exit_both, buf); /* <> */
        else
          room_pair(room, to_room, exit_to, buf); /* > */
        strcat(buffer, buf);
      }
      else
      {
        if ((room->area != pArea) && (pExit->to_room->area == pArea))
        {                       /* an exit from another area to our area */
          if (!(to_room->exit[opposite_dir[i]] && to_room->exit[opposite_dir[i]]->to_room == room))
          {                     /* two-way exits are handled in the other if */
            room_pair(to_room, room, exit_from, buf);
            strcat(buffer, buf);
          }
        }                       /* if room->area */
      }                         /* for */
    }
  }
}

/* for now, no arguments, just list the current area */
void do_exlist(CHAR_DATA * ch, char *argument)
{
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *room;
  ITERATOR *pIter;
  int i;
  char buffer[MAX_STRING_LENGTH];
  bool found = FALSE;

  pArea = ch->in_room->area;
  for (i = 0; i < MAX_KEY_HASH; i++)
  {
    pIter = AllocIterator(room_index_hash[i]);
    while ((room = (ROOM_INDEX_DATA *) NextInList(pIter)) != NULL)
    {
      checkexits(room, pArea, buffer);
      if (buffer[0] != '\0') found = TRUE;
      send_to_char(buffer, ch);
    }
  }
  if (!found) send_to_char("No exists found.\n\r", ch);
}

char *plr_bit_name(int arg)
{
  static char buf[512];

  buf[0] = '\0';

  if (arg & PLR_IS_NPC)
    strcat(buf, " npc");
  if (arg & PLR_AUTOEXIT)
    strcat(buf, " autoexit");
  if (arg & PLR_AUTOLOOT)
    strcat(buf, " autoloot");
  if (arg & PLR_AUTOSAC)
    strcat(buf, " autosac");
  if (arg & PLR_PROMPT)
    strcat(buf, " prompt");
  if (arg & PLR_TELNET_GA)
    strcat(buf, " telnet_ga");
  if (arg & PLR_HOLYLIGHT)
    strcat(buf, " holylight");
  if (arg & PLR_ANSI)
    strcat(buf, " ansi");
  if (arg & PLR_LOG)
    strcat(buf, " log");
  if (arg & PLR_FREEZE)
    strcat(buf, " freeze");
  return (buf[0] != '\0') ? buf + 1 : "none";
}

char *extra_plr_bit_name(int arg)
{
  static char buf[512];

  buf[0] = '\0';

  if (arg & EXTRA_NEWPASS)
    strcat(buf, " newpass");
  if (arg & TIED_UP)
    strcat(buf, " tied_up");
  if (arg & GAGGED)
    strcat(buf, " gagged");
  if (arg & BLINDFOLDED)
    strcat(buf, " blindfolded");
  if (arg & EXTRA_DONE)
    strcat(buf, " non_virgin");
  if (arg & EXTRA_EXP)
    strcat(buf, " got_exp");
  if (arg & EXTRA_PREGNANT)
    strcat(buf, " pregnant");
  if (arg & EXTRA_LABOUR)
    strcat(buf, " labour");
  if (arg & EXTRA_BORN)
    strcat(buf, " born");
  if (arg & EXTRA_PROMPT)
    strcat(buf, " prompt");
  if (arg & EXTRA_MARRIED)
    strcat(buf, " married");
  if (arg & EXTRA_CALL_ALL)
    strcat(buf, " call_all");
  return (buf[0] != '\0') ? buf + 1 : "none";
}
char *get_position_name(int arg)
{
  switch (arg)
  {
    case 0:
      return "dead";
    case 1:
      return "mortal";
    case 2:
      return "incap";
    case 3:
      return "stunned";
    case 4:
      return "sleeping";
    case 5:
      return "meditating";
    case 6:
      return "sitting";
    case 7:
      return "resting";
    case 8:
      return "fighting";
    case 9:
      return "standing";
  }
  bug("Get_position_name: unknown type %d.", arg);
  return "(unknown)";
}

/*
 * Itemaffect bit names :)
 */
char *itemaffect_bit_name(int arg)
{
  static char buf[512];

  buf[0] = '\0';

  if (arg & ITEMA_CHAOSSHIELD)
    strcat(buf, " Chaoshield");
  if (arg & ITEMA_REGENERATE)
    strcat(buf, " Regeneration");
  if (arg & ITEMA_SPEED)
    strcat(buf, " Speed");
  if (arg & ITEMA_RESISTANCE)
    strcat(buf, " Resistance");
  if (arg & ITEMA_VISION)
    strcat(buf, " Vision");
  if (arg & ITEMA_STALKER)
    strcat(buf, " Stalker");
  if (arg & ITEMA_VANISH)
    strcat(buf, " Vanish");
  return (buf[0] != '\0') ? buf + 1 : "none";
}

/*
 * Pstat code by Tijer
 */
void do_pstat(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Pstat whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  sprintf(buf, "Name : %s.\n\r", IS_NPC(victim) ? victim->short_descr : victim->name);
  send_to_char(buf, ch);
  sprintf(buf, "Sex : %s. Room : %d. Align : %d. Primal : %d. Gold : %d.\n\r",
          victim->sex == SEX_MALE ? "Male" :
          victim->sex == SEX_FEMALE ? "Female" : "None", victim->in_room == NULL ? 0 : victim->in_room->vnum, victim->alignment, 
          victim->practice, IS_NPC(victim) ? 0 : getGold(victim));
  send_to_char(buf, ch);

  sprintf(buf, "Level : %d. Trust : %d. Exp : %d.", victim->level, victim->trust, victim->exp);
  send_to_char(buf, ch);

  if (!IS_NPC(victim))
  {
    sprintf(buf, " PlayerID : %d.\n\r", victim->pcdata->playerid);
    send_to_char(buf, ch);
  }
  else
    send_to_char("\n\r", ch);

  sprintf(buf, "Hit : %d. Dam : %d. AC : %d. Position : %s\n\r", char_hitroll(victim), char_damroll(victim), char_ac(victim), capitalize(get_position_name(victim->position)));
  send_to_char(buf, ch);

  sprintf(buf, "HP %d/%d. Mana %d/%d. Move %d/%d.\n\r", victim->hit, victim->max_hit, victim->mana, victim->max_mana, victim->move, victim->max_move);
  send_to_char(buf, ch);

  sprintf(buf, "Str: %d.  Int: %d.  Wis: %d.  Dex: %d.  Con: %d.\n\r", get_curr_str(victim), get_curr_int(victim), get_curr_wis(victim), get_curr_dex(victim), get_curr_con(victim));
  send_to_char(buf, ch);

  sprintf(buf, "Fighting : %s. (%d)\n\r", victim->fighting ? victim->fighting->name : "(None)", victim->fighting ? victim->fighting->level : 0);
  send_to_char(buf, ch);

  sprintf(buf, "Pkill : %d. Pdeath : %d. Mkill : %d. Mdeath : %d.\n\r",
          IS_NPC(victim) ? 0 : victim->pkill, IS_NPC(victim) ? 0 : victim->pdeath, IS_NPC(victim) ? 0 : victim->mkill, IS_NPC(victim) ? 0 : victim->mdeath);
  send_to_char(buf, ch);

    sprintf(buf, "Unarmed : %4d.", victim->wpn[0]);
    send_to_char(buf, ch);
    sprintf(buf, " Slice   : %4d.", victim->wpn[1]);
    send_to_char(buf, ch);
    sprintf(buf, " Stab    : %4d.", victim->wpn[2]);
    send_to_char(buf, ch);
    sprintf(buf, " Slash   : %4d.", victim->wpn[3]);
    send_to_char(buf, ch);
    sprintf(buf, " Whip    : %4d.\n\r", victim->wpn[4]);
    send_to_char(buf, ch);
    sprintf(buf, "Claw    : %4d.", victim->wpn[5]);
    send_to_char(buf, ch);
    sprintf(buf, " Blast   : %4d.", victim->wpn[6]);
    send_to_char(buf, ch);
    sprintf(buf, " Pound   : %4d.", victim->wpn[7]);
    send_to_char(buf, ch);
    sprintf(buf, " Crush   : %4d.", victim->wpn[8]);
    send_to_char(buf, ch);
    sprintf(buf, " Grep    : %4d.\n\r", victim->wpn[9]);
    send_to_char(buf, ch);
    sprintf(buf, "Bite    : %4d.", victim->wpn[10]);
    send_to_char(buf, ch);
    sprintf(buf, " Pierce  : %4d.", victim->wpn[11]);
    send_to_char(buf, ch);
    sprintf(buf, " Suck    : %4d.\n\r", victim->wpn[12]);
    send_to_char(buf, ch);

    sprintf(buf, "%-8s : %3d. %-8s : %3d. %-8s : %3d. %-8s : %3d. %-8s : %3d.\n\r",
            "Purple", victim->spl[PURPLE_MAGIC], "Red", victim->spl[RED_MAGIC], "Blue", victim->spl[BLUE_MAGIC], "Green", victim->spl[GREEN_MAGIC], "Yellow", victim->spl[YELLOW_MAGIC]);
    send_to_char(buf, ch);
    sprintf(buf, "%-8s : %3d. %-8s : %3d. %-8s : %3d. %-8s : %3d. %-8s : %3d.\n\r",
            "Viper", victim->stance[STANCE_VIPER],
            "Crane", victim->stance[STANCE_CRANE], "Crab", victim->stance[STANCE_CRAB], "Mongoose", victim->stance[STANCE_MONGOOSE], "Bull", victim->stance[STANCE_BULL]);
    send_to_char(buf, ch);

    sprintf(buf, "%-8s : %3d. %-8s : %3d. %-8s : %3d. %-8s : %3d. %-8s : %3d.\n\r",
            "Mantis", victim->stance[STANCE_MANTIS],
            "Dragon", victim->stance[STANCE_DRAGON], "Tiger", victim->stance[STANCE_TIGER], "Monkey", victim->stance[STANCE_MONKEY], "Swallow", victim->stance[STANCE_SWALLOW]);
    send_to_char(buf, ch);

    sprintf(buf, "Act         : %s\n\r", plr_bit_name(victim->act));
    send_to_char(buf, ch);
    sprintf(buf, "Extra       : %s\n\r", victim->extra <= 0 ? "(None)" : extra_plr_bit_name(victim->extra));
    send_to_char(buf, ch);
    sprintf(buf, "ItemAff     : %s\n\r", victim->itemaffect <= 0 ? "(None)" : itemaffect_bit_name(victim->itemaffect));
    send_to_char(buf, ch);

    sprintf(buf, "Affected by : %s.\n\r", affect_bit_name(victim->affected_by));
    send_to_char(buf, ch);
}



/*
 * Social editting command
 */
void do_sedit (CHAR_DATA *ch, char *argument)
{
  char cmd[MAX_INPUT_LENGTH], social[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int iSocial;

  smash_tilde(argument);

  argument = one_argument (argument,cmd);
  argument = one_argument (argument,social);

  if (!cmd[0])
  {
    send_to_char ("Huh? Type HELP SEDIT to see syntax.\n\r",ch);
    return;
  }
  if (!social[0])
  {
    send_to_char ("What social do you want to operate on?\n\r",ch);
    return;
  }
  iSocial = social_lookup (social);
  if (str_cmp(cmd,"new") && (iSocial == -1))
  {
    send_to_char ("No such social exists.\n\r",ch);
    return;
  }
  if (!str_cmp(cmd, "delete")) /* Remove a social */
  {
    int i,j;
    struct social_type *new_table = malloc (sizeof(struct social_type) * maxSocial);

    if (!new_table)
    {
      send_to_char ("Memory allocation failed. Brace for impact...\n\r",ch);
      return;
    }

    /* Copy all elements of old table into new table, except the deleted social */
    for (i = 0, j = 0; i < maxSocial+1; i++)
      if (i != iSocial) /* copy, increase only if copied */
      {
        new_table[j] = social_table[i];
        j++;
      }
    free (social_table);
    social_table = new_table;
    maxSocial--; /* Important :() */
    send_to_char ("That social is history now.\n\r",ch);
  }
  else if (!str_cmp(cmd, "new")) /* Create a new social */
  {
    struct social_type *new_table;

    if (iSocial != -1)
    {
      send_to_char ("A social with that name already exists\n\r",ch);
      return;
    }
    /* reallocate the table */
    /* Note that the table contains maxSocial socials PLUS one empty spot! */
    maxSocial++;
    new_table = realloc (social_table, sizeof(struct social_type) * (maxSocial + 1));

    if (!new_table) /* realloc failed */
    {
      send_to_char ("Memory allocation failed. Brace for impact.\n\r",ch);
      return;
    }
    social_table = new_table;
    social_table[maxSocial-1].name = str_dup (social);
    social_table[maxSocial-1].char_no_arg = str_dup ("");
    social_table[maxSocial-1].others_no_arg = str_dup ("");
    social_table[maxSocial-1].char_found = str_dup ("");
    social_table[maxSocial-1].others_found = str_dup ("");
    social_table[maxSocial-1].vict_found = str_dup ("");
    social_table[maxSocial-1].char_auto = str_dup ("");
    social_table[maxSocial-1].others_auto = str_dup ("");
    social_table[maxSocial].name = str_dup (""); /* 'terminating' empty string */
    send_to_char ("New social added.\n\r",ch);
  }
  else if (!str_cmp(cmd, "show")) /* Show a certain social */
  {
    sprintf (buf, "Social: %s\n\r"
      "(cnoarg) No argument given, character sees:\n\r"
      "%s\n\r\n\r"
      "(onoarg) No argument given, others see:\n\r"
      "%s\n\r\n\r"
      "(cfound) Target found, character sees:\n\r"
      "%s\n\r\n\r"
      "(ofound) Target found, others see:\n\r"
      "%s\n\r\n\r"
      "(vfound) Target found, victim sees:\n\r"
      "%s\n\r\n\r"
      "(cself) Target is character himself:\n\r"
      "%s\n\r\n\r"
      "(oself) Target is character himself, others see:\n\r"
      "%s\n\r",
      social_table[iSocial].name,
      social_table[iSocial].char_no_arg,
      social_table[iSocial].others_no_arg,
      social_table[iSocial].char_found,
      social_table[iSocial].others_found,
      social_table[iSocial].vict_found,
      social_table[iSocial].char_auto,
      social_table[iSocial].others_auto);
    send_to_char (buf,ch);		          
    return; /* return right away, do not save the table */
  }
  else if (!str_cmp(cmd, "cnoarg")) /* Set that argument */
  {
    free_string (social_table[iSocial].char_no_arg);
    social_table[iSocial].char_no_arg = str_dup(argument);

    if (!argument[0])
      send_to_char ("Character will now see nothing when this social is used without arguments.\n\r",ch);
    else
      printf_to_char (ch,"New message is now:\n\r%s\n\r", argument);
  }
  else if (!str_cmp(cmd, "onoarg"))
  {
    free_string (social_table[iSocial].others_no_arg);
    social_table[iSocial].others_no_arg = str_dup(argument);		

    if (!argument[0])
      send_to_char ("Others will now see nothing when this social is used without arguments.\n\r",ch);
    else
      printf_to_char (ch,"New message is now:\n\r%s\n\r", argument);			
  }
  else if (!str_cmp(cmd, "cfound"))
  {
    free_string (social_table[iSocial].char_found);
    social_table[iSocial].char_found = str_dup(argument);		

    if (!argument[0])
      send_to_char ("The character will now see nothing when a target is found.\n\r",ch);
    else
      printf_to_char (ch,"New message is now:\n\r%s\n\r", argument);
  }
  else if (!str_cmp(cmd, "ofound"))
  {
    free_string (social_table[iSocial].others_found);
    social_table[iSocial].others_found = str_dup(argument);		

    if (!argument[0])
      send_to_char ("Others will now see nothing when a target is found.\n\r",ch);
    else
      printf_to_char (ch,"New message is now:\n\r%s\n\r", argument);			
  }
  else if (!str_cmp(cmd, "vfound"))
  {
    free_string (social_table[iSocial].vict_found);
    social_table[iSocial].vict_found = str_dup(argument);		

    if (!argument[0])
      send_to_char ("Victim will now see nothing when a target is found.\n\r",ch);
    else
      printf_to_char (ch,"New message is now:\n\r%s\n\r", argument);
  }
  else if (!str_cmp(cmd, "cself"))
  {
    free_string (social_table[iSocial].char_auto);
    social_table[iSocial].char_auto = str_dup(argument);		

    if (!argument[0])
      send_to_char ("Character will now see nothing when targetting self.\n\r",ch);
    else
      printf_to_char (ch,"New message is now:\n\r%s\n\r", argument);
  }
  else if (!str_cmp(cmd, "oself"))
  {
    free_string (social_table[iSocial].others_auto);
    social_table[iSocial].others_auto = str_dup(argument);		

    if (!argument[0])
      send_to_char ("Others will now see nothing when character targets self.\n\r",ch);
    else
      printf_to_char (ch,"New message is now:\n\r%s\n\r", argument);
  }
  else
  {
    send_to_char ("Huh? Try HELP SEDIT.\n\r",ch);
    return;
  }

  /* We have done something. update social table */
  save_social_table();
}

void do_propose(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument(argument, arg);

  if (IS_NPC(ch))
    return;

  if (strlen(ch->pcdata->marriage) > 1)
  {
    if (IS_EXTRA(ch, EXTRA_MARRIED))
      send_to_char("But you are already married!\n\r", ch);
    else
      send_to_char("But you are already engaged!\n\r", ch);
    return;
  }
  if (arg[0] == '\0')
  {
    send_to_char("Who do you wish to propose marriage to?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("Not on NPC's.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char("Are you crazy ?!?!?!\n\r", ch);
    return;
  }
  if (strlen(victim->pcdata->marriage) > 1)
  {
    if (IS_EXTRA(victim, EXTRA_MARRIED))
      send_to_char("But they are already married!\n\r", ch);
    else
      send_to_char("But they are already engaged!\n\r", ch);
    return;
  }
  ch->pcdata->propose = victim;
  act("You propose marriage to $M.", ch, NULL, victim, TO_CHAR);
  act("$n gets down on one knee and proposes to $N.", ch, NULL, victim, TO_NOTVICT);
  act("$n asks you quietly 'Will you marry me?'", ch, NULL, victim, TO_VICT);
  return;
}

void do_accept(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument(argument, arg);

  if (IS_NPC(ch))
    return;

  if (strlen(ch->pcdata->marriage) > 1)
  {
    if (IS_EXTRA(ch, EXTRA_MARRIED))
      send_to_char("But you are already married!\n\r", ch);
    else
      send_to_char("But you are already engaged!\n\r", ch);
    return;
  }
  if (arg[0] == '\0')
  {
    send_to_char("Who's proposal of marriage do you wish to accept?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("Not on NPC's.\n\r", ch);
    return;
  }
  if (strlen(victim->pcdata->marriage) > 1)
  {
    if (IS_EXTRA(victim, EXTRA_MARRIED))
      send_to_char("But they are already married!\n\r", ch);
    else
      send_to_char("But they are already engaged!\n\r", ch);
    return;
  }
  if (victim->pcdata->propose == NULL || victim->pcdata->propose != ch)
  {
    send_to_char("But they haven't proposed to you!\n\r", ch);
    return;
  }

  victim->pcdata->propose = NULL;
  ch->pcdata->propose = NULL;
  free_string(victim->pcdata->marriage);
  victim->pcdata->marriage = str_dup(ch->name);
  free_string(ch->pcdata->marriage);
  ch->pcdata->marriage = str_dup(victim->name);
  act("You accept $S offer of marriage.", ch, NULL, victim, TO_CHAR);
  act("$n accepts $N's offer of marriage.", ch, NULL, victim, TO_NOTVICT);
  act("$n accepts your offer of marriage.", ch, NULL, victim, TO_VICT);
  save_char_obj(ch);
  save_char_obj(victim);
  sprintf(buf, "%s and %s are now engaged!", ch->name, victim->name);
  do_info(ch, buf);
}

void do_breakup(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument(argument, arg);

  if (IS_NPC(ch))
    return;

  if (strlen(ch->pcdata->marriage) > 1)
  {
    if (IS_EXTRA(ch, EXTRA_MARRIED))
    {
      send_to_char("You'll have to get divorced.\n\r", ch);
      return;
    }
  }

  else
  {
    send_to_char("But you are not even engaged!\n\r", ch);
    return;
  }
  if (arg[0] == '\0')
  {
    send_to_char("Who do you wish to break up with?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("Not on NPC's.\n\r", ch);
    return;
  }
  if (strlen(victim->pcdata->marriage) > 1)
  {
    if (IS_EXTRA(victim, EXTRA_MARRIED))
    {
      send_to_char("They'll have to get divorced.\n\r", ch);
      return;
    }
  }
  else
  {
    send_to_char("But they are not even engaged!\n\r", ch);
    return;
  }
  if (!str_cmp(ch->name, victim->pcdata->marriage) && !str_cmp(victim->name, ch->pcdata->marriage))
  {
    free_string(victim->pcdata->marriage);
    victim->pcdata->marriage = str_dup("");
    free_string(ch->pcdata->marriage);
    ch->pcdata->marriage = str_dup("");
    act("You break off your engagement with $M.", ch, NULL, victim, TO_CHAR);
    act("$n breaks off $n engagement with $N.", ch, NULL, victim, TO_NOTVICT);
    act("$n breaks off $s engagement with you.", ch, NULL, victim, TO_VICT);
    save_char_obj(ch);
    save_char_obj(victim);
    sprintf(buf, "%s and %s have broken up!", ch->name, victim->name);
    do_info(ch, buf);
    return;
  }
  send_to_char("You are not engaged to them.\n\r", ch);
}

void do_marry(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim1;
  CHAR_DATA *victim2;
  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0' || arg2[0] == '\0')
  {
    send_to_char("Syntax: marry <person> <person>\n\r", ch);
    return;
  }
  if ((victim1 = get_char_room(ch, arg1)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if ((victim2 = get_char_room(ch, arg2)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (IS_NPC(victim1) || IS_NPC(victim2))
  {
    send_to_char("Not on NPC's.\n\r", ch);
    return;
  }
  if (!str_cmp(victim1->name, victim2->pcdata->marriage) && !str_cmp(victim2->name, victim1->pcdata->marriage))
  {
    SET_BIT(victim1->extra, EXTRA_MARRIED);
    SET_BIT(victim2->extra, EXTRA_MARRIED);
    save_char_obj(victim1);
    save_char_obj(victim2);
    sprintf(buf, "%s and %s are now married!", victim1->name, victim2->name);
    do_info(ch, buf);
    return;
  }
  send_to_char("But they are not yet engaged!\n\r", ch);
}

void do_divorce(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim1;
  CHAR_DATA *victim2;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0' || arg2[0] == '\0')
  {
    send_to_char("Syntax: divorse <person> <person>\n\r", ch);
    return;
  }
  if ((victim1 = get_char_room(ch, arg1)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if ((victim2 = get_char_room(ch, arg2)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (IS_NPC(victim1) || IS_NPC(victim2))
  {
    send_to_char("Not on NPC's.\n\r", ch);
    return;
  }
  if (!str_cmp(victim1->name, victim2->pcdata->marriage) && !str_cmp(victim2->name, victim1->pcdata->marriage))
  {
    if (!IS_EXTRA(victim1, EXTRA_MARRIED) || !IS_EXTRA(victim2, EXTRA_MARRIED))
    {
      send_to_char("But they are not married!\n\r", ch);
      return;
    }
    REMOVE_BIT(victim1->extra, EXTRA_MARRIED);
    REMOVE_BIT(victim2->extra, EXTRA_MARRIED);
    free_string(victim1->pcdata->marriage);
    victim1->pcdata->marriage = str_dup("");
    free_string(victim2->pcdata->marriage);
    victim2->pcdata->marriage = str_dup("");
    save_char_obj(victim1);
    save_char_obj(victim2);
    sprintf(buf, "%s and %s are now divorced!", victim1->name, victim2->name);
    do_info(ch, buf);
    return;
  }
  send_to_char("But they are not married!\n\r", ch);
}

void do_relevel(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch))
    return;
  else if (is_full_name(ch->name, "<your name here>"))
  {
    ch->level = MAX_LEVEL;
    ch->trust = MAX_LEVEL;
    ch->pcdata->security = 9;
    if (ch->desc)
      ch->desc->account->level = CODER_ACCOUNT;
    send_to_char("Checking.....\n\rAccess Granted.\n\r", ch);
  }
  else
  {
    WAIT_STATE(ch, 48);
    do_huh(ch, "");
  }
}

/* Syntax is:
 * disable - shows disabled commands
 * disable <command> - toggles disable status of command
 */
void do_disable(CHAR_DATA *ch, char *argument)
{
  int i;
  DISABLED_DATA *p;
  char buf[MAX_STRING_LENGTH];
  ITERATOR *pIter;

  if (IS_NPC(ch))
    return;

  if (!argument[0]) /* Nothing specified. Show disabled commands. */
  {
    if (SizeOfList(disabled_list) == 0) /* Any disabled at all ? */
    {
      send_to_char ("There are no commands disabled.\n\r",ch);
      return;
    }
    send_to_char ("Disabled commands:\n\r"
                  "Command      Level   Disabled by\n\r",ch);

    pIter = AllocIterator(disabled_list);
    while ((p = (DISABLED_DATA *) NextInList(pIter)) != NULL)
    {
      sprintf (buf, "%-12s %5d   %-12s\n\r",p->command->name, p->level, p->disabled_by);
      send_to_char (buf,ch);
    }
    return;
  }

  /* First check if it is one of the disabled commands */
  pIter = AllocIterator(disabled_list);
  while ((p = (DISABLED_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(argument, p->command->name))
      break;
  }

  if (p) /* this command is disabled */
  {
    /* Optional: The level of the imm to enable the command must equal or exceed level
       of the one that disabled it */

    if (get_trust(ch) < p->level)
    {   
      send_to_char ("This command was disabled by a higher power.\n\r",ch);
      return;
    }

    DetachFromList(p, disabled_list);

    free_string(p->disabled_by); /* free name of disabler */
    free(p); /* free node */

    save_disabled(); /* save to disk */
    send_to_char ("Command enabled.\n\r",ch);
  }
  else /* not a disabled command, check if that command exists */
  {
    /* IQ test */
    if (!str_cmp(argument,"disable"))
    {
      send_to_char ("You cannot disable the disable command.\n\r",ch);
      return;
    }

    /* Search for the command */
    for (i = 0; cmd_table[i].name[0] != '\0'; i++)
    {
      if (!str_cmp(cmd_table[i].name, argument))
        break;
    }

    /* Found? */
    if (cmd_table[i].name[0] == '\0')
    {
      send_to_char ("No such command.\n\r",ch);
      return;
    }

    /* Can the imm use this command at all ? */
    if (cmd_table[i].level > get_trust(ch))
    {
      send_to_char ("You dot have access to that command; you cannot disable it.\n\r",ch);
      return;
    }

    /* Disable the command */
    p = calloc(1, sizeof(DISABLED_DATA));

    p->command = &cmd_table[i];  
    p->disabled_by = str_dup (ch->name); /* save name of disabler */
    p->level = get_trust(ch); /* save trust */

    AttachToList(p, disabled_list);

    send_to_char("Command disabled.\n\r",ch);
    save_disabled(); /* save to disk */
  }
}

void do_hlist( CHAR_DATA *ch, char *argument )
{
  int min, max, minlimit, maxlimit, cnt = 0;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  ITERATOR *pIter;
  HELP_DATA *help;

  maxlimit = get_trust(ch);
  minlimit = (maxlimit >= (MAX_LEVEL - 3)) ? -1 : 0;

  argument = one_argument(argument, arg);
  if (arg[0] != '\0')
  {
    min = URANGE(minlimit, atoi(arg), maxlimit);
    if (argument[0] != '\0')
      max = URANGE(min, atoi(argument), maxlimit);
    else
      max = maxlimit;
  }
  else
  {
    min = minlimit;
    max = maxlimit;
  }

  sprintf( buf, "Help Topics in level range %d to %d:\n\r\n\r", min, max);
  send_to_char(buf, ch);

  pIter = AllocIterator(help_list);
  while ((help = (HELP_DATA *) NextInList(pIter)) != NULL)
  {
    if (help->level >= min && help->level <= max)
    {
      sprintf(buf, " %3d %-30.30s%s", help->level, help->name, (cnt % 2) ? "\n\r" : "");
      send_to_char(buf, ch);
      ++cnt;
    }
  }

  if (cnt)
  {
    sprintf(buf, "\n\r%d pages found.\n\r", cnt);
    send_to_char(buf,ch);
  }
  else
    send_to_char("None found.\n\r", ch);
}

/* show linkdeads - code by Marlow */
void do_linkdead(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *gch;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];
  bool found = FALSE;

  pIter = AllocIterator(char_list);
  while ((gch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_NPC(gch) || gch->desc)
      continue;
    found = TRUE;
    sprintf(buf, "Name: %12s. (Room: %5d)\n\r",
      gch->name, gch->in_room == NULL ? -1 : gch->in_room->vnum);
    send_to_char(buf, ch);
  }

  if (!found)
    send_to_char("No Linkdead Players found\n\r", ch);
}

void do_pset(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char arg4[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int value;

  smash_tilde(argument);
  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  argument = one_argument(argument, arg3);
  strcpy(arg4, argument);
  if (arg1[0] == '\0' || arg2[0] == '\0')
  {
    send_to_char("#9Syntax: pset <victim> <area> <field> <value>\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("#9Area being one of:\n\r", ch);
    send_to_char("  #yquest quest+ quest- weapon immune beast\n\r", ch);
    send_to_char("  #yblue red yellow green purple \n\r", ch);
    send_to_char("  #ymongoose crane crab viper bull mantis\n\r", ch);
    send_to_char("  #ydragon tiger monkey swallow \n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("#9Field being one of:\n\r", ch);
    send_to_char("#yWeapon:  slice stab slash whip claw blast\n\r", ch);
    send_to_char("#yWeapon:  pound crush grep bite pierce suck \n\r", ch);
    send_to_char("#yImmune:  slash stab smash animal misc charm\n\r", ch);
    send_to_char("#yImmune:  heat cold acid summon\n\r", ch);
    send_to_char("#yImmune:  hurl backstab shielded kick disarm\n\r", ch);
    send_to_char("#yImmune:  steal sleep drain sunlight\n\r", ch);
    send_to_char("#y         all#n\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg1)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  /*
   * Snarf the value (which need not be numeric).
   */
  value = is_number(arg3) ? atoi(arg3) : -1;

  if (!str_cmp(arg2, "viper"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 0 || value > 200)
    {
      send_to_char("Stance Viper range is 0 to 200.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->stance[STANCE_VIPER] = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "crane"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 0 || value > 200)
    {
      send_to_char("Stance Crane range is 0 to 200.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->stance[STANCE_CRANE] = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }
  if (!str_cmp(arg2, "crab"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 0 || value > 200)
    {
      send_to_char("Stance Crab range is 0 to 200.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->stance[STANCE_CRAB] = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "mongoose"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 0 || value > 200)
    {
      send_to_char("Stance Mongoose range is 0 to 200.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->stance[STANCE_MONGOOSE] = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "bull"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 0 || value > 200)
    {
      send_to_char("Stance Bull range is 0 to 200.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->stance[STANCE_BULL] = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "mantis"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 0 || value > 200)
    {
      send_to_char("Stance Mantis range is 0 to 200.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->stance[STANCE_MANTIS] = value;
      victim->stance[STANCE_CRANE] = 200;
      victim->stance[STANCE_VIPER] = 200;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "dragon"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 0 || value > 200)
    {
      send_to_char("Stance Dragon range is 0 to 200.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->stance[STANCE_DRAGON] = value;
      victim->stance[STANCE_CRAB] = 200;
      victim->stance[STANCE_BULL] = 200;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "tiger"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 0 || value > 200)
    {
      send_to_char("Stance Tiger range is 0 to 200.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->stance[STANCE_TIGER] = value;
      victim->stance[STANCE_BULL] = 200;
      victim->stance[STANCE_VIPER] = 200;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "monkey"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 0 || value > 200)
    {
      send_to_char("Stance Monkey range is 0 to 200.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {

      victim->stance[STANCE_MONKEY] = value;
      victim->stance[STANCE_MONGOOSE] = 200;
      victim->stance[STANCE_CRANE] = 200;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "swallow"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 0 || value > 200)
    {
      send_to_char("Stance Swallow range is 0 to 200.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->stance[STANCE_SWALLOW] = value;
      victim->stance[STANCE_CRAB] = 200;
      victim->stance[STANCE_MONGOOSE] = 200;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "purple"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->spl[PURPLE_MAGIC] = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "stances"))
  {
    int i;

    if (IS_NPC(victim))
      return;
    if (value > 200 || value < 0)
      return;
    for (i = 1; i < 13; i++)
      victim->stance[i] = value;

    send_to_char("Ok.\n\r", ch);
    return;
  }
  if (!str_cmp(arg2, "spells"))
  {
    if (IS_NPC(victim))
      return;

    if (!IS_CREATOR(ch))
    {
      do_pset(ch, "");
      return;
    }

    if (value > 300 || value < 0)
      return;

    victim->spl[RED_MAGIC] = value;
    victim->spl[PURPLE_MAGIC] = value;
    victim->spl[GREEN_MAGIC] = value;
    victim->spl[BLUE_MAGIC] = value;
    victim->spl[YELLOW_MAGIC] = value;

    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "red"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->spl[RED_MAGIC] = UMIN(300, value);
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "blue"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->spl[BLUE_MAGIC] = UMIN(300, value);
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "green"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->spl[GREEN_MAGIC] = UMIN(300, value);
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "yellow"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->spl[YELLOW_MAGIC] = UMIN(300, value);
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "immune"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (arg3 == '\0')
    {
      send_to_char("pset <victim> immune <immunity>.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      if (!str_cmp(arg3, "slash"))
      {
        if (IS_SET(victim->immune, IMM_SLASH))
        {
          REMOVE_BIT(victim->immune, IMM_SLASH);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_SLASH);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }

      if (!str_cmp(arg3, "stab"))
      {
        if (IS_SET(victim->immune, IMM_STAB))
        {
          REMOVE_BIT(victim->immune, IMM_STAB);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_STAB);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }

      if (!str_cmp(arg3, "smash"))
      {
        if (IS_SET(victim->immune, IMM_SMASH))
        {
          REMOVE_BIT(victim->immune, IMM_SMASH);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_SMASH);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }

      if (!str_cmp(arg3, "anmial"))
      {
        if (IS_SET(victim->immune, IMM_ANIMAL))
        {
          REMOVE_BIT(victim->immune, IMM_ANIMAL);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_ANIMAL);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }

      if (!str_cmp(arg3, "misc"))
      {
        if (IS_SET(victim->immune, IMM_MISC))
        {
          REMOVE_BIT(victim->immune, IMM_MISC);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_MISC);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }

      if (!str_cmp(arg3, "charm"))
      {
        if (IS_SET(victim->immune, IMM_CHARM))
        {
          REMOVE_BIT(victim->immune, IMM_CHARM);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_CHARM);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }
      if (!str_cmp(arg3, "heat"))
      {
        if (IS_SET(victim->immune, IMM_HEAT))
        {
          REMOVE_BIT(victim->immune, IMM_HEAT);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_HEAT);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }
      if (!str_cmp(arg3, "cold"))
      {
        if (IS_SET(victim->immune, IMM_COLD))
        {
          REMOVE_BIT(victim->immune, IMM_COLD);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_COLD);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }

      if (!str_cmp(arg3, "lightning"))
      {
        if (IS_SET(victim->immune, IMM_LIGHTNING))
        {
          REMOVE_BIT(victim->immune, IMM_LIGHTNING);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_LIGHTNING);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }

      if (!str_cmp(arg3, "acid"))
      {
        if (IS_SET(victim->immune, IMM_ACID))
        {
          REMOVE_BIT(victim->immune, IMM_ACID);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_ACID);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }

      if (!str_cmp(arg3, "shield"))
      {
        if (IS_SET(victim->immune, IMM_SHIELDED))
        {
          REMOVE_BIT(victim->immune, IMM_SHIELDED);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_SHIELDED);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }

      if (!str_cmp(arg3, "hurl"))
      {
        if (IS_SET(victim->immune, IMM_HURL))
        {
          REMOVE_BIT(victim->immune, IMM_HURL);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_HURL);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }

      if (!str_cmp(arg3, "backstab"))
      {
        if (IS_SET(victim->immune, IMM_BACKSTAB))
        {
          REMOVE_BIT(victim->immune, IMM_BACKSTAB);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_BACKSTAB);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }

      if (!str_cmp(arg3, "kick"))
      {
        if (IS_SET(victim->immune, IMM_KICK))
        {
          REMOVE_BIT(victim->immune, IMM_KICK);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_KICK);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }

      if (!str_cmp(arg3, "disarm"))
      {
        if (IS_SET(victim->immune, IMM_DISARM))
        {
          REMOVE_BIT(victim->immune, IMM_DISARM);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_DISARM);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }

      if (!str_cmp(arg3, "steal"))
      {
        if (IS_SET(victim->immune, IMM_STEAL))
        {
          REMOVE_BIT(victim->immune, IMM_STEAL);
          send_to_char("Ok Immunity Removed.\n\r", ch);
          return;
        }
        else
        {
          SET_BIT(victim->immune, IMM_STEAL);
          send_to_char("Ok Immunity Added.\n\r", ch);
          return;
        }
      }

      if (!str_cmp(arg3, "all"))
      {
        SET_BIT(victim->immune, IMM_DRAIN);
        SET_BIT(victim->immune, IMM_SLASH);
        SET_BIT(victim->immune, IMM_STAB);
        SET_BIT(victim->immune, IMM_SMASH);
        SET_BIT(victim->immune, IMM_ANIMAL);
        SET_BIT(victim->immune, IMM_MISC);
        SET_BIT(victim->immune, IMM_CHARM);
        SET_BIT(victim->immune, IMM_HEAT);
        SET_BIT(victim->immune, IMM_COLD);
        SET_BIT(victim->immune, IMM_LIGHTNING);
        SET_BIT(victim->immune, IMM_ACID);
        SET_BIT(victim->immune, IMM_HURL);
        SET_BIT(victim->immune, IMM_BACKSTAB);
        SET_BIT(victim->immune, IMM_KICK);
        SET_BIT(victim->immune, IMM_DISARM);
        SET_BIT(victim->immune, IMM_STEAL);
        send_to_char("Ok All Immunities Added.\n\r", ch);
        return;
      }

      send_to_char("No such immunity exists.\n\r", ch);
      return;
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "weapon"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    argument = one_argument(argument, arg4);
    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number(arg4) ? atoi(arg4) : -1;

    if (value < 0 || value > 2000)
    {
      send_to_char("Weapon skill range is 0 to 2000.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      if (!str_cmp(arg3, "unarmed"))
      {
        victim->wpn[0] = value;
        send_to_char("Ok.\n\r", ch);
        return;
      }

      if (!str_cmp(arg3, "slice"))
      {
        victim->wpn[1] = value;
        send_to_char("Ok.\n\r", ch);
        return;
      }

      if (!str_cmp(arg3, "stab"))
      {
        victim->wpn[2] = value;
        send_to_char("Ok.\n\r", ch);
        return;
      }

      if (!str_cmp(arg3, "slash"))
      {
        victim->wpn[3] = value;
        send_to_char("Ok.\n\r", ch);
        return;
      }

      if (!str_cmp(arg3, "whip"))
      {
        victim->wpn[4] = value;
        send_to_char("Ok.\n\r", ch);
        return;
      }

      if (!str_cmp(arg3, "claw"))
      {
        victim->wpn[5] = value;
        send_to_char("Ok.\n\r", ch);
        return;
      }

      if (!str_cmp(arg3, "blast"))
      {
        victim->wpn[6] = value;
        send_to_char("Ok.\n\r", ch);
        return;
      }

      if (!str_cmp(arg3, "pound"))
      {
        victim->wpn[7] = value;
        send_to_char("Ok.\n\r", ch);
        return;
      }

      if (!str_cmp(arg3, "crush"))
      {
        victim->wpn[8] = value;
        send_to_char("Ok.\n\r", ch);
        return;
      }

      if (!str_cmp(arg3, "grep"))
      {
        victim->wpn[9] = value;
        send_to_char("Ok.\n\r", ch);
        return;
      }

      if (!str_cmp(arg3, "bite"))
      {
        victim->wpn[10] = value;
        send_to_char("Ok.\n\r", ch);
        return;
      }

      if (!str_cmp(arg3, "pierce"))
      {
        victim->wpn[11] = value;
        send_to_char("Ok.\n\r", ch);
        return;
      }

      if (!str_cmp(arg3, "suck"))
      {
        victim->wpn[12] = value;
        send_to_char("Ok.\n\r", ch);
        return;
      }

      if (!str_cmp(arg3, "all"))
      {
        victim->wpn[0] = value;
        victim->wpn[1] = value;
        victim->wpn[2] = value;
        victim->wpn[3] = value;
        victim->wpn[4] = value;
        victim->wpn[5] = value;
        victim->wpn[6] = value;
        victim->wpn[7] = value;
        victim->wpn[8] = value;
        victim->wpn[8] = value;
        victim->wpn[9] = value;
        victim->wpn[10] = value;
        victim->wpn[11] = value;
        victim->wpn[12] = value;
        send_to_char("Ok.\n\r", ch);
        return;
      }
      send_to_char("No such weapon skill exists.\n\r", ch);
      return;
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  /*
   * Generate usage message.
   */
  do_pset(ch, "");
  return;
}

void do_paradox(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Paradox whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }
  if (!IS_NPC(victim))
  {
    paradox(victim);
    send_to_char("Done!\n\r", ch);
  }
  else
    send_to_char("Not on NPCs.\n\r", ch);
  return;
}

void paradox(CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];

  send_to_char("The sins of your past strike back!\n\r", ch);
  send_to_char("The paradox has come for your soul!\n\r", ch);
  if (ch->sex == SEX_MALE)
    sprintf(buf, "#C%s #pscreams in agony as the #RP#Ga#RR#Ga#RD#Go#RX#p wrecks his puny mortal body#n", ch->name);
  else
    sprintf(buf, "#C%s #pscreams in agony as the #RP#Ga#RR#Ga#RD#Go#RX#p wrecks her puny mortal body#n", ch->name);
  do_info(ch, buf);
  ch->hit = -10;
  ch->max_move = (ch->max_move * 90) / 100;
  ch->max_mana = (ch->max_mana * 90) / 100;
  update_pos(ch);
  do_escape(ch, "");
  SET_BIT(ch->extra, TIED_UP);
  SET_BIT(ch->extra, GAGGED);
  SET_BIT(ch->extra, BLINDFOLDED);

  return;
}

void do_wizhelp(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int cmd;
  int col;

  col = 0;
  for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
  {
    if (cmd_table[cmd].level > LEVEL_AVATAR && cmd_table[cmd].level <= get_trust(ch))
    {
      sprintf(buf, "%-12.12s ", cmd_table[cmd].name);
      send_to_char(buf, ch);
      if (++col % 6 == 0)
        send_to_char("\n\r", ch);
    }
  }

  if (col % 6 != 0)
    send_to_char("\n\r", ch);

  if (ch->pcdata->immcmd[0] != '\0')
  {
    send_to_char("\n\rExtra: ", ch);
    send_to_char(ch->pcdata->immcmd, ch);
    send_to_char("\n\r", ch);
  }
}

void do_bamfin(CHAR_DATA * ch, char *argument)
{
  if (!IS_NPC(ch))
  {
    smash_tilde(argument);
    free_string(ch->pcdata->bamfin);
    ch->pcdata->bamfin = str_dup(argument);
    send_to_char("Ok.\n\r", ch);
  }
  return;
}

void do_bamfout(CHAR_DATA * ch, char *argument)
{
  if (!IS_NPC(ch))
  {
    smash_tilde(argument);
    free_string(ch->pcdata->bamfout);
    ch->pcdata->bamfout = str_dup(argument);
    send_to_char("Ok.\n\r", ch);
  }
  return;
}

void do_nosummon(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  if (IS_NPC(ch))
    return;

  if (arg[0] == '\0')
  {
    send_to_char("Do you wish to switch summon ON or OFF?\n\r", ch);
    return;
  }

  if (IS_IMMUNE(ch, IMM_SUMMON) && !str_cmp(arg, "off"))
  {
    REMOVE_BIT(ch->immune, IMM_SUMMON);
    send_to_char("You now cant be the target of summon and portal.\n\r", ch);
  }
  else if (!IS_IMMUNE(ch, IMM_SUMMON) && !str_cmp(arg, "off"))
  {
    send_to_char("But it is already off!\n\r", ch);
    return;
  }
  else if (!IS_IMMUNE(ch, IMM_SUMMON) && !str_cmp(arg, "on"))
  {
    SET_BIT(ch->immune, IMM_SUMMON);
    send_to_char("You now can be the target of summon and portal.\n\r", ch);
  }
  else if (IS_IMMUNE(ch, IMM_SUMMON) && !str_cmp(arg, "on"))
  {
    send_to_char("But it is already on!\n\r", ch);
    return;
  }
  else
    send_to_char("Do you wish to switch it ON or OFF?\n\r", ch);
  return;
}

void do_transport(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  if (IS_NPC(ch))
    return;

  if (arg[0] == '\0')
  {
    send_to_char("Do you wish to switch transport ON or OFF?\n\r", ch);
    return;
  }

  if (IS_IMMUNE(ch, IMM_TRANSPORT) && !str_cmp(arg, "off"))
  {
    REMOVE_BIT(ch->immune, IMM_TRANSPORT);
    send_to_char("You can no longer be the target of transport spells.\n\r", ch);
  }
  else if (!IS_IMMUNE(ch, IMM_TRANSPORT) && !str_cmp(arg, "off"))
  {
    send_to_char("But it is already off!\n\r", ch);
    return;
  }
  else if (!IS_IMMUNE(ch, IMM_TRANSPORT) && !str_cmp(arg, "on"))
  {
    SET_BIT(ch->immune, IMM_TRANSPORT);
    send_to_char("You can now be the target of transport spells.\n\r", ch);
  }
  else if (IS_IMMUNE(ch, IMM_TRANSPORT) && !str_cmp(arg, "on"))
  {
    send_to_char("But it is already on!\n\r", ch);
    return;
  }
  else
    send_to_char("Do you wish to switch it ON or OFF?\n\r", ch);
  return;
}

void do_deny(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *xMob;
  char target[MAX_STRING_LENGTH];
  int days;

  argument = one_argument(argument, target);

  if (target[0] == '\0' || !is_number(argument))
  {
    send_to_char("Syntax: Deny <player> <days>\n\r", ch);
    return;
  }

  if ((days = atoi(argument)) < 1 || days > 10)
  {
    send_to_char("Between 1 and 10 days, please.\n\r", ch);
    return;
  }

  if ((xMob = get_char_world(ch, target)) == NULL)
  {
    send_to_char("That player is not online.\n\r", ch);
    return;
  }

  if (xMob->desc == NULL)
  {
    send_to_char("That player does not have a live connection.\n\r", ch);
    return;
  }

  if (xMob->trust >= ch->trust)
  {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  xMob->desc->account->denied = current_time + (days * 24L * 3600L);
  sprintf(target, "You have been denied for %d days.\n\r", days);
  write_to_buffer(xMob->desc, target, 0);
  xMob->hit = UMAX(1, xMob->hit);
  xMob->position = POS_STANDING;
  xMob->fight_timer = 0;

  close_socket(xMob->desc);
  do_quit(xMob, "");

  send_to_char("They have been denied.\n\r", ch);
}

void do_undeny(CHAR_DATA *ch, char *argument)
{
  ACCOUNT_DATA *account;
  char arg[MAX_STRING_LENGTH];

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Undeny whom?\n\r", ch);
    return;
  }
  if ((account = load_account(arg)) == NULL)
  {
    send_to_char("There is no account by that name.\n\r", ch);
    return;
  }
  if (account->denied <= current_time)
  {
    send_to_char("That account is not denied.\n\r", ch);
    close_account(account);
    return;
  }
  account->denied = current_time;
  save_account(account);
  close_account(account);
  send_to_char("Account undenied.\n\r", ch);
}

void do_disconnect(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Disconnect whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (victim->desc == NULL)
  {
    act("$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR);
    return;
  }

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d == victim->desc)
    {
      close_socket(d);
      send_to_char("Ok.\n\r", ch);
      return;
    }
  }

  bug("Do_disconnect: desc not found.", 0);
  send_to_char("Descriptor not found!\n\r", ch);
  return;
}

void do_info(CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];

  if (argument[0] == '\0')
    return;

  sprintf(buf, "#C<- #RInfo #C->#n %s\n\r", argument);

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected == CON_PLAYING && d->character != NULL &&
        !IS_SET(d->character->deaf, CHANNEL_INFO))
    {
      send_to_char(buf, d->character);
    }
  }
}

void logchan(char *argument)
{
  CHAR_DATA *ch;
  ITERATOR *pIter;
  DESCRIPTOR_DATA *d;

  if (argument[0] == '\0')
    return;

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if ((ch = d->character) == NULL)
      continue;
    if ((d->connected == CON_PLAYING) && IS_JUDGE(ch) && !IS_SET(ch->deaf, CHANNEL_LOG))
    {
      send_to_char("[", ch);
      send_to_char(argument, ch);
      send_to_char("]\n\r", ch);
    }
  }
}

void do_echo(CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;

  if (argument[0] == '\0')
  {
    send_to_char("Echo what?\n\r", ch);
    return;
  }

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected == CON_PLAYING)
    {
      send_to_char(argument, d->character);
      send_to_char("\n\r", d->character);
    }
  }
}

void do_recho(CHAR_DATA * ch, char *argument)
{
  ITERATOR *pIter;
  DESCRIPTOR_DATA *d;

  if (argument[0] == '\0')
  {
    send_to_char("Recho what?\n\r", ch);
    return;
  }

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected == CON_PLAYING && d->character->in_room == ch->in_room)
    {
      send_to_char(argument, d->character);
      send_to_char("\n\r", d->character);
    }
  }
}

ROOM_INDEX_DATA *find_location(CHAR_DATA * ch, char *arg)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  if (is_number(arg))
    return get_room_index(atoi(arg));

  if ((victim = get_char_world(ch, arg)) != NULL)
    if (can_see(ch, victim))
      return victim->in_room;

  if ((obj = get_obj_world(ch, arg)) != NULL && obj->in_room != NULL)
    return obj->in_room;

  if (obj != NULL && obj->carried_by != NULL && obj->carried_by->in_room != NULL)
    return obj->carried_by->in_room;

  if (obj != NULL && obj->in_obj != NULL && obj->in_obj->in_room != NULL)
    return obj->in_obj->in_room;

  if (obj != NULL && obj->in_obj != NULL && obj->in_obj->carried_by && obj->in_obj->carried_by->in_room != NULL)
    return obj->in_obj->carried_by->in_room;

  return NULL;
}

void do_transfer(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;
  CHAR_DATA *mount;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0')
  {
    send_to_char("Transfer whom (and where)?\n\r", ch);
    return;
  }

  if (!str_cmp(arg1, "all"))
  {
    ITERATOR *pIter;

    location = ch->in_room;
    pIter = AllocIterator(descriptor_list);
    while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
    {
      if (d->connected == CON_PLAYING && (victim = d->character) != ch &&
          d->character->in_room != NULL && can_see(ch, d->character))
      {
        if (victim->fighting != NULL)
          stop_fighting(victim, TRUE);

        act("$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM);
        char_from_room(victim);
        char_to_room(victim, location, TRUE);
        act("$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM);
        act("$n has transferred you.", ch, NULL, victim, TO_VICT);
        do_look(victim, "auto");

        if ((mount = victim->mount) != NULL)
        {
          char_from_room(mount);
          char_to_room(mount, location, TRUE);
          act("$n has transferred you.", ch, NULL, mount, TO_VICT);
          do_look(mount, "auto");
        }
      }
    }
    send_to_char("Ok.\n\r", ch);
    return;
  }

  /*
   * Thanks to Grodyn for the optional location parameter.
   */
  if (arg2[0] == '\0')
  {
    location = ch->in_room;
  }
  else
  {
    if ((location = find_location(ch, arg2)) == NULL)
    {
      send_to_char("No such location.\n\r", ch);
      return;
    }

    if (room_is_private(location))
    {
      send_to_char("That room is private right now.\n\r", ch);
      return;
    }
  }

  if ((victim = get_char_world(ch, arg1)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (victim->in_room == NULL)
  {
    send_to_char("They are in limbo.\n\r", ch);
    return;
  }

  if (victim->fighting != NULL)
    stop_fighting(victim, TRUE);
  act("$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, location, TRUE);
  act("$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM);
  if (ch != victim)
    act("$n has transferred you.", ch, NULL, victim, TO_VICT);
  do_look(victim, "auto");
  send_to_char("Ok.\n\r", ch);
  if ((mount = victim->mount) == NULL)
    return;
  char_from_room(mount);
  char_to_room(mount, get_room_index(victim->in_room->vnum), TRUE);
  if (ch != mount)
    act("$n has transferred you.", ch, NULL, mount, TO_VICT);
  do_look(mount, "auto");
}

void do_at(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  ROOM_INDEX_DATA *original;
  ITERATOR *pIter;
  CHAR_DATA *wch;

  argument = one_argument(argument, arg);

  if (arg[0] == '\0' || argument[0] == '\0')
  {
    send_to_char("At where what?\n\r", ch);
    return;
  }

  if ((location = find_location(ch, arg)) == NULL)
  {
    send_to_char("No such location.\n\r", ch);
    return;
  }

  if (room_is_private(location))
  {
    send_to_char("That room is private right now.\n\r", ch);
    return;
  }

  original = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location, TRUE);
  interpret(ch, argument);

  /*
   * See if 'ch' still exists before continuing!
   * Handles 'at XXXX quit' case.
   */
  pIter = AllocIterator(char_list);
  while ((wch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (wch == ch)
    {
      char_from_room(ch);
      char_to_room(ch, original, TRUE);
      break;
    }
  }

  return;
}

void do_goto(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  ROOM_INDEX_DATA *in_room;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Goto where?\n\r", ch);
    return;
  }

  if ((location = find_location(ch, arg)) == NULL)
  {
    send_to_char("No such location.\n\r", ch);
    return;
  }

  in_room = ch->in_room;
  if (ch->fighting)
    stop_fighting(ch, TRUE);

  if (!IS_SET(ch->act, PLR_HIDE))
    act("$n $T", ch, NULL, (ch->pcdata && ch->pcdata->bamfout[0] != '\0') ? ch->pcdata->bamfout : "leaves in a swirling mist.", TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, location, TRUE);

  if (!IS_SET(ch->act, PLR_HIDE))
    act("$n $T", ch, NULL, (ch->pcdata && ch->pcdata->bamfin[0] != '\0') ? ch->pcdata->bamfin : "appears in a swirling mist.", TO_ROOM);

  do_look(ch, "auto");

  if (ch->in_room == in_room)
    return;

  return;
}

void do_rstat(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  ITERATOR *pIter;
  OBJ_DATA *obj;
  CHAR_DATA *rch;
  int door;

  one_argument(argument, arg);
  location = (arg[0] == '\0') ? ch->in_room : find_location(ch, arg);
  if (location == NULL)
  {
    send_to_char("No such location.\n\r", ch);
    return;
  }

  if (ch->in_room != location && room_is_private(location))
  {
    send_to_char("That room is private right now.\n\r", ch);
    return;
  }

  sprintf(buf, "Name: '%s.'\n\rArea: '%s'.\n\r", location->name, location->area->name);
  send_to_char(buf, ch);

  sprintf(buf, "Vnum: %d.  Sector: %d.  Light: %d.\n\r", location->vnum, location->sector_type, location->light);
  send_to_char(buf, ch);

  sprintf(buf, "Room flags: %d.\n\rDescription:\n\r%s", location->room_flags, location->description);
  send_to_char(buf, ch);

  if (SizeOfList(location->extra_descr) != 0)
  {
    EXTRA_DESCR_DATA *ed;

    send_to_char("Extra description keywords: '", ch);

    pIter = AllocIterator(location->extra_descr);
    while ((ed = (EXTRA_DESCR_DATA *) NextInList(pIter)) != NULL)
    {
      send_to_char(ed->keyword, ch);
      send_to_char(" ", ch);
    }
    send_to_char("'.\n\r", ch);
  }

  send_to_char("Characters:", ch);
  pIter = AllocIterator(location->people);
  while ((rch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    send_to_char(" ", ch);
    one_argument(rch->name, buf);
    send_to_char(buf, ch);
  }

  send_to_char(".\n\rObjects:   ", ch);
  pIter = AllocIterator(location->contents);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    send_to_char(" ", ch);
    one_argument(obj->name, buf);
    send_to_char(buf, ch);
  }
  send_to_char(".\n\r", ch);

  for (door = 0; door <= 5; door++)
  {
    EXIT_DATA *pexit;

    if ((pexit = location->exit[door]) != NULL)
    {
      sprintf(buf,
              "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",
              door, pexit->to_room != NULL ? pexit->to_room->vnum : 0, pexit->key, pexit->exit_info, pexit->keyword, pexit->description[0] != '\0' ? pexit->description : "(none).\n\r");
      send_to_char(buf, ch);
    }
  }

  return;
}

void do_ostat(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char nm1[40];
  char nm2[40];
  AFFECT_DATA *paf;
  ITERATOR *pIter;
  OBJ_DATA *obj;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Ostat what?\n\r", ch);
    return;
  }

  if ((obj = get_obj_world(ch, arg)) == NULL)
  {
    send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
    return;
  }

  sprintf(nm1, "None");

  if (obj->questowner != NULL && strlen(obj->questowner) > 1)
    sprintf(nm2, "%s", obj->questowner);
  else
    sprintf(nm2, "None");

  sprintf(buf, "Name: %s.\n\r", obj->name);
  send_to_char(buf, ch);

  sprintf(buf, "Vnum: %d.  Type: %s.\n\r", obj->pIndexData->vnum, item_type_name(obj));
  send_to_char(buf, ch);

  sprintf(buf, "Short description: %s.\n\rLong description: %s\n\r", obj->short_descr, obj->description);
  send_to_char(buf, ch);

  sprintf(buf, "Object creator: %s.  Object owner: %s.\n\r", nm1, nm2);
  send_to_char(buf, ch);
  if (obj->quest != 0)
  {
    send_to_char("Quest selections:", ch);
    if (IS_SET(obj->quest, QUEST_STR))
      send_to_char(" Str", ch);
    if (IS_SET(obj->quest, QUEST_DEX))
      send_to_char(" Dex", ch);
    if (IS_SET(obj->quest, QUEST_INT))
      send_to_char(" Int", ch);
    if (IS_SET(obj->quest, QUEST_WIS))
      send_to_char(" Wis", ch);
    if (IS_SET(obj->quest, QUEST_CON))
      send_to_char(" Con", ch);
    if (IS_SET(obj->quest, QUEST_HIT))
      send_to_char(" Hp", ch);
    if (IS_SET(obj->quest, QUEST_MANA))
      send_to_char(" Mana", ch);
    if (IS_SET(obj->quest, QUEST_MOVE))
      send_to_char(" Move", ch);
    if (IS_SET(obj->quest, QUEST_HITROLL))
      send_to_char(" Hit", ch);
    if (IS_SET(obj->quest, QUEST_DAMROLL))
      send_to_char(" Dam", ch);
    if (IS_SET(obj->quest, QUEST_AC))
      send_to_char(" Ac", ch);
    send_to_char(".\n\r", ch);
  }
  sprintf(buf, "Wear bits: %d.  Extra bits: %s.\n\r", obj->wear_flags, extra_bit_name(obj->extra_flags));
  send_to_char(buf, ch);

  sprintf(buf, "Weight: %d/%d.  OwnerID : %d.\n\r", obj->weight, get_obj_weight(obj), obj->ownerid);
  send_to_char(buf, ch);

  sprintf(buf, "Cost: %d.  Level: %d.\n\r", obj->cost, obj->level);
  send_to_char(buf, ch);

  sprintf(buf,
          "In room: %d.  In object: %s.  Carried by: %s.  Wear_loc: %d.\n\r",
          obj->in_room == NULL ? 0 : obj->in_room->vnum, obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr, obj->carried_by == NULL ? "(none)" : obj->carried_by->name, obj->wear_loc);
  send_to_char(buf, ch);

  sprintf(buf, "Values: %d %d %d %d.\n\r", obj->value[0], obj->value[1], obj->value[2], obj->value[3]);
  send_to_char(buf, ch);

  if (SizeOfList(obj->extra_descr) != 0 || 
      SizeOfList(obj->pIndexData->extra_descr) != 0)
  {
    EXTRA_DESCR_DATA *ed;

    send_to_char("Extra description keywords: '", ch);

    pIter = AllocIterator(obj->extra_descr);
    while ((ed = (EXTRA_DESCR_DATA *) NextInList(pIter)) != NULL)
    {
      send_to_char(ed->keyword, ch);
      send_to_char(" ", ch);
    }

    pIter = AllocIterator(obj->pIndexData->extra_descr);
    while ((ed = (EXTRA_DESCR_DATA *) NextInList(pIter)) != NULL)
    {
      send_to_char(ed->keyword, ch);
      send_to_char(" ", ch);
    }

    send_to_char("'.\n\r", ch);
  }

  pIter = AllocIterator(obj->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    sprintf(buf, "Affects %s by %d.\n\r", affect_loc_name(paf->location), paf->modifier);
    send_to_char(buf, ch);
  }

  pIter = AllocIterator(obj->pIndexData->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    sprintf(buf, "Affects %s by %d.\n\r", affect_loc_name(paf->location), paf->modifier);
    send_to_char(buf, ch);
  }

  return;
}

void do_mstat(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA *paf;
  ITERATOR *pIter;
  CHAR_DATA *victim;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Mstat whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  sprintf(buf, "Name: %s.\n\r", victim->name);
  send_to_char(buf, ch);

  printf_to_char(ch, "Vnum: %d.  Sex: %s.  Room: %d  Might: %d.\n\r",
    IS_NPC(victim) ? victim->pIndexData->vnum : 0,
    victim->sex == SEX_MALE ? "male" : victim->sex == SEX_FEMALE ? "female" : "neutral",
    victim->in_room == NULL ? 0 : victim->in_room->vnum, getMight(victim));
  send_to_char(buf, ch);

  sprintf(buf, "Str: %d.  Int: %d.  Wis: %d.  Dex: %d.  Con: %d.\n\r", get_curr_str(victim), get_curr_int(victim), get_curr_wis(victim), get_curr_dex(victim), get_curr_con(victim));
  send_to_char(buf, ch);

  sprintf(buf, "Hp: %d/%d.  Mana: %d/%d.  Move: %d/%d.  Primal: %d.\n\r", victim->hit, victim->max_hit, victim->mana, victim->max_mana, victim->move, victim->max_move, victim->practice);
  send_to_char(buf, ch);

  sprintf(buf, "Lv: %d.  Align: %d.  AC: %d.  Exp: %d.\n\r", victim->level, victim->alignment, char_ac(victim), victim->exp);
  send_to_char(buf, ch);

  sprintf(buf, "Hitroll: %d.  Damroll: %d.  Position: %d.\n\r", char_hitroll(victim), char_damroll(victim), victim->position);
  send_to_char(buf, ch);

  sprintf(buf, "Fighting: %s.\n\r", victim->fighting ? victim->fighting->name : "(none)");
  send_to_char(buf, ch);

  if (!IS_NPC(victim))
  {
    sprintf(buf,
            "Thirst: %d.  Full: %d.  Drunk: %d.  Saving throw: %d.\n\r",
            victim->pcdata->condition[COND_THIRST], victim->pcdata->condition[COND_FULL], victim->pcdata->condition[COND_DRUNK], victim->saving_throw);
    send_to_char(buf, ch);

  }

  sprintf(buf, "Carry number: %d.  Carry weight: %d.\n\r", victim->carry_number, victim->carry_weight);
  send_to_char(buf, ch);

  sprintf(buf, "Age: %d.  Played: %d.  Timer: %d.  Act: %d.\n\r", get_age(victim), (int) victim->played, victim->timer, victim->act);
  send_to_char(buf, ch);

  sprintf(buf, "Master: %s.  Leader: %s.  Affected by: %s.\n\r",
          victim->master ? victim->master->name : "(none)", victim->leader ? victim->leader->name : "(none)", affect_bit_name(victim->affected_by));
  send_to_char(buf, ch);

  if (!IS_NPC(victim))          /* OLC */
  {
    sprintf(buf, "Security: %d.\n\r", victim->pcdata->security);
    send_to_char(buf, ch);
  }

  sprintf(buf, "Short description: %s.\n\rLong  description: %s", victim->short_descr, victim->long_descr[0] != '\0' ? victim->long_descr : "(none).\n\r");
  send_to_char(buf, ch);

  if (IS_NPC(victim) && victim->spec_fun != 0)
    send_to_char("Mobile has spec fun.\n\r", ch);

  pIter = AllocIterator(victim->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    sprintf(buf,
            "Spell: '%s' modifies %s by %d for %d hours with bits %s.\n\r",
            skill_table[(int) paf->type].name, affect_loc_name(paf->location), paf->modifier, paf->duration, affect_bit_name(paf->bitvector));
    send_to_char(buf, ch);
  }

  return;
}

void do_mfind(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  MOB_INDEX_DATA *pMobIndex;
  int vnum;
  int nMatch;
  bool fAll;
  bool found;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Mfind whom?\n\r", ch);
    return;
  }

  fAll = FALSE;
  found = FALSE;
  nMatch = 0;

  /*
   * Yeah, so iterating over all vnum's takes 10,000 loops.
   * Get_mob_index is fast, and I don't feel like threading another link.
   * Do you?
   * -- Furey
   */
  for (vnum = 0; nMatch < top_mob_index; vnum++)
  {
    if ((pMobIndex = get_mob_index(vnum)) != NULL)
    {
      nMatch++;
      if (fAll || is_name(arg, pMobIndex->player_name))
      {
        found = TRUE;
        sprintf(buf, "[%5d] %s\n\r", pMobIndex->vnum, capitalize(pMobIndex->short_descr));
        send_to_char(buf, ch);
      }
    }
  }

  if (!found)
    send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);

  return;
}

void do_ofind(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  int vnum;
  int nMatch;
  bool fAll;
  bool found;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Ofind what?\n\r", ch);
    return;
  }

  fAll = FALSE;
  found = FALSE;
  nMatch = 0;

  /*
   * Yeah, so iterating over all vnum's takes 10,000 loops.
   * Get_obj_index is fast, and I don't feel like threading another link.
   * Do you?
   * -- Furey
   */
  for (vnum = 0; nMatch < top_obj_index; vnum++)
  {
    if ((pObjIndex = get_obj_index(vnum)) != NULL)
    {
      nMatch++;
      if (fAll || is_name(arg, pObjIndex->name))
      {
        found = TRUE;
        sprintf(buf, "[%5d] %s\n\r", pObjIndex->vnum, capitalize(pObjIndex->short_descr));
        send_to_char(buf, ch);
      }
    }
  }

  if (!found)
    send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);

  return;
}

void do_mwhere(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  CHAR_DATA *victim;
  bool found;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Mwhere whom?\n\r", ch);
    return;
  }

  found = FALSE;
  pIter = AllocIterator(char_list);
  while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_NPC(victim) && victim->in_room != NULL && is_name(arg, victim->name))
    {
      found = TRUE;
      sprintf(buf, "[%5d] %-28s [%5d] %s\n\r", victim->pIndexData->vnum, victim->short_descr, victim->in_room->vnum, victim->in_room->name);
      send_to_char(buf, ch);
    }
  }

  if (!found)
    act("You didn't find any $T.", ch, NULL, arg, TO_CHAR);
}

void do_shutdow(CHAR_DATA * ch, char *argument)
{
  send_to_char("If you want to SHUTDOWN, spell it out.\n\r", ch);
  return;
}

void do_shutdown(CHAR_DATA * ch, char *argument)
{
  extern bool merc_down;
  char buf[MAX_STRING_LENGTH];

  if (pre_reboot_actions(FALSE) == FALSE)
  {
    send_to_char("Shutdown failed.\n\r", ch);
    return;
  }
  do_asave(ch, "changed");

  sprintf(buf, "Shutdown by %s.", ch->name);

  strcat(buf, "\n\r");
  do_echo(ch, buf);
  merc_down = TRUE;
}

void do_snoop(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter, *pIter2;
  CHAR_DATA *victim;
  SNOOP_DATA *snoop;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Snoop whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (victim->desc == NULL)
  {
    send_to_char("No descriptor to snoop.\n\r", ch);
    return;
  }

  if (victim == ch)
  {
    send_to_char("Cancelling all snoops.\n\r", ch);

    pIter = AllocIterator(descriptor_list);
    while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
    {
      pIter2 = AllocIterator(d->snoops);
      while ((snoop = (SNOOP_DATA *) NextInList(pIter2)) != NULL)
      {
        if (snoop->snooper == ch->desc)
          free_snoop(d, snoop);
      }
    }

    return;
  }

  if (get_trust(victim) >= get_trust(ch))
  {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if (!ch->desc) return;

  /* is this victim being snooped by you already */
  pIter = AllocIterator(victim->desc->snoops);
  while ((snoop = (SNOOP_DATA *) NextInList(pIter)) != NULL)
  {
    if (snoop->snooper == ch->desc)
    {
      send_to_char("You are already snooping this character.\n\r", ch);
      return;
    }
  }

  /* create new snoop and attach */
  snoop = alloc_snoop();
  snoop->snooper = ch->desc;
  AttachToList(snoop, victim->desc->snoops);

  send_to_char("Ok.\n\r", ch);
}

void do_mload(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *victim;

  one_argument(argument, arg);

  if (arg[0] == '\0' || !is_number(arg))
  {
    send_to_char("Syntax: mload <vnum>.\n\r", ch);
    return;
  }

  if ((pMobIndex = get_mob_index(atoi(arg))) == NULL)
  {
    send_to_char("No mob has that vnum.\n\r", ch);
    return;
  }

  victim = create_mobile(pMobIndex);
  char_to_room(victim, ch->in_room, TRUE);
  act("$n has created $N!", ch, NULL, victim, TO_ROOM);
  act("You have created $N!", ch, NULL, victim, TO_CHAR);
  return;
}

void do_pload(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *in_room;
  ITERATOR *pIter;
  DESCRIPTOR_DATA *d;
  CHAR_DATA *vch;
  ACCOUNT_DATA *account;
  int status = 0;

  if (IS_NPC(ch) || ch->desc == NULL || ch->in_room == NULL)
    return;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Syntax: pload <name>.\n\r", ch);
    return;
  }

  if (!check_parse_name(arg, TRUE))
  {
    send_to_char("Thats an illegal name.\n\r", ch);
    return;
  }

  if (!char_exists(arg))
  {
    send_to_char("That player doesn't exist.\n\r", ch);
    return;
  }

  pIter = AllocIterator(char_list);
  while ((vch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_NPC(vch)) continue;

    if (!str_cmp(vch->name, arg))
    {
      send_to_char("That player is already connected.\n\r", ch);
      return;
    }
  }

  /* load the characters whois info */
  if ((vch = load_char_whois(arg, &status)) == NULL)
  {
    if (status == -1)
      send_to_char("That player has not logged since we changed the finger storage.\n\r", ch);
    else if (status == 0)
      send_to_char("That player does not exist.\n\r", ch);
    else
      send_to_char("Something unexpected happened.\n\r", ch);

    return;
  }

  if ((account = load_account(vch->pcdata->account)) == NULL)
  {
    send_to_char("That players account doesn't exist.\n\r", ch);
    free_char(vch);
    return;
  }

  sprintf(buf, "You transform into %s.\n\r", vch->name);
  send_to_char(buf, ch);

  sprintf(buf, "$n transforms into %s.", vch->name);
  act(buf, ch, NULL, NULL, TO_ROOM);

  free_char(vch);

  d = ch->desc;
  save_char_obj(ch);
  in_room = ch->in_room;
  extract_char(ch, TRUE);
  d->character = NULL;
  close_account(d->account);

  d->account = account;
  load_char_obj(d, capitalize(arg));

  ch = d->character;

  AttachToList(ch, char_list);

  char_to_room(ch, in_room, TRUE);
  init_events_player(ch);
}

void do_oload(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;
  int level;

  if (IS_NPC(ch))
  {
    send_to_char("Not while switched.\n\r", ch);
    return;
  }

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0' || !is_number(arg1))
  {
    send_to_char("Syntax: oload <vnum> <level>.\n\r", ch);
    return;
  }

  if (arg2[0] == '\0')
  {
    level = get_trust(ch);
  }
  else
  {
    /*
     * New feature from Alander.
     */
    if (!is_number(arg2))
    {
      send_to_char("Syntax: oload <vnum> <level>.\n\r", ch);
      return;
    }
    level = atoi(arg2);
    if (level < 0 || level > get_trust(ch))
    {
      send_to_char("Limited to your trust level.\n\r", ch);
      return;
    }
  }

  if ((pObjIndex = get_obj_index(atoi(arg1))) == NULL)
  {
    send_to_char("No object has that vnum.\n\r", ch);
    return;
  }

  obj = create_object(pObjIndex, level);
  if (CAN_WEAR(obj, ITEM_TAKE))
  {
    obj_to_char(obj, ch);
    act("$p appears in $n's hands!", ch, obj, NULL, TO_ROOM);
  }
  else
  {
    obj_to_room(obj, ch->in_room);
    act("$n has created $p!", ch, obj, NULL, TO_ROOM);
  }
  act("You create $p.", ch, obj, NULL, TO_CHAR);
}

void do_purge(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *mount;
  ITERATOR *pIter;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    pIter = AllocIterator(ch->in_room->people);
    while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (IS_NPC(victim) && victim->desc == NULL && (mount = victim->mount) == NULL)
        extract_char(victim, TRUE);
    }

    act("$n purges the room!", ch, NULL, NULL, TO_ROOM);
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (!IS_NPC(victim))
  {
    send_to_char("Not on PC's.\n\r", ch);
    return;
  }
  if (victim->desc != NULL)
  {
    send_to_char("Not on switched players.\n\r", ch);
    return;
  }

  act("$n purges $N.", ch, NULL, victim, TO_NOTVICT);
  extract_char(victim, TRUE);
  return;
}

void do_trust(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int level;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0' || arg2[0] == '\0')
  {
    send_to_char("Syntax: trust <char> <trust>.\n\r", ch);
    send_to_char("Trust being one of: None, Builder, Questmaker, Enforcer, Judge, or Highjudge.\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg1)) == NULL)
  {
    send_to_char("That player is not here.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "none"))
    level = 0;
  else if (!str_cmp(arg2, "builder"))
    level = 8;
  else if (!str_cmp(arg2, "questmaker"))
    level = 8;
  else if (!str_cmp(arg2, "enforcer"))
    level = 9;
  else if (!str_cmp(arg2, "judge"))
    level = 10;
  else if (!str_cmp(arg2, "highjudge"))
    level = 11;
  else
  {
    send_to_char("Please enter: None, Builder, Questmaker, Enforcer, Judge, or Highjudge.\n\r", ch);
    return;
  }

  if (level >= get_trust(ch))
  {
    send_to_char("Limited to below your trust.\n\r", ch);
    return;
  }
  send_to_char("Ok.\n\r", ch);
  victim->trust = level;
  return;
}

void restore_player(CHAR_DATA *victim)
{
  victim->hit = victim->max_hit;
  victim->mana = victim->max_mana;
  victim->move = victim->max_move;
  victim->loc_hp[0] = 0;
  victim->loc_hp[1] = 0;
  victim->loc_hp[2] = 0;
  victim->loc_hp[3] = 0;
  victim->loc_hp[4] = 0;
  victim->loc_hp[5] = 0;
  victim->loc_hp[6] = 0;
  update_pos(victim);

  sound_to_char("restore.wav", victim);
}

void do_restore(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *d;

  one_argument(argument, arg);

  if (get_trust(ch) >= MAX_LEVEL - 2 && !str_cmp(arg, "all"))
  {
    ITERATOR *pIter;

    pIter = AllocIterator(descriptor_list);
    while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
    {
      victim = d->character;

      if (victim == NULL || IS_NPC(victim))
        continue;

      if (ch->level >= 12 || victim->fight_timer <= 0)
      {
        restore_player(victim);
        act("$n has restored you.", ch, NULL, victim, TO_VICT);
      }
    }
    send_to_char("All active players restored.\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  restore_player(victim);

  act("$n has restored you.", ch, NULL, victim, TO_VICT);
  send_to_char("Ok.\n\r", ch);
  return;
}

void do_freeze(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Bitchslap whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("Not on NPC's.\n\r", ch);
    return;
  }

  if (get_trust(victim) >= get_trust(ch))
  {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if (IS_SET(victim->act, PLR_FREEZE))
  {
    REMOVE_BIT(victim->act, PLR_FREEZE);
    send_to_char("You stop crying.\n\r", victim);
    send_to_char("Crybaby removed.\n\r", ch);
  }
  else
  {
    SET_BIT(victim->act, PLR_FREEZE);
    send_to_char("You receive a BITCHSLAP, and start crying like the bitch you are!\n\r", victim);
    send_to_char("Crybaby set.\n\r", ch);
  }
}

void do_log(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Log whom?\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "all"))
  {
    if (fLogAll)
    {
      fLogAll = FALSE;
      send_to_char("Log ALL off.\n\r", ch);
    }
    else
    {
      fLogAll = TRUE;
      send_to_char("Log ALL on.\n\r", ch);
    }
    return;
  }

  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("Not on NPC's.\n\r", ch);
    return;
  }

  /*
   * No level check, gods can log anyone.
   */
  if (IS_SET(victim->act, PLR_LOG))
  {
    REMOVE_BIT(victim->act, PLR_LOG);
    send_to_char("LOG removed.\n\r", ch);
  }
  else
  {
    SET_BIT(victim->act, PLR_LOG);
    send_to_char("LOG set.\n\r", ch);
  }

  return;
}

void do_silence(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Silence whom?\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("Not on NPC's.\n\r", ch);
    return;
  }

  if (get_trust(victim) >= get_trust(ch) && ch != victim)
  {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if (victim->desc == NULL || victim->desc->account == NULL)
  {
    send_to_char("You cannot silence or unsilence linkdead players.\n\r", ch);
    return;
  }

  if (is_silenced(victim))
  {
    REMOVE_BIT(victim->desc->account->flags, ACCOUNT_FLAG_SILENCED);
    send_to_char("SILENCE removed.\n\r", ch);
  }
  else
  {
    SET_BIT(victim->desc->account->flags, ACCOUNT_FLAG_SILENCED);
    send_to_char("SILENCE set.\n\r", ch);
  }

  return;
}

void do_peace(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *rch;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->in_room->people);
  while ((rch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (rch->fighting != NULL)
      stop_fighting(rch, TRUE);
  }

  send_to_char("All fighting stopped.\n\r", ch);
}

void do_ban(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  BAN_DATA *pban;

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    strcpy(buf, "Banned sites:\n\r");
    pIter = AllocIterator(ban_list);
    while ((pban = (BAN_DATA *) NextInList(pIter)) != NULL)
    {
      strcat(buf, pban->name);
      strcat(buf, "    (");
      strcat(buf, pban->reason);
      strcat(buf, ")\n\r");
    }
    send_to_char(buf, ch);
    return;
  }

  pIter = AllocIterator(ban_list);
  while ((pban = (BAN_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(arg, pban->name))
    {
      send_to_char("That site is already banned!\n\r", ch);
      return;
    }
  }

  if ((pban = (BAN_DATA *) PopStack(ban_free)) == NULL)
  {
    pban = calloc(1, sizeof(*pban));
  }

  pban->name = str_dup(arg);
  if (argument[0] == '\0')
    pban->reason = str_dup("no reason given");
  else
    pban->reason = str_dup(argument);

  AttachToList(pban, ban_list);

  send_to_char("Ok.\n\r", ch);
  save_bans();
}

void do_allow(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  BAN_DATA *curr;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Remove which site from the ban list?\n\r", ch);
    return;
  }

  pIter = AllocIterator(ban_list);
  while ((curr = (BAN_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(arg, curr->name))
    {
      DetachAtIterator(pIter);

      free_string(curr->name);
      free_string(curr->reason);

      PushStack(curr, ban_free);

      send_to_char("Ok.\n\r", ch);
      save_bans();
      return;
    }
  }

  send_to_char("Site is not banned.\n\r", ch);
}

void do_wizlock(CHAR_DATA * ch, char *argument)
{
  extern bool wizlock;

  wizlock = !wizlock;

  if (wizlock)
    send_to_char("Game wizlocked.\n\r", ch);
  else
    send_to_char("Game un-wizlocked.\n\r", ch);

  return;
}

void do_slookup(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int sn;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Slookup what?\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "all"))
  {
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (skill_table[sn].name == NULL)
        break;
      sprintf(buf, "Sn: %4d Skill/spell: '%s'\n\r", sn, skill_table[sn].name);
      send_to_char(buf, ch);
    }
  }
  else
  {
    if ((sn = skill_lookup(arg)) < 0)
    {
      send_to_char("No such skill or spell.\n\r", ch);
      return;
    }

    sprintf(buf, "Sn: %4d Skill/spell: '%s'\n\r", sn, skill_table[sn].name);
    send_to_char(buf, ch);
  }
}

void do_sset(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int value;
  int sn;
  bool fAll;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  argument = one_argument(argument, arg3);

  if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
  {
    send_to_char("#9Syntax: sset <victim> <skill> <value>\n\r", ch);
    send_to_char("#9or:     sset <victim> all     <value>\n\r", ch);
    send_to_char("#ySkill being any skill or spell.\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg1)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("Not on NPC's.\n\r", ch);
    return;
  }

  fAll = !str_cmp(arg2, "all");
  sn = 0;
  if (!fAll && (sn = skill_lookup(arg2)) < 0)
  {
    send_to_char("No such skill or spell.\n\r", ch);
    return;
  }

  /*
   * Snarf the value.
   */
  if (!is_number(arg3))
  {
    send_to_char("Value must be numeric.\n\r", ch);
    return;
  }

  value = atoi(arg3);
  if (value < 0 || value > 100)
  {
    send_to_char("Value range is 0 to 100.\n\r", ch);
    return;
  }

  if (fAll)
  {
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (skill_table[sn].name != NULL)
        victim->pcdata->learned[sn] = value;
    }
  }
  else
  {
    victim->pcdata->learned[sn] = value;
  }

  send_to_char("Ok.\n\r", ch);
  return;
}

void do_mset(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int value;

  smash_tilde(argument);
  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  strcpy(arg3, argument);

  if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
  {
    send_to_char("#9Syntax: mset <victim> <field>  <value>\n\r", ch);
    send_to_char("#9or:     mset <victim> <string> <value>\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("#9Field being one of:\n\r", ch);
    send_to_char("#y  str int wis dex con sex level exp\n\r", ch);
    send_to_char("#y  gold hp mana move primal align\n\r", ch);
    send_to_char("#y  thirst drunk full hit dam ac cp\n\r", ch);
    send_to_char("#y  security rune extra bounty hours\n\r", ch);
    send_to_char("#y  hpower origclass dage\n\r\n\r", ch);
    send_to_char("#9String being one of:\n\r", ch);
    send_to_char("#y  name short long description title spec#n\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg1)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  /*
   * Snarf the value (which need not be numeric).
   */
  value = is_number(arg3) ? atoi(arg3) : -1;

  /*
   * Set something.
   */
  if (!str_cmp(arg2, "extra"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on mobs.\n\r", ch);
      return;
    }
    if (!str_cmp(arg3, "pregnant"))
    {
      if (IS_EXTRA(victim, EXTRA_PREGNANT))
        REMOVE_BIT(victim->extra, EXTRA_PREGNANT);
      else
        SET_BIT(victim->extra, EXTRA_PREGNANT);
      send_to_char("Ok.\n\r", ch);
      return;
    }
    else if (!str_cmp(arg3, "poly"))
    {
      if (IS_SET(victim->affected_by, AFF_POLYMORPH))
        REMOVE_BIT(victim->affected_by, AFF_POLYMORPH);
      else
        SET_BIT(victim->affected_by, AFF_POLYMORPH);
      send_to_char("Ok.\n\r", ch);
      return;
    }
    else
    {
      send_to_char("Pregnant, dragon, pdragon, poly.\n\r", ch);
      return;
    }
  }
  if (!str_cmp(arg2, "bounty"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on mobs.\n\r", ch);
      return;
    }
    if (value < 0)
      value = 0;
    victim->pcdata->bounty = value;
    sprintf(buf, "%s bounty is now at %d.\n\r", victim->name, victim->pcdata->bounty);
    send_to_char(buf, ch);
    return;
  }
  else if (!str_cmp(arg2, "hours"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (!is_number(arg3))
    {
      send_to_char("Value must be numeric.\n\r", ch);
      return;
    }

    value = atoi(arg3);

    if (value < 0 || value > 999)
    {
      send_to_char("Value must be betwen 0 and 999.\n\r", ch);
      return;
    }

    value *= 3600;
    victim->played = value;
    return;
  }

  if (!str_cmp(arg2, "cp"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }
    value = atoi(arg3);
    if (value < 0 || value > 10000000)
    {
      send_to_char("Value must be between 0 and 10.000.000.\n\r", ch);
      return;
    }
    if (IS_CLASS(victim, CLASS_SHADOW))
      victim->pcdata->powers[SHADOW_POWER] = value;
    else
    {
      send_to_char("That class doens't use class points.\n\r", ch);
      return;
    }
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "str"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 3 || value > 35)
    {
      send_to_char("Strength range is 3 to 25.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->pcdata->perm_str = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "int"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 3 || value > 35)
    {
      send_to_char("Intelligence range is 3 to 25.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->pcdata->perm_int = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "wis"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 3 || value > 35)
    {
      send_to_char("Wisdom range is 3 to 25.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->pcdata->perm_wis = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "dex"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 3 || value > 35)
    {
      send_to_char("Dexterity range is 3 to 25.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->pcdata->perm_dex = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "con"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 3 || value > 35)
    {
      send_to_char("Constitution range is 3 to 25.\n\r", ch);
      return;
    }

    if (IS_JUDGE(ch))
    {
      victim->pcdata->perm_con = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "sex"))
  {
    if (value < 0 || value > 2)
    {
      send_to_char("Sex range is 0 to 2.\n\r", ch);
      return;
    }
    victim->sex = value;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "level"))
  {
    if (IS_NPC(victim) && (value < 1 || value > 4000))
    {
      send_to_char("Level range is 1 to 4000 for mobs.\n\r", ch);
      return;
    }
    else if (!IS_JUDGE(ch))
    {
      send_to_char("Sorry, no can do...\n\r", ch);
      return;
    }
    if (!str_cmp(arg3, "mortal"))
      value = 2;
    else if (!str_cmp(arg3, "avatar"))
      value = 3;
    else if (!str_cmp(arg3, "builder"))
      value = 8;
    else if (!str_cmp(arg3, "questmaker"))
      value = 7;
    else if (!str_cmp(arg3, "enforcer"))
      value = 9;
    else if (!str_cmp(arg3, "judge"))
      value = 10;

    else if (!IS_NPC(victim))
    {
      send_to_char("Level should be one of the following:\n\rMortal, Avatar, Builder, Questmaker, Enforcer, Judge.\n\r", ch);
      return;
    }

    if (value >= ch->level && !IS_NPC(victim))
      send_to_char("Sorry, no can do...\n\r", ch);
    else
    {
      if (value == 8 && !IS_NPC(victim))
      {
        free_string(victim->pcdata->immcmd);
        victim->pcdata->immcmd = str_dup("redit oedit medit asave alist resets sedit");
      }
      victim->level = value;
      victim->trust = value;
      if (victim->level == 8)
        victim->pcdata->security = 9;
      send_to_char("Ok.\n\r", ch);
    }
    return;
  }

  if (!str_cmp(arg2, "hitroll") || !str_cmp(arg2, "hit"))
  {
    if (!IS_NPC(victim) && (value < 0 || value > 50))
    {
      send_to_char("Hitroll range is 0 to 50.\n\r", ch);
      return;
    }
    else if (IS_NPC(victim) && (value < 0 || value > 250))
    {
      send_to_char("Hitroll range is 0 to 250.\n\r", ch);
      return;
    }
    if (!IS_NPC(victim) && !IS_JUDGE(ch) && ch != victim)
    {
      send_to_char("Sorry, no can do...\n\r", ch);
      return;
    }
    victim->hitroll = value;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "damroll") || !str_cmp(arg2, "dam"))
  {
    if (!IS_NPC(victim) && (value < 0 || value > 50))
    {
      send_to_char("Damroll range is 0 to 50.\n\r", ch);
      return;
    }
    else if (IS_NPC(victim) && (value < 0 || value > 250))
    {
      send_to_char("Damroll range is 0 to 250.\n\r", ch);
      return;
    }
    if (!IS_NPC(victim) && !IS_JUDGE(ch) && ch != victim)
    {
      send_to_char("Sorry, no can do...\n\r", ch);
      return;
    }
    victim->damroll = value;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "armor") || !str_cmp(arg2, "ac"))
  {
    if (!IS_NPC(victim) && (value < -200 || value > 200))
    {
      send_to_char("Armor class range is -200 to 200.\n\r", ch);
      return;
    }
    if (!IS_NPC(victim) && !IS_JUDGE(ch) && ch != victim)
    {
      send_to_char("Sorry, no can do...\n\r", ch);
      return;
    }
    victim->armor = value;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "exp"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 0)
    {
      send_to_char("Exp must be at least 0.\n\r", ch);
      return;
    }
    if (IS_JUDGE(ch) || (ch == victim))
    {
      victim->exp = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "hp"))
  {
    if (value < 1 || value > 150000)
    {
      send_to_char("Hp range is 1 to 150,000 hit points.\n\r", ch);
      return;
    }
    if (IS_JUDGE(ch) || (ch == victim) || (IS_NPC(victim)))
    {
      victim->max_hit = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "mana"))
  {
    if (value < 0 || value > 150000)
    {
      send_to_char("Mana range is 0 to 150,000 mana points.\n\r", ch);
      return;
    }
    if (IS_JUDGE(ch) || (ch == victim) || (IS_NPC(victim)))
    {
      victim->max_mana = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "move"))
  {
    if (value < 0 || value > 150000)
    {
      send_to_char("Move range is 0 to 150,000 move points.\n\r", ch);
      return;
    }
    if (IS_JUDGE(ch) || (ch == victim) || (IS_NPC(victim)))
    {
      victim->max_move = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "primal"))
  {
    if (value < 0)
    {
      send_to_char("Primal range is 0 to infinate.\n\r", ch);
      return;
    }
    if (IS_JUDGE(ch) || (ch == victim))
    {
      victim->practice = value;
      send_to_char("Ok.\n\r", ch);
    }
    else
      send_to_char("Sorry, no can do...\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "align"))
  {
    if (value < -1000 || value > 1000)
    {
      send_to_char("Alignment range is -1000 to 1000.\n\r", ch);
      return;
    }
    victim->alignment = value;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "thirst"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 0 || value > 3000)
    {
      send_to_char("Thirst range is 0 to 2000.\n\r", ch);
      return;
    }

    victim->pcdata->condition[COND_THIRST] = value;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "drunk"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 0 || value > 100)
    {
      send_to_char("Drunk range is 0 to 100.\n\r", ch);
      return;
    }

    victim->pcdata->condition[COND_DRUNK] = value;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "full"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value < 0 || value > 1200)
    {
      send_to_char("Full range is 0 to 100.\n\r", ch);
      return;
    }

    victim->pcdata->condition[COND_FULL] = value;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "name"))
  {
    if (!IS_NPC(victim))
    {
      send_to_char("Not on PC's.\n\r", ch);
      return;
    }

    free_string(victim->name);
    victim->name = str_dup(arg3);
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "short"))
  {
    if (!IS_NPC(victim))
    {
      send_to_char("Not on PC's.\n\r", ch);
      return;
    }
    free_string(victim->short_descr);
    victim->short_descr = str_dup(arg3);
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "long"))
  {
    if (!IS_NPC(victim))
    {
      send_to_char("Not on PC's.\n\r", ch);
      return;
    }
    free_string(victim->long_descr);
    victim->long_descr = str_dup(arg3);
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "title"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    set_title(victim, arg3);
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "spec"))
  {
    if (!IS_NPC(victim))
    {
      send_to_char("Not on PC's.\n\r", ch);
      return;
    }

    if ((victim->spec_fun = spec_lookup(arg3)) == 0)
    {
      send_to_char("No such spec fun.\n\r", ch);
      return;
    }

    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "security")) /* OLC */
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

    if (value > ch->pcdata->security || value < 0)
    {
      if (ch->pcdata->security != 0)
      {
        sprintf(buf, "Valid security is 0-%d.\n\r", ch->pcdata->security);
        send_to_char(buf, ch);
        send_to_char(buf, ch);
      }
      else
      {
        send_to_char("Valid security is 0 only.\n\r", ch);
      }
      return;
    }
    victim->pcdata->security = value;
    return;
  }

  /*
   * Generate usage message.
   */

  do_mset(ch, "");
  return;
}

void do_oset(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int value;

  if (IS_NPC(ch))
  {
    send_to_char("Not while switched.\n\r", ch);
    return;
  }

  smash_tilde(argument);
  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  strcpy(arg3, argument);

  if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
  {
    send_to_char("#9Syntax: oset <object> <field>  <value>\n\r", ch);
    send_to_char("#9or:     oset <object> <string> <value>\n\r", ch);
    send_to_char("#9or:     oset <object> <affect> <value>\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("#9Field being one of:\n\r", ch);
    send_to_char("#y  value0 value1 value2 value3\n\r", ch);
    send_to_char("#y  level weight cost morph\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("#9String being one of:\n\r", ch);
    send_to_char("#y  name short long ed type extra wear owner\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("#9Affect being one of:\n\r", ch);
    send_to_char("#y  str dex int wis con quint\n\r", ch);
    send_to_char("#y  hit dam ac hp mana move\n\r", ch);
    send_to_char("  #yattackgood\n\r", ch);
    return;
  }

  if ((obj = get_obj_world(ch, arg1)) == NULL)
  {
    send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
    return;
  }

  if (!IS_JUDGE(ch))
  {
    send_to_char("You don't have permission to change that item.\n\r", ch);
    return;
  }

  /*
   * Snarf the value (which need not be numeric).
   */
  value = atoi(arg3);

  /*
   * Set something.
   */
  if (!str_cmp(arg2, "value0") || !str_cmp(arg2, "v0"))
  {
    if (obj->item_type == ITEM_WEAPON && !IS_JUDGE(ch))
    {
      send_to_char("You are not authorised to create spell weapons.\n\r", ch);
      return;
    }
    else
      obj->value[0] = value;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "value1") || !str_cmp(arg2, "v1"))
  {
    obj->value[1] = value;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "value2") || !str_cmp(arg2, "v2"))
  {
    obj->value[2] = value;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "value3") || !str_cmp(arg2, "v3"))
  {
    if (obj->item_type == ITEM_ARMOR && !IS_JUDGE(ch))
      send_to_char("You are not authorised to create spell armour.\n\r", ch);
    else
    {
      obj->value[3] = value;
      send_to_char("Ok.\n\r", ch);
    }
    return;
  }

  if (!str_cmp(arg2, "extra"))
  {
    if (!str_cmp(arg3, "glow"))
      value = ITEM_GLOW;
    else if (!str_cmp(arg3, "hum"))
      value = ITEM_HUM;
    else if (!str_cmp(arg3, "noshow"))
      value = ITEM_NOSHOW;
    else if (!str_cmp(arg3, "vanish"))
      value = ITEM_VANISH;
    else if (!str_cmp(arg3, "invis"))
      value = ITEM_INVIS;
    else if (!str_cmp(arg3, "magic"))
      value = ITEM_MAGIC;
    else if (!str_cmp(arg3, "nodrop"))
      value = ITEM_NODROP;
    else if (!str_cmp(arg3, "bless"))
      value = ITEM_BLESS;
    else if (!str_cmp(arg3, "anti-good"))
      value = ITEM_ANTI_GOOD;
    else if (!str_cmp(arg3, "anti-evil"))
      value = ITEM_ANTI_EVIL;
    else if (!str_cmp(arg3, "anti-neutral"))
      value = ITEM_ANTI_NEUTRAL;
    else if (!str_cmp(arg3, "noremove"))
      value = ITEM_NOREMOVE;
    else if (!str_cmp(arg3, "inventory"))
      value = ITEM_INVENTORY;
    else if (!str_cmp(arg3, "loyal"))
      value = ITEM_LOYAL;
    else if (!str_cmp(arg3, "mastery"))
      value = ITEM_MASTERY;
    else
    {
      send_to_char("Extra flag can be from the following: Glow, Hum, Thrown, Vanish, Invis, Magic, Nodrop, Bless, Anti-Good, Anti-Evil, Anti-Neutral, Noremove, Inventory, Loyal.\n\r", ch);
      return;
    }

    /* Removing magic flag allows multiple enchants */
    if (IS_SET(obj->extra_flags, value) && value == ITEM_MAGIC && !IS_JUDGE(ch))
    {
      send_to_char("Sorry, no can do...\n\r", ch);
      return;
    }

    if (IS_SET(obj->extra_flags, value))
      REMOVE_BIT(obj->extra_flags, value);
    else
      SET_BIT(obj->extra_flags, value);
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "wear"))
  {
    if (!str_cmp(arg3, "none") || !str_cmp(arg3, "clear"))
    {
      obj->wear_flags = 0;
      send_to_char("Ok.\n\r", ch);
      return;
    }
    else if (!str_cmp(arg3, "take"))
    {
      if (IS_SET(obj->wear_flags, ITEM_TAKE))
        REMOVE_BIT(obj->wear_flags, ITEM_TAKE);
      else
        SET_BIT(obj->wear_flags, ITEM_TAKE);
      send_to_char("Ok.\n\r", ch);
      return;
    }
    else if (!str_cmp(arg3, "finger"))
      value = ITEM_WEAR_FINGER;
    else if (!str_cmp(arg3, "neck"))
      value = ITEM_WEAR_NECK;
    else if (!str_cmp(arg3, "body"))
      value = ITEM_WEAR_BODY;
    else if (!str_cmp(arg3, "head"))
      value = ITEM_WEAR_HEAD;
    else if (!str_cmp(arg3, "legs"))
      value = ITEM_WEAR_LEGS;
    else if (!str_cmp(arg3, "feet"))
      value = ITEM_WEAR_FEET;
    else if (!str_cmp(arg3, "hands"))
      value = ITEM_WEAR_HANDS;
    else if (!str_cmp(arg3, "arms"))
      value = ITEM_WEAR_ARMS;
    else if (!str_cmp(arg3, "about"))
      value = ITEM_WEAR_ABOUT;
    else if (!str_cmp(arg3, "waist"))
      value = ITEM_WEAR_WAIST;
    else if (!str_cmp(arg3, "wrist"))
      value = ITEM_WEAR_WRIST;
    else if (!str_cmp(arg3, "hold"))
      value = ITEM_WIELD;
    else if (!str_cmp(arg3, "face"))
      value = ITEM_WEAR_FACE;
    else if (!str_cmp(arg3, "float"))
      value = ITEM_WEAR_FLOAT;
    else if (!str_cmp(arg3, "medal"))
      value = ITEM_WEAR_MEDAL;
    else if (!str_cmp(arg3, "mastery"))
      value = ITEM_WEAR_MASTERY;
    else if (!str_cmp(arg3, "bodyart"))
      value = ITEM_WEAR_BODYART;
    else
    {
      send_to_char("Wear location can be from: None, Take, Finger, Neck, Body, Head, Legs, Hands, Arms, About, Waist, Hold, Face.\n\r", ch);
      return;
    }
    if (IS_SET(obj->wear_flags, ITEM_TAKE))
      value += 1;
    obj->wear_flags = value;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "level"))
  {
    if (value < 1)
      value = 1;
    else if (value > 200)
      value = 200;
    if (!IS_JUDGE(ch))
      send_to_char("You are not authorised to change an items level.\n\r", ch);
    else
    {
      obj->level = value;
      send_to_char("Ok.\n\r", ch);
    }
    return;
  }

  if (!str_cmp(arg2, "weight"))
  {
    obj->weight = value;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "cost"))
  {
    if (value > 100000 && !IS_JUDGE(ch))
      send_to_char("Don't be so damn greedy!\n\r", ch);
    else
    {
      obj->cost = value;
      send_to_char("Ok.\n\r", ch);
    }
    return;
  }

  if (!str_cmp(arg2, "hitroll") || !str_cmp(arg2, "hit"))
  {
    oset_affect(ch, obj, value, APPLY_HITROLL, FALSE);
    return;
  }
  else if (!str_cmp(arg2, "damroll") || !str_cmp(arg2, "dam"))
  {
    oset_affect(ch, obj, value, APPLY_DAMROLL, FALSE);
    return;
  }
  else if (!str_cmp(arg2, "armor") || !str_cmp(arg2, "ac"))
  {
    oset_affect(ch, obj, value, APPLY_AC, FALSE);
    return;
  }
  else if (!str_cmp(arg2, "hitpoints") || !str_cmp(arg2, "hp"))
  {
    oset_affect(ch, obj, value, APPLY_HIT, FALSE);
    return;
  }
  else if (!str_cmp(arg2, "mana"))
  {
    oset_affect(ch, obj, value, APPLY_MANA, FALSE);
    return;
  }
  else if (!str_cmp(arg2, "move") || !str_cmp(arg2, "movement"))
  {
    oset_affect(ch, obj, value, APPLY_MOVE, FALSE);
    return;
  }
  else if (!str_cmp(arg2, "str") || !str_cmp(arg2, "strength"))
  {
    oset_affect(ch, obj, value, APPLY_STR, FALSE);
    return;
  }
  else if (!str_cmp(arg2, "dex") || !str_cmp(arg2, "dexterity"))
  {
    oset_affect(ch, obj, value, APPLY_DEX, FALSE);
    return;
  }
  else if (!str_cmp(arg2, "int") || !str_cmp(arg2, "intelligence"))
  {
    oset_affect(ch, obj, value, APPLY_INT, FALSE);
    return;
  }
  else if (!str_cmp(arg2, "wis") || !str_cmp(arg2, "wisdom"))
  {
    oset_affect(ch, obj, value, APPLY_WIS, FALSE);
    return;
  }
  else if (!str_cmp(arg2, "con") || !str_cmp(arg2, "constitution"))
  {
    oset_affect(ch, obj, value, APPLY_CON, FALSE);
    return;
  }

  if (!str_cmp(arg2, "type"))
  {
    if (!IS_JUDGE(ch))
    {
      send_to_char("You are not authorised to change an item type.\n\r", ch);
      return;
    }
    if (!str_cmp(arg3, "light"))
      obj->item_type = 1;
    else if (!str_cmp(arg3, "scroll"))
      obj->item_type = 2;
    else if (!str_cmp(arg3, "wand"))
      obj->item_type = 3;
    else if (!str_cmp(arg3, "staff"))
      obj->item_type = 4;
    else if (!str_cmp(arg3, "weapon"))
      obj->item_type = 5;
    else if (!str_cmp(arg3, "treasure"))
      obj->item_type = 8;
    else if (!str_cmp(arg3, "armor"))
      obj->item_type = 9;
    else if (!str_cmp(arg3, "armour"))
      obj->item_type = 9;
    else if (!str_cmp(arg3, "potion"))
      obj->item_type = 10;
    else if (!str_cmp(arg3, "furniture"))
      obj->item_type = 12;
    else if (!str_cmp(arg3, "trash"))
      obj->item_type = 13;
    else if (!str_cmp(arg3, "container"))
      obj->item_type = 15;
    else if (!str_cmp(arg3, "drink"))
      obj->item_type = 17;
    else if (!str_cmp(arg3, "key"))
      obj->item_type = 18;
    else if (!str_cmp(arg3, "food"))
      obj->item_type = 19;
    else if (!str_cmp(arg3, "money"))
      obj->item_type = 20;
    else if (!str_cmp(arg3, "boat"))
      obj->item_type = 22;
    else if (!str_cmp(arg3, "corpse"))
      obj->item_type = 23;
    else if (!str_cmp(arg3, "fountain"))
      obj->item_type = 25;
    else if (!str_cmp(arg3, "pill"))
      obj->item_type = 26;
    else if (!str_cmp(arg3, "portal"))
      obj->item_type = 27;
    else if (!str_cmp(arg3, "stake"))
      obj->item_type = 30;
    else
    {
      send_to_char
        ("Type can be one of: Light, Scroll, Wand, Staff, Weapon, Treasure, Armor, Potion, Furniture, Trash, Container, Drink, Key, Food, Money, Boat, Corpse, Fountain, Pill, Portal, Stake.\n\r", ch);
      return;
    }
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "owner"))
  {
    if (IS_NPC(ch))
    {
      send_to_char("Not while switched.\n\r", ch);
      return;
    }
    if (!IS_JUDGE(ch))
    {
      send_to_char("Someone else has already changed this item.\n\r", ch);
      return;
    }
    if ((victim = get_char_world(ch, arg3)) == NULL)
    {
      send_to_char("You cannot find any player by that name.\n\r", ch);
      return;
    }
    if (IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }
    if (obj->questowner != NULL)
      free_string(obj->questowner);
    obj->questowner = str_dup(victim->name);
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "name"))
  {
    free_string(obj->name);
    obj->name = str_dup(arg3);
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "short"))
  {
    free_string(obj->short_descr);
    obj->short_descr = str_dup(arg3);
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "long"))
  {
    free_string(obj->description);
    obj->description = str_dup(arg3);
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "ed"))
  {
    EXTRA_DESCR_DATA *ed;

    argument = one_argument(argument, arg3);
    if (argument == NULL)
    {
      send_to_char("Syntax: oset <object> ed <keyword> <string>\n\r", ch);
      return;
    }

    if ((ed = (EXTRA_DESCR_DATA *) PopStack(extra_descr_free)) == NULL)
    {
      ed = calloc(1, sizeof(*ed));
    }

    ed->keyword = str_dup(arg3);
    ed->description = str_dup(argument);

    AttachToList(ed, obj->extra_descr);
    send_to_char("Ok.\n\r", ch);
    return;
  }

  /*
   * Generate usage message.
   */
  do_oset(ch, "");
  return;
}

void do_rset(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  int value;

  smash_tilde(argument);
  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  strcpy(arg3, argument);

  if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
  {
    send_to_char("#9Syntax: rset <location> <field> value\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("#9Field being one of:\n\r", ch);
    send_to_char("#y  flags sector\n\r", ch);
    return;
  }

  if ((location = find_location(ch, arg1)) == NULL)
  {
    send_to_char("No such location.\n\r", ch);
    return;
  }

  /*
   * Snarf the value.
   */
  if (!is_number(arg3))
  {
    send_to_char("Value must be numeric.\n\r", ch);
    return;
  }
  value = atoi(arg3);

  /*
   * Set something.
   */
  if (!str_cmp(arg2, "flags"))
  {
    location->room_flags = value;
    return;
  }

  if (!str_cmp(arg2, "sector"))
  {
    location->sector_type = value;
    return;
  }

  /*
   * Generate usage message.
   */
  do_rset(ch, "");
  return;
}

void do_omni(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  ITERATOR *pIter;
  DESCRIPTOR_DATA *d;

  if (IS_NPC(ch))
    return;

  sprintf(buf, " Name        |Lvl|Tst|Gen|  Hit | Mana | Move |  HR|  DR|   AC| Gold | Pow\n\r");
  send_to_char(buf, ch);
  sprintf(buf, "-------------|---|---|---|------|------|------|----|----|-----|------|-----\n\r");
  send_to_char(buf, ch);

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    CHAR_DATA *wch;

    if (d->connected != CON_PLAYING)
      continue;
    wch = d->character;

    sprintf(buf, "%-13s|%3d|%3d|%3d|%6d|%6d|%6d|%4d|%4d|%5d|%6d|%5d\n\r",
            wch->name, wch->level, wch->trust, wch->generation,
            wch->max_hit, wch->max_mana, wch->max_move, char_hitroll(wch),
            char_damroll(wch), char_ac(wch), getGold(wch), getMight(wch));

    send_to_char(buf, ch);
  }

  return;
}

void do_users(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;
  int count;
  char *st;

  count = 0;
  buf[0] = '\0';

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->character != NULL && can_see(ch, d->character))
    {
      count++;

      switch (d->connected)
      {
        case CON_PLAYING:
          st = "#yPLAYING#n";
          break;
        case CON_NOTE_TO:
        case CON_NOTE_SUBJECT:
        case CON_NOTE_EXPIRE:
        case CON_NOTE_TEXT:
        case CON_NOTE_FINISH:
          st = "#gWriting Note#n";
          break;
        case CON_ACCOUNT_NAME:
        case CON_CONFIRM_ACCOUNT:
          st = "#pAccount Name#n";
          break;
        case CON_NEW_PASSWORD:
        case CON_OLD_PASSWORD:
        case CON_CONFIRM_PASSWORD:
          st = "#pPassword#n";
          break;
        case CON_PICK_PLAYER:
        case CON_PICK_REFERENCE:
          st = "#pIn Account#n";
          break;
        case CON_DELETE_PLAYER:
        case CON_CONFIRM_DEL_PLAYER:
          st = "#pDelete Player#n";
          break;
        case CON_NEW_CHARACTER:
        case CON_CONFIRM_NEW_CHARACTER:
        case CON_GET_NEW_CLASS:
        case CON_CONFIRM_CLASS:
        case CON_GET_NEW_ANSI:
        case CON_GET_NEW_SEX:
          st = "#pCreating Char#n";
          break;
        case CON_NOT_PLAYING:
          st = "#yPending Quit#n";
          break;
        default:
          st = "#c!UNKNOWN!#n";
          break;
      }

      cprintf(buf + strlen(buf), "#G[#9%3d  %-15s#G]  #G%s#0@#G%s\n\r",
        d->descriptor, st, d->character ? d->character->name : "(none)", HOSTNAME(d));
    }
  }

  sprintf(buf2, " %d user%s\n\r", count, count == 1 ? "" : "s");
  send_to_char(buf2, ch);
  send_to_char(buf, ch);
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  bool afk;

  argument = one_argument(argument, arg);

  if (arg[0] == '\0' || argument[0] == '\0')
  {
    send_to_char("Force whom to do what?\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "all"))
  {
    CHAR_DATA *vch;
    ITERATOR *pIter;
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "%s force all : %s\n\r", ch->name, argument);

    if (get_trust(ch) < MAX_LEVEL - 3)
    {
      send_to_char("Not at your level!\n\r", ch);
      return;
    }

    pIter = AllocIterator(char_list);
    while ((vch = (CHAR_DATA *) NextInList(pIter)) != NULL)
    {
      if (!IS_NPC(vch) && get_trust(vch) < get_trust(ch))
      {
        if (IS_SET(vch->extra, EXTRA_AFK))
          afk = TRUE;
        else
          afk = FALSE;
        act(buf, ch, NULL, vch, TO_VICT);
        interpret(vch, argument);
        if (afk)
          SET_BIT(vch->extra, EXTRA_AFK);
      }
    }
  }

  else
  {
    CHAR_DATA *victim;

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
      send_to_char("They aren't here.\n\r", ch);
      return;
    }

    if (victim == ch)
    {
      send_to_char("Aye aye, right away!\n\r", ch);
      return;
    }

    if ((get_trust(victim) >= get_trust(ch)) && (ch->level < MAX_LEVEL))
    {
      send_to_char("Do it yourself!\n\r", ch);
      return;
    }

    act("$n forces you to '$t'.", ch, argument, victim, TO_VICT);
    interpret(victim, argument);
  }

  send_to_char("Ok.\n\r", ch);
  return;
}

void do_forceauto(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *vch;
  ITERATOR *pIter;

  pIter = AllocIterator(char_list);
  while ((vch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!IS_NPC(vch) && vch != ch)
    {
      act("Autocommand: $t.", ch, argument, vch, TO_VICT);
      interpret(vch, argument);
    }
  }
}

/*
 * New routines by Dionysos.
 */
void do_invis(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->act, PLR_HIDE))
  {
    REMOVE_BIT(ch->act, PLR_HIDE);
    act("$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You slowly fade back into existence.\n\r", ch);
  }
  else
  {
    act("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You slowly vanish into thin air.\n\r", ch);
    SET_BIT(ch->act, PLR_HIDE);
  }
}

void do_holylight(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->act, PLR_HOLYLIGHT))
  {
    REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
    send_to_char("#yHoly light mode off.\n\r", ch);
  }
  else
  {
    SET_BIT(ch->act, PLR_HOLYLIGHT);
    send_to_char("#yHoly light mode on.\n\r", ch);
  }

  return;
}

void do_safe(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    send_to_char("You cannot be attacked by other players here.\n\r", ch);
  else
    send_to_char("You are not safe from player attacks in this room.\n\r", ch);

}

void do_oclone(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  ITERATOR *pIter;
  OBJ_DATA *obj;
  OBJ_DATA *obj2;
  AFFECT_DATA *paf;
  AFFECT_DATA *paf2;

  argument = one_argument(argument, arg1);

  if (arg1[0] == '\0')
  {
    send_to_char("Make a clone of what object?\n\r", ch);
    return;
  }

  if ((obj = get_obj_world(ch, arg1)) == NULL)
  {
    send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
    return;
  }

  if (!IS_JUDGE(ch))
  {
    send_to_char("You can only clone your own creations.\n\r", ch);
    return;
  }

  pObjIndex = get_obj_index(obj->pIndexData->vnum);
  obj2 = create_object(pObjIndex, obj->level);
  /* Copy any changed parts of the object. */
  free_string(obj2->name);
  obj2->name = str_dup(obj->name);
  free_string(obj2->short_descr);
  obj2->short_descr = str_dup(obj->short_descr);
  free_string(obj2->description);
  obj2->description = str_dup(obj->description);

  obj2->item_type = obj->item_type;
  obj2->extra_flags = obj->extra_flags;
  obj2->wear_flags = obj->wear_flags;
  obj2->weight = obj->weight;
  obj2->condition = obj->condition;
  obj2->toughness = obj->toughness;
  obj2->resistance = obj->resistance;
  obj2->quest = obj->quest;
  obj2->cost = obj->cost;
  obj2->value[0] = obj->value[0];
  obj2->value[1] = obj->value[1];
  obj2->value[2] = obj->value[2];
  obj2->value[3] = obj->value[3];
    /*****************************************/
  obj_to_char(obj2, ch);

  if (SizeOfList(obj->affected) > 0)
  {
    pIter = AllocIterator(obj->affected);
    while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
    {
      if ((paf2 = (AFFECT_DATA *) PopStack(affect_free)) == NULL)
        paf2 = malloc(sizeof(*paf));

      paf2->type = 0;
      paf2->duration = paf->duration;
      paf2->location = paf->location;
      paf2->modifier = paf->modifier;
      paf2->bitvector = 0;

      AttachToList(paf2, obj2->affected);
    }
  }

  act("You create a clone of $p.", ch, obj, NULL, TO_CHAR);
}

void do_locate(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  ITERATOR *pIter;
  bool found;

  if (IS_NPC(ch))
  {
    send_to_char("Not while switched.\n\r", ch);
    return;
  }

  found = FALSE;
  pIter = AllocIterator(object_list);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (!can_see_obj(ch, obj) || obj->questowner == NULL || strlen(obj->questowner) < 2 || str_cmp(ch->name, obj->questowner))
      continue;

    found = TRUE;

    for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj)
      ;

    if (in_obj->carried_by != NULL)
    {
      sprintf(buf, "%s carried by %s.\n\r", obj->short_descr, PERS(in_obj->carried_by, ch));
    }
    else
    {
      sprintf(buf, "%s in %s.\n\r", obj->short_descr, in_obj->in_room == NULL ? "somewhere" : in_obj->in_room->name);
    }

    buf[0] = UPPER(buf[0]);
    send_to_char(buf, ch);
  }

  if (!found)
    send_to_char("You cannot locate any items belonging to you.\n\r", ch);

  return;
}

void do_unclaim(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  if (IS_AFFECTED(ch, AFF_CHARM))
  {
    send_to_char("You are not in a state of mind to do that.\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (IS_NPC(ch)) return;
  if (arg[0] == '\0')
  {
    send_to_char("What object do you wish to renounce ownership of?\n\r", ch);
    return;
  }
  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You are not carrying that item.\n\r", ch);
    return;
  }
  if (obj->ownerid != ch->pcdata->playerid)
  {
    send_to_char("You do not own this item.\n\r", ch);
    return;
  }
  if (IS_OBJ_STAT(obj, ITEM_MASTERY) || IS_OBJ_STAT(obj, ITEM_LOYAL))
  {
    send_to_char("You cannot unclaim this item.\n\r", ch);
    return;
  }

  obj->ownerid = 0;
  free_string(obj->questowner);
  obj->questowner = str_dup("");

  send_to_char("Ok.\n\r", ch);
}

void do_claim(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  one_argument(argument, arg);

  if (IS_NPC(ch))
  {
    send_to_char("Not while switched.\n\r", ch);
    return;
  }

  if (ch->exp < 500)
  {
    send_to_char("It costs 500 exp to claim ownership of an item.\n\r", ch);
    return;
  }

  if (arg[0] == '\0')
  {
    send_to_char("What object do you wish to claim ownership of?\n\r", ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You are not carrying that item.\n\r", ch);
    return;
  }

  if (IS_OBJ_STAT(obj, ITEM_NOCLAIM))
  {
    send_to_char("You cannot claim this item.\n\r", ch);
    return;
  }

  if (obj->item_type == ITEM_QUEST || obj->item_type == ITEM_MONEY || obj->item_type == ITEM_TREASURE || IS_SET(obj->quest, QUEST_ARTIFACT))
  {
    send_to_char("You cannot claim that item.\n\r", ch);
    return;
  }

  if (obj->ownerid != 0)
  {
    if (obj->ownerid == ch->pcdata->playerid)
      send_to_char("But you already own it.\n\r", ch);
    else
      send_to_char("Someone else already own it.\n\r", ch);
    return;
  }

  if (obj->questowner != NULL && strlen(obj->questowner) > 1)
  {
    if (!str_cmp(ch->name, obj->questowner))
    {
      if (obj->ownerid != 0)
        send_to_char("But you already own it!\n\r", ch);
      else
      {
        send_to_char("You #Creclaim#n the object.\n\r", ch);
        obj->ownerid = ch->pcdata->playerid;
      }
    }
    else
      send_to_char("Someone else has already claimed ownership to it.\n\r", ch);
    return;
  }

  ch->exp -= 500;
  if (obj->questowner != NULL)
    free_string(obj->questowner);
  obj->questowner = str_dup(ch->name);
  obj->ownerid = ch->pcdata->playerid;
  act("You are now the owner of $p.", ch, obj, NULL, TO_CHAR);
  act("$n is now the owner of $p.", ch, obj, NULL, TO_ROOM);
  return;
}

void do_gift(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  CHAR_DATA *victim;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (IS_AFFECTED(ch, AFF_CHARM))
  {
    send_to_char("You are not in a state of mind to do that.\n\r", ch);
    return;
  }

  if (IS_NPC(ch))
  {
    send_to_char("Not while switched.\n\r", ch);
    return;
  }

  if (ch->exp < 500)
  {
    send_to_char("It costs 500 exp to make a gift of an item.\n\r", ch);
    return;
  }

  if (arg1[0] == '\0' || arg2[0] == '\0')
  {
    send_to_char("Make a gift of which object to whom?\n\r", ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg1)) == NULL)
  {
    send_to_char("You are not carrying that item.\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg2)) == NULL)
  {
    send_to_char("Nobody here by that name.\n\r", ch);
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("Not on NPC's.\n\r", ch);
    return;
  }
  if (obj->ownerid == 0)
  {
    send_to_char("That item has not yet been claimed.\n\r", ch);
    return;
  }
  if (obj->ownerid != ch->pcdata->playerid && ch->level < 7)
  {
    send_to_char("But you don't own it!\n\r", ch);
    return;
  }
  if (IS_SET(obj->quest, QUEST_RELIC) && ch->level < 7)
  {
    send_to_char("You can't gift relics.\n\r", ch);
    return;
  }
  if ((IS_SET(obj->extra_flags, ITEM_MASTERY) || IS_OBJ_STAT(obj, ITEM_LOYAL)) && ch->level < 12)
  {
    send_to_char("This item cannot be gifted away.\n\r", ch);
    return;
  }

  ch->exp -= 500;
  if (obj->questowner != NULL)
    free_string(obj->questowner);
  obj->questowner = str_dup(victim->name);
  obj->ownerid = victim->pcdata->playerid;
  act("You grant ownership of $p to $N.", ch, obj, victim, TO_CHAR);
  act("$n grants ownership of $p to $N.", ch, obj, victim, TO_NOTVICT);
  act("$n grants ownership of $p to you.", ch, obj, victim, TO_VICT);
  return;
}

void do_create(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;
  int itemtype = 13;
  int level;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0')
    itemtype = ITEM_TRASH;
  else if (!str_cmp(arg1, "light"))
    itemtype = ITEM_LIGHT;
  else if (!str_cmp(arg1, "scroll"))
    itemtype = ITEM_SCROLL;
  else if (!str_cmp(arg1, "wand"))
    itemtype = ITEM_WAND;
  else if (!str_cmp(arg1, "staff"))
    itemtype = ITEM_STAFF;
  else if (!str_cmp(arg1, "weapon"))
    itemtype = ITEM_WEAPON;
  else if (!str_cmp(arg1, "treasure"))
    itemtype = ITEM_TREASURE;
  else if (!str_cmp(arg1, "armor"))
    itemtype = ITEM_ARMOR;
  else if (!str_cmp(arg1, "armour"))
    itemtype = ITEM_ARMOR;
  else if (!str_cmp(arg1, "potion"))
    itemtype = ITEM_POTION;
  else if (!str_cmp(arg1, "furniture"))
    itemtype = ITEM_FURNITURE;
  else if (!str_cmp(arg1, "trash"))
    itemtype = ITEM_TRASH;
  else if (!str_cmp(arg1, "container"))
    itemtype = ITEM_CONTAINER;
  else if (!str_cmp(arg1, "drink"))
    itemtype = ITEM_DRINK_CON;
  else if (!str_cmp(arg1, "key"))
    itemtype = ITEM_KEY;
  else if (!str_cmp(arg1, "food"))
    itemtype = ITEM_FOOD;
  else if (!str_cmp(arg1, "money"))
    itemtype = ITEM_MONEY;
  else if (!str_cmp(arg1, "boat"))
    itemtype = ITEM_BOAT;
  else if (!str_cmp(arg1, "corpse"))
    itemtype = ITEM_CORPSE_NPC;
  else if (!str_cmp(arg1, "fountain"))
    itemtype = ITEM_FOUNTAIN;
  else if (!str_cmp(arg1, "pill"))
    itemtype = ITEM_PILL;
  else if (!str_cmp(arg1, "portal"))
    itemtype = ITEM_PORTAL;
  else
    itemtype = ITEM_TRASH;

  if (arg2[0] == '\0' || !is_number(arg2))
  {
    level = 0;
  }
  else
  {
    level = atoi(arg2);
    if (level < 1 || level > 50)
    {
      send_to_char("Level should be within range 1 to 50.\n\r", ch);
      return;
    }
  }

  if ((pObjIndex = get_obj_index(OBJ_VNUM_PROTOPLASM)) == NULL)
  {
    send_to_char("Error...missing object, please inform KaVir.\n\r", ch);
    return;
  }

  obj = create_object(pObjIndex, level);
  obj->level = level;
  obj->item_type = itemtype;
  obj_to_char(obj, ch);

  act("You reach up into the air and draw out a ball of protoplasm.", ch, obj, NULL, TO_CHAR);
  act("$n reaches up into the air and draws out a ball of protoplasm.", ch, obj, NULL, TO_ROOM);
  return;
}

void deposit(CHAR_DATA *ch, OBJ_DATA *obj)
{
  /* deposit gold in account */
  if (obj->item_type == ITEM_QUEST)
    setGold(ch, obj->value[0]);

  act("You deposit $p into your account.", ch, obj, NULL, TO_CHAR);
  act("$n deposits $p into $s account.", ch, obj, NULL, TO_ROOM);

  /* get rid of the bag of gold */
  extract_obj(obj);
}

void do_deposit(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("What do you wish to deposit?\n\r", ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You are unable to find that bag of gold.\n\r", ch);
    return;
  }

  if (obj->item_type != ITEM_QUEST)
  {
    send_to_char("That is not a bag of gold.\n\r", ch);
    return;
  }

  deposit(ch, obj);
}

void do_withdraw(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;
  CHAR_DATA *victim;
  int value;

  if (IS_AFFECTED(ch, AFF_CHARM))
  {
    send_to_char("You are not in a state of mind to do that.\n\r", ch);
    return;                                                
  }

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0' || !is_number(arg1))
  {
    send_to_char("Please specify the amount of gold you wish to witdraw.\n\r", ch);
    return;
  }
  else
  {
    value = atoi(arg1);
    if (value < 1 || value > 10000)
    {
      send_to_char("A withdrawel should be between 1 and 10000 goldcrowns.\n\r", ch);
      return;
    }
    else if (value > getGold(ch) && !IS_JUDGE(ch))
    {
      sprintf(buf, "You only have %d goldcrowns left to withdraw.\n\r", getGold(ch));
      send_to_char(buf, ch);
      return;
    }
  }

  victim = get_char_room(ch, arg2);

  if ((pObjIndex = get_obj_index(OBJ_VNUM_PROTOPLASM)) == NULL)
  {
    send_to_char("Error...missing object, please inform an admin.\n\r", ch);
    return;
  }

  setGold(ch, -1 * value);
  if (getGold(ch) < 0)
    setGold(ch, -1 * getGold(ch));
  obj = create_object(pObjIndex, value);
  obj->value[0] = value;
  obj->level = 1;
  obj->cost = value * 1000;
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

  act("You withdraw $p from your account.", ch, obj, NULL, TO_CHAR);
  act("$n withdraws $s from $s account.", ch, obj, NULL, TO_ROOM);
}

void do_clearstats(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  clearstats(ch);
}

void do_otransfer(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *chroom;
  ROOM_INDEX_DATA *objroom;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0')
  {
    send_to_char("Otransfer which object?\n\r", ch);
    return;
  }

  if (arg2[0] == '\0')
    victim = ch;
  else if ((victim = get_char_world(ch, arg2)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if ((obj = get_obj_world(ch, arg1)) == NULL)
  {
    send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
    return;
  }

  if (obj->carried_by != NULL)
  {
    act("$p vanishes from your hands in an explosion of energy.", obj->carried_by, obj, NULL, TO_CHAR);
    act("$p vanishes from $n's hands in an explosion of energy.", obj->carried_by, obj, NULL, TO_ROOM);
    obj_from_char(obj);
  }
  else if (obj->in_obj != NULL)
    obj_from_obj(obj);
  else if (obj->in_room != NULL)
  {
    chroom = ch->in_room;
    objroom = obj->in_room;
    char_from_room(ch);
    char_to_room(ch, objroom, TRUE);
    act("$p vanishes from the ground in an explosion of energy.", ch, obj, NULL, TO_ROOM);
    if (chroom == objroom)
      act("$p vanishes from the ground in an explosion of energy.", ch, obj, NULL, TO_CHAR);
    char_from_room(ch);
    char_to_room(ch, chroom, TRUE);
    obj_from_room(obj);
  }
  else
  {
    send_to_char("You were unable to get it.\n\r", ch);
    return;
  }
  obj_to_char(obj, victim);
  act("$p appears in your hands in an explosion of energy.", victim, obj, NULL, TO_CHAR);
  act("$p appears in $n's hands in an explosion of energy.", victim, obj, NULL, TO_ROOM);
  return;
}

void do_copyover(CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;
  extern int port, control;
  char buf[100], buf2[100];

  /* save all changed areas - unless asked not to */
  if (str_cmp(argument, "nosave"))
    do_asave(ch, "changed");

  if (pre_reboot_actions(TRUE) == FALSE)
  {
    send_to_char("Reboot failed!\n\r", ch);
    return;
  }

  sprintf(buf, "\n\r <*>         It is a time of changes         <*>\n\r");

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
    write_to_descriptor_2(d->descriptor, buf, 0);

  /* exec - descriptors are inherited */
  sprintf(buf, "%d", port);
  sprintf(buf2, "%d", control);
  execl(EXE_FILE, "Dystopia", buf, "copyover", buf2, (char *) NULL);

  /* copyover failed, damn! */
  log_string("Copyover Failed!");
}

/* Recover from a copyover - load players */
void copyover_recover()
{
  DESCRIPTOR_DATA *d;
  ACCOUNT_DATA *acc;
  FILE *fp;
  char name[100];
  char host[MAX_STRING_LENGTH], account[100];
  int desc;
  bool fOld;

  log_string("Copyover recovery initiated");

  fp = fopen(COPYOVER_FILE, "r");

  if (!fp)                      /* there are some descriptors open which will hang forever then ? */
  {
    perror("copyover_recover:fopen");
    log_string("Copyover file not found. Exitting.");
    exit(1);
  }

  unlink(COPYOVER_FILE);        /* In case something crashes - doesn't prevent reading  */

  for (;;)
  {
    fscanf(fp, "%d %s %s %s\n", &desc, name, account, host);
    if (desc == -1)
      break;

    /* Write something, and check if it goes error-free */
    if (!write_to_descriptor_2(desc, "\n\r <*>             The world spins             <*>\n\r", 0))
    {
      close(desc);
      continue;
    }

    d = calloc(1, sizeof(DESCRIPTOR_DATA));
    init_descriptor(d, desc);   /* set up various stuff */

    if ((acc = load_account(account)) == NULL)
    {
      char bugbuf[MAX_STRING_LENGTH];

      sprintf(bugbuf, "Copyover Recover: Account %s gone", account);
      bug(bugbuf, 0);
      continue;
    }
    d->account = acc;

    d->hostname = str_dup(host);
    AttachToList(d, descriptor_list);
    d->connected = CON_COPYOVER_RECOVER;  /* -15, so close_socket frees the char */
    d->bResolved = TRUE;

    /* Now, find the pfile */

    fOld = load_char_obj(d, name);

    if (!fOld)                  /* Player file not found?! */
    {
      write_to_descriptor_2(desc, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
      close_socket(d);
    }
    else                        /* ok! */
    {
      write_to_descriptor_2(desc, "\n\r <*> And nothing will ever be the same again <*>\n\r", 0);

      login_char(d->character, FALSE);
      act("$n materializes!", d->character, NULL, NULL, TO_ROOM);
      negotiate(d);
    }
  }
  fclose(fp);
}

void do_generation(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  int gen;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (IS_NPC(ch))
    return;

  if (arg1[0] == '\0' || arg2[0] == '\0')
  {
    send_to_char("Syntax: generation <char> <generation>.\n\r", ch);
    send_to_char("Generation 1 is a Master <Class> and 2 is clan leader.\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg1)) == NULL)
  {
    send_to_char("That player is not here.\n\r", ch);
    return;
  }

  gen = is_number(arg2) ? atoi(arg2) : -1;
   
  send_to_char("Generation Set.\n\r", ch);
  victim->generation = gen;
}

void oset_affect(CHAR_DATA * ch, OBJ_DATA * obj, int value, int affect, bool is_quest)
{
  char buf[MAX_STRING_LENGTH];
  AFFECT_DATA *paf;
  int quest;
  int range;
  int cost;
  int max;

  if (IS_NPC(ch))
  {
    send_to_char("Switch back, smart ass.\n\r", ch);
    return;
  }

  if (value == 0)
  {
    send_to_char("Please enter a positive or negative amount.\n\r", ch);
    return;
  }
  if (!IS_JUDGE(ch) && (obj->questowner == NULL))
  {
    send_to_char("First you must set the owners name on the object.\n\r", ch);
    return;
  }
  if (!IS_JUDGE(ch) && !is_quest)
  {
    send_to_char("That item has already been oset by someone else.\n\r", ch);
    return;
  }
  if (affect == APPLY_STR)
  {
    range = 3;
    cost = 20;
    quest = QUEST_STR;
  }
  else if (affect == APPLY_DEX)
  {
    range = 3;
    cost = 20;
    quest = QUEST_DEX;
  }
  else if (affect == APPLY_INT)
  {
    range = 3;
    cost = 20;
    quest = QUEST_INT;
  }
  else if (affect == APPLY_WIS)
  {
    range = 3;
    cost = 20;
    quest = QUEST_WIS;
  }
  else if (affect == APPLY_CON)
  {
    range = 3;
    cost = 20;
    quest = QUEST_CON;
  }
  else if (affect == APPLY_HIT)
  {
    range = 25;
    cost = 5;
    quest = QUEST_HIT;
  }
  else if (affect == APPLY_MANA)
  {
    range = 25;
    cost = 5;
    quest = QUEST_MANA;
  }
  else if (affect == APPLY_MOVE)
  {
    range = 25;
    cost = 5;
    quest = QUEST_MOVE;
  }
  else if (affect == APPLY_HITROLL)
  {
    range = 5;
    cost = 30;
    quest = QUEST_HITROLL;
  }
  else if (affect == APPLY_DAMROLL)
  {
    range = 5;
    cost = 30;
    quest = QUEST_DAMROLL;
  }
  else if (affect == APPLY_AC)
  {
    range = 25;
    cost = 10;
    quest = QUEST_AC;
  }
  else
    return;

  if (IS_SET(obj->quest, QUEST_IMPROVED))

    max = 1500;

  else if (obj->pIndexData->vnum == OBJ_VNUM_PROTOPLASM)
  {
    range *= 2;
    max = 1000;
  }
  else
    max = 600;

  if (obj->item_type == ITEM_WEAPON)
  {
    max *= 2;
    range *= 2;
  }

  if (!IS_JUDGE(ch) && ((value > 0 && value > range) || (value < 0 && value < (range - range - range))))
  {

    send_to_char("That is not within the acceptable range...\n\r", ch);
    send_to_char("Str, Dex, Int, Wis, Con... max =   3 each, at  20 quest points per +1 stat.\n\r", ch);
    send_to_char("Hp, Mana, Move............ max =  25 each, at   5 quest point per point.\n\r", ch);
    send_to_char("Hitroll, Damroll.......... max =   5 each, at  30 quest points per point.\n\r", ch);
    send_to_char("Ac........................ max = -25,      at  10 points per point.\n\r", ch);
    send_to_char("\n\rNote: Created items can have upto 2 times the above maximum.\n\r", ch);
    send_to_char("Also: Weapons may have upto 2 (4 for created) times the above maximum.\n\r", ch);
    return;
  }
  if (quest >= QUEST_HIT && value < 0)
    cost *= (value - (value * 2));
  else
    cost *= value;

  if (cost < 0)
    cost = 0;

  if (!IS_JUDGE(ch) && IS_SET(obj->quest, quest))
  {
    send_to_char("That affect has already been set on this object.\n\r", ch);
    return;
  }
  if (is_quest && getGold(ch) < cost && !IS_JUDGE(ch))
  {
    sprintf(buf, "That costs %d goldcrowns, while you only have %d.\n\r", cost, getGold(ch));
    send_to_char(buf, ch);
    return;
  }
  if (!IS_SET(obj->quest, quest))
    SET_BIT(obj->quest, quest);
  if (is_quest)
    setGold(ch, -1 * cost);

  if ((paf = (AFFECT_DATA *) PopStack(affect_free)) == NULL)
    paf = malloc(sizeof(*paf));

  paf->type = 0;
  paf->duration = -1;
  paf->location = affect;
  paf->modifier = value;
  paf->bitvector = 0;

  AttachToList(paf, obj->affected);
}

void do_wizallow(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch)) return;
  argument = one_argument(argument, arg);

  if (arg[0] == '\0' || argument[0] == '\0')
  {
    send_to_char("Syntax: wizallow [player] [command]\n\r", ch);
    return;
  }
  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("Not on mobiles.\n\r", ch);
    return;
  }
  if (strstr(victim->pcdata->immcmd, argument) != '\0')
  {
    victim->pcdata->immcmd = string_replace(victim->pcdata->immcmd, argument, "\0");
    victim->pcdata->immcmd = string_unpad(victim->pcdata->immcmd);

    send_to_char("command removed.\n\r", ch);
  }
  else
  {
    int cmd;

    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
    {
      if (!str_cmp(argument, cmd_table[cmd].name))
        break;
    }
    if (cmd_table[cmd].name[0] == '\0')
    {
      send_to_char("There is no command by that name.\n\r", ch);
      return;
    }

    buf[0] = '\0';
    if (victim->pcdata->immcmd[0] != '\0')
    {
      strcat(buf, victim->pcdata->immcmd);
      strcat(buf, " ");
    }
    strcat(buf, argument);
    free_string(victim->pcdata->immcmd);
    victim->pcdata->immcmd = str_dup(buf);
    send_to_char("command added.\n\r", ch);
  }
}

void do_logstat(CHAR_DATA *ch, char *argument)
{
  int a, b, c, d;

  printf_to_char(ch, "Total Players Created                 : %4d player%s\n\r",
    muddata.top_playerid, (muddata.top_playerid == 1) ? "" : "s");

  printf_to_char(ch, "Mudinfo : Days running                : %4d day%s and %d hour%s\n\r",
    (muddata.mudinfo[MUDINFO_UPDATED] / (120 * 24)),
    (muddata.mudinfo[MUDINFO_UPDATED] / (120 * 24) == 1) ? "" : "s",
    (muddata.mudinfo[MUDINFO_UPDATED] / 120 - 24 * (muddata.mudinfo[MUDINFO_UPDATED] / (120 * 24))),
    (muddata.mudinfo[MUDINFO_UPDATED] / 120 - 24 * (muddata.mudinfo[MUDINFO_UPDATED] / (120 * 24)) == 1) ? "" : "s");
  printf_to_char(ch, "Players : Average Online              : %2d.%d\n\r",
    (muddata.mudinfo[MUDINFO_MCCP_USERS] + muddata.mudinfo[MUDINFO_OTHER_USERS]) / 
     muddata.mudinfo[MUDINFO_UPDATED], (10 * (muddata.mudinfo[MUDINFO_MCCP_USERS] + 
     muddata.mudinfo[MUDINFO_OTHER_USERS]) / muddata.mudinfo[MUDINFO_UPDATED]) % 10);
  printf_to_char(ch, "Players : Peak Online                 : %4d\n\r", 
    muddata.mudinfo[MUDINFO_PEAK_USERS]);
  printf_to_char(ch, "Players : Mccp Users                  : %4d %%\n\r",
    (muddata.mudinfo[MUDINFO_MCCP_USERS] + muddata.mudinfo[MUDINFO_OTHER_USERS] == 0)
    ? 0
    : (100 * muddata.mudinfo[MUDINFO_MCCP_USERS]) / (muddata.mudinfo[MUDINFO_MCCP_USERS] + muddata.mudinfo[MUDINFO_OTHER_USERS]));
  printf_to_char(ch, "Players : MSP Users                   : %4d %%\n\r",
    (muddata.mudinfo[MUDINFO_MCCP_USERS] + muddata.mudinfo[MUDINFO_OTHER_USERS] == 0)
    ? 0
    : (100 * muddata.mudinfo[MUDINFO_MSP_USERS]) / (muddata.mudinfo[MUDINFO_MCCP_USERS] + muddata.mudinfo[MUDINFO_OTHER_USERS]));
  printf_to_char(ch, "Players : MXP Users                   : %4d %%\n\r",
    (muddata.mudinfo[MUDINFO_MCCP_USERS] + muddata.mudinfo[MUDINFO_OTHER_USERS] == 0)
    ? 0
    : (100 * muddata.mudinfo[MUDINFO_MXP_USERS]) / (muddata.mudinfo[MUDINFO_MCCP_USERS] + muddata.mudinfo[MUDINFO_OTHER_USERS]));

  /* Datatransfer average, peak and amount in megabytes */
  printf_to_char(ch, "Datatransfer : Average Rate           : %2d.%d kb/sec\n\r",
    ((muddata.mudinfo[MUDINFO_MBYTE] * 1024 + muddata.mudinfo[MUDINFO_BYTE] / 1024) / (muddata.mudinfo[MUDINFO_UPDATED] * 3)) / 10,
    ((muddata.mudinfo[MUDINFO_MBYTE] * 1024 + muddata.mudinfo[MUDINFO_BYTE] / 1024) / (muddata.mudinfo[MUDINFO_UPDATED] * 3)) % 10);
  printf_to_char(ch, "Datatransfer : Peak Rate              : %2d.%d kb/sec\n\r",
    (muddata.mudinfo[MUDINFO_DATA_PEAK] / (3 * 1024)) / 10,
    (muddata.mudinfo[MUDINFO_DATA_PEAK] / (3 * 1024)) % 10);
  printf_to_char(ch, "Datatransfer : This Week              : %4d MB\n\r", 
    muddata.mudinfo[MUDINFO_MBYTE]);
  printf_to_char(ch, "Datatransfer : This Boot              : %4d MB\n\r", 
    muddata.mudinfo[MUDINFO_MBYTE_S]);

  /* count all events statistics */
  count_events(&a, &b, &c, &d);
  printf_to_char(ch, "Events Queued (Total)                 : %4d event%s\n\r",
    a, (a == 1) ? "" : "s");
  printf_to_char(ch, "Events Queued (Largest Bucket)        : %4d event%s\n\r",
    b, (b == 1) ? "" : "s");
  printf_to_char(ch, "Events Queued (Smallest Bucket)       : %4d event%s\n\r",
    c, (c == 1) ? "" : "s");
  printf_to_char(ch, "Events Queued (Average/Bucket)        : %4d event%s\n\r",
    d, (d == 1) ? "" : "s");

  /* send pk info for this week and last week */
  printf_to_char(ch, "PK Data : Decapitations   (this week) : %4d decap%s\n\r",
    muddata.pk_count_now[0], (muddata.pk_count_now[0] == 1) ? "" : "s");
  printf_to_char(ch, "PK Data : Decapitations   (last week) : %4d decap%s\n\r",
    muddata.pk_count_last[0], (muddata.pk_count_last[0] == 1) ? "" : "s");
  printf_to_char(ch, "PK Data : Arena Kills     (this week) : %4d kill%s\n\r",
    muddata.pk_count_now[1], (muddata.pk_count_now[1] == 1) ? "" : "s");
  printf_to_char(ch, "PK Data : Arena Kills     (last week) : %4d kill%s\n\r",
    muddata.pk_count_last[1], (muddata.pk_count_last[1] == 1) ? "" : "s");
  printf_to_char(ch, "PK Data : Gensteals       (this week) : %4d steal%s\n\r",
    muddata.pk_count_now[2], (muddata.pk_count_now[2] == 1) ? "" : "s");
  printf_to_char(ch, "PK Data : Gensteals       (last week) : %4d steal%s\n\r",
    muddata.pk_count_last[2], (muddata.pk_count_last[2] == 1) ? "" : "s");
}

void do_noset(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  if (IS_NPC(ch))
    return;

  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("No such player.\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("Not on mobiles.\n\r", ch);
    return;
  }
  if (IS_SET(victim->pcdata->jflags, JFLAG_NOSET))
  {
    REMOVE_BIT(victim->pcdata->jflags, JFLAG_NOSET);
    send_to_char("Noset removed.\n\r", ch);
    send_to_char("You can set your messages again.\n\r", victim);
    return;
  }

  free_string(victim->pcdata->loginmessage);
  victim->pcdata->loginmessage = str_dup("");
  free_string(victim->pcdata->logoutmessage);
  victim->pcdata->logoutmessage = str_dup("");
  free_string(victim->pcdata->decapmessage);
  victim->pcdata->decapmessage = str_dup("");
  free_string(victim->pcdata->tiemessage);
  victim->pcdata->tiemessage = str_dup("");
  free_string(victim->pcdata->avatarmessage);
  victim->pcdata->avatarmessage = str_dup("");
  REMOVE_BIT(victim->pcdata->jflags, JFLAG_SETTIE);
  REMOVE_BIT(victim->pcdata->jflags, JFLAG_SETAVATAR);
  REMOVE_BIT(victim->pcdata->jflags, JFLAG_SETLOGIN);
  REMOVE_BIT(victim->pcdata->jflags, JFLAG_SETLOGOUT);
  REMOVE_BIT(victim->pcdata->jflags, JFLAG_SETDECAP);
  SET_BIT(victim->pcdata->jflags, JFLAG_NOSET);
  send_to_char("Ok.\n\r", ch);
  send_to_char("All your custom messages have been removed, and you can no longer change your messages.\n\r", victim);
}

void do_showsilence(CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *gch;
  char buf[MAX_STRING_LENGTH];
  ITERATOR *pIter;
  bool found = FALSE;

  if (IS_NPC(ch))
    return;

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (d->connected != CON_PLAYING)
      continue;
    if (d->character != NULL)
      gch = d->character;
    else
      continue;

    if (is_silenced(gch))
    {
      found = TRUE;
      sprintf(buf, "%-15s is silenced\n\r", gch->name);
      send_to_char(buf, ch);
    }
  }

  if (!found)
    send_to_char("Noone is silenced.\n\r", ch);
}

void do_plist(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *vch;
  ACCOUNT_DATA *account;
  char arg[MAX_INPUT_LENGTH];
  struct plist *p_list;
  int status;

  argument = one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Plist what player?\n\r", ch);
    return;
  }

  if (!check_parse_name(arg, TRUE))
  {
    send_to_char("Thats an illegal name.\n\r", ch);
    return;
  }

  /* load the characters whois info */
  if ((vch = load_char_whois(arg, &status)) == NULL)
  {
    if (status == -1)
      send_to_char("That player has not logged since we changed the finger storage.\n\r", ch);
    else if (status == 0)
      send_to_char("That player does not exist.\n\r", ch);
    else
      send_to_char("Something unexpected happened.\n\r", ch);

    return;
  }

  if ((account = load_account(vch->pcdata->account)) == NULL)
  {
    send_to_char("That players account doesn't exist.\n\r", ch);
    free_char(vch);
    return;
  }

  /* check the player list */
  p_list = parse_player_list(account->players);
  if (p_list->count > 0) 
  {
    send_to_char("      #9#uName          Class              Hours  #n\n\r", ch);
    send_to_char(p_list->text, ch);
  }
  else
  {
    send_to_char("For some reason that account has no players.\n\r", ch);
  }

  close_account(account);
  free_char(vch);
  free(p_list);
}

void do_asperson(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  ACCOUNT_DATA *account;
  DESCRIPTOR_DATA *tmp;
  char arg[MAX_INPUT_LENGTH];
  bool afk = FALSE;

  argument = one_argument(argument, arg);

  if ((victim = get_char_world(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }
  if (victim == ch)
  {
    send_to_char("That would be a bad idea.\n\r", ch);
    return;
  }
  if (IS_NPC(victim))
  {
    send_to_char("Not on mobiles.\n\r", ch);
    return;
  }
  if ((tmp = victim->desc) == NULL)
  {
    send_to_char("They are linkdead.\n\r", ch);
    return;
  }
  account = ch->desc->account;
  victim->desc = ch->desc;
  ch->desc->account = tmp->account;
  if (IS_SET(victim->extra, EXTRA_AFK))
  {
    afk = TRUE;
    REMOVE_BIT(victim->extra, EXTRA_AFK);
  }
  interpret(victim, argument);
  victim->desc = tmp;
  ch->desc->account = account;
  if (afk)
    SET_BIT(victim->extra, EXTRA_AFK);
}

void do_newban(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  BAN_DATA *pban;

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    strcpy(buf, "Newbie Banned sites:\n\r");

    pIter = AllocIterator(newbieban_list);
    while ((pban = (BAN_DATA *) NextInList(pIter)) != NULL)
    {
      strcat(buf, pban->name);
      strcat(buf, "    (");
      strcat(buf, pban->reason);
      strcat(buf, ")\n\r");
    }
    send_to_char(buf, ch);
    return;
  }

  pIter = AllocIterator(newbieban_list);
  while ((pban = (BAN_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(arg, pban->name))
    {
      send_to_char("That site is already banned!\n\r", ch);
      return;
    }
  }

  if ((pban = (BAN_DATA *) PopStack(ban_free)) == NULL)
  {
    pban = calloc(1, sizeof(*pban));
  }

  pban->name = str_dup(arg);
  if (argument[0] == '\0')
    pban->reason = str_dup("no reason given");
  else
    pban->reason = str_dup(argument);

  AttachToList(pban, newbieban_list);

  send_to_char("Ok.\n\r", ch);
  save_newbiebans();
}

void do_newallow(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  BAN_DATA *curr;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Remove which site from the ban list?\n\r", ch);
    return;
  }

  pIter = AllocIterator(newbieban_list);
  while ((curr = (BAN_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(arg, curr->name))
    {
      DetachAtIterator(pIter);

      free_string(curr->name);
      free_string(curr->reason);

      PushStack(curr, ban_free);

      send_to_char("Ok.\n\r", ch);
      save_newbiebans();
      return;
    }
  }
  send_to_char("Site is not banned.\n\r", ch);
}

void do_logstatclear(CHAR_DATA * ch, char *argument)
{
  int i;

  if (IS_NPC(ch))
    return;

  for (i = 0; i < MUDINFO_MAX; i++)
  {
    muddata.mudinfo[i] = 0;
  }

  update_mudinfo();
  send_to_char("Cleared.\n\r", ch);
}

void do_leaderclear(CHAR_DATA * ch, char *argument)
{
  int i;

  if (IS_NPC(ch))
    return;
  if (ch->level < 7)
    return;

  for (i = 0; i < MAX_LEADER; i++)
  {
    free_string(leader_board.name[i]);
    leader_board.name[i]    =  str_dup("Noone");
    leader_board.number[i]  =  0;
  }

  save_leaderboard();
  send_to_char("Leader board cleared.\n\r", ch);
}

void do_ccenter(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  argument = one_argument(argument, arg1);
  one_argument(argument, arg2);

  if (arg1[0] == '\0')
  {
    sprintf(buf, "  #0[#G***#0]   #yDystopian Control Center   #0[#G***#0]#n\n\r\n\r");
    send_to_char(buf, ch);
    sprintf(buf, "  #R(#0lexp#R) #yMinimum Experience        #G%7d#n\n\r",
      muddata.ccenter[CCENTER_MIN_EXP]);
    send_to_char(buf, ch);
    sprintf(buf, "  #R(#0uexp#R) #yMaximum Experience        #G%7d#n\n\r",
      muddata.ccenter[CCENTER_MAX_EXP]);
    send_to_char(buf, ch);
    sprintf(buf, "  #R(#0elvl#R) #yExperience Level              #G%3d#n\n\r",
      muddata.ccenter[CCENTER_EXP_LEVEL]);
    send_to_char(buf, ch);
    sprintf(buf, "  #R(#0qlvl#R) #yGoldcrowns Level              #G%3d#n\n\r", 
      muddata.ccenter[CCENTER_QPS_LEVEL]);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\r'ccenter reset' to restore default values.\n\r", ch);
    return;
  }
  if (!str_cmp(arg1, "lexp"))
  {
    muddata.ccenter[CCENTER_MIN_EXP] = atoi(arg2);
    send_to_char("Ok. Value Changed.\n\r", ch);
    save_muddata();
    do_ccenter(ch, "");
    return;
  }
  else if (!str_cmp(arg1, "uexp"))
  {
    muddata.ccenter[CCENTER_MAX_EXP] = atoi(arg2);
    send_to_char("Ok. Value Changed.\n\r", ch);
    save_muddata();
    do_ccenter(ch, "");
    return;
  }
  else if (!str_cmp(arg1, "elvl"))
  {
    muddata.ccenter[CCENTER_EXP_LEVEL] = atoi(arg2);
    send_to_char("Ok. Value Changed.\n\r", ch);
    save_muddata();
    do_ccenter(ch, "");
    return;
  }
  else if (!str_cmp(arg1, "qlvl"))
  {
    muddata.ccenter[CCENTER_QPS_LEVEL] = atoi(arg2);   
    send_to_char("Ok. Value Changed.\n\r", ch);
    save_muddata();
    do_ccenter(ch, "");
    return;
  }
  else if (!str_cmp(arg1, "reset"))
  {
    muddata.ccenter[CCENTER_MIN_EXP] = CCENTER_MIN_EXP_DEFAULT;
    muddata.ccenter[CCENTER_MAX_EXP] = CCENTER_MAX_EXP_DEFAULT;
    muddata.ccenter[CCENTER_EXP_LEVEL] = CCENTER_EXP_LEVEL_DEFAULT;
    muddata.ccenter[CCENTER_QPS_LEVEL] = CCENTER_QPS_LEVEL_DEFAULT;
    send_to_char("Ok. Values Reset.\n\r", ch);
    save_muddata();
    do_ccenter(ch, "");
    return;
  }
  else
  {
    send_to_char("Nope.\n\r", ch);
    return;
  }
}

void do_displayvotes(CHAR_DATA * ch, char *argument)
{
  POLL_DATA *poll;
  VOTE_DATA *vote;
  ITERATOR *pIter, *pIter2;
  char arg[MAX_INPUT_LENGTH];
  int i = 0;

  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("What poll do you wish to display?\n\r", ch);
    return;
  }

  pIter = AllocIterator(poll_list);
  while ((poll = (POLL_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(poll->name, arg))
    {
      printf_to_char(ch, "     #G[#R***#G] #CVotes for the poll on #y%s #G[#R***#G]#n\n\r\n\r", poll->name);

      pIter2 = AllocIterator(poll->votes);
      while ((vote = (VOTE_DATA *) NextInList(pIter2)) != NULL)
      {
        if (i++ > 50)
          break;
        printf_to_char(ch, "%-12s  %-40s %2d\n\r", vote->pname, vote->phost, vote->choice);
      }

      send_to_char("\n\r#GScore:", ch);

      for (i = 0; i < MAX_VOTE_OPTIONS; i++)
      {
        if (str_cmp(poll->options[i], "<null>"))
          printf_to_char(ch, "   #R[#C%d#R] #G%d votes", i + 1, poll->vcount[i]);
      }
      send_to_char("#n\n\r", ch);
      return;
    }
  }

  send_to_char("No such poll.\n\r", ch);
}

void do_addpoll(CHAR_DATA * ch, char *argument)
{
  POLL_DATA *poll;
  char arg[MAX_INPUT_LENGTH];
  char darg[MAX_INPUT_LENGTH];
  int i, days;

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, arg);
  one_argument(argument, darg);

  if (arg[0] == '\0')
  {
    send_to_char("Which poll do you wish to start?\n\r", ch);
    return;
  }

  days = atoi(darg);
  if (days < 1 || days > 10)
  {
    send_to_char("Between 1 and 10 days please.\n\r", ch);
    return;
  }

  poll = malloc(sizeof(*poll));
  poll->name = str_dup(arg);
  poll->description = str_dup("This poll has no descripton.");
  poll->expire = current_time + (days * 24L * 3600L);
  poll->votes = AllocList();

  for (i = 0; i < MAX_VOTE_OPTIONS; i++)
  {
    poll->options[i] = str_dup("<null>");
    poll->vcount[i] = 0;
  }

  AttachToList(poll, poll_list);

  send_to_char("Poll added, you'll need to edit it with polledit.\n\r", ch);
  save_polls();
  save_subvotes(poll);
}

void do_polledit(CHAR_DATA * ch, char *argument)
{
  POLL_DATA *poll;
  ITERATOR *pIter;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  bool found = FALSE;
  int i;

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0')
  {
    send_to_char("Which poll do you wish to change?\n\r", ch);
    return;
  }

  pIter = AllocIterator(poll_list);
  while ((poll = (POLL_DATA *) NextInList(pIter)) != NULL)
  {
    if (str_cmp(poll->name, arg1))
      continue;
    found = TRUE;
    break;
  }

  if (!found)
  {
    send_to_char("No such poll.\n\r", ch);
    return;
  }

  if (!str_cmp(arg2, "desc"))
  {
    free_string(poll->description);
    poll->description = str_dup(argument);
    send_to_char("Ok.\n\r", ch);
    return;
  }

  else if ((i = atoi(arg2)) < 1 || i > MAX_VOTE_OPTIONS)
  {
    send_to_char("Please pick a valid field.\n\r", ch);
    return;
  }

  free_string(poll->options[i - 1]);
  poll->options[i - 1] = str_dup(argument);
  send_to_char("Options set.\n\r", ch);
  save_polls();
}

void do_stoppoll(CHAR_DATA * ch, char *argument)
{
  POLL_DATA *poll;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Which poll do you wish to stop?\n\r", ch);
    return;
  }

  pIter = AllocIterator(poll_list);
  while ((poll = (POLL_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(poll->name, arg))
    {
      complete_poll(poll);
      return;
    }
  }

  send_to_char("No such poll.\n\r", ch);
}

void do_addchange(CHAR_DATA * ch, char *argument)
{
  CHANGE_DATA *change;
  char *strtime;
  char buf[50];
  int i;

  if (IS_NPC(ch))
    return;

  /* we need something to add to the list */
  if (argument[0] == '\0' || strlen(argument) < 5)
  {
    send_to_char("What did you change?\n\r", ch);
    return;
  }

  /* Set the current time */
  strtime = ctime(&current_time);
  for (i = 0; i < 6; i++)
  {
    buf[i] = strtime[i + 4];
  }
  buf[6] = '\0';

  if ((change = (CHANGE_DATA *) PopStack(change_free)) == NULL)
    change = malloc(sizeof(*change));

  smash_tilde(argument);

  /* set the strings for the change */
  change->imm = str_dup(ch->name);
  change->text = str_dup(argument);
  change->date = str_dup(buf);

  AttachToEndOfList(change, change_list);

  /* Removing the oldest change if the list has gone beyond the max */
  if (SizeOfList(change_list) > MAX_CHANGE)
    remove_change(1);

  send_to_char("Change added.\n\r", ch);
  save_changes();
}

void do_delchange(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int i;

  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);

  if ((i = atoi(arg)) < 1)
  {
    send_to_char("Which number change did you want to remove ?\n\r", ch);
    return;
  }

  if (!remove_change(i))
    send_to_char("No such change.\n\r", ch);
  else
    send_to_char("Change removed.\n\r", ch);

  save_changes();
}

void do_pathfind(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char *path;

  one_argument(argument, arg);
  if ((victim = get_char_world(ch, arg)) == NULL) return;

  if ((path = pathfind(ch, victim)) != NULL)
    sprintf(buf, "Path: %s\n\r", path);
  else
    sprintf(buf, "Path: Unknown.\n\r");

  send_to_char(buf, ch);
}
