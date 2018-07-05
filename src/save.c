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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "dystopia.h"

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST        100
static OBJ_DATA *rgObjNest[MAX_NEST];

/*
 * Local functions.
 */
void fwrite_char           ( CHAR_DATA *ch, FILE *fp );
void fwrite_obj            ( OBJ_DATA *obj, FILE *fp, int iNest );
void fread_char            ( CHAR_DATA *ch, FILE *fp );
OBJ_DATA *fread_obj        ( CHAR_DATA *ch, FILE *fp );
void save_char_obj_finger  ( CHAR_DATA *ch );


/* the two auction load/save funtions follows here */
void save_auctions()
{
  FILE *fp;

  if ((fp = fopen("../txt/auctions.txt", "w")) != NULL)
  {
    AUCTION_DATA *auction;
    ITERATOR *pIter;

    fprintf(fp, "%d\n", muddata.questpool);

    pIter = AllocIterator(auction_list);
    while ((auction = (AUCTION_DATA *) NextInList(pIter)) != NULL)
    {
      fprintf(fp, "#AUCTION\n");
      fprintf(fp, "%s~\n", auction->seller_name);
      fprintf(fp, "%s~\n", auction->seller_account);
      fprintf(fp, "%s~\n", auction->bidder_name);
      fprintf(fp, "%s~\n", auction->bidder_account);
      fprintf(fp, "%ld\n", auction->expire);
      fprintf(fp, "%d\n",  auction->bid);
      fprintf(fp, "%d\n",  auction->bidout);
      fwrite_obj(auction->obj, fp, 0);
    }
    fprintf(fp, "#END\n");

    fclose(fp);
  }
  else
  {
    bug("save_auctions: failed to open file auctions.txt.", 0);
  }
}

void load_auctions()
{
  AUCTION_DATA *auction = NULL;
  FILE *fp;

  if ((fp = fopen("../txt/auctions.txt", "r")) != NULL)
  {
    int iNest;

    for (iNest = 0; iNest < MAX_NEST; iNest++)
      rgObjNest[iNest] = NULL;

    muddata.questpool = fread_number(fp);

    for (;;)
    {
      char letter;
      char *word;

      letter = fread_letter(fp);
      if (letter != '#')
      {
        bug("load_auctions: # not found.", 0);
        break;
      }

      word = fread_word(fp);
      if (!str_cmp(word, "AUCTION"))
      {
	auction = alloc_auction();
	auction->seller_name = fread_string(fp);
	auction->seller_account = fread_string(fp);
	auction->bidder_name = fread_string(fp);
	auction->bidder_account = fread_string(fp);
	auction->expire = fread_number(fp);
	auction->bid = fread_number(fp);
        auction->bidout = fread_number(fp);
      }
      else if (!str_cmp(word, "OBJECT"))
      {
	OBJ_DATA *obj;

	if (auction == NULL)
	{
	  bug("load_auctions: loading object to NULL auction.", 0);
	  break;
	}

	if (auction->obj != NULL)
	{
	  bug("load_auctions: loading two objects to same auction.", 0);
	  break;
	}

        if ((obj = fread_obj(NULL, fp)) == NULL)
	{
	  bug("load_auctions: loading NULL obj.", 0);
	  break;
	}

        auction->obj = obj;
        AttachToList(auction, auction_list);
      }
      else if (!str_cmp(word, "END"))
      {
        break;
      }
      else
      {
        bug("load_auctions: bad section.", 0);
        break;
      }
    }

    fclose(fp);
  }
}

/*
 * Save a character and inventory.
 */
void save_char_obj(CHAR_DATA *ch)
{
  ITERATOR *pIter;
  KINGDOM_DATA *kingdom;
  MEMBER_DATA *member;
  OBJ_DATA *obj;
  char strsave[MAX_STRING_LENGTH];
  FILE *fp;

  if (IS_NPC(ch))
    return;

  /* A new player saved for the first time */
  if (ch->pcdata->playerid == 0)
    ch->pcdata->playerid = get_next_playerid();

  /* if the player is actually connected, we only save if PLAYING */
  if (ch->desc != NULL && (ch->desc->connected != CON_PLAYING ||
     (ch->desc->connected >= CON_NOTE_TO && ch->desc->connected <= CON_NOTE_FINISH)))
    return;

  if ((kingdom = get_kingdom(ch)) != NULL)
  {
    pIter = AllocIterator(kingdom->members);
    while ((member = (MEMBER_DATA *) NextInList(pIter)) != NULL)
    {
      if (!str_cmp(member->name, ch->name))
      {
        member->might = getMight(ch);
        break;
      }
    }

    save_kingdom(kingdom);
  }

  /* update account information */
  if (ch->desc)
  {
    account_update(ch->desc->account, ch);

    /* check and update leaderboard/top10 */
    if (ch->level < 7 && ch->desc->account->level == PLAYER_ACCOUNT)
    {
      check_leaderboard(ch);
      check_top10(ch);
    }
  }

  sprintf(strsave, "%s%s/%s.pfile",
    PLAYER_DIR, ch->pcdata->account, capitalize(ch->name));

  /* Here we do a backup of the playerfile if it already
   * exists. There is a 80% chance that no backup
   * will be made - to ensure that the pfilebackup is a bit old.
   */
  if (number_percent() > 80 && (fp = fopen(strsave, "r")) != NULL)
  {
    char tBuf[MAX_STRING_LENGTH];

    fclose(fp);

    sprintf(tBuf, "%s%s/%s.bck",
      PLAYER_DIR, ch->pcdata->account, capitalize(ch->name));

    /* delete old backup file */
    if ((fp = fopen(tBuf, "r")) != NULL)
    {
      fclose(fp);
      unlink(tBuf);
    }

    rename(strsave, tBuf);
  }

  if ((fp = fopen(strsave, "w")) == NULL)
  {
    bug("Save_char_obj: fopen", 0);
    perror(strsave);
    return;
  }
  else
  {
    fwrite_char(ch, fp);

    pIter = AllocIterator(ch->carrying);
    while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
      fwrite_obj(obj, fp, 0);
    fprintf(fp, "#END\n");
  }
  fclose(fp);
  save_char_obj_finger(ch);
}

