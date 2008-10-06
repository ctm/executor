/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qStdText[] =
	    "$Id: qStdText.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "FontMgr.h"
#include "MemoryMgr.h"
#include "ToolboxUtil.h"

#include "rsys/cquick.h"
#include "rsys/picture.h"
#include "rsys/font.h"
#include "rsys/text.h"

#include "rsys/safe_alloca.h"

#undef ALLOCABEGIN

#if !defined(NDEBUG)
#define ALLOCABEGIN     SAFE_DECL();
#else
#define ALLOCABEGIN
#endif

#undef ALLOCA
#define ALLOCA(n)       SAFE_alloca(n)

#include "ultable.c"

A5(PRIVATE, void, charblit, BitMap *, fbmp, BitMap *, tbmp,	/* INTERNAL */
			      Rect *, srect, Rect *, drect, INTEGER, firsttime)
{
    INTEGER firstfrom, lastfrom, firstto, lastto;
    INTEGER firstmaskend, lastmaskend, wordstodo, wordsfromdo, shiftsize;
    INTEGER numfromwords, numtowords;
    INTEGER i, j, numrows;
    unsigned short *firstfromword, *firsttoword, *nextfromword, *nexttoword;
    ULONGINT nextlong, firstmask, lastmask;
    INTEGER firsttomod16;

    if (srect->left == srect->right)
/*-->*/ return;
/* Find the first column of pixels to be copied. */

    firstfrom = CW(srect->left) - CW(fbmp->bounds.left);

/* Find the last column of pixels to be copied. */

    lastfrom = CW(srect->right) - CW(fbmp->bounds.left);

/* Find the first column pixels are to be copied to. */

    firstto = CW(drect->left) - CW(tbmp->bounds.left);

/* Find the last column pixels are to be copied to. */

    lastto = CW(drect->right) - CW(tbmp->bounds.left);
    numrows = CW(srect->bottom) - CW(srect->top);
/* 
 * If it is to be put into italics, start with the characters
 * shifted over and shift them back one pixel at a time later.
 * Only do this once (the firsttime)
 */
    if ((PORT_TX_FACE (thePort) & (int) italic) && firsttime) {
	INTEGER temp;
	temp = (numrows) / 2;
	firstto += temp;
	lastto += temp;
    }
/*
 * Find where in the bitmaps to copy from and to, shifted up one row
 * because the BLITROW macro increments the row number at the beginning.
 * I had a reason for this, but I don't currently remember it.
 */
    firstfromword = (unsigned short *)
		    (MR(fbmp->baseAddr) + BITMAP_ROWBYTES (fbmp) *
		(CW(srect->top) - CW(fbmp->bounds.top) - 1)) + firstfrom / 16;

    firsttoword = (unsigned short *) (MR(tbmp->baseAddr) + BITMAP_ROWBYTES (tbmp) *
		   (CW(drect->top) - CW(tbmp->bounds.top) - 1)) + firstto / 16;
/*
 * Calculate the number of words to be taken from one bitmap and the
 * number to be put to the other minus one.  The minus one is because
 * the loop that needs wordstodo has an extra write at the end.
 * wordsfromdo is only used in comparisons.  They can be looked at
 * as the number of word boundaries crossed.
 */
    wordstodo = (lastto >> 4) - (firstto >> 4);
    wordsfromdo = (lastfrom >> 4) - (firstfrom >> 4);
/*
 * Find the number of bits from the first and last words that are to
 * be used and make the appropriate mask.
 */
    lastmaskend = lastfrom & 0x000F;
    firstmaskend = firstfrom & 0x000F;
    firsttomod16 = firstto & 0x000F;
/*
 * Calculate how far each bit needs to be right shifted so it comes out
 * correct in the destination bitmap.
 */
    shiftsize = firsttomod16 - firstmaskend;
/*
 * If more bits need to be put in the first word written than are taken
 * from the first word read, two words should be read in and the amount
 * to be shifted should be adjusted accordingly.
 */
    if (shiftsize < 0) {
	shiftsize += 16;
	firstmask = (ULONGINT) 0xFFFFFFFF >> firstmaskend;
    } else {
/*
 * If there are enough bits, start reading one word earlier and adjust
 * the mask so the state will be the same as the other case.
 */
	firstfromword--;
	firstmask = 0x0000FFFF >> firstmaskend;
    }
/* Calculate the number of bits to be used from the last from word.
 * TODO: elaborate
 */
    if ((wordstodo > wordsfromdo) || ((lastto & 0x000F) < lastmaskend)) {
	lastmask = 0xFFFFFFFF << (32 - lastmaskend);
    } else {
	lastmask = 0xFFFFFFFF << (16 - lastmaskend);
    }

/* Variables used for speed. */

    numfromwords = BITMAP_ROWBYTES (fbmp) / 2;
    numtowords   = BITMAP_ROWBYTES (tbmp) / 2;

#define BLITROW {							\
/* Start reading and writing the next line */				\
	    nextfromword = firstfromword += numfromwords;		\
	    nexttoword = firsttoword += numtowords;			\
/* Read the first two words and mask off the unused bits */		\
	    nextlong = (((ULONGINT) CW(*nextfromword) << 16) +	\
				CW(*(nextfromword + 1))) & firstmask;	\
	    nextfromword += 2;						\
	    for ( j = wordstodo ; --j >= 0 ; ) {			\
/* Write the next word, calculated by taking the appropriate		\
 * 16 bits from a LONGINT. */						\
		*nexttoword++ |= CW(nextlong >> shiftsize);		\
/* Prepare the LONGINT for the next pass. */				\
		nextlong = (nextlong << 16) + CW(*nextfromword++);	\
	    }								\
/* Write the last word with the appropriate bits masked out. */		\
	    *nexttoword |= CW((nextlong & lastmask) >> shiftsize);	\
	}


