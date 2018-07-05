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

#ifndef _DYSTOPIA_HEADER_
#define _DYSTOPIA_HEADER_

/* system libraries */
#include <arpa/telnet.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <sys/cdefs.h>
#include <sys/time.h>
#include <pthread.h>
#include <zlib.h>

/* local header files */
#include "vnums.h"
#include "quests.h"
#include "kingdoms.h"
#include "artifacts.h"
#include "mxp.h"
#include "event.h"

/* class header files */
#include "shadow.h"
#include "warlock.h"
#include "giant.h"
#include "fae.h"

#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_QUEST_FUN( fun )        QUEST_FUN fun
#define DECLARE_DEATH_FUN( fun )	DEATH_FUN fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#define DECLARE_SHOP_FUN( fun )         SHOP_FUN  fun

#define FALSE	 0
#define TRUE	 1

typedef short int       sh_int;
typedef unsigned char   bool;

#define TELOPT_COMPRESS        85
#define TELOPT_MSP             90
#define COMPRESS_BUF_SIZE   16384

/*
 * Structure types.
 */
typedef struct history_data         HISTORY_DATA;
typedef struct auction_data         AUCTION_DATA;
typedef struct area_affect          AREA_AFFECT;
typedef struct event_data           EVENT_DATA;
typedef struct List                 LIST;
typedef struct Iterator             ITERATOR;
typedef struct Stack                STACK;
typedef struct Cell                 CELL;
typedef struct StackCell            SCELL;
typedef struct kingdom_data         KINGDOM_DATA;
typedef struct kingdom_structure    KINGDOM_STRUCTURE;
typedef struct kingdom_quest        KINGDOM_QUEST;
typedef struct evolve_data          EVOLVE_DATA;
typedef struct account_data         ACCOUNT_DATA;
typedef struct feed_data            FEED_DATA;
typedef struct session_data         SESSION_DATA;
typedef struct affect_data          AFFECT_DATA;
typedef struct ignore_data          IGNORE_DATA;
typedef struct area_data            AREA_DATA;
typedef struct ban_data             BAN_DATA;
typedef struct char_data            CHAR_DATA;
typedef struct quest_data           QUEST_DATA;
typedef struct change_data          CHANGE_DATA;
typedef struct vote_data            VOTE_DATA;
typedef struct poll_data            POLL_DATA;
typedef struct l_board              LEADER_BOARD;
typedef struct teamarena_data       TEAMARENA_DATA;
typedef struct map_type             MAP_TYPE;
typedef struct editor_data          EDITOR_DATA;
typedef struct dummy_arg            DUMMY_ARG;
typedef struct descriptor_data      DESCRIPTOR_DATA;
typedef struct exit_data            EXIT_DATA;
typedef struct extra_descr_data     EXTRA_DESCR_DATA;
typedef struct help_data            HELP_DATA;
typedef struct kill_data            KILL_DATA;
typedef struct mob_index_data       MOB_INDEX_DATA;
typedef struct war_data             WAR_DATA;
typedef struct note_data            NOTE_DATA;
typedef struct obj_data             OBJ_DATA;
typedef struct obj_index_data       OBJ_INDEX_DATA;
typedef struct pc_data              PC_DATA;
typedef struct reset_data           RESET_DATA;
typedef struct room_index_data      ROOM_INDEX_DATA;
typedef struct time_info_data       TIME_INFO_DATA;
typedef struct weather_data         WEATHER_DATA;
typedef struct disabled_data        DISABLED_DATA;
typedef struct alias_data           ALIAS_DATA;
typedef struct snoop_data           SNOOP_DATA;

/* the event functions looks like this */
typedef bool EVENT_FUN   ( EVENT_DATA *event );

/* dynamic buffer by Erwin Andreasen */
typedef struct buffer_type
{
  char   * data;        /*  The data                          */
  int      len;         /*  The current length of the buffer  */
  int      size;        /*  The allocated size of data        */
} BUFFER;

struct disabled_data
{
  struct cmd_type const * command;       /* pointer to the command struct */
  char                  * disabled_by;   /* name of disabler              */
  sh_int                  level;         /* level of disabler             */
};

extern LIST *disabled_list;

/*
 * Function types.
 */
typedef void DO_FUN     ( CHAR_DATA * ch, char *argument );
typedef void SPEC_FUN   ( CHAR_DATA * ch, char *argument );
typedef void QUEST_FUN  ( CHAR_DATA * questmaster, CHAR_DATA * ch, char *argument );
typedef void DEATH_FUN  ( CHAR_DATA *ch, CHAR_DATA *killer );
typedef void SPELL_FUN  ( int sn, int level, CHAR_DATA * ch, void *vo );
typedef void SHOP_FUN   ( CHAR_DATA * shopkeeper, CHAR_DATA * ch, char *argument );

/*
 * String and memory management parameters.
 */
#define	MAX_KEY_HASH		 1024
#define MAX_STRING_LENGTH	 8192
#define MAX_INPUT_LENGTH	  400

/* 
 * Rotains Gobal Procedures
 */
void  improve_wpn        ( CHAR_DATA *ch, int dtype, int right_hand );
void  improve_stance     ( CHAR_DATA *ch );
void  skillstance        ( CHAR_DATA *ch, CHAR_DATA *victim );
void  show_spell         ( CHAR_DATA *ch, int dtype );
void  crack_head         ( CHAR_DATA *ch, OBJ_DATA *obj, char *argument );
void  critical_hit       ( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, int dt, int dam );
void  take_item          ( CHAR_DATA *ch, OBJ_DATA *obj );
void  raw_kill           ( CHAR_DATA *victim, CHAR_DATA *killer );
void  trip               ( CHAR_DATA *ch, CHAR_DATA *victim );
void  disarm             ( CHAR_DATA *ch, CHAR_DATA *victim );
void  make_corpse        ( CHAR_DATA *ch, CHAR_DATA *killer );
void  one_hit            ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, int handtype );
void  special_hurl       ( CHAR_DATA *ch, CHAR_DATA *victim );
void  make_part          ( CHAR_DATA *ch, char *argument );
void  behead             ( CHAR_DATA *victim );
void  paradox            ( CHAR_DATA *ch );

/* 
 * Godwars Game Parameters
 * By Rotain
 */
#define SKILL_ADEPT               100
#define MAX_ALIAS                  60
#define MAX_CHARACTERS             20
#define MAX_EVENT_HASH            128

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_CHANGE                 15
#define MAX_VOTE_OPTIONS	    5
#define CURRENT_REVISION            0
#define REVISION_OUTDATED          -1
#define REVISION_FROZEN            -2
#define MAX_SKILL		  158
#define MAX_SPELL		   72
#define MAX_LEVEL		   12
#define MAX_CLASS                   4

#define BRIEF_1      0
#define BRIEF_2      1
#define BRIEF_3      2
#define BRIEF_4      3
#define BRIEF_5      4
#define BRIEF_6      5
#define BRIEF_7      6
#define BRIEF_8      7
#define BRIEF_9      8
#define BRIEF_10     9
#define MAX_BRIEF   10

#define BRIEF5_NUM_RECEIVED     0
#define BRIEF5_NUM_DEALT        1
#define BRIEF5_AMOUNT_RECEIVED  2
#define BRIEF5_AMOUNT_DEALT     3

/* for reduce_cost */
#define eMana  1   
#define eMove  2
#define eHit   3

/* arena status flags */
#define ARENA_FORTRESS_INUSE       1
#define ARENA_FORTRESS_READY       2
#define ARENA_FORTRESS_1VS1        4
#define ARENA_FORTRESS_TEAM        8
#define ARENA_FORTRESS_DEATH      16
#define ARENA_FORTRESS_SPAR       32
#define ARENA_FORTRESS_CONTEST    64
#define ARENA_ARENA_VETRANK      128
#define ARENA_ARENA_LEGRANK      256
#define ARENA_ARENA_CLOSED       512
#define ARENA_ARENA_OPEN        1024
#define ARENA_ARENA_INUSE       2048

/* Size of the map and depth of recursion to undertake */
#define MAPX     10
#define MAPY      8
#define MAXDEPTH  2

#define LEVEL_MORTAL		   (MAX_LEVEL - 10)
#define LEVEL_AVATAR		   (MAX_LEVEL - 9)
#define LEVEL_IMMORTAL             (MAX_LEVEL - 5)
#define LEVEL_QUESTMAKER	   (MAX_LEVEL - 4)
#define LEVEL_ENFORCER		   (MAX_LEVEL - 3)
#define LEVEL_JUDGE		   (MAX_LEVEL - 2)
#define LEVEL_HIGHJUDGE		   (MAX_LEVEL - 1)
#define LEVEL_IMPLEMENTOR	   (MAX_LEVEL)

#define RANK_ALMIGHTY           1000
#define RANK_RULER               900
#define RANK_SUPREME             810
#define RANK_KING                755
#define RANK_BARON               700
#define RANK_DUKE                645
#define RANK_GENERAL             590
#define RANK_CAPTAIN             535
#define RANK_MASTER              480
#define RANK_LEGENDARY           425
#define RANK_HERO                370
#define RANK_ADVENTURER          315
#define RANK_VETERAN             260
#define RANK_PRIVATE             205
#define RANK_CADET               150
#define RANK_WANNABE               0

#define PULSE_PER_SECOND	    4
#define PULSE_TICK		  (30 * PULSE_PER_SECOND)
#define PULSE_TRACK               (40 * PULSE_PER_SECOND)

#define AREA_AFF_EARTHMOTHER       1
#define AREA_AFF_PLAGUE            2
#define AREA_AFF_MILKANDHONEY      3

/* include the board specifications, they require some defs from above... */
#include "board.h"

/*
 * Site ban structure.
 */
struct ban_data
{
  char *name;
  char *reason;
};

/*
 * Time and weather stuff.
 */
#define SUN_DARK		    0
#define SUN_RISE		    1
#define SUN_LIGHT		    2
#define SUN_SET			    3

#define SKY_CLOUDLESS		    0
#define SKY_CLOUDY		    1
#define SKY_RAINING		    2
#define SKY_LIGHTNING		    3

struct time_info_data
{
  int hour;
  int day;
  int month;
  int year;
};

struct weather_data
{
  int mmhg;
  int change;
  int sky;
  int sunlight;
};

/*
 * Extra Descr bits
 */
#define ED_TYPE_NONE        0
#define ED_TYPE_PULL        1
#define ED_TYPE_PRESS       2
#define ED_TYPE_PUSH        3
#define ED_TYPE_TOUCH       4

#define ED_ACTION_NONE      0
#define ED_ACTION_TELEPORT  1
#define ED_ACTION_OBJECT    2
#define ED_ACTION_SPELL     3
#define ED_ACTION_ELEVATOR  4

/*
 * Mudinfo Bits
 */
#define MUDINFO_UPDATED      0
#define MUDINFO_MCCP_USERS   1
#define MUDINFO_OTHER_USERS  2
#define MUDINFO_PEAK_USERS   3
#define MUDINFO_MBYTE        4
#define MUDINFO_BYTE         5
#define MUDINFO_DATA_PEAK    6
#define MUDINFO_MSP_USERS    7
#define MUDINFO_MXP_USERS    8
#define MUDINFO_MBYTE_S      9
#define MUDINFO_BYTE_S      10
#define MUDINFO_MAX         11

/*
 * Taking care of the control center
 */
#define CCENTER_MIN_EXP                     0
#define CCENTER_MAX_EXP                     1
#define CCENTER_EXP_LEVEL                   2
#define CCENTER_QPS_LEVEL                   3
#define CCENTER_MAX                         4

#define CCENTER_MIN_EXP_DEFAULT          4000
#define CCENTER_MAX_EXP_DEFAULT       2500000
#define CCENTER_EXP_LEVEL_DEFAULT         100
#define CCENTER_QPS_LEVEL_DEFAULT         100

/*
 * Connected state for a channel.
 */
#define CON_PLAYING			 0
#define CON_ACCOUNT_NAME                 1
#define CON_CONFIRM_ACCOUNT              2
#define CON_NEW_PASSWORD                 3
#define CON_OLD_PASSWORD                 4
#define CON_CONFIRM_PASSWORD             5
#define CON_PICK_PLAYER                  6
#define CON_DELETE_PLAYER                7
#define CON_CONFIRM_DEL_PLAYER           8
#define CON_NEW_CHARACTER                9
#define CON_CONFIRM_NEW_CHARACTER        10
#define CON_GET_NEW_CLASS                11
#define CON_CONFIRM_CLASS                12
#define CON_GET_NEW_SEX			 13
#define CON_GET_NEW_ANSI                 14
#define CON_NOT_PLAYING			 15
#define CON_COPYOVER_RECOVER             16
#define CON_NOTE_TO			 17
#define CON_NOTE_SUBJECT	         18
#define CON_NOTE_EXPIRE			 19
#define CON_NOTE_TEXT			 20
#define CON_NOTE_FINISH			 21

#define CON_PICK_REFERENCE               23

/*
 * Needed for threads
 */
struct dummy_arg
{
  DESCRIPTOR_DATA *d;
  char *buf;
  sh_int status;
};

/*
 * better color parsing
 */
#define eBOLD        1
#define eTHIN        2

struct sAnsiColor
{
  const char    cTag;
  const char  * cString;
  int           aFlag;
};

struct sHTMLColor
{
  const char    cTag;
  const char  * cString;
};

struct evolve_entry
{
  char   * name;
  int      req_field;
  int      req_bit;
  int      evolve_field;
  int      evolve_bit;  
  int      oppose_field;
  int      oppose_bit;  
  int      hps;
  int      mana;
  int      move;
  int      exp; 
  int      gold; 
};

struct auction_data
{
  OBJ_DATA         * obj;               /* a pointer to the object being auctioned      */
  char             * seller_name;       /* the name of the player selling the item      */
  char             * seller_account;    /* the accountname of the selling player        */
  char             * bidder_name;       /* the name of the player with the highest bid  */
  char             * bidder_account;    /* the accountname of the highest bidder        */
  time_t             expire;            /* the unix date upon which the auction expires */
  int                bid;               /* the current bid or the minimum bid           */
  int                bidout;            /* the bidout amount for this auction           */
  int                id;                /* the unique ID given at boot time             */
};

struct area_affect
{
  int             owner;
  sh_int          type;
  sh_int          duration;
  sh_int          level;
};

typedef struct top10_entry
{
  char               * name;
  int                  pkscore;
  int                  pkills;
  int                  pdeaths;
  int                  hours;
} TOP10_ENTRY;

typedef struct muddata
{
  /* saved data */
  int         mudinfo[MUDINFO_MAX];
  int         ccenter[CCENTER_MAX];
  int         class_count[MAX_CLASS];
  int         pk_count_now[3];
  int         pk_count_last[3];
  int         total_output;
  int         top_playerid;
  int         questpool;

  /* session only data */
  int         events_queued;
  int         events_allocated;
} MUDDATA;

typedef struct member_data
{
  char               * name;
  char               * invited_by;
  sh_int               level;
  int                  flags;
  int                  might;
  int                  pk;
  int                  pd;
} MEMBER_DATA;

struct evolve_data
{
  bool      valid;
  int     * field;
  int       bit;
  int       mana;
  int       hps;
  int       move;
  int       exp;
  int       gold;
  char      error[MAX_STRING_LENGTH];
};

struct kingdom_quest
{
  char            * namelist;
  char            * passphrase;
  char              clues[16];
  sh_int            current_quest;
  sh_int            time_left;
  sh_int            player_count;
};

/*
 * A kingdom can build structures.
 */
struct kingdom_structure
{
  int                  vnum;        /* room the building is in */
  int                  type;        /* type of building        */
  int                  values[4];   /* different values for this building */
};

/* one kingdom */
struct kingdom_data
{
  LIST              * buildings;
  LIST              * members;
  LIST              * invited;
  char              * file;
  char              * whoname;
  char              * longname;
  char              * shortname;
  char              * leader;
  char              * prefix;
  char              * suffix;
  sh_int              taxrate;
  int                 treasury;
  int                 kingid;
  int                 pkills;
  int                 pdeaths;
  int                 vnums;
  int                 flags;
  int                 king_active;
  int                 entry;
};

/* this is one artifact */
struct arti_type
{
  char                * owner;
  char                * fun;
  int                   vnum;
  int                   active;
};

struct arti_entry
{
  char       * name;
  EVENT_FUN  * fun;
  int          type;
  int          delay;
};

struct List
{
  CELL  *_pFirstCell;
  CELL  *_pLastCell;
  int    _iterators;
  int    _size;
  int    _valid;
  int    _invalidcells;
};
  
struct Cell
{
  CELL  *_pNextCell;
  CELL  *_pPrevCell;
  void  *_pContent;
  int    _valid;
};
  
struct Iterator
{
  ITERATOR    *_pNext;
  LIST        *_pList;
  CELL        *_pCell;
  int          _reverse;
};

struct Stack
{
  SCELL *_pCells;
  int    _iSize; 
};
  
struct StackCell
{
  SCELL  *_pNext;
  void   *_pContent;
};

/*
 * One event.
 */
struct event_data
{
  EVENT_FUN        * fun;              /* the function being called               */
  char             * argument;         /* the text argument given (if any)        */
  sh_int             passes;           /* how long before this event executes     */
  sh_int             type;             /* event type ETYPE_XXX                    */
  sh_int             ownertype;        /* type of owner (unlinking req)           */
  sh_int             bucket;           /* which bucket is this event in           */

  union                                /*                                         */
  {                                    /* this is the owner of the event, we use  */
    CHAR_DATA       * ch;              /* a union to make sure any of the types   */
    OBJ_DATA        * obj;             /* can be used for an event.               */
    DESCRIPTOR_DATA * desc;            /*                                         */
    ROOM_INDEX_DATA * room;            /*                                         */
    AREA_DATA       * area;
  } owner;
};

/* An attempt to keep track of feeders. 
 * Doubt we would want to use it, but it's always nice to
 * have that option.
 */
