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
 *  File: olc_save.c                                                       *
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

/* OLC_SAVE.C
 * This takes care of saving all the .are information.
 * Notes:
 * -If a good syntax checker is used for setting vnum ranges of areas
 *  then it would become possible to just cycle through vnums instead
 *  of using the iHash stuff and checking that the room or reset or
 *  mob etc is part of that area.
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dystopia.h"
#include "olc.h"

/* dirty little hack to improve the performance when saving the world */
ITERATOR *QuickIter = NULL;
ITERATOR *QuickIter2 = NULL;

/*****************************************************************************
 Name:		fix_string
 Purpose:	Returns a string without \r and ~.
 ****************************************************************************/
char *fix_string(const char *str)
{
  static char strfix[MAX_STRING_LENGTH];
  int i;
  int o;

  if (str == NULL)
    return '\0';

  for (o = i = 0; str[i + o] != '\0'; i++)
  {
    if (str[i + o] == '\r' || str[i + o] == '~')
      o++;
    if ((strfix[i] = str[i + o]) == '\0')
      break;
  }
  strfix[i] = '\0';
  return strfix;
}

/*
 * doubles %%'s - a must for saving helps.
 */
char *fix_string2(const char *str)
{
  static char strfix[2 * MAX_STRING_LENGTH];
  int i;
  int o;

  if (str == NULL)
    return '\0';

  for (o = i = 0; str[i] != '\0'; i++)
  {
    if ((strfix[i + o] = str[i]) == '%')
    {
      o++;
      strfix[i + o] = '%';
    }
  }
  strfix[i + o] = '\0';
  return strfix;
}

/*****************************************************************************
 Name:		save_area_list
 Purpose:	Saves the listing of files to be loaded at startup.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area_list()
{
  FILE *fp;
  AREA_DATA *pArea;
  ITERATOR *pIter;

  if ((fp = fopen("../txt/area.lst", "w")) == NULL)
  {
    bug("Save_area_list: fopen", 0);
    perror("area.lst");
  }
  else
  {
    /* first save _all_ generic areas */
    pIter = AllocIterator(area_list);
    while ((pArea = (AREA_DATA *) NextInList(pIter)) != NULL)
    {
      if (!str_prefix("Generic", pArea->name))
        fprintf(fp, "%s\n", pArea->filename);
    }

    /* then save all the non-generic areas */
    pIter = AllocIterator(area_list);
    while ((pArea = (AREA_DATA *) NextInList(pIter)) != NULL)
    {
      if (str_prefix("Generic", pArea->name))
        fprintf(fp, "%s\n", pArea->filename);
    }

    fprintf(fp, "$\n");
    fclose(fp);
  }

  return;
}

