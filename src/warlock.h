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

/* warlock header file */
#define CLASS_WARLOCK           4  /* bitvector           */
#define CC_WARLOCK              3  /* not a bitvector     */

/* warlock magic spheres */
#define WARLOCK_RANK            0 /* stored in ch->pcdata->powers[] */
#define SPHERE_NECROMANCY       1 /* stored in ch->pcdata->powers[] */
#define SPHERE_ABJURATION       2 /* stored in ch->pcdata->powers[] */
#define SPHERE_INVOCATION       3 /* stored in ch->pcdata->powers[] */
#define SPHERE_DIVINATION       4 /* stored in ch->pcdata->powers[] */
#define SPHERE_ENCHANTMENT      5 /* stored in ch->pcdata->powers[] */
#define SPHERE_SUMMONING        6 /* stored in ch->pcdata->powers[] */
#define WARLOCK_PATH            7 /* stored in ch->pcdata->powers[] */

/* warlock ranks */
#define WLCK_RNK_APPRENTICE     0 /* WARLOCK_RANK */
#define WLCK_RNK_WARLOCK        1 /* WARLOCK_RANK */
#define WLCK_RNK_ARCHMAGE       2 /* WARLOCK_RANK */

#define PATH_NECROMANCY         1
#define PATH_ABJURATION         2
#define PATH_INVOCATION         3
#define PATH_DIVINATION         4
#define PATH_ENCHANTMENT        5
#define PATH_SUMMONING          6

/* warlock spell types */
#define SPELL_TYPE_HEAL         1
#define SPELL_TYPE_DAMAGE       2
#define SPELL_TYPE_AFFECT       3
#define SPELL_TYPE_TRAIN        4
#define SPELL_TYPE_GLIMMER      5

/* warlock spell target types */
#define SPELL_TARGET_ROOM          1
#define SPELL_TARGET_PERSON        2
#define SPELL_TARGET_OBJECT        4
#define SPELL_TARGET_LOCAL         8
#define SPELL_TARGET_GLOBAL       16

/* warlock spell exclude types */
#define SPELL_EXCLUDE_TARGET      32
#define SPELL_EXCLUDE_MOBILES     64
#define SPELL_EXCLUDE_PLAYERS    128
#define SPELL_EXCLUDE_GROUP      256
#define SPELL_EXCLUDE_NONGROUP   512

typedef struct spell_data
{
  int                   flags;
  int                   type;
  int                   power;
  char                * argument;
} SPELL_DATA;

struct warlock_affect
{
  char    * name;
  int       bit;
};

/* level 1 evolves */
#define WARLOCK_EVOLVE_SPACE           1
#define WARLOCK_EVOLVE_TIME            2
#define WARLOCK_EVOLVE_CONTINGENCY     4
#define WARLOCK_EVOLVE_HOMING          8
#define WARLOCK_EVOLVE_DOOMBOLT       16
#define WARLOCK_EVOLVE_MOUNTAIN       32

/* level 2 evolves */
#define WARLOCK_EVOLVE_GLIMMER              1
#define WARLOCK_EVOLVE_OLDAGE               2
#define WARLOCK_EVOLVE_TIMETRAVEL           4
#define WARLOCK_EVOLVE_PLAGUE               8
#define WARLOCK_EVOLVE_BACKLASH            16
#define WARLOCK_EVOLVE_HUNTINGSTARS        32
#define WARLOCK_EVOLVE_PRECOGNITION        64
#define WARLOCK_EVOLVE_STITCHES           128
#define WARLOCK_EVOLVE_DISJUNCTION        256
#define WARLOCK_EVOLVE_EARTHPULSE         512
#define WARLOCK_EVOLVE_DOOMSTORM         1024
#define WARLOCK_EVOLVE_DOOMCHARGE        2048

#define WARLOCK_EVOLVE_DECAYCHAIN           1
#define WARLOCK_EVOLVE_ZOMBIEENDURANCE      2
#define WARLOCK_EVOLVE_XX                   4
#define WARLOCK_EVOLVE_X                    8

struct archmage_type
{
  char     *  player;
  int         active;
};

extern struct archmage_type archmage_list[7];