struct feed_data
{
  int           time;         /* time left before this expires */
  int           playerid;     /* the id of the attacker        */
  int           might;        /* the mightrate of the attacker */
  bool          fair;         /* is this a fair attacker       */
};

struct session_data
{
  int              mana;
  int              move;
  int              hit;
  int              exp;
  int              gold;
  int              quests;
  int              mkills;
  int              pkills;
};

/* how many players can there maximum be in one team red/blue */
#define MAX_TEAM_SIZE  4

struct teamarena_data
{
  CHAR_DATA  * blueteam [2 * MAX_TEAM_SIZE];   /* the ID's of the blue players              */
  CHAR_DATA  * redteam  [2 * MAX_TEAM_SIZE];   /* the ID's of the red players               */
  CHAR_DATA  * signup   [2 * MAX_TEAM_SIZE];   /* when players signup, they are placed here */
  int          status;                         /* the status of arena/fortress              */
};

/*
 * account data
 */
struct account_data
{
  BOARD_DATA    *  board;                /*  the noteboard data                    */
  char          *  owner;                /*  the accounts owner (login name)       */
  char          *  password;             /*  the accounts password (crypt)         */
  char          *  new_password;         /*  used when changing password           */
  char          *  players;              /*  the list of players owned             */
  char          *  reference;            /*  who to credit for this account        */
  char          *  notes;                /*  admin notes about this account        */
  time_t           created;              /*  when this account was created         */
  time_t           denied;               /*  how long is this account denied       */
  time_t           lastlogged;           /*  last time this account was logged     */
  time_t           last_note[MAX_BOARD]; /*  pointer to last note on each board    */
  int              flags;                /*  one bitvector for setting flags       */
  int              goldcrowns;           /*  amount of goldcrowns in account       */
  int              goldtotal;            /*  total amount of goldcrowns earned     */
  sh_int           level;                /*  the trust-level of this account       */
  sh_int           p_count;              /*  amount of created characters          */
  sh_int           reimb_points;         /*  reimb points for pwipes/awards        */
  sh_int           max_might;            /*  mightrate of most powerful character  */
  sh_int           timezone;             /*  accounts difference in timezone       */
  sh_int           popup;                /*  newbie popup stuff                    */
};

/*
 * Descriptor (channel) structure.
 */
struct descriptor_data
{
  LIST                  *   snoops;
  LIST                  *   events; 
  CHAR_DATA             *   character;
  ACCOUNT_DATA          *   account;
  char                  *   hostname;
  char                  *   ip_address;
  char                  *   outbuf;
  char                      inbuf[4 * MAX_INPUT_LENGTH];
  char                      incomm[MAX_INPUT_LENGTH];
  char                      inlast[MAX_INPUT_LENGTH];
  char                 **   pString;
  sh_int                    descriptor;
  sh_int                    connected;
  sh_int                    alias_block;
  bool                      fcommand;
  bool                      mxp;
  bool                      msp;
  bool                      bClosed;
  bool                      bResolved;
  bool                      bBanned;
  int                       repeat;
  int                       outsize;
  int                       outtop;
  int                       editor;
  void                  *   pEdit;
  z_stream              *   out_compress;
  unsigned char         *   out_compress_buf;
};

struct class_type
{
  char                        *  class_name;
  int                            class_num;
  bool                           autoclass;
};

/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3
#define TO_ALL		    4

/*
 * Help table types.
 */
struct help_data
{
  int           level;
  char        * name;
  char        * keyword;
  char        * text;
};

/*
 * Data structure for notes.
 */
struct note_data
{
  char       * sender;
  char       * date;
  char       * to_list;
  char       * subject;
  char       * text;
  time_t       date_stamp;
  time_t       expire;
  int          flags;
};

/*
 * An affect.
 */
struct affect_data
{
  sh_int type;
  sh_int duration;
  sh_int location;
  sh_int modifier;
  int bitvector;
};

struct plist
{
  char     text[MAX_STRING_LENGTH];     /*  the list of players    */
  int      count;                       /*  the amount of players  */
};

/*
 * A quest
 */
struct quest_data
{
  time_t       expire;           /* datestamp for expire      */
  sh_int       type;             /* the type of quest         */
  int          giver;            /* the questmasters vnum     */
  int          vnums[4];         /* vnum of questor           */
};

/* Structure for the map itself */
struct map_type
{
  char   tegn;                  /* Character to print at this map coord */
  int    vnum;                  /* Room this coord represents */
  int    depth;                 /* Recursive depth this coord was found at */
  int    info;
  bool   can_see;
};

/*
 * one single change
 */
struct change_data
{
  char         * imm;
  char         * text;
  char         * date;
};

typedef struct heap_data
{
  sh_int              iVertice;
  ROOM_INDEX_DATA  ** knude;
  int                 size;
} HEAP;

/*
 * one poll
 */
struct poll_data
{
  LIST        * votes;
  time_t        expire;
  char        * name;
  char        * description;
  char        * options[MAX_VOTE_OPTIONS];
  int           vcount[MAX_VOTE_OPTIONS];
};

/*
 * one vote
 */
struct vote_data
{
  char        * pname;
  char        * phost;
  int           choice;
};

/*
 * A kill structure (indexed by level).
 */
struct kill_data
{
  sh_int number;
  sh_int killed;
};

/* attack types in 100x */
#define DT_UNARMED      1000
#define DT_SLICE        1001
#define DT_STAB         1002
#define DT_SLASH        1003
#define DT_WHIP         1004
#define DT_CLAW         1005
#define DT_BLAST        1006
#define DT_POUND        1007
#define DT_CRUSH        1008
#define DT_GREP         1009
#define DT_BITE         1010
#define DT_PIERCE       1011
#define DT_SUCK         1012

/*
 * Immunities, for players.  KaVir.
 */
#define IMM_SLASH             1       /* Resistance to slash, slice.          */
#define IMM_STAB              2       /* Resistance to stab, pierce.          */
#define IMM_SMASH             4       /* Resistance to blast, pound, crush.   */
#define IMM_ANIMAL            8       /* Resistance to bite, claw.            */
#define IMM_MISC             16       /* Resistance to grep, suck, whip.      */
#define IMM_CHARM            32       /* Immune to charm spell.               */
#define IMM_HEAT             64       /* Immune to fire/heat spells.          */
#define IMM_COLD            128       /* Immune to frost/cold spells.         */
#define IMM_LIGHTNING       256       /* Immune to lightning spells.          */
#define IMM_ACID            512       /* Immune to acid spells.               */
#define IMM_SUMMON         1024       /* Immune to being summoned.            */
#define IMM_SHIELDED       2048       /* For Obfuscate. Block scry, etc.      */
#define IMM_HURL           4096       /* Cannot be hurled.                    */
#define IMM_BACKSTAB       8192       /* Cannot be backstabbed.               */
#define IMM_KICK          16384       /* Cannot be kicked.                    */
#define IMM_DISARM        32768       /* Cannot be disarmed.                  */
#define IMM_STEAL         65536       /* Cannot have stuff stolen.            */

#define IMM_DRAIN        262144       /* Immune to energy drain.              */
#define IMM_TRANSPORT    524288       /* Objects can't be transported to you. */

/*
 * ACT bits for mobs.  FIX ME - should probably not store these as ch->act
 * but rather use a different approach like ch->mob_act
 */
#define ACT_IS_NPC                  1   /* Auto set for mobs    */
#define ACT_SENTINEL                2   /* Stays in one room    */
#define ACT_SCAVENGER               4   /* Picks up objects     */
#define ACT_AGGRESSIVE              8   /* Attacks PC's         */
#define ACT_STAY_AREA              16   /* Won't leave area     */
#define ACT_WIMPY                  32   /* Flees when hurt      */
#define ACT_FISH                   64   /* Can only swim        */
#define ACT_NOSWIM                128   /* cannot enter water   */
#define ACT_MOUNT                 512   /* Can be mounted       */
#define ACT_NOPARTS              1024   /* Dead = no body parts */
#define ACT_NOEXP                2048   /* No exp for killing   */
#define ACT_NOAUTOKILL           4096
#define ACT_BANDING              8192
#define ACT_SEMIAGGRESSIVE      16384
#define ACT_NOSTANCE            32768

/* bits for affects on objects */
#define OAFF_LIQUID                   1
#define OAFF_FROSTBITE                2

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND		      1
#define AFF_INVISIBLE		      2
#define AFF_DETECT_EVIL		      4
#define AFF_DETECT_INVIS	      8
#define AFF_DETECT_MAGIC	     16
#define AFF_DETECT_HIDDEN	     32
#define AFF_BEACON		     64
#define AFF_SANCTUARY		    128
#define AFF_FIRESHIELD		    256
#define AFF_INFRARED		    512
#define AFF_CURSE		   1024
#define AFF_FLAMING		   2048   /* For burning creatures - KaVir */
#define AFF_POISON		   4096
#define AFF_PROTECT		   8192
#define AFF_ETHEREAL		  16384 /* For ethereal creatures - KaVir */
#define AFF_SNEAK		  32768
#define AFF_MINDBLANK             65536
#define AFF_CHARM		 131072
#define AFF_FLYING		 262144
#define AFF_PASS_DOOR		 524288
#define AFF_POLYMORPH		1048576 /* For polymorphed creatures - KaVir */
#define AFF_SHADOWSIGHT		2097152 /* Can see between planes - KaVir */
#define AFF_PROTECT_GOOD        4194304
#define AFF_MINDBOOST           8388608
#define AFF_WEBBED             16777216
#define AFF_MVEST              33554432
#define AFF_WALLSWORDS         67108864
#define AFF_SHATTERSHIELD     134217728

/*
 * Bits for 'itemaffect'.
 * Used in #MOBILES.
 */
#define ITEMA_CHAOSSHIELD             1
/* free one - was itema_artifact */
#define ITEMA_REGENERATE              4
#define ITEMA_SPEED                   8
/* free one - was itema_vorpal */
#define ITEMA_RESISTANCE             32
#define ITEMA_VISION                 64
#define ITEMA_STALKER               128
#define ITEMA_VANISH                256

/* this is for weapon->value[0] / 1000 && armor->value[3] */
#define OBJECT_BLIND            1
#define OBJECT_DETECTINVIS      2
#define OBJECT_FLYING           3
#define OBJECT_INFRARED         4
#define OBJECT_INVISIBLE        5
#define OBJECT_PASSDOOR         6
#define OBJECT_PROTECT          7
#define OBJECT_PROTECTGOOD      8
#define OBJECT_SANCTUARY        9
#define OBJECT_DETECTHIDDEN    10
#define OBJECT_SNEAK           11
#define OBJECT_CHAOSSHIELD     12
#define OBJECT_REGENERATE      13
#define OBJECT_SPEED           14
#define OBJECT_VORPAL          15  /* should only be set on weapons */
#define OBJECT_RESISTANCE      16
#define OBJECT_VISION          17
#define OBJECT_STALKER         18
#define OBJECT_VANISH          19

#define IREAD(sKey, sPtr)             \
{                                     \
  if (!str_cmp(sKey, word))           \
  {                                   \
    sPtr = fread_number(fp);          \
    found = TRUE;                     \
    break;                            \
  }                                   \
}
#define SREAD(sKey, sPtr)             \
{                                     \
  if (!str_cmp(sKey, word))           \
  {                                   \
    sPtr = fread_string(fp);          \
    found = TRUE;                     \
    break;                            \
  }                                   \
}

/*
 * Damcap values.
 */
#define DAM_CAP		      0
#define DAM_CHANGE	      1

/*
 * Mounts
 */
#define IS_ON_FOOT		      0
#define IS_MOUNT		      1
#define IS_RIDING		      2
#define IS_CARRIED		      4
#define IS_CARRYING		      8

/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL		      0
#define SEX_MALE		      1
#define SEX_FEMALE		      2

/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT		      1
#define ITEM_SCROLL		      2
#define ITEM_WAND		      3
#define ITEM_STAFF		      4
#define ITEM_WEAPON		      5
#define ITEM_TREASURE		      6
#define ITEM_ARMOR		      7
#define ITEM_POTION		      8
#define ITEM_FURNITURE		      9
#define ITEM_TRASH		     10
#define ITEM_CONTAINER		     11
#define ITEM_DRINK_CON		     12
#define ITEM_KEY		     13
#define ITEM_FOOD		     14
#define ITEM_MONEY		     15
#define ITEM_BOAT		     16
#define ITEM_CORPSE_NPC		     17
#define ITEM_CORPSE_PC		     18
#define ITEM_FOUNTAIN		     19
#define ITEM_PILL		     20
#define ITEM_PORTAL		     21
#define ITEM_QUEST		     22
#define ITEM_WALL                    23
#define ITEM_HEAD		     24
#define ITEM_QUESTCLUE               25
#define ITEM_HOMING                  26
#define ITEM_FAETOKEN                27

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		      1
#define ITEM_HUM		      2
#define ITEM_NOSHOW		      4
#define ITEM_KEEP		      8
#define ITEM_VANISH		     16
#define ITEM_INVIS		     32
#define ITEM_MAGIC		     64
#define ITEM_NODROP		    128
#define ITEM_BLESS		    256
#define ITEM_ANTI_GOOD		    512
#define ITEM_ANTI_EVIL		   1024
#define ITEM_ANTI_NEUTRAL	   2048
#define ITEM_NOREMOVE		   4096
#define ITEM_INVENTORY		   8192
#define ITEM_LOYAL		  16384
#define ITEM_OLC                  32768
#define ITEM_RARE                 65536
#define ITEM_NOCLAIM             131072
#define ITEM_NOLOCATE            262144
#define ITEM_SENTIENT            524288
#define ITEM_GEMMED             1048576
#define ITEM_FAE_BLAST          2097152
#define ITEM_SHADOWEDGE         4194304
#define ITEM_BLOODENHANCE       8388608
#define ITEM_MASTERY           16777216
#define ITEM_UNBREAKABLE       33554432
#define ITEM_NOREPAIR          67108864

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		      1
#define ITEM_WEAR_FINGER	      2
#define ITEM_WEAR_NECK		      4
#define ITEM_WEAR_BODY		      8
#define ITEM_WEAR_HEAD		     16
#define ITEM_WEAR_LEGS		     32
#define ITEM_WEAR_FEET		     64
#define ITEM_WEAR_HANDS		    128
#define ITEM_WEAR_ARMS		    256
#define ITEM_WEAR_SHIELD	    512
#define ITEM_WEAR_ABOUT		   1024
#define ITEM_WEAR_WAIST		   2048
#define ITEM_WEAR_WRIST		   4096
#define ITEM_WIELD		   8192
#define ITEM_HOLD		  16384
#define ITEM_WEAR_FACE		  32768
#define ITEM_WEAR_MASTERY	  65536
#define ITEM_WEAR_BODYART 	 131072
#define ITEM_WEAR_MEDAL          262144
#define ITEM_WEAR_FLOAT          524288
#define ITEM_WEAR_FAE            614401
#define ITEM_WEAR_ALL           1048575  /* should be the last item_wear flag times two minus one */

/*
 * Apply types (for quest affects).
 * Used in #OBJECTS.
 */
#define QUEST_STR		      1
#define QUEST_DEX		      2
#define QUEST_INT		      4
#define QUEST_WIS		      8
#define QUEST_CON		     16
#define QUEST_HITROLL		     32
#define QUEST_DAMROLL		     64
#define QUEST_HIT		    128
#define QUEST_MANA		    256
#define QUEST_MOVE		    512
#define QUEST_AC		   1024
#define QUEST_ENCHANTED		   2048
#define QUEST_SPELLPROOF	   4096
#define QUEST_ARTIFACT		   8192
#define QUEST_IMPROVED		  16384
#define QUEST_PRIZE      	  32768
#define QUEST_RELIC               65536
#define QUEST_GIANTSTONE         131072

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE		      0
#define APPLY_STR		      1
#define APPLY_DEX		      2
#define APPLY_INT		      3
#define APPLY_WIS		      4
#define APPLY_CON		      5
#define APPLY_SEX		      6
#define APPLY_CLASS		      7
#define APPLY_LEVEL		      8
#define APPLY_AGE		      9
#define APPLY_HEIGHT		     10
#define APPLY_WEIGHT		     11
#define APPLY_MANA		     12
#define APPLY_HIT		     13
#define APPLY_MOVE		     14
#define APPLY_GOLD		     15
#define APPLY_EXP		     16
#define APPLY_AC		     17
#define APPLY_HITROLL		     18
#define APPLY_DAMROLL		     19
#define APPLY_SAVING_PARA	     20
#define APPLY_SAVING_ROD	     21
#define APPLY_SAVING_PETRI	     22
#define APPLY_SAVING_BREATH	     23
#define APPLY_SAVING_SPELL	     24

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK		      1

#define ROOM_NO_MOB		      4
#define ROOM_INDOORS		      8
#define ROOM_PRIVATE		     16
#define ROOM_SAFE		     32
#define ROOM_NO_RECALL		     64
#define ROOM_DISPEL_MAGIC           128
#define ROOM_TOTAL_DARKNESS         256
#define ROOM_BLADE_BARRIER          512

#define ROOM_FLAMING               2048
#define ROOM_SILENCE               4096
#define ROOM_ASTRAL		   8192
#define ROOM_KINGDOM              16384

#define ROOM_LIGHT                65536
#define ROOM_NO_RANDOM           131072
#define ROOM_NO_HOME             262144

/*
 * Room text flags (KaVir).
 * Used in #ROOMS.
 */
