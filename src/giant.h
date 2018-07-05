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

/* Headers for Giants */

#define CLASS_GIANT             2  /* bitvector (but could/should be changed)  */
#define CC_GIANT                2  /* not a bitvector                          */

/*
 * ch->pcdata->powers[GIANT_RANK]
 */
#define FOOT_10         1
#define FOOT_15         2
#define FOOT_20		3
#define FOOT_25         4
#define FOOT_30         5

/*
 * ch->pcdata->powers
 */
#define GIANT_ATTACK      1
#define GIANT_ATT         2
#define GIANT_GIFTS       3
#define GGIFTS_GAINED     4
#define GIANT_STANDFIRM   5
#define GIANT_POINTS      6
#define GIANT_DEF         7
#define GIANT_RANK        8

/*
 * bit vector for giant gifts
 */
#define GGIFT_LEATHERSKIN    1
#define GGIFT_REVIVAL        2
#define GGIFT_EARTHPUNCH     4
#define GGIFT_STANDFIRM      8
#define GGIFT_STONESHAPE    16
#define GGIFT_LONGLEGS      32

/* giant evoles 1 */
#define GIANT_EVOLVE_WIND         1
#define GIANT_EVOLVE_EARTH        2
#define GIANT_EVOLVE_FIRE         4
#define GIANT_EVOLVE_WATER        8
#define GIANT_EVOLVE_WARRIOR     16
#define GIANT_EVOLVE_SHAMAN      32

/* giant evolves 2 */
#define GIANT_EVOLVE_HASTE          1
#define GIANT_EVOLVE_SLOW           2
#define GIANT_EVOLVE_CHOP           4
#define GIANT_EVOLVE_DEATHFRENZY    8
#define GIANT_EVOLVE_MALLET        16
#define GIANT_EVOLVE_IGNITE        32
#define GIANT_EVOLVE_WINDWALK      64
#define GIANT_EVOLVE_WHIRLWIND    128
#define GIANT_EVOLVE_LIQUIFY      256
#define GIANT_EVOLVE_WATERDOME    512
#define GIANT_EVOLVE_SINKHOLE    1024
#define GIANT_EVOLVE_EARTHFLUX   2048

/* giant evolves 3 */
#define GIANT_EVOLVE_KABALISTIC      1
#define GIANT_EVOLVE_TORTOISE        2
#define GIANT_EVOLVE_SPECTRAL        4
#define GIANT_EVOLVE_VOODOO          8
