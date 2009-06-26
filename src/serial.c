/*
 * Copyright 1989, 1990, 1995, 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/*
 * This file is an ugly mess of UNIX specific code portable Mac-side code.
 * The UNIX specific stuff should be put in its appropriate place sometime.
 */


#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_serial[] =
		"$Id: serial.c 88 2005-05-25 03:59:37Z ctm $";
#endif

#include "rsys/common.h"

#if defined (MACOSX)
#warning "No serial support for now"
#else /* !defined (MACOSX) */

#include "Serial.h"
#include "DeviceMgr.h"
#include "FileMgr.h"
#include "MemoryMgr.h"
#include "OSUtil.h"

#include "rsys/file.h"
#include "rsys/hfs.h"
#include "rsys/serial.h"

#if defined (MSDOS)
#include "dosserial.h"
#endif

#if defined (CYGWIN32)
#include "win_serial.h"
#endif

#if !defined (CYGWIN32)
#include <termios.h>
#endif

#if defined (LINUX)
#include <sys/ioctl.h>
#endif

#if defined (__alpha) || defined (LINUX)
#define TERMIO
/* #define	MSDOS	ick! */
#endif

/*
 * TODO:
 *
 * Post events on break action or hardware handshake changes IMII-247
 * Support KillIO from the device manager IMII-248
 * Hardware handshaking on machines that support it IMII-252
 *
 * Finish up the get status routine?
 */

#define AINNAME		"\004.AIn"
#define AOUTNAME	"\005.AOut"
#define BINNAME		"\004.BIn"
#define BOUTNAME	"\005.BOut"
#define AINREFNUM	(-6)
#define AOUTREFNUM	(-7)
#define BINREFNUM	(-8)
#define BOUTREFNUM	(-9)

#if !defined(XONC)
#define XONC		'\21'
#define XOFFC		'\23'
#endif

#define SER_START	(1)
#define SER_STOP	(0)

A1(PUBLIC, OSErr, RAMSDOpen, SPortSel, port)	/* IMII-249 */
{
    OSErr err;
    INTEGER rn;

    switch (port) {
    case sPortA:
	if ((err = OpenDriver((StringPtr) AINNAME, &rn)) == noErr)
	    err = OpenDriver((StringPtr) AOUTNAME, &rn);
	break;
    case sPortB:
	if ((err = OpenDriver((StringPtr) BINNAME, &rn)) == noErr)
	    err = OpenDriver((StringPtr) BOUTNAME, &rn);
	break;
    default:
	err = openErr;
	break;
    }
    return err;
}

A1(PUBLIC, void, RAMSDClose, SPortSel, port)	/* IMII-250 */
{
    switch (port) {
    case sPortA:
	(void) CloseDriver(AINREFNUM);
	(void) CloseDriver(AOUTREFNUM);
	break;
    case sPortB:
	(void) CloseDriver(BINREFNUM);
	(void) CloseDriver(BOUTREFNUM);
	break;
    }
}

#define SERSET	8	/* IMII-250 */

A2(PUBLIC, OSErr, SerReset, INTEGER, rn, INTEGER, config)	/* IMII-250 */
{
  config = CW (config);

  return Control(rn, SERSET, (Ptr) &config);
}

#define SERSETBUF	9	/* IMII-251 */

A3(PUBLIC, OSErr, SerSetBuf, INTEGER, rn, Ptr, p, INTEGER, len)	/* IMII-251 */
{
    sersetbuf_t temp;

    temp.p = RM (p);
    temp.i = CW (len);

    return Control(rn, SERSETBUF, (Ptr) &temp);
}

#define SERHSHAKE	10	/* IMII-251 */

A2(PUBLIC, OSErr, SerHShake, INTEGER, rn, SerShk, flags)	/* IMII-251 */
{
    return Control(rn, SERHSHAKE, (Ptr) &flags);
}

#define SERSETBRK	12	/* IMII-252 */

A1(PUBLIC, OSErr, SerSetBrk, INTEGER, rn)			/* IMII-252 */
{
    return Control(rn, SERSETBRK, (Ptr) 0);
}

#define SERCLRBRK	11	/* IMII-253 */

A1(PUBLIC, OSErr, SerClrBrk, INTEGER, rn)			/* IMII-253 */
{
    return Control(rn, SERCLRBRK, (Ptr) 0);
}

