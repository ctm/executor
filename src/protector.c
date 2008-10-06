/* Copyright 1992 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_protector[] =
	    "$Id: protector.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#if defined(NEXTSTEP) || defined(LINUX)

/*
 * TODO: make sure no one interferes with SIGIO
 */

#include "rsys/next.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include "rsys/sigio_multiplex.h"

#if defined(NEXTSTEP)
#include <libc.h>
#endif

/*
 * Here's the plan:
 *
 * We try to get a port number as close to BASEPORT as we can.
 * We listen on whatever we get.
 * We broadcast on all ports between BASEPORT and the highest we've seen.
 *    (except the who's alive message is broadcast to all the ports)
 * We advertise when we start up and when we die.
 * We send an alive message whenever in response to a who's alive message.
 * If we don't see an alive message for five minutes we send one.
 */

static LONGINT sock;

#define WHOLIVES	0x50505050
#define ILIVE		0x55555555
#define IDIED		0xAAAAAAAA

#define HASHSIZE	1031

#define BASEPORT	62331
#define NPORTS		10

#define STALE	(5 * 60)	/* 5 minutes */

static LONGINT portcount[NPORTS];

typedef struct __hashentry {
    struct sockaddr_in addr;
    LONGINT datestarted;
    LONGINT datenow;
    struct __hashentry *next;
} hashentry;

hashentry *hashtable[HASHSIZE];
LONGINT hashcount;

time_t datestarted;

typedef struct {
    LONGINT message;
    LONGINT serialnumber;
    LONGINT datenow;
    LONGINT datestarted;
} packet;

static LONGINT ourserialnumber;
static LONGINT maxconcurrent;

static unsigned short curbigport = BASEPORT;

static struct sockaddr_in ourname, broadname;

/*
 * NOTE: sorted hash chain.  This is necessary so we can figure out which
 *	 copies are running on which machines.
 */

static int
compareaddrs(struct sockaddr_in *addr1, struct sockaddr_in *addr2,
	     char *partialmatchp)
{
    ULONGINT u1, u2;
    char retval;

    if (partialmatchp)
	*partialmatchp = FALSE;
    if ((u1 = addr1->sin_addr.s_addr) < (u2 = addr2->sin_addr.s_addr))
	retval = -1;
    else if (u1 == u2) {
	if (partialmatchp)
	    *partialmatchp = TRUE;
	if ((u1 = ntohs (addr1->sin_port)) < (u2 = ntohs (addr2->sin_port)))
	    retval = -1;
	else if (u1 == u2)
	    retval = 0;
	else
	    retval = 1;
    } else
	retval = 1;

    return retval;
}

static hashentry **find(struct sockaddr_in *fromp, char *partialmatchp)
{
    hashentry **retval;
    ULONGINT key;

    key = fromp->sin_addr.s_addr;
    *partialmatchp = FALSE;
    retval = &hashtable[key % HASHSIZE];
    while (*retval && compareaddrs(&(*retval)->addr, fromp, partialmatchp) < 0)
	retval = &(*retval)->next;
    return retval;
}

hashentry *freechain;

void mymallocinit(LONGINT nneeded)
{
    hashentry *p;

    freechain = malloc(sizeof(hashentry) * nneeded);
    for (p = freechain; --nneeded >= 0; ++p) {
	p->next = p+1;
    }
    p[-1].next = 0;
}

void myfree(hashentry *tofree)
{
    tofree->next = freechain;
    freechain = tofree;
}

hashentry *mymalloc( void )
{
    hashentry *retval;

    retval = freechain;
    if (freechain)
	freechain = freechain->next;
    return retval;
}

static void hash_delete(struct sockaddr_in *fromp)
{
    hashentry *totoss;
    hashentry **loc;
    char partialmatch;

    loc = find(fromp, &partialmatch);
    if (*loc && compareaddrs(&(*loc)->addr, fromp, (char *) 0) == 0) {
	totoss = *loc;
	*loc = (*loc)->next;
	myfree(totoss);
	/* the test after the && is to make sure that there aren't other
	   entries with this addr remaining */
	if (!partialmatch && (!*loc ||
		       (*loc)->addr.sin_addr.s_addr != fromp->sin_addr.s_addr))
	    --hashcount;
	if (--portcount[ntohs (fromp->sin_port) - BASEPORT] == 0
	    && ntohs (fromp->sin_port) == curbigport) {
	    while (!portcount[--curbigport - BASEPORT])
		;
	}
    }
}