/*
 * SHIFTROWONELEFT recalculates all the values BLITROW uses.  It
 * saves some time because not all the values change and some
 * change in a simple way.
 */

#define SHIFTROWONELEFT {						\
/* All the bits are going to shift one left */				\
    firstto--;								\
    lastto--;								\
/* The next three lines are the same as the main code. */		\
    firsttomod16 = firstto & 0x000F;					\
    wordstodo = (lastto >> 4) - (firstto >> 4);				\
    shiftsize = firsttomod16 - firstmaskend;				\
/*									\
 * The calculation of firsttoword ends with + firstfrom / 16.		\
 * If firsttomod16 == 0xF after firstto has been decremented,		\
 * it needs to be corrected.  						\
 */									\
    if (firsttomod16 == 0x000F)						\
	firsttoword--;							\
    if (shiftsize < 0) {						\
/*									\
 * The first time shiftsize goes negative is the only time		\
 * firstfromword should be corrected from the change made		\
 * in the other part of the if.						\
 */									\
        if (shiftsize == -1)						\
            firstfromword++;						\
/* From the main code */						\
	shiftsize += 16;						\
	firstmask = (ULONGINT) 0xFFFFFFFF >> firstmaskend;		\
    } else {								\
        if (firsttomod16 == 0x000F) {					\
/* The first time shiftsize >= 0 is when firsttomod16 changes		\
 * from 0 to 0xF.  This line corresponds to the line in the 		\
 * main code which only happens once.  					\
 */									\
	    if (firstmaskend != 0)					\
		firstfromword--;					\
	}								\
/* From the main code */						\
	firstmask = 0x0000FFFF >> firstmaskend;				\
    }									\
    if ((wordstodo > wordsfromdo) || ((lastto & 0x000F) < lastmaskend))	\
	lastmask = 0xFFFFFFFF << (32 - lastmaskend);			\
    else								\
	lastmask = 0xFFFFFFFF << (16 - lastmaskend);			\
}

    if ((PORT_TX_FACE (thePort) & (int) italic) && firsttime) {
	if (!((i = numrows) & 1)) {
	    BLITROW;
	    SHIFTROWONELEFT;
	}
	while ((i -= 2) >= 1) {
	    BLITROW;
	    SHIFTROWONELEFT;
	    BLITROW;
	}
	BLITROW;
    } else {
	for (i = numrows ; --i >= 0 ; ) {
	    BLITROW;
	}
    }
}

#define FIXEDONEHALF	(1L << 15)
#define FIXED(n)	((LONGINT) (n) << 16)

/*
 * This is a mongrel routine.  Right now it's hand-crafted to do the work
 * of StdText and a xStdTxMeas, which is a helper routine that other
 * measurement routines call.
 */

