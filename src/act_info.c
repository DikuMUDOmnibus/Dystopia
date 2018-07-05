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

#define _XOPEN_SOURCE
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "dystopia.h"

LIST *top10_list = NULL;

char *const where_name[] = {
  "#0[#GLight         #0]#n ",
  "#0[#GOn Finger     #0]#n ",
  "#0[#GOn Finger     #0]#n ",
  "#0[#GAround Neck   #0]#n ",
  "#0[#GAround Neck   #0]#n ",
  "#0[#GOn Body       #0]#n ",
  "#0[#GOn Head       #0]#n ",
  "#0[#GOn Legs       #0]#n ",
  "#0[#GOn Feet       #0]#n ",
  "#0[#GOn Hands      #0]#n ",
  "#0[#GOn Arms       #0]#n ",
  "#0[#GShield Hand   #0]#n ",
  "#0[#GAround Body   #0]#n ",
  "#0[#GAround Waist  #0]#n ",
  "#0[#GAround Wrist  #0]#n ",
  "#0[#GAround Wrist  #0]#n ",
  "#0[#GRight Hand    #0]#n ",
  "#0[#GLeft Hand     #0]#n ",
  "#0[#GThird Hand    #0]#n ",
  "#0[#GFourth Hand   #0]#n ",
  "#0[#GOn Face       #0]#n ",
  "#0[#GLeft Scabbard #0]#n ",
  "#0[#GRight Scabbard#0]#n ",
  "#0[#GMastery       #0]#n ",
  "#0[#GFloating      #0]#n ",
  "#0[#GMedal         #0]#n ",
  "#0[#GBodyart       #0]#n ",
};

const char *exitname[6] = {
  "to the north",
  "to the east",
  "to the south",
  "to the west",
  "above you",
  "below you"
};

/*
 * Local functions.
 */
void  show_char_to_char_0  ( CHAR_DATA * victim, CHAR_DATA * ch );
void  show_char_to_char_1  ( CHAR_DATA * victim, CHAR_DATA * ch );
void  show_char_to_char    ( LIST *list, CHAR_DATA *ch );
void  check_left_arm       ( CHAR_DATA * ch, CHAR_DATA * victim );
void  check_right_arm      ( CHAR_DATA * ch, CHAR_DATA * victim );
void  check_left_leg       ( CHAR_DATA * ch, CHAR_DATA * victim );
void  check_right_leg      ( CHAR_DATA * ch, CHAR_DATA * victim );
void  add_vote             ( POLL_DATA *poll, CHAR_DATA *ch, int choice );
char *birth_date           ( CHAR_DATA * ch, bool is_self );

void do_immune(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument(argument, arg);

  if (IS_NPC(ch))
    return;

  send_to_char("----------------------------------------------------------------------------\n\r", ch);
  send_to_char("                                -= Immunities =-\n\r", ch);
  send_to_char("----------------------------------------------------------------------------\n\r", ch);
  /* Display weapon resistances */
  send_to_char("Weapons:", ch);
  if (!(!IS_IMMUNE(ch, IMM_SLASH) && !IS_IMMUNE(ch, IMM_STAB) && !IS_IMMUNE(ch, IMM_SMASH) && !IS_IMMUNE(ch, IMM_ANIMAL) && !IS_IMMUNE(ch, IMM_MISC)))
  {
    if (IS_IMMUNE(ch, IMM_SLASH))
      send_to_char(" Slash Slice", ch);
    if (IS_IMMUNE(ch, IMM_STAB))
      send_to_char(" Stab Pierce", ch);
    if (IS_IMMUNE(ch, IMM_SMASH))
      send_to_char(" Blast Crush Pound", ch);
    if (IS_IMMUNE(ch, IMM_ANIMAL))
      send_to_char(" Claw Bite", ch);
    if (IS_IMMUNE(ch, IMM_MISC))
      send_to_char(" Grep Whip Suck", ch);
  }
  else
    send_to_char(" None", ch);
  send_to_char(".\n\r", ch);

  /* Display spell immunities */
  send_to_char("Spells :", ch);
  if (!(!IS_IMMUNE(ch, IMM_CHARM) && !IS_IMMUNE(ch, IMM_HEAT) &&
        !IS_IMMUNE(ch, IMM_COLD) && !IS_IMMUNE(ch, IMM_LIGHTNING) && !IS_IMMUNE(ch, IMM_ACID) && !IS_IMMUNE(ch, IMM_SUMMON) && !IS_IMMUNE(ch, IMM_DRAIN)))
  {
    if (IS_IMMUNE(ch, IMM_CHARM))
      send_to_char(" Charm", ch);
    if (IS_IMMUNE(ch, IMM_HEAT))
      send_to_char(" Heat", ch);
    if (IS_IMMUNE(ch, IMM_COLD))
      send_to_char(" Cold", ch);
    if (IS_IMMUNE(ch, IMM_LIGHTNING))
      send_to_char(" Lightning", ch);
    if (IS_IMMUNE(ch, IMM_ACID))
      send_to_char(" Acid", ch);
    if (!IS_IMMUNE(ch, IMM_SUMMON))
      send_to_char(" Summon", ch);
    if (IS_IMMUNE(ch, IMM_DRAIN))
      send_to_char(" Drain", ch);
    if (IS_IMMUNE(ch, IMM_TRANSPORT))
      send_to_char(" Transport", ch);
  }
  else
    send_to_char(" None", ch);
  send_to_char(".\n\r", ch);

  /* Display skill immunities */
  send_to_char("Skills :", ch);
  if (!(!IS_IMMUNE(ch, IMM_HURL) && !IS_IMMUNE(ch, IMM_BACKSTAB) && !IS_IMMUNE(ch, IMM_KICK) && !IS_IMMUNE(ch, IMM_DISARM) && !IS_IMMUNE(ch, IMM_STEAL)))
  {
    if (IS_IMMUNE(ch, IMM_HURL))
      send_to_char(" Hurl", ch);
    if (IS_IMMUNE(ch, IMM_BACKSTAB))
      send_to_char(" Backstab", ch);
    if (IS_IMMUNE(ch, IMM_KICK))
      send_to_char(" Kick", ch);
    if (IS_IMMUNE(ch, IMM_DISARM))
      send_to_char(" Disarm", ch);
    if (IS_IMMUNE(ch, IMM_STEAL))
      send_to_char(" Steal", ch);
  }
  else
    send_to_char(" None", ch);
  send_to_char(".\n\r", ch);

  send_to_char("----------------------------------------------------------------------------\n\r", ch);
  return;
}


void do_auction(CHAR_DATA *ch, char *argument)
{
  AUCTION_DATA *auction;
  EVENT_DATA *pEvent;
  ITERATOR *pIter;
  OBJ_DATA *obj;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch) || ch->desc == NULL)
    return;

  argument = one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("Syntax: auction [bid|sell|list|id|remove|mybids]\n\r", ch);
    return;
  }

  if (!str_cmp("bid", arg))
  {
    int id, bid;

    argument = one_argument(argument, arg);
    if (arg[0] == '\0')
    {
      send_to_char("Syntax: auction bid [id of item] [your bid]\n\r", ch);
      return;
    }

    if ((id = atoi(arg)) < 1)
    {
      send_to_char("There are not items with an ID below 1.\n\r", ch);
      return;
    }

    one_argument(argument, arg);
    bid = atoi(arg);

    pIter = AllocIterator(auction_list);
    while ((auction = (AUCTION_DATA *) NextInList(pIter)) != NULL)
    {
      if (auction->id != id) continue;

      if ((obj = auction->obj) == NULL)
      {
	send_to_char("You have encountered a bug, please report this.\n\r", ch);
	return;
      }

      if (!str_cmp(auction->seller_account, ch->pcdata->account))
      {
	send_to_char("You cannot bid on your own sales.\n\r", ch);
	return;
      }

      if (auction->bidder_name[0] == '\0' && bid < auction->bid)
      {
	printf_to_char(ch, "You must bid at least %d gold.\n\r", auction->bid);
	return;
      }
      else if (auction->bidder_name[0] != '\0' && bid < 21 * auction->bid / 20)
      {
	printf_to_char(ch, "You must bid at least %d gold.\n\r", 21 * auction->bid / 20);
	return;
      }

      if (getGold(ch) < bid)
      {
	send_to_char("You do not have that much gold.\n\r", ch);
	return;
      }

      /* new bidder, repay the old bidder */
      if (auction->bidder_account[0] != '\0')
      {
	ACCOUNT_DATA *account;

	if (!str_cmp(auction->bidder_account, ch->pcdata->account))
	{
	  send_to_char("You already have the highest bid on this item.\n\r", ch);
	  return;
	}

	if ((account = load_account(auction->bidder_account)) != NULL)
	{
	  account->goldcrowns += auction->bid;
	  save_account(account);
	  close_account(account);
	}
      }

      free_string(auction->bidder_name);
      free_string(auction->bidder_account);
      auction->bidder_name = str_dup(ch->name);
      auction->bidder_account = str_dup(ch->pcdata->account);
      auction->bid = bid;
      setGold(ch, -1 * bid);

      /* if this is a bidout, then set the expire time to now */
      if (auction->bidout > 0 && auction->bid >= auction->bidout)
        auction->expire = current_time;

      save_auctions();

      send_to_char("New bid accepted.\n\r", ch);
      return;
    }

    send_to_char("There is no item with that ID.\n\r", ch);
  }
  else if (!str_cmp("list", arg))
  {
    bool found = FALSE, information = FALSE;
    int section, count = 0;

    one_argument(argument, arg);
    if ((section = atoi(arg)) <= 0)
    {
      section = 1;
      information = TRUE;
    }

    pIter = AllocIterator(auction_list);
    while ((auction = (AUCTION_DATA *) NextInList(pIter)) != NULL)
    {
      count++;

      if (count <= 10 * (section - 1) || count > 10 * section)
	continue;

      if ((obj = auction->obj) == NULL)
      {
	bug("do_auction: auction without item.\n\r", 0);
	continue;
      }

      if (!found)
      {
	printf_to_char(ch, "[%3s] [%-58s] [%10s]\n\r", "ID", "name of item", "cost");
	send_to_char("----------------------------------------"
                     "---------------------------------------\n\r", ch);
	found = TRUE;
      }

      /* add the correct colour */
      if (!str_cmp(auction->seller_name, ch->name))
        sprintf(arg, "#R");
      else if (auction->bidder_name[0] != '\0')
      {
        if (!str_cmp(auction->bidder_name, ch->name))
          sprintf(arg, "#R");
        else
          sprintf(arg, "#y");
      }
      else
        arg[0] = '\0';

      cprintf(buf, "[%3d] [%-58.58s#n] [%s%5d gold#n]\n\r",
        auction->id, obj->short_descr, arg, auction->bid);
      send_to_char(buf, ch);
    }

    if (!found)
      send_to_char("There are no more items for auction.\n\r", ch);
    else if (information)
      send_to_char("\n\rType 'auction list [2|3|4|etc] to get more auctions.\n\r", ch);
  }
  else if (!str_cmp("id", arg))
  {
    int id;

    one_argument(argument, arg);
    if (arg[0] == '\0')
    {
      send_to_char("Syntax: auction id [id of item]\n\r", ch);
      return;
    }

    if ((id = atoi(arg)) < 1)
    {
      send_to_char("There are not items with an ID below 1.\n\r", ch);
      return;
    }

    pIter = AllocIterator(auction_list);
    while ((auction = (AUCTION_DATA *) NextInList(pIter)) != NULL)
    {
      BUFFER *buf_id, *buf_box;

      if (auction->id != id) continue;

      if ((obj = auction->obj) == NULL)
      {
	send_to_char("You have encountered a bug, please report this.\n\r", ch);
	return;
      }

      buf_id = identify_obj(obj);

      bprintf(buf_id, "\n\r #o-:|:-~-:|:-~-:|:-~-:|:-~-:|:--:|:-~-:|:-~-:|:-~-:|:-~-:|:-~-:|:-~-:|:-#n\n\n\r");
      bprintf(buf_id, "This auction will complete at %s.\n\r", smudge_time(auction->expire));
      bprintf(buf_id, "You must bid at least #C%d#n gold to enter the auction.\n\r",
       (auction->bidder_name[0] != '\0') ? 21 * auction->bid / 20 : auction->bid);
      if (auction->bidder_name[0] != '\0')
        bprintf(buf_id, "Highest bid is currently held by #C%s#n.\n\r", auction->bidder_name);

      buf_box = box_text(buf_id->data, "Auction ID");
      send_to_char(buf_box->data, ch);
      buffer_free(buf_id);

      return;
    }

    send_to_char("There is no item with that ID.\n\r", ch);
  }
  else if (!str_cmp("sell", arg))
  {
    AFFECT_DATA *paf;
    int cost, sell_cost = 5, bidout = 0;

    argument = one_argument(argument, arg);
    if (arg[0] == '\0')
    {
      send_to_char("Syntax: auction sell [object] [price] [bidout]\n\r"
                   "        The bidout is optional.\n\r", ch);
      return;
    }

    if ((obj = get_obj_carry(ch, arg)) == NULL)
    {
      send_to_char("You are not carrying that item.\n\r", ch);
      return;
    }

    /* must be claimed, rare, etc etc */
    if (obj->ownerid != ch->pcdata->playerid)
    {
      send_to_char("You can only auction items that you own.\n\r", ch);
      return;
    }

    if (!IS_OBJ_STAT(obj, ITEM_RARE) && !IS_OBJ_STAT(obj, ITEM_SENTIENT))
    {
      send_to_char("You can only auction rare/sentient items.\n\r", ch);
      return;
    }

    if (obj->cost < 50)
    {
      send_to_char("The item must have a value of at least 50.\n\r", ch);
      return;
    }

    argument = one_argument(argument, arg);
    if ((cost = atoi(arg)) < 1 || cost > 10000)
    {
      send_to_char("The price should be between 1 and 10000 goldcrowns.\n\r", ch);
      return;
    }

    argument = one_argument(argument, arg);
    if (arg[0] != '\0')
    {
      bidout = atoi(arg);
      if (bidout < cost || bidout > 20000)
      {
        printf_to_char(ch, "The bidout should be between %d and 20000 goldcrowns.\n\r", cost);
        return;
      }
    }

    if (getGold(ch) < sell_cost)
    {
      printf_to_char(ch, "You need %d more gold to auction this item.\n\r", sell_cost - getGold(ch));
      return;
    }
    setGold(ch, -1 * sell_cost);

    /* strip all events on item */
    pIter = AllocIterator(obj->events);
    while ((pEvent = (EVENT_DATA *) NextInList(pIter)) != NULL)
      dequeue_event(pEvent, TRUE);

    obj_from_char(obj);

    /* strip all temporary affects on this item */
    pIter = AllocIterator(obj->affected);
    while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
    {
      if (paf->duration > 0)
      {
        switch(paf->type)
        {
          default:
            break;
          case OAFF_FROSTBITE:
            REMOVE_BIT(obj->spellflags, OAFF_FROSTBITE);
            break;
          case OAFF_LIQUID:
            REMOVE_BIT(obj->spellflags, OAFF_LIQUID);
            break;
        }

        DetachAtIterator(pIter);
        PushStack(paf, affect_free);
      }
    }

    /* remove the item from the game */
    DetachFromList(obj, object_list);

    obj->ownerid = 0;
    free_string(obj->questowner);
    obj->questowner = str_dup("");

    auction = alloc_auction();
    auction->seller_name = str_dup(ch->name);
    auction->seller_account = str_dup(ch->pcdata->account);
    auction->bid = cost;
    auction->bidout = bidout;
    auction->obj = obj;
    AttachToList(auction, auction_list);

    save_auctions();

    sprintf(buf, "%s has put a new item up for auction (help auction)", ch->name);
    do_info(ch, buf);

    if (IS_SET(ch->deaf, CHANNEL_INFO))
      send_to_char("Ok.\n\r", ch);
  }
  else if (!str_cmp(arg, "remove"))
  {
    int id;

    one_argument(argument, arg);
    if (arg[0] == '\0')
    {
      send_to_char("Syntax: auction remove [id of item]\n\r", ch);
      return;
    }

    if ((id = atoi(arg)) < 1)
    {
      send_to_char("There are not items with an ID below 1.\n\r", ch);
      return;
    }

    pIter = AllocIterator(auction_list);
    while ((auction = (AUCTION_DATA *) NextInList(pIter)) != NULL)
    {
      if (auction->id != id) continue;

      if ((obj = auction->obj) == NULL)
      {
        send_to_char("You have encountered a bug, please report this.\n\r", ch);
        return;
      }

      if (str_cmp(auction->seller_name, ch->name))
      {
        send_to_char("You do not own this auction.\n\r", ch);
        return;
      }

      if (auction->bidder_name[0] != '\0')
      {
        send_to_char("Someone have already bid on this item, you cannot remove it.\n\r", ch);
        return;
      }

      /* give the item to the character */
      auction->obj = NULL;

      AttachToList(obj, object_list);

      obj_to_char(obj, ch);
      free_auction(auction);

      act("$p appears in $n's hands in a blast of lightning.", ch, obj, NULL, TO_ROOM);
      act("$p appears in your hands in a blast of lightning.", ch, obj, NULL, TO_CHAR);
      send_to_char("You cancel the auction.\n\r", ch);
      return;
    }

    send_to_char("There is no item with that ID.\n\r", ch);
  }
  else if (!str_cmp(arg, "mybids"))
  {
    bool found = FALSE;

    pIter = AllocIterator(auction_list);
    while ((auction = (AUCTION_DATA *) NextInList(pIter)) != NULL)
    {
      if (!str_cmp(auction->bidder_name, ch->name))
      {
        if ((obj = auction->obj) == NULL)
        {
          bug("do_auction: auction without item.\n\r", 0);
          continue;
        }

        if (!found)
        {
          printf_to_char(ch, "[%3s] [%-58s] [%10s]\n\r", "ID", "name of item", "cost");
          send_to_char("----------------------------------------"
                       "---------------------------------------\n\r", ch);
          found = TRUE;
        }

        cprintf(buf, "[%3d] [%-58.58s#n] [%5d gold]\n\r",
          auction->id, obj->short_descr, auction->bid);

        send_to_char(buf, ch);
      }
    }

    if (!found)
      send_to_char("You do not have the highest bid on any items.\n\r", ch);
  }
  else
  {
    do_auction(ch, "");
    return;
  }
}

void do_pkready(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch) || !ch->desc)
    return;

  if (argument[0] == '\0')
  {
    do_help(ch, "pkready");
    return;
  }

  if (IS_SET(ch->extra, EXTRA_PKREADY))
  {
    send_to_char("You have already joined the PK rank.\n\r", ch);
    return;
  }

  if (strcmp(crypt(argument, ch->desc->account->owner), ch->desc->account->password))
  {
    send_to_char("Illegal password.\n\r", ch);
    return;
  }

  SET_BIT(ch->extra, EXTRA_PKREADY);
  sprintf(buf, "%s considers %s pkready!", ch->name, (ch->sex == SEX_FEMALE) ? "herself" : "himself");
  do_info(ch, buf);
}

void do_session(CHAR_DATA *ch, char *argument)
{
  BUFFER *buf, *buf2;
  int minutes, hours;

  if (IS_NPC(ch))  
    return;

  /* calculate information */
  minutes = (current_time - ch->xlogon) / 60;
  hours   = minutes / 60;
  minutes = minutes % 60;

  /* present data */
  buf = buffer_new(MAX_STRING_LENGTH);

  bprintf(buf, "  You have been online for #C%d#n hour%s and #C%d#n minute%s.\n\n\r",
    hours, (hours == 1) ? "" : "s", minutes, (minutes == 1) ? "" : "s");

  bprintf(buf, "  Hitpoints  : #G+#C%-5d#n  Magics : #G+#C%-5d#n  Movement   : #G+#C%d#n\n\r",
    ch->pcdata->session->hit, ch->pcdata->session->mana, ch->pcdata->session->move);

  bprintf(buf, "  Quest Runs : #G+#C%-5d#n  Gold   : #G+#C%-5d#n  Experience : #G+#C%d#n\n\r",
    ch->pcdata->session->quests, ch->pcdata->session->gold, ch->pcdata->session->exp);

  bprintf(buf, "  MKills     : #G+#C%-5d#n  PKills : #G+#C%-5d#n\n\r",
    ch->pcdata->session->mkills, ch->pcdata->session->pkills);

  buf2 = box_text(buf->data, "Session Info");
  send_to_char(buf2->data, ch);
  buffer_free(buf);
}

void do_spectate(CHAR_DATA *ch, char *argument)
{
  ITERATOR *pIter, *pIter2;
  SNOOP_DATA *snoop;
  char arg[MAX_STRING_LENGTH];

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    if (in_fortress(ch) || in_arena(ch))
      send_to_char("Spectate [on/off]\n\r", ch);
    else
      send_to_char("Spectate whom?\n\r", ch);

    return;
  }

  if (in_arena(ch) || in_fortress(ch))
  {
    if (!str_cmp(arg, "on"))
    {
      SET_BIT(ch->pcdata->tempflag, TEMP_SPECTATE);
      send_to_char("You now allow others to spectate on you.\n\r", ch);
    }
    else if (!str_cmp(arg, "off"))
    {
      REMOVE_BIT(ch->pcdata->tempflag, TEMP_SPECTATE);
      send_to_char("You no longer allow anyone to spectate you.\n\r", ch);
      stop_spectating(ch);
    }
    else
    {
      do_spectate(ch, "");
    }

    return;
  }
  else
  {
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    bool found = FALSE;

    if (!str_cmp(arg, "off"))
    {
      pIter = AllocIterator(descriptor_list);
      while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
      {
        pIter2 = AllocIterator(d->snoops);
        while ((snoop = (SNOOP_DATA *) NextInList(pIter2)) != NULL)
        {
          if (snoop->snooper == ch->desc)
          {
            if (d->character)
              act("$n stops spectating on you.", ch, NULL, d->character, TO_VICT);
            free_snoop(d, snoop);
            found = TRUE;
          }
        }
      }

      if (found)
        send_to_char("You stop spectating.\n\r", ch);
      else
        send_to_char("You are not spectating on anyone.\n\r", ch);
      return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
      send_to_char("They are not here.\n\r", ch);
      return;
    }

    if (IS_NPC(victim))
    {
      send_to_char("Spectating on monsters is boring.\n\r", ch);
      return;
    }

    if (!in_fortress(victim) && !in_arena(victim))
    {
      send_to_char("They are not in the fortress or arena.\n\r", ch);
      return;
    }

    if (!IS_SET(victim->pcdata->tempflag, TEMP_SPECTATE))
    {
      send_to_char("They do not allow spectating.\n\r", ch);
      return;
    }

    if (victim->desc == NULL)
    {
      send_to_char("You cannot specate on linkdead players.\n\r", ch);
      return;
    }

    pIter = AllocIterator(descriptor_list);
    while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
    {
      pIter2 = AllocIterator(d->snoops);
      while ((snoop = (SNOOP_DATA *) NextInList(pIter2)) != NULL)
      {
        if (snoop->snooper == ch->desc)
        {
          send_to_char("You are already spectating on someone else.\n\r", ch);
          send_to_char("If you want to stop spectating, type 'spectate off'.\n\r", ch);
          return;
        }
      }
    }

    /* create new snoop and attach */
    snoop = alloc_snoop();
    snoop->snooper = ch->desc;
    AttachToList(snoop, victim->desc->snoops);

    act("You start spectating on $N.", ch, NULL, victim, TO_CHAR);
    act("$n starts spectating on you.", ch, NULL, victim, TO_VICT);
  }
}

void do_evolve(CHAR_DATA *ch, char *argument)
{
  EVOLVE_DATA evolve;
  char buf[MAX_STRING_LENGTH];

  /* reset evolve data */
  evolve.valid    = FALSE;
  evolve.field    = NULL;
  evolve.error[0] = '\0';
  evolve.mana     = 0;
  evolve.hps      = 0;
  evolve.move     = 0;
  evolve.gold     = 0;
  evolve.exp      = 0;
  evolve.bit      = 0;

  if (IS_NPC(ch) || !ch->desc)
    return;
  else if (!str_prefix("clear ", argument))
  {
    argument = one_argument(argument, buf);

    if (strcmp(crypt(argument, ch->desc->account->owner), ch->desc->account->password))
    {
      send_to_char("You must confirm with your password. Please remember that you can\n\r"
                   "only clear your evolves once. After this attempt, your will be stuck\n\r"
                   "with your chosen evolves forever.\n\n\r"
                   "Syntax: evolve clear [password]\n\r", ch);
      return;
    }

    ch->pcdata->powers[EVOLVE_1] = 0;
    ch->pcdata->powers[EVOLVE_2] = 0;
    ch->pcdata->powers[EVOLVE_3] = 0;
    ch->pcdata->evolveCount = 0;
    send_to_char("All your evolves has been cleared.\n\r", ch);
    clearstats(ch);
    return;
  }
  else if (IS_CLASS(ch, CLASS_WARLOCK))
    warlock_evolve(ch, argument, &evolve);
  else if (IS_CLASS(ch, CLASS_SHADOW))
    shadow_evolve(ch, argument, &evolve);
  else if (IS_CLASS(ch, CLASS_GIANT))
    giant_evolve(ch, argument, &evolve);
  else if (IS_CLASS(ch, CLASS_FAE))
    fae_evolve(ch, argument, &evolve);
  else
  {
    send_to_char("Your class cannot evolve yet.\n\r", ch);
    return;
  }

  /* not possible to do this ? */
  if (evolve.valid == FALSE)
  {
    mxp_to_char(evolve.error, ch, MXP_ALL);
    return;
  }

  if (ch->max_hit < evolve.hps)
  {
    sprintf(buf, "You need %d more hps before you can gain this evolve.\n\r", evolve.hps - ch->max_hit);
    send_to_char(buf, ch);
    return;
  }

  if (ch->max_mana < evolve.mana)
  {
    sprintf(buf, "You need %d more mana before you can gain this evolve.\n\r", evolve.mana - ch->max_mana);
    send_to_char(buf, ch);
    return;
  }

  if (ch->max_move < evolve.move)
  {
    sprintf(buf, "You need %d more move before you can gain this evolve.\n\r", evolve.move - ch->max_move);
    send_to_char(buf, ch);
    return;
  }

  if (ch->exp < evolve.exp)
  {
    sprintf(buf, "You need %d more exp before you can gain this evolve.\n\r", evolve.exp - ch->exp);
    send_to_char(buf, ch);
    return;
  }

  if (getGold(ch) < evolve.gold)   
  {
    sprintf(buf, "You need %d more goldcrowns before you can gain this evolve.\n\r", evolve.gold - getGold(ch));
    send_to_char(buf, ch);
    return;
  }

  /* take the quest points and experience points */
  ch->exp -= evolve.exp;
  setGold(ch, -1 * evolve.gold);
  ch->pcdata->evolveCount++;

  /* reduce statistics */
  ch->max_hit  = 3 * ch->max_hit  / 4;
  ch->max_mana = 3 * ch->max_mana / 4;
  ch->max_move = 3 * ch->max_move / 4;

  /* do a restore of statistics */
  ch->hit = ch->max_hit;
  ch->move = ch->max_move;
  ch->mana = ch->max_mana;

  /* set the bit on the field */
  SET_BIT(*evolve.field, evolve.bit);

  send_to_char("You have succesfully evolved.\n\r", ch);
}

void do_mudinfo(CHAR_DATA *ch, char *argument)
{
  printf_to_char(ch, " %s\n\n\r", get_dystopia_banner("Mudinfo", 56));

  printf_to_char(ch, "  Decapitations this week                  %3d kill%s\n\r",
    muddata.pk_count_now[0], (muddata.pk_count_now[0] == 1) ? "" : "s");

  printf_to_char(ch, "  Gensteals this week                      %3d steal%s\n\r",
    muddata.pk_count_now[2], (muddata.pk_count_now[2] == 1) ? "" : "s");

  printf_to_char(ch, "  Average Players Online                   %3d player%s\n\r",
    (muddata.mudinfo[MUDINFO_MCCP_USERS] + muddata.mudinfo[MUDINFO_OTHER_USERS]) /
     muddata.mudinfo[MUDINFO_UPDATED], ((muddata.mudinfo[MUDINFO_MCCP_USERS] +
     muddata.mudinfo[MUDINFO_OTHER_USERS]) / muddata.mudinfo[MUDINFO_UPDATED] == 1) ? "" : "s");

  printf_to_char(ch, "  Peak Players Online (this week)          %3d player%s\n\r",
    muddata.mudinfo[MUDINFO_PEAK_USERS], (muddata.mudinfo[MUDINFO_PEAK_USERS] == 1) ? "" : "s");

  printf_to_char(ch, "  Number of areas for Calim's Cradle       %3d area%s\n\r",
    top_area, (top_area == 1) ? "" : "s");

  printf_to_char(ch, "  Number of rooms for Calim's Cradle      %4d room%s\n\r",
    top_room, (top_room == 1) ? "" : "s");

  printf_to_char(ch, "\n\r  Data collected over %d day%s and %d hour%s\n\r",
    (muddata.mudinfo[MUDINFO_UPDATED] / (120 * 24)),
    (muddata.mudinfo[MUDINFO_UPDATED] / (120 * 24) == 1) ? "" : "s",
    (muddata.mudinfo[MUDINFO_UPDATED] / 120 - 24 * (muddata.mudinfo[MUDINFO_UPDATED] / (120 * 24))),
    (muddata.mudinfo[MUDINFO_UPDATED] / 120 - 24 * (muddata.mudinfo[MUDINFO_UPDATED] / (120 * 24)) == 1) ? "" : "s");

  printf_to_char(ch, "\n\r %s\n\r", get_dystopia_banner("", 56));
}

void do_top10(CHAR_DATA *ch, char *argument)
{
  TOP10_ENTRY *entry;
  ITERATOR *pIter;
  BUFFER *buf = buffer_new(MAX_STRING_LENGTH), *buf2;

  bprintf(buf, "  #G%-14s %-7s  %-6s  %-7s  %-5s#n\n\n\r",
    "name", "pkscore", "pkills", "pdeaths", "hours");

  pIter = AllocIterator(top10_list);
  while ((entry = (TOP10_ENTRY *) NextInList(pIter)) != NULL)
  {
    bprintf(buf, "  %-14s  %5d    %3d     %3d      %3d\n\r",
      entry->name, entry->pkscore, entry->pkills, entry->pdeaths, entry->hours);
  }

  buf2 = box_text(buf->data, "Top 10 PK'ers");
  send_to_char(buf2->data, ch);
  buffer_free(buf);
}

void do_oldprompt(CHAR_DATA *ch, char *argument)
{
  if (IS_SET(ch->extra, EXTRA_PROMPT))
  {
    send_to_char("You already have a customized prompt.\n\r", ch);
    return;
  }

  free_string(ch->prompt);
  free_string(ch->cprompt);

  ch->prompt  = str_dup("#n<[#C%x#n] [%hH %mM %vV] [#C%t#n ft]> ");
  ch->cprompt = str_dup("#n<[%f] [%hH %mM %vV] [#C%t#n ft]> ");

  SET_BIT(ch->extra, EXTRA_PROMPT);

  send_to_char("Ok. You now use the old prompt.\n\r", ch);
}

void do_rarelist(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  LIST *list;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];
  bool found = FALSE;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    list = ch->carrying;
    if (SizeOfList(list) == 0)
    {
      send_to_char("You are not carrying anything.\n\r", ch);
      return;
    }

    send_to_char("You are carrying:\n\r", ch);
  }
  else
  {
    OBJ_DATA *container;

    if ((container = get_obj_carry(ch, arg)) == NULL)
    {
      if ((container = get_obj_wear(ch, arg)) == NULL)
      {
        if ((container = get_obj_list(ch, arg, ch->in_room->contents)) == NULL)
        {
          send_to_char("You where unable to find that container of rares.\n\r", ch);
          return;
        }
      }
    }

    if (container->item_type != ITEM_CONTAINER)
    {
      act("$p is not a container.", ch, container, NULL, TO_CHAR);
      return;
    }

    list = container->contains;
    if (SizeOfList(list) == 0)
    {
      act("$p is empty.", ch, container, NULL, TO_CHAR);
      return;
    }

    act("$p contains:", ch, container, NULL, TO_CHAR);
  }

  pIter = AllocIterator(list);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    char buf[MAX_STRING_LENGTH];

    if (obj->wear_loc != WEAR_NONE)
      continue;

    if (IS_OBJ_STAT(obj, ITEM_RARE) || IS_OBJ_STAT(obj, ITEM_SENTIENT))
    {
      found = TRUE;

      sprintf(buf, " [%3d] %s\n\r", obj->cost, format_obj_to_char(obj, ch, TRUE));
      send_to_char(buf, ch);
    }
  }

  if (!found)
    send_to_char("       Nothing.\n\r", ch);
}