#define RT_LIGHTS		      1     /* Toggles lights on/off */
#define RT_SAY			      2     /* Use this if no others powers */
#define RT_ENTER		      4
#define RT_CAST			      8
#define RT_THROWOUT		     16   /* Erm...can't remember ;) */
#define RT_OBJECT		     32     /* Creates an object */
#define RT_MOBILE		     64     /* Creates a mobile */
#define RT_LIGHT		    128     /* Lights on ONLY */
#define RT_DARK			    256     /* Lights off ONLY */
#define RT_OPEN_LIFT		    512 /* Open lift */
#define RT_CLOSE_LIFT		   1024 /* Close lift */
#define RT_MOVE_LIFT		   2048 /* Move lift */
#define RT_SPELL		   4096     /* Cast a spell */
#define RT_PORTAL		   8192     /* Creates a one-way portal */
#define RT_TELEPORT		  16384   /* Teleport player to room */
#define RT_ACTION		  32768
#define RT_BLANK_1		  65536
#define RT_BLANK_2		 131072
#define RT_TEXT                  262144 /* Send a block of text to char */
#define RT_RETURN		1048576     /* Perform once */
#define RT_PERSONAL		2097152   /* Only shows message to char */
#define RT_TIMER		4194304     /* Sets object timer to 1 tick */

/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH		      0
#define DIR_EAST		      1
#define DIR_SOUTH		      2
#define DIR_WEST		      3
#define DIR_UP			      4
#define DIR_DOWN		      5

/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		      1
#define EX_CLOSED		      2
#define EX_LOCKED		      4
#define EX_PICKPROOF		     32
#define EX_NOPASS                    64
#define EX_EASY                     128
#define EX_HARD                     256
#define EX_INFURIATING              512
#define EX_NOCLOSE                 1024
#define EX_NOLOCK                  2048
#define EX_ICE_WALL                4096
#define EX_FIRE_WALL               8192
#define EX_SWORD_WALL             16384
#define EX_PRISMATIC_WALL         32768
#define EX_SHADOW_WALL            65536
#define EX_MUSHROOM_WALL         131072
#define EX_CALTROP_WALL          262144
#define EX_ASH_WALL              524288
#define EX_WARDING              1048576

#define MAX_EXFLAG		      20
#define MAX_WALL		       8

/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE		      0
#define SECT_CITY		      1
#define SECT_FIELD		      2
#define SECT_FOREST		      3
#define SECT_HILLS		      4
#define SECT_MOUNTAIN		      5
#define SECT_WATER_SWIM		      6
#define SECT_WATER_NOSWIM	      7
#define SECT_UNDERGROUND	      8
#define SECT_AIR		      9
#define SECT_DESERT		     10
#define SECT_MAX		     11

/*
 * Equipment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE		     -1
#define WEAR_LIGHT		      0
#define WEAR_FINGER_L		      1
#define WEAR_FINGER_R		      2
#define WEAR_NECK_1		      3
#define WEAR_NECK_2		      4
#define WEAR_BODY		      5
#define WEAR_HEAD		      6
#define WEAR_LEGS		      7
#define WEAR_FEET		      8
#define WEAR_HANDS		      9
#define WEAR_ARMS		     10
#define WEAR_SHIELD		     11
#define WEAR_ABOUT		     12
#define WEAR_WAIST		     13
#define WEAR_WRIST_L		     14
#define WEAR_WRIST_R		     15
#define WEAR_WIELD		     16
#define WEAR_HOLD		     17
#define WEAR_THIRD		     18
#define WEAR_FOURTH		     19
#define WEAR_FACE		     20
#define WEAR_SCABBARD_L		     21
#define WEAR_SCABBARD_R		     22
#define WEAR_MASTERY                 23
#define WEAR_FLOAT                   24
#define WEAR_MEDAL                   25
#define WEAR_BODYART                 26
#define MAX_WEAR		     27

/*
 * Locations for damage.
 */
#define LOC_HEAD		      0
#define LOC_BODY		      1
#define LOC_ARM_L		      2
#define LOC_ARM_R		      3
#define LOC_LEG_L		      4
#define LOC_LEG_R		      5

/*
 * For Head
 */
#define LOST_EYE_L		       1
#define LOST_EYE_R		       2
#define LOST_EAR_L		       4
#define LOST_EAR_R		       8
#define LOST_NOSE		      16
#define BROKEN_NOSE		      32
#define BROKEN_JAW		      64
#define BROKEN_SKULL		     128
/* free one - was LOST_HEAD */
#define LOST_TOOTH_1		     512  /* These should be added..... */
#define LOST_TOOTH_2		    1024  /* ...together to caculate... */
#define LOST_TOOTH_4		    2048  /* ...the total number of.... */
#define LOST_TOOTH_8		    4096  /* ...teeth lost.  Total..... */
#define LOST_TOOTH_16		    8192  /* ...possible is 31 teeth.   */
#define LOST_TONGUE		   16384

/*
 * For Body
 */
#define BROKEN_RIBS_1		       1  /* Remember there are a total */
#define BROKEN_RIBS_2		       2  /* of 12 pairs of ribs in the */
#define BROKEN_RIBS_4		       4  /* human body, so not all of  */
#define BROKEN_RIBS_8		       8  /* these bits should be set   */
#define BROKEN_RIBS_16		      16  /* at the same time.          */
#define BROKEN_SPINE		      32
#define BROKEN_NECK		      64
#define CUT_THROAT		     128
#define CUT_STOMACH		     256
#define CUT_CHEST		     512

/*
 * For Arms
 */
#define BROKEN_ARM		       1
#define LOST_ARM		       2
#define LOST_HAND		       4
#define LOST_FINGER_I		       8  /* Index finger */
#define LOST_FINGER_M		      16  /* Middle finger */
#define LOST_FINGER_R		      32  /* Ring finger */
#define LOST_FINGER_L		      64  /* Little finger */
#define LOST_THUMB		     128
#define BROKEN_FINGER_I		     256  /* Index finger */
#define BROKEN_FINGER_M		     512  /* Middle finger */
#define BROKEN_FINGER_R		    1024  /* Ring finger */
#define BROKEN_FINGER_L		    2048  /* Little finger */
#define BROKEN_THUMB		    4096

/*
 * For Legs
 */
#define BROKEN_LEG		       1
#define LOST_LEG		       2
#define LOST_FOOT		       4
#define LOST_TOE_A		       8
#define LOST_TOE_B		      16
#define LOST_TOE_C		      32
#define LOST_TOE_D		      64  /* Smallest toe */
#define LOST_TOE_BIG		     128
#define BROKEN_TOE_A		     256
#define BROKEN_TOE_B		     512
#define BROKEN_TOE_C		    1024
#define BROKEN_TOE_D		    2048  /* Smallest toe */
#define BROKEN_TOE_BIG		    4096

/*
 * For Bleeding
 */
#define BLEEDING_HEAD		       1
#define BLEEDING_THROAT		       2
#define BLEEDING_ARM_L		       4
#define BLEEDING_ARM_R		       8
#define BLEEDING_HAND_L		      16
#define BLEEDING_HAND_R		      32
#define BLEEDING_LEG_L		      64
#define BLEEDING_LEG_R		     128
#define BLEEDING_FOOT_L		     256
#define BLEEDING_FOOT_R		     512

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK		      0
#define COND_FULL		      1
#define COND_THIRST		      2

/*
 * Positions.
 */
#define POS_DEAD		      0
#define POS_MORTAL		      1
#define POS_INCAP		      2
#define POS_STUNNED		      3
#define POS_SLEEPING		      4
#define POS_MEDITATING		      5
#define POS_SITTING		      6
#define POS_RESTING		      7
#define POS_FIGHTING                  8
#define POS_STANDING                  9

/*
 * ACT bits for players.
 */
#define PLR_IS_NPC                1   /* Don't EVER set.  */
#define PLR_AUTOHEAD              2
#define PLR_LEFTHAND              4
#define PLR_AUTOEXIT              8
#define PLR_AUTOLOOT             16
#define PLR_AUTOSAC              32
#define PLR_AMBI                128
#define PLR_FREEZE              256
#define PLR_MAP                 512
#define PLR_PROMPT             1024
#define PLR_TELNET_GA          2048
#define PLR_HOLYLIGHT          4096
#define PLR_AUTOSPECTATE       8192
#define PLR_ANSI              16384
#define PLR_PARADOX           32768
#define PLR_HIDE              65536
#define PLR_RIGHTHAND        131072
#define PLR_MUSIC            262144
#define PLR_LOG              524288

/* New bits for playrs */
#define NEW_IRONMIND           1
#define NEW_FUMBLE             2
#define THIRD_HAND             4
#define FOURTH_HAND            8
#define NEW_MASTERY           16
#define NEW_CUBEFORM          32
#define NEW_REIMBCLASS        64
#define NEW_MUDFORM          128
#define NEW_FUMES            256
#define NEW_SHADOWPLANE      512
#define NEW_TENDRIL1        1024
#define NEW_TENDRIL2        2048
#define NEW_TENDRIL3        4096
#define NEW_ENH_COMBAT      8192
#define NEW_CHAMELEON      16384
#define NEW_BANDED         32768  /* mobiles only */
#define NEW_SUPKEEP2       65536
#define NEW_MOUNTAIN      131072
#define NEW_SUPKEEP1      262144
#define NEW_BACKLASH      524288
#define NEW_HSTARS       1048576
#define NEW_STITCHES     2097152
#define NEW_FAEHALO      4194304
#define NEW_SINKHOLE     8388608
#define NEW_PSPRAY      16777216

/* kingdom flags for ch->pcdata->kingflags */
#define KFLAG_CANINVITE        1
#define KFLAG_CANOUTCAST       2
#define KFLAG_CANBUY           4

/* evolve flags (ch->pcdata->powers[???]) */
#define EVOLVE_1              10
#define EVOLVE_2              11
#define EVOLVE_3              12

/*
 * JFLAGS : ch->pcdata->jflags
 */
#define JFLAG_SETDECAP                1
#define JFLAG_SETLOGIN                2
#define JFLAG_SETLOGOUT               4
#define JFLAG_SETAVATAR               8

#define JFLAG_SETTIE                 32
#define JFLAG_NOSET                  64
#define JFLAG_POLICY                128

/*
 * TEMPFLAGS : ch->pcdata->tempflag
 */
#define TEMP_EDGE                     1
#define TEMP_DRAGONORB                2
#define TEMP_ARTIFACT                 4
#define TEMP_DOOMBOLT                 8
#define TEMP_SPECTATE                16

/*
 * EXTRA bits for players. (KaVir)
 */
#define EXTRA_NEWPASS		      1
#define EXTRA_WINDWALK                2
#define TIED_UP			      4
#define GAGGED			      8
#define BLINDFOLDED		     16
#define EXTRA_DONE		     32
#define EXTRA_EXP		     64
#define EXTRA_PREGNANT		    128
#define EXTRA_LABOUR		    256
#define EXTRA_BORN		    512
#define EXTRA_PROMPT		   1024
#define EXTRA_MARRIED		   2048
#define EXTRA_AFK        	   4096
#define EXTRA_CALL_ALL		   8192
#define EXTRA_HEAL                16384
#define EXTRA_PKREADY             32768

/*
 * Stances for combat
 */
#define STANCE_NONE		      0
#define STANCE_VIPER		      1
#define STANCE_CRANE		      2
#define STANCE_CRAB		      3
#define STANCE_MONGOOSE		      4
#define STANCE_BULL		      5
#define STANCE_MANTIS		      6
#define STANCE_DRAGON		      7
#define STANCE_TIGER		      8
#define STANCE_MONKEY		      9
#define STANCE_SWALLOW		     10
#define STANCE_SPIRIT                11
#define STANCE_MOBSTANCE             12
#define STANCE_PKSTANCE              13

/*
 * account levels, feel free to extend the range
 */
#define PLAYER_ACCOUNT                1
#define BUILDER_ACCOUNT               2
#define QUEST_ACCOUNT                 3
#define JUDGE_ACCOUNT                 4
#define CODER_ACCOUNT                 5

/* account flags */
#define ACCOUNT_FLAG_REFER1           1
#define ACCOUNT_FLAG_REFER2           2
#define ACCOUNT_FLAG_MASTERY          4
#define ACCOUNT_FLAG_VACATION         8
#define ACCOUNT_FLAG_SILENCED        16
#define ACCOUNT_FLAG_CHANNELLOG      32

/*
 * Channel bits.
 */

#define	CHANNEL_CHAT		      2
#define CHANNEL_CLASS                 4
#define	CHANNEL_IMMTALK		      8
#define	CHANNEL_MUSIC		     16
#define	CHANNEL_YELL		     32
#define	CHANNEL_LOG		     64
#define	CHANNEL_INFO		    128
#define	CHANNEL_FLAME		    256
#define	CHANNEL_TELL		    512
#define CHANNEL_NEWBIE		   1024
#define CHANNEL_KINGDOM            2048

#define LEADER_PK        0
#define LEADER_TIME      1
#define LEADER_QUEST     2
#define LEADER_MOBKILLS  3
#define LEADER_PKSCORE   4
#define LEADER_ARENA     5
#define LEADER_KINGDOM   6
#define LEADER_STATUS    7
#define LEADER_ACTIVE    8
#define MAX_LEADER       9

#define MAX_HISTORY     20
#define MAX_STORAGE     15

/*
 * History data used for storing messages;
 */
struct history_data
{
  char *player;
  char *message;
};

struct l_board
{
  char   * name[MAX_LEADER];
  int      number[MAX_LEADER];
};

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct mob_index_data
{
  SPEC_FUN       *     spec_fun;
  QUEST_FUN      *     quest_fun;
  DEATH_FUN      *     death_fun;
  SHOP_FUN       *     shop_fun;
  AREA_DATA      *     area;
  char           *     player_name;
  char           *     short_descr;
  char           *     long_descr;
  char           *     description;
  int                  loc_hp[7];
  int                  vnum;
  sh_int               count;
  sh_int               killed;
  sh_int               sex;
  sh_int               mounted;
  sh_int               natural_attack;
  int                  level;
  int                  act;
  int                  extra;
  int                  affected_by;
  int                  alignment;

  /* values 0-4 in OLC */
  int                  toughness;
  int                  dam_modifier;
  int                  extra_attack;
  int                  extra_dodge;
  int                  extra_parry;
};

struct editor_data
{
  sh_int   numlines;
  sh_int   on_line;
  sh_int   size;
  char     line[49][81];
};

/*
 * One character (PC or NPC).
 */
struct char_data
{
  CHAR_DATA        *  master;
  CHAR_DATA        *  leader;
  CHAR_DATA        *  fighting;
  CHAR_DATA        *  mount;
  CHAR_DATA        *  precognition;
  CHAR_DATA        *  challenger;
  CHAR_DATA        *  wizard;
  CHAR_DATA        *  simulacrum;
  LIST             *  events; 
  SPEC_FUN         *  spec_fun;
  QUEST_FUN        *  quest_fun;
  DEATH_FUN        *  death_fun;
  SHOP_FUN         *  shop_fun;
  MOB_INDEX_DATA   *  pIndexData;
  DESCRIPTOR_DATA  *  desc;
  LIST             *  affected;
  LIST             *  carrying;
  ROOM_INDEX_DATA  *  in_room;
  ROOM_INDEX_DATA  *  was_in_room;
  PC_DATA          *  pcdata;
  RESET_DATA       *  respawn;
  char             *  name;
  char             *  short_descr;
  char             *  long_descr;
  char             *  description;
  char             *  morph;
  char             *  createtime;
  char             *  lasttime;
  char             *  lasthost;
  char             *  prompt;
  char             *  cprompt;
  bool                deathmatch;
  bool                dead;
  time_t              logon;
  time_t              xlogon;
  sh_int              gcount;
  sh_int              sex;
  sh_int              generation;
  sh_int              loc_hp[7];
  sh_int              wpn[13];
  sh_int              spl[5];
  sh_int              mounted;
  sh_int              level;
  sh_int              trust;
  sh_int              timer;
  sh_int              wait;
  int                 class;
  int                 immune;
  int                 fight_timer;
  int                 itemaffect;
  int                 form;
  int                 stance[14];
  int                 home[3];
  int                 played;
  int                 pkill;
  int                 pdeath;
  int                 mkill;
  int                 mdeath;
  int                 hit;
  int                 max_hit;
  int                 mana;
  int                 max_mana;
  int                 move;
  int                 max_move;
  int                 exp;
  int                 act;
  int                 extra;
  int                 newbits;
  int                 special;
  int                 affected_by;
  int                 position;
  int                 practice;
  int                 carry_weight;
  int                 carry_number;
  int                 saving_throw;
  int                 alignment;
  int                 hitroll;
  int                 damroll;
  int                 armor;
  int                 deaf;
  int                 damcap[2];
};

/*
 * Data which only PC's have.
 */
struct pc_data
{
  CHAR_DATA    * familiar;
  CHAR_DATA    * partner;
  CHAR_DATA    * propose;
  CHAR_DATA    * pfile;
  CHAR_DATA    * reply;
  NOTE_DATA    * in_progress;
  LIST         * quests;
  AREA_DATA    * prev_area;
  LIST         * history;
  LIST         * spells;
  LIST         * contingency;
  LIST         * aliases;
  LIST         * feeders;
  SESSION_DATA * session;
  LIST         * ignores;
  char         * account;
  char         * last_decap[2];
  char         * retaliation[2];
  char         * bamfin;
  char         * bamfout;
  char         * title;
  char         * conception;
  char         * parents;
  char         * cparents;
  char         * marriage;
  char         * decapmessage;
  char         * tiemessage;
  char         * loginmessage;
  char         * logoutmessage;
  char         * avatarmessage;
  char         * last_global;
  char         * soultarget;
  char         * immcmd;
  char         * enh_combat;
  sh_int         brief[MAX_BRIEF];
  sh_int         legend;
  sh_int         status;
  sh_int         revision;
  sh_int         evolveCount;
  sh_int         mean_paradox_counter;
  sh_int         safe_counter;
  sh_int         perm_str;
  sh_int         perm_int;
  sh_int         perm_wis;
  sh_int         perm_dex;   
  sh_int         perm_con;
  sh_int         mod_str;
  sh_int         mod_int; 
  sh_int         mod_wis;
  sh_int         mod_dex;   
  sh_int         mod_con;  
  sh_int         time_tick;
  sh_int         mortal;
  sh_int         stage[3]; 
  sh_int         condition[3];
  sh_int         learned[MAX_SKILL];
  sh_int         followers;
  sh_int         kingdom;
  sh_int         kingdomrank;
  int            brief5data[4];
  int            kingdomflags;
  int            betting_amount;
  int            betting_char;
  int            playerid;
  int            tempflag;
  int            jflags;
  int            questsrun;
  int            sit_safe;
  int            powers[13];
  int            genes[10];
  int            awins;
  int            alosses;
  int            security;
  int            bounty;
  int            wlck_vnum;
};

