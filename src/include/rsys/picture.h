/*
 * Copyright 1986, 1989, 1990, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: picture.h 63 2004-12-24 18:19:43Z ctm $
 */

#if !defined (_PICTURE_H_)
#define _PICTURE_H_

#include "rsys/quick.h"

namespace Executor {
#define PIC_SHORT_COMMENT	0xA0
#define PIC_LONG_COMMENT	0xA1

#define PICOP(x)	{ INTEGER op = CW(x); PICWRITE(&op, sizeof(op));  }

#define PIC_SAVE_EXCURSION(body) { if (PORT_PIC_SAVE_X (thePort)) { body } }

#define PICSAVEBEGIN(x)	if (thePort->picSave) {				\
			    PICOP((x))

#define PICSAVEBEGIN0()	if (thePort->picSave) {				\

#define PICSAVEEND	}

#define PAUSEDECL	Handle savepichand
#define PAUSERECORDING	savepichand = thePort->picSave, thePort->picSave = 0
#define RESUMERECORDING	thePort->picSave = savepichand

#define PAUSE_PIC_EXCURSION(body)		\
  {						\
    PAUSEDECL;					\
						\
    PAUSERECORDING;				\
    { body }					\
    RESUMERECORDING;				\
  }
  
/*
 * Since this data structure doesn't appear to be documented anywhere, I
 * figured it out by opening pictures, doing operations and closing them on
 * our Mac+.
 *
 * I had to figure this stuff out, because the Word 5 EPS/TIFF/PICT module
 * actually uses it's own PutPicProc, and it didn't like our old hacky
 * picture construction routines (i.e. write the opcodes out immediately).
 *
 * There are only two bytes that I couldn't figure out (picidunno).  I
 * suspect it's the ColorBit info, but I couldn't get QuickDraw to change
 * it.
 *
 * --ctm Sun Aug  2 19:48:35 MDT 1992
 */


