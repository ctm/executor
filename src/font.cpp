/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_font[] =
		    "$Id: font.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in FontMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ResourceMgr.h"
#include "CQuickDraw.h"
#include "QuickDraw.h"
#include "MemoryMgr.h"
#include "FontMgr.h"
#include "ToolboxUtil.h"
#include "OSUtil.h"
#include "rsys/cquick.h"
#include "rsys/font.h"
#include "rsys/glue.h"

using namespace Executor;

#define MAXTABLES	12

PRIVATE void
reset_myfmi (void)
{
  ROMlib_myfmi.family = -1;
  ROMlib_myfmi.size = -1;
  ROMlib_myfmi.face = -1;
}

P0(PUBLIC pascal trap, void, InitFonts)	/* IMI-222 */
{
    static BOOLEAN beenhere = FALSE;
    GUEST<THz> saveZone;

    if (!beenhere) {
	saveZone = TheZone;
	TheZone = SysZone;
	SetResLoad(TRUE);
	ROMFont0 = RM(GetResource(TICK("FONT"), FONTRESID(systemFont, 12)));
	WidthListHand = RM(NewHandle(MAXTABLES * sizeof(GUEST<Handle>)));
	memset(STARH(MR(WidthListHand)), 0, MAXTABLES * sizeof(GUEST<Handle>));
	TheZone = saveZone;
	beenhere = TRUE;
    }
    ApFontID = CW(Cx(SPFont) + 1);
    SysFontSiz = CWC(12);
    SysFontFam = 0;
#if 0
    /* 95-09-20: PAUP uses a built in font that has fried width tables.
       The bad tables won't hurt us if FractEnable is not set, so I ran
       a test program on the Mac and found that FractEnable wasn't set
       at the beginning of the program or even after InitFonts was called.
       This suggests that the hack for WordPerfect is wrong.  Perhaps
       there's some bit in the Size resource that controls this thing,
       perhpas the WordPerfect hack was always fried.  Yahoo. */

    FractEnable = 0xff;	/* new mod for WordPerfect */
#endif
    FScaleDisable = 0;
    reset_myfmi ();
}

PRIVATE void
invalidate_all_widths (void)
{
  int i, n_entries;
  GUEST<Handle> *hp;
  Handle wlh;

  wlh = MR (WidthListHand);
  HLock (wlh);
  n_entries = GetHandleSize (wlh) / sizeof (GUEST<Handle>);
  hp = (GUEST<Handle> *) STARH (wlh);
  for (i = 0; i < n_entries; ++i)
    {
      DisposHandle (MR (hp[i]));
      hp[i] = nullptr;
    }
  HUnlock (wlh);
}

PUBLIC void
Executor::ROMlib_shutdown_font_manager (void)
{
  invalidate_all_widths ();
}

P2(PUBLIC pascal trap, void, GetFontName, INTEGER, fnum,	/* IMI-223 */
							     StringPtr, fnam)
{
    Handle h;
    GUEST<INTEGER> i;
    GUEST<ResType> rest;
    
    if (fnum == systemFont)
	fnum = CW(SysFontFam);
    else if (fnum == applFont)
	fnum = CW(ApFontID);
    SetResLoad(FALSE);
    h = GetResource(TICK("FONT"), FONTRESID(fnum, 0));
    if (!h)
	h = GetResource(TICK("NFNT"), FONTRESID(fnum, 0));
    if (!h)
	h = GetResource(TICK("FOND"), fnum);
    GetResInfo(h, &i, &rest, fnam);
    if (ResError())
	*fnam = 0;
    SetResLoad(TRUE);
}

/*
 * callable w/o pascal conventions
 */

A2(PUBLIC, void, ROMlib_GetFontName, LONGINT, fnum, char *, fnam)
{
    GetFontName(fnum, (StringPtr) fnam);
}

P2(PUBLIC pascal trap, void, GetFNum, StringPtr, fnam,		/* IMI-223 */
						       GUEST<INTEGER> *, fnum)
{
    Handle h;
    GUEST<ResType> rest;
    BOOLEAN shift;
    
    SetResLoad(FALSE);

    h = GetNamedResource(TICK("FOND"), fnam);
    if (h)
      shift = FALSE;
    else
      {
	shift = TRUE;
	h = GetNamedResource(TICK("FONT"), fnam);
	if (!h)
	  h = GetNamedResource(TICK("NFNT"), fnam);
      }

    GetResInfo(h, fnum, &rest, NULL);
    if (ResError())
	*fnum = 0;
    else if (shift)
	*fnum =CW( CW(*(GUEST<uint16_t> *) fnum) >> 7);
    SetResLoad(TRUE);
}

