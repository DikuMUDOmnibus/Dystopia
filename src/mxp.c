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
 * source file for mxp
 *
 * Brian Graversen
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dystopia.h"

/* telnet negotiating */
const unsigned char mxp_will [] = { IAC, WILL, TELOPT_MXP, '\0' };
const unsigned char mxp_do   [] = { IAC, DO,   TELOPT_MXP, '\0' };
const unsigned char mxp_dont [] = { IAC, DONT, TELOPT_MXP, '\0' };

/*
 * mxp_to_char()
 * -------------
 *
 * When sending MXP messages with this function, you should
 * always use <BR> instead of \n\r, since \n\r resets the
 * state of MXP. <BR> will do a linebreak just like \n\r,
 * but the state is kept.
 *
 * The following MXP tags are supported, and works well
 * in zmud 6.16 and mushclient 3.17. Other clients may
 * work, but it's very unlikely.
 *
 * <B>text</B>   : BOLD
 * <I>text</I>   : ITALIC
 * <U>text</U>   : UNDERLINE
 * <BR>          : LINEBREAK (is replaced with \n\r if no MXP)
 *
 * Some of these more advanced features are also supported
 * -------------------------------------------------------
 * <A href="URL">text</A>    : web hyperlinks.
 * <SEND [args]>text</SEND>  : mud hyperlinks and menus.
 * <EXPIRE>                  : used for stopping links.
 *
 */
void mxp_to_char(char *txt, CHAR_DATA *ch, int mxp_style)
{
  char buf[2*MAX_STRING_LENGTH];
  int i = 0, j = 0;

  if (!ch->desc)
    return;

  if (USE_MXP(ch))
  {
    switch(mxp_style)
    {
      default:
	bug("Mxp_to_char: strange style '%d'.", mxp_style);
	strcpy(buf, txt);
      case MXP_SAFE:
        sprintf(buf, "%c%s%s%c%s", 27, "[0z", txt, 27, "[7z");
	break;
      case MXP_ALL:
	/* There is a bug in zmud 6.16 so we can not do the obvious
	 * here. Instead we'll use this workaround. In the future,
	 * the bug in zmud might be fixed, and the following line
	 * can replace everything else.
	 *
	 *   sprintf(buf, "%c%s%s%c%s", 27, "[1z", txt, 27, "[7z");
	 *
	 */
	buf[j++] = 27; buf[j++] = '['; buf[j++] = '1'; buf[j++] = 'z';
	while (txt[i] != '\0')
	{
          switch(txt[i])
	  {
            default:
	      buf[j++] = txt[i++];
	      break;
            case '<':
	      if (!memcmp(&txt[i], "<BR>", strlen("<BR>")))
	      {
	        i += strlen("<BR>");
		buf[j++] = '\n'; buf[j++] = '\r';
                buf[j++] = 27; buf[j++] = '['; buf[j++] = '1'; buf[j++] = 'z';
	      }
              else
                buf[j++] = txt[i++];
	      break;
	  }
	}
        buf[j++] = 27; buf[j++] = '['; buf[j++] = '7'; buf[j++] = 'z';
        buf[j] = '\0';
    	break;
      case MXP_NONE:
        sprintf(buf, "%c%s%s", 27, "[7z", txt);
	break;
    }
  }
  else
  {
    while(txt[i] != '\0')
    {
      switch(txt[i])
      {
        default:
	  buf[j++] = txt[i++];
	  break;
        case '<':
	  if (!memcmp(&txt[i], "<B>", strlen("<B>")))
	    i += strlen("<B>");
	  else if (!memcmp(&txt[i], "</B>", strlen("</B>")))
	    i += strlen("</B>");
	  else if (!memcmp(&txt[i], "<U>", strlen("<U>")))
	    i += strlen("<U>");
	  else if (!memcmp(&txt[i], "</U>", strlen("</U>")))
	    i += strlen("</U>");
	  else if (!memcmp(&txt[i], "<I>", strlen("<I>")))
	    i += strlen("<I>");
	  else if (!memcmp(&txt[i], "</I>", strlen("</I>")))
	    i += strlen("</I>");
	  else if (!memcmp(&txt[i], "<BR>", strlen("<BR>")))
	  {
            if (mxp_style == MXP_ALL)
            {
              buf[j++] = '\n';
              buf[j++] = '\r';
            }
	    i += strlen("<BR>");
	  }
	  else if (!memcmp(&txt[i], "<SEND", strlen("<SEND")))
	  {
	    i += strlen("<SEND");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<A href", strlen("<A href")))
	  {
	    i += strlen("<A href");
	    while(txt[i++] != '>')
              ;
	  }
          else if (!memcmp(&txt[i], "<EXPIRE", strlen("<EXPIRE")))
          {
            i += strlen("<EXPIRE");
            while(txt[i++] != '>')
              ;
          }
	  else if (!memcmp(&txt[i], "</SEND>", strlen("</SEND>")))
	    i += strlen("</SEND>");
	  else if (!memcmp(&txt[i], "</A>", strlen("</A>")))
	    i += strlen("</A>");
          else
	    buf[j++] = txt[i++];
	  break;
      }
    }
    buf[j] = '\0';
  }

  send_to_char(buf, ch);
}

void init_mxp(DESCRIPTOR_DATA *d)
{
  const unsigned char enable_mxp[] = { IAC, SB, TELOPT_MXP, IAC, SE, '\0' };
  const char lock_mode[] = { 27, '[', '7', 'z', '\0' };

  write_to_buffer(d, (char *) enable_mxp, 0);
  write_to_buffer(d, lock_mode, 0); /* default to locked mode */
  d->mxp = TRUE;
}

void shutdown_mxp(DESCRIPTOR_DATA *d)
{
  d->mxp = FALSE;
}
