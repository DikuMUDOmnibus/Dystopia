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

#define _XOPEN_SOURCE /* glibc2 needs this */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

#include "dystopia.h"

extern int _filbuf (FILE *);

/*
 * Globals.
 */
STACK             *  dummy_free = NULL;
LIST              *  dummy_list = NULL;
STACK             *  char_free;
STACK             *  feed_free = NULL;
STACK             *  extra_descr_free = NULL;
STACK             *  note_free = NULL;
STACK             *  obj_free = NULL;
STACK             *  pcdata_free = NULL;
STACK             *  ban_free = NULL;
LIST              *  char_list = NULL;
CHAR_DATA         *  sacrificer = NULL;
char              *  help_greeting;
KILL_DATA            kill_table[MAX_LEVEL];
LIST              *  object_list = NULL;
LIST              *  help_list = NULL;
TIME_INFO_DATA       time_info;
WEATHER_DATA         weather_info;
MUDDATA              muddata;
LEADER_BOARD         leader_board;
int                  maxSocial;
struct social_type * social_table;

sh_int gsn_heroism;
sh_int gsn_tackle;
sh_int gsn_omniscience;
sh_int gsn_endurance;
sh_int gsn_brilliance;
sh_int gsn_nimbleness;
sh_int gsn_chaosblast;
sh_int gsn_firebreath;
sh_int gsn_gasblast;
sh_int gsn_magicmissile;
sh_int gsn_groupheal;
sh_int gsn_impfireball;
sh_int gsn_energydrain;
sh_int gsn_earthquake;
sh_int gsn_blindness;
sh_int gsn_knee;
sh_int gsn_sweep;
sh_int gsn_charm_person;
sh_int gsn_curse;
sh_int gsn_invis;
sh_int gsn_mass_invis;
sh_int gsn_poison;
sh_int gsn_backstab;
sh_int gsn_garotte;
sh_int gsn_disarm;
sh_int gsn_hurl;
sh_int gsn_kick;
sh_int gsn_circle;
sh_int gsn_peek;
sh_int gsn_pick_lock;
sh_int gsn_rescue;
sh_int gsn_sneak;
sh_int gsn_steal;
sh_int gsn_punch;
sh_int gsn_elbow;
sh_int gsn_headbutt;
sh_int gsn_berserk;
sh_int gsn_track;
sh_int gsn_fireball;
sh_int gsn_lightning;
sh_int gsn_nova;
sh_int gsn_paradox;
sh_int gsn_ballista;

/* shadow attacks */
sh_int gsn_knifespin;
sh_int gsn_wakasashislice;
sh_int gsn_hthrust;
sh_int gsn_dullcut;
sh_int gsn_frostbite;
sh_int gsn_whirl;
sh_int gsn_caltrops;
sh_int gsn_soulreaper;
sh_int gsn_moonstrike;
sh_int gsn_shadowthrust;
sh_int gsn_gutcutter;
sh_int gsn_dirtthrow;
sh_int gsn_soulseeker;
sh_int gsn_spiritsoaring;

/* warlock attacks */
sh_int gsn_flamberge;
sh_int gsn_huntingstars;
sh_int gsn_doombolt;
sh_int gsn_fireshield;
sh_int gsn_deathspell;
sh_int gsn_chillbolt;
sh_int gsn_meteor;
sh_int gsn_flamestorm;
sh_int gsn_chantspell;

/* fae attacks */
sh_int gsn_telekinetic;
sh_int gsn_plasma;
sh_int gsn_matter;
sh_int gsn_spiritkiss;

/* Giant attacks */
sh_int gsn_bash; 
sh_int gsn_crush;
sh_int gsn_smack; 
sh_int gsn_thwack;
sh_int gsn_backfist;
sh_int gsn_lavaburst;
sh_int gsn_spikes;

/*
 * Locals.
 */
LIST             * mob_index_hash[MAX_KEY_HASH];
LIST             * obj_index_hash[MAX_KEY_HASH];
LIST             * room_index_hash[MAX_KEY_HASH];
char             * string_hash[MAX_KEY_HASH];
char             * strdup(const char *s);

LIST               * area_list = NULL;
HELP_DATA          * first_help = NULL;
HELP_DATA          * last_help = NULL;
char               * string_space;
char               * top_string;
char                 str_empty[1];

int top_affect;
int top_area;
int top_rt;
int top_ed;
int top_exit;
int top_help;
int top_mob_index;
int top_obj_index;
int top_reset;
int top_room;
int top_vnum_room;              /* OLC */
int top_vnum_mob;               /* OLC */
int top_vnum_obj;               /* OLC */

/*
 * Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing.
 */

#define			MAX_STRING	1048576
#define			MAX_PERM_BLOCK	131072
#define			MAX_MEM_LIST	11

void *rgFreeList[MAX_MEM_LIST];
const int rgSizeList[MAX_MEM_LIST] = {
  16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768 - 64
};

int nAllocString;
int sAllocString;

/*
 * Semi-locals.
 */
bool fBootDb;
FILE *fpArea;
char strArea[MAX_INPUT_LENGTH];

/*
 * Local booting procedures.
 */
void init_mm        ( void );
void load_helps     ( void );
void load_mobiles   ( FILE *fp );
void load_objects   ( FILE *fp );
void load_resets    ( FILE *fp );
void load_specials  ( FILE *fp );
void load_area      ( FILE *fp );
void load_rooms     ( FILE *fp );
void fix_exits      ( void );

/* The class table, remember to add your new classes here */
const struct class_type class_table[] =
{
  /* name            class-bit         can autoclass
   -------------------------------------------------*/
  { "Shadow",        CLASS_SHADOW,        TRUE     },
  { "Giant",         CLASS_GIANT,         TRUE     },
  { "Warlock",       CLASS_WARLOCK,       TRUE     },
  { "Fae",           CLASS_FAE,           TRUE     },
  { "None",          0,                   FALSE    },

  /* NULL Terminator */
  { "", 0, FALSE }
};


/*
 * Big mama top level function.
 */
void boot_db(bool fCopyOver)
{
  /*
   * Init some data space stuff.
   */
  {
    string_space = calloc(1, MAX_STRING);
    top_string = string_space;
    fBootDb = TRUE;
  }

  /*
   * Init random number generator.
   */
  {
    init_mm();
  }

  /*
   * Init event queue
   */
  init_event_queue(1);

  /*
   * Set time and weather.
   */
  {
    long lhour, lday, lmonth;

    lhour = (current_time - 650336715) / (PULSE_TICK / PULSE_PER_SECOND);
    time_info.hour = lhour % 24;
    lday = lhour / 24;
    time_info.day = lday % 35;
    lmonth = lday / 35;
    time_info.month = lmonth % 17;
    time_info.year = lmonth / 17;

    if (time_info.hour < 5)
      weather_info.sunlight = SUN_DARK;
    else if (time_info.hour < 6)
      weather_info.sunlight = SUN_RISE;
    else if (time_info.hour < 19)
      weather_info.sunlight = SUN_LIGHT;
    else if (time_info.hour < 20)
      weather_info.sunlight = SUN_SET;
    else
      weather_info.sunlight = SUN_DARK;

    weather_info.change = 0;
    weather_info.mmhg = 960;
    if (time_info.month >= 7 && time_info.month <= 12)
      weather_info.mmhg += number_range(1, 50);
    else
      weather_info.mmhg += number_range(1, 80);

    if (weather_info.mmhg <= 980)
      weather_info.sky = SKY_LIGHTNING;
    else if (weather_info.mmhg <= 1000)
      weather_info.sky = SKY_RAINING;
    else if (weather_info.mmhg <= 1020)
      weather_info.sky = SKY_CLOUDY;
    else
      weather_info.sky = SKY_CLOUDLESS;

  }

  /*
   * Assign gsn's for skills which have them.
   */
  {
    int sn;

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (skill_table[sn].pgsn != NULL)
        *skill_table[sn].pgsn = sn;
    }
  }

  /*
   * Read in all the area files.
   */
  {
    FILE *fpList;

    if ((fpList = fopen(AREA_LIST, "r")) == NULL)
    {
      perror(AREA_LIST);
      exit(1);
    }

    for (;;)
    {
      strcpy(strArea, fread_word(fpList));
      if (strArea[0] == '$')
        break;

      if (strArea[0] == '-')
      {
        fpArea = stdin;
      }
      else
      {
        log_string("loading %s", strArea);
        if ((fpArea = fopen(strArea, "r")) == NULL)
        {
          perror(strArea);
          exit(1);
        }
      }

      for (;;)
      {
        char *word;

        if (fread_letter(fpArea) != '#')
        {
          bug("Boot_db: # not found.", 0);
          exit(1);
        }

        word = fread_word(fpArea);

        if (word[0] == '$')
          break;
        else if (!str_cmp(word, "MOBILES"))
          load_mobiles(fpArea);
        else if (!str_cmp(word, "OBJECTS"))
          load_objects(fpArea);
        else if (!str_cmp(word, "RESETS"))
          load_resets(fpArea);
        else if (!str_cmp(word, "SPECIALS"))
          load_specials(fpArea);
        else if (!str_cmp(word, "AREADATA"))
          load_area(fpArea);
        else if (!str_cmp(word, "ROOMDATA"))
          load_rooms(fpArea);
        else
        {
          bug("Boot_db: bad section name.", 0);
          exit(1);
        }
      }

      if (fpArea != stdin)
        fclose(fpArea);
      fpArea = NULL;
    }
    fclose(fpList);
  }

  /*
   * Load stuff
   */
  {
    load_helps();
    fix_exits();
    fBootDb = FALSE;
    load_muddata();
    init_event_queue(2);
    load_bans();
    load_newbiebans();
    load_leaderboard();
    load_top10();
    load_boards();
    load_archmages();
    save_notes();
    load_disabled();
    load_changes();
    load_polls();
    load_social_table();
    load_kingdoms();
    load_artifact_table();
    load_auctions();
    init_teamarena();
  }

  if (fCopyOver)
    copyover_recover();
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                \
                if ( !str_cmp( word, literal ) )    \
                {                                   \
                    field  = value;                 \
                    fMatch = TRUE;                  \
                    break;                          \
                }

#define SKEY( string, field )                       \
                if ( !str_cmp( word, string ) )     \
                {                                   \
                    free_string( field );           \
                    field = fread_string( fp );     \
                    fMatch = TRUE;                  \
                    break;                          \
                }

/* OLC
 * Snarf an 'area' header line.   Check this format.  MUCH better.  Add fields
 * too.             dv *
 * #AREAFILE
 * Name   { All } Locke    Newbie School~
 * Repop  A teacher pops in the room and says, 'Repop coming!'~
 * End
 */
void load_area(FILE * fp)
{
  AREA_DATA *pArea;
  char *word;
  bool fMatch;

  pArea              = calloc(1, sizeof(*pArea));
  pArea->filename    = str_dup(strArea);
  pArea->vnum        = top_area;
  pArea->name        = str_dup("New Area");
  pArea->builders    = str_dup("");
  pArea->music       = str_dup("default.mid");
  pArea->areabits    = 0;
  pArea->security    = 3;
  pArea->lvnum       = 0;
  pArea->uvnum       = 0;
  pArea->cvnum       = 0;
  pArea->area_flags  = 0;
  pArea->affects     = AllocList();
  pArea->events      = AllocList();

  for (;;)
  {
    word = feof(fp) ? "End" : fread_word(fp);
    fMatch = FALSE;

    switch (UPPER(word[0]))
    {
      case 'N':
        SKEY("Name", pArea->name);
        break;
      case 'A':
        KEY("Areabits", pArea->areabits, fread_number(fp));
        break;
      case 'S':
        KEY("Security", pArea->security, fread_number(fp));
        break;
      case 'C':
        KEY("Cvnum", pArea->cvnum, fread_number(fp));
        break;
      case 'V':
        if (!str_cmp(word, "VNUMs"))
        {
          pArea->lvnum = fread_number(fp);
          pArea->uvnum = fread_number(fp);
        }
        break;
      case 'E':
        if (!str_cmp(word, "End"))
        {
          fMatch = TRUE;
          AttachToEndOfList(pArea, area_list);
          top_area++;
          return;
        }
        break;
      case 'B':
        SKEY("Builders", pArea->builders);
        break;
      case 'M':
        SKEY("Music", pArea->music);
        break;
      case 'R':
        if (!str_cmp(word, "Recall"))
        {
          fread_number(fp);
          break;
        }
        break;
    }
  }
  log_string("%s", pArea->name);
}