void save_char_obj_finger(CHAR_DATA *ch)
{
  char strsave[MAX_STRING_LENGTH];
  FILE *fp;

  if (IS_NPC(ch) || ch->level < 2)
    return;

  sprintf(strsave, "%swhois/%s.whois", PLAYER_DIR, capitalize(ch->name));
  if ((fp = fopen(strsave, "w")) == NULL)
  {
    bug("Save_char_obj: fopen", 0);
    perror(strsave);
  }
  else
  {
    fprintf(fp, "#WHOIS\n");
    fprintf(fp, "Name         %s~\n", ch->name);
    fprintf(fp, "Title        %s~\n", ch->pcdata->title);
    fprintf(fp, "Lasthost     %s~\n", ch->lasthost);
    fprintf(fp, "Account      %s~\n", ch->pcdata->account);
    fprintf(fp, "Lasttime     %s~\n", ch->lasttime);
    fprintf(fp, "Extra        %d\n",  ch->extra);
    fprintf(fp, "Sex          %d\n",  ch->sex);
    fprintf(fp, "Conception   %s~\n", ch->pcdata->conception);
    fprintf(fp, "Createtime   %s~\n", ch->createtime);
    fprintf(fp, "Level        %d\n",  ch->level);
    fprintf(fp, "Played       %d\n",  ch->played + (int)(current_time - ch->logon));
    fprintf(fp, "Marriage     %s~\n", ch->pcdata->marriage);
    fprintf(fp, "PkPdMkMd     %d %d %d %d\n",  ch->pkill, ch->pdeath, ch->mkill, ch->mdeath);
    fprintf(fp, "Awin         %d\n",  ch->pcdata->awins);
    fprintf(fp, "Alos         %d\n",  ch->pcdata->alosses);
  }
  fprintf(fp, "End\n");

  fclose(fp);
}

CHAR_DATA *load_char_whois(char *name, int *status)
{
  char strsave[MAX_STRING_LENGTH];
  CHAR_DATA *ch;
  FILE *fp;
  char *word;

  /* if char doesn't exist, we return NULL and
   * set the status to 0, which means that we couldn't
   * find the given whois data anywhere.
   */
  if (!char_exists(name))
  {
    *status = 0;
    return NULL;
  }

  /* allocate memory for char_data and pc_data */
  if ((ch = (CHAR_DATA *) PopStack(char_free)) == NULL)
    ch = calloc(1, sizeof(*ch));
  clear_char(ch);

  if ((ch->pcdata = (PC_DATA *) PopStack(pcdata_free)) == NULL)
    ch->pcdata = calloc(1, sizeof(*ch->pcdata));
  clear_pcdata(ch->pcdata);

  /* now open the file and start scanning */
  sprintf(strsave, "%swhois/%s.whois", PLAYER_DIR, capitalize(name));
  if ((fp = fopen(strsave, "r")) == NULL)
  {
    free_char(ch);
    *status = 0;
    return NULL;
  }

  word = fread_word(fp);
  if (!str_cmp(word, "#WHOIS"))
    fread_char(ch, fp);
  else
  {
    fclose(fp);
    free_char(ch);
    *status = -1;
    return NULL;
  }

  fclose(fp);
  return ch;
}

/*
 * Write the char.
 */
