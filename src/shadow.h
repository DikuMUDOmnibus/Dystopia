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

/*
 * Shadow header file.
 */
#define CLASS_SHADOW               1  /* bitvector (but could/should be changed)  */
#define CC_SHADOW                  1  /* not a bitvector                          */

/*
 * ch->pcdata->powers[x]
 */
#define SHADOW_VANISH_STRESS       0   /* stress factor for being vanished           */
#define SHADOW_COMBO               1   /* What combos have been used ?               */
#define SHADOW_POWERS              2   /* Skills in shadows                          */
#define SHADOW_MARTIAL             3   /* Skills in martial arts                     */
#define SHADOW_ATTACK              4   /* Learned attack types                       */
#define SHADOW_BITS                5   /* Active shadow powers                       */
#define SHADOW_COMBATTICK          6   /* Makes sure combat powers fades             */
#define SHADOW_SOULAMMO            7   /* How many charges left on the soulseekers ? */
#define SHADOW_POWER               8   /* Yes they use class points                  */

/*
 * Combo counter bits
 */
#define NCOMBO_MOONSTRIKE_1         1
#define NCOMBO_MOONSTRIKE_2         2
#define NCOMBO_SHADOWTHRUST_1       4
#define NCOMBO_SHADOWTHRUST_2       8
#define NCOMBO_DIRTTHROW_1         16
#define NCOMBO_DIRTTHROW_2         32
#define NCOMBO_GUTCUTTER_1         64
#define NCOMBO_GUTCUTTER_2        128
#define NCOMBO_SOULREAPER_1       256
#define NCOMBO_SOULREAPER_2       512
#define NCOMBO_KNIFESPIN_1       1024
#define NCOMBO_KNIFESPIN_2       2048
#define NCOMBO_WAKASASHISLICE_1  4096
#define NCOMBO_WAKASASHISLICE_2  8192
#define NCOMBO_CALTROPS_1       16384
#define NCOMBO_CALTROPS_2       32768
#define NCOMBO_WHIRL_1          65536
#define NCOMBO_WHIRL_2         131072
#define NCOMBO_HTHRUST         262144
#define NCOMBO_DULLCUT         524288

/*
 * Skills in shadows (costs class points to buy)
 */
#define NSHADOWS_SILENTWALK        1  /* supersneak (auto)           */
#define NSHADOWS_HIDE              2  /* invis power (vanish)        */
#define NSHADOWS_AURA              4  /* health aura reading         */
#define NSHADOWS_SPIRIT            8  /* damroll/hitroll power       */
#define NSHADOWS_SIGHT            16  /* truesight                   */
#define NSHADOWS_TPACT            32  /* pact for toughness          */
#define NSHADOWS_DPACT            64  /* pact for damage modifier    */
#define NSHADOWS_SCRY            128  /* scrying power               */
#define NSHADOWS_SHIELD          256  /* shadow shield               */
#define NSHADOWS_SOULSEEKERS     512  /* long range attack           */
#define NSHADOWS_ASSASSINATE    1024  /* assassinating power         */
#define NSHADOWS_EDGE           2048  /* shadowedge power            */
#define NSHADOWS_BLOOD          4096  /* bloodenhance power          */
#define NSHADOWS_PORTAL         8192  /* shadowportal power          */
#define NSHADOWS_VEIL          16384  /* shadowveil power            */

/*
 * Learned Attack Types
 */
#define NATTACK_MOONSTRIKE        1  /* must be night          */
#define NATTACK_SHADOWTHRUST      2
#define NATTACK_DIRTTHROW         4
#define NATTACK_GUTCUTTER         8
#define NATTACK_SOULREAPER       16
#define NATTACK_KNIFESPIN        32  /* must be wearing pierce */
#define NATTACK_WAKASASHISLICE   64  /* must be wearing slice  */
#define NATTACK_CALTROPS        128

/*
 * Ninja powers (each power costs class points to maintain, or perhaps something else)
 */
#define NPOWER_KNIFESHIELD        1  /* free attacks                               */
#define NPOWER_BLUR               2  /* blurring effect, negates attacks           */
#define NPOWER_BLOODRAGE          4  /* more attacks/damage, negates at next combo */
#define NPOWER_SHADOWFORM         8  /* high toughness, low damage                 */
#define NPOWER_AURASIGHT         16  /* check health                               */

/* shadowform and bloodrage not possibly together.... */

/* evolve level 1 */
#define SHADOW_EVOLVE_SHADOWPLANE       1
#define SHADOW_EVOLVE_CONFUSION         2
#define SHADOW_EVOLVE_TENDRILS          4
#define SHADOW_EVOLVE_FUMES             8
#define SHADOW_EVOLVE_SKULLDUGGERY     16
#define SHADOW_EVOLVE_ASSASSIN         32

/* evolve level 2 */
#define SHADOW_EVOLVE_PLANEGRAB         1
#define SHADOW_EVOLVE_PLANESHRED        2
#define SHADOW_EVOLVE_MINDBLANK         4
#define SHADOW_EVOLVE_MINDBOOST         8
#define SHADOW_EVOLVE_BLOODTHEFT       16
#define SHADOW_EVOLVE_RAZORPUNCH       32
#define SHADOW_EVOLVE_GAROTTE          64
#define SHADOW_EVOLVE_AWARENESS       128
#define SHADOW_EVOLVE_ACIDTENDRILS    256
#define SHADOW_EVOLVE_BLURTENDRILS    512
#define SHADOW_EVOLVE_FROSTBLAST     1024
#define SHADOW_EVOLVE_MIRROR         2048

/* evolve level 3 */
#define SHADOW_EVOLVE_WITNESS           1
#define SHADOW_EVOLVE_FEINTS            2
#define SHADOW_EVOLVE_POWERSHRED        4
#define SHADOW_EVOLVE_DTOUCH            8