/*
 * NOTE: we call hash_delete below instead of doing it by hand because we don't
 *	 want to worry about the trickiness of when to decrement hashcount
 *	 and curbigport.
 */


#if !defined(NEXTSTEP)

/*
 * NOTE: we need better routines than this eventually
 */

void toomanycopiesonnet( void )
{
    printf("too many copies on net\n");
    exit(1);
}
#endif

static void incrhashcount( void )
{
    short i;
    hashentry **loc;
    time_t now;

    ++hashcount;
    if (hashcount > maxconcurrent) {
	time(&now);
	/* loop through, purging old stuff */
	for (i = 0; i < HASHSIZE; ++i) {
	    for (loc = &hashtable[i]; *loc; loc = &(*loc)->next) {
		if (now - (*loc)->datenow >= STALE)
		    hash_delete(&(*loc)->addr);
	    }
	}
	if (hashcount > maxconcurrent) {
	    toomanycopiesonnet();
	    exit(1);
	}
    }
}

static void add(packet *bufp, struct sockaddr_in *fromp)
{
    hashentry **loc;
    hashentry *newentryp;
    LONGINT olddate;
    char partialmatch;

    loc = find(fromp, &partialmatch);
    if (*loc && compareaddrs(&(*loc)->addr, fromp, (char *) 0) == 0) {
	if (bufp->datestarted < (olddate = (*loc)->datestarted)) {
#if 0
	    fprintf(stderr, "startdate went backwards\n");
	    /* exit(1); */
	    (*loc)->datestarted = bufp->datestarted;
	    if (olddate > datestarted && bufp->datestarted <= datestarted)
		incrhashcount();
#endif
	}
    } else {
	newentryp = mymalloc();
	if (newentryp) {
	    newentryp->addr = *fromp;
	    newentryp->datestarted = bufp->datestarted;
	    newentryp->datenow = bufp->datenow;
	    newentryp->next = *loc;
	    *loc = newentryp;
	    if (!partialmatch && bufp->datestarted <= datestarted + 2)
		incrhashcount();
	}
    }
    if (ntohs (fromp->sin_port) > curbigport)
	curbigport = ntohs (fromp->sin_port);
    ++portcount[ntohs (fromp->sin_port) - BASEPORT];
}

#if 0
#if defined(BINCOMPAT)
#define OURID 0x40000000
#else
#define OURID 0x20000000
#endif

#else

#define OURID 0x00000000

#endif

static void sendpacket(LONGINT message)
{
    packet buf;
    unsigned short lastport;
    struct sockaddr_in to;
    time_t thetime;

    if (ourserialnumber)
      {
	buf.message = CL(message);
	buf.serialnumber = CL(ourserialnumber | OURID);
	time(&thetime);
	buf.datenow = CL(thetime);
	buf.datestarted = CL(datestarted);
	if (message == WHOLIVES)
	    lastport = BASEPORT + NPORTS - 1;
	else
	    lastport = MAX(ntohs (ourname.sin_port), curbigport);
	for (to = broadname; ntohs (to.sin_port) <= lastport;
	     to.sin_port = htons (ntohs (to.sin_port) + 1))
	    sendto(sock, &buf, sizeof(buf), 0, (struct sockaddr *) &to,
								   sizeof(to));
      }
}

#if !defined(BINCOMPAT)
#define TRAP
#else
#define TRAP	trap
#endif

