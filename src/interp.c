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
#include <unistd.h>

#include "dystopia.h"

bool  check_social      ( CHAR_DATA *ch, char *command, char *argument );
bool  check_xsocial     ( CHAR_DATA *ch, char *command, char *argument );

int can_interpret(CHAR_DATA *ch, int cmd)
{
  if (IS_NPC(ch))
    return 0;

  if (cmd_table[cmd].level > get_trust(ch))
  {
    if (is_full_name(cmd_table[cmd].name, ch->pcdata->immcmd))
      return 1;
    else
      return 0;
  }

  if (!can_use_command(ch, cmd))
    return 0;

  if (ch->position < cmd_table[cmd].position)
    return -1;

  return 1;
}

/*
 * Command logging types.
 */
#define LOG_NORMAL	0  /* Doesn't log unless the player is logged                  */
#define LOG_ALWAYS	1  /* Will always log this command (both file and logchannel)  */
#define LOG_NEVER	2  /* Will never log this command, no matter what              */
#define LOG_SEMI        3  /* Will always log this command (file only)                 */

/*
 * Log-all switch.
 */
char *  last_command    = NULL;
bool    ragnarok        = FALSE;
bool    new_ragnarok    = TRUE;
bool	fLogAll		= FALSE;
bool    cmd_done        = TRUE;
int     thread_count    = 0;
int     top_playerid    = 0;
int     ragnarok_cost   = 3000;
int	iDelete		= 0;
int     total_output    = 0;
int     mudinfo[MUDINFO_MAX];
int     ccenter[CCENTER_MAX];


bool    check_disabled (const struct cmd_type *command);

LIST *disabled_list = NULL;
LIST *ban_list = NULL;
LIST *newbieban_list = NULL;

/*
 * Command table.
 */