PUBLIC LONGINT
text_helper (LONGINT n, Ptr textbufp, Point *nump, Point *denp,
	     FontInfo *finfop, INTEGER *charlocp, text_helper_action_t action)
{
  Point num, den;
  unsigned char *p;
  FMInput fmi;
  FMOutput *fmop;
  unsigned char *ep;
  INTEGER wid, offset, missing, *widp, *locp;
  unsigned out;
  register INTEGER c;
  FontRec *fp;
  BitMap fmap;
  Rect srect, drect, misrect;
  unsigned char count;
  INTEGER extra;
  Fixed fixed_extra;
  INTEGER strwidth;
  BitMap stylemap, stylemap2, stylemap3, *bmp;
  int nbytes;
  INTEGER i;
  INTEGER carryforward;
  INTEGER first, max, misintwidth, kernmax, hOutput, vOutput;
  Fixed hOutputInverse, *widths, width, left, misfixwidth, spacewidth;
  Fixed left_begin;
  Fixed space_extra;
  INTEGER lineabove, linebelow, descent, leftmost;
  INTEGER rightitalicoffset, leftitalicoffset;
  INTEGER fmopstate, widthstate;
  LONGINT retval;
  ALLOCABEGIN
  PAUSEDECL;

  p = (unsigned char *) textbufp;
  num.h = CW (nump->h);
  num.v = CW (nump->v);
  den.h = CW (denp->h);
  den.v = CW (denp->v);
  retval = 0;
  if (action == text_helper_measure)
    {
      if (n < 0)
	n = 0;
    }
  else
    {
      if (n <= 0)
/*-->*/	return retval;
      
      PIC_SAVE_EXCURSION
	({
	  ROMlib_textpicupdate (num, den);
	  PICOP (OP_LongText);
	  PICWRITE (&PORT_PEN_LOC (thePort), sizeof (PORT_PEN_LOC (thePort)));
	  count = n;
	  PICWRITE (&count, sizeof(count));
	  PICWRITE (p, count);
	  if (!(count & 1))
	    PICWRITE ("", 1);	/* even things out */
	});
	  
      if (PORT_PEN_VIS (thePort) < 0)
	{
	  Point swapped_num;
	  Point swapped_den;

	  swapped_num.h = CW (num.h);
	  swapped_num.v = CW (num.v);
	  swapped_den.h = CW (den.h);
	  swapped_den.v = CW (den.v);
	  PORT_PEN_LOC (thePort).h = CW (CW (PORT_PEN_LOC (thePort).h)
					 + (CALLTXMEAS (n, textbufp,
							&swapped_num,
							&swapped_den, 0)));
/*-->*/	  return retval;
	}
    }

  fmi.needBits = CB(TRUE);
  fmi.family   = PORT_TX_FONT_X (thePort);
  fmi.size     = PORT_TX_SIZE_X (thePort);
  fmi.face     = PORT_TX_FACE_X (thePort);
  fmi.device   = PORT_DEVICE_X (thePort);
  fmi.numer.h  = CW(num.h);
  fmi.numer.v  = CW(num.v);
  fmi.denom.h  = CW(den.h);
  fmi.denom.v  = CW(den.v);
  fmop = FMSwapFont(&fmi);

  if (action == text_helper_measure)
    {
      if (fmop->numer.h && fmop->denom.h)
	nump->h = CW((LONGINT) Cx(fmop->numer.h) << 8 / Cx(fmop->denom.h));
      else
	nump->h = fmop->numer.h;

      if (fmop->numer.v && fmop->denom.h)
	nump->v = CW((LONGINT) Cx(fmop->numer.v) << 8 / Cx(fmop->denom.h));
      else
	nump->v = fmop->numer.v;
      denp->h = WIDTHPTR->hFactor;
      denp->v = WIDTHPTR->hFactor;
      if (finfop)
	{
	  finfop->ascent  = CW((unsigned short) CB(fmop->ascent));
	  finfop->descent = CW((unsigned short) CB(fmop->descent));
	  finfop->widMax  = CW((unsigned short) CB(fmop->widMax));
	  finfop->leading = CW((unsigned short) CB(fmop->leading));
	}
    }

  extra = Cx(fmop->extra);
  fixed_extra = FIXED (extra);
  widthstate = HGetState((Handle) MR(WidthTabHandle));
  HLock((Handle) MR(WidthTabHandle));
  if ((PORT_TX_FACE (thePort) & (int) underline) && Cx(fmop->descent) < 2)
    descent = 2;
  else
    descent = Cx(fmop->descent);

  PAUSERECORDING;
  fmopstate = HGetState((Handle) MR(fmop->fontHandle));
  HLock(MR(fmop->fontHandle));
  fp = (FontRec *) STARH(MR(fmop->fontHandle));
  fmap.baseAddr = RM((Ptr) (&fp->rowWords + 1));
  fmap.rowBytes = CW(CW(fp->rowWords) * 2);
  fmap.bounds.left = fmap.bounds.top = 0;
  fmap.bounds.right = CW(CW(fp->rowWords) * 16);
  fmap.bounds.bottom = fp->fRectHeight;
  srect.top = misrect.top = 0;
  srect.bottom = misrect.bottom = fp->fRectHeight;
  widp = (INTEGER *)&(fp->owTLoc) + Cx(fp->owTLoc);
  locp = ((INTEGER *)&(fp->rowWords) + Cx(fp->rowWords) * Cx(fp->fRectHeight)
	  + 1);
  missing = *(widp + Cx(fp->lastChar) - Cx(fp->firstChar) + 1) & 0xff;
  misrect.left = *(locp + Cx(fp->lastChar) - Cx(fp->firstChar) + 1);
  misrect.right = *(locp + Cx(fp->lastChar) - Cx(fp->firstChar) + 2);
  drect.left = PORT_PEN_LOC (thePort).h;
  drect.top = CW(CW (PORT_PEN_LOC (thePort).v) - CW(fp->ascent));
  drect.bottom = CW(CW(drect.top) + Cx(fp->fRectHeight));

  hOutput = Cx(WIDTHPTR->hOutput);
  vOutput = Cx(WIDTHPTR->vOutput);
  hOutputInverse = FixRatio (1 << 8, hOutput);

  space_extra = PORT_SP_EXTRA (thePort);
  spacewidth = (CL (WIDTHPTR->tabData[' ']) + space_extra
		- CL (WIDTHPTR->sExtra));

  if (action == text_helper_draw)
    {
      Point swapped_num;
      Point swapped_den;
    
      swapped_num.h = CW (num.h);
      swapped_num.v = CW (num.v);
      swapped_den.h = CW (den.h);
      swapped_den.v = CW (den.v);

      strwidth = text_helper (n, textbufp, &swapped_num, &swapped_den, 0, 0,
			       text_helper_measure);
    }
#if !defined (LETGCCWAIL)
  else
    {
      strwidth = 0;
    }
#endif
			  
  /* SPEEDUP:  make the stylemap be of the same alignment as
     thePort.portBits when it matters to the blitter */

  rightitalicoffset = ((PORT_TX_FACE (thePort) & (int) italic)
		       ? CB(fmop->ascent) / 2 - 1 : 0);
  leftitalicoffset = ((PORT_TX_FACE (thePort) & (int) italic)
		      ? descent / 2 + 1 : 0);

  if (action == text_helper_draw)
    {
      if (strwidth <= 0)
/*-->*/	return 0;

      stylemap.rowBytes = CW((strwidth - Cx(fp->kernMax) + leftitalicoffset +
			      rightitalicoffset + 31)/32 *  4);
      stylemap.bounds.top    = CW (CW (PORT_PEN_LOC (thePort).v)
				   - CB(fmop->ascent));
#if 0
      stylemap.bounds.bottom = CW (CW (PORT_PEN_LOC (thePort).v)
				   + descent);
#else
      {
	int height;

	height = MAX (CB(fmop->ascent) + descent, Cx (fp->fRectHeight));
	stylemap.bounds.bottom = CW (CW (PORT_PEN_LOC (thePort).v)
				   - CB(fmop->ascent) + height);
      }
#endif
      stylemap.bounds.left   = CW (CW (PORT_PEN_LOC (thePort).h)
				   + Cx(fp->kernMax) - leftitalicoffset);
      stylemap.bounds.right  = CW(CW(PORT_PEN_LOC (thePort).h)
				  + strwidth + rightitalicoffset);
      if (fmop->shadow)
	stylemap.bounds.left = CW(CW(stylemap.bounds.left) - 1);
      nbytes = ((CW(stylemap.bounds.bottom) - CW(stylemap.bounds.top)) *
		CW(stylemap.rowBytes));
      stylemap.baseAddr = RM((Ptr) ALLOCA(nbytes));
      memset(MR(stylemap.baseAddr), 0, nbytes);
      bmp = &stylemap;
    }
#if !defined(LETGCCWAIL)
  else
    {
      bmp = 0;
      nbytes = 0;
    }
#endif
/*
 * Note:  We aren't using any image height info
 */

/*
 * TODO:  don't just check txFace & bold, you need to see whether it
 *	  has already been emboldened... (This is simple to fix)
 */
  
  
  first = Cx(fp->firstChar);
  max   = Cx(fp->lastChar) - Cx(fp->firstChar);
  misintwidth = CW(misrect.right) - CW(misrect.left);
  misfixwidth = FIXED(misintwidth);
  left    = FIXED(CW(PORT_PEN_LOC (thePort).h)) + FIXEDONEHALF;
  left_begin = left;
  widths  = WIDTHPTR->tabData;
  kernmax = Cx(fp->kernMax);
  leftmost = left >> 16;
  WIDTHPTR->tabData[' '] = CL (spacewidth);
  WIDTHPTR->sExtra = CL (space_extra);
  if (action == text_helper_draw)
    ASSERT_SAFE(MR(stylemap.baseAddr));
  for (ep = p + n; p != ep; p++)
    {
      if (charlocp)
	*charlocp++ = CW ((left - left_begin + 0xffff) >> 16);
      c = *p;
      width = CL(widths[c]);
      if ((c -= Cx(fp->firstChar)) < 0 || c > max
	  || (wid = CW(widp[c])) == -1)
	{
	  drect.left  = CW(left >> 16);
	  if (CW(drect.left) < leftmost)
	    leftmost = CW(drect.left);
	  drect.right = CW(CW(drect.left) + misintwidth);
	  if (action == text_helper_draw)
	    {
	      charblit(&fmap, bmp, &misrect, &drect, TRUE);
	      ASSERT_SAFE(MR(stylemap.baseAddr));
	    }
	  if (FractEnable)
	    left += width;
	  else
	    left += misfixwidth;
	}
      else
	{
	  srect.left  = locp[c];
	  srect.right = locp[c+1];
	  drect.left  = CW(left >> 16);
	  drect.left = CW(CW(drect.left) + (offset = (wid >> 8) + kernmax));
	  if (CW(drect.left) < leftmost)
	    leftmost = CW(drect.left);
	  drect.right = CW(CW(drect.left) + CW(srect.right) - CW(srect.left));
	  if (action == text_helper_draw)
	    {
	      charblit(&fmap, bmp, &srect, &drect, TRUE);
	      ASSERT_SAFE(MR(stylemap.baseAddr));
	    }
	  if (FractEnable)
	    left += width;
	  else
	    {
	      if (c == ' ' - Cx(fp->firstChar))
		left += spacewidth;
	      else
		left += FIXED((wid & 0xFF) + extra);
	    }
	}
    }
  if (charlocp)
    *charlocp = CW ((left - left_begin + 0xffff) >> 16);
  if (action == text_helper_measure)
    {
      retval = (left - left_begin + 0xFFFF) >> 16;
    }
  else
    {
      ASSERT_SAFE(MR(stylemap.baseAddr));
      stylemap.bounds.right = CW((left >> 16) + rightitalicoffset); 
      if (PORT_TX_FACE (thePort) & (int) bold)
	{
	  stylemap2 = stylemap;
	  stylemap2.baseAddr = RM((Ptr) ALLOCA(nbytes));
	  ASSERT_SAFE(MR(stylemap.baseAddr));
#if 0
	  BlockMove(MR(stylemap.baseAddr), MR(stylemap2.baseAddr), (Size) nbytes);
#else
	  memcpy(MR(stylemap2.baseAddr), MR(stylemap.baseAddr), (Size) nbytes);
#endif
	  srect = stylemap.bounds;
	  drect = srect;
      
	  ASSERT_SAFE(MR(stylemap.baseAddr));
	  for (i = 0; i++ < Cx(fmop->bold);)
	    {
	      drect.left = CW(CW(drect.left) + 1);
	      srect.right = CW(CW(srect.right) - 1);
	      charblit(&stylemap2, bmp, &srect, &drect, FALSE);
	      ASSERT_SAFE(MR(stylemap.baseAddr));
	    }
	  ASSERT_SAFE(MR(stylemap.baseAddr));
	}
      
      ASSERT_SAFE(MR(stylemap.baseAddr));
      if (PORT_TX_FACE (thePort) & (int) underline)
	{
	  p = ((unsigned char *) MR(bmp->baseAddr) + BITMAP_ROWBYTES (bmp) *
	       (CB(fmop->ascent)+1));
	  for (i = 0; i++ < Cx(fmop->ulThick) ;)
	    {
	      carryforward = 0;
	      /*
	       * linebelow is zero if we shouldn't be peering into the following line
	       */
	      lineabove = -BITMAP_ROWBYTES (bmp);
	      linebelow = descent > 2 ? BITMAP_ROWBYTES (bmp) : 0;
	      for (ep = p + BITMAP_ROWBYTES (bmp); p != ep; )
		{
		  c = *p | p[lineabove] | p[linebelow];
		  out = ultable[c];
		  if (c & 0x80)
		    {
		      if (!carryforward)
			p[-1] &= 0xFE;
		    }
		  else
		    if (carryforward)
		      out &= 0x7F;
		  carryforward = *p & 1;
		  *p = out | *p;
		  p++;
		}
	    }
	}
      ASSERT_SAFE(MR(stylemap.baseAddr));
      
      if (PORT_TX_FACE (thePort) & (int) (outline|shadow))
	{
	  stylemap2 = stylemap;
	  stylemap3 = stylemap;
	  stylemap2.baseAddr = stylemap.baseAddr;
	  stylemap3.baseAddr = RM((Ptr) ALLOCA(nbytes));
#if 0
	  BlockMove(MR(stylemap2.baseAddr), MR(stylemap3.baseAddr),	(Size) nbytes);
#else
	  memcpy(MR(stylemap3.baseAddr), MR(stylemap2.baseAddr), (Size) nbytes);
#endif
	  stylemap.baseAddr = RM((Ptr) ALLOCA(nbytes));

	  srect = stylemap.bounds;
	  drect = srect;

	  srect.left = CW(CW(srect.left) + 1);
	  drect.right = CW(CW(drect.right) - 1);
	  ASSERT_SAFE(MR(stylemap.baseAddr));
	  charblit(&stylemap2, &stylemap3, &srect, &drect, FALSE);
	  ASSERT_SAFE(MR(stylemap.baseAddr));
	  srect.left = CW(CW(srect.left) - 1); /* restore */
	  drect.right = CW(CW(drect.right) + 1);
	  for (i = 0; i++ < Cx(fmop->shadow); )
	    {
	      drect.left = CW(CW(drect.left) + 1);
	      srect.right = CW(CW(srect.right) - 1);
	      charblit(&stylemap2, &stylemap3, &srect, &drect, FALSE);
	    }
	  ASSERT_SAFE(MR(stylemap.baseAddr));
	  drect.left  = CW(CW(drect.left)  - Cx(fmop->shadow));	/* restore */
	  srect.right = CW(CW(srect.right) + Cx(fmop->shadow));

#if 0
	  BlockMove(MR(stylemap3.baseAddr), MR(bmp->baseAddr), (Size) nbytes);
#else
	  memcpy(MR(bmp->baseAddr), MR(stylemap3.baseAddr), (Size) nbytes);
#endif

	  srect.top = CW(CW(srect.top) + 1);
	  drect.bottom = CW(CW(drect.bottom) - 1);
	  ASSERT_SAFE(MR(stylemap.baseAddr));
	  charblit(&stylemap3, bmp, &srect, &drect, FALSE);
	  srect.top = CW(CW(srect.top) - 1); /* restore */
	  drect.bottom = CW(CW(drect.bottom) + 1);
	  ASSERT_SAFE(MR(stylemap.baseAddr));
	  for (i = 0; i++ < Cx(fmop->shadow); )
	    {
	      drect.top = CW(CW(drect.top) + 1);
	      srect.bottom = CW(CW(srect.bottom) - 1);
	      charblit(&stylemap3, bmp, &srect, &drect, FALSE);
	    }
	  drect.top  = CW(CW(drect.top) - Cx(fmop->shadow)); /* restore */
	  srect.bottom = CW(CW(srect.bottom) + Cx(fmop->shadow));

	  ASSERT_SAFE(MR(stylemap3.baseAddr));
	  ASSERT_SAFE(MR(stylemap.baseAddr));
	}
      
      drect.top    = CW(CW(bmp->bounds.top)
			- (FixMul((LONGINT) CB(fmop->ascent) << 16,
				  (Fixed) (vOutput - 256) << 8) >> 16));
      drect.left   = CW(leftmost);
      drect.bottom = CW(CW(drect.top)
			+ (FixMul((LONGINT) (CW(bmp->bounds.bottom)
					     - CW(bmp->bounds.top)) << 16,
				  (Fixed) vOutput << 8) >> 16));
      drect.right  = CW(leftmost +
			(FixMul((LONGINT) (CW(bmp->bounds.right)
					   - leftmost) << 16,
				(Fixed) hOutput << 8) >> 16));
      srect = bmp->bounds;
      srect.left = CW(leftmost);
      ASSERT_SAFE(MR(stylemap.baseAddr));
      StdBits(bmp, &srect, &drect, PORT_TX_MODE (thePort) & 0x37, (RgnHandle)0);
      ASSERT_SAFE(MR(stylemap.baseAddr));
      if (PORT_TX_FACE (thePort) & (int) (outline|shadow))
	{
	  ASSERT_SAFE(MR(stylemap.baseAddr));
	  StdBits(&stylemap2, &srect, &drect, srcXor, (RgnHandle)0);
	  ASSERT_SAFE(MR(stylemap.baseAddr));
	  ASSERT_SAFE (MR (stylemap2.baseAddr));
	}
      
      PORT_PEN_LOC (thePort).h = CW(CW(drect.right) - 
				    (FixMul((LONGINT) rightitalicoffset << 16,
					    (Fixed) hOutput << 8) >> 16));
      ASSERT_SAFE(MR(stylemap.baseAddr));
    }
  HSetState(MR(fmop->fontHandle), fmopstate);
  HSetState((Handle) MR(WidthTabHandle), widthstate);
  RESUMERECORDING;
  ALLOCAEND
  return retval;
}