#define SERGETBUF	2	/* IMII-253 */

A2(PUBLIC, OSErr, SerGetBuf, INTEGER, rn, LONGINT *, lp)	/* IMII-253 */
{
    INTEGER status[11];
    OSErr err;

    if ((err = Status(rn, SERGETBUF, (Ptr) status)) == noErr)
	*lp = *(LONGINT *)status;
    return err;
}

#define SERSTATUS	8	/* IMII-253 */

A2(PUBLIC, OSErr, SerStatus, INTEGER, rn, SerStaRec *, serstap)	/* IMII-253 */
{
    INTEGER status[11];
    OSErr err;

    if ((err = Status(rn, SERSTATUS, (Ptr) status)) == noErr)
	BlockMove((Ptr) status, (Ptr) serstap, (Size) sizeof(*serstap));
    return err;
}

#define OPENBIT	(1 << 0)

#if !defined(MSDOS) && !defined (CYGWIN32)
#if defined(TERMIO)

typedef struct {
    LONGINT fd;		/* will be the same for both In and Out */
    struct termio state;
} hidden, *hiddenp;

#else

typedef struct {
    LONGINT fd;		/* will be the same for both In and Out */
    struct sgttyb sgttyb;
    struct tchars tchars;
    ULONGINT lclmode;
} hidden, *hiddenp;

#endif
#else

typedef struct
{
  LONGINT fd; /* probably need to save more state information */
  /* right now we just use fd to keep track of which port we're talking to */
}
hidden, *hiddenp;

#endif

MAKE_HIDDEN(hiddenp);
typedef HIDDEN_hiddenp *hiddenh;


A1(PRIVATE, DCtlPtr, otherdctl, ParmBlkPtr, pbp)
{
    DCtlHandle h;

    h = 0;
    switch (CW(pbp->cntrlParam.ioCRefNum)) {
    case AINREFNUM:
	h = GetDCtlEntry(AOUTREFNUM);
	break;
    case AOUTREFNUM:
	h = GetDCtlEntry(AINREFNUM);
	break;
    case BINREFNUM:
	h = GetDCtlEntry(BOUTREFNUM);
	break;
    case BOUTREFNUM:
	h = GetDCtlEntry(BINREFNUM);
	break;
    }
    return h ? STARH(h) : 0;
}

#if defined (LINUX) || defined (NEXTSTEP)

/*
 * NOTE:  Currently we're using cufa and cufb; we really should
 *	  delay the opening until we know just what type of flow
 *	  control the user wants to do.
 */
PRIVATE char *specialname(ParmBlkPtr pbp, const char **lockfilep,
						        const char **tempfilep)
{
    char *retval;

    switch (CW(pbp->cntrlParam.ioCRefNum)) {
    case AINREFNUM:
    case AOUTREFNUM:
#if defined (NEXTSTEP)
	retval = "/dev/cufa";
	*lockfilep = "/usr/spool/uucp/LCK/LCK..cufa";
	*tempfilep = "/usr/spool/uucp/LCK/etempcufa";
#elif defined (LINUX)
	retval = "/dev/cua0";
	*lockfilep = 0; /* for now */
	*tempfilep = 0; /* for now */
#endif
	break;
    case BINREFNUM:
    case BOUTREFNUM:
#if defined (NEXTSTEP)
	retval = "/dev/cufb";
	*lockfilep = "/usr/spool/uucp/LCK/LCK..cufb";
	*tempfilep = "/usr/spool/uucp/LCK/etempcufa";
#elif defined (LINUX)
	retval = "/dev/cua1";
	*lockfilep = 0;
	*tempfilep = 0;
#endif
	break;
    default:
	*lockfilep = 0;
	retval = 0;
    }
    return retval;
}
#endif /* defined (LINUX) || defined (NEXTSTEP) */

typedef void (*compfuncp)( void );

#if defined(BINCOMPAT)

void callcomp(ParmBlkPtr pbp, compfuncp comp, OSErr err)
{
    EM_A0 = (LONGINT) (long) US_TO_SYN68K(pbp);
    EM_A1 = (LONGINT) (long) US_TO_SYN68K(comp);
    EM_D0 = (unsigned short) err;	/* TODO: unsigned short ? */
    CALL_EMULATOR((syn68k_addr_t) (long) comp);
}

