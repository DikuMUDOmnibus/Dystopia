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
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "dystopia.h"

 /*
  * Liquid properties.
  * Used in world.obj.
  */
const struct liq_type liq_table [LIQ_MAX] =
{
  { "water",             "clear",	{  0, 1, 10 }	},  /*  0 */
  { "beer",              "amber",	{  3, 2,  5 }	},
  { "wine",              "rose",	{  5, 2,  5 }	},
  { "ale",               "brown",	{  2, 2,  5 }	},
  { "dark ale",          "dark",	{  1, 2,  5 }	},
  { "whisky",            "golden",	{  6, 1,  4 }	},  /*  5 */
  { "lemonade",          "pink",	{  0, 1,  8 }	},
  { "firebreather",      "boiling",	{ 10, 0,  0 }	},
  { "local specialty",   "everclear",	{  3, 3,  3 }	},
  { "slime mold juice",  "green",	{  0, 4, -8 }	},
  { "milk",              "white",	{  0, 3,  6 }	},  /* 10 */
  { "tea",               "tan",		{  0, 1,  6 }	},
  { "coffee",            "black",	{  0, 1,  6 }	},
  { "blood",             "red",		{  0, 0,  5 }	},
  { "salt water",        "clear",	{  0, 1, -2 }	},
  { "cola",              "cherry",	{  0, 1,  5 }	},   /* 15 */
};