P1(PUBLIC pascal trap, void, SetFontLock, BOOLEAN, lflag)	/* IMI-223 */
{
    INTEGER attrs;
    
    attrs = GetResAttrs(MR(ROMlib_fmo.fontHandle));
    if (lflag) {
	LoadResource(MR(ROMlib_fmo.fontHandle));
	SetResAttrs(MR(ROMlib_fmo.fontHandle), attrs & ~resPurgeable);
    } else {	/* TODO: is the next line necessary */
	SetResAttrs(MR(ROMlib_fmo.fontHandle), attrs | resPurgeable);
    }
}

typedef char ctrip[3];
typedef struct {
    INTEGER dpiv, dpih;
    ctrip boldt, italt, nat, outt, shadt, condt, extt, undert;
} fchartstr;

PRIVATE fchartstr ftstr = {
    80, 80,		 /* dpiv, dpih */
    { 0,  1,  1, },	/* boldt */
    { 1,  8,  0, },	/* italt */
    { 0,  0,  0, },	/* Not Used */
    { 5,  1,  1, },	/* outt */
    { 5,  2,  2, },	/* shadt */
    { 0,  0, -1, },	/* condt */
    { 0,  0,  1, },	/* extt */
    { 1,  1,  1, },	/* undert */
};

namespace Executor {
  PRIVATE void mungfmo(ctrip,FMOutput*);
  PRIVATE BOOLEAN widthlistmatch(FMInput *);
  PRIVATE int countones(unsigned short);
  PRIVATE GUEST<INTEGER> *findfondwidths();
  PRIVATE void buildtable(INTEGER);
  PRIVATE void findclosestfont(INTEGER family, INTEGER size,
							   INTEGER *lesserp, INTEGER *greaterp);
  PRIVATE void findclosestfond(FHandle fh, INTEGER size, INTEGER *powerof2p,
							   INTEGER * lesserp, INTEGER * greaterp);
  PRIVATE INTEGER closestface();
  PRIVATE void newwidthtable(FMInput *fmip);
}

A2(PRIVATE, void, mungfmo, ctrip, cp, FMOutput *, fmop)
{
    Byte *p;
    INTEGER i;
    
    p = &(fmop->bold);
    i = cp[1];
    if (i & 0x80)
	i |= 0xFF00;
    *(p + cp[0]) += i;
    i = cp[2];
    if (i & 0x80)
	i |= 0xFF00;
    fmop->extra = CB(CB(fmop->extra) + i);
}

#define MAXTABLES	12

A1(PRIVATE, BOOLEAN, widthlistmatch, FMInput *, fmip)
{
    
    GUEST<WHandle> *whp, *ewhp;

    for (whp = (GUEST<WHandle> *) STARH(WIDTHLISTHAND), ewhp = whp + MAXTABLES; whp != ewhp; whp++) {
	if (*whp && (LONGINT) (*whp).raw() != (LONGINT) -1) {
	    WidthPtr = *(MR(*whp));
	    if (WIDTHPTR->aFID  == fmip->family &&
				  WIDTHPTR->aSize     == fmip->size       &&
			   (char) WIDTHPTR->aFace     == (char) fmip->face &&
				  WIDTHPTR->device    == fmip->device     &&
				  WIDTHPTR->inNumer.h == fmip->numer.h    &&
				  WIDTHPTR->inNumer.v == fmip->numer.v    &&
				  WIDTHPTR->inDenom.h == fmip->denom.h    &&
				  WIDTHPTR->inDenom.v == fmip->denom.v	  &&
				  WIDTHPTR->fSize     != CWC(-1) &&
				  WIDTHPTR->fSize     !=  CWC(0) &&
				 !WIDTHPTR->usedFam   == !FractEnable) {
		WidthTabHandle = guest_cast<WidthTableHandle>(*whp);
		HLock((Handle) MR(WidthTabHandle));
		return TRUE;
	    }
	}
    }
    return FALSE;
}

A1(PRIVATE, int, countones, unsigned short, i)
{
    int retval;
    static int counts[] = {
	0,	/* 0 0 0 0 */
	1,	/* 0 0 0 1 */
	1,	/* 0 0 1 0 */
	2,	/* 0 0 1 1 */
	1,	/* 0 1 0 0 */
	2,	/* 0 1 0 1 */
	2,	/* 0 1 1 0 */
	3,	/* 0 1 1 1 */
	1,	/* 1 0 0 0 */
	2,	/* 1 0 0 1 */
	2,	/* 1 0 1 0 */
	3,	/* 1 0 1 1 */
	2,	/* 1 1 0 0 */
	3,	/* 1 1 0 1 */
	3,	/* 1 1 1 0 */
	4,	/* 1 1 1 1 */
    };

    retval   = counts[i&0xF];
    i >>= 4;
    retval  += counts[i&0xF];
    i >>= 4;
    retval  += counts[i&0xF];
    i >>= 4;
    return retval + counts[i&0xF];
}

