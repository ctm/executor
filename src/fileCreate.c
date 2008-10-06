/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_fileCreate[] =
	    "$Id: fileCreate.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in FileMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "FileMgr.h"
#include "OSEvent.h"
#include "VRetraceMgr.h"
#include "OSUtil.h"

#include "rsys/file.h"
#include "rsys/glue.h"
#include "rsys/assert.h"
#include "rsys/filedouble.h"
#include "rsys/suffix_maps.h"

A4(PUBLIC, OSErr, Create, StringPtr, filen, INTEGER, vrn,	/* IMIV-112 */
					      OSType, creator, OSType, filtyp)
{
    ParamBlockRec pbr;
    OSErr temp;
    LONGINT t;

    pbr.fileParam.ioNamePtr = RM(filen);
    pbr.fileParam.ioVRefNum = CW(vrn);
    pbr.fileParam.ioFVersNum = 0;

    temp = PBCreate(&pbr, 0);
    if (temp != noErr)
	return(temp);

    OSASSIGN(pbr.fileParam.ioFlFndrInfo.fdType, CL(filtyp));
    OSASSIGN(pbr.fileParam.ioFlFndrInfo.fdCreator, CL(creator));
    pbr.fileParam.ioFlFndrInfo.fdFlags = 0;
    ZEROPOINT(pbr.fileParam.ioFlFndrInfo.fdLocation);
    pbr.fileParam.ioFlFndrInfo.fdFldr = 0;

    GetDateTime(&t);
    pbr.fileParam.ioFlCrDat = t;
    pbr.fileParam.ioFlMdDat = t;

    temp = PBSetFInfo(&pbr, 0);
/*
 * The dodge below of not returning fnfErr is necessary because people might
 * want to create a file in a directory without a .Rsrc.  This has some
 * unpleasant side effects (notably no file type and no logical eof), but we
 * allow it anyway (for now).
 */
    return temp == fnfErr ? noErr : temp;
}

A2(PUBLIC, OSErr, FSDelete, StringPtr, filen, INTEGER, vrn)	/* IMIV-113 */
{
    ParamBlockRec pbr;

    pbr.fileParam.ioNamePtr = RM(filen);
    pbr.fileParam.ioVRefNum = CW(vrn);
    pbr.fileParam.ioFVersNum = 0;
    return(PBDelete(&pbr, 0));
}

A4(PRIVATE, OSErr, PBCreateForD, ParmBlkPtr, pb, BOOLEAN, a,
						  FOrDType, ford, LONGINT, dir)
{
    char *pathname;
    OSErr err;
    int fd;
    VCBExtra *vcbp;
    LONGINT dirid;
    ParamBlockRec newparam;

    pathname = 0;
    /* Find out where this new entity should reside */
    if ((err = ROMlib_nami(pb, dir, NoIndex, &pathname, (char **)0, (char **)0,
				  TRUE, &vcbp, (struct stat *) 0)) == noErr &&
				      (err = volumenotlocked(vcbp)) == noErr) {
	if (ford == File)
	    fd = Uopen(pathname, O_BINARY|O_CREAT|O_EXCL, 0666L);
	else {
	    fd = Umkdir(pathname, 0777);
	    if (fd >= 0) {
		dirid = 0;
                ROMlib_dbm_store(vcbp, pathname, &dirid, TRUE);
		((HParmBlkPtr) pb)->fileParam.ioDirID = CL(dirid);
	    }
	}

	if (fd < 0) {
	    switch (errno) {
	    case EACCES:
#if defined(EROFS)
	    case EROFS:
#endif /* defined(EROFS) */
		err = wPrErr;
		break;
	    case ENOSPC:
#if defined (EDQUOT)
	    case EDQUOT:
#endif /* EDQUOT */
		err = dirFulErr;
		break;
	    case EEXIST:
		err = dupFNErr;
		break;
	    default:
		err = ioErr;
/* fprintf(stderr, "%s(%d): ioErr returned\n", __FILE__, __LINE__); */
		break;
	    }
	} else {
	    if (ford == File)
		(void) Uclose(fd);
	}
    }
    if (err == noErr)
	ROMlib_rewinddir();
    if (err == noErr) {
        LONGINT swapped_dir;

	TRANSFER_ASSIGN (newparam.fileParam, pb->fileParam);
	memset(&newparam.fileParam.ioFlFndrInfo, 0,
	       sizeof(newparam.fileParam.ioFlFndrInfo));
        memset(&newparam.fileParam.ioFlCrDat, 0,
	       sizeof(newparam.fileParam.ioFlCrDat));
	memset(&newparam.fileParam.ioFlMdDat, 0,
	       sizeof(newparam.fileParam.ioFlMdDat));
	swapped_dir = CL(dir);
	if (!ROMlib_creator_and_type_from_filename (strlen (pathname), 
						    pathname, NULL, NULL))
	  ROMlib_PBGetSetFInfoD(&newparam, FALSE, Set, &swapped_dir, FALSE);
    }
    if (pathname)
	free (pathname);
    return err;
}