#define DOCOMPLETION(pbp, err)						      \
    (pbp)->ioParam.ioResult = CW(err);					      \
    if (((pbp)->ioParam.ioTrap & asyncTrpBit) && (pbp)->ioParam.ioCompletion) \
	callcomp(pbp, (compfuncp) CL((long)(pbp)->ioParam.ioCompletion), err);	      \
    return err

#else

#warning BINCOMPAT not defined

#define DOCOMPLETION(pbp, err)				\
    (pbp)->ioParam.ioResult = CW(err);			\
    if ((pbp)->ioParam.ioTrap & asyncTrpBit)		\
	(*(compfuncp) (pbp)->ioParam.ioCompletion)();	\
    return err

#endif

#define SERIALDEBUG

#if defined (LINUX) || defined (NEXTSTEP)
PRIVATE const char *lockname;
#endif

A2(PUBLIC, OSErr, ROMlib_serialopen, ParmBlkPtr, pbp,		/* INTERNAL */
								 DCtlPtr, dcp)
{
    OSErr err;
    auto DCtlPtr otherp;	/* auto due to old compiler bug */
    hiddenh h;
#if defined (LINUX) || defined (NEXTSTEP)
    const char *devname, *tempname;
    LONGINT fd, ourpid, theirpid, newfd, oumask;
#endif

    err = noErr;
    if (!(dcp->dCtlFlags & CWC(OPENBIT))) {
	h = (hiddenh) NewHandle(sizeof(hidden));
	dcp->dCtlStorage = (Handle) RM(h);
	otherp = otherdctl(pbp);
	if (otherp && (otherp->dCtlFlags & CWC(OPENBIT))) {
	    *STARH(h) = *STARH((hiddenh) (long) MR(otherp->dCtlStorage));
	    dcp->dCtlFlags |= CWC(OPENBIT);
	} else {
#if defined (LINUX) || defined (NEXTSTEP)
	    err = permErr;
	    if ((devname = specialname(pbp, &lockname, &tempname))) {
		oumask = umask(0);
		if (!tempname)
		  err = noErr;
		else if ((fd = Uopen(tempname, O_BINARY|O_CREAT|O_WRONLY, 0666L))
									 >= 0) {
		    ourpid = getpid();
		    if (write(fd, &ourpid, sizeof(ourpid)) == sizeof(ourpid)) {
			if (Ulink(tempname, lockname) == 0)
			    err = noErr;
			else {
			    if ((newfd = Uopen(lockname, O_BINARY|O_RDWR, 0))
									>= 0) {
				if (read(newfd, &theirpid, sizeof(theirpid))
							   == sizeof(theirpid))
				    if ((kill(theirpid, 0) != 0) &&
							      errno == ESRCH) {
					err = noErr;
					Uunlink(lockname);
					Ulink(tempname, lockname);
				    }
				close(newfd);
			    }
			}
			Uunlink(tempname);
		    }
		    close(fd);
		}
		umask(oumask);
	    }
#endif
	    if (err == noErr) {
#if defined (LINUX) || defined (NEXTSTEP)
		HxX(h, fd) = ROMlib_priv_open(devname, O_BINARY|O_RDWR);
		if (HxX(h, fd) < 0) 
		    err = HxX(h, fd);	/* error return piggybacked */
		else {
#if defined(TERMIO)
		    err = ioctl(HxX(h, fd), TCGETA, &HxX(h, state)) < 0 ?
						     ROMlib_maperrno() : noErr;
#else
		    if (ioctl(HxX(h, fd), TIOCGETP, &HxX(h, sgttyb)) < 0 ||
			    ioctl(HxX(h, fd), TIOCGETC, &HxX(h, tchars)) < 0 ||
			     ioctl(HxX(h, fd), TIOCLGET, &HxX(h, lclmode)) < 0)
			err = ROMlib_maperrno();
#endif
#else
		    HxX(h, fd) = (CW(pbp->cntrlParam.ioCRefNum) == AINREFNUM ||
		      CW(pbp->cntrlParam.ioCRefNum) == AOUTREFNUM) ? 0 : 1;
#endif
		    dcp->dCtlFlags |= CWC(OPENBIT);
		    SerReset(CW(pbp->cntrlParam.ioCRefNum),
			    (CW(pbp->cntrlParam.ioCRefNum) == AINREFNUM ||
			     CW(pbp->cntrlParam.ioCRefNum) == AOUTREFNUM) ?
						    CW(SPPortA) : CW(SPPortB));
#if defined (LINUX) || defined (NEXTSTEP)
		}
#endif
	    }
	}
    }
#if defined(SERIALDEBUG)
    warning_trace_info("serial open returning %d", (LONGINT) err);
#endif
    DOCOMPLETION(pbp, err);
}