/*****************************************************************************
 Name:		save_mobiles
 Purpose:	Save #MOBILES secion of an area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_mobiles(FILE * fp, AREA_DATA * pArea)
{
  int vnum;
  MOB_INDEX_DATA *pMobIndex;

  fprintf(fp, "#MOBILES\n");
  for (vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++)
  {
    if ((pMobIndex = get_mob_index(vnum)))
    {
      if (pMobIndex->area == pArea)
      {
        fprintf(fp, "#%d\n", pMobIndex->vnum);
        fprintf(fp, "%s~\n", pMobIndex->player_name);
        fprintf(fp, "%s~\n", pMobIndex->short_descr);
        fprintf(fp, "%s~\n", fix_string(pMobIndex->long_descr));
        fprintf(fp, "%s~\n", fix_string(pMobIndex->description));
        fprintf(fp, "%d ", pMobIndex->act);
        fprintf(fp, "%d ", pMobIndex->affected_by);
        fprintf(fp, "%d\n", pMobIndex->alignment);
        fprintf(fp, "%d ", pMobIndex->level);
        fprintf(fp, "%d %d %d %d %d\n",
          pMobIndex->toughness, pMobIndex->extra_attack, pMobIndex->dam_modifier,
          pMobIndex->extra_parry, pMobIndex->extra_dodge);
        fprintf(fp, "%d %d\n", pMobIndex->sex, pMobIndex->natural_attack);

        fprintf( fp, "S\n");
      }
    }
  }
  fprintf(fp, "#0\n\n\n\n");
  return;
}

/*****************************************************************************
 Name:		save_objects
 Purpose:	Save #OBJECTS section of an area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_objects(FILE * fp, AREA_DATA * pArea)
{
  int vnum;
  OBJ_INDEX_DATA *pObjIndex;
  AFFECT_DATA *pAf;
  EXTRA_DESCR_DATA *pEd;

  fprintf(fp, "#OBJECTS\n");
  for (vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++)
  {
    if ((pObjIndex = get_obj_index(vnum)))
    {
      if (pObjIndex->area == pArea)
      {
        fprintf(fp, "#%d\n", pObjIndex->vnum);
        fprintf(fp, "%s~\n", pObjIndex->name);
        fprintf(fp, "%s~\n", pObjIndex->short_descr);
        fprintf(fp, "%s~\n", fix_string(pObjIndex->description));
        fprintf(fp, "%d ", pObjIndex->item_type);
        fprintf(fp, "%d ", pObjIndex->extra_flags);
        fprintf(fp, "%d\n", pObjIndex->wear_flags);

        switch(pObjIndex->item_type)
        {
          default:
            fprintf(fp, "%d %d %d %d\n", pObjIndex->value[0], pObjIndex->value[1], pObjIndex->value[2], pObjIndex->value[3]);
            break;
          case ITEM_SCROLL:
          case ITEM_POTION:
          case ITEM_PILL:
            fprintf(fp, "%d '%s' '%s' '%s'\n", pObjIndex->value[0],
            (pObjIndex->value[1] >= 0 && pObjIndex->value[1] < MAX_SKILL) ? skill_table[pObjIndex->value[1]].name : "reserved",
            (pObjIndex->value[2] >= 0 && pObjIndex->value[2] < MAX_SKILL) ? skill_table[pObjIndex->value[2]].name : "reserved",
            (pObjIndex->value[3] >= 0 && pObjIndex->value[3] < MAX_SKILL) ? skill_table[pObjIndex->value[3]].name : "reserved");
            break;
          case ITEM_STAFF:
          case ITEM_WAND:
            fprintf(fp, "%d %d %d '%s'\n", pObjIndex->value[0], pObjIndex->value[1], pObjIndex->value[2],
            (pObjIndex->value[3] >= 0 && pObjIndex->value[3] < MAX_SKILL) ? skill_table[pObjIndex->value[3]].name : "reserved");
            break;
        }

        fprintf(fp, "%d ", pObjIndex->weight);
        fprintf(fp, "%d\n", pObjIndex->cost);

        QUICKITER(pObjIndex->affected);
        while ((pAf = (AFFECT_DATA *) NextInList(QuickIter)) != NULL)
        {
          fprintf(fp, "A\n%d %d\n", pAf->location, pAf->modifier);
        }

        QUICKITER(pObjIndex->extra_descr);
        while ((pEd = (EXTRA_DESCR_DATA *) NextInList(QuickIter)) != NULL)
        {
          fprintf(fp, "E\n%s~\n%s~\n%s~\n%s~\n%d %d %d\n", pEd->keyword, fix_string(pEd->description), pEd->buffer1, pEd->buffer2, pEd->type, pEd->vnum, pEd->action);
        }
      }
    }
  }
  fprintf(fp, "#0\n\n\n\n");
}

/* OLC 1.1b */
/*****************************************************************************
 Name:		save_rooms
 Purpose:	Save #ROOMDATA section of an area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_rooms(FILE * fp, AREA_DATA * pArea)
{
  ROOM_INDEX_DATA *pRoomIndex;
  EXTRA_DESCR_DATA *pEd;
  ROOMTEXT_DATA *prt;
  EXIT_DATA *pExit;
  int vnum;
  int door;

  fprintf(fp, "#ROOMDATA\n");
  for (vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++)
  {
    if ((pRoomIndex = get_room_index(vnum)))
    {
      if (pRoomIndex->area == pArea)
      {
        fprintf(fp, "#%d\n", pRoomIndex->vnum);
        fprintf(fp, "%s~\n", pRoomIndex->name);
        fprintf(fp, "%s~\n", fix_string(pRoomIndex->description));
        fprintf(fp, "%d ", pRoomIndex->room_flags);
        fprintf(fp, "%d\n", pRoomIndex->sector_type);

        QUICKITER(pRoomIndex->extra_descr);
        while ((pEd = (EXTRA_DESCR_DATA *) NextInList(QuickIter)) != NULL)
        {
          fprintf(fp, "E\n%s~\n%s~\n%s~\n%s~\n%d %d %d\n", pEd->keyword, fix_string(pEd->description), pEd->buffer1, pEd->buffer2, pEd->type, pEd->vnum, pEd->action);
        }

        QUICKITER(pRoomIndex->roomtext);
        while ((prt = (ROOMTEXT_DATA *) NextInList(QuickIter)) != NULL)
        {
          fprintf(fp, "T\n%s~\n%s~\n%s~\n%s~\n%d %d %d\n", prt->input, prt->output, prt->choutput, prt->name, prt->type, prt->power, prt->mob);

        }

        for (door = 0; door < MAX_DIR; door++)
        {
          if ((pExit = pRoomIndex->exit[door]))
          {
            fprintf(fp, "D%d\n", door);
            fprintf(fp, "%s~\n", fix_string(pExit->description));
            fprintf(fp, "%s~\n", pExit->keyword);
            fprintf(fp, "%d %d %d\n", pExit->rs_flags, pExit->key, pExit->to_room ? pExit->to_room->vnum : 0);
          }
        }
        fprintf(fp, "S\n");
      }
    }
  }
  fprintf(fp, "#0\n\n\n\n");
}

/* OLC 1.1b */
/*****************************************************************************
 Name:		save_specials
 Purpose:	Save #SPECIALS section of area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_specials(FILE * fp, AREA_DATA * pArea)
{
  int vnum;
  MOB_INDEX_DATA *pMobIndex;

  fprintf(fp, "#SPECIALS\n");

  for (vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++)
  {
    if ((pMobIndex = get_mob_index(vnum)))
    {
      if (pMobIndex->area == pArea && pMobIndex->spec_fun)
      {
        fprintf(fp, "M %d %s\n", pMobIndex->vnum, spec_string(pMobIndex->spec_fun));
      }
      if (pMobIndex->area == pArea && pMobIndex->quest_fun)
      {
        fprintf(fp, "Q %d %s\n", pMobIndex->vnum, quest_string(pMobIndex->quest_fun));
      }
      if (pMobIndex->area == pArea && pMobIndex->death_fun)
      {
        fprintf(fp, "D %d %s\n", pMobIndex->vnum, death_string(pMobIndex->death_fun));
      }
      if (pMobIndex->area == pArea && pMobIndex->shop_fun)
      {
        fprintf(fp, "Z %d %s\n", pMobIndex->vnum, shop_string(pMobIndex->shop_fun));
      }

    }
  }

  fprintf(fp, "S\n\n\n\n");
}

/* OLC 1.1b */
/*****************************************************************************
 Name:		save_resets
 Purpose:	Saves the #RESETS section of an area file.
                New format thanks to Rac.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_resets(FILE * fp, AREA_DATA * pArea)
{
  RESET_DATA *pReset;
  MOB_INDEX_DATA *pLastMob = NULL;
  OBJ_INDEX_DATA *pLastObj;
  ROOM_INDEX_DATA *pRoomIndex;
  char buf[MAX_STRING_LENGTH];
  int vnum;

  fprintf(fp, "#RESETS\n");

  for (vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++)
  {
    if ((pRoomIndex = get_room_index(vnum)))
    {
      if (pRoomIndex->area == pArea)
      {
        QUICKITER(pRoomIndex->resets);
        while ((pReset = (RESET_DATA *) NextInList(QuickIter)) != NULL)
        {
          switch (pReset->command)
          {
            default:
              bug("Save_resets: bad command %c.", pReset->command);
              break;

            case 'M':
              pLastMob = get_mob_index(pReset->arg1);
              fprintf(fp, "M 0 %d %d %d\n", pReset->arg1, pReset->arg2, pReset->arg3);
              break;

            case 'O':
              pLastObj = get_obj_index(pReset->arg1);
              fprintf(fp, "O 0 %d 0 %d\n", pReset->arg1, pReset->arg3);
              break;

            case 'P':
              pLastObj = get_obj_index(pReset->arg1);
              fprintf(fp, "P 0 %d 0 %d\n", pReset->arg1, pReset->arg3);
              break;

            case 'G':
              fprintf(fp, "G 0 %d 0\n", pReset->arg1);
              if (!pLastMob)
              {
                sprintf(buf, "Save_resets: !NO_MOB! in [%s]", pArea->filename);
                bug(buf, 0);
              }
              break;

            case 'E':
              fprintf(fp, "E 0 %d 0 %d\n", pReset->arg1, pReset->arg3);
              if (!pLastMob)
              {
                sprintf(buf, "Save_resets: !NO_MOB! in [%s]", pArea->filename);
                bug(buf, 0);
              }
              break;

            case 'D':
              break;

            case 'R':
              fprintf(fp, "R 0 %d %d\n", pReset->arg1, pReset->arg2);
              break;
          }                     /* End switch */
        }                       /* End for pReset */
      }                         /* End if correct area */
    }                           /* End if pRoomIndex */
  }                             /* End for vnum */
  fprintf(fp, "S\n\n\n\n");
}

