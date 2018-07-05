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


/***************************************************************************
 *  File: olc.h                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/*
 * This is a header file for all the OLC files.  Feel free to copy it into
 * merc.h if you wish.  Many of these routines may be handy elsewhere in
 * the code.  -Jason Dinkel
 */

/*
 * The version info.  Please use this info when reporting bugs.
 * It is displayed in the game by typing 'version' while editing.
 * Do not remove these from the code - by request of Jason Dinkel
 */
#define VERSION	"ILAB Online Creation [Beta 1.1]"
#define AUTHOR	"     By Jason(jdinkel@mines.colorado.edu)"
#define DATE	"     (May. 15, 1995)"
#define CREDITS "     Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)"

/*
 * New typedefs.
 */
typedef bool OLC_FUN (CHAR_DATA * ch, char *argument);

#define DECLARE_OLC_FUN( fun )	OLC_FUN    fun

/*
 * Connected states for editor.
 */
#define ED_AREA     1
#define ED_ROOM     2
#define ED_OBJECT   3
#define ED_MOBILE   4

/*
 * Interpreter Prototypes
 */
void aedit (CHAR_DATA * ch, char *argument);
void redit (CHAR_DATA * ch, char *argument);
void medit (CHAR_DATA * ch, char *argument);
void oedit (CHAR_DATA * ch, char *argument);

/*
 * OLC Constants
 */
#define MAX_MOB	1               /* Default maximum number for resetting mobs */

/*
 * Structure for an OLC editor command.
 */
struct olc_cmd_type
{
  char *const name;
  OLC_FUN *olc_fun;
};

/*
 * Structure for an OLC editor startup command.
 */
struct editor_cmd_type
{
  char *const name;
  DO_FUN *do_fun;
};

/*
 * Utils.
 */
AREA_DATA *get_vnum_area (int vnum);
AREA_DATA *get_area_data (int vnum);
int flag_value (const struct flag_type * flag_table, char *argument);
void add_reset (ROOM_INDEX_DATA * room, RESET_DATA * pReset, int idx);
char *flag_string (const struct flag_type * flag_table, int bits);

/*
 * Interpreter Table Prototypes
 */
extern const struct olc_cmd_type aedit_table[];
extern const struct olc_cmd_type redit_table[];
extern const struct olc_cmd_type oedit_table[];
extern const struct olc_cmd_type medit_table[];

/*
 * General Functions
 */
bool show_commands (CHAR_DATA * ch, char *argument);
bool show_help (CHAR_DATA * ch, char *argument);
bool edit_done (CHAR_DATA * ch);
bool show_version (CHAR_DATA * ch, char *argument);

/*
 * Area Editor Prototypes
 */
DECLARE_OLC_FUN( aedit_show	);
DECLARE_OLC_FUN( aedit_cvnum	);
DECLARE_OLC_FUN( aedit_create	);
DECLARE_OLC_FUN( aedit_name	);
DECLARE_OLC_FUN( aedit_file	);
DECLARE_OLC_FUN( aedit_music	);
DECLARE_OLC_FUN( aedit_reset	);
DECLARE_OLC_FUN( aedit_security	);
DECLARE_OLC_FUN( aedit_builder	);
DECLARE_OLC_FUN( aedit_vnum	);
DECLARE_OLC_FUN( aedit_lvnum	);
DECLARE_OLC_FUN( aedit_uvnum	);

/*
 * Room Editor Prototypes
 */