A2(PUBLIC, OSErr, ROMlib_serialprime, ParmBlkPtr, pbp,		/* INTERNAL */
								 DCtlPtr, dcp)
{
    OSErr err;
    hiddenh h;
    char *buf;
    size_t req_count;

    buf = (char *) MR (pbp->ioParam.ioBuffer);
    req_count = CL (pbp->ioParam.ioReqCount);

    err = noErr;
    if (!(dcp->dCtlFlags & CWC(OPENBIT)))
	err = notOpenErr;
    else {
#if defined(SERIALDEBUG)
    warning_trace_info ("serial prime code %d", (LONGINT) (CW(pbp->ioParam.ioTrap) & 0xFF));
#endif
	h = (hiddenh) MR(dcp->dCtlStorage);
	switch (CW(pbp->ioParam.ioTrap) & 0xFF) {
	case aRdCmd:
	    if (CW(pbp->cntrlParam.ioCRefNum) != AINREFNUM &&
				    CW(pbp->cntrlParam.ioCRefNum) != BINREFNUM)
		err = readErr;
	    else {
		/* this may have to be changed since we aren't looking for
		   parity and framing errors */
#if defined (LINUX) || defined (NEXTSTEP)
		pbp->ioParam.ioActCount = CL(read(HxX(h, fd), buf, req_count));
#elif defined (MSDOS) || defined (CYGWIN32)
		pbp->ioParam.ioActCount = CL(serial_bios_read(HxX(h, fd), buf,
							      req_count));
#else
#warning not sure what to do here
#endif
#if defined(SERIALDEBUG)
    warning_trace_info ("serial prime read %d bytes, first is 0x%0x",
					    (LONGINT) CL(pbp->ioParam.ioActCount),
			  (LONGINT) (unsigned char) buf[0]);
#endif
	    }
	    break;
	case aWrCmd:
	    if (CW(pbp->cntrlParam.ioCRefNum) != AOUTREFNUM &&
				       CW(pbp->cntrlParam.ioCRefNum) != BOUTREFNUM)
		err = writErr;
	    else {
#if defined(SERIALDEBUG)
    warning_trace_info ("serial prime writing %d bytes, first is 0x%0x",
					        (LONGINT) req_count,
			      (LONGINT) (unsigned char) buf[0]);
#endif
#if defined (LINUX) || defined (NEXTSTEP)
		pbp->ioParam.ioActCount = CL(write(HxX(h, fd),
			       buf, req_count));
#elif defined (MSDOS) || defined (CYGWIN32)
		pbp->ioParam.ioActCount = CL( serial_bios_write(HxX(h, fd),
			       buf, req_count));
#else
#warning not sure what to do here
#endif
	    }
	    break;
	default:
	    err = badUnitErr;
	    break;
	}
    }
#if defined(SERIALDEBUG)
    warning_trace_info ("serial prime returning %d", (LONGINT) err);
#endif
    DOCOMPLETION(pbp, err);
}

#if defined(TERMIO)

#define OURCS7		(CS7)
#define OURCS8		(CS8)
#define OURODDP		(PARENB|PARODD)
#define OUREVENP	(PARENB)

#else

#define OURCS7		(0)

#if defined(NEXTSTEP)
#define OURCS8		(LPASS8|LPASS8OUT)
#else
#define OURCS8		(LPASS8)
#endif


#define OURODDP		(ODDP)
#define OUREVENP	(EVENP)

#endif