/*
 * Sets vnum range for area using OLC protection features.
 */
void assign_area_vnum(int vnum)
{
  AREA_DATA *pArea;

  if ((pArea = (AREA_DATA *) LastInList(area_list)) != NULL)
  {
    if (pArea->lvnum == 0 || pArea->uvnum == 0)
      pArea->lvnum = pArea->uvnum = vnum;

    if (vnum != URANGE(pArea->lvnum, vnum, pArea->uvnum))
    {
      if (vnum < pArea->lvnum)
        pArea->lvnum = vnum;
      else
        pArea->uvnum = vnum;
    }
  }
}

/*
 * Returns an uppercase string.
 */
char *strupper(const char *str)
{
  static char strup[MAX_STRING_LENGTH];
  int i;

  for (i = 0; str[i] != '\0'; i++)
    strup[i] = UPPER(str[i]);
  strup[i] = '\0';
  return strup;
}

void add_help(HELP_DATA *pHelp)
{
  HELP_DATA *tHelp;
  ITERATOR *pIter;
  int plen = strlen(pHelp->name);

  pIter = AllocReverseIterator(help_list);
  while ((tHelp = (HELP_DATA *) NextInList(pIter)) != NULL)
  {
    int tlen = strlen(tHelp->name);
    bool match = TRUE;
    bool fullmatch = FALSE;
    int i;

    for (i = 0; i < plen && i < tlen; i++)
    {
      if (UPPER(pHelp->name[i]) < UPPER(tHelp->name[i]))
      {
        match = FALSE;
        break;
      }
      if (UPPER(pHelp->name[i]) > UPPER(tHelp->name[i]))
      {
        fullmatch = TRUE;
        break;
      }
    }

    if (!match)
      continue;

    if (tlen < plen || fullmatch)
    {
      AttachToListAfterItem(pHelp, help_list, tHelp);
      break;
    }
  }

  if (!tHelp)
  {
    AttachToList(pHelp, help_list);
  }

  top_help++;
}

void load_helps()
{
  HELP_DATA *pHelp;
  DIR *directory;
  FILE *fp;
  char buf[MAX_STRING_LENGTH];
  struct dirent *entry;

  log_string("Load_helps: Starting.");

  directory = opendir("../helps/");
  for (entry = readdir(directory); entry; entry = readdir(directory))
  {
    if (!str_cmp(entry->d_name, ".") || !str_cmp(entry->d_name, ".."))
      continue;

    if (str_suffix(".hlp", entry->d_name))
      continue;

    sprintf(buf, "../helps/%s", entry->d_name);
    if ((fp = fopen(buf, "r")) != NULL)
    {
      char name[MAX_INPUT_LENGTH];
      char keyword[MAX_INPUT_LENGTH];
      char text[MAX_STRING_LENGTH];
      int level, i, c;

      i = 0;
      c = getc(fp);
      do {
        name[i++] = c;
        c = getc(fp);
      } while (c != '\n' && c != EOF && i < MAX_INPUT_LENGTH - 1);
      name[i] = '\0';

      if (c == EOF)
      {
        sprintf(buf, "load_helps: %s is corrupt.", entry->d_name);
        bug(buf, 0);
        fclose(fp);
        continue;
      }

      i = 0;
      c = getc(fp);
      do {
        keyword[i++] = c;
        c = getc(fp);
      } while (c != '\n' && c != EOF && i < MAX_INPUT_LENGTH - 1);
      keyword[i] = '\0';

      if (c == EOF)
      {
        sprintf(buf, "load_helps: %s is corrupt.", entry->d_name);
        bug(buf, 0);
        fclose(fp);
        continue;
      }

      i = 0;
      c = getc(fp);
      do {
        buf[i++] = c;
        c = getc(fp);
      } while (c != '\n' && c != EOF && i < MAX_INPUT_LENGTH - 1);
      buf[i] = '\0';

      if (c == EOF)
      {
        sprintf(buf, "load_helps: %s is corrupt.", entry->d_name);
        bug(buf, 0);
        fclose(fp);
        continue;
      }

      level = atoi(buf);
      i = 0;
      c = getc(fp);
      do {
        if (c == '\n')
          text[i++] = '\r';
        text[i++] = c;

        c = getc(fp);
      } while (c != EOF && i < MAX_STRING_LENGTH - 1);

      while (isspace(text[i-1]))
        i--;
      text[i] = '\0';

      if (!str_cmp(name, "greeting"))
      {
        text[i] = ' ';
        text[i+1] = '\0';
      }

      pHelp = calloc(1, sizeof(*pHelp));
      pHelp->level = level;
      pHelp->name = str_dup(name);
      pHelp->keyword = str_dup(keyword);
      pHelp->text = str_dup(text);
      add_help(pHelp);

      if (!str_cmp(pHelp->name, "greeting"))
        help_greeting = pHelp->text;

      fclose(fp);
    }
  }
  closedir(directory);

  log_string("Load_helps: Completed.");
}

/*
 * Snarf a mob section.
 */
void load_mobiles(FILE *fp)
{
  MOB_INDEX_DATA *pMobIndex;
  AREA_DATA *pArea;

  if (SizeOfList(area_list) == 0)
  {
    bug("Load_mobiles: no #AREA seen yet.", 0);
    exit(1);
  }

  pArea = (AREA_DATA *) LastInList(area_list);

  for (;;)
  {
    int vnum;
    char letter;
    int iHash;

    letter = fread_letter(fp);
    if (letter != '#')
    {
      bug("Load_mobiles: # not found.", 0);
      exit(1);
    }

    vnum = fread_number(fp);
    if (vnum == 0)
      break;

    fBootDb = FALSE;
    if (get_mob_index(vnum) != NULL)
    {
      bug("Load_mobiles: vnum %d duplicated.", vnum);
      exit(1);
    }
    fBootDb = TRUE;

    pMobIndex                   = calloc(1, sizeof(*pMobIndex));
    pMobIndex->vnum             = vnum;
    pMobIndex->area             = pArea;
    pMobIndex->player_name      = fread_string(fp);
    pMobIndex->short_descr      = fread_string(fp);
    pMobIndex->long_descr       = fread_string(fp);
    pMobIndex->description      = fread_string(fp);

    pMobIndex->long_descr[0]    = UPPER(pMobIndex->long_descr[0]);
    pMobIndex->description[0]   = UPPER(pMobIndex->description[0]);

    pMobIndex->act              = fread_number(fp) | ACT_IS_NPC;
    pMobIndex->affected_by      = fread_number(fp);
    pMobIndex->alignment        = fread_number(fp);
    pMobIndex->level            = fread_number(fp);
    pMobIndex->toughness        = fread_number(fp);
    pMobIndex->extra_attack     = fread_number(fp);
    pMobIndex->dam_modifier     = fread_number(fp);
    pMobIndex->extra_parry      = fread_number(fp);
    pMobIndex->extra_dodge      = fread_number(fp);
    pMobIndex->sex              = fread_number(fp);
    pMobIndex->natural_attack   = fread_number(fp);

    /* fix depracted act values */
    {
      int flags = 0;
      int i;

      for (i = 0; act_flags[i].name[0] != '\0'; i++)
      {
        if (IS_SET(pMobIndex->act, act_flags[i].bit))
          flags += act_flags[i].bit;
      }

      pMobIndex->act = flags;
    }

    /* set default values for special programs */
    pMobIndex->shop_fun         = NULL;
    pMobIndex->quest_fun        = NULL;
    pMobIndex->death_fun        = NULL;
    pMobIndex->spec_fun         = NULL;

    letter                      = fread_letter( fp );
    if ( letter != 'S' )  
    {
      bug( "Load_mobiles: vnum %d non-S.", vnum );
      exit( 1 );
    }

    iHash                       = vnum % MAX_KEY_HASH;

    AttachToList(pMobIndex, mob_index_hash[iHash]);

    top_mob_index++;
    top_vnum_mob                = top_vnum_mob < vnum ? vnum : top_vnum_mob; /* OLC */
    assign_area_vnum(vnum);     /* OLC */

    kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL - 1)].number++;
  }
}

/*
 * Snarf an obj section.
 */
