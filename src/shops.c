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
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dystopia.h"

DECLARE_SHOP_FUN( shopspec_generic		);
DECLARE_SHOP_FUN( shopspec_enchanter		);
DECLARE_SHOP_FUN( shopspec_engraver		);
DECLARE_SHOP_FUN( shopspec_gatekeeper		);
DECLARE_SHOP_FUN( shopspec_gemdragon		);
DECLARE_SHOP_FUN( shopspec_gemenchanter		);
DECLARE_SHOP_FUN( shopspec_smithy		);

const struct shop_type shop_table [] =
{
  { "shopspec_generic",         shopspec_generic      },
  { "shopspec_enchanter",       shopspec_enchanter    },
  { "shopspec_engraver",        shopspec_engraver     },
  { "shopspec_gatekeeper",      shopspec_gatekeeper   },
  { "shopspec_gemdragon",       shopspec_gemdragon    },
  { "shopspec_gemenchanter",    shopspec_gemenchanter },
  { "shopspec_smithy",          shopspec_smithy       },

  /* end of table */
  { "", 0 }
};

/* two wrapper commands to ease the use of shops */
void do_buy(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  ITERATOR *pIter;
  CHAR_DATA *keeper;

  if (argument[0] != '\0') sprintf(buf, "buy %s", argument);
  else sprintf(buf, "buy");

  pIter = AllocIterator(ch->in_room->people);
  while ((keeper = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (keeper == ch) continue;
    if (!keeper->shop_fun) continue;
    (*keeper->shop_fun)(keeper, ch, buf);
    return;
  }
  send_to_char("There are no shopkeepers in this room.\n\r", ch);
}

void do_list(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH + 32];
  CHAR_DATA *keeper;
  ITERATOR *pIter;

  sprintf(buf, "list %s", argument);

  pIter = AllocIterator(ch->in_room->people);
  while ((keeper = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (keeper == ch) continue;
    if (!keeper->shop_fun) continue;
    (*keeper->shop_fun)(keeper, ch, buf);
    return;
  }
  send_to_char("There are no shopkeepers in this room.\n\r", ch);
}

void do_shop(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *keeper;
  char arg[MAX_INPUT_LENGTH];
  
  if (IS_NPC(ch)) return;
  argument = one_argument(argument, arg);
  if ((keeper = get_char_room(ch, arg)) == NULL)
  {
    send_to_char("You cannot seem to find that shopkeeper.\n\r", ch);
    return;
  }
  if (!IS_NPC(keeper))
  {
    send_to_char("That's not a shopkeeper.\n\r", ch);
    return;
  }
  if (keeper->shop_fun != 0)
  {
    (*keeper->shop_fun)(keeper, ch, argument);
    return;
  }
  send_to_char("That's not a shopkeeper", ch);
}

SHOP_FUN *shop_lookup(const char *name)
{
  int cmd;

  for (cmd = 0; *shop_table[cmd].shop_name; cmd++)
    if (!str_cmp(name, shop_table[cmd].shop_name))
      return shop_table[cmd].shop_fun;
  return 0;
}

char *shop_string(SHOP_FUN *fun)
{
  int cmd;

  for (cmd = 0; *shop_table[cmd].shop_fun; cmd++)
    if (fun == shop_table[cmd].shop_fun)
      return shop_table[cmd].shop_name;

  return 0;
}

void shopspec_engraver(CHAR_DATA *keeper, CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char item[MAX_INPUT_LENGTH];
  char command[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  const int cost = 10;
 
  argument = one_argument(argument, arg);
  argument = one_argument(argument, item);
  argument = one_argument(argument, command);

  if (IS_NPC(ch)) return;

  if (!str_cmp(arg, "list"))
  {
    act("$n says '#yHey there $N, what do you want me to engrave?.#n'.", keeper, NULL, ch, TO_VICT);  
    send_to_char("\n\rSyntax for engraving is : buy <object> <short|long|keyword> <new name>\n\r", ch);
  }  
  else if (!str_cmp(arg, "buy"))
  {
    if ((obj = get_obj_carry(ch, item)) == NULL)
    {
      act("$n asks '#yWhat item do you want me to engrave?#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }
    if (ch->pcdata->playerid != obj->ownerid)
    {
      send_to_char("You do not own this item.\n\r", ch);
      return;
    }
    if (getGold(ch) < cost)
    {
      printf_to_char(ch, "I'm afraid you need %d more goldcrowns.\n\r", cost - getGold(ch));
      return;
    }
    if (strlen(argument) < 3 || strlen(argument) > 80)
    {
      act("$n says '#yI can only engrave between 3 and 80 characters.#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }
    smash_tilde(argument);
    if (!str_cmp(command, "short"))
    {
      free_string(obj->short_descr);
      obj->short_descr = str_dup(argument);
      act("$n says '#yThere you go sir.#n'.", keeper, NULL, ch, TO_VICT);
      setGold(ch, -1 * cost);
    }
    else if (!str_cmp(command, "long"))
    {
      free_string(obj->description);
      obj->description = str_dup(argument);
      act("$n says '#yThere you go sir.#n'.", keeper, NULL, ch, TO_VICT);
      setGold(ch, -1 * cost);
    }
    else if (!str_cmp(command, "keyword"))
    {
      free_string(obj->name);
      obj->name = str_dup(argument);
      act("$n says '#yThere you go sir.#n'.", keeper, NULL, ch, TO_VICT);
      setGold(ch, -1 * cost);
    }
    else
      act("$n says '#yI'm afraid I don't know how to do that.#n'.", keeper, NULL, ch, TO_VICT);
  }
  else
  {
    act("$n says '#yI'm afraid I don't understand.#n'.", keeper, NULL, ch, TO_VICT);
  }
}

void shopspec_gatekeeper(CHAR_DATA *keeper, CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH]; 
  char item[MAX_INPUT_LENGTH];
  int cost = 250;

  argument = one_argument(argument, arg);
  one_argument(argument, item);

  if (!str_cmp(arg, "list"))
  {
    act("$n says '#yIf you want to buy a 'passage' just tell me.#n'.", keeper, NULL, ch, TO_VICT);
  }
  else if (!str_cmp(arg, "buy"))
  {
    if (!str_cmp(item, "passage"))
    {
      if (getGold(ch) < cost)
      {
        act("$n says '#yRead the sign kid, it says 250 goldcrowns.#n'.", keeper, NULL, ch, TO_VICT);
      }
      else
      {
        ROOM_INDEX_DATA *pRoom;

        if ((pRoom = get_room_index(ROOM_VNUM_CATACOMBS)) == NULL)
        {
          act("$n says '#yHey! Go tell an immortal that I'm broken.#n'.", keeper, NULL, ch, TO_VICT);
          return;
        }

        if (ch->fight_timer > 0)
        {
          act("$n says '#yI'm afraid the gate is closed at the moment.#n'.", keeper, NULL, ch, TO_VICT);
          return;
        }

        setGold(ch, -1 * cost);

        act("$n mumbles an arcane phrase, and the world starts spinning.", keeper, NULL, ch, TO_VICT);
        act("$n mumbles an arcane phrase, and $N fades from sight.", keeper, NULL, ch, TO_NOTVICT);
        char_from_room(ch);
        char_to_room(ch, pRoom, TRUE);
        do_look(ch, "");
      }
    }
    else
    {
      act("$n says '#yEh? I don't sell that kind of funky stuff.#n'.", keeper, NULL, ch, TO_VICT);
    }
  }
  else
  {
    act("$n says '#yWhat are you talking about, your making no sense.#n'.", keeper, NULL, ch, TO_VICT);
  }
}

void shopspec_smithy(CHAR_DATA *keeper, CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument(argument, arg);

  if (!str_cmp(arg, "list"))
  {
    act("$n spits on the floor.", keeper, NULL, ch, TO_VICT);
    act("$n says '#yI can enhance weapons an armour, costs 500 goldcrowns.#n'.", keeper, NULL, ch, TO_VICT);
  }
  else if (!str_cmp(arg, "buy"))
  {
    AFFECT_DATA paf;
    OBJ_DATA *obj;
    int cost = 500;

    argument = one_argument(argument, arg);
    if (str_cmp(arg, "enhance"))
    {
      act("$n scowls at you.", keeper, NULL, ch, TO_VICT);
      act("$n says '#yJust read the sign; you can read, can't you?#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }

    one_argument(argument, arg);
    if ((obj = get_obj_carry(ch, arg)) == NULL)
    {
      act("$n raises a bushy eyebrow.", keeper, NULL, ch, TO_VICT);
      act("$n says '#yAnd next you'll want me to fetch a pink elephant, bah!#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }

    if (obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR)
    {
      act("$n shakes $s head.", keeper, NULL, ch, TO_VICT);
      act("$n says '#yI only deal with weapons and armour.#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }

    if (obj->cost >= 200 || !IS_OBJ_STAT(obj, ITEM_RARE))
    {
      act("$n whistles while he looks at $p.", keeper, obj, ch, TO_VICT);
      act("$n says '#yI'm afraid I can't do anything about that piece.#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }

    if (getGold(ch) < cost)
    {
      act("$n shakes $s head.", keeper, NULL, ch, TO_VICT);
      act("$n says '#yNo money, no service. Go away!#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }
    setGold(ch, -1 * cost);

    switch(number_range(1, 4))
    {
      default:
        if (obj->item_type == ITEM_WEAPON)
        {
          obj->value[1]++;

          if (obj->value[2] < 60)
            obj->value[2]++;
        }
        else
        {
          act("$n says '#yI'll make this piece REALLY hard to break.#n'.", keeper, NULL, ch, TO_VICT);
          obj->resistance--;
        }
        break;
      case 2:
        paf.type           = 0;
        paf.duration       = -1;
        paf.location       = APPLY_HITROLL;
        paf.modifier       = number_range(1, 2);
        paf.bitvector      = 0;
        affect_to_obj(obj, &paf);
        break;
      case 3:
        paf.type           = 0;
        paf.duration       = -1;
        paf.location       = APPLY_DAMROLL;
        paf.modifier       = number_range(1, 2);
        paf.bitvector      = 0;
        affect_to_obj(obj, &paf);
        break;
      case 4:
        paf.type           = 0;
        paf.duration       = -1;
        paf.location       = APPLY_AC;
        paf.modifier       = -1 * number_range(2, 5);
        paf.bitvector      = 0;
        affect_to_obj(obj, &paf);
        break;
    }
    obj->cost += 10;

    act("$n grunts '#yThere, best job I ever done.#n'.", keeper, NULL, ch, TO_VICT);
  }
  else
  {
    act("$n grunts '#yDon't waste my time, if you want something say it!#n'.", keeper, NULL, ch, TO_VICT);
  }
}

void shopspec_gemenchanter(CHAR_DATA *keeper, CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj, *pass;
  ITERATOR *pIter;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  argument = one_argument(argument, arg3);

  if (!str_cmp(arg1, "list"))
  {
    act("$n says '#yI can lay the following enchantments on weapons.#n'.\n\r", keeper, NULL, ch, TO_VICT);
    send_to_char(" [o] Magic-missile\n\r", ch);
    send_to_char(" [o] Group-heal\n\r", ch);
    send_to_char(" [o] Earthquake\n\r", ch);
    send_to_char(" [0] Remove-spell\n\r", ch);
  }
  else if (!str_cmp(arg1, "buy"))
  {
    if ((obj = get_obj_carry(ch, arg3)) == NULL)
    {
      act("$n asks '#yWhat weapon do you want enchanted?#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }
    else if (obj->item_type != ITEM_WEAPON)
    {         
      act("$n says '#yI can only enchant weapons.#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }
    else if (obj->value[0] != 0 && str_cmp(arg2, "remove-spell"))
    {
      act("$n says '#yThis weapon have already been enchanted.#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }

    /* check to see if he can pay */
    pIter = AllocIterator(ch->carrying);
    while ((pass = (OBJ_DATA *) NextInList(pIter)) != NULL)
    {
      if (pass->pIndexData->vnum != OBJ_VNUM_GEMSTONEPASS)
        continue;

      break;
    }

    if (pass == NULL)
    {
      act("$n says '#yYou need a gemstone pass before I can sell you anything. Go ask the Queen.#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }

    if (!str_cmp(arg2, "remove-spell"))
    {
      obj->value[0] = 0;
      act("$n says '#yThe item has been disenchanted.#n'.", keeper, NULL, ch, TO_VICT);
    }
    else if (!str_cmp(arg2, "magic-missile"))
    {
      obj->value[0] = skill_lookup("magic missile");
      act("$n says '#yThe item has been enchanted.#n'.", keeper, NULL, ch, TO_VICT);
    }
    else if (!str_cmp(arg2, "earthquake"))
    {
      obj->value[0] = skill_lookup("earthquake");
      act("$n says '#yThe item has been enchanted.#n'.", keeper, NULL, ch, TO_VICT);
    }
    else if (!str_cmp(arg2, "group-heal"))
    {
      obj->value[0] = skill_lookup("group heal");
      act("$n says '#yThe item has been enchanted.#n'.", keeper, NULL, ch, TO_VICT);
    }
    else
    {
      act("$n says '#yI'm afraid I cannot cast that enchantment.#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }

    extract_obj(pass);
    sound_to_char("pay.wav", ch);
  }
}

void shopspec_gemdragon(CHAR_DATA *keeper, CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char item[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  ITERATOR *pIter;
  OBJ_DATA *obj;

  argument = one_argument(argument, arg);
  one_argument(argument, item);

  if (!str_cmp(arg, "list"))
  {
    if (!keeper->carrying)
    {
      act("$n says '#yI'm afraid I'm not selling anything right now.#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }
    else if (item[0] == '\0')
    {
      act("$n says '#yTake a look at my goods.#n'.", keeper, NULL, ch, TO_VICT);
      send_to_char("\n\r", ch);

      pIter = AllocIterator(keeper->carrying);
      while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
      {
        sprintf(buf, " [o] %s\n\r", obj->short_descr);
        send_to_char(buf, ch);
      }
      return;
    }
    else
    {
      pIter = AllocIterator(keeper->carrying);
      while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
      {
        if (is_name(item, obj->name))
          break;
      }
      if (obj)
      {
        int sn;

        if ((sn = skill_lookup("identify")) <= 0)
        {
          act("$n says '#yI'm broken, please tell an admin.#n'.", keeper, NULL, ch, TO_VICT);
          return;
        }
        act("$n shows you $s goods.", keeper, NULL, ch, TO_VICT);
        spell_identify(sn, 50, ch, obj);
        return;
      }
      else
      {
        act("$n says '#yI do not sell that.#n'.", keeper, NULL, ch, TO_VICT);
        return;
      }
    }
  }
  else if (!str_cmp(arg, "buy"))
  {
    if (!keeper->carrying)
    {
      act("$n says '#yI'm afraid I'm not selling anything right now.#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }
    else
    {
      pIter = AllocIterator(keeper->carrying);
      while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
      {
        if (is_full_name(item, obj->name))
        {
          ITERATOR *pIter2;
          OBJ_DATA *pass;

          pIter2 = AllocIterator(ch->carrying);
          while ((pass = (OBJ_DATA *) NextInList(pIter2)) != NULL)
          {
            if (pass->pIndexData->vnum != OBJ_VNUM_GEMSTONEPASS)
              continue;

            break;
          }

          if (pass == NULL)
          {
            act("$n says '#yYou need a gemstone pass before I can sell you anything. Go ask the Queen.#n'.", keeper, NULL, ch, TO_VICT);
            return;
          }

          extract_obj(pass);

          obj = create_object(get_obj_index(obj->pIndexData->vnum), 50);
          obj_to_char(obj, ch);
          act("$N gives you $p.", ch, obj, keeper, TO_CHAR);
          act("$N gives $n $p.", ch, obj, keeper, TO_ROOM);
          act("$n says '#yNice doing buisness with you.#n'.", keeper, NULL, ch, TO_VICT);

          sound_to_char("pay.wav", ch);
          return;
        }
      }
      act("$n says '#yI'm afraid I don't sell that here.#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }
  }
  else act("$n says '#yI'm afraid I don't understand.#n'.", keeper, NULL, ch, TO_VICT);
}

void shopspec_generic(CHAR_DATA *keeper, CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH]; 
  char item[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  ITERATOR *pIter;
  OBJ_DATA *obj;

  argument = one_argument(argument, arg);
  one_argument(argument, item);

  if (!str_cmp(arg, "list"))
  {
    if (!keeper->carrying)
    {
      act("$n says '#yI'm afraid I'm not selling anything right now.#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }
    else if (item[0] == '\0')
    {
      act("$n says '#yTake a look at my goods.#n'.", keeper, NULL, ch, TO_VICT);
      send_to_char("\n\r", ch);

      pIter = AllocIterator(keeper->carrying);
      while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
      {
        sprintf(buf, " [o] %-40.40s   %4d\n\r", obj->short_descr, obj->cost);
        send_to_char(buf, ch);
      }
      return;
    }
    else
    {
      pIter = AllocIterator(keeper->carrying);
      while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
      {
        if (is_name(item, obj->name))
          break;
      }
      if (obj)
      {
        int sn;

        if ((sn = skill_lookup("identify")) <= 0)
        {
          act("$n says '#yI'm broken, please tell an admin.#n'.", keeper, NULL, ch, TO_VICT);
          return;
        }
        act("$n shows you $s goods.", keeper, NULL, ch, TO_VICT);
        spell_identify(sn, 50, ch, obj);
        return;
      }
      else
      {
        act("$n says '#yI do not sell that.#n'.", keeper, NULL, ch, TO_VICT);
        return;
      }
    }
  }
  else if (!str_cmp(arg, "buy"))
  {
    if (!keeper->carrying)
    {
      act("$n says '#yI'm afraid I'm not selling anything right now.#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }
    else
    {
      pIter = AllocIterator(keeper->carrying);
      while ((obj = (OBJ_DATA *) NextInList(pIter)) != NULL)
      {
        if (is_full_name(item, obj->name))
        {
          if (getGold(ch) < obj->cost)
          {
            act("$n says '#yYou cannot afford that item.#n'.", keeper, NULL, ch, TO_VICT);
            return;
          }
          else setGold(ch, -1 * obj->cost);
          obj = create_object(get_obj_index(obj->pIndexData->vnum), 50);
          obj_to_char(obj, ch);
          act("$N gives you $p.", ch, obj, keeper, TO_CHAR);
          act("$N gives $n $p.", ch, obj, keeper, TO_ROOM);
          act("$n says '#yNice doing buisness with you.#n'.", keeper, NULL, ch, TO_VICT);

          sound_to_char("pay.wav", ch);

          return;
        }
      }
      act("$n says '#yI'm afraid I don't sell that here.#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }
  }
  else act("$n says '#yI'm afraid I don't understand.#n'.", keeper, NULL, ch, TO_VICT);
}

void shopspec_enchanter(CHAR_DATA *keeper, CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  argument = one_argument(argument, arg3);

  if (!str_cmp(arg1, "list"))
  {
    act("$n says '#yI can lay the following enchantments on armour.#n'.\n\r", keeper, NULL, ch, TO_VICT);
    send_to_char("* Sanctuary           200\n\r", ch);
    send_to_char("* Fly                 150\n\r", ch);
    send_to_char("* Passdoor             75\n\r", ch);
    send_to_char("* Regeneration        250\n\r", ch);
    send_to_char("* Remove-spell         50\n\r", ch);
    send_to_char("* Invisibility        125\n\r", ch);
  }
  else if (!str_cmp(arg1, "buy"))
  {
    if ((obj = get_obj_carry(ch, arg3)) == NULL)
    {
      act("$n asks '#yWhat piece of armour do you want enchanted?#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }
    else if (obj->item_type != ITEM_ARMOR)
    {
      act("$n says '#yI can only enchant armour.#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }
    else if (obj->value[3] != 0 && str_cmp(arg2, "remove-spell"))
    {
      act("$n says '#yThis piece of armour have already been enchanted.#n'.", keeper, NULL, ch, TO_VICT);
      return;
    }
    if (!str_cmp(arg2, "remove-spell"))
    {
      int cost = 50;

      if (getGold(ch) < cost)
      {
        act("$n says '#yYou cannot afford to disenchant that item.#n'.", keeper, NULL, ch, TO_VICT);
        return;
      }

      obj->value[3] = 0;
      act("$n says '#yThe item has been disenchanted.#n'.", keeper, NULL, ch, TO_VICT);
      setGold(ch, -1 * cost);
    }
    else if (!str_cmp(arg2, "sanctuary"))
    {
      int cost = 200;

      if (getGold(ch) < cost)
      {
        act("$n says '#yYou cannot afford that enchantment.#n'.", keeper, NULL, ch, TO_VICT);
        return;
      }

      obj->value[3] = OBJECT_SANCTUARY;
      act("$n says '#yThe item has been enchanted.#n'.", keeper, NULL, ch, TO_VICT);
      setGold(ch, -1 * cost);
    }
    else if (!str_cmp(arg2, "invisibility"))
    {
      int cost = 125;
     
      if (getGold(ch) < cost)
      {
        act("$n says '#yYou cannot afford that enchantment.#n'.", keeper, NULL, ch, TO_VICT);
        return;
      }

      obj->value[3] = OBJECT_INVISIBLE;
      act("$n says '#yThe item has been enchanted.#n'.", keeper, NULL, ch, TO_VICT);
      setGold(ch, -1 * cost);
    }
    else if (!str_cmp(arg2, "fly"))
    {
      int cost = 150;

      if (getGold(ch) < cost)
      {
        act("$n says '#yYou cannot afford that enchantment.#n'.", keeper, NULL, ch, TO_VICT);
        return;
      }

      obj->value[3] = OBJECT_FLYING;
      act("$n says '#yThe item has been enchanted.#n'.", keeper, NULL, ch, TO_VICT);
      setGold(ch, -1 * cost);
    }
    else if (!str_cmp(arg2, "passdoor"))
    {
      int cost = 75;

      if (getGold(ch) < cost)
      {
        act("$n says '#yYou cannot afford that enchantment.#n'.", keeper, NULL, ch, TO_VICT);
        return;
      }

      obj->value[3] = OBJECT_PASSDOOR;
      act("$n says '#yThe item has been enchanted.#n'.", keeper, NULL, ch, TO_VICT);
      setGold(ch, -1 * cost);
    }
    else if (!str_cmp(arg2, "regeneration"))
    {
      int cost = 250;

      if (getGold(ch) < cost)
      {
        act("$n says '#yYou cannot afford that enchantment.#n'.", keeper, NULL, ch, TO_VICT);
        return;
      }

      obj->value[3] = OBJECT_REGENERATE;
      act("$n says '#yThe item has been enchanted.#n'.", keeper, NULL, ch, TO_VICT);
      setGold(ch, -1 * cost);
    }
    else
    {
      act("$n says '#yI'm afraid I cannot cast that enchantment.#n'.", keeper, NULL, ch, TO_VICT);
    }
  }
  else
  {
    act("$n says '#yI'm afraid I don't understand.#n'.", keeper, NULL, ch, TO_VICT);
  }
}