A2(PRIVATE, OSErr, serset, LONGINT, fd, INTEGER, param)
{
#if defined (MSDOS) || defined (CYGWIN32)
  OSErr retval;

  retval = serial_bios_serset (fd, param);
  return retval;
#else


#if defined(TERMIO)
    struct termio settings;
#else
    struct sgttyb sgttyb;
    struct tchars tchars;
    ULONGINT lclmode;
#endif

    OSErr err;
    LONGINT baud, csize, stopbits, parity;

#if !defined (LETGCCWAIL)
    baud = 0;
    csize = 0;
    stopbits = 0;
#endif

    err = noErr;

    switch (param & 0x3FF) {
    case baud300:
	baud = B300; 
	break;
    case baud600:
	baud = B600; 
	break;
    case baud1200:
	baud = B1200; 
	break;
    case baud1800:
	baud = B1800; 
	break;
    case baud2400:
	baud = B2400; 
	break;
    case baud4800:
	baud = B4800; 
	break;
    case baud9600:
	baud = B9600; 
	break;
    case baud19200:
	baud = EXTA; 
	break;
    case 1:		/* really 38400 according to Terminal 2.2 source */
	baud = EXTB;
	break;
    case baud3600:	/* these case labels aren't necessary since they */
    case baud7200:	/* fall into default anyway, just thought you */
    case baud57600:	/* might like to know what you're missing */
    default:
/*
 * NOTE: setting controlErr here isn't probably a good idea, since
 *       Compuserve Signup does a serset with baud57600 and then later
 *	 adjusts the baud with a "setbaud"
 */
#if 0
	err = controlErr;
#else
	baud = B2400;
#endif
	break;
    }

    switch (param & 0xC00) {

    case data5:
    case data6:
	err = controlErr;
	break;

    case data7:
	csize = OURCS7;
	break;

    case data8:
	csize = OURCS8;
	break;

    }

    switch (param & 0x3000) {

    case oddParity:
	parity = OURODDP;
	break;

    case evenParity:
	parity = OUREVENP;
	break;

    default:
	parity = 0;
	break;
    }

    switch ((param >> 14) & 0x3) {
    case 1:
	stopbits = 0;
	break;
    case 3:
#if defined(TERMIO)
	stopbits = CSTOPB;
	break;
#endif
    case 0:
    case 2:
	err = controlErr;
	break;
    }

    if (err == noErr) {
#if defined(TERMIO)
	settings.c_iflag = IGNPAR | IXON | IXOFF;
	settings.c_oflag = 0;
	settings.c_cflag = baud | csize | stopbits | parity;
	settings.c_lflag = 0;
	settings.c_line  = 0;	/* TODO: find out real value here */
	err = ioctl(fd, TCSETAW, &settings) < 0 ? ROMlib_maperrno() : noErr;
#else
	sgttyb.sg_ispeed = sgttyb.sg_ospeed = baud;
	sgttyb.sg_erase = -1;
	sgttyb.sg_kill = -1;
	sgttyb.sg_flags = TANDEM | CBREAK | parity;
	tchars.t_intrc = -1;
	tchars.t_quitc = -1;
#if 0 /* This isn't the place to mess with flow control */
	if (sershkp->fXOn || sershkp->fInX) {
	    tchars.t_startc = sershkp->xOn;
	    tchars.t_stopc = sershkp->xOff;
	} else {
	    tchars.t_startc = sershkp->xOn;
	    tchars.t_stopc = sershkp->xOff;
	}
	/* Cliff, LDECCTQ restricts XON to startc ONLY! --Lee */
	lclmode = sershkp->fXOn ? LDECCTQ : 0;
#else
	lclmode = 0;
#endif
	tchars.t_eofc = -1;
	tchars.t_brkc = -1;
	lclmode |= LLITOUT | LNOHANG | csize | LNOFLSH;
	if (ioctl(fd, TIOCSETP, &sgttyb) < 0 ||
					    ioctl(fd, TIOCSETC, &tchars) < 0 ||
					     ioctl(fd, TIOCLSET, &lclmode) < 0)
	    err = ROMlib_maperrno();
#endif
    }
    return err;
#endif
}

