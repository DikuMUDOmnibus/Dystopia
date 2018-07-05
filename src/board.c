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
#include "olc.h"

/*
 
 Note Board system, (c) 1995-96 Erwin S. Andreasen, erwin@pip.dknet.dk
 =====================================================================
 
 Basically, the notes are split up into several boards. The boards do not
 exist physically, they can be read anywhere and in any position.
 
 Each of the note boards has its own file. Each of the boards can have its own
 "rights": who can read/write.
 
 Each character has an extra field added, namele the timestamp of the last note
 read by him/her on a certain board.
 
 The note entering system is changed too, making it more interactive. When
 entering a note, a character is put AFK and into a special CON_ state.
 Everything typed goes into the note.
 
 For the immortals it is possible to purge notes based on age. An Archive
 options is available which moves the notes older than X days into a special
 board. The file of this board should then be moved into some other directory
 during e.g. the startup script and perhaps renamed depending on date.
 
 Note that write_level MUST be >= read_level or else there will be strange
 output in certain functions.
 
 Board DEFAULT_BOARD must be at least readable by *everyone*.
 
*/

#define L_SUP (MAX_LEVEL - 1)   /* if not already defined */

BOARD_DATA boards[MAX_BOARD] =
{
  { "General",  "General discussion",           0, 2, "all", DEF_INCLUDE, 14, NULL, FALSE},
  { "Ideas",    "Suggestion for improvement",   0, 2, "all", DEF_INCLUDE, 14, NULL, FALSE},
  { "Announce", "Announcements from Immortals", 0, 8, "all", DEF_INCLUDE, 35, NULL, FALSE},
  { "Bugs",     "Typos, bugs, errors",          2, 2, "imm", DEF_INCLUDE, 35, NULL, FALSE},
  { "Personal", "Personal messages",            0, 2, "all", DEF_EXCLUDE, 14, NULL, FALSE},
  { "Immortal", "Immortal Board",               8, 8, "imm", DEF_INCLUDE, 60, NULL, FALSE},
  { "Builder",  "Builder Board",                7, 7, "imm", DEF_INCLUDE, 21, NULL, FALSE}
};

const struct flag_type note_flags[] = 
{
  { "silenced",    NOTE_FLAG_SILENCED,  TRUE },
  { "accepted",    NOTE_FLAG_ACCEPTED,  TRUE },
  { "rejected",    NOTE_FLAG_REJECTED,  TRUE },
  { "fixed",       NOTE_FLAG_FIXED,     TRUE },

  /*
   * End of table.
   */
  { "", 0, FALSE }
};

/* The prompt that the character is given after finishing a note with ~ or END */
const char *szFinishPrompt = "(#9C#n)ontinue, (#9V#n)iew, (#9P#n)ost or (#9F#n)orget it?";

long last_note_stamp = 0;       /* To generate unique timestamps on notes */

#define BOARD_NOACCESS -1
#define BOARD_NOTFOUND -1

static bool next_board(CHAR_DATA * ch);

/* recycle a note */
void free_note(NOTE_DATA * note)
{
  if (note->sender)
    free_string(note->sender);
  if (note->to_list)
    free_string(note->to_list);
  if (note->subject)
    free_string(note->subject);
  if (note->date)               /* was note->datestamp for some reason */
    free_string(note->date);
  if (note->text)
    free_string(note->text);

  PushStack(note, note_free);
}

/* allocate memory for a new note or recycle */
NOTE_DATA *new_note()
{
  NOTE_DATA *note;

  if ((note = (NOTE_DATA *) PopStack(note_free)) == NULL)
  {
    note = malloc(sizeof(NOTE_DATA));
  }

  note->sender = NULL;
  note->expire = 0;
  note->to_list = NULL;
  note->subject = NULL;
  note->date = NULL;
  note->date_stamp = 0;
  note->text = NULL;
  note->flags = 0;

  return note;
}

/* append this note to the given file */
static void append_note(FILE * fp, NOTE_DATA * note)
{
  fprintf(fp, "Sender  %s~\n", note->sender);
  fprintf(fp, "Date    %s~\n", note->date);
  fprintf(fp, "Stamp   %ld\n", note->date_stamp);
  fprintf(fp, "Expire  %ld\n", note->expire);
  fprintf(fp, "Flags   %d\n",  note->flags);
  fprintf(fp, "To      %s~\n", note->to_list);
  fprintf(fp, "Subject %s~\n", note->subject);
  fprintf(fp, "Text\n%s~\n\n", note->text);
}

/* Save a note in a given board */
void finish_note(BOARD_DATA * board, NOTE_DATA * note)
{
  FILE *fp;
  char filename[200];

  /* The following is done in order to generate unique date_stamps */
  if (last_note_stamp >= current_time)
    note->date_stamp = ++last_note_stamp;
  else
  {
    note->date_stamp = current_time;
    last_note_stamp = current_time;
  }

  AttachToEndOfList(note, board->notes);

  sprintf(filename, "%s%s", NOTE_DIR, board->short_name);

  if ((fp = fopen(filename, "a")) == NULL)
  {
    bug("Could not open one of the note files in append mode", 0);
    board->changed = TRUE;      /* set it to TRUE hope it will be OK later? */
    return;
  }
  append_note(fp, note);
  fclose(fp);
}