/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX		16

struct liq_type
{
  char *liq_name;
  char *liq_color;
  sh_int liq_affect[3];
};

/*
 * Extra description data for a room or object.
 */
struct extra_descr_data
{
  char *keyword;                /* Keyword in look/examine          */
  char *description;            /* What to see                      */
  char *buffer1;                /* special action buffer 1          */
  char *buffer2;                /* special action buffer 2          */
  sh_int type;                  /* activation type                  */
  sh_int vnum;                  /* action related vnum              */
  sh_int action;                /* action type                      */
};

/*
 * Prototype for an object.
 */
struct obj_index_data
{
  LIST                * extra_descr;
  LIST                * affected;
  AREA_DATA           * area;
  char                * name;
  char                * short_descr;
  char                * description;
  sh_int                item_type;
  sh_int                count;
  sh_int                weight;
  int                   vnum;
  int                   extra_flags;
  int                   wear_flags;
  int                   cost;
  int                   value[4];
};

/*
 * One object.
 */
struct obj_data
{
  LIST                *  contains;
  OBJ_DATA            *  in_obj;
  LIST                *  events; 
  CHAR_DATA           *  carried_by;
  LIST                *  extra_descr;
  LIST                *  affected;
  OBJ_INDEX_DATA      *  pIndexData;
  ROOM_INDEX_DATA     *  in_room;
  char                *  name;
  char                *  short_descr;
  char                *  description;
  char                *  questowner;
  sh_int                 sentient_points;
  sh_int                 love;
  int                    spellflags;
  int                    item_type;
  int                    ownerid;
  int                    extra_flags;
  int                    wear_flags;
  int                    wear_loc;
  int                    weight;
  int                    condition;
  int                    toughness;
  int                    resistance;
  int                    quest;
  int                    cost;
  int                    level;
  int                    value[4];
};

/*
 * Exit data.
 */
struct exit_data
{
  EXIT_DATA        * rexit;         /* Reverse exit pointer          */
  ROOM_INDEX_DATA  * to_room;       /* Pointer to destination room   */
  char             * keyword;       /* Keywords for exit or door     */
  char             * description;   /* Description of exit           */
  int                vnum;          /* Vnum of room exit leads to    */
  int                exit_info;     /* door states & other flags     */
  int                key;           /* Key vnum                      */
  sh_int             vdir;          /* 0,5 N\E\S\W\U\D shit          */
  int                rs_flags;      /* OLC                           */
  bool               color;         /* pathfinding                   */
};

/*
 * Room text checking data.
 */
typedef struct roomtext_data
{
  int type;
  int power;
  int mob;
  char *input;
  char *output;
  char *choutput;
  char *name;
} ROOMTEXT_DATA;

/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile 
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct reset_data
{
  bool repop;
  char command;
  int arg1;
  int arg2;
  int arg3;
};

/*
 * Area definition.
 */
struct area_data
{
  LIST             *  affects;
  LIST             *  events;
  char             *  name;
  char             *  filename;
  char             *  builders;
  char             *  music;
  int                 security;
  int                 lvnum;
  int                 uvnum;
  int                 vnum;
  int                 cvnum;
  int                 area_flags;
  int                 areabits;
};

/*
 * Room type.
 */
struct room_index_data
{
  LIST              * events;
  LIST              * people;
  LIST              * contents;
  LIST              * extra_descr;
  AREA_DATA         * area;
  EXIT_DATA         * exit[6];
  LIST              * roomtext;
  LIST              * resets;
  char              * name;
  char              * description;
  int                 vnum;
  int                 room_flags;
  sh_int              light;
  sh_int              blood;
  sh_int              sector_type;
  sh_int              heap_index;
  sh_int              steps;
  bool                visited;
};

/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000

/*
 *  Target types.
 */
#define TAR_IGNORE		    0
#define TAR_CHAR_OFFENSIVE	    1
#define TAR_CHAR_DEFENSIVE	    2
#define TAR_CHAR_SELF		    3
#define TAR_OBJ_INV		    4

#define PURPLE_MAGIC		    0
#define RED_MAGIC		    1
#define BLUE_MAGIC		    2
#define GREEN_MAGIC		    3
#define YELLOW_MAGIC		    4

/*
 * Skills include spells as a particular case.
 */
struct skill_type
{
  char       * name;                 /* Name of skill               */
  sh_int       skill_level;          /* Level needed by class       */
  SPELL_FUN  * spell_fun;            /* Spell pointer (for spells)  */
  sh_int       target;               /* Legal targets               */
  sh_int       minimum_position;     /* Position for caster / user  */
  sh_int     * pgsn;                 /* Pointer to associated gsn   */
  sh_int       min_mana;             /* Minimum mana used           */
  sh_int       beats;                /* Waiting time after use      */
  char       * noun_damage;          /* Damage message              */
  char       * msg_off;              /* Wear off message            */
  char       * msg_off_others;       /* Wear off message for others */
  int          race;
  sh_int       powertype;
  sh_int       powerlevel;
  sh_int       classtype; 
};

/*
 * These are skill_lookup return values for common skills and spells.
 */
extern sh_int gsn_heroism;
extern sh_int gsn_tackle;
extern sh_int gsn_omniscience;
extern sh_int gsn_endurance;
extern sh_int gsn_brilliance;
extern sh_int gsn_nimbleness;
extern sh_int gsn_chaosblast;
extern sh_int gsn_firebreath;
extern sh_int gsn_gasblast;
extern sh_int gsn_magicmissile;
extern sh_int gsn_groupheal;
extern sh_int gsn_impfireball;
extern sh_int gsn_energydrain;
extern sh_int gsn_earthquake;
extern sh_int gsn_blindness;
extern sh_int gsn_knee;
extern sh_int gsn_sweep;
extern sh_int gsn_charm_person;
extern sh_int gsn_curse;
extern sh_int gsn_invis;
extern sh_int gsn_mass_invis;
extern sh_int gsn_poison;
extern sh_int gsn_backstab;
extern sh_int gsn_garotte;
extern sh_int gsn_disarm;
extern sh_int gsn_hurl;
extern sh_int gsn_kick;
extern sh_int gsn_circle;
extern sh_int gsn_peek;
extern sh_int gsn_pick_lock;
extern sh_int gsn_rescue;
extern sh_int gsn_sneak;
extern sh_int gsn_steal;
extern sh_int gsn_punch;
extern sh_int gsn_elbow;
extern sh_int gsn_headbutt;
extern sh_int gsn_berserk;
extern sh_int gsn_track;
extern sh_int gsn_fireball;
extern sh_int gsn_lightning;
extern sh_int gsn_nova;
extern sh_int gsn_paradox;
extern sh_int gsn_ballista;

/* shadow attacks */
extern sh_int gsn_knifespin;
extern sh_int gsn_wakasashislice;
extern sh_int gsn_frostbite;
extern sh_int gsn_whirl;
extern sh_int gsn_caltrops;
extern sh_int gsn_soulreaper;
extern sh_int gsn_moonstrike;
extern sh_int gsn_shadowthrust;
extern sh_int gsn_gutcutter;
extern sh_int gsn_dirtthrow;
extern sh_int gsn_soulseeker;
extern sh_int gsn_spiritsoaring;
extern sh_int gsn_hthrust;
extern sh_int gsn_dullcut;

/* warlock attacks */
extern sh_int gsn_doombolt;
extern sh_int gsn_flamberge;
extern sh_int gsn_huntingstars;
extern sh_int gsn_fireshield;
extern sh_int gsn_deathspell;
extern sh_int gsn_chillbolt;
extern sh_int gsn_meteor;
extern sh_int gsn_flamestorm;
extern sh_int gsn_chantspell;

/* fae attacks */
extern sh_int gsn_telekinetic;
extern sh_int gsn_matter;
extern sh_int gsn_plasma;
extern sh_int gsn_spiritkiss;

/* Giant attacks */
extern sh_int gsn_bash;
extern sh_int gsn_crush;
extern sh_int gsn_smack;
extern sh_int gsn_thwack;
extern sh_int gsn_backfist;
extern sh_int gsn_lavaburst;
extern sh_int gsn_spikes;

/*
 * Utility macros.
 */
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)	((var) ^= (bit))


/* Ensure coord is on the map */
#define BOUNDARY(x, y) (((x) < 0) || ((y) < 0) || ((x) > MAPX) || ((y) > MAPY))

/*
 * Character macros.
 */
#define IS_CREATOR(ch)          (get_trust(ch) >= MAX_LEVEL)
#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC))
#define IS_JUDGE(ch)		(get_trust(ch) >= LEVEL_JUDGE)
#define IS_IMMORTAL(ch)		(get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust(ch) >= LEVEL_AVATAR)
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define IS_ITEMAFF(ch, sn)	(IS_SET((ch)->itemaffect, (sn)))
#define IS_IMMUNE(ch, sn)	(IS_SET((ch)->immune, (sn)))
#define IS_FORM(ch, sn)		(IS_SET((ch)->form, (sn)))
#define IS_EXTRA(ch, sn)	(IS_SET((ch)->extra, (sn)))
#define IS_STANCE(ch, sn)	(ch->stance[0] == sn)
#define IS_CLASS(ch, CLASS)	(IS_SET((ch)->class, CLASS) && (ch->level >= LEVEL_AVATAR))
#define IS_HEAD(ch, sn)		(IS_SET((ch)->loc_hp[0], (sn)))
#define IS_BODY(ch, sn)		(IS_SET((ch)->loc_hp[1], (sn)))
#define IS_ARM_L(ch, sn)	(IS_SET((ch)->loc_hp[2], (sn)))
#define IS_ARM_R(ch, sn)	(IS_SET((ch)->loc_hp[3], (sn)))
#define IS_LEG_L(ch, sn)	(IS_SET((ch)->loc_hp[4], (sn)))
#define IS_LEG_R(ch, sn)	(IS_SET((ch)->loc_hp[5], (sn)))
#define IS_BLEEDING(ch, sn)	(IS_SET((ch)->loc_hp[6], (sn)))
#define IS_GOOD(ch)		(ch->alignment >= 100)
#define IS_EVIL(ch)		(ch->alignment <= -100)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))
#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)
#define IS_OUTSIDE(ch)		(!IS_SET((ch)->in_room->room_flags, ROOM_INDOORS))
#define HOSTNAME(d)             ((d->bResolved) ? d->hostname : d->ip_address)

#define WAIT_STATE(ch, npulse) {                               \
  if (!IS_CREATOR((ch))) {                                     \
    if (IS_AFFECTED((ch), AFF_MINDBLANK) && (npulse) > 4)      \
      (ch)->wait = UMAX((ch)->wait, (npulse) + 2);             \
    else if (IS_AFFECTED((ch), AFF_MINDBOOST) && (npulse) > 4) \
      (ch)->wait = UMAX((ch)->wait, (npulse) - 2);             \
    else                                                       \
      (ch)->wait = UMAX((ch)->wait, (npulse));                 \
  }                                                            \
}

#define QUICKITER(pList)                \
QuickIter->_pList = pList;              \
QuickIter->_pCell = pList->_pFirstCell

#define QUICKITER2(pList)                \
QuickIter2->_pList = pList;              \
QuickIter2->_pCell = pList->_pFirstCell

/*
 * Object Macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))

/*
 * Description macros.
 */
#define PERS(ch, looker)	( can_see( looker, (ch) ) ?		\
				( IS_NPC(ch) ? (ch)->short_descr	\
                                : ( IS_AFFECTED( (ch), AFF_POLYMORPH) ? \
				(ch)->morph : (ch)->name ) )		\
				: "someone" )

/*
 * This structure is used in bit.c to lookup flags and stats.
 */
struct flag_type
{
  char *name;
  int bit;
  bool settable;
};

/* tables for J.O.P.E. */
extern const struct flag_type extra_table[];
extern const struct flag_type act_table[];
extern const struct flag_type jflags_table[];
extern const struct flag_type newbits_table[];


/* special table for note flags */
extern const struct flag_type note_flags[];


/*
 * Structure for a command in the command lookup table.
 */
struct cmd_type
{
  char   * const name;
  DO_FUN * do_fun;
  sh_int   position;
  sh_int   level;
  sh_int   log;
  int      race;
  sh_int   powertype;
  sh_int   powerlevel;
  sh_int   classtype;
};

/*
 * Structure for a social in the socials table.
 */
struct social_type
{
  char * name;
  char * char_no_arg;
  char * others_no_arg;
  char * char_found;
  char * others_found;
  char * vict_found;
  char * char_auto;
  char * others_auto;
};

/*
 * Structure for an X-social in the socials table.
 */
struct xsocial_type
{
  char *const name;
  char *const char_no_arg;
  char *const others_no_arg;
  char *const char_found;
  char *const others_found;
  char *const vict_found;
  char *const char_auto;
  char *const others_auto;
  sh_int gender;
  sh_int stage;
  sh_int position;
  sh_int self;
  sh_int other;
  bool chance;
};

struct ignore_data
{
  char              * player;
  char              * account;
};

struct snoop_data
{
  DESCRIPTOR_DATA   * snooper;
  int                 flags;
};

struct alias_data
{
  char          * name;            /*  they trigger word              */
  char          * expand_string;   /*  the expanded, unparsed string  */
};

/*
 * Global constants.
 */
extern const struct cmd_type cmd_table[];
extern const struct liq_type liq_table[LIQ_MAX];
extern const struct skill_type skill_table[MAX_SKILL];
extern const struct xsocial_type xsocial_table[];

extern int maxSocial;
extern ITERATOR * QuickIter;
extern ITERATOR * QuickIter2;

/* artifact stuff */
extern LIST *artifact_table;
extern const struct arti_entry artifact_programs[];

/* social table */
extern struct social_type *social_table;  


/*
 * Global variables.
 */
extern   LIST              *  eventqueue[MAX_EVENT_HASH];
extern   STACK             *  account_free;
extern   STACK             *  feed_free;
extern   STACK             *  session_free;
extern   LIST              *  help_list;
extern   LIST              *  ban_list;
extern   STACK             *  ban_free;
extern   STACK             *  quest_free;
extern   LIST              *  newbieban_list;
extern   LIST              *  char_list;
extern   LIST              *  auction_list;
extern   STACK             *  auction_free;
extern   CHAR_DATA         *  sacrificer;
extern   LIST              *  top10_list;
extern   LIST              *  global_event_list;
extern   LIST              *  descriptor_list;
extern   LIST              *  kingdom_list;
extern   STACK             *  member_free;
extern   LIST              *  object_list;
extern   ITERATOR          *  iterator_list;
extern   STACK             *  history_free;
extern   ITERATOR          *  iterator_free;
extern   STACK             *  scell_free;
extern   STACK             *  affect_free;
extern   STACK             *  ignore_free;
extern   STACK             *  area_affect_free;
extern   STACK             *  dummy_free;
extern   LIST              *  dummy_list;
extern   LIST              *  poll_list;
extern   STACK             *  char_free;
extern   STACK             *  mob_index_free;
extern   STACK             *  obj_index_free;
extern   STACK             *  snoop_free;
extern   STACK             *  descriptor_free;
extern   STACK             *  extra_descr_free;
extern   STACK             *  note_free;
extern   STACK             *  obj_free;
extern   STACK             *  reset_free;
extern   STACK             *  exit_free;
extern   STACK             *  pcdata_free;
extern   LIST              *  change_list;
extern   STACK             *  change_free;
extern   STACK             *  alias_free;
extern   STACK             *  spell_free;
extern   time_t               current_time;
extern   bool                 fLogAll;
extern   KILL_DATA            kill_table[];
extern   TIME_INFO_DATA       time_info;
extern   WEATHER_DATA         weather_info;
extern   bool                 NoMasteryExtract;
extern   bool                 cmd_done;
extern   int                  thread_count;
extern   int                  iDelete;
extern   int                  auction_id;
extern   bool                 ragnarok;
extern   bool                 new_ragnarok;
extern   int                  ragnarok_cost;

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN( do_areaaff		);
DECLARE_DO_FUN( do_pstat		);
DECLARE_DO_FUN( do_sedit		);
DECLARE_DO_FUN( do_history		);
DECLARE_DO_FUN( do_showsilence		);
DECLARE_DO_FUN( do_pathfind		);
DECLARE_DO_FUN( do_showcompress		);
DECLARE_DO_FUN( do_ccenter		);
DECLARE_DO_FUN( do_ragnarok		);
DECLARE_DO_FUN( do_omni			);
DECLARE_DO_FUN( do_afk			);
DECLARE_DO_FUN( do_configure		);
DECLARE_DO_FUN( do_enhcombat		);
DECLARE_DO_FUN( do_oldprompt		);
DECLARE_DO_FUN( do_sound		);
DECLARE_DO_FUN( do_noset		);
DECLARE_DO_FUN( do_timer		);

/* OLC */
DECLARE_DO_FUN( do_hlist		);
DECLARE_DO_FUN( do_aedit 	       );
DECLARE_DO_FUN( do_redit        	);
DECLARE_DO_FUN( do_oedit        	);
DECLARE_DO_FUN( do_medit        	);
DECLARE_DO_FUN( do_asave        	);
DECLARE_DO_FUN( do_alist        	);
DECLARE_DO_FUN( do_resets       	);

