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
 *  File: mem.c                                                            *
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


#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dystopia.h"


STACK         		*	area_free = NULL;
STACK         		*	exit_free = NULL;
STACK			*	obj_index_free = NULL;
STACK                   *	mob_index_free = NULL;
STACK          		*	reset_free = NULL;


/*****************************************************************************
 Name:		new_reset_data
 Purpose:	Creates and clears a reset structure.
 ****************************************************************************/
RESET_DATA *new_reset_data( void )
{
    RESET_DATA *pReset;

    if ((pReset = (RESET_DATA *) PopStack(reset_free)) == NULL)
    {
	pReset		= malloc(sizeof(*pReset));
	top_reset++;
    }

    pReset->repop       = TRUE;
    pReset->command	= 'X';
    pReset->arg1	= 0;
    pReset->arg2	= 0;
    pReset->arg3	= 0;

    return pReset;
}



/*****************************************************************************
 Name:		free_reset_data
 Purpose:	Clears and deletes a reset structure.
 ****************************************************************************/
void free_reset_data( RESET_DATA *pReset )
{
  PushStack(pReset, reset_free);
}



/*****************************************************************************
 Name:		new_area
 Purpose:	Creates and clears a new area structure.
 ****************************************************************************/
AREA_DATA *new_area( void )
{
    AREA_DATA *pArea;
    char buf[MAX_INPUT_LENGTH];

    if ((pArea = (AREA_DATA *) PopStack(area_free)) == NULL)
    {
	pArea		= malloc(sizeof(*pArea));
	top_area++;
    }

    pArea->name		= str_dup( "New area" );
    pArea->music        = str_dup( "default.mid" );
    pArea->area_flags	= AREA_ADDED;
    pArea->security	= 1;
    pArea->builders	= str_dup( "None" );
    pArea->lvnum	= 0;
    pArea->uvnum	= 0;
    pArea->affects      = AllocList();
    pArea->events       = AllocList();
    pArea->vnum		= top_area-1;		/* OLC 1.1b */
    sprintf( buf, "area%d.are", pArea->vnum );
    pArea->filename	= str_dup( buf );

    return pArea;
}

EXIT_DATA *new_exit( void )
{
    EXIT_DATA *pExit;
 
    if ((pExit = (EXIT_DATA *) PopStack(exit_free)) == NULL)
    {
        pExit           =   malloc(sizeof(*pExit));
        top_exit++;
    }

    pExit->to_room      =   NULL;
    pExit->vnum         =   0;
    pExit->exit_info    =   0;
    pExit->key          =   0;
    pExit->keyword      =   &str_empty[0];;
    pExit->description  =   &str_empty[0];;
    pExit->rs_flags     =   0;

    return pExit;
}



void free_exit( EXIT_DATA *pExit )
{
    free_string( pExit->keyword );
    free_string( pExit->description );

    PushStack(pExit, exit_free);
}



EXTRA_DESCR_DATA *new_extra_descr( void )
{
    EXTRA_DESCR_DATA *pExtra;

    if ((pExtra = (EXTRA_DESCR_DATA *) PopStack(extra_descr_free)) == NULL)
    {
        pExtra              =   calloc(1,  sizeof(*pExtra));
        top_ed++;
    }

    pExtra->keyword         =   NULL;
    pExtra->description     =   NULL;
    pExtra->buffer1         =   NULL;
    pExtra->buffer2         =   NULL;
    pExtra->type            =   0;
    pExtra->vnum            =   0;
    pExtra->action          =   0;

    return pExtra;
}



void free_extra_descr( EXTRA_DESCR_DATA *pExtra )
{
    free_string( pExtra->keyword );
    free_string( pExtra->description );
    free_string( pExtra->buffer1);
    free_string( pExtra->buffer2);
    pExtra->type = ED_TYPE_NONE;

    PushStack(pExtra, extra_descr_free);
}



ROOM_INDEX_DATA *new_room_index( void )
{
    ROOM_INDEX_DATA *pRoom;
    int door;

    pRoom                   =   malloc(sizeof(*pRoom));
    pRoom->people           =   AllocList();
    pRoom->contents         =   AllocList();
    pRoom->extra_descr      =   AllocList();
    pRoom->events           =   AllocList();
    pRoom->roomtext         =   AllocList();
    pRoom->resets           =   AllocList();
    pRoom->area             =   NULL;

    for ( door=0; door < MAX_DIR; door++ )
        pRoom->exit[door]   =   NULL;

    pRoom->name             =   &str_empty[0];
    pRoom->description      =   &str_empty[0];
    pRoom->vnum             =   0;
    pRoom->room_flags       =   0;
    pRoom->light            =   0;
    pRoom->sector_type      =   0;

    return pRoom;
}

AFFECT_DATA *new_affect( void )
{
  AFFECT_DATA *pAf;

  if ((pAf = (AFFECT_DATA *) PopStack(affect_free)) == NULL)
    pAf = malloc(sizeof(*pAf));

  pAf->location   =   0;
  pAf->modifier   =   0;
  pAf->type       =   0;
  pAf->duration   =   0;
  pAf->bitvector  =   0;

  return pAf;
}

OBJ_INDEX_DATA *new_obj_index( void )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ((pObj = (OBJ_INDEX_DATA *) PopStack(obj_index_free)) == NULL)
    {
        pObj           =   calloc(1,  sizeof(*pObj));
        top_obj_index++;
    }

    pObj->extra_descr   =   AllocList();
    pObj->affected      =   AllocList();
    pObj->area          =   NULL;
    pObj->name          =   str_dup( "no name" );
    pObj->short_descr   =   str_dup( "(no short description)" );
    pObj->description   =   str_dup( "(no description)" );
    pObj->vnum          =   0;
    pObj->item_type     =   ITEM_TRASH;
    pObj->extra_flags   =   0;
    pObj->wear_flags    =   0;
    pObj->count         =   0;
    pObj->weight        =   1;
    pObj->cost          =   0;
    for ( value=0; value<4; value++ )
        pObj->value[value]  =   0;

    return pObj;
}



MOB_INDEX_DATA *new_mob_index( void )
{
    MOB_INDEX_DATA *pMob;

    if ((pMob = (MOB_INDEX_DATA *) PopStack(mob_index_free)) == NULL)
    {
        pMob           =   calloc(1,  sizeof(*pMob));
        top_mob_index++;
    }

    pMob->spec_fun      =   NULL;
    pMob->quest_fun     =   NULL;
    pMob->death_fun     =   NULL;
    pMob->shop_fun      =   NULL;
    pMob->area          =   NULL;
    pMob->player_name   =   str_dup( "no name" );
    pMob->short_descr   =   str_dup( "(no short description)" );
    pMob->long_descr    =   str_dup( "(no long description)\n\r" );
    pMob->description   =   &str_empty[0];
    pMob->vnum          =   0;
    pMob->count         =   0;
    pMob->killed        =   0;
    pMob->sex           =   0;
    pMob->level         =   0;
    pMob->act           =   ACT_IS_NPC;
    pMob->affected_by   =   0;
    pMob->alignment     =   0;
    pMob->toughness	=   0;
    pMob->extra_attack	=   0;
    pMob->dam_modifier	=   0;

    return pMob;
}