/* Find the number of a board */
int board_number(const BOARD_DATA * board)
{
  int i;

  for (i = 0; i < MAX_BOARD; i++)
    if (board == &boards[i])
      return i;

  return -1;
}

/* Find a board number based on  a string */
int board_lookup(const char *name)
{
  int i;

  for (i = 0; i < MAX_BOARD; i++)
    if (!str_cmp(boards[i].short_name, name))
      return i;

  return -1;
}

/* Find the nth note on a board. Return NULL if ch has no access to that note */
static NOTE_DATA *find_note(CHAR_DATA * ch, BOARD_DATA * board, int num)
{
  int count = 0;
  NOTE_DATA *p;
  ACCOUNT_DATA *account;
  ITERATOR *pIter;

  if (ch->desc == NULL || (account = ch->desc->account) == NULL)
    return NULL;

  pIter = AllocIterator(board->notes);
  while ((p = (NOTE_DATA *) NextInList(pIter)) != NULL)
  {
    if (++count == num)
      break;
  }

  if ((count == num) && is_note_to(account, p))
    return p;

  return NULL;
}

/* save a single board */
static void save_board(BOARD_DATA * board)
{
  FILE *fp;
  char filename[200];
  char buf[200];
  NOTE_DATA *note;
  ITERATOR *pIter;

  sprintf(filename, "%s%s", NOTE_DIR, board->short_name);

  fp = fopen(filename, "w");
  if (!fp)
  {
    sprintf(buf, "Error writing to: %s", filename);
    bug(buf, 0);
  }
  else
  {
    pIter = AllocIterator(board->notes);
    while ((note = (NOTE_DATA *) NextInList(pIter)) != NULL)
      append_note(fp, note);

    fclose(fp);
  }
}

/* Show one not to a character */
static void show_note_to_char(CHAR_DATA *ch, NOTE_DATA *note, int num)
{
  char buf[4 * MAX_STRING_LENGTH];

  if (ch->desc == NULL)
    return;

  sprintf(buf,
          "[#9%4d#n] #y%s#n: #G%s#n\n\r"
          "#yDate#n:  %s\n\r"
          "#yTo#n:    %s\n\r"
          "#yFlags#n: %s\n\r"
          "#G===========================================================================#n\n\r%s\n\r",
    num, note->sender, note->subject, note->date, note->to_list,
    (IS_SET(note->flags, NOTE_FLAG_SILENCED) && ch->level < MAX_LEVEL) ? "none" : flag_string(note_flags, note->flags),
    note->text);

  send_to_char(buf, ch);
}

/* Save changed boards */
void save_notes()
{
  int i;

  for (i = 0; i < MAX_BOARD; i++)
  {
    if (boards[i].changed)      /* only save changed boards */
      save_board(&boards[i]);
  }
}

/* Load a single board */
static void load_board(BOARD_DATA * board)
{
  FILE *fp, *fp_archive;
  char filename[200];

  board->notes = AllocList();

  sprintf(filename, "%s%s", NOTE_DIR, board->short_name);
  if ((fp = fopen(filename, "r")) == NULL)
    return;

  for ( ; ; )
  {
    NOTE_DATA *pnote;
    char letter;

    do
    {
      letter = getc(fp);
      if (feof(fp))
      {
        fclose(fp);
        return;
      }
    }
    while (isspace(letter));
    ungetc(letter, fp);

    pnote = new_note();

    if (str_cmp(fread_word(fp), "sender"))
      break;
    pnote->sender = fread_string(fp);

    if (str_cmp(fread_word(fp), "date"))
      break;
    pnote->date = fread_string(fp);

    if (str_cmp(fread_word(fp), "stamp"))
      break;
    pnote->date_stamp = fread_number(fp);

    if (str_cmp(fread_word(fp), "expire"))
      break;
    pnote->expire = fread_number(fp);

    if (str_cmp(fread_word(fp), "flags"))
      break;
    pnote->flags = fread_number(fp);

    if (str_cmp(fread_word(fp), "to"))
      break;
    pnote->to_list = fread_string(fp);

    if (str_cmp(fread_word(fp), "subject"))
      break;
    pnote->subject = fread_string(fp);

    if (str_cmp(fread_word(fp), "text"))
      break;
    pnote->text = fread_string(fp);

    /* Should this note be archived right now ? */
    if (pnote->expire < current_time)
    {
      char archive_name[200];

      sprintf(archive_name, "%s%s.old", NOTE_DIR, board->short_name);
      fp_archive = fopen(archive_name, "a");
      if (!fp_archive)
        bug("Could not open archive boards for writing", 0);
      else
      {
        append_note(fp_archive, pnote);
        fclose(fp_archive);     /* it might be more efficient to close this later */
      }

      free_note(pnote);
      board->changed = TRUE;
      continue;
    }

    AttachToEndOfList(pnote, board->notes);
  }

  bug("Load_board: bad key word.", 0);
}

/* Initialize structures. Load all boards. */
void load_boards()
{
  int i;

  for (i = 0; i < MAX_BOARD; i++)
    load_board(&boards[i]);
}

