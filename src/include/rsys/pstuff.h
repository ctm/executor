#if !defined (_PSTUFF_H_)
#define _PSTUFF_H_

namespace Executor {
extern toolstuff_t pstuff[];
extern osstuff_t osstuff[];

#define P_ROMlib_mytrack		(pstuff[ 0].orig)
#define P_ROMlib_stdftrack		(pstuff[ 1].orig)
#define P_ROMlib_myfilt			(pstuff[ 2].orig)
#define P_ROMlib_stdffilt		(pstuff[ 3].orig)
#define P_ROMlib_numsonlyfilterproc	(pstuff[ 4].orig)
#define P_ROMlib_stlfilterproc		(pstuff[ 5].orig)
#define P_ROMlib_wakeup			(pstuff[ 6].orig)
#define P_ROMlib_vcatch			(pstuff[ 7].orig)
#define P_ROMlib_dotext			(pstuff[ 8].orig)
#define P_handle_refresh		(pstuff[ 9].orig)

#define P_ROMlib_mysound		(pstuff[10].orig)
#define P_ROMlib_myjobproc		(pstuff[11].orig)
#define P_ROMlib_mystlproc		(pstuff[12].orig)

#define P_cdef0				(pstuff[13].orig)
#define P_cdef16			(pstuff[14].orig)
#define P_ldef0				(pstuff[15].orig)
#define P_mdef0				(pstuff[16].orig)
#define P_mbdf0				(pstuff[17].orig)
#define P_wdef0				(pstuff[18].orig)
#define P_wdef16			(pstuff[19].orig)
#define P_snth5				(pstuff[20].orig)

#define P_IUMagString			(pstuff[21].orig)	/* Pack6 */
#define P_PrStlInit			(pstuff[22].orig)	/* PrGlue */
#define P_PrJobInit			(pstuff[23].orig)	/* PrGlue */

#define P_StdText			(pstuff[24].orig)	/* This */
#define P_StdLine			(pstuff[25].orig)	/* block */
#define P_StdRect			(pstuff[26].orig)	/* contains */
#define P_StdOval			(pstuff[27].orig)	/* routines */
#define P_StdRRect			(pstuff[28].orig)	/* that have */
#define P_StdArc			(pstuff[29].orig)	/* their own */
#define P_StdRgn			(pstuff[30].orig)	/* traps */
#define P_StdPoly			(pstuff[31].orig)
#define P_StdBits			(pstuff[32].orig)
#define P_StdComment			(pstuff[33].orig)
#define P_StdTxMeas			(pstuff[34].orig)
#define P_StdPutPic			(pstuff[35].orig)

#define P_FMSwapFont			(pstuff[36].orig)
#define P_InitCursor			(pstuff[37].orig)

#define P_textasPS			(pstuff[38].orig)
#define P_PrText			(pstuff[39].orig)
#define P_PrLine			(pstuff[40].orig)
#define P_PrRect			(pstuff[41].orig)
#define P_PrRRect			(pstuff[42].orig)
#define P_PrOval			(pstuff[43].orig)
#define P_PrArc				(pstuff[44].orig)
#define P_PrPoly			(pstuff[45].orig)
#define P_PrRgn				(pstuff[46].orig)
#define P_PrBits			(pstuff[47].orig)
#define P_PrComment			(pstuff[48].orig)
#define P_PrTxMeas			(pstuff[49].orig)
#define P_PrGetPic			(pstuff[50].orig)
#define P_PrPutPic			(pstuff[51].orig)
#define P_donotPrText			(pstuff[52].orig)
#define P_donotPrLine			(pstuff[53].orig)
#define P_donotPrRect			(pstuff[54].orig)
#define P_donotPrRRect			(pstuff[55].orig)
#define P_donotPrOval			(pstuff[56].orig)
#define P_donotPrArc			(pstuff[57].orig)
#define P_donotPrPoly			(pstuff[58].orig)
#define P_donotPrRgn			(pstuff[59].orig)
#define P_donotPrBits			(pstuff[60].orig)
#define P_donotPrGetPic			(pstuff[61].orig)
#define P_donotPrPutPic			(pstuff[62].orig)

#define P_ROMlib_filebox		(pstuff[63].orig)
#define P_StdGetPic			(pstuff[64].orig)

#define P_flushcache			(pstuff[65].orig)
#define P_Key1Trans			(pstuff[66].orig)
#define P_Key2Trans			(pstuff[67].orig)

#define P_ROMlib_licensefilt		(pstuff[68].orig)
#define P_unixmount			(pstuff[69].orig)
#define P_GestaltTablesOnly		(pstuff[70].orig)
#define P_sound_timer_handler		(pstuff[71].orig)
#define P_adb_service_stub		(pstuff[72].orig)

#define P_cdef1008			(pstuff[73].orig)
#define P_bad_trap_unimplemented	(pstuff[74].orig)

#define P_pack8_unknown_selector	(pstuff[75].orig)
#define P_PhysicalGestalt		(pstuff[76].orig)

#define P_HideCursor			(pstuff[77].orig)
#define P_ShowCursor			(pstuff[78].orig)
#define P_ShieldCursor			(pstuff[79].orig)
#define P_SetCursor			(pstuff[80].orig)
#define P_ObscureCursor			(pstuff[81].orig)

#define P_Unknown574			(pstuff[82].orig)

#define P_ROMlib_circle_ok		(pstuff[83].orig)
#define P_ROMlib_orientation		(pstuff[84].orig)

#define P_new_draw_scroll		(pstuff[85].orig)
#define P_new_pos_ctl			(pstuff[86].orig)
}
#endif /* !defined(_PSTUFF_H_) */