/* More commands */
DECLARE_DO_FUN( do_accept		);
DECLARE_DO_FUN( do_alignment		);
DECLARE_DO_FUN( do_showquest		);
DECLARE_DO_FUN( do_qgain		);
DECLARE_DO_FUN( do_qcomplete		);
DECLARE_DO_FUN( do_allow		);
DECLARE_DO_FUN( do_bounty		);
DECLARE_DO_FUN( do_bountylist		);
DECLARE_DO_FUN( do_ansi			);
DECLARE_DO_FUN( do_areas		);
DECLARE_DO_FUN( do_artifact		);
DECLARE_DO_FUN( do_artiwiz		);
DECLARE_DO_FUN( do_at			);
DECLARE_DO_FUN( do_autoexit		);
DECLARE_DO_FUN( do_autoloot		);
DECLARE_DO_FUN( do_autosac		);
DECLARE_DO_FUN( do_autohead		);
DECLARE_DO_FUN( do_autostance		);
DECLARE_DO_FUN( do_backstab		);
DECLARE_DO_FUN( do_bamfin		);
DECLARE_DO_FUN( do_bamfout		);
DECLARE_DO_FUN( do_ban			);
DECLARE_DO_FUN( do_berserk		);
DECLARE_DO_FUN( do_birth		);
DECLARE_DO_FUN( do_blindfold		);
DECLARE_DO_FUN( do_brandish		);
DECLARE_DO_FUN( do_breakup		);
DECLARE_DO_FUN( do_automap		);
DECLARE_DO_FUN( do_brief		);
DECLARE_DO_FUN( do_brief1		);
DECLARE_DO_FUN( do_brief2		);
DECLARE_DO_FUN( do_brief3		);
DECLARE_DO_FUN( do_brief4		);
DECLARE_DO_FUN( do_brief5		);
DECLARE_DO_FUN( do_brief6		);
DECLARE_DO_FUN( do_brief7               );
DECLARE_DO_FUN( do_call			);
DECLARE_DO_FUN( do_cast			);
DECLARE_DO_FUN( do_changelight		);
DECLARE_DO_FUN( do_channels		);
DECLARE_DO_FUN( do_compress		);
DECLARE_DO_FUN( do_compres		);
DECLARE_DO_FUN( do_chat			);
DECLARE_DO_FUN( do_claim		);
DECLARE_DO_FUN( do_unclaim		);
DECLARE_DO_FUN( do_flame		);
DECLARE_DO_FUN( do_fortressstats	);
DECLARE_DO_FUN( do_arenastats		);
DECLARE_DO_FUN( do_spectate		);
DECLARE_DO_FUN( do_arenajoin		);
DECLARE_DO_FUN( do_teamjoin		);
DECLARE_DO_FUN( do_resign		);
DECLARE_DO_FUN( do_challenge		);
DECLARE_DO_FUN( do_decline		);
DECLARE_DO_FUN( do_donate		);
DECLARE_DO_FUN( do_accept2		);
DECLARE_DO_FUN( do_newban		);
DECLARE_DO_FUN( do_newallow		);
DECLARE_DO_FUN( do_addchange		);
DECLARE_DO_FUN( do_changes		);
DECLARE_DO_FUN( do_delchange		);
DECLARE_DO_FUN( do_displayvotes		);
DECLARE_DO_FUN( do_vote			);
DECLARE_DO_FUN( do_pshift		);
DECLARE_DO_FUN( do_polledit		);
DECLARE_DO_FUN( do_addpoll		);
DECLARE_DO_FUN( do_stoppoll		);
DECLARE_DO_FUN( do_leader		);
DECLARE_DO_FUN( do_leaderclear		);
DECLARE_DO_FUN( do_top10		);
DECLARE_DO_FUN( do_kingdoms		);
DECLARE_DO_FUN( do_kwho			);
DECLARE_DO_FUN( do_kbuy			);
DECLARE_DO_FUN( do_kballista		);
DECLARE_DO_FUN( do_kreload		);
DECLARE_DO_FUN( do_kmix			);
DECLARE_DO_FUN( do_kdecline		);
DECLARE_DO_FUN( do_kuninvite		);
DECLARE_DO_FUN( do_koutcast		);
DECLARE_DO_FUN( do_kinvite		);
DECLARE_DO_FUN( do_kleave		);
DECLARE_DO_FUN( do_kinfo		);
DECLARE_DO_FUN( do_klist		);
DECLARE_DO_FUN( do_kquest		);
DECLARE_DO_FUN( do_kdonate		);
DECLARE_DO_FUN( do_kset			);
DECLARE_DO_FUN( do_ktalk		);
DECLARE_DO_FUN( do_kjoin		);
DECLARE_DO_FUN( do_disenchant		);
DECLARE_DO_FUN( do_clearstats		);
DECLARE_DO_FUN( do_close		);
DECLARE_DO_FUN( do_commands		);
DECLARE_DO_FUN( do_config		);
DECLARE_DO_FUN( do_consent		);
DECLARE_DO_FUN( do_consider		);
DECLARE_DO_FUN( do_cprompt		);
DECLARE_DO_FUN( do_crack		);
DECLARE_DO_FUN( do_create		);
DECLARE_DO_FUN( do_credits		);
DECLARE_DO_FUN( do_dcredits		);
DECLARE_DO_FUN( do_decapitate		);
DECLARE_DO_FUN( do_tackle		);
DECLARE_DO_FUN( do_deny			);
DECLARE_DO_FUN( do_description		);
DECLARE_DO_FUN( do_diagnose		);
DECLARE_DO_FUN( do_dismount		);
DECLARE_DO_FUN( do_disable		);
DECLARE_DO_FUN( do_disarm		);
DECLARE_DO_FUN( do_disconnect		);
DECLARE_DO_FUN( do_divorce		);
DECLARE_DO_FUN( do_down			);
DECLARE_DO_FUN( do_draw			);
DECLARE_DO_FUN( do_drink		);
DECLARE_DO_FUN( do_drop			);
DECLARE_DO_FUN( do_settie		);
DECLARE_DO_FUN( do_setlogout		);
DECLARE_DO_FUN( do_setlogin		);
DECLARE_DO_FUN( do_setdecap		);
DECLARE_DO_FUN( do_setavatar		);
DECLARE_DO_FUN( do_evolve		);
DECLARE_DO_FUN( do_east			);
DECLARE_DO_FUN( do_eat			);
DECLARE_DO_FUN( do_echo			);
DECLARE_DO_FUN( do_empty		);
DECLARE_DO_FUN( do_escape		);
DECLARE_DO_FUN( do_emote		);
DECLARE_DO_FUN( do_enter		);
DECLARE_DO_FUN( do_equipment		);
DECLARE_DO_FUN( do_examine		);
DECLARE_DO_FUN( do_exits		);
DECLARE_DO_FUN( do_exlist		);
DECLARE_DO_FUN( do_favor		);
DECLARE_DO_FUN( do_fill			);
DECLARE_DO_FUN( do_finger		);
DECLARE_DO_FUN( do_flee			);
DECLARE_DO_FUN( do_flex			);
DECLARE_DO_FUN( do_follow		);
DECLARE_DO_FUN( do_force		);
DECLARE_DO_FUN( do_asperson		);
DECLARE_DO_FUN( do_forceauto		);
DECLARE_DO_FUN( do_freeze		);
DECLARE_DO_FUN( do_gag			);
DECLARE_DO_FUN( do_get			);
DECLARE_DO_FUN( do_generation		);
DECLARE_DO_FUN( do_gift			);
DECLARE_DO_FUN( do_give			);
DECLARE_DO_FUN( do_goto			);
DECLARE_DO_FUN( do_group		);
DECLARE_DO_FUN( do_gtell		);
DECLARE_DO_FUN( do_help			);
DECLARE_DO_FUN( do_search		);
DECLARE_DO_FUN( do_holylight		);
DECLARE_DO_FUN( do_home			);
DECLARE_DO_FUN( do_huh			);
DECLARE_DO_FUN( do_hurl			);
DECLARE_DO_FUN( do_idea			);
DECLARE_DO_FUN( do_immune		);
DECLARE_DO_FUN( do_immtalk		);
DECLARE_DO_FUN( do_info			);
DECLARE_DO_FUN( do_inventory		);
DECLARE_DO_FUN( do_rarelist		);
DECLARE_DO_FUN( do_invis		);
DECLARE_DO_FUN( do_kick			);
DECLARE_DO_FUN( do_kill			);
DECLARE_DO_FUN( do_combatswitch		);
DECLARE_DO_FUN( do_locate		);
DECLARE_DO_FUN( do_lock			);
DECLARE_DO_FUN( do_log			);
DECLARE_DO_FUN( do_logstat		);
DECLARE_DO_FUN( do_mudinfo		);
DECLARE_DO_FUN( do_wizallow		);
DECLARE_DO_FUN( do_logstatclear		);
DECLARE_DO_FUN( do_look			);
DECLARE_DO_FUN( do_marry		);
DECLARE_DO_FUN( do_memory		);
DECLARE_DO_FUN( do_mfind		);
DECLARE_DO_FUN( do_mload		);
DECLARE_DO_FUN( do_mount		);
DECLARE_DO_FUN( do_mset			);
DECLARE_DO_FUN( do_kedit		);
DECLARE_DO_FUN( do_undeny		);
DECLARE_DO_FUN( do_mstat		);
DECLARE_DO_FUN( do_mwhere		);
DECLARE_DO_FUN( do_music		);
DECLARE_DO_FUN( do_sing			);
DECLARE_DO_FUN( do_north		);
DECLARE_DO_FUN( do_note			);
DECLARE_DO_FUN( do_board                );
DECLARE_DO_FUN( do_oclone		);
DECLARE_DO_FUN( do_ofind		);
DECLARE_DO_FUN( do_oload		);
DECLARE_DO_FUN( do_open			);
DECLARE_DO_FUN( do_order		);
DECLARE_DO_FUN( do_oset			);
DECLARE_DO_FUN( do_ostat		);
DECLARE_DO_FUN( do_otransfer		);
DECLARE_DO_FUN( do_paradox		);
DECLARE_DO_FUN( do_peace		);
DECLARE_DO_FUN( do_pick			);
DECLARE_DO_FUN( do_pload		);
DECLARE_DO_FUN( do_practice		);
DECLARE_DO_FUN( do_prompt		);
DECLARE_DO_FUN( do_propose		);
DECLARE_DO_FUN( do_pset			);
DECLARE_DO_FUN( do_punch		);
DECLARE_DO_FUN( do_purge		);
DECLARE_DO_FUN( do_put			);
DECLARE_DO_FUN( do_quaff		);
DECLARE_DO_FUN( do_shop			);
DECLARE_DO_FUN( do_pull			);
DECLARE_DO_FUN( do_press		);
DECLARE_DO_FUN( do_touch		);
DECLARE_DO_FUN( do_push			);
DECLARE_DO_FUN( do_list			);
DECLARE_DO_FUN( do_buy			);
DECLARE_DO_FUN( do_auction              );
DECLARE_DO_FUN( do_qui			);
DECLARE_DO_FUN( do_quit			);
DECLARE_DO_FUN( do_policy		);
DECLARE_DO_FUN( do_ignore		);
DECLARE_DO_FUN( do_copyover		);
DECLARE_DO_FUN( do_recall		);
DECLARE_DO_FUN( do_recho		);
DECLARE_DO_FUN( do_recite		);
DECLARE_DO_FUN( do_relevel		);
DECLARE_DO_FUN( do_remove		);
DECLARE_DO_FUN( do_reply		);
DECLARE_DO_FUN( do_report		);
DECLARE_DO_FUN( do_rescue		);
DECLARE_DO_FUN( do_rest			);
DECLARE_DO_FUN( do_restore		);
DECLARE_DO_FUN( do_return		);
DECLARE_DO_FUN( do_rset			);
DECLARE_DO_FUN( do_rstat		);
DECLARE_DO_FUN( do_sacrifice		);
DECLARE_DO_FUN( do_safe			);
DECLARE_DO_FUN( do_save			);
DECLARE_DO_FUN( do_say			);
DECLARE_DO_FUN( do_racecommands		);
DECLARE_DO_FUN( do_scan			);
DECLARE_DO_FUN( do_stat			);
DECLARE_DO_FUN( do_score		);
DECLARE_DO_FUN( do_session		);
DECLARE_DO_FUN( do_level		);
DECLARE_DO_FUN( do_mastery		);
DECLARE_DO_FUN( do_gensteal		);

/* More commands */
DECLARE_DO_FUN( do_alias                );
DECLARE_DO_FUN( do_delalias             );
DECLARE_DO_FUN( do_sheath		);
DECLARE_DO_FUN( do_shutdow		);
DECLARE_DO_FUN( do_shutdown		);
DECLARE_DO_FUN( do_silence		);
DECLARE_DO_FUN( do_sit			);
DECLARE_DO_FUN( do_skill		);
DECLARE_DO_FUN( do_sleep		);
DECLARE_DO_FUN( do_slookup		);
DECLARE_DO_FUN( do_spell		);
DECLARE_DO_FUN( do_stance		);
DECLARE_DO_FUN( do_smother		);
DECLARE_DO_FUN( do_sneak		);
DECLARE_DO_FUN( do_snoop		);
DECLARE_DO_FUN( do_plist		);
DECLARE_DO_FUN( do_socials		);
DECLARE_DO_FUN( do_south		);
DECLARE_DO_FUN( do_spy			);
DECLARE_DO_FUN( do_spydirection		);
DECLARE_DO_FUN( do_sset			);
DECLARE_DO_FUN( do_stand		);
DECLARE_DO_FUN( do_steal		);
DECLARE_DO_FUN( do_summon		);
DECLARE_DO_FUN( do_nosummon		);
DECLARE_DO_FUN( do_switch		);
DECLARE_DO_FUN( do_tell			);
DECLARE_DO_FUN( do_throw		);
DECLARE_DO_FUN( do_tie			);
DECLARE_DO_FUN( do_time			);
DECLARE_DO_FUN( do_title		);
DECLARE_DO_FUN( do_withdraw		);
DECLARE_DO_FUN( do_deposit		);
DECLARE_DO_FUN( do_track		);
DECLARE_DO_FUN( do_train		);
DECLARE_DO_FUN( do_pkready		);
DECLARE_DO_FUN( do_transfer		);
DECLARE_DO_FUN( do_transport		);
DECLARE_DO_FUN( do_truesight		);
DECLARE_DO_FUN( do_trust		);
DECLARE_DO_FUN( do_typo			);
DECLARE_DO_FUN( do_unlock		);
DECLARE_DO_FUN( do_untie		);
DECLARE_DO_FUN( do_up			);
DECLARE_DO_FUN( do_upkeep		);
DECLARE_DO_FUN( do_users		);
DECLARE_DO_FUN( do_version		);
DECLARE_DO_FUN( do_visible		);
DECLARE_DO_FUN( do_wake			);
DECLARE_DO_FUN( do_wear			);
DECLARE_DO_FUN( do_west			);
DECLARE_DO_FUN( do_where		);
DECLARE_DO_FUN( do_whisper		);
DECLARE_DO_FUN( do_who			);
DECLARE_DO_FUN( do_wizhelp		);
DECLARE_DO_FUN( do_linkdead		);
DECLARE_DO_FUN( do_wizlist		);
DECLARE_DO_FUN( do_reimb		);
DECLARE_DO_FUN( do_wizlock		);
DECLARE_DO_FUN( do_xemote		);
DECLARE_DO_FUN( do_xsocials		);
DECLARE_DO_FUN( do_yell			);
DECLARE_DO_FUN( do_zap			);
DECLARE_DO_FUN( do_relearn		);
DECLARE_DO_FUN( do_scry                 );
DECLARE_DO_FUN( do_shield               );

/* the fae class */
DECLARE_DO_FUN( do_faetalk		);
DECLARE_DO_FUN( do_acidheart		);
DECLARE_DO_FUN( do_energize		);
DECLARE_DO_FUN( do_halo			);
DECLARE_DO_FUN( do_freezeancients	);
DECLARE_DO_FUN( do_hspirits		);
DECLARE_DO_FUN( do_pwall		);
DECLARE_DO_FUN( do_pspray		);
DECLARE_DO_FUN( do_bloodimmune		);
DECLARE_DO_FUN( do_bloodacid		);
DECLARE_DO_FUN( do_pchain		);
DECLARE_DO_FUN( do_bloodsacrifice	);
DECLARE_DO_FUN( do_warding		);
DECLARE_DO_FUN( do_chameleon		);
DECLARE_DO_FUN( do_faepipes		);
DECLARE_DO_FUN( do_dragonbreath		);
DECLARE_DO_FUN( do_ghostgauntlets	);
DECLARE_DO_FUN( do_nibbleeye		);
DECLARE_DO_FUN( do_watchfuleye		);
DECLARE_DO_FUN( do_spiritkiss		);
DECLARE_DO_FUN( do_unleashed		);
DECLARE_DO_FUN( do_gaseous		);
DECLARE_DO_FUN( do_bloodhunger		);
DECLARE_DO_FUN( do_bloodtrack		);
DECLARE_DO_FUN( do_chaossigil		);
DECLARE_DO_FUN( do_phantom		);
DECLARE_DO_FUN( do_martyr		);
DECLARE_DO_FUN( do_geyser		);
DECLARE_DO_FUN( do_faeblast		);
DECLARE_DO_FUN( do_faetune		);
DECLARE_DO_FUN( do_blastbeams		);
DECLARE_DO_FUN( do_disctrain		);
DECLARE_DO_FUN( do_spidercall		);
DECLARE_DO_FUN( do_timewarp		);
DECLARE_DO_FUN( do_reform		);
DECLARE_DO_FUN( do_infuse		);
DECLARE_DO_FUN( do_channel		);
DECLARE_DO_FUN( do_will			);
DECLARE_DO_FUN( do_energy		);
DECLARE_DO_FUN( do_matter		);
DECLARE_DO_FUN( do_plasma		);
DECLARE_DO_FUN( do_elementalform	);
DECLARE_DO_FUN( do_ancients		);