/* Returns TRUE if the specified note is address to ch */
bool is_note_to(ACCOUNT_DATA *account, NOTE_DATA *note)
{
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;
  char temp[MAX_INPUT_LENGTH];
  char *ptr;

  /* is this a valid question? */
  if (account == NULL || note == NULL)
    return FALSE;

  /* coders can read everything */
  if (account->level >= CODER_ACCOUNT)
    return TRUE;

  /* compare account->players to note->sender */
  ptr = account->players;
  ptr = one_argument(ptr, temp);
  while(temp[0] != '\0')
  {
    if (!str_cmp(temp, note->sender))
      return TRUE;

    ptr = one_argument(ptr, temp);
    ptr = one_argument(ptr, temp);
    ptr = one_argument(ptr, temp);
    ptr = one_argument(ptr, temp);
  }

  /* do not display silenced notes */
  if (IS_SET(note->flags, NOTE_FLAG_SILENCED))
    return FALSE;

  ptr = note->to_list;

  /* note is to everyone */
  if (is_full_name("all", ptr))
    return TRUE;

  /* note is to immortals and account is admin account */
  if (is_full_name("imm", ptr) && account->level >= BUILDER_ACCOUNT)
    return TRUE;

  /* compare note->to_list to account->players */
  ptr = one_argument(ptr, temp);
  while(temp[0] != '\0')
  {
    if (is_full_name(temp, account->players))
      return TRUE;

    ptr = one_argument(ptr, temp);
  }

  /* now let's do the kingdom trick */
  pIter = AllocIterator(kingdom_list);
  while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
  {
    /* note is to a kingdom, does this account have a player in that kingdom */
    if (is_full_name(kingdom->shortname, note->to_list))
    {
      MEMBER_DATA *member;
      ITERATOR *pIter2;

      pIter2 = AllocIterator(kingdom->members);
      while ((member = (MEMBER_DATA *) NextInList(pIter2)) != NULL)
      {
        if (is_full_name(member->name, account->players))
          return TRUE;
      }
    }
  }

  return FALSE;
}

/* Return the number of unread notes 'ch' has in 'board' */
/* Returns BOARD_NOACCESS if ch has no access to board */
int unread_notes(CHAR_DATA * ch, BOARD_DATA * board)
{
  ACCOUNT_DATA *account;
  NOTE_DATA *note;
  ITERATOR *pIter;
  time_t last_read;
  int count = 0;

  if (ch->desc == NULL || (account = ch->desc->account) == NULL)
    return BOARD_NOACCESS;

  if (board->read_level > get_trust(ch))
    return BOARD_NOACCESS;

  last_read = account->last_note[board_number(board)];

  pIter = AllocIterator(board->notes);
  while ((note = (NOTE_DATA *) NextInList(pIter)) != NULL)
  {
    if (is_note_to(account, note) && ((long) last_read < (long) note->date_stamp))
      count++;
  }

  return count;
}

int total_notes(CHAR_DATA * ch, BOARD_DATA * board)
{
  ACCOUNT_DATA *account;
  NOTE_DATA *note;
  ITERATOR *pIter;
  int count = 0;

  if (ch->desc == NULL || (account = ch->desc->account) == NULL)
    return 0;

  pIter = AllocIterator(board->notes);
  while ((note = (NOTE_DATA *) NextInList(pIter)) != NULL)
  {
    if (is_note_to(account, note))
      count++;
  }

  return count;
}

/*
 * COMMANDS
 */

