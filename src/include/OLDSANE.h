#if !defined(__SANE__)
#define __SANE__

/*
 * Copyright 1990, 1991 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: OLDSANE.h 63 2004-12-24 18:19:43Z ctm $
 */

typedef struct {
    INTEGER exp	PACKED;
    INTEGER man[4]	PACKED;
} extended80;

typedef struct {
    INTEGER exp	PACKED;
    INTEGER zero	PACKED;
    INTEGER man[4]	PACKED;
} extended96;

typedef struct {
    LONGINT hilong	PACKED;
    LONGINT lowlong	PACKED;
} comp;

#define SIGDIGLEN 20

typedef struct {
    INTEGER sgn	PACKED;
    INTEGER exp	PACKED;
    unsigned char sig[SIGDIGLEN]	PACKED;
} Decimal;

typedef enum { FloatDecimal, FixedDecimal = 256 } toobigdecformstyle_t;

typedef INTEGER DecFormStyle;

#define DECIMALTYPEMASK 0x0100

typedef enum {SNaN = 1, QNaN, Infinite, ZeroNum, NormalNum, DenormalNum}
								      NumClass;

typedef struct {
    DecFormStyle style	PACKED;
    INTEGER  digits	PACKED;
} DecForm;

#define Decstr char *

#define FX_OPERAND	0x0000
#define FD_OPERAND	0x0800
#define FS_OPERAND	0x1000
#define FC_OPERAND	0x3000
#define FI_OPERAND	0x2000
#define FL_OPERAND	0x2800

#define Fx2X_OPCODE	0x000E

#define FI2X	(FI_OPERAND + Fx2X_OPCODE)
/* DO NOT DELETE THIS LINE */
#if !defined (__STDC__)
extern pascal void ROMlib_Fcomp2X(); 
extern void ROMlib_Fsetenv(); 
extern void ROMlib_Fgetenv(); 
extern void ROMlib_Fprocentry(); 
extern void ROMlib_Fprocexit(); 
extern void ROMlib_Ftestxcp(); 
extern void ROMlib_FsqrtX(); 
extern void ROMlib_FscalbX(); 
extern void ROMlib_FlogbX(); 
extern void ROMlib_FabsX(); 
extern void ROMlib_FnegX(); 
extern void ROMlib_FrintX(); 
extern void ROMlib_FtintX(); 
extern void ROMlib_Fcpysgnx(); 
extern void ROMlib_Faddx(); 
extern void ROMlib_Fsubx(); 
extern void ROMlib_Fmulx(); 
extern void ROMlib_Fdivx(); 
extern void ROMlib_Fremx(); 
extern void ROMlib_Fcmpx(); 
extern void ROMlib_FcpXx(); 
extern void ROMlib_FX2x(); 
extern void ROMlib_Fx2X(); 
extern void ROMlib_Fx2dec(); 
extern void ROMlib_Fdec2x(); 
extern void ROMlib_Fclassx(); 
extern void ROMlib_FlnX(); 
extern void ROMlib_Flog2X(); 
extern void ROMlib_Fln1X(); 
extern void ROMlib_Flog21X(); 
extern void ROMlib_FexpX(); 
extern void ROMlib_Fexp2X(); 
extern void ROMlib_Fexp1X(); 
extern void ROMlib_Fexp21X(); 
extern void ROMlib_Fxpwri(); 
extern void ROMlib_Fxpwry(); 
extern void ROMlib_Fcompound(); 
extern void ROMlib_Fannuity(); 
extern void ROMlib_FsinX(); 
extern void ROMlib_FcosX(); 
extern void ROMlib_FtanX(); 
extern void ROMlib_FatanX(); 
extern void ROMlib_FrandX(); 
extern void ROMlib_Fdec2str(); 
extern void ROMlib_Fxstr2dec(); 
extern void ROMlib_Fcstr2dec(); 
extern void ROMlib_Fpstr2dec(); 
#else /* __STDC__ */
extern pascal void C_ROMlib_Fcomp2X( comp volatile sp, extended80 *
 volatile dp ); extern pascal void P_ROMlib_Fcomp2X( comp volatile sp, extended80 *
 volatile dp ); 
extern pascal trap void C_ROMlib_Fsetenv( INTEGER *volatile dp, 
 INTEGER volatile sel ); extern pascal trap void P_ROMlib_Fsetenv( INTEGER *volatile dp, 
 INTEGER volatile sel ); 
extern pascal trap void C_ROMlib_Fgetenv( INTEGER *volatile dp, 
 INTEGER volatile sel ); extern pascal trap void P_ROMlib_Fgetenv( INTEGER *volatile dp, 
 INTEGER volatile sel ); 
extern pascal trap void C_ROMlib_Fprocentry( INTEGER *volatile dp, 
 INTEGER volatile sel ); extern pascal trap void P_ROMlib_Fprocentry( INTEGER *volatile dp, 
 INTEGER volatile sel ); 