/* the warlock class */
DECLARE_DO_FUN( do_winit		);
DECLARE_DO_FUN( do_wcancel		);
DECLARE_DO_FUN( do_wchain               );
DECLARE_DO_FUN( do_wpower		);
DECLARE_DO_FUN( do_wcast                );
DECLARE_DO_FUN( do_wglimmer		);
DECLARE_DO_FUN( do_wtarget              );
DECLARE_DO_FUN( do_wexclude             );
DECLARE_DO_FUN( do_wtype                );
DECLARE_DO_FUN( do_wfocus               );
DECLARE_DO_FUN( do_warlocktalk          );
DECLARE_DO_FUN( do_disjunction		);
DECLARE_DO_FUN( do_precognition		);
DECLARE_DO_FUN( do_stitches		);
DECLARE_DO_FUN( do_backlash		);
DECLARE_DO_FUN( do_hstars		);
DECLARE_DO_FUN( do_mking                );
DECLARE_DO_FUN( do_doombolt		);
DECLARE_DO_FUN( do_mummyrot		);
DECLARE_DO_FUN( do_doomcharge		);
DECLARE_DO_FUN( do_doomstorm		);
DECLARE_DO_FUN( do_sunset		);
DECLARE_DO_FUN( do_timetrip		);
DECLARE_DO_FUN( do_lockroom             );
DECLARE_DO_FUN( do_foldspace            );
DECLARE_DO_FUN( do_homingdevice		);
DECLARE_DO_FUN( do_contingency		);
DECLARE_DO_FUN( do_meditate		);
DECLARE_DO_FUN( do_archmages		);
DECLARE_DO_FUN( do_milkandhoney		);
DECLARE_DO_FUN( do_phylactery		);
DECLARE_DO_FUN( do_simulacrum		);
DECLARE_DO_FUN( do_ironmind		);
DECLARE_DO_FUN( do_eyespy		);
DECLARE_DO_FUN( do_implode		);
DECLARE_DO_FUN( do_beacon		);
DECLARE_DO_FUN( do_truthtell		);
DECLARE_DO_FUN( do_catalyst		);
DECLARE_DO_FUN( do_catalyze             );
DECLARE_DO_FUN( do_plague		);
DECLARE_DO_FUN( do_paradisebirds	);
DECLARE_DO_FUN( do_displacement		);
DECLARE_DO_FUN( do_earthmother		);
DECLARE_DO_FUN( do_magicvest		);
DECLARE_DO_FUN( do_pvipers		);
DECLARE_DO_FUN( do_flamestorm		);
DECLARE_DO_FUN( do_meteorswarm		);
DECLARE_DO_FUN( do_meteorstrike		);
DECLARE_DO_FUN( do_wallofswords		);
DECLARE_DO_FUN( do_fireshield		);
DECLARE_DO_FUN( do_shattershield	);
DECLARE_DO_FUN( do_lifedrain		);
DECLARE_DO_FUN( do_tarukeye		);
DECLARE_DO_FUN( do_chillbolt		);
DECLARE_DO_FUN( do_deathspell		);
DECLARE_DO_FUN( do_steelfists		);
DECLARE_DO_FUN( do_corpsedrain          );
DECLARE_DO_FUN( do_flamberge            );
DECLARE_DO_FUN( do_pentagram            );
DECLARE_DO_FUN( do_study                );
DECLARE_DO_FUN( do_bindingvines         );
DECLARE_DO_FUN( do_crows                );
DECLARE_DO_FUN( do_leeches              );
DECLARE_DO_FUN( do_callwild             );
DECLARE_DO_FUN( do_warscry              );
DECLARE_DO_FUN( do_readaura		);

/* The Giant Class */
DECLARE_DO_FUN( do_waterflux		);
DECLARE_DO_FUN( do_liquify		);
DECLARE_DO_FUN( do_waterdome		);
DECLARE_DO_FUN( do_sinkhole		);
DECLARE_DO_FUN( do_earthflux		);
DECLARE_DO_FUN( do_grow         	);
DECLARE_DO_FUN( do_bash         	);
DECLARE_DO_FUN( do_smack        	);
DECLARE_DO_FUN( do_thwack       	);
DECLARE_DO_FUN( do_crush		);
DECLARE_DO_FUN( do_dawnstrength		);
DECLARE_DO_FUN( do_gsweep		);
DECLARE_DO_FUN( do_giantgift		);
DECLARE_DO_FUN( do_windwalk		);
DECLARE_DO_FUN( do_whirlwind		);
DECLARE_DO_FUN( do_battletrain  	);
DECLARE_DO_FUN( do_earthswallow 	);
DECLARE_DO_FUN( do_standfirm    	);
DECLARE_DO_FUN( do_earthpunch   	);
DECLARE_DO_FUN( do_revival      	);
DECLARE_DO_FUN( do_stoneshape   	);
DECLARE_DO_FUN( do_rumble       	);
DECLARE_DO_FUN( do_ignitemetal		);
DECLARE_DO_FUN( do_burnmetal		);
DECLARE_DO_FUN( do_mallet		);
DECLARE_DO_FUN( do_hastespell		);
DECLARE_DO_FUN( do_slowspell		);
DECLARE_DO_FUN( do_chopattack		);
DECLARE_DO_FUN( do_deathfrenzy  	);
DECLARE_DO_FUN( do_waterstream		);
DECLARE_DO_FUN( do_mudform		);
DECLARE_DO_FUN( do_heatmetal		);
DECLARE_DO_FUN( do_gustwind		);

/* the shadow class */
DECLARE_DO_FUN( do_spirits		);
DECLARE_DO_FUN( do_moonstrike           );
DECLARE_DO_FUN( do_shadowthrust         );
DECLARE_DO_FUN( do_razorpunch		);
DECLARE_DO_FUN( do_bloodtheft		);
DECLARE_DO_FUN( do_supkeep		);
DECLARE_DO_FUN( do_shadowplane		);
DECLARE_DO_FUN( do_planegrab		);
DECLARE_DO_FUN( do_callwitness		);
DECLARE_DO_FUN( do_planeshred		);
DECLARE_DO_FUN( do_planerift		);
DECLARE_DO_FUN( do_confusion		);
DECLARE_DO_FUN( do_mindblank		);
DECLARE_DO_FUN( do_mindboost		);
DECLARE_DO_FUN( do_fumeblast		);
DECLARE_DO_FUN( do_mirrorimage		);
DECLARE_DO_FUN( do_frostblast		);
DECLARE_DO_FUN( do_tendrils		);
DECLARE_DO_FUN( do_acidtendrils		);
DECLARE_DO_FUN( do_blurtendrils		);
DECLARE_DO_FUN( do_shadowveil		);
DECLARE_DO_FUN( do_shadowportal		);
DECLARE_DO_FUN( do_shadowedge		);
DECLARE_DO_FUN( do_bloodenhance		);
DECLARE_DO_FUN( do_dirtthrow            );
DECLARE_DO_FUN( do_gutcutter            );
DECLARE_DO_FUN( do_soulreaper           );
DECLARE_DO_FUN( do_knifespin            );
DECLARE_DO_FUN( do_wakasashislice       );
DECLARE_DO_FUN( do_whirl		);
DECLARE_DO_FUN( do_caltrops             );
DECLARE_DO_FUN( do_shadowlearn          );
DECLARE_DO_FUN( do_aurasight		);
DECLARE_DO_FUN( do_soulseek             );
DECLARE_DO_FUN( do_soultarget           );
DECLARE_DO_FUN( do_shadowtalk           );
DECLARE_DO_FUN( do_vanish               );
DECLARE_DO_FUN( do_assassinate          );
DECLARE_DO_FUN( do_dullcut              );
DECLARE_DO_FUN( do_hthrust              );

/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN( spell_null			);
DECLARE_SPELL_FUN( spell_acid_blast		);
DECLARE_SPELL_FUN( spell_armor			);
DECLARE_SPELL_FUN( spell_godbless		);
DECLARE_SPELL_FUN( spell_ghostgauntlets		);
DECLARE_SPELL_FUN( spell_bless			);
DECLARE_SPELL_FUN( spell_blindness		);
DECLARE_SPELL_FUN( spell_charm_person		);
DECLARE_SPELL_FUN( spell_chill_touch		);
DECLARE_SPELL_FUN( spell_cure_blindness		);
DECLARE_SPELL_FUN( spell_cure_poison		);
DECLARE_SPELL_FUN( spell_curse			);
DECLARE_SPELL_FUN( spell_detect_evil		);
DECLARE_SPELL_FUN( spell_detect_hidden		);
DECLARE_SPELL_FUN( spell_detect_invis		);
DECLARE_SPELL_FUN( spell_detect_magic		);
DECLARE_SPELL_FUN( spell_detect_poison		);
DECLARE_SPELL_FUN( spell_dispel_evil		);
DECLARE_SPELL_FUN( spell_counter_spell		);
DECLARE_SPELL_FUN( spell_mind_blank		);
DECLARE_SPELL_FUN( spell_mind_boost		);
DECLARE_SPELL_FUN( spell_dispel_magic		);
DECLARE_SPELL_FUN( spell_earthquake		);
DECLARE_SPELL_FUN( spell_enchant_weapon		);
DECLARE_SPELL_FUN( spell_energy_drain		);
DECLARE_SPELL_FUN( spell_faerie_fog		);
DECLARE_SPELL_FUN( spell_fireball		);
DECLARE_SPELL_FUN( spell_heroism		);
DECLARE_SPELL_FUN( spell_omniscience		);
DECLARE_SPELL_FUN( spell_endurance		);
DECLARE_SPELL_FUN( spell_brilliance		);
DECLARE_SPELL_FUN( spell_nimbleness		);
DECLARE_SPELL_FUN( spell_haste			);
DECLARE_SPELL_FUN( spell_slow			);
DECLARE_SPELL_FUN( spell_nerve_pinch		);
DECLARE_SPELL_FUN( spell_gas_blast		);
DECLARE_SPELL_FUN( spell_shadow_guard		);
DECLARE_SPELL_FUN( spell_planebind		);
DECLARE_SPELL_FUN( spell_steelfists		);
DECLARE_SPELL_FUN( spell_noxious_fumes		);
DECLARE_SPELL_FUN( spell_desanct		);
DECLARE_SPELL_FUN( spell_imp_heal		);
DECLARE_SPELL_FUN( spell_imp_fireball		);
DECLARE_SPELL_FUN( spell_imp_teleport		);
DECLARE_SPELL_FUN( spell_fly			);
DECLARE_SPELL_FUN( spell_giant_strength		);
DECLARE_SPELL_FUN( spell_harm			);
DECLARE_SPELL_FUN( spell_heal			);
DECLARE_SPELL_FUN( spell_newbie			);
DECLARE_SPELL_FUN( spell_group_heal		);
DECLARE_SPELL_FUN( spell_identify		);
DECLARE_SPELL_FUN( spell_readaura		);
DECLARE_SPELL_FUN( spell_infravision		);
DECLARE_SPELL_FUN( spell_invis			);
DECLARE_SPELL_FUN( spell_know_alignment		);
DECLARE_SPELL_FUN( spell_locate_object		);
DECLARE_SPELL_FUN( spell_magic_missile		);
DECLARE_SPELL_FUN( spell_spellguard		);
DECLARE_SPELL_FUN( spell_cantrip		);
DECLARE_SPELL_FUN( spell_rupture		);
DECLARE_SPELL_FUN( spell_golden_gate		);
DECLARE_SPELL_FUN( spell_spelltrap		);
DECLARE_SPELL_FUN( spell_mass_invis		);
DECLARE_SPELL_FUN( spell_pass_door		);
DECLARE_SPELL_FUN( spell_poison			);
DECLARE_SPELL_FUN( spell_protection		);
DECLARE_SPELL_FUN( spell_protection_vs_good	);
DECLARE_SPELL_FUN( spell_refresh		);
DECLARE_SPELL_FUN( spell_remove_curse		);
DECLARE_SPELL_FUN( spell_sanctuary		);
DECLARE_SPELL_FUN( spell_shield			);
DECLARE_SPELL_FUN( spell_permanency		);
DECLARE_SPELL_FUN( spell_stone_skin		);
DECLARE_SPELL_FUN( spell_summon			);
DECLARE_SPELL_FUN( spell_weaken			);
DECLARE_SPELL_FUN( spell_word_of_recall		);
DECLARE_SPELL_FUN( spell_fire_breath		);
DECLARE_SPELL_FUN( spell_soulblade		);
DECLARE_SPELL_FUN( spell_mana			);
DECLARE_SPELL_FUN( spell_frenzy			);
DECLARE_SPELL_FUN( spell_darkblessing		);
DECLARE_SPELL_FUN( spell_energyflux		);
DECLARE_SPELL_FUN( spell_transport		);
DECLARE_SPELL_FUN( spell_regenerate		);
DECLARE_SPELL_FUN( spell_clot			);
DECLARE_SPELL_FUN( spell_mend			);
DECLARE_SPELL_FUN( spell_spiritkiss		);
DECLARE_SPELL_FUN( spell_mount			);
DECLARE_SPELL_FUN( spell_repair			);
DECLARE_SPELL_FUN( spell_spellproof		);
DECLARE_SPELL_FUN( spell_chaos_blast		);
DECLARE_SPELL_FUN( spell_resistance		);

/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 * United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif

/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 */
#define PLAYER_DIR	"../accounts/"             /* Player files     */
#define END_MARKER      "END"                      /* All sorts of load functions */
#define EXE_FILE        "../src/Dystopia"
#define SOCIAL_FILE     "../txt/socials.txt"
#define COPYOVER_FILE   "copyover.data"
#define DISABLED_FILE   "../txt/disabled.txt"      /* disabled commands           */
#define AREA_LIST	"../txt/area.lst"          /* List of areas               */
#define BAN_LIST	"../txt/ban.txt"           /* ban                         */
#define NEWBIEBAN_LIST  "../txt/newban.txt"        /* Newbie ban                  */
#define BUG_FILE	"../txt/bugs.txt"          /* For 'bug' and bug( )        */
#define CRASH_TEMP_FILE "../txt/crashtmp.txt"      /* Need it for crash-recover   */

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define QF      QUEST_FUN
#define ED	EXIT_DATA

/* stack methods */
STACK     *AllocStack         ( void );
void       FreeStack          ( STACK *pStack );
void      *PopStack           ( STACK *pStack );
void       PushStack          ( void *pContent, STACK *pStack );
int        StackSize          ( STACK *pStack );
 
/* list methods */
LIST      *AllocList               ( void );
ITERATOR  *AllocIterator           ( LIST *pList );
ITERATOR  *AllocReverseIterator    ( LIST *pList );
ITERATOR  *AllocQuickIterator      ( void );  /* NEVER use this one */
void      *NextInList              ( ITERATOR *pIter );
void      *PeekNextInList          ( ITERATOR *pIter );
void      *FirstInList             ( LIST *pList );
void      *LastInList              ( LIST *pList );
void       ReleaseAllIterators     ( void );
void       AttachToList            ( void *pContent, LIST *pList );
void       AttachToEndOfList       ( void *pContent, LIST *pList );
void       AttachToListAfterItem   ( void *pContent, LIST *pList, void *pItem );
void       AttachToListBeforeItem  ( void *pContent, LIST *pList, void *pItem );
void       DetachFromList          ( void *pContent, LIST *pList );
void       DetachAtIterator        ( ITERATOR *pIter );
void       FreeList                ( LIST *pList );
int        SizeOfList              ( LIST *pList );

/* act_comm.c */
IGNORE_DATA *alloc_ignore        ( void );
void  free_ignore                ( IGNORE_DATA *ignore, CHAR_DATA *ch );
bool  check_ignore               ( CHAR_DATA *ch, CHAR_DATA *victim );
void  delete_last_line_in_note   ( CHAR_DATA *ch );
void  showchannel                ( CHAR_DATA * ch, int chan );
void  talk_channel               ( CHAR_DATA * ch, char *argument, int channel, int sub_channel, const char *verb );
void  update_history             ( CHAR_DATA * ch, CHAR_DATA * talker, char *buf, char *argument, int mode );
char *drunktalk                  ( char *argument );
void  add_follower               ( CHAR_DATA * ch, CHAR_DATA * master );
void  stop_follower              ( CHAR_DATA * ch, bool isDead );
void  die_follower               ( CHAR_DATA * ch );
bool  is_same_group              ( CHAR_DATA * ach, CHAR_DATA * bch );
void  room_text                  ( CHAR_DATA * ch, char *argument );
char *strlower                   ( char *ip );
bool  check_parse_name           ( char *name, bool player );
void  room_message               ( ROOM_INDEX_DATA * room, char *message );

/* act_info.c */
char *format_obj_to_char  ( OBJ_DATA * obj, CHAR_DATA * ch, bool fShort );
void  set_title           ( CHAR_DATA *ch, char *title );
void  show_list_to_char   ( LIST *list, CHAR_DATA *ch, bool fShort, bool fShowNothing );
int   char_hitroll        ( CHAR_DATA *ch );
int   char_damroll        ( CHAR_DATA *ch );
int   char_ac             ( CHAR_DATA *ch );
bool  check_blind         ( CHAR_DATA *ch );
BUFFER *get_leader        ( void );
BUFFER *get_who           ( CHAR_DATA *ch, char *argument );
BUFFER *get_finger        ( CHAR_DATA *ch, char *player );

/* act_move.c */
void  move_char           ( CHAR_DATA * ch, int door );
void  open_lift           ( CHAR_DATA * ch );
void  close_lift          ( CHAR_DATA * ch );
void  move_lift           ( CHAR_DATA * ch, int to_room );
void  move_door           ( CHAR_DATA * ch );
void  thru_door           ( CHAR_DATA * ch, int doorexit );
void  open_door           ( CHAR_DATA * ch, bool be_open );
bool  is_open             ( CHAR_DATA * ch );
bool  same_floor          ( CHAR_DATA * ch, int cmp_room );
void  check_hunt          ( CHAR_DATA * ch );
RID  *get_rand_room       ( void );
RID  *get_rand_room_area  ( AREA_DATA *area );


/* act_obj.c */
bool  is_ok_to_wear   ( CHAR_DATA * ch, bool wolf_ok, char *argument );
bool  remove_obj      ( CHAR_DATA * ch, int iWear, bool fReplace );
bool  wear_obj        ( CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace );
void  call_all        ( CHAR_DATA * ch );

