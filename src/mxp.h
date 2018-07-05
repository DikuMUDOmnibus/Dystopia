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
 * header file for mxp.c
 *
 * Brian Graversen
 */
#include <arpa/telnet.h>

/* MXP defs */
#define MXP_SAFE                        1
#define MXP_ALL                         2
#define MXP_NONE                        3
#define TELOPT_MXP                     91

/* global strings */
extern const unsigned char mxp_do[];
extern const unsigned char mxp_dont[];
extern const unsigned char mxp_will[];