DECLARE_OLC_FUN(redit_spell);
DECLARE_OLC_FUN(redit_show);
DECLARE_OLC_FUN(redit_create);
DECLARE_OLC_FUN(redit_name);
DECLARE_OLC_FUN(redit_desc);
DECLARE_OLC_FUN(redit_ed);
DECLARE_OLC_FUN(redit_format);
DECLARE_OLC_FUN(redit_north);
DECLARE_OLC_FUN(redit_south);
DECLARE_OLC_FUN(redit_east);
DECLARE_OLC_FUN(redit_west);
DECLARE_OLC_FUN(redit_up);
DECLARE_OLC_FUN(redit_down);
DECLARE_OLC_FUN(redit_move);
DECLARE_OLC_FUN(redit_mreset);
DECLARE_OLC_FUN(redit_oreset);
DECLARE_OLC_FUN(redit_mlist);
DECLARE_OLC_FUN(redit_teleport);
DECLARE_OLC_FUN(redit_portal);
DECLARE_OLC_FUN(redit_rprog);
DECLARE_OLC_FUN(redit_texttrig);
DECLARE_OLC_FUN(redit_action);
DECLARE_OLC_FUN(redit_rlist);
DECLARE_OLC_FUN(redit_olist);
DECLARE_OLC_FUN(redit_mshow);
DECLARE_OLC_FUN(redit_oshow);

/*
 * Object Editor Prototypes
 */
DECLARE_OLC_FUN(oedit_show);
DECLARE_OLC_FUN(oedit_create);
DECLARE_OLC_FUN(oedit_name);
DECLARE_OLC_FUN(oedit_short);
DECLARE_OLC_FUN(oedit_long);
DECLARE_OLC_FUN(oedit_addaffect);
DECLARE_OLC_FUN(oedit_delaffect);
DECLARE_OLC_FUN(oedit_value0);
DECLARE_OLC_FUN(oedit_value1);
DECLARE_OLC_FUN(oedit_value2);
DECLARE_OLC_FUN(oedit_value3);
DECLARE_OLC_FUN(oedit_weight);
DECLARE_OLC_FUN(oedit_cost);
DECLARE_OLC_FUN(oedit_ed);

/*
 * Mobile Editor Prototypes
 */
DECLARE_OLC_FUN(medit_show);
DECLARE_OLC_FUN(medit_create);
DECLARE_OLC_FUN(medit_name);
DECLARE_OLC_FUN(medit_short);
DECLARE_OLC_FUN(medit_long);
DECLARE_OLC_FUN(medit_desc);
DECLARE_OLC_FUN(medit_level);
DECLARE_OLC_FUN(medit_align);
DECLARE_OLC_FUN(medit_spec);
DECLARE_OLC_FUN(medit_quest);
DECLARE_OLC_FUN(medit_death);
DECLARE_OLC_FUN(medit_shop);
DECLARE_OLC_FUN(medit_weapon);
DECLARE_OLC_FUN(medit_value0);
DECLARE_OLC_FUN(medit_value1);
DECLARE_OLC_FUN(medit_value2);
DECLARE_OLC_FUN(medit_value3);
DECLARE_OLC_FUN(medit_value4);

/*
 * Macros
 */
#define IS_BUILDER(ch, Area)	(( strstr( Area->builders, ch->name )  \
                                || ch->level == MAX_LEVEL              \
				|| strstr( Area->builders, "All" )))

#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))

/* Return pointers to what is being edited. */
#define EDIT_MOB(Ch, Mob)	( Mob = (MOB_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_OBJ(Ch, Obj)	( Obj = (OBJ_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_ROOM(Ch, Room)	( Room = Ch->in_room )
#define EDIT_AREA(Ch, Area)	( Area = (AREA_DATA *)Ch->desc->pEdit )

/*
 * Prototypes
 */
/* mem.c - memory prototypes. */
#define ED	EXTRA_DESCR_DATA
RESET_DATA *new_reset_data (void);
void free_reset_data (RESET_DATA * pReset);
AREA_DATA *new_area (void);
EXIT_DATA *new_exit (void);
void free_exit (EXIT_DATA * pExit);
ED *new_extra_descr (void);
void free_extra_descr (ED * pExtra);
ROOM_INDEX_DATA *new_room_index (void);
AFFECT_DATA *new_affect (void);
void free_affect (AFFECT_DATA * pAf);
OBJ_INDEX_DATA *new_obj_index (void);
MOB_INDEX_DATA *new_mob_index (void);

#undef	ED