/* Start writing a note */
static void do_nwrite(CHAR_DATA * ch, char *argument)
{
  char *strtime;
  char buf[200];

  if (IS_NPC(ch) || ch->desc == NULL)
    return;

  if (ch->desc->account->max_might < RANK_CADET && ch->level < 7)
  {
    send_to_char("You need at least one character with cadet or better rank.\n\r", ch);
    return;
  }

  if (has_timer(ch))
    return;

  if (ch->desc->account->board == NULL)
  {
    send_to_char("You're not on a board.\n\r", ch);
    return;
  }

  if (get_trust(ch) < ch->desc->account->board->write_level)
  {
    send_to_char("You cannot post notes on this board.\n\r", ch);
    return;
  }

  if (ch->position != POS_STANDING)
  {
    send_to_char("You can only write notes while standing.\n\r", ch);
    return;
  }

  /* continue previous note, if any text was written */
  if (ch->pcdata->in_progress && (!ch->pcdata->in_progress->text))
  {
    send_to_char("Note in progress cancelled because you did not manage to write any text \n\r" "before losing link.\n\r\n\r", ch);
    free_note(ch->pcdata->in_progress);
    ch->pcdata->in_progress = NULL;
  }

  if (!ch->pcdata->in_progress)
  {
    ch->pcdata->in_progress = new_note();
    ch->pcdata->in_progress->sender = str_dup(ch->name);

    /* convert to ascii. ctime returns a string which last character is \n, so remove that */
    strtime = ctime(&current_time);
    strtime[strlen(strtime) - 1] = '\0';

    ch->pcdata->in_progress->date = str_dup(strtime);
  }

  SET_BIT(ch->extra, EXTRA_AFK);
  strip_event_mobile(ch, EVENT_MOBILE_CASTING);

  if (ch->master)
    stop_follower(ch, FALSE);

  act("#G$n starts writing a note.#n", ch, NULL, NULL, TO_ROOM);

  /* Begin writing the note ! */
  sprintf(buf, "You are now %s a new note on the #9%s#n board.\n\r"
          "If you are using tintin, type ##verbose to turn off alias expansion!\n\r\n\r", 
          ch->pcdata->in_progress->text ? "continuing" : "posting", 
          ch->desc->account->board->short_name);
  send_to_char(buf, ch);

  sprintf(buf, "#yFrom#n:    %s\n\r\n\r", ch->name);
  send_to_char(buf, ch);

  if (!ch->pcdata->in_progress->text) /* Are we continuing an old note or not? */
  {
    switch (ch->desc->account->board->force_type)
    {
      case DEF_NORMAL:
        sprintf(buf, "If you press Return, default recipient \"#9%s#n\" will be chosen.\n\r", ch->desc->account->board->names);
        break;
      case DEF_INCLUDE:
        sprintf(buf, "The recipient list MUST include \"#9%s#n\". If not, it will be added automatically.\n\r", ch->desc->account->board->names);
        break;

      case DEF_EXCLUDE:
        sprintf(buf, "The recipient of this note must NOT include: \"#9%s#n\".", ch->desc->account->board->names);
        break;
    }

    send_to_char(buf, ch);
    send_to_char("\n\r#yTo#n:      ", ch);

    ch->desc->connected = CON_NOTE_TO;
    /* nanny takes over from here */

  }
  else                          /* we are continuing, print out all the fields and the note so far */
  {
    sprintf(buf, "#yTo#n:      %s\n\r"
                 "#yExpires#n: %s\n\r"
                 "#ySubject#n: %s\n\r",
      ch->pcdata->in_progress->to_list, ctime(&ch->pcdata->in_progress->expire), ch->pcdata->in_progress->subject);
    send_to_char(buf, ch);
    send_to_char("#GYour note so far:#n\n\r", ch);
    if (ch->pcdata->in_progress != NULL)
      send_to_char(ch->pcdata->in_progress->text, ch);

    send_to_char("\n\rEnter text. Type #9~#n or #9END#n on an empty line to end note.\n\r"
                 "=======================================================\n\r", ch);
    ch->desc->connected = CON_NOTE_TEXT;

  }

}

/* Read next note in current group. If no more notes, go to next board */
static void do_nread(CHAR_DATA * ch, char *argument)
{
  ACCOUNT_DATA *account;
  NOTE_DATA *p;
  ITERATOR *pIter;
  int count = 0, number;
  time_t *last_note;

  if (ch->desc == NULL || (account = ch->desc->account) == NULL)
    return;

  last_note = &ch->desc->account->last_note[board_number(ch->desc->account->board)];

  if (is_number(argument))
  {
    number = atoi(argument);

    pIter = AllocIterator(account->board->notes);
    while ((p = (NOTE_DATA *) NextInList(pIter)) != NULL)
    {
      if (++count == number)
        break;
    }

    if (!p || !is_note_to(account, p))
      send_to_char("No such note.\n\r", ch);
    else
    {
      show_note_to_char(ch, p, count);
      *last_note = UMAX(*last_note, p->date_stamp);
    }
  }
  else                          /* just next one */
  {
    char buf[200];

    count = 1;
    if (account->board == NULL)
    {
      send_to_char("You are not on a board.\n\r", ch);
      return;
    }
    if (SizeOfList(account->board->notes) == 0)
    {
      send_to_char("There are no notes.\n\r", ch);
      return;
    }

    pIter = AllocIterator(account->board->notes);
    while ((p = (NOTE_DATA *) NextInList(pIter)) != NULL)
    {
      if ((p->date_stamp > *last_note) && is_note_to(account, p))
      {
        show_note_to_char(ch, p, count);

        /* Advance if new note is newer than the currently newest for that char */
        *last_note = UMAX(*last_note, p->date_stamp);
        return;
      }
    }

    send_to_char("No new notes in this board.\n\r", ch);

    if (next_board(ch))
      sprintf(buf, "Changed to next board, %s.\n\r", account->board->short_name);
    else
      sprintf(buf, "There are no more boards.\n\r");

    send_to_char(buf, ch);
  }
}

/* Remove a note */
static void do_nremove(CHAR_DATA * ch, char *argument)
{
  NOTE_DATA *p;

  if (ch->desc == NULL)
    return;

  if (!is_number(argument))
  {
    send_to_char("Remove which note?\n\r", ch);
    return;
  }

  p = find_note(ch, ch->desc->account->board, atoi(argument));
  if (!p)
  {
    send_to_char("No such note.\n\r", ch);
    return;
  }

  if (str_cmp(ch->name, p->sender) && ch->trust < MAX_LEVEL)
  {
    send_to_char("You are not authorized to remove this note.\n\r", ch);
    return;
  }

  DetachFromList(p, ch->desc->account->board->notes);
  free_note(p);
  send_to_char("Note removed!\n\r", ch);

  save_board(ch->desc->account->board);  /* save the board */
}