static INTEGER nhappybits(unsigned short want, unsigned short have)
{
    return (have & ~want) ? -1 : countones(have);
}

#define WIDTHBIT	(1 << 1)

A0(PRIVATE, GUEST<INTEGER> *, findfondwidths)
{
    GUEST<INTEGER> *retval, *numentriesminusone;
    LONGINT offset;
    INTEGER bitsmatched, newbits;
    INTEGER want;
    INTEGER tabsize;
    widentry_t *widp;
    INTEGER i;

    retval = 0;
    if (STARH((FHandle) MR(LastFOND))->ffFamID == WIDTHPTR->aFID &&
		    (offset = CL(STARH((FHandle)MR(LastFOND))->ffWTabOff))) {
	bitsmatched = -1;
	want = Cx(WIDTHPTR->aFace);
/*
 * NOTE: we add 3 to lastchar - firstchar to include the missing character
 *	 entry and what appears to be a zero entry located thereafter.
 */
	tabsize = (CW(STARH((FHandle)MR(LastFOND))->ffLastChar) -
		   CW(STARH((FHandle)MR(LastFOND))->ffFirstChar) + 3)
					   * sizeof(INTEGER) + sizeof(INTEGER);
	numentriesminusone = (GUEST<INTEGER> *)
		    ((char *) &STARH((FHandle)MR(LastFOND))->ffFlags + offset);
	i =Cx( *numentriesminusone) + 1;
	widp = (widentry_t *) (numentriesminusone + 1);
	for (; --i >= 0; widp = (widentry_t *) ((char *) widp + tabsize)) {
	    newbits = nhappybits(want, Cx(widp->style));
	    if (newbits > bitsmatched) {
		retval = widp->table;
		bitsmatched = newbits;
	    }
	}
    }

    return retval;
}

#define FIXED(n)     ((LONGINT) (n) << 16)
#define FIXED8(n)    ((LONGINT) ((unsigned short) n) << 8)
#define FIXED4(n)   (((LONGINT) ((unsigned short) n) << 4) * (Cx(WIDTHPTR->aSize)))

PUBLIC Fixed
Executor::font_width_expand (Fixed width, Fixed fixed_extra, Fixed hOutputInverse)
{
  Fixed retval;

  retval = FixMul(width + fixed_extra, hOutputInverse);
  return retval;
}

typedef enum { FontFract, FontInt, FondFract } howtobuild_t;

PRIVATE void buildtabdata(howtobuild_t howtobuild, INTEGER extra,
						       GUEST<INTEGER> *fondwidthtable)
{
    INTEGER c, firstchar, lastchar;
    GUEST<INTEGER> *widp, width;
    GUEST<Fixed> *p, *ep;
    Fixed misswidth, hOutputInverse, fixed_extra;
    FontRec *fp;
    
    fp = MR( *(FontRec **)MR(WIDTHPTR->tabFont));
    firstchar = Cx(fp->firstChar);
    lastchar  = Cx(fp->lastChar);

    switch (howtobuild) {
    case FontFract:
	WIDTHPTR->usedFam = 0;	/* don't really know what this should be */
	widp = &fp->owTLoc + Cx(fp->owTLoc) + (lastchar - firstchar + 2);
	fixed_extra = FIXED(extra);
	misswidth = FIXED8(CW(widp[lastchar - firstchar + 1])) + fixed_extra;

/* NOTE: this assumes missing characters have the missing width in the
	 table.  If this is wrong we need to look for missing characters
	 explictly */
	 
	for (c = 0, p = WIDTHPTR->tabData, ep = p + 256;  p < ep; c++) {
	    if (c < firstchar || c > lastchar)
		*p++ = CL(misswidth);
	    else
		*p++ = CL(FIXED8(CW(*widp++)) + fixed_extra);
	}
	break;
    case FontInt:
	WIDTHPTR->usedFam = 0;
	widp      = &fp->owTLoc + Cx(fp->owTLoc);
	misswidth = FIXED((CW(widp[lastchar - firstchar + 1]) & 0xFF) + extra);
	for (c = 0, p = WIDTHPTR->tabData, ep = p + 256;  p < ep; c++) {
	    if (c < firstchar || c > lastchar)
		*p++ = CL(misswidth);
	    else {
		*p++ = (width = *widp++) == CWC(-1) ?
				     CL(misswidth)
				 :
				     CL(FIXED((CW(width) & 0xFF) + extra));
	    }
	}
	break;
    case FondFract:
	WIDTHPTR->usedFam = -1;
	hOutputInverse = FixRatio(1<<8, Cx(WIDTHPTR->hOutput));
	widp = fondwidthtable;
	fixed_extra = FIXED(extra);
	misswidth = FIXED4 (CW (widp[lastchar - firstchar + 1]));
	misswidth = font_width_expand (misswidth, fixed_extra, hOutputInverse);
	for (c = 0, p = WIDTHPTR->tabData, ep = p + 256;  p < ep; c++) {
	    if (c < firstchar || c > lastchar)
		*p++ = CL(misswidth);
	    else {
		*p++ = CL(FixMul(FIXED4(CW(*widp)) + fixed_extra,
							      hOutputInverse));
		widp++;
	    }
	}
	break;
    }
    WIDTHPTR->tabData[' '] = CL(Cx(WIDTHPTR->tabData[' ']) +
				Cx(WIDTHPTR->sExtra));
}