static TRAP void gotsigio (int signo)
{
    struct sockaddr_in from;
    packet buf;
    LONGINT count;
    time_t currenttime;
    LONGINT fromlen;

    time(&currenttime);
    fromlen = sizeof(from);
    do {
	count = recvfrom(sock, &buf, sizeof(buf), 0, (struct sockaddr *) &from,
							    (void *) &fromlen);
	if (count == sizeof(buf) &&
			     buf.serialnumber == CL(ourserialnumber | OURID)) {

	    buf.message      = CL(buf.message);
	    buf.serialnumber = CL(buf.serialnumber);
	    buf.datenow      = CL(buf.datenow);
	    buf.datestarted  = CL(buf.datestarted);

	    buf.datestarted += currenttime - buf.datenow;
	    buf.datenow = currenttime;
	    switch (buf.message) {
	    case WHOLIVES:
		add(&buf, &from);
		sendpacket(ILIVE);
		break;
	    case ILIVE:
		add(&buf, &from);
		break;
	    case IDIED:
		hash_delete(&from);
		break;
	    }
	}
    } while (count >= 0);
}


#define TRANSFORM_CHAR(c) ((c) ^ 0xA3)

PRIVATE void
transform_string (char *p)
{
  for (;*p;++p)
    *p = TRANSFORM_CHAR (*p);
}

PRIVATE boolean_t
check_net (void)
{
  char env[] =
  {
    TRANSFORM_CHAR('E'),
    TRANSFORM_CHAR('X'),
    TRANSFORM_CHAR('E'),
    TRANSFORM_CHAR('C'),
    TRANSFORM_CHAR('U'),
    TRANSFORM_CHAR('T'),
    TRANSFORM_CHAR('O'),
    TRANSFORM_CHAR('R'),
    TRANSFORM_CHAR('_'),
    TRANSFORM_CHAR('N'),
    TRANSFORM_CHAR('E'),
    TRANSFORM_CHAR('T'),
    TRANSFORM_CHAR('C'),
    TRANSFORM_CHAR('H'),
    TRANSFORM_CHAR('E'),
    TRANSFORM_CHAR('C'),
    TRANSFORM_CHAR('K'),
    0
  };
  boolean_t retval;
  char *name;

  retval = TRUE;

  transform_string (env);
  name = getenv (env);
  transform_string (env);
  if (name && strcmp (name, "0") == 0)
    retval = FALSE;

  return retval;
}

void protectus(LONGINT serialnumber, LONGINT max)
{
  LONGINT err;
  LONGINT val;
  static char beenhere;
  struct ifreq ifr;

  if (check_net ())
    {
      if (serialnumber && !freechain)
	{
	  mymallocinit(MIN(50, max * 4));
	  ourserialnumber = serialnumber;
	  maxconcurrent = max;
	}
      if (!beenhere)
	{
	  int try_port;

	  time(&datestarted);
	  sock = socket(AF_INET, SOCK_DGRAM, 0);
	  sigio_multiplex_install_handler (sock, gotsigio);
#if defined(LINUX)
	  strcpy(ifr.ifr_name, "eth0");
	  if (ioctl(sock, SIOCGIFBRDADDR, &ifr) < 0)
#endif
	    ((struct sockaddr_in *) &ifr.ifr_broadaddr)->sin_addr.s_addr
	      = INADDR_BROADCAST;
	  val = 1;
	  err = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &val, sizeof(val));
	  err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	  ourname.sin_family = AF_INET;
	  ourname.sin_addr.s_addr = INADDR_ANY;
	  memset(ourname.sin_zero, 0, sizeof(ourname.sin_zero));
	  try_port = BASEPORT;
	  do
	    {
	      ourname.sin_port = htons (try_port);
	      err = bind(sock, (struct sockaddr *) &ourname, sizeof(ourname));
	    }
	  while (err < 0 && errno == EADDRINUSE
		 && ++try_port < BASEPORT + NPORTS);
	  if (err == 0)
	    {
	      err = fcntl(sock, F_SETFL, FASYNC|FNDELAY);
	      err = fcntl(sock, F_SETOWN, getpid());
	      broadname.sin_family = AF_INET;
	      broadname.sin_port = htons (BASEPORT);
	      broadname.sin_addr =
		((struct sockaddr_in *) &ifr.ifr_broadaddr)->sin_addr;
	      memset(broadname.sin_zero, 0, sizeof(broadname.sin_zero));
	      beenhere = 1;
	    }
	}
      if (ourserialnumber && beenhere)
	sendpacket(WHOLIVES);
    }
}

void stopprotectingus( void )
{
    sendpacket(IDIED);
}
#endif /* defined(NEXTSTEP) || defined(LINUX) */