static void do_nflag(CHAR_DATA *ch, char *argument)
{
  NOTE_DATA *p;
  char note[MAX_INPUT_LENGTH];
  int i;

  if (ch->desc == NULL)
    return;

  argument = one_argument(argument, note);

  if (!is_number(note))
  {
    send_to_char("Flag which note?\n\r", ch);
    return;
  }
   
  if ((p = find_note(ch, ch->desc->account->board, atoi(note))) == NULL)
  {
    send_to_char("No such note.\n\r", ch);
    return;
  }

  if (ch->trust < MAX_LEVEL)
  {
    send_to_char("You are not authorized to flag this note.\n\r", ch);
    return;
  }

  for (i = 0; note_flags[i].name[0] != '\0'; i++)
  {
    if (!str_cmp(argument, note_flags[i].name))
    {
      TOGGLE_BIT(p->flags, note_flags[i].bit);
      send_to_char("Note flagged.\n\r", ch);
      save_board(ch->desc->account->board);
      return;
    }
  }

  send_to_char("Unknown flag.\n\r", ch);
}

/* List all notes or if argument given, list N of the last notes */
/* Shows REAL note numbers! */
static void do_nlist(CHAR_DATA * ch, char *argument)
{
  int count = 0, show = 0, num = 0, has_shown = 0;
  time_t last_note;
  NOTE_DATA *p;
  ACCOUNT_DATA *account;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];

  if (ch->desc == NULL || (account = ch->desc->account) == NULL)
    return;

  if (is_number(argument))      /* first, count the number of notes */
  {
    show = atoi(argument);

    pIter = AllocIterator(account->board->notes);
    while ((p = (NOTE_DATA *) NextInList(pIter)) != NULL)
    {
      if (is_note_to(account, p))
        count++;
    }
  }

  send_to_char("Notes on this board:\n\r#GNum#9> #GAuthor        Subject#n\n\r", ch);

  last_note = account->last_note[board_number(account->board)];

  pIter = AllocIterator(account->board->notes);
  while ((p = (NOTE_DATA *) NextInList(pIter)) != NULL)
  {
    num++;
    if (is_note_to(account, p))
    {
      has_shown++;              /* note that we want to see X VISIBLE note, not just last X */
      if (!show || ((count - show) < has_shown))
      {
        sprintf(buf, "#G%3d#9>#G%c#C%-13s #y%s#n\n\r",
          num, last_note < p->date_stamp ? '*' : ' ', p->sender, p->subject);
        send_to_char(buf, ch);
      }
    }

  }
}

/* catch up with some notes */
static void do_ncatchup(CHAR_DATA * ch, char *argument)
{
  NOTE_DATA *p = NULL, *pNote;
  ITERATOR *pIter;

  if (ch->desc == NULL)
    return;

  /* Find last note */
  pIter = AllocIterator(ch->desc->account->board->notes);
  while ((pNote = (NOTE_DATA *) NextInList(pIter)) != NULL)
  {
    p = pNote;
  }

  if (!p)
    send_to_char("Alas, there are no notes in that board.\n\r", ch);
  else
  {
    ch->desc->account->last_note[board_number(ch->desc->account->board)] = p->date_stamp;
    send_to_char("All mesages skipped.\n\r", ch);
  }
}

/* Dispatch function for backwards compatibility */
void do_note(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch) || ch->desc == NULL)
    return;

  argument = one_argument(argument, arg);

  if ((!arg[0]) || (!str_cmp(arg, "read"))) /* 'note' or 'note read X' */
    do_nread(ch, argument);

  else if (!str_cmp(arg, "list"))
    do_nlist(ch, argument);

  else if (!str_cmp(arg, "write"))
    do_nwrite(ch, argument);

  else if (!str_cmp(arg, "remove"))
    do_nremove(ch, argument);

  else if (!str_cmp(arg, "purge"))
    send_to_char("Obsolete.\n\r", ch);

  else if (!str_cmp(arg, "archive"))
    send_to_char("Obsolete.\n\r", ch);

  else if (!str_cmp(arg, "catchup"))
    do_ncatchup(ch, argument);

  else if (!str_cmp(arg, "flag"))
    do_nflag(ch, argument);

  else
    do_help(ch, "note");
}

/* Show all accessible boards with their numbers of unread messages OR
   change board. New board name can be given as a number or as a name (e.g.
    board personal or board 4 */