void fwrite_char(CHAR_DATA *ch, FILE *fp)
{
  AFFECT_DATA *paf;
  QUEST_DATA *quest;
  ITERATOR *pIter;
  ALIAS_DATA *alias;
  IGNORE_DATA *ignore;
  int sn;

  fprintf(fp, "#PLAYERS\n");

  fprintf(fp, "Name          %s~\n", ch->name);
  fprintf(fp, "PlayerID      %d\n",  ch->pcdata->playerid);
  fprintf(fp, "ShortDescr    %s~\n", ch->short_descr);
  fprintf(fp, "LongDescr     %s~\n", ch->long_descr);
  fprintf(fp, "Description   %s~\n", ch->description);
  fprintf(fp, "Class         %d\n",  ch->class);
  fprintf(fp, "Morph         %s~\n", ch->morph);
  fprintf(fp, "Createtime    %s~\n", ch->createtime);
  fprintf(fp, "Lasttime      %s~\n", ch->lasttime);
  fprintf(fp, "Lasthost      %s~\n", ch->lasthost);
  fprintf(fp, "Logouttime    %ld\n", current_time);
  fprintf(fp, "Prompt        %s~\n", ch->prompt);
  fprintf(fp, "Cprompt       %s~\n", ch->cprompt);
  fprintf(fp, "Decapmessage  %s~\n", ch->pcdata->decapmessage);
  fprintf(fp, "Loginmessage  %s~\n", ch->pcdata->loginmessage);
  fprintf(fp, "Logoutmessage %s~\n", ch->pcdata->logoutmessage);
  fprintf(fp, "Avatarmessage %s~\n", ch->pcdata->avatarmessage);
  fprintf(fp, "Tiemessage    %s~\n", ch->pcdata->tiemessage);
  fprintf(fp, "ImmCmd        %s~\n", ch->pcdata->immcmd);
  fprintf(fp, "EnhCombat     %s~\n", ch->pcdata->enh_combat);
  fprintf(fp, "Jflags        %d\n",  ch->pcdata->jflags);
  fprintf(fp, "Status        %d\n",  ch->pcdata->status);
  fprintf(fp, "Sex           %d\n",  ch->sex);
  fprintf(fp, "Meanparadox   %d\n",  ch->pcdata->mean_paradox_counter);
  fprintf(fp, "Timetick      %d\n",  ch->pcdata->time_tick);
  fprintf(fp, "Revision      %d\n",  ch->pcdata->revision);
  fprintf(fp, "EvolveCount   %d\n",  ch->pcdata->evolveCount);
  fprintf(fp, "Immune        %d\n",  ch->immune);
  fprintf(fp, "Generation    %d\n",  ch->generation);
  fprintf(fp, "Itemaffect    %d\n",  ch->itemaffect);
  fprintf(fp, "Session       %d %d %d %d %d %d %d %d\n",
    ch->pcdata->session->hit, ch->pcdata->session->move, ch->pcdata->session->mana,
    ch->pcdata->session->quests, ch->pcdata->session->gold, ch->pcdata->session->exp,
    ch->pcdata->session->pkills, ch->pcdata->session->mkills);
  fprintf(fp, "Form          %d\n",  ch->form);
  fprintf(fp, "Home1         %d\n",  ch->home[0]);
  fprintf(fp, "Home2         %d\n",  ch->home[1]);
  fprintf(fp, "Home3         %d\n",  ch->home[2]);
  fprintf(fp, "Level         %d\n",  ch->level);
  fprintf(fp, "Trust         %d\n",  ch->trust);
  fprintf(fp, "Security      %d\n",  ch->pcdata->security);       /* OLC */
  fprintf(fp, "Played        %d\n",  ch->played + (int)(current_time - ch->logon));
  fprintf(fp, "Room          %d\n", (ch->in_room == get_room_index(ROOM_VNUM_LIMBO)
                                  && ch->was_in_room != NULL)
                                   ? ch->was_in_room->vnum
                                   : ch->in_room->vnum);
  fprintf(fp, "Xlogon        %ld\n", ch->xlogon);
  fprintf(fp, "PkPdMkMd      %d %d %d %d\n", ch->pkill, ch->pdeath, ch->mkill, ch->mdeath);
  fprintf(fp, "Alos          %d\n", ch->pcdata->alosses);
  fprintf(fp, "Awin          %d\n", ch->pcdata->awins);
  fprintf(fp, "Weapons       %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
    ch->wpn[0], ch->wpn[1], ch->wpn[2], ch->wpn[3], ch->wpn[4], ch->wpn[5],
    ch->wpn[6], ch->wpn[7], ch->wpn[8], ch->wpn[9], ch->wpn[10], ch->wpn[11],
    ch->wpn[12]);
  fprintf(fp, "Spells        %d %d %d %d %d\n",
    ch->spl[0], ch->spl[1], ch->spl[2], ch->spl[3], ch->spl[4]);
  fprintf(fp, "Stances       %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
    ch->stance[0], ch->stance[1], ch->stance[2], ch->stance[3], ch->stance[4],
    ch->stance[5], ch->stance[6], ch->stance[7], ch->stance[8], ch->stance[9],
    ch->stance[10], ch->stance[11], ch->stance[12], ch->stance[13]);
  fprintf(fp, "Locationhp    %d %d %d %d %d %d %d\n",
    ch->loc_hp[0], ch->loc_hp[1], ch->loc_hp[2], ch->loc_hp[3],
    ch->loc_hp[4], ch->loc_hp[5], ch->loc_hp[6]);
  fprintf(fp, "HpManaMove    %d %d %d %d %d %d\n",
    ch->hit, ch->max_hit,
    ch->mana, ch->max_mana,
    ch->move, ch->max_move);
  fprintf(fp, "Briefs        %d %d %d %d %d %d %d %d %d %d\n",
    ch->pcdata->brief[0], ch->pcdata->brief[1], ch->pcdata->brief[2], ch->pcdata->brief[3],
    ch->pcdata->brief[4], ch->pcdata->brief[5], ch->pcdata->brief[6], ch->pcdata->brief[7],
    ch->pcdata->brief[8], ch->pcdata->brief[9]);
  fprintf(fp, "Legend        %d\n", ch->pcdata->legend);
  fprintf(fp, "Exp           %d\n", ch->exp);
  fprintf(fp, "Act           %d\n", ch->act);
  fprintf(fp, "Special       %d\n", ch->special);
  fprintf(fp, "Newbits       %d\n", ch->newbits);
  fprintf(fp, "Extra         %d\n", ch->extra);
  fprintf(fp, "AffectedBy    %d\n", ch->affected_by);
  fprintf(fp, "Position      %d\n", ch->position == POS_FIGHTING
    ? POS_STANDING
    : ch->position);
  fprintf(fp, "Practice      %d\n", ch->practice);
  fprintf(fp, "SavingThrow   %d\n", ch->saving_throw);
  fprintf(fp, "Alignment     %d\n", ch->alignment);
  fprintf(fp, "Hitroll       %d\n", ch->hitroll);
  fprintf(fp, "Damroll       %d\n", ch->damroll);
  fprintf(fp, "Armor         %d\n", ch->armor);
  fprintf(fp, "Deaf          %d\n", ch->deaf);
  fprintf(fp, "Bamfin        %s~\n", ch->pcdata->bamfin);
  fprintf(fp, "Bamfout       %s~\n", ch->pcdata->bamfout);
  fprintf(fp, "Lastdecap1    %s~\n", ch->pcdata->last_decap[0]);
  fprintf(fp, "Lastdecap2    %s~\n", ch->pcdata->last_decap[1]);
  fprintf(fp, "Retaliation1  %s~\n", ch->pcdata->retaliation[0]);
  fprintf(fp, "Retaliation2  %s~\n", ch->pcdata->retaliation[1]);
  fprintf(fp, "Kingdom       %d\n", ch->pcdata->kingdom);
  fprintf(fp, "Questsrun     %d\n", ch->pcdata->questsrun);
  fprintf(fp, "Title         %s~\n", ch->pcdata->title);
  fprintf(fp, "Bounty	     %d\n", ch->pcdata->bounty);
  fprintf(fp, "Conception    %s~\n", ch->pcdata->conception);
  fprintf(fp, "Parents       %s~\n", ch->pcdata->parents);
  fprintf(fp, "Cparents      %s~\n", ch->pcdata->cparents);
  fprintf(fp, "Marriage      %s~\n", ch->pcdata->marriage);
  fprintf(fp, "AttrPerm      %d %d %d %d %d\n",
    ch->pcdata->perm_str, ch->pcdata->perm_int, ch->pcdata->perm_wis,
    ch->pcdata->perm_dex, ch->pcdata->perm_con);
  fprintf(fp, "AttrMod       %d %d %d %d %d\n",
    ch->pcdata->mod_str, ch->pcdata->mod_int, ch->pcdata->mod_wis, 
    ch->pcdata->mod_dex, ch->pcdata->mod_con);
  fprintf(fp, "Stage         %d %d %d\n",
    ch->pcdata->stage[0], ch->pcdata->stage[1], ch->pcdata->stage[2]);
  fprintf(fp, "Genes         %d %d %d %d %d %d %d %d %d %d\n",
          ch->pcdata->genes[0], ch->pcdata->genes[1],
          ch->pcdata->genes[2], ch->pcdata->genes[3],
          ch->pcdata->genes[4], ch->pcdata->genes[5],
          ch->pcdata->genes[6], ch->pcdata->genes[7],
          ch->pcdata->genes[8], ch->pcdata->genes[9]);
  fprintf(fp, "Power        ");
  for (sn = 0; sn < 13; sn++)
  {
    fprintf(fp, "%d ", ch->pcdata->powers[sn]);
  }
  fprintf(fp, "\n");
  fprintf(fp, "Condition    %d %d %d\n",
    ch->pcdata->condition[0], ch->pcdata->condition[1],
    ch->pcdata->condition[2]);

  for (sn = 0; sn < MAX_SKILL; sn++)
  {
    if (skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0)
    {
      fprintf(fp, "Skill        %d '%s'\n",
        ch->pcdata->learned[sn], skill_table[sn].name);
    }
  }

  /* save all aliases */
  pIter = AllocIterator(ch->pcdata->aliases);
  while ((alias = (ALIAS_DATA *) NextInList(pIter)) != NULL)
  {
    fprintf(fp, "Alias           %s~\n", alias->name);
    fprintf(fp, "%s~\n", strip_returns(alias->expand_string));
  }

  /* save all ignores */
  pIter = AllocIterator(ch->pcdata->ignores);
  while ((ignore = (IGNORE_DATA *) NextInList(pIter)) != NULL)
  {
    fprintf(fp, "Ignore         %s~ %s~\n", ignore->player, ignore->account);
  }

  pIter = AllocIterator(ch->pcdata->quests);
  while ((quest = (QUEST_DATA *) NextInList(pIter)) != NULL)
  {
    fprintf(fp, "QuestData %ld %d %d %d %d %d %d\n",
      quest->expire, quest->type, quest->giver, quest->vnums[0],
      quest->vnums[1], quest->vnums[2], quest->vnums[3]);
  }

  /* save all events (that should save) */
  save_player_events(ch, fp);

  pIter = AllocIterator(ch->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    /* Thanks Alander */
    if (paf->type < 0 || paf->type >= MAX_SKILL)
      continue;

    fprintf(fp, "AffectData   '%s' %3d %3d %3d %10d\n",
      skill_table[paf->type].name, paf->duration, paf->modifier,
      paf->location, paf->bitvector);
  }

  fprintf(fp, "End\n\n");
}

