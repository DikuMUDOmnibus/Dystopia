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
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 */

#define _XOPEN_SOURCE
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
#include <sys/ioctl.h>

#include "dystopia.h"
#include "olc.h"

const unsigned char echo_off_str   [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const unsigned char echo_on_str    [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const unsigned char go_ahead_str   [] = { IAC, GA, '\0' };
const unsigned char compress_will  [] = { IAC, WILL, TELOPT_COMPRESS, '\0' };
const unsigned char compress_do    [] = { IAC, DO, TELOPT_COMPRESS, '\0' };
const unsigned char compress_dont  [] = { IAC, DONT, TELOPT_COMPRESS, '\0' };

/*
 * Global variables.
 */
STACK           *   descriptor_free;	/* Free list for descriptors	*/
LIST            *   descriptor_list;	/* All open descriptors		*/
bool		    merc_down = FALSE;	/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
char		    str_boot_time[MAX_INPUT_LENGTH];
time_t		    current_time;	/* Time of this pulse		*/


void	game_loop_unix		( int control, int webport );
int	init_socket		( int port );
void	new_descriptor		( int control );
bool	read_from_descriptor	( DESCRIPTOR_DATA *d );
void    create_datastructures   ( void );
int	main			( int argc, char **argv );
void	nanny			( DESCRIPTOR_DATA *d, char *argument );
bool	process_output		( DESCRIPTOR_DATA *d, bool fPrompt );
void	read_from_buffer	( DESCRIPTOR_DATA *d );
void	stop_idling		( CHAR_DATA *ch );
void	bust_a_prompt		( DESCRIPTOR_DATA *d );
void  * lookup_address          ( void *arg )  __attribute__((noreturn));
void    panic_shutdown          ( int iSignal )  __attribute__((noreturn));
bool    check_banned            ( DESCRIPTOR_DATA *dnew );
bool    check_newbiebanned      ( DESCRIPTOR_DATA *dnew );


/* need to define this here, to make sure that the
 * compiler will allow us to use this on all platforms
 * where it exists... if you try to compile on a platform
 * that does not support gethostbyaddr_r, then look for
 * the __CYGWIN__ check below, and add whatever check
 * is needed on your platform to skip past that bit of
 * code.
 */
struct hostent *gethostbyaddr_r(const void      * addr,       \
                                socklen_t         len,        \
                                int               type,       \
                                struct hostent  * result_buf, \
                                char            * buf,        \
                                size_t            buflen,     \
                                struct hostent ** result,     \
                                int             * h_errnop    );

int proc_pid;
int port, control;

int main( int argc, char **argv )
{
  struct timeval now_time;
  bool fCopyOver = FALSE;

#ifdef RLIMIT_NOFILE
{ 
  struct  rlimit rlp;

  (void) getrlimit(RLIMIT_NOFILE, &rlp);
  rlp.rlim_cur = UMIN(rlp.rlim_max,FD_SETSIZE);
  (void) setrlimit(RLIMIT_NOFILE, &rlp);
}
#endif

  /*
   * Crash recovery by Mandrax
   */
  signal(SIGSEGV, crashrecov);
  signal(SIGTERM, panic_shutdown);
  proc_pid = getpid();

  create_datastructures();

  /*
   * Init time
   */
  gettimeofday(&now_time, NULL);
  current_time = (time_t) now_time.tv_sec;
  strcpy(str_boot_time, ctime(&current_time));

  /*
   * Get the port number.
   */
  port = 9009; /* defaults to port 9009 */
  if ( argc > 1 )
  {
    if ( !is_number( argv[1] ) )
    {
      fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
      exit( 1 );
    }
    else if ( ( port = atoi( argv[1] ) ) <= 1024 )
    {
      fprintf( stderr, "Port number must be above 1024.\n" );
      exit( 1 );
    }
  }
  if (argv[2] && argv[2][0])
  {
    fCopyOver = TRUE;
    control = atoi(argv[3]);
  }
  else
    fCopyOver = FALSE;

  if (!fCopyOver) /* We have already the port if copyover'ed */
    control = init_socket (port);
  boot_db (fCopyOver);

  log_string("Dystopia is ready to rock on port %d.", port);
  game_loop_unix( control, port-1 );
  close( control );


  /*
   * That's all, folks.
   */
  log_string( "Normal termination of game." );
  return 1;
}

void create_datastructures()
{
  int i;

  /* allocate data lists */
  char_list            =  AllocList();
  area_list            =  AllocList();
  descriptor_list      =  AllocList();
  object_list          =  AllocList();
  help_list            =  AllocList();
  dummy_list           =  AllocList();
  auction_list         =  AllocList();
  kingdom_list         =  AllocList();
  artifact_table       =  AllocList();
  change_list          =  AllocList();
  poll_list            =  AllocList();

  /* allocate index hashes */
  for (i = 0; i < MAX_KEY_HASH; i++)
  {
    mob_index_hash[i]  =  AllocList();
    obj_index_hash[i]  =  AllocList();
    room_index_hash[i] =  AllocList();
  }

  /* allocate free "lists" */
  note_free            =  AllocStack();
  alias_free           =  AllocStack();
  spell_free           =  AllocStack();
  snoop_free           =  AllocStack();
  area_free            =  AllocStack();
  extra_descr_free     =  AllocStack();
  mob_index_free       =  AllocStack();
  obj_index_free       =  AllocStack();
  history_free         =  AllocStack();
  ignore_free          =  AllocStack();
  change_free          =  AllocStack();
  quest_free           =  AllocStack();
  affect_free          =  AllocStack();
  account_free         =  AllocStack();
  session_free         =  AllocStack();
  feed_free            =  AllocStack();
  member_free          =  AllocStack();
  area_affect_free     =  AllocStack();
  dummy_free           =  AllocStack();
  auction_free         =  AllocStack();
  scell_free           =  AllocStack();
  pcdata_free          =  AllocStack();
  char_free            =  AllocStack();
  descriptor_free      =  AllocStack();
  obj_free             =  AllocStack();
  reset_free           =  AllocStack();
  exit_free            =  AllocStack();

  /* allocate the quick iterator */
  QuickIter            =  AllocQuickIterator();
  QuickIter2           =  AllocQuickIterator();
}

int init_socket(int iport)
{
  static struct sockaddr_in sa_zero;
  struct sockaddr_in sa;
  int x = 1;
  int fd;

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Init_socket: socket");
    exit( 1 );
  }

  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &x, sizeof(int)) < 0)
  {
    perror("Init_socket: SO_REUSEADDR");
    close(fd);
    exit(1);
  }

  sa		    = sa_zero;
  sa.sin_family     = AF_INET;
  sa.sin_port	    = htons(iport);

  if (bind(fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0)
  {
    perror("Init_socket: bind");
    close(fd);
    exit(1);
  }

  if (listen( fd, 3 ) < 0)
  {
    perror("Init_socket: listen");
    close(fd);
    exit(1);
  }

  return fd;
}

void game_loop_unix( int icontrol, int webport )
{
  ITERATOR *pIter;
  static struct timeval null_time;
  struct timeval last_time;

  signal( SIGPIPE, SIG_IGN );
  gettimeofday( &last_time, NULL );
  current_time = (time_t) last_time.tv_sec;

  /* start running the webserver */
  init_webserver(webport);

  /* Main loop */
  while (!merc_down)
  {
    fd_set in_set;
    fd_set out_set;
    fd_set exc_set;
    DESCRIPTOR_DATA *d;
    int maxdesc;

    /*
     * Poll all active descriptors.
     */
    FD_ZERO( &in_set  );
    FD_ZERO( &out_set );
    FD_ZERO( &exc_set );
    FD_SET( icontrol, &in_set );
    maxdesc	= icontrol;

    pIter = AllocIterator(descriptor_list);
    while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
    {
      maxdesc = UMAX( maxdesc, d->descriptor );
      FD_SET( d->descriptor, &in_set  );
      FD_SET( d->descriptor, &out_set );
      FD_SET( d->descriptor, &exc_set );
    }

    if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
    {
      perror( "Game_loop: select: poll" );
      exit( 1 );
    }

    /*
     * New connection?
     */
    if ( FD_ISSET( icontrol, &in_set ) )
      new_descriptor( icontrol );

    /*
     * Kick out the freaky folks.
     */
    pIter = AllocIterator(descriptor_list);
    while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
    {
      if ( FD_ISSET( d->descriptor, &exc_set ) )
      {
	FD_CLR( d->descriptor, &in_set  );
	FD_CLR( d->descriptor, &out_set );
	if ( d->character )
	  save_char_obj( d->character );
       	d->outtop	= 0;
       	close_socket( d );
      }
    }

    /*
     * Process input.
     */
    pIter = AllocIterator(descriptor_list);
    while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
    {
      d->fcommand = FALSE;

      if (!d->bBanned && check_banned(d))
      {
        write_to_buffer(d, "Your site has been banned from this mud.\n\r", 0);
        close_socket(d);
        continue;
      }

      if ( FD_ISSET( d->descriptor, &in_set ) )
      {
        if ( d->character != NULL )
          d->character->timer = 0;
       	if ( !read_from_descriptor( d ) )
       	{
          FD_CLR( d->descriptor, &out_set );
          if ( d->character != NULL )
	    save_char_obj( d->character );
          d->outtop	= 0;
	  close_socket( d );
	  continue;
	}
      }
      if ( d->character != NULL && d->character->wait > 0 )
      {
        --d->character->wait;
	continue;
      }
      read_from_buffer( d );
      if ( d->incomm[0] != '\0' )
      {
        d->fcommand	= TRUE;
	stop_idling( d->character );

        if ( d->pString )
          string_add( d->character, d->incomm );
        else 
	  switch( d->connected )
	  {
  	    default:
 	      nanny( d, d->incomm );
	      break;
	    case CON_PLAYING:
              if ( !run_olc_editor( d ) )
		interpret( d->character, d->incomm );
	      break;
	  }

   	d->incomm[0]	= '\0';
      }
    }

    /*
     * Event queue heartbeat
     */
    heartbeat();

    /* To make sure that all lists are cleaned of invalid
     * cells, we release ALL iterators in one go.
     */
    ReleaseAllIterators();

    /*
     * Poll for new web-requests
     */
    poll_web_requests();

    /*
     * Output.
     */
    pIter = AllocIterator(descriptor_list);
    while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
    {
      if ((d->fcommand || d->outtop > 0) && FD_ISSET(d->descriptor, &out_set))
      {
	if ( !process_output( d, TRUE ) )
	{
	  if ( d->character != NULL )
	    save_char_obj( d->character );
	  d->outtop	= 0;
	  close_socket( d );
        }
      }
    }

    /* recycle all closed descriptors */
    recycle_descriptors();

    /* 
     * Synchronize to a clock.
     * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
     * Careful here of signed versus unsigned arithmetic.
     */
{
  struct timeval now_time;
  long secDelta;
  long usecDelta;

  gettimeofday( &now_time, NULL );
  usecDelta = ((int) last_time.tv_usec) - ((int) now_time.tv_usec) + 1000000 / PULSE_PER_SECOND;
  secDelta  = ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );

  while ( usecDelta < 0 )
  {
    usecDelta += 1000000;
    secDelta  -= 1;
  }
  while ( usecDelta >= 1000000 )
  {
    usecDelta -= 1000000;
    secDelta  += 1;
  }
  if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
  {
    struct timeval stall_time;

    stall_time.tv_usec = usecDelta;
    stall_time.tv_sec  = secDelta;

    if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
    {
      perror( "Game_loop: select: stall" );
      exit( 1 );
    }
  }
}

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;
  }

  /* stop running the webserver */
  close_webserver();
}

