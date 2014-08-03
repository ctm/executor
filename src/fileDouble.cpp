/* Copyright 1992-2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_fileDouble[] =
	    "$Id: fileDouble.c 86 2005-05-25 00:47:12Z ctm $";
#endif

#include "rsys/common.h"
#include "FileMgr.h"
#include "rsys/file.h"

#if defined (CYGWIN32)
#include "winfs.h"
#endif

int Executor::afpd_conventions_p;
int Executor::netatalk_conventions_p;

char Executor::apple_double_quote_char;
const char *Executor::apple_double_fork_prefix;
int Executor::apple_double_fork_prefix_length;

using namespace Executor;

/*
 * Coded more or less up to the spec:  APDA M0908LL/A
 *				       AppleSingle/AppleDouble Formats
 *				       For Foreign Files Developer's Note
 *
 * I don't like this spec for a variety of reasons:
 *
 *	The idea that you should ever have to look inside a file to determine
 *	whether %father is really 0xfather or the header file for "father"
 *	is messed.  The way we get around this is we always quote the first
 *	if that's really what they want, so the header file for "father"
 *	will be %%66ather (actually we only do that if the two characters
 *	are hex for something that we'd expand normally (i.e. '%', '/', NUL
 *	or something with the high bit on, like 0xFA.
 *
 *	The spec says you can put the descriptors in any order you want, but
 *	it would be much nicer if they had to be sorted by offset.  This is
 *	what we do/assume.
 */

#include "rsys/filedouble.h"
#include "rsys/suffix_maps.h"
#include "rsys/osutil.h"

PRIVATE defaulthead_t ourdefault
#if !defined(__alpha)
= {
    {
	CLC(DOUBLEMAGIC),
	CLC(SINGLEVERSION),
	{ 0L, 0L, 0L, 0L, },
	CWC(4),
    },
    {
#define LEN0	(sizeof(struct defaulthead))
	{ CLC(Macintosh_File_Info_ID), CLC(LEN0), CLC(sizeof(Single_attribs)) },
#define LEN1	(LEN0 + sizeof(Single_attribs))
	{ CLC(File_Dates_Info_ID),     CLC(LEN1), CLC(sizeof(Single_dates)) },
#define LEN2	(LEN1 + sizeof(Single_dates))
	{ CLC(Finder_Info_ID),         CLC(LEN2), CLC(sizeof(Single_finfo)) },
	{ CLC(Resource_Fork_ID),       CLC(512), 0L },
    }
};

#define initialize_ourdefault()

#else /* defined(__alpha) */
;

/*
 * compiler bug forces us to do this
 */

#define LEN0	(sizeof(struct defaulthead))
#define LEN1	(LEN0 + sizeof(Single_attribs))
#define LEN2	(LEN1 + sizeof(Single_dates))

PRIVATE void initialize_ourdefault( void )
{
    static int beenhere = 0;

    if (!beenhere) {
	beenhere = 1;
	ourdefault.head.magic    = CLC(DOUBLEMAGIC);
	ourdefault.head.version  = CLC(SINGLEVERSION);
	ourdefault.head.nentries = CWC(4);

	ourdefault.desc[0].id     = CLC(Macintosh_File_Info_ID);
	ourdefault.desc[0].offset = CLC(LEN0);
	ourdefault.desc[0].length = CLC(sizeof(Single_attribs));

	ourdefault.desc[1].id     = CLC(File_Dates_Info_ID);
	ourdefault.desc[1].offset = CLC(LEN1);
	ourdefault.desc[1].length = CLC(sizeof(Single_dates));

	ourdefault.desc[2].id     = CLC(Finder_Info_ID);
	ourdefault.desc[2].offset = CLC(LEN2);
	ourdefault.desc[2].length = CLC(sizeof(Single_finfo));

	ourdefault.desc[3].id     = CLC(Resource_Fork_ID);
	ourdefault.desc[3].offset = CLC(512);
	ourdefault.desc[3].length = CLC(0L);
    }
}
#endif /* defined(__alpha) */

PRIVATE defaultentries_t ourentries;

