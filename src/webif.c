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
#include <stdio.h> 
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "dystopia.h"
#include "webif.h"
#include "olc.h"

/* this is our cute little webserver */
WEBSERVER *pWebServer = NULL;

/* we need a local variable */
int    streamTableSize = 0;
char **streamTable[2];
char   escapeSymbol[] = { 27, '\0' };

/* local functions */
char *getEntry                   ( char *key );
char *get_web_help               ( char *txt );
bool  streamBuffer               ( WEBCLIENT *wc );
void  send_to_webclient          ( WEBCLIENT *wc, char *txt, bool preparse );
void  flush_webclient            ( WEBCLIENT *wc );
void  close_webclient            ( WEBCLIENT *wc );
void  handle_web_who             ( WEBCLIENT *wc );
void  handle_web_leader          ( WEBCLIENT *wc );
void  handle_web_finger          ( WEBCLIENT *wc );
void  handle_web_noteboard       ( WEBCLIENT *wc );
void  handle_web_helpfile        ( WEBCLIENT *wc );
void  handle_web_postboard       ( WEBCLIENT *wc );
void  handle_web_statistics      ( WEBCLIENT *wc );
void  handle_web_changes         ( WEBCLIENT *wc );
void  handle_web_login           ( WEBCLIENT *wc );
void  handle_web_prefs           ( WEBCLIENT *wc );
void  handle_web_changeprefs     ( WEBCLIENT *wc );
void  handle_web_account_level   ( WEBCLIENT *wc );
void  handle_web_account_players ( WEBCLIENT *wc );
void  handle_web_load_room       ( WEBCLIENT *wc );
void  handle_web_save_room       ( WEBCLIENT *wc );
void  handle_web_404             ( WEBCLIENT *wc );

ACCOUNT_DATA *load_web_account   ( char *account, char *password );

/* Start listening for HTTP requests on the given port.
 * Will setup everything needed, remember to shutdown
 * the webserver when closing the mud or doing a copyover.
 */
void init_webserver(int port)
{
  struct sockaddr_in my_addr;
  static struct sockaddr_in zero_addr;
  int reuse = 1;

  /* check to see if we already have a webserver */
  if (pWebServer != NULL)
  {
    bug("Calling init_webserver to many times.", 0);
    return;
  }

  /* allocate memory for our webserver */
  pWebServer = malloc(sizeof(*pWebServer));
  pWebServer->clients = NULL;
  pWebServer->clientcount = 0;

  /* get a control descriptor */
  if ((pWebServer->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    bug("Init_webserver: failed to call socket().", 0);
    abort();
  }

  /* setting the correct values */
  my_addr            = zero_addr;
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  my_addr.sin_port   = htons(port);

  /* set the reuse flag to 1 (turning it on) */
  if ((setsockopt(pWebServer->sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int))) == -1)
  {
    bug("Init_webserver: error in setsockopt()", 0);
    abort();
  }

  /* bind the port */
  if ((bind(pWebServer->sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))) == -1)
  {
    perror("Init_webserver: bind");
    exit(1);
  }

  /* start listening */
  listen(pWebServer->sockfd, 3);

  log_string("WebServer is ready to rock on port %d.", port);
}

/* Stop the webserver, will only work if we already have
 * a listening webserver running. If this is not the case
 * calling this function will do nothing. This clears up
 * any memory related to the webserver.
 */
void close_webserver()
{
  WEBCLIENT *wc, *wc_next;

  /* do we even have a webserver running ? */
  if (pWebServer == NULL) return;

  /* close the listening port */
  close(pWebServer->sockfd);

  /* run through all the clients and flush them */
  for (wc = pWebServer->clients; wc; wc = wc_next)
  {
    wc_next = wc->next;

    flush_webclient(wc);
  }

  /* free the memory for the webserver */
  free(pWebServer);

  /* NULL'ify the webserver */
  pWebServer = NULL;
}

/* This should be called every gamepulse (or perhaps every
 * second, if it's heavily used). It will handle any incoming
 * requests to the port the webserver is listening on.
 * If no webserver is running, this will do nothing.
 */