/*
 * Write an object and its contents.
 */
void fwrite_obj(OBJ_DATA *obj, FILE *fp, int iNest)
{
  EXTRA_DESCR_DATA *ed;
  AFFECT_DATA *paf;
  ITERATOR *pIter, *pIter2;
  OBJ_DATA *obj_content;
  bool first_aff = TRUE;

  /* do not save decaying objects */
  if (event_isset_object(obj, EVENT_OBJECT_DECAY))
    return;

  /* do not save keys */
  if (obj->item_type == ITEM_KEY)
    return;

  /* do not save items from unfinished areas */
  if (IS_SET(obj->extra_flags, ITEM_OLC))
    return;

  fprintf(fp, "#OBJECT\n");
  fprintf(fp, "Nest         %d\n",  iNest);
  fprintf(fp, "Name         %s~\n", obj->name);
  fprintf(fp, "OwnerID      %d\n",  obj->ownerid);
  fprintf(fp, "ShortDescr   %s~\n", obj->short_descr);
  fprintf(fp, "Description  %s~\n", obj->description);
  fprintf(fp, "Questowner   %s~\n", obj->questowner);
  fprintf(fp, "Vnum         %d\n",  obj->pIndexData->vnum);
  fprintf(fp, "ExtraFlags   %d\n",  obj->extra_flags);
  fprintf(fp, "WearFlags    %d\n",  obj->wear_flags);
  fprintf(fp, "WearLoc      %d\n",  obj->wear_loc);
  fprintf(fp, "ItemType     %d\n",  obj->item_type);
  fprintf(fp, "Weight       %d\n",  obj->weight);
  fprintf(fp, "Condition    %d\n",  obj->condition);
  fprintf(fp, "Toughness    %d\n",  obj->toughness);
  fprintf(fp, "SentientP    %d\n",  obj->sentient_points);
  fprintf(fp, "Spellflags   %d\n",  obj->spellflags);
  fprintf(fp, "Resistance   %d\n",  obj->resistance);
  fprintf(fp, "Quest        %d\n",  obj->quest);
  fprintf(fp, "Level        %d\n",  obj->level);
  fprintf(fp, "Cost         %d\n",  obj->cost);
  fprintf(fp, "Values       %d %d %d %d\n",
    obj->value[0], obj->value[1], obj->value[2], obj->value[3]);

  switch (obj->item_type)
  {
    case ITEM_POTION:
      if (obj->value[1] > 0)
      {
        fprintf(fp, "Spell 1      '%s'\n", skill_table[obj->value[1]].name);
      }
      if (obj->value[2] > 0)
      {
        fprintf(fp, "Spell 2      '%s'\n", skill_table[obj->value[2]].name);
      }
      if (obj->value[3] > 0)
      {
        fprintf(fp, "Spell 3      '%s'\n", skill_table[obj->value[3]].name);
      }
      break;
    case ITEM_SCROLL:
      if (obj->value[1] > 0)
      {
        fprintf(fp, "Spell 1      '%s'\n", skill_table[obj->value[1]].name);
      }
      if (obj->value[2] > 0)
      {
        fprintf(fp, "Spell 2      '%s'\n", skill_table[obj->value[2]].name);
      }
      if (obj->value[3] > 0)
      {
        fprintf(fp, "Spell 3      '%s'\n", skill_table[obj->value[3]].name);
      }
      break;
    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
      if (obj->value[3] > 0)
      {
        fprintf(fp, "Spell 3      '%s'\n", skill_table[obj->value[3]].name);
      }
      break;
  }

  /* save temporary affects */
  pIter = AllocIterator(obj->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    if (paf->duration < 0) continue;

    fprintf(fp, "AffectData   %d %d %d %d %d\n",
      paf->type, paf->duration, paf->modifier, paf->location, paf->bitvector);
  }

  /* save permanent affects */
  pIter = AllocIterator(obj->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    AFFECT_DATA *af;
    int modi = paf->modifier;

    /* we don't save temp affects */
    if (paf->duration >= 0) continue;

    if (paf->location != APPLY_NONE && !first_aff)
    {
      pIter2 = AllocIterator(obj->affected);
      while ((af = (AFFECT_DATA *) NextInList(pIter2)) != NULL)
      {
        if (af == paf) break;
        if (af->duration >= 0) continue;

        /* if we have a previous affect with same affect location
         * we have allready collected all these affects in one
         */
        if (af->location == paf->location && af->bitvector == paf->bitvector)
        {
          af = NULL;
          break;
        }
      }
    }
    else
    {
      af = paf;
    }

    first_aff = FALSE;

    if (af != NULL)
    {
      if (af->location != APPLY_NONE)
      {
        bool match = FALSE;
        AFFECT_DATA *af2 = af;

        pIter2 = AllocIterator(obj->affected);
        while ((af = (AFFECT_DATA *) NextInList(pIter2)) != NULL)
        {
          if (!match)
          {
            if (af == af2) match = TRUE;
            continue;
          }

          if (af->duration >= 0) continue;

          /* add up the modifier */
          if (af->location == paf->location && af->bitvector == paf->bitvector)
            modi += af->modifier;
        }
      }

      fprintf(fp, "AffectData   %d %d %d %d %d\n",
        paf->type, paf->duration, modi, paf->location, paf->bitvector);
    }
  }

  pIter = AllocIterator(obj->extra_descr);
  while ((ed = (EXTRA_DESCR_DATA *) NextInList(pIter)) != NULL)
  {
    fprintf(fp, "ExtraDescr   %s~ %s~\n", ed->keyword, ed->description);
  }
  fprintf(fp, "End\n\n");

  pIter = AllocIterator(obj->contains);
  while ((obj_content = (OBJ_DATA *) NextInList(pIter)) != NULL)
    fwrite_obj(obj_content, fp, iNest + 1);
}