void do_enhcombat(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int i, found1 = -1, found2 = -1;
  const char *c_table[8][2] =
  {
    { "red",      "#R" },
    { "green",    "#G" },
    { "cyan",     "#C" },
    { "blue",     "#L" },
    { "purple",   "#P" },
    { "yellow",   "#y" },
    { "none",     "#n" },

    /* terminate */
    { "", "" }
  };

  if (IS_NPC(ch)) return;

  argument = one_argument(argument, arg);
  one_argument(argument, arg2);

  if (arg[0] == '\0' || arg2[0] == '\0')
  {
    send_to_char("What colour should the emphasized text be in?\n\r", ch);
    send_to_char("  - red, green, cyan, blue, purple, yellow or none.\n\n\r", ch);
    send_to_char("Syntax: enhcombat [your messages] [opponent messages]\n\r", ch);
    return;
  }

  for (i = 0; c_table[i][0][0] != '\0'; i++)
  {
    if (!str_cmp(arg, c_table[i][0]))
      found1 = i;
    if (!str_cmp(arg2, c_table[i][0]))
      found2 = i;
  }

  if (found1 == -1 || found2 == -1)
  {
    do_enhcombat(ch, "");
    return;
  }

  if (!str_cmp(arg, "none") && !str_cmp(arg2, "none"))
  {
    REMOVE_BIT(ch->newbits, NEW_ENH_COMBAT);
  }
  else
  {
    SET_BIT(ch->newbits, NEW_ENH_COMBAT);

    sprintf(arg, "%s%s", c_table[found1][1], c_table[found2][1]);
    free_string(ch->pcdata->enh_combat);
    ch->pcdata->enh_combat = str_dup(arg);
  }
  send_to_char("Ok.\n\r", ch);
}

void do_reimb(CHAR_DATA *ch, char *argument)
{
  char field[MAX_INPUT_LENGTH];
  char value[MAX_INPUT_LENGTH];
  int age;

  if (IS_NPC(ch)) return;

  if (!ch->desc || !ch->desc->account)
    return;

  argument = one_argument(argument, field);
  argument = one_argument(argument, value);

  age = (ch->played + (int) (current_time - ch->logon)) / 3600;

  if (field[0] == '\0')
  {
    BUFFER *buf = buffer_new(MAX_STRING_LENGTH);

    bprintf(buf, " %s\n\n\r", get_dystopia_banner("Reimb Screen", 72));

    bprintf(buf, "  Syntax: reimb [field] [value]\n\n\r");
    bprintf(buf, "  #u#9Field#n\n\r");
    bprintf(buf, "   spells  (10 pts) : green blue red yellow purple.\n\r");
    bprintf(buf, "   stances ( 5 pts) : viper crane crab mongoose bull dragon swallow.\n\r");
    bprintf(buf, "   weapons ( 5 pts) : slice slash whip claw pound crush pierce unarmed.\n\r");
    bprintf(buf, "   gold    ( 1 pts) : gain 25 goldcrowns per reimb point spend.\n\r");
    bprintf(buf, "   exp     (35 pts) : gain 10.000.000 experience points.\n\r");

    if (!IS_SET(ch->newbits, NEW_REIMBCLASS) && ch->class != 0)
      bprintf(buf, "   class   (35 pts) : boosts some of your class powers.\n\r");
    if (!IS_SET(ch->newbits, NEW_MASTERY) && IS_SET(ch->desc->account->flags, ACCOUNT_FLAG_MASTERY))
      bprintf(buf, "   mastery (10 pts) : gain mastery.\n\r");

    bprintf(buf, "\n\r  You have #C%d#n reimb points.\n\r", ch->desc->account->reimb_points);
    bprintf(buf, "\n\r %s\n\r", get_dystopia_banner("", 72));

    send_to_char(buf->data, ch);
    buffer_free(buf);
    return;
  }
  if (!str_cmp(field, "spells"))
  {
    int cost = 10;

    if (value[0] == '\0')
    {
      send_to_char("What spell colour do you wish to raise to 200?\n\r", ch);
      return;
    }

    if (ch->desc->account->reimb_points < cost)
    {
      send_to_char("You do not have enough reimb points to do that.\n\r", ch);
      return;
    }

    if (!str_cmp(value, "red"))
    {
      if (ch->spl[RED_MAGIC] >= 200)
      {
        send_to_char("You have already mastered that spell colour.\n\r", ch);
        return;
      }
      ch->spl[RED_MAGIC] = 200;
    }
    else if (!str_cmp(value, "green"))
    {
      if (ch->spl[GREEN_MAGIC] >= 200)
      {
        send_to_char("You have already mastered that spell colour.\n\r", ch);
        return;
      }
      ch->spl[GREEN_MAGIC] = 200;
    }
    else if (!str_cmp(value, "yellow"))
    {
      if (ch->spl[YELLOW_MAGIC] >= 200)
      {
        send_to_char("You have already mastered that spell colour.\n\r", ch);
        return;
      }
      ch->spl[YELLOW_MAGIC] = 200;
    }
    else if (!str_cmp(value, "blue"))
    {
      if (ch->spl[BLUE_MAGIC] >= 200)
      {
        send_to_char("You have already mastered that spell colour.\n\r", ch);
        return;
      }
      ch->spl[BLUE_MAGIC] = 200;
    }
    else if (!str_cmp(value, "purple"))
    {
      if (ch->spl[PURPLE_MAGIC] >= 200)
      {
        send_to_char("You have already mastered that spell colour.\n\r", ch);
        return;
      }
      ch->spl[PURPLE_MAGIC] = 200;
    }
    else
    {
      send_to_char("There is no spell colour by that name.\n\r", ch);
      return;
    }

    ch->desc->account->reimb_points -= cost;
    send_to_char("Reimbed.\n\r", ch);
    return;
  }
  else if (!str_cmp(field, "stances"))
  {
    int cost = 5;
    
    if (value[0] == '\0')
    {
      send_to_char("What stance do you wish to raise to 200?\n\r", ch);
      return;
    }

    if (age < 1)
    {
      send_to_char("Your character must be atleast 1 hour old before you can reimb stances.\n\r", ch);
      return;
    }

    if (ch->desc->account->reimb_points < cost)
    {
      send_to_char("You do not have enough reimb points to do that.\n\r", ch);
      return;
    }

    if (!str_cmp(value, "viper"))
    {
      if (ch->stance[STANCE_VIPER] >= 200)
      {
        send_to_char("You have already mastered that stance.\n\r", ch);
        return;
      }
      ch->stance[STANCE_VIPER] = 200;
    }
    else if (!str_cmp(value, "crane"))
    {
      if (ch->stance[STANCE_CRANE] >= 200)
      {
        send_to_char("You have already mastered that stance.\n\r", ch);
        return;
      }
      ch->stance[STANCE_CRANE] = 200;
    }
    else if (!str_cmp(value, "crab"))
    {
      if (ch->stance[STANCE_CRAB] >= 200)
      {
        send_to_char("You have already mastered that stance.\n\r", ch);
        return;
      }
      ch->stance[STANCE_CRAB] = 200;
    }
    else if (!str_cmp(value, "mongoose"))
    {
      if (ch->stance[STANCE_MONGOOSE] >= 200)
      {
        send_to_char("You have already mastered that stance.\n\r", ch);
        return;
      }
      ch->stance[STANCE_MONGOOSE] = 200;
    }
    else if (!str_cmp(value, "bull"))
    {
      if (ch->stance[STANCE_BULL] >= 200)
      {
        send_to_char("You have already mastered that stance.\n\r", ch);
        return;
      }
      ch->stance[STANCE_BULL] = 200;
    }
    else if (!str_cmp(value, "dragon"))
    {
      if (ch->stance[STANCE_DRAGON] >= 200)
      {
        send_to_char("You have already mastered that stance.\n\r", ch);
        return;
      }
      ch->stance[STANCE_DRAGON] = 200;
    }
    else if (!str_cmp(value, "swallow"))
    {
      if (ch->stance[STANCE_SWALLOW] >= 200)
      {
        send_to_char("You have already mastered that stance.\n\r", ch);
        return;
      }
      ch->stance[STANCE_SWALLOW] = 200;
    }
    else
    {
      send_to_char("There is no stance by that name.\n\r", ch);
      return;
    }

    ch->desc->account->reimb_points -= cost;
    send_to_char("Reimbed.\n\r", ch);
    return;
  }
  else if (!str_cmp(field, "weapons"))
  {
    int cost = 5;

    if (value[0] == '\0')
    {
      send_to_char("What weapon do you wish to raise to 200?\n\r", ch);
      return;
    }

    if (age < 1)
    {
      send_to_char("Your character must be atleast 1 hour old before you can reimb weapons.\n\r", ch);
      return;
    }

    if (ch->desc->account->reimb_points < cost)
    {
      send_to_char("You do not have enough reimb points to do that.\n\r", ch);
      return;
    }

    if (!str_cmp(value, "unarmed"))
    {
      if (ch->wpn[0] >= 200)
      {
        send_to_char("You have already mastered that weapon.\n\r", ch);
        return;
      }
      ch->wpn[0] = 200;
    }
    else if (!str_cmp(value, "slice"))
    {
      if (ch->wpn[1] >= 200)
      {
        send_to_char("You have already mastered that weapon.\n\r", ch);
        return;
      }
      ch->wpn[1] = 200;
    }
    else if (!str_cmp(value, "slash"))
    {
      if (ch->wpn[3] >= 200)
      {
        send_to_char("You have already mastered that weapon.\n\r", ch);
        return;
      }
      ch->wpn[3] = 200;
    }
    else if (!str_cmp(value, "whip"))
    {
      if (ch->wpn[4] >= 200)
      {
        send_to_char("You have already mastered that weapon.\n\r", ch);
        return;
      }
      ch->wpn[4] = 200;
    }
    else if (!str_cmp(value, "claw"))
    {
      if (ch->wpn[5] >= 200)
      {
        send_to_char("You have already mastered that weapon.\n\r", ch);
        return;
      }
      ch->wpn[5] = 200;
    }
    else if (!str_cmp(value, "pound"))
    {
      if (ch->wpn[7] >= 200)
      {
        send_to_char("You have already mastered that weapon.\n\r", ch);
        return;
      }
      ch->wpn[7] = 200;
    }
    else if (!str_cmp(value, "crush"))
    {
      if (ch->wpn[8] >= 200)
      {
        send_to_char("You have already mastered that weapon.\n\r", ch);
        return;
      }
      ch->wpn[8] = 200;
    }
    else if (!str_cmp(value, "pierce"))
    {
      if (ch->wpn[11] >= 200)
      {
        send_to_char("You have already mastered that weapon.\n\r", ch);
        return;
      }
      ch->wpn[11] = 200;
    }
    else
    {  
      send_to_char("There is no weapon skill by that name.\n\r", ch);
      return;
    }

    ch->desc->account->reimb_points -= cost;
    send_to_char("Reimbed.\n\r", ch);
    return;
  }
  else if (!str_cmp(field, "gold"))
  {
    int amount;

    if ((amount = atoi(value)) <= 0 || amount > ch->desc->account->reimb_points)
    {
      send_to_char("Syntax: reimb gold [amount of reimbpoints to use]\n\r", ch);
      return;
    }

    ch->desc->account->reimb_points -= amount;
    setGold(ch, amount * 25);
    setGoldTotal(ch, amount * 25);
    send_to_char("Reimbed.\n\r", ch);
    return;
  }
  else if (!str_cmp(field, "exp"))
  {
    int cost = 35;

    if (age < 1)
    {
      send_to_char("Your character must be atleast 1 hour old to gain this reimb.\n\r", ch);
      return;
    }
    if (ch->desc->account->reimb_points < cost)
    {
      send_to_char("You don't have enough reimb points to do this.\n\r", ch);
      return;
    }
    ch->exp += 10000000;
    ch->desc->account->reimb_points -= cost;
    send_to_char("Reimbed.\n\r", ch);
    return;
  }
  else if (!str_cmp(field, "class"))
  {
    int cost = 35;

    if (ch->class == 0)
    {
      send_to_char("You do not have a class.\n\r", ch);
      return;
    }
    if (age < 2)
    {
      send_to_char("Your character must be at least 2 hours old before you can boost its class.\n\r", ch);
      return;
    }
    if (IS_SET(ch->newbits, NEW_REIMBCLASS))
    {
      send_to_char("You have already boosted your class powers.\n\r", ch);
      return;
    }
    if (ch->desc->account->reimb_points < cost)
    {
      send_to_char("You don't have enough reimb points to do this.\n\r", ch);
      return;
    }

    if (ch->class == CLASS_GIANT)
    {
      ch->pcdata->powers[GIANT_POINTS] += 100000;
      ch->pcdata->powers[GIANT_ATT] = UMIN(10, ch->pcdata->powers[GIANT_ATT] + 2);
      ch->pcdata->powers[GIANT_DEF] = UMIN(10, ch->pcdata->powers[GIANT_DEF] + 2);
    }
    else if (ch->class == CLASS_SHADOW)
    {
      ch->pcdata->powers[SHADOW_POWER] += 100000;
      ch->pcdata->powers[SHADOW_MARTIAL] = UMIN(25, ch->pcdata->powers[SHADOW_MARTIAL] + 3);
    }
    else if (ch->class == CLASS_FAE)
    {
      if (ch->pcdata->powers[FAE_PLASMA] >= 7
       && ch->pcdata->powers[FAE_MATTER] >= 7
       && ch->pcdata->powers[FAE_ENERGY] >= 7
       && ch->pcdata->powers[FAE_WILL] >= 7)
      {
        send_to_char("You would not gain anything from this.\n\r", ch);
        return;
      }

      if (ch->pcdata->powers[FAE_PLASMA] < 7)
        ch->pcdata->powers[FAE_PLASMA]++;
      if (ch->pcdata->powers[FAE_MATTER] < 7)
        ch->pcdata->powers[FAE_MATTER]++;
      if (ch->pcdata->powers[FAE_ENERGY]  < 7)
        ch->pcdata->powers[FAE_ENERGY]++;
      if (ch->pcdata->powers[FAE_WILL] < 7)
        ch->pcdata->powers[FAE_WILL]++;

      ch->practice += 200;
    }
    else if (ch->class == CLASS_WARLOCK)
    {
      EVENT_DATA *event;

      if (ch->pcdata->powers[SPHERE_DIVINATION] >= 4
       && ch->pcdata->powers[SPHERE_ABJURATION] >= 4
       && ch->pcdata->powers[SPHERE_ENCHANTMENT] >= 4
       && ch->pcdata->powers[SPHERE_INVOCATION] >= 4
       && ch->pcdata->powers[SPHERE_NECROMANCY] >= 4
       && ch->pcdata->powers[SPHERE_SUMMONING] >= 4)
      {
        send_to_char("You would not gain anything from this.\n\r", ch);
        return;
      }

      /* remove any study events */
      if ((event = event_isset_mobile(ch, EVENT_PLAYER_STUDY)) != NULL)
        dequeue_event(event, TRUE);

      if (ch->pcdata->powers[SPHERE_DIVINATION] < 4)
        ch->pcdata->powers[SPHERE_DIVINATION]++;
      if (ch->pcdata->powers[SPHERE_ABJURATION] < 4)
        ch->pcdata->powers[SPHERE_ABJURATION]++;
      if (ch->pcdata->powers[SPHERE_ENCHANTMENT] < 4)
        ch->pcdata->powers[SPHERE_ENCHANTMENT]++;
      if (ch->pcdata->powers[SPHERE_INVOCATION] < 4)
        ch->pcdata->powers[SPHERE_INVOCATION]++;
      if (ch->pcdata->powers[SPHERE_NECROMANCY] < 4)
        ch->pcdata->powers[SPHERE_NECROMANCY]++;
      if (ch->pcdata->powers[SPHERE_SUMMONING] < 4)
        ch->pcdata->powers[SPHERE_SUMMONING]++;
    }
    else
    {
      send_to_char("Your class cannot be reimbed. Tell an immortal.\n\r", ch);
      return;
    }

    SET_BIT(ch->newbits, NEW_REIMBCLASS);
    ch->desc->account->reimb_points -= cost;
    send_to_char("Reimbed.\n\r", ch);
    return;
  }
  else if (!str_cmp(field, "mastery"))
  {
    int cost = 10, i;

    if (age < 4)
    {
      send_to_char("Your character must be atleast 4 hours old before it can regain mastery.\n\r", ch);
      return;
    }
    if (!IS_SET(ch->desc->account->flags, ACCOUNT_FLAG_MASTERY))
    {
      send_to_char("You cannot reimb mastery with this account.\n\r", ch);
      return;
    }
    if (IS_SET(ch->newbits, NEW_MASTERY))
    {
      send_to_char("This character already have mastery.\n\r", ch);
      return;
    }
    if (ch->desc->account->reimb_points < cost)
    {
      send_to_char("You don't have enough reimb points to do this.\n\r", ch);
      return;
    }

    for (i = 0; i < 5; i++)
      ch->spl[i] = UMAX(ch->spl[i], 200);
    for (i = 0; i < 13; i++)
      ch->wpn[i] = UMAX(ch->wpn[i], 200);
    for (i = 1; i < 11; i++)
      ch->stance[i] = UMAX(ch->stance[i], 200);

    REMOVE_BIT(ch->desc->account->flags, ACCOUNT_FLAG_MASTERY);
    ch->desc->account->reimb_points -= cost;
    send_to_char("Reimbed. Please type 'mastery'.\n\r", ch);
    return;
  }
  else
  {
    do_reimb(ch, "");
    return;
  }
}

void do_timer(CHAR_DATA *ch, char *argument)
{
  EVENT_DATA *event;
  ITERATOR *pIter;

  pIter = AllocIterator(global_event_list);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == EVENT_GAME_ARENA)
    {
      char buf[MAX_INPUT_LENGTH];

      sprintf(buf, "The next arena is in %s\n\r", event_time_left(event));
      send_to_char(buf, ch);

      return;
    }
  }

  send_to_char("The arena is broken, please report this.\n\r", ch);
}

void do_donate(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj, *pit;
  ROOM_INDEX_DATA *pRoom;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("What item do you wish to donate?\n\r", ch);
    return;
  }

  if ((obj = get_obj_carry(ch, arg)) == NULL)
  {
    send_to_char("You do not have that item.\n\r", ch);
    return;
  }

  if (obj->ownerid != 0)
  {
    send_to_char("That item is owned, first you must unclaim it.\n\r", ch);
    return;
  }

  if ((pRoom = get_room_index(ROOM_VNUM_CITYSAFE)) == NULL)
  {
    send_to_char("Some gremlins have stolen the MUD, please report this.\n\r", ch);
    return;
  }

  pIter = AllocIterator(pRoom->contents);
  while ((pit = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (pit->pIndexData->vnum == OBJ_VNUM_PIT)
      break;
  }

  if (pit == NULL)
  {
    send_to_char("The pit seems to be missing.\n\r", ch);
    return;
  }

  act("$n donates $p to the donation pit.", ch, obj, NULL, TO_ROOM);
  act("You donate $p to the donation pit.", ch, obj, NULL, TO_CHAR);
  obj_from_char(obj);
  obj_to_obj(obj, pit);
}

void do_disenchant(CHAR_DATA *ch, char *argument)
{
  AREA_DATA *pArea;
  AREA_AFFECT *paf;
  ITERATOR *pIter;
  bool found = FALSE;

  if (ch->in_room == NULL || IS_NPC(ch))
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }
  pArea = ch->in_room->area;

  pIter = AllocIterator(pArea->affects);
  while ((paf = (AREA_AFFECT *) NextInList(pIter)) != NULL)
  {
    if (paf->owner == ch->pcdata->playerid)
    {
      paf->duration = 1;
      found = TRUE;
    }
  }
  if (!found)
    send_to_char("You have no area enchantments running in this area.\n\r", ch);
  else
    send_to_char("Ok.\n\r", ch);
}

void do_areaaff(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *gch;
  char buf[MAX_STRING_LENGTH];
  AREA_AFFECT *aff;
  ITERATOR *pIter;
  bool found = FALSE;
  AREA_DATA *pArea;

  if (!ch->in_room || (pArea = ch->in_room->area) == NULL)
  {
    send_to_char("You have encounted a bug, please report this.\n\r", ch);
    return;
  }

  send_to_char("#u#9Area Affects in this area#n\n\r", ch);
  pIter = AllocIterator(pArea->affects);
  while ((aff = (AREA_AFFECT *) NextInList(pIter)) != NULL)
  {
    found = TRUE;

    switch(aff->type)
    {
      default:
        sprintf(buf, "o Unknown enchantment laid by %s for %d seconds.\n\r",
          ((gch = get_online_player(aff->owner)) == NULL) ? "unknown" : gch->name,
          aff->duration * 10);
        break;
      case AREA_AFF_EARTHMOTHER:
        sprintf(buf, "o Earthmother enchantment laid by %s for %d seconds.\n\r", 
          ((gch = get_online_player(aff->owner)) == NULL) ? "unknown" : gch->name,
          aff->duration * 10);
        break;
      case AREA_AFF_PLAGUE:
        sprintf(buf, "o Plague enchantment laid by %s for %d seconds.\n\r",
          ((gch = get_online_player(aff->owner)) == NULL) ? "unknown" : gch->name,
          aff->duration * 10);
        break;
      case AREA_AFF_MILKANDHONEY:
        sprintf(buf, "o Land of milk and honey enchantment laid by %s for %d seconds.\n\r",
          ((gch = get_online_player(aff->owner)) == NULL) ? "unknown" : gch->name,
          aff->duration * 10);
        break;
    }
    send_to_char(buf, ch);
  }

  if (!found)
  {
    send_to_char("There are no enchantments in this area.\n\r", ch);
  }
}

void do_bountylist(CHAR_DATA *ch, char *argument)
{
  BUFFER *buf = buffer_new(MAX_STRING_LENGTH), *buf2;
  CHAR_DATA *gch;
  ITERATOR *pIter;
  DESCRIPTOR_DATA *d;

  bprintf(buf, "  #G%-12s    %6s    %7s    %3s    %6s#n\n\r", 
    "Name", "Bounty", "Pkscore", "Gen", "Status");

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!d->connected == CON_PLAYING || (gch = d->character) == NULL)
      continue;
    if (gch->level > 6)
      continue;
    if (!can_see(ch, d->character))
      continue;

    bprintf(buf, "  #C%-12s#n    %5d      %5d      %d      %3d\n\r",
      gch->name, gch->pcdata->bounty, get_ratio(gch), gch->generation, gch->pcdata->status);
  }

  buf2 = box_text(buf->data, "Bountylist");
  buffer_free(buf);
  send_to_char(buf2->data, ch);
}

void do_dcredits(CHAR_DATA *ch, char *argument)
{
  do_help(ch, "dcredits");
}

void do_version(CHAR_DATA *ch, char *argument) 
{
  BUFFER *buf = buffer_new(MAX_STRING_LENGTH);
  BUFFER *buf2;

  bprintf(buf, "      This MUD is based on the Dystopia 2 codebase. The original\n\r");
  bprintf(buf, "               Dystopia 2 was called Calim's Cradle.\n\n\r");

  bprintf(buf, "   Calim's Cradle was build on a mix of GangWars and Dystopia 1.4 CE,\n\r");
  bprintf(buf, "     making it one of the more advanced GodWars MUDs out there.\n\n\r");
  bprintf(buf, "          GangWars is based on the SocketMud(tm) codebase,\n\r");

  buf2 = box_text(buf->data, "Version");
  send_to_char(buf2->data, ch);
  buffer_free(buf);
}

void do_upkeep(CHAR_DATA *ch, char *argument)
{
  BUFFER *buf;
  AFFECT_DATA *paf, *prev_paf, *next_paf;
  ITERATOR *pIter, *pIter2;
  bool found;
  int evocount = 0, sn;

  if (IS_NPC(ch)) return;

  buf = buffer_new(MAX_STRING_LENGTH);
  bprintf(buf, " %s<BR><BR>", get_dystopia_banner("Upkeep List", 68));

  bprintf(buf, "  [%s] Chaos Shield      [%s] Extra Speed       [%s] Sanctuary<BR>",
    (IS_ITEMAFF(ch, ITEMA_CHAOSSHIELD)) ? "o" : " ",
    (IS_ITEMAFF(ch, ITEMA_SPEED)) ? "o" : " ",
    (IS_AFFECTED(ch, AFF_SANCTUARY)) ? "o" : " ");
  bprintf(buf, "  [%s] Pro. vs Evil      [%s] Pro. vs Good      [%s] Flying<BR>",
    (IS_AFFECTED(ch, AFF_PROTECT)) ? "o" : " ",
    (IS_AFFECTED(ch, AFF_PROTECT_GOOD)) ? "o" : " ",
    (IS_AFFECTED(ch, AFF_FLYING)) ? "o" : " ");
  bprintf(buf, "  [%s] Passdoor          [%s] Invisibility      [%s] Sneak<BR>",
    (IS_AFFECTED(ch, AFF_PASS_DOOR)) ? "o" : " ",
    (IS_AFFECTED(ch, AFF_INVISIBLE)) ? "o" : " ",
    (IS_AFFECTED(ch, AFF_SNEAK)) ? "o" : " ");
  bprintf(buf, "  [%s] Regeneration<BR>",
    (IS_ITEMAFF(ch, ITEMA_REGENERATE)) ? "o" : " ");

  bprintf(buf, "<BR> %s<BR><BR>", get_dystopia_banner("Spell Affects", 68));

  pIter = AllocIterator(ch->affected);
  while ((paf = (AFFECT_DATA *) NextInList(pIter)) != NULL)
  {
    found = FALSE;

    pIter2 = AllocIterator(ch->affected);
    while ((prev_paf = (AFFECT_DATA *) NextInList(pIter2)) != NULL)
    {
      if (prev_paf == paf)
        break;

      if (prev_paf->type == paf->type)
      {
        found = TRUE;
        break;
      }
    }
    if (found) continue;

    bprintf(buf, "  Spell: '%s'<BR>", skill_table[paf->type].name);

    pIter2 = AllocIterator(ch->affected);
    while ((next_paf = (AFFECT_DATA *) NextInList(pIter2)) != NULL)
    {
      if (next_paf == paf)
        break;
    }

    do {
      if (next_paf->type != paf->type) continue;

      bprintf(buf, "    * modifies %s by %d for %d hour%s with bits %s.<BR>",
        affect_loc_name(next_paf->location), next_paf->modifier,
        paf->duration, (next_paf->duration != 1) ? "s" : "",
        affect_bit_name(next_paf->bitvector));
    } while ((next_paf = (AFFECT_DATA *) NextInList(pIter2)) != NULL);

  }
  if (SizeOfList(ch->affected) == 0)
  {
    bprintf(buf, "  You are not affected by any spells.<BR>");
  }

  bprintf(buf, "<BR> %s<BR><BR>", get_dystopia_banner("Class Powers", 68));
  if (IS_CLASS(ch, CLASS_FAE))
  {
    bprintf(buf, "  [%s] <SEND href=\"spiritkiss\">Spirit Kiss</SEND>     "
                 "  [%s] <SEND href=\"truesight\">Truesight</SEND>       "
                 "  [%s] <SEND href=\"blastbeams\">Blastbeams</SEND><BR>",
      (is_affected(ch, gsn_spiritkiss)) ? "o" : " ",
      (IS_SET(ch->act, PLR_HOLYLIGHT)) ? "o" : " ",
      (IS_SET(ch->pcdata->powers[FAE_BITS], FAE_BLASTBEAMS)) ? "o" : " ");
    bprintf(buf, "  [%s] <SEND href=\"vanish\">Vanish</SEND>          "
                 "  [%s] <SEND href=\"elementalform\">Elemental Form</SEND>  "
                 "  [%s] <SEND href=\"gaseous\">Gaseous</SEND><BR>",
      (IS_SET(ch->act, PLR_HIDE)) ? "o" : " ",
      (IS_SET(ch->newbits, NEW_CUBEFORM)) ? "o" : " ",
      (IS_SET(ch->pcdata->powers[FAE_BITS], FAE_GASEOUS)) ? "o" : " ");

    if (ch->pcdata->powers[DISC_FAE_ARCANE] >= 10)
    {
      if ((sn = skill_lookup("ghost gauntlets")) < 0)
        bug("do_upkeep: ghost gauntlets doesn't exist as a spell.", 0);

      bprintf(buf, "  [%s] <SEND href=\"ghostgauntlets\">Ghost Gauntlets</SEND> %s",
        (is_affected(ch, sn)) ? "o" : " ", ((++evocount) % 3 == 0) ? "<BR>" : "");
    }
    if (IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_CHAMELEON))
    {
      bprintf(buf, "  [%s] <SEND href=\"chameleon\">Chameleon Skin</SEND>  %s",
        (IS_SET(ch->newbits, NEW_CHAMELEON)) ? "o" : " ", ((++evocount) % 3 == 0) ? "<BR>" : "");
    }
    if (IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_HALO))
    {
      bprintf(buf, "  [%s] <SEND href=\"halo\">Halo</SEND>           %s",
        (IS_SET(ch->newbits, NEW_FAEHALO)) ? "o" : " ", ((++evocount) % 3 == 0) ? "<BR>" : "");
    }
    if (IS_SET(ch->pcdata->powers[EVOLVE_2], FAE_EVOLVE_PSPRAY))
    {
      bprintf(buf, "  [%s] <SEND href=\"pspray\">Prismatic Spray</SEND> %s",
        (IS_SET(ch->newbits, NEW_PSPRAY)) ? "o" : " ", ((++evocount) % 3 == 0) ? "<BR>" : "");
    }

    /* make sure we break that line */
    if (evocount > 0 && (evocount % 3) != 0)
      bprintf(buf, "<BR>");
  }
  else if (IS_CLASS(ch, CLASS_GIANT))
  {
    bprintf(buf, "  [%s] <SEND href=\"dawnstrength\">Dawnstrength</SEND>    "
                 "  [%s] <SEND href=\"truesight\">Truesight</SEND>       "
                 "  [%s] <SEND href=\"standfirm\">Standfirm</SEND><BR>",
      (IS_SET(ch->newbits, NEW_CUBEFORM)) ? "o" : " ",
      (IS_SET(ch->act, PLR_HOLYLIGHT)) ? "o" : " ",
      (ch->pcdata->powers[GIANT_STANDFIRM] == 1) ? "o" : " ");

    if (IS_SET(ch->pcdata->powers[EVOLVE_1], GIANT_EVOLVE_EARTH))
    {
      bprintf(buf, "  [%s] <SEND href=\"mudform\">Mudform</SEND>         %s",
        (IS_SET(ch->newbits, NEW_MUDFORM)) ? "o" : " ", ((++evocount) % 3 == 0) ? "<BR>" : "");
    }
    if (IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_WINDWALK))
    {
      bprintf(buf, "  [%s] <SEND href=\"windwalk\">Windwalk</SEND>        %s",
        (IS_SET(ch->extra, EXTRA_WINDWALK)) ? "o" : " ", ((++evocount) % 3 == 0) ? "<BR>" : "");
    }
    if (IS_SET(ch->pcdata->powers[EVOLVE_2], GIANT_EVOLVE_SINKHOLE))
    {
      bprintf(buf, "  [%s] <SEND href=\"sinkhole\">Sinkhole</SEND>        %s",
        (IS_SET(ch->newbits, NEW_SINKHOLE)) ? "o" : " ", ((++evocount) % 3 == 0) ? "<BR>" : "");
    }

    /* make sure we break that line */
    if (evocount > 0 && (evocount % 3) != 0)
      bprintf(buf, "<BR>");
  }
  else if (IS_CLASS(ch, CLASS_WARLOCK))
  {
    bprintf(buf, "  [%s] <SEND href=\"shattershield\">Shattershield</SEND>   "
                 "  [%s] <SEND href=\"magicvest\">Magical Vestment</SEND>"
                 "  [%s] <SEND href=\"wallofswords\">Wall of Swords</SEND><BR>",
      (IS_SET(ch->affected_by, AFF_SHATTERSHIELD)) ? "o" : " ",
      (IS_SET(ch->affected_by, AFF_MVEST)) ? "o" : " ", 
      (IS_SET(ch->affected_by, AFF_WALLSWORDS)) ? "o" : " ");
    bprintf(buf, "  [%s] <SEND href=\"displacement\">Displaced</SEND>       "
                 "  [%s] <SEND href=\"truesight\">Truesight</SEND>       "
                 "  [%s] <SEND href=\"fireshield\">Fire Shield</SEND><BR>",
      (event_isset_mobile(ch, EVENT_PLAYER_DISPLACE)) ? "o" : " ",
      (IS_SET(ch->act, PLR_HOLYLIGHT)) ? "o" : " ",
      (IS_SET(ch->affected_by, AFF_FIRESHIELD)) ? "o" : " ");
    bprintf(buf, "  [%s] <SEND href=\"steelfists\">Steel Fists</SEND>     ",
      (is_affected(ch, skill_lookup("steel fists"))) ? "o" : " ");

    /* we will use evocount to know when to add a linebreak */
    evocount = 1;

    if (IS_SET(ch->pcdata->powers[EVOLVE_1], WARLOCK_EVOLVE_MOUNTAIN))
    {
      bprintf(buf, "  [%s] <SEND href=\"mking\">Mountain King</SEND>   %s",
        (IS_SET(ch->newbits, NEW_MOUNTAIN)) ? "o" : " ", ((++evocount) % 3 == 0) ? "<BR>" : "");
    }

    if (IS_SET(ch->pcdata->powers[EVOLVE_1], WARLOCK_EVOLVE_CONTINGENCY))
    {
      bprintf(buf, "  [%s] Contingency     %s",
        (SizeOfList(ch->pcdata->contingency) > 0) ? "o" : " ", ((++evocount) % 3 == 0) ? "<BR>" : "");
    }

    if (IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_BACKLASH))
    {
      bprintf(buf, "  [%s] <SEND href=\"backlash\">Backlash</SEND>        %s",
        (IS_SET(ch->newbits, NEW_BACKLASH)) ? "o" : " ", ((++evocount) % 3 == 0) ? "<BR>" : "");
    }

    if (IS_SET(ch->pcdata->powers[EVOLVE_2], WARLOCK_EVOLVE_HUNTINGSTARS))
    {
      bprintf(buf, "  [%s] <SEND href=\"hstars\">Hunting Stars</SEND>   %s",
        (IS_SET(ch->newbits, NEW_HSTARS)) ? "o" : " ", ((++evocount) % 3 == 0) ? "<BR>" : "");
    }

    /* make sure we break that line */
    if ((evocount % 3) != 0)
      bprintf(buf, "<BR>");
  }
  else if (IS_CLASS(ch, CLASS_SHADOW))
  {
    bprintf(buf, "  [%s] Shadowform      "
                 "  [%s] <SEND href=\"truesight\">Truesight</SEND>       "
                 "  [%s] Blurred<BR>",
      (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_SHADOWFORM)) ? "o" : " ",
      (IS_SET(ch->act, PLR_HOLYLIGHT)) ? "o" : " ",
      (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLUR)) ? "o" : " ");
    bprintf(buf, "  [%s] <SEND href=\"vanish\">Vanish</SEND>          "
                 "  [%s] Knifeshield     "
                 "  [%s] Bloodrage<BR>",
      (IS_SET(ch->act, PLR_HIDE)) ? "o" : " ",
      (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_KNIFESHIELD)) ? "o" : " ",
      (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLOODRAGE)) ? "o" : " ");
    bprintf(buf, "  [%s] <SEND href=\"shield\">Shielded</SEND>        "
                 "  [%s] <SEND href=\"aurasight\">Aura Sight</SEND>      ",
      (IS_IMMUNE(ch, IMM_SHIELDED)) ? "o" : " ",
      (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_AURASIGHT)) ? "o" : " ");

    /* we will use evocount to know when to add a linebreak */
    evocount = 2;

    if (IS_SET(ch->pcdata->powers[EVOLVE_1], SHADOW_EVOLVE_SHADOWPLANE))
    {
      bprintf(buf, "  [%s] <SEND href=\"shadowplane\">Shadowplane</SEND>     %s",
        (IS_SET(ch->newbits, NEW_SHADOWPLANE)) ? "o" : " ", ((++evocount) % 3 == 0) ? "<BR>" : "");
    }

    /* make sure we break that line */
    if ((evocount % 3) != 0)
      bprintf(buf, "<BR>");
  }
  else
  {
    bprintf(buf, "  You have no class powers.<BR>");
  }

  bprintf(buf, "<BR> %s<BR>", get_dystopia_banner("", 68));

  /* send and free data */
  mxp_to_char(buf->data, ch, MXP_ALL);
  buffer_free(buf);
}