    /*
 * NOTE: After doing:
 *	MoveTo(120	PACKED, 121)	PACKED;
 *	Line(3	PACKED, 5)	PACKED;
 *	DrawString("\pfoo")	PACKED;
 *	Line(0	PACKED, 0)	PACKED;
 *
 *	0x2C-0x2F was (123	PACKED, 126) but
 *	0x30-0x33 was (145	PACKED, 126)
 *
 *	This combined with bad experiences when Executor first
 *	started drawing pictures is how I came up with picdrawpnloc
 *	and pictextpnloc.
 */



struct piccache : GuestStruct {
    GUEST< PicHandle> pichandle;    /* 0x00 - 0x03 */
    GUEST< LONGINT> picsize;    /* 0x04 - 0x07 */
    GUEST< LONGINT> pichowfar;    /* 0x08 - 0x0B */
    GUEST< RgnHandle> picclip;    /* 0x0C - 0x0F */
    GUEST< Pattern> picbkpat;    /* 0x10 - 0x17 */
    GUEST< INTEGER> picfont;    /* 0x18 - 0x19 */
    GUEST< Style> picface;    /* 0x1A */
    GUEST< char> picfiller;    /* 0x1B */
    GUEST< INTEGER> pictxmode;    /* 0x1C - 0x1D */
    GUEST< INTEGER> pictxsize;    /* 0x1E - 0x1F */
    GUEST< Fixed> picspextra;    /* 0x20 - 0x23 */
    GUEST< Point> pictxnum;    /* 0x24 - 0x27 */
    GUEST< Point> pictxden;    /* 0x28 - 0x2B */
    GUEST< Point> picdrawpnloc;    /* 0x2C - 0x2F */
    GUEST< Point> pictextpnloc;    /* 0x30 - 0x33 */
    GUEST< Point> picpnsize;    /* 0x34 - 0x37 */
    GUEST< INTEGER> picpnmode;    /* 0x38 - 0x39 */
    GUEST< Pattern> picpnpat;    /* 0x3A - 0x41 */
    GUEST< Pattern> picfillpat;    /* 0x42 - 0x49 */
    GUEST< Rect> piclastrect;    /* 0x4A - 0x51 */
    GUEST< Point> picov;    /* 0x52 - 0x55 */
    GUEST< INTEGER> picidunno;    /* 0x56 - 0x59 ColorBit ? */
    GUEST< LONGINT> picforeColor;    /* 0x5A - 0x5D */
    GUEST< LONGINT> picbackColor;    /* 0x5E - 0x61 */
};

extern void ROMlib_textpicupdate( Point num, Point den );
extern void ROMlib_drawingpicupdate( void );
extern void ROMlib_drawingverbpicupdate( GrafVerb v );
extern void ROMlib_drawingverbrectpicupdate( GrafVerb v, Rect *rp );
extern void ROMlib_drawingverbrectovalpicupdate( GrafVerb v, Rect *rp,
								  Point *ovp );

#define INITIALPICSIZE 0x100

#define OP_NOP			0x0000
#define OP_Clip			0x0001
#define OP_BkPat		0x0002
#define OP_TxFont		0x0003
#define OP_TxFace		0x0004
#define OP_TxMode		0x0005
#define OP_SpExtra		0x0006
#define OP_PnSize		0x0007
#define OP_PnMode		0x0008
#define OP_PnPat		0x0009
#define OP_FillPat		0x000A
#define OP_OvSize		0x000B
#define OP_Origin		0x000C
#define OP_TxSize		0x000D
#define OP_FgColor		0x000E
#define OP_BkColor		0x000F
#define OP_TxRatio		0x0010
#define OP_Version		0x0011
#define OP_BkPixPat		0x0012
#define OP_PnPixPat		0x0013
#define OP_FillkPixPat		0x0014
#define OP_PnLocHFrac		0x0015
#define OP_ChExtra		0x0016

#define OP_RGBFgCol		0x001A
#define OP_RGBBkCol		0x001B
#define OP_HiliteMode		0x001C
#define OP_HiliteColor		0x001D
#define OP_DefHilite		0x001E
#define OP_OpColor		0x001F
#define OP_Line			0x0020
#define OP_LineFrom		0x0021
#define OP_ShortLine		0x0022
#define OP_ShortLineFrom	0x0023

#define OP_LongText		0x0028
#define OP_DHText		0x0029
#define OP_DVText		0x002A
#define OP_DHDVText		0x002B
#define OP_frameRect		0x0030
#define OP_paintRect		0x0031
#define OP_eraseRect		0x0032
#define OP_invertRect		0x0033
#define OP_fillRect		0x0034

#define OP_frameSameRect	0x0038
#define OP_paintSameRect	0x0039
#define OP_eraseSameRect	0x003A
#define OP_invertSameRect	0x003B
#define OP_fillSameRect		0x003C

#define OP_frameRRect		0x0040
#define OP_paintRRect		0x0041
#define OP_eraseRRect		0x0042
#define OP_invertRRect		0x0043
#define OP_fillRRect		0x0044

#define OP_frameSameRRect	0x0048
#define OP_paintSameRRect	0x0049
#define OP_eraseSameRRect	0x004A
#define OP_invertSameRRect	0x004B
#define OP_fillSameRRect	0x004C

#define OP_frameOval		0x0050
#define OP_paintOval		0x0051
#define OP_eraseOval		0x0052
#define OP_invertOval		0x0053
#define OP_fillOval		0x0054

#define OP_frameSameOval	0x0058
#define OP_paintSameOval	0x0059
#define OP_eraseSameOval	0x005A
#define OP_invertSameOval	0x005B
#define OP_fillSameOval		0x005C

#define OP_frameArc		0x0060
#define OP_paintArc		0x0061
#define OP_eraseArc		0x0062
#define OP_invertArc		0x0063
#define OP_fillArc		0x0064

#define OP_frameSameArc		0x0068
#define OP_paintSameArc		0x0069
#define OP_eraseSameArc		0x006A
#define OP_invertSameArc	0x006B
#define OP_fillSameArc		0x006C

#define OP_framePoly		0x0070
#define OP_paintPoly		0x0071
#define OP_erasePoly		0x0072
#define OP_invertPoly		0x0073
#define OP_fillPoly		0x0074

#define OP_frameSamePoly	0x0078
#define OP_paintSamePoly	0x0079
#define OP_eraseSamePoly	0x007A
#define OP_invertSamePoly	0x007B
#define OP_fillSamePoly		0x007C

#define OP_frameRgn		0x0080
#define OP_paintRgn		0x0081
#define OP_eraseRgn		0x0082
#define OP_invertRgn		0x0083
#define OP_fillRgn		0x0084

#define OP_frameSameRgn		0x0088
#define OP_paintSameRgn		0x0089
#define OP_eraseSameRgn		0x008A
#define OP_invertSameRgn	0x008B
#define OP_fillSameRgn		0x008C

#define OP_BitsRect		0x0090
#define OP_BitsRgn		0x0091

#define OP_PackBitsRect		0x0098
#define OP_PackBitsRgn		0x0099

#define OP_DirectBitsRect	0x009A
#define OP_DirectBitsRgn	0x009B
									
#define OP_ShortComment		0x00A0
#define OP_LongComment		0x00A1

#define OP_EndPic		0x00FF

#define OP_Header		0x0C00

typedef piccache *piccacheptr;
MAKE_HIDDEN(piccacheptr);
typedef HIDDEN_piccacheptr *piccachehand;

extern PicHandle ROMlib_OpenPicture_helper (const Rect *pf,
					    const OpenCPicParams *params);

}
#endif /* _PICTURE_H_ */