/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj(DESCRIPTOR_DATA *d, char *name)
{
  char strsave[MAX_STRING_LENGTH];
  char *strtime;
  CHAR_DATA *ch;
  FILE *fp;
  bool found;

  if (d->account == NULL)
  {
    bug("Load_char_obj: Descriptor has no account.", 0);
    return FALSE;
  }

  if ((ch = (CHAR_DATA *) PopStack(char_free)) == NULL)
    ch = calloc(1, sizeof(*ch));

  clear_char(ch);

  /* set create time just in case this is a new char */
  strtime = ctime(&current_time);
  strtime[strlen(strtime) - 1] = '\0';
  ch->createtime = str_dup(strtime);

  if ((ch->pcdata = (PC_DATA *) PopStack(pcdata_free)) == NULL)
    ch->pcdata = calloc(1, sizeof(*ch->pcdata));

  clear_pcdata(ch->pcdata);

  d->character        = ch;
  ch->desc            = d;
  ch->name            = str_dup(name);
  ch->pcdata->account = str_dup(d->account->owner);

  found = FALSE;
  sprintf(strsave, "%s%s/%s.pfile",
    PLAYER_DIR, d->account->owner, capitalize(name));
  if ((fp = fopen(strsave, "r")) != NULL)
  {
    int iNest;

    for (iNest = 0; iNest < MAX_NEST; iNest++)
      rgObjNest[iNest] = NULL;

    found = TRUE;
    for (;;)
    {
      char letter;
      char *word;

      letter = fread_letter(fp);
      if (letter == '*')
      {
        fread_to_eol(fp);
        continue;
      }
      if (letter != '#')
      {
        bug("Load_char_obj: # not found.", 0);
        break;
      }
      word = fread_word(fp);
      if (!str_cmp(word, "PLAYERS"))
        fread_char(ch, fp);
      else if (!str_cmp(word, "OBJECT"))
        fread_obj(ch, fp);
      else if (!str_cmp(word, "END"))
        break;
      else
      {
        bug("Load_char_obj: bad section.", 0);
        break;
      }
    }
    fclose(fp);
  }
  return found;
}

/*
 * Read in a char.
 */
#if defined(KEY)
#undef KEY
#endif

#define KEY(literal, field, value) \
  if (!str_cmp(word, literal))     \
  {                                \
    field = value;                 \
    fMatch = TRUE;                 \
    break;                         \
  }

/* There is a slight overloading of this function. The function
 * will (rightly so) return NULL when used for loading objects
 * from a playerfile (ie. when ch != NULL), but when ch is NULL,
 * it will return a pointer to the top-level object (containers
 * should thus be possible, though not tested). This was done
 * to avoid duplicating code for the auction systems loading and
 * saving of objects.
 */
OBJ_DATA *fread_obj(CHAR_DATA *ch, FILE *fp)
{
  static OBJ_DATA obj_zero;
  OBJ_DATA *obj;
  char *word;
  int iNest;
  bool fMatch;
  bool fNest;
  bool fVnum;

  if ((obj = (OBJ_DATA *) PopStack(obj_free)) == NULL)
  {
    obj = calloc(1, sizeof(*obj));
  }

  *obj = obj_zero;
  obj->name          = str_dup("");
  obj->short_descr   = str_dup("");
  obj->description   = str_dup("");
  obj->questowner    = str_dup("");
  obj->condition     = 100;
  obj->toughness     = 0;
  obj->resistance    = 100;
  obj->quest         = 0;
  obj->ownerid       = 0;

  obj->events = AllocList();
  obj->contains = AllocList();
  obj->affected = AllocList();
  obj->extra_descr = AllocList();

  fNest = FALSE;
  fVnum = TRUE;
  iNest = 0;

  for (;;)
  {
    word = feof(fp) ? "End" : fread_word(fp);
    fMatch = FALSE;

    switch (UPPER(word[0]))
    {
      case '*':
        fMatch = TRUE;
        fread_to_eol(fp);
        break;

      case 'A':
        if (!str_cmp(word, "AffectData"))
        {
          AFFECT_DATA *paf;

          if ((paf = (AFFECT_DATA *) PopStack(affect_free)) == NULL)
          {
            paf = malloc(sizeof(*paf));
          }

          paf->type = fread_number(fp);
          paf->duration = fread_number(fp);
          paf->modifier = fread_number(fp);
          paf->location = fread_number(fp);
          paf->bitvector = fread_number(fp);

          AttachToList(paf, obj->affected);

          fMatch = TRUE;
          break;
        }
        break;

      case 'C':
        KEY("Condition", obj->condition, fread_number(fp));
        KEY("Cost", obj->cost, fread_number(fp));
        break;

      case 'D':
        KEY("Description", obj->description, fread_string(fp));
        break;

      case 'E':
        KEY("ExtraFlags", obj->extra_flags, fread_number(fp));

        if (!str_cmp(word, "ExtraDescr"))
        {
          EXTRA_DESCR_DATA *ed;

          if ((ed = (EXTRA_DESCR_DATA *) PopStack(extra_descr_free)) == NULL)
          {
            ed = calloc(1, sizeof(*ed));
          }

          ed->keyword = fread_string(fp);
          ed->description = fread_string(fp);
          AttachToList(ed, obj->extra_descr);
          fMatch = TRUE;
        }

        if (!str_cmp(word, "End"))
        {
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

          if (!fNest || !fVnum || obj->pIndexData == NULL)
          {
            bug("Fread_obj: incomplete object.", 0);

            free_string(obj->name);
            free_string(obj->description);
            free_string(obj->short_descr);
            free_string(obj->questowner);

            PushStack(obj, obj_free);
            return NULL;
          }
          else if (IS_SET(obj->quest, QUEST_ARTIFACT) && (ch == NULL || !check_arti_ownership(ch, obj)))
          {
            free_string(obj->name);
            free_string(obj->description);
            free_string(obj->short_descr);
            free_string(obj->questowner);

            PushStack(obj, obj_free);
            return NULL;
          }
          else if (ch != NULL)
          {
            AttachToList(obj, object_list);

            obj->pIndexData->count++;
            if (iNest == 0 || rgObjNest[iNest - 1] == NULL)
              obj_to_char_end(obj, ch);
            else
              obj_to_obj_end(obj, rgObjNest[iNest - 1]);

            /* initialize events */
            init_events_object(obj);

            return NULL;
          }
          else
	  {
            if (iNest == 0 || rgObjNest[iNest - 1] == NULL)
              return obj;
            else
              obj_to_obj(obj, rgObjNest[iNest - 1]);

            return NULL;
	  }
        }
        break;

      case 'I':
        KEY("ItemType", obj->item_type, fread_number(fp));
        break;

      case 'L':
        KEY("Level", obj->level, fread_number(fp));
        break;

      case 'N':
        KEY("Name", obj->name, fread_string(fp));
        if (!str_cmp(word, "Nest"))
        {
          iNest = fread_number(fp);
          if (iNest < 0 || iNest >= MAX_NEST)
          {
            bug("Fread_obj: bad nest %d.", iNest);
          }
          else
          {
            rgObjNest[iNest] = obj;
            fNest = TRUE;
          }
          fMatch = TRUE;
        }
        break;

      case 'O':
        KEY("OwnerID", obj->ownerid, fread_number(fp));
        break;

      case 'Q':
        KEY("Quest", obj->quest, fread_number(fp));
        KEY("Questowner", obj->questowner, fread_string(fp));
        break;

      case 'R':
        KEY("Resistance", obj->resistance, fread_number(fp));
        break;

      case 'S':
        KEY("ShortDescr", obj->short_descr, fread_string(fp));
        KEY("SentientP", obj->sentient_points, fread_number(fp));
        KEY("Spellflags", obj->spellflags, fread_number(fp));
        if (!str_cmp(word, "Spell"))
        {
          int iValue;
          int sn;

          iValue = fread_number(fp);
          sn = skill_lookup(fread_word(fp));

          if (iValue < 0 || iValue > 3)
            bug("Fread_obj: bad iValue %d.", iValue);
          else if (sn < 0)
            obj->value[iValue] = 0;
          else
            obj->value[iValue] = sn;

          fMatch = TRUE;
          break;
        }
        break;

      case 'T':
        KEY("Toughness", obj->toughness, fread_number(fp));
        break;

      case 'V':
        if (!str_cmp(word, "Values"))
        {
          obj->value[0] = fread_number(fp);
          obj->value[1] = fread_number(fp);
          obj->value[2] = fread_number(fp);
          obj->value[3] = fread_number(fp);
          fMatch = TRUE;
          break;
        }

        if (!str_cmp(word, "Vnum"))
        {
          int vnum;

          vnum = fread_number(fp);
          if (!(obj->pIndexData = get_obj_index(vnum)))
            obj->pIndexData = get_obj_index(OBJ_VNUM_DUMMY);

          fVnum = TRUE;
          fMatch = TRUE;
          break;
        }
        break;

      case 'W':
        KEY("WearFlags", obj->wear_flags, fread_number(fp));
        KEY("WearLoc", obj->wear_loc, fread_number(fp));
        KEY("Weight", obj->weight, fread_number(fp));
        break;
    }
    if (!fMatch)
    {
      bug("Fread_obj: no match.", 0);
      fread_to_eol(fp);
    }
  }

  return NULL;
}