/* act_wiz.c */
void  deposit              ( CHAR_DATA *ch, OBJ_DATA *obj );
void  logchan              ( char *argument );
void  copyover_recover     ( void );
void  restore_player       ( CHAR_DATA *victim );

bool  check_reconnect        ( DESCRIPTOR_DATA *d, char *name, bool fConn );
void  close_socket           ( DESCRIPTOR_DATA * dclose );
void  close_socket2          ( DESCRIPTOR_DATA * dclose, bool kickoff );
void  write_to_buffer        ( DESCRIPTOR_DATA * d, const char *txt, int length );
void  send_to_char           ( const char *txt, CHAR_DATA * ch );
void  act                    ( const char *format, CHAR_DATA * ch, const void *arg1, const void *arg2, int type );
void  act_brief              ( const char *format, CHAR_DATA * ch, const void *arg1, const void *arg2, int type, int iBrief );
void  printf_to_char         ( CHAR_DATA *ch, char *fmt, ... ) \
                               __attribute__ ((format(printf, 2, 3)));
bool  write_to_descriptor    ( DESCRIPTOR_DATA *d, char *txt, int length );
bool  write_to_descriptor_2  ( int desc, char *txt, int length );


/* prototypes from db.c */
void load_disabled          ( void );
void save_disabled          ( void );
void load_social_table     ( void );
void save_social_table     ( void );

/* db.c */
void  boot_db            ( bool fCopyOver );
CD   *create_mobile      ( MOB_INDEX_DATA * pMobIndex );
OD   *create_object      ( OBJ_INDEX_DATA * pObjIndex, int level );
void  clear_char         ( CHAR_DATA * ch );
void  clear_pcdata       ( PC_DATA *pcdata );
void  free_char          ( CHAR_DATA * ch );
char *get_extra_descr    ( char *name, LIST *list );
char *get_roomtext       ( const char *name, ROOMTEXT_DATA * rt );
MID  *get_mob_index      ( int vnum );
OID  *get_obj_index      ( int vnum );
RID  *get_room_index     ( int vnum );
char  fread_letter       ( FILE * fp );
int   fread_number       ( FILE * fp );
char *fread_string       ( FILE * fp );
void  fread_to_eol       ( FILE * fp );
char *fread_word         ( FILE * fp );
int   social_lookup      ( const char *name );
char *str_dup            ( const char *str );
void  free_string        ( char *pstr );
int   number_fuzzy       ( int number );
int   number_range       ( int from, int to );
int   number_percent     ( void );
int   number_door        ( void );
int   number_bits        ( int width );
int   number_mm          ( void );
int   dice               ( int number, int size );
void  smash_tilde        ( char *str );
bool  str_cmp            ( const char *astr, const char *bstr );
bool  str_prefix         ( const char *astr, const char *bstr );
bool  str_infix          ( const char *astr, const char *bstr );
bool  str_suffix         ( const char *astr, const char *bstr );
char *capitalize         ( const char *str );
void  bug                ( const char *str, int param );
void  log_string         ( const char *str, ... ) __attribute__ ((format(printf, 1, 2)));
void  add_help           ( HELP_DATA * pHelp );
char *strupper           ( const char *str );

/* arena.c */
void  begin_team_arena               ( void );
void  win_arena                      ( CHAR_DATA *ch );
void  strip_arena_eq                 ( CHAR_DATA *ch );
void  open_fortress                  ( void );
void  init_teamarena                 ( void );
void  close_arena                    ( void );
void  open_arena                     ( void );

/* desc_map.c */
void  draw_map           ( CHAR_DATA * ch, char *desc );
int   get_line           ( char *desc, int max_len );

/* mxp.c */
void  mxp_to_char               ( char *txt, CHAR_DATA *ch, int mxp_style );
void  shutdown_mxp              ( DESCRIPTOR_DATA *d );
void  init_mxp                  ( DESCRIPTOR_DATA *d );

/* mxp macro */
#define USE_MXP(ch)             ( ch->desc->mxp )

/* event.c */
void  add_event_room             ( EVENT_DATA *event, ROOM_INDEX_DATA *pRoom, int delay );
void  add_event_object           ( EVENT_DATA *event, OBJ_DATA *obj, int delay );
void  add_event_desc             ( EVENT_DATA *event, DESCRIPTOR_DATA *d, int delay );
void  add_event_area             ( EVENT_DATA *event, AREA_DATA *pArea, int delay );
void  add_event_char             ( EVENT_DATA *event, CHAR_DATA *ch, int delay );
void  add_event_world            ( EVENT_DATA *event, int delay );
void  delay_message              ( char *message, CHAR_DATA *ch, int delay );
void  event_look                 ( CHAR_DATA *ch );
void  save_player_events         ( CHAR_DATA *ch, FILE *fp );
void  load_player_event          ( CHAR_DATA *ch, FILE *fp );
int   event_pulses_left          ( EVENT_DATA *event );
char *event_time_left            ( EVENT_DATA *event );
EVENT_DATA *event_isset_object   ( OBJ_DATA *obj, int type );
EVENT_DATA *event_isset_area     ( AREA_DATA *pArea, int type );
void  strip_event_object         ( OBJ_DATA *obj, int type );
void  strip_event_world          ( int type );
EVENT_DATA *event_isset_mobile   ( CHAR_DATA *ch, int type );
void  strip_event_mobile         ( CHAR_DATA *ch, int type );
EVENT_DATA *event_isset_room     ( ROOM_INDEX_DATA *pRoom, int type );
void  strip_event_socket         ( DESCRIPTOR_DATA *d, int type );
EVENT_DATA *event_isset_socket   ( DESCRIPTOR_DATA *d, int type );
void  object_decay               ( OBJ_DATA *obj, int delay );
void  count_events               ( int *a, int *b, int *c, int *d );
bool  enqueue_event              ( EVENT_DATA *event, int game_pulses );
void  dequeue_event              ( EVENT_DATA *event, bool dequeue_global );
EVENT_DATA *alloc_event          ( void );
void  init_event_queue           ( int section );
void  heartbeat                  ( void );
void  init_events_player         ( CHAR_DATA *ch );
void  init_events_room           ( ROOM_INDEX_DATA *pRoom );
void  init_events_mobile         ( CHAR_DATA *ch );
void  init_events_object         ( OBJ_DATA *obj );

/* event.c (the actual events) */
bool  event_dummy                ( EVENT_DATA *event );
bool  event_player_legend        ( EVENT_DATA *event );
bool  event_socket_idle          ( EVENT_DATA *event );
bool  event_player_save          ( EVENT_DATA *event );
bool  event_player_heal          ( EVENT_DATA *event );
bool  event_area_reset           ( EVENT_DATA *event );
bool  event_room_aggrocheck      ( EVENT_DATA *event );
bool  event_mobile_extract       ( EVENT_DATA *event );
bool  event_room_bladebarrier    ( EVENT_DATA *event );
bool  event_room_dispel_magic    ( EVENT_DATA *event );
bool  event_mobile_heal          ( EVENT_DATA *event );
bool  event_mobile_move          ( EVENT_DATA *event );
bool  event_mobile_spec          ( EVENT_DATA *event );
bool  event_mobile_scavenge      ( EVENT_DATA *event );
bool  event_char_update          ( EVENT_DATA *event );
bool  event_game_weather         ( EVENT_DATA *event );
bool  event_game_articheck       ( EVENT_DATA *event );
bool  event_game_crashsafe       ( EVENT_DATA *event );
bool  event_game_areasave        ( EVENT_DATA *event );
bool  event_game_ragnarok        ( EVENT_DATA *event );
bool  event_game_ragnarok_reset  ( EVENT_DATA *event );
bool  event_game_arena           ( EVENT_DATA *event );
bool  event_game_arena_close     ( EVENT_DATA *event );
bool  event_game_violence        ( EVENT_DATA *event );
bool  event_game_pulse30         ( EVENT_DATA *event );
bool  event_object_affects       ( EVENT_DATA *event );

/* webif.c */
void  init_webserver             ( int port );
void  close_webserver            ( void );
void  poll_web_requests          ( void );

/* giant.c */
void  giant_commands             ( CHAR_DATA *ch );
void  giant_evolve               ( CHAR_DATA *ch, char *argument, EVOLVE_DATA *evolve );
void  update_giant               ( CHAR_DATA *ch );

/* shadow.c */
void  shadow_commands            ( CHAR_DATA *ch );
void  shadow_evolve              ( CHAR_DATA *ch, char *argument, EVOLVE_DATA *evolve );
void  update_shadow              ( CHAR_DATA *ch );
bool  event_player_shadowportal  ( EVENT_DATA *event );
bool  event_player_witnessgrab   ( EVENT_DATA *event );
bool  event_mobile_shadowgrabbed ( EVENT_DATA *event );

/* warlock.c */
bool  event_player_huntingstars  ( EVENT_DATA *event );
void  warlock_commands           ( CHAR_DATA *ch );
void  discharge_contingency      ( CHAR_DATA *ch );
void  free_spell                 ( SPELL_DATA *spell );
void  warlock_evolve             ( CHAR_DATA *ch, char *argument, EVOLVE_DATA *evolve );
void  update_warlock             ( CHAR_DATA *ch );
void  update_archmage            ( CHAR_DATA *ch );
void  spell_from_char            ( SPELL_DATA *spell, CHAR_DATA *ch, bool spellchain );
bool  event_player_study         ( EVENT_DATA *event );
bool  event_player_catalyst      ( EVENT_DATA *event );
void  save_archmages             ( void );
void  load_archmages             ( void );

/* fae.c */
void fae_hunger                 ( CHAR_DATA *ch );
void fae_commands               ( CHAR_DATA *ch );
void fae_evolve                 ( CHAR_DATA *ch, char *argument, EVOLVE_DATA *evolve );
void update_fae                 ( CHAR_DATA *ch );
void fae_shield                 ( CHAR_DATA *ch, CHAR_DATA *victim, int dam );
bool event_player_fae_plasma    ( EVENT_DATA *event );
bool event_player_fae_matter    ( EVENT_DATA *event );
bool event_player_fae_energy    ( EVENT_DATA *event );
bool event_player_fae_will      ( EVENT_DATA *event );
bool event_player_spiders       ( EVENT_DATA *event );
bool event_player_phantom       ( EVENT_DATA *event );
bool event_mobile_acidblood     ( EVENT_DATA *event );
bool event_room_geyser          ( EVENT_DATA *event );
bool event_room_blastward       ( EVENT_DATA *event );
bool event_player_fae_timewarp  ( EVENT_DATA *event );
bool event_player_fae_sigil     ( EVENT_DATA *event );

/* kingdoms.c */
KINGDOM_QUEST*get_kingdom_quest ( void );
void  clear_kingdom_quest       ( void );
KINGDOM_DATA *vnum_kingdom      ( int vnum );
int   get_kingdom_upkeep        ( KINGDOM_DATA *kingdom );
bool  member_of_other_kingdom   ( CHAR_DATA *ch, KINGDOM_DATA *kingdom );
int   getKingdomMight           ( KINGDOM_DATA *kingdom );
void  delete_kingdom            ( KINGDOM_DATA *kingdom );
void  kingdom_look              ( CHAR_DATA *ch );
bool  in_kingdom_hall           ( CHAR_DATA *ch );
void  clear_kingdom             ( KINGDOM_DATA *kingdom );
MEMBER_DATA *alloc_member       ( void );
void  delay_act                 ( char *mess, ROOM_INDEX_DATA *pRoom, int pulses );
char *get_token                 ( char *argument, char *token );
void  load_kingdoms             ( void );
void  save_kingdoms             ( void );
void  save_kingdom              ( KINGDOM_DATA *kingdom );
void  update_kingdom_membership ( CHAR_DATA *ch, bool invite );
void  free_member               ( MEMBER_DATA *member );
bool  event_game_kingdomquest   ( EVENT_DATA *event );
bool  event_room_kingdomquest   ( EVENT_DATA *event );
bool  event_room_act            ( EVENT_DATA *event );
char *get_kingdomname           ( int kingid );
KINGDOM_DATA *get_kingdom       ( CHAR_DATA *ch );
KINGDOM_DATA *get_kingdom2      ( char *name );

/* kingdoms.c tables */
extern const struct flag_type kingdom_flags[];

/* artifacts.c */
void  update_artifact_table     ( OBJ_DATA *artifact, CHAR_DATA *owner, bool active );
bool  check_arti_ownership      ( CHAR_DATA *ch, OBJ_DATA *obj );

/* utility.c */
void  modify_hps                ( CHAR_DATA *ch, int modifier );
void  modify_mana               ( CHAR_DATA *ch, int modifier );
void  modify_move               ( CHAR_DATA *ch, int modifier );
char *GetSoundexKey             ( const char *szTxt );
int   SoundexMatch              ( char *szFirst, char *szSecond );
void  clearstats                ( CHAR_DATA *ch );
char *string_restrict           ( char *str, int size );
int   cprintf                   ( char *buf, char *ptr, ... ) __attribute__ ((format(printf, 2, 3)));
int   cnprintf                  ( char *buf, int maxlen, char *ptr, ... ) __attribute__ ((format(printf, 3, 4)));
int   collen                    ( const char *str );
char *get_time_left             ( int secs );
BUFFER *identify_obj            ( OBJ_DATA *obj );
BUFFER *box_text                ( char *txt, char *title );
void  band_description          ( CHAR_DATA *ch );
void  show_class_help           ( DESCRIPTOR_DATA *d, int class );
char *dot_it_up                 ( int value );
bool  can_use_skill             ( CHAR_DATA *ch, int sn );
bool  can_use_command           ( CHAR_DATA *ch, int cmd );
char *smudge_time               ( time_t pTime );
void  mob_outstance             ( CHAR_DATA *ch );
void  update_auctions           ( void );
AUCTION_DATA *alloc_auction     ( void );
void  free_auction              ( AUCTION_DATA *auction );
bool  object_is_affected        ( OBJ_DATA *obj, int bit);
int   reduce_cost               ( CHAR_DATA *ch, CHAR_DATA *victim, int cost, int type );
int   next_rank_in              ( CHAR_DATA *ch );
char *get_rare_rank_name        ( int value );
int   get_rare_rank_value       ( int value );
void  free_snoop                ( DESCRIPTOR_DATA *d, SNOOP_DATA *snoop );
SNOOP_DATA *alloc_snoop         ( void );
bool  account_exists            ( char *name );
char *strip_returns             ( char *txt );
void  aggress                   ( CHAR_DATA *ch, CHAR_DATA *victim );
void  stop_spectating           ( CHAR_DATA *ch );
bool  ccenter_not_stock         ( void );
char *col_scale                 ( int current, int max );
SESSION_DATA *alloc_session     ( void );
void  free_session              ( CHAR_DATA *ch );
FEED_DATA *alloc_feed           ( void );
bool  can_kill_lowbie           ( CHAR_DATA *highbie, CHAR_DATA *lowbie );
bool  check_feed                ( CHAR_DATA *attacker, CHAR_DATA *defender );
void  update_feed               ( CHAR_DATA *attacker, CHAR_DATA *defender );
void  free_feed                 ( CHAR_DATA *ch, FEED_DATA *feed );
bool  account_olc_area          ( ACCOUNT_DATA *account, AREA_DATA *pArea );
void  login_char                ( CHAR_DATA *ch, bool new_player );
void  damage_obj                ( CHAR_DATA *ch, OBJ_DATA *obj, int dam );
int   getGold                   ( CHAR_DATA *ch );
void  setGold                   ( CHAR_DATA *ch, int amount );
int   getGoldTotal              ( CHAR_DATA *ch );
void  setGoldTotal              ( CHAR_DATA *ch, int amount );
void  char_popup                ( CHAR_DATA *ch );
void  fix_weaponskill           ( CHAR_DATA *ch, bool stolen );
void  fix_magicskill            ( CHAR_DATA *ch, bool stolen );
void  obj_say                   ( OBJ_DATA *obj, char *txt, char *type );
bool  pre_reboot_actions        ( bool reboot );
char *replace_letter_with_word  ( char *txt, char letter, char *word );
char *get_date_string           ( time_t tm );
bool  is_silenced               ( CHAR_DATA *ch );
bool  in_newbiezone             ( CHAR_DATA *ch );
int   check_pkstatus            ( CHAR_DATA *ch, CHAR_DATA *victim );
void  check_top10               ( CHAR_DATA *ch );
bool  can_refer                 ( ACCOUNT_DATA *account );
void  system_integrity          ( void );
CHAR_DATA *get_online_player    ( int playerid );
void  affect_from_area          ( AREA_DATA *pArea, AREA_AFFECT *paf );
void  affect_to_area            ( AREA_DATA *pArea, AREA_AFFECT *paf );
AREA_AFFECT *has_area_affect    ( AREA_DATA *pArea, int affect, int owner );
void  force_account_reload      ( ACCOUNT_DATA *account );
void  break_obj                 ( CHAR_DATA *ch, OBJ_DATA *obj );
void  powerdown                 ( CHAR_DATA *ch );
void  show_help_player          ( char *txt, CHAR_DATA *ch );
OBJ_DATA *pop_rand_equipment    ( int level, bool forced );
OBJ_DATA *pop_rand_loweq        ( void );
OBJ_DATA *pop_random_magic      ( void );
bool  is_valid_class_choice     ( char *choice, CHAR_DATA *ch );
void  show_class_options        ( DESCRIPTOR_DATA *d );
char *class_lookup              ( CHAR_DATA *ch );
void  create_new_account        ( ACCOUNT_DATA *account );
void  account_new_player        ( ACCOUNT_DATA *account, CHAR_DATA *dMob );
void  account_update            ( ACCOUNT_DATA *account, CHAR_DATA *dMob );
bool  already_logged            ( char *name );
char *get_option_login          ( char *list, int option );
void  show_options              ( DESCRIPTOR_DATA *dsock );
struct plist *parse_player_list ( char *list );
void  close_account             ( ACCOUNT_DATA *account );
ACCOUNT_DATA *alloc_account     ( void );
TOP10_ENTRY *alloc_top10entry   ( CHAR_DATA *ch );
void  free_top10entry           ( TOP10_ENTRY *entry );
char *pathfind                  ( CHAR_DATA *ch, CHAR_DATA *victim );
char *get_exits                 ( CHAR_DATA *ch );
char *strip_ansi                ( char *str );
bool  in_arena                  ( CHAR_DATA *ch );
char *line_indent               ( char *text, int wBegin, int wMax );
void  ragnarok_stop             ( void );
void  dump_last_command         ( void );
int   get_ratio                 ( CHAR_DATA * ch );
bool  is_contained              ( const char *astr, const char *bstr );
bool  is_contained2             ( const char *astr, const char *bstr );
void  increase_total_output     ( int clenght );
void  update_mudinfo            ( void );
void  recycle_descriptors       ( void );
void  recycle_dummys            ( void );
int   get_next_playerid         ( void );
void  log_string2               ( const char *str, ... ) __attribute__ ((format(printf, 1, 2)));
bool  check_help_soundex        ( char *argument, CHAR_DATA * ch );
void  special_decap_message     ( CHAR_DATA * ch, CHAR_DATA * victim );
int   strlen2                   ( const char *b );
int   calc_ratio                ( int a, int b );
void  logout_message            ( CHAR_DATA * ch );
void  login_message             ( CHAR_DATA * ch );
void  avatar_message            ( CHAR_DATA * ch );
void  tie_message               ( CHAR_DATA * ch, CHAR_DATA * victim );
void  enter_info                ( char *str );
void  leave_info                ( char *str );
void  death_info                ( char *str );
void  avatar_info               ( char *str );
int   getMight                  ( CHAR_DATA *ch );
int   getMobMight               ( MOB_INDEX_DATA *pMob );
int   getRank                   ( CHAR_DATA *ch, int bFull );
void  forge_affect              ( OBJ_DATA *obj, int value );
void  update_revision           ( CHAR_DATA *ch );
bool  in_fortress               ( CHAR_DATA *ch );
bool  fair_fight                ( CHAR_DATA *ch, CHAR_DATA *victim );
void  update_polls              ( void );
void  complete_poll             ( POLL_DATA *poll );
bool  remove_change             ( int i );
char *get_dystopia_banner       ( char *title, int size );
char *parse_alias               ( char *pString, char *argument, char *output_rest, \
                                        DESCRIPTOR_DATA *dsock );