A1(PRIVATE, void, buildtable, INTEGER, extra)
{
    howtobuild_t howtobuild;
    GUEST<INTEGER> *fondwidthtable;
    Fixed tempfix;
    INTEGER numerh, numerv, denomh, denomv;

#if !defined(LETGCCWAIL)
    fondwidthtable = 0;
#endif
    if (!FractEnable)
	howtobuild = FontInt;
    else if (MR(*(FontRec **)MR(WIDTHPTR->tabFont))->fontType & CWC(WIDTHBIT))
      {
	howtobuild = FontFract;
	extra = 0;
      }
/*
 * NOTE: The sleazy hack for checking for geneva below.  This is because we
 *	 used to use a System file that didn't have widths for geneva and
 *	 everything worked fine.  Now we have widths for geneva, but they
 *	 appear to be too wide.  I don't have time to figure out what's
 *	 going on here, but since geneva is a toy font, I'm not too concerned
 *	 right now.  The trouble that the new sizes cause is in the Word 5.1
 *	 ribbon control labels.
 */
    else if (WIDTHPTR->aFID != CWC(geneva) &&
					   (fondwidthtable = findfondwidths()))
      {
	howtobuild = FondFract;
	extra = 0;
      }
    else
	howtobuild = FontInt;
    numerh = Cx(WIDTHPTR->inNumer.h);
    numerv = Cx(WIDTHPTR->inNumer.v);
    denomh = Cx(WIDTHPTR->inDenom.h);
    denomv = Cx(WIDTHPTR->inDenom.v);
    if (WIDTHPTR->fSize == WIDTHPTR->aSize) {
	WIDTHPTR->hOutput = CW(FixRatio(numerh, denomh) >> 8);
	WIDTHPTR->vOutput = CW(FixRatio(numerv, denomv) >> 8);
	WIDTHPTR->hFactor =
	WIDTHPTR->vFactor = CWC(256);
    } else if (Cx(WIDTHPTR->fSize) < Cx(WIDTHPTR->aSize) && FScaleDisable) {
	WIDTHPTR->hOutput = CW(FixRatio(numerh, denomh) >> 8);
	WIDTHPTR->vOutput = CW(FixRatio(numerv, denomv) >> 8);
	WIDTHPTR->hFactor =
	WIDTHPTR->vFactor = CW(FixRatio(Cx(WIDTHPTR->aSize),
						    Cx(WIDTHPTR->fSize)) >> 8);
    } else {
	tempfix	   = FixRatio(Cx(WIDTHPTR->aSize), Cx(WIDTHPTR->fSize));
	WIDTHPTR->hOutput = CW(FixMul(tempfix, FixRatio(numerh, denomh)) >> 8);
	WIDTHPTR->vOutput = CW(FixMul(tempfix, FixRatio(numerv, denomv)) >> 8);
	WIDTHPTR->hFactor =
	WIDTHPTR->vFactor = CWC(256);
    }
#if 0	/* WTF is going on here? */
    WIDTHPTR->style = CW(extra);
#endif

    buildtabdata(howtobuild, extra, fondwidthtable);
}