void load_objects(FILE *fp)
{
  char *word;
  char buf[MAX_STRING_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  AREA_DATA *pArea;
  int sn;

  if (SizeOfList(area_list) == 0)
  {
    bug("Load_objects: no #AREA seen yet.", 0);
    exit(1);
  }

  pArea = (AREA_DATA *) LastInList(area_list);

  for (;;)
  {
    int vnum;
    char letter;
    int iHash;

    letter = fread_letter(fp);
    if (letter != '#')
    {
      bug("Load_objects: # not found.", 0);
      exit(1);
    }

    vnum = fread_number(fp);
    if (vnum == 0)
      break;

    fBootDb = FALSE;
    if (get_obj_index(vnum) != NULL)
    {
      bug("Load_objects: vnum %d duplicated.", vnum);
      exit(1);
    }
    fBootDb = TRUE;

    /* init object */
    pObjIndex                 = calloc(1, sizeof(*pObjIndex));
    pObjIndex->vnum           = vnum;
    pObjIndex->area           = pArea;
    pObjIndex->extra_descr    = AllocList();
    pObjIndex->affected       = AllocList();

    /* loading object from file */
    pObjIndex->name           = fread_string(fp);
    pObjIndex->short_descr    = fread_string(fp);
    pObjIndex->description    = fread_string(fp);
    pObjIndex->item_type      = fread_number(fp);
    pObjIndex->extra_flags    = fread_number(fp);
    pObjIndex->wear_flags     = fread_number(fp);

    /* the values are loaded differently depending on the items type */
    switch(pObjIndex->item_type)
    {
      default:
        pObjIndex->value[0] = fread_number(fp);
        pObjIndex->value[1] = fread_number(fp);
        pObjIndex->value[2] = fread_number(fp);
        pObjIndex->value[3] = fread_number(fp);
        break;
      case ITEM_WAND:
      case ITEM_STAFF:
        pObjIndex->value[0] = fread_number(fp);
        pObjIndex->value[1] = fread_number(fp);
        pObjIndex->value[2] = fread_number(fp);

        word = fread_word(fp);
        if ((sn = skill_lookup(word)) < 0)
        {
          sprintf(buf, "load_objects: spell '%s' not in spelltable.", word);
          bug(buf, 0);
          pObjIndex->value[3] = 0;
        }
        else
          pObjIndex->value[3] = sn;
        break;
      case ITEM_SCROLL:
      case ITEM_PILL:
      case ITEM_POTION:
        pObjIndex->value[0] = fread_number(fp);

        word = fread_word(fp);
        if ((sn = skill_lookup(word)) < 0)
        {
          sprintf(buf, "load_objects: spell '%s' not in spelltable.", word);
          bug(buf, 0);
          pObjIndex->value[1] = 0;
        }
        else
          pObjIndex->value[1] = sn;

        word = fread_word(fp);
        if ((sn = skill_lookup(word)) < 0)
        {
          sprintf(buf, "load_objects: spell '%s' not in spelltable.", word);
          bug(buf, 0);
          pObjIndex->value[2] = 0;
        }
        else
          pObjIndex->value[2] = sn;

        word = fread_word(fp);
        if ((sn = skill_lookup(word)) < 0)
        {
          sprintf(buf, "load_objects: spell '%s' not in spelltable.", word);
          bug(buf, 0);
          pObjIndex->value[3] = 0;
        }
        else
          pObjIndex->value[3] = sn;
    }

    /* get weight and value of item */
    pObjIndex->weight         = fread_number(fp);
    pObjIndex->cost           = fread_number(fp);

    /* setting case */
    pObjIndex->short_descr[0] = LOWER(pObjIndex->short_descr[0]);
    pObjIndex->description[0] = UPPER(pObjIndex->description[0]);

    for (;;)
    {
      letter = fread_letter(fp);

      if (letter == 'A')
      {
        AFFECT_DATA *paf;

        paf = calloc(1, sizeof(*paf));
        paf->type = -1;
        paf->duration = -1;
        paf->location = fread_number(fp);
        paf->modifier = fread_number(fp);
        paf->bitvector = 0;

        AttachToList(paf, pObjIndex->affected);
        top_affect++;
      }

      else if (letter == 'E')
      {
        EXTRA_DESCR_DATA *ed;

        ed = calloc(1, sizeof(*ed));
        ed->keyword = fread_string(fp);
        ed->description = fread_string(fp);
        ed->buffer1 = fread_string(fp);
        ed->buffer2 = fread_string(fp);
        ed->type = fread_number(fp);
        ed->vnum = fread_number(fp);
        ed->action = fread_number(fp);

        AttachToList(ed, pObjIndex->extra_descr);
        top_ed++;
      }

      else
      {
        ungetc(letter, fp);
        break;
      }
    }

    iHash                  = vnum % MAX_KEY_HASH;
    AttachToList(pObjIndex, obj_index_hash[iHash]);
    top_obj_index++;
    top_vnum_obj           = top_vnum_obj < vnum ? vnum : top_vnum_obj; /* OLC */
    assign_area_vnum(vnum);     /* OLC */
  }
  return;
}

/*
 * Adds a reset to a room.  OLC
 * Similar to add_reset in olc.c
 */
void new_reset(ROOM_INDEX_DATA * pRoom, RESET_DATA * pReset)
{
  if (pRoom == NULL)
    return;

  AttachToEndOfList(pReset, pRoom->resets);
  pReset->repop = TRUE;
  top_reset++;
}

/*   
 * Snarf a reset section.       Changed for OLC.
 */
void load_resets(FILE * fp)
{
  RESET_DATA *pReset;
  int iLastRoom = 0;
  int iLastObj = 0;

  if (SizeOfList(area_list) == 0)
  {
    bug("Load_resets: no #AREA seen yet.", 0);
    exit(1);
  }

  for (;;)
  {
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *pRoomIndex;
    char letter;

    if ((letter = fread_letter(fp)) == 'S')
      break;

    if (letter == '*')
    {
      fread_to_eol(fp);
      continue;
    }

    pReset = calloc(1, sizeof(*pReset));
    pReset->command = letter;
    /* if_flag */ fread_number(fp);
    pReset->arg1 = fread_number(fp);
    pReset->arg2 = fread_number(fp);
    pReset->arg3 = (letter == 'G' || letter == 'R') ? 0 : fread_number(fp);
    fread_to_eol(fp);

    /*
     * Validate parameters.
     * We're calling the index functions for the side effect.
     */
    switch (letter)
    {
      default:
        bug("Load_resets: bad command '%c'.", letter);
        exit(1);
        break;

      case 'M':
        get_mob_index(pReset->arg1);
        if ((pRoomIndex = get_room_index(pReset->arg3)))
        {
          new_reset(pRoomIndex, pReset);
          iLastRoom = pReset->arg3;
        }
        break;

      case 'O':
        get_obj_index(pReset->arg1);
        if ((pRoomIndex = get_room_index(pReset->arg3)))
        {
          new_reset(pRoomIndex, pReset);
          iLastObj = pReset->arg3;
        }
        break;

      case 'P':
        get_obj_index(pReset->arg1);
        if ((pRoomIndex = get_room_index(iLastObj)))
        {
          new_reset(pRoomIndex, pReset);
        }
        break;

      case 'G':
      case 'E':
        get_obj_index(pReset->arg1);
        if ((pRoomIndex = get_room_index(iLastRoom)))
        {
          new_reset(pRoomIndex, pReset);
          iLastObj = iLastRoom;
        }
        break;

      case 'D':
        pRoomIndex = get_room_index(pReset->arg1);

        if (pReset->arg2 < 0 || pReset->arg2 > 5 || !pRoomIndex || !(pexit = pRoomIndex->exit[pReset->arg2]) || !IS_SET(pexit->rs_flags, EX_ISDOOR))
        {
          bug("Load_resets: 'D': exit %d not door.", pReset->arg2);
          exit(1);
        }

        switch (pReset->arg3)   /* OLC 1.1b */
        {
          default:
            bug("Load_resets: 'D': bad 'locks': %d.", pReset->arg3);
          case 0:
            break;
          case 1:
            SET_BIT(pexit->rs_flags, EX_CLOSED);
            break;
          case 2:
            SET_BIT(pexit->rs_flags, EX_CLOSED | EX_LOCKED);
            break;
        }
        break;

      case 'R':
        if (pReset->arg2 < 0 || pReset->arg2 > 6) /* Last Door. */
        {
          bug("Load_resets: 'R': bad exit %d.", pReset->arg2);
          exit(1);
        }

        if ((pRoomIndex = get_room_index(pReset->arg1)))
          new_reset(pRoomIndex, pReset);

        break;
    }
  }

  return;
}

/* OLC 1.1b */
void load_rooms(FILE *fp)
{
  ROOM_INDEX_DATA *pRoomIndex;
  AREA_DATA *pArea;

  if (SizeOfList(area_list) == 0)
  {
    bug("Load_rooms: no #AREA seen yet.", 0);
    exit(1);
  }

  pArea = (AREA_DATA *) LastInList(area_list);

  for (;;)
  {
    char letter;
    int vnum, door, iHash, i, flags = 0;

    letter = fread_letter(fp);
    if (letter != '#')
    {
      bug("Load_rooms: # not found.", 0);
      exit(1);
    }

    vnum = fread_number(fp);
    if (vnum == 0)
      break;

    fBootDb = FALSE;
    if (get_room_index(vnum))
    {
      bug("Load_rooms: vnum %d duplicated.", vnum);
      exit(1);
    }
    fBootDb = TRUE;

    pRoomIndex = calloc(1, sizeof(*pRoomIndex));
    pRoomIndex->people         =  AllocList();
    pRoomIndex->contents       =  AllocList();
    pRoomIndex->extra_descr    =  AllocList();
    pRoomIndex->events         =  AllocList();
    pRoomIndex->roomtext       =  AllocList();
    pRoomIndex->resets         =  AllocList();
    pRoomIndex->area           =  pArea;
    pRoomIndex->vnum           =  vnum;
    pRoomIndex->name           =  fread_string(fp);
    pRoomIndex->description    =  fread_string(fp);
    pRoomIndex->room_flags     =  fread_number(fp);
    pRoomIndex->sector_type    =  fread_number(fp);
    pRoomIndex->light          =  0;
    pRoomIndex->blood          =  0;

    /* now fix all roomflags, making sure only 'existing' flags  are set. */
    for (i = 0; room_flags[i].name[0] != '\0'; i++)
    {
      if (IS_SET(pRoomIndex->room_flags, room_flags[i].bit))
        flags += room_flags[i].bit;
    }
    pRoomIndex->room_flags = flags;

    for (door = 0; door <= 5; door++)
      pRoomIndex->exit[door] = NULL;

    for (;;)
    {
      letter = fread_letter(fp);

      if (letter == 'S' || letter == 's')
      {
        if (letter == 's')
          bug("Load_rooms: vnum %d has lowercase 's'", vnum);
        break;
      }

      if (letter == 'D')
      {
        EXIT_DATA *pexit;
        int locks;

        door = fread_number(fp);
        if (door < 0 || door > 5)
        {
          bug("Fread_rooms: vnum %d has bad door number.", vnum);
          exit(1);
        }

        pexit = calloc(1, sizeof(*pexit));
        pexit->description   =  fread_string(fp);
        pexit->keyword       =  fread_string(fp);
        locks                =  fread_number(fp);
        pexit->exit_info     =  locks;
        pexit->rs_flags      =  locks;
        pexit->key           =  fread_number(fp);
        pexit->vnum          =  fread_number(fp);

        pRoomIndex->exit[door] = pexit;
        top_exit++;
      }
      else if (letter == 'E')
      {
        EXTRA_DESCR_DATA *ed;

        ed = calloc(1, sizeof(*ed));
        ed->keyword        =  fread_string(fp);
        ed->description    =  fread_string(fp);
        ed->buffer1        =  fread_string(fp);
        ed->buffer2        =  fread_string(fp);
        ed->type           =  fread_number(fp);
        ed->vnum           =  fread_number(fp);
        ed->action         =  fread_number(fp);

        AttachToList(ed, pRoomIndex->extra_descr);
        top_ed++;
      }
      else if (letter == 'T')
      {
        ROOMTEXT_DATA *rt;

        rt = malloc(sizeof(*rt));
        rt->input       =  fread_string(fp);
        rt->output      =  fread_string(fp);
        rt->choutput    =  fread_string(fp);
        rt->name        =  fread_string(fp);
        rt->type        =  fread_number(fp);
        rt->power       =  fread_number(fp);
        rt->mob         =  fread_number(fp);

        AttachToList(rt, pRoomIndex->roomtext);
        top_rt++;
      }

      else
      {
        bug("Load_rooms: vnum %d has flag not 'DES'.", vnum);
        exit(1);
      }
    }

    /* initialize room events */
    init_events_room(pRoomIndex);

    iHash = vnum % MAX_KEY_HASH;
    AttachToList(pRoomIndex, room_index_hash[iHash]);
    top_room++;
    top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room;
    assign_area_vnum(vnum);
  }

  return;
}

/*
 * Snarf spec proc declarations.
 */
void load_specials(FILE * fp)
{
  for (;;)
  {
    MOB_INDEX_DATA *pMobIndex;
    char letter;

    switch (letter = fread_letter(fp))
    {
      default:
        bug("Load_specials: letter '%c' not *MS.", letter);
        exit(1);

      case 'S':
        return;

      case '*':
        break;

      case 'M':
        pMobIndex = get_mob_index(fread_number(fp));
        pMobIndex->spec_fun = spec_lookup(fread_word(fp));
        if (pMobIndex->spec_fun == 0)
        {
          bug("Load_specials: 'M': vnum %d.", pMobIndex->vnum);
          exit(1);
        }
        break;

      case 'Q':
        pMobIndex = get_mob_index(fread_number(fp));
        pMobIndex->quest_fun = quest_lookup(fread_word(fp));
        if (pMobIndex->quest_fun == 0)
        {
          bug("Load_specials: 'Q': vnum %d.", pMobIndex->vnum);
          exit(1);
        }
        break;

      case 'D':
        pMobIndex = get_mob_index(fread_number(fp));
        pMobIndex->death_fun = death_lookup(fread_word(fp));
        if (pMobIndex->death_fun == 0)
        {
          bug("Load_specials: 'D': vnum %d.", pMobIndex->vnum);
          exit(1);
        }
        break;

      case 'Z':
        pMobIndex = get_mob_index(fread_number(fp));
        pMobIndex->shop_fun = shop_lookup(fread_word(fp));
        if (pMobIndex->shop_fun == 0)
        {
          bug("Load_specials: 'Z': vnum %d.", pMobIndex->vnum);
          exit(1);
        }
        break;

    }

    fread_to_eol(fp);
  }
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits(void)
{
  ROOM_INDEX_DATA *pRoomIndex;
  EXIT_DATA *pexit;
  ITERATOR *pIter;
  int iHash;
  int door;

  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
  {
    pIter = AllocIterator(room_index_hash[iHash]);
    while ((pRoomIndex = (ROOM_INDEX_DATA *) NextInList(pIter)) != NULL)
    {
      bool fexit;

      fexit = FALSE;
      for (door = 0; door <= 5; door++)
      {
        if ((pexit = pRoomIndex->exit[door]) != NULL)
        {
          fexit = TRUE;
          if (pexit->vnum <= 0)
            pexit->to_room = NULL;
          else
            pexit->to_room = get_room_index(pexit->vnum);
        }
      }

      if (!fexit)
        SET_BIT(pRoomIndex->room_flags, ROOM_NO_MOB);
    }
  }

  return;
}


/* OLC
 * Reset one room.  Called by reset_area and olc.
 */
void reset_room(ROOM_INDEX_DATA * pRoom)
{
  RESET_DATA *pReset;
  CHAR_DATA *pMob;
  OBJ_DATA *pObj;
  ITERATOR *pIter;
  CHAR_DATA *LastMob = NULL;
  OBJ_DATA *LastObj = NULL;
  int iExit;
  int level = 0;
  bool last;

  if (!pRoom)
    return;

  pMob = NULL;
  last = FALSE;

  for (iExit = 0; iExit < MAX_DIR; iExit++)
  {
    EXIT_DATA *pExit;

    if ((pExit = pRoom->exit[iExit]))
    {
      pExit->exit_info = pExit->rs_flags;
      if ((pExit->to_room != NULL) && ((pExit = pExit->to_room->exit[rev_dir[iExit]])))
      {
        /* nail the other side */
        pExit->exit_info = pExit->rs_flags;
      }
    }
  }

  pIter = AllocIterator(pRoom->resets);
  while ((pReset = (RESET_DATA *) NextInList(pIter)) != NULL)
  {
    MOB_INDEX_DATA *pMobIndex;
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_INDEX_DATA *pObjToIndex;
    ROOM_INDEX_DATA *pRoomIndex;

    switch (pReset->command)
    {
      default:
        bug("Reset_room: bad command %c.", pReset->command);
        break;

      case 'M':
        if (!(pMobIndex = get_mob_index(pReset->arg1)))
        {
          bug("Reset_room: 'M': bad vnum %d.", pReset->arg1);
          continue;
        }

        if (pReset->repop == FALSE)
        {
          last = FALSE;
          break;
        }

        /* create mobile, and set respawn */
        pMob = create_mobile(pMobIndex);
        pMob->respawn = pReset;
        pReset->repop = FALSE;

        /*
         * Some more hard coding.
         */
        if (room_is_dark(pRoom))
          SET_BIT(pMob->affected_by, AFF_INFRARED);

        char_to_room(pMob, pRoom, TRUE);

        LastMob = pMob;
        level = URANGE(0, pMob->level - 2, LEVEL_AVATAR);
        last = TRUE;
        break;

      case 'O':
        if (!(pObjIndex = get_obj_index(pReset->arg1)))
        {
          bug("Reset_room: 'O': bad vnum %d.", pReset->arg1);
          continue;
        }

        if (!(pRoomIndex = get_room_index(pReset->arg3)))
        {
          bug("Reset_room: 'O': bad vnum %d.", pReset->arg3);
          continue;
        }

        if (count_obj_list(pObjIndex, pRoom->contents) > 0)
          break;

        pObj = create_object(pObjIndex, number_fuzzy(level));
        pObj->cost = 0;
        obj_to_room(pObj, pRoom);
        break;

      case 'P':
        if (!(pObjIndex = get_obj_index(pReset->arg1)))
        {
          bug("Reset_room: 'P': bad vnum %d.", pReset->arg1);
          continue;
        }
        if (!(pObjToIndex = get_obj_index(pReset->arg3)))
        {
          bug("Reset_room: 'P': bad vnum %d.", pReset->arg3);
          continue;
        }

        if (!(LastObj = get_obj_type(pObjToIndex)) || count_obj_list(pObjIndex, LastObj->contains) > 0)
          break;

        pObj = create_object(pObjIndex, number_fuzzy(level));
        obj_to_obj(pObj, LastObj);

        /*
         * Ensure that the container gets reset.    OLC 1.1b
         */
        if (LastObj->item_type == ITEM_CONTAINER)
        {
          LastObj->value[1] = LastObj->pIndexData->value[1];
        }
        else
        {
          /* THIS SPACE INTENTIONALLY LEFT BLANK */
        }
        break;

      case 'G':
      case 'E':
        if (!(pObjIndex = get_obj_index(pReset->arg1)))
        {
          bug("Reset_room: 'E' or 'G': bad vnum %d.", pReset->arg1);
          continue;
        }

        if (!last)
          break;

        if (!LastMob)
        {
          bug("Reset_room: 'E' or 'G': null mob for vnum %d.", pReset->arg1);
          last = FALSE;
          break;
        }

        pObj = create_object(pObjIndex, number_fuzzy(level));
        obj_to_char(pObj, LastMob);
        if (pReset->command == 'E')
          equip_char(LastMob, pObj, pReset->arg3);
        last = TRUE;
        break;

      case 'D':
        break;

      case 'R':
        break;
    }
  }

  return;
}

/* OLC
 * Reset one area.
 */
void reset_area(AREA_DATA * pArea)
{
  ROOM_INDEX_DATA *pRoom;
  int vnum;

  for (vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++)
  {
    if ((pRoom = get_room_index(vnum)))
      reset_room(pRoom);
  }

  return;
}

/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile(MOB_INDEX_DATA * pMobIndex)
{
  CHAR_DATA *mob;
  int tempvalue, i;

  if (pMobIndex == NULL)
  {
    bug("Create_mobile: NULL pMobIndex.", 0);
    exit(1);
  }

  if ((mob = (CHAR_DATA *) PopStack(char_free)) == NULL)
  {
    mob = calloc(1, sizeof(*mob));
  }

  clear_char(mob);
  mob->pIndexData = pMobIndex;

  mob->name         =  str_dup(pMobIndex->player_name);  /* OLC */
  mob->short_descr  =  str_dup(pMobIndex->short_descr);  /* OLC */
  mob->long_descr   =  str_dup(pMobIndex->long_descr);   /* OLC */
  mob->description  =  str_dup(pMobIndex->description);  /* OLC */

  mob->spec_fun   =  pMobIndex->spec_fun;
  mob->quest_fun  =  pMobIndex->quest_fun;
  mob->shop_fun   =  pMobIndex->shop_fun;
  mob->death_fun  =  pMobIndex->death_fun;

  for (i = 0; i < 2; i++)
    mob->home[i]    =  ROOM_VNUM_CITYCENTER;

  mob->form         =  ITEM_WEAR_ALL;
  mob->level        =  number_fuzzy(pMobIndex->level);
  mob->affected_by  =  pMobIndex->affected_by;
  mob->alignment    =  pMobIndex->alignment;
  mob->act          =  pMobIndex->act;
  mob->sex          =  pMobIndex->sex;
  mob->armor        =  100 - 2 * UMIN(150, mob->level) - UMAX(0, mob->level - 150);

  tempvalue = mob->level * 12 + number_range(mob->level * mob->level / 4, mob->level * mob->level);

  if (tempvalue > 400000 || tempvalue < 0)
    mob->max_hit = 400000;
  else
    mob->max_hit = tempvalue;

  mob->hit       =  mob->max_hit;
  mob->hitroll   =  mob->level;
  mob->damroll   =  mob->level;
  mob->practice  =  mob->level * (number_range(10, 20) / 5);

  /* set stances */
  if (mob->level > 100 && !IS_SET(mob->act, ACT_NOSTANCE))
  {
    int count = 0, pick;

    /* all 100+ mobs have up to 3 basic stances */
    for (i = 0; i < 3; i++)
      mob->stance[number_range(1, 5)] = UMIN(200, mob->level / 5);

    /* all 500+ mobs have up to 2 advanced stances */
    for (i = 0; i < 2; i++)
      mob->stance[number_range(6, 10)] = UMIN(200, mob->level / 5);

    /* count the amount of stances */
    for (i = 1; i < 11; i++)
    {
      if (mob->stance[i] > 0)
        count++;
    }

    /* set autostance vs players */
    if (count >= 1)
    {
      pick = number_range(1, count);

      for (i = 1; i < 11; i++)
      {
        if (mob->stance[i] <= 0)
          continue;

        if (--count <= 0)
          break;
      }

      if (i != 11)
        mob->stance[STANCE_PKSTANCE] = i;
    }
  }

  /* initialize events for mobile */
  init_events_mobile(mob);

  /*
   * Insert in list.
   */
  AttachToList(mob, char_list);

  pMobIndex->count++;
  return mob;
}

/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object(OBJ_INDEX_DATA * pObjIndex, int level)
{
  static OBJ_DATA obj_zero;
  OBJ_DATA *obj;

  if (pObjIndex == NULL)
  {
    bug("Create_object: NULL pObjIndex.", 0);
    exit(1);
  }

  if ((obj = (OBJ_DATA *) PopStack(obj_free)) == NULL)
  {
    obj = calloc(1, sizeof(*obj));
  }

  *obj = obj_zero;
  obj->pIndexData = pObjIndex;
  obj->in_room = NULL;
  obj->level = level;
  obj->wear_loc = -1;

  obj->events = AllocList();
  obj->contains = AllocList();
  obj->affected = AllocList();
  obj->extra_descr = AllocList();

  obj->name = str_dup(pObjIndex->name); /* OLC */
  obj->short_descr = str_dup(pObjIndex->short_descr); /* OLC */
  obj->description = str_dup(pObjIndex->description); /* OLC */

  obj->questowner      = str_dup("");

  obj->quest           = 0;
  obj->sentient_points = 0;
  obj->love            = 0;
  obj->spellflags      = 0;

  obj->item_type = pObjIndex->item_type;
  obj->extra_flags = pObjIndex->extra_flags;
  obj->wear_flags = pObjIndex->wear_flags;
  obj->value[0] = pObjIndex->value[0];
  obj->value[1] = pObjIndex->value[1];
  obj->value[2] = pObjIndex->value[2];
  obj->value[3] = pObjIndex->value[3];
  obj->weight = pObjIndex->weight;
  obj->cost = pObjIndex->cost;

  if (obj->pIndexData->vnum >= 171 && obj->pIndexData->vnum <= 200)  /* artifacts */
  {
    SET_BIT(obj->quest, QUEST_ARTIFACT);
    obj->condition = 100;
    obj->toughness = 100;
    obj->resistance = 1;
    obj->level = 60;
    obj->cost = 1000000;
  }
  else if (obj->pIndexData->vnum >= 221 && obj->pIndexData->vnum <= 300)  /* Prizes */
  {
    SET_BIT(obj->quest, QUEST_PRIZE);
    obj->condition = 100;
    obj->toughness = 100;
    obj->resistance = 1;
    obj->level = 60;
    obj->cost = 1000000;
  }
  else if (obj->pIndexData->vnum >= 301 && obj->pIndexData->vnum <= 700) /* relics */
  {
    obj->condition = 100;
    obj->toughness = 100;
    obj->resistance = 1;
    SET_BIT(obj->quest, QUEST_RELIC);
  }
  else
  {
    obj->condition = 100;
    obj->toughness = 5;
    obj->resistance = 25;
  }

  /*
   * Mess with object properties.
   */
  switch (obj->item_type)
  {
    default:
      bug("Read_object: vnum %d bad type.", pObjIndex->vnum);
      break;

    case ITEM_LIGHT:
    case ITEM_TREASURE:
    case ITEM_FURNITURE:
    case ITEM_TRASH:
    case ITEM_QUESTCLUE:
    case ITEM_HOMING:
    case ITEM_CONTAINER:
    case ITEM_DRINK_CON:
    case ITEM_KEY:
    case ITEM_FOOD:
    case ITEM_BOAT:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_FOUNTAIN:
    case ITEM_PORTAL:
    case ITEM_QUEST:
    case ITEM_SCROLL:
    case ITEM_WAND:
    case ITEM_STAFF:
    case ITEM_FAETOKEN:
      break;

    case ITEM_WEAPON:
      if (!IS_SET(obj->quest, QUEST_ARTIFACT) && !IS_SET(obj->quest, QUEST_RELIC) && !IS_SET(obj->quest, QUEST_PRIZE))
      {
        obj->value[1] = number_fuzzy(obj->value[1]);
        obj->value[2] = number_fuzzy(obj->value[2]);
      }
      break;

    case ITEM_ARMOR:
      if (!IS_SET(obj->quest, QUEST_ARTIFACT) && !IS_SET(obj->quest, QUEST_RELIC) && !IS_SET(obj->quest, QUEST_PRIZE))
        obj->value[0] = number_range(10, obj->value[0]);
      break;

    case ITEM_POTION:
    case ITEM_PILL:
      obj->value[0] = number_fuzzy(number_fuzzy(obj->value[0]));
      break;

    case ITEM_MONEY:
      obj->value[0] = obj->cost;
      break;
  }

  /* initialize events */
  init_events_object(obj);

  /* set ITEM_OLC flag on not-active objects */
  if (pObjIndex->area && IS_SET(pObjIndex->area->areabits, AREA_BIT_OLC))
    SET_BIT(obj->extra_flags, ITEM_OLC);

  /* fix anti-alignment */
  if (IS_OBJ_STAT(obj, ITEM_ANTI_EVIL))
  {
    REMOVE_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);
    REMOVE_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
  }
  else if (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))
  {
    REMOVE_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);
    REMOVE_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
  }
  else if (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL))
  {
    REMOVE_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
    REMOVE_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
  }

  AttachToList(obj, object_list);
  pObjIndex->count++;

  return obj;
}

