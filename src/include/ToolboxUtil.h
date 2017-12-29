#if !defined(_TOOLBOX_UTIL_H_)
#define _TOOLBOX_UTIL_H_

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "QuickDraw.h"
#include "SANE.h"

namespace Executor
{
enum
{
    sysPatListID = 0,
};

enum
{
    iBeamCursor = 1,
    crossCursor = 2,
    plusCursor = 3,
    watchCursor = 4,
};

struct Int64Bit
{
    GUEST_STRUCT;
    GUEST<LONGINT> hiLong;
    GUEST<LONGINT> loLong;
};

#if 1 || !defined(__alpha)
typedef Pattern *PatPtr;
#else /* defined(__alpha) */
typedef char *PatPtr;
// FIXME: #warning "Bad PatPtr def ... 'cause of problems with my gcc mods -- ctm"
#endif /* defined(__alpha) */

typedef GUEST<PatPtr> *PatHandle;

/* DO NOT DELETE THIS LINE */
extern pascal trap Fract C_FracSqrt(Fract x);
PASCAL_TRAP(FracSqrt, 0xA849);

extern pascal trap Fract C_FracSin(Fixed x);
PASCAL_TRAP(FracSin, 0xA848);

extern Fixed FixATan2(LONGINT x, LONGINT y);
extern pascal trap Fixed C_FixAtan2(LONGINT x, LONGINT y);
PASCAL_TRAP(FixAtan2, 0xA818);

extern pascal trap Fract C_FracCos(Fixed x);
PASCAL_TRAP(FracCos, 0xA847);

extern pascal trap Fixed C_FixRatio(INTEGER n, INTEGER d);
PASCAL_TRAP(FixRatio, 0xA869);

extern pascal trap Fixed C_FixMul(Fixed a, Fixed b);
PASCAL_TRAP(FixMul, 0xA868);

extern pascal trap INTEGER C_FixRound(Fixed x);
PASCAL_TRAP(FixRound, 0xA86C);

extern pascal trap StringHandle C_NewString(StringPtr s);
PASCAL_TRAP(NewString, 0xA906);

extern pascal trap void C_SetString(StringHandle h, StringPtr s);
PASCAL_TRAP(SetString, 0xA907);

extern pascal trap StringHandle C_GetString(INTEGER i);
PASCAL_TRAP(GetString, 0xA9BA);

extern void GetIndString(StringPtr s, INTEGER sid, INTEGER index);
extern pascal trap LONGINT C_Munger(Handle h, LONGINT off, Ptr p1,
                                    LONGINT len1, Ptr p2, LONGINT len2);
PASCAL_TRAP(Munger, 0xA9E0);
extern pascal trap void C_PackBits(GUEST<Ptr> *sp, GUEST<Ptr> *dp, INTEGER len);
PASCAL_TRAP(PackBits, 0xA8CF);

extern pascal trap void C_UnpackBits(GUEST<Ptr> *sp, GUEST<Ptr> *dp, INTEGER len);
PASCAL_TRAP(UnpackBits, 0xA8D0);

extern pascal trap BOOLEAN C_BitTst(Ptr bp, LONGINT bn);
PASCAL_TRAP(BitTst, 0xA85D);

extern pascal trap void C_BitSet(Ptr bp, LONGINT bn);
PASCAL_TRAP(BitSet, 0xA85E);

extern pascal trap void C_BitClr(Ptr bp, LONGINT bn);
PASCAL_TRAP(BitClr, 0xA85F);

extern pascal trap LONGINT C_BitAnd(LONGINT a, LONGINT b);
PASCAL_TRAP(BitAnd, 0xA858);

extern pascal trap LONGINT C_BitOr(LONGINT a, LONGINT b);
PASCAL_TRAP(BitOr, 0xA85B);

extern pascal trap LONGINT C_BitXor(LONGINT a, LONGINT b);
PASCAL_TRAP(BitXor, 0xA859);

extern pascal trap LONGINT C_BitNot(LONGINT a);
PASCAL_TRAP(BitNot, 0xA85A);

extern pascal trap LONGINT C_BitShift(LONGINT a, INTEGER n);
PASCAL_TRAP(BitShift, 0xA85C);

extern pascal trap INTEGER C_HiWord(LONGINT a);
PASCAL_TRAP(HiWord, 0xA86A);

extern pascal trap INTEGER C_LoWord(LONGINT a);
PASCAL_TRAP(LoWord, 0xA86B);

extern pascal trap void C_LongMul(LONGINT a, LONGINT b, Int64Bit *c);
PASCAL_TRAP(LongMul, 0xA867);

extern pascal trap PatHandle C_GetPattern(INTEGER id);
PASCAL_TRAP(GetPattern, 0xA9B8);

extern void GetIndPattern(Byte *op, INTEGER plistid, INTEGER index);
extern pascal trap CursHandle C_GetCursor(INTEGER id);
PASCAL_TRAP(GetCursor, 0xA9B9);

extern pascal trap PicHandle C_GetPicture(INTEGER id);
PASCAL_TRAP(GetPicture, 0xA9BC);

extern pascal trap LONGINT C_DeltaPoint(Point a, Point b);
PASCAL_TRAP(DeltaPoint, 0xA94F);

extern pascal trap Fixed C_SlopeFromAngle(INTEGER a);
PASCAL_TRAP(SlopeFromAngle, 0xA8BC);

extern pascal trap INTEGER C_AngleFromSlope(Fixed s);
PASCAL_TRAP(AngleFromSlope, 0xA8C4);

extern pascal trap Fract C_FracMul(Fract x, Fract y);
PASCAL_TRAP(FracMul, 0xA84A);

extern pascal trap Fixed C_FixDiv(Fixed x, Fixed y);
PASCAL_TRAP(FixDiv, 0xA84D);

extern pascal trap Fract C_FracDiv(Fract x, Fract y);
PASCAL_TRAP(FracDiv, 0xA84B);

extern pascal trap Fixed C_Long2Fix(LONGINT x);
PASCAL_TRAP(Long2Fix, 0xA83F);

extern pascal trap LONGINT C_Fix2Long(Fixed x);
PASCAL_TRAP(Fix2Long, 0xA840);

extern pascal trap Fract C_Fix2Frac(Fixed x);
PASCAL_TRAP(Fix2Frac, 0xA841);

extern pascal trap Fixed C_Frac2Fix(Fract x);
PASCAL_TRAP(Frac2Fix, 0xA842);

extern Extended Fix2X(Fixed x);
extern Fixed X2Fix(Extended *xp);
extern Extended Frac2X(Fract x);
extern Fract X2Frac(Extended *xp);
extern pascal trap Fixed C_R_X2Fix(extended80 *x);
PASCAL_TRAP(R_X2Fix, 0xA844);
extern pascal trap Fract C_R_X2Frac(extended80 *x);
PASCAL_TRAP(R_X2Frac, 0xA846);
}

#endif /* _TOOLBOX_UTIL_H_ */