A4(PRIVATE, void, findclosestfont, INTEGER, family, INTEGER, size,
				       INTEGER *, lesserp, INTEGER *, greaterp)
{
    INTEGER i, lesser, greater, newsize, nres;
    GUEST<ResType> rest;
    GUEST<INTEGER> id_s;
    Handle h;

    lesser = 0;
    greater = 32767;
    SetResLoad(FALSE);
    nres = CountResources(TICK("FONT"));	/* how about NFNT? */
    for (i = 1; i <= nres; i++) {
	h = GetIndResource(TICK("FONT"), i);
        GetResInfo(h, &id_s, &rest, (StringPtr) 0);
        INTEGER id = id_s.get();
	if (((unsigned short) id >> 7) == family) {     // ### WTF ENDIAN
	    if ((newsize = id & ((1 << 7) - 1)) <= size) {
		if (newsize > lesser)
		    lesser = newsize;
	    } else {
		if (newsize < greater)
		    greater = newsize;
	    }
	}
    }
    SetResLoad(TRUE);
    *lesserp = lesser;
    *greaterp = greater == 32767 ? 0 : greater;
}

A5(PRIVATE, void, findclosestfond, FHandle, fh, INTEGER, size,
   INTEGER *, powerof2p, INTEGER *, lesserp, INTEGER *, greaterp)
{
    fatabentry *p, *ep;
    INTEGER newsize;
    INTEGER powerof2, lesser, greater;
    GUEST<INTEGER> *ip;

    powerof2 = 0;
    lesser = 0;
    greater = 32767;
    ip = &STARH(fh)->ffVersion + 1;
    for (p = (fatabentry *) (ip + 1), ep = p + Cx(*ip) + 1; p < ep; p++) {
	newsize = Cx(p->size);
	if (newsize < size) {
	    if (newsize == size / 2)
		powerof2 = newsize;	/* lower priority than  *2, but */
					/* we know these entries are ordered */
	    if (newsize > lesser)
		lesser = newsize;
	} else if (newsize == size) {
	    powerof2 = newsize;
/*-->*/	    break;
	} else {
	    if (newsize == size * 2)
		powerof2 = newsize;
	    if (newsize < greater)
		greater = newsize;
	}
    }
    *powerof2p = powerof2;
    *lesserp   = lesser;
    *greaterp  = greater == 32767 ? 0 : greater;
}

P2(PUBLIC pascal trap, BOOLEAN, RealFont, INTEGER, fnum,	/* IMI-223 */
							    INTEGER, sz)
{
    Handle h;
    int retval;
    
    SetResLoad(FALSE);
    h = GetResource(TICK("FONT"), FONTRESID(fnum, sz));
    if (!h)
	h = GetResource(TICK("NFNT"), FONTRESID(fnum, sz));
    retval = !!h;
    SetResLoad(TRUE);
    if (!retval)
      {
	FHandle fh;

	fh = (FHandle) GetResource (TICK ("FOND"), fnum);
        if (fh)
	  {
	    INTEGER powerof2, lesser, greater;
	    findclosestfond (fh, sz, &powerof2, &lesser, &greater);
	    if (powerof2 == sz)
	      retval = TRUE;
	  }
      }
    return retval;
}

A0(PRIVATE, INTEGER, closestface)	/* no args, uses WIDTHPTR */
{
    fatabentry *p, *ep, *bestp;
    INTEGER size;
    GUEST<INTEGER> *ip;
    INTEGER nmatch, want, newmatch;

    bestp = 0;
    size = Cx(WIDTHPTR->fSize);
    ip = &STARH((FHandle)MR(LastFOND))->ffVersion + 1;
    nmatch = -1;
    want = Cx(WIDTHPTR->aFace);
    for (p = (fatabentry *) (ip + 1), ep = p + CW(*ip) + 1;
					  p < ep && Cx(p->size) <= size; p++) {
	if (!bestp)
	    bestp = p;	/* pick something */
	if ((Cx(p->size) == size) && !(Cx(p->style) & ~want)) {
	    if ((newmatch = countones(Cx(p->style) & want)) >= nmatch) {
		/* note, IMIV is vague on precedence, so I choose the
			 last entry, though all I need is some comparison
			 function for bestp->style and p->style */
		nmatch = newmatch;
		bestp = p;
	    }
	}
    }
    WIDTHPTR->face = bestp->style;
    return Cx(bestp->fontresid);
}

PRIVATE FHandle
at_least_one_fond_entry (INTEGER family)
{
  FHandle retval;

  retval = (FHandle) GetResource (TICK("FOND"), family);
  if (retval)
    {
      INTEGER powerof2, lesser, greater;
      LoadResource ((Handle) retval);
      findclosestfond (retval, 1, &powerof2, &lesser, &greater);
      if (greater == 0)
	retval = 0;
    }
  return retval;
}

/*
 * TODO:  NFNT below
 */

#define AVAILABLE(x) (WIDTHPTR->fSize = CW((x)), WIDTHPTR->tabFont = \
		         RM(GetResource(TICK("FONT"), FONTRESID(family, (x)))))

