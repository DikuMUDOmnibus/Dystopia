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

typedef struct webclient   WEBCLIENT;
typedef struct webserver   WEBSERVER;

#define MAX_BUFFER (MAX_STRING_LENGTH * 4)

struct webclient
{
  WEBCLIENT  * next;
  char         buffer[MAX_BUFFER];
  char         inbuf[MAX_BUFFER];
  int          outtop;
  int          sockfd;
  int          timeout;
  bool         ready;
};

struct webserver
{
  WEBCLIENT  * clients;
  int          sockfd;
  int          clientcount;
};
