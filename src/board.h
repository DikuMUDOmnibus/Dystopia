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

#define NOTE_DIR  	        "../notes/"
#define DEF_NORMAL                 0
#define DEF_INCLUDE                1
#define DEF_EXCLUDE                2
#define MAX_BOARD                  7
#define DEFAULT_BOARD              0
#define MAX_LINE_LENGTH           80
#define MAX_NOTE_TEXT            MAX_STRING_LENGTH
#define BOARD_NOTFOUND            -1

/* bits for notes */
#define NOTE_FLAG_SILENCED         1
#define NOTE_FLAG_ACCEPTED         2
#define NOTE_FLAG_REJECTED         4
#define NOTE_FLAG_FIXED            8

/* Data about a board */
struct board_data
{
  char *short_name;             /* Max 8 chars */
  char *long_name;              /* Explanatory text, should be no more than 40 ? chars */

  int read_level;               /* minimum level to see board */
  int write_level;              /* minimum level to post notes */

  char *names;                  /* Default recipient */
  int force_type;               /* Default action (DEF_XXX) */

  int purge_days;               /* Default expiration */

  /* Non-constant data */

  LIST      *notes;             /* pointer to board's first note */
  bool changed;                 /* currently unused */

};

typedef struct board_data BOARD_DATA;

/* External variables */

extern BOARD_DATA boards[MAX_BOARD];  /* Declare */

/* Prototypes */

void finish_note(BOARD_DATA * board, NOTE_DATA * note); /* attach a note to a board */
void free_note(NOTE_DATA * note); /* deallocate memory used by a note */
void load_boards(void);         /* load all boards */
int board_lookup(const char *name); /* Find a board with that name */
bool is_note_to(ACCOUNT_DATA * account, NOTE_DATA * note);  /* is tha note to ch? */
void make_note(const char *board_name, const char *sender, const char *to, const char *subject, const int expire_days, const char *text, int flags);
void save_notes();
bool double_post(char *playername, char *noteboard, char *subject);

/* for nanny */
void handle_con_note_to(DESCRIPTOR_DATA * d, char *argument);
void handle_con_note_subject(DESCRIPTOR_DATA * d, char *argument);
void handle_con_note_expire(DESCRIPTOR_DATA * d, char *argument);
void handle_con_note_text(DESCRIPTOR_DATA * d, char *argument);
void handle_con_note_finish(DESCRIPTOR_DATA * d, char *argument);

/* Commands */

