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

/* kingdom structures */
#define KSTRUCT_CAULDRON              1
#define KSTRUCT_BALLISTA              2
#define KSTRUCT_TELEPORT              3

/* kingdom vnums - special */
#define KVNUM_HEALER                  0
#define KVNUM_DUMMY                   1
#define KVNUM_WIZARD                  2
#define KVNUM_GUARD                   3

#define ROOM_VNUM_KINGDOMHALLS    33000   /* this is where kingdom halls start */

/* kingdom vnums - for items */
#define KVNUM_PORTAL                  0

/* kingdom flags */
#define KINGFLAG_WARNED               1
#define KINGFLAG_INACTIVE             2
#define KINGFLAG_ALARM                4
#define KINGFLAG_STRIKE               8