void fread_char(CHAR_DATA *ch, FILE *fp)
{
  char buf[MAX_STRING_LENGTH];
  char *word;
  bool fMatch;
  int sn;
  time_t logout_time = (time_t) 0;
  time_t login_time = (time_t) 0;

  for (;;)
  {
    word = feof(fp) ? "End" : fread_word(fp);
    fMatch = FALSE;

    switch (UPPER(word[0]))
    {
      case '*':
        fMatch = TRUE;
        fread_to_eol(fp);
        break;

      case 'A':
        KEY("Act", ch->act, fread_number(fp));
        KEY("Account", ch->pcdata->account, fread_string(fp));
        KEY("AffectedBy", ch->affected_by, fread_number(fp));
        KEY("Alignment", ch->alignment, fread_number(fp));
        KEY("Armor", ch->armor, fread_number(fp));
        KEY("Avatarmessage", ch->pcdata->avatarmessage, fread_string(fp));
        KEY("Awin", ch->pcdata->awins, fread_number(fp));
        KEY("Alos", ch->pcdata->alosses, fread_number(fp));
        if (!str_cmp(word, "Alias"))
        {
          ALIAS_DATA *alias;

          /* allocate memory for the new alias */
          alias = alloc_alias();

          alias->name = fread_string(fp);
          alias->expand_string = fread_string(fp);

          /* attach it to the rest */
          AttachToList(alias, ch->pcdata->aliases);
          fMatch = TRUE;
          break;
        }
        if (!str_cmp(word, "AffectData"))
        {
          AFFECT_DATA *paf;

          if ((paf = (AFFECT_DATA *) PopStack(affect_free)) == NULL)
          {
            paf = malloc(sizeof(*paf));
          }

          sn = skill_lookup(fread_word(fp));

          if (sn < 0) /* affect has been removed */
          {
            fread_number(fp);
            fread_number(fp);
            fread_number(fp);
            fread_number(fp);
            fMatch = TRUE;
            break;
          }

          paf->type      = sn;
          paf->duration  = fread_number(fp);
          paf->modifier  = fread_number(fp);
          paf->location  = fread_number(fp);
          paf->bitvector = fread_number(fp);

          AttachToList(paf, ch->affected);

          fMatch = TRUE;
          break;
        }

        if (!str_cmp(word, "AttrMod"))
        {
          int mod_str = fread_number(fp);
          int mod_int = fread_number(fp);
          int mod_wis = fread_number(fp);
          int mod_dex = fread_number(fp);
          int mod_con = fread_number(fp);

          ch->pcdata->mod_str = UMIN(25, mod_str);
          ch->pcdata->mod_int = UMIN(25, mod_int);
          ch->pcdata->mod_wis = UMIN(25, mod_wis);
          ch->pcdata->mod_dex = UMIN(25, mod_dex);
          ch->pcdata->mod_con = UMIN(25, mod_con);
          fMatch = TRUE;
          break;
        }

        if (!str_cmp(word, "AttrPerm"))
        {
          ch->pcdata->perm_str = fread_number(fp);
          ch->pcdata->perm_int = fread_number(fp);
          ch->pcdata->perm_wis = fread_number(fp);
          ch->pcdata->perm_dex = fread_number(fp);
          ch->pcdata->perm_con = fread_number(fp);
          fMatch = TRUE;
          break;
        }

        break;

      case 'B':
        KEY("Bamfin", ch->pcdata->bamfin, fread_string(fp));
        KEY("Bamfout", ch->pcdata->bamfout, fread_string(fp));
        KEY("Bounty", ch->pcdata->bounty, fread_number(fp));

        if (!str_cmp(word, "Briefs"))
        {
          ch->pcdata->brief[0] = fread_number(fp);
          ch->pcdata->brief[1] = fread_number(fp);
          ch->pcdata->brief[2] = fread_number(fp);
          ch->pcdata->brief[3] = fread_number(fp);
          ch->pcdata->brief[4] = fread_number(fp);
          ch->pcdata->brief[5] = fread_number(fp);
          ch->pcdata->brief[6] = fread_number(fp);
          ch->pcdata->brief[7] = fread_number(fp);
          ch->pcdata->brief[8] = fread_number(fp);
          ch->pcdata->brief[9] = fread_number(fp);

          fMatch = TRUE;
          break;
        }
        break;

      case 'C':
        KEY("Class", ch->class, fread_number(fp));
        KEY("Conception", ch->pcdata->conception, fread_string(fp));

        if (!str_cmp(word, "Condition"))
        {
          ch->pcdata->condition[0] = fread_number(fp);
          ch->pcdata->condition[1] = fread_number(fp);
          ch->pcdata->condition[2] = fread_number(fp);
          fMatch = TRUE;
          break;
        }
        KEY("Cparents", ch->pcdata->cparents, fread_string(fp));
        KEY("Cprompt", ch->cprompt, fread_string(fp));
        if (!str_cmp(word, "Createtime"))       /* because we need to call free_string */
        {
          free_string(ch->createtime);
          ch->createtime = fread_string(fp);
          fMatch = TRUE;
          break;
        }
        break;

      case 'D':
        KEY("Damroll", ch->damroll, fread_number(fp));
        KEY("Deaf", ch->deaf, fread_number(fp));
        KEY("Decapmessage", ch->pcdata->decapmessage, fread_string(fp));
        KEY("Description", ch->description, fread_string(fp));

      case 'E':
        if (!str_cmp(word, "End"))
          return;
        if (!str_cmp(word, "Event"))
        {
          load_player_event(ch, fp);
          fMatch = TRUE;
          break;
        }
        KEY("EnhCombat", ch->pcdata->enh_combat, fread_string(fp));
        KEY("Exp", ch->exp, fread_number(fp));
        KEY("Extra", ch->extra, fread_number(fp));
        KEY("EvolveCount", ch->pcdata->evolveCount, fread_number(fp));
        break;

      case 'F':
        KEY("Form", ch->form, fread_number(fp));
        break;

      case 'G':
        KEY("Generation", ch->generation, fread_number(fp));
        if (!str_cmp(word, "Genes"))
        {
          ch->pcdata->genes[0] = fread_number(fp);
          ch->pcdata->genes[1] = fread_number(fp);
          ch->pcdata->genes[2] = fread_number(fp);
          ch->pcdata->genes[3] = fread_number(fp);
          ch->pcdata->genes[4] = fread_number(fp);
          ch->pcdata->genes[5] = fread_number(fp);
          ch->pcdata->genes[6] = fread_number(fp);
          ch->pcdata->genes[7] = fread_number(fp);
          ch->pcdata->genes[8] = fread_number(fp);
          ch->pcdata->genes[9] = fread_number(fp);
          fMatch = TRUE;
          break;
        }

        break;

      case 'H':
        KEY("Hitroll", ch->hitroll, fread_number(fp));
        KEY("Home1", ch->home[0], fread_number(fp));
        KEY("Home2", ch->home[1], fread_number(fp));
        KEY("Home3", ch->home[2], fread_number(fp));

        if (!str_cmp(word, "HpManaMove"))
        {
          ch->hit = fread_number(fp);
          ch->max_hit = fread_number(fp);
          ch->mana = fread_number(fp);
          ch->max_mana = fread_number(fp);
          ch->move = fread_number(fp);
          ch->max_move = fread_number(fp);
          fMatch = TRUE;
          break;
        }
        break;

      case 'I':
        KEY("Immune", ch->immune, fread_number(fp));
        KEY("ImmCmd", ch->pcdata->immcmd, fread_string(fp));
        KEY("Itemaffect", ch->itemaffect, fread_number(fp));
        if (!str_cmp(word, "Ignore"))
        {
          IGNORE_DATA *ignore;

          ignore = alloc_ignore();
          ignore->player = fread_string(fp);
          ignore->account = fread_string(fp);
          AttachToList(ignore, ch->pcdata->ignores);

          fMatch = TRUE;
          break;
        }
        break;

      case 'J':
        KEY("Jflags", ch->pcdata->jflags, fread_number(fp));
        break;

      case 'K':
        KEY("Kingdom", ch->pcdata->kingdom, fread_number(fp));
        break;
      case 'L':
        KEY("Lasthost", ch->lasthost, fread_string(fp));
        KEY("Lastdecap1", ch->pcdata->last_decap[0], fread_string(fp));
        KEY("Lastdecap2", ch->pcdata->last_decap[1], fread_string(fp));
        KEY("Lasttime", ch->lasttime, fread_string(fp));
        KEY("Legend", ch->pcdata->legend, fread_number(fp));
        KEY("Level", ch->level, fread_number(fp));
        KEY("Logouttime", logout_time, fread_number(fp));

        if (!str_cmp(word, "Locationhp"))
        {
          ch->loc_hp[0] = fread_number(fp);
          ch->loc_hp[1] = fread_number(fp);
          ch->loc_hp[2] = fread_number(fp);
          ch->loc_hp[3] = fread_number(fp);
          ch->loc_hp[4] = fread_number(fp);
          ch->loc_hp[5] = fread_number(fp);
          ch->loc_hp[6] = fread_number(fp);
          fMatch = TRUE;
          break;
        }
        KEY("Loginmessage", ch->pcdata->loginmessage, fread_string(fp));
        KEY("Logoutmessage", ch->pcdata->logoutmessage, fread_string(fp));
        KEY("LongDescr", ch->long_descr, fread_string(fp));
        if (!str_cmp(word, "Lord"))
        {
          fread_string(fp);
          fMatch = TRUE;
          break;
        }
        break;

      case 'M':
        KEY("Meanparadox", ch->pcdata->mean_paradox_counter, fread_number(fp));
        KEY("Marriage", ch->pcdata->marriage, fread_string(fp));
        KEY("Morph", ch->morph, fread_string(fp));
        break;
      case 'N':
        KEY("Newbits", ch->newbits, fread_number(fp));

        if (!str_cmp(word, "Name"))
        {
          free_string(ch->name);
          ch->name = fread_string(fp);
          fMatch = TRUE;
          break;
        }

        break;

      case 'P':
        KEY("Parents", ch->pcdata->parents, fread_string(fp));
        KEY("Played", ch->played, fread_number(fp));
        KEY("PlayerID", ch->pcdata->playerid, fread_number(fp));

        if (!str_cmp(word, "Power"))
        {
          for (sn = 0; sn < 13; sn++)
            ch->pcdata->powers[sn] = fread_number(fp);

          fMatch = TRUE;
          break;
        }

        KEY("Position", ch->position, fread_number(fp));
        KEY("Practice", ch->practice, fread_number(fp));
        if (!str_cmp(word, "PkPdMkMd"))
        {
          ch->pkill = fread_number(fp);
          ch->pdeath = fread_number(fp);
          ch->mkill = fread_number(fp);
          ch->mdeath = fread_number(fp);
          fMatch = TRUE;
          break;
        }
        KEY("Prompt", ch->prompt, fread_string(fp));
        break;

      case 'Q':
        KEY("Questsrun", ch->pcdata->questsrun, fread_number(fp));

        if (!str_cmp(word, "QuestData"))
        {
          QUEST_DATA *quest;

          if ((quest = (QUEST_DATA *) PopStack(quest_free)) == NULL)
          {
            quest = malloc(sizeof(*quest));
          }

          quest->expire = fread_number(fp);
          quest->type = fread_number(fp);
          quest->giver = fread_number(fp);
          quest->vnums[0] = fread_number(fp);
          quest->vnums[1] = fread_number(fp);
          quest->vnums[2] = fread_number(fp);
          quest->vnums[3] = fread_number(fp);

          AttachToList(quest, ch->pcdata->quests);
          fMatch = TRUE;
          break;
        }
        break;

      case 'R':
        KEY("Retaliation1", ch->pcdata->retaliation[0], fread_string(fp));
        KEY("Retaliation2", ch->pcdata->retaliation[1], fread_string(fp));
        KEY("Revision", ch->pcdata->revision, fread_number(fp));

        if (!str_cmp(word, "Room"))
        {
          ch->in_room = get_room_index(fread_number(fp));

          /* first try altar, second assume limbo exists */
          if (ch->in_room == NULL)
          {
            if (get_room_index(ROOM_VNUM_CITYSAFE) != NULL)
              ch->in_room = get_room_index(ROOM_VNUM_CITYSAFE);
            else
              ch->in_room = get_room_index(ROOM_VNUM_LIMBO);
          }

          fMatch = TRUE;
          break;
        }

        break;

      case 'S':
        if (!str_cmp(word, "Session"))
        {
          ch->pcdata->session->hit     =  fread_number(fp);
          ch->pcdata->session->move    =  fread_number(fp);
          ch->pcdata->session->mana    =  fread_number(fp);
          ch->pcdata->session->quests  =  fread_number(fp);
          ch->pcdata->session->gold    =  fread_number(fp);
          ch->pcdata->session->exp     =  fread_number(fp);
          ch->pcdata->session->pkills  =  fread_number(fp);
          ch->pcdata->session->mkills  =  fread_number(fp);

          /* to long since last logon, reset */
          if (logout_time < current_time - 5 * 60)
          {
            ch->pcdata->session->hit     = 0;
            ch->pcdata->session->move    = 0;
            ch->pcdata->session->mana    = 0;
            ch->pcdata->session->quests  = 0;
            ch->pcdata->session->gold    = 0;
            ch->pcdata->session->exp     = 0;
            ch->pcdata->session->pkills  = 0;
            ch->pcdata->session->mkills  = 0;
          }

          fMatch = TRUE;
          break;
        }
        KEY("SavingThrow", ch->saving_throw, fread_number(fp));
        KEY("Sex", ch->sex, fread_number(fp));
        KEY("ShortDescr", ch->short_descr, fread_string(fp));
        KEY("Security", ch->pcdata->security, fread_number(fp));
        KEY("Status", ch->pcdata->status, fread_number(fp));
        if (!str_cmp(word, "Skill"))
        {
          int value;

          value = fread_number(fp);
          sn = skill_lookup(fread_word(fp));

          if (sn >= 0)
            ch->pcdata->learned[sn] = value;
          fMatch = TRUE;
        }
        KEY("Special", ch->special, fread_number(fp));

        if (!str_cmp(word, "Spells"))
        {
          ch->spl[0] = fread_number(fp);
          ch->spl[1] = fread_number(fp);
          ch->spl[2] = fread_number(fp);
          ch->spl[3] = fread_number(fp);
          ch->spl[4] = fread_number(fp);
          fMatch = TRUE;
          break;
        }

        if (!str_cmp(word, "Stage"))
        {
          ch->pcdata->stage[0] = fread_number(fp);
          ch->pcdata->stage[1] = fread_number(fp);
          ch->pcdata->stage[2] = fread_number(fp);
          fMatch = TRUE;
          break;
        }

        if (!str_cmp(word, "Stances"))
        {
          ch->stance[0] = fread_number(fp);
          ch->stance[1] = fread_number(fp);
          ch->stance[2] = fread_number(fp);
          ch->stance[3] = fread_number(fp);
          ch->stance[4] = fread_number(fp);
          ch->stance[5] = fread_number(fp);
          ch->stance[6] = fread_number(fp);
          ch->stance[7] = fread_number(fp);
          ch->stance[8] = fread_number(fp);
          ch->stance[9] = fread_number(fp);
          ch->stance[10] = fread_number(fp);
          ch->stance[11] = fread_number(fp);
          ch->stance[12] = fread_number(fp);
          ch->stance[13] = fread_number(fp);
          fMatch = TRUE;
          break;
        }
        break;

      case 'T':
        KEY("Tiemessage", ch->pcdata->tiemessage, fread_string(fp));
        KEY("Trust", ch->trust, fread_number(fp));

        if (!str_cmp(word, "Timetick"))
        {
          ch->pcdata->time_tick = fread_number(fp);

          /* if the player logged out more than 5 minutes ago */
          if (logout_time < current_time - 5 * 60)
            ch->pcdata->time_tick = 0;

          fMatch = TRUE;
          break;
        }

        if (!str_cmp(word, "Title"))
        {
          ch->pcdata->title = fread_string(fp);
          if (isalpha(ch->pcdata->title[0]) || isdigit(ch->pcdata->title[0]))
          {
            sprintf(buf, " %s", ch->pcdata->title);
            free_string(ch->pcdata->title);
            ch->pcdata->title = str_dup(buf);
          }
          fMatch = TRUE;
          break;
        }

        break;

      case 'V':
        if (!str_cmp(word, "Vnum"))
        {
          ch->pIndexData = get_mob_index(fread_number(fp));
          fMatch = TRUE;
          break;
        }
        break;

      case 'W':
        if (!str_cmp(word, "Weapons"))
        {
          ch->wpn[0] = fread_number(fp);
          ch->wpn[1] = fread_number(fp);
          ch->wpn[2] = fread_number(fp);
          ch->wpn[3] = fread_number(fp);
          ch->wpn[4] = fread_number(fp);
          ch->wpn[5] = fread_number(fp);
          ch->wpn[6] = fread_number(fp);
          ch->wpn[7] = fread_number(fp);
          ch->wpn[8] = fread_number(fp);
          ch->wpn[9] = fread_number(fp);
          ch->wpn[10] = fread_number(fp);
          ch->wpn[11] = fread_number(fp);
          ch->wpn[12] = fread_number(fp);
          fMatch = TRUE;
          break;
        }
        break;
      case 'X':
        /* it is vital that Logouttime is stored _before_ this is */
        if (!str_cmp(word, "Xlogon"))
        {
          login_time = fread_number(fp);

          if (logout_time < current_time - 5 * 60)
            ch->xlogon = current_time;
          else
            ch->xlogon = login_time;

          fMatch = TRUE;
          break;
        }
        break;
    }

    if (!fMatch)
    {
      sprintf(buf, "Fread_char: no match. WORD: %s", word);
      bug(buf, 0);
      fread_to_eol(fp);
    }
  }
}