void clear_pcdata(PC_DATA *pcdata)
{
  static PC_DATA pcdata_zero;
  int sn;

  /* do the auto clear */
  *pcdata = pcdata_zero;

  /* set standard values */
  pcdata->brief[BRIEF_3]          = 1;
  pcdata->legend                  = 0;
  pcdata->history                 = AllocList();
  pcdata->session                 = alloc_session();
  pcdata->aliases                 = AllocList();
  pcdata->familiar                = NULL;
  pcdata->pfile                   = NULL;
  pcdata->reply                   = NULL;
  pcdata->partner                 = NULL;
  pcdata->propose                 = NULL;
  pcdata->spells                  = AllocList();
  pcdata->contingency             = AllocList();
  pcdata->feeders                 = AllocList();
  pcdata->quests                  = AllocList();
  pcdata->prev_area               = NULL;
  pcdata->ignores                 = AllocList();
  pcdata->safe_counter            = 0;
  pcdata->status                  = 0;
  pcdata->mean_paradox_counter    = 0;
  pcdata->betting_amount          = 0;
  pcdata->betting_char            = 0;
  pcdata->tempflag                = 0;   
  pcdata->playerid                = 0;   
  pcdata->time_tick               = 0;   
  pcdata->revision                = CURRENT_REVISION;
  pcdata->evolveCount             = 0;
  pcdata->jflags                  = 0;
  pcdata->kingdom                 = 0;
  pcdata->wlck_vnum               = 0;
  pcdata->decapmessage            = str_dup("");
  pcdata->avatarmessage           = str_dup("");
  pcdata->logoutmessage           = str_dup("");
  pcdata->loginmessage            = str_dup("");
  pcdata->tiemessage              = str_dup("");
  pcdata->last_global             = str_dup("");
  pcdata->bamfin                  = str_dup("");
  pcdata->bamfout                 = str_dup("");
  pcdata->last_decap[0]           = str_dup("");
  pcdata->last_decap[1]           = str_dup("");
  pcdata->retaliation[0]          = str_dup("");
  pcdata->retaliation[1]          = str_dup("");
  pcdata->soultarget              = str_dup("");
  pcdata->immcmd                  = str_dup("");
  pcdata->enh_combat              = str_dup("");
  pcdata->account                 = str_dup("");
  pcdata->title                   = str_dup("");
  pcdata->bounty                  = 0;   
  pcdata->conception              = str_dup("");
  pcdata->parents                 = str_dup("");
  pcdata->cparents                = str_dup("");
  pcdata->marriage                = str_dup("");
  pcdata->followers               = 0;
  pcdata->perm_str                = 13;
  pcdata->perm_int                = 13;
  pcdata->perm_wis                = 13;
  pcdata->perm_dex                = 13;
  pcdata->perm_con                = 13;
  for (sn = 0; sn < 3; sn++)
    pcdata->stage[sn]             = 0;
  for (sn = 0; sn < 6; sn++)
    pcdata->genes[sn]             = 0;
  for (sn = 0; sn < 13; sn++)
    pcdata->powers[sn]            = 0;
  pcdata->security                = 0;
  pcdata->condition[COND_THIRST]  = 48;
  pcdata->condition[COND_FULL]    = 48;
}