void init_descriptor(DESCRIPTOR_DATA *dnew, int desc)
{
  static DESCRIPTOR_DATA d_zero;

  *dnew = d_zero;
  dnew->descriptor     =  desc;
  dnew->hostname       =  str_dup("");
  dnew->ip_address     =  str_dup("");
  dnew->character      =  NULL;
  dnew->account        =  NULL;
  dnew->events         =  AllocList();
  dnew->snoops         =  AllocList();
  dnew->connected      =  CON_ACCOUNT_NAME;
  dnew->bClosed        =  FALSE;
  dnew->bResolved      =  FALSE;
  dnew->bBanned        =  FALSE;
  dnew->pEdit          =  NULL;		/* OLC */
  dnew->pString        =  NULL;		/* OLC */
  dnew->editor         =  0;		/* OLC */
  dnew->outsize        =  2000;
  dnew->alias_block    =  0;
  dnew->mxp            =  FALSE;
  dnew->msp            =  FALSE;
  dnew->outbuf         =  calloc(1, sizeof(char *) * dnew->outsize);
}

void new_descriptor(int icontrol)
{
  EVENT_DATA *event;
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *dnew;
  struct sockaddr_in sock;
  int desc, argp = 1;
  socklen_t size;
  bool DOS_ATTACK = FALSE;

  size = sizeof(sock);
  getsockname(icontrol, (struct sockaddr *) &sock, &size);

  if ((desc = accept(icontrol, (struct sockaddr *) &sock, &size)) < 0)
  {
    perror("New_descriptor: accept");
    return;
  }

  /* allocate a new descriptor */
  if ((dnew = (DESCRIPTOR_DATA *) PopStack(descriptor_free)) == NULL)
    dnew = calloc(1, sizeof(*dnew));
  init_descriptor(dnew, desc);

  /* set the socket as non-blocking */
  ioctl(dnew->descriptor, FIONBIO, &argp);

  if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0)
  { 
    perror("New_descriptor: getpeername");
    dnew->hostname = str_dup("(unknown hostname)");
    dnew->ip_address = str_dup("(unknown ip address)");
    dnew->bResolved = TRUE;
  }
  else
  {
    int addr = ntohl(sock.sin_addr.s_addr);

    sprintf(buf, "%d.%d.%d.%d",
       ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
       ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF);

    dnew->ip_address = str_dup(buf);

    /* we do a hostlookup on everyone but localhost */
    if (str_cmp(dnew->ip_address, "127.0.0.1"))
    {
#ifndef __CYGWIN__
      if (thread_count < 50)
      {
        pthread_attr_t attr;
        pthread_t thread_lookup;
        DUMMY_ARG *dummyarg; 

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  
        if ((dummyarg = (DUMMY_ARG *) PopStack(dummy_free)) == NULL)
          dummyarg = calloc(1, sizeof(*dummyarg));

        dummyarg->status = 1;
        dummyarg->buf = str_dup((char *) &sock.sin_addr);
        dummyarg->d = dnew;

        AttachToList(dummyarg, dummy_list);
        pthread_create(&thread_lookup, &attr, &lookup_address, (void *) dummyarg);
      }
      else
      {
        DOS_ATTACK = TRUE;
      }
#else
      dnew->bResolved = TRUE;
      dnew->hostname = str_dup("Hostname not resolved");
#endif
    }
    else
    {
      dnew->bResolved = TRUE;
      dnew->hostname = str_dup("localhost");
    }
  }

  /* init descriptor data */
  AttachToList(dnew, descriptor_list);

  if (DOS_ATTACK)
  {
    write_to_buffer(dnew, "Sorry, currently under attack, try again later.\n\r", 0 );
    close_socket(dnew);
    return;
  }

  /* tell tehe client what we support */
  negotiate(dnew);

  /* send greeting */
  {
    extern char *help_greeting;

    if (help_greeting[0] == '.')
      write_to_buffer(dnew, help_greeting+1, 0);
    else
      write_to_buffer(dnew, help_greeting  , 0);
  }

  /* new sockets should idle out after 10 minutes */
  event = alloc_event();
  event->fun = &event_socket_idle;
  event->type = EVENT_SOCKET_IDLE;
  add_event_desc(event, dnew, 10 * 60 * PULSE_PER_SECOND);
}

#ifndef __CYGWIN__
void *lookup_address(void *arg)   
{
  DUMMY_ARG *darg = (DUMMY_ARG *) arg;
  struct hostent *from = 0;
  struct hostent ent;
  char buf[16384];
  int err;

  thread_count++;
  gethostbyaddr_r(darg->buf, sizeof(darg->buf), AF_INET, &ent, buf, 16384, &from, &err);

  if (from && from->h_name)
    darg->d->hostname = str_dup(from->h_name);
  else
    darg->d->hostname = str_dup(darg->d->ip_address);

  free_string(darg->buf);
  darg->status = 0;
  darg->d->bResolved = TRUE;
  thread_count--;

  pthread_exit(0);
}
#endif

bool check_banned(DESCRIPTOR_DATA *dnew)
{
  BAN_DATA *pban;
  ITERATOR *pIter;

  /* do not check until we are done resolving */
  if (!dnew->bResolved)
    return FALSE;

  dnew->bBanned = TRUE;

  pIter = AllocIterator(ban_list);
  while ((pban = (BAN_DATA *) NextInList(pIter)) != NULL)
    if (!str_suffix(pban->name, HOSTNAME(dnew))) return TRUE;

  return FALSE;
}

bool check_newbiebanned( DESCRIPTOR_DATA *dnew )
{
  BAN_DATA *pban;
  ITERATOR *pIter;

  pIter = AllocIterator(newbieban_list);
  while ((pban = (BAN_DATA *) NextInList(pIter)) != NULL)
    if (!str_suffix(pban->name, HOSTNAME(dnew))) return TRUE;

  return FALSE;
}

void close_socket( DESCRIPTOR_DATA *dclose )
{
  EVENT_DATA *event;
  ITERATOR *pIter, *pIter2;
  CHAR_DATA *ch;
   
  if (dclose->bClosed)
    return;

  dclose->bClosed = TRUE;
   
  if (dclose->outtop > 0)
    process_output(dclose, FALSE);

  if (SizeOfList(dclose->snoops) > 0)
  {
    SNOOP_DATA *snoop;

    pIter = AllocIterator(dclose->snoops);
    while ((snoop = (SNOOP_DATA *) NextInList(pIter)) != NULL)
    {
      write_to_buffer(snoop->snooper, "Your victim has left the game.\n\r", 0);
      free_snoop(dclose, snoop);
    }
  }
     
  /*
   * Clear snoops
   */
  {
    DESCRIPTOR_DATA *d;

    pIter = AllocIterator(descriptor_list);
    while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
    {
      SNOOP_DATA *snoop;

      pIter2 = AllocIterator(d->snoops);
      while ((snoop = (SNOOP_DATA *) NextInList(pIter2)) != NULL)
      {
        if (snoop->snooper == dclose)
          free_snoop(d, snoop);
      }
    }
  }

  /* 
   * Loose link or free char
   */
  if ((ch = dclose->character) != NULL)
  {
    log_string("Closing link to %s.", ch->name );
    /* If ch is writing note or playing, just lose link otherwise clear char */
    if (dclose->connected == CON_PLAYING ||
       (dclose->connected >= CON_NOTE_TO && dclose->connected <= CON_NOTE_FINISH))
    {
      REMOVE_BIT(ch->extra,EXTRA_AFK);
      call_all(ch);
      act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
      ch->desc = NULL;
    }
    else
    {
      free_char( dclose->character );
    }
  } 

  if (dclose->account)
  {
    if (dclose->connected != CON_CONFIRM_ACCOUNT &&
        dclose->connected != CON_NEW_PASSWORD &&
        dclose->connected != CON_CONFIRM_PASSWORD)
    {
      save_account(dclose->account);
    }
    close_account(dclose->account);
  }

  /* remove any pending events */
  pIter = AllocIterator(dclose->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
    dequeue_event(event, TRUE);
  FreeList(dclose->events);

  dclose->connected = CON_NOT_PLAYING;
}
     
/* For a better kickoff message :) KaVir */
void close_socket2( DESCRIPTOR_DATA *dclose, bool kickoff )
{
  EVENT_DATA *event;
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter, *pIter2;
  CHAR_DATA *ch;
     
  /* only allow close_socket to be called once */
  if (dclose->bClosed)
    return;

  /* flag descriptor as closed */
  dclose->bClosed = TRUE;
   
  /* process any residue output */
  if (dclose->outtop > 0)
    process_output( dclose, FALSE );

  /* tell any snoopers that the player has gone bye bye */
  if (SizeOfList(dclose->snoops) > 0)
  { 
    SNOOP_DATA *snoop;

    pIter = AllocIterator(dclose->snoops);
    while ((snoop = (SNOOP_DATA *) NextInList(pIter)) != NULL)
    {
      write_to_buffer(snoop->snooper, "Your victim has left the game.\n\r", 0);
      free_snoop(dclose, snoop);
    }
  } 

 /*
  * Clear snoops
  */
  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    SNOOP_DATA *snoop;

    pIter2 = AllocIterator(d->snoops);
    while ((snoop = (SNOOP_DATA *) NextInList(pIter2)) != NULL)
    {
      if (snoop->snooper == dclose)
        free_snoop(d, snoop);
    }
  }

  /* check for reconnect */
  if ((ch = dclose->character) != NULL)
  {
    if (dclose->connected == CON_PLAYING ||
       (dclose->connected >= CON_NOTE_TO && dclose->connected <= CON_NOTE_FINISH)) 
    {
      if (kickoff)
        act( "$n doubles over in agony and $s eyes roll up into $s head.", ch, NULL, NULL, TO_ROOM);
      save_char_obj( ch );
      ch->desc = NULL;
    }
    else
    {
      free_char( dclose->character );
    }
  }

  if (dclose->account)
  {
    save_account(dclose->account);
    close_account(dclose->account);
  }

  /* remove any pending events */
  pIter = AllocIterator(dclose->events);
  while ((event = (EVENT_DATA *) NextInList(pIter)) != NULL)
    dequeue_event(event, TRUE);
  FreeList(dclose->events);

  dclose->connected = CON_NOT_PLAYING;
}