const struct skill_type skill_table [MAX_SKILL] =
{
 /*  name, level, spell_fun,    spell_type,     position,
  *  gsn,  cost,  casting time, damage message, fade message you,
  *  fade message others, class, field, level/bit, flags
  */

 { "reserved",        99,  0,                    TAR_IGNORE,         POS_STANDING,
    NULL,              0,  0,                    "", "",
   "", 0, 0, 0, 0
 },
 
 { "armor",            1,  spell_armor,          TAR_CHAR_DEFENSIVE, POS_STANDING,
    NULL,              5,  12,                   "", "You feel less protected.",
   "$n seems less protected.", 0, 0, 0, 0
 },
 
 { "bless",            1,  spell_bless,          TAR_CHAR_DEFENSIVE, POS_STANDING,
    NULL,              5,  12,                   "", "You feel less righteous.",
   "$n looks less righteous.", 0, 0, 0, 0
 },

 { "blindness",        1,  spell_blindness,      TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   &gsn_blindness,     5,  12,                   "", "You can see again.",
   "$n's vision is restored.", 0, 0, 0, 0
 },

 { "charm person",     7,  spell_charm_person,   TAR_CHAR_OFFENSIVE, POS_STANDING,
   &gsn_charm_person, 99,  12,                   "", "You feel more self-confident.",
   "$n is back in control of $mself.", 0, 0, 0, 0
 },

 { "chill touch",      2,  spell_chill_touch,    TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,              15,  12,                   "chill touch", "You feel less cold.",
   "$n stops shivering.", 0, 0, 0, 0
 },

 { "cure blindness",   1,  spell_cure_blindness, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL,               5,  12,                   "", "!Cure Blindness!",
   "", 0, 0, 0, 0
 },

 { "cure poison",      1,  spell_cure_poison,    TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,               5,  12,                   "", "!Cure Poison!",
   "", 0, 0, 0, 0
 },

 { "curse",            1,  spell_curse,          TAR_CHAR_OFFENSIVE, POS_STANDING,
   &gsn_curse,        20,  12,                   "curse", "The curse wears off.",
   "$n is no longer cursed.", 0, 0, 0, 0
 },

 { "detect evil",      2,  spell_detect_evil,    TAR_CHAR_SELF, POS_STANDING,
   NULL,               5,  12,                   "", "The red in your vision disappears.",
   "The red shimmer in $n's eyes vanish.", 0, 0, 0, 0
 },

 { "detect hidden",    1,  spell_detect_hidden,	 TAR_CHAR_SELF, POS_STANDING,
   NULL,               5,  12,                   "", "You feel less aware of your suroundings.",
   "The green shimmer in $n's eyes vanish.", 0, 0, 0, 0
 },

 { "detect invis",     1,  spell_detect_invis,   TAR_CHAR_SELF, POS_STANDING,
   NULL,               5,  12,                   "", "You no longer see invisible objects.",
   "$n seems less aware of $s surroundings.", 0, 0, 0, 0
 },

 { "detect magic",     2,  spell_detect_magic,   TAR_CHAR_SELF, POS_STANDING,
   NULL,               5,  12,                   "", "The detect magic wears off.",
   "$n seems less aware of $s surroundings.", 0, 0, 0, 0
 },

 { "detect poison",    2,  spell_detect_poison,  TAR_OBJ_INV, POS_STANDING,
   NULL,               5,  12,                   "", "!Detect Poison!",
   "", 0, 0, 0, 0
 },

 { "dispel evil",      2,  spell_dispel_evil,    TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,              15,  12,                   "dispel evil", "!Dispel Evil!",
   "", 0, 0, 0, 0
 },

 { "dispel magic",     1,  spell_dispel_magic,   TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,              15,  12,                   "", "!Dispel Magic!",
   "", 0, 0, 0, 0
 },

 { "earthquake",       2,  spell_earthquake,     TAR_IGNORE, POS_FIGHTING,
   &gsn_earthquake,   15,  12,                   "earthquake", "!Earthquake!",
   "", 0, 0, 0, 0
 },

 { "enchant weapon",   1,  spell_enchant_weapon, TAR_OBJ_INV, POS_STANDING,
   NULL,             100,  24,                   "", "!Enchant Weapon!",
   "", 0, 0, 0, 0
 },

 { "energy drain",     1,  spell_energy_drain,   TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   &gsn_energydrain,  35,  12,                   "energy drain", "!Energy Drain!",
   "", 0, 0, 0, 0
 },

 { "fire shield",     99,  spell_null,           TAR_IGNORE, POS_FIGHTING,
   &gsn_fireshield,    0,  24,                   "fire shield", "fire shield",
   "", 0, 0, 0, 0
 },

 { "faerie fog",       2,  spell_faerie_fog,     TAR_IGNORE, POS_STANDING,
   NULL,              12,  12,                   "faerie fog", "!Faerie Fog!",
   "", 0, 0, 0, 0
 },

 { "fireball",         1,  spell_fireball,       TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,              15,  12,                   "fireball", "!Fireball!",
   "", 0, 0, 0, 0
 },

 { "fly",              1,  spell_fly,            TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,              10,  0,                    "", "You slowly float to the ground.",
   "$n slowly floats to the ground.", 0, 0, 0, 0
 },

 { "giant strength",   1,  spell_giant_strength, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,              20,  12,                   "", "You feel weaker.",
   "$n looks weaker than before.", 0, 0, 0, 0
 },

 { "harm",             1,  spell_harm,           TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,              35,  12,                   "harm spell", "!Harm!",
   "", 0, 0, 0, 0
 },

 { "heal",             1,  spell_heal,           TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL,              50,  12,                   "", "!Heal!",
   "", 0, 0, 0, 0
 },

 { "identify",         1,  spell_identify,       TAR_OBJ_INV, POS_STANDING,
   NULL,              12,  0,                    "", "!Identify!",
   "", 0, 0, 0, 0
 },

 { "infravision",      1,  spell_infravision,    TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,               5,  18,                   "", "You no longer see in the dark.",
   "$n is no longer able to see in the dark.", 0, 0, 0, 0
 },

 { "invis",            1,  spell_invis,          TAR_CHAR_DEFENSIVE, POS_STANDING,
   &gsn_invis,         5,  12,                   "", "You are no longer invisible.",
   "$n fades into view.", 0, 0, 0, 0
 },

 { "know alignment",   2,  spell_know_alignment, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,               9,  12,                   "", "!Know Alignment!",
   "", 0, 0, 0, 0
 },

 { "locate object",    1,  spell_locate_object,  TAR_IGNORE, POS_STANDING,
   NULL,              20,  18,                   "", "!Locate Object!",
   "", 0, 0, 0, 0
 },

 { "magic missile",    2,  spell_magic_missile,  TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   &gsn_magicmissile, 15,  4,                    "magic missile", "!Magic Missile!",
   "", 0, 0, 0, 0
 },

 { "mass invis",       1,  spell_mass_invis,     TAR_IGNORE, POS_STANDING,
   &gsn_mass_invis,   20,  24,                   "", "!Mass Invis!",
   "", 0, 0, 0, 0
 },

 { "pass door",        1,  spell_pass_door,      TAR_CHAR_SELF, POS_STANDING,
   NULL,              20,  6,                    "", "You feel solid again.",
   "$n looks solid again.", 0, 0, 0, 0
 },

 { "poison",           1,  spell_poison,         TAR_CHAR_OFFENSIVE, POS_STANDING,
   &gsn_poison,       10,  12,                   "poison", "You feel less sick.",
   "$n looks less sick.", 0, 0, 0, 0
 },

 { "protection",       1,  spell_protection,     TAR_CHAR_SELF, POS_STANDING,
   NULL,               5,  12,                   "", "You feel less protected.",
   "$n looks less protected.", 0, 0, 0, 0
 },

 { "readaura",        12,  spell_readaura,       TAR_CHAR_DEFENSIVE, POS_MEDITATING,
   NULL,               1,  1,                    "", "!readaura!",
   "", 0, 0, 0, 0
 },

 { "refresh",          2,  spell_refresh,        TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,              12,  18,                   "refresh", "!Refresh!",
   "", 0, 0, 0, 0
 },

 { "remove curse",     2,  spell_remove_curse,   TAR_IGNORE, POS_STANDING,
   NULL,               5,  4,                    "", "!Remove Curse!",
   "", 0, 0, 0, 0
 },

 { "sanctuary",        1,  spell_sanctuary,      TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,              75,  12,                   "", "The white aura around your body fades.",
   "The white aura around $n's body fades.", 0, 0, 0, 0
 },

 { "shield",           1,  spell_shield,         TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,              12,  18,                   "", "Your force shield shimmers then fades away.",
   "The forcefield around $n shimmers then fades away.", 0, 0, 0, 0
 },

 { "stone skin",       1,  spell_stone_skin,     TAR_CHAR_SELF, POS_STANDING,
   NULL,              12,  12,                   "", "Your skin feels soft again.",
   "$n's skin regains its fleshy colour.", 0, 0, 0, 0
 },

 { "summon",           2,  spell_summon,         TAR_IGNORE, POS_STANDING,
   NULL,              50,  12,                   "",  "!Summon!",
   "", 0, 0, 0, 0
 },

 { "weaken",           2,  spell_weaken,         TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,              20,  12,                   "spell", "You feel stronger.",
   "$n looks much stronger.", 0, 0, 0, 0
 },

 { "word of recall",   9,  spell_word_of_recall, TAR_CHAR_SELF,	POS_RESTING,
   NULL,               5,  12,                   "", "!Word of Recall!",
   "", 0, 0, 0, 0
 },

 { "fire breath",     13,  spell_fire_breath,    TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   &gsn_firebreath,   50,  12,                   "blast of flame", "The smoke leaves your eyes.",
   "The smoke leaes $n's eyes.", 0, 0, 0, 0
 },

 { "gas blast",       13,  spell_gas_blast,      TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   &gsn_gasblast,     50,  12,                   "blast of gas", "The gas leaves your eyes.",
   "The gas leaves $n's eyes.", 0, 0, 0, 0
 },

 { "godbless",        13,  spell_godbless,       TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,               5,  12,                   "", "You feel God's blessing leave you.",
   "God's blessing leaves $n.", 0, 0, 0, 0
 },
 
 { "backstab",         1,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_backstab,      0,  24,                   "backstab", "!Backstab!",
   "", 0, 0, 0, 0
 },

 { "garotte",         99,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_garotte,       0,  24,                   "garotte", "!Garotte!",
   "", 0, 0, 0, 0
 },

 { "disarm",           1,  spell_null,           TAR_IGNORE, POS_FIGHTING,
   &gsn_disarm,        0,  24,                   "", "!Disarm!",
   "", 0, 0, 0, 0
 },

 { "heroism",         99,  spell_heroism,        TAR_CHAR_DEFENSIVE, POS_STANDING,
   &gsn_heroism,       0,  12,                   "", "You feel less heroic.",
   "$n looks less heroic.", 0, 0, 0, 0
 },

 { "hurl",             1,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_hurl,          0,  24,                   "", "!Hurl!",
   "", 0, 0, 0, 0
 },

 { "spirit kiss",     99,  spell_spiritkiss,     TAR_CHAR_DEFENSIVE, POS_STANDING,
   &gsn_spiritkiss,   15,  12,                   "", "Your spirit blessing wears off.",
   "The spirit blessings leaves $n's body.", 0, 0, 0, 0
 },

 { "kick",             1,  spell_null,           TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   &gsn_kick,          0,  8,                    "kick", "!Kick!",
   "", 0, 0, 0, 0
 },

 { "circle",          99,  spell_null,           TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   &gsn_circle,        0,  24,                   "swift circle attack", "!Circle!",
   "", 0, 0, 0, 0
 },

 { "peek",             1,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_peek,          0,  0,                    "", "!Peek!",
   "", 0, 0, 0, 0
 },

 { "pick lock",        1,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_pick_lock,     0,  12,                   "", "!Pick!",
   "", 0, 0, 0, 0
 },

 { "rescue",           1,  spell_null,           TAR_IGNORE, POS_FIGHTING,
   &gsn_rescue,        0,  12,                   "", "!Rescue!",
   "", 0, 0, 0, 0
 },

 { "sneak",            1,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_sneak,         0,  12,                   "", "Your footsteps are no longer so quiet.",
   "$n's footsteps are no longer so quiet.", 0, 0, 0, 0
 },

 { "steal",            1,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_steal,         0,  24,                   "", "!Steal!",
   "", 0, 0, 0, 0
 },

 { "soulblade",        1,  spell_soulblade,      TAR_IGNORE, POS_STANDING,
   NULL,             100,  12,                   "", "!Soulblade!",
   "", 0, 0, 0, 0
 },

 { "mana",             1,  spell_mana,           TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,               0,  12,                   "", "!Mana!",
   "", 0, 0, 0, 0
 },

 { "frenzy",           1,  spell_frenzy,         TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,              20,  12,                   "", "Your bloodlust subsides.",
   "$n's bloodlust subsides.", 0, 0, 0, 0
 },

 { "darkblessing",     1,  spell_darkblessing,   TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,              20,  12,                   "", "You feel less wicked.",
   "$n looks less wicked.", 0, 0, 0, 0
 },

 { "energyflux",       2,  spell_energyflux,     TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,               0,  12,                   "", "!EnergyFlux!",
   "", 0, 0, 0, 0
 },

 { "transport",        1,  spell_transport,      TAR_OBJ_INV, POS_STANDING,
   NULL,              12,  24,                   "", "!Transport!",
   "", 0, 0, 0, 0
 },

 { "regenerate",       1,  spell_regenerate,     TAR_OBJ_INV, POS_STANDING,
   NULL,             100,  12,                   "", "!Regenerate!",
   "", 0, 0, 0, 0
 },

 { "clot",             1,  spell_clot,           TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,              50,  12,                   "", "!Clot!",
   "", 0, 0, 0, 0
 },

 { "mend",             1,  spell_mend,           TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,              50,  12,                   "", "!Mend!",
   "", 0, 0, 0, 0
 },

 { "punch",            1,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_punch,         0,  24,                   "punch", "!Punch!",
   "", 0, 0, 0, 0
 },

 { "elbow",           99,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_elbow,         0, 24,                    "elbow", "!Elbow!",
   "", 0, 0, 0, 0
 },

 { "headbutt",        99,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_headbutt,      0,  24,                   "headbutt", "!Headbutt!",
   "", 0, 0, 0, 0
 },

 { "mount",            2,  spell_mount,          TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,             100,  12,                   "", "!Mount!",
   "", 0, 0, 0, 0
 },

 { "berserk",          2,  spell_null,           TAR_IGNORE, POS_FIGHTING,
   &gsn_berserk,       0,  24,                   "", "!Berserk!",
   "", 0, 0, 0, 0
 },

 { "protection vs good", 1, spell_protection_vs_good, TAR_CHAR_SELF, POS_STANDING,
   NULL,               5,  12,                   "", "You feel less protected.",
   "$n looks less protected.", 0, 0, 0, 0
 },

 { "scan",             1,  spell_null,           TAR_IGNORE, POS_STANDING,
   NULL,               6,  24,                   "", "!Scan!",
   "", 0, 0, 0, 0
 },

 { "repair",           2,  spell_repair,         TAR_IGNORE, POS_STANDING,
   NULL,             100,  24,                   "", "!Repair!",
   "", 0, 0, 0, 0
 },

 { "spellproof",       2,  spell_spellproof,     TAR_OBJ_INV, POS_STANDING,
   NULL,              50,  12,                   "", "!Spellproof!",
   "", 0, 0, 0, 0
 },

 { "sunset age",      13,  spell_null,           TAR_CHAR_OFFENSIVE, POS_STANDING,
   NULL,              12,  24,                   "", "Your youth returns.",
   "$n's grows younger before your eyes.", 0, 0, 0, 0
 },

 { "track",            1,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_track,         0,  0,                    "", "!Track!",
   "", 0, 0, 0, 0
 },

 { "purple sorcery",  99,  spell_null,           TAR_IGNORE, POS_STANDING,
   NULL,             100,  12,                   "", "The purple spell on you fades away.",
   "The purple spell on $n fades away.", 0, 0, 0, 0
 },

 { "red sorcery",     99,  spell_null,           TAR_IGNORE, POS_STANDING,
   NULL,             100,  12,                   "", "The red spell on you fades away.",
   "The red spell on $n fades away.", 0, 0, 0, 0
 },

 { "blue sorcery",    99,  spell_null,           TAR_IGNORE, POS_STANDING,
   NULL,             100,  12,                   "",         "The blue spell on you fades away.",
   "The blue spell on $n fades away.", 0, 0, 0, 0
 },

 { "green sorcery",   99,  spell_null,           TAR_IGNORE, POS_STANDING,
   NULL,             100,  12,                   "", "The green spell on you fades away.",
   "The green spell on $n fades away.", 0, 0, 0, 0
 },

 { "yellow sorcery",  99,  spell_null,           TAR_IGNORE, POS_STANDING,
   NULL,             100,  12,                   "", "The yellow spell on you fades away.",
   "The yellow spell on $n fades away.", 0, 0, 0, 0
 },

 { "chaos blast",     99,  spell_chaos_blast,    TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   &gsn_chaosblast,   20,  12,                   "chaos blast", "!Chaos Blast!",
   "", 0, 0, 0, 0
 },

 { "resistance",       1,  spell_resistance,     TAR_OBJ_INV, POS_STANDING,
   NULL,              50,  12,                   "", "!Resistance!",
   "", 0, 0, 0, 0
 },

 { "desanct",         13,  spell_desanct,        TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,            1500,  12,                   "desanct","!Desanct!",
   "", 0, 0, 0, 0
 },

 { "imp heal",        13,  spell_imp_heal,       TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL,            1500,  12,                   "super heal", "!Super Heal!",
   "", 0, 0, 0, 0
 },

 { "imp fireball",    13,  spell_imp_fireball,   TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   &gsn_impfireball, 1500, 12,                   "super fireball", "!Super Fireball!",
   "", 0, 0, 0, 0
 },

 { "nerve pinch",     99,  spell_nerve_pinch,    TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,            1500,  12,                   "nerve pinch", "Your body's reactions return to normal.",
   "$n regains control of $s nerve system.", 0, 0, 0, 0
 },

 { "imp teleport",    13,  spell_imp_teleport,   TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,            1500,  12,                   "super teleport", "!Super Teleport!",
   "", 0, 0, 0, 0
 },

 { "fireball",        13,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_fireball,      0,  24,                   "fireball", "fireball!",
   "", 0, 0, 0, 0
 },

 { "lightning blast", 13,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_lightning,     0,  24,                   "lightning blast", "lightning!",
   "", 0, 0, 0, 0
 },

 { "group heal",       1,  spell_group_heal,     TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   &gsn_groupheal,    50,  14,                   "", "!Group Heal!",
   "", 0, 0, 0, 0
 },

 { "solar flare",     13,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_nova,          0,  24,                   "solar flare", "solar flare!",
   "", 0, 0, 0, 0
 },

 { "ballista shot",   13,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_ballista,      0,  24,                   "ballista shot", "ballista shot",
   "", 0, 0, 0, 0
 },

 { "knee strike",     99,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_knee,          0,  24,                   "knee strike", "!knee strike!",
   "", 0, 0, 0, 0
 },

 { "leg sweep",       99,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_sweep,         0,  24,                   "leg sweep", "!leg sweep!",
   "", 0, 0, 0, 0
 },

 { "counter spell",    3,  spell_counter_spell,  TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,             100,  4,                    "counter spell", "!Counter Spell!",
   "", 0, 0, 0, 0
 },

 { "ghost gauntlets", 13,  spell_ghostgauntlets, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,               5,  12, "", "The ghost gauntlets floating beside you vanishes in a puff of smoke.",
   "The ghost gauntlets floating besides $n vanishes in a puff of smoke.", 0, 0, 0, 0
 },

 { "mind blank",      13,  spell_mind_blank,     TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,               5,  12,                   "mind blank", "Your mind returns to normal.",
   "$n regains control of his mind.", 0, 0, 0, 0
 },

 { "mind boost",      13,  spell_mind_boost,     TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL,               5,  12,                   "mind boost", "Your mind returns to normal.",
   "$n seems slightly disoriented.", 0, 0, 0, 0
 },

 { "orange sorcery",  99,  spell_null,           TAR_IGNORE, POS_STANDING,
   NULL,             100,  12,                   "", "The orange spell on you fades away.",
   "The orange spell on $n fades away.", 0, 0, 0, 0
 },

 { "slow spell",      99,  spell_slow,           TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,            1500,  12,                   "","You regain your normal speed.",
   "$n stops moving so sluggishly.", 0, 0, 0, 0
 },

 { "haste spell",     99,  spell_haste,          TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL,            1500,  12,                   "", "You regain your normal speed.",
   "$n stops moving so damned fast.", 0, 0, 0, 0
 },

 { "newbiespell",      2,  spell_newbie,         TAR_CHAR_SELF, POS_FIGHTING,
    NULL,             35,  12,                   "", "!nothing!",
    "", 0, 0, 0, 0
 },

 { "permanency",       3,  spell_permanency,     TAR_IGNORE, POS_STANDING,
   NULL,            2000,  8,                    "", "!Permanency!",
   "", 0, 0, 0, 0
 },

 { "omniscience",    99,  spell_omniscience,    TAR_CHAR_DEFENSIVE, POS_STANDING,
   &gsn_omniscience,  0,  12,                   "", "You feel less wise.",
   "$n looks less wise.", 0, 0, 0, 0
 },

 { "endurance",      99,  spell_endurance,      TAR_CHAR_DEFENSIVE, POS_STANDING,
   &gsn_endurance,    0,  12,                   "", "You feel less endurant.",
   "$n looks less endurant.", 0, 0, 0, 0
 },

 { "brilliance",     99,  spell_brilliance,     TAR_CHAR_DEFENSIVE, POS_STANDING,
   &gsn_brilliance,   0,  12,                   "", "You feel less brilliant.",
   "$n seems less brilliant.", 0, 0, 0, 0
 },

 { "nimbleness",     99,  spell_nimbleness,     TAR_CHAR_DEFENSIVE, POS_STANDING,
   &gsn_nimbleness,   0,  12,                   "", "You feel less nimble.",
   "$n looks less nimble.", 0, 0, 0, 0
 },

 { "mind wreck",      13,  spell_null,           TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,               5,  12,                   "mind wreck", "Your mind returns to normal.",
   "$n regains control of his mind.", 0, 0, 0, 0
 },

 { "steel fists",    99,  spell_steelfists,     TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL,             12,  6,                    "", "Your fists return to normal.",
   "$n's fists return to their fleshy form.", 0, 0, 0, 0
 },

 { "noxious fumes",  99,  spell_noxious_fumes,  TAR_CHAR_OFFENSIVE, POS_STANDING,
   NULL,             12,  6,                    "", "Your senses returns to normal.",
   "$n's senses returns to normal.", 0, 0, 0, 0
 },

 { "soaring spirit", 99,  spell_null,           TAR_CHAR_DEFENSIVE, POS_STANDING,
   &gsn_spiritsoaring, 0, 24,                   "", "Your soaring spirit fades away.",
   "$n is no longer possessed by a soaring spirit.", 0, 0, 0, 0
 },

 { "paradox",        13,  spell_null,           TAR_CHAR_SELF, POS_STANDING,
   &gsn_paradox,      5,  12,                   "", "Your paradox fades.",
   "$n's paradox fades.", 0, 0, 0, 0
 },

 { "knife spin",     99,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_knifespin,    0,  24,                   "knife spin", "Bug: not a spell",
   "", 0, 0, 0, 0
 },

 { "wakasashi slice", 99, spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_wakasashislice, 0, 24,                  "wakasashi slice", "Bug: not a spell",
   "", 0, 0, 0, 0
 },

 { "harmonic thrust", 99, spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_hthrust,       0, 24,                   "harmonic thrust", "Bug: not a spell",
   "", 0, 0, 0, 0
 },

 { "dull cut",       99,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_dullcut,      0,  24,                   "dull cut", "Bug: not a spell",
   "", 0, 0, 0, 0
 },

 { "whirling blades", 99, spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_whirl,        0,   24,                  "whirling blades", "Bug: not a spell",
   "", 0, 0, 0, 0
 },

 { "caltrop toss",   99,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_caltrops,     0,  24,                   "caltrop toss", "Bug: not a spell",
   "", 0, 0, 0, 0
 },

 { "soulreaper",     99,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_soulreaper,   0,  24,                   "soulreaper", "Bug: not a spell",
   "", 0, 0, 0, 0
 },

 { "moonstrike",     99,  spell_null,           TAR_IGNORE, POS_STANDING,
  &gsn_moonstrike,    0,  24,                   "moonstrike", "Bug: not a spell",
   "", 0, 0, 0, 0
 },

 { "shadow thrust",  99,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_shadowthrust, 0,  24,                   "shadow thrust", "Bug: not a spell",
   "", 0, 0, 0, 0
 },

 { "gutcutter",      99,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_gutcutter,    0,  24,                   "gutcutter", "Bug: not a spell",
   "", 0, 0, 0, 0
 },

 { "dirtthrow",      99,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_dirtthrow,    0,  24,                   "dirtthrow", "Bug: not a spell",
   "", 0, 0, 0, 0
 },

 { "soulseeker",     99,  spell_null,           TAR_IGNORE, POS_STANDING,
   &gsn_soulseeker,   0,  24,                   "soulseeker", "Bug: not a spell",
   "", 0, 0, 0, 0
 },

 { "shadow guard",     3,  spell_shadow_guard,  TAR_IGNORE, POS_FIGHTING,
   NULL,             500,  18,                  "", "",
   "", CLASS_SHADOW, EVOLVE_3, SHADOW_EVOLVE_WITNESS, CP_BIT
 },

 { "planebind",        3,  spell_planebind,     TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,             500,  10,                  "", "Your shadow-bindings crumble and fade away.",
   "The shadow-bindings around $n crumble and fade away.",
   CLASS_SHADOW, EVOLVE_3, SHADOW_EVOLVE_FEINTS, CP_BIT
 },

 { "spellguard",       3,  spell_spellguard,    TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL,             500,  14,                  "", "Your spellguard fades.",
   "The blue aura around $n fades away.",
   CLASS_GIANT, EVOLVE_3,  GIANT_EVOLVE_SPECTRAL, CP_BIT
 },

 { "cantrip",          3,  spell_cantrip,       TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL,             500,  10,                  "", "Your cantrip spell fades.",
   "The blue aura around $n fades away.",
   CLASS_GIANT, EVOLVE_3,  GIANT_EVOLVE_VOODOO + GIANT_EVOLVE_TORTOISE, CP_BIT
 },

 { "rupture",          3,  spell_rupture,       TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,             500,  16,                  "", "", "",
   CLASS_GIANT, EVOLVE_3,  GIANT_EVOLVE_KABALISTIC, CP_BIT
 },

 { "spelltrap",        3,  spell_spelltrap,     TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL,             500,  8,                   "", "", "",
   CLASS_GIANT, EVOLVE_3,  GIANT_EVOLVE_VOODOO + GIANT_EVOLVE_TORTOISE, CP_BIT
 },

 { "golden gate",      3,  spell_golden_gate,   TAR_OBJ_INV, POS_FIGHTING,
   NULL,             500,  10,                  "", "", "",
   CLASS_GIANT, EVOLVE_3,  GIANT_EVOLVE_SPECTRAL + GIANT_EVOLVE_KABALISTIC, CP_BIT
 },

 { "frostbite",       99,  spell_null,          TAR_IGNORE, POS_STANDING,
   &gsn_frostbite,     0,  24,                  "frostbite", "The frostbite fades away.",
  "$n is no longer frostbitten.", 0, 0, 0, 0
 },

 { "flamberge",       99,  spell_null,          TAR_IGNORE, POS_FIGHTING,
   &gsn_flamberge,     0,  24,                  "flamberge", "flamberge",
   "", 0, 0, 0, 0
 },

 { "hunting star",    99,  spell_null,          TAR_IGNORE, POS_FIGHTING,
   &gsn_huntingstars,  0,  24,                  "hunting star", "hunting star",
   "", 0, 0, 0, 0
 },

 { "doom bolt",       99,  spell_null,          TAR_IGNORE, POS_FIGHTING,
   &gsn_doombolt,      0,  24,                  "doom bolt", "doom bolt",
   "", 0, 0, 0, 0
 },

 { "chill bolt",      99,  spell_null,          TAR_IGNORE, POS_FIGHTING,
   &gsn_chillbolt,     0,  24,                  "chill bolt","chill bolt",
   "", 0, 0, 0, 0
 },

 { "death spell",     99,  spell_null,          TAR_IGNORE, POS_FIGHTING,
   &gsn_deathspell,    0,  24,                  "death spell", "death spell",
   "", 0, 0, 0, 0
 },

 { "meteor swarm",    99,  spell_null,          TAR_IGNORE, POS_FIGHTING,
   &gsn_meteor,        0,  24,                  "meteor swarm", "meteor swarm",
   "", 0, 0, 0, 0
 },

 { "flamestorm",      99,  spell_null,          TAR_IGNORE, POS_FIGHTING,
   &gsn_flamestorm,    0,  24,                  "flamestorm", "flamestorm",
   "", 0, 0, 0, 0
 },

 { "solar flare",     99,  spell_null,          TAR_IGNORE, POS_FIGHTING,
   &gsn_chantspell,    0,  24, "#y<>#o**#y<> #rS#Rolar #rF#Rlare #y<>#o**#y<>#n", "",
   "", 0, 0, 0, 0
 },

 { "smack",           99,  spell_null,          TAR_IGNORE, POS_STANDING,
   &gsn_smack,         0,  24,                  "#C*#9SmAcK#C*#n", "",
   "", 0, 0, 0, 0
 },

 { "bash",            99,  spell_null,          TAR_IGNORE, POS_STANDING,
   &gsn_bash,          0,  24,                  "#C*#9BaSh#C*#n", "",
   "", 0, 0, 0, 0
 },

 { "crush",           99,  spell_null,          TAR_IGNORE, POS_STANDING,
   &gsn_crush,         0,  24,                  "#C*#9CrUsH#C*#n", "",
   "", 0, 0, 0, 0
 },

 { "thwack",          99,  spell_null,          TAR_IGNORE, POS_STANDING,
   &gsn_thwack,        0,  24,                  "#C*#9ThWaCk#C*#n", "",
   "", 0, 0, 0, 0
 },

 { "backfist",        99,  spell_null,          TAR_IGNORE, POS_STANDING,
   &gsn_backfist,       0,  24,                 "backfist", "",
   "", 0, 0, 0, 0
 },

 { "lava burst",      99,  spell_null,          TAR_IGNORE, POS_STANDING,
   &gsn_lavaburst,     0,  24,                  "lava burst", "",
   "", 0, 0, 0, 0
 },

 { "spikes",          99,  spell_null,          TAR_IGNORE, POS_STANDING,
   &gsn_spikes,        0,  24,                  "spikes", "",
   "", 0, 0, 0, 0
 },

 { "shimmering_plasma", 99, spell_null,         TAR_IGNORE, POS_STANDING,
   &gsn_plasma,        0,  24,                  "shimmering plasma", "",
   "", 0, 0, 0, 0
 },

 { "glowing material", 99, spell_null,          TAR_IGNORE, POS_STANDING,
   &gsn_matter,        0,  24,                  "glowing material", "",
   "", 0, 0, 0, 0
 },

 { "telekinetic burst", 99, spell_null,         TAR_IGNORE, POS_STANDING,
   &gsn_telekinetic,   0,  24,                  "telekinetic burst", "",
   "", 0, 0, 0, 0
 },

 { "tackle",          99,  spell_null,           TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   &gsn_tackle,        0,  24,                   "tackle", "tackle",
   "", 0, 0, 0, 0
 },

 /* terminate list */
 { "reserved_end",    99,  spell_null,          TAR_IGNORE, POS_DEAD,
   NULL,               0,  0,                   "reserved_end", "reserved_end",
   "reserved_end", 0, 0, 0, 0
 }
};