PUBLIC void
Executor::double_dir_op (char *name, double_dir_op_t op)
{
  char *last_slash;
  
  last_slash = strrchr (name, '/');
  if (last_slash)
    {
      char save;
      
      save = *last_slash;
      *last_slash = 0;
      switch (op)
	{
	case mkdir_op:
	  Umkdir (name, 0777);
	  break;
	case rmdir_op:
	  {
	    char *to_del, *to_unlink;
	    int len;

#define PARENT_FILE ".Parent"

	    *last_slash = save;
	    len = strlen (name) + 1 + apple_double_fork_prefix_length + 1;
	    to_del = (char*)alloca (len);
	    sprintf (to_del, "%s/%s", name, apple_double_fork_prefix);
	    to_unlink = (char*)alloca (len + strlen(PARENT_FILE));
	    sprintf (to_unlink, "%s%s", to_del, PARENT_FILE);
	    unlink (to_unlink);
	    to_del[len-2] = 0; /* remove trailing slash */
	    Urmdir (to_del);
	  }
	  break;
	default:
	  warning_unexpected ("op = %d", op);
	  break;
	}
      *last_slash = save;
    }
}

A3(PUBLIC, OSErr, ROMlib_newresfork, char *, name, LONGINT *, fdp,
   boolean_t, unix_p)
{
    LONGINT fd;
    OSErr retval;


    if (netatalk_conventions_p)
      double_dir_op (name, mkdir_op);
    if (unix_p)
      {
	ourentries.finfo.finfo.fdType = TICKX("TEXT");
	ourentries.finfo.finfo.fdCreator = TICKX("UNIX");
      }
    else
      {
	ourentries.finfo.finfo.fdType = 0;
	ourentries.finfo.finfo.fdCreator = 0;
      }
    initialize_ourdefault();
    if ((fd = Uopen(name, (O_BINARY|O_RDWR|O_CREAT), 0666L)) < 0 ||
	     write(fd, (char *) &ourdefault, sizeof(ourdefault))
						       != sizeof(ourdefault) ||
	     write(fd, (char *) &ourentries, sizeof(ourentries))
						       != sizeof(ourentries)) {
	retval = ROMlib_maperrno();
	Uclose(fd);
    } else {
	retval = noErr;
	*fdp = fd;
    }
    fs_err_hook (retval);
    return retval;
}

/*
 * Modifications to hiddenbyname:
 *
 * haven't thought it through yet.
 */

#define OURBSIZE	512

namespace Executor {
  PRIVATE BOOLEAN getsetentry(GetOrSetType gors, LONGINT fd,
	 Single_ID sid, Single_descriptor * savesdp,
							  ULONGINT * lengthp);
  PRIVATE void writebyteat(LONGINT fd, LONGINT loc);
  PRIVATE BOOLEAN getsetpiece(GetOrSetType gors, LONGINT fd,
							  Single_descriptor *sdp, char *bufp, LONGINT length);

}

A5(PRIVATE, BOOLEAN, getsetentry, GetOrSetType, gors, LONGINT, fd,
   Single_ID, sid, Single_descriptor *, savesdp,
   ULONGINT *, lengthp)
{
    off_t saveloc;
    struct defaulthead *dfp;
    INTEGER nread;
    Single_descriptor *sdp;
    char buf[OURBSIZE];
    BOOLEAN retval;
    INTEGER n;

    retval = FALSE;
    saveloc = lseek(fd, 0L, L_INCR);
    lseek(fd, 0L, L_SET);
    nread = read(fd, buf, sizeof(buf));
    dfp = (struct defaulthead *) buf;
    if (nread >= (int) sizeof(struct defaulthead)
	&& (dfp->head.magic == CLC(SINGLEMAGIC)
	    || dfp->head.magic == CLC(DOUBLEMAGIC)
	    || dfp->head.magic == SINGLEMAGIC
	    || dfp->head.magic == DOUBLEMAGIC)) {
	n = MIN(Cx(dfp->head.nentries),
		((nread - (int) sizeof(Single_header)) /
		 (int) sizeof(Single_descriptor)));
	for (sdp = dfp->desc; --n >= 0 && Cx(sdp->id) != sid; ++sdp)
	    ;
	if (n >= 0) {
	    switch (gors) {
	    case Get:
		*savesdp = *sdp;
		if (n == 0) {
		    if (lengthp)
			*lengthp = 0x7FFFFFFF;	/* unlimited */
		} else
		    if (lengthp)
			*lengthp = CL(sdp[1].offset) - CL(sdp[0].offset);
		break;
	    case Set:
		*sdp = *savesdp;
		lseek(fd, 0L, L_SET);
		write(fd, buf, nread);
		break;
	    default:
		gui_assert(0);
		break;
	    }
	    retval = TRUE;
	}
    }
    lseek(fd, saveloc, L_SET);
    return retval;
}

#define IDWANTED(fp) ((fp->fcflags & fcfisres) ? Resource_Fork_ID : \
								  Data_Fork_ID)