PUBLIC boolean_t ROMlib_text_output_disabled_p;

PUBLIC boolean_t
disable_text_printing (void)
{
  boolean_t retval;

  retval = ROMlib_text_output_disabled_p;
  ROMlib_text_output_disabled_p = TRUE;
  return retval;
}

PUBLIC void
set_text_printing (boolean_t state)
{
  ROMlib_text_output_disabled_p = state;
}

P4(PUBLIC pascal trap, void, StdText, INTEGER, n, Ptr, textbufp,
							Point, num, Point, den)
{
  /*  if (!ROMlib_text_output_disabled_p) */
    {
      Point swapped_num;
      Point swapped_den;
  
      swapped_num.h = CW (num.h);
      swapped_num.v = CW (num.v);
      swapped_den.h = CW (den.h);
      swapped_den.v = CW (den.v);
      text_helper (n, textbufp, &swapped_num, &swapped_den, 0, 0,
		   text_helper_draw);
    }
}

#define FIXEDONEHALF	(1L << 15)

PUBLIC INTEGER
xStdTxMeas (INTEGER n, Byte *p, Point *nump, Point *denp,
	    FontInfo *finfop, INTEGER *charlocp)
{
  INTEGER retval;

  retval = text_helper (n, (Ptr) p, nump, denp, finfop,	charlocp,
			text_helper_measure);
  return retval;
}

P5(PUBLIC pascal trap, INTEGER, StdTxMeas, INTEGER, n, Ptr, p,
			      Point *, nump, Point *, denp, FontInfo *, finfop)
{
    return xStdTxMeas(n, (unsigned char *) p, nump, denp, finfop,
							        (INTEGER *) 0);
}

A5(PUBLIC, INTEGER, ROMlib_StdTxMeas, LONGINT, n, Ptr, p,
			      Point *, nump, Point *, denp, FontInfo *, finfop)
{
    return xStdTxMeas(n, (unsigned char *) p, nump, denp, finfop,
							        (INTEGER *) 0);
}

P3(PUBLIC pascal trap, void, MeasureText, INTEGER, n, Ptr, text, /* IMIV-25 */
								 Ptr, chars)
{
  Point num, den;
  
  num.h = num.v = den.h = den.v = CWC (1);
  xStdTxMeas(n, (unsigned char *) text, &num, &den,
	     (FontInfo *) 0, (INTEGER *) chars);
}