bool read_from_descriptor(DESCRIPTOR_DATA *d)
{
  int iStart;

  /* Hold horses if pending command already. */
  if (d->incomm[0] != '\0')
    return TRUE;

  /* one dirty patch to avoid spams of EOF's */
  if (d->connected == CON_NOT_PLAYING)
    return TRUE;

  /* Check for overflow. */
  if ((iStart = strlen(d->inbuf)) >= (int) sizeof(d->inbuf) - 10)
  {
    log_string("%s input overflow!", HOSTNAME(d));
    write_to_descriptor(d, "\n\r*** PUT A LID ON IT!!! ***\n\r", 0);
    return FALSE;
  }

  /* Snarf input. */
  for ( ; ; )
  {
    int nRead;

    nRead = read(d->descriptor, d->inbuf + iStart, sizeof(d->inbuf) - 10 - iStart);

    if (nRead > 0)
    {
      iStart += nRead;

      if (d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r')
        break;
    }
    else if (nRead == 0)
    {
      log_string("EOF encountered on read.");
      return FALSE;
    }
    else if (errno == EWOULDBLOCK)
      break;
    else
    {
      perror("Read_from_descriptor");
      return FALSE;
    }
  }

  d->inbuf[iStart] = '\0';
  return TRUE;
}

/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
  const unsigned char msp_do   [] = { IAC, DO,   TELOPT_MSP, '\0' };
  const unsigned char msp_dont [] = { IAC, DONT, TELOPT_MSP, '\0' };
  char buf[MAX_STRING_LENGTH];
  int i, j, k, old_repeat = 0;
  bool new_cmd = FALSE;

  /*
   * Hold horses if pending command already.
   */
  if (d->incomm[0] != '\0')
    return;

  /*
   * Look for at least one new line.
   */
  for (i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
  {
    if (d->inbuf[i] == '\0')
      return;
  }

  /*
   * Canonical input processing.
   */
  for (i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
  {
    if (k >= MAX_INPUT_LENGTH - 2)
    {
      write_to_descriptor( d, "Line too long.\n\r", 0 );

      /* skip the rest of the line */
      for (; d->inbuf[i] != '\0'; i++)
      {
	if (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
	  break;
      }
      d->inbuf[i]   = '\n';
      d->inbuf[i+1] = '\0';
      break;
    }
    if (d->inbuf[i] == '\b' && k > 0)
    {
      --k;
    }
    else if (d->inbuf[i] == (signed char) IAC)
    {
      if (!memcmp(&d->inbuf[i], compress_do, strlen((char *) compress_do)))
      {
        i += strlen((char *) compress_do) - 1;
        compressStart(d);
      }
      else if (!memcmp(&d->inbuf[i], compress_dont, strlen((char *) compress_dont)))
      {
        i += strlen((char *) compress_dont) - 1;
        compressEnd(d);
      }
      else if (!memcmp(&d->inbuf[i], msp_do, strlen((char *) msp_do)))
      {
        i += strlen((char *) msp_do) - 1;
        d->msp = TRUE;
      }
      else if (!memcmp(&d->inbuf[i], msp_dont, strlen((char *) msp_dont)))
      {
        i += strlen((char *) msp_dont) - 1;
        d->msp = FALSE;
      }
      else if (!memcmp(&d->inbuf[i], mxp_do, strlen((char *) mxp_do)))
      {
        i += strlen((char *) mxp_do) - 1;
        init_mxp(d);
      }
      else if (!memcmp(&d->inbuf[i], mxp_dont, strlen((char *) mxp_dont)))
      {
        i += strlen((char *) mxp_dont) - 1;
        shutdown_mxp(d);
      }
    }
    else if (isascii(d->inbuf[i]) && isprint(d->inbuf[i]))
    {
      buf[k++] = d->inbuf[i];
    }
  }

  /*
   * Finish off the line.
   */
  if (k == 0)
    buf[k++] = ' ';
  buf[k] = '\0';

  /*
   * Deal with bozos with #repeat 1000 ...
   */
  if (k > 1 || buf[0] == '!')
  {
    if (buf[0] != '!' && strcmp(buf, d->inlast))
    {
      old_repeat = d->repeat;
      d->repeat = 0;
    }
    else
    {
      if (++d->repeat >= 40)
      {
        log_string("%s input overflow!", HOSTNAME(d));
	write_to_descriptor( d, "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
	sprintf(buf, "quit");
      }
    }
  }

  /*
   * Do '!' substitution.
   */
  if (buf[0] == '!')
    strcpy(buf, d->inlast);
  else
    new_cmd = TRUE;

  /*
   * Shift the input buffer.
   */
  if (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
  {
    if ((d->inbuf[++i] == '\n' || d->inbuf[i] == '\r') &&
        (d->inbuf[i - 1] != d->inbuf[i]))
    i++;
  }
  for (j = 0; (d->inbuf[j] = d->inbuf[i+j]) != '\0'; j++)
    ;

  /* do that alias thingy... */
  if (d->alias_block == 0 && check_alias(buf, d))
    d->repeat = old_repeat;
  else
    strcpy(d->incomm, buf);

  if (new_cmd)
    strcpy(d->inlast, d->incomm);

  /* count the blocker one down if needed */
  if (d->alias_block > 0)
    d->alias_block--;
}

void panic_shutdown(int iSignal)
{
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;
  CHAR_DATA *gch;
  char buf[200];

  sprintf(buf, "\n\rServer reboot, saving and doing a panic shutdown.");

  /* We really don't want to lose this info */
  save_muddata();

  /* save all connected players */
  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
  {
    if ((gch = d->character) == NULL) continue;
    call_all(gch);
    save_char_obj(gch);
    write_to_descriptor_2(d->descriptor, buf, 0);
  }

  /* create a bug message so it's easy to know what happened */
  bug("Server Rebooted, handling it safely.", 0);

  /* and we die */
  exit(0);
}

/*
 * Crash recovery system written by Mandrax, based on copyover
 * Includes call to signal() in main.
 * Mostly copied from copyover.
 */
void crashrecov(int iSignal)
{
  DESCRIPTOR_DATA *d;
  ITERATOR *pIter;
  char buf[200], buf2[100];
  int pid, iFork;
  FILE *fReturn, *fCrash;

  /* dump debug message */
  dump_last_command();

  /* an attempt to avoid spam crashes */
  if ((fReturn = fopen(CRASH_TEMP_FILE, "r")) != NULL)
  {
    signal(SIGSEGV, SIG_IGN);
    raise(SIGSEGV);
    return;
  }

  fCrash = fopen(CRASH_TEMP_FILE, "w");
  fprintf(fCrash, "0");
  fclose(fCrash);

  /* This will cause a core dump, even though the signal was handled */
  iFork = fork();
  wait(NULL);
  if((pid = getpid()) != proc_pid)
  {
    signal(SIGSEGV, SIG_IGN);
    raise(SIGSEGV);
    return;
  }

  if (pre_reboot_actions(TRUE) == FALSE)
    return;

  /* save the world - let's hope it doesn't loop crash :) */
  do_asave(NULL, "changed");

  sprintf (buf, "\n\r <*>       Calim's Cradle has crashed        <*>\n\n\r"
                    " <*>   Attempting to restore last savefile   <*>\n\r");

  pIter = AllocIterator(descriptor_list);
  while ((d = (DESCRIPTOR_DATA *) NextInList(pIter)) != NULL)
    write_to_descriptor_2(d->descriptor, buf, 0);

  /* exec - descriptors are inherited */
  sprintf(buf, "%d", port);
  sprintf(buf2, "%d", control);

  execl(EXE_FILE, "dystopia", buf, "crashrecov", buf2, (char *) NULL);

  /* Failed - sucessful exec will not return */
  perror("crashrecov: execl");
  log_string("Crash recovery FAILED!\n\r");

  /* damn! */
  exit(1);
}

bool event_socket_negotiate(EVENT_DATA *event)
{
  const unsigned char msp_will[] = { IAC, WILL, TELOPT_MSP, '\0' };
  DESCRIPTOR_DATA *d;

  if ((d = event->owner.desc) == NULL)
  {
    bug("event_socket_negotiate: no owner.", 0);
    return FALSE;
  }
 
  write_to_buffer(d, (char *) msp_will, 0);
  write_to_buffer(d, (char *) compress_will, 0);
  write_to_buffer(d, (char *) mxp_will, 0);

  return FALSE;
}

void negotiate(DESCRIPTOR_DATA *d)
{
  EVENT_DATA *event;

  event = alloc_event();
  event->fun = &event_socket_negotiate;
  event->type = EVENT_SOCKET_NEGOTIATE;
  add_event_desc(event, d, 5 * PULSE_PER_SECOND);
}

bool process_output(DESCRIPTOR_DATA *d, bool fPrompt)
{
  SNOOP_DATA *snoop;
  ITERATOR *pIter;

  /* dirty hack to get rid of prompt when editing descs */
  if (d->pString != NULL)
    fPrompt = FALSE;

  /*
   * Bust a prompt.
   */
  if (fPrompt && !merc_down && d->connected == CON_PLAYING)
  {
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *ch = d->character;
    CHAR_DATA *victim = ch->fighting;

    if (ch->pcdata->brief[BRIEF_5])
    {
      if (ch->pcdata->brief5data[BRIEF5_NUM_DEALT] > 0)
      {
        sprintf(buf, "#GYou've dealt #C%d #Gattacks causing #C%d #Gdamage. #0(#y%d #Gaverage#0)#n\n\r",
          ch->pcdata->brief5data[BRIEF5_NUM_DEALT], ch->pcdata->brief5data[BRIEF5_AMOUNT_DEALT],
          ch->pcdata->brief5data[BRIEF5_AMOUNT_DEALT] / ch->pcdata->brief5data[BRIEF5_NUM_DEALT]);
        send_to_char(buf, ch);
      }
      if (ch->pcdata->brief5data[BRIEF5_NUM_RECEIVED] > 0)
      {
        sprintf(buf, "#GYou've been struck by #C%d #Gattacks, causing #C%d #Gdamage. #0(#y%d #Gaverage#0)#n\n\r",
          ch->pcdata->brief5data[BRIEF5_NUM_RECEIVED], ch->pcdata->brief5data[BRIEF5_AMOUNT_RECEIVED],
          ch->pcdata->brief5data[BRIEF5_AMOUNT_RECEIVED] / ch->pcdata->brief5data[BRIEF5_NUM_RECEIVED]);
        send_to_char(buf, ch);
      }
      ch->pcdata->brief5data[BRIEF5_NUM_DEALT] = 0;
      ch->pcdata->brief5data[BRIEF5_AMOUNT_DEALT] = 0;
      ch->pcdata->brief5data[BRIEF5_NUM_RECEIVED] = 0;
      ch->pcdata->brief5data[BRIEF5_AMOUNT_RECEIVED] = 0;
    }
    write_to_buffer( d, "\n\r", 2 );

    if (IS_SET(ch->act, PLR_PROMPT) && IS_EXTRA(ch, EXTRA_PROMPT))
      bust_a_prompt(d);
    else if (IS_SET(ch->act, PLR_PROMPT))
    {
      char cond[MAX_INPUT_LENGTH];
      char hit_str[MAX_INPUT_LENGTH];
      char mana_str[MAX_INPUT_LENGTH];
      char move_str[MAX_INPUT_LENGTH];
      char exp_str[MAX_INPUT_LENGTH];

      sprintf(hit_str, "%s", col_scale(ch->hit, ch->max_hit));
      sprintf(mana_str, "%s", col_scale(ch->mana, ch->max_mana));
      sprintf(move_str, "%s", col_scale(ch->move, ch->max_move));

      if (ch->position == POS_FIGHTING)
      {
        if (victim != NULL)
        {
          if ((victim->hit * 100 / UMAX(1, victim->max_hit)) < 25)
            sprintf(cond, "#R*");
          else if ((victim->hit * 100 / UMAX(1, victim->max_hit)) < 50)
            sprintf(cond, "#L*");
          else if ((victim->hit * 100 / UMAX(1, victim->max_hit)) < 75)
            sprintf(cond, "#G*");
          else if ((victim->hit * 100 / UMAX(1, victim->max_hit)) < 100)
            sprintf(cond, "#y*");
          else
            sprintf(cond, "#C*");

          if (victim->max_hit != 0)
          {
            int k;

            for (k = 1; k < 8; k++)
            {
              if (victim->hit > (victim->max_hit * k / 8))
                strcat(cond, "*");
              else strcat(cond, " ");
            }
            strcat(cond, "#n");
          }
          else strcat(cond, "       ");
        }
        else
        {
          sprintf(cond, "#CN/A#n");
        }

        sprintf(buf, "#0-=(%s#0)=- -=(%sH %sM %sV#0)=- -=(#C%d#n ft#0)=-#n ",
          cond, hit_str, mana_str, move_str, ch->fight_timer);
      }
      else
      {
        sprintf(exp_str, "#C%d#n", ch->exp);

        sprintf(buf, "#0-=(%s#0)=- -=(%sH %sM %sV#0)=- -=(#C%d#n ft#0)=-#n ",
          exp_str, hit_str, mana_str, move_str, ch->fight_timer);
      }
      write_to_buffer(d, buf, 0);
    }

    if (IS_SET(ch->act, PLR_TELNET_GA))
      write_to_buffer(d, (char *) go_ahead_str, 0);
  }

  /*
   * Short-circuit if nothing to write.
   */
  if (d->outtop == 0)
    return TRUE;

  /*
   * Snoop-o-rama.
   */
  pIter = AllocIterator(d->snoops);
  while ((snoop = (SNOOP_DATA *) NextInList(pIter)) != NULL)
  {
    bool closeSnoop = FALSE;

    if (!write_to_descriptor(snoop->snooper, "% ", 2))
      closeSnoop = TRUE;

    if (closeSnoop || !write_to_descriptor(snoop->snooper, d->outbuf, d->outtop))
      closeSnoop = TRUE;

    if (closeSnoop)
    {
      if (snoop->snooper->character != NULL)
        save_char_obj(snoop->snooper->character);
      snoop->snooper->outtop = 0;
      close_socket(snoop->snooper);
    }
  }

  /*
   * OS-dependent output.
   */
  if (!write_to_descriptor(d, d->outbuf, d->outtop))
  {
    d->outtop = 0;
    return FALSE;
  }
  else
  {
    d->outtop = 0;
    return TRUE;
  }
}

void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length)
{
  CHAR_DATA *ch = d->character;
  static char output[2 * MAX_STRING_LENGTH];
  bool underline = FALSE, bold = FALSE;
  int iPtr = 0, last = -1, i = 0, j, k;

  /* the color table... */
  const struct sAnsiColor ansiTable[] =
  {
    { '8',  "30",  eTHIN },
    { '0',  "30",  eBOLD },
    { 'r',  "31",  eTHIN },
    { 'R',  "31",  eBOLD },
    { 'g',  "32",  eTHIN },
    { 'G',  "32",  eBOLD },
    { 'o',  "33",  eTHIN },
    { 'y',  "33",  eBOLD },
    { 'l',  "34",  eTHIN },
    { 'L',  "34",  eBOLD },
    { 'p',  "35",  eTHIN },
    { 'P',  "35",  eBOLD },
    { 'c',  "36",  eTHIN },
    { 'C',  "36",  eBOLD },
    { '7',  "37",  eTHIN },
    { '9',  "37",  eBOLD },

    /* the end tag */
    { '\0',  "",   eTHIN }
  };

  if (length <= 0)
    length = strlen(txt);

  if (length >= MAX_STRING_LENGTH)
  {
    log_string("Write_to_buffer: Way too big. Closing.");
    return;
  }

  /* initial linebreak */
  if (d->outtop == 0 && !d->fcommand)
  {
    d->outbuf[0] = '\n';
    d->outbuf[1] = '\r';
    d->outtop    = 2;
  }

  while (*txt != '\0' && i++ < length)
  {
    /* simple bound checking */
    if (iPtr > (2 * MAX_STRING_LENGTH - 15))
    {
      break;
    }

    switch(*txt)
    {
      default:
        output[iPtr++] = *txt++;
        break;
      case '#':
        i++; txt++;

        /* toggle underline on/off with #u */
        if (*txt == 'u')
        {
          txt++;
          if (underline)
          {
            underline = FALSE;
            output[iPtr++] =  27; output[iPtr++] = '['; output[iPtr++] = '0';
            if (bold)
            {
              output[iPtr++] = ';'; output[iPtr++] = '1';
            }
            if (last != -1 && (!ch || IS_SET(ch->act, PLR_ANSI)))
            {
              output[iPtr++] = ';';
              for (j = 0; ansiTable[last].cString[j] != '\0'; j++)
              {
                output[iPtr++] = ansiTable[last].cString[j];
              }
            }
            output[iPtr++] = 'm';
          }
          else
          {
            underline = TRUE;
            output[iPtr++] =  27; output[iPtr++] = '[';
            output[iPtr++] = '4'; output[iPtr++] = 'm';
          }
        }

        /* parse ## to # */
        else if (*txt == '#')
        {
          txt++;
          output[iPtr++] = '#';
        }

        /* parse #- to ~ */
        else if (*txt == '-')
        {
          txt++;
          output[iPtr++] = '~';
        }

        /* parse #+ to % */
        else if (*txt == '+')
        {
          txt++;
          output[iPtr++] = '%';
        }

        /* #n should clear all tags */
        else if (*txt == 'n')
        {
          txt++;
          if (last != -1 || underline || bold)
          {  
            underline = FALSE;
            bold = FALSE;
            output[iPtr++] =  27; output[iPtr++] = '[';
            output[iPtr++] = '0'; output[iPtr++] = 'm';
          }

          last = -1;
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
              if (last != j && (!ch || IS_SET(ch->act, PLR_ANSI)))
              {
                bool cSequence = FALSE;

                /* escape sequence */
                output[iPtr++] = 27; output[iPtr++] = '[';

                /* remember if a color change is needed */
                if (last == -1 || last / 2 != j / 2)
                  cSequence = TRUE;

                /* handle font boldness */
                if (bold && ansiTable[j].aFlag == eTHIN)
                {
                  output[iPtr++] = '0';
                  bold = FALSE;

                  if (underline)
                  {
                    output[iPtr++] = ';'; output[iPtr++] = '4';
                  }

                  /* changing to eTHIN wipes the old color */
                  output[iPtr++] = ';';
                  cSequence = TRUE;
                }
                else if (!bold && ansiTable[j].aFlag == eBOLD)
                {
                  output[iPtr++] = '1';
                  bold = TRUE;

                  if (cSequence)
                    output[iPtr++] = ';';
                }

                /* add color sequence if needed */
                if (cSequence)
                {
                  for (k = 0; ansiTable[j].cString[k] != '\0'; k++)
                  {
                    output[iPtr++] = ansiTable[j].cString[k];
                  }
                }

                output[iPtr++] = 'm';
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

  /* and terminate it with the standard color */
  if (last != -1 || underline || bold)
  {
    output[iPtr++] =  27; output[iPtr++] = '[';
    output[iPtr++] = '0'; output[iPtr++] = 'm';
  }
  output[iPtr] = '\0';

  /* Expand the buffer as needed. */
  while (d->outtop + iPtr >= d->outsize)
  {
    char *obuf;

    /* There is a limit to our patience */
    if (d->outsize >= 32000)
    {
      log_string("Buffer overflow. Closing.");
      close_socket(d);
      return;
    }
    obuf = calloc(1, sizeof(char *) * 2 * d->outsize);
    strncpy(obuf, d->outbuf, d->outtop);
    free(d->outbuf);
    d->outbuf = obuf;
    d->outsize *= 2;
  }

  /* copy */
  strncpy(d->outbuf + d->outtop, output, iPtr);
  d->outtop += iPtr;
}


/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 * try lowering the max block size.
 */
bool write_to_descriptor_2( int desc, char *txt, int length )
{
  int iStart, nWrite, nBlock;

  if (length <= 0)
    length = strlen(txt);

  increase_total_output(length);

  for ( iStart = 0; iStart < length; iStart += nWrite )
  {
    nBlock = UMIN( length - iStart, 4096 );

    if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
    {
      perror( "Write_to_descriptor" );
      return FALSE;
    }
  }

  return TRUE;
}

/* mccp: write_to_descriptor wrapper */
bool write_to_descriptor(DESCRIPTOR_DATA *d, char *txt, int length)
{
    if (d->out_compress)
        return writeCompressed(d, txt, length);
    else
        return write_to_descriptor_2(d->descriptor, txt, length);
}



/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
  ACCOUNT_DATA *account;
  CHAR_DATA *ch;
  char buf[MAX_STRING_LENGTH];
  char *pwdnew, *p, *option;

  if (d->connected != CON_NOTE_TEXT)
    while (isspace(*argument))
      argument++;

  ch = d->character;

  switch ( d->connected )
  {
    default:
      bug( "Nanny: bad d->connected %d.", d->connected );
      close_socket( d );
      return;
    case CON_NOT_PLAYING:
      break;
    case CON_ACCOUNT_NAME:
      if (argument[0] == '\0')
      {
        close_socket( d );
	return;
      }
      argument = capitalize(argument);
      if (!check_parse_name(argument, FALSE))
      {
        write_to_buffer( d, "Illegal account name, try another.\n\rWhat is your account name? ", 0 );
	return;
      }
      if (strlen(argument) < 3 || (account = load_account(argument)) == NULL)
      {
        if (!d->bResolved)
        {
          write_to_buffer(d,
            "The server is currently committing a DNS search, please wait 5 seconds.\n\r"
            "What is your account name? ", 0);
          return;
        }
        if (check_newbiebanned(d))
        {
          write_to_buffer(d, "Your site is not allowed to create new accounts on this mud\n\r", 0);
          close_socket(d);
          return;
        }
	/* wizlock */
	if (wizlock)
	{
	  write_to_buffer(d, "Currently wizlocked.\n\r", 0);
	  close_socket(d);
	  return;
	}
	account = alloc_account();
	account->owner = str_dup(argument);
	d->account = account;
	d->connected = CON_CONFIRM_ACCOUNT;
	sprintf(buf, "Do you wish to create a new account called '%s' [Y/N]? ", argument);
	write_to_buffer(d, buf, 0);
	return;
      }
      d->account = account;
      if (account->denied > current_time)
      {
        log_string("Denying access to %s@%s.", account->owner, HOSTNAME(d));
	write_to_buffer(d, "You are denied access.\n\r", 0);
        close_socket(d);
	return;
      }
      else if (IS_SET(account->flags, ACCOUNT_FLAG_VACATION))
      {
        /* spam this to the player, so he knows what to do */
        write_to_buffer(d, "#GYou are on vacation - visit http://calim.kyndig.com/index.php to change this.#n\n\r", 0);
        write_to_buffer(d, "#RYou are on vacation - visit http://calim.kyndig.com/index.php to change this.#n\n\r", 0);
        write_to_buffer(d, "#CYou are on vacation - visit http://calim.kyndig.com/index.php to change this.#n\n\r", 0);
        write_to_buffer(d, "#yYou are on vacation - visit http://calim.kyndig.com/index.php to change this.#n\n\r", 0);
        close_socket(d);
        return;
      }
      else if (wizlock && account->level == PLAYER_ACCOUNT)
      {
        write_to_buffer(d, "Currenly wizlocked.\n\r", 0);
        close_socket(d);
        return;
      }

      /* ask for password */
      write_to_buffer(d, "Please enter password: ", 0);
      write_to_buffer(d, (char *) echo_off_str, 0);
      d->connected = CON_OLD_PASSWORD;
      break;
    case CON_OLD_PASSWORD:
      write_to_buffer(d, "\n\r", 2);
      write_to_buffer(d, (char *) echo_on_str, 0);
      if (str_cmp(crypt(argument, d->account->owner), d->account->password))
      {
	write_to_buffer(d, "Wrong Password!\n\r", 0);
	close_socket(d);
	return;
      }
      show_options(d);
      d->connected = CON_PICK_PLAYER;
      break;
    case CON_CONFIRM_ACCOUNT:
      switch (*argument)
      {
        case 'y': case 'Y':
          sprintf(buf, "New account created.\n\rGive me a password for %s: %s",
            d->account->owner, echo_off_str );
          write_to_buffer(d, buf, 0);
	  d->connected = CON_NEW_PASSWORD;
	  break;
	case 'n': case 'N':
	  write_to_buffer(d, "Ok, what IS it, then? ", 0);
	  close_account(d->account);
	  d->account = NULL;
	  d->connected = CON_ACCOUNT_NAME;
	  break;
	default:
          write_to_buffer(d, "Please type Yes or No? ", 0);
	  break;
      }
      break;
    case CON_PICK_REFERENCE:
      if (strlen(argument) < 3 || strlen(argument) > 12 ||
         !str_cmp(argument, d->account->owner) ||
         (account = load_account(argument)) == NULL)
      {
        write_to_buffer(d, "That's not a valid account.\n\r", 0);
        show_options(d);
        d->connected = CON_PICK_PLAYER;
        return;
      }
      log_string("account '%s' is referencing account '%s'.", d->account->owner, account->owner);
      d->account->reference = str_dup(account->owner);
      close_account(account);
      save_account(d->account);
      write_to_buffer(d, "Reference Set.\n\r", 0);
      show_options(d);
      d->connected = CON_PICK_PLAYER;
      break;
    case CON_NEW_PASSWORD:
      #if defined(unix)
        write_to_buffer( d, "\n\r", 2 );
      #endif
      if (strlen(argument) < 5)
      {
        write_to_buffer(d, "Password must be at least five characters long.\n\rPassword: ", 0);
	return;
      }
      pwdnew = crypt(argument, d->account->owner);
      for (p = pwdnew; *p != '\0'; p++)
      {
        if (*p == '~')
        {
          write_to_buffer(d, "New password not acceptable, try again.\n\rPassword: ", 0);
  	  return;
	}
      }
      free_string(d->account->new_password);
      d->account->new_password = str_dup(pwdnew);
      write_to_buffer( d, "Please retype password: ", 0 );
      d->connected = CON_CONFIRM_PASSWORD;
      break;
    case CON_CONFIRM_PASSWORD:
      #if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
      #endif
      if (strcmp(crypt(argument, d->account->owner), d->account->new_password))
      {
        write_to_buffer(d, "Passwords don't match.\n\rRetype password: ", 0);
	d->connected = CON_NEW_PASSWORD;
	return;
      }
      free_string(d->account->password);
      d->account->password = str_dup(d->account->new_password);
      write_to_buffer( d, (char *) echo_on_str, 0 );
      show_options(d);

      /* if this is a new account, then create it */
      if (!account_exists(d->account->owner))
        create_new_account(d->account);

      d->connected = CON_PICK_PLAYER;
      break;
    case CON_PICK_PLAYER:
      if (toupper(argument[0]) == 'C')
      {
	write_to_buffer(d, "What will your name be? ", 0);
	d->connected = CON_NEW_CHARACTER;
      }
      else if (toupper(argument[0]) == 'D')
      {
	write_to_buffer(d, "Which player do you wish to delete? ", 0);
        d->connected = CON_DELETE_PLAYER;
      }
      else if (toupper(argument[0]) == 'P')
      {
	write_to_buffer(d, (char *) echo_off_str, 0);
	write_to_buffer(d, "Please pick a new password: ", 0);
	d->connected = CON_NEW_PASSWORD;
      }
      else if (toupper(argument[0]) == 'R')
      {
        if (!can_refer(d->account))
        {
          write_to_buffer(d, "That is not an option!\n\rWhat will it be? ", 0);
          return;
        }
        write_to_buffer(d, "What account do you wish to reference? ", 0);
        d->connected = CON_PICK_REFERENCE;
      }
      else if (toupper(argument[0]) == 'Q')
      {
        write_to_buffer(d, "\n\n\r", 0);
        switch (number_range(1, 10))
        {
          case 1:
            write_to_buffer(d, "      Honesty is the best policy, but insanity is a better defence\n\r", 0);
            break;
          case 2:
            write_to_buffer(d, "      Some people wish to get what they deserve, while others fear the same\n\r", 0);
            break;
          case 3:
            write_to_buffer(d, "      A wise man gets more use from his enemies than a fool from his friends\n\r", 0);
            break;
          case 4: 
            write_to_buffer(d, "      The best days to drink beer are days that end in the letter, 'Y'\n\r", 0);
            break;
          case 5:
            write_to_buffer(d, "      Pain is only weakness leaving the body\n\r", 0);
            break;
          case 6:
            write_to_buffer(d, "      Trans corpus meum mortuum. - Over my dead body\n\r", 0);
            break;   
          case 7:
            write_to_buffer(d, "                      \\=/, \n\r", 0);
            write_to_buffer(d, "                      |  @___oo  \n\r", 0);
            write_to_buffer(d, "            /\\  /\\   / (___,,,}  \n\r", 0);
            write_to_buffer(d, "          ) /^\\) ^\\/ _)  \n\r", 0);
            write_to_buffer(d, "          )   /^\\/   _)  \n\r", 0);
            write_to_buffer(d, "          )   _ /  / _)  \n\r", 0);
            write_to_buffer(d, "       /\\  )/\\/ ||  | )_)             See you later, alligator\n\r", 0);
            write_to_buffer(d, "      <  >      |(,,) )__)  \n\r", 0);
            write_to_buffer(d, "       ||      /    \\)___)\\  \n\r", 0);
            write_to_buffer(d, "       | \\____(      )___) )___  \n\r", 0);
            write_to_buffer(d, "        \\______(_______;;; __;;; \n\r", 0);
            break;
          case 8:
            write_to_buffer(d, "      To HELL with the Prime Directive.... FIRE!!! - Kirk \n\r", 0);
            break;
          case 9:
            write_to_buffer(d, "      You, in the red uniform, go see what that noise is! \n\r", 0);
            break;
          case 10:
            write_to_buffer(d, "      C.O.B.O.L - Completely Obsolete Boring Old Language \n\r", 0);
            break;
        }
	close_socket(d);
      }
      else
      {
        if ((option = get_option_login(d->account->players, atoi(argument))) == NULL)
	{
	  write_to_buffer(d, "That is not an option!\n\rWhat will it be? ", 0);
	  return;
	}
        else if (check_reconnect(d, option, TRUE))
        {
	  return;
        }
        else if (already_logged(d->account->owner) && d->account->level != CODER_ACCOUNT)
        {
	  write_to_buffer(d, "You already have a different character in the game.\n\r", 0);
	  close_socket(d);
	  return;
        }
        else if (!load_char_obj(d, option))
        {
          write_to_buffer(d, "ERROR: Your pfile is missing!\n\r", 0);
          close_socket(d);
          return;
        }
        else
        {
	  ch = d->character;

          /* run all the login stuff on this char */
          login_char(ch, FALSE);

          ch->pcdata->safe_counter = 5;

          /* add a log entry */
	  log_string("%s@%s has connected.", ch->name, HOSTNAME(d));

          /* remember who is silenced... */
          if (is_silenced(ch))
          {
            log_string("%s@%s is silenced.", ch->name, HOSTNAME(d));
          }

          /* login information */
          {
            int count = 0;

            do_board(ch, "");

            count = SizeOfList(auction_list);

            if (count > 0)
            {
              printf_to_char(ch, "There are (#y%d#n) items up for auction.\n\r", count);
            }
          }

          if (ch->level > 6)
            ;
          else if (IS_SET(ch->pcdata->jflags, JFLAG_SETLOGIN))
            login_message(ch);
          else
 	  {
  	    if (!ragnarok)
              sprintf(buf,"#G%s #9enters #RCalim's Cradle.#n", ch->name );
            else
              sprintf(buf,"#G%s #9enters #RCalim's Cradle #y(#0Ragnarok#y).#n", ch->name );
            enter_info(buf);
	  }

          if (ch->level >= 10 && ccenter_not_stock())
            send_to_char("#RThe Control Center has been modified!#n\n\r", ch);

	  act("$n has entered the game.", ch, NULL, NULL, TO_ROOM);
	  room_text(ch,">ENTER<");
	}
      }
      break;
    case CON_DELETE_PLAYER:
      if (check_reconnect(d, argument, FALSE))
      {
        write_to_buffer(d, "You cannot delete a character who is online.\n\r", 0);
        show_options(d);
        d->connected = CON_PICK_PLAYER;
        return;
      }
      else if (!load_char_obj(d, argument))
      {
        free_char(d->character);
        d->character = NULL;
	write_to_buffer(d, "There is no character with that name.\n\r", 0);
	show_options(d);
	d->connected = CON_PICK_PLAYER;
	return;
      }
      ch = d->character;
      if (str_cmp(ch->pcdata->account, d->account->owner))
      {
	free_char(ch);
	d->character = NULL;
	write_to_buffer(d, "That is not your character.\n\r", 0);
	show_options(d);
	d->connected = CON_PICK_PLAYER;
	return;
      }

      /* delete this character */
      sprintf(buf, "%s %s %d %d", ch->name, class_lookup(ch), ch->played/3600, ch->pcdata->revision);
      free_string(d->account->new_password);
      d->account->new_password = str_dup(buf);
      sprintf(buf, "Are you sure you wish to delete %s [Y/N]? ", ch->name);
      write_to_buffer(d, buf, 0);
      free_char(ch);
      d->character = NULL;
      d->connected = CON_CONFIRM_DEL_PLAYER;
      break;
    case CON_CONFIRM_DEL_PLAYER:
      switch(argument[0])
      {
        default:
	  write_to_buffer(d, "Please answer Yes or No.\n\rAre you sure? ", 0);
	  break;
        case 'y': case'Y':
        {
	  char strsave[MAX_STRING_LENGTH];
	  char *ptr;
	  int len = strlen(d->account->new_password), i;

          if (check_reconnect(d, d->account->new_password, FALSE))
          {
            write_to_buffer(d, "You cannot delete a character who is online.\n\r", 0);
            show_options(d);
            d->connected = CON_PICK_PLAYER;
            return;
          }

	  /* rip out the substring (string parsing) */
	  strcpy(buf, d->account->players);
	  if ((ptr = strstr(buf, d->account->new_password)) == NULL)
	  {
	    sprintf(buf, "Unable to delete '%s' from account '%s'",
              d->account->new_password, d->account->owner);
	    bug(buf, 0);
            write_to_buffer(d, "\n\rDeletion FAILED!\n\r", 0);
            show_options(d);
            d->connected = CON_PICK_PLAYER;
  	    return;
	  }
          d->account->p_count--;
	  if (((i = strlen(buf)) - len) == 0)
	  {
	    free_string(d->account->players);
	    d->account->players = str_dup("");
	  }
          else
	  {
  	    i -= strlen(ptr);
	    if (i > 0 && buf[i-1] == ' ')
	      buf[i-1] = '\0';
            else
	      buf[i++] = '\0';
	    strcat(buf, &d->account->players[i + len]);
	    free_string(d->account->players);
	    d->account->players = str_dup(buf);
	  }

          /* unlink all relevant files and save account */
	  one_argument(d->account->new_password, buf);
	  buf[0] = toupper(buf[0]);
	  for (i = 1; buf[i] != '\0'; i++)
	    buf[i] = tolower(buf[i]);

	  sprintf(strsave, "../accounts/%s/%s.pfile", d->account->owner, buf);
	  unlink(strsave);

          sprintf(strsave, "../accounts/%s/%s.bck", d->account->owner, buf);
          unlink(strsave);

	  sprintf(strsave, "../accounts/whois/%s.whois", buf);
	  unlink(strsave);

          save_account(d->account);

          write_to_buffer(d, "\n\rCharacter deleted.\n\r", 0);
          show_options(d);
          d->connected = CON_PICK_PLAYER;
	  break;
	}
        case 'n': case 'N':
	  show_options(d);
	  d->connected = CON_PICK_PLAYER;
	  break;
      }
      break;
    case CON_NEW_CHARACTER:
      if (d->account->p_count > MAX_CHARACTERS)
      {
	write_to_buffer(d, "You cannot create any more characters.\n\r", 0);
        show_options(d);
        d->connected = CON_PICK_PLAYER;
	return;
      }
      if (argument[0] == '\0')
      {
        show_options(d);
        d->connected = CON_PICK_PLAYER;
        return;
      }
      if (!check_parse_name(argument, TRUE))
      {
	write_to_buffer(d, "That is not a legal name!\n\rWhat will your name be? ", 0);
	return;
      }
      argument[0] = toupper(argument[0]);
      if (char_exists(argument))
      {
	write_to_buffer(d, "There is already another player with that name.\n\rWhat will your name be? ", 0);
	return;
      }

      log_string("%s@%s is trying to connect (new player)", argument, HOSTNAME(d));

      /* create character */
      load_char_obj(d, argument);
      d->connected = CON_CONFIRM_NEW_CHARACTER;
      write_to_buffer(d, "Are you sure? ", 0);
      break;
    case CON_CONFIRM_NEW_CHARACTER:
      switch(argument[0])
      {
        default:
	  write_to_buffer(d, "Please answer Yes or No!\n\rAre you sure? ", 0);
	  break;
        case 'y': case 'Y':
	  d->connected = CON_GET_NEW_CLASS;
	  show_class_options(d);
	  break;
        case 'n': case 'N':
	  free_char(d->character);
	  d->character = NULL;
	  d->connected = CON_NEW_CHARACTER;
	  write_to_buffer(d, "Then what is it? ", 0);
	  break;
      }
      break;
    case CON_GET_NEW_CLASS:
      if (is_valid_class_choice(argument, ch))
      {
        show_class_help(d, ch->class);
        d->connected = CON_CONFIRM_CLASS;
        write_to_buffer(d, "Are you sure you want this class [Y/N] ? ", 0);
      }
      else
      {
	write_to_buffer(d, "That is not a valid class!\n\n\r", 0);
        show_class_options(d);
      }
      break;
    case CON_CONFIRM_CLASS:
      switch(argument[0])
      {   
        default:
          write_to_buffer(d, "Please answer Yes or No!\n\rAre you sure? ", 0);
          break;
        case 'n': case 'N':
          d->connected = CON_GET_NEW_CLASS;
          show_class_options(d);
          break;
        case 'y': case 'Y':
          d->connected = CON_GET_NEW_SEX;
          write_to_buffer(d, "What is your sex [M/F] ? ", 0);
          break;
      }
      break;
    case CON_GET_NEW_SEX:
      switch(argument[0])
      {
	case 'm': case 'M':
          ch->sex = SEX_MALE;
          break;
	case 'f': case 'F':
          ch->sex = SEX_FEMALE;
          break;
	default:
	  write_to_buffer(d, "That's not a sex.\n\rWhat IS your sex? ", 0);
	  return;
      }
      write_to_buffer(d, "Do you wish to use ANSI color (y/n)? ", 0);
      d->connected = CON_GET_NEW_ANSI;
      break;
    case CON_GET_NEW_ANSI:
      switch (argument[0])
      {
	case 'y': case 'Y':
          SET_BIT(ch->act, PLR_ANSI);
          break;
	case 'n': case 'N':
          break;
	default:
	  write_to_buffer(d, "Do you wish to use ANSI (y/n)? ", 0);
	  return;
      }

      if (check_reconnect(d, ch->name, TRUE))
      {                                         
        free_char(ch);
        return;
      }
      else if (already_logged(d->account->owner) && d->account->level != CODER_ACCOUNT)
      {
        write_to_buffer(d, "You already have a different character in the game.\n\r", 0);
        close_socket(d);
        return;
      }

      /* add a log entry */
      log_string("%s@%s new player.", ch->name, HOSTNAME(d));

      /* run login procedures */
      ch->pcdata->revision = CURRENT_REVISION;
      login_char(ch, TRUE);

      /* save this new player */
      account_new_player(d->account, d->character);
      save_char_obj(d->character);

      /* set autostance to mongoose (helps newbies get started */
      ch->stance[STANCE_MOBSTANCE] = STANCE_MONGOOSE;

      /* enter information */
      sprintf(buf,"#9A #Rnew player#9 named #G%s #9enters #RCalim's Cradle.#n", ch->name );
      enter_info(buf);

      /* trigger enter events */
      act("$n has entered the game.", ch, NULL, NULL, TO_ROOM);
      room_text(ch, ">ENTER<");
      break;
    case CON_NOTE_TO:
      handle_con_note_to(d, argument);
      break;
    case CON_NOTE_SUBJECT:
      handle_con_note_subject(d, argument);
      break;
    case CON_NOTE_EXPIRE:
      handle_con_note_expire(d, argument);
      break;
    case CON_NOTE_TEXT:
      handle_con_note_text(d, argument);
      break;
    case CON_NOTE_FINISH:
      handle_con_note_finish(d, argument);
      break;
  }
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name, bool player )
{
  KINGDOM_DATA *kingdom;
  ITERATOR *pIter;
  int i;

  if (player)
  {
    /* Reserved words */
    if (is_name(name, "all wtf auto immortal self someone none save quit why who noone level"))
      return FALSE;

    if (is_contained("fuck",name) || is_contained("bitch",name) || is_contained("whore",name))
      return FALSE;

    /* prevent players from naming themselfs after classes */
    for (i = 1; class_table[i].class_name[0] != '\0'; i++)
    {
      if (!str_cmp(name, class_table[i].class_name))
        return FALSE;
    }

    pIter = AllocIterator(kingdom_list);
    while ((kingdom = (KINGDOM_DATA *) NextInList(pIter)) != NULL)
    {
      if (!str_cmp(kingdom->shortname, name))
          return FALSE;
    }
  }

  /* Length restrictions */
  if (strlen(name) <  3)
    return FALSE;
  if (strlen(name) > 12)
    return FALSE;

  /* Alphanumerics only.
   * Lock out IllIll twits.
   */
  {
    char *pc;
    bool fIll = TRUE;

    for ( pc = name; *pc != '\0'; pc++ )
    {
      if ( !isalpha(*pc) )
       	return FALSE;
      if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
        fIll = FALSE;
    }
    if ( fIll )
      return FALSE;
  }

  /*
   * Prevent players from naming themselves after mobs.
   */
  if (player)
  {
    MOB_INDEX_DATA *pMobIndex;
    int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
      pIter = AllocIterator(mob_index_hash[iHash]);
      while ((pMobIndex = (MOB_INDEX_DATA *) NextInList(pIter)) != NULL)
      {
       	if ( is_name( name, pMobIndex->player_name ))
	  return FALSE;
      }
    }
  }
  return TRUE;
}

/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect(DESCRIPTOR_DATA *d, char *name, bool fConn)
{
  CHAR_DATA *ch;
  ITERATOR *pIter;

  pIter = AllocIterator(char_list);
  while ((ch = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    if (!IS_NPC(ch) && !str_cmp(name, ch->name))
    {
      if (fConn)
      {
        /* drop old connection */
        if (ch->desc)
        {
          send_to_char("This body has been taken over.\n\r", ch);
          close_socket2(ch->desc, TRUE);
        }

	d->character     = ch;
	ch->desc	 = d;
	ch->timer	 = 0;
	send_to_char( "Reconnecting.\n\r", ch );
        act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
	log_string("%s@%s reconnected.",ch->name, HOSTNAME(d));
	d->connected = CON_PLAYING;

        strip_event_socket(d, EVENT_SOCKET_IDLE);

        if (ch->pcdata->in_progress)
 	  send_to_char("You have a note in progress. Type NWRITE to continue it.\n\r", ch);
      }
      return TRUE;
    }
  }

  return FALSE;
}

void stop_idling( CHAR_DATA *ch )
{
  if (ch == NULL || ch->desc == NULL || ch->desc->connected != CON_PLAYING
   || ch->was_in_room == NULL || ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
    return;

  ch->timer = 0;
  char_from_room(ch);
  char_to_room(ch, ch->was_in_room, TRUE);
  ch->was_in_room = NULL;
  act("$n has returned from the void.", ch, NULL, NULL, TO_ROOM);
}


void send_to_char(const char *txt, CHAR_DATA *ch)
{
  CHAR_DATA *wizard;
  CHAR_DATA *familiar;
  
  if (ch->desc == NULL && IS_NPC(ch) && (wizard = ch->wizard) != NULL) 
  {  
    if (!IS_NPC(wizard) && (familiar = wizard->pcdata->familiar) != NULL
      && familiar == ch && ch->in_room != wizard->in_room)
    {    
      if (txt != NULL && wizard->desc != NULL)
      { 
        write_to_buffer(wizard->desc, "[EYE] ", strlen("[EYE] "));
        write_to_buffer(wizard->desc, txt, strlen(txt));
      }  
    }
  }

  if (txt != NULL && ch->desc != NULL)
    write_to_buffer(ch->desc, txt, strlen(txt));
}

/* source: EOD, by John Booth */
void printf_to_char(CHAR_DATA *ch, char *fmt, ...)
{
  char buf [MAX_STRING_LENGTH];
  va_list args;

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  send_to_char (buf, ch);
}

/*
 * The primary output interface for formatted output.
 */
void act( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type )
{
  static char * const he_she [] = { "it",  "he",  "she" };
  static char * const him_her[] = { "it",  "him", "her" };
  static char * const his_her[] = { "its", "his", "her" };
  char buf[MAX_STRING_LENGTH] = { '\0' };
  char fname[MAX_INPUT_LENGTH];
  CHAR_DATA *to, *to_old, *wizard;
  LIST *list;
  ITERATOR *pIter;
  CHAR_DATA *vch = (CHAR_DATA *) arg2;
  CHAR_DATA *familiar = NULL;
  OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
  OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
  const char *str, *i;
  char *point;
  bool is_fam;

  if (format == NULL || format[0] == '\0' || ch == NULL || ch->in_room == NULL)
    return;

  list = ch->in_room->people;

  if (type == TO_VICT)
  {
    if (vch == NULL || vch->in_room == NULL)
      return;

    list = vch->in_room->people;
  }

  pIter = AllocIterator(list);
  while ((to = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    is_fam = FALSE;
    to_old = to;

    if (type == TO_CHAR && to != ch) continue;
    if (type == TO_VICT && (to != vch || to == ch)) continue;
    if (type == TO_ROOM && to == ch) continue;
    if (type == TO_NOTVICT && (to == ch || to == vch)) continue;

    if (to->desc == NULL && IS_NPC(to) && (wizard = to->wizard) != NULL)
    {
      if (!IS_NPC(wizard) && ((familiar = wizard->pcdata->familiar) != NULL) && familiar == to)
      {
        if (to->in_room == ch->in_room && wizard->in_room != to->in_room)
        {
          to = wizard;
          is_fam = TRUE;
        }
      }
    }

    if (to->desc == NULL || !IS_AWAKE(to))
    {
      if (is_fam) to = to_old;
      continue;
    }

    point = buf;
    str = format;

    while (*str != '\0')
    {
      if (*str != '$')
      {
        *point++ = *str++;
        continue;
      }
      ++str;

      if (arg2 == NULL && *str >= 'A' && *str <= 'Z')
      {
        i = " <@@@> ";
      }
      else
      {
        switch (*str)
        {
          default:
            bug("Act: bad code %d.", *str);
            i = " <@@@> ";
            break;
          case 't':
            i = (char *) arg1;
            break;
          case 'T':
            i = (char *) arg2;
            break;
          case 'n':
            i = PERS( ch,  to  );
            break;
          case 'N':
            i = PERS( vch, to  );
            break;   
          case 'e':
            i = he_she[URANGE(0, ch->sex, 2)];
            break;   
          case 'E':
            i = he_she[URANGE(0, vch->sex, 2)];
            break;   
          case 'm':
            i = him_her[URANGE(0, ch->sex, 2)];
            break;   
          case 'M':
            i = him_her[URANGE(0, vch->sex, 2)];
            break;   
          case 's':
            i = his_her[URANGE(0, ch->sex, 2)];
            break;   
          case 'S':
            i = his_her[URANGE(0, vch->sex, 2)];
            break;   
          case 'p':
            i = can_see_obj(to, obj1)
              ? obj1->short_descr
              : "something";
            break;
          case 'P':
            i = can_see_obj( to, obj2 )
              ? obj2->short_descr
              : "something";
            break;
          case 'd':
            if (arg2 == NULL || ((char *) arg2)[0] == '\0')
            {
              i = "door";
            }
            else
            {
              one_argument((char *) arg2, fname);
              i = fname;
            }
            break;
        }
      }
      ++str;
      while ((*point = *i ) != '\0')
        ++point, ++i;
    }
    *point++ = '\n';
    *point++ = '\r';

    if (is_fam)
    {
      if (to->in_room != ch->in_room && familiar && familiar->in_room == ch->in_room)
        send_to_char("[ ", to);
      else
      {
        to = to_old;
        continue;
      }
    }

    buf[0] = UPPER(buf[0]);
    if (to->desc && (to->desc->connected == CON_PLAYING))
      write_to_buffer( to->desc, buf, point - buf );

    if (is_fam) to = to_old;
  }
}


void act_brief(const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type, int iBrief)
{
  static char * const he_she  [] = { "it",  "he",  "she" };
  static char * const him_her [] = { "it",  "him", "her" };
  static char * const his_her [] = { "its", "his", "her" };
  char buf[MAX_STRING_LENGTH] = { '\0' };
  char fname[MAX_INPUT_LENGTH];
  CHAR_DATA *to, *to_old;
  ITERATOR *pIter;
  LIST *list;
  CHAR_DATA *vch = (CHAR_DATA *) arg2;
  CHAR_DATA *familiar = NULL;
  OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
  OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
  const char *str;
  const char *i;
  char *point;
  bool is_fam;

  if (format == NULL ||
      format[0] == '\0' ||
      ch == NULL ||
      ch->in_room == NULL)
    return;
           
  list = ch->in_room->people;

  if (type == TO_VICT)
  {
    if (vch == NULL || vch->in_room == NULL)
      return;

    list = vch->in_room->people;
  }

  pIter = AllocIterator(list);
  while ((to = (CHAR_DATA *) NextInList(pIter)) != NULL)
  {
    is_fam = FALSE;
    to_old = to;

    if (type == TO_CHAR && to != ch) continue;
    if (type == TO_VICT && ( to != vch || to == ch)) continue;
    if (type == TO_ROOM && to == ch) continue;
    if (type == TO_NOTVICT && (to == ch || to == vch)) continue;
    if (to->desc == NULL || !IS_AWAKE(to))
    {
      if (is_fam) to = to_old;
      continue;
    }

    /*
     * Checking for brief
     */
    if (!IS_NPC(to) && to->pcdata->brief[iBrief]) continue;

    point = buf;
    str = format;
    while (*str != '\0')
    {
      if (*str != '$')
      {
        *point++ = *str++;
        continue;
      }
      ++str;

      if (arg2 == NULL && *str >= 'A' && *str <= 'Z')
        i = " <@@@> ";
      else
      {
        switch (*str)
        {
          default:
            bug("Act: bad code %d.", *str);
            i = " <@@@> ";
            break;
          /* Thx alex for 't' idea */
          case 't': i = (char *) arg1;                            break;
          case 'T': i = (char *) arg2;                            break;
          case 'n': i = PERS( ch,  to  );                         break;
          case 'N': i = PERS( vch, to  );                         break;
          case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];        break;
          case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];        break;
          case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];        break;
          case 'M': i = him_her [URANGE(0, vch ->sex, 2)];        break;
          case 's': i = his_her [URANGE(0, ch  ->sex, 2)];        break;
          case 'S': i = his_her [URANGE(0, vch ->sex, 2)];        break;

          case 'p':
            i = can_see_obj( to, obj1 )
                ? obj1->short_descr
                : "something";
            break;

          case 'P':
            i = can_see_obj( to, obj2 )
                ? obj2->short_descr
                : "something";
            break;

          case 'd':
            if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
            {
              i = "door";
            }
            else
            {  
              one_argument( (char *) arg2, fname );
              i = fname;
            }
            break;
        }
      }
      ++str;
      while ( ( *point = *i ) != '\0' )
        ++point, ++i;
    }

    *point++ = '\n';
    *point++ = '\r';
                
    if (is_fam)
    {
      if (to->in_room != ch->in_room && familiar != NULL && familiar->in_room == ch->in_room)
        send_to_char("[ ", to);
      else
      {
        to = to_old;
        continue;
      }
    }
    buf[0] = UPPER(buf[0]);

    if (to->desc && (to->desc->connected == CON_PLAYING))
      write_to_buffer( to->desc, buf, point - buf );

    if (is_fam) to = to_old;
  }
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt( DESCRIPTOR_DATA *d )
{
  CHAR_DATA *ch;
  CHAR_DATA *victim;
  CHAR_DATA *tank;
  EVENT_DATA *event;
  const char *str;
  const char *i = NULL;
  char buf[MAX_STRING_LENGTH] = { '\0' };
  char buf2[MAX_STRING_LENGTH];
  char *point = buf;
  bool is_fighting = TRUE;

  if ((ch = d->character) == NULL) return;
  if (ch->pcdata == NULL)
  {
    send_to_char("\n\r\n\r", ch);
    return;
  }
  if (ch->position == POS_FIGHTING && ch->cprompt[0] == '\0')
  {
    if (ch->prompt[0] == '\0')
    {
      send_to_char("\n\r\n\r", ch);
      return;
    }
    is_fighting = FALSE;
  }
  else if (ch->position != POS_FIGHTING && ch->prompt[0] == '\0')
  {
    send_to_char("\n\r\n\r", ch);
    return;
  }

  if (ch->position == POS_FIGHTING && is_fighting)
    str = d->character->cprompt;
  else
    str = d->character->prompt;

  while (*str != '\0')
  {
    if (*str != '%' || *(str + 1) == '\0')
    {
      *point++ = *str++;
      continue;
    }

    switch (*(++str))
    {
      default:
        i = " ";
        break;
      case 'b':  /* server time */
      {
        char *ptr = ctime(&current_time);

        ptr[16] = '\0';
        sprintf(buf2, "%s", &ptr[11]);
        i = buf2;
        break;
      }
      case 'B':  /* local time */
      {
        char *ptr = ctime(&current_time);
        int hours;

        ptr[13] = '\0';
        ptr[16] = '\0';

        hours = atoi(&ptr[11]);
        if (d->account)
        {
          hours = (hours + d->account->timezone) % 24;
        }

        sprintf(buf2, "%s%d:%s", (hours < 10) ? "0" : "", hours, &ptr[14]);
        i = buf2;
        break;
      }
      case 'h':
        sprintf(buf2, "%s", col_scale(ch->hit, ch->max_hit));
        i = buf2;
        break;
      case 'H':
        sprintf(buf2, "#C%d#n", ch->max_hit);
        i = buf2;
        break;
      case 'm':
        sprintf(buf2, "%s", col_scale(ch->mana, ch->max_mana));
        i = buf2;
        break;
      case 'M':
        sprintf(buf2, "#C%d#n", ch->max_mana);
        i = buf2;
        break;
      case 'v':
        sprintf(buf2, "%s", col_scale(ch->move, ch->max_move));
        i = buf2;
        break;
      case 'V':
        sprintf(buf2, "#C%d#n", ch->max_move);
        i = buf2;
        break;
      case 'x':
        sprintf(buf2, "%d", ch->exp);
        i = buf2;
        break;
      case 'X':
        sprintf(buf2, "\n\r");
        i = buf2; break;
      case 'q':
        sprintf(buf2, "%d", getGold(ch));
        i = buf2;
        break;
      case 'Q':
        if (IS_CLASS(ch, CLASS_SHADOW))
          sprintf(buf2, "%d", ch->pcdata->powers[SHADOW_POWER]);
        else if (IS_CLASS(ch, CLASS_GIANT))
          sprintf(buf2, "%d", ch->pcdata->powers[GIANT_POINTS]);
        else
          sprintf(buf2, "N/A");
        i = buf2;
        break;
      case 'f':
        if ((victim = ch->fighting) == NULL)
        {
          sprintf(buf2, "#CN/A#n");
        }
        else
        {
          if ((victim->hit * 100 / UMAX(1, victim->max_hit)) < 25)
            sprintf(buf2, "#RAwful#n");
          else if ((victim->hit * 100 / UMAX(1, victim->max_hit)) < 50)
            sprintf(buf2, "#LPoor#n");
          else if ((victim->hit * 100 / UMAX(1, victim->max_hit)) < 75)
            sprintf(buf2, "#GFair#n");
          else if ((victim->hit * 100 / UMAX(1, victim->max_hit)) < 100)
            sprintf(buf2, "#yGood#n");
          else if ((victim->hit * 100 / UMAX(1, victim->max_hit)) >= 100)
            sprintf(buf2, "#CPerfect#n");
        }
        i = buf2;
        break;
      case 'F':
        if ((victim = ch->fighting) == NULL)
        {
          sprintf(buf2, "#CN/A#n");
        }
        else if ((tank = victim->fighting) == NULL)
        {
          sprintf(buf2, "#CN/A#n");
        }
        else
        {
          if ((tank->hit * 100 / UMAX(1, tank->max_hit)) < 25)
            sprintf(buf2, "#RAwful#n");
          else if ((tank->hit * 100 / UMAX(1, tank->max_hit)) < 50)
            sprintf(buf2, "#LPoor#n");
          else if ((tank->hit * 100 / UMAX(1, tank->max_hit)) < 75)
            sprintf(buf2, "#GFair#n");
          else if ((tank->hit * 100 / UMAX(1, tank->max_hit)) < 100)
            sprintf(buf2, "#yGood#n");
          else if ((tank->hit * 100 / UMAX(1, tank->max_hit)) >= 100)
            sprintf(buf2, "#CPerfect#n");
        }
        i = buf2;
        break;
      case 'n':
        if ((victim = ch->fighting) == NULL)
          sprintf(buf2, "N/A");
        else
        {
          sprintf(buf2, "%s", PERS(victim, ch));
          buf2[0] = UPPER(buf2[0]);
        }
        i = buf2;
        break;
      case 'N':
        if ((victim = ch->fighting) == NULL)
          sprintf(buf2, "N/A");
        else if ((tank = victim->fighting) == NULL)
          sprintf(buf2, "N/A");
        else
        {
          if (ch == tank)
            sprintf(buf2, "You");
          else
            sprintf(buf2, "%s", PERS(tank, ch));
          buf2[0] = UPPER(buf2[0]);
        }
        i = buf2;
        break;
      case 't':
        sprintf(buf2, "%d", ch->fight_timer);
        i = buf2;
        break;
      case 'T':
        switch(ch->stance[0])
        {
          default:
            sprintf(buf2, "None");
            break;
          case STANCE_BULL:
            sprintf(buf2, "Bull");
            break;
          case STANCE_CRAB:
            sprintf(buf2, "Crab");
            break;
          case STANCE_MONGOOSE:
            sprintf(buf2, "Mongoose");
            break;
          case STANCE_VIPER:
            sprintf(buf2, "Viper");
            break;
          case STANCE_CRANE:
            sprintf(buf2, "Crane");
            break;
          case STANCE_TIGER:
            sprintf(buf2, "Tiger");
            break;
          case STANCE_MONKEY:
            sprintf(buf2, "Monkey");
            break;
          case STANCE_DRAGON:
            sprintf(buf2, "Dragon");
            break;
          case STANCE_MANTIS:
            sprintf(buf2, "Mantis");
            break;
          case STANCE_SWALLOW:
            sprintf(buf2, "Swallow");
            break;
          case STANCE_SPIRIT:
            sprintf(buf2, "Spirit");
            break;
        }
        i = buf2;
        break;
      case 'A':
        if (!ch->fighting)
          sprintf(buf2, "#CN/A#n");
        else
        {
          sprintf(buf2, "*");
          if (ch->fighting->max_hit != 0)
          {
            CHAR_DATA *och = ch->fighting;
            int k;

            for (k = 1; k < 8; k++)
            {
              if (och->hit > (och->max_hit * k / 8))
                strcat(buf2, "*");
              else strcat (buf2, " ");
            }
          }
          else strcat(buf2, "       ");
        }
        i = buf2;
        break;
      case 'a':
        if (!ch->fighting)
          sprintf(buf2, "#CN/A#n");
        else
        {
          if ((ch->fighting->hit * 100 / UMAX(1, ch->fighting->max_hit)) < 25)
            sprintf(buf2, "#R*");
          else if ((ch->fighting->hit * 100 / UMAX(1, ch->fighting->max_hit)) < 50)
            sprintf(buf2, "#L*");
          else if ((ch->fighting->hit * 100 / UMAX(1, ch->fighting->max_hit)) < 75)
            sprintf(buf2, "#G*");
          else if ((ch->fighting->hit * 100 / UMAX(1, ch->fighting->max_hit)) < 100)
            sprintf(buf2, "#y*");
          else
            sprintf(buf2, "#C*");
          if (ch->fighting->max_hit != 0)
          {
            CHAR_DATA *och = ch->fighting;
            int k;

            for (k = 1; k < 8; k++)
            {
              if (och->hit > (och->max_hit * k / 8))
                strcat(buf2, "*");
              else strcat (buf2, " ");
            }
            strcat(buf2, "#n");
          }
          else strcat(buf2, "       ");
        }
        i = buf2;
        break;
      case 'p':
        sprintf(buf2, "%s", col_scale(char_hitroll(ch), 500));
        i = buf2;
        break;
      case 'P' :
        sprintf(buf2, "%s", col_scale(char_damroll(ch), 500));
        i = buf2;
        break;
      case 'k':
        if (IS_CLASS(ch, CLASS_FAE))
        {
          if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_MATTER)) == NULL)
            sprintf(buf2, "0");
          else
          {
            int clvl = atoi(event->argument);

            if (clvl > ch->pcdata->powers[FAE_MATTER])
              sprintf(buf2, "#R%d#n", clvl);
            else if (clvl > 6)
              sprintf(buf2, "#L%d#n", clvl);
            else if (clvl > 3)
              sprintf(buf2, "#G%d#n", clvl);
            else
              sprintf(buf2, "#y%d#n", clvl);
          }
        }
        else if (IS_CLASS(ch, CLASS_WARLOCK))
        {
          if ((event = event_isset_mobile(ch, EVENT_PLAYER_STUDY)) != NULL)
          {
            char arg[MAX_INPUT_LENGTH];
            char *argument = one_argument(event->argument, arg);
  
            sprintf(buf2, "%d", atoi(argument));
          }
          else
          {
            sprintf(buf2, "-");
          }
        }
        else if (IS_CLASS(ch, CLASS_SHADOW))
        {
          if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_KNIFESHIELD))
            sprintf(buf2, "o");
          else
            sprintf(buf2, "-");
        }
        else if (IS_CLASS(ch, CLASS_GIANT))
        {
          if (ch->pcdata->powers[GIANT_STANDFIRM] == 1)
            sprintf(buf2, "o");
          else
            sprintf(buf2, "-");
        }
        else   
        {
          sprintf(buf2, "N/A");
        }
        i = buf2;
        break;
      case 'K':
        if (IS_CLASS(ch, CLASS_FAE))
        {
          if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_ENERGY)) == NULL)
            sprintf(buf2, "0");
          else
          {
            int clvl = atoi(event->argument);

            if (clvl > ch->pcdata->powers[FAE_ENERGY])
              sprintf(buf2, "#R%d#n", clvl);
            else if (clvl > 6)
              sprintf(buf2, "#L%d#n", clvl);
            else if (clvl > 3)
              sprintf(buf2, "#G%d#n", clvl);
            else
              sprintf(buf2, "#y%d#n", clvl);
          }
        }
        else if (IS_CLASS(ch, CLASS_WARLOCK))
        {
          if ((event = event_isset_mobile(ch, EVENT_PLAYER_STUDY)) != NULL)
          {
            sprintf(buf2, "%d", (event_pulses_left(event) / (60 * PULSE_PER_SECOND) + 1));
          }
          else
          {
            sprintf(buf2, "-");
          }
        }
        else if (IS_CLASS(ch, CLASS_SHADOW))
        {
          if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLUR))
            sprintf(buf2, "o");
          else
            sprintf(buf2, "-");
        }
        else if (IS_CLASS(ch, CLASS_GIANT))
        {
          if (event_isset_mobile(ch, EVENT_PLAYER_EARTHFLUX))
            sprintf(buf2, "o");
          else
            sprintf(buf2, "-");
        }
        else 
        {
          sprintf(buf2, "N/A");
        }
        i = buf2;
        break;
      case 'c':
        if (IS_CLASS(ch, CLASS_FAE))
        {
          if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_PLASMA)) == NULL)
            sprintf(buf2, "0");
          else
          { 
            int clvl = atoi(event->argument);

            if (clvl > ch->pcdata->powers[FAE_PLASMA])
              sprintf(buf2, "#R%d#n", clvl);
            else if (clvl > 6)
              sprintf(buf2, "#L%d#n", clvl);
            else if (clvl > 3)
              sprintf(buf2, "#G%d#n", clvl);
            else
              sprintf(buf2, "#y%d#n", clvl);
          }
        }
        else if (IS_CLASS(ch, CLASS_SHADOW))
        {
          if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_BLOODRAGE))
            sprintf(buf2, "o");
          else
            sprintf(buf2, "-");
        }
        else
        {
          sprintf(buf2, "N/A");
        }
        i = buf2;
        break;
      case 'C':
        if (IS_CLASS(ch, CLASS_FAE))
        {
          if ((event = event_isset_mobile(ch, EVENT_PLAYER_FAE_WILL)) == NULL)
            sprintf(buf2, "0");
          else
          { 
            int clvl = atoi(event->argument);

            if (clvl > ch->pcdata->powers[FAE_WILL])
              sprintf(buf2, "#R%d#n", clvl);
            else if (clvl > 6)
              sprintf(buf2, "#L%d#n", clvl);
            else if (clvl > 3)
              sprintf(buf2, "#G%d#n", clvl);
            else
              sprintf(buf2, "#y%d#n", clvl);
          }
        }
        else if (IS_CLASS(ch, CLASS_SHADOW))
        {
          if (IS_SET(ch->pcdata->powers[SHADOW_BITS], NPOWER_SHADOWFORM))
            sprintf(buf2, "o");
          else
            sprintf(buf2, "-");
        }
        else
        {
          sprintf(buf2, "N/A");
        }
        i = buf2;
        break;
      case 'y':
        if (IS_CLASS(ch, CLASS_FAE))
        {
          sprintf(buf2, "%d", ch->pcdata->powers[FAE_SHIELD]);
        }
        else if (IS_CLASS(ch, CLASS_SHADOW))
        {
          if ((victim = ch->fighting) != NULL)
          {
            int health;
 
            health = (victim->max_hit == 0) ? 0 : (100 * victim->hit) / victim->max_hit;
            sprintf(buf2, "%d%%", health);
          }
          else
          {
            sprintf(buf2, "N/A");
          }
        }
        else
        {
          sprintf(buf2, "N/A");
        }
        i = buf2;
        break;
      case 'Y':
        sprintf(buf2, "N/A");
        i = buf2;
        break;
      case '%' :
        sprintf(buf2, "%%");
        i = buf2;
        break;
    }
    ++str;

    while((*point = *i) != '\0')
      ++point, ++i;      
 }

 write_to_buffer(d, buf, point - buf);
}