extern pascal trap void C_ROMlib_Fprocexit( INTEGER *volatile dp, 
 INTEGER volatile sel ); extern pascal trap void P_ROMlib_Fprocexit( INTEGER *volatile dp, 
 INTEGER volatile sel ); 
extern pascal trap void C_ROMlib_Ftestxcp( INTEGER *volatile dp, 
 INTEGER volatile sel ); extern pascal trap void P_ROMlib_Ftestxcp( INTEGER *volatile dp, 
 INTEGER volatile sel ); 
extern pascal trap void C_ROMlib_FsqrtX( extended80 *volatile dp, 
 unsigned short volatile sel ); extern pascal trap void P_ROMlib_FsqrtX( extended80 *volatile dp, 
 unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_FscalbX( INTEGER *volatile sp, 
 extended80 *volatile dp, unsigned short volatile sel ); extern pascal trap void P_ROMlib_FscalbX( INTEGER *volatile sp, 
 extended80 *volatile dp, unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_FlogbX( extended80 *volatile dp, 
 unsigned short volatile sel ); extern pascal trap void P_ROMlib_FlogbX( extended80 *volatile dp, 
 unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_FabsX( extended80 *volatile dp, 
 unsigned short volatile sel ); extern pascal trap void P_ROMlib_FabsX( extended80 *volatile dp, 
 unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_FnegX( extended80 *volatile dp, 
 unsigned short volatile sel ); extern pascal trap void P_ROMlib_FnegX( extended80 *volatile dp, 
 unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_FrintX( extended80 *volatile dp, 
 unsigned short volatile sel ); extern pascal trap void P_ROMlib_FrintX( extended80 *volatile dp, 
 unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_FtintX( extended80 *volatile dp, 
 unsigned short volatile sel ); extern pascal trap void P_ROMlib_FtintX( extended80 *volatile dp, 
 unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_Fcpysgnx( float *volatile sp, 
 float *volatile dp, unsigned short volatile sel ); extern pascal trap void P_ROMlib_Fcpysgnx( float *volatile sp, 
 float *volatile dp, unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_Faddx( void *volatile sp, extended80 *
 volatile dp, unsigned short volatile sel ); extern pascal trap void P_ROMlib_Faddx( void *volatile sp, extended80 *
 volatile dp, unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_Fsubx( void *volatile sp, extended80 *
 volatile dp, unsigned short volatile sel ); extern pascal trap void P_ROMlib_Fsubx( void *volatile sp, extended80 *
 volatile dp, unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_Fmulx( void *volatile sp, extended80 *
 volatile dp, unsigned short volatile sel ); extern pascal trap void P_ROMlib_Fmulx( void *volatile sp, extended80 *
 volatile dp, unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_Fdivx( void *volatile sp, extended80 *
 volatile dp, unsigned short volatile sel ); extern pascal trap void P_ROMlib_Fdivx( void *volatile sp, extended80 *
 volatile dp, unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_Fremx( void *volatile sp, 
 extended80 *volatile dp, unsigned short volatile sel ); extern pascal trap void P_ROMlib_Fremx( void *volatile sp, 
 extended80 *volatile dp, unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_Fcmpx( void *volatile sp, extended80 *
 volatile dp, unsigned short volatile sel ); extern pascal trap void P_ROMlib_Fcmpx( void *volatile sp, extended80 *
 volatile dp, unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_FcpXx( void *volatile sp, extended80 *
 volatile dp, unsigned short volatile sel ); extern pascal trap void P_ROMlib_FcpXx( void *volatile sp, extended80 *
 volatile dp, unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_FX2x( extended80 *volatile sp, void *
 volatile dp, unsigned short volatile sel ); extern pascal trap void P_ROMlib_FX2x( extended80 *volatile sp, void *
 volatile dp, unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_Fx2X( void *volatile sp, extended80 *
 volatile dp, unsigned short volatile sel ); extern pascal trap void P_ROMlib_Fx2X( void *volatile sp, extended80 *
 volatile dp, unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_Fx2dec( DecForm *volatile sp2, void *
 volatile sp, Decimal *volatile dp, unsigned short volatile sel ); extern pascal trap void P_ROMlib_Fx2dec( DecForm *volatile sp2, void *
 volatile sp, Decimal *volatile dp, unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_Fdec2x( Decimal *volatile sp, void *
 volatile dp, unsigned short volatile sel ); extern pascal trap void P_ROMlib_Fdec2x( Decimal *volatile sp, void *
 volatile dp, unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_Fclassx( void *volatile sp, INTEGER *
 volatile dp, unsigned short volatile sel ); extern pascal trap void P_ROMlib_Fclassx( void *volatile sp, INTEGER *
 volatile dp, unsigned short volatile sel ); 
extern pascal trap void C_ROMlib_FlnX( extended80 *volatile dp ); extern pascal trap void P_ROMlib_FlnX( extended80 *volatile dp); 
extern pascal trap void C_ROMlib_Flog2X( extended80 *volatile dp ); extern pascal trap void P_ROMlib_Flog2X( extended80 *volatile dp); 
extern pascal trap void C_ROMlib_Fln1X( extended80 *volatile dp ); extern pascal trap void P_ROMlib_Fln1X( extended80 *volatile dp); 
extern pascal trap void C_ROMlib_Flog21X( extended80 *volatile dp ); extern pascal trap void P_ROMlib_Flog21X( extended80 *volatile dp); 
extern pascal trap void C_ROMlib_FexpX( extended80 *volatile dp ); extern pascal trap void P_ROMlib_FexpX( extended80 *volatile dp); 
extern pascal trap void C_ROMlib_Fexp2X( extended80 *volatile dp ); extern pascal trap void P_ROMlib_Fexp2X( extended80 *volatile dp); 
extern pascal trap void C_ROMlib_Fexp1X( extended80 *volatile dp ); extern pascal trap void P_ROMlib_Fexp1X( extended80 *volatile dp); 
extern pascal trap void C_ROMlib_Fexp21X( extended80 *volatile dp ); extern pascal trap void P_ROMlib_Fexp21X( extended80 *volatile dp); 
extern pascal trap void C_ROMlib_Fxpwri( INTEGER *volatile sp, 
 extended80 *volatile dp ); extern pascal trap void P_ROMlib_Fxpwri( INTEGER *volatile sp, 
 extended80 *volatile dp ); 
extern pascal trap void C_ROMlib_Fxpwry( extended80 *volatile sp, 
 extended80 *volatile dp ); extern pascal trap void P_ROMlib_Fxpwry( extended80 *volatile sp, 
 extended80 *volatile dp ); 
extern pascal trap void C_ROMlib_Fcompound( extended80 *volatile sp2, 
 extended80 *volatile sp, extended80 *volatile dp ); extern pascal trap void P_ROMlib_Fcompound( extended80 *volatile sp2, 
 extended80 *volatile sp, extended80 *volatile dp ); 
extern pascal trap void C_ROMlib_Fannuity( extended80 *volatile sp2, 
 extended80 *volatile sp, extended80 *volatile dp ); extern pascal trap void P_ROMlib_Fannuity( extended80 *volatile sp2, 
 extended80 *volatile sp, extended80 *volatile dp ); 
extern pascal trap void C_ROMlib_FsinX( extended80 *volatile dp ); extern pascal trap void P_ROMlib_FsinX( extended80 *volatile dp); 
extern pascal trap void C_ROMlib_FcosX( extended80 *volatile dp ); extern pascal trap void P_ROMlib_FcosX( extended80 *volatile dp); 
extern pascal trap void C_ROMlib_FtanX( extended80 *volatile dp ); extern pascal trap void P_ROMlib_FtanX( extended80 *volatile dp); 
extern pascal trap void C_ROMlib_FatanX( extended80 *volatile dp ); extern pascal trap void P_ROMlib_FatanX( extended80 *volatile dp); 
extern pascal trap void C_ROMlib_FrandX( extended80 *volatile dp ); extern pascal trap void P_ROMlib_FrandX( extended80 *volatile dp); 
extern pascal trap void C_ROMlib_Fdec2str( DecForm *volatile sp2, 
 Decimal *volatile sp, Decstr volatile dp ); extern pascal trap void P_ROMlib_Fdec2str( DecForm *volatile sp2, 
 Decimal *volatile sp, Decstr volatile dp ); 
extern pascal trap void C_ROMlib_Fxstr2dec( Decstr volatile sp2, 
 INTEGER *volatile sp, Decimal *volatile dp2, Byte *volatile dp, 
 INTEGER lastchar ); extern pascal trap void P_ROMlib_Fxstr2dec( Decstr volatile sp2, 
 INTEGER *volatile sp, Decimal *volatile dp2, Byte *volatile dp, 
 INTEGER lastchar ); 
extern pascal trap void C_ROMlib_Fcstr2dec( Decstr volatile sp2, 
 INTEGER *volatile sp, Decimal *volatile dp2, Byte *volatile dp ); extern pascal trap void P_ROMlib_Fcstr2dec( Decstr volatile sp2, 
 INTEGER *volatile sp, Decimal *volatile dp2, Byte *volatile dp ); 
extern pascal trap void C_ROMlib_Fpstr2dec( Decstr volatile sp2, 
 INTEGER *volatile sp, Decimal *volatile dp2, Byte *volatile dp ); extern pascal trap void P_ROMlib_Fpstr2dec( Decstr volatile sp2, 
 INTEGER *volatile sp, Decimal *volatile dp2, Byte *volatile dp ); 
#endif /* __STDC__ */
#endif