/* Mastery command to gain mastery items */

void do_mastery(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  char buf[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  
  if (IS_NPC(ch)) return;
  one_argument(argument, arg);

  if (IS_SET(ch->newbits, NEW_MASTERY) && ch->level < 6)
  {
    send_to_char("You've already gotten your mastery. If you lost it, tough luck!\n\r",ch);
    return;
  }
  if (ch->wpn[0] < 200 || ch->wpn[1] < 200 || ch->wpn[2] < 200 || ch->wpn[3] < 200 || ch->wpn[4] < 200
   || ch->wpn[5] < 200 || ch->wpn[6] < 200 || ch->wpn[7] < 200 || ch->wpn[8] < 200
   || ch->wpn[9] < 200 || ch->wpn[10] < 200 || ch->wpn[11] < 200 || ch->wpn[12] < 200 )
  {
    send_to_char("Maybe you should grandmaster your weapons first.\n\r",ch);
    return;
  }
  if (ch->spl[0] < 200 || ch->spl[1] < 200 || ch->spl[2] < 200 || ch->spl[3] < 200 || ch->spl[4] < 200 )
  {
    send_to_char("Maybe you should be grand sorcerer in all spell colors first.\n\r",ch);
    return;
  }
  if (ch->stance[1] < 200 || ch->stance[2] < 200 || ch->stance[3] < 200 || ch->stance[4] < 200
   || ch->stance[5] < 200 || ch->stance[6] < 200 || ch->stance[7] < 200 || ch->stance[8] < 200
   || ch->stance[9] < 200 || ch->stance[10] < 200 )
  {
    send_to_char("Maybe you should grandmaster your stances first.\n\r",ch);
    return;
  }

  obj              = create_object(get_obj_index(OBJ_VNUM_PROTOPLASM), 50);
  obj->questowner  = str_dup(ch->name);
  obj->ownerid     = ch->pcdata->playerid;
  obj->item_type   = ITEM_ARMOR;
  obj->weight      = 1;
  obj->value[0]    = 25;
  forge_affect(obj, 25);

  if (IS_CLASS(ch, CLASS_WARLOCK))
  {
    if (arg[0] == '\0')
    {
      send_to_char("What magic sphere do you wish to master?\n\r", ch);
      extract_obj(obj);
      return;
    }
    else if (!str_cmp(arg, "invocation"))
    {
      free_string(obj->short_descr);
      free_string(obj->name);
      free_string(obj->description);
      obj->short_descr = str_dup("the ion stone of invocation");
      obj->name        = str_dup("ion stone mastery");
      obj->description = str_dup("a glowing ion stone floats here.");
      ch->pcdata->powers[WARLOCK_PATH] = PATH_INVOCATION;
    }
    else if (!str_cmp(arg, "necromancy"))
    {
      free_string(obj->short_descr);
      free_string(obj->name);
      free_string(obj->description);
      obj->short_descr = str_dup("the ion stone of necromancy");
      obj->name        = str_dup("ion stone mastery");
      obj->description = str_dup("a glowing ion stone floats here.");
      ch->pcdata->powers[WARLOCK_PATH] = PATH_NECROMANCY;
    }
    else if (!str_cmp(arg, "abjuration"))
    {
      free_string(obj->short_descr);
      free_string(obj->name);
      free_string(obj->description);
      obj->short_descr = str_dup("the ion stone of abjuration");
      obj->name        = str_dup("ion stone mastery");
      obj->description = str_dup("a glowing ion stone floats here.");
      ch->pcdata->powers[WARLOCK_PATH] = PATH_ABJURATION;
    }
    else if (!str_cmp(arg, "divination"))
    {
      free_string(obj->short_descr);
      free_string(obj->name);
      free_string(obj->description);
      obj->short_descr = str_dup("the ion stone of divination");
      obj->name        = str_dup("ion stone mastery");
      obj->description = str_dup("a glowing ion stone floats here.");
      ch->pcdata->powers[WARLOCK_PATH] = PATH_DIVINATION;
    }
    else if (!str_cmp(arg, "enchantment"))
    {
      free_string(obj->short_descr);
      free_string(obj->name);
      free_string(obj->description);
      obj->short_descr = str_dup("the ion stone of enchantment");
      obj->name        = str_dup("ion stone mastery");
      obj->description = str_dup("a glowing ion stone floats here.");
      ch->pcdata->powers[WARLOCK_PATH] = PATH_ENCHANTMENT;
    }
    else if (!str_cmp(arg, "summoning"))
    {
      free_string(obj->short_descr);
      free_string(obj->name);
      free_string(obj->description);
      obj->short_descr = str_dup("the ion stone of summoning");
      obj->name        = str_dup("ion stone mastery");
      obj->description = str_dup("a glowing ion stone floats here.");
      ch->pcdata->powers[WARLOCK_PATH] = PATH_SUMMONING;
    }
    else
    {
      send_to_char("There is no magic sphere by that name.\n\r", ch);
      extract_obj(obj);
      return;
    }

    ch->pcdata->powers[WARLOCK_RANK] = WLCK_RNK_WARLOCK;
    update_archmage(ch);
  }
  else if (IS_CLASS(ch, CLASS_GIANT))
  {
    free_string(obj->short_descr);
    free_string(obj->name);
    free_string(obj->description);
    obj->short_descr = str_dup("the pendant of ages");
    obj->name        = str_dup("pendant mastery ages");
    obj->description = str_dup("a shiny pendant lies on the floor.");
  }
  else if (IS_CLASS(ch, CLASS_SHADOW))
  {
    free_string(obj->short_descr);
    free_string(obj->name);
    free_string(obj->description);
    obj->short_descr = str_dup("a pair of whirling blades");
    obj->name        = str_dup("whirling blades mastery");
    obj->description = str_dup("a pair of whirling blades hover in the air.");
  }
  else if (IS_CLASS(ch, CLASS_FAE))
  {
    if (arg[0] == '\0')
    {
      send_to_char("What ancient do you wish to master?\n\r", ch);
      extract_obj(obj);
      return;
    }
    else if (!str_cmp(arg, "matter"))
    {
      free_string(obj->short_descr);
      free_string(obj->name);
      free_string(obj->description);
      obj->short_descr = str_dup("a glowing matter band");
      obj->name        = str_dup("glowing band mastery");
      obj->description = str_dup("a glowing band lies on the floor.");
      ch->pcdata->powers[FAE_PATH] = FAE_MATTER;
    }
    else if (!str_cmp(arg, "energy"))
    {
      free_string(obj->short_descr);
      free_string(obj->name);
      free_string(obj->description);
      obj->short_descr = str_dup("a glowing energy band");
      obj->name        = str_dup("glowing band mastery");
      obj->description = str_dup("a glowing band lies on the floor.");
      ch->pcdata->powers[FAE_PATH] = FAE_ENERGY;
    }
    else if (!str_cmp(arg, "plasma"))
    {
      free_string(obj->short_descr);
      free_string(obj->name);
      free_string(obj->description);
      obj->short_descr = str_dup("a glowing plasma band");
      obj->name        = str_dup("glowing band mastery");
      obj->description = str_dup("a glowing band lies on the floor.");
      ch->pcdata->powers[FAE_PATH] = FAE_PLASMA;
    }
    else if (!str_cmp(arg, "will"))
    {
      free_string(obj->short_descr);
      free_string(obj->name);
      free_string(obj->description);
      obj->short_descr = str_dup("a glowing willpower band");
      obj->name        = str_dup("glowing band mastery");
      obj->description = str_dup("a glowing band lies on the floor.");
      ch->pcdata->powers[FAE_PATH] = FAE_WILL;
    }
    else
    {
      send_to_char("There is no ancient by that name.\n\r", ch);
      extract_obj(obj);
      return;
    }
  }
  else
  {
    send_to_char("Your class mastery is not done yet.\n\r", ch);
    extract_obj(obj);
    return;
  }

  obj->wear_flags  = ITEM_WEAR_MASTERY + ITEM_TAKE;
  SET_BIT(ch->newbits, NEW_MASTERY);
  SET_BIT(obj->extra_flags, ITEM_MASTERY);
  obj_to_char(obj,ch);

  sprintf(buf,"%s has achieved mastery.",ch->name);
  do_info(ch,buf);
}

void do_ragnarok( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  int amount;

  if (IS_NPC(ch)) return;

  argument = one_argument(argument, arg);

  if (!new_ragnarok)
  {
    send_to_char("The gods are tired, and will not listen to your pleas.\n\r", ch);
    return;
  }

  /* players with fighttimers shouldn't be able to use
   * this to chicken out of a fight.
   */
  if (has_timer(ch)) return;

  if (arg[0] == '\0' || !is_number(arg))
  {
    printf_to_char(ch, "The ragnarok pool is at : %d gold.\n\r", 3000 - ragnarok_cost);
    send_to_char("How many goldcrowns do you want to bid ?\n\r",ch);
    return;
  }
  amount = atoi(arg);

  if (amount < 100)
  {
    send_to_char("You cannot tempt the gods with this puny amount.\n\r",ch);
    return;
  }

  if (getGold(ch) < amount)
  {
    send_to_char("You don't have that many goldcrowns.\n\r",ch);
    return;
  }

  setGold(ch, -1 * UMIN(amount, ragnarok_cost));
  ragnarok_cost -= amount;

  if (ragnarok_cost <= 0)
  {
    EVENT_DATA *event;

    ragnarok_cost = 3000;
    ragnarok = TRUE;
    new_ragnarok = FALSE;
    do_info(ch,"#0The world comes to an end, #yRAGNAROK#0 is over us all!!!!#n");

    event        =  alloc_event();
    event->fun   =  &event_game_ragnarok;
    event->type  =  EVENT_GAME_RAGNAROK;
    add_event_world(event, 15 * 60 * PULSE_PER_SECOND);
  }
  else
  {
    do_info(ch,"The ragnarok moves closer, the gods shiver with fear");
  }
}

void do_favor( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument,arg);

  if (IS_NPC(ch)) return;

  if (arg[0] == '\0')
  {
    send_to_char("Favor left, right or both hands ?\n\r",ch);
    return;
  }
  if (!str_cmp(arg,"left"))
  {
    SET_BIT(ch->act, PLR_LEFTHAND);
    send_to_char("You favor your left arm in combat.\n\r",ch);
  }
  else if (!str_cmp(arg,"right"))
  {
    SET_BIT(ch->act, PLR_RIGHTHAND);
    send_to_char("You favor your right arm in combat.\n\r",ch);
  }
  else if (!str_cmp(arg,"both"))
  {
    SET_BIT(ch->act, PLR_AMBI);
    send_to_char("You fight well with both arms.\n\r",ch);
  }
  else do_favor(ch,"");
}

void do_birth(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *dummy;
  ACCOUNT_DATA *account;

  if (IS_NPC(ch)) return;

  argument = one_argument(argument, arg);

  if (ch->sex != SEX_FEMALE)
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }
  if (!IS_EXTRA(ch, EXTRA_PREGNANT))
  {
    send_to_char("But you are not even pregnant!\n\r", ch);   
    return;
  }
  if (!IS_EXTRA(ch, EXTRA_LABOUR))
  {
    send_to_char("You're not ready to give birth yet.\n\r", ch);
    return;
  }
  if (arg[0] == '\0')
  {
    if (ch->pcdata->genes[4] == SEX_MALE)
      send_to_char("What do you wish to name your little boy?\n\r", ch);
    else if (ch->pcdata->genes[4] == SEX_FEMALE)
      send_to_char("What do you wish to name your little girl?\n\r", ch);
    else
      send_to_char("What do you wish to name your child?\n\r", ch);
    return;
  }
  if (!check_parse_name(arg, TRUE))
  {
    send_to_char("Thats an illegal name.\n\r", ch);
    return;
  }
  if (char_exists(arg))
  {
    send_to_char("That player already exists.\n\r", ch);
    return;
  }

  /* make sure we have an live connection and a fitting account */
  if (ch->desc == NULL || (account = load_account(ch->pcdata->account)) == NULL)
  {
    send_to_char("You have encountered a bug, please report this.\n\r", ch);
    return;
  }

  if ((dummy = (DESCRIPTOR_DATA *) PopStack(descriptor_free)) == NULL)
  {
    dummy = calloc(1, sizeof(*dummy));
  }

  /* make sure it's added to the right account */
  dummy->account = account;

  arg[0] = UPPER(arg[0]);
  load_char_obj(dummy, arg);
  victim = dummy->character;

  /* set the statistics for the child */
  victim->pcdata->perm_str = 18;
  victim->pcdata->perm_int = 18;
  victim->pcdata->perm_wis = 18;
  victim->pcdata->perm_dex = 18;
  victim->pcdata->perm_con = 18;
  victim->max_hit          = 4000;
  victim->hit              = 4000;
  victim->max_mana         = 4000;
  victim->mana             = 4000;
  victim->max_move         = 4000;
  victim->move             = 4000;
  victim->sex              = ch->pcdata->genes[4];
  victim->level            = 2;
  victim->in_room          = ch->in_room;

  /* save this account, and force a reload on the original */
  account_new_player(dummy->account, victim);
  save_account(dummy->account);

  /* save and free the child */
  save_char_obj(victim);
  free_char(victim);

  /* Put the descriptor back into the free list */
  PushStack(dummy, descriptor_free);

  ch->pcdata->genes[9]++;  
  REMOVE_BIT(ch->extra, EXTRA_PREGNANT);
  REMOVE_BIT(ch->extra, EXTRA_LABOUR);
  free_string(ch->pcdata->conception);
  ch->pcdata->conception = str_dup("");
  save_char_obj(ch);

  if (ch->pcdata->genes[4] == SEX_MALE)
  {
    sprintf(buf, "%s gives birth to %s's son, named %s!",
      ch->name, ch->pcdata->cparents, arg);
  }
  else
  {
    sprintf(buf, "%s gives birth to %s's daughter, named %s!",
      ch->name, ch->pcdata->cparents, arg);
  }

  do_info(ch, buf);
  send_to_char("Pop!\n\r", ch);
}

void do_leader(CHAR_DATA *ch, char *argument)
{
  BUFFER *leaderlist;
  
  leaderlist = get_leader();
  if (leaderlist != NULL)
  {
    send_to_char(leaderlist->data, ch);
    buffer_free(leaderlist);
  }
  else
  {
    send_to_char("Something went wrong. please report this.\n\r", ch);
  }
}

BUFFER *get_leader()
{
  BUFFER *buf = buffer_new(MAX_STRING_LENGTH);
  BUFFER *buf2;

  bprintf(buf, "   #RMost Player Kills     #C--->   #G%-13s #Rwith #G%d #RKills#n\n\r",
    leader_board.name[LEADER_PK], leader_board.number[LEADER_PK]);
  
  bprintf(buf, "   #RMost Hours Played     #C--->   #G%-13s #Rwith #G%d #RHours#n\n\r",
    leader_board.name[LEADER_TIME], leader_board.number[LEADER_TIME]);

  bprintf(buf, "   #RMost Active Player    #C--->   #G%-13s #Rwith #G%d.%d #R%%#n\n\r",
    leader_board.name[LEADER_ACTIVE], leader_board.number[LEADER_ACTIVE] / 10,
    leader_board.number[LEADER_ACTIVE] % 10);

  bprintf(buf, "   #RMost Quest Completed  #C--->   #G%-13s #Rwith #G%d #RQuests#n\n\r",
    leader_board.name[LEADER_QUEST], leader_board.number[LEADER_QUEST]);

  bprintf(buf, "   #RMost Mobs Killed      #C--->   #G%-13s #Rwith #G%d #RKills#n\n\r",
    leader_board.name[LEADER_MOBKILLS], leader_board.number[LEADER_MOBKILLS]);

  bprintf(buf, "   #RHighest PK score      #C--->   #G%-13s #Rwith #G%d #RPoints#n\n\r",
    leader_board.name[LEADER_PKSCORE], leader_board.number[LEADER_PKSCORE]);

  bprintf(buf, "   #RMost Status           #C--->   #G%-13s #Rwith #G%d #RStatus#n\n\r",
    leader_board.name[LEADER_STATUS], leader_board.number[LEADER_STATUS]);

  bprintf(buf, "   #RMost Arena Wins       #C--->   #G%-13s #Rwith #G%d #RWins#n\n\r",
    leader_board.name[LEADER_ARENA], leader_board.number[LEADER_ARENA]);

  bprintf(buf, "   #RRichest Kingdom       #C--->   #G%-13s #Rwith #G%d #RGold#n\n\r",
    leader_board.name[LEADER_KINGDOM], leader_board.number[LEADER_KINGDOM]);

  buf2 = box_text(buf->data, "Leader Board");
  buffer_clear(buf);
  bprintf(buf, "%s", buf2->data);

  return buf;
}

void do_policy(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch) || !ch->desc) return;

  argument = one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    do_help(ch, "policy");
    return;
  }

  if (IS_SET(ch->pcdata->jflags, JFLAG_POLICY))
  {
    send_to_char("You have already accepted the policy.\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "accept"))
  {
    if (strcmp(crypt(argument, ch->desc->account->owner), ch->desc->account->password))
    {
      send_to_char("Illegal password.\n\r", ch);  
      WAIT_STATE(ch, 12);
      return;
    }
    send_to_char("You have accepted the player policy at Calim's Cradle.\n\r", ch);
    SET_BIT(ch->pcdata->jflags, JFLAG_POLICY);
    log_string("%s has accepted the player policy.", ch->name);
  }
  else if (!str_cmp(arg, "decline"))
  {
    if (strcmp(crypt(argument, ch->desc->account->owner), ch->desc->account->password))
    {
      send_to_char("Illegal password.\n\r", ch);
      WAIT_STATE(ch, 12);
      return;
    }
    log_string("%s has declined the player policy.", ch->name);

    ch->fight_timer = 0;
    char_from_room(ch);
    char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO), TRUE);
    do_quit(ch, "");
  }
  else
  {
    send_to_char("Syntax : policy [accept|decline] [password]\n\r", ch);
  }
}