A1(PRIVATE, void, newwidthtable, FMInput *, fmip)
{
    FHandle fh;
    INTEGER lesser, greater, wanted_family, family, fontresid, powerof2;
    LONGINT todelete;
    GUEST<THz> savezone;
    boolean_t tried_app_font;
    int n_tried_sys_font;

    savezone = TheZone;
    TheZone = SysZone;
    WidthTabHandle =  RM((WidthTableHandle) NewHandle((Size) sizeof(WidthTable)));
    TheZone = savezone;
    Munger((Handle) MR(WidthListHand), (LONGINT) 0, (Ptr) 0, 0,
				(Ptr) &WidthTabHandle, sizeof(WidthTabHandle));

/*
 * NOTE: the code below is a bit confusing.  What is going on is we always want
 *       to put the new entry in the front, but if we've grown too large then
 *	 we try to find a (Handle) 0 or a (Handle) -1) to delete.  If we find
 *	 neither we delete the last entry.
 */

    if (GetHandleSize((Handle) MR(WidthListHand)) >
			(int) sizeof(GUEST<Handle>) * MAXTABLES) {
	todelete = 0;
	if (Munger((Handle) MR(WidthListHand), (LONGINT) 0, (Ptr) &todelete,
					  sizeof(todelete), (Ptr) "", 0) < 0) {
	    todelete = -1;
	    if (Munger((Handle) MR(WidthListHand), (LONGINT) 0, (Ptr) &todelete,
					  sizeof(todelete), (Ptr) "", 0) < 0) {
		DisposHandle((Handle) MR(STARH(WIDTHLISTHAND)[MAXTABLES]));
		SetHandleSize(MR(WidthListHand),
				    (Size) sizeof(GUEST<Handle>) * MAXTABLES);
	    }
	}
    }
    HLock((Handle) MR(WidthTabHandle));
    WidthPtr = *MR(WidthTabHandle);

    WIDTHPTR->sExtra  = PORT_SP_EXTRA_X (thePort);
    WIDTHPTR->inNumer = fmip->numer;
    WIDTHPTR->inDenom = fmip->denom;
    WIDTHPTR->aFID    = fmip->family;
    WIDTHPTR->aFace   = fmip->face;
    WIDTHPTR->aSize   = fmip->size;
    WIDTHPTR->device  = fmip->device;
    WIDTHPTR->tabSize = 0;	/* I don't know what this field is, but it */
				/* appears to have flags in it */

    SetResLoad(TRUE);
    fh = 0;
    wanted_family = Cx (fmip->family);
    tried_app_font = FALSE;
    n_tried_sys_font = 0;
    if ((fh = at_least_one_fond_entry(wanted_family))
	|| GetResource(TICK("FONT"), FONTRESID(wanted_family, Cx(fmip->size)))
	|| GetResource(TICK("FONT"), FONTRESID(wanted_family, 0)))
      family = wanted_family;
    else if ((fh = at_least_one_fond_entry (Cx(ApFontID)))
	     || GetResource(TICK("FONT"), FONTRESID(Cx(ApFontID), 0)))
      {
	family = Cx(ApFontID);
	tried_app_font = TRUE;
      }	
    else
      {
	fh = at_least_one_fond_entry (Cx(SysFontFam));
	family = Cx(SysFontFam);
	tried_app_font = TRUE;
	++n_tried_sys_font;
      }
    WIDTHPTR->tabFont = 0;
  while (!WIDTHPTR->tabFont && n_tried_sys_font < 2) {
    WIDTHPTR->fHand = RM((Handle) fh);
    WIDTHPTR->fID = CW(family);
    if (fh) {
	LastFOND = RM((FamRecHandle) fh);

	findclosestfond(fh, Cx(fmip->size), &powerof2, &lesser, &greater);
	if (powerof2 == Cx(fmip->size))
	    WIDTHPTR->fSize = CW(powerof2);
	else {
	    if (FScaleDisable)
		WIDTHPTR->fSize   = lesser ? CW(lesser) : CW(greater);
	    else {
		if (powerof2)
		    WIDTHPTR->fSize = CW(powerof2);
		else {
		    if (!greater || (lesser && 
			  Cx(fmip->size) - lesser <= greater - Cx(fmip->size)))
			WIDTHPTR->fSize = CW(lesser);
		    else
			WIDTHPTR->fSize = CW(greater);
		}
	    }
	}
	if (!WIDTHPTR->fSize)
	    fprintf(stderr, "fh:  fSize = 0\n");
	fontresid = closestface();
	if (!(WIDTHPTR->tabFont = RM(GetResource(TICK("NFNT"), fontresid))))
	      WIDTHPTR->tabFont = RM(GetResource(TICK("FONT"), fontresid));
	if (!WIDTHPTR->tabFont)
	  WIDTHPTR->tabFont = 
	    RM(GetResource(TICK("FONT"),
			   FONTRESID(family, Cx(WIDTHPTR->fSize))));
	if (!WIDTHPTR->tabFont)
	  warning_unexpected (NULL_STRING);
    }
    if (!WIDTHPTR->tabFont) {
	WIDTHPTR->face  = 0;

	if (!AVAILABLE(Cx(fmip->size))) {
	    if (FScaleDisable) {
		findclosestfont(family, Cx(fmip->size), &lesser, &greater);
		WIDTHPTR->fSize   = lesser ? CW(lesser) : CW(greater);
		WIDTHPTR->tabFont = RM(GetResource(TICK("FONT"),
				      FONTRESID(family, Cx(WIDTHPTR->fSize))));
	    } else {
		if (!AVAILABLE(Cx(fmip->size) * 2) &&
					      !AVAILABLE(Cx(fmip->size) / 2)) {
		    findclosestfont(family, Cx(fmip->size), &lesser, &greater);
		    if (lesser &&
			 (Cx(fmip->size) - lesser <= greater - Cx(fmip->size)))
			WIDTHPTR->fSize = CW(lesser);
		    else
			WIDTHPTR->fSize = CW(greater);
		    WIDTHPTR->tabFont = RM(GetResource(TICK("FONT"),
				      FONTRESID(family, Cx(WIDTHPTR->fSize))));
		}
	    }
	}
    }
    if (!WIDTHPTR->tabFont) {
      if (!tried_app_font)
	{
	  tried_app_font = TRUE;
	  family = Cx(ApFontID);
	  fh = at_least_one_fond_entry (family);
	}
      else if (n_tried_sys_font < 2)
	{
	  ++n_tried_sys_font;
	  family = Cx (SysFontFam);
	  fh = at_least_one_fond_entry (family);
	}
    }
  }
  LoadResource (MR (WIDTHPTR->tabFont));
  if (!WIDTHPTR->fSize)
    warning_unexpected ("!fh:  fSize = 0, family = %d\n",
			(LONGINT) Cx(fmip->family));
}