A2(PUBLIC, OSErr, ufsPBCreate, ParmBlkPtr, pb, BOOLEAN, a)	/* INTERNAL */
{
    return PBCreateForD(pb, a, File, 0);
}

A2(PUBLIC, OSErr, ufsPBHCreate, HParmBlkPtr, pb,	/* INTERNAL */
						      BOOLEAN, a)
{
    return PBCreateForD((ParmBlkPtr) pb, a, File, Cx(pb->fileParam.ioDirID));
}

A2(PUBLIC, OSErr, ufsPBDirCreate, HParmBlkPtr, pb,	/* INTERNAL */
							BOOLEAN, a)
{
    return PBCreateForD((ParmBlkPtr) pb, a, Directory, Cx(pb->fileParam.ioDirID));
}

A4(PRIVATE, OSErr, PBDeleteForD, ParmBlkPtr, pb, BOOLEAN, a,
						  FOrDType, ford, LONGINT, dir)
{
    char *pathname, *filename, *endname, *rpathname;
    OSErr err;
    int deletefailed;
    VCBExtra *vcbp;
    struct stat sbuf;

    pathname = 0;
    rpathname = 0;
    /* Find out where this new entity should reside */
    if ((err = ROMlib_nami(pb, dir, NoIndex, &pathname, &filename, &endname,
					       TRUE, &vcbp, &sbuf)) == noErr &&
				      (err = volumenotlocked(vcbp)) == noErr) {

	rpathname = ROMlib_resname(pathname, filename, endname);
	deletefailed = Uunlink(rpathname);
	if (netatalk_conventions_p)
	  double_dir_op (pathname, rmdir_op);
	if (!deletefailed || errno == ENOTDIR || errno == ENOENT)
	    deletefailed = Uunlink(pathname);
	if (deletefailed && ford == Directory) {
	  struct stat sbuf;

	  if (Ustat (pathname, &sbuf) != 0)
	    deletefailed = TRUE;
	  else
	    {
	      if ((deletefailed = Urmdir(pathname)) == 0)
		ROMlib_dbm_delete_inode (vcbp, ST_INO (sbuf));
	    }
	}
	if (deletefailed)
	    switch (errno) {
	    case ENOTDIR:
	        warning_trace_info ("pathname = '%s', rpathname = '%s'",
				    pathname, rpathname);
		err = dirNFErr;
		break;

#if defined(EBUSY)
	    case EBUSY:
#endif /* defined(EBUSY) */
#if defined (ENOTEMPTY)
	    case ENOTEMPTY:
#endif /* ENOTEMPTY */
#if defined(EFAULT)
	    case EFAULT:
#endif /* defined(EFAULT) */
		err = fBsyErr;
		break;

	    case EINVAL:
#if defined (ENAMETOOLONG)
	    case ENAMETOOLONG:
#endif /* ENAMETOOLONG */
#if defined (ELOOP)
	    case ELOOP:
#endif /* ELOOP */
		err = bdNamErr;
		break;

	    case ENOENT:
		err = fnfErr;
		break;

	    case EPERM:
		err = fLckdErr;
		break;

	    case EIO:
		err = ioErr;
/* fprintf(stderr, "%s(%d): ioErr returned\n", __FILE__, __LINE__); */
		break;

#if (EACCES != EPERM)
	    case EACCES:
#if defined (CYGWIN32)
	      err = fBsyErr;
	      break;
#endif
#endif /* (EACCESS != EPERM) */
#if defined(EROFS)
	    case EROFS:
#endif /* defined(EROFS) */
		err = wPrErr;
		break;
	    case ENOSPC:
#if defined (EDQUOT)
	    case EDQUOT:
#endif /* EDQUOT */
		err = dirFulErr;
		break;
	    case EEXIST:
		err = dupFNErr;
		break;
	    default:
		err = ioErr;
/* fprintf(stderr, "%s(%d): ioErr returned\n", __FILE__, __LINE__); */
		break;
	    }
    }
    if (pathname)
	free (pathname);
    if (rpathname)
	free (rpathname);
    return err;
}

A2(PUBLIC, OSErr, ufsPBDelete, ParmBlkPtr, pb, BOOLEAN, a)/* INTERNAL */
{
    return PBDeleteForD(pb, a, File, 0);
}

A2(PUBLIC, OSErr, ufsPBHDelete, HParmBlkPtr, pb,	/* INTERNAL */
						      BOOLEAN, a)
{
    return PBDeleteForD((ParmBlkPtr) pb, a, Directory, Cx(pb->fileParam.ioDirID));
}