A2(PRIVATE, OSErr, serxhshake, LONGINT, fd, SerShk *, sershkp)
{
#if defined (MSDOS) || defined (CYGWIN32)
  OSErr retval;

  retval = serial_bios_serxhshake (fd, sershkp);
  return retval;
#else

#if defined(TERMIO)
    struct termio settings;
#else
    struct sgttyb sgttyb;
    struct tchars tchars;
#endif
    OSErr err;

    err = noErr;
#if defined(TERMIO)
    if (ioctl(fd, TCGETA, &settings) < 0)
	err = ROMlib_maperrno();
#else
    if (ioctl(fd, TIOCGETP, &sgttyb) < 0 || ioctl(fd, TIOCGETC, &tchars) < 0)
	    err = ROMlib_maperrno();
#endif
    else {
	if (sershkp->fXOn) {
#if defined(TERMIO)
	    settings.c_iflag |=  IXON;
	} else
	    settings.c_iflag &= ~IXON;
#else
	    /* NB: LDECCTQ is left (un)set in the local mode word. */
	    tchars.t_startc = -1;
	    tchars.t_stopc = -1;
	} else {
	    tchars.t_startc = sershkp->xOn;
	    tchars.t_stopc = sershkp->xOff;
	}
#endif
	if (sershkp->fInX) {
#if defined(TERMIO)
	    settings.c_iflag |=  IXOFF;
	} else
	    settings.c_iflag &= ~IXOFF;
	    err = ioctl(fd, TCSETAW, &settings) < 0 ? ROMlib_maperrno() : noErr;
#else
	    /* Can't enable TANDEM without setting startc & stopc. If I
	     * do that, input flow control is also enabled. This is what
	     * has been implemented here. The alternative is to refuse the
	     * request.
	     */
	    sgttyb.sg_flags |= TANDEM;
	    tchars.t_startc = sershkp->xOn;
	    tchars.t_stopc = sershkp->xOff;
	} else
	    sgttyb.sg_flags &= ~TANDEM;
	if (ioctl(fd, TIOCSETP, &sgttyb) < 0 ||
					      ioctl(fd, TIOCSETC, &tchars) < 0)
	    err = ROMlib_maperrno();
#endif
    }
    return err;
#endif
}

A2(PRIVATE, OSErr, setbaud, LONGINT, fd, INTEGER, baud)
{
#if defined (MSDOS) || defined (CYGWIN32)
  OSErr retval;

  retval = serial_bios_setbaud (fd, baud);
  return retval;
#else

#if defined(TERMIO)
    struct termio settings;
#else
    struct sgttyb sgttyb;
#endif
    static INTEGER rates[] = {
	     50,	  75,	 110,	 134,	 150,	 200,	  300,
	    600,	1200,	1800,	2400,	4800,	9600,	19200,
	  38400, 	32767,
    }, *ip;
    OSErr err;

    for (ip = rates; *ip < baud ; ip++)
	;
    if (*ip == 32767 || ip[0] - baud > baud - ip[-1])
	--ip;
#if defined(TERMIO)
    if (ioctl(fd, TCGETA, &settings) < 0)
#else
    if (ioctl(fd, TIOCGETP, &sgttyb) < 0)
#endif
	err = ROMlib_maperrno();
    else {
#if defined(TERMIO)
	settings.c_cflag &= ~CBAUD;
	settings.c_cflag |= ip - rates + 1;
	err = ioctl(fd, TCSETAW, &settings) < 0 ? ROMlib_maperrno() : noErr;
#else
	sgttyb.sg_ispeed = sgttyb.sg_ospeed = ip - rates + 1;
	err = ioctl(fd, TIOCGETP, &sgttyb) < 0 ? ROMlib_maperrno() : noErr;
#endif
    }
    return err;
#endif
}

A2(PRIVATE, OSErr, ctlbrk, LONGINT, fd, INTEGER, flag)
{
#if defined (MSDOS) || defined (CYGWIN32)
  OSErr retval;

  retval = serial_bios_ctlbrk (fd, flag);
  return retval;
#else
	OSErr err;

#if defined(TERMIO)
    if (flag == SER_START) {
	/* NOTE:  This will send a break for 1/4 sec */
	err = ioctl(fd, TCSBRK, 0);
    } else
	err = noErr;
#else
    err = ioctl(fd, flag == SER_START ? TIOCSBRK : TIOCCBRK, 0);
#endif
    return err < 0 ? ROMlib_maperrno() : noErr;
#endif
}