A1(PUBLIC, LONGINT, ROMlib_FORKOFFSET, fcbrec *, fp)	/* INTERNAL */
{
    Single_descriptor d;
    Single_ID idwanted;

    if (fp->fcfd != fp->hiddenfd)
/*-->*/	return 0L;
    idwanted = IDWANTED(fp);
    if (getsetentry(Get, fp->fcfd, IDWANTED(fp), &d, NULL))
	return CL(d.offset);
    else
	return RESOURCEPREAMBLE;
}

A2(PRIVATE, void, writebyteat, LONGINT, fd, LONGINT, loc)
{
    off_t saveloc;

    saveloc = lseek(fd, 0L, L_INCR);
    lseek(fd, loc, L_SET);
    write(fd, "", 1);
    lseek(fd, saveloc, L_SET);
}

/* TODO: better error checking */

A1(PUBLIC, OSErr, ROMlib_seteof, fcbrec *, fp)	/* INTERNAL */
{
    ULONGINT leof, peof;
  off_t curloc;
    LONGINT fd;
    Single_descriptor d;
    Single_ID idwanted;
    INTEGER i;
    OSErr err;

    fd = fp->fcfd;
    leof = Cx(fp->fcleof);
    err = noErr;
    if (fd == fp->hiddenfd) {	/* mixed file */
	idwanted = IDWANTED(fp);
	if (getsetentry(Get, fd, idwanted, &d, &peof)) {
	    if (leof > peof) {
		gui_assert(0);	/* TODO: We need to move stuff around.
			 	   There are many things we could do */
	    } else if (leof < peof) {
		d.length = CL(leof);
		getsetentry(Set, fd, idwanted, &d, NULL);
	    }
	} else
	  {
	    err = fsDSIntErr;
	    warning_unexpected ("getsetentry (Get, %d, %d) failed", fd,
				idwanted);
	  }
	
    } else {			/* pure file */
	peof = Cx(fp->fcPLen);
	if (leof > peof)
	    writebyteat(fd, leof-1);
	else if (leof < peof) {
	    curloc = lseek(fd, 0L, L_INCR);
	    ftruncate(fd, leof);
	    if (curloc > leof)
		lseek(fd, leof, L_SET);
	}
	peof = leof;
    }
    if (err == noErr) {
	for (i = 0; i < NFCB; i++) {
	    if (ROMlib_fcblocks[i].fdfnum == fp->fdfnum &&
				     ROMlib_fcblocks[i].fcvptr == fp->fcvptr &&
				     (ROMlib_fcblocks[i].fcflags & fcfisres) ==
						    (fp->fcflags & fcfisres)) {
		ROMlib_fcblocks[i].fcleof  = CL(leof);
		ROMlib_fcblocks[i].fcPLen = CL(peof);
	    }
	}
    }
    fs_err_hook (err);
    return err;
}

A5(PRIVATE, BOOLEAN, getsetpiece, GetOrSetType, gors, LONGINT, fd,
		       Single_descriptor *, sdp, char *, bufp, LONGINT, length)
{
    off_t saveloc;
    BOOLEAN retval;

    saveloc = lseek(fd, 0L, L_INCR);
    lseek(fd, Cx(sdp->offset), L_SET);
    switch (gors) {
    case Get:
	retval =  read(fd, bufp, length) == length;
	break;
    case Set:
	retval = write(fd, bufp, length) == length;
	break;
    default:
	retval = FALSE;
	gui_assert(0);
	break;
    }
    lseek(fd, saveloc, L_SET);
    return retval;
}

A1(PUBLIC, OSErr, ROMlib_geteofostype, fcbrec *, fp)	/* INTERNAL */
{
    LONGINT fd;
    Single_descriptor d;
    Single_ID idwanted;
    Single_finfo finfo;
    OSErr err;
    struct stat sbuf;

    fd = fp->hiddenfd;
    if (fstat(fp->fcfd, &sbuf) < 0)
	err = ROMlib_maperrno();
    else {
	err = noErr;
	if (fd == fp->fcfd) {	/* mixed file */
	    idwanted = IDWANTED(fp);
	    if (!getsetentry(Get, fd, idwanted, &d, NULL))
		err = fp->fcPLen = 0;
	    else
		fp->fcPLen = fp->fcleof = d.length;
	} else
	    fp->fcleof = fp->fcPLen = CL((int)sbuf.st_size);
	if (err == noErr) {
	    if (!getsetentry(Get, fd, Finder_Info_ID, &d, NULL) ||
		   (!getsetpiece(Get, fd, &d, (char *) &finfo, sizeof(finfo))))
	      {
		uint32 type;

		if (ROMlib_creator_and_type_from_filename
		    (fp->fcname[0], (char *) fp->fcname+1, NULL, &type))
		  fp->fcbFType = CL (type);
		else
		  fp->fcbFType = TICKX("TEXT");
	      }
	    else
		fp->fcbFType = finfo.finfo.fdType;
	}
    }
    fs_err_hook (err);
    return err;
}