void do_board(CHAR_DATA * ch, char *argument)
{
  int i, count, number;
  char buf[200];

  if (IS_NPC(ch) || ch->desc == NULL)
    return;

  if (!argument[0])             /* show boards */
  {
    int unread;

    count = 1;
    send_to_char("\n\r", ch);
    send_to_char("#GNum Name        New/All  Description#n\n\r", ch);
    send_to_char("#L=== ========== ========= ===========#n\n\r", ch);
    for (i = 0; i < MAX_BOARD; i++)
    {
      unread = unread_notes(ch, &boards[i]);  /* how many unread notes? */
      if (unread != BOARD_NOACCESS)
      {                         /* watch out for the non-portable &%c !!! */
        sprintf(buf, "#G%2d#R> #G%-10s#R [#y%3d#R/#0%3d#R] #G%s#n\n\r", count, boards[i].short_name, unread, total_notes(ch, &boards[i]), boards[i].long_name);
        send_to_char(buf, ch);
        count++;
      }                         /* if has access */
    }                           /* for each board */
    if (ch->desc->account->board != NULL)
    {
      sprintf(buf, "\n\rYou current board is #9%s#n.\n\r", ch->desc->account->board->short_name);
      send_to_char(buf, ch);

      /* Inform of rights */
      if (ch->desc->account->board->read_level > get_trust(ch))
        send_to_char("You cannot read nor write notes on this board.\n\r", ch);
      else if (ch->desc->account->board->write_level > get_trust(ch))
        send_to_char("You can only read notes from this board.\n\r", ch);
      else
        send_to_char("You can both read and write on this board.\n\r", ch);
    }
    send_to_char("\n\r", ch);
    return;
  }                             /* if empty argument */
  /* Change board based on its number */
  if (is_number(argument))
  {
    count = 0;
    number = atoi(argument);
    for (i = 0; i < MAX_BOARD; i++)
      if (unread_notes(ch, &boards[i]) != BOARD_NOACCESS)
        if (++count == number)
          break;

    if (count == number)        /* found the board.. change to it */
    {
      ch->desc->account->board = &boards[i];
      sprintf(buf, "Current board changed to #9%s#n. %s.\n\r", boards[i].short_name,
              (get_trust(ch) < boards[i].write_level) ? "You can only read here" : "You can both read and write here");
      send_to_char(buf, ch);
    }
    else                        /* so such board */
      send_to_char("No such board.\n\r", ch);

    return;
  }

  /* Non-number given, find board with that name */

  for (i = 0; i < MAX_BOARD; i++)
    if (!str_cmp(boards[i].short_name, argument))
      break;

  if (i == MAX_BOARD)
  {
    send_to_char("No such board.\n\r", ch);
    return;
  }

  /* Does ch have access to this board? */
  if (unread_notes(ch, &boards[i]) == BOARD_NOACCESS)
  {
    send_to_char("No such board.\n\r", ch);
    return;
  }

  ch->desc->account->board = &boards[i];
  sprintf(buf, "Current board changed to #9%s#n. %s.\n\r", boards[i].short_name,
          (get_trust(ch) < boards[i].write_level) ? "You can only read here" : "You can both read and write here");
  send_to_char(buf, ch);
}

bool double_post(char *player, char *boardname, char *subject)
{
  BOARD_DATA *board;
  NOTE_DATA *note = NULL, *pnote;
  ITERATOR *pIter;
  int board_index = board_lookup(boardname);

  /* a little safety check */
  if (board_index == BOARD_NOTFOUND)
    return TRUE;

  board = &boards[board_index];

  /* no old notes, no problems. */
  if (SizeOfList(board->notes) == 0)
    return FALSE;

  /* scan till we meet the last note */
  pIter = AllocIterator(board->notes);
  while ((pnote = (NOTE_DATA *) NextInList(pIter)) != NULL)
  {
    note = pnote;
  }

  /* not same poster, no problems */
  if (str_cmp(note->sender, player))
    return FALSE;

  /* if this player posted a note on this board less than 30 seconds ago,
   * with the same subject, then it's very likely that this is the same
   * note, and we shouldn't allow this post.
   */
  if (note->date_stamp >= current_time - 30L && !str_cmp(subject, note->subject))
    return TRUE;

  return FALSE;
}

void make_note(const char *board_name, const char *sender, const char *to, const char *subject, const int expire_days, const char *text, int flags)
{
  int board_index = board_lookup(board_name);
  BOARD_DATA *board;
  NOTE_DATA *note;
  char *strtime;

  if (board_index == BOARD_NOTFOUND)
  {
    bug("make_note: board not found", 0);
    return;
  }

  if (strlen2(text) > MAX_NOTE_TEXT)
  {
    bug("make_note: text too long (%d bytes)", strlen2(text));
    return;
  }

  board = &boards[board_index];

  note = new_note();            /* allocate new note */

  note->sender = str_dup(sender);
  note->to_list = str_dup(to);
  note->subject = str_dup(subject);
  note->expire = current_time + expire_days * 60 * 60 * 24;
  note->text = str_dup(text);
  note->flags = flags;

  /* convert to ascii. ctime returns a string which last character is \n, so remove that */
  strtime = ctime(&current_time);
  strtime[strlen(strtime) - 1] = '\0';

  note->date = str_dup(strtime);

  finish_note(board, note);
}

/* tries to change to the next accessible board */
static bool next_board(CHAR_DATA * ch)
{
  int i;

  if (ch->desc == NULL)
    return FALSE;

  i = board_number(ch->desc->account->board) + 1;

  while ((i < MAX_BOARD) && (unread_notes(ch, &boards[i]) == BOARD_NOACCESS))
    i++;

  if (i == MAX_BOARD)
    return FALSE;
  else
  {
    ch->desc->account->board = &boards[i];
    return TRUE;
  }
}