/*
 * Clear a new character.
 */
void clear_char(CHAR_DATA *ch)
{
  static CHAR_DATA ch_zero;
  int i;

 *ch               =  ch_zero;

  /* char pointers */
  ch->name         =  &str_empty[0];
  ch->short_descr  =  &str_empty[0];
  ch->long_descr   =  &str_empty[0];
  ch->description  =  &str_empty[0];
  ch->morph        =  &str_empty[0];
  ch->createtime   =  &str_empty[0];
  ch->lasthost     =  &str_empty[0];
  ch->lasttime     =  &str_empty[0];
  ch->prompt       =  &str_empty[0];
  ch->cprompt      =  &str_empty[0];

  /* integer default values */
  for (i = 0; i < 7; i++)
    ch->loc_hp[i] =  0;
  for (i = 0; i < 13; i++)
    ch->wpn[i]    =  0;
  for (i = 0; i < 5; i++)
    ch->spl[i]    =  0;
  for (i = 0; i < 14; i++)
    ch->stance[i] =  0;

  ch->events       =  AllocList();
  ch->carrying     =  AllocList();
  ch->affected     =  AllocList();

  /* settings */
  ch->dead         =  FALSE;
  ch->logon        =  current_time;
  ch->xlogon       =  current_time;
  ch->position     =  POS_STANDING;
  ch->act          =  PLR_AUTOEXIT + PLR_MAP + PLR_PROMPT + PLR_AUTOLOOT;
  ch->immune       =  0;
  ch->home[0]      =  ROOM_VNUM_CITYCENTER;
  ch->home[1]      =  ROOM_VNUM_SEWERS;
  ch->home[2]      =  ROOM_VNUM_BLACKDRAGON;

  /* CHAR_DATA pointers */
  ch->respawn      =  NULL;
  ch->challenger   =  NULL;
  ch->precognition =  NULL;
  ch->master       =  NULL;
  ch->leader       =  NULL;
  ch->fighting     =  NULL;
  ch->mount        =  NULL;
  ch->wizard       =  NULL;
  ch->simulacrum   =  NULL;

  ch->played       =  0;
  ch->pkill        =  0;
  ch->pdeath       =  0;
  ch->mkill        =  0;
  ch->mdeath       =  0;
  ch->exp          =  0;
  ch->extra        =  0;
  ch->gcount       =  1;  /* all monster groups starts out as 1 */
  ch->newbits      =  0;
  ch->special      =  0;
  ch->affected_by  =  0;
  ch->carry_weight =  0;
  ch->carry_number =  0;
  ch->saving_throw =  0;
  ch->alignment    =  0;
  ch->hitroll      =  0;
  ch->damroll      =  0;
  ch->deaf         =  CHANNEL_FLAME;
  ch->class        =  0;
  ch->fight_timer  =  0;
  ch->itemaffect   =  0;
  ch->form         =  ITEM_WEAR_ALL;
  ch->mounted      =  0;
  ch->level        =  2;  /* all new characters can save.. */
  ch->trust        =  0;
  ch->generation   =  6;
  ch->damcap[0]    =  1000;
  ch->damcap[1]    =  0;
  ch->practice     =  0;
  ch->hit          =  1500;
  ch->max_hit      =  1500;
  ch->mana         =  1500;
  ch->max_mana     =  1500;
  ch->move         =  1500;
  ch->max_move     =  1500;
  ch->armor        =  100;
}

/*
 * Free a character.
 */
void free_char(CHAR_DATA * ch)
{
  OBJ_DATA *obj;
  IGNORE_DATA *ignore;
  AFFECT_DATA *paf;
  ALIAS_DATA *alias;
  QUEST_DATA *quest;
  FEED_DATA *feed;
  EVENT_DATA *event;
  HISTORY_DATA *pHistory;
  SPELL_DATA *spell;
  ITERATOR *pIter;

  pIter = AllocIterator(ch->carrying);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    extract_obj(obj);
  }
  FreeList(ch->carrying);

  pIter = AllocIterator(ch->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    affect_remove(ch, paf);
  }
  FreeList(ch->affected);

  /* remove any pending events */
  pIter = AllocIterator(ch->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
    dequeue_event(event, TRUE);
  FreeList(ch->events);

  free_string(ch->name);
  free_string(ch->short_descr);
  free_string(ch->long_descr);
  free_string(ch->description);
  free_string(ch->morph);
  free_string(ch->createtime);
  free_string(ch->lasttime);
  free_string(ch->lasthost);
  free_string(ch->prompt);
  free_string(ch->cprompt);

  if (ch->pcdata != NULL)
  {
    pIter = AllocIterator(ch->pcdata->history);
    while ((pHistory = (HISTORY_DATA *) NextInList(pIter)) != NULL)
    {
      free_string(pHistory->message);
      free_string(pHistory->player);
      DetachAtIterator(pIter);
      PushStack(pHistory, history_free);
    }
    FreeList(ch->pcdata->history);

    pIter = AllocIterator(ch->pcdata->ignores);
    while ((ignore = (IGNORE_DATA *) NextInList(pIter)) != NULL)
      free_ignore(ignore, ch);
    FreeList(ch->pcdata->ignores);

    /* free session */
    free_session(ch);

    /* remove all spells */
    pIter = AllocIterator(ch->pcdata->spells);
    while ((spell = (SPELL_DATA *) NextInList(pIter)) != NULL)
      spell_from_char(spell, ch, TRUE);
    FreeList(ch->pcdata->spells);

    pIter = AllocIterator(ch->pcdata->feeders);
    while ((feed = (FEED_DATA *) NextInList(pIter)) != NULL)
      free_feed(ch, feed);
    FreeList(ch->pcdata->feeders);

    pIter = AllocIterator(ch->pcdata->contingency);
    while ((spell = (SPELL_DATA *) NextInList(pIter)) != NULL)
      spell_from_char(spell, ch, FALSE);
    FreeList(ch->pcdata->contingency);

    /* remove all aliases */
    pIter = AllocIterator(ch->pcdata->aliases);
    while ((alias = (ALIAS_DATA *) NextInList(pIter)) != NULL)
      alias_from_player(ch, alias);
    FreeList(ch->pcdata->aliases);

    pIter = AllocIterator(ch->pcdata->quests);
    while ((quest = (QUEST_DATA *) NextInList(pIter)) != NULL)
    {
      quest_from_char(ch, quest);
    }
    FreeList(ch->pcdata->quests);

    free_string(ch->pcdata->account);
    free_string(ch->pcdata->retaliation[0]);
    free_string(ch->pcdata->retaliation[1]);
    free_string(ch->pcdata->last_decap[0]);
    free_string(ch->pcdata->last_decap[1]);
    free_string(ch->pcdata->logoutmessage);
    free_string(ch->pcdata->avatarmessage);
    free_string(ch->pcdata->loginmessage);
    free_string(ch->pcdata->decapmessage);
    free_string(ch->pcdata->tiemessage);
    free_string(ch->pcdata->last_global);
    free_string(ch->pcdata->bamfin);
    free_string(ch->pcdata->bamfout);
    free_string(ch->pcdata->title);
    free_string(ch->pcdata->conception);
    free_string(ch->pcdata->parents);
    free_string(ch->pcdata->cparents);
    free_string(ch->pcdata->marriage);
    free_string(ch->pcdata->soultarget);
    free_string(ch->pcdata->immcmd);
    free_string(ch->pcdata->enh_combat);

    PushStack(ch->pcdata, pcdata_free);
  }

  PushStack(ch, char_free);

  ch->dead = TRUE;
}

/*
 * Get an extra description from a list.
 */