A8(PUBLIC, OSErr,  ROMlib_hiddenbyname, GetOrSetType, gors,	/* INTERNAL */
		   char *, pathname, char *, rpathname, Single_dates *, datep,
		   FInfo *, finfop, FXInfo *, fxinfop, LONGINT *, lenp,
							      LONGINT *, rlenp)
{
    LONGINT rfd;
    struct stat sbuf;
    Single_finfo sfinfo;
    Single_descriptor d;
    OSErr retval;
    BOOLEAN done;

    retval = noErr;
    if (Ustat(pathname, &sbuf) < 0)
	retval = ROMlib_maperrno();
    else {
	done = FALSE;
	rfd = Uopen(rpathname, O_BINARY|(gors == Set ? O_RDWR : O_RDONLY), 0);
/*
if (rfd == -1)
fprintf(stderr, "%s(%d): open '%s' fails\n", __FILE__, __LINE__, rpathname);
*/
	if (rfd < 0) {
	    done = TRUE;
	    if (errno == ENOENT) {
		/* no resource fork (or AppleSingle) */
		/* right now we ignore that it could be AppleSingle */
		switch (gors) {
		case Get:
		    memset(datep,   0, sizeof(*datep));
		    memset(finfop,  0, sizeof(*finfop));
		    memset(fxinfop, 0, sizeof(*fxinfop));
		    *lenp = CL((int)sbuf.st_size);
		    *rlenp = 0;
		    break;
		case Set:
		    retval = ROMlib_newresfork(rpathname, &rfd, FALSE);
		    done = FALSE;
		    break;
		default:
		    gui_assert(0);
		    break;
		}
	    } else
		retval = ROMlib_maperrno();
	}
	if (!done && retval == noErr) {
	    switch (gors) {
	    case Get:
		if (getsetentry(Get, rfd, File_Dates_Info_ID, &d, NULL))
		  getsetpiece(Get, rfd, &d, (char *) datep, sizeof(*datep));
		else
		  {
		    datep->crdat = CL (UNIXTIMETOMACTIME (MIN (sbuf.st_ctime, sbuf.st_mtime)));
		    datep->moddat = CL (UNIXTIMETOMACTIME (sbuf.st_mtime));
		    datep->accessdat = CL (UNIXTIMETOMACTIME (sbuf.st_atime));
		    datep->backupdat = 0;
		  }

		if (!getsetentry(Get, rfd, Finder_Info_ID,   &d, NULL))
		  warning_unexpected ("no finder info");
		else
		  {
		    getsetpiece(Get, rfd, &d, (char *) &sfinfo, sizeof(sfinfo));

		    if (finfop)
		      *finfop  = sfinfo.finfo;
		    if (fxinfop)
		      *fxinfop = sfinfo.fxinfo;

		    *lenp  = CL((int)sbuf.st_size);
		  }
		if (getsetentry(Get, rfd, Resource_Fork_ID,   &d, NULL))
		    *rlenp = d.length;
		else
		    *rlenp = 0;
		break;
	    case Set:
		if (getsetentry(Get, rfd, File_Dates_Info_ID, &d, NULL))
		  getsetpiece(Set, rfd, &d, (char *) datep, sizeof(*datep));

		if (!getsetentry(Get, rfd, Finder_Info_ID,   &d, NULL))
		  warning_unexpected ("no finfo");
		else
		  {
		    if (!finfop || !fxinfop) {
		      sfinfo.finfo.fdCreator = TICKX("UNIX");
		      sfinfo.finfo.fdType    = TICKX("TEXT");
		      getsetpiece(Get, rfd, &d, (char *) &sfinfo,
							       sizeof(sfinfo));
		    }

		    if (finfop)
		      sfinfo.finfo  = *finfop;
		    if (fxinfop)
		      sfinfo.fxinfo = *fxinfop;
		    getsetpiece(Set, rfd, &d, (char *) &sfinfo, sizeof(sfinfo));
		  }
		break;
	    default:
		gui_assert(0);
		break;
	    }
	}
	Uclose(rfd);
    }
    fs_err_hook (retval);
    return retval;
}