void save_help(HELP_DATA *pHelp)
{
  char name[MAX_STRING_LENGTH];
  char strsave[MAX_STRING_LENGTH];
  FILE *fp;
  char *ptr = pHelp->name;
  bool capital = TRUE;
  int i = 0;

  while (*ptr != '\0')
  {
    switch(*ptr)
    {
      default:
        if (capital)
          name[i++] = UPPER(*ptr);
        else
          name[i++] = LOWER(*ptr);
        capital = FALSE;
        break;
      case ' ':
        capital = TRUE;
        break;
    }

     ptr++;
  }
  name[i] = '\0';

  sprintf(strsave, "../helps/%s.hlp", name);
  if ((fp = fopen(strsave, "w")) == NULL)
  {
    sprintf(strsave, "save_help: %s cannot be saved.", name);
    bug(strsave, 0);
    return;
  }

  fprintf(fp, "%s\n", pHelp->name);
  fprintf(fp, "%s\n", pHelp->keyword);
  fprintf(fp, "%d\n", pHelp->level);
  fprintf(fp, "%s", fix_string(pHelp->text));

  fclose(fp);
}

void save_helps()
{
  HELP_DATA *pHelp;
  ITERATOR *pIter;

  pIter = AllocIterator(help_list);
  while ((pHelp = (HELP_DATA *) NextInList(pIter)) != NULL)
    save_help(pHelp);
}