void poll_web_requests()
{
  WEBCLIENT *wc, *wc_next;
  static struct timeval tv;
  fd_set fSet;
  int fmax;

  /* only do something if we have a webserver */
  if (pWebServer == NULL) return;

  /* set the default value for max descriptor */
  fmax = pWebServer->sockfd;

  /* clear out the file socket set */
  FD_ZERO(&fSet);

  /* add the webserver to the set */
  FD_SET(pWebServer->sockfd, &fSet);

  /* clear clientcount */
  pWebServer->clientcount = 0;

  /* add any old webclients to the list */
  for (wc = pWebServer->clients; wc; wc = wc->next)
  {
    FD_SET(wc->sockfd, &fSet);

    /* update fmax */
    if (wc->sockfd > fmax)
      fmax = wc->sockfd;

    pWebServer->clientcount++;
  }

  /* wait for something to happen (the waittime is set to 0, so...) */
  if (select(fmax + 1, &fSet, NULL, NULL, &tv) < 0)
    return;

  /* accept new connections */
  if (FD_ISSET(pWebServer->sockfd, &fSet))
  {
    struct sockaddr_in sock;
    int newConnection;
/*
    unsigned int socksize;
*/
    socklen_t socksize;

    socksize = sizeof(sock);
    if ((newConnection = accept(pWebServer->sockfd, (struct sockaddr*) &sock, &socksize)) >=0)
    {
      int argp = 1;

      /* allocate memory */
      wc = malloc(sizeof(*wc));

      /* set default values */
      wc->buffer[0] = '\0';
      wc->inbuf[0]  = '\0';
      wc->outtop    = 0;
      wc->sockfd    = newConnection;
      wc->ready     = FALSE;

      /* handle timeout....
       * we timeout instantly if we have more than 10 clients
       */
      if (pWebServer->clientcount > 10)
        wc->timeout   = 1;
      else
        wc->timeout   = 100;

      /* set the client as non-blocking */
      ioctl(newConnection, FIONBIO, &argp);

      /* attach to webservers list */
      wc->next = pWebServer->clients;
      pWebServer->clients = wc;
    }
  }

  /* read from all webclients that wants to be read from */
  for (wc = pWebServer->clients; wc; wc = wc_next)
  {
    bool closed = FALSE;
    int size = strlen(wc->inbuf);

    /* update next pointer */
    wc_next = wc->next;

    /* can we read anymore from this client ? */
    if (size >= (int) sizeof(wc->inbuf) - 2)
      continue;

    /* read as much as possible */
    for (;;)
    {
      int sInput;
      int wanted = sizeof(wc->inbuf) - 2 - size;

      sInput = read(wc->sockfd, wc->inbuf + size, wanted);

      if (sInput > 0)
      {
        size += sInput;
      }
      else if (errno == EAGAIN || sInput == wanted)
      {
        break;
      }
      else
      {
        close_webclient(wc);
        closed = TRUE;
        break;
      }
    }

    if (closed) continue;

    wc->inbuf[size] = '\0';

    if (size >= 4)
    {
      if (wc->inbuf[size - 1] == '\n' && wc->inbuf[size - 2] == '\r' &&
          wc->inbuf[size - 3] == '\n' && wc->inbuf[size - 4] == '\r')
        wc->ready = TRUE;
    }
  }
  
  /* examine input and handle it if possible */
  for (wc = pWebServer->clients; wc; wc = wc_next)
  {
    wc_next = wc->next;

    /* auto timeout after 25 seconds */
    if (--wc->timeout <= 0)
    {
      close_webclient(wc);
      continue;
    }

    if (wc->ready == FALSE)
      continue;

    /* after serving a webclient, we should close it */
    if (!str_prefix("GET wholist.html", wc->inbuf))
    {
      handle_web_who(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
    else if (!str_prefix("GET leader.html", wc->inbuf))
    {
      handle_web_leader(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
    else if (!str_prefix("GET stats.html", wc->inbuf))
    {
      handle_web_statistics(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
    else if (!str_prefix("GET finger.html", wc->inbuf))
    {
      handle_web_finger(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
    else if (!str_prefix("GET note.html", wc->inbuf))
    {
      handle_web_noteboard(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
    else if (!str_prefix("POST note.html", wc->inbuf))
    {
      handle_web_postboard(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
    else if (!str_prefix("POST login.html", wc->inbuf))
    {
      handle_web_login(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
    else if (!str_prefix("GET prefs.html", wc->inbuf))
    {
      handle_web_prefs(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
    else if (!str_prefix("POST prefs.html", wc->inbuf))
    {
      handle_web_changeprefs(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
    else if (!str_prefix("GET changes.html", wc->inbuf))
    {
      handle_web_changes(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
    else if (!str_prefix("GET help.html", wc->inbuf))
    {
      handle_web_helpfile(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
    else if (!str_prefix("REQUEST lvl.info", wc->inbuf))
    {
      handle_web_account_level(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
    else if (!str_prefix("REQUEST player.info", wc->inbuf))
    {
      handle_web_account_players(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
    else if (!str_prefix("GET build.html", wc->inbuf))
    {
      handle_web_load_room(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
    else if (!str_prefix("POST build.html", wc->inbuf))
    {
      handle_web_save_room(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
    else
    {
      handle_web_404(wc);
      flush_webclient(wc);
      close_webclient(wc);
    }
  }
}

char *getEntry(char *key)
{
  int i;

  for (i = 0; i < streamTableSize; i++)
  {
    if (!str_cmp(streamTable[0][i], key))
      return streamTable[1][i];
  }

  return NULL;
}

/* This function will clear the old streamTable, and
 * try to allocate a new one which will fit the size of
 * the new table given by wc->inbuf. It will return FALSE
 * if anything unexpected happened during the read.
 *
 * \e2\eaccount\eJobo\epassword\eSomePass\e
 *
 * The above result is transformed into a size 2 table with
 *
 *   Key      | Entry
 *  ------------------------
 *   account  | Jobo
 *   password | SomePass
 *
 * The table will be left at size 0 with nothing allocated
 * should something bad happen.
 */
bool streamBuffer(WEBCLIENT *wc)
{
  char buf[MAX_STRING_LENGTH];
  int i, j, newsize = 0;
  char *ptr;

  /* free the old streamTable if needed */
  if (streamTableSize > 0)
  {
    for (i = 0; i < streamTableSize; i++)
    {
      free_string(streamTable[0][i]);
      free_string(streamTable[1][i]);
    }
    free(streamTable[0]);
    free(streamTable[1]);
  }

  /* reset size of streamTable */
  streamTableSize = 0;

  /* find first escape entry */
  if ((ptr = strstr(wc->inbuf, escapeSymbol)) == NULL)
    return FALSE;

  while (isdigit(*(++ptr)))
  {
    newsize *= 10;
    newsize += *ptr - '0';
  }

  /* got anything for us ? */
  if (newsize <= 0)
    return FALSE;

  /* allocate memory for streamTable */
  streamTable[0] = calloc(newsize, sizeof(char *));
  streamTable[1] = calloc(newsize, sizeof(char *));

  /* now we start reading everything into the new table */
  for (i = 0; i < newsize; i++)
  {
    /* we expect to see an escape sequence now */
    if ((ptr = strstr(ptr, escapeSymbol)) == NULL)
    {
      for (j = 0; j < i; j++)
      {
        free_string(streamTable[0][j]);
        free_string(streamTable[1][j]);
      }

      free(streamTable[0]);
      free(streamTable[1]);

      return FALSE;
    }

    /* scan and copy the keyword */
    j = 0;
    while (*(++ptr) != 27 && *ptr != '\0' && j < MAX_STRING_LENGTH - 1)
      buf[j++] = *ptr;
    buf[j] = '\0';

    if ((ptr = strstr(ptr, escapeSymbol)) == NULL)
    {
      for (j = 0; j < i; j++)
      {
        free_string(streamTable[0][j]);
        free_string(streamTable[1][j]);
      }

      free(streamTable[0]);
      free(streamTable[1]);

      return FALSE;
    }

    /* safely copy the old result */
    streamTable[0][i] = str_dup(buf);

    /* scan and copy the entryfield */
    j = 0;
    while (*(++ptr) != 27 && *ptr != '\0' && j < MAX_STRING_LENGTH - 1)
      buf[j++] = *ptr;
    buf[j] = '\0';

    /* safely copy the old result */
    streamTable[1][i] = str_dup(buf);
  }

  /* set the size of the new streamtable */
  streamTableSize = newsize;

  return TRUE;
}

/* Buffer data so it can be send later to this webclient.
 * Overflows are ignored, and ansi parsing is done here,
 * allowing for easy page-writing.
 */
void send_to_webclient(WEBCLIENT *wc, char *txt, bool preparse)
{
  static char output[MAX_BUFFER];
  bool underline = FALSE, overflow = FALSE;
  int iPtr = 0, last = -1, j, k;

  /* The color table... It uses clear websafe colors.
   * These colors are roughly the same as used on the
   * mud itself, but not exactly, so run a few tests :)
   */
  const struct sHTMLColor ansiTable[] =
  {
    { '8',  "<span class=\"black\">" },
    { '0',  "<span class=\"bblack\">" },
    { 'r',  "<span class=\"red\">" },
    { 'R',  "<span class=\"bred\">" },
    { 'g',  "<span class=\"green\">" },
    { 'G',  "<span class=\"bgreen\">" },
    { 'o',  "<span class=\"yellow\">" },
    { 'y',  "<span class=\"byellow\">" },
    { 'l',  "<span class=\"blue\">" },
    { 'L',  "<span class=\"bblue\">" },
    { 'p',  "<span class=\"purple\">" },
    { 'P',  "<span class=\"bpurple\">" },
    { 'c',  "<span class=\"cyan\">" },
    { 'C',  "<span class=\"bcyan\">" },
    { '7',  "<span class=\"white\">" },
    { '9',  "<span class=\"bwhite\">" },

    /* the end tag */
    { '\0',  "" }
  };

  while (*txt != '\0')
  {
    /* simple bound checking */
    if (iPtr > (MAX_BUFFER - 100))
    {
      overflow = TRUE;
      break;
    }
    switch(*txt)
    {
      default:
        output[iPtr++] = *txt++;
        break;
      case '\r':
        txt++;
        break;
      case '<':
        txt++;
        if (preparse)
        {
          output[iPtr++] = '&'; output[iPtr++] = 'l';
          output[iPtr++] = 't'; output[iPtr++] = ';';
        }
        else output[iPtr++] = '<';
        break;
      case '"':
        txt++;
        if (preparse)
        {
          output[iPtr++] = '&'; output[iPtr++] = 'q';
          output[iPtr++] = 'u'; output[iPtr++] = 'o';
          output[iPtr++] = 't'; output[iPtr++] = ';';
        }
        else output[iPtr++] = '"';
        break;
      case '&':
        txt++;
        if (preparse)
        {
          output[iPtr++] = '&'; output[iPtr++] = 'a';
          output[iPtr++] = 'm'; output[iPtr++] = 'p';
          output[iPtr++] = ';';
        }
        else output[iPtr++] = '&';
        break;
      case '>':
        txt++;
        if (preparse) 
        {
          output[iPtr++] = '&'; output[iPtr++] = 'g';
          output[iPtr++] = 't'; output[iPtr++] = ';';
        }
        else output[iPtr++] = '>';
        break;
      case '#':
        txt++;
   
        /* toggle underline on/off with #u */
        if (*txt == '#')
        {
          output[iPtr++] = *txt++;
        }
        else if (*txt == 'u')
        {
          txt++;

          if (underline)   
          {
            underline = FALSE;

            output[iPtr++] = '<'; output[iPtr++] = '/';
            output[iPtr++] = 'u'; output[iPtr++] = '>';
          }
          else
          {
            underline = TRUE;

            output[iPtr++] = '<'; output[iPtr++] = 'u'; output[iPtr++] = '>';
          }
        }

        /* #n should clear all tags */
        else if (*txt == 'n')
        {
          txt++;

          if (underline)
          {
            underline = FALSE;
            output[iPtr++] = '<'; output[iPtr++] = '/';
            output[iPtr++] = 'u'; output[iPtr++] = '>';
          }

          if (last != -1)
          {
            output[iPtr++] = '<'; output[iPtr++] = '/'; output[iPtr++] = 's';
            output[iPtr++] = 'p'; output[iPtr++] = 'a'; output[iPtr++] = 'n';
            output[iPtr++] = '>';
            last = -1;
          }
        }

        /* check for valid color tag and parse */
        else
        {
          bool validTag = FALSE;

          for (j = 0; ansiTable[j].cString[0] != '\0'; j++)
          {
            if (*txt == ansiTable[j].cTag)
            {
              validTag = TRUE;

              /* we only add the color sequence if it's needed */
              if (last != j)
              {
                /* end underline - will be re-enabled */
                if (underline)
                {
                  output[iPtr++] = '<'; output[iPtr++] = '/'; output[iPtr++] = 'u'; output[iPtr++] = '>';
                }

                /* end previous color tag */
                if (last != -1)
                {
                  output[iPtr++] = '<'; output[iPtr++] = '/'; output[iPtr++] = 's';
                  output[iPtr++] = 'p'; output[iPtr++] = 'a'; output[iPtr++] = 'n';
                  output[iPtr++] = '>';
                }

                /* add the color from the table */
                for (k = 0; ansiTable[j].cString[k] != '\0'; k++)
                {
                  output[iPtr++] = ansiTable[j].cString[k];
                }

                /* re-enable underline if needed */
                if (underline)
                {
                  output[iPtr++] = '<'; output[iPtr++] = 'u'; output[iPtr++] = '>';
                }
              }

              /* remember the last color */
              last = j;
            }
          }

          /* it wasn't a valid color tag */
          if (!validTag)
          {
            output[iPtr++] = '#';
          }
          else
          {
            txt++;
          }
        }

        break;
    }
  }

  /* clean up any residue mess */
  if (underline)
  {
    output[iPtr++] = '<'; output[iPtr++] = '/';
    output[iPtr++] = 'u'; output[iPtr++] = '>';
  }
  if (last != -1)
  {
    output[iPtr++] = '<'; output[iPtr++] = '/'; output[iPtr++] = 's';
    output[iPtr++] = 'p'; output[iPtr++] = 'a'; output[iPtr++] = 'n';
    output[iPtr++] = '>';
  }
  output[iPtr] = '\0';

  /* make sure we know about any possible overflows */
  if (iPtr + wc->outtop >= MAX_BUFFER - 1)
  {
    bug("Send_to_webclient: output overflow - refusing to send data.", 0);
    return;
  }
  else if (overflow)
  {
    bug("Send_to_webclient: output overflow - avoided.", 0);
  }

  /* copy */
  strncpy(wc->buffer + wc->outtop, output, iPtr); 
  wc->outtop += iPtr;
}

/* This will flush a given webclients output buffer,
 * effectivly sending all data (if possible) to that
 * receiver.
 */
void flush_webclient(WEBCLIENT *wc)
{
  int iPtr, iBlck, iWrt;

  /* then we send whatever we have pending */
  for (iPtr = 0; iPtr < wc->outtop; iPtr += iWrt)
  {
    iBlck = UMIN(wc->outtop - iPtr, 4096);

    if ((iWrt = write(wc->sockfd, wc->buffer + iPtr, iBlck)) < 0)
    {
      perror("Flush_webclient:");
      return;
    }
    increase_total_output(iWrt);
  }

  /* reset top output pointer, in case we want to reuse */
  wc->outtop = 0;
}

/* Closes the webclient, freeing any residue memory.
 * The webclient is also unlinked from the webserver.
 */
void close_webclient(WEBCLIENT *wc)
{
  WEBCLIENT *wc_prev;

  if (wc == pWebServer->clients)
  {
    pWebServer->clients = wc->next;
  }
  else
  {
    for (wc_prev = pWebServer->clients; wc_prev && wc_prev->next != wc; wc_prev = wc_prev->next)
      ;
    if (wc_prev == NULL)
    {
      bug("Close_webclient: Unable to find webclient in webservers list.", 0);
      return;
    }

    /* unlink */
    wc_prev->next = wc->next;
  }

  /* close the client */
  close(wc->sockfd);

  /* and finally free memory */
  free(wc);
}

void handle_web_who(WEBCLIENT *wc)
{
  BUFFER *wholist;

  if ((wholist = get_who(NULL, "")) != NULL)
  {
    send_to_webclient(wc, wholist->data, TRUE);
    buffer_free(wholist);
  }
  else
  {
    send_to_webclient(wc, "Sorry, the web-who is broken.", FALSE);
  }
}

void handle_web_statistics(WEBCLIENT *wc)
{
  BUFFER *buf = buffer_new(MAX_STRING_LENGTH);

  /* add relevant game statistics */
  bprintf(buf, "<b>Data collected over the last</b>             %5d day%s and %d hour%s\n",
    (muddata.mudinfo[MUDINFO_UPDATED] / (120 * 24)),
    (muddata.mudinfo[MUDINFO_UPDATED] / (120 * 24) != 1) ? "s" : "",
    (muddata.mudinfo[MUDINFO_UPDATED] / 120) - (muddata.mudinfo[MUDINFO_UPDATED] / (120 * 24)) * 24,
    (muddata.mudinfo[MUDINFO_UPDATED] / 120 != 1) ? "s" : "");

  bprintf(buf, "<b>Average players online</b>                   %5d player%s\n",
   (muddata.mudinfo[MUDINFO_MCCP_USERS] + muddata.mudinfo[MUDINFO_OTHER_USERS]) /
   (muddata.mudinfo[MUDINFO_UPDATED]), ((muddata.mudinfo[MUDINFO_MCCP_USERS] +
    muddata.mudinfo[MUDINFO_OTHER_USERS]) / muddata.mudinfo[MUDINFO_UPDATED] != 1) ? "s" : "");

  bprintf(buf, "<b>Peak players online</b>                      %5d player%s\n",
    muddata.mudinfo[MUDINFO_PEAK_USERS], (muddata.mudinfo[MUDINFO_PEAK_USERS] != 1) ? "s" : "");
  bprintf(buf, "<b>Number of rooms</b>                          %5d rooms\n", top_room);
  bprintf(buf, "<b>Number of areas</b>                          %5d areas\n", top_area);

  bprintf(buf, "<b>MCCP users</b>                               %5d %%\n",
    (muddata.mudinfo[MUDINFO_MCCP_USERS] + muddata.mudinfo[MUDINFO_OTHER_USERS])
    ? (100 * muddata.mudinfo[MUDINFO_MCCP_USERS]) /
    (muddata.mudinfo[MUDINFO_MCCP_USERS] + muddata.mudinfo[MUDINFO_OTHER_USERS]) : 0);

  bprintf(buf, "<b>MSP users</b>                                %5d %%\n",
    (muddata.mudinfo[MUDINFO_MCCP_USERS] + muddata.mudinfo[MUDINFO_OTHER_USERS])
    ? (100 * muddata.mudinfo[MUDINFO_MSP_USERS]) /
    (muddata.mudinfo[MUDINFO_MCCP_USERS] + muddata.mudinfo[MUDINFO_OTHER_USERS]) : 0);

  bprintf(buf, "<b>MXP users</b>                                %5d %%\n",
    (muddata.mudinfo[MUDINFO_MCCP_USERS] + muddata.mudinfo[MUDINFO_OTHER_USERS])
    ? (100 * muddata.mudinfo[MUDINFO_MXP_USERS]) /
    (muddata.mudinfo[MUDINFO_MCCP_USERS] + muddata.mudinfo[MUDINFO_OTHER_USERS]) : 0);

  bprintf(buf, "<b>Events currently queued</b>                  %5d events\n", muddata.events_queued);
  bprintf(buf, "<b>Max events queued this week</b>              %5d events\n", muddata.events_allocated);

  bprintf(buf, "</pre><h3>Last Weeks Game Statistics</h3><pre>\n");

  bprintf(buf, "<b>Shadowlords</b>          %3d player%s        <b>Giants</b>               %3d player%s\n",
    muddata.class_count[0], (muddata.class_count[0] != 1) ? "s" : " ",
    muddata.class_count[1], (muddata.class_count[1] != 1) ? "s" : " ");

  bprintf(buf, "<b>Warlocks</b>             %3d player%s        <b>Faes</b>                 %3d player%s\n",
    muddata.class_count[2], (muddata.class_count[2] != 1) ? "s" : " ",  
    muddata.class_count[3], (muddata.class_count[3] != 1) ? "s" : " ");

  send_to_webclient(wc, buf->data, FALSE);

  buffer_free(buf);
}

void handle_web_leader(WEBCLIENT *wc)
{
  BUFFER *leaderlist;

  if ((leaderlist = get_leader()) != NULL)
  {
    send_to_webclient(wc, leaderlist->data, TRUE);
    buffer_free(leaderlist);
  }
  else
  {
    send_to_webclient(wc, "Sorry, the leaderboard is broken.", FALSE);
  }
}

void handle_web_404(WEBCLIENT *wc)
{
  send_to_webclient(wc, "<h1>404 Unknown Service Request</h1>\n", FALSE);
  send_to_webclient(wc, "I'm afraid that service does not exist.", FALSE);
}

void handle_web_finger(WEBCLIENT *wc)
{
  char *ptr;
  BUFFER *finger;

  streamBuffer(wc);

  if ((ptr = getEntry("name")) == NULL)
  {
    send_to_webclient(wc, "Whom exactly did you want to finger?", FALSE);
    return;
  }

  if (strlen(ptr) >= 3 && (finger = get_finger(NULL, ptr)) != NULL)
  {
    send_to_webclient(wc, finger->data, TRUE);
    buffer_free(finger);
  }
  else
  {
    send_to_webclient(wc, "The Web-Finger was unable to locate that player.\n", FALSE);
  }
}

void handle_web_changes(WEBCLIENT *wc)
{
  CHANGE_DATA *change;
  BUFFER *buf = buffer_new(MAX_STRING_LENGTH);
  ITERATOR *pIter;
  bool found = FALSE;

  pIter = AllocIterator(change_list);
  while ((change = (CHANGE_DATA *) NextInList(pIter)) != NULL)
  {
    found = TRUE;

    bprintf(buf, " #G%-8s #0%s#n\n\r",
      change->date, line_indent(change->text, 10, 90));
  }

  if (found)
    send_to_webclient(wc, buf->data, FALSE);

  buffer_free(buf);
}

void handle_web_changeprefs(WEBCLIENT *wc)
{
  char buf[MAX_INPUT_LENGTH];
  ACCOUNT_DATA *acc;
  bool changed = FALSE;
  char *ptr, *password, *account;

  streamBuffer(wc);

  if ((account = getEntry("account")) == NULL ||
      (password = getEntry("password")) == NULL ||
      (acc = load_web_account(account, password)) == NULL)
  {
    send_to_webclient(wc, "Your password/accountname is invalid.", FALSE);
    return;
  }

  if ((ptr = getEntry("reference")) != NULL)
  {
    ACCOUNT_DATA *test;

    if ((test = load_account(ptr)) != NULL)
    {
      if (can_refer(acc))
      {
        free_string(acc->reference);
        acc->reference = str_dup(buf);
        changed = TRUE;
      }
      close_account(test);
    }
  }

  if ((ptr = getEntry("timezone")) != NULL)
  {
    int timez = atoi(ptr);

    /* get us a valid timezone */
    while (timez < 0)
      timez += 24;
    timez = timez % 24;

    ptr = ctime(&current_time);
    ptr[13] = '\0';
    acc->timezone = timez - atoi(&ptr[11]);

    /* fix out timezone between 0 and 23 */
    while (acc->timezone < 0)
      acc->timezone += 24;
    acc->timezone = acc->timezone % 24;

    changed = TRUE;
  }

  if ((ptr = getEntry("vacation")) != NULL)
  {
    if ((IS_SET(acc->flags, ACCOUNT_FLAG_VACATION) && !str_cmp(ptr, "false")) ||
       (!IS_SET(acc->flags, ACCOUNT_FLAG_VACATION) && !str_cmp(ptr, "true")))
    {
      TOGGLE_BIT(acc->flags, ACCOUNT_FLAG_VACATION);
      changed = TRUE;
    }
  }

  if (changed)
  {
    send_to_webclient(wc, "All valid changes saved!", FALSE);
    save_account(acc); 
  }
  else
  {
    send_to_webclient(wc, "No valid changes made, nothing is saved!", FALSE);
  }

  close_account(acc);
}

void handle_web_prefs(WEBCLIENT *wc)
{
  ACCOUNT_DATA *acc;
  char buf[MAX_INPUT_LENGTH];
  struct plist *p_list;
  char *ptr, *account, *password;
  int hours;

  streamBuffer(wc);

  if ((account = getEntry("account")) == NULL ||
      (password = getEntry("password")) == NULL ||
      (acc = load_web_account(account, password)) == NULL)
  {
    send_to_webclient(wc, "Your password/accountname is invalid.", FALSE);
    return;
  }

  p_list = parse_player_list(acc->players);

  send_to_webclient(wc, "<pre>\n", FALSE);
  send_to_webclient(wc, "      #9#uName          Class              Hours  #n\n", FALSE);
  send_to_webclient(wc, p_list->text, FALSE);
  send_to_webclient(wc, "\n\n", FALSE);

  if (acc->created != 0)
  {
    sprintf(buf, " Account created               %s", ctime(&acc->created));
    send_to_webclient(wc, buf, FALSE);
  }
  if (acc->lastlogged != 0)
  {
    sprintf(buf, " Last connected                %s", ctime(&acc->lastlogged));
    send_to_webclient(wc, buf, FALSE);
  }
  if (acc->reference[0] != '\0')
  {
    sprintf(buf, " Referencing account           %s\n", acc->reference);
    send_to_webclient(wc, buf, FALSE);
  }
  if (acc->denied > current_time)
  {
    int denytime = (acc->denied - current_time) / 3600 + 1;

    sprintf(buf, " You are denied for another %4d hour%s\n", denytime, (denytime != 1) ? "s" : "");
    send_to_webclient(wc, buf, FALSE);
  }
  send_to_webclient(wc, "</pre>\n", FALSE);
  send_to_webclient(wc, "<form method=\"post\">\n", FALSE);
  send_to_webclient(wc, "<table cellpadding=\"2\">\n", FALSE);

  /* vacation flag */
  send_to_webclient(wc, "<tr><td>Account Status</td>", FALSE);
  if (IS_SET(acc->flags, ACCOUNT_FLAG_VACATION))
  {
    send_to_webclient(wc, "<td><input type=\"radio\" name=\"vacation\" value=\"false\">active</input>\n", FALSE);
    send_to_webclient(wc, "<input checked type=\"radio\" name=\"vacation\" value=\"true\">vacation</input></td></tr>\n", FALSE);
  }
  else
  {
    send_to_webclient(wc, "<td><input checked type=\"radio\" name=\"vacation\" value=\"false\">active</input>\n", FALSE);
    send_to_webclient(wc, "<input type=\"radio\" name=\"vacation\" value=\"true\">vacation</input></td></tr>\n", FALSE);
  }

  /* set timezone */
  ptr = ctime(&current_time);
  ptr[13] = '\0';
  ptr[16] = '\0';
  hours = (UMAX(-1 * (atoi(&ptr[11]) + acc->timezone), (atoi(&ptr[11]) + acc->timezone))) % 24;
  sprintf(buf, "<tr><td>Set Timezone</td>\n"
               "<td><input type=\"text\" size=\"2\" maxlength=\"2\" name=\"timezone\" value=\"%s%d\"> : %s</input></td></tr>\n",
    (hours < 10) ? "0" : "", hours, &ptr[14]);
  send_to_webclient(wc, buf, FALSE);

  /* change account reference */
  if (can_refer(acc))
  {
    send_to_webclient(wc, "<tr><td>Set Reference</td>\n"
                          "<td><input type=\"text\" size=\"12\" name=\"reference\"></input></td></tr>", FALSE);
  }

  /* end of form */
  send_to_webclient(wc, "</table><p></p>\n", FALSE);
  send_to_webclient(wc, "<input type=\"submit\" value=\"Save Settings\"></input></form>\n", FALSE);


  /* clean up our mess */
  close_account(acc);
  free(p_list);
}

ACCOUNT_DATA *load_web_account(char *account, char *password)
{
  ACCOUNT_DATA *acc;

  if ((acc = load_account(account)) == NULL)
    return NULL;

  if (str_cmp(crypt(password, acc->owner), acc->password))
  {
    close_account(acc);
    return NULL;
  }

  return acc;
}

/* This functions returns a slightly different value, and the PHP scripts
 * connects to this function with a slightly different method. We need
 * to know if the login was succesful or not before we parse the page.
 */
void handle_web_login(WEBCLIENT *wc)
{
  ACCOUNT_DATA *acc;
  char *account, *password;

  streamBuffer(wc);

  if ((account = getEntry("account")) == NULL ||
      (password = getEntry("password")) == NULL ||
      (acc = load_web_account(account, password)) == NULL)
  {
    send_to_webclient(wc, "0", FALSE);
    return;
  }

  send_to_webclient(wc, "1", FALSE);
  close_account(acc);
}

void handle_web_save_room(WEBCLIENT *wc)
{
  ROOM_INDEX_DATA *pRoom;
  char desc[MAX_STRING_LENGTH];
  char *ptr;
  int i = 0, j;
  ACCOUNT_DATA *acc;
  char *account, *password;

  streamBuffer(wc);

  if ((account = getEntry("account")) == NULL ||
      (password = getEntry("password")) == NULL ||
      (acc = load_web_account(account, password)) == NULL)
  {
    send_to_webclient(wc, "Your password/accountname is invalid.", FALSE);
    return;
  }

  if (acc->level < BUILDER_ACCOUNT)
  {
    send_to_webclient(wc, "Account not builder!", FALSE);
    close_account(acc);
    return;
  }

  if ((ptr = getEntry("desc")) == NULL)
  {
    send_to_webclient(wc, "Description not submitted!", FALSE);
    close_account(acc);
    return;
  }

  strcpy(desc, ptr);
  for (i = 0, j = 0; desc[i] != '\0'; i++, j++)
  {
    if (desc[i] == '~')
      desc[j] = '-';
    else if (desc[i] == '\\' && desc[i+1] == '\'')
      desc[j] = desc[++i];
    else if (desc[i] == '\\' && desc[i+1] == '\\')
      desc[j] = desc[++i];
    else if (desc[i] == '\\' && desc[i+1] == '\"')
      desc[j] = desc[++i];
    else
      desc[j] = desc[i];
  }
  desc[j] = '\0';

  if (j <= 0)
  {
    send_to_webclient(wc, "Description not submitted!", FALSE);
    close_account(acc);
    return;
  }

  if ((ptr = getEntry("vnum")) == NULL)
  {
    send_to_webclient(wc, "[vnum] not submitted!", FALSE);
    close_account(acc);
    return;
  }

  i = atoi(ptr);
  if ((pRoom = get_room_index(i)) == NULL)
  {
    send_to_webclient(wc, "Room not found!", FALSE);
    close_account(acc);
    return;
  }

  if (!account_olc_area(acc, pRoom->area))
  {
    send_to_webclient(wc, "You do not have write access to this area.", FALSE);
    close_account(acc);
    return;
  }
  close_account(acc);

  /* make sure desc has a linebreak */
  strcat(desc, "\r\n");

  free_string(pRoom->description);
  pRoom->description = str_dup(format_string(desc));
  SET_BIT(pRoom->area->area_flags, AREA_CHANGED);

  send_to_webclient(wc, "Room description changed.", FALSE);
}

void handle_web_load_room(WEBCLIENT *wc)
{
  ACCOUNT_DATA *acc;
  ROOM_INDEX_DATA *pRoom;
  char *account, *password;
  char *ptr;
  int i = 0;

  streamBuffer(wc);

  if ((account = getEntry("account")) == NULL ||
      (password = getEntry("password")) == NULL ||
      (acc = load_web_account(account, password)) == NULL)
  {
    send_to_webclient(wc, "2", FALSE);
    return;
  }

  if (acc->level < BUILDER_ACCOUNT)
  {
    send_to_webclient(wc, "3", FALSE);
    close_account(acc);
    return;
  }

  if ((ptr = getEntry("vnum")) == NULL)
  {
    send_to_webclient(wc, "4", FALSE);
    close_account(acc);
    return;
  }

  i = atoi(ptr);
  if ((pRoom = get_room_index(i)) == NULL)
  {
    send_to_webclient(wc, "6", FALSE);
    close_account(acc);
    return;
  }

  if (!account_olc_area(acc, pRoom->area))
  {
    send_to_webclient(wc, "7", FALSE);
    close_account(acc);
    return;
  }
  close_account(acc);

  send_to_webclient(wc, "1", FALSE);
  send_to_webclient(wc, pRoom->description, FALSE);
}

void handle_web_account_players(WEBCLIENT *wc)
{
  ACCOUNT_DATA *acc;
  char arg[MAX_STRING_LENGTH];
  char *ptr, *account, *password;
  BUFFER *buf;

  streamBuffer(wc);

  if ((account = getEntry("account")) == NULL ||
      (password = getEntry("password")) == NULL ||
      (acc = load_web_account(account, password)) == NULL)
    return;

  buf = buffer_new(MAX_STRING_LENGTH);

  ptr = get_token(acc->players, arg);
  do
  {
    if (strlen(arg) >= 3)
      bprintf(buf, "%s%c", arg, 27);

    /* scan forward to next character */
    ptr = get_token(ptr, arg);
    ptr = get_token(ptr, arg);
    ptr = get_token(ptr, arg);
    ptr = get_token(ptr, arg);
  } while (*ptr != '\0');

  send_to_webclient(wc, buf->data, FALSE);

  buffer_free(buf);
  close_account(acc);
}

void handle_web_account_level(WEBCLIENT *wc)
{
  ACCOUNT_DATA *acc;
  char buf[MAX_INPUT_LENGTH];
  char *account, *password;

  streamBuffer(wc);

  if ((account = getEntry("account")) == NULL ||
      (password = getEntry("password")) == NULL ||
      (acc = load_web_account(account, password)) == NULL)
  {
    send_to_webclient(wc, "0", FALSE);
    return;
  }

  sprintf(buf, "%d", acc->level);
  send_to_webclient(wc, buf, FALSE);
  close_account(acc);
}

void handle_web_postboard(WEBCLIENT *wc)
{
  ACCOUNT_DATA *acc;
  char *playername, *account, *password, *contents1, *subject1, *to1, *boardname;
  char contents[2 * MAX_STRING_LENGTH];
  char subject[2 * MAX_STRING_LENGTH];
  char to[2 * MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int i = 0, flags = 0, j;
  const char emptystring[] = { '\0' };

  streamBuffer(wc);

  if ((account = getEntry("account")) == NULL ||
      (password = getEntry("password")) == NULL ||
      (acc = load_web_account(account, password)) == NULL)
  {
    send_to_webclient(wc, "Your password/accountname is invalid.", FALSE);
    return;
  }

  if ((playername = getEntry("poster")) == NULL)
  {
    send_to_webclient(wc, "You need to pick which character is posting the note.", FALSE);
    close_account(acc);
    return;
  }

  if ((contents1 = getEntry("contents")) == NULL)
  {
    send_to_webclient(wc, "Your note does not have any contents.", FALSE);
    close_account(acc);
    return;
  }

  for (i = 0, j = 0; contents1[i] != '\0'; i++)
  {
    if ((isascii(contents1[i]) && isprint(contents1[i])) || (isspace(contents1[i])))
    {
      switch(contents1[i])
      {
        default:
          contents[j++] = contents1[i];
          break;
        case '~':
          contents[j++] = '-';
          break;
        case '\\':
          switch(contents1[i+1])
          {
            default:
              contents[j++] = '\\';
              break;
            case '\\':
              contents[j++] = '\\';
              i++;
              break;
            case '\'':
              contents[j++] = '\'';
              i++;
              break;
            case '\"':
              contents[j++] = '\"';
              i++;
              break;
          }
          break;
      }
    }
  }
  contents[j] = '\0';

  if ((boardname = getEntry("board")) == NULL)
  {
    send_to_webclient(wc, "What board are you planning to post on?", FALSE);
    close_account(acc);
    return;
  }

  if ((subject1 = getEntry("subject")) == NULL)
  {
    send_to_webclient(wc, "Under what subject do you wish to post this note?", FALSE);
    close_account(acc);
    return;
  }
  strcpy(subject, subject1);
  smash_tilde(subject);

  if ((to1 = getEntry("to")) == NULL)
  {
    if (!str_cmp(boardname, "personal"))
    {
      send_to_webclient(wc, "Whom are you writing the note to?", FALSE);
      close_account(acc);
      return;
    }
    to1 = (char *) emptystring;
  }
  strcpy(to, to1);
  smash_tilde(to);

  if (acc->level < BUILDER_ACCOUNT &&
     (!str_cmp(boardname, "announce") ||
      !str_cmp(boardname, "immortal") ||
      !str_cmp(boardname, "builder")))
  {
    send_to_webclient(wc, "This account cannot post on this board.", FALSE);
    close_account(acc);
    return;
  }
  if (acc->max_might < RANK_CADET)
  {
    send_to_webclient(wc, "Your account cannot post notes.", FALSE);
    close_account(acc);
    return;
  }

  if (IS_SET(acc->flags, ACCOUNT_FLAG_SILENCED))
    flags = NOTE_FLAG_SILENCED;

  if (double_post(playername, boardname, subject))
  {
    send_to_webclient(wc, "Ooops! You pressed the \"post note\" button to many times.<br>", FALSE);
    send_to_webclient(wc, "Don't worry, only one noted was posted.", FALSE);
  }
  else
  {
    char to_list[MAX_STRING_LENGTH];

    if (!str_cmp(boardname, "general") || !str_cmp(boardname, "ideas") || !str_cmp(boardname, "announce"))
    {
      if (!is_contained("all", to))
        sprintf(to_list, "all %s", to);
      else
        sprintf(to_list, "%s", to);
    }
    else if (!str_cmp(boardname, "bugs"))
    {
      if (!is_contained("imm", to))
        sprintf(to_list, "imm %s", to);
      else
        sprintf(to_list, "%s", to);
    }
    else if (!str_cmp(boardname, "personal"))
    {
      for (i = 0; to[i] != '\0'; i++)
        to[i] = LOWER(to[i]);

      sprintf(to_list, "%s", to);

      if (is_full_name("all", to_list))
      {
        send_to_webclient(wc, "You cannot send a personal note to 'all'.", FALSE);
        close_account(acc);
        return;
      }
    }
    else if (!str_cmp(boardname, "immortal") || !str_cmp(boardname, "builder"))
    {
      sprintf(to_list, "imm");
    }
    else
    {
      sprintf(to_list, "%s", to);
    }

    make_note(boardname, playername, to_list, subject, 14, contents, flags);

    if (!str_cmp(boardname, "general") || !str_cmp(boardname, "ideas") || !str_cmp(boardname, "announce"))
    {
      if (!IS_SET(flags, NOTE_FLAG_SILENCED))
      {
        sprintf(buf, "A new note has been posted by %s on the %s board (webnote)", playername, boardname);
        do_info(NULL, buf);
      }
      else
      {
        CHAR_DATA *gch;
        ITERATOR *pIter;
        DESCRIPTOR_DATA *d;

        pIter = AllocIterator(descriptor_list);
        while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
        {
          if (d->connected != CON_PLAYING) continue;

          if (d->account && !str_cmp(d->account->owner, acc->owner) && (gch = d->character) != NULL)
          {
            sprintf(buf, "#C<- #RInfo #C->#n A new note has been posted by %s on the %s board (webnote)\n\r", playername, boardname);
            send_to_char(buf, gch);
          }
        }
      }
    }

    send_to_webclient(wc, "Note Posted!", FALSE);
  }

  close_account(acc);
}

void handle_web_noteboard(WEBCLIENT *wc)
{
  ACCOUNT_DATA *account;
  char noteboard[MAX_STRING_LENGTH];
  ITERATOR *pIter;
  char *ptr, *accstr, *password;
  bool found = FALSE, latest = FALSE, clear = FALSE;
  int i = 0, iNote;

  streamBuffer(wc);

  if ((accstr = getEntry("account")) != NULL && (password = getEntry("password")) != NULL)
    account = load_web_account(accstr, password);
  else
    account = NULL;

  if ((ptr = getEntry("board")) == NULL)
    strcpy(noteboard, "general");
  else
    strcpy(noteboard, ptr);

  if ((ptr = getEntry("latest")) != NULL && !str_cmp(ptr, "1"))
    latest = TRUE;
  else
    latest = FALSE;

  if ((ptr = getEntry("clear")) != NULL && !str_cmp(ptr, "1"))
    clear = TRUE;
  else
    clear = FALSE;

  if ((ptr = getEntry("note")) == NULL)
  {
    send_to_webclient(wc, "That note does not exist!", FALSE);
    close_account(account);
    return;
  }
  iNote = atoi(ptr);

  for (i = 0; i < MAX_BOARD; i++)
  {
    if (!str_cmp(noteboard, boards[i].short_name))
    {
      found = TRUE;
      break;
    }
  }

  if (!found)
  {
    send_to_webclient(wc, "That board does not exist!", FALSE);
    close_account(account);
    return;
  }

  if (iNote != 0)
  {
    NOTE_DATA *note = NULL, *pnote;

    pIter = AllocIterator(boards[i].notes);
    while ((pnote = (NOTE_DATA *) NextInList(pIter)) != NULL)
    {
      note = pnote;
      if (--iNote <= 0)
        break;
    }

    if (note == NULL)
    {
      send_to_webclient(wc, "That note does not exist!", FALSE);
    }
    else
    {
      char buf1[1 * MAX_STRING_LENGTH];
      char buf2[1 * MAX_STRING_LENGTH];
      char buf3[4 * MAX_STRING_LENGTH];
      NOTE_DATA *scanner;
      bool before = TRUE;
      int prev = 0, next = 0, count = 0;

      /* security check */
      if (account == NULL)
      {
        if (!is_full_name("all", note->to_list))
        {
          send_to_webclient(wc, "You do not have access to that note.", FALSE);
          return;
        }
      }
      else if (!is_note_to(account, note))
      { 
        send_to_webclient(wc, "You do not have access to that note.", FALSE);
        close_account(account);
        return;
      }

      /* find previous and next note */
      pIter = AllocIterator(boards[i].notes);
      while ((scanner = (NOTE_DATA *) NextInList(pIter)) != NULL)
      {
        if (prev != 0 && next != 0)
          break;

        count++;

        if (scanner == note)
        {
          before = FALSE;
          continue;
        }

        if (!is_note_to(account, scanner)) continue;

        if (before)
          prev = count;
        else if (next == 0)
          next = count;
      }

      sprintf(buf1, "%d%c%d%c", prev, 27, next, 27);
      sprintf(buf2, "<B>%-12s</B> %s#n\n\r"
                    "<B>Date</B>         %s\n\r"
                    "<B>To</B>           %s\n\r"
                    "<B>Flags</B>        %s\n\r"
                    "#G===========================================================================#n\n\r",
          note->sender, note->subject, note->date, note->to_list,
          (IS_SET(note->flags, NOTE_FLAG_SILENCED)) ? "none" : flag_string(note_flags, note->flags));
      sprintf(buf3, "%s\n\r", note->text);

      send_to_webclient(wc, buf1, FALSE);
      send_to_webclient(wc, buf2, FALSE);
      send_to_webclient(wc, buf3, TRUE);
    }
  }
  else
  {
    char buf[MAX_STRING_LENGTH];
    NOTE_DATA *note = NULL, *pnote;
    int num = 1;

    /* if logged in, and asking to clear the board */
    if (clear && account)
    {
      pIter = AllocIterator(boards[i].notes);
      while ((pnote = (NOTE_DATA *) NextInList(pIter)) != NULL)
      {
        note = pnote;
      }

      if (note)
      {
        account->last_note[i] = note->date_stamp;
        save_account(account);
      }
    }

    found = FALSE;
    iNote = 1;

    send_to_webclient(wc, "<table class=\"notes\">\n", FALSE);
    send_to_webclient(wc, "<tr>\n<th width=\"15%\">Date</th>\n"
                          "<th width=\"20%\">Author</th>\n"
                          "<th width=\"65%\">Subject</th>\n</tr>\n", FALSE);

    pIter = AllocIterator(boards[i].notes);
    while ((note = (NOTE_DATA *) NextInList(pIter)) != NULL)
    {
      if (account == NULL)
      {
        if (!is_full_name("all", note->to_list))
        {
          num++;
          continue;
        }
      }
      else if (account && latest && account->last_note[i] >= note->date_stamp)
      {
        num++;
        continue;
      }
      else if (!is_note_to(account, note))
      {
        num++;
        continue;
      }

      found = TRUE;
      sprintf(buf, "<tr class=\"%s\" "
                   "onmousedown=\"update('%d');\" "
                   "onmouseover=\"this.className='rsel';\" "
                   "onmouseout=\"this.className='%s';\">\n"
                   "<td>%s</td>\n"
                   "<td>%s</td>\n<td>",
        (iNote %2) ? "odd" : "even", num, (iNote %2) ? "odd" : "even", get_date_string(note->date_stamp), note->sender);
      send_to_webclient(wc, buf, FALSE);
      send_to_webclient(wc, note->subject, TRUE);
      send_to_webclient(wc, "</td>\n</tr>\n", FALSE);

      iNote++;
      num++;
    }
    send_to_webclient(wc, "</table>\n", FALSE);

    if (!found)
      send_to_webclient(wc, "No new notes on this board.", TRUE);

    close_account(account);
  }
}

void handle_web_helpfile(WEBCLIENT *wc)
{
  char buf[MAX_STRING_LENGTH];
  HELP_DATA *pHelp;
  ITERATOR *pIter;
  char *ptr;
  int col = 0;

  streamBuffer(wc);

  if ((ptr = getEntry("help")) != NULL)
  {
    if ((pHelp = get_help(NULL, ptr)) == NULL)
    {
      sprintf(buf, "Sorry, could not find the helpfile '%s'\n", ptr);
    }
    else
    {
      sprintf(buf, "<b>%s</b>\n%s\n", pHelp->name, get_web_help(pHelp->text));
    }

    /* we do not preparse helpfiles, because they contain links */
    send_to_webclient(wc, buf, FALSE);
  }
  else if ((ptr = getEntry("range")) != NULL)
  {
    if (strlen(ptr) < 3)
    {
      send_to_webclient(wc, "That is not a valid range of helpfiles.", FALSE);
    }
    else
    {
      if (ptr[0] < 'a' || ptr[0] > 'z' || ptr[2] < 'a' || ptr[2] > 'z' || ptr[2] < ptr[0])
      {
        send_to_webclient(wc, "That is not a valid range.", FALSE);
      }
      else
      {
        BUFFER *buf2 = buffer_new(MAX_STRING_LENGTH);

        pIter = AllocIterator(help_list);
        while ((pHelp = (HELP_DATA *) NextInList(pIter)) != NULL)
        {
          int len;

          /* we do not show immortal help files */  
          if (pHelp->level > LEVEL_AVATAR) continue;

          if ((tolower(pHelp->keyword[0]) >= ptr[0] && tolower(pHelp->keyword[0]) <= ptr[2]) ||
              (pHelp->keyword[0] == '\'' && tolower(pHelp->keyword[1]) >= ptr[0] && tolower(pHelp->keyword[1]) <= ptr[2]))
          {
            bprintf(buf2, "<a href=\"?help=%s\">%.16s</a>", pHelp->name, pHelp->name);

            for (len = UMIN(16, strlen(pHelp->name)); len < 17; len++)
              bprintf(buf2, " ");

            if (++col % 5 == 0) bprintf(buf2, "\n");
          }
        }

        send_to_webclient(wc, buf2->data, FALSE);
        buffer_free(buf2);
      }
    }
  }
  else
  {
    send_to_webclient(wc, "There is no helpfile by that name.", FALSE);
  }
}

char *get_web_help(char *txt)
{
  static char buf[MAX_STRING_LENGTH];
  int i = 0;

  while (*txt != '\0')
  {
    switch(*txt)
    {
      default:
        buf[i++] = *txt++;
        break;
      case '@':
        if (*(++txt) == '@')
        {
          char helpentry[MAX_STRING_LENGTH];
          int j = 0;

          /* copy the help name to temporary buffer */
          while ((helpentry[j] = *(++txt)) != '@' && helpentry[j] != '\0')
            j++;
          helpentry[j] = '\0';
          txt++;

          buf[i] = '\0';
          strcat(buf, "<a href=\"?help=");
          strcat(buf, helpentry);
          strcat(buf, "\">");
          strcat(buf, helpentry);
          strcat(buf, "</a>");
          i += (strlen("<a href=\"?help=\">") + 2 * j + strlen("</a>"));
        }
        else
        {
          buf[i++] = '@';
        }
        break;
    }
  }
  buf[i] = '\0';

  return buf;
}