char *get_extra_descr(char *name, LIST *list)
{
  EXTRA_DESCR_DATA *ed;
  ITERATOR *pIter = AllocIterator(list);

  while ((ed = (EXTRA_DESCR_DATA *) NextInList(pIter)) != NULL)
  {
    if (is_name(name, ed->keyword))
      return ed->description;
  }

  return NULL;
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index(int vnum)
{
  MOB_INDEX_DATA *pMobIndex;

  QUICKITER2(mob_index_hash[vnum % MAX_KEY_HASH]);
  while ((pMobIndex = (MOB_INDEX_DATA *) NextInList(QuickIter2)) != NULL)
  {
    if (pMobIndex->vnum == vnum)
      return pMobIndex;
  }

  if (fBootDb)
  {
    bug("Get_mob_index: bad vnum %d.", vnum);
    exit(1);
  }

  return NULL;
}

/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index(int vnum)
{
  OBJ_INDEX_DATA *pObjIndex;

  QUICKITER2(obj_index_hash[vnum % MAX_KEY_HASH]);
  while ((pObjIndex = (OBJ_INDEX_DATA *) NextInList(QuickIter2)) != NULL)
  {
    if (pObjIndex->vnum == vnum)
      return pObjIndex;
  }

  if (fBootDb)
  {
    bug("Get_obj_index: bad vnum %d.", vnum);
    exit(1);
  }

  return NULL;
}

/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index(int vnum)
{
  ROOM_INDEX_DATA *pRoomIndex;

  QUICKITER2(room_index_hash[vnum % MAX_KEY_HASH]);
  while ((pRoomIndex = (ROOM_INDEX_DATA *) NextInList(QuickIter2)) != NULL)
  {
    if (pRoomIndex->vnum == vnum)
      return pRoomIndex;
  }

  if (fBootDb)
  {
    bug("Get_room_index: bad vnum %d.", vnum);
    exit(1);
  }

  return NULL;
}

/*
 * Read a letter from a file.
 */
char fread_letter(FILE * fp)
{
  char c;

  do
  {
    c = getc(fp);
  }
  while (isspace(c));

  return c;
}

/*
 * Read a number from a file.
 */
int fread_number(FILE * fp)
{
  int number;
  bool sign;
  char *ptr;
  int c;

  ptr = top_string + sizeof(char *);

  do
  {
    c = getc(fp);
    *ptr = c;
  }
  while (isspace(c));

  number = 0;

  sign = FALSE;
  if (c == '+')
  {
    c = getc(fp);
    *ptr = c;
  }
  else if (c == '-')
  {
    sign = TRUE;
    c = getc(fp);
    *ptr = c;
  }

  if (!isdigit(c))
  {
    bug("Fread_number: bad format.", 0);
    exit(1);
  }

  while (isdigit(c))
  {
    number = number * 10 + c - '0';
    c = getc(fp);
    *ptr = c;
  }

  if (sign)
    number = 0 - number;

  if (c == '|')
    number += fread_number(fp);
  else if (c != ' ')
    ungetc(c, fp);

  return number;
}

/*
 * Read and allocate space for a string from a file.
  * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 * This function takes 40% to 50% of boot-up time.
 */
char *fread_string(FILE * fp)
{
  char *plast;
  int c;

  plast = top_string + sizeof(char *);
  if (plast > &string_space[MAX_STRING - MAX_STRING_LENGTH])
  {
    bug("Fread_string: MAX_STRING %d exceeded.", MAX_STRING);
    exit(1);
  }

  /*
   * Skip blanks.
   * Read first char.
   */
  do
  {
    c = getc(fp);
  }
  while (isspace(c));

  if ((*plast++ = c) == '~')
    return &str_empty[0];
  for (;;)
  {
    /*
     * Back off the char type lookup,
     *   it was too dirty for portability.
     *   -- Furey
     */
    c = getc(fp);
    *plast = c;
    switch (c)
    {
      default:
        plast++;
        break;

      case EOF:
        bug("Fread_string: EOF", 0);
        exit(1);
        break;

      case '\n':
        plast++;
        *plast++ = '\r';
        break;

      case '\r':
        break;

      case '~':
        plast++;
        {
          union
          {
            char *pc;
            char rgc[sizeof(char *)];
          }
          u1;
          int ic;
          int iHash;
          char *pHash;
          char *pHashPrev;
          char *pString;

          plast[-1] = '\0';
          iHash = UMIN(MAX_KEY_HASH - 1, plast - 1 - top_string);
          for (pHash = string_hash[iHash]; pHash; pHash = pHashPrev)
          {
            for (ic = 0; ic < (int) sizeof(char *); ic++)
              u1.rgc[ic] = pHash[ic];
            pHashPrev = u1.pc;
            pHash += sizeof(char *);

            if (top_string[sizeof(char *)] == pHash[0] && !strcmp(top_string + sizeof(char *) + 1, pHash + 1))
              return pHash;
          }

          if (fBootDb)
          {
            pString = top_string;
            top_string = plast;
            u1.pc = string_hash[iHash];
            for (ic = 0; ic < (int) sizeof(char *); ic++)
              pString[ic] = u1.rgc[ic];
            string_hash[iHash] = pString;

            nAllocString += 1;
            sAllocString += top_string - pString;
            return pString + sizeof(char *);
          }
          else
          {
            return str_dup(top_string + sizeof(char *));
          }
        }
    }
  }
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol(FILE * fp)
{
  char c;

  do
  {
    c = getc(fp);
  }
  while (c != '\n' && c != '\r');

  do
  {
    c = getc(fp);
  }
  while (c == '\n' || c == '\r');

  ungetc(c, fp);
  return;
}

/*
 * Read one word (into static buffer).
 */
char *fread_word(FILE * fp)
{
  static char word[MAX_INPUT_LENGTH];
  char buf[100];
  char *pword;
  char cEnd;

  do
  {
    cEnd = getc(fp);
  }
  while (isspace(cEnd));

  if (cEnd == '\'' || cEnd == '"')
  {
    pword = word;
  }
  else
  {
    word[0] = cEnd;
    pword = word + 1;
    cEnd = ' ';
  }

  for (; pword < word + MAX_INPUT_LENGTH; pword++)
  {
    *pword = getc(fp);
    if (cEnd == ' ' ? isspace(*pword) : *pword == cEnd)
    {
      if (cEnd == ' ')
        ungetc(*pword, fp);
      *pword = '\0';
      return word;
    }
  }

  word[20] = '\0';
  sprintf(buf, "Fread_word: word '%s' too long.", word);
  bug(buf, 0);
  exit(1);
  return NULL;
}

/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
char *str_dup(const char *str)
{
  char *str_new;

  if (str[0] == '\0')
    return &str_empty[0];

  if (str >= string_space && str < top_string)
    return (char *) str;

  str_new = strdup(str);

  return str_new;
}

/*
 * Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched.
 */
void free_string(char *pstr)
{
  if (pstr == NULL || pstr == &str_empty[0] || (pstr >= string_space && pstr < top_string))
    return;

  free(pstr);
}

void do_areas(CHAR_DATA * ch, char *argument)
{
  do_help(ch, "area");
}

void do_memory(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];

  sprintf(buf, "Affects %5d\n\r", top_affect);
  send_to_char(buf, ch);
  sprintf(buf, "Areas   %5d\n\r", top_area);
  send_to_char(buf, ch);
  sprintf(buf, "RmTxt   %5d\n\r", top_rt);
  send_to_char(buf, ch);
  sprintf(buf, "ExDes   %5d\n\r", top_ed);
  send_to_char(buf, ch);
  sprintf(buf, "Exits   %5d\n\r", top_exit);
  send_to_char(buf, ch);
  sprintf(buf, "Helps   %5d\n\r", top_help);
  send_to_char(buf, ch);
  sprintf(buf, "Mobs    %5d\n\r", top_mob_index);
  send_to_char(buf, ch);
  sprintf(buf, "Objs    %5d\n\r", top_obj_index);
  send_to_char(buf, ch);
  sprintf(buf, "Resets  %5d\n\r", top_reset);
  send_to_char(buf, ch);
  sprintf(buf, "Rooms   %5d\n\r", top_room);
  send_to_char(buf, ch);

  sprintf(buf, "Strings %5d strings of %7d bytes (max %d).\n\r", nAllocString, sAllocString, MAX_STRING);
  send_to_char(buf, ch);
}

/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy(int number)
{
  switch (number_bits(2))
  {
    case 0:
      number -= 1;
      break;
    case 3:
      number += 1;
      break;
  }

  return UMAX(1, number);
}

/*
 * Generate a random number.
 */
int number_range(int from, int to)
{
  int power;
  int number;

  if ((to = to - from + 1) <= 1)
    return from;

  for (power = 2; power < to; power <<= 1)
    ;

  while ((number = number_mm() & (power - 1)) >= to)
    ;

  return from + number;
}

/*
 * Generate a percentile roll.
 */
int number_percent(void)
{
  int percent;

  while ((percent = number_mm() & (128 - 1)) > 99)
    ;

  return 1 + percent;
}

/*
 * Generate a random door.
 */
int number_door(void)
{
  int door;

  while ((door = number_mm() & (8 - 1)) > 5)
    ;

  return door;
}

int number_bits(int width)
{
  return number_mm() & ((1 << width) - 1);
}

/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static int rgiState[2 + 55];

void init_mm()
{
  int *piState;
  int iState;

  piState = &rgiState[2];

  piState[-2] = 55 - 55;
  piState[-1] = 55 - 24;

  piState[0] = ((int) current_time) & ((1 << 30) - 1);
  piState[1] = 1;
  for (iState = 2; iState < 55; iState++)
  {
    piState[iState] = (piState[iState - 1] + piState[iState - 2]) & ((1 << 30) - 1);
  }
  return;
}

int number_mm(void)
{
  int *piState;
  int iState1;
  int iState2;
  int iRand;

  piState = &rgiState[2];
  iState1 = piState[-2];
  iState2 = piState[-1];
  iRand = (piState[iState1] + piState[iState2]) & ((1 << 30) - 1);
  piState[iState1] = iRand;
  if (++iState1 == 55)
    iState1 = 0;
  if (++iState2 == 55)
    iState2 = 0;
  piState[-2] = iState1;
  piState[-1] = iState2;
  return iRand >> 6;
}

/*
 * Roll some dice.
 */
int dice(int number, int size)
{
  int idice;
  int sum;

  switch (size)
  {
    case 0:
      return 0;
    case 1:
      return number;
  }

  for (idice = 0, sum = 0; idice < number; idice++)
    sum += number_range(1, size);

  return sum;
}

/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde(char *str)
{
  for (; *str != '\0'; str++)
  {
    if (*str == '~')
      *str = '-';
  }

  return;
}

/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp(const char *astr, const char *bstr)
{
  if (astr == NULL)
  {
    bug("Str_cmp: null astr.", 0);
    return TRUE;
  }

  if (bstr == NULL)
  {
    bug("Str_cmp: null bstr.", 0);
    return TRUE;
  }

  for (; *astr || *bstr; astr++, bstr++)
  {
    if (LOWER(*astr) != LOWER(*bstr))
      return TRUE;
  }

  return FALSE;
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix(const char *astr, const char *bstr)
{
  if (astr == NULL)
  {
    bug("Strn_cmp: null astr.", 0);
    return TRUE;
  }

  if (bstr == NULL)
  {
    bug("Strn_cmp: null bstr.", 0);
    return TRUE;
  }

  for (; *astr; astr++, bstr++)
  {
    if (LOWER(*astr) != LOWER(*bstr))
      return TRUE;
  }

  return FALSE;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix(const char *astr, const char *bstr)
{
  int sstr1;
  int sstr2;
  int ichar;
  char c0;

  if ((c0 = LOWER(astr[0])) == '\0')
    return FALSE;

  sstr1 = strlen(astr);
  sstr2 = strlen(bstr);

  for (ichar = 0; ichar <= sstr2 - sstr1; ichar++)
  {
    if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
      return FALSE;
  }

  return TRUE;
}

/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix(const char *astr, const char *bstr)
{
  int sstr1;
  int sstr2;

  sstr1 = strlen(astr);
  sstr2 = strlen(bstr);
  if (sstr1 <= sstr2 && !str_cmp(astr, bstr + sstr2 - sstr1))
    return FALSE;
  else
    return TRUE;
}

/*
 * Returns an initial-capped string.
 */
char *capitalize(const char *str)
{
  static char strcap[MAX_STRING_LENGTH];
  int i;

  for (i = 0; str[i] != '\0'; i++)
    strcap[i] = LOWER(str[i]);
  strcap[i] = '\0';
  strcap[0] = UPPER(strcap[0]);
  return strcap;
}

/*
 * Reports a bug.
 */
void bug(const char *str, int param)
{
  char buf[MAX_STRING_LENGTH];
  FILE *fp;

  if (fpArea != NULL)
  {
    int iLine;
    int iChar;

    if (fpArea == stdin)
    {
      iLine = 0;
    }
    else
    {
      iChar = ftell(fpArea);
      fseek(fpArea, 0, 0);
      for (iLine = 0; ftell(fpArea) < iChar; iLine++)
      {
        while (getc(fpArea) != '\n')
          ;
      }
      fseek(fpArea, iChar, 0);
    }

    log_string("[*****] FILE: %s LINE: %d", strArea, iLine);
  }

  strcpy(buf, "[*****] BUG: ");
  sprintf(buf + strlen(buf), str, param);

  log_string("%s", buf);

  if ((fp = fopen(BUG_FILE, "a")) != NULL)
  {
    fprintf(fp, "%s\n", buf);
    fclose(fp);
  }
}

/*
 * Writes a string to the log.
 */
void log_string(const char *str, ...)
{
  char buf[MAX_STRING_LENGTH];
  char datef[MAX_INPUT_LENGTH];
  char logfile[MAX_INPUT_LENGTH];
  char *strtime = ctime(&current_time);
  va_list args;
  FILE *fp;

  /* generate logentry */
  va_start(args, str);   
  vsprintf(buf, str, args);
  va_end(args);

  /* broadcast it on wiznet */
  logchan(buf);

  /* generate logfile name */
  strtime[7] = '\0';
  strtime[10] = '\0';
  sprintf(logfile, "../log/%s-%s.txt", &strtime[4], &strtime[8]);

  /* try to open logfile */
  if ((fp = fopen(logfile, "a")) == NULL)
  {
    logchan("log_string: cannot open logfile");
    return;
  }

  /* generate dateformat */
  strtime[19] = '\0';
  sprintf(datef, "%s %s %s", &strtime[4], &strtime[8], &strtime[11]);

  /* output log to file */
  fprintf(fp, "%s :: %s\n", datef, buf);

  /* close logfile */
  fclose(fp);
}

void load_social (FILE *fp, struct social_type *social)
{
  social->name =          fread_string (fp);
  social->char_no_arg =   fread_string (fp);
  social->others_no_arg = fread_string (fp);
  social->char_found =    fread_string (fp);
  social->others_found =  fread_string (fp);
  social->vict_found =    fread_string (fp);
  social->char_auto =     fread_string (fp);
  social->others_auto =   fread_string (fp);
}

void load_social_table ()
{
  FILE *fp;
  int i;

  fp = fopen (SOCIAL_FILE, "r");
  if (!fp)
  {
    bug ("Could not open " SOCIAL_FILE " for reading.",0);
    exit(1);
  }

  fscanf (fp, "%d\n", &maxSocial);

  /* IMPORTANT to use malloc so we can realloc later on */

  social_table = malloc (sizeof(struct social_type) * (maxSocial+1));

  for (i = 0; i < maxSocial; i++)
    load_social (fp,&social_table[i]);

  /* For backwards compatibility */
  social_table[maxSocial].name = str_dup(""); /* empty! */		

  fclose (fp);
}

void save_social (const struct social_type *s, FILE *fp)
{
  /* get rid of (null) */
  fprintf (fp, "%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n\n",
    s->name           ? s->name : "" ,
    s->char_no_arg    ? s->char_no_arg   : "" ,
    s->others_no_arg  ? s->others_no_arg : "" ,
    s->char_found     ? s->char_found    : "" ,
    s->others_found   ? s->others_found  : "" ,
    s->vict_found     ? s->vict_found    : "" ,
    s->char_auto      ? s->char_auto     : "" ,
    s->others_auto    ? s->others_auto   : "");
}


void save_social_table()
{
  FILE *fp;
  int i;

  fp = fopen (SOCIAL_FILE, "w");
  if (!fp)
  {
    bug ("Could not open " SOCIAL_FILE " for writing.",0);
    return;
  }
  for (maxSocial = 0 ; social_table[maxSocial].name[0] ; maxSocial++)
    ; /* empty */
  fprintf (fp, "%d\n", maxSocial);
  for ( i = 0 ; i < maxSocial ; i++)
    save_social (&social_table[i], fp);
  fclose (fp);
}


/* Find a social based on name */ 
int social_lookup (const char *name)
{
  int i;

  for (i = 0; i < maxSocial ; i++)
    if (!str_cmp(name, social_table[i].name))
      return i;

  return -1;
}

void save_muddata()
{
  FILE *fp;
  int i, j;

  for (j = 0; j < 2; j++)
  {
    if (j == 0)
      fp = fopen("../txt/muddata.txt", "w");
    else
      fp = fopen("../txt/muddata.bck", "w");

    if (fp == NULL)
    {
      bug("Unable to write to muddata.txt", 0);
      continue;
    }

    fprintf(fp, "%d\n", muddata.top_playerid);

    for (i = 0; i < (MUDINFO_MAX - 2); i++)
      fprintf(fp, "%d\n", muddata.mudinfo[i]);

    for (i = 0; i < CCENTER_MAX; i++)
      fprintf(fp, "%d\n", muddata.ccenter[i]);

    for (i = 0; i < MAX_CLASS; i++)
      fprintf(fp, "%d\n", muddata.class_count[i]);

    for (i = 0; i < 3; i++)
      fprintf(fp, "%d\n", muddata.pk_count_now[i]);

    for (i = 0; i < 3; i++)
      fprintf(fp, "%d\n", muddata.pk_count_last[i]);

    fclose(fp);
  }
}

void load_muddata()
{
  FILE *fp;
  int i;

  if ((fp = fopen("../txt/muddata.txt", "r")) == NULL)
  {
    bug("Unable to open muddata.txt", 0);
    abort();
  }

  muddata.top_playerid = fread_number(fp);

  for (i = 0; i < (MUDINFO_MAX - 2); i++)
    muddata.mudinfo[i] = fread_number(fp);
  muddata.mudinfo[i++] = 0;
  muddata.mudinfo[i++] = 0;

  for (i = 0; i < CCENTER_MAX; i++)
    muddata.ccenter[i] = fread_number(fp);

  for (i = 0; i < MAX_CLASS; i++)
    muddata.class_count[i] = fread_number(fp);

  for (i = 0; i < 3; i++)
    muddata.pk_count_now[i] = fread_number(fp);

  for (i = 0; i < 3; i++)
    muddata.pk_count_last[i] = fread_number(fp);

  /* set sessions only data */
  muddata.events_queued     =  0;
  muddata.events_allocated  =  0;

  fclose(fp);
}

void load_top10()
{
  TOP10_ENTRY *entry;
  FILE *fp;
  int i;

  top10_list = AllocList();

  if ((fp = fopen("../txt/top10.txt", "r")) == NULL)
  {
    log_string("TOP 10 list not found!");
    return;
  }

  for (i = 0; i < 10; i++)
  {
    entry = alloc_top10entry(NULL);

    entry->name     =  str_dup(fread_word(fp));
    entry->pkscore  =  fread_number(fp);
    entry->pkills   =  fread_number(fp);
    entry->pdeaths  =  fread_number(fp);
    entry->hours    =  fread_number(fp);

    AttachToEndOfList(entry, top10_list);
  }

  fclose(fp);
}

void save_top10()
{
  TOP10_ENTRY *entry;
  ITERATOR *pIter;
  FILE *fp;

  if ((fp = fopen("../txt/top10.txt","w")) == NULL)
  {
    log_string("Error writing to top10.txt");
    return;
  }

  pIter = AllocIterator(top10_list);
  while ((entry = (TOP10_ENTRY *) NextInList(pIter)) != NULL)
  {
    fprintf(fp, "%-14s %5d %3d %3d %3d\n",
      entry->name, entry->pkscore, entry->pkills, entry->pdeaths, entry->hours);
  }

  fclose(fp);
}

void load_leaderboard()
{
  FILE *fp;
  int i;

  if ((fp = fopen("../txt/leader.txt", "r")) == NULL)
  {   
    log_string("Error: leader.txt not found!");
    exit(1);
  }

  for (i = 0; i < MAX_LEADER; i++)
  {
    leader_board.name[i]   = fread_string(fp);
    leader_board.number[i] = fread_number(fp);
  }

  fclose(fp);
}

void save_leaderboard()
{
  FILE *fp;
  int i;

  if ((fp = fopen("../txt/leader.txt","w")) == NULL)
  {
    log_string("Error writing to leader.txt");
    return;
  }

  for (i = 0; i < MAX_LEADER; i++)
  {
    fprintf(fp, "%s~\n", leader_board.name[i]); 
    fprintf(fp, "%d\n",  leader_board.number[i]);
  }

  fclose (fp);
}

void check_leaderboard(CHAR_DATA *ch)
{
  bool changed = FALSE;
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;
  struct tm tm;
  time_t createtime;
  int a, b;

  if (IS_NPC(ch) || ch->level > 6) return;

  if (get_ratio(ch) > leader_board.number[LEADER_PKSCORE] ||
     !str_cmp(leader_board.name[LEADER_PKSCORE], ch->name))
  {
    leader_board.number[LEADER_PKSCORE] = get_ratio(ch);
    free_string(leader_board.name[LEADER_PKSCORE]);
    leader_board.name[LEADER_PKSCORE] = str_dup(ch->name);
    changed = TRUE;
  }
  if (ch->pcdata->status > leader_board.number[LEADER_STATUS] ||
     !str_cmp(leader_board.name[LEADER_STATUS], ch->name))
  {
    leader_board.number[LEADER_STATUS] = ch->pcdata->status;
    free_string(leader_board.name[LEADER_STATUS]);
    leader_board.name[LEADER_STATUS] = str_dup(ch->name);
    changed = TRUE;
  }
  if (ch->mkill > leader_board.number[LEADER_MOBKILLS])
  {
    leader_board.number[LEADER_MOBKILLS] = ch->mkill;
    free_string(leader_board.name[LEADER_MOBKILLS]);
    leader_board.name[LEADER_MOBKILLS] = str_dup(ch->name);
    changed = TRUE;
  }
  if (ch->pkill > leader_board.number[LEADER_PK])
  {
    leader_board.number[LEADER_PK] = ch->pkill;
    free_string(leader_board.name[LEADER_PK]);
    leader_board.name[LEADER_PK] = str_dup(ch->name);
    changed = TRUE;
  }
  if (ch->pcdata->questsrun > leader_board.number[LEADER_QUEST])
  {
    leader_board.number[LEADER_QUEST] = ch->pcdata->questsrun;
    free_string(leader_board.name[LEADER_QUEST]);
    leader_board.name[LEADER_QUEST] = str_dup(ch->name);
    changed = TRUE;
  }
  if ((ch->played + (int) (current_time - ch->logon))/3600 > leader_board.number[LEADER_TIME])
  {
    leader_board.number[LEADER_TIME] = (ch->played + (int) (current_time - ch->logon))/3600;
    free_string(leader_board.name[LEADER_TIME]);
    leader_board.name[LEADER_TIME] = str_dup(ch->name);
    changed = TRUE;
  }
  if (ch->pcdata->awins > leader_board.number[LEADER_ARENA])
  {
    leader_board.number[LEADER_ARENA] = ch->pcdata->awins;
    free_string(leader_board.name[LEADER_ARENA]);
    leader_board.name[LEADER_ARENA] = str_dup(ch->name);
    changed = TRUE;
  }

  /* this ONLY works on UNIX machines... beware */
  strptime(ch->createtime, "%a %b %d %T %Y", &tm);
  createtime = mktime(&tm);

  /* how many minutes since creation (+1 to avoid 0) */
  a = (int) (current_time - createtime) / 60 + 1;

  /* how many minutes have been spent online */
  b = (ch->played + (int) (current_time - ch->logon)) / 60;

  /* more than 24 hours played required */
  if (b > 3600 && (1000 * b / a > leader_board.number[LEADER_ACTIVE] ||
     !str_cmp(leader_board.name[LEADER_ACTIVE], ch->name)))
  {
    leader_board.number[LEADER_ACTIVE] = 1000 * b / a;
    free_string(leader_board.name[LEADER_ACTIVE]);
    leader_board.name[LEADER_ACTIVE] = str_dup(ch->name);
    changed = TRUE;
  }

  pIter = AllocIterator(kingdom_list);
  while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
  {
    if (kingdom->treasury > leader_board.number[LEADER_KINGDOM] ||
       !str_cmp(kingdom->shortname, leader_board.name[LEADER_KINGDOM]))
    {
      free_string(leader_board.name[LEADER_KINGDOM]);
      leader_board.name[LEADER_KINGDOM] = str_dup(kingdom->shortname);
      leader_board.number[LEADER_KINGDOM] = kingdom->treasury;
      changed = TRUE;
    }
  }

  if (changed) save_leaderboard();
}

void write_mudinfo_database()
{
  FILE *fp;
  int ratio, mxpusers, mspusers, avusers, a, b, c;

  if ((fp = fopen("../txt/mud_data.txt","a")) == NULL)
  {
    log_string("Error writing to mud_data.txt");
    return;
  }

  /* Calculate the ratio of users that use msp */
  mspusers =
    (100 * muddata.mudinfo[MUDINFO_MSP_USERS] /
    (muddata.mudinfo[MUDINFO_MCCP_USERS] +
     muddata.mudinfo[MUDINFO_OTHER_USERS]));

  /* Calculate the ratio of users that use mxp */
  mxpusers =
    (100 * muddata.mudinfo[MUDINFO_MXP_USERS] /
    (muddata.mudinfo[MUDINFO_MCCP_USERS] +
     muddata.mudinfo[MUDINFO_OTHER_USERS]));

  /* Calculate the ratio of users that use mccp */
  ratio =
    (100 * muddata.mudinfo[MUDINFO_MCCP_USERS] /
    (muddata.mudinfo[MUDINFO_MCCP_USERS] +
     muddata.mudinfo[MUDINFO_OTHER_USERS]));

  /* Calculate the amount of average players online */
  avusers =
    (muddata.mudinfo[MUDINFO_MCCP_USERS] +  
     muddata.mudinfo[MUDINFO_OTHER_USERS]) /
     muddata.mudinfo[MUDINFO_UPDATED];

  /* Calculate the average tranfer rate in kbyte */
  a = muddata.mudinfo[MUDINFO_MBYTE] * 1024 + muddata.mudinfo[MUDINFO_BYTE] / 1024;
  b = a / (muddata.mudinfo[MUDINFO_UPDATED] * 3);
  c = b / 10;
  c = c * 10;
  c = b - c;

  /* Append it all to the file */
  fprintf(fp, "\nMudinfo Database Entry\n");
  fprintf(fp, "Average Online Users       %d\n", avusers);
  fprintf(fp, "Peak Users Online          %d\n", muddata.mudinfo[MUDINFO_PEAK_USERS]);
  fprintf(fp, "Mccp Ratio                 %d %%\n", ratio);
  fprintf(fp, "MSP Ratio                  %d %%\n", mspusers);
  fprintf(fp, "MXP Ratio                  %d %%\n", mxpusers);
  fprintf(fp, "Amount of MB send          %d MB\n", muddata.mudinfo[MUDINFO_MBYTE]);
  fprintf(fp, "Datatransfer Average       %d.%d\n", b/10, c);

  /* Calculating the peak transfer rate */
  b = muddata.mudinfo[MUDINFO_DATA_PEAK] / (3 * 1024);
  c = b / 10;
  c = c * 10;
  c = b - c;

  fprintf(fp, "Datatransfer Peak          %d.%d\n", b/10, c);

  fclose(fp);
}

void load_newbiebans()
{
  FILE *fp;
  BAN_DATA *p;
  char *name;

  newbieban_list = AllocList();

  if ((fp = fopen(NEWBIEBAN_LIST, "r")) == NULL)
    return;

  name = fread_word(fp);
  while (str_cmp(name, END_MARKER))
  {
    p = calloc(1, sizeof(BAN_DATA));
    p->name = str_dup(name);
    p->reason = fread_string(fp);

    AttachToList(p, newbieban_list);

    name = fread_word(fp);
  }
  fclose(fp);
}

void save_newbiebans()
{
  FILE *fp;
  ITERATOR *pIter;
  BAN_DATA *p;

  if (!newbieban_list)
  {
    unlink (BAN_LIST);
    return;
  }
  fp = fopen (NEWBIEBAN_LIST, "w");
  if (!fp)
  {
    bug("could not open newban.txt",0);
    return;
  }

  pIter = AllocIterator(newbieban_list);
  while ((p = (BAN_DATA *) NextInList(pIter)) != NULL)
  {
    fprintf(fp, "%s\n", p->name);
    fprintf(fp, "%s~\n", p->reason);
  }
  fprintf (fp, "%s\n",END_MARKER);
  fclose (fp); 
}

void load_polls()
{
  POLL_DATA *poll;
  FILE *fp;
  char *name;
  int i;

  if ((fp = fopen("../txt/votes.txt", "r")) == NULL)
  {
    log_string("Non-fatal error: votes.txt not found!");
    return;  
  }
  name = fread_word(fp);
  while (str_cmp(name, END_MARKER))
  {
    poll = malloc(sizeof(POLL_DATA));
    poll->name = str_dup(name);
    poll->description = fread_string(fp);
    poll->expire = fread_number(fp);
    poll->votes = AllocList();

    for (i = 0; i < MAX_VOTE_OPTIONS; i++)
    {
      poll->options[i] = fread_string(fp);
      poll->vcount[i]  = fread_number(fp);
    }
    load_subvotes(poll);

    AttachToList(poll, poll_list);
    name = fread_word(fp);
  }
  fclose(fp);
}  

void load_subvotes(POLL_DATA *poll)
{  
  FILE *fp;
  VOTE_DATA *vote;
  char strsave[200];
  char *name;
     
  sprintf(strsave, "../votes/%s", poll->name);
  if ((fp = fopen(strsave, "r")) == NULL) 
  {  
    log_string("Fatal error: vote file not found!");
    exit(1);
  }
  name = fread_word(fp);  
  while (str_cmp(name, END_MARKER))
  {
    vote = malloc(sizeof(VOTE_DATA));
    vote->pname = str_dup(name);
    vote->phost = fread_string(fp);
    vote->choice = fread_number(fp);

    AttachToList(vote, poll->votes);
    name = fread_word(fp);
  }  
  fclose(fp);
}

void save_polls()
{
  FILE *fp;
  POLL_DATA *poll;
  ITERATOR *pIter;
  int i;
    
  if ((fp = fopen("../txt/votes.txt", "w")) == NULL)
  {
    log_string("Non-fatal error: could not save votes.txt!");
    return;
  }  

  pIter = AllocIterator(poll_list);
  while ((poll = (POLL_DATA *) NextInList(pIter)) != NULL)
  {
    fprintf(fp, "%s\n", poll->name);
    fprintf(fp, "%s~\n", poll->description);
    fprintf(fp, "%ld\n", poll->expire);

    for (i = 0; i < MAX_VOTE_OPTIONS; i++)
    {
      fprintf(fp, "%s~\n", poll->options[i]);
      fprintf(fp, "%d\n", poll->vcount[i]);
    }
  }
  fprintf(fp, "%s\n", END_MARKER);
  fclose(fp);
}   

void save_subvotes(POLL_DATA *poll)
{
  FILE *fp;
  ITERATOR *pIter;
  VOTE_DATA *vote;
  char strsave[200];
   
  sprintf(strsave, "../votes/%s", poll->name);
  if ((fp = fopen(strsave, "w")) == NULL)
  {
    log_string("Non-fatal error: could not save vote file!");
    return;
  }

  pIter = AllocIterator(poll->votes);
  while ((vote = (VOTE_DATA *) NextInList(pIter)) != NULL)
  {
    fprintf(fp, "%-12s %-40s~ %2d\n", vote->pname, vote->phost, vote->choice);
  }
  fprintf(fp, "%s\n", END_MARKER);
  fclose(fp);
}

void load_bans()
{
  FILE *fp;
  BAN_DATA *p;
  char *name;

  ban_list = AllocList();

  if ((fp = fopen (BAN_LIST, "r")) == NULL)
    return;

  name = fread_word(fp);
  while (str_cmp(name, END_MARKER))
  {
    p = calloc(1, sizeof(BAN_DATA));
    p->name = str_dup(name);
    p->reason = fread_string(fp);

    AttachToList(p, ban_list);

    name = fread_word(fp);
  }
  fclose(fp); 
}

void save_bans()
{        
  FILE *fp;
  ITERATOR *pIter;
  BAN_DATA *p;

  if (SizeOfList(ban_list) == 0)
  {
    unlink(BAN_LIST);
    return;
  }

  if ((fp = fopen(BAN_LIST, "w")) == NULL)
  {
    bug("could not open ban.txt",0);
    return;
  }

  pIter = AllocIterator(ban_list);
  while ((p = (BAN_DATA *) NextInList(pIter)) != NULL)
  {
    fprintf(fp, "%s\n", p->name);
    fprintf(fp, "%s~\n", p->reason);
  }
  fprintf (fp, "%s\n",END_MARKER);
  fclose (fp);
}

void load_changes()
{  
  CHANGE_DATA *change;
  FILE *fp;
  char *name;

  if ((fp = fopen("../txt/changes.txt", "r")) == NULL)
  {
    log_string("Non-fatal error: changes.txt not found!");
    return;
  }

  name = fread_word(fp);
  while (str_cmp(name, END_MARKER))
  {
    change = malloc(sizeof(CHANGE_DATA));
    change->imm  = str_dup(name);
    change->date = fread_string(fp);
    change->text = fread_string(fp);

    AttachToList(change, change_list);

    name = fread_word(fp);
  }
  
  fclose(fp);
}

void save_changes()
{
  FILE *fp;
  CHANGE_DATA *change;
  ITERATOR *pIter;
  int i = 0;
   
  if ((fp = fopen("../txt/changes.txt","w")) == NULL)
  {
    log_string("Error writing to changes.txt");
    return;
  }

  pIter = AllocReverseIterator(change_list);
  while ((change = (CHANGE_DATA *) NextInList(pIter)) != NULL)
  {
    if (++i > MAX_CHANGE)
      break;

    fprintf(fp, "%s\n", change->imm);
    fprintf(fp, "%s~\n", change->date);
    fprintf(fp, "%s~\n", change->text);
  }
  fprintf(fp, "%s\n", END_MARKER);
  fclose(fp);
}

ACCOUNT_DATA *load_account(char *name)
{
  ACCOUNT_DATA *account = NULL;
  FILE *fp;
  char pfile[256];
  char *word;
  bool done = FALSE, found;

  /* open the account so we can read from it */
  sprintf(pfile, "../accounts/%s/account.dat", capitalize(name));
  if ((fp = fopen(pfile, "r")) == NULL)
    return NULL;

  /* create new account data */
  account = alloc_account();

  /* load data */
  word = fread_word(fp);
  while (!done)
  {
    found = FALSE;
    switch (word[0])
    {
      default:
        break;
      case 'B':
        if (!str_cmp(word, "Boards"))
        {
          int i, num = fread_number(fp);
          char *boardname;

          for (; num; num--)
          {
            boardname = fread_word(fp);
            i = board_lookup(boardname);

            if (i == BOARD_NOTFOUND)
            {
              fread_number(fp);
            }
            else
              account->last_note[i] = fread_number(fp);
          }
          found = TRUE;
        }
        break;
      case 'C':
        IREAD( "Created",      account->created       );
        break;
      case 'D':
	IREAD( "Denied",       account->denied        );
        break;
      case 'E':
        if (!str_cmp(word, "EOF")) {done = TRUE; found = TRUE; break;}
        break;
      case 'F':
        IREAD( "Flags",        account->flags         );
        break;
      case 'G':
        IREAD( "Goldcrowns",   account->goldcrowns    );
        IREAD( "Goldtotal",    account->goldtotal     );
        break;
      case 'L':
        IREAD( "LastLogged",   account->lastlogged    );
	IREAD( "Level",        account->level         );
	break;
      case 'M':
        IREAD( "MaxMight",     account->max_might     );
        break;
      case 'N':
        SREAD( "Notes",        account->notes         );
        break;
      case 'O':
        SREAD( "Owner",        account->owner         );
        break;
      case 'P':
        SREAD( "Password",     account->password      );
	IREAD( "PCount",       account->p_count       );
	SREAD( "Players",      account->players       );
        IREAD( "Popup",        account->popup         );
        break;
      case 'R':
        IREAD( "ReimbPoints",  account->reimb_points  );
        SREAD( "Reference",    account->reference     );
        break;
      case 'T':
        IREAD( "TimeZone",     account->timezone      );
        break;
    }
    if (!found)
    {
      char buf[MAX_STRING_LENGTH];

      sprintf(buf, "Load_account: unexpected '%s' in /%s/account.dat.", word, name);
      bug(buf, 0);
      close_account(account);
      return NULL;
    }

    /* read one more */
    if (!done) word = fread_word(fp);
  }

  fclose(fp);
  return account;
}

void save_account(ACCOUNT_DATA *account)
{
  FILE *fp;
  char pfile[256];
  int i;

  /* open the account file so we can write to it */
  sprintf(pfile, "../accounts/%s/account.dat", account->owner);
  if ((fp = fopen(pfile, "w")) == NULL)
  {
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "Unable to write to %s's account file", account->owner);
    bug(buf, 0);
    return;
  }

  /* save the account data */
  fprintf(fp, "Created           %ld\n",   account->created      );
  fprintf(fp, "Denied            %ld\n",   account->denied       );
  fprintf(fp, "LastLogged        %ld\n",   current_time          );
  fprintf(fp, "Level             %d\n",    account->level        );
  fprintf(fp, "Owner             %s~\n",   account->owner        );
  fprintf(fp, "Password          %s~\n",   account->password     );
  fprintf(fp, "PCount            %d\n",    account->p_count      );
  fprintf(fp, "Players           %s~\n",   account->players      );
  fprintf(fp, "Reference         %s~\n",   account->reference    );
  fprintf(fp, "Flags             %d\n",    account->flags        );
  fprintf(fp, "ReimbPoints       %d\n",    account->reimb_points );
  fprintf(fp, "Notes             %s~\n",   account->notes        );
  fprintf(fp, "MaxMight          %d\n",    account->max_might    );
  fprintf(fp, "TimeZone          %d\n",    account->timezone     );
  fprintf(fp, "Goldcrowns        %d\n",    account->goldcrowns   );
  fprintf(fp, "Goldtotal         %d\n",    account->goldtotal    );
  fprintf(fp, "Popup             %d\n",    account->popup        );

  /* Save note board status */
  fprintf(fp, "Boards            %d ", MAX_BOARD);
  for (i = 0; i < MAX_BOARD; i++)
    fprintf(fp, "%s %ld ", boards[i].short_name, account->last_note[i]);
  fprintf(fp, "\n");

  /* terminate the file */
  fprintf(fp, "EOF\n");
  fclose(fp);

  /* force reload */
  force_account_reload(account);
}