const	struct	cmd_type	cmd_table	[] =
{
    /* Name           Function         Min Position Level    Log   Class, DiscLevel, DiscName */

    /* commone movement commands */
    { "north",        do_north,        POS_STANDING,  0,  LOG_NEVER, 0, 0, 0, 0 },
    { "east",         do_east,         POS_STANDING,  0,  LOG_NEVER, 0, 0, 0, 0 },
    { "south",        do_south,        POS_STANDING,  0,  LOG_NEVER, 0, 0, 0, 0 },
    { "west",         do_west,         POS_STANDING,  0,  LOG_NEVER, 0, 0, 0, 0 },
    { "up",           do_up,           POS_STANDING,  0,  LOG_NEVER, 0, 0, 0, 0 },
    { "down",         do_down,         POS_STANDING,  0,  LOG_NEVER, 0, 0, 0, 0 },

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
    { "cast",           do_cast,        POS_FIGHTING,    0,  LOG_NEVER,  0, 0, 0, 0 },
    { "call",           do_call,	POS_SLEEPING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "consider",	do_consider,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "crack",		do_crack,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "diagnose",	do_diagnose,	POS_FIGHTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "dismount",	do_dismount,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "enter",		do_enter,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "exits",		do_exits,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "get",		do_get,		POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "inventory",	do_inventory,   POS_DEAD, 	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "rarelist",       do_rarelist,    POS_DEAD,        0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kill",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "combatswitch",	do_combatswitch,POS_FIGHTING,    0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "look",		do_look,	POS_MEDITATING,	 0,  LOG_NEVER,  0, 0, 0, 0 },
    { "ls",		do_look,	POS_MEDITATING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "mount",		do_mount,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "order",		do_order,	POS_SITTING,	 1,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "reply",          do_reply,       POS_MEDITATING,  0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "rest",		do_rest,	POS_MEDITATING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "sit",		do_sit,	        POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "stand",		do_stand,	POS_SLEEPING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "tell",		do_tell,	POS_MEDITATING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "whisper",	do_whisper,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "wield",		do_wear,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "wizhelp",	do_wizhelp,	POS_DEAD,	 4,  LOG_NORMAL, 0, 0, 0, 0 },
    { "version",	do_version,	POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "linkdead",	do_linkdead,    POS_DEAD,        7,  LOG_NORMAL, 0, 0, 0, 0 },

    /*
     * Informational commands.
     */
    { "affects",        do_upkeep,      POS_DEAD,        0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "areas",		do_areas,	POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "areaaff",        do_areaaff,     POS_DEAD,        0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "commands",	do_commands,	POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "credits",	do_credits,	POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "dcredits",       do_dcredits,    POS_DEAD,  	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "equipment",	do_equipment,	POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "examine",	do_examine,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "help",		do_help,	POS_DEAD,	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "search",         do_search,      POS_DEAD,        1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "idea",		do_idea,	POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "report",		do_report,	POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "autostance",	do_autostance,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "mastery",        do_mastery,     POS_STANDING,    3,  LOG_SEMI,   0, 0, 0, 0 },
    { "gensteal",       do_gensteal, 	POS_STANDING,    3,  LOG_NORMAL, 0, 0, 0, 0 },
    { "level",          do_level,  	POS_DEAD,        0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "score",		do_score,	POS_DEAD, 	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "session",        do_session,     POS_DEAD,        0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "skill",		do_skill,	POS_MEDITATING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "spells",		do_spell,	POS_MEDITATING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "socials",	do_socials,	POS_DEAD, 	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "time",		do_time,	POS_DEAD, 	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "typo",		do_typo,	POS_DEAD, 	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "who",		do_who,		POS_DEAD, 	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "mudinfo",        do_mudinfo,     POS_DEAD,        1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "timer",          do_timer,       POS_DEAD,        2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "wizlist",	do_wizlist,	POS_DEAD, 	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "xemot",		do_huh,		POS_DEAD, 	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "xemote",		do_xemote,	POS_SITTING,	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "xsocial",	do_huh,		POS_DEAD, 	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "xsocials",	do_xsocials,	POS_DEAD, 	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "group",          do_group,       POS_DEAD,        2,  LOG_NORMAL, 0, 0, 0, 0 },

    /*
     * Configuration commands.
     */

    { "alignment",	do_alignment,   POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "config",		do_config,	POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "enhcombat",      do_enhcombat,   POS_DEAD,        1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "compres",        do_compres,     POS_DEAD,        1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "compress",       do_compress,    POS_DEAD,        1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "description",	do_description, POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "title",		do_title,	POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "ansi",		do_ansi,	POS_DEAD, 	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "autoexit",	do_autoexit,    POS_DEAD, 	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "autoloot",	do_autoloot,    POS_DEAD, 	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "autosac",	do_autosac,	POS_DEAD, 	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "autohead",       do_autohead,    POS_DEAD,        1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "automap",        do_automap,     POS_DEAD,        1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "brief",          do_brief,       POS_DEAD,        2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "brief1",	        do_brief1,	POS_DEAD, 	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "brief2",         do_brief2,      POS_DEAD,        0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "brief3",         do_brief3,      POS_DEAD,        0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "brief4",         do_brief4,      POS_DEAD,        0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "brief5",         do_brief5,      POS_DEAD,        0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "brief6",         do_brief6,      POS_DEAD,        0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "brief7",         do_brief7,      POS_DEAD,        0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "cprompt",	do_cprompt,	POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "prompt",		do_prompt,	POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "oldprompt",      do_oldprompt,   POS_DEAD,        2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "sound",          do_sound,       POS_DEAD,        1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "music",          do_music,       POS_DEAD,        1,  LOG_NORMAL, 0, 0, 0, 0 },

    /*
     * Communication commands.
     */

    { "flame",		do_flame,        POS_DEAD,       0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "chat",		do_chat,	 POS_DEAD,       0,  LOG_NORMAL, 0, 0, 0, 0 },
    { ".",	        do_chat,	 POS_DEAD,       2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "emote",		do_xemote,	 POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { ",",		do_xemote,	 POS_SITTING,    0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "gtell",		do_gtell,	 POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { ";",		do_gtell,	 POS_DEAD, 	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "sing",		do_sing,	 POS_SLEEPING,	 2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "note",		do_note,	 POS_DEAD, 	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "board",		do_board,	 POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "pose",		do_emote,	 POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "say",		do_say,		 POS_MEDITATING, 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "'",		do_say,		 POS_MEDITATING, 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "yell",		do_yell,	 POS_SITTING,	 2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "powers",         do_racecommands, POS_DEAD,       1,  LOG_NORMAL, 0, 0, 0, 0 },

    /*
     * Object manipulation commands.
     */

    { "brandish",	do_brandish,    POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "close",		do_close,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "draw",		do_draw,	POS_FIGHTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "drink",		do_drink,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "drop",		do_drop,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "eat",		do_eat,	        POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "empty",		do_empty,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "fill",		do_fill,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "give",		do_give,	POS_SITTING,	 0,  LOG_SEMI,   0, 0, 0, 0 },
    { "gift",           do_gift,        POS_STANDING,    0,  LOG_SEMI,   0, 0, 0, 0 },
    { "donate",         do_donate,      POS_STANDING,    1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "hold",		do_wear,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "lock",		do_lock,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "open",		do_open,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "pick",		do_pick,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "put",		do_put,	        POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "quaff",		do_quaff,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "recite",		do_recite,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "remove",		do_remove,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "sheath",		do_sheath,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "take",		do_get,	        POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "throw",		do_throw,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "sacrifice",	do_sacrifice,	POS_SITTING,	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "unlock",		do_unlock,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "wear",		do_wear,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "zap",		do_zap,     	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "junk",           do_sacrifice,   POS_STANDING,    1,  LOG_NORMAL, 0, 0, 0, 0 },

    /*
     * Combat commands.
     */

    { "generation",     do_generation,  POS_STANDING,   12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "backstab",	do_backstab,    POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "berserk",	do_berserk,	POS_FIGHTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "bs",		do_backstab,    POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "disarm",		do_disarm,	POS_FIGHTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "flee",		do_flee,	POS_FIGHTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "hurl",		do_hurl,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kick",		do_kick,	POS_FIGHTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "punch",		do_punch,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "rescue",		do_rescue,	POS_FIGHTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "stance",		do_stance,	POS_FIGHTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },


    /*
     * Miscellaneous commands.
     */
    { "accep",		do_huh,		POS_STANDING,	 2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "accept",		do_accept,	POS_STANDING,	 2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "artifact",	do_artifact,	POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "artiwiz",        do_artiwiz,     POS_DEAD,       12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "birth",		do_birth,	POS_STANDING,	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "blindfold",	do_blindfold,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "breaku",		do_huh,		POS_STANDING,	 2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "breakup",	do_breakup,	POS_STANDING,	 2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "claim",		do_claim,	POS_STANDING,	 2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "unclaim",        do_unclaim,     POS_STANDING,    2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "consen",		do_huh,		POS_STANDING,	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "consent",	do_consent,	POS_STANDING,	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "finger",		do_finger,	POS_SITTING,	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "follow",		do_follow,	POS_STANDING,	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "gag",		do_gag,		POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "home",		do_home,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "locate",		do_locate,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "practice",	do_practice,	POS_SLEEPING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "propos",		do_huh,		POS_STANDING,	 2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "propose",	do_propose,	POS_STANDING,	 2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "pshift",         do_pshift,      POS_STANDING,    2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "qui",		do_qui,		POS_DEAD,        0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "quit",		do_quit,	POS_SLEEPING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "recall",		do_recall,	POS_FIGHTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "escape",		do_escape,	POS_DEAD,	 3,  LOG_NORMAL, 0, 0, 0, 0 },
    { "/",		do_recall,	POS_FIGHTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "safe",		do_safe,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "save",		do_save,	POS_DEAD, 	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "sleep",		do_sleep,	POS_SLEEPING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "smother",	do_smother,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "sneak",		do_sneak,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "scan",		do_scan,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "spy",		do_spy,		POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "steal",		do_steal,	POS_FIGHTING,	 0,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "summon", 	do_nosummon,	POS_DEAD, 	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "tie",		do_tie,		POS_STANDING,	 3,  LOG_NORMAL, 0, 0, 0, 0 },
    { "token",		do_withdraw,	POS_STANDING,	 2,  LOG_SEMI,   0, 0, 0, 0 },
    { "withdraw",       do_withdraw,    POS_STANDING,    2,  LOG_SEMI,   0, 0, 0, 0 },
    { "deposit",        do_deposit,     POS_STANDING,    2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "track",		do_track,	POS_STANDING,	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "train",		do_train,       POS_STANDING,	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "pkready",        do_pkready,     POS_STANDING,    3,  LOG_NORMAL, 0, 0, 0, 0 },
    { "untie",		do_untie,	POS_STANDING,	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "visible",	do_visible,	POS_SLEEPING,	 1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "wake",		do_wake,	POS_SLEEPING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "where",		do_where,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "info",		do_info,	POS_DEAD,       12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "ban",		do_ban, 	POS_DEAD,	12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "transfer",       do_transfer,    POS_DEAD,        7,  LOG_NORMAL, 0, 0, 0, 0 },
    { "afk",      	do_afk,         POS_STANDING,    3,  LOG_NORMAL, 0, 0, 0, 0 },
    { "bitchslap", 	do_freeze,      POS_DEAD,        9,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "hlist",		do_hlist,	POS_DEAD,       12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "flex",		do_flex,	POS_SITTING,	 0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "relevel",	do_relevel,	POS_DEAD,   	 0,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "upkeep",		do_upkeep,	POS_DEAD,        2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "bounty", 	do_bounty, 	POS_STANDING, 	 2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "bountylist",	do_bountylist, 	POS_FIGHTING, 	 2,  LOG_NORMAL, 0, 0, 0, 0 },

     /* quest stuff */
    { "showquest",      do_showquest,   POS_DEAD,        1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "qgain",          do_qgain,       POS_STANDING,    1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "qcomplete",      do_qcomplete,   POS_STANDING,    1,  LOG_NORMAL, 0, 0, 0, 0 },

     /* extra description commands */
    { "pull",         do_pull,        POS_STANDING, 1, LOG_NORMAL, 0, 0, 0, 0 },
    { "push",         do_push,        POS_STANDING, 1, LOG_NORMAL, 0, 0, 0, 0 },
    { "press",        do_press,       POS_STANDING, 1, LOG_NORMAL, 0, 0, 0, 0 },
    { "touch",        do_touch,       POS_STANDING, 1, LOG_NORMAL, 0, 0, 0, 0 },

     /* shopping commands */
    { "shop",         do_shop,        POS_STANDING, 1, LOG_NORMAL, 0, 0, 0, 0 },
    { "buy",          do_buy,         POS_STANDING, 1, LOG_NORMAL, 0, 0, 0, 0 },
    { "list",         do_list,        POS_STANDING, 1, LOG_NORMAL, 0, 0, 0, 0 },
    { "auction",      do_auction,     POS_STANDING, 2, LOG_NORMAL, 0, 0, 0, 0 },

     /* alias and other nifty commands */
    { "alias",        do_alias,       POS_DEAD,     2, LOG_NORMAL, 0, 0, 0, 0 },
    { "delalias",     do_delalias,    POS_DEAD,     2, LOG_NORMAL, 0, 0, 0, 0 },
    { "history",      do_history,     POS_DEAD,     1, LOG_NORMAL, 0, 0, 0, 0 },
    { "changes",      do_changes,     POS_DEAD,     1, LOG_NORMAL, 0, 0, 0, 0 },
    { "news",         do_changes,     POS_DEAD,     1, LOG_NORMAL, 0, 0, 0, 0 },
    { "evolve",       do_evolve,      POS_FIGHTING, 3, LOG_NORMAL, 0, 0 ,0, 0 },

    /*
     * more commands.
     */

    { "ragnarok",       do_ragnarok,     POS_STANDING,     3,  LOG_NORMAL, 0, 0, 0, 0 },
    { "showsilence",    do_showsilence,  POS_DEAD,        12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "pathfind",       do_pathfind,     POS_DEAD,        12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "addchange",      do_addchange,    POS_STANDING,    12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "delchange",      do_delchange,    POS_STANDING,    12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "displayvotes",   do_displayvotes, POS_STANDING,    12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "vote",           do_vote,         POS_STANDING,     3,  LOG_NORMAL, 0, 0, 0, 0 },
    { "addpoll",        do_addpoll,      POS_STANDING,    12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "polledit",       do_polledit,     POS_STANDING,    12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "stoppoll",       do_stoppoll,     POS_STANDING,    12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "ccenter",        do_ccenter,      POS_DEAD,        12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "showcomp",       do_showcompress, POS_DEAD,        12,  LOG_NORMAL, 0, 0, 0, 0 }, 
    { "trust",		do_trust,	 POS_DEAD,	  12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "allow",		do_allow,	 POS_DEAD,	  12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "clearstats",	do_clearstats,	 POS_STANDING,	   0,  LOG_NORMAL, 0, 0, 0, 0 },
    { "disenchant",     do_disenchant,   POS_SLEEPING,     2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "create",		do_create,	 POS_STANDING,	  12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "deny",	        do_deny,	 POS_DEAD,	   9,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "disable",	do_disable,	 POS_DEAD,	  10,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "disconnect",	do_disconnect,	 POS_DEAD,	  10,  LOG_NEVER,  0, 0, 0, 0 },
    { "divorce",	do_divorce,	 POS_DEAD,	   7,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "marry",		do_marry,	 POS_DEAD,	   7,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "paradox",	do_paradox,	 POS_DEAD,	  12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "noset",          do_noset,        POS_DEAD,        12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "copyover",	do_copyover,	 POS_DEAD,	  12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "shutdow",	do_shutdow,	 POS_DEAD,	  12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "shutdown",	do_shutdown,     POS_DEAD,	  12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "users",		do_users,	 POS_DEAD,	   8,  LOG_NORMAL, 0, 0, 0, 0 },
    { "omni",		do_omni,	 POS_DEAD,	  10,  LOG_NORMAL, 0, 0, 0, 0 },
    { "wizlock",	do_wizlock,	 POS_DEAD,	  12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "force",		do_force,	 POS_DEAD,	  10,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "asperson",       do_asperson,     POS_DEAD,        12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "exlist",		do_exlist,	 POS_DEAD,	   8,  LOG_NORMAL, 0, 0, 0, 0 },
    { "mload",		do_mload,	 POS_DEAD,	   7,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "undeny",         do_undeny,  	 POS_DEAD,         9,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "mset",		do_mset,	 POS_DEAD,	  10,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "kedit",          do_kedit,        POS_DEAD,        10,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "oclone",		do_oclone,	 POS_DEAD,	  12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "oload",		do_oload,	 POS_DEAD,	   7,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "oset",		do_oset,	 POS_DEAD,	  12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "otransfer",	do_otransfer,    POS_DEAD,	   7,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "pload",		do_pload,	 POS_DEAD,	  12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "pset",		do_pset,	 POS_DEAD,	  10,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "pstat",          do_pstat,        POS_DEAD,         9,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "purge",		do_purge,	 POS_DEAD,	   8,  LOG_NORMAL, 0, 0, 0, 0 },
    { "restore",	do_restore,	 POS_DEAD,	   9,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "rset",		do_rset,	 POS_DEAD,	  12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "silence",	do_silence,	 POS_DEAD,	   8,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "decapitate",	do_decapitate,   POS_STANDING,	   3,  LOG_NORMAL, 0, 0, 0, 0 },
    { "tackle",         do_tackle,       POS_FIGHTING,     2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "sset",		do_sset,	 POS_DEAD,	  12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "transport",	do_transport,    POS_DEAD,	   1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "at",		do_at,	         POS_DEAD,	   7,  LOG_NORMAL, 0, 0, 0, 0 },
    { "bamfin",		do_bamfin,	 POS_DEAD,	   7,  LOG_NORMAL, 0, 0, 0, 0 },
    { "bamfout",	do_bamfout,	 POS_DEAD,	   7,  LOG_NORMAL, 0, 0, 0, 0 },
    { "echo",		do_echo,	 POS_DEAD,	   7,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "goto",		do_goto,	 POS_DEAD,	   7,  LOG_NORMAL, 0, 0, 0, 0 },
    { "holylight",	do_holylight,    POS_DEAD,	   7,  LOG_NORMAL, 0, 0, 0, 0 },
    { "invis",		do_invis,	 POS_DEAD,	   8,  LOG_NORMAL, 0, 0, 0, 0 },
    { "log",		do_log,	         POS_DEAD,	  12,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "logstat",        do_logstat,      POS_DEAD,        12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "reimb",          do_reimb,        POS_DEAD,         2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "wizallow",       do_wizallow,     POS_DEAD,        12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "logstatclear",   do_logstatclear, POS_DEAD,        12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "memusage",	do_memory, 	 POS_DEAD,	  12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "mfind",		do_mfind,	 POS_DEAD,	   7,  LOG_NORMAL, 0, 0, 0, 0 },
    { "mstat",		do_mstat,	 POS_DEAD,	   7,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "mwhere",		do_mwhere,	 POS_DEAD,	   8,  LOG_NORMAL, 0, 0, 0, 0 },
    { "ofind",		do_ofind,	 POS_DEAD,	   7,  LOG_NORMAL, 0, 0, 0, 0 },
    { "ostat",		do_ostat,	 POS_DEAD,	   7,  LOG_NORMAL, 0, 0, 0, 0 },
    { "peace",		do_peace,	 POS_DEAD,	  12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "recho",		do_recho,	 POS_DEAD,	  10,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "rstat",		do_rstat,	 POS_DEAD,	   8,  LOG_NORMAL, 0, 0, 0, 0 },
    { "slookup",	do_slookup,	 POS_DEAD,	   8,  LOG_NORMAL, 0, 0, 0, 0 },
    { "snoop",		do_snoop,	 POS_DEAD,	  10,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "plist",          do_plist,        POS_DEAD,         9,  LOG_ALWAYS, 0, 0, 0, 0 },
    { "immune",		do_immune,	 POS_DEAD,	   1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "immtalk",	do_immtalk,	 POS_DEAD,	   7,  LOG_NORMAL, 0, 0, 0, 0 },

    { "leader",       do_leader,        POS_DEAD,          2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "top10",        do_top10,         POS_DEAD,          2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kingdoms",     do_kingdoms,      POS_DEAD,          2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kuninvite",    do_kuninvite,     POS_DEAD,          2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "koutcast",     do_koutcast,      POS_DEAD,          2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kset",         do_kset,          POS_DEAD,          2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "ktalk",        do_ktalk,         POS_DEAD,          2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kbuy",         do_kbuy,          POS_STANDING,      2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kreload",      do_kreload,       POS_STANDING,      3,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kballista",    do_kballista,     POS_FIGHTING,      3,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kmix",         do_kmix,          POS_STANDING,      2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kjoin",        do_kjoin,         POS_DEAD,          2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kdecline",     do_kdecline,      POS_DEAD,          2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kinvite",      do_kinvite,       POS_DEAD,          2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kinfo",        do_kinfo,         POS_DEAD,          2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "klist",        do_klist,         POS_DEAD,          2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kwho",         do_kwho,          POS_DEAD,          2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kquest",       do_kquest,        POS_STANDING,      2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kdonate",      do_kdonate,       POS_DEAD,          2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "kleave",       do_kleave,        POS_DEAD,          2,  LOG_NORMAL, 0, 0, 0, 0 },
    { "leaderclear",  do_leaderclear,   POS_DEAD,         12,  LOG_NORMAL, 0, 0, 0, 0 },

    /* newbie banning and unbanning */
    { "newban",       do_newban,        POS_DEAD,         10,  LOG_NORMAL, 0, 0, 0, 0 },
    { "newallow",     do_newallow,      POS_DEAD,         10,  LOG_NORMAL, 0, 0, 0, 0 },

    /* Arena stuff */
    { "resign",       do_resign,        POS_STANDING,   3, LOG_NORMAL, 0, 0, 0, 0 },
    { "arenastats",   do_arenastats,    POS_STANDING,   2, LOG_NORMAL, 0, 0, 0, 0 },
    { "teamjoin",     do_teamjoin,      POS_STANDING,   3, LOG_NORMAL, 0, 0, 0, 0 },
    { "arenajoin",    do_arenajoin,     POS_STANDING,   3, LOG_NORMAL, 0, 0, 0, 0 },
    { "spectate",     do_spectate,      POS_STANDING,   2, LOG_NORMAL, 0, 0, 0, 0 },

    { "fortressstats", do_fortressstats, POS_STANDING, 2, LOG_NORMAL, 0, 0, 0, 0 },
    { "challenge",     do_challenge,     POS_STANDING, 3, LOG_NORMAL, 0, 0, 0, 0 },
    { "decline",       do_decline,       POS_STANDING, 3, LOG_NORMAL, 0, 0, 0, 0 },
    { "agree",         do_accept2,       POS_STANDING, 3, LOG_NORMAL, 0, 0, 0, 0 },

    { "ignore",  do_ignore,  POS_DEAD,     2, LOG_NORMAL, 0, 0, 0, 0 },
    { "policy",  do_policy,  POS_STANDING, 2, LOG_NEVER,  0, 0, 0, 0 },
    { "favor",   do_favor,   POS_STANDING, 2, LOG_NORMAL, 0, 0, 0, 0 },

    { "setavatar",   do_setavatar,   POS_STANDING,   3,  LOG_NORMAL, 0, 0, 0, 0 },
    { "setdecap",    do_setdecap,    POS_STANDING,   3,  LOG_NORMAL, 0, 0, 0, 0 },
    { "setlogout",   do_setlogout,   POS_STANDING,   3,  LOG_NORMAL, 0, 0, 0, 0 },
    { "setlogin",    do_setlogin,    POS_STANDING,   3,  LOG_NORMAL, 0, 0, 0, 0 },
    { "settie",      do_settie,      POS_STANDING,   3,  LOG_NORMAL, 0, 0, 0, 0 },

    /*
     * OLC 1.1b and Social Edit
     */

    { "aedit",          do_aedit,       POS_DEAD,   12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "redit",          do_redit,       POS_DEAD,   12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "oedit",          do_oedit,       POS_DEAD,   12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "medit",          do_medit,       POS_DEAD,   12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "asave",          do_asave,       POS_DEAD,   12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "alist",          do_alist,       POS_DEAD,    9,  LOG_NORMAL, 0, 0, 0, 0,},
    { "resets",         do_resets,      POS_DEAD,   12,  LOG_NORMAL, 0, 0, 0, 0 },
    { "sedit",          do_sedit,       POS_DEAD,   12,  LOG_NORMAL, 0, 0, 0, 0 },

    /* this is commands for shadows */
    { "moonstrike",     do_moonstrike,     POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_ATTACK,     1,                 CP_BIT       },
    { "shadowthrust",   do_shadowthrust,   POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_ATTACK,     2,                 CP_BIT       },
    { "dirtthrow",      do_dirtthrow,      POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_ATTACK,     4,                 CP_BIT       },
    { "gutcutter",      do_gutcutter,      POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_ATTACK,     8,                 CP_BIT       },
    { "soulreaper",     do_soulreaper,     POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_ATTACK,     16,                CP_BIT       },
    { "knifespin",      do_knifespin,      POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_ATTACK,     32,                CP_BIT       },
    { "wakasashislice", do_wakasashislice, POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_ATTACK,     64,                CP_BIT       },
    { "caltrops",       do_caltrops,       POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_ATTACK,     128,               CP_BIT       },
    { "shadowlearn",    do_shadowlearn,    POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    0,                 0,                 0            },
    { "soulseek",       do_soulseek,       POS_STANDING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_POWERS,     512,               CP_BIT       },
    { "soultarget",     do_soultarget,     POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_POWERS,     512,               CP_BIT       },
    { "shadowtalk",     do_shadowtalk,     POS_DEAD,      3,  LOG_NORMAL,
       CLASS_SHADOW,    0,                 0,                 0            },
    { "supkeep",        do_supkeep,        POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    0,                 0,                 0            },
    { "shadowedge",     do_shadowedge,     POS_STANDING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_POWERS,     NSHADOWS_EDGE,     CP_BIT       },
    { "bloodenhance",   do_bloodenhance,   POS_STANDING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_POWERS,     NSHADOWS_BLOOD,    CP_BIT       },
    { "shadowportal",   do_shadowportal,   POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_POWERS,     NSHADOWS_PORTAL,   CP_BIT       },
    { "shadowveil",     do_shadowveil,     POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_POWERS,     NSHADOWS_VEIL,     CP_BIT       },
    { "scry",           do_scry,           POS_STANDING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_POWERS,     128,               CP_BIT       },
    { "vanish",         do_vanish,         POS_STANDING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_POWERS,     2,                 CP_BIT       },
    { "aurasight",      do_aurasight,      POS_STANDING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_POWERS,     4,                 CP_BIT       },
    { "shield",         do_shield,         POS_STANDING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_POWERS,     256,               CP_BIT       },
    { "assassinate",    do_assassinate,    POS_STANDING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_POWERS,     1024,              CP_BIT       },
    { "spirits",        do_spirits,        POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_POWERS,     8,                 CP_BIT       },
    { "truesight",      do_truesight,      POS_STANDING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    SHADOW_POWERS,     16,                CP_BIT       },
    { "whirl",          do_whirl,          POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_SHADOW,    0,                 0,                 CP_MASTERY   },

    /* shadow evolve commands */
    { "shadowplane",    do_shadowplane,    POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_1,          SHADOW_EVOLVE_SHADOWPLANE,   CP_BIT       },
    { "confusion",      do_confusion,      POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_1,          SHADOW_EVOLVE_CONFUSION,     CP_BIT       },
    { "tendrils",       do_tendrils,       POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_1,          SHADOW_EVOLVE_TENDRILS,      CP_BIT       },
    { "fumeblast",      do_fumeblast,      POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_1,          SHADOW_EVOLVE_FUMES,         CP_BIT       },
    { "planegrab",      do_planegrab,      POS_STANDING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_2,          SHADOW_EVOLVE_PLANEGRAB,     CP_BIT       },
    { "planeshred",     do_planeshred,     POS_STANDING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_2,          SHADOW_EVOLVE_PLANESHRED,    CP_BIT       },
    { "mindblank",      do_mindblank,      POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_2,          SHADOW_EVOLVE_MINDBLANK,     CP_BIT       },
    { "mindboost",      do_mindboost,      POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_2,          SHADOW_EVOLVE_MINDBOOST,     CP_BIT       },
    { "razorpunch",     do_razorpunch,     POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_2,          SHADOW_EVOLVE_RAZORPUNCH,    CP_BIT       },
    { "bloodtheft",     do_bloodtheft,     POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_2,          SHADOW_EVOLVE_BLOODTHEFT,    CP_BIT       },
    { "blurtendrils",   do_blurtendrils,   POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_2,          SHADOW_EVOLVE_BLURTENDRILS,  CP_BIT       },
    { "acidtendrils",   do_acidtendrils,   POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_2,          SHADOW_EVOLVE_ACIDTENDRILS,  CP_BIT       },
    { "frostblast",     do_frostblast,     POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_2,          SHADOW_EVOLVE_FROSTBLAST,    CP_BIT       },
    { "mirrorimage",    do_mirrorimage,    POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_2,          SHADOW_EVOLVE_MIRROR,        CP_BIT       },
    { "callwitness",    do_callwitness,    POS_STANDING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_3,          SHADOW_EVOLVE_WITNESS,       CP_BIT       },
    { "hthrust",        do_hthrust,        POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_3,          SHADOW_EVOLVE_FEINTS,        CP_BIT       },
    { "dullcut"  ,      do_dullcut,        POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_3,          SHADOW_EVOLVE_FEINTS,        CP_BIT       },
    { "planerift",      do_planerift,      POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_3,          SHADOW_EVOLVE_POWERSHRED,    CP_BIT       },
    { "dullcut"  ,      do_dullcut,        POS_FIGHTING,            3,  LOG_NORMAL,
       CLASS_SHADOW,    EVOLVE_3,          SHADOW_EVOLVE_DTOUCH,        CP_BIT       },

    /*  this it the fae commands  */
    { "energize",       do_energize,       POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_FAE,       0,                 0,                 0            },
    { "faetalk",        do_faetalk,        POS_DEAD,      3,  LOG_NORMAL,
       CLASS_FAE,       0,                 0,                 0            },
    { "truesight",      do_truesight,      POS_STANDING,  3,  LOG_NORMAL,
       CLASS_FAE,       0,                 0,                 0            },
    { "disctrain",      do_disctrain,      POS_STANDING,  3,  LOG_NORMAL,
       CLASS_FAE,       0,                 0,                 0            },
    { "channel",        do_channel,        POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_FAE,       0,                 0,                 0            },
    { "infuse",         do_infuse,         POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_FAE,       0,                 0,                 0            },
    { "ancients",       do_ancients,       POS_STANDING,  3,  LOG_NORMAL,
       CLASS_FAE,       0,                 0,                 0            },
    { "blastbeams",     do_blastbeams,     POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_ARCANE,   1,                 CP_LEVEL     },
    { "reform",         do_reform,         POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_ARCANE,   2,                 CP_LEVEL     },
    { "readaura",       do_readaura,       POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_ARCANE,   3,                 CP_LEVEL     },
    { "vanish",         do_vanish,         POS_STANDING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_ARCANE,   4,                 CP_LEVEL     },
    { "phantom",        do_phantom,        POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_ARCANE,   5,                 CP_LEVEL     },
    { "timewarp",       do_timewarp,       POS_FIGHTING,  3,  LOG_NORMAL,   
       CLASS_FAE,       DISC_FAE_ARCANE,   6,                 CP_LEVEL     },
    { "chaossigil",     do_chaossigil,     POS_FIGHTING,  3,  LOG_NORMAL,   
       CLASS_FAE,       DISC_FAE_ARCANE,   7,                 CP_LEVEL     },
    { "faeblast",       do_faeblast,       POS_STANDING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_ARCANE,   8,                 CP_LEVEL     },
    { "watchfuleye",    do_watchfuleye,    POS_STANDING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_ARCANE,   9,                 CP_LEVEL     },
    { "eyenibble",      do_nibbleeye,      POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_ARCANE,   9,                 CP_LEVEL     },
    { "ghostgauntlets", do_ghostgauntlets, POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_ARCANE,   10,                CP_LEVEL     },
    { "elementalform",  do_elementalform,  POS_STANDING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_NATURE,   1,                 CP_LEVEL     },
    { "scry",           do_scry,           POS_STANDING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_NATURE,   2,                 CP_LEVEL     },
    { "gaseous",        do_gaseous,        POS_STANDING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_NATURE,   3,                 CP_LEVEL     },
    { "martyr",         do_martyr,         POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_NATURE,   4,                 CP_LEVEL     },
    { "spiritkiss",     do_spiritkiss,     POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_NATURE,   5,                 CP_LEVEL     },
    { "spidercall",     do_spidercall,     POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_NATURE,   6,                 CP_LEVEL     },
    { "geyser",         do_geyser,         POS_STANDING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_NATURE,   7,                 CP_LEVEL     },
    { "unleashed",      do_unleashed,      POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_NATURE,   8,                 CP_LEVEL     },
    { "faetune",        do_faetune,        POS_STANDING,  3,  LOG_NORMAL,
       CLASS_FAE,       DISC_FAE_NATURE,   9,                 CP_LEVEL     },
    { "faepipes",       do_faepipes,       POS_STANDING,  3,  LOG_NORMAL,   
       CLASS_FAE,       DISC_FAE_NATURE,   10,                CP_LEVEL     },
    { "will",           do_will,           POS_STANDING,  3,  LOG_NORMAL,
       CLASS_FAE,       FAE_WILL,          8,                 CP_LEVEL     },
    { "matter",         do_matter,         POS_STANDING,  3,  LOG_NORMAL,
       CLASS_FAE,       FAE_MATTER,        8,                 CP_LEVEL     },       
    { "plasma",         do_plasma,         POS_STANDING,  3,  LOG_NORMAL,
       CLASS_FAE,       FAE_PLASMA,        8,                 CP_LEVEL     },       
    { "energy",         do_energy,         POS_STANDING,  3,  LOG_NORMAL,
       CLASS_FAE,       FAE_ENERGY,        8,                 CP_LEVEL     },

    /* fae evolve commands */
    { "dragonbreath",   do_dragonbreath,   POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_FAE,       EVOLVE_1,          FAE_EVOLVE_DRAGON,       CP_BIT       },
    { "bloodsacrifice", do_bloodsacrifice, POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_FAE,       EVOLVE_1,          FAE_EVOLVE_SACRIFICE,    CP_BIT       },
    { "pwall",          do_pwall,          POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_FAE,       EVOLVE_1,          FAE_EVOLVE_WALL,         CP_BIT       },
    { "warding",        do_warding,        POS_STANDING,      3,    LOG_NORMAL,
       CLASS_FAE,       EVOLVE_2,          FAE_EVOLVE_WARDING,      CP_BIT       },
    { "chameleon",      do_chameleon,      POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_FAE,       EVOLVE_2,          FAE_EVOLVE_CHAMELEON,    CP_BIT       },
    { "hspirits",       do_hspirits,       POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_FAE,       EVOLVE_2,          FAE_EVOLVE_HAUNTING,     CP_BIT       },
    { "freezeancients", do_freezeancients, POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_FAE,       EVOLVE_2,          FAE_EVOLVE_FREEZE,       CP_BIT       },
    { "pspray",         do_pspray,         POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_FAE,       EVOLVE_2,          FAE_EVOLVE_PSPRAY,       CP_BIT       },
    { "pchain",         do_pchain,         POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_FAE,       EVOLVE_2,          FAE_EVOLVE_PBLAST,       CP_BIT       },
    { "bloodacid",      do_bloodacid,      POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_FAE,       EVOLVE_2,          FAE_EVOLVE_ACIDBLOOD,    CP_BIT       },
    { "bloodimmune",    do_bloodimmune,    POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_FAE,       EVOLVE_2,          FAE_EVOLVE_BLOODIMMUNE,  CP_BIT       },
    { "halo",           do_halo,           POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_FAE,       EVOLVE_2,          FAE_EVOLVE_HALO,         CP_BIT       },
    { "bloodhunger",    do_bloodhunger,    POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_FAE,       EVOLVE_3,          FAE_EVOLVE_BLOODTASTE,   CP_BIT       },
    { "bloodtrack",     do_bloodtrack,     POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_FAE,       EVOLVE_3,          FAE_EVOLVE_BLOODTASTE,   CP_BIT       },
    { "acidheart",      do_acidheart,      POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_FAE,       EVOLVE_3,          FAE_EVOLVE_ACIDHEART,    CP_BIT       },

    /*  This is the warlock commands  */
    { "warlocktalk",    do_warlocktalk,    POS_DEAD,      3,  LOG_NORMAL,   
       CLASS_WARLOCK,   0,                 0,                 0            },
    { "meditate",       do_meditate,       POS_STANDING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   0,                 0,                 0            },
    { "study",          do_study,          POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   0,                 0,                 0            },
    { "archmages",      do_archmages,      POS_DEAD,      3,  LOG_NORMAL,
       CLASS_WARLOCK,   0,                 0,                 0            },
    { "winit",          do_winit,          POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   0,                 0,                 0            },
    { "wchain",         do_wchain,         POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   0,                 0,                 0            },
    { "wpower",         do_wpower,         POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   0,                 0,                 0            },
    { "wcast",          do_wcast,          POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   0,                 0,                 0            },
    { "wcancel",        do_wcancel,        POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   0,                 0,                 0            },
    { "wtarget",        do_wtarget,        POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   0,                 0,                 0            },
    { "wexclude",       do_wexclude,       POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   0,                 0,                 0            },
    { "wtype",          do_wtype,          POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   0,                 0,                 0            },
    { "wfocus",         do_wfocus,         POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   0,                 0,                 0            },
    { "steelfists",     do_steelfists,     POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_ENCHANTMENT,1,                 CP_LEVEL     },
    { "earthmother",    do_earthmother,    POS_STANDING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_ENCHANTMENT,2,                 CP_LEVEL     },
    { "plague",         do_plague,         POS_STANDING,  3,  LOG_NORMAL,   
       CLASS_WARLOCK,   SPHERE_ENCHANTMENT,3,                 CP_LEVEL     },
    { "milkandhoney",   do_milkandhoney,   POS_STANDING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_ENCHANTMENT,4,                 CP_LEVEL     },
    { "displacement",   do_displacement,   POS_STANDING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_ENCHANTMENT,5,                 CP_LEVEL     },
    { "beacon",         do_beacon,         POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_ENCHANTMENT,6,                 CP_LEVEL     },
    { "corpsedrain",    do_corpsedrain,    POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_NECROMANCY, 1,                 CP_LEVEL     },
    { "lifedrain",      do_lifedrain,      POS_FIGHTING,  3,  LOG_NORMAL,   
       CLASS_WARLOCK,   SPHERE_NECROMANCY, 2,                 CP_LEVEL     },
    { "tarukeye",       do_tarukeye,       POS_FIGHTING,  3,  LOG_NORMAL,   
       CLASS_WARLOCK,   SPHERE_NECROMANCY, 3,                 CP_LEVEL     },
    { "chillbolt",      do_chillbolt,      POS_FIGHTING,  3,  LOG_NORMAL,   
       CLASS_WARLOCK,   SPHERE_NECROMANCY, 4,                 CP_LEVEL     },
    { "deathspell",     do_deathspell,     POS_FIGHTING,  3,  LOG_NORMAL,   
       CLASS_WARLOCK,   SPHERE_NECROMANCY, 5,                 CP_LEVEL     },
    { "simulacrum",     do_simulacrum,     POS_STANDING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_NECROMANCY, 6,                 CP_LEVEL     },
    { "phylactery",     do_phylactery,     POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_NECROMANCY, 6,                 CP_LEVEL     },
    { "flamberge",      do_flamberge,      POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_INVOCATION, 1,                 CP_LEVEL     },
    { "meteorstrike",   do_meteorstrike,   POS_FIGHTING,  3,  LOG_NORMAL,   
       CLASS_WARLOCK,   SPHERE_INVOCATION, 2,                 CP_LEVEL     },
    { "pvipers",        do_pvipers,        POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_INVOCATION, 3,                 CP_LEVEL     },
    { "flamestorm",     do_flamestorm,     POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_INVOCATION, 4,                 CP_LEVEL     },
    { "meteorswarm",    do_meteorswarm,    POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_INVOCATION, 5,                 CP_LEVEL     },
    { "implode",        do_implode,        POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_INVOCATION, 6,                 CP_LEVEL     },
    { "binding",        do_bindingvines,   POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_SUMMONING,  1,                 CP_LEVEL     },
    { "pentagram",      do_pentagram,      POS_STANDING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_SUMMONING,  2,                 CP_LEVEL     },
    { "leeches",        do_leeches,        POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_SUMMONING,  3,                 CP_LEVEL     },
    { "crows",          do_crows,          POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_SUMMONING,  4,                 CP_LEVEL    },
    { "callwild",       do_callwild,       POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_SUMMONING,  5,                 CP_LEVEL     },
    { "truesight",      do_truesight,      POS_STANDING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_DIVINATION, 1,                 CP_LEVEL     },
    { "readaura",       do_readaura,       POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_DIVINATION, 2,                 CP_LEVEL     },
    { "warscry",        do_warscry,        POS_STANDING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_DIVINATION, 3,                 CP_LEVEL     },
    { "paradisebirds",  do_paradisebirds,  POS_STANDING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_DIVINATION, 4,                 CP_LEVEL     },
    { "truthtell",      do_truthtell,      POS_STANDING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_DIVINATION, 5,                 CP_LEVEL     },
    { "eyespy",         do_eyespy,         POS_STANDING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_DIVINATION, 6,                 CP_LEVEL     },
    { "magicvest",      do_magicvest,      POS_STANDING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_ABJURATION, 1,                 CP_LEVEL     },
    { "wallofswords",   do_wallofswords,   POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_ABJURATION, 2,                 CP_LEVEL     },
    { "fireshield",     do_fireshield,     POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_ABJURATION, 3,                 CP_LEVEL     },
    { "shattershield",  do_shattershield,  POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_ABJURATION, 4,                 CP_LEVEL     },
    { "catalyst",       do_catalyst,       POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_ABJURATION, 5,                 CP_LEVEL     },
    { "catalyze",       do_catalyze,       POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_ABJURATION, 5,                 CP_LEVEL     },
    { "ironmind",       do_ironmind,       POS_STANDING,  3,  LOG_NORMAL,
       CLASS_WARLOCK,   SPHERE_ABJURATION, 6,                 CP_LEVEL     },

    /* warlock evolve commands */
    { "contingency",    do_contingency,    POS_STANDING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_1,          WARLOCK_EVOLVE_CONTINGENCY,  CP_BIT       },
    { "homingdevice",   do_homingdevice,   POS_FIGHTING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_1,          WARLOCK_EVOLVE_HOMING,       CP_BIT       },
    { "lockroom",       do_lockroom,       POS_FIGHTING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_1,          WARLOCK_EVOLVE_SPACE,        CP_BIT       },
    { "foldspace",      do_foldspace,      POS_FIGHTING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_1,          WARLOCK_EVOLVE_SPACE,        CP_BIT       },
    { "doombolt",       do_doombolt,       POS_FIGHTING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_1,          WARLOCK_EVOLVE_DOOMBOLT,     CP_BIT       },
    { "mking",          do_mking,          POS_FIGHTING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_1,          WARLOCK_EVOLVE_MOUNTAIN,     CP_BIT       },
    { "sunset",         do_sunset,         POS_FIGHTING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_2,          WARLOCK_EVOLVE_OLDAGE,       CP_BIT       },
    { "timetrip",       do_timetrip,       POS_FIGHTING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_2,          WARLOCK_EVOLVE_TIMETRAVEL,   CP_BIT       },
    { "wglimmer",       do_wglimmer,       POS_FIGHTING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_2,          WARLOCK_EVOLVE_GLIMMER,      CP_BIT       },
    { "backlash",       do_backlash,       POS_FIGHTING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_2,          WARLOCK_EVOLVE_BACKLASH,     CP_BIT       },
    { "hstars",         do_hstars,         POS_FIGHTING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_2,          WARLOCK_EVOLVE_HUNTINGSTARS, CP_BIT       },
    { "precogntion",    do_precognition,   POS_FIGHTING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_2,          WARLOCK_EVOLVE_PRECOGNITION, CP_BIT       },
    { "stitches",       do_stitches,       POS_FIGHTING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_2,          WARLOCK_EVOLVE_STITCHES,     CP_BIT       },
    { "disjunction",    do_disjunction,    POS_FIGHTING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_2,          WARLOCK_EVOLVE_DISJUNCTION,  CP_BIT       },
    { "doomcharge",     do_doomcharge,     POS_STANDING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_2,          WARLOCK_EVOLVE_DOOMCHARGE,   CP_BIT       },
    { "mummyrot",       do_mummyrot,       POS_FIGHTING,      3,        LOG_NORMAL,
       CLASS_WARLOCK,   EVOLVE_3,          WARLOCK_EVOLVE_DECAYCHAIN,   CP_BIT       },

    /* Giant Powers */
    { "truesight",      do_truesight,      POS_STANDING,  3,  LOG_NORMAL,
       CLASS_GIANT,     0,                 0,                 0            },
    { "growth",         do_grow,           POS_STANDING,  3,  LOG_NORMAL,
       CLASS_GIANT,     0,                 0,                 0            },
    { "smack",          do_smack,          POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_GIANT,     0,                 0,                 0            },
    { "thwack",         do_thwack,         POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_GIANT,     0,                 0,                 0            },
    { "bash",           do_bash,           POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_GIANT,     0,                 0,                 0            },
    { "crush",          do_crush,          POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_GIANT,     0,                 0,                 CP_MASTERY   },
    { "dawnstrength",   do_dawnstrength,   POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_GIANT,     GIANT_RANK,        1,                 CP_LEVEL     },
    { "scry",           do_scry,           POS_STANDING,  3,  LOG_NORMAL,
       CLASS_GIANT,     GIANT_RANK,        4,                 CP_LEVEL     },
    { "giantgift",      do_giantgift,      POS_STANDING,  3,  LOG_NORMAL,
       CLASS_GIANT,     0,                 0,                 0            },
    { "earthswallow",   do_earthswallow,   POS_STANDING,  3,  LOG_NORMAL,
       CLASS_GIANT,     GIANT_RANK,        3,                 CP_LEVEL     },
    { "battletrain",    do_battletrain,    POS_STANDING,  3,  LOG_NORMAL,
       CLASS_GIANT,     0,                 0,                 0            },
    { "sweep",          do_gsweep,         POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_GIANT,     GIANT_RANK,        2,                 CP_LEVEL     },
    { "standfirm",      do_standfirm,      POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_GIANT,     GIANT_GIFTS,       GGIFT_STANDFIRM,   CP_BIT       },
    { "revival",        do_revival,        POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_GIANT,     GIANT_GIFTS,       GGIFT_REVIVAL,     CP_BIT       },
    { "stoneshape",     do_stoneshape,     POS_STANDING,  3,  LOG_NORMAL,
       CLASS_GIANT,     GIANT_GIFTS,       GGIFT_STONESHAPE,  CP_BIT       },
    { "earthpunch",     do_earthpunch,     POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_GIANT,     GIANT_GIFTS,       GGIFT_EARTHPUNCH,  CP_BIT       },
    { "rumble",         do_rumble,         POS_DEAD,      3,  LOG_NORMAL,
       CLASS_GIANT,     0,                 0,                 0            },
    { "deathfrenzy",    do_deathfrenzy,    POS_FIGHTING,  3,  LOG_NORMAL,
       CLASS_GIANT,     GIANT_RANK,        5,                 CP_LEVEL     },

    /* giant evolves */
    { "heatmetal",      do_heatmetal,      POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_1,          GIANT_EVOLVE_FIRE,       CP_BIT       },
    { "mudform",        do_mudform,        POS_STANDING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_1,          GIANT_EVOLVE_EARTH,      CP_BIT       },
    { "gustwind",       do_gustwind,       POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_1,          GIANT_EVOLVE_WIND,       CP_BIT       },
    { "waterstream",    do_waterstream,    POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_1,          GIANT_EVOLVE_WATER,      CP_BIT       },
    { "hastespell",     do_hastespell,     POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_2,          GIANT_EVOLVE_HASTE,      CP_BIT       },
    { "slowspell",      do_slowspell,      POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_2,          GIANT_EVOLVE_SLOW,       CP_BIT       },
    { "chopattack",     do_chopattack,     POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_2,          GIANT_EVOLVE_CHOP,       CP_BIT       },
    { "mallet",         do_mallet,         POS_STANDING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_2,          GIANT_EVOLVE_MALLET,     CP_BIT       },
    { "ignitemetal",    do_ignitemetal,    POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_2,          GIANT_EVOLVE_IGNITE,     CP_BIT       },
    { "burnmetal",      do_burnmetal,      POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_2,          GIANT_EVOLVE_IGNITE,     CP_BIT       },
    { "windwalk",       do_windwalk,       POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_2,          GIANT_EVOLVE_WINDWALK,   CP_BIT       },
    { "whirlwind",      do_whirlwind,      POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_2,          GIANT_EVOLVE_WHIRLWIND,  CP_BIT       },
    { "waterflux",      do_waterflux,      POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_2,          GIANT_EVOLVE_LIQUIFY,    CP_BIT       },
    { "liquify",        do_liquify,        POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_2,          GIANT_EVOLVE_LIQUIFY,    CP_BIT       },
    { "waterdome",      do_waterdome,      POS_STANDING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_2,          GIANT_EVOLVE_WATERDOME,  CP_BIT       },
    { "sinkhole",       do_sinkhole,       POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_2,          GIANT_EVOLVE_SINKHOLE,   CP_BIT       },
    { "earthflux",      do_earthflux,      POS_FIGHTING,      3,    LOG_NORMAL,
       CLASS_GIANT,     EVOLVE_2,          GIANT_EVOLVE_EARTHFLUX,  CP_BIT       },


    { "relearn",        do_relearn,     POS_DEAD,    1,  LOG_NORMAL, 0, 0, 0, 0 },
    { "channels",       do_channels,    POS_DEAD,    1,  LOG_NORMAL, 0, 0, 0, 0 },

    /*
     * End of list.
     */
    { "",		0,		POS_DEAD,	 0,  LOG_NORMAL, 0, 0, 0, 0 }
};


/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_STRING_LENGTH];
  char argu[MAX_STRING_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  char command[MAX_STRING_LENGTH];
  char logline[MAX_STRING_LENGTH];
  char cmd_copy[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  bool found, foundstar = FALSE;
  int cmd, trust, star = 0;
  sh_int col = 0;

  /* due to the way threads has been implanted */
  if (!ch || !ch->in_room)
    return;

  /* freeze command */
  if (!IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE))
  {
    send_to_char("You cannot do anything while frozen!\n\r", ch);
    return;
  }

  /* check for AFK */
  if (!IS_NPC(ch) && IS_SET(ch->extra, EXTRA_AFK) && str_cmp(argument, "afk"))
    do_afk(ch, "");

  if (IS_SET(ch->newbits, NEW_FUMBLE))
  {
    REMOVE_BIT(ch->newbits, NEW_FUMBLE);
    act("$n fumbles around with something.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You fumble, and forget what your about to do.\n\r", ch);
    return;
  }

  sprintf(argu, "%s %s", arg, one_argument(argument, arg));

  /*
   * Strip leading spaces.
   */
  while (isspace(*argument))
    argument++;

  if (argument[0] == '\0')
    return;

  strcpy(cmd_copy, argument);

  /*
   * Grab the command word.
   * Special parsing so ' can be a command,
   * also no spaces needed after punctuation.
   */
  strcpy(logline, argument);
  if (!isalpha(argument[0]) && !isdigit(argument[0]))
  {
    command[0] = argument[0];
    command[1] = '\0';
    argument++;
    while (isspace(*argument))
      argument++;
  }
  else
  {
    argument = one_argument(argument, command);
  }

  /*
   * List all valid commands
   */
  if (command[strlen(command) - 1] == '*')
  {
    command[strlen(command) - 1] = '\0';

    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
    {
      if ((!str_prefix(command, cmd_table[cmd].name) || strlen(command) == 0) && can_interpret(ch, cmd))
      {
        foundstar = TRUE;
        star++;
        sprintf(buf, "%-15s", cmd_table[cmd].name);
        send_to_char(buf, ch);
        if ((++col % 5) == 0)
          send_to_char("\n\r", ch);
      }
    }
    if ((col % 5) != 0 && foundstar)
      send_to_char("\n\r", ch);

    if (foundstar && star != 0)
    {
      sprintf(buf, "\n%d command%s found.\n\r", star, (star > 1) ? "s" : "");
      send_to_char(buf, ch);
    }
    if (!foundstar)
    {
      send_to_char("No commands found.\n\r", ch);
    }
    return;
  }

  /*
   * Look for command in command table.
   */
  found = FALSE;
  trust = get_trust(ch);
  for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
  {
    if (command[0] == cmd_table[cmd].name[0] && !str_prefix(command, cmd_table[cmd].name) &&
       (cmd_table[cmd].race == 0 || cmd_table[cmd].race == ch->class || IS_IMMORTAL(ch)) &&
       (cmd_table[cmd].level <= trust || (!IS_NPC(ch) && is_full_name(cmd_table[cmd].name, ch->pcdata->immcmd))))
    {
      if (IS_EXTRA(ch, TIED_UP))
      {
        if      (!str_cmp( cmd_table[cmd].name, "say"  )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "'"    )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "chat" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "."    )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "yell" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "shout")) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "look" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "call" )) found = TRUE; 
        else if (!str_cmp( cmd_table[cmd].name, "save" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "exits")) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "inventory" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "tell" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "restore" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "order" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "who" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "where" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "relevel" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "safe" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "scan" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "spy"  )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "sleep" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "wake" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "upkeep" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "score" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "immune" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "consent" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "report" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "goto" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "flex" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "wake" )) found = TRUE;
        else if (!str_cmp( cmd_table[cmd].name, "drink" )) found = TRUE;
        else
        {
          send_to_char( "Not while tied up.\n\r", ch );
          if (ch->position > POS_STUNNED)
            act("$n strains against $s bonds.",ch,NULL,NULL,TO_ROOM);
          return;
        }
      }
      found = TRUE;
      break;
    }
  }

  /*
   * Log and snoop.
   */
  if ((!IS_NPC(ch) && IS_SET(ch->act, PLR_LOG))
      || fLogAll
      || cmd_table[cmd].log == LOG_ALWAYS)
  {
    if (!IS_CREATOR(ch) && !IS_NPC(ch) && cmd_table[cmd].log != LOG_NEVER)
    {
      bool deaf = (IS_SET(ch->deaf, CHANNEL_LOG));

      if (!deaf)
        SET_BIT(ch->deaf, CHANNEL_LOG);

      log_string("Log %s: %s", ch->name, logline);

      if (!deaf)
        REMOVE_BIT(ch->deaf, CHANNEL_LOG);
    }
  }

  if (ch->desc != NULL && SizeOfList(ch->desc->snoops) > 0L && cmd_table[cmd].log != LOG_NEVER)
  {
    SNOOP_DATA *snoop;

    pIter = AllocIterator(ch->desc->snoops);
    while ((snoop = (SNOOP_DATA *) NextInList(pIter)) != NULL)
    {
      write_to_descriptor(snoop->snooper, "% ", 2);
      write_to_descriptor(snoop->snooper, logline, 0);
      write_to_descriptor(snoop->snooper, "\n\r", 2);
    }
  }

  if (ch->desc != NULL)
    write_to_buffer(ch->desc, "\n\r", 2);

  if (!found)
  {
    /*
     * Look for command in socials table.
     */
    if (!check_social(ch, command, argument))
    {
      if (!check_xsocial(ch, command, argument))
        send_to_char("Huh?\n\r", ch );
    }
    return;
  }
  else if (check_disabled(&cmd_table[cmd]))
  {
    send_to_char("This command has been temporarily disabled.\n\r", ch );
    return;
  }

  /*
   * Character not in position for command?
   */
  if (ch->position < cmd_table[cmd].position)
  {
    switch(ch->position)
    {
      case POS_DEAD:
        send_to_char("Lie still; you are DEAD.\n\r", ch);
        break;
      case POS_MORTAL:
      case POS_INCAP:
      {
        send_to_char("You are hurt far too bad for that.\n\r", ch);
        break;
      }
      case POS_STUNNED:
      {
        send_to_char("You are too stunned to do that.\n\r", ch);
        break;
      }
      case POS_SLEEPING:
        send_to_char("In your dreams, or what?\n\r", ch);
        break;
      case POS_MEDITATING:
      case POS_SITTING:
      case POS_RESTING:
        send_to_char("Nah... You feel too relaxed...\n\r", ch);
        break;
      case POS_FIGHTING:
        send_to_char("No way!  You are still fighting!\n\r", ch);
        break;
    }
    return;
  }

  /*
   * Log last command.
   */
  if (last_command != NULL)
    free_string(last_command);
  sprintf(command, "%s %s BY %s", cmd_table[cmd].name, argument, ch->name); 
  last_command = str_dup(command);

  /*
   * Dispatch the command.
   */
  cmd_done = FALSE;
  (*cmd_table[cmd].do_fun)(ch, argument);
  cmd_done = TRUE;
}

bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;

    found  = FALSE;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }


    if ( !found )
	return FALSE;

    switch ( ch->position )
    {

    case POS_DEAD:
	send_to_char( "Lie still; you are DEAD.\n\r", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	send_to_char( "You are hurt far too bad for that.\n\r", ch );
	return TRUE;

    case POS_STUNNED:
	send_to_char( "You are too stunned to do that.\n\r", ch );
	return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social_table[cmd].name, "snore" ) )
	    break;
	send_to_char( "In your dreams, or what?\n\r", ch );
	return TRUE;

    }

    one_argument( argument, arg );
    victim = NULL;

    if ( arg[0] == '\0' )
    {
	act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
	act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
    }
    else
    {
	act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
	act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
	act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );

	if ( !IS_NPC(ch) && IS_NPC(victim)
	&&   !IS_AFFECTED(victim, AFF_CHARM)
	&&   IS_AWAKE(victim) )
	{
	    switch ( number_bits( 4 ) )
	    {
	    case 0:
		multi_hit(victim, ch, 1);
		break;

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
		act( social_table[cmd].others_found,
		    victim, NULL, ch, TO_NOTVICT );
		act( social_table[cmd].char_found,
		    victim, NULL, ch, TO_CHAR    );
		act( social_table[cmd].vict_found,
		    victim, NULL, ch, TO_VICT    );
		break;

	    case 9: case 10: case 11: case 12:
		act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
		act( "You slap $N.",  victim, NULL, ch, TO_CHAR    );
		act( "$n slaps you.", victim, NULL, ch, TO_VICT    );
		break;
	    }
	}
    }
    return TRUE;
}