P1(PUBLIC pascal trap, FMOutPtr, FMSwapFont, FMInput *, fmip)	/* IMI-223 */
{
    Style style;
    FontRec *fp;
    GUEST<INTEGER> savesize, savefamily;
    BOOLEAN needtobuild;
    
    savesize = fmip->size;
    if (Cx(fmip->size) <= 0)
	fmip->size = SysFontSiz;

    if (Cx(fmip->size) <= 0)
	fmip->size = CWC(12);

    if (fmip->denom.h == fmip->numer.h) {
	fmip->denom.h = CWC(256);
	fmip->numer.h = CWC(256);
    }
    if (fmip->denom.v == fmip->numer.v) {
	fmip->denom.v = CWC(256);
	fmip->numer.v = CWC(256);
    }

    if (Cx(fmip->family) == applFont) {
	savefamily   = fmip->family;
	fmip->family = ApFontID;
    } else
	savefamily = fmip->family;
    
    if (fmip->family == CW(0))
      {
	savefamily = 0;
	fmip->family = SysFontFam;
      }

    if (fmip->family  !=   ROMlib_myfmi.family  ||
	fmip->size    !=   ROMlib_myfmi.size    ||
	fmip->face    !=   ROMlib_myfmi.face    ||
	fmip->device  !=   ROMlib_myfmi.device  ||
	fmip->numer.h !=   ROMlib_myfmi.numer.h ||
	fmip->numer.v !=   ROMlib_myfmi.numer.v ||
	fmip->denom.h !=   ROMlib_myfmi.denom.h ||
	fmip->denom.v !=   ROMlib_myfmi.denom.v ||
	WIDTHPTR->fSize == CW(-1)	        ||
	WIDTHPTR->fSize ==  CW(0)	        ||
       (fmip->needBits && !ROMlib_myfmi.needBits)) {
	if ((needtobuild = !widthlistmatch(fmip)))
	    newwidthtable(fmip);

	ROMlib_myfmi = *fmip;
	ROMlib_fmo.errNum     = 0;
	ROMlib_fmo.bold       = 0;
	ROMlib_fmo.italic     = 0;
	ROMlib_fmo.ulOffset   = 0;
	ROMlib_fmo.ulShadow   = 0;
	ROMlib_fmo.ulThick    = 0;
	ROMlib_fmo.shadow     = 0;
	ROMlib_fmo.extra      = 0;
	ROMlib_fmo.numer.h = CW(
			    (LONGINT) Cx(fmip->numer.h) * 256 * Cx(fmip->size) /
				      Cx(fmip->denom.h) / Cx(WIDTHPTR->fSize));
	ROMlib_fmo.numer.v = CW(
			    (LONGINT) Cx(fmip->numer.v) * 256 * Cx(fmip->size) /
				      Cx(fmip->denom.v) / Cx(WIDTHPTR->fSize));
	ROMlib_fmo.denom.h    = CWC(256);
	ROMlib_fmo.denom.v    = CWC(256);
	ROMlib_fmo.fontHandle = WIDTHPTR->tabFont;
	LoadResource(MR(ROMlib_fmo.fontHandle));
	fp = (FontRec *) STARH((MR(ROMlib_fmo.fontHandle)));
	style = Cx(fmip->face) ^ Cx(WIDTHPTR->face);
	if (style & (int) bold )
	    mungfmo(ftstr.boldt, &ROMlib_fmo);
	if (style & (int) italic)
	    mungfmo(ftstr.italt, &ROMlib_fmo);
	if (style & (int) outline)
	    mungfmo(ftstr.outt, &ROMlib_fmo);
	if (style & (int) shadow)
	    mungfmo(ftstr.shadt, &ROMlib_fmo);
	if (style & (int) condense)
	    mungfmo(ftstr.condt, &ROMlib_fmo);
	if (style & (int) extend)
	    mungfmo(ftstr.extt, &ROMlib_fmo);
	if (style & (int) underline) {
	    ROMlib_fmo.ulOffset += ftstr.undert[0];
	    ROMlib_fmo.ulShadow += ftstr.undert[1];
	    ROMlib_fmo.ulThick += ftstr.undert[2];
	}
	if (needtobuild)
	    buildtable(ROMlib_fmo.extra);
	/*
	 * TODO:  examine the stuff below, particularly widMax.  I don't think
	 *	  widMax will be correct if scaling is disabled and we expand
	 *	  characters.  This will cause serious trouble when we allocate
	 *	  too small a bitmap
	 */
	ROMlib_fmo.ascent  = CW(fp->ascent)  + !!ROMlib_fmo.shadow;
	ROMlib_fmo.descent = CW(fp->descent) +   ROMlib_fmo.shadow;
	ROMlib_fmo.widMax  = CW(fp->widMax)  + !!ROMlib_fmo.shadow +
				 ROMlib_fmo.shadow + ROMlib_fmo.bold;
	ROMlib_fmo.leading = CW(fp->leading);
    } else {
	LoadResource(MR(ROMlib_fmo.fontHandle));
	WidthPtr = *MR(WidthTabHandle);
    }
    fmip->size   = savesize;
    fmip->family = savefamily;
    HUnlock((Handle) MR(WidthTabHandle));
    LastFOND = guest_cast<FamRecHandle>(WIDTHPTR->fHand);
    return &ROMlib_fmo;
}