void do_vote(CHAR_DATA *ch, char *argument)
{
  POLL_DATA *poll;
  ITERATOR *pIter;
  VOTE_DATA *vote;
  char *strtime;  
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf2[20];
  int i, choice;

  if (IS_NPC(ch) || !ch->desc) return;

  argument = one_argument(argument, arg1);
  one_argument(argument, arg2);
  
  if (arg1[0] == '\0' || arg2[0] == '\0')
  {
    send_to_char("      #R[#G***#R] #CCurrent Polls at Calim's Cradle #R[#G***#R]#n\n\r", ch);

    pIter = AllocIterator(poll_list);
    while ((poll = (POLL_DATA *) NextInList(pIter)) != NULL)
    {
      strtime = ctime(&poll->expire);

      for (i = 0; i < 6; i++)
        buf2[i] = strtime[i + 4];
      buf2[6] = '\0';

      printf_to_char(ch, "\n\r  A poll on #y%s#n with the following options. (expires %s)\n\n\r", poll->name, buf2);
      printf_to_char(ch, "  * %s\n\n\r", line_indent(poll->description, 4, 76));

      for (i = 0; i < MAX_VOTE_OPTIONS; i++)
      {
        if (!str_cmp("<null>", poll->options[i]))
          continue;

        printf_to_char(ch, " #0[#R%2d#0] #y%s\n\r", i+1, poll->options[i]);
      }
    }
    send_to_char("\n\r  #CPlease read HELP VOTE for details on voting.#n\n\r", ch);
    return;
  }

  if (ch->desc->account->max_might < RANK_CADET)
  {
    send_to_char("You need at least one character of cadet rank or better to vote.\n\r", ch);
    return;
  }

  pIter = AllocIterator(poll_list);
  while ((poll = (POLL_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(arg1, poll->name))
      break;
  }

  if (poll == NULL)
  {
    send_to_char("There is no poll active by that name.\n\r", ch);
    return;
  }

  pIter = AllocIterator(poll->votes);
  while ((vote = (VOTE_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(ch->desc->account->owner, vote->pname))
    {
      send_to_char("You have already voted on this poll.\n\r", ch);
      return;
    }
  }

  /* count amount of valid fields in poll */
  for (i = 0; i < MAX_VOTE_OPTIONS; i++)
  {  
    if (!str_cmp(poll->options[i], "<null>"))
      break;
  }

  if ((choice = atoi(arg2)) < 1 || choice > i)   
  {
    printf_to_char(ch, "You can only vote on 1 to %d.\n\r", i + 1);
    return;
  }

  add_vote(poll, ch, choice);
  send_to_char("You vote has been accepted.\n\r", ch);
  save_subvotes(poll);
  save_polls();
}

void add_vote(POLL_DATA *poll, CHAR_DATA *ch, int choice)
{
  VOTE_DATA *vote;

  if (!ch->desc || !ch->desc->account)
    return;

  poll->vcount[choice-1]++;

  vote = malloc(sizeof(*vote));
  vote->pname = str_dup(ch->desc->account->owner);
  vote->phost = str_dup(HOSTNAME(ch->desc));
  vote->choice = choice;

  AttachToList(vote, poll->votes);
}

void do_changes(CHAR_DATA *ch, char *argument)
{
  CHANGE_DATA *change;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];
  char tempbuf[MAX_STRING_LENGTH];
  bool found = FALSE;
  int i = 0;

  if (IS_NPC(ch)) return;

  sprintf(buf, "%11s#0[#C***#0] #GThe latest changes made to Calim's Cradle #0[#C***#0]#n\n\r\n\r", "");

  pIter = AllocIterator(change_list);
  while ((change = (CHANGE_DATA *) NextInList(pIter)) != NULL)
  {
    found = TRUE;
    ++i;
    sprintf(tempbuf, " #0[#C%3d#0] #G%-8s #C%s#n\n\r",
      i, change->date, line_indent(change->text, 16, 79));
    strcat(buf, tempbuf);
  }

  if (found)
    send_to_char(buf, ch);
  else
    send_to_char("No changes.\n\r", ch);
}

void do_score(CHAR_DATA *ch, char *argument)
{
  BUFFER *buf, *buf2;
  char tempbuf[MAX_STRING_LENGTH];
  char col_str[MAX_STRING_LENGTH];
  char *seperator = "#o-:|:-~-:|:-~-:|:-~-:|:-~-:|:--:|:-~-:|:-~-:|:-~-:|:-~-:|:-~-:|:-#n";
  int nextRank = next_rank_in(ch);

  if (IS_NPC(ch))
    return;

  buf = buffer_new(MAX_STRING_LENGTH);

  bprintf(buf, "  Your name is #C%s #nand you have been playing for #C%d #nhour%s\n\r",
    ch->name, (ch->played + (int) (current_time - ch->logon))/3600,
    ((ch->played + (int) (current_time - ch->logon))/3600 == 1) ? "" : "s");
  bprintf(buf, "%s", birth_date(ch, FALSE));

  sprintf(tempbuf, "  STR: #G%2d#n    Hitpoints : %s/#C%d#n",
    get_curr_str(ch), col_scale(ch->hit, ch->max_hit), ch->max_hit);
  cprintf(col_str, "%-44s #uLast two kills#n\n\r", tempbuf);
  bprintf(buf, "%s", col_str);

  sprintf(tempbuf, "  DEX: #G%2d#n    Movement  : %s/#C%d#n",
    get_curr_dex(ch), col_scale(ch->move, ch->max_move), ch->max_move);
  cprintf(col_str, "%-44s #C%s#n\n\r", tempbuf, ch->pcdata->last_decap[0]);
  bprintf(buf, "%s", col_str);

  sprintf(tempbuf, "  CON: #G%2d#n    Magics    : %s/#C%d#n",
    get_curr_con(ch), col_scale(ch->mana, ch->max_mana), ch->max_mana);
  cprintf(col_str, "%-44s #C%s#n\n\r", tempbuf, ch->pcdata->last_decap[1]);
  bprintf(buf, "%s", col_str);

  sprintf(tempbuf, "  WIS: #G%2d#n    PKscore   : #y%d#n",
    get_curr_wis(ch), get_ratio(ch));
  cprintf(col_str, "%-44s #uRetaliation#n\n\r", tempbuf);
  bprintf(buf, "%s", col_str);

  sprintf(tempbuf, "  INT: #G%2d#n    Gold      : #y%d#n/#C%d#n",
    get_curr_int(ch), getGold(ch), getGoldTotal(ch));
  cprintf(col_str, "%-44s #C%s#n\n\r", tempbuf, ch->pcdata->retaliation[0]);
  bprintf(buf, "%s", col_str);

  sprintf(tempbuf, "             Quests    : #y%d#n", ch->pcdata->questsrun);
  bprintf(buf, "%-48s #C%s#n\n\r", tempbuf,  ch->pcdata->retaliation[1]);

  bprintf(buf, "\n\r %s\n\n\r", seperator);

  bprintf(buf, "  Damroll : #C%4d#n    Damcap : #C%5d#n    Saving Throw : #C%d#n (%s%d)\n\r",
    char_damroll(ch), ch->damcap[0],
    (ch->spl[0] + ch->spl[1] + ch->spl[2] + ch->spl[3] + ch->spl[4]) / 20,
    (ch->saving_throw >= 0) ? "+" : "", ch->saving_throw);

  bprintf(buf, "  Hitroll : #C%4d#n    Armor  : #C%5d#n    Paradox Tick : #C%d#n\n\r",
    char_hitroll(ch), char_ac(ch), ch->pcdata->mean_paradox_counter);

  bprintf(buf, "  Primal  : #C%4d#n    Status : #C%5d#n    Experience   : #C%d#n\n\n\r",
    ch->practice, ch->pcdata->status, ch->exp);

  bprintf(buf, "  You are carrying #C%d#n/#C%d#n items with weight #C%d#n/#C%d#n lbs\n\r",
    ch->carry_number, can_carry_n(ch), ch->carry_weight, can_carry_w(ch));

  /* Class stuff */
  if (IS_CLASS(ch, CLASS_SHADOW))
  {
    bprintf(buf, "\n\r %s\n\n\r", seperator);
    bprintf(buf, "  Shadow Points  :  #C%d#n      Shadow Upkeep  :  #C%d#n seconds\n\r",
      ch->pcdata->powers[SHADOW_POWER], (IS_SET(ch->newbits, NEW_SUPKEEP2)) ? 75 : ((IS_SET(ch->newbits, NEW_SUPKEEP1) ? 50 : 25)));
  }
  else if (IS_CLASS(ch, CLASS_GIANT))
  {
    bprintf(buf, "\n\r %s\n\n\r", seperator);
    bprintf(buf, "  Giant Size  :  #C%d#n Feet      Giant Points  :  #C%d#n\n\r",
      (ch->pcdata->powers[GIANT_RANK] + 1) * 5, ch->pcdata->powers[GIANT_POINTS]);
  }
  else if (IS_CLASS(ch, CLASS_FAE))
  {
    ;
  }
  else if (IS_CLASS(ch, CLASS_WARLOCK))
  {
    EVENT_DATA *event;

    if ((event = event_isset_mobile(ch, EVENT_PLAYER_STUDY)) != NULL)
    {
      int cost;
      char *earg;
      char arg1[MAX_INPUT_LENGTH];
      char arg2[MAX_INPUT_LENGTH];

      earg = one_argument(event->argument, arg1);
      one_argument(earg, arg2);
      cost = atoi(arg2);

      bprintf(buf, "\n\r %s\n\n\r", seperator);
      bprintf(buf, "  You are studying #C%s#n and need #C%d#n exp in #C%d#n min\n\r",
        arg1, cost, (event_pulses_left(event) / (60 * PULSE_PER_SECOND) + 1));
    }
  }

  bprintf(buf, "\n\r %s\n\n\r", seperator);

  if (nextRank != -1)
  {
    int i;

    bprintf(buf, "  Rank     :  #0[#C");
    for (i = 0; i < nextRank / 10; i++)
      bprintf(buf, "*");
    bprintf(buf, "#n");
    for ( ; i < 10; i++)
      bprintf(buf, "*");
    bprintf(buf, "#0]#n\n\r");
  }

  sprintf(tempbuf, "  Players  : #C%6d#n %s and #C%3d#n %s",
    ch->pkill, (ch->pkill == 1) ? "kill " : "kills",
    ch->pdeath, (ch->pdeath == 1) ? "death " : "deaths");
  cprintf(col_str, "%-42s Ratio #C%3d#n%s\n\r",
    tempbuf, calc_ratio(ch->pkill, ch->pdeath), "%");
  bprintf(buf, "%s", col_str);

  sprintf(tempbuf, "  Arena    : #C%6d#n %s and #C%3d#n %s",
    ch->pcdata->awins, (ch->pcdata->awins == 1) ? "kill " : "kills",
    ch->pcdata->alosses, (ch->pcdata->alosses == 1) ? "death " : "deaths");
  cprintf(col_str, "%-42s Ratio #C%3d#n%s\n\r",
    tempbuf, calc_ratio(ch->pcdata->awins, ch->pcdata->alosses), "%");
  bprintf(buf, "%s", col_str);

  bprintf(buf, "  Monsters : #C%6d#n %s and #C%3d#n %s\n\r",
    ch->mkill, (ch->mkill == 1) ? "kill " : "kills",
    ch->mdeath, (ch->mdeath == 1) ? "death " : "deaths");

  buf2 = box_text(buf->data, "Calim's Cradle");
  send_to_char(buf2->data, ch);
  buffer_free(buf);
}

bool event_room_extra_action(EVENT_DATA *event)
{
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;
  ROOM_INDEX_DATA *pRoom, *location;
  ITERATOR *pIter, *pIter2;
  CHAR_DATA *ch, *vch;
  EXTRA_DESCR_DATA *ed;
  char name[MAX_INPUT_LENGTH];
  char *ptr;
  int sn;

  if ((pRoom = event->owner.room) == NULL)
  {
    bug("event_room_extra_action: no owner.", 0);
    return FALSE;
  }

  ptr = one_argument(event->argument, name);

  /* ch is either NULL or a pointer to the action initiator */
  pIter = AllocIterator(pRoom->people);
  while ((ch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(name, ch->name))
      break;
  }

  pIter = AllocIterator(pRoom->extra_descr);
  while ((ed = (EXTRA_DESCR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(ptr, ed->keyword))
    {
      switch(ed->action)
      {
        default:
          break;
        case ED_ACTION_TELEPORT:
          if (ed->vnum > 0)
          {
            if ((location = get_room_index(ed->vnum)) == NULL)
              return FALSE;

            if (ch != NULL)
            {
              act(line_indent(ed->buffer1, 0, 79), ch, NULL, NULL, TO_CHAR);
              act(line_indent(ed->buffer2, 0, 79), ch, NULL, NULL, TO_ROOM);
              char_from_room(ch);
              char_to_room(ch, location, TRUE);
              do_look(ch, "auto");
            }
          }
          else
          {
            location = get_rand_room();

            pIter2 = AllocIterator(pRoom->people);
            while ((ch = (CHAR_DATA *) NextInList(pIter2)) != NULL)
            {
              if (IS_NPC(ch))
                continue;

              act(line_indent(ed->buffer1, 0, 79), ch, NULL, NULL, TO_CHAR);
              act(line_indent(ed->buffer2, 0, 79), ch, NULL, NULL, TO_ROOM);
              char_from_room(ch);
              char_to_room(ch, location, TRUE);
              do_look(ch, "auto");
            }
          }
          break;
        case ED_ACTION_OBJECT:
          if ((pObjIndex = get_obj_index(ed->vnum)) == NULL)
            return FALSE;

          obj = create_object(pObjIndex, 50);

          if (CAN_WEAR(obj, ITEM_TAKE) && ch != NULL && ch->in_room == pRoom)
            obj_to_char(obj, ch);
          else
            obj_to_room(obj, pRoom);

          if (ch != NULL)
          {
            act(line_indent(ed->buffer1, 0, 79), ch, NULL, NULL, TO_CHAR);
            act(line_indent(ed->buffer2, 0, 79), ch, NULL, NULL, TO_ROOM);
          }

          break;;
        case ED_ACTION_SPELL:
          if ((sn = skill_lookup(ed->buffer1)) < 0)
            return FALSE;

          if (ch != NULL)
            (*skill_table[sn].spell_fun) (sn, number_range(40,50), ch, ch);

          break;
        case ED_ACTION_ELEVATOR:
          if ((location = get_room_index(ed->vnum)) == NULL)
            return FALSE;

          if (FirstInList(location->people) != NULL && (vch = (CHAR_DATA *) FirstInList(pRoom->people)) != NULL)
          {
            act(line_indent(ed->buffer2, 0, 79), vch, NULL, NULL, TO_ALL);
            return FALSE;
          }

          pIter2 = AllocIterator(pRoom->people);
          while ((ch = (CHAR_DATA *) NextInList(pIter2)) != NULL)
          {
            char_from_room(ch);
            char_to_room(ch, location, TRUE);
            act(line_indent(ed->buffer1, 0, 79), ch, NULL, NULL, TO_CHAR);
          }
          break;
      }

      return FALSE;
    }
  }

  return FALSE;
}

void extra_action(CHAR_DATA *ch, char *argument, int type)
{
  EVENT_DATA *event;
  EXTRA_DESCR_DATA *ed;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  one_argument(argument, arg);
 
  pIter = AllocIterator(ch->in_room->extra_descr);
  while ((ed = (EXTRA_DESCR_DATA *) NextInList(pIter)) != NULL)
  {
    if (is_full_name(arg, ed->keyword))
    {
      if (ed->type == type)
      {
        switch(type)
        {
          case ED_TYPE_PULL:
            sprintf(buf1, "You pull %s.\n\r", ed->keyword);
            sprintf(buf2, "$n pulls %s.", ed->keyword);
            break;
          case ED_TYPE_PRESS:
            sprintf(buf1, "You press %s.\n\r", ed->keyword);
            sprintf(buf2, "$n presses %s.", ed->keyword);
            break;
          case ED_TYPE_PUSH:
            sprintf(buf1, "You push %s.\n\r", ed->keyword);
            sprintf(buf2, "$n pushes %s.", ed->keyword);
            break;
          case ED_TYPE_TOUCH:
            sprintf(buf1, "You touch %s.\n\r", ed->keyword);
            sprintf(buf2, "$n touches %s.", ed->keyword);
            break;
        }
        send_to_char(buf1, ch);
        act(buf2, ch, NULL, NULL, TO_ROOM);
      }
      else
      {
        switch(type)
        {
          case ED_TYPE_PULL:
            send_to_char("That cannot be pulled.\n\r", ch);
            break;
          case ED_TYPE_PUSH:
            send_to_char("That cannot be pushed.\n\r", ch);
            break;
          case ED_TYPE_PRESS:
            send_to_char("That cannot be pressed.\n\r", ch);
            break;
          case ED_TYPE_TOUCH:
            send_to_char("You seem unable to touch it.\n\r", ch);
            break;
        }
        return;
      }
      switch(ed->action)  
      {
        default:
          send_to_char("Nothing happens.\n\r", ch);
          return;
        case ED_ACTION_TELEPORT:
        case ED_ACTION_OBJECT:
        case ED_ACTION_SPELL:
        case ED_ACTION_ELEVATOR:
          sprintf(buf1, "%s %s", ch->name, ed->keyword);
          event = alloc_event();
          event->fun = &event_room_extra_action;
          event->type = EVENT_ROOM_EXTRA_ACTION;
          event->argument = str_dup(buf1);
          add_event_room(event, ch->in_room, PULSE_PER_SECOND + 1);
          return;
      }
    }
  }

  send_to_char("You are unable to find that.\n\r", ch);
}

void do_pull(CHAR_DATA *ch, char *argument)
{
  extra_action(ch, argument, ED_TYPE_PULL);
}

void do_press(CHAR_DATA *ch, char *argument)
{
  extra_action(ch, argument, ED_TYPE_PRESS);
}

void do_push(CHAR_DATA *ch, char *argument)
{
  extra_action(ch, argument, ED_TYPE_PUSH);
}

void do_touch(CHAR_DATA *ch, char *argument)
{
  extra_action(ch, argument, ED_TYPE_TOUCH);
}

void do_ignore(CHAR_DATA *ch, char *argument)
{
  IGNORE_DATA *ignore;
  CHAR_DATA *victim;
  ITERATOR *pIter, *pIter2;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);  
  if (arg[0] == '\0')
  {
    bool found = FALSE;
    send_to_char("You are currently ignoring :\n\n\r", ch);
 
    pIter = AllocIterator(ch->pcdata->ignores);
    while ((ignore = (IGNORE_DATA *) NextInList(pIter)) != NULL)
    {
      printf_to_char(ch, " [o] All players in %s's account.\n\r", ignore->player);
      found = TRUE;
    }

    if (!found)
      send_to_char("Noone.\n\r", ch);
    else
      send_to_char("\n\rType 'ignore clear' to remove all ignores.\n\r", ch);

    return;
  }

  if (!str_cmp(arg, "clear"))
  {
    pIter = AllocIterator(ch->pcdata->ignores);
    while ((ignore = (IGNORE_DATA *) NextInList(pIter)) != NULL)
      free_ignore(ignore, ch);

    send_to_char("All ignores cleared.\n\r", ch);
    return;
  }

  pIter = AllocIterator(char_list);
  while ((victim = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (IS_NPC(victim))
      continue;

    if (!str_prefix(victim->name, arg))
    {
      if (victim->desc == NULL || victim->desc->account == NULL)
      {
        send_to_char("You cannot ignore linkdead characters.\n\r", ch);
        return;
      }

      /* stop ignoring? */
      pIter2 = AllocIterator(ch->pcdata->ignores);
      while ((ignore = (IGNORE_DATA *) NextInList(pIter2)) != NULL)
      {
        if (!str_cmp(ignore->account, victim->desc->account->owner))
        {
          free_ignore(ignore, ch);

          printf_to_char(ch, "You stop ignoring %s.\n\r", victim->name);
          return;
        }
      }

      /* add this to the ignore list */
      printf_to_char(ch, "You now ignore %s.\n\r", victim->name);

      /* allocate and attach */
      ignore = alloc_ignore();
      ignore->player = str_dup(victim->name);
      ignore->account = str_dup(victim->desc->account->owner);

      AttachToList(ignore, ch->pcdata->ignores);
      return;
    }
  }

  send_to_char("Ignore/unignore whom?\n\r", ch);
}

void do_setdecap(CHAR_DATA *ch, char *argument)
{
  int cost = 10;
  
  if (IS_NPC(ch)) return;
  if (IS_SET(ch->pcdata->jflags, JFLAG_NOSET))
  {
    send_to_char("Your not allowed to use custom settings.\n\r", ch);
    return;
  }

  if (getMight(ch) < RANK_SUPREME)
  {
    send_to_char("You need supreme rank to use this command.\n\r", ch);
    return;
  }

  if (getGold(ch) < cost)
  {
    printf_to_char(ch, "You need %d more gold to use this command.\n\r", cost - getGold(ch));
    return;
  }

  smash_tilde(argument);
  if (collen(argument) < 10 || collen(argument) > 90)
  {
    send_to_char("Your custom message should be no less than 10 characters and no more than 90.\n\r",ch);
    return;
  }

  if (!is_contained2("$n", argument) || !is_contained2("$N", argument))
  {
    send_to_char("You really need to include both $n and $N in the message.\n\r",ch);
    return;
  }
  if (!IS_SET(ch->pcdata->jflags, JFLAG_SETDECAP)) SET_BIT(ch->pcdata->jflags, JFLAG_SETDECAP);
  free_string(ch->pcdata->decapmessage);
  ch->pcdata->decapmessage = str_dup(argument);
  setGold(ch, -1 * cost);
  send_to_char("done.\n\r",ch);
}
 
void do_settie(CHAR_DATA *ch, char *argument)
{
  int cost = 10;
  
  if (IS_NPC(ch)) return;
  if (IS_SET(ch->pcdata->jflags, JFLAG_NOSET))
  {
    send_to_char("Your not allowed to use custom settings.\n\r", ch);
    return;
  }

  if (getMight(ch) < RANK_PRIVATE)
  {
    send_to_char("You need private rank to use this command.\n\r", ch);
    return;
  }

  if (getGold(ch) < cost)
  {
    printf_to_char(ch, "You need %d more gold to use this command.\n\r", cost - getGold(ch));      
    return;
  }

  smash_tilde(argument);

  if (collen(argument) < 10 || collen(argument) > 90)
  {
    send_to_char("Your custom message should be no less than 10 characters and no more than 90.\n\r",ch);
    return;
  }

  if (!is_contained2("$n", argument) || !is_contained2("$N", argument))
  {
    send_to_char("You really need to include both $n and $N in the message.\n\r",ch);
    return;
  }

  if (!IS_SET(ch->pcdata->jflags, JFLAG_SETTIE)) SET_BIT(ch->pcdata->jflags, JFLAG_SETTIE);
  free_string(ch->pcdata->tiemessage);
  ch->pcdata->tiemessage = str_dup(argument);
  setGold(ch, -1 * cost); 
  send_to_char("done.\n\r",ch);
}
 
void do_setlogout(CHAR_DATA *ch, char *argument)
{
  int cost = 10;
  
  if (IS_NPC(ch)) return;
  if (IS_SET(ch->pcdata->jflags, JFLAG_NOSET))
  {
    send_to_char("Your not allowed to use custom settings.\n\r", ch);
    return;
  }

  if (getMight(ch) < RANK_MASTER)
  {
    send_to_char("You need master rank to use this command.\n\r", ch);
    return;
  }

  if (getGold(ch) < cost)
  {
    printf_to_char(ch, "You need %d more gold to use this command.\n\r", cost - getGold(ch));      
    return;
  }

  smash_tilde(argument);

  if (collen(argument) < 10 || collen(argument) > 90)
  {
    send_to_char("Your custom message should be no less than 10 characters and no more than 90.\n\r",ch);
    return;
  }

  if (!is_contained2("$n", argument))
  {
    send_to_char("You really need to include $n in the message.\n\r",ch);
    return;
  }
  if (!IS_SET(ch->pcdata->jflags, JFLAG_SETLOGOUT)) SET_BIT(ch->pcdata->jflags, JFLAG_SETLOGOUT);
  free_string(ch->pcdata->logoutmessage);
  ch->pcdata->logoutmessage = str_dup(argument);
  setGold(ch, -1 * cost);
  send_to_char("done.\n\r",ch);
}
 
void do_setlogin(CHAR_DATA *ch, char *argument)
{
  int cost = 10;
  
  if (IS_NPC(ch)) return;
  if (IS_SET(ch->pcdata->jflags, JFLAG_NOSET))
  {
    send_to_char("Your not allowed to use custom settings.\n\r", ch);
    return;
  }

  if (getMight(ch) < RANK_ADVENTURER)
  {
    send_to_char("You need adventurer rank to use this command.\n\r", ch);
    return;
  }

  if (getGold(ch) < cost)
  {
    printf_to_char(ch, "You need %d more gold to use this command.\n\r", cost - getGold(ch));      
    return;
  }

  smash_tilde(argument);

  if (collen(argument) < 10 || collen(argument) > 90)
  {
    send_to_char("Your custom message should be no less than 10 characters and no more than 90.\n\r",ch);
    return;
  }

  if (!is_contained2("$n", argument))
  {
    send_to_char("You really need to include $n in the message.\n\r",ch);
    return;
  }

  if (!IS_SET(ch->pcdata->jflags, JFLAG_SETLOGIN)) SET_BIT(ch->pcdata->jflags, JFLAG_SETLOGIN);
  free_string(ch->pcdata->loginmessage);
  ch->pcdata->loginmessage = str_dup(argument);
  setGold(ch, -1 * cost);
  send_to_char("done.\n\r",ch);
}
 
void do_setavatar(CHAR_DATA *ch, char *argument)
{
  int cost = 10;
  
  if (IS_NPC(ch)) return;
  if (IS_SET(ch->pcdata->jflags, JFLAG_NOSET))
  {
    send_to_char("Your not allowed to use custom settings.\n\r", ch);
    return;
  }

  if (getMight(ch) < RANK_DUKE)
  {
    send_to_char("You need duke rank to use this command.\n\r", ch);
    return;
  }

  if (getGold(ch) < cost)
  {
    printf_to_char(ch, "You need %d more gold to use this command.\n\r", cost - getGold(ch));      
    return;
  }

  smash_tilde(argument);
  if (collen(argument) < 10 || collen(argument) > 90)
  {
    send_to_char("Your custom message should be no less than 10 characters and no more than 90.\n\r",ch);
    return;
  }

  if (!is_contained2("$n", argument))
  {
    send_to_char("You really need to include $n in the message.\n\r",ch);
    return;
  }

  if (!IS_SET(ch->pcdata->jflags, JFLAG_SETAVATAR)) SET_BIT(ch->pcdata->jflags, JFLAG_SETAVATAR);
  free_string(ch->pcdata->avatarmessage);
  ch->pcdata->avatarmessage = str_dup(argument);
  setGold(ch, -1 * cost);
  send_to_char("done.\n\r",ch);
}

void do_alias(CHAR_DATA *dMob, char *arg)
{
  char name[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  ITERATOR *pIter;
  ALIAS_DATA *alias;
  int i = 0, count = 0;

  arg = one_argument(arg, name);
  smash_tilde(arg);
  smash_tilde(name);

  if (name[0] == '\0')
  {
    int cln = 0;

    send_to_char("#uYour aliases are#n\n\r", dMob);

    /* list all aliases, then do the syntax thing */
    pIter = AllocIterator(dMob->pcdata->aliases);
    while ((alias = (ALIAS_DATA *) NextInList(pIter)) != NULL)
    {
      sprintf(buf, "%-18.17s %s", alias->name, (++cln % 4 == 0) ? "\n\r" : "");
      send_to_char(buf, dMob);
    }
    send_to_char("\n\r", dMob);
    if (cln % 4)
      send_to_char("\n\r", dMob);

    send_to_char("Syntax: alias <keyword> <string>\n\rRead HELP ALIAS for more details.\n\r", dMob);
    return;
  }
  if (arg[0] == '\0')
  {
    pIter = AllocIterator(dMob->pcdata->aliases);
    while ((alias = (ALIAS_DATA *) NextInList(pIter)) != NULL)
    {
      if (!str_cmp(alias->name, name))
      {
        sprintf(buf, "#u%s#n\n\r%s", alias->name, alias->expand_string);
        send_to_char(buf, dMob);
        return;
      }
    }
    sprintf(buf, "What do you want to alias '%s' to?\n\r", name);
    send_to_char(buf, dMob);
    return;
  }

  if (SizeOfList(dMob->pcdata->aliases) >= MAX_ALIAS)
  {
    send_to_char("You are not allowed to create any more aliases.\n\r", dMob);
    send_to_char("To create a new alias, you must delete an old alias first.\n\r", dMob);
    return;
  }

  pIter = AllocIterator(dMob->pcdata->aliases);
  while ((alias = (ALIAS_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(name, alias->name))
    {
      send_to_char("You already have an alias with that name.\n\r", dMob);
      return;
    }
  }
  if (is_full_name(name, "alias delalias"))
  {
    send_to_char("Please don't alias the alias commands.\n\r", dMob);
    return;
  }
  if (strlen(arg) > 400)
  {
    send_to_char("That alias is way to big, please behave.\n\r", dMob);
    return;
  }
  while (arg[i] != '\0')
  {
    if (arg[i] == '@')
      count++;
    i++;
  }
  if (count > 2)
  {
    send_to_char("Please control your use of the @ character.\n\r", dMob);
    return;
  }
  alias = alloc_alias();
  alias->name = str_dup(name);
  alias->expand_string = str_dup(create_alias_string(arg));

  /* link it */
  AttachToList(alias, dMob->pcdata->aliases);

  send_to_char("Alias created.\n\r", dMob);
}

void do_delalias(CHAR_DATA *dMob, char *arg)
{
  char name[MAX_STRING_LENGTH];
  ALIAS_DATA *alias;
  ITERATOR *pIter;

  arg = one_argument(arg, name);

  pIter = AllocIterator(dMob->pcdata->aliases);
  while ((alias = (ALIAS_DATA *) NextInList(pIter)) != NULL)
  {
    if (!str_cmp(name, alias->name))
    {
      alias_from_player(dMob, alias);
      send_to_char("The alias has been deleted.\n\r", dMob);
      return;
    }
  }
  send_to_char("You have no alias by that name.\n\r", dMob);
}

void do_pshift(CHAR_DATA *ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *vch = ch;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char *ptr;
  char silent_string[] = { 27, 27, '\0' };

  if ((d = ch->desc) == NULL)
    return;

  if (has_timer(ch))
    return;

  if (in_arena(ch) || in_fortress(ch))
  {
    send_to_char("You cannot shift here.\n\r", ch);
    return;
  }

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    show_options(d);
    send_to_char("Whom do you wish to change into?\n\r", ch);
    return;
  }

  ptr = one_argument(d->account->players, buf);
  do
  {
    if (!str_cmp(buf, arg) && str_cmp(buf, ch->name))
    {
      if (check_reconnect(d, arg, FALSE))
      {
        send_to_char("That character is already connected.\n\r", ch);
        return;
      }
      if (!load_char_obj(d, arg))
      {
        d->character = ch;
        send_to_char("Something went wrong, please report this.\n\r", ch);
        return;
      }

      ch = d->character;

      login_char(ch, FALSE);

      log_string("%s has shifted into %s.", vch->name, ch->name);

      sprintf(buf, "You shift into %s.\n\r", ch->name);
      send_to_char(buf, ch);

      if (ch->level < 7 && vch->level < 7)
      {
        sprintf(buf, "%s changes skin, and becomes %s", vch->name, ch->name);
        do_info(ch, buf);
      }
      else if (ch->level < 7 && vch->level >= 7)
      {
        sprintf(buf,"#G%s #9enters #RCalim's Cradle.#n", ch->name);
        enter_info(buf);
      }
      else if (ch->level >= 7 && vch->level < 7)
      {
        sprintf(buf, "#R%s #9has fled from #RCalim's Cradle#9.#n", vch->name);
        leave_info(buf);
      }

      if (ch->level < 7 && vch->level < 7)
        WAIT_STATE(ch, 12);

      vch->desc = NULL;
      do_quit(vch, silent_string);
      return;
    }

    /* scan forward to next character */
    ptr = one_argument(ptr, buf);
    ptr = one_argument(ptr, buf);
    ptr = one_argument(ptr, buf);
    ptr = one_argument(ptr, buf);
  } while (*ptr != '\0');

  send_to_char("You cannot change into that character.\n\r", ch);
}

void skillstance(CHAR_DATA * ch, CHAR_DATA * victim)
{
  char buf[MAX_INPUT_LENGTH];
  char bufskill[25];
  char stancename[10];
  int stance;

  stance = victim->stance[0];
  if (stance < 1 || stance > 11)
    return;
  if (victim->stance[stance] <= 0)
    sprintf(bufskill, "completely unskilled in");
  else if (victim->stance[stance] <= 25)
    sprintf(bufskill, "an apprentice of");
  else if (victim->stance[stance] <= 50)
    sprintf(bufskill, "a trainee of");
  else if (victim->stance[stance] <= 75)
    sprintf(bufskill, "a student of");
  else if (victim->stance[stance] <= 100)
    sprintf(bufskill, "fairly experienced in");
  else if (victim->stance[stance] <= 125)
    sprintf(bufskill, "well trained in");
  else if (victim->stance[stance] <= 150)
    sprintf(bufskill, "highly skilled in");
  else if (victim->stance[stance] <= 175)
    sprintf(bufskill, "an expert of");
  else if (victim->stance[stance] <= 199)
    sprintf(bufskill, "a master of");
  else if (victim->stance[stance] >= 200)
    sprintf(bufskill, "a grand master of");
  else
    return;

  if (stance == STANCE_VIPER)
    sprintf(stancename, "viper");
  else if (stance == STANCE_CRANE)
    sprintf(stancename, "crane");
  else if (stance == STANCE_CRAB)
    sprintf(stancename, "crab");
  else if (stance == STANCE_MONGOOSE)
    sprintf(stancename, "mongoose");
  else if (stance == STANCE_BULL)
    sprintf(stancename, "bull");
  else if (stance == STANCE_MANTIS)
    sprintf(stancename, "mantis");
  else if (stance == STANCE_DRAGON)
    sprintf(stancename, "dragon");
  else if (stance == STANCE_TIGER)
    sprintf(stancename, "tiger");
  else if (stance == STANCE_MONKEY)
    sprintf(stancename, "monkey");
  else if (stance == STANCE_SWALLOW)
    sprintf(stancename, "swallow");
  else if (stance == STANCE_SPIRIT)
    sprintf(stancename, "spirit");
  else
    return;

  if (ch == victim)
    sprintf(buf, "You are %s the %s stance.", bufskill, stancename);
  else
    sprintf(buf, "$N is %s the %s stance.", bufskill, stancename);

  act(buf, ch, NULL, victim, TO_CHAR);
}

void do_spell(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    show_spell(ch, 0);
    show_spell(ch, 1);
    show_spell(ch, 2);
    show_spell(ch, 3);
    show_spell(ch, 4);
  }
  else if (!str_cmp(arg, "purple"))
    show_spell(ch, 0);
  else if (!str_cmp(arg, "red"))
    show_spell(ch, 1);
  else if (!str_cmp(arg, "blue"))
    show_spell(ch, 2);
  else if (!str_cmp(arg, "green"))
    show_spell(ch, 3);
  else if (!str_cmp(arg, "yellow"))
    show_spell(ch, 4);
  else
    send_to_char("You know of no such magic.\n\r", ch);
  return;
}

void show_spell(CHAR_DATA * ch, int dtype)
{
  char buf[MAX_INPUT_LENGTH];
  char bufskill[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  if (ch->spl[dtype] == 00)
    sprintf(bufskill, "untrained at");
  else if (ch->spl[dtype] <= 25)
    sprintf(bufskill, "an apprentice at");
  else if (ch->spl[dtype] <= 50)
    sprintf(bufskill, "a student at");
  else if (ch->spl[dtype] <= 75)
    sprintf(bufskill, "a scholar at");
  else if (ch->spl[dtype] <= 100)
    sprintf(bufskill, "a magus at");
  else if (ch->spl[dtype] <= 125)
    sprintf(bufskill, "an adept at");
  else if (ch->spl[dtype] <= 150)
    sprintf(bufskill, "a mage at");
  else if (ch->spl[dtype] <= 175)
    sprintf(bufskill, "a warlock at");
  else if (ch->spl[dtype] <= 199)
    sprintf(bufskill, "a master wizard at");
  else if (ch->spl[dtype] >= 300)
    sprintf(bufskill, "a lich master of");
  else if (ch->spl[dtype] >= 240)
    sprintf(bufskill, "the complete master of");
  else if (ch->spl[dtype] >= 200)
    sprintf(bufskill, "a grand sorcerer at");

  else
    return;
  if (dtype == 0)
    sprintf(buf, "You are %s purple magic.\n\r", bufskill);
  else if (dtype == 1)
    sprintf(buf, "You are %s red magic.\n\r", bufskill);
  else if (dtype == 2)
    sprintf(buf, "You are %s blue magic.\n\r", bufskill);
  else if (dtype == 3)
    sprintf(buf, "You are %s green magic.\n\r", bufskill);
  else if (dtype == 4)
    sprintf(buf, "You are %s yellow magic.\n\r", bufskill);
  else
    return;
  send_to_char(buf, ch);
  return;
}

void do_scan(CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location;

  location = ch->in_room;

  send_to_char("[North]\n\r", ch);
  do_spydirection(ch, "n");
  char_from_room(ch);
  char_to_room(ch, location, FALSE);

  send_to_char("[East]\n\r", ch);
  do_spydirection(ch, "e");
  char_from_room(ch);
  char_to_room(ch, location, FALSE);

  send_to_char("[South]\n\r", ch);
  do_spydirection(ch, "s");
  char_from_room(ch);
  char_to_room(ch, location, FALSE);

  send_to_char("[West]\n\r", ch);
  do_spydirection(ch, "w");
  char_from_room(ch);
  char_to_room(ch, location, FALSE);

  send_to_char("[Up]\n\r", ch);
  do_spydirection(ch, "u");
  char_from_room(ch);
  char_to_room(ch, location, FALSE);

  send_to_char("[Down]\n\r", ch);
  do_spydirection(ch, "d");
  char_from_room(ch);
  char_to_room(ch, location, FALSE);
}

void do_spy(CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location;
  char arg1[MAX_INPUT_LENGTH];
  int door;

  argument = one_argument(argument, arg1);

  if (arg1[0] == '\0')
  {
    send_to_char("Which direction do you wish to spy?\n\r", ch);
    return;
  }

  if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north"))
    door = 0;
  else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east"))
    door = 1;
  else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south"))
    door = 2;
  else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west"))
    door = 3;
  else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up"))
    door = 4;
  else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down"))
    door = 5;
  else
  {
    send_to_char("You can only spy people north, south, east, west, up or down.\n\r", ch);
    return;
  }

  location = ch->in_room;

  send_to_char("[Short Range]\n\r", ch);
  do_spydirection(ch, arg1);
  send_to_char("\n\r", ch);
  send_to_char("[Medium Range]\n\r", ch);
  do_spydirection(ch, arg1);
  send_to_char("\n\r", ch);
  send_to_char("[Long Range]\n\r", ch);
  do_spydirection(ch, arg1);
  send_to_char("\n\r", ch);

  /* Move them back */
  char_from_room(ch);
  char_to_room(ch, location, FALSE);
}

void do_spydirection(CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *to_room;
  EXIT_DATA *pexit;
  CHAR_DATA *vch;
  ITERATOR *pIter, *pIter2;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  int door;
  bool is_empty;

  argument = one_argument(argument, arg);

  is_empty = TRUE;

  if (arg[0] == '\0')
    return;

  if (!str_cmp(arg, "n") || !str_cmp(arg, "north"))
    door = 0;
  else if (!str_cmp(arg, "e") || !str_cmp(arg, "east"))
    door = 1;
  else if (!str_cmp(arg, "s") || !str_cmp(arg, "south"))
    door = 2;
  else if (!str_cmp(arg, "w") || !str_cmp(arg, "west"))
    door = 3;
  else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"))
    door = 4;
  else if (!str_cmp(arg, "d") || !str_cmp(arg, "down"))
    door = 5;
  else
    return;

  if ((pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
  {
    send_to_char("   No exit.\n\r", ch);
    return;
  }
  pexit = ch->in_room->exit[door];
  if (IS_SET(pexit->exit_info, EX_CLOSED))
  {
    send_to_char("   Closed door.\n\r", ch);
    return;
  }
  char_from_room(ch);
  char_to_room(ch, to_room, FALSE);

  pIter = AllocIterator(ch->in_room->people);
  while ((vch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (vch == ch)
      continue;
    if (!can_see(ch, vch))
      continue;

    if (!IS_NPC(vch) && IS_CLASS(vch, CLASS_FAE) && IS_SET(vch->newbits, NEW_CHAMELEON))
    {
      CHAR_DATA *mobs;
      bool was_mobile = FALSE;

      pIter2 = AllocIterator(ch->in_room->people);
      while ((mobs = (CHAR_DATA *) NextInList(pIter2)) != NULL)
      {
        if (IS_NPC(mobs))
        {
          sprintf(buf, "   %s\n\r", capitalize(mobs->short_descr));
          send_to_char(buf, ch);
          was_mobile = TRUE;
        }
      }

      /* 66% chance of being invis if not a mobile */
      if (!was_mobile && number_range(1, 3) != 2)
        continue;
    }
    else if (!IS_NPC(vch) && !IS_AFFECTED(vch, AFF_POLYMORPH))
      sprintf(buf, "   %s (Player)\n\r", vch->name);
    else if (!IS_NPC(vch) && IS_AFFECTED(vch, AFF_POLYMORPH))
      sprintf(buf, "   %s (Player)\n\r", vch->morph);
    else if (IS_NPC(vch))
      sprintf(buf, "   %s\n\r", capitalize(vch->short_descr));
    send_to_char(buf, ch);

    is_empty = FALSE;
  }

  if (is_empty)
  {
    send_to_char("   Nobody here.\n\r", ch);
    return;
  }
}

void take_item(CHAR_DATA * ch, OBJ_DATA * obj)
{
  if (obj == NULL)
    return;
  obj_from_char(obj);
  obj_to_room(obj, ch->in_room);
  act("You wince in pain and $p falls to the ground.", ch, obj, NULL, TO_CHAR);
  act("$n winces in pain and $p falls to the ground.", ch, obj, NULL, TO_ROOM);
  return;
}

char *birth_date(CHAR_DATA * ch, bool is_self)
{
  static char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char mstatus[20];
  char dstatus[20];
  char omin[3];
  char cmin[3];
  char ohour[3];
  char chour[3];
  char oday0[2];
  char cday0[2];
  char oday1[2];
  char cday1[2];
  char omonth[4];
  char cmonth[4];
  char oyear[5];
  char cyear[5];
  char *strtime;
  int oyear2 = 0;
  int cyear2 = 0;
  int omonth2 = 0;
  int cmonth2 = 0;
  int oday2 = 0;
  int cday2 = 0;
  int total = 0;
  int dd = 0;
  int mm = 0;
  int yy = 0;
  bool o_month = FALSE;
  bool c_month = FALSE;

  if (is_self)
    strcpy(buf, ch->createtime);
  else
    strcpy(buf, ch->pcdata->conception);
  if (strlen(buf) < 24)
  {
    sprintf(buf, "\n\r");
    return buf;
  }
  strtime = ctime(&current_time);
  strtime[strlen(strtime) - 1] = '\0';
  strcpy(buf2, strtime);
  oyear[0] = buf[20];
  oyear[1] = buf[21];
  oyear[2] = buf[22];
  oyear[3] = buf[23];
  oyear[4] = '\0';
  omonth[0] = buf[4];
  omonth[1] = buf[5];
  omonth[2] = buf[6];
  omonth[3] = '\0';
  oday0[0] = buf[8];
  oday0[1] = '\0';
  oday1[0] = buf[9];
  oday1[1] = '\0';
  ohour[0] = buf[11];
  ohour[1] = buf[12];
  ohour[2] = '\0';
  omin[0] = buf[14];
  omin[1] = buf[15];
  omin[2] = '\0';

  cyear[0] = buf2[20];
  cyear[1] = buf2[21];
  cyear[2] = buf2[22];
  cyear[3] = buf2[23];
  cyear[4] = '\0';
  cmonth[0] = buf2[4];
  cmonth[1] = buf2[5];
  cmonth[2] = buf2[6];
  cmonth[3] = '\0';
  cday0[0] = buf2[8];
  cday0[1] = '\0';
  cday1[0] = buf2[9];
  cday1[1] = '\0';
  chour[0] = buf2[11];
  chour[1] = buf2[12];
  chour[2] = '\0';
  cmin[0] = buf2[14];
  cmin[1] = buf2[15];
  cmin[2] = '\0';

  if (!str_cmp(omonth, "Dec"))
  {
    omonth2 += 31;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Nov") || o_month)
  {
    omonth2 += 30;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Oct") || o_month)
  {
    omonth2 += 31;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Sep") || o_month)
  {
    omonth2 += 30;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Aug") || o_month)
  {
    omonth2 += 31;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Jul") || o_month)
  {
    omonth2 += 31;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Jun") || o_month)
  {
    omonth2 += 30;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "May") || o_month)
  {
    omonth2 += 31;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Apr") || o_month)
  {
    omonth2 += 30;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Mar") || o_month)
  {
    omonth2 += 31;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Feb") || o_month)
  {
    omonth2 += 28;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Jan") || o_month)
  {
    omonth2 += 31;
    o_month = TRUE;
  }
  if (!o_month)
  {
    sprintf(buf, "Error! Please inform an admin.\n\r");
    return buf;
  }
  if (!str_cmp(cmonth, "Dec"))
  {
    cmonth2 += 31;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Nov") || c_month)
  {
    cmonth2 += 30;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Oct") || c_month)
  {
    cmonth2 += 31;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Sep") || c_month)
  {
    cmonth2 += 30;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Aug") || c_month)
  {
    cmonth2 += 31;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Jul") || c_month)
  {
    cmonth2 += 31;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Jun") || c_month)
  {
    cmonth2 += 30;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "May") || c_month)
  {
    cmonth2 += 31;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Apr") || c_month)
  {
    cmonth2 += 30;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Mar") || c_month)
  {
    cmonth2 += 31;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Feb") || c_month)
  {
    cmonth2 += 28;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Jan") || c_month)
  {
    cmonth2 += 31;
    c_month = TRUE;
  }
  if (!c_month)
  {
    sprintf(buf, "Error! Please inform an admin.\n\r");
    return buf;
  }
  if (is_number(oyear))
    oyear2 += atoi(oyear);
  if (is_number(cyear))
    cyear2 += atoi(cyear);
  if (is_number(oday0))
    oday2 += (atoi(oday0) * 10);
  if (is_number(oday1))
    oday2 += atoi(oday1);
  if (is_number(cday0))
    cday2 += (atoi(cday0) * 10);
  if (is_number(cday1))
    cday2 += atoi(cday1);
  total += ((cyear2 - oyear2) * 365);
  total += (cmonth2 - omonth2);
  total += (cday2 - oday2);
  total *= 24;                  /* Total playing time is now in hours */
  if (is_number(chour) && is_number(ohour))
    total += (atoi(chour) - atoi(ohour));
  total *= 60;                  /* Total now in minutes */
  if (is_number(cmin) && is_number(omin))
    total += (atoi(cmin) - atoi(omin));
  if (total < 1)
    total = 0;
  else
    total /= 12;                /* Time now in game days */
  for (;;)
  {
    if (total >= 365)
    {
      total -= 365;
      yy += 1;
    }
    else if (total >= 30)
    {
      total -= 30;
      mm += 1;
    }
    else
    {
      dd += total;
      break;
    }
  }
  if (mm == 1)
    sprintf(mstatus, "month");
  else
    sprintf(mstatus, "months");
  if (dd == 1)
    sprintf(dstatus, "day");
  else
    sprintf(dstatus, "days");
  if (is_self)
  {
    if (!IS_EXTRA(ch, EXTRA_BORN))
      yy += 17;
    sprintf(buf, "#R[#nYou are #C%d #nyears#R][#C%d#n %s#R][#nand #C%d #n%s old#R]\n\r", yy, mm, mstatus, dd, dstatus);
    return buf;
  }
  else
  {
    if (yy > 0 || (dd > 0 && mm >= 9) || IS_EXTRA(ch, EXTRA_LABOUR))
    {
      if (!IS_EXTRA(ch, EXTRA_LABOUR))
        SET_BIT(ch->extra, EXTRA_LABOUR);
      sprintf(buf, "  You are ready to give birth.\n\r");
      return buf;
    }
    else if (yy > 0 || mm > 0)
    {
      sprintf(buf, "  You are %d %s and %d %s pregnant.\n\r", mm, mstatus, dd, dstatus);
      return buf;
    }
    else
    {
      sprintf(buf, "\n\r");
    }
  }
  return buf;
}

char *other_age(int extra, bool is_preg, char *argument)
{
  static char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char mstatus[20];
  char dstatus[20];
  char omin[3];
  char cmin[3];
  char ohour[3];
  char chour[3];
  char oday0[2];
  char cday0[2];
  char oday1[2];
  char cday1[2];
  char omonth[4];
  char cmonth[4];
  char oyear[5];
  char cyear[5];
  char *strtime;
  int oyear2 = 0;
  int cyear2 = 0;
  int omonth2 = 0;
  int cmonth2 = 0;
  int oday2 = 0;
  int cday2 = 0;
  int total = 0;
  int dd = 0;
  int mm = 0;
  int yy = 0;
  bool o_month = FALSE;
  bool c_month = FALSE;

  strcpy(buf, argument);
  if (strlen(buf) < 24)
  {
    sprintf(buf, "You have encountered an error, please inform an admin.\n\r");
    return buf;
  }
  strtime = ctime(&current_time);
  strtime[strlen(strtime) - 1] = '\0';
  strcpy(buf2, strtime);
  oyear[0] = buf[20];
  oyear[1] = buf[21];
  oyear[2] = buf[22];
  oyear[3] = buf[23];
  oyear[4] = '\0';
  omonth[0] = buf[4];
  omonth[1] = buf[5];
  omonth[2] = buf[6];
  omonth[3] = '\0';
  oday0[0] = buf[8];
  oday0[1] = '\0';
  oday1[0] = buf[9];
  oday1[1] = '\0';
  ohour[0] = buf[11];
  ohour[1] = buf[12];
  ohour[2] = '\0';
  omin[0] = buf[14];
  omin[1] = buf[15];
  omin[2] = '\0';

  cyear[0] = buf2[20];
  cyear[1] = buf2[21];
  cyear[2] = buf2[22];
  cyear[3] = buf2[23];
  cyear[4] = '\0';
  cmonth[0] = buf2[4];
  cmonth[1] = buf2[5];
  cmonth[2] = buf2[6];
  cmonth[3] = '\0';
  cday0[0] = buf2[8];
  cday0[1] = '\0';
  cday1[0] = buf2[9];
  cday1[1] = '\0';
  chour[0] = buf2[11];
  chour[1] = buf2[12];
  chour[2] = '\0';
  cmin[0] = buf2[14];
  cmin[1] = buf2[15];
  cmin[2] = '\0';

  if (!str_cmp(omonth, "Dec"))
  {
    omonth2 += 31;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Nov") || o_month)
  {
    omonth2 += 30;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Oct") || o_month)
  {
    omonth2 += 31;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Sep") || o_month)
  {
    omonth2 += 30;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Aug") || o_month)
  {
    omonth2 += 31;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Jul") || o_month)
  {
    omonth2 += 31;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Jun") || o_month)
  {
    omonth2 += 30;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "May") || o_month)
  {
    omonth2 += 31;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Apr") || o_month)
  {
    omonth2 += 30;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Mar") || o_month)
  {
    omonth2 += 31;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Feb") || o_month)
  {
    omonth2 += 28;
    o_month = TRUE;
  }
  if (!str_cmp(omonth, "Jan") || o_month)
  {
    omonth2 += 31;
    o_month = TRUE;
  }
  if (!o_month)
  {
    sprintf(buf, "Error! Please inform an admin.\n\r");
    return buf;
  }
  if (!str_cmp(cmonth, "Dec"))
  {
    cmonth2 += 31;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Nov") || c_month)
  {
    cmonth2 += 30;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Oct") || c_month)
  {
    cmonth2 += 31;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Sep") || c_month)
  {
    cmonth2 += 30;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Aug") || c_month)
  {
    cmonth2 += 31;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Jul") || c_month)
  {
    cmonth2 += 31;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Jun") || c_month)
  {
    cmonth2 += 30;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "May") || c_month)
  {
    cmonth2 += 31;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Apr") || c_month)
  {
    cmonth2 += 30;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Mar") || c_month)
  {
    cmonth2 += 31;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Feb") || c_month)
  {
    cmonth2 += 28;
    c_month = TRUE;
  }
  if (!str_cmp(cmonth, "Jan") || c_month)
  {
    cmonth2 += 31;
    c_month = TRUE;
  }
  if (!c_month)
  {
    sprintf(buf, "Error! Please inform an admin.\n\r");
    return buf;
  }
  if (is_number(oyear))
    oyear2 += atoi(oyear);
  if (is_number(cyear))
    cyear2 += atoi(cyear);
  if (is_number(oday0))
    oday2 += (atoi(oday0) * 10);
  if (is_number(oday1))
    oday2 += atoi(oday1);
  if (is_number(cday0))
    cday2 += (atoi(cday0) * 10);
  if (is_number(cday1))
    cday2 += atoi(cday1);
  total += ((cyear2 - oyear2) * 365);
  total += (cmonth2 - omonth2);
  total += (cday2 - oday2);
  total *= 24;                  /* Total playing time is now in hours */
  if (is_number(chour) && is_number(ohour))
    total += (atoi(chour) - atoi(ohour));
  total *= 60;                  /* Total now in minutes */
  if (is_number(cmin) && is_number(omin))
    total += (atoi(cmin) - atoi(omin));
  if (total < 1)
    total = 0;
  else
    total /= 12;                /* Time now in game days */
  for (;;)
  {
    if (total >= 365)
    {
      total -= 365;
      yy += 1;
    }
    else if (total >= 30)
    {
      total -= 30;
      mm += 1;
    }
    else
    {
      dd += total;
      break;
    }
  }
  if (mm == 1)
    sprintf(mstatus, "month");
  else
    sprintf(mstatus, "months");
  if (dd == 1)
    sprintf(dstatus, "day");
  else
    sprintf(dstatus, "days");
  if (!is_preg)
  {
    yy += 17;
    sprintf(buf, "Age: %d years, %d %s and %d %s old.\n\r", yy, mm, mstatus, dd, dstatus);
  }
  else
  {
    if (yy > 0 || (dd > 0 && mm >= 9) || IS_SET(extra, EXTRA_LABOUR))
    {
      sprintf(buf, "She is ready to give birth.\n\r");
    }
    else if (yy > 0 || mm > 0)
    {
      sprintf(buf, "She is %d %s and %d %s pregnant.\n\r", mm, mstatus, dd, dstatus);
    }
  }

  return buf;
}

void do_huh(CHAR_DATA * ch, char *argument)
{
  send_to_char("Huh?\n\r", ch);
}

void do_consent(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument(argument, arg);

  if (IS_NPC(ch))
    return;

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They are not here.\n\r", ch);
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("Not on NPC's.\n\r", ch);
    return;
  }

  if (ch == victim)
  {
    send_to_char("Not on yourself!\n\r", ch);
    return;
  }

  ch->pcdata->partner = victim;
  act("You give $N permission to FUCK on you.", ch, NULL, victim, TO_CHAR);
  act("$n has given you permission to FUCK on $m.", ch, NULL, victim, TO_VICT);
}

char *format_obj_to_char(OBJ_DATA * obj, CHAR_DATA * ch, bool fShort)
{
  static char buf[MAX_STRING_LENGTH];

  buf[0] = '\0';
  if (IS_SET(obj->quest, QUEST_ARTIFACT))
    strcat(buf, "#9(Artifact)#n ");
  else if (IS_SET(obj->quest, QUEST_PRIZE))
    strcat(buf, "#r(#CPrize#r)#n ");
  else if (IS_SET(obj->quest, QUEST_RELIC))
    strcat(buf, "#r(#9Relic#r)#n ");
  if (IS_OBJ_STAT(obj, ITEM_GLOW))
    strcat(buf, "#y(#rGlow#y)#n ");
  if (IS_OBJ_STAT(obj, ITEM_HUM))
    strcat(buf, "#y(#rHum#y)#n ");
  if (IS_OBJ_STAT(obj, ITEM_RARE))
    strcat(buf, "#0(#RRare#0)#n ");
  if (IS_OBJ_STAT(obj, ITEM_MASTERY))
    strcat(buf, "#0(#PMastery#0)#n ");
  if (IS_OBJ_STAT(obj, ITEM_SENTIENT))
    strcat(buf, "#0(#RSentient#0)#n ");
  if (object_is_affected(obj, OAFF_LIQUID))
    strcat(buf, "#L(#CLiquid#L)#n ");
  if (IS_OBJ_STAT(obj, ITEM_INVIS))
    strcat(buf, "#r(Invis)#n ");
  if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && !IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_OBJ_STAT(obj, ITEM_ANTI_EVIL))
    strcat(buf, "#l(Blue Aura)#n ");
  else if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && !IS_OBJ_STAT(obj, ITEM_ANTI_EVIL))
    strcat(buf, "#r(Red Aura)#n ");
  else if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && !IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_OBJ_STAT(obj, ITEM_ANTI_EVIL))
    strcat(buf, "#y(Yellow Aura)#n ");
  if (IS_AFFECTED(ch, AFF_DETECT_MAGIC) && IS_OBJ_STAT(obj, ITEM_MAGIC))
    strcat(buf, "#c(Magical)#n ");

  if (fShort)
  {
    if (obj->short_descr != NULL)
      strcat(buf, obj->short_descr);
    if (obj->condition < 100 && obj->condition > 0)
      strcat(buf, " #r(Damaged)#n");
    else if (obj->condition <= 0)
      strcat(buf, " #r(Broken)#n");
  }
  else
  {
    if (obj->description != NULL)
      strcat(buf, obj->description);
  }

  return buf;
}

int char_damroll(CHAR_DATA *ch)
{
  int value = ch->damroll + ((IS_NPC(ch)) ? 0 : ch->pcdata->legend * 25);

  value += UMIN(25, get_curr_str(ch)) / 2;

  value = URANGE(-100, value, 4000);

  return value;
}

int char_hitroll(CHAR_DATA *ch)
{
  int value = ch->hitroll + ((IS_NPC(ch)) ? 0 : ch->pcdata->legend * 25);

  value += UMIN(25, get_curr_str(ch)) / 2;  
    
  value = URANGE(-100, value, 4000);
  
  return value;
}

int char_ac(CHAR_DATA *ch)
{
  int value = ch->armor;

  if (IS_AWAKE(ch))
    value -= UMIN(25, get_curr_dex(ch)) * 8;

  value = URANGE(-1000, value, 200);

  return value;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char(LIST *list, CHAR_DATA *ch, bool fShort, bool fShowNothing)
{
  char buf[MAX_STRING_LENGTH];
  ITERATOR *pIter;
  char **prgpstrShow;
  int *prgnShow;
  char *pstrShow;
  OBJ_DATA *obj;
  int nShow = 0;
  int iShow;
  int count;
  bool fCombine;

  if (ch->desc == NULL)
    return;

  /*
   * Alloc space for output lines.
   */
  if ((count = SizeOfList(list)) == 0)
  {
    if (fShowNothing)
    {
      send_to_char("     ", ch);
      send_to_char("Nothing.\n\r", ch);
    }
    return;
  }

  prgpstrShow = calloc(count, sizeof(char *));
  prgnShow = calloc(count, sizeof(int));

  /*
   * Format the list of objects.
   */
  pIter = AllocIterator(list);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (!fShort && IS_OBJ_STAT(obj, ITEM_NOSHOW))
      continue;

    if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj))
    {
      pstrShow = format_obj_to_char(obj, ch, fShort);
      fCombine = FALSE;

      /*
       * Look for duplicates, case sensitive.
       * Matches tend to be near end so run loop backwords.
       */
      for (iShow = nShow - 1; iShow >= 0; iShow--)
      {
        if (!strcmp(prgpstrShow[iShow], pstrShow))
        {
          prgnShow[iShow]++;
          fCombine = TRUE;
          break;
        }
      }

      /*
       * Couldn't combine, or didn't want to.
       */
      if (!fCombine)
      {
        prgpstrShow[nShow] = str_dup(pstrShow);
        prgnShow[nShow] = 1;
        nShow++;
      }
    }
  }

  /*
   * Output the formatted list.
   */
  for (iShow = 0; iShow < nShow; iShow++)
  {
    if (prgnShow[iShow] != 1)
    {
      sprintf(buf, "(%2d) ", prgnShow[iShow]);
      send_to_char(buf, ch);
    }
    else
    {
      send_to_char("     ", ch);
    }

    send_to_char(prgpstrShow[iShow], ch);
    send_to_char("\n\r", ch);
    free_string(prgpstrShow[iShow]);
  }

  if (fShowNothing && nShow == 0)
  {
    send_to_char("     ", ch);
    send_to_char("Nothing.\n\r", ch);
  }

  /*
   * Clean up.
   */
  free(prgpstrShow);
  free(prgnShow);
}

void show_char_to_char_0(CHAR_DATA *victim, CHAR_DATA *ch)
{
  BUFFER *buf;
  CHAR_DATA *mount;

  if ((mount = victim->mount) != NULL && IS_SET(victim->mounted, IS_MOUNT))
    return;

  buf = buffer_new(MAX_STRING_LENGTH);

  /* add shadow aura sight */
  if (!IS_NPC(ch) && IS_CLASS(ch, CLASS_SHADOW) && IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_AURASIGHT))
  {
    bprintf(buf, "#0(#9%d%%#0)#n ", (victim->max_hit == 0) ? 0 : (100 * victim->hit) / victim->max_hit);
  }

  if (IS_SET(ch->newbits, NEW_SHADOWPLANE) && !IS_SET(victim->newbits, NEW_SHADOWPLANE))
    bprintf(buf, "#0(#CNormal plane#0)#n ");
  else if (!IS_SET(ch->newbits, NEW_SHADOWPLANE) && IS_SET(victim->newbits, NEW_SHADOWPLANE))
    bprintf(buf, "#C(#0Shadowplane#C)#n ");

  if (!IS_NPC(ch) && IS_NPC(victim) && is_quest_target(ch, victim))
    bprintf(buf, "#y(#RQuest#y)#n ");
  if (!IS_NPC(victim) && victim->desc == NULL)
    bprintf(buf, "#y(Link-Dead)#n ");
  if (IS_AFFECTED(victim, AFF_INVISIBLE))
    bprintf(buf, "#L(Invis)#n ");
  if (IS_AFFECTED(victim, AFF_CHARM))
    bprintf(buf, "#R(Charmed)#n ");
  if (IS_AFFECTED(victim, AFF_PASS_DOOR) || IS_AFFECTED(victim, AFF_ETHEREAL))
    bprintf(buf, "#l(Translucent)#n ");
  if (IS_EVIL(victim) && IS_AFFECTED(ch, AFF_DETECT_EVIL))
    bprintf(buf, "#R(Red Aura)#n ");
  if (IS_AFFECTED(victim, AFF_SANCTUARY))
    bprintf(buf, "#C(White Aura)#n ");

  /* mobiles have a special long_descr instead when not engaged in activity */
  if (IS_NPC(victim) && victim->position == POS_STANDING)
    bprintf(buf, "%s", victim->long_descr);
  else
  {
    bprintf(buf, "%s", PERS(victim, ch));

    if ((mount = victim->mount) != NULL && victim->mounted == IS_RIDING)
      bprintf(buf, " is here riding %s", PERS(mount, ch));
    else if (victim->position == POS_STANDING && IS_AFFECTED(victim, AFF_FLYING))
      bprintf(buf, " is hovering here.\n\r");
    else if (!IS_NPC(victim) || victim->position != POS_STANDING)
    {
      switch (victim->position)
      {
        case POS_DEAD:
          bprintf(buf, " is DEAD!!\n\r");
          break;
        case POS_MORTAL:
          bprintf(buf, " is #Rmortally wounded#n.\n\r");
          break;
        case POS_INCAP:
          bprintf(buf, " is #rincapacitated#n.\n\r");
          break;
        case POS_STUNNED:
          bprintf(buf, " is lying here #Cstunned#n.\n\r");
          break;
        case POS_SLEEPING:
          bprintf(buf, " is sleeping here.\n\r");
          break;
        case POS_RESTING:
          bprintf(buf, " is resting here.\n\r");
          break;
        case POS_MEDITATING:
          bprintf(buf, " is meditating here.\n\r");
          break;
        case POS_SITTING:
          bprintf(buf, " is sitting here.\n\r");
          break;
        case POS_STANDING:
          if (!IS_NPC(victim) && victim->stance[0] == STANCE_VIPER)
            bprintf(buf, " is here, crouched in a viper fighting stance.\n\r");
          else if (!IS_NPC(victim) && victim->stance[0] == STANCE_CRANE)
            bprintf(buf, " is here, crouched in a crane fighting stance.\n\r");
          else if (!IS_NPC(victim) && victim->stance[0] == STANCE_CRAB)
            bprintf(buf, " is here, crouched in a crab fighting stance.\n\r");
          else if (!IS_NPC(victim) && victim->stance[0] == STANCE_MONGOOSE)
            bprintf(buf, " is here, crouched in a mongoose fighting stance.\n\r");
          else if (!IS_NPC(victim) && victim->stance[0] == STANCE_BULL)
            bprintf(buf, " is here, crouched in a bull fighting stance.\n\r");
          else if (!IS_NPC(victim) && victim->stance[0] == STANCE_MANTIS)
            bprintf(buf, " is here, crouched in a mantis fighting stance.\n\r");
          else if (!IS_NPC(victim) && victim->stance[0] == STANCE_DRAGON)
            bprintf(buf, " is here, crouched in a dragon fighting stance.\n\r");
          else if (!IS_NPC(victim) && victim->stance[0] == STANCE_TIGER)
            bprintf(buf, " is here, crouched in a tiger fighting stance.\n\r");
          else if (!IS_NPC(victim) && victim->stance[0] == STANCE_MONKEY)
            bprintf(buf, " is here, crouched in a monkey fighting stance.\n\r");
          else if (!IS_NPC(victim) && victim->stance[0] == STANCE_SWALLOW)
            bprintf(buf, " is here, crouched in a swallow fighting stance.\n\r");
          else if (!IS_NPC(victim) && victim->stance[0] == STANCE_SPIRIT)
            bprintf(buf, " is here, crouched in a spirit fighting stance.\n\r");
          else
            bprintf(buf, " is here.\n\r");
          break;
        case POS_FIGHTING:  /* fighting is handled elsewhere */
          break;
      }
    }

    if (victim->position == POS_FIGHTING)
    {
      if (mount == NULL)
        bprintf(buf, " is here, fighting ");
      else
        bprintf(buf, ", fighting ");

      if (victim->fighting == NULL)
        bprintf(buf, "thin air??\n\r");
      else if (victim->fighting == ch)
        bprintf(buf, "YOU!\n\r");
      else if (victim->in_room == victim->fighting->in_room)
        bprintf(buf, "%s.\n\r", PERS(victim->fighting, ch));
      else
        bprintf(buf, "someone who left??\n\r");
    }
    else if (mount != NULL)
      bprintf(buf, ".\n\r");
  }

  if (IS_AFFECTED(victim, AFF_FLAMING))
    bprintf(buf, "...%s is engulfed in blazing flames!\n\r", PERS(victim, ch));

  if (IS_EXTRA(victim, TIED_UP))
  {
    bprintf(buf, "...%s is tied up", PERS(victim, ch));
    if (IS_EXTRA(victim, GAGGED) && IS_EXTRA(victim, BLINDFOLDED))
      bprintf(buf, ", gagged and blindfolded!\n\r");
    else if (IS_EXTRA(victim, GAGGED))
      bprintf(buf, " and gagged!\n\r");
    else if (IS_EXTRA(victim, BLINDFOLDED))
      bprintf(buf, " and blindfolded!\n\r");
    else
      bprintf(buf, "!\n\r");
  }

  if (IS_AFFECTED(victim, AFF_WEBBED))
    bprintf(buf, "...%s is coated in a sticky web.\n\r", PERS(victim, ch));

  if (IS_SET(victim->newbits, NEW_TENDRIL3))
    bprintf(buf, "...%s is entangled in three black tendrils.\n\r", PERS(victim, ch));
  else if (IS_SET(victim->newbits, NEW_TENDRIL2))
    bprintf(buf, "...%s is entangled in two black tendrils.\n\r", PERS(victim, ch));
  else if (IS_SET(victim->newbits, NEW_TENDRIL1))
    bprintf(buf, "...%s is entangled in one black tendrils.\n\r", PERS(victim, ch));

  if (IS_ITEMAFF(victim, ITEMA_CHAOSSHIELD))
    bprintf(buf, "...%s is surrounded by a swirling shield of chaos.\n\r", PERS(victim, ch));

  send_to_char(buf->data, ch);
  buffer_free(buf);
}

void show_char_to_char_1(CHAR_DATA * victim, CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  int iWear;
  int percent;
  bool found;

  if (can_see(victim, ch))
  {
    act("$n looks at you.", ch, NULL, victim, TO_VICT);
    act("$n looks at $N.", ch, NULL, victim, TO_NOTVICT);
  }

  if (victim->description[0] != '\0')
  {
    send_to_char(victim->description, ch);
  }
  else
  {
    act("You see nothing special about $M.", ch, NULL, victim, TO_CHAR);
  }

  if (victim->max_hit > 0)
    percent = (100 * victim->hit) / victim->max_hit;
  else
    percent = -1;

  strcpy(buf, PERS(victim, ch));

  if (percent >= 100)
    strcat(buf, " is in perfect health.\n\r");
  else if (percent >= 90)
    strcat(buf, " is slightly scratched.\n\r");
  else if (percent >= 80)
    strcat(buf, " has a few bruises.\n\r");
  else if (percent >= 70)
    strcat(buf, " has some cuts.\n\r");
  else if (percent >= 60)
    strcat(buf, " has several wounds.\n\r");
  else if (percent >= 50)
    strcat(buf, " has many nasty wounds.\n\r");
  else if (percent >= 40)
    strcat(buf, " is bleeding freely.\n\r");
  else if (percent >= 30)
    strcat(buf, " is covered in blood.\n\r");
  else if (percent >= 20)
    strcat(buf, " is leaking guts.\n\r");
  else if (percent >= 10)
    strcat(buf, " is almost dead.\n\r");
  else
    strcat(buf, " is DYING.\n\r");

  buf[0] = UPPER(buf[0]);
  send_to_char(buf, ch);

  if (!IS_NPC(victim))
  {
    if (IS_AFFECTED(victim, AFF_INFRARED))
      act("$N's eyes are glowing bright red.", ch, NULL, victim, TO_CHAR);
    if (IS_AFFECTED(victim, AFF_FLYING))
      act("$N is hovering in the air.", ch, NULL, victim, TO_CHAR);
  }
  found = FALSE;
  for (iWear = 0; iWear < MAX_WEAR; iWear++)
  {
    if ((obj = get_eq_char(victim, iWear)) != NULL && can_see_obj(ch, obj))
    {
      if (!found)
      {
        send_to_char("\n\r", ch);
        act("$N is using:", ch, NULL, victim, TO_CHAR);
        found = TRUE;
      }
      send_to_char(where_name[iWear], ch);
      send_to_char(format_obj_to_char(obj, ch, TRUE), ch);
      send_to_char("\n\r", ch);
    }
  }

  if (victim != ch && !IS_NPC(ch) && number_percent() < ch->pcdata->learned[gsn_peek])
  {
    send_to_char("\n\rYou peek at the inventory:\n\r", ch);
    show_list_to_char(victim->carrying, ch, TRUE, TRUE);
  }
}

void show_char_to_char(LIST *list, CHAR_DATA *ch)
{
  CHAR_DATA *rch;
  ITERATOR *pIter, *pIter2;

  pIter = AllocIterator(list);
  while ((rch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (rch == ch)
      continue;

    if (can_see(ch, rch))
    {
      if (!IS_NPC(rch) && IS_CLASS(rch, CLASS_FAE) && IS_SET(rch->newbits, NEW_CHAMELEON))
      {
        CHAR_DATA *mobs;
        bool was_mobile = FALSE;


        pIter2 = AllocIterator(list);
        while ((mobs = (CHAR_DATA *) NextInList(pIter2)) != NULL)
        {
          if (IS_NPC(mobs))
          {
            show_char_to_char_0(mobs, ch);
            was_mobile = TRUE;
          }
        }

        /* 66% chance of being invis if not a mobile */
        if (!was_mobile && number_range(1, 3) != 2)
          continue;
      }

      show_char_to_char_0(rch, ch);
    }
    else if (room_is_dark(ch->in_room) && IS_AFFECTED(rch, AFF_INFRARED))
    {
      send_to_char("You see glowing #Rred#n eyes watching YOU!\n\r", ch);
    }
  }
}

bool check_blind(CHAR_DATA * ch)
{
  if (IS_EXTRA(ch, BLINDFOLDED))
  {
    send_to_char("You can't see a thing through the blindfold!\n\r", ch);
    return FALSE;
  }

  if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
    return TRUE;

  if (IS_ITEMAFF(ch, ITEMA_VISION))
    return TRUE;

  if (IS_HEAD(ch, LOST_EYE_L) && IS_HEAD(ch, LOST_EYE_R))
  {
    send_to_char("You have no eyes!\n\r", ch);
    return FALSE;
  }

  if (IS_AFFECTED(ch, AFF_BLIND) && !IS_AFFECTED(ch, AFF_SHADOWSIGHT))
  {
    send_to_char("You can't see a thing!\n\r", ch);
    return FALSE;
  }

  return TRUE;
}

void do_racecommands(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (IS_CLASS(ch, CLASS_SHADOW))
    shadow_commands(ch);
  else if (IS_CLASS(ch, CLASS_WARLOCK))
    warlock_commands(ch);
  else if (IS_CLASS(ch, CLASS_FAE))
    fae_commands(ch);
  else if (IS_CLASS(ch, CLASS_GIANT))
    giant_commands(ch);
  else
    send_to_char("You do not have any special powers.\n\r", ch);
}

void do_truesight(CHAR_DATA * ch, char *argument)
{
  bool cando = FALSE;

  if (IS_NPC(ch)) return;

  if (IS_CLASS(ch, CLASS_SHADOW) && IS_SET(ch->pcdata->powers[SHADOW_POWERS], NSHADOWS_SIGHT))
    cando = TRUE;
  if (IS_CLASS(ch, CLASS_WARLOCK) && ch->pcdata->powers[SPHERE_DIVINATION] >= 1)
    cando = TRUE;
  if (IS_CLASS(ch, CLASS_GIANT) || IS_CLASS(ch, CLASS_FAE))
    cando = TRUE;

  if (!cando)
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  if (IS_SET(ch->act, PLR_HOLYLIGHT))
  {
    REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
    send_to_char("Your senses return to normal.\n\r", ch);
  }
  else
  {
    SET_BIT(ch->act, PLR_HOLYLIGHT);
    send_to_char("Your senses increase to incredible proportions.\n\r", ch);
  }
}  

void do_look(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  EXIT_DATA *pexit;
  CHAR_DATA *victim;
  CHAR_DATA *vch;
  ITERATOR *pIter;
  OBJ_DATA *obj;
  ROOM_INDEX_DATA *pRoomIndex;
  ROOM_INDEX_DATA *location;
  char *pdesc;
  int door;

  if (ch->desc == NULL)
    return;

  if (ch->in_room == NULL)
    return;

  if (ch->position < POS_SLEEPING)
  {
    send_to_char("You can't see anything but stars!\n\r", ch);
    return;
  }

  if (ch->position == POS_SLEEPING)
  {
    send_to_char("You can't see anything, you're sleeping!\n\r", ch);
    return;
  }

  if (!check_blind(ch))
    return;

  if (!IS_NPC(ch) && IS_SET(ch->in_room->room_flags, ROOM_TOTAL_DARKNESS) && !IS_ITEMAFF(ch, ITEMA_VISION) && !IS_IMMORTAL(ch))
  {
    send_to_char("It is pitch black ... \n\r", ch);
    return;
  }

  if (!IS_NPC(ch) && !IS_SET(ch->act, PLR_HOLYLIGHT) && !IS_ITEMAFF(ch, ITEMA_VISION) && room_is_dark(ch->in_room))
  {
    send_to_char("It is pitch black ... \n\r", ch);
    show_char_to_char(ch->in_room->people, ch);
    return;
  }

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0' || !str_cmp(arg1, "auto") || !str_cmp(arg1, "scry"))
  {
    /* 'look' or 'look auto' */
    if (!IS_SET(ch->act, PLR_MAP))
    {
      if (!IS_NPC(ch) && ch->level > 6)
      {
        sprintf(buf, "%s #0[#9%d#0]#n\n\r", ch->in_room->name, ch->in_room->vnum);
        send_to_char(buf, ch);
      }
      else
      {
        sprintf(buf, "%s\n\r", ch->in_room->name);
        send_to_char(buf, ch);
      }
    }

    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) && (!IS_SET(ch->act, PLR_MAP) || ch->pcdata->brief[BRIEF_1]))
      mxp_to_char(get_exits(ch), ch, MXP_ALL);

    if ((!IS_NPC(ch) && !ch->pcdata->brief[BRIEF_1]) && (arg1[0] == '\0' || !str_cmp(arg1, "auto") || !str_cmp(arg1, "scry")))
    {
      if (IS_SET(ch->act, PLR_MAP))
        draw_map(ch, ch->in_room->description);
      else
        send_to_char(ch->in_room->description, ch);

      event_look(ch);
      kingdom_look(ch);

      if (ch->in_room->blood > 0)
      {
        if (ch->in_room->blood >= 1000)
          send_to_char("#RYou notice that the room is completely drenched in blood.#n\n\r", ch);
        else if (ch->in_room->blood > 750)
          send_to_char("#RYou notice that there is a very large amount of blood around the room.#n\n\r", ch);
        else if (ch->in_room->blood > 500)
          send_to_char("#RYou notice that there is a large quantity of blood around the room.#n\n\r", ch);
        else if (ch->in_room->blood > 250)
          send_to_char("#RYou notice a fair amount of blood on the floor.#n\n\r", ch);
        else if (ch->in_room->blood > 100)
          send_to_char("#RYou notice several blood stains on the floor.#n\n\r", ch);
        else if (ch->in_room->blood > 50)
          send_to_char("#RYou notice a few blood stains on the floor.#n\n\r", ch);
        else if (ch->in_room->blood > 25)
          send_to_char("#RYou notice a couple of blood stains on the floor.#n\n\r", ch);
        else
          send_to_char("#RYou notice a few drops of blood on the floor.#n\n\r", ch);
      }
    }
    if (IS_SET(ch->in_room->room_flags, ROOM_FLAMING))
      send_to_char("..This room is engulfed in flames!\n\r", ch);

    show_list_to_char(ch->in_room->contents, ch, FALSE, FALSE);

    for (door = 0; door < 6; door++)
    {
      if (ch->in_room == NULL)
        continue;
      if (ch->in_room->exit[door] == NULL)
        continue;

      if (IS_SET(ch->in_room->exit[door]->exit_info, EX_PRISMATIC_WALL))
      {
        sprintf(buf, "     You see a shimmering wall of many colours %s.\n\r", exitname[door]);
        send_to_char(buf, ch);
      }
      else if (IS_SET(ch->in_room->exit[door]->exit_info, EX_ICE_WALL))
      {
        sprintf(buf, "     You see a glacier of ice %s.\n\r", exitname[door]);
        send_to_char(buf, ch);
      }
      else if (IS_SET(ch->in_room->exit[door]->exit_info, EX_CALTROP_WALL))
      {
        sprintf(buf, "     You see a wall of caltrops %s.\n\r", exitname[door]);
        send_to_char(buf, ch);
      }
      else if (IS_SET(ch->in_room->exit[door]->exit_info, EX_FIRE_WALL))
      {
        sprintf(buf, "     You see a blazing wall of fire %s.\n\r", exitname[door]);
        send_to_char(buf, ch);
      }
      else if (IS_SET(ch->in_room->exit[door]->exit_info, EX_SWORD_WALL))
      {
        sprintf(buf, "     You see a spinning wall of swords %s.\n\r", exitname[door]);
        send_to_char(buf, ch);
      }
      else if (IS_SET(ch->in_room->exit[door]->exit_info, EX_MUSHROOM_WALL))
      {
        sprintf(buf, "     You see a vibrating mound of mushrooms %s.\n\r", exitname[door]);
        send_to_char(buf, ch);
      }
      else if (IS_SET(ch->in_room->exit[door]->exit_info, EX_SHADOW_WALL))
      {
        sprintf(buf, "    You see veil of shadows %s.\n\r", exitname[door]);
        send_to_char(buf, ch);
      }
      else if (IS_SET(ch->in_room->exit[door]->exit_info, EX_ASH_WALL))
      {
        sprintf(buf, "    You see a deadly wall of ash %s.\n\r", exitname[door]);
        send_to_char(buf, ch);
      }
    }
    show_char_to_char(ch->in_room->people, ch);

    return;
  }

  if (!str_cmp(arg1, "i") || !str_cmp(arg1, "in"))
  {
    /* 'look in' */
    if (arg2[0] == '\0')
    {
      send_to_char("Look in what?\n\r", ch);
      return;
    }

    if ((obj = get_obj_here(ch, arg2)) == NULL)
    {
      send_to_char("You do not see that here.\n\r", ch);
      return;
    }

    switch (obj->item_type)
    {
      default:
        send_to_char("That is not a container.\n\r", ch);
        break;

      case ITEM_PORTAL:
        pRoomIndex = get_room_index(obj->value[0]);
        location = ch->in_room;
        if (pRoomIndex == NULL)
        {
          send_to_char("It doesn't seem to lead anywhere.\n\r", ch);
          return;
        }
        char_from_room(ch);
        char_to_room(ch, pRoomIndex, FALSE);
        do_look(ch, "scry");
        char_from_room(ch);
        char_to_room(ch, location, FALSE);
        break;

      case ITEM_CONTAINER:
      case ITEM_CORPSE_NPC:
      case ITEM_CORPSE_PC:
        if (IS_SET(obj->value[1], CONT_CLOSED))
        {
          send_to_char("It is closed.\n\r", ch);
          break;
        }

        act("$p contains:", ch, obj, NULL, TO_CHAR);
        show_list_to_char(obj->contains, ch, TRUE, TRUE);
        break;
    }
    return;
  }

  if ((victim = get_char_room(ch, arg1)) != NULL)
  {
    show_char_to_char_1(victim, ch);
    return;
  }

  pIter = AllocIterator(char_list);
  while ((vch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (vch->in_room == NULL)
      continue;
    if (vch->in_room == ch->in_room)
    {
      if (!IS_NPC(vch) && !str_cmp(arg1, vch->morph))
      {
        show_char_to_char_1(vch, ch);
        return;
      }
      continue;
    }
  }

  pIter = AllocIterator(ch->carrying);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (can_see_obj(ch, obj))
    {
      pdesc = get_extra_descr(arg1, obj->extra_descr);
      if (pdesc != NULL)
      {
        send_to_char(pdesc, ch);
        return;
      }

      pdesc = get_extra_descr(arg1, obj->pIndexData->extra_descr);
      if (pdesc != NULL)
      {
        send_to_char(pdesc, ch);
        return;
      }
    }

    if (is_name(arg1, obj->name))
    {
      send_to_char(obj->description, ch);
      send_to_char("\n\r", ch);
      return;
    }
  }

  pIter = AllocIterator(ch->in_room->contents);
  while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
  {
    if (can_see_obj(ch, obj))
    {
      pdesc = get_extra_descr(arg1, obj->extra_descr);
      if (pdesc != NULL)
      {
        send_to_char(pdesc, ch);
        return;
      }

      pdesc = get_extra_descr(arg1, obj->pIndexData->extra_descr);
      if (pdesc != NULL)
      {
        send_to_char(pdesc, ch);
        return;
      }
    }

    if (is_name(arg1, obj->name))
    {
      send_to_char(obj->description, ch);
      send_to_char("\n\r", ch);
      return;
    }
  }

  pdesc = get_extra_descr(arg1, ch->in_room->extra_descr);
  if (pdesc != NULL)
  {
    send_to_char(pdesc, ch);
    return;
  }

  if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north"))
    door = 0;
  else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east"))
    door = 1;
  else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south"))
    door = 2;
  else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west"))
    door = 3;
  else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up"))
    door = 4;
  else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down"))
    door = 5;
  else
  {
    send_to_char("You do not see that here.\n\r", ch);
    return;
  }

  /* 'look direction' */
  if ((pexit = ch->in_room->exit[door]) == NULL)
  {
    send_to_char("Nothing special there.\n\r", ch);
    return;
  }

  if (pexit->keyword != NULL && pexit->keyword[0] != '\0' && pexit->keyword[0] != ' ')
  {
    if (IS_SET(pexit->exit_info, EX_CLOSED))
    {
      act("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
    }
    else if (IS_SET(pexit->exit_info, EX_ISDOOR))
    {
      act("The $d is open.", ch, NULL, pexit->keyword, TO_CHAR);
      if ((pexit = ch->in_room->exit[door]) == NULL)
        return;
      if ((pRoomIndex = pexit->to_room) == NULL)
        return;
      location = ch->in_room;
      char_from_room(ch);
      char_to_room(ch, pRoomIndex, FALSE);
      do_look(ch, "auto");
      char_from_room(ch);
      char_to_room(ch, location, FALSE);
    }
    else
    {
      if ((pexit = ch->in_room->exit[door]) == NULL)
        return;
      if ((pRoomIndex = pexit->to_room) == NULL)
        return;
      location = ch->in_room;
      char_from_room(ch);
      char_to_room(ch, pRoomIndex, FALSE);
      do_look(ch, "auto");
      char_from_room(ch);
      char_to_room(ch, location, FALSE);
    }
  }
  else if (IS_SET(pexit->exit_info, EX_CLOSED))
  {
    act("The door is closed.", ch, NULL, NULL, TO_CHAR);
    return;
  }
  else
  {
    if ((pexit = ch->in_room->exit[door]) == NULL)
      return;
    if ((pRoomIndex = pexit->to_room) == NULL)
      return;
    location = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, pRoomIndex, FALSE);
    do_look(ch, "auto");
    char_from_room(ch);
    char_to_room(ch, location, FALSE);
  }
}

void do_search(CHAR_DATA *ch, char *argument)
{
  HELP_DATA *pHelp;
  BUFFER *buf;
  ITERATOR *pIter;
  char arg[MAX_INPUT_LENGTH];
  char keyword[MAX_INPUT_LENGTH];
  char *str;
  bool found = FALSE;
  int i = 0;

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    send_to_char("What kind of helpfiles are you searching for?\n\r", ch);
    return;
  }

  buf = buffer_new(MAX_STRING_LENGTH);

  bprintf(buf, "%s\n\r", get_dystopia_banner("Search Results", 80));
  bprintf(buf, " The following helpfile entries are the results when searching for\n\r");
  bprintf(buf, " the keyword '#G%s#n'. There may still exist helpfiles on the subject\n\r", arg);
  bprintf(buf, " which was not found with this search tool.\n\n\r");

  pIter = AllocIterator(help_list);
  while ((pHelp = (HELP_DATA *) NextInList(pIter)) != NULL)
  {
    bool match = FALSE;

    if (pHelp->level > ch->level)
      continue;

    if (SoundexMatch(GetSoundexKey(arg), GetSoundexKey(pHelp->name)) > 75)
      match = TRUE;
    else
    {
      str = pHelp->keyword;
      str = one_argument(str, keyword);
      while (keyword[0] != '\0')
      {
        if (!str_prefix(arg, keyword))
        {
          match = TRUE;
          break;
        }
        str = one_argument(str, keyword);
      }
    }

    if (match)
    {
      found = TRUE;
      bprintf(buf, "  %-22.22s%s", pHelp->name, (++i % 3) ? "  " : "\n\r");
    }
  }

  if (!found)
    bprintf(buf, "  Nothing found...\n\r");

  bprintf(buf, "%s\n\r%s\n\r", (i % 3) ? "\n" : "", get_dystopia_banner("", 80));
  send_to_char(buf->data, ch);
  buffer_free(buf);
}

void do_help(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  HELP_DATA *pHelp;

  if ((pHelp = get_help(ch, argument)) == NULL)
  {
    if (!check_help_soundex(argument, ch))
    {
      printf_to_char(ch, "%s\n\n\r", get_dystopia_banner("Nothing Found", 56));
      printf_to_char(ch, " There is no helpfile by the name '#G%s#n'.\n\r", argument);
      printf_to_char(ch, " Perhaps you should try the #Gsearch#n command.\n\r");
      printf_to_char(ch, "\n\r%s\n\r", get_dystopia_banner("", 56));
    }

    /* what help files does newbies want ? */
    if ((get_age(ch) - 17) < 2)
    {
      sprintf(buf, "Do_help: %s tried '%s'", ch->name, argument);
      log_string2("%s", buf);
    }
    return;
  }

  if (pHelp->level >= 0)
    printf_to_char(ch, "%s\n\r", get_dystopia_banner(pHelp->name, 80));
  else
    printf_to_char(ch, "%s\n\r", get_dystopia_banner("", 80));

  /* Strip leading '.' to allow initial blanks. */
  if (pHelp->text[0] == '.')
    show_help_player(pHelp->text+1, ch);
  else
    show_help_player(pHelp->text, ch);

  printf_to_char(ch, "\n\r%s\n\r", get_dystopia_banner("", 80));
}

void do_examine(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Examine what?\n\r", ch);
    return;
  }

  do_look(ch, arg);

  if ((obj = get_obj_here(ch, arg)) != NULL)
  {
    if (obj->condition >= 100)
      sprintf(buf, "You notice that %s is in perfect condition.\n\r", obj->short_descr);
    else if (obj->condition >= 75)
      sprintf(buf, "You notice that %s is in good condition.\n\r", obj->short_descr);
    else if (obj->condition >= 50)
      sprintf(buf, "You notice that %s is in average condition.\n\r", obj->short_descr);
    else if (obj->condition >= 25)
      sprintf(buf, "You notice that %s is in poor condition.\n\r", obj->short_descr);
    else
      sprintf(buf, "You notice that %s is in awful condition.\n\r", obj->short_descr);
    send_to_char(buf, ch);
    switch (obj->item_type)
    {
      default:
        break;

      case ITEM_DRINK_CON:
      case ITEM_CONTAINER:
      case ITEM_CORPSE_NPC:
      case ITEM_CORPSE_PC:
        send_to_char("When you look inside, you see:\n\r", ch);
        sprintf(buf, "in %s", arg);
        do_look(ch, buf);
    }
  }
}

void do_exits(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  EXIT_DATA *pexit;
  EVENT_DATA *event;
  ITERATOR *pIter;
  bool found;
  int door;
  int wlck_doors[4];

  /* reset warlock doors */
  for (door = 0; door < 4; door++)
    wlck_doors[door] = 0;

  buf[0] = '\0';

  if (!check_blind(ch))
    return;

  pIter = AllocIterator(ch->in_room->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
  {
    if (event->type == EVENT_ROOM_MISDIRECT)
    {
      char temp[MAX_INPUT_LENGTH];
      int newdir;
       
      one_argument(event->argument, temp);
      newdir = atoi(temp);

      wlck_doors[newdir] = 1;
    }
  }

  strcpy(buf, "Obvious exits:\n\r");

  found = FALSE;
  for (door = 0; door <= 5; door++)
  {
    if (((pexit = ch->in_room->exit[door]) != NULL && pexit->to_room != NULL) ||
         (door < 4 && wlck_doors[door] != 0))
    {
      found = TRUE;
      if (pexit)
      {
        sprintf(buf + strlen(buf), "%-5s - %s\n\r",
          capitalize(dir_name[door]), 
          room_is_dark(pexit->to_room) ? "Too dark to tell" : pexit->to_room->name);
      }
      else
      {
        sprintf(buf + strlen(buf), "%-5s - Unable to tell\n\r", capitalize(dir_name[door]));
      }
    }
  }

  if (!found)
    strcat(buf, "None.\n\r");

  send_to_char(buf, ch);
}

char *const day_name[] = {
  "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
  "the Great Gods", "the Sun"
};

char *const month_name[] = {
  "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
  "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
  "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
  "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time(CHAR_DATA * ch, char *argument)
{
  extern char str_boot_time[];
  char buf[MAX_STRING_LENGTH];
  char *suf;
  int day;

  day = time_info.day + 1;

  if (day > 4 && day < 20)
    suf = "th";
  else if (day % 10 == 1)
    suf = "st";
  else if (day % 10 == 2)
    suf = "nd";
  else if (day % 10 == 3)
    suf = "rd";
  else
    suf = "th";

  sprintf(buf,
          "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r"
          "Calim's Cradle started up at %s\rThe system time is %s\r",
          (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
          time_info.hour >= 12 ? "pm" : "am", day_name[day % 7], day, suf, month_name[time_info.month], str_boot_time, 
          (char *) ctime(&current_time));

  send_to_char(buf, ch);
}

void do_level(CHAR_DATA * ch, char *argument)
{
  BUFFER *buf, *buf2;
  char skill[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *pRoom;
  char *seperator = "#o-:|:-~-:|:-~-:|:-~-:|:-~-:|:--:|:-~-:|:-~-:|:-~-:|:-~-:|:-~-:|:-#n";

  if (IS_NPC(ch))
    return;

  buf = buffer_new(MAX_STRING_LENGTH);

  bprintf(buf, " Slice   #C: %-4d#n", ch->wpn[1]);
  bprintf(buf, "   Stab   #C: %-4d#n", ch->wpn[2]);
  bprintf(buf, "   Slash  #C: %-4d#n", ch->wpn[3]);
  bprintf(buf, "   Whip   #C: %-4d#n\n\r", ch->wpn[4]);
  bprintf(buf, " Claw    #C: %-4d#n", ch->wpn[5]);
  bprintf(buf, "   Blast  #C: %-4d#n", ch->wpn[6]);
  bprintf(buf, "   Pound  #C: %-4d#n", ch->wpn[7]);
  bprintf(buf, "   Crush  #C: %-4d#n\n\r", ch->wpn[8]);
  bprintf(buf, " Grep    #C: %-4d#n", ch->wpn[9]);
  bprintf(buf, "   Bite   #C: %-4d#n", ch->wpn[10]);
  bprintf(buf, "   Pierce #C: %-4d#n", ch->wpn[11]);
  bprintf(buf, "   Suck   #C: %-4d#n\n\r", ch->wpn[12]);
  bprintf(buf, " Unarmed #C: %-4d#n\n\r", ch->wpn[0]);

  bprintf(buf, "\n\r %s\n\n\r", seperator);

  bprintf(buf, " Viper   #C: %-4d#n", ch->stance[1]);
  bprintf(buf, "   Crane   #C: %-4d#n", ch->stance[2]);
  bprintf(buf, "  Crab   #C: %-4d#n", ch->stance[3]);
  bprintf(buf, "   Mongoose#C : %-4d#n\n\r", ch->stance[4]);
  bprintf(buf, " Bull    #C: %-4d#n", ch->stance[5]);
  bprintf(buf, "   Mantis  #C: %-4d#n", ch->stance[6]);
  bprintf(buf, "  Dragon #C: %-4d#n", ch->stance[7]);
  bprintf(buf, "   Tiger    #C: %-4d#n\n\r", ch->stance[8]);
  bprintf(buf, " Monkey  #C: %-4d#n", ch->stance[9]);
  bprintf(buf, "   Swallow #C: %-4d#n", ch->stance[10]);

  if (IS_CLASS(ch, CLASS_FAE))
    bprintf(buf, "  Spirit #C: %-4d\n\r", ch->stance[11]);
  else
    bprintf(buf, "\n\r");

  bprintf(buf, "\n\r %s\n\n\r", seperator);

  bprintf(buf, " #pPurple#n #C: %-4d", ch->spl[0]);  
  bprintf(buf, " #lBlue#n #C: %-4d", ch->spl[2]);
  bprintf(buf, " #rRed#n #C: %-4d", ch->spl[1]);  
  bprintf(buf, " #gGreen#n #C: %-4d", ch->spl[3]);
  bprintf(buf, " #yYellow#n #C: %-4d#n\n\r", ch->spl[4]);

  bprintf(buf, "\n\r %s\n\n\r", seperator);

  switch(ch->stance[STANCE_MOBSTANCE])
  {
    default:
      sprintf(skill, "None");
      break;
    case STANCE_VIPER:
      sprintf(skill, "Viper");
      break;
    case STANCE_BULL:
      sprintf(skill, "Bull");
      break;
    case STANCE_CRANE:
      sprintf(skill, "Crane");
      break;
    case STANCE_CRAB:
      sprintf(skill, "Crab");
      break;
    case STANCE_MONGOOSE:
      sprintf(skill, "Mongoose");
      break;
    case STANCE_MANTIS:
      sprintf(skill, "Mantis");
      break;
    case STANCE_DRAGON:
      sprintf(skill, "Dragon");
      break;
    case STANCE_TIGER:
      sprintf(skill, "Tiger");
      break;
    case STANCE_MONKEY:
      sprintf(skill, "Monkey");
      break;
    case STANCE_SWALLOW:
      sprintf(skill, "Swallow");
      break;
    case STANCE_SPIRIT:
      sprintf(skill, "Spirit");
      break;
  }
  switch(ch->stance[STANCE_PKSTANCE])
  {
    default:
      bprintf(buf, " PK Autostance   #C: %-12s#n Mob Autostance #C: %-12s#n\n\r", "None", skill);
      break;
    case STANCE_VIPER:
      bprintf(buf, " PK Autostance   #C: %-12s#n Mob Autostance #C: %-12s#n\n\r", "Viper", skill);
      break;
    case STANCE_BULL:
      bprintf(buf, " PK Autostance   #C: %-12s#n Mob Autostance #C: %-12s#n\n\r", "Bull", skill);
      break;
    case STANCE_CRANE:
      bprintf(buf, " PK Autostance   #C: %-12s#n Mob Autostance #C: %-12s#n\n\r", "Crane", skill);
      break;
    case STANCE_CRAB:
      bprintf(buf, " PK Autostance   #C: %-12s#n Mob Autostance #C: %-12s#n\n\r", "Crab", skill);
      break;
    case STANCE_MONGOOSE:
      bprintf(buf, " PK Autostance   #C: %-12s#n Mob Autostance #C: %-12s#n\n\r", "Mongoose", skill);
      break;
    case STANCE_TIGER:
      bprintf(buf, " PK Autostance   #C: %-12s#n Mob Autostance #C: %-12s#n\n\r", "Tiger", skill);
      break;
    case STANCE_MANTIS:
      bprintf(buf, " PK Autostance   #C: %-12s#n Mob Autostance #C: %-12s#n\n\r", "Mantis", skill);
      break;
    case STANCE_MONKEY:
      bprintf(buf, " PK Autostance   #C: %-12s#n Mob Autostance #C: %-12s#n\n\r", "Monkey", skill);
      break;
    case STANCE_SWALLOW:
      bprintf(buf, " PK Autostance   #C: %-12s#n Mob Autostance #C: %-12s#n\n\r", "Swallow", skill);
      break;
    case STANCE_DRAGON:
      bprintf(buf, " PK Autostance   #C: %-12s#n Mob Autostance #C: %-12s#n\n\r", "Dragon", skill);
      break;
    case STANCE_SPIRIT:
      bprintf(buf, " PK Autostance   #C: %-12s#n Mob Autostance #C: %-12s#n\n\r", "Spirit", skill);
      break;
  }

  bprintf(buf, " Generation      #C: %d#n         ", ch->generation);

  if (IS_SET(ch->act, PLR_RIGHTHAND))
    bprintf(buf, "   You favor your #Cright#n arm in combat\n\r");
  else if (IS_SET(ch->act, PLR_LEFTHAND))
    bprintf(buf, "   You favor your #Cleft#n arm in combat\n\r");
  else
    bprintf(buf, "   You fight well with #Cboth#n your arms\n\r");

  bprintf(buf, " 1st Recall Room #C:#n %s#n\n\r",
    ((pRoom = get_room_index(ch->home[0])) != NULL)
     ? string_restrict(pRoom->name, 45) : "Unknown");
  bprintf(buf, " 2nd Recall Room #C:#n %s#n\n\r",
    ((pRoom = get_room_index(ch->home[1])) != NULL)
     ? string_restrict(pRoom->name, 45) : "Unknown");
  bprintf(buf, " 3rd Recall Room #C:#n %s#n\n\r",
    ((pRoom = get_room_index(ch->home[2])) != NULL)
     ? string_restrict(pRoom->name, 45) : "Unknown");

  buf2 = box_text(buf->data, "Player Levels");
  send_to_char(buf2->data, ch);
  buffer_free(buf);
}

void do_who(CHAR_DATA *ch, char *argument)
{
  BUFFER *wholist;

  wholist = get_who(ch, argument);
  if (wholist != NULL)
  {
    send_to_char(wholist->data, ch);
    buffer_free(wholist);
  }
}

BUFFER *get_who(CHAR_DATA *ch, char *argument)
{
  BUFFER *wholist = buffer_new(MAX_STRING_LENGTH);
  const char *sexes[] = { "?", "M", "F" };
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;
  CHAR_DATA *gch;
  char arg[MAX_INPUT_LENGTH];
  char pkratio[MAX_STRING_LENGTH];
  char kav[MAX_STRING_LENGTH];
  char faith[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];  /* banners */
  char buf1[MAX_STRING_LENGTH]; /* Admin.  */
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  char buf4[MAX_STRING_LENGTH];
  char buf5[MAX_STRING_LENGTH];
  char buf6[MAX_STRING_LENGTH];
  char buf7[MAX_STRING_LENGTH];
  char buf8[MAX_STRING_LENGTH]; /* avatars : buf2 - buf17 */
  char buf9[MAX_STRING_LENGTH];
  char buf10[MAX_STRING_LENGTH];
  char buf11[MAX_STRING_LENGTH];
  char buf12[MAX_STRING_LENGTH];
  char buf13[MAX_STRING_LENGTH];
  char buf14[MAX_STRING_LENGTH];
  char buf15[MAX_STRING_LENGTH];
  char buf16[MAX_STRING_LENGTH];
  char buf17[MAX_STRING_LENGTH];
  char buf18[MAX_STRING_LENGTH];  /* mortals. */
  int nPlayerAll = 0;
  int nPlayerVis = 0;
  int nImmVis = 0;
  int mightRate, pkStatus;
  bool rangeOnly = FALSE;
  bool avatars = FALSE;
  bool a1 = FALSE;
  bool a2 = FALSE;
  bool a3 = FALSE;
  bool a4 = FALSE;
  bool a5 = FALSE;
  bool a6 = FALSE;
  bool a7 = FALSE;
  bool a8 = FALSE;
  bool a9 = FALSE;
  bool a10 = FALSE;
  bool a11 = FALSE;
  bool a12 = FALSE;
  bool a13 = FALSE;
  bool a14 = FALSE;
  bool a15 = FALSE;
  bool a16 = FALSE;
  bool a17 = FALSE;
  bool a18 = FALSE;

  if (ch && IS_NPC(ch))
  {
    buffer_free(wholist);
    return NULL;
  }

  one_argument(argument, arg);

  if (!str_cmp(arg, "kill"))
    rangeOnly = TRUE;

  buf1[0] = '\0';
  buf2[0] = '\0';
  buf3[0] = '\0';
  buf4[0] = '\0';
  buf5[0] = '\0';
  buf6[0] = '\0';
  buf7[0] = '\0';
  buf8[0] = '\0';
  buf9[0] = '\0';
  buf10[0] = '\0';
  buf11[0] = '\0';
  buf12[0] = '\0';
  buf13[0] = '\0';
  buf14[0] = '\0';
  buf15[0] = '\0';
  buf16[0] = '\0';
  buf17[0] = '\0';
  buf18[0] = '\0';

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    char const *title;

    if (d->connected != CON_PLAYING &&
      !(d->connected >= CON_NOTE_TO && d->connected <= CON_NOTE_FINISH))
      continue;

    if ((gch = d->character) == NULL)
      continue;

    /*
     * Immortals are not players, and should not be counted as such.
     */
    if (gch->level < 7)
      nPlayerAll++;

    /*
     * Visibility :-) (allows ch == NULL)
     */
    if (IS_SET(gch->act, PLR_HIDE) && !can_see(ch, gch))
      continue;

    if (gch->level < 7)
      nPlayerVis++;
    else
      nImmVis++;

    /*
     * The playerkill range is decided
     */
    if (ch)
      pkStatus = check_pkstatus(ch, gch);
    else
      pkStatus = 0;

    if (rangeOnly && !IS_SET(pkStatus, 1))
      continue;

    /*
     * status info, and cankill info
     */
    if (pkStatus == (1 + 2))
      sprintf(pkratio, "#R(#G%s#R)#n", sexes[URANGE(0, gch->sex, 2)]);
    else if (pkStatus == (2))
      sprintf(pkratio, "#0(#G%s#0)#n", sexes[URANGE(0, gch->sex, 2)]);
    else if (pkStatus == (1))
      sprintf(pkratio, "#y(#G%s#y)#n", sexes[URANGE(0, gch->sex, 2)]);
    else
      sprintf(pkratio, "#L(#G%s#L)#n", sexes[URANGE(0, gch->sex, 2)]);

    /*
     * Now let's parse the title.
     */
    mightRate = getMight(gch);

    if (d->connected >= CON_NOTE_TO && d->connected <= CON_NOTE_FINISH)
      title = "#yWriting#n";
    else if (IS_EXTRA(gch, EXTRA_AFK))  
      title = "#C*#0**A#CF#0K**#C*#n";
    else
    {
      switch (gch->level)
      {
        default:
          title = " ";
          break;
        case MAX_LEVEL:
          title = "#RC#0oder#n";
          break;
        case MAX_LEVEL - 1:
          title = "#RH#0igh #RJ#0udge#n";
          break;
        case MAX_LEVEL - 2:
          title = "#RJ#0udge#n";
	  break;
        case MAX_LEVEL - 3:
          if (!str_cmp(gch->name, "Archanos"))
            title = "#RH#0ead #RB#0uilder#n";
          else
            title = "#RE#0nforcer#n";
          break;
        case MAX_LEVEL - 4:
          title = "#RB#0uilder#n";
          break;
        case MAX_LEVEL - 5:
          title = "#RQ#0uestor#n";
          break;
        case MAX_LEVEL - 6:
        case MAX_LEVEL - 7:
        case MAX_LEVEL - 8:
        case MAX_LEVEL - 9:
          if (mightRate >= RANK_ALMIGHTY)
            title = "#9Almighty#n";
          else if (mightRate >= RANK_RULER)
            title = "#CRuler#n";
          else if (mightRate >= RANK_SUPREME)
            title = "#oSupreme#n";
          else if (mightRate >= RANK_KING)
          {
            if (gch->sex == SEX_FEMALE)
              title = "#pQueen#n";
            else
              title = "#pKing#n";
          }
          else if (mightRate >= RANK_BARON)
          {
            if (gch->sex == SEX_FEMALE)
              title = "#RBaroness#n";
            else
              title = "#RBaron#n";
          }
          else if (mightRate >= RANK_DUKE)
          {
            if (gch->sex == SEX_FEMALE)
              title = "#yDutchess#n";
            else
              title = "#yDuke#n";
          }
          else if (mightRate >= RANK_GENERAL)
            title = "#gGeneral#n";
          else if (mightRate >= RANK_CAPTAIN)
            title = "#CCaptain#n";
          else if (mightRate >= RANK_MASTER)
          {
            if (gch->sex == SEX_FEMALE)
              title = "#RMistress#n";
            else
              title = "#RMaster#n";
          }
          else if (mightRate >= RANK_LEGENDARY)
            title = "#yLegendary#n";
          else if (mightRate >= RANK_HERO)
          {
            if (gch->sex == SEX_FEMALE)
              title = "#GHeroine#n";
            else
              title = "#GHero#n";
          }
          else if (mightRate >= RANK_ADVENTURER)
            title = "#LAdventurer#n";
          else if (mightRate >= RANK_VETERAN)
            title = "#rVeteran#n";
          else if (mightRate >= RANK_PRIVATE)
            title = "#yPrivate#n";
          else if (mightRate >= RANK_CADET)
            title = "#oCadet#n";
          else
            title = "#pWannabe#n";
          break;
        case MAX_LEVEL - 10:
          title = "#RMortal#n";
          break;
        case MAX_LEVEL - 11:
          title = "#CNewbie#n";
          break;
        case MAX_LEVEL - 12:
          title = "#CUndefined#n";
          break;
      }
    }

    /*
     * then the class name.
     */
    switch(gch->class)
    {
      default:
        sprintf(kav, "[None]");
        break;
      case CLASS_SHADOW:
        if (IS_SET(gch->pcdata->powers[EVOLVE_1], SHADOW_EVOLVE_ASSASSIN))
          sprintf(kav, "#y-*(#9A#0ssassin#y)*-#n");
        else if (IS_SET(gch->pcdata->powers[EVOLVE_1], SHADOW_EVOLVE_SKULLDUGGERY))
          sprintf(kav, "#y-*(#9S#0hadow #9T#0hief#y)*-#n");
        else
          sprintf(kav, "#y-*(#9S#0hadowlord#y)*-#n");
        break;        
      case CLASS_WARLOCK:
        switch(gch->pcdata->powers[WARLOCK_RANK])
        {
          default:
            if (gch->pcdata->powers[WARLOCK_PATH] == PATH_NECROMANCY)
              sprintf(kav, "#y/#R<#G-#PNecromancer#G-#R>#y\\#n");
            else if (gch->pcdata->powers[WARLOCK_PATH] == PATH_DIVINATION)
              sprintf(kav, "#y/#R<#G-#PDiviner#G-#R>#y\\#n");
            else if (gch->pcdata->powers[WARLOCK_PATH] == PATH_ENCHANTMENT)
              sprintf(kav, "#y/#R<#G-#PEnchanter#G-#R>#y\\#n");
            else if (gch->pcdata->powers[WARLOCK_PATH] == PATH_ABJURATION)
              sprintf(kav, "#y/#R<#G-#PAbjurer#G-#R>#y\\#n");
            else if (gch->pcdata->powers[WARLOCK_PATH] == PATH_SUMMONING)
              sprintf(kav, "#y/#R<#G-#PSummoner#G-#R>#y\\#n");
            else if (gch->pcdata->powers[WARLOCK_PATH] == PATH_INVOCATION)
              sprintf(kav, "#y/#R<#G-#PInvoker#G-#R>#y\\#n");
            else
              sprintf(kav, "#y/#R<#G-#PWarlock#G-#R>#y\\#n");
            break;
          case WLCK_RNK_APPRENTICE:
            sprintf(kav, "#y/#R<#G-#PApprentice#G-#R>#y\\#n");
            break;
          case WLCK_RNK_ARCHMAGE:
            sprintf(kav, "#y/#R<#G-#PArchmage#G-#R>#y\\#n");
            break;
        }
        break;
      case CLASS_GIANT:
        if (IS_SET(gch->pcdata->powers[EVOLVE_1], GIANT_EVOLVE_WARRIOR))
          sprintf(kav, "#C<>#GG#gian#Gt W#garrio#Gr#C<>#n");
        else if (IS_SET(gch->pcdata->powers[EVOLVE_1], GIANT_EVOLVE_SHAMAN))
          sprintf(kav, "#C<>#GG#gian#Gt S#ghama#Gn#C<>#n");
        else
          sprintf(kav, "#C<>#GG#gian#Gt#C<>#n");
        break;
      case CLASS_FAE:
        if (gch->pcdata->powers[FAE_PATH] == FAE_PLASMA)
          sprintf(kav, "#G<<#R(#yPlasma Fae#R)#G>>#n");
        else if (gch->pcdata->powers[FAE_PATH] == FAE_ENERGY)
          sprintf(kav, "#G<<#R(#yEnergy Fae#R)#G>>#n");
        else if (gch->pcdata->powers[FAE_PATH] == FAE_WILL)
          sprintf(kav, "#G<<#R(#yWillpower Fae#R)#G>>#n");
        else if (gch->pcdata->powers[FAE_PATH] == FAE_MATTER)
          sprintf(kav, "#G<<#R(#yMatter Fae#R)#G>>#n");
        else
          sprintf(kav, "#G<<#R(#yFae#R)#G>>#n");
        break;
    }

    /* clan, religion, kingdom, etc */
    sprintf(faith, "%s", get_kingdomname(gch->pcdata->kingdom));

    /* 
     * Let's figure out where to place the player.
     */
    if (gch->level > 6)
    {
      cprintf(buf1 + strlen(buf1), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
      a1 = TRUE;
    }
    else if (gch->level >= 3 && gch->level <= 6)
    {
      if (mightRate >= RANK_ALMIGHTY)
      {
        cprintf(buf2 + strlen(buf2), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a2 = TRUE;
        avatars = TRUE;
      }
      else if (mightRate >= RANK_RULER)
      {
        cprintf(buf3 + strlen(buf3), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a3 = TRUE;
        avatars = TRUE;
      }
      else if (mightRate >= RANK_SUPREME)
      {
        cprintf(buf4 + strlen(buf4), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a4 = TRUE;
        avatars = TRUE;
      }
      else if (mightRate >= RANK_KING)
      {
        cprintf(buf5 + strlen(buf5), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a5 = TRUE;
        avatars = TRUE;
      }
      else if (mightRate >= RANK_BARON)
      {
        cprintf(buf6 + strlen(buf6), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a6 = TRUE;
        avatars = TRUE;
      }
      else if (mightRate >= RANK_DUKE)
      {
        cprintf(buf7 + strlen(buf7), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a7 = TRUE;
        avatars = TRUE;
      }
      else if (mightRate >= RANK_GENERAL)
      {
        cprintf(buf8 + strlen(buf8), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a8 = TRUE;
        avatars = TRUE;
      }
      else if (mightRate >= RANK_CAPTAIN)
      {
        cprintf(buf9 + strlen(buf9), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a9 = TRUE;
        avatars = TRUE;
      }
      else if (mightRate >= RANK_MASTER)
      {
        cprintf(buf10 + strlen(buf10), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a10 = TRUE;
        avatars = TRUE;
      }
      else if (mightRate >= RANK_LEGENDARY)
      {
        cprintf(buf11 + strlen(buf11), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a11 = TRUE;
        avatars = TRUE;
      }
      else if (mightRate >= RANK_HERO)
      {
        cprintf(buf12 + strlen(buf12), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a12 = TRUE;
        avatars = TRUE;
      }
      else if (mightRate >= RANK_ADVENTURER)
      {
        cprintf(buf13 + strlen(buf13), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a13 = TRUE;
        avatars = TRUE;
      }
      else if (mightRate >= RANK_VETERAN)
      {
        cprintf(buf14 + strlen(buf14), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a14 = TRUE;
        avatars = TRUE;
      }
      else if (mightRate >= RANK_PRIVATE)
      {
        cprintf(buf15 + strlen(buf15), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a15 = TRUE;
        avatars = TRUE;
      }
      else if (mightRate >= RANK_CADET)
      {
        cprintf(buf16 + strlen(buf16), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a16 = TRUE;
        avatars = TRUE;
      }
      else
      {
        cprintf(buf17 + strlen(buf17), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
        a17 = TRUE;
        avatars = TRUE;
      }
    }
    else if (gch->level < 3)
    {
      cprintf(buf18 + strlen(buf18), " %-13s %-4s %-22s %-12s %s\n\r", title, pkratio, kav, gch->name, faith);
      a18 = TRUE;
    }
  }

  /*   
   * Let's send the whole thing to the player.
   */
  bprintf(wholist, " %s\n\r", get_dystopia_banner("Calim's Cradle", 76));

  if (a1)
  {
    sprintf(buf, "\n\r %s\n\r", get_dystopia_banner("Gods of Calim's Cradle", 76));
    bprintf(wholist, "%s", buf);
    bprintf(wholist, "%s", buf1);
  }
  if (avatars)
  {
    sprintf(buf, "\n\r %s\n\r", get_dystopia_banner("    Avatars     ", 76));
    bprintf(wholist, "%s", buf);
    if (a2)
      bprintf(wholist, "%s", buf2);
    if (a3)
      bprintf(wholist, "%s", buf3);
    if (a4)
      bprintf(wholist, "%s", buf4);
    if (a5)
      bprintf(wholist, "%s", buf5);
    if (a6)
      bprintf(wholist, "%s", buf6);
    if (a7)
      bprintf(wholist, "%s", buf7);
    if (a8)
      bprintf(wholist, "%s", buf8);
    if (a9)
      bprintf(wholist, "%s", buf9);
    if (a10)
      bprintf(wholist, "%s", buf10);
    if (a11)
      bprintf(wholist, "%s", buf11);
    if (a12)
      bprintf(wholist, "%s", buf12);
    if (a13)
      bprintf(wholist, "%s", buf13);
    if (a14)
      bprintf(wholist, "%s", buf14);
    if (a15)
      bprintf(wholist, "%s", buf15);
    if (a16)
      bprintf(wholist, "%s", buf16);
    if (a17)
      bprintf(wholist, "%s", buf17);
  }
  if (a18)
  {
    sprintf(buf, "\n\r %s\n\r", get_dystopia_banner("    Mortals     ", 76));
    bprintf(wholist, "%s", buf);
    bprintf(wholist, "%s", buf18);
  }
  sprintf(buf, "\n\r %s\n\r", get_dystopia_banner("", 76));
  bprintf(wholist, "%s", buf);
  sprintf(buf, "   #C%d#0/#C%d #Gvisible player%s and #C%d #Gvisible immortal%s connected to Calim's Cradle#n\n\r",
    nPlayerVis, nPlayerAll, (nPlayerAll == 1) ? "" : "s", nImmVis, (nImmVis == 1) ? "" : "s");
  bprintf(wholist, "%s", buf);
  sprintf(buf, " %s\n\r", get_dystopia_banner("", 76));
  bprintf(wholist, "%s", buf);

  return wholist;
}

void do_inventory(CHAR_DATA * ch, char *argument)
{
  send_to_char("You are carrying:\n\r", ch);
  show_list_to_char(ch->carrying, ch, TRUE, TRUE);
}

void do_equipment(CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj;
  int iWear;

  send_to_char("You are using:\n\r", ch);
  for (iWear = 0; iWear < MAX_WEAR; iWear++)
  {
    obj = get_eq_char(ch, iWear);

    if (obj == NULL)
    {
      if (IS_CLASS(ch, CLASS_FAE) && iWear != WEAR_HOLD && iWear != WEAR_WIELD && iWear != WEAR_MASTERY && iWear != WEAR_LIGHT)
        continue;

      if (iWear == WEAR_THIRD && !IS_SET(ch->newbits, THIRD_HAND)) continue;
      if (iWear == WEAR_FOURTH && !IS_SET(ch->newbits, FOURTH_HAND)) continue;

      send_to_char(where_name[iWear], ch);
      send_to_char("nothing.\n\r", ch);
    }
    else if (can_see_obj(ch, obj))
    {
      send_to_char(where_name[iWear], ch);
      send_to_char(format_obj_to_char(obj, ch, TRUE), ch);
      send_to_char("\n\r", ch);
    }
    else
    {
      send_to_char(where_name[iWear], ch);
      send_to_char("something.\n\r", ch);
    }
  }
}

void do_credits(CHAR_DATA * ch, char *argument)
{
  do_help(ch, "diku");
  return;
}

void do_wizlist(CHAR_DATA * ch, char *argument)
{
  do_help(ch, "wizlist");
  return;
}

void do_where(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *d;
  bool found;

  if (ch->in_room == NULL)
    return;

  printf_to_char(ch, "#CArea   :#n       %s\n\r", ch->in_room->area->name);
  printf_to_char(ch, "#CRoom   :#n       %s\n\n\r", ch->in_room->name);

  one_argument(argument, arg);
  if (arg[0] == '\0')
  {
    ITERATOR *pIter;

    send_to_char("#CPlayers near you:#n\n\r", ch);
    found = FALSE;
    pIter = AllocIterator(descriptor_list);
    while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
    {
      if ((d->connected == CON_PLAYING) &&
          (victim = d->character) != NULL && !IS_NPC(victim) && victim != ch &&
           victim->in_room != NULL && victim->in_room->area == ch->in_room->area &&
           can_see(ch, victim))
      {
        /* chameleon 66% invis */
        if (IS_CLASS(victim, CLASS_FAE) && IS_SET(victim->newbits, NEW_CHAMELEON))
        {
          if (number_range(1, 3) != 2) continue;
        }

        found = TRUE;
        sprintf(buf, "%-14s %s\n\r", victim->name, victim->in_room->name);
        send_to_char(buf, ch);
      }
    }
    if (!found)
      send_to_char("Noone.\n\r", ch);
  }
  else
  {
    if ((victim = get_char_area(ch, arg)) != NULL)
    {
      printf_to_char(ch, "%-28s %s\n\r", PERS(victim, ch), victim->in_room->name);
      return;
    }

    act("You didn't find any $T.", ch, NULL, arg, TO_CHAR);
  }
}

/*
 * Dystopian consider
 */
void do_consider(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int pMight, vMight;

  one_argument(argument, arg);

  if (IS_NPC(ch))
    return;

  if (arg[0] == '\0')
  {
    send_to_char("Consider killing whom?\n\r", ch);
    return;
  }
  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("They're not here.\n\r", ch);
    return;
  }
  act("You examine $N closely, looking for $S weaknesses.", ch, NULL, victim, TO_CHAR);
  act("$n examine $N closely, looking for $S weaknesses.", ch, NULL, victim, TO_NOTVICT);
  act("$n examines you closely, looking for your weaknesses.", ch, NULL, victim, TO_VICT);

  pMight = getMight(ch);

  if (IS_NPC(victim))
  {
    int level = getMobMight(victim->pIndexData);

    if (level < 100)
    {
      if (pMight < RANK_CADET)
        send_to_char("It's one mean bugger, but you think you can take it.\n\r", ch);
      else
        send_to_char("Shouldn't be much of a problem beating that one.\n\r", ch);
    }
    else if (level < 300)
    {
      if (pMight < RANK_PRIVATE)
        send_to_char("No chance, it'll cream you.\n\r", ch);
      else if (pMight < RANK_ADVENTURER)
        send_to_char("It's one mean bugger, but you think you can take it.\n\r", ch);
      else
        send_to_char("Shouldn't be much of a problem beating that one.\n\r", ch);
    }
    else if (level < 1000)
    {
      if (pMight < RANK_CAPTAIN)
        send_to_char("No chance, it'll cream you.\n\r", ch);
      else if (pMight < RANK_DUKE)
        send_to_char("It's one mean bugger, but you think you can take it.\n\r", ch);
      else
        send_to_char("Shouldn't be much of a problem beating that one.\n\r", ch);
    }
    else if (level < 2000)
    {
      if (pMight < RANK_GENERAL)
        send_to_char("No chance, it'll cream you.\n\r", ch);
      else if (pMight < RANK_BARON)
        send_to_char("It's one mean bugger, but you think you can take it.\n\r", ch);
      else
        send_to_char("Shouldn't be much of a problem beating that one.\n\r", ch);
    }
    else
    {
      if (pMight < RANK_DUKE)
        send_to_char("No chance, it'll cream you.\n\r", ch);
      else if (pMight < RANK_SUPREME)
        send_to_char("It's one mean bugger, but you think you can take it.\n\r", ch);
      else
        send_to_char("Shouldn't be much of a problem beating that one.\n\r", ch);
    }
    return;
  }

  vMight = getMight(victim);

  if (pMight > 1.5 * vMight)
  {
    act("That would be to easy, better find someone worthy of the challenge.", ch, NULL, victim, TO_CHAR);
  }
  else if (pMight > 1.1 * vMight)
  {
    act("Shouldn't be to hard, but take care.", ch, NULL, victim, TO_CHAR);
  }
  else if (pMight > 0.9 * vMight)
  {
    act("Seems like a fair fight, you should give it a try.", ch, NULL, victim, TO_CHAR);
  }
  else if (pMight > 0.75 * vMight)
  {
    act("With a little luck, you might beat $N.", ch, NULL, victim, TO_CHAR);
  }
  else if (pMight > 0.6 * vMight)
  {
    act("$N will give you a solid beating.", ch, NULL, victim, TO_CHAR);
  }
  else
  {
    act("RUN!!! $N will rip out your heart before you land your first blow", ch, NULL, victim, TO_CHAR);
  }
}

void set_title(CHAR_DATA * ch, char *title)
{
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
  {
    bug("Set_title: NPC.", 0);
    return;
  }

  if (isalpha(title[0]) || isdigit(title[0]))
  {
    buf[0] = ' ';
    strcpy(buf + 1, title);
  }
  else
  {
    strcpy(buf, title);
  }

  free_string(ch->pcdata->title);
  ch->pcdata->title = str_dup(buf);
}

void do_title(CHAR_DATA * ch, char *argument)
{

  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
    return;

  if (argument[0] == '\0')
  {
    send_to_char("Change your title to what?\n\r", ch);
    return;
  }

  if (strlen(argument) > 25)
    argument[25] = '\0';

  smash_tilde(argument);
  sprintf(buf, "#n");
  strcpy(buf, argument);
  set_title(ch, argument);
  send_to_char("Ok.\n\r", ch);
}

bool event_player_afk(EVENT_DATA *event)
{
  CHAR_DATA *ch;

  if ((ch = event->owner.ch) == NULL)
  {
    bug("event_player_afk: no owner.", 0);
    return FALSE;
  }

  if (ch->fight_timer > 0)
  {
    send_to_char("You fail to go AFK with a fighttimer.\n\r", ch);
    return FALSE;
  }

  if (ch->position != POS_STANDING)
  {
    send_to_char("You must be standing to go AFK.\n\r", ch);
    return FALSE;
  }

  if (ch->master != NULL)
    stop_follower(ch, FALSE);

  SET_BIT(ch->extra, EXTRA_AFK);

  send_to_char("You are now AFK!\n\r", ch);
  act("$n is now AFK!", ch, NULL, NULL, TO_ROOM);

  return FALSE;
}

void do_afk(CHAR_DATA * ch, char *argument)
{
  EVENT_DATA *event;

  if (IS_NPC(ch)) return;

  if (IS_SET(ch->extra, EXTRA_AFK))
  {
    send_to_char("You are no longer AFK.\n\r", ch);
    REMOVE_BIT(ch->extra, EXTRA_AFK);
    return;
  }

  if (event_isset_mobile(ch, EVENT_PLAYER_AFK))
  {
    send_to_char("You are already trying to go AFK!\n\r", ch);
    return;
  }

  event = alloc_event();
  event->fun = &event_player_afk;
  event->type = EVENT_PLAYER_AFK;
  add_event_char(event, ch, 3 * PULSE_PER_SECOND);

  send_to_char("You attempt to go AFK!\n\r", ch);
}

void do_description(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];

  if (argument[0] != '\0')
  {
    buf[0] = '\0';
    smash_tilde(argument);

    if (argument[0] == '+')
    {
      if (ch->description != NULL)
        strcat(buf, ch->description);
      argument++;

      while (isspace(*argument))
        argument++;
    }

    if (collen(argument) > 80)
    {
      send_to_char("No more than 80 chars per line, thanks.\n\r", ch);
      return;
    }

    if (strlen2(buf) + strlen2(argument) >= MAX_STRING_LENGTH - 2)
    {
      send_to_char("Description too long.\n\r", ch);
      return;
    }

    strcat(buf, argument);
    strcat(buf, "\n\r");
    free_string(ch->description);
    ch->description = str_dup(buf);
  }

  send_to_char("Your description is:\n\r", ch);
  send_to_char(ch->description ? ch->description : "(None).\n\r", ch);
}

void do_report(CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *vch;
  ITERATOR *pIter;
  char buf[MAX_STRING_LENGTH];
  char hit_str[MAX_INPUT_LENGTH];
  char mana_str[MAX_INPUT_LENGTH];
  char move_str[MAX_INPUT_LENGTH];
  char mhit_str[MAX_INPUT_LENGTH];
  char mmana_str[MAX_INPUT_LENGTH];
  char mmove_str[MAX_INPUT_LENGTH];
  char exp_str[MAX_INPUT_LENGTH];

  sprintf(hit_str, "%s", col_scale(ch->hit, ch->max_hit));
  sprintf(mana_str, "%s", col_scale(ch->mana, ch->max_mana));
  sprintf(move_str, "%s", col_scale(ch->move, ch->max_move));
  sprintf(exp_str, "%d", ch->exp);
  sprintf(mhit_str, "#C%d#n", ch->max_hit);
  sprintf(mmana_str, "#C%d#n", ch->max_mana);
  sprintf(mmove_str, "#C%d#n", ch->max_move);
  sprintf(buf, "You report: %s/%s hp %s/%s mana %s/%s mv %s xp.\n\r",
    hit_str, mhit_str, mana_str, mmana_str, move_str, mmove_str, exp_str);

  send_to_char(buf, ch);

  pIter = AllocIterator(char_list);
  while ((vch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (vch == NULL)
      continue;
    if (vch == ch)
      continue;
    if (vch->in_room == NULL)
      continue;
    if (vch->in_room != ch->in_room)
      continue;

    sprintf(hit_str, "%s", col_scale(ch->hit, ch->max_hit));
    sprintf(mana_str, "%s", col_scale(ch->mana, ch->max_mana));
    sprintf(move_str, "%s", col_scale(ch->move, ch->max_move));
    sprintf(exp_str, "%s", col_scale(ch->exp, 1000000));
    sprintf(mhit_str, "#C%d#n", ch->max_hit);
    sprintf(mmana_str, "#C%d#n", ch->max_mana);
    sprintf(mmove_str, "#C%d#n", ch->max_move);
    if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_POLYMORPH))
      sprintf(buf, "%s reports: %s/%s hp %s/%s mana %s/%s mv %s xp.\n\r", ch->morph, hit_str, mhit_str, mana_str, mmana_str, move_str, mmove_str, exp_str);
    else
      sprintf(buf, "%s reports: %s/%s hp %s/%s mana %s/%s mv %s xp.\n\r", IS_NPC(ch) ? capitalize(ch->short_descr) : ch->name, hit_str, mhit_str, mana_str, mmana_str, move_str, mmove_str, exp_str);
    buf[0] = UPPER(buf[0]);
    send_to_char(buf, vch);
  }
}

void do_practice(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int sn;

  if (IS_NPC(ch))
    return;

  if (argument[0] == '\0')
  {
    int col, i;

    col = 0;
    for (i = 0; i < 5; i++)
    {
      for (sn = 0; sn < MAX_SKILL; sn++)
      {
        if (skill_table[sn].name == NULL)
          break;
        if (ch->level < skill_table[sn].skill_level)
          continue;
        if (skill_table[sn].target != i)
          continue;
        if (skill_table[sn].spell_fun == spell_null)
          continue;

        if (can_use_skill(ch, sn))
        {
          switch (i)
          {
            case 0:
              sprintf(buf, "#p%18s #9%3d%%  ", skill_table[sn].name, ch->pcdata->learned[sn]);
              break;
            case 1:
              sprintf(buf, "#R%18s #9%3d%%  ", skill_table[sn].name, ch->pcdata->learned[sn]);
              break;
            case 2:
              sprintf(buf, "#L%18s #9%3d%%  ", skill_table[sn].name, ch->pcdata->learned[sn]);
              break;
            case 3:
              sprintf(buf, "#G%18s #9%3d%%  ", skill_table[sn].name, ch->pcdata->learned[sn]);
              break;
            case 4:
              sprintf(buf, "#y%18s #9%3d%%  ", skill_table[sn].name, ch->pcdata->learned[sn]);
              break;
            default:
              sprintf(buf, " ");
              break;
          }
          send_to_char(buf, ch);
          if (++col % 3 == 0)
            send_to_char("\n\r", ch);
        }
      }
    }

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (skill_table[sn].name == NULL)
        break;
      if (ch->level < skill_table[sn].skill_level)
        continue;
      if (skill_table[sn].spell_fun != spell_null)
        continue;

      if (can_use_skill(ch, sn))
      {
        sprintf(buf, "#C%18s #9%3d%%  ", skill_table[sn].name, ch->pcdata->learned[sn]);
        send_to_char(buf, ch);

        if (++col % 3 == 0)
          send_to_char("\n\r", ch);
      }
    }
    send_to_char("#n\n\r", ch);

    if (col % 3 != 0)
      send_to_char("\n\r", ch);

    sprintf(buf, "You have %d exp left.\n\r", ch->exp);
    send_to_char(buf, ch);
  }
  else if (!strcmp(argument, "all"))
  {
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (ch->exp < 5000)
        break;

      if ((skill_table[sn].name != NULL) && (ch->level >= skill_table[sn].skill_level) && ch->pcdata->learned[sn] < 100)
      {
        ch->pcdata->learned[sn] = 100;
        ch->exp -= 5000;
      }
    }

    send_to_char("Ok.\n\r", ch);
    return;
  }
  else
  {
    if (!IS_AWAKE(ch))
    {
      send_to_char("In your dreams, or what?\n\r", ch);
      return;
    }

    if (ch->exp <= 0)
    {
      send_to_char("You have no exp left.\n\r", ch);
      return;
    }

    if ((sn = skill_lookup(argument)) < 0 || (!IS_NPC(ch) && ch->level < skill_table[sn].skill_level))
    {
      send_to_char("You can't practice that.\n\r", ch);
      return;
    }

    if (can_use_skill(ch, sn))
    {
      if (ch->pcdata->learned[sn] >= SKILL_ADEPT)
      {
        sprintf(buf, "You are already an adept of %s.\n\r", skill_table[sn].name);
        send_to_char(buf, ch);
      }
      else if (ch->exp < 5000)
      {
        sprintf(buf, "You need 5000 exp to increase %s any more.\n\r", (skill_table[sn].name));
        send_to_char(buf, ch);
      }
      else if (ch->pcdata->learned[sn] < SKILL_ADEPT)
      {
        ch->exp -= 5000;
        ch->pcdata->learned[sn] = SKILL_ADEPT;
        act("You are now an adept of $T.", ch, NULL, skill_table[sn].name, TO_CHAR);
      }
    }
    else
    {
      send_to_char("You can't practice that.\n\r", ch);
      return;
    }
  }
}

void do_socials(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int iSocial;
  int col;

  col = 0;

  for (iSocial = 0; social_table[iSocial].name != NULL && social_table[iSocial].name[0] != '\0'; iSocial++)
  {
    sprintf(buf, "%-12s", social_table[iSocial].name);
    send_to_char(buf, ch);
    if (++col % 6 == 0)
      send_to_char("\n\r", ch);
  }

  if (col % 6 != 0)
    send_to_char("\n\r", ch);
}

void do_xsocials(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int iSocial;
  int col;

  col = 0;

  for (iSocial = 0; xsocial_table[iSocial].name[0] != '\0'; iSocial++)
  {
    sprintf(buf, "%-12s", xsocial_table[iSocial].name);
    send_to_char(buf, ch);
    if (++col % 6 == 0)
      send_to_char("\n\r", ch);
  }

  if (col % 6 != 0)
    send_to_char("\n\r", ch);
}

void do_spells(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int sn;
  int col;

  col = 0;
  for (sn = 0; sn < MAX_SKILL && skill_table[sn].name != NULL; sn++)
  {
    sprintf(buf, "%-12s", skill_table[sn].name);
    send_to_char(buf, ch);
    if (++col % 6 == 0)
      send_to_char("\n\r", ch);
  }

  if (col % 6 != 0)
    send_to_char("\n\r", ch);
}

/*
 * Contributed by Alander.
 */
void do_commands(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int cmd;
  int col;

  col = 0;
  for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
  {
    if (cmd_table[cmd].level == 0 && cmd_table[cmd].level <= get_trust(ch))
    {
      sprintf(buf, "%-12s", cmd_table[cmd].name);
      send_to_char(buf, ch);
      if (++col % 6 == 0)
        send_to_char("\n\r", ch);
    }
  }

  if (col % 6 != 0)
    send_to_char("\n\r", ch);
}

void do_channels(CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  if (IS_NPC(ch))
    return;
  if (arg[0] == '\0')
  {
    send_to_char("Channels:", ch);
    send_to_char(!IS_SET(ch->deaf, CHANNEL_CHAT) ? " +CHAT" : " -chat", ch);
    send_to_char(!IS_SET(ch->deaf, CHANNEL_FLAME) ? " +FLAME" : " -flame", ch);
    send_to_char(!IS_SET(ch->deaf, CHANNEL_NEWBIE) ? " +NEWBIE" : " -newbie", ch);
    if (IS_IMMORTAL(ch))
    {
      send_to_char(!IS_SET(ch->deaf, CHANNEL_IMMTALK) ? " +IMMTALK" : " -immtalk", ch);
      send_to_char(!IS_SET(ch->deaf, CHANNEL_LOG) ? " +LOG" : " -log", ch);
    }
    send_to_char(!IS_SET(ch->deaf, CHANNEL_MUSIC) ? " +MUSIC" : " -music", ch);
    if (ch->class != 0 || IS_IMMORTAL(ch))
    {
      send_to_char(!IS_SET(ch->deaf, CHANNEL_CLASS) ? " +CLASS" : " -class", ch);
    }
    send_to_char(!IS_SET(ch->deaf, CHANNEL_INFO) ? " +INFO" : " -info", ch);
    send_to_char(!IS_SET(ch->deaf, CHANNEL_TELL) ? " +TELL" : " -tell", ch);

    if (get_kingdom(ch) != NULL)
    {
      send_to_char(!IS_SET(ch->deaf, CHANNEL_KINGDOM) ? " +KTALK" : " -ktell", ch);
    }

    send_to_char(".\n\r", ch);
  }
  else
  {
    bool fClear;
    int bit;

    if (arg[0] == '+')
      fClear = TRUE;
    else if (arg[0] == '-')
      fClear = FALSE;
    else
    {
      send_to_char("Channels -channel or +channel?\n\r", ch);
      return;
    }

    if (!str_cmp(arg + 1, "chat"))
      bit = CHANNEL_CHAT;
    else if (!str_cmp(arg + 1, "immtalk"))
      bit = CHANNEL_IMMTALK;
    else if (!str_cmp(arg + 1, "flame"))
      bit = CHANNEL_FLAME;
    else if (!str_cmp(arg + 1, "music"))
      bit = CHANNEL_MUSIC;
    else if (!str_cmp(arg + 1, "newbie"))
      bit = CHANNEL_NEWBIE;
    else if (!str_cmp(arg + 1, "yell"))
      bit = CHANNEL_YELL;
    else if (!str_cmp(arg + 1, "class"))
      bit = CHANNEL_CLASS;
    else if (IS_IMMORTAL(ch) && !str_cmp(arg + 1, "log"))
      bit = CHANNEL_LOG;
    else if (get_kingdom(ch) != NULL && !str_cmp(arg + 1, "ktalk"))
      bit = CHANNEL_KINGDOM;
    else if (!str_cmp(arg + 1, "info"))
      bit = CHANNEL_INFO;
    else if (!str_cmp(arg + 1, "tell"))
      bit = CHANNEL_TELL;
    else
    {
      send_to_char("Set or clear which channel?\n\r", ch);
      return;
    }

    if (fClear)
      REMOVE_BIT(ch->deaf, bit);
    else
      SET_BIT(ch->deaf, bit);

    send_to_char("Ok.\n\r", ch);
  }
}

/*
 * Contributed by Grodyn.
 */
void do_config(CHAR_DATA * ch, char *argument)
{
  BUFFER *buf;
  char arg[MAX_INPUT_LENGTH];
  int bit;

  if (IS_NPC(ch)) return;

  one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    buf = buffer_new(MAX_STRING_LENGTH);

    bprintf(buf, " <<----------------  System Configuration  ---------------->><BR><BR>");

    bprintf(buf, "  [%s] <SEND href=\"config ansi\">Ansi</SEND>      - %s<BR>",
      (IS_SET(ch->act, PLR_ANSI)) ? "o" : " ",
      (IS_SET(ch->act, PLR_ANSI)) ? "You are using ANSI colors."
                                  : "You are not using ANSI colors.");
    bprintf(buf, "  [%s] <SEND href=\"config autoexit\">Autoexit</SEND>  - %s<BR>",
      (IS_SET(ch->act, PLR_AUTOEXIT)) ? "o" : " ",
      (IS_SET(ch->act, PLR_AUTOEXIT)) ? "You automatically see exits."
                                      : "You don't automatically see exits.");
    bprintf(buf, "  [%s] <SEND href=\"config autoloot\">Autoloot</SEND>  - %s<BR>",
      (IS_SET(ch->act, PLR_AUTOLOOT)) ? "o" : " ",
      (IS_SET(ch->act, PLR_AUTOLOOT)) ? "You automatically loot corpses."
                                      : "You don't automatically loot corpses.");
    bprintf(buf, "  [%s] <SEND href=\"config autosac\">Autosac</SEND>   - %s<BR>",
      (IS_SET(ch->act, PLR_AUTOSAC)) ? "o" : " ",
      (IS_SET(ch->act, PLR_AUTOSAC)) ? "You automatically sacrifice corpses."
                                     : "You don't automatically sacrifice corpses.");
    bprintf(buf, "  [%s] <SEND href=\"config automap\">Automap</SEND>   - %s<BR>",
      (IS_SET(ch->act, PLR_MAP)) ? "o" : " ",
      (IS_SET(ch->act, PLR_MAP)) ? "You see the automapper."
                                 : "You don't see the automapper.");
    bprintf(buf, "  [%s] <SEND href=\"config autohead\">Autohead</SEND>  - %s<BR>",
      (IS_SET(ch->act, PLR_AUTOHEAD)) ? "o" : " ",
      (IS_SET(ch->act, PLR_AUTOHEAD)) ? "You automatically sacrifice heads."
                                      : "You don't automatically sacrifice heads.");

    bprintf(buf, "  [%s] <SEND href=\"config paradox\">Paradox</SEND>   - %s<BR>",
      (IS_SET(ch->act, PLR_PARADOX)) ? "o" : " ",
      (IS_SET(ch->act, PLR_PARADOX)) ? "You cannot commit paradox decapitations."
                                     : "You can commit paradox decapitations.");

    bprintf(buf, "  [%s] <SEND href=\"config spectate\">Spectate</SEND>  - %s<BR>",
      (IS_SET(ch->act, PLR_AUTOSPECTATE)) ? "o" : " ",
      (IS_SET(ch->act, PLR_AUTOSPECTATE)) ? "You automatically allow players to spectate you."
                                          : "You don't automatically allow spectators.");

    mxp_to_char(buf->data, ch, MXP_ALL);
    buffer_free(buf);
  }
  else
  {
    char buf2[MAX_INPUT_LENGTH];

    if (!str_cmp(arg, "ansi"))
      bit = PLR_ANSI;
    else if (!str_cmp(arg, "autoexit"))
      bit = PLR_AUTOEXIT;
    else if (!str_cmp(arg, "autoloot"))
      bit = PLR_AUTOLOOT;
    else if (!str_cmp(arg, "autosac"))
      bit = PLR_AUTOSAC;
    else if (!str_cmp(arg, "automap"))
      bit = PLR_MAP;
    else if (!str_cmp(arg, "autohead"))
      bit = PLR_AUTOHEAD;
    else if (!str_cmp(arg, "prompt"))
      bit = PLR_PROMPT;
    else if (!str_cmp(arg, "paradox"))
      bit = PLR_PARADOX;
    else if (!str_cmp(arg, "spectate"))
      bit = PLR_AUTOSPECTATE;
    else
    {
      send_to_char("Config which option?\n\r", ch);
      return;
    }

    TOGGLE_BIT(ch->act, bit);
    if (IS_SET(ch->act, bit))
    {
      sprintf(buf2, "%s is now ON\n\r", arg);
      send_to_char(buf2, ch);
    }
    else
    {
      sprintf(buf2, "%s is now OFF\n\r", arg);
      send_to_char(buf2, ch);
    }
  }
}

void do_ansi(CHAR_DATA * ch, char *argument)
{
  do_config(ch, "ansi");
}

void do_autoexit(CHAR_DATA * ch, char *argument)
{
  do_config(ch, "autoexit");
}

void do_autoloot(CHAR_DATA * ch, char *argument)
{
  do_config(ch, "autoloot");
}

void do_autosac(CHAR_DATA * ch, char *argument)
{
  do_config(ch, "autosac");
}

void do_autohead(CHAR_DATA *ch, char *argument)
{
  do_config(ch, "autohead");
}

void do_automap(CHAR_DATA * ch, char *argument)
{
  do_config(ch, "automap");
}

void do_brief(CHAR_DATA *ch, char *argument)
{
  BUFFER *buf = buffer_new(MAX_STRING_LENGTH);

  if (IS_NPC(ch))
  {
    buffer_free(buf);
    return;
  }

  bprintf(buf, " <<-----       Your current brief settings are       ----->><BR><BR>");
  bprintf(buf, "  [%c] <SEND href=\"brief1\">Brief1</SEND> - Showing %s room descriptions.<BR>",
    (ch->pcdata->brief[BRIEF_1]) ? 'o' : ' ',
    (ch->pcdata->brief[BRIEF_1]) ? "short" : "long");
  bprintf(buf, "  [%c] <SEND href=\"brief2\">Brief2</SEND> - Irrelevant combat messages are %s.<BR>",
    (ch->pcdata->brief[BRIEF_2]) ? 'o' : ' ',
    (ch->pcdata->brief[BRIEF_2]) ? "removed" : "shown");
  bprintf(buf, "  [%c] <SEND href=\"brief3\">Brief3</SEND> - Dodge and Parry messages are %s.<BR>",
    (ch->pcdata->brief[BRIEF_3]) ? 'o' : ' ',
    (ch->pcdata->brief[BRIEF_3]) ? "removed" : "shown");
  bprintf(buf, "  [%c] <SEND href=\"brief4\">Brief4</SEND> - Experience modifiers are %s.<BR>",
    (ch->pcdata->brief[BRIEF_4]) ? 'o' : ' ',
    (ch->pcdata->brief[BRIEF_4]) ? "removed" : "shown");
  bprintf(buf, "  [%c] <SEND href=\"brief5\">Brief5</SEND> - Combat summary is turned %s.<BR>",
    (ch->pcdata->brief[BRIEF_5]) ? 'o' : ' ',
    (ch->pcdata->brief[BRIEF_5]) ? "on" : "off");
  bprintf(buf, "  [%c] <SEND href=\"brief6\">Brief6</SEND> - Currently %s all 3rd party combat spam.<BR>",
    (ch->pcdata->brief[BRIEF_6]) ? 'o' : ' ',
    (ch->pcdata->brief[BRIEF_6]) ? "blocking" : "showing");
  bprintf(buf, "  [%c] <SEND href=\"brief7\">Brief7</SEND> - Currently %s all class spam.<BR>",
    (ch->pcdata->brief[BRIEF_7]) ? 'o' : ' ',
    (ch->pcdata->brief[BRIEF_7]) ? "blocking" : "showing");

  mxp_to_char(buf->data, ch, MXP_ALL);
  buffer_free(buf);
}

void do_brief1(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch)) return;

  if (ch->pcdata->brief[BRIEF_1])
  {
    ch->pcdata->brief[BRIEF_1] = 0;
    send_to_char("Brief 1 has been turned off.\n\r", ch);
  }
  else
  {
    ch->pcdata->brief[BRIEF_1] = 1;
    send_to_char("Brief 1 has been turned on.\n\r", ch);
  }
}

void do_brief2(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch)) return;
  
  if (ch->pcdata->brief[BRIEF_2])
  {
    ch->pcdata->brief[BRIEF_2] = 0;
    send_to_char("Brief 2 has been turned off.\n\r", ch);
  }
  else
  {
    ch->pcdata->brief[BRIEF_2] = 1;
    send_to_char("Brief 2 has been turned on.\n\r", ch);
  }
}

void do_brief3(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch)) return;
  
  if (ch->pcdata->brief[BRIEF_3])
  {
    ch->pcdata->brief[BRIEF_3] = 0;
    send_to_char("Brief 3 has been turned off.\n\r", ch);
  }
  else
  {
    ch->pcdata->brief[BRIEF_3] = 1;
    send_to_char("Brief 3 has been turned on.\n\r", ch);
  }
}

void do_brief4(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch)) return;
  
  if (ch->pcdata->brief[BRIEF_4])
  {
    ch->pcdata->brief[BRIEF_4] = 0;
    send_to_char("Brief 4 has been turned off.\n\r", ch);
  }
  else
  {
    ch->pcdata->brief[BRIEF_4] = 1;
    send_to_char("Brief 4 has been turned on.\n\r", ch);
  }
}

void do_brief5(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch)) return;
  
  if (ch->pcdata->brief[BRIEF_5])
  {
    ch->pcdata->brief[BRIEF_5] = 0;
    send_to_char("Brief 5 has been turned off.\n\r", ch);
  }
  else
  {
    ch->pcdata->brief[BRIEF_5] = 1;
    send_to_char("Brief 5 has been turned on.\n\r", ch);
  }
}

void do_brief6(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch)) return;
  
  if (ch->pcdata->brief[BRIEF_6])
  {
    ch->pcdata->brief[BRIEF_6] = 0;
    send_to_char("Brief 6 has been turned off.\n\r", ch);
  }
  else
  {
    ch->pcdata->brief[BRIEF_6] = 1;
    send_to_char("Brief 6 has been turned on.\n\r", ch);
  }
}

void do_brief7(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch)) return;
 
  if (ch->pcdata->brief[BRIEF_7])
  {
    ch->pcdata->brief[BRIEF_7] = 0;
    send_to_char("Brief 7 has been turned off.\n\r", ch);
  }
  else
  {
    ch->pcdata->brief[BRIEF_7] = 1;
    send_to_char("Brief 7 has been turned on.\n\r", ch); 
  }
}

void do_diagnose(CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int teeth = 0;
  int ribs = 0;
  CHAR_DATA *victim;

  argument = one_argument(argument, arg);

  if (arg == '\0')
  {
    send_to_char("Who do you wish to diagnose?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("Nobody here by that name.\n\r", ch);
    return;
  }
  act("$n examines $N carefully, diagnosing $S injuries.", ch, NULL, victim, TO_NOTVICT);
  act("$n examines you carefully, diagnosing your injuries.", ch, NULL, victim, TO_VICT);
  act("Your diagnoses of $N reveals the following...", ch, NULL, victim, TO_CHAR);
  send_to_char("----------------------------------------------------------------------------\n\r", ch);
  if ((victim->loc_hp[0] + victim->loc_hp[1] + victim->loc_hp[2] + victim->loc_hp[3] + victim->loc_hp[4] + victim->loc_hp[5] + victim->loc_hp[6]) == 0)
  {
    act("$N has no apparent injuries.", ch, NULL, victim, TO_CHAR);
    send_to_char("----------------------------------------------------------------------------\n\r", ch);
    return;
  }
  /* Check head */
  if (IS_HEAD(victim, LOST_EYE_L) && IS_HEAD(victim, LOST_EYE_R))
    act("$N has lost both of $S eyes.", ch, NULL, victim, TO_CHAR);
  else if (IS_HEAD(victim, LOST_EYE_L))
    act("$N has lost $S left eye.", ch, NULL, victim, TO_CHAR);
  else if (IS_HEAD(victim, LOST_EYE_R))
    act("$N has lost $S right eye.", ch, NULL, victim, TO_CHAR);
  if (IS_HEAD(victim, LOST_EAR_L) && IS_HEAD(victim, LOST_EAR_R))
    act("$N has lost both of $S ears.", ch, NULL, victim, TO_CHAR);
  else if (IS_HEAD(victim, LOST_EAR_L))
    act("$N has lost $S left ear.", ch, NULL, victim, TO_CHAR);
  else if (IS_HEAD(victim, LOST_EAR_R))
    act("$N has lost $S right ear.", ch, NULL, victim, TO_CHAR);
  if (IS_HEAD(victim, LOST_NOSE))
    act("$N has lost $S nose.", ch, NULL, victim, TO_CHAR);
  else if (IS_HEAD(victim, BROKEN_NOSE))
    act("$N has got a broken nose.", ch, NULL, victim, TO_CHAR);
  if (IS_HEAD(victim, BROKEN_JAW))
    act("$N has got a broken jaw.", ch, NULL, victim, TO_CHAR);
  if (IS_BODY(victim, BROKEN_NECK))
    act("$N has got a broken neck.", ch, NULL, victim, TO_CHAR);
  if (IS_BODY(victim, CUT_THROAT))
  {
    act("$N has had $S throat cut open.", ch, NULL, victim, TO_CHAR);
    if (IS_BLEEDING(victim, BLEEDING_THROAT))
      act("...Blood is pouring from the wound.", ch, NULL, victim, TO_CHAR);
  }
  if (IS_HEAD(victim, BROKEN_SKULL))
    act("$N has got a broken skull.", ch, NULL, victim, TO_CHAR);
  if (IS_HEAD(victim, LOST_TOOTH_1))
    teeth += 1;
  if (IS_HEAD(victim, LOST_TOOTH_2))
    teeth += 2;
  if (IS_HEAD(victim, LOST_TOOTH_4))
    teeth += 4;
  if (IS_HEAD(victim, LOST_TOOTH_8))
    teeth += 8;
  if (IS_HEAD(victim, LOST_TOOTH_16))
    teeth += 16;
  if (teeth > 0)
  {
    sprintf(buf, "$N has had %d teeth knocked out.", teeth);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  if (IS_HEAD(victim, LOST_TONGUE))
    act("$N has had $S tongue ripped out.", ch, NULL, victim, TO_CHAR);
  /* Check body */
  if (IS_BODY(victim, BROKEN_RIBS_1))
    ribs += 1;
  if (IS_BODY(victim, BROKEN_RIBS_2))
    ribs += 2;
  if (IS_BODY(victim, BROKEN_RIBS_4))
    ribs += 4;
  if (IS_BODY(victim, BROKEN_RIBS_8))
    ribs += 8;
  if (IS_BODY(victim, BROKEN_RIBS_16))
    ribs += 16;
  if (ribs > 0)
  {
    sprintf(buf, "$N has got %d broken ribs.", ribs);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  if (IS_BODY(victim, BROKEN_SPINE))
    act("$N has got a broken spine.", ch, NULL, victim, TO_CHAR);
  /* Check arms */
  check_left_arm(ch, victim);
  check_right_arm(ch, victim);
  check_left_leg(ch, victim);
  check_right_leg(ch, victim);
  send_to_char("----------------------------------------------------------------------------\n\r", ch);
}

void check_left_arm(CHAR_DATA * ch, CHAR_DATA * victim)
{
  char buf[MAX_STRING_LENGTH];
  char finger[10];
  int fingers = 0;

  if (IS_ARM_L(victim, LOST_ARM) && IS_ARM_R(victim, LOST_ARM))
  {
    act("$N has lost both of $S arms.", ch, NULL, victim, TO_CHAR);
    if (IS_BLEEDING(victim, BLEEDING_ARM_L) && IS_BLEEDING(victim, BLEEDING_ARM_R))
      act("...Blood is spurting from both stumps.", ch, NULL, victim, TO_CHAR);
    else if (IS_BLEEDING(victim, BLEEDING_ARM_L))
      act("...Blood is spurting from the left stump.", ch, NULL, victim, TO_CHAR);
    else if (IS_BLEEDING(victim, BLEEDING_ARM_R))
      act("...Blood is spurting from the right stump.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (IS_ARM_L(victim, LOST_ARM))
  {
    act("$N has lost $S left arm.", ch, NULL, victim, TO_CHAR);
    if (IS_BLEEDING(victim, BLEEDING_ARM_L))
      act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (IS_ARM_L(victim, BROKEN_ARM) && IS_ARM_R(victim, BROKEN_ARM))
    act("$N arms are both broken.", ch, NULL, victim, TO_CHAR);
  else if (IS_ARM_L(victim, BROKEN_ARM))
    act("$N's left arm is broken.", ch, NULL, victim, TO_CHAR);
  if (IS_ARM_L(victim, LOST_HAND) && IS_ARM_R(victim, LOST_HAND) && !IS_ARM_R(victim, LOST_ARM))
  {
    act("$N has lost both of $S hands.", ch, NULL, victim, TO_CHAR);
    if (IS_BLEEDING(victim, BLEEDING_HAND_L) && IS_BLEEDING(victim, BLEEDING_HAND_R))
      act("...Blood is spurting from both stumps.", ch, NULL, victim, TO_CHAR);
    else if (IS_BLEEDING(victim, BLEEDING_HAND_L))
      act("...Blood is spurting from the left stump.", ch, NULL, victim, TO_CHAR);
    else if (IS_BLEEDING(victim, BLEEDING_HAND_R))
      act("...Blood is spurting from the right stump.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (IS_ARM_L(victim, LOST_HAND))
  {
    act("$N has lost $S left hand.", ch, NULL, victim, TO_CHAR);
    if (IS_BLEEDING(victim, BLEEDING_HAND_L))
      act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (IS_ARM_L(victim, LOST_FINGER_I))
    fingers += 1;
  if (IS_ARM_L(victim, LOST_FINGER_M))
    fingers += 1;
  if (IS_ARM_L(victim, LOST_FINGER_R))
    fingers += 1;
  if (IS_ARM_L(victim, LOST_FINGER_L))
    fingers += 1;
  if (fingers == 1)
    sprintf(finger, "finger");
  else
    sprintf(finger, "fingers");
  if (fingers > 0 && IS_ARM_L(victim, LOST_THUMB))
  {
    sprintf(buf, "$N has lost %d %s and $S thumb from $S left hand.", fingers, finger);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (fingers > 0)
  {
    sprintf(buf, "$N has lost %d %s from $S left hand.", fingers, finger);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (IS_ARM_L(victim, LOST_THUMB))
  {
    sprintf(buf, "$N has lost the thumb from $S left hand.");
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  fingers = 0;
  if (IS_ARM_L(victim, BROKEN_FINGER_I) && !IS_ARM_L(victim, LOST_FINGER_I))
    fingers += 1;
  if (IS_ARM_L(victim, BROKEN_FINGER_M) && !IS_ARM_L(victim, LOST_FINGER_M))
    fingers += 1;
  if (IS_ARM_L(victim, BROKEN_FINGER_R) && !IS_ARM_L(victim, LOST_FINGER_R))
    fingers += 1;
  if (IS_ARM_L(victim, BROKEN_FINGER_L) && !IS_ARM_L(victim, LOST_FINGER_L))
    fingers += 1;
  if (fingers == 1)
    sprintf(finger, "finger");
  else
    sprintf(finger, "fingers");
  if (fingers > 0 && IS_ARM_L(victim, BROKEN_THUMB) && !IS_ARM_L(victim, LOST_THUMB))
  {
    sprintf(buf, "$N has broken %d %s and $S thumb on $S left hand.", fingers, finger);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (fingers > 0)
  {
    sprintf(buf, "$N has broken %d %s on $S left hand.", fingers, finger);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (IS_ARM_L(victim, BROKEN_THUMB) && !IS_ARM_L(victim, LOST_THUMB))
  {
    sprintf(buf, "$N has broken the thumb on $S left hand.");
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  return;
}

void check_right_arm(CHAR_DATA * ch, CHAR_DATA * victim)
{
  char buf[MAX_STRING_LENGTH];
  char finger[10];
  int fingers = 0;

  if (IS_ARM_L(victim, LOST_ARM) && IS_ARM_R(victim, LOST_ARM))
    return;
  if (IS_ARM_R(victim, LOST_ARM))
  {
    act("$N has lost $S right arm.", ch, NULL, victim, TO_CHAR);
    if (IS_BLEEDING(victim, BLEEDING_ARM_R))
      act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (!IS_ARM_L(victim, BROKEN_ARM) && IS_ARM_R(victim, BROKEN_ARM))
    act("$N's right arm is broken.", ch, NULL, victim, TO_CHAR);
  else if (IS_ARM_L(victim, LOST_ARM) && IS_ARM_R(victim, BROKEN_ARM))
    act("$N's right arm is broken.", ch, NULL, victim, TO_CHAR);
  if (IS_ARM_L(victim, LOST_HAND) && IS_ARM_R(victim, LOST_HAND))
    return;
  if (IS_ARM_R(victim, LOST_HAND))
  {
    act("$N has lost $S right hand.", ch, NULL, victim, TO_CHAR);
    if (IS_BLEEDING(victim, BLEEDING_HAND_R))
      act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (IS_ARM_R(victim, LOST_FINGER_I))
    fingers += 1;
  if (IS_ARM_R(victim, LOST_FINGER_M))
    fingers += 1;
  if (IS_ARM_R(victim, LOST_FINGER_R))
    fingers += 1;
  if (IS_ARM_R(victim, LOST_FINGER_L))
    fingers += 1;
  if (fingers == 1)
    sprintf(finger, "finger");
  else
    sprintf(finger, "fingers");
  if (fingers > 0 && IS_ARM_R(victim, LOST_THUMB))
  {
    sprintf(buf, "$N has lost %d %s and $S thumb from $S right hand.", fingers, finger);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (fingers > 0)
  {
    sprintf(buf, "$N has lost %d %s from $S right hand.", fingers, finger);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (IS_ARM_R(victim, LOST_THUMB))
  {
    sprintf(buf, "$N has lost the thumb from $S right hand.");
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  fingers = 0;
  if (IS_ARM_R(victim, BROKEN_FINGER_I) && !IS_ARM_R(victim, LOST_FINGER_I))
    fingers += 1;
  if (IS_ARM_R(victim, BROKEN_FINGER_M) && !IS_ARM_R(victim, LOST_FINGER_M))
    fingers += 1;
  if (IS_ARM_R(victim, BROKEN_FINGER_R) && !IS_ARM_R(victim, LOST_FINGER_R))
    fingers += 1;
  if (IS_ARM_R(victim, BROKEN_FINGER_L) && !IS_ARM_R(victim, LOST_FINGER_L))
    fingers += 1;
  if (fingers == 1)
    sprintf(finger, "finger");
  else
    sprintf(finger, "fingers");
  if (fingers > 0 && IS_ARM_R(victim, BROKEN_THUMB) && !IS_ARM_R(victim, LOST_THUMB))
  {
    sprintf(buf, "$N has broken %d %s and $S thumb on $S right hand.", fingers, finger);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (fingers > 0)
  {
    sprintf(buf, "$N has broken %d %s on $S right hand.", fingers, finger);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (IS_ARM_R(victim, BROKEN_THUMB) && !IS_ARM_R(victim, LOST_THUMB))
  {
    sprintf(buf, "$N has broken the thumb on $S right hand.");
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  return;
}

void check_left_leg(CHAR_DATA * ch, CHAR_DATA * victim)
{
  char buf[MAX_STRING_LENGTH];
  char toe[10];
  int toes = 0;

  if (IS_LEG_L(victim, LOST_LEG) && IS_LEG_R(victim, LOST_LEG))
  {
    act("$N has lost both of $S legs.", ch, NULL, victim, TO_CHAR);
    if (IS_BLEEDING(victim, BLEEDING_LEG_L) && IS_BLEEDING(victim, BLEEDING_LEG_R))
      act("...Blood is spurting from both stumps.", ch, NULL, victim, TO_CHAR);
    else if (IS_BLEEDING(victim, BLEEDING_LEG_L))
      act("...Blood is spurting from the left stump.", ch, NULL, victim, TO_CHAR);
    else if (IS_BLEEDING(victim, BLEEDING_LEG_R))
      act("...Blood is spurting from the right stump.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (IS_LEG_L(victim, LOST_LEG))
  {
    act("$N has lost $S left leg.", ch, NULL, victim, TO_CHAR);
    if (IS_BLEEDING(victim, BLEEDING_LEG_L))
      act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (IS_LEG_L(victim, BROKEN_LEG) && IS_LEG_R(victim, BROKEN_LEG))
    act("$N legs are both broken.", ch, NULL, victim, TO_CHAR);
  else if (IS_LEG_L(victim, BROKEN_LEG))
    act("$N's left leg is broken.", ch, NULL, victim, TO_CHAR);
  if (IS_LEG_L(victim, LOST_FOOT) && IS_LEG_R(victim, LOST_FOOT))
  {
    act("$N has lost both of $S feet.", ch, NULL, victim, TO_CHAR);
    if (IS_BLEEDING(victim, BLEEDING_FOOT_L) && IS_BLEEDING(victim, BLEEDING_FOOT_R))
      act("...Blood is spurting from both stumps.", ch, NULL, victim, TO_CHAR);
    else if (IS_BLEEDING(victim, BLEEDING_FOOT_L))
      act("...Blood is spurting from the left stump.", ch, NULL, victim, TO_CHAR);
    else if (IS_BLEEDING(victim, BLEEDING_FOOT_R))
      act("...Blood is spurting from the right stump.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (IS_LEG_L(victim, LOST_FOOT))
  {
    act("$N has lost $S left foot.", ch, NULL, victim, TO_CHAR);
    if (IS_BLEEDING(victim, BLEEDING_FOOT_L))
      act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (IS_LEG_L(victim, LOST_TOE_A))
    toes += 1;
  if (IS_LEG_L(victim, LOST_TOE_B))
    toes += 1;
  if (IS_LEG_L(victim, LOST_TOE_C))
    toes += 1;
  if (IS_LEG_L(victim, LOST_TOE_D))
    toes += 1;
  if (toes == 1)
    sprintf(toe, "toe");
  else
    sprintf(toe, "toes");
  if (toes > 0 && IS_LEG_L(victim, LOST_TOE_BIG))
  {
    sprintf(buf, "$N has lost %d %s and $S big toe from $S left foot.", toes, toe);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (toes > 0)
  {
    sprintf(buf, "$N has lost %d %s from $S left foot.", toes, toe);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (IS_LEG_L(victim, LOST_TOE_BIG))
  {
    sprintf(buf, "$N has lost the big toe from $S left foot.");
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  toes = 0;
  if (IS_LEG_L(victim, BROKEN_TOE_A) && !IS_LEG_L(victim, LOST_TOE_A))
    toes += 1;
  if (IS_LEG_L(victim, BROKEN_TOE_B) && !IS_LEG_L(victim, LOST_TOE_B))
    toes += 1;
  if (IS_LEG_L(victim, BROKEN_TOE_C) && !IS_LEG_L(victim, LOST_TOE_C))
    toes += 1;
  if (IS_LEG_L(victim, BROKEN_TOE_D) && !IS_LEG_L(victim, LOST_TOE_D))
    toes += 1;
  if (toes == 1)
    sprintf(toe, "toe");
  else
    sprintf(toe, "toes");
  if (toes > 0 && IS_LEG_L(victim, BROKEN_TOE_BIG) && !IS_LEG_L(victim, LOST_TOE_BIG))
  {
    sprintf(buf, "$N has broken %d %s and $S big toe from $S left foot.", toes, toe);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (toes > 0)
  {
    sprintf(buf, "$N has broken %d %s on $S left foot.", toes, toe);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (IS_LEG_L(victim, BROKEN_TOE_BIG) && !IS_LEG_L(victim, LOST_TOE_BIG))
  {
    sprintf(buf, "$N has broken the big toe on $S left foot.");
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  return;
}

void check_right_leg(CHAR_DATA * ch, CHAR_DATA * victim)
{
  char buf[MAX_STRING_LENGTH];
  char toe[10];
  int toes = 0;

  if (IS_LEG_L(victim, LOST_LEG) && IS_LEG_R(victim, LOST_LEG))
    return;
  if (IS_LEG_R(victim, LOST_LEG))
  {
    act("$N has lost $S right leg.", ch, NULL, victim, TO_CHAR);
    if (IS_BLEEDING(victim, BLEEDING_LEG_R))
      act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (!IS_LEG_L(victim, BROKEN_LEG) && IS_LEG_R(victim, BROKEN_LEG))
    act("$N's right leg is broken.", ch, NULL, victim, TO_CHAR);
  if (IS_LEG_L(victim, LOST_FOOT) && IS_LEG_R(victim, LOST_FOOT))
    return;
  if (IS_LEG_R(victim, LOST_FOOT))
  {
    act("$N has lost $S right foot.", ch, NULL, victim, TO_CHAR);
    if (IS_BLEEDING(victim, BLEEDING_FOOT_R))
      act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
    return;
  }
  if (IS_LEG_R(victim, LOST_TOE_A))
    toes += 1;
  if (IS_LEG_R(victim, LOST_TOE_B))
    toes += 1;
  if (IS_LEG_R(victim, LOST_TOE_C))
    toes += 1;
  if (IS_LEG_R(victim, LOST_TOE_D))
    toes += 1;
  if (toes == 1)
    sprintf(toe, "toe");
  else
    sprintf(toe, "toes");
  if (toes > 0 && IS_LEG_R(victim, LOST_TOE_BIG))
  {
    sprintf(buf, "$N has lost %d %s and $S big toe from $S right foot.", toes, toe);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (toes > 0)
  {
    sprintf(buf, "$N has lost %d %s from $S right foot.", toes, toe);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (IS_LEG_R(victim, LOST_TOE_BIG))
  {
    sprintf(buf, "$N has lost the big toe from $S right foot.");
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  toes = 0;
  if (IS_LEG_R(victim, BROKEN_TOE_A) && !IS_LEG_R(victim, LOST_TOE_A))
    toes += 1;
  if (IS_LEG_R(victim, BROKEN_TOE_B) && !IS_LEG_R(victim, LOST_TOE_B))
    toes += 1;
  if (IS_LEG_R(victim, BROKEN_TOE_C) && !IS_LEG_R(victim, LOST_TOE_C))
    toes += 1;
  if (IS_LEG_R(victim, BROKEN_TOE_D) && !IS_LEG_R(victim, LOST_TOE_D))
    toes += 1;
  if (toes == 1)
    sprintf(toe, "toe");
  else
    sprintf(toe, "toes");
  if (toes > 0 && IS_LEG_R(victim, BROKEN_TOE_BIG) && !IS_LEG_R(victim, LOST_TOE_BIG))
  {
    sprintf(buf, "$N has broken %d %s and $S big toe on $S right foot.", toes, toe);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (toes > 0)
  {
    sprintf(buf, "$N has broken %d %s on $S right foot.", toes, toe);
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  else if (IS_LEG_R(victim, BROKEN_TOE_BIG) && !IS_LEG_R(victim, LOST_TOE_BIG))
  {
    sprintf(buf, "$N has broken the big toe on $S right foot.");
    act(buf, ch, NULL, victim, TO_CHAR);
  }
  return;
}

/* Do_prompt from Morgenes from Aldara Mud */
void do_prompt(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (argument[0] == '\0')
  {
    do_help(ch, "prompt");

    if (ch->prompt[0] != '\0')
      printf_to_char(ch, "\n\rCurrent prompt:\n\r%s\n\r", ch->prompt);

    return;
  }

  if (!strcmp(argument, "on"))
  {
    if (IS_EXTRA(ch, EXTRA_PROMPT))
      send_to_char("But you already have customised prompt on!\n\r", ch);
    else
    {
      send_to_char("Ok.\n\r", ch);
      SET_BIT(ch->extra, EXTRA_PROMPT);
    }
    return;
  }
  else if (!strcmp(argument, "off"))
  {
    if (!IS_EXTRA(ch, EXTRA_PROMPT))
      send_to_char("But you already have customised prompt off!\n\r", ch);
    else
    {
      send_to_char("Ok.\n\r", ch);
      REMOVE_BIT(ch->extra, EXTRA_PROMPT);
    }
    return;
  }
  else if (!strcmp(argument, "clear"))
  {
    free_string(ch->prompt);
    ch->prompt = str_dup("");
    return;
  }
  else
  {
    if (collen(argument) > 79)
    {
      send_to_char("That prompt is to large.\n\r", ch);
      return;
    }
    smash_tilde(argument);
    free_string(ch->prompt);
    ch->prompt = str_dup(argument);
    send_to_char("Ok.\n\r", ch);
  }
}

/* Do_prompt from Morgenes from Aldara Mud */
void do_cprompt(CHAR_DATA * ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (argument[0] == '\0')
  {
    do_help(ch, "cprompt");

    if (ch->cprompt[0] != '\0')
      printf_to_char(ch, "\n\rCurrent cprompt:\n\r%s\n\r", ch->cprompt);

    return;
  }
  if (!strcmp(argument, "clear"))
  {
    free_string(ch->cprompt);
    ch->cprompt = str_dup("");
    return;
  }
  else
  {
    if (collen(argument) > 79)
    {
      send_to_char("That prompt is to large.\n\r", ch);
      return;
    }
    smash_tilde(argument);
    free_string(ch->cprompt);
    ch->cprompt = str_dup(argument);
    send_to_char("Ok.\n\r", ch);
  }
}

void do_finger(CHAR_DATA *ch, char *argument)
{
  BUFFER *finger;
  char arg[MAX_STRING_LENGTH];

  one_argument(argument, arg);

  if ((finger = get_finger(ch, arg)) != NULL)
  {
    send_to_char(finger->data, ch);
    buffer_free(finger);
  }
  else
  {
    send_to_char("That player doesn't exist.\n\r", ch);
  }
}

BUFFER *get_finger(CHAR_DATA *ch, char *player)
{
  CHAR_DATA *vch;
  BUFFER *finger;
  int status = 1;

  /* something wrong with the playername ? */
  if (!check_parse_name(player, TRUE) || !char_exists(player))
    return NULL;

  /* allocate memory for the finger data */
  finger = buffer_new(MAX_STRING_LENGTH);

  /* load the characters whois info */
  if ((vch = load_char_whois(player, &status)) == NULL)
  {
    if (status == -1)
      bprintf(finger, "That player has not logged since we changed the finger storage.\n\r");
    else if (status == 0)
      bprintf(finger, "That player does not exist.\n\r");
    else
      bprintf(finger, "Something unexpected happened.\n\r");

    return finger;
  }

  bprintf(finger, "%s\n\r", get_dystopia_banner("Calim's Cradle", 68));

  bprintf(finger, "%s %s @%s (account:%s)\n\r", vch->name, vch->pcdata->title,
    (ch && ch->level >= 10) ? vch->lasthost : "xxx.xxx.xxx.xxx",
    (ch && ch->level >= 10) ? vch->pcdata->account : "xxxxx");

  bprintf(finger, "%s\n\r", get_dystopia_banner("", 68));
  bprintf(finger, "Last connected at %s.\n\r", vch->lasttime);
  bprintf(finger, "%s\n\r", get_dystopia_banner("", 68));
  bprintf(finger, "Sex: %s. Age: %s",
    (vch->sex == SEX_MALE) ? "Male" : ((vch->sex == SEX_FEMALE) ? "Female" : "None"),
    other_age(vch->extra, FALSE, vch->createtime));

  switch (vch->level)
  {
    default:
      bprintf(finger, "They are mortal, ");
      break;
    case LEVEL_AVATAR:
      bprintf(finger, "They are an Avatar, ");
      break;
    case LEVEL_IMMORTAL:
    case LEVEL_QUESTMAKER:
    case LEVEL_ENFORCER:
    case LEVEL_JUDGE:
    case LEVEL_HIGHJUDGE:
    case LEVEL_IMPLEMENTOR:
      bprintf(finger, "They are a God, ");
      break;
  }
  bprintf(finger, "and have been playing for %d hours.\n\r", vch->played / 3600);
  bprintf(finger, "Player kills: %d, Player Deaths: %d.\n\r", vch->pkill, vch->pdeath);
  bprintf(finger, "Arena kills: %d, Arena Deaths: %d.\n\r", vch->pcdata->awins, vch->pcdata->alosses);
  bprintf(finger, "Mob Kills: %d, Mob Deaths: %d.\n\r", vch->mkill, vch->mdeath);
  bprintf(finger, "%s\n\r", get_dystopia_banner("", 68));

  free_char(vch);
  return finger;
}