void handle_con_note_to(DESCRIPTOR_DATA * d, char *argument)
{
  char buf[MAX_INPUT_LENGTH * 6];
  CHAR_DATA *ch = d->character;

  if (!ch->pcdata->in_progress)
  {
    d->connected = CON_PLAYING;
    bug("nanny: In CON_NOTE_TO, but no note in progress", 0);
    return;
  }

  strcpy(buf, argument);
  smash_tilde(buf);             /* change ~ to - as we save this field as a string later */

  switch (ch->desc->account->board->force_type)
  {
    case DEF_NORMAL:           /* default field */
      if (!buf[0])              /* empty string? */
      {
        ch->pcdata->in_progress->to_list = str_dup(ch->desc->account->board->names);
        sprintf(buf, "Assumed default recipient: #9%s#n\n\r", ch->desc->account->board->names);
        write_to_buffer(d, buf, 0);
      }
      else
        ch->pcdata->in_progress->to_list = str_dup(buf);

      break;

    case DEF_INCLUDE:          /* forced default */
      if (!is_full_name(ch->desc->account->board->names, buf))
      {
        strcat(buf, " ");
        strcat(buf, ch->desc->account->board->names);
        ch->pcdata->in_progress->to_list = str_dup(buf);

        sprintf(buf, "\n\rYou did not specify %s as recipient, so it was automatically added.\n\r"
                "#yNew To#n :  %s\n\r", ch->desc->account->board->names, ch->pcdata->in_progress->to_list);
        write_to_buffer(d, buf, 0);
      }
      else
        ch->pcdata->in_progress->to_list = str_dup(buf);
      break;

    case DEF_EXCLUDE:          /* forced exclude */
      if (is_full_name(ch->desc->account->board->names, buf))
      {
        sprintf(buf, "You are not allowed to send notes to %s on this board. Try again.\n\r#yTo#n:      ", ch->desc->account->board->names);
        write_to_buffer(d, buf, 0);
        return;                 /* return from nanny, not changing to the next state! */
      }
      else
        ch->pcdata->in_progress->to_list = str_dup(buf);
      break;

  }

  write_to_buffer(d, "\n\r#ySubject#n: ", 0);
  d->connected = CON_NOTE_SUBJECT;
}

void handle_con_note_subject(DESCRIPTOR_DATA * d, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *ch = d->character;

  if (!ch->pcdata->in_progress)
  {
    d->connected = CON_PLAYING;
    bug("nanny: In CON_NOTE_SUBJECT, but no note in progress", 0);
    return;
  }

  strcpy(buf, argument);
  smash_tilde(buf);             /* change ~ to - as we save this field as a string later */

  /* Do not allow empty subjects */

  if (!buf[0])
  {
    write_to_buffer(d, "Please find a meaningful subject!\n\r", 0);
    write_to_buffer(d, "#ySubject#n: ", 0);
  }
  else if (strlen(buf) > 60)
  {
    write_to_buffer(d, "No, no. This is just the Subject. You're not writing the note yet. Twit.\n\r", 0);
  }
  else
    /* advance to next stage */
  {
    ch->pcdata->in_progress->subject = str_dup(buf);
    if (IS_IMMORTAL(ch))        /* immortals get to choose number of expire days */
    {
      sprintf(buf, "\n\rHow many days do you want this note to expire in?\n\r"
              "Press Enter for default value for this board, #9%d#n days.\n\r#yExpire#n:  ", ch->desc->account->board->purge_days);
      write_to_buffer(d, buf, 0);
      d->connected = CON_NOTE_EXPIRE;
    }
    else
    {
      ch->pcdata->in_progress->expire = current_time + ch->desc->account->board->purge_days * 24L * 
3600L;
      sprintf(buf, "This note will expire %s\r", ctime(&ch->pcdata->in_progress->expire));
      write_to_buffer(d, buf, 0);
      write_to_buffer(d, "\n\rEnter text. Type #9~#n or #9END#n on an empty line to end note.\n\r"
                         "=======================================================\n\r", 0);
      d->connected = CON_NOTE_TEXT;
    }
  }
}

void handle_con_note_expire(DESCRIPTOR_DATA * d, char *argument)
{
  CHAR_DATA *ch = d->character;
  char buf[MAX_STRING_LENGTH];
  time_t expire;
  int days;

  if (!ch->pcdata->in_progress)
  {
    d->connected = CON_PLAYING;
    bug("nanny: In CON_NOTE_EXPIRE, but no note in progress", 0);
    return;
  }

  /* Numeric argument. no tilde smashing */
  strcpy(buf, argument);
  if (!buf[0])                  /* assume default expire */
    days = ch->desc->account->board->purge_days;
  else /* use this expire */ if (!is_number(buf))
  {
    write_to_buffer(d, "Write the number of days!\n\r", 0);
    write_to_buffer(d, "#yExpire#n:  ", 0);
    return;
  }
  else
  {
    days = atoi(buf);
    if (days <= 0)
    {
      write_to_buffer(d, "This is a positive MUD. Use positive numbers only! :)\n\r", 0);
      write_to_buffer(d, "#yExpire#n:  ", 0);
      return;
    }
  }

  expire = current_time + (days * 24L * 3600L); /* 24 hours, 3600 seconds */

  ch->pcdata->in_progress->expire = expire;

  /* note that ctime returns XXX\n so we only need to add an \r */

  write_to_buffer(d, "\n\rEnter text. Type #9~#n or #9END#n on an empty line to end note.\n\r"
                     "=======================================================\n\r", 0);

  d->connected = CON_NOTE_TEXT;
}