A2(PRIVATE, OSErr, flow, LONGINT, fd, LONGINT, flag)
{
#if defined (MSDOS) || defined (CYGWIN32)
  OSErr retval;

  retval = serial_bios_setflow (fd, flag);
  return retval;
#else
    OSErr err;

#if defined(TERMIO)
#if !defined(__alpha)
    err = ioctl(fd, TCXONC, flag == SER_STOP ? 0 : 1);
#else
    err = ioctl(fd, TCXONC, (void *)(flag == SER_STOP ? 0L : 1L));
#endif
#else
    err = ioctl(fd, flag == SER_STOP ? TIOCSTOP : TIOCSTART, 0);
#endif
    return err < 0 ? ROMlib_maperrno() : noErr;
#endif
}

#define SERBAUDRATE	13

#define SERXHSHAKE	14	/* IMIV-226 */
#define SERMISC		16	/* IMIV-226 */
#define SERSETDTR	17	/* IMIV-226 */
#define SERCLRDTR	18	/* IMIV-226 */

#define SERCHAR		19

#define SERXCHAR	20	/* IMIV-226 */

#define SERUSETXOFF	21
#define SERUCLRXOFF	22
#define SERCXMITXON	23
#define SERUXMITXON	24
#define SERCXMITXOFF	25
#define SERUXMITXOFF	26
#define SERRESET	27

#define SERKILLIO	 1

A2(PUBLIC, OSErr, ROMlib_serialctl, ParmBlkPtr, pbp,		/* INTERNAL */
								 DCtlPtr, dcp)
{
    OSErr err;
    hiddenh h;
	char c;

    if (!(dcp->dCtlFlags & CWC(OPENBIT)))
	err = notOpenErr;
    else {
#if defined(SERIALDEBUG)
	warning_trace_info ("serial control code = %d, param0 = 0x%x",
					 (LONGINT) CW(pbp->cntrlParam.csCode),
		       (LONGINT) (unsigned short) CW(pbp->cntrlParam.csParam[0]));
#endif
	h = (hiddenh) MR(dcp->dCtlStorage);
	switch (CW(pbp->cntrlParam.csCode)) {
	case SERKILLIO:
	    err = noErr;	/* All I/O done synchronously */
	    break;
	case SERSET:
	    err = serset(HxX(h, fd), CW(pbp->cntrlParam.csParam[0]));
	    break;
	case SERSETBUF:
	    err = noErr;	/* ignored */
	    break;
	case SERHSHAKE:
	case SERXHSHAKE:	/* NOTE:  DTR handshake isn't supported  */
	    err = serxhshake(HxX(h, fd), (SerShk *) pbp->cntrlParam.csParam);
	    break;
	case SERSETBRK:
	    err = ctlbrk(HxX(h, fd), SER_START);
	    break;
	case SERCLRBRK:
	    err = ctlbrk(HxX(h, fd), SER_STOP);
	    break;
	case SERBAUDRATE:
	    err = setbaud(HxX(h, fd), CW(pbp->cntrlParam.csParam[0]));
	    break;
	case SERMISC:
#if defined (MSDOS) || defined (CYGWIN32)
	    err = serial_bios_setdtr (HxX(h, fd));
#else
	    err = controlErr;	/* not supported */
#endif
	    break;
	case SERSETDTR:
#if defined (MSDOS) || defined (CYGWIN32)
	    err = serial_bios_clrdtr (HxX(h, fd));
#else
	    err = controlErr;	/* not supported */
#endif
	    break;
	case SERCLRDTR:
	    err = controlErr;	/* not supported */
	    break;
	case SERCHAR:
	    err = controlErr;	/* not supported */
	    break;
	case SERXCHAR:
	    err = controlErr;	/* not supported */
	    break;
	case SERUSETXOFF:
	    err = flow(HxX(h, fd), SER_STOP);
	    break;
	case SERUCLRXOFF:
	    err = flow(HxX(h, fd), SER_START);
	    break;
	case SERCXMITXON:
	    err = controlErr;	/* not supported */
	    break;
	case SERUXMITXON:
	    c = XONC;
	    err = write(HxX(h, fd), &c, 1) != 1 ? ROMlib_maperrno() : noErr;
	    break;
	case SERCXMITXOFF:
	    err = controlErr;	/* not supported */
	    break;
	case SERUXMITXOFF:
	    c = XOFFC;
	    err = write(HxX(h, fd), &c, 1) != 1 ? ROMlib_maperrno() : noErr;
	    break;
	case SERRESET:
	    err = controlErr;	/* not supported */
	    break;
	default:
	    err = controlErr;
	    break;
	}
    }
#if defined(SERIALDEBUG)
    warning_trace_info ("serial control returning %d", (LONGINT) err);
#endif
    DOCOMPLETION(pbp, err);
}