/*****************************************************************************
 Name:		save_area
 Purpose:	Save an area, note that this format is new.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area(AREA_DATA * pArea)
{
  ROOM_INDEX_DATA *pRoomIndex;
  FILE *fp;

  if (!(fp = fopen(pArea->filename, "w")))
  {
    bug("Open_area: fopen", 0);
    perror(pArea->filename);
  }

  fprintf(fp, "#AREADATA\n");
  fprintf(fp, "Name        %s~\n", pArea->name);
  fprintf(fp, "Builders    %s~\n", fix_string(pArea->builders));
  fprintf(fp, "Music       %s~\n", pArea->music);
  fprintf(fp, "VNUMs       %d %d\n", pArea->lvnum, pArea->uvnum);

  if (pArea->cvnum < pArea->lvnum || pArea->cvnum > pArea->uvnum ||
     (pRoomIndex = get_room_index(pArea->cvnum)) == NULL)
  {
    char buf[MAX_INPUT_LENGTH];

    sprintf(buf, "area '%s' has a bad cvnum at %d.", pArea->name, pArea->cvnum);
    bug(buf, 0);
  }

  fprintf(fp, "Cvnum       %d\n", pArea->cvnum);
  fprintf(fp, "Security    %d\n", pArea->security);
  fprintf(fp, "Areabits    %d\n", pArea->areabits);
  fprintf(fp, "End\n\n\n\n");

  save_mobiles(fp, pArea);
  save_objects(fp, pArea);
  save_rooms(fp, pArea);
  save_specials(fp, pArea);
  save_resets(fp, pArea);

  fprintf(fp, "#$\n");

  fclose(fp);
  return;
}

/* OLC 1.1b */
/*****************************************************************************
 Name:		do_asave
 Purpose:	Entry point for saving area data.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_asave(CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  ITERATOR *pIter;
  AREA_DATA *pArea;
  int value;

  if (!ch) /* Do an autosave */
  {
    save_area_list();

    pIter = AllocIterator(area_list);
    while ((pArea = (AREA_DATA *) NextInList(pIter)) != NULL)
    {
      REMOVE_BIT(pArea->area_flags, AREA_CHANGED | AREA_ADDED);
      save_area(pArea);
    }
    return;
  }

  if (IS_NPC(ch))
    return;

  if (ch->pcdata->security < 2)
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  argument = one_argument(argument, arg1);
  if (arg1[0] == '\0')
  {
    send_to_char("Syntax:\n\r", ch);
    send_to_char("  asave <vnum>    - saves a particular area\n\r", ch);
    send_to_char("  asave list      - saves the area.lst file\n\r", ch);
    send_to_char("  asave helps     - saves the help file\n\r", ch);
    send_to_char("  asave area      - saves the area being edited\n\r", ch);
    send_to_char("  asave changed   - saves all changed zones\n\r", ch);
    send_to_char("  asave world     - saves the world! (db dump)\n\r", ch);
    send_to_char("  asave ^ verbose - saves in verbose mode\n\r", ch);
    send_to_char("\n\r", ch);
    return;
  }

  /* Snarf the value (which need not be numeric). */
  value = atoi(arg1);

  /* Save the area of given vnum. */
  /* ---------------------------- */

  if (!(pArea = get_area_data(value)) && is_number(arg1))
  {
    send_to_char("That area does not exist.\n\r", ch);
    return;
  }

  if (is_number(arg1))
  {
    if (!IS_BUILDER(ch, pArea))
    {
      send_to_char("You are not a builder for this area.\n\r", ch);
      return;
    }

    save_area_list();
    save_area(pArea);

    send_to_char("Area saved.\n\r", ch);
    return;
  }

  /* Save the world, only authorized areas. */
  /* -------------------------------------- */

  if (!str_cmp("world", arg1))
  {
    save_area_list();

    pIter = AllocIterator(area_list);
    while ((pArea = (AREA_DATA *) NextInList(pIter)) != NULL)
    {
      /* Builder must be assigned this area. */
      if (!IS_BUILDER(ch, pArea))
        continue;

      save_area(pArea);
    }

    send_to_char("You saved the world.\n\r", ch);
    return;
  }

  if (!str_cmp(arg1, "helps"))
  {
    if (ch->level > 6)
    {
      save_helps();
      send_to_char(" Helps saved.\n\r", ch);
    }
    return;
  }

  /* Save changed areas, only authorized areas. */
  /* ------------------------------------------ */

  if (!str_cmp("changed", arg1))
  {
    char buf[MAX_INPUT_LENGTH];

    save_area_list();

    send_to_char("Saved zones:\n\r", ch);
    sprintf(buf, "None.\n\r");

    pIter = AllocIterator(area_list);
    while ((pArea = (AREA_DATA *) NextInList(pIter)) != NULL)
    {
      /* Builder must be assigned this area. */
      if (!IS_BUILDER(ch, pArea))
        continue;

      /* Save changed areas. */
      if (IS_SET(pArea->area_flags, AREA_CHANGED) || IS_SET(pArea->area_flags, AREA_ADDED))
      {
        save_area(pArea);
        sprintf(buf, "%24s - '%s'\n\r", pArea->name, pArea->filename);
        send_to_char(buf, ch);
      }
    }
    if (!str_cmp(buf, "None.\n\r"))
      send_to_char(buf, ch);
    return;
  }

  /* Save the area.lst file. */
  /* ----------------------- */
  if (!str_cmp(arg1, "list"))
  {
    save_area_list();
    return;
  }

  /* Save area being edited, if authorized. */
  /* -------------------------------------- */
  if (!str_cmp(arg1, "area"))
  {
    /* Find the area to save. */
    switch (ch->desc->editor)
    {
      case ED_AREA:
        pArea = (AREA_DATA *) ch->desc->pEdit;
        break;
      case ED_ROOM:
        pArea = ch->in_room->area;
        break;
      case ED_OBJECT:
        pArea = ((OBJ_INDEX_DATA *) ch->desc->pEdit)->area;
        break;
      case ED_MOBILE:
        pArea = ((MOB_INDEX_DATA *) ch->desc->pEdit)->area;
        break;
      default:
        pArea = ch->in_room->area;
        break;
    }

    if (!IS_BUILDER(ch, pArea))
    {
      send_to_char("You are not a builder for this area.\n\r", ch);
      return;
    }

    save_area_list();
    save_area(pArea);
    REMOVE_BIT(pArea->area_flags, AREA_CHANGED | AREA_ADDED);
    send_to_char("Area saved.\n\r", ch);
    return;
  }

  /* Show correct syntax. */
  /* -------------------- */
  do_asave(ch, "");
}