bool check_xsocial( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *partner = NULL;
    int cmd;
    int stage;
    int amount;
    bool is_ok = FALSE;
    bool found = FALSE;
    bool one = FALSE;
    bool two = FALSE;

    if (IS_NPC(ch)) return FALSE;

    for ( cmd = 0; xsocial_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == xsocial_table[cmd].name[0]
	&&   !str_prefix( command, xsocial_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    switch ( ch->position )
    {

    case POS_DEAD:
	send_to_char( "Lie still; you are DEAD.\n\r", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	send_to_char( "You are hurt far too bad for that.\n\r", ch );
	return TRUE;

    case POS_STUNNED:
	send_to_char( "You are too stunned to do that.\n\r", ch );
	return TRUE;

    case POS_SLEEPING:
	send_to_char( "In your dreams, or what?\n\r", ch );
	return TRUE;

    }

    one_argument( argument, arg );
    victim = NULL;

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return TRUE;
    }
    if (IS_NPC(victim))
    {
	send_to_char("You can only perform xsocials on players.\n\r",ch);
	return TRUE;
    }

	if( !str_cmp(ch->name, "") )
	{
		victim->pcdata->partner = ch;
	}
  else if (IS_SET(victim->extra, TIED_UP))
         { victim->pcdata->partner = ch;
         }

    
	if (IS_EXTRA(ch, TIED_UP))
    {
	send_to_char("You wiggle and strain but the ropes only tighten.\n\r",ch);
        act("$n strains helplessly against $m bonds.",ch,NULL,NULL,TO_ROOM);
	return FALSE;
    }
    else if ( arg[0] == '\0' )
    {
	act( xsocial_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
	act( xsocial_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
    else if ( victim == ch )
    {
	act( xsocial_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM );
	act( xsocial_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR );
    }
    else
    {
	if (xsocial_table[cmd].gender == SEX_MALE && ch->sex != SEX_MALE)
	{
	    send_to_char("Only men can perform this type of social.\n\r",ch);
	}
	else if (xsocial_table[cmd].gender == SEX_FEMALE && ch->sex != SEX_FEMALE)
	{
	    send_to_char("Only women can perform this type of social.\n\r",ch);
	}
	else if (xsocial_table[cmd].gender == SEX_MALE && victim->sex != SEX_FEMALE)
	{
	    send_to_char("You can only perform this social on a woman.\n\r",ch);
	}
	else if (xsocial_table[cmd].gender == SEX_FEMALE && victim->sex != SEX_MALE)
	{
	    send_to_char("You can only perform this social on a man.\n\r",ch);
	}
	else if (xsocial_table[cmd].gender == 3 && ch->sex != SEX_FEMALE)
	{
		send_to_char( "Only females may preform this command.\n\r",ch);
	}
	else if (xsocial_table[cmd].gender == 3 && victim->sex != SEX_FEMALE)
	{
		send_to_char( "You can only preform this command on a female.\n\r",ch);
	}
	else if (((partner = victim->pcdata->partner) == NULL || partner != ch))
  	{
         	send_to_char("You cannot perform an xsocial on someone without their consent.\n\r",ch);
	 }
	else if (xsocial_table[cmd].stage == 0 && ch->pcdata->stage[0] < 1
	    && ch->pcdata->stage[2] > 0 && ch->sex == 5)
	    send_to_char("You have not yet recovered from last time!\n\r",ch);
	else if (xsocial_table[cmd].stage == 0 && victim->pcdata->stage[0] < 1
	    && victim->pcdata->stage[2] > 0 && victim->sex == 5)
	    send_to_char("They have not yet recovered from last time!\n\r",ch);
	else if (xsocial_table[cmd].stage > 0 && ch->pcdata->stage[0] < 100)
	    send_to_char("You are not sufficiently aroused.\n\r",ch);
	else if (xsocial_table[cmd].stage > 0 && victim->pcdata->stage[0] < 100)
	    send_to_char("They are not sufficiently aroused.\n\r",ch);
	else if (xsocial_table[cmd].stage > 1 && ch->pcdata->stage[1] < 1)
	    send_to_char("You are not in the right position.\n\r",ch);
	else if (xsocial_table[cmd].stage > 1 && victim->pcdata->stage[1] < 1)
	    send_to_char("They are not in the right position.\n\r",ch);
	else
	{
	    act(xsocial_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT);
	    act(xsocial_table[cmd].char_found,    ch, NULL, victim, TO_CHAR   );
	    act(xsocial_table[cmd].vict_found,    ch, NULL, victim, TO_VICT   );
	    if (xsocial_table[cmd].chance)
	    {
		if (ch->sex == SEX_FEMALE && 
		    !IS_EXTRA(ch, EXTRA_PREGNANT) && number_range(1,3) == 1) 
		make_preg(ch,victim);
		else if (victim->sex == SEX_FEMALE && 
		    !IS_EXTRA(victim, EXTRA_PREGNANT) && 
		    number_range(1,3) == 1) 
		make_preg(victim,ch);
	    }
	    if (!str_prefix(xsocial_table[cmd].name,"x-tie"))
	    {
		SET_BIT(victim->extra, TIED_UP);
	    }
	    if (!str_prefix(xsocial_table[cmd].name,"x-gag"))
	    {
		SET_BIT(victim->extra, GAGGED);
	    }
	    if (!str_prefix(xsocial_table[cmd].name,"x-blindfold"))
	    {
		SET_BIT(victim->extra, BLINDFOLDED);
	    }
	    if (xsocial_table[cmd].stage == 1)
	    {
		ch->pcdata->stage[1] = xsocial_table[cmd].position;
		victim->pcdata->stage[1] = xsocial_table[cmd].position;
		if (!IS_SET(ch->extra, EXTRA_DONE))
		{
		    SET_BIT(ch->extra, EXTRA_DONE);
		    if (ch->sex == SEX_FEMALE)
		    {
			act("You feel $n bleed as you enter $m.",ch,NULL,victim,TO_VICT);
			act("You feel yourself bleed as $N enters you.",ch,NULL,victim,TO_CHAR);
			ch->in_room->blood += 1;
		    }
		}
		if (!IS_SET(victim->extra, EXTRA_DONE))
		{
		    SET_BIT(victim->extra, EXTRA_DONE);
		    if (victim->sex == SEX_FEMALE)
		    {
			act("You feel $N bleed as you enter $M.",ch,NULL,victim,TO_CHAR);
			act("You feel yourself bleed as $n enters you.",ch,NULL,victim,TO_VICT);
			ch->in_room->blood += 1;
		    }
		}
		stage = 2;
	    }
	    else stage = xsocial_table[cmd].stage;
	    if (stage == 2) amount = ch->pcdata->stage[1];
		else amount = 100;
	    if (xsocial_table[cmd].self > 0)
	    {
		is_ok = FALSE;
		if (ch->pcdata->stage[stage] >= amount) is_ok = TRUE;
		ch->pcdata->stage[stage] += xsocial_table[cmd].self;
		if (!is_ok && ch->pcdata->stage[stage] >= amount) 
		{
		    stage_update(ch,victim,stage,xsocial_table[cmd].name);
		    one = TRUE;
		}
	    }
	    if (xsocial_table[cmd].other > 0)
	    {
		is_ok = FALSE;
		if (victim->pcdata->stage[stage] >= amount) is_ok = TRUE;
		victim->pcdata->stage[stage] += xsocial_table[cmd].other;
		if (!is_ok && victim->pcdata->stage[stage] >= amount) 
		{
		    stage_update(victim,ch,stage,xsocial_table[cmd].name);
		    two = TRUE;
		}
	    }
	    if ( one && two )
	    {
		    ch->pcdata->stage[0] = 0;
		    victim->pcdata->stage[0] = 0;
		if (!IS_EXTRA(ch, EXTRA_EXP))
		{
		    send_to_char("Congratulations on achieving a simultanious orgasm!  Receive 100000 exp!\n\r",ch);
		    SET_BIT(ch->extra, EXTRA_EXP);
		    ch->exp += 100000;
		}
		if (!IS_EXTRA(victim, EXTRA_EXP))
		{
		    send_to_char("Congratulations on achieving a simultanious orgasm!  Receive 100000 exp!\n\r",victim);
		    SET_BIT(victim->extra, EXTRA_EXP);
		    victim->exp += 100000;
		}
	    }
	}
    }
    return TRUE;
}

void stage_update( CHAR_DATA *ch, CHAR_DATA *victim, int stage,char *argument )
{
    if (IS_NPC(ch) || IS_NPC(victim)) return;
    if (stage == 0)
    {
	if (ch->sex == SEX_MALE)
	{
	    send_to_char("You get a boner.\n\r",ch);
	    act("You feel $n get a boner.",ch,NULL,victim,TO_VICT);
	    return;
	}
	else if (ch->sex == SEX_FEMALE)
	{
	    send_to_char("You get wet.\n\r",ch);
	    act("You feel $n get wet.",ch,NULL,victim,TO_VICT);
	    return;
	}
    }
    else if (stage == 2)
    {
	if (ch->sex == SEX_MALE)
	{
	    if( str_cmp(argument,"xm-cum")   && str_cmp(argument,"xm-facial") && str_cmp(argument,"xm-canal") &&
		str_cmp(argument,"xm-canal") && str_cmp(argument,"xm-cbreasts") && str_cmp(argument,"xm-chair") &&
		str_cmp(argument,"xm-chand") && str_cmp(argument,"xm-cstomach") && str_cmp(argument,"xf-chands") &&
		str_cmp(argument,"xf-cbreasts") )
	    {
		act("You grit your teeth as you shoot your creamy load inside of $M.",ch,NULL,victim,TO_CHAR);
		act("$n grits his teeth as he shoots his load inside of you.",ch,NULL,victim,TO_VICT);
		act("$n grits his teeth as he shoots a load of cum inside of $N.",ch,NULL,victim,TO_NOTVICT);
	    }
	    ch->pcdata->genes[8] += 1;
	    victim->pcdata->genes[8] += 1;
	    save_char_obj(ch);
	    save_char_obj(victim);
	    if (victim->pcdata->stage[2] < 1 || victim->pcdata->stage[2] >= 250)
	    {
		ch->pcdata->stage[2] = 0;
		if (ch->pcdata->stage[0] >= 200) ch->pcdata->stage[0] -= 100;
	    }
	    else ch->pcdata->stage[2] = 200;
	    if (victim->sex == SEX_FEMALE && 
		!IS_EXTRA(victim, EXTRA_PREGNANT) && number_percent() <= 8) 
	    make_preg(victim,ch);
	    return;
	}
	else if (ch->sex == SEX_FEMALE)
	{
	    if( str_cmp(argument,"xf-cum") && str_cmp(argument,"xf-cface") )
	    {
		act("You whimper as you cum.",ch,NULL,victim,TO_CHAR);
		act("$n whimpers as $e cums.",ch,NULL,victim,TO_ROOM);
	    }
	    if (victim->pcdata->stage[2] < 1 || victim->pcdata->stage[2] >= 250)
	    {
		ch->pcdata->stage[2] = 0;
		if (ch->pcdata->stage[0] >= 200) ch->pcdata->stage[0] -= 100;
	    }
	    else ch->pcdata->stage[2] = 200;
	    return;
	}
    }
    return;
}

void make_preg( CHAR_DATA *mother, CHAR_DATA *father )
{
    char *strtime;
    char buf [MAX_STRING_LENGTH];

    if (IS_NPC(mother) || IS_NPC(father)) return;

  strtime = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
    free_string(mother->pcdata->conception);
    mother->pcdata->conception = str_dup(strtime);
    sprintf(buf,"%s", father->name);
    free_string(mother->pcdata->cparents);
    mother->pcdata->cparents = str_dup(buf);
    SET_BIT(mother->extra, EXTRA_PREGNANT);
    mother->pcdata->genes[0] = (mother->max_hit + father->max_hit) * 0.5;
    mother->pcdata->genes[1] = (mother->max_mana + father->max_mana) * 0.5;
    mother->pcdata->genes[2] = (mother->max_move + father->max_move) * 0.5;
    if (IS_IMMUNE(mother, IMM_SLASH) && IS_IMMUNE(father, IMM_SLASH))
	SET_BIT(mother->pcdata->genes[3], IMM_SLASH);
    if (IS_IMMUNE(mother, IMM_STAB) && IS_IMMUNE(father, IMM_STAB))
	SET_BIT(mother->pcdata->genes[3], IMM_STAB);
    if (IS_IMMUNE(mother, IMM_SMASH) && IS_IMMUNE(father, IMM_SMASH))
	SET_BIT(mother->pcdata->genes[3], IMM_SMASH);
    if (IS_IMMUNE(mother, IMM_ANIMAL) && IS_IMMUNE(father, IMM_ANIMAL))
	SET_BIT(mother->pcdata->genes[3], IMM_ANIMAL);
    if (IS_IMMUNE(mother, IMM_MISC) && IS_IMMUNE(father, IMM_MISC))
	SET_BIT(mother->pcdata->genes[3], IMM_MISC);
    if (IS_IMMUNE(mother, IMM_CHARM) && IS_IMMUNE(father, IMM_CHARM))
	SET_BIT(mother->pcdata->genes[3], IMM_CHARM);
    if (IS_IMMUNE(mother, IMM_HEAT) && IS_IMMUNE(father, IMM_HEAT))
	SET_BIT(mother->pcdata->genes[3], IMM_HEAT);
    if (IS_IMMUNE(mother, IMM_COLD) && IS_IMMUNE(father, IMM_COLD))
	SET_BIT(mother->pcdata->genes[3], IMM_COLD);
    if (IS_IMMUNE(mother, IMM_LIGHTNING) && IS_IMMUNE(father, IMM_LIGHTNING))
	SET_BIT(mother->pcdata->genes[3], IMM_LIGHTNING);
    if (IS_IMMUNE(mother, IMM_ACID) && IS_IMMUNE(father, IMM_ACID))
	SET_BIT(mother->pcdata->genes[3], IMM_ACID);
    if (IS_IMMUNE(mother, IMM_HURL) && IS_IMMUNE(father, IMM_HURL))
	SET_BIT(mother->pcdata->genes[3], IMM_HURL);
    if (IS_IMMUNE(mother, IMM_BACKSTAB) && IS_IMMUNE(father, IMM_BACKSTAB))
	SET_BIT(mother->pcdata->genes[3], IMM_BACKSTAB);
    if (IS_IMMUNE(mother, IMM_KICK) && IS_IMMUNE(father, IMM_KICK))
	SET_BIT(mother->pcdata->genes[3], IMM_KICK);
    if (IS_IMMUNE(mother, IMM_DISARM) && IS_IMMUNE(father, IMM_DISARM))
	SET_BIT(mother->pcdata->genes[3], IMM_DISARM);
    if (IS_IMMUNE(mother, IMM_STEAL) && IS_IMMUNE(father, IMM_STEAL))
	SET_BIT(mother->pcdata->genes[3], IMM_STEAL);
    if (IS_IMMUNE(mother, IMM_DRAIN) && IS_IMMUNE(father, IMM_DRAIN))
	SET_BIT(mother->pcdata->genes[3], IMM_DRAIN);
    mother->pcdata->genes[4] = number_range(1,2);
}

/*
 * Return true if an argument is completely numeric.
 */
bool is_number( char *arg )
{
    if ( *arg == '\0' )
	return FALSE;

    for ( ; *arg != '\0'; arg++ )
    {
	if ( !isdigit(*arg) )
	    return FALSE;
    }

    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/*
 * Check if that command is disabled 
 * Note that we check for equivalence of the do_fun pointers; this means
 * that disabling 'chat' will also disable the '.' command
 */
bool check_disabled(const struct cmd_type *command)
{
  DISABLED_DATA *p;
  ITERATOR *pIter;

  pIter = AllocIterator(disabled_list);
  while ((p = (DISABLED_DATA *) NextInList(pIter)) != NULL)
  {
    if (p->command->do_fun == command->do_fun)
      return TRUE;
  }

  return FALSE;
}

/* Load disabled commands */
void load_disabled()
{
  FILE *fp;
  DISABLED_DATA *p;
  char *name;
  int i;

  disabled_list = AllocList();

  if ((fp = fopen(DISABLED_FILE, "r")) == NULL) /* No disabled file.. no disabled commands : */
    return;
  name = fread_word (fp);
  while (str_cmp(name, END_MARKER)) /* as long as name is NOT END_MARKER :) */
  {
    /* Find the command in the table */
    for (i = 0; cmd_table[i].name[0] ; i++)
      if (!str_cmp(cmd_table[i].name, name))
        break;

    if (!cmd_table[i].name[0]) /* command does not exist? */
    {
      bug("Skipping uknown command in " DISABLED_FILE " file.",0);
      fread_number(fp); /* level */
      fread_word(fp); /* disabled_by */
    }
    else /* add new disabled command */
    {
      p = calloc(1, sizeof(DISABLED_DATA));
      p->command = &cmd_table[i];
      p->level = fread_number(fp);
      p->disabled_by = str_dup(fread_word(fp)); 

      AttachToList(p, disabled_list);
    }
    name = fread_word(fp);
  }
  fclose (fp);		
}

/* Save disabled commands */
void save_disabled()
{
  FILE *fp;
  DISABLED_DATA *p;
  ITERATOR *pIter;

  if (SizeOfList(disabled_list) == 0) /* delete file if no commands are disabled */
  {
    unlink(DISABLED_FILE);
    return;
  }

  if ((fp = fopen(DISABLED_FILE, "w")) == NULL)
  {
    bug("Could not open " DISABLED_FILE " for writing", 0);
    return;
  }

  pIter = AllocIterator(disabled_list);
  while ((p = (DISABLED_DATA *) NextInList(pIter)) != NULL)
    fprintf (fp, "%s %d %s\n", p->command->name, p->level, p->disabled_by);

  fprintf (fp, "%s\n",END_MARKER);
  fclose (fp);
}
