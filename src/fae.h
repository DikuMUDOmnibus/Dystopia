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

/* fae header file */
#define CLASS_FAE           8  /* bitvector           */
#define CC_FAE              4  /* not a bitvector     */

/* ch->pcdata->powers[] */
#define FAE_VANISH_STRESS   0
#define FAE_PLASMA          1
#define FAE_WILL            2
#define FAE_ENERGY          3
#define FAE_MATTER          4
#define FAE_SHIELD          5
#define DISC_FAE_ARCANE     6
#define DISC_FAE_NATURE     7
#define FAE_BITS            8
#define FAE_PATH            9

/* and here comes the fae_bits */
#define FAE_BLASTBEAMS      1
#define FAE_GASEOUS         2
#define FAE_BLOODHUNGER     4
#define FAE_ACIDHEART       8

/* level 1 fae evolves */
#define FAE_EVOLVE_NATURE           1
#define FAE_EVOLVE_ARCANE           2
#define FAE_EVOLVE_SPIRIT           4
#define FAE_EVOLVE_DRAGON           8
#define FAE_EVOLVE_SACRIFICE       16
#define FAE_EVOLVE_WALL            32

/* level 2 fae evolves */
#define FAE_EVOLVE_WARDING          1
#define FAE_EVOLVE_CHAMELEON        2
#define FAE_EVOLVE_DEFLECT          4
#define FAE_EVOLVE_ABSORB           8
#define FAE_EVOLVE_HAUNTING        16
#define FAE_EVOLVE_BLASTBEAMS      32
#define FAE_EVOLVE_FREEZE          64
#define FAE_EVOLVE_HALO           128
#define FAE_EVOLVE_PSPRAY         256
#define FAE_EVOLVE_PBLAST         512
#define FAE_EVOLVE_ACIDBLOOD     1024
#define FAE_EVOLVE_BLOODIMMUNE   2048

/* level 3 fae evolves */
#define FAE_EVOLVE_BLOODTASTE       1
#define FAE_EVOLVE_ACIDHEART        2