char *create_alias_string       ( char *arg );
bool  check_alias               ( char *incom, DESCRIPTOR_DATA *dsock );
ALIAS_DATA *alloc_alias         ( void );
void  alias_from_player         ( CHAR_DATA *ch, ALIAS_DATA *alias );


/* quests.c */
void  quest_to_char             ( CHAR_DATA * ch, QUEST_DATA * quest );
void  quest_from_char           ( CHAR_DATA * ch, QUEST_DATA * quest );
bool  is_quest_target           ( CHAR_DATA * ch, CHAR_DATA * victim );
QF   *quest_lookup              ( const char *name );
char *quest_string              ( QUEST_FUN * fun );

/* msp.c */
void  update_music             ( CHAR_DATA * ch );
void  sound_to_char            ( char *txt, CHAR_DATA *ch );
void  sound_to_room            ( char *txt, CHAR_DATA *ch );

/* shops.c */
SHOP_FUN *shop_lookup           ( const char *name );
char     *shop_string           ( SHOP_FUN * fun );

/* fight.c */
void  special_move            ( CHAR_DATA * ch, CHAR_DATA * victim );
void  fortresskill            ( CHAR_DATA *ch, CHAR_DATA *victim );
void  ragnarokdecap           ( CHAR_DATA *ch, CHAR_DATA *victim );
char *death_string            ( DEATH_FUN *fun );
DEATH_FUN *death_lookup       ( const char *name );
int   dambonus                ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int stance );
void  decap_message           ( CHAR_DATA *ch, CHAR_DATA *victim );
bool  check_dodge             ( CHAR_DATA *ch, CHAR_DATA *victim, int dt );
bool  event_mobile_flee       ( EVENT_DATA *event );
bool  event_mobile_fighting   ( EVENT_DATA *event );
void  dropinvis               ( CHAR_DATA *ch );
int   cap_dam                 ( CHAR_DATA *ch, CHAR_DATA *victim, int dam );
int   up_dam                  ( CHAR_DATA *ch, CHAR_DATA *victim, int dam );
int   randomize_damage        ( CHAR_DATA *ch, int dam, int am );
void  update_damcap           ( CHAR_DATA *ch, CHAR_DATA *victim );
bool  killperson              ( CHAR_DATA *victim );
void  multi_hit               ( CHAR_DATA *ch, CHAR_DATA *victim, int turn );
void  damage                  ( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, int dam, int dt );
void  update_pos              ( CHAR_DATA *victim );
void  stop_fighting           ( CHAR_DATA *ch, bool fBoth );
void  stop_embrace            ( CHAR_DATA *ch, CHAR_DATA *victim );
bool  is_safe                 ( CHAR_DATA *ch, CHAR_DATA *victim );
void  hurt_person             ( CHAR_DATA *ch, CHAR_DATA *victim, int dam );
void  set_fighting            ( CHAR_DATA *ch, CHAR_DATA *victim );
bool  has_timer               ( CHAR_DATA *ch );

void  check_leaderboard    ( CHAR_DATA *ch );
void  crashrecov           ( int iSignal );
void  negotiate            ( DESCRIPTOR_DATA *d );

/* handler.c */
HELP_DATA *get_help           ( CHAR_DATA *ch, char *argument );
ROOM_INDEX_DATA *locate_obj   ( OBJ_DATA *obj );
bool  char_exists             ( char *argument );
int   get_trust               ( CHAR_DATA *ch );
int   get_age                 ( CHAR_DATA *ch );
int   get_curr_str            ( CHAR_DATA *ch );
int   get_curr_int            ( CHAR_DATA *ch );
int   get_curr_wis            ( CHAR_DATA *ch );
int   get_curr_dex            ( CHAR_DATA *ch );
int   get_curr_con            ( CHAR_DATA *ch );
int   can_carry_n             ( CHAR_DATA *ch );
int   can_carry_w             ( CHAR_DATA *ch );
bool  is_name                 ( char *str, char *namelist );
bool  is_full_name            ( const char *str, char *namelist );
void  affect_to_char          ( CHAR_DATA *ch, AFFECT_DATA * paf );
void  affect_to_obj           ( OBJ_DATA *obj, AFFECT_DATA * paf );
void  affect_remove           ( CHAR_DATA *ch, AFFECT_DATA * paf );
void  affect_strip            ( CHAR_DATA *ch, int sn );
bool  is_affected             ( CHAR_DATA *ch, int sn );
void  affect_join             ( CHAR_DATA *ch, AFFECT_DATA * paf );
void  char_from_room          ( CHAR_DATA *ch );
void  char_to_room            ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex, bool trigger );
void  obj_to_char             ( OBJ_DATA *obj, CHAR_DATA * ch );
void  obj_to_char_end         ( OBJ_DATA *obj, CHAR_DATA * ch );
void  obj_from_char           ( OBJ_DATA *obj );
int   apply_ac                ( OBJ_DATA *obj, int iWear );
OD   *get_eq_char             ( CHAR_DATA *ch, int iWear );
void  equip_char              ( CHAR_DATA *ch, OBJ_DATA * obj, int iWear );
void  unequip_char            ( CHAR_DATA *ch, OBJ_DATA * obj );
int   count_obj_list          ( OBJ_INDEX_DATA *obj, LIST * list );
void  obj_from_room           ( OBJ_DATA *obj );
void  obj_to_room             ( OBJ_DATA *obj, ROOM_INDEX_DATA * pRoomIndex );
void  obj_to_obj              ( OBJ_DATA *obj, OBJ_DATA * obj_to );
void  obj_to_obj_end          ( OBJ_DATA *obj, OBJ_DATA * obj_to );
void  obj_from_obj            ( OBJ_DATA *obj );
void  extract_obj             ( OBJ_DATA *obj );
void  free_obj                ( OBJ_DATA *obj );
void  extract_char            ( CHAR_DATA *ch, bool fPull );
CD   *get_char_room           ( CHAR_DATA * ch, char *argument );
CD   *get_char_world          ( CHAR_DATA * ch, char *argument );
CD   *get_char_area           ( CHAR_DATA * ch, char *argument );
OD   *get_obj_type            ( OBJ_INDEX_DATA * pObjIndexData );
OD   *get_obj_list            ( CHAR_DATA * ch, char *argument, LIST *list );
OD   *get_obj_carry           ( CHAR_DATA * ch, char *argument );
OD   *get_obj_wear            ( CHAR_DATA * ch, char *argument );
OD   *get_obj_here            ( CHAR_DATA * ch, char *argument );
OD   *get_obj_room            ( CHAR_DATA * ch, char *argument );
OD   *get_obj_world           ( CHAR_DATA * ch, char *argument );
int   get_obj_number          ( OBJ_DATA * obj );
int   get_obj_weight          ( OBJ_DATA * obj );
bool  room_is_dark            ( ROOM_INDEX_DATA * pRoomIndex );
bool  room_is_private         ( ROOM_INDEX_DATA * pRoomIndex );
bool  can_see                 ( CHAR_DATA * ch, CHAR_DATA * victim );
bool  can_see_obj             ( CHAR_DATA * ch, OBJ_DATA * obj );
bool  can_drop_obj            ( CHAR_DATA * ch, OBJ_DATA * obj );
char *item_type_name          ( OBJ_DATA * obj );
char *affect_loc_name         ( int location );
char *affect_bit_name         ( int vector );
char *extra_bit_name          ( int extra_flags );
void  affect_modify           ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd );

/* interp.c */
void  interpret               ( CHAR_DATA * ch, char *argument );
bool  is_number               ( char *arg );
int   number_argument         ( char *argument, char *arg );
char *one_argument            ( char *argument, char *arg_first );
void  stage_update            ( CHAR_DATA * ch, CHAR_DATA * victim, int stage, char *argument );
void  make_preg               ( CHAR_DATA * mother, CHAR_DATA * father );

/* magic.c */
bool  cast_spell         ( EVENT_DATA *event );
int   skill_lookup       ( const char *name );
bool  saves_spell        ( int level, CHAR_DATA * victim );
void  obj_cast_spell     ( int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj );

/* save.c */
void  save_auctions             ( void );
void  load_auctions             ( void );
CD   *load_char_whois           ( char *name, int *status );
void  save_char_obj             ( CHAR_DATA * ch );
bool  load_char_obj             ( DESCRIPTOR_DATA * d, char *name );

/* special.c */
SF   *spec_lookup        ( const char *name );

/* mccp.c */
bool  compressStart      ( DESCRIPTOR_DATA * desc );
bool  compressEnd        ( DESCRIPTOR_DATA * desc );
bool  processCompressed  ( DESCRIPTOR_DATA * desc );
bool  writeCompressed    ( DESCRIPTOR_DATA * desc, char *txt, int length );

/* update.c */
void  regen_hps                   ( CHAR_DATA *ch, int multiplier );
void  regen_mana                  ( CHAR_DATA *ch, int multiplier );
void  regen_move                  ( CHAR_DATA *ch, int multiplier );
bool  update_player_idle          ( CHAR_DATA *ch );
void  reg_mend                    ( CHAR_DATA *ch );
void  update_active_counters      ( CHAR_DATA *ch );
void  update_morted_timer         ( CHAR_DATA *ch );
void  update_sit_safe_counter     ( CHAR_DATA *ch );
void  update_drunks               ( CHAR_DATA *ch );
void  sex_update                  ( CHAR_DATA *ch );
void  update_arti_regen           ( CHAR_DATA *ch );
void  regen_limb                  ( CHAR_DATA *ch );
void  update_midi                 ( CHAR_DATA *ch );
void  update_edge                 ( CHAR_DATA *ch );

#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef  ED

/*****************************************************************************
 *                                    OLC                                    *
 *****************************************************************************/

/*
 * This structure is used in special.c to lookup spec funcs and
 * also in olc_act.c to display listings of spec funcs.
 */
struct spec_type
{
  char *spec_name;
  SPEC_FUN *spec_fun;
};

/*
 * Used in quests.c
 */
struct quest_type
{
  char *quest_name;
  QUEST_FUN *quest_fun;
};

struct death_type
{
  char *death_name;
  DEATH_FUN *death_fun;
};

/*
 * used in shops.c
 */
struct shop_type
{
  char *shop_name;
  SHOP_FUN *shop_fun;
};

/*
 * Area flags.
 */
#define         AREA_NONE       0
#define         AREA_CHANGED    1 /* Area has been modified. */
#define         AREA_ADDED      2 /* Area has been added to. */
#define         AREA_LOADING    4 /* Used for counting in db.c */

/*
 * Area bits
 */
#define AREA_BIT_NONE         0
#define AREA_BIT_NO_TRAVEL    1 /* it is not possibly to use class travels to mobs in this area */
#define AREA_BIT_IREGEN       2 /* increases regen rate for the area              */
#define AREA_BIT_DREGEN       4 /* decreates regen rate for the area              */
#define AREA_BIT_NOMAGIC      8 /* no magic spells (player/object) can be cast    */
#define AREA_BIT_NOHOME      16 /* it is not possible to home in this area        */
#define AREA_BIT_OLC         32 /* currently under construction                   */
#define AREA_BIT_NOPENTA     64 /* cannot use pentagram like powers in this area  */

#define MAX_DIR	6
#define NO_FLAG -99             /* Must not be used in flags or stats. */

/* command bits (for class commands) */
#define CP_LEVEL              1
#define CP_BIT                2
#define CP_MASTERY            3

/*
 * Global Constants
 */
extern char *const dir_name[];
extern const sh_int rev_dir[];
extern const struct spec_type spec_table[];
extern const struct quest_type quest_table[];
extern const struct death_type death_table[];
extern const struct shop_type shop_table[];

/*
 * Global variables
 */
extern char             *  last_command;
extern int                 nAllocString;
extern LIST             *  area_list;
extern STACK            *  area_free;
extern int                 top_affect;
extern int                 top_area;
extern int                 top_ed;
extern int                 top_exit;
extern int                 top_help;
extern int                 top_mob_index;
extern int                 top_obj_index;
extern int                 top_reset;
extern int                 top_room;
extern int                 top_vnum_mob;
extern int                 top_vnum_obj;
extern int                 top_vnum_room;
extern char                str_empty[1];
extern LIST             *  mob_index_hash[MAX_KEY_HASH];
extern LIST             *  obj_index_hash[MAX_KEY_HASH];
extern LIST             *  room_index_hash[MAX_KEY_HASH];
extern LEADER_BOARD        leader_board;
extern TEAMARENA_DATA      arena;
extern MUDDATA             muddata;
extern MAP_TYPE            map[MAPX + 1][MAPY + 1];

/* db.c */
ACCOUNT_DATA *load_account      ( char *name );
void save_account               ( ACCOUNT_DATA *account );
void  load_top10                ( void );
void  save_top10                ( void );
void  load_newbiebans           ( void );   
void  save_newbiebans           ( void );
void  load_leaderboard          ( void );
void  save_leaderboard          ( void );
void  save_muddata              ( void );
void  load_muddata              ( void );
void  write_mudinfo_database    ( void );
void  load_bans                 ( void );
void  save_bans                 ( void );
void  load_changes              ( void );
void  save_changes              ( void );
void  load_polls                ( void );
void  save_polls                ( void );
void  load_subvotes             ( POLL_DATA * poll );
void  save_subvotes             ( POLL_DATA * poll );
void  reset_area                ( AREA_DATA * pArea );
void  reset_room                ( ROOM_INDEX_DATA * pRoom );

#define  buffer_new(size)             __buffer_new     ( size)
#define  buffer_strcat(buffer,text)   __buffer_strcat  ( buffer, text )

/* string.c */
void  buffer_clear          ( BUFFER *buffer );
int   bprintf               ( BUFFER *buffer, char *fmt, ... ) \
                               __attribute__ ((format(printf, 2, 3)));
void  buffer_free           ( BUFFER *buffer );
BUFFER *__buffer_new        ( int size );
void    __buffer_strcat     ( BUFFER *buffer, const char *text );
void  string_edit           ( CHAR_DATA * ch, char **pString );
void  string_append         ( CHAR_DATA * ch, char **pString );
char *string_replace        ( char *orig, char *old, char *new );
void  string_add            ( CHAR_DATA * ch, char *argument );
char *format_string         ( char *oldstring );
char *first_arg             ( char *argument, char *arg_first, bool fCase );
char *string_unpad          ( char *argument );
char *string_proper         ( char *argument );
char *all_capitalize        ( const char *str );

/* olc.c */
bool  run_olc_editor        ( DESCRIPTOR_DATA * d );
char *olc_ed_name           ( CHAR_DATA * ch );
char *olc_ed_vnum           ( CHAR_DATA * ch );

/* special.c */
char *spec_string           ( SPEC_FUN * fun );

/* bit.c */
extern const struct flag_type area_flags[];
extern const struct flag_type area_bits[];
extern const struct flag_type sex_flags[];
extern const struct flag_type exit_flags[];
extern const struct flag_type door_resets[];
extern const struct flag_type room_flags[];
extern const struct flag_type sector_flags[];
extern const struct flag_type type_flags[];
extern const struct flag_type extra_flags[];
extern const struct flag_type wear_flags[];
extern const struct flag_type act_flags[];
extern const struct flag_type affect_flags[];
extern const struct flag_type apply_flags[];
extern const struct flag_type wear_loc_strings[];
extern const struct flag_type wear_loc_flags[];
extern const struct flag_type weapon_flags[];
extern const struct flag_type container_flags[];
extern const struct flag_type liquid_flags[];
extern const struct class_type class_table[];

#endif /* DYSTOPIA_HEADER */
