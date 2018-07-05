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

#include "dystopia.h"

#define  MAX_TYPE_METAL          6
#define  MAX_TYPE_JEWEL          5
#define  MAX_TYPE_ENCHANTMENT   12
#define  MAX_TYPE_SPELL          9
#define  MAX_TYPE_TIPPED         3
#define  MAX_TYPE_WANDSPELL     10

struct type_metal
{
  char   * name;
  int      damroll;
  int      hitroll;
  int      weap;
  int      ac;
  int      value;
};

struct type_spell_wand
{
  char   * name;
  char   * spell;
};

struct type_tipped
{
  char   * name;
  int      charges;
};

struct type_jewel
{
  char   * name;
  int      damroll;
  int      hitroll;   
  int      ac;
  int      value;
};

struct type_spell
{
  char     * name;
  int        damroll;
  int        hitroll;
  int        ac;
  sh_int   * sn_magic;
  int        value;
};

struct type_enchantment
{
  char   * name;
  int      damroll;
  int      hitroll;
  int      ac;
  int      weap;
  int      sn_magic;
  int      value;
};

extern const struct type_metal metal_table[MAX_TYPE_METAL];
extern const struct type_jewel jewel_table[MAX_TYPE_JEWEL];
extern const struct type_spell spell_table[MAX_TYPE_SPELL];
extern const struct type_enchantment enchantment_table[MAX_TYPE_ENCHANTMENT];
extern const struct type_tipped tipped_table[MAX_TYPE_TIPPED];
extern const struct type_spell_wand spell_wand_table[MAX_TYPE_WANDSPELL];