void handle_con_note_text(DESCRIPTOR_DATA * d, char *argument)
{
  CHAR_DATA *ch = d->character;
  char buf[MAX_STRING_LENGTH];
  char letter[4 * MAX_STRING_LENGTH];

  if (!ch->pcdata->in_progress)
  {
    d->connected = CON_PLAYING;
    bug("nanny: In CON_NOTE_TEXT, but no note in progress", 0);
    return;
  }

  /* First, check for EndOfNote marker */

  strcpy(buf, argument);
  if ((!str_cmp(buf, "~")) || (!str_cmp(buf, "END")))
  {
    write_to_buffer(d, "\n\r\n\r", 0);
    write_to_buffer(d, szFinishPrompt, 0);
    write_to_buffer(d, "\n\r", 0);
    d->connected = CON_NOTE_FINISH;
    return;
  }
  else if (!str_cmp(buf, "~delete"))
  {
    delete_last_line_in_note(ch);
    return;
  }

  smash_tilde(buf);             /* smash it now */

  /* Check for too long lines. Do not allow lines longer than 80 chars */

  if (strlen(buf) > MAX_LINE_LENGTH)
  {
    write_to_buffer(d, "Too long line rejected. Do NOT go over 80 characters!\n\r", 0);
    return;
  }

  /* Not end of note. Copy current text into temp buffer, add new line, and copy back */

  /* How would the system react to strcpy( , NULL) ? */
  if (ch->pcdata->in_progress->text != NULL)
  {
    strcpy(letter, ch->pcdata->in_progress->text);
    free_string(ch->pcdata->in_progress->text);
    ch->pcdata->in_progress->text = NULL; /* be sure we don't free it twice */
  }
  else
    strcpy(letter, "");

  /* Check for overflow */

  if ((strlen2(letter) + strlen2(buf)) > MAX_NOTE_TEXT)
  {                             /* Note too long, take appropriate steps */
    write_to_buffer(d, "Note too long!\n\r", 0);
    free_note(ch->pcdata->in_progress);
    ch->pcdata->in_progress = NULL; /* important */
    d->connected = CON_PLAYING;
    return;
  }

  /* Add new line to the buffer */

  strcat(letter, buf);
  strcat(letter, "\r\n");       /* new line. \r first to make note files better readable */

  /* allocate dynamically */
  ch->pcdata->in_progress->text = str_dup(letter);
}

void handle_con_note_finish(DESCRIPTOR_DATA * d, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *ch = d->character;

  if (!ch->pcdata->in_progress)
  {
    d->connected = CON_PLAYING;
    bug("nanny: In CON_NOTE_FINISH, but no note in progress", 0);
    return;
  }
  switch (tolower(argument[0]))
  {
    case 'c':                  /* keep writing */
      write_to_buffer(d, "Continuing note...\n\r", 0);
      d->connected = CON_NOTE_TEXT;
      break;
    case 'v':                  /* view note so far */
      if (ch->pcdata->in_progress->text)
      {
        write_to_buffer(d, "#GText of your note so far:#n\n\r", 0);
        write_to_buffer(d, ch->pcdata->in_progress->text, 0);
      }
      else
        write_to_buffer(d, "You haven't written a thing!\n\r\n\r", 0);
      write_to_buffer(d, szFinishPrompt, 0);
      write_to_buffer(d, "\n\r", 0);
      break;
    case 'p':                  /* post note */
      if (board_number(ch->desc->account->board) < 4 && is_full_name("all", ch->pcdata->in_progress->to_list) && !is_silenced(ch))
      {
        sprintf(buf, "A new note has been posted by %s on board %d", ch->name, board_number(ch->desc->account->board) + 1);
        do_info(ch, buf);
      }
      else if (board_number(ch->desc->account->board) == 4)
      {
        CHAR_DATA *gch;
        DESCRIPTOR_DATA *dcon;
        ITERATOR *pIter;

        pIter = AllocIterator(descriptor_list);
        while ((dcon = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
        {
          if (dcon->connected != CON_PLAYING || (gch = dcon->character) == NULL)
            continue;

          if (is_full_name(gch->name, ch->pcdata->in_progress->to_list))
            send_to_char("#GA personal note has arrived on the personal board!#n\n\r", gch);
        }
      }

      if (is_silenced(ch))
        SET_BIT(ch->pcdata->in_progress->flags, NOTE_FLAG_SILENCED);

      finish_note(ch->desc->account->board, ch->pcdata->in_progress);
      write_to_buffer(d, "Note posted.\n\r", 0);
      d->connected = CON_PLAYING;
      ch->pcdata->in_progress = NULL;
      act("#G$n finishes $s note.#n", ch, NULL, NULL, TO_ROOM);
      break;
    case 'f':
      write_to_buffer(d, "Note cancelled!\n\r", 0);
      free_note(ch->pcdata->in_progress);
      ch->pcdata->in_progress = NULL;
      d->connected = CON_PLAYING;
      break;
    default:                   /* invalid response */
      write_to_buffer(d, "Huh? Valid answers are:\n\r\n\r", 0);
      write_to_buffer(d, szFinishPrompt, 0);
      write_to_buffer(d, "\n\r", 0);
  }
  if (d->connected == CON_PLAYING)
    REMOVE_BIT(ch->extra, EXTRA_AFK);
}