/*
 * NOTE:  SERSTATUS lies about everything except rdPend.
 */

A2(PUBLIC, OSErr, ROMlib_serialstatus, ParmBlkPtr, pbp,		/* INTERNAL */
								 DCtlPtr, dcp)
{
    OSErr err;
    hiddenh h;
    LONGINT n;

    if (!(dcp->dCtlFlags & CWC(OPENBIT)))
	err = notOpenErr;
    else {
#if defined(SERIALDEBUG)
    warning_trace_info ("serial status csCode = %d", (LONGINT) CW(pbp->cntrlParam.csCode));
#endif
	h = (hiddenh) MR(dcp->dCtlStorage);
	switch (CW(pbp->cntrlParam.csCode)) {
	case SERGETBUF:
#if defined (LINUX) || defined (NEXTSTEP)
	    if (ioctl(HxX(h, fd), FIONREAD, &n) < 0)
#else
	    if (serial_bios_fionread(HxX(h, fd), &n) < 0)
#endif
		err = ROMlib_maperrno();
	    else {
#if defined(SERIALDEBUG)
    warning_trace_info ("serial status getbuf = %d", (LONGINT) n);
#endif
		*(LONGINT *) pbp->cntrlParam.csParam = CL(n);
		err = noErr;
	    }
	    break;
	case SERSTATUS:
#if defined (LINUX) || defined (NEXTSTEP)
	    if (ioctl(HxX(h, fd), FIONREAD, &n) < 0)
#else
	    if (serial_bios_fionread (HxX(h, fd), &n) < 0)
#endif
		err = ROMlib_maperrno();
	    else {
		((SerStaRec *)pbp->cntrlParam.csParam)->cumErrs  = 0;
		((SerStaRec *)pbp->cntrlParam.csParam)->xOffSent = 0;
		((SerStaRec *)pbp->cntrlParam.csParam)->rdPend   = n>0 ? 1 : 0;
		((SerStaRec *)pbp->cntrlParam.csParam)->wrPend   = 0;
		((SerStaRec *)pbp->cntrlParam.csParam)->ctsHold  = 0;
		((SerStaRec *)pbp->cntrlParam.csParam)->xOffHold = 0;
		err = noErr;
	    }
	    break;
	default:
	    err = statusErr;
	    break;
	}
    }
#if defined(SERIALDEBUG)
    warning_trace_info ("serial status returning %d", (LONGINT) err);
#endif
    DOCOMPLETION(pbp, err);
}

PRIVATE void restorecloseanddispose(hiddenh h)
{
#if defined (LINUX) || defined (NEXTSTEP)
#if defined(TERMIO)
    ioctl(HxX(h, fd), TCSETAW, &HxX(h, state));
#else
    ioctl(HxX(h, fd), TIOCSETP, &HxX(h, sgttyb));
    ioctl(HxX(h, fd), TIOCSETC, &HxX(h, tchars));
    ioctl(HxX(h, fd), TIOCLSET, &HxX(h, lclmode));
#endif
    close(HxX(h, fd));
    Uunlink(lockname);
#endif
    DisposHandle((Handle) h);
}

A2(PUBLIC, OSErr, ROMlib_serialclose, ParmBlkPtr, pbp,		/* INTERNAL */
								 DCtlPtr, dcp)
{
    OSErr err;
    hiddenh h;

    if (dcp->dCtlFlags & CWC(OPENBIT)) {
	h = (hiddenh) MR(dcp->dCtlStorage);
	restorecloseanddispose(h);
	dcp->dCtlFlags &= CWC(~OPENBIT);
	err = noErr;
    } else
	err = notOpenErr;
#if defined(SERIALDEBUG)
    warning_trace_info ("serial close returning %d", (LONGINT) err);
#endif
    DOCOMPLETION(pbp, err);
}
#endif /* !defined (MACOSX) */