#define SCALE(x) FixRatio(x *  CW(fmop->numer.v), CW(fmop->denom.v))

P1(PUBLIC pascal trap, void, FontMetrics, FMetricRec *, metrp)	/* IMIV-32 */
{
    FMInput fmi;
    FMOutPtr fmop;

    fmi.family   = PORT_TX_FONT_X (thePort);
    fmi.size     = PORT_TX_SIZE_X (thePort);
    fmi.face     = PORT_TX_FACE_X (thePort);
    fmi.needBits = CB(FALSE);
    fmi.device   = PORT_DEVICE_X (thePort);
    fmi.numer.v  = CWC(1);
    fmi.numer.h  = CWC(1);
    fmi.denom.v  = CWC(1);
    fmi.denom.h  = CWC(1);

    fmop = FMSwapFont(&fmi);

    /* TODO:  check out thePort->device and use the FOND stuff if not
	      going to the screen */

    metrp->ascent     = CL(SCALE(Cx(fmop->ascent)));
    metrp->descent    = CL(SCALE(Cx(fmop->descent)));
    metrp->leading    = CL(SCALE(Cx(fmop->leading)));
    metrp->widMax     = CL(SCALE(Cx(fmop->widMax)));
    metrp->wTabHandle = guest_cast<Handle>( WidthTabHandle );
}

P1(PUBLIC pascal trap, void, SetFScaleDisable,			/* IMIV-32 */
					       BOOLEAN, disable)
{
  if (FScaleDisable != (Byte) disable)
    {
      FScaleDisable = disable;
      invalidate_all_widths ();
      reset_myfmi ();
    }
}

P1(PUBLIC pascal trap, void, SetFractEnable, BOOLEAN, enable)	/* IMIV-32 */
{
    FractEnable = enable;
}
