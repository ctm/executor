#if !defined(__RSYS_PRINT__)
#define __RSYS_PRINT__

/*
 * Copyright 1989 - 1997 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: print.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "rsys/pstuff.h"
#include "DialogMgr.h"
#include "PrintMgr.h"
#include "rsys/nextprint.h"
#include "rsys/ini.h"

#if defined (CYGWIN32)
#include "win_print.h"

extern win_printp_t ROMlib_wp;
#endif
namespace Executor {
enum
{
  postscriptbegin = 190,
  postscriptend = 191,
  postscripthandle = 192,
  postscriptfile = 193,
  postscripttextis = 194,
  psbeginnosave= 196,
  rotatebegin = 200,
  rotateend = 201,
  rotatecenter = 202,
  textbegin = 150,
  textend = 151,
  textcenter = 154,
};

enum
{
  LAYOUT_OK_NO = 1,
  LAYOUT_PRINTER_NAME_NO = 3,
  LAYOUT_PAPER_NO = 5,
  LAYOUT_PRINTER_TYPE_NO = 6,
  LAYOUT_PORTRAIT_ICON_NO = 8,
  LAYOUT_LANDSCAPE_ICON_NO = 9,
  LAYOUT_PRINTER_TYPE_LABEL_NO = 11,
  LAYOUT_CIRCLE_OK_NO = 12,
  LAYOUT_PORTRAIT_NO = 13,
  LAYOUT_LANDSCAPE_NO = 14,
  LAYOUT_PORT_LABEL_NO = 15,
  LAYOUT_PORT_MENU_NO = 16,
  LAYOUT_FILENAME_LABEL_NO= 17,
  LAYOUT_FILENAME_NO = 18,
};

enum
{
  PRINT_OK_NO = 1,
  PRINT_COPIES_LABEL_NO = 6,
  PRINT_COPIES_BOX_NO = 7,
  PRINT_ALL_RADIO_NO = 9,
  PRINT_FROM_RADIO_NO = 10,
  PRINT_FIRST_BOX_NO = 11,
  PRINT_LAST_BOX_NO = 13,
  PRINT_CIRCLE_OK_NO = 14,
};

extern comGrafPort printport;
extern INTEGER ROMlib_printresfile;
extern LONGINT pagewanted;
extern char ROMlib_needtorestore;
extern char ROMlib_suppressclip;

extern char *ROMlib_spool_file;
extern ini_key_t ROMlib_printer;
extern ini_key_t ROMlib_port;

extern boolean_t substitute_fonts_p;

enum { GetRslData = 4, SetRsl, DraftBits, NoDraftBits, GetRotn };

struct TGnlData { GUEST_STRUCT;
    GUEST< INTEGER> iOpCode;
    GUEST< INTEGER> iError;
    GUEST< LONGINT> lReserved;
};

struct TRslRg { GUEST_STRUCT;
    GUEST< INTEGER> iMin;
    GUEST< INTEGER> iMax;
};

struct TRslRec { GUEST_STRUCT;
    GUEST< INTEGER> iXRsl;
    GUEST< INTEGER> iYRsl;
};

struct TGetRslBlk { GUEST_STRUCT;
    GUEST< INTEGER> iOpCode;
    GUEST< INTEGER> iError;
    GUEST< LONGINT> lReserved;
    GUEST< INTEGER> iRgType;
    GUEST< TRslRg> xRslRg;
    GUEST< TRslRg> yRslRg;
    GUEST< INTEGER> iRslRecCnt;
    GUEST< TRslRec[27]> rgRslRec;
};

struct TSetRslBlk { GUEST_STRUCT;
    GUEST< INTEGER> iOpCode;
    GUEST< INTEGER> iError;
    GUEST< LONGINT> lReserved;
    GUEST< THPrint> hPrint;
    GUEST< INTEGER> iXRsl;
    GUEST< INTEGER> iYRsl;
};

typedef struct TTxtPicRec { GUEST_STRUCT;
    GUEST< Byte> tJus;
    GUEST< Byte> tFlop;
    GUEST< INTEGER> tAngle;
    GUEST< Byte> tLine;
    GUEST< Byte> tCmnt;
    GUEST< Fixed> tAngleFixed;
} *TTxtPicPtr;

#define TEXTPIC_JUST(h)          (HxX (h, tJus))
#define TEXTPIC_FLOP(h)          (HxX (h, tFlop))
#define TEXTPIC_ANGLE_X(h)       (HxX (h, tAngle))
#define TEXTPIC_ANGLE(h)         (CW (TEXTPIC_ANGLE_X (h)))
#define TEXTPIC_LINE(h)          (HxX (h, tLine))
#define TEXTPIC_COMMENT(h)       (HxX (h, tCmnt))
#define TEXTPIC_ANGLE_FIXED_X(h) (HxX (h, tAngleFixed))
#define TEXTPIC_ANGLE_FIXED(h)   (CL (TEXTPIC_ANGLE_FIXED_X (h)))

MAKE_HIDDEN(TTxtPicPtr);
typedef HIDDEN_TTxtPicPtr *TTxtPicHdl;

enum
{
  tJusNone = 0,
  tJusLeft = 1,
  tJusCenter = 2,
  tJusRight = 3,
  tJusFull = 4
};

enum
{
  tFlipNone = 0,
  tFlipHorizontal = 1,
  tFlipVertical = 2,
};

typedef struct TCenterRec { GUEST_STRUCT;
    GUEST< Fixed> y;
    GUEST< Fixed> x;
} *TCenterRecPtr;

#define TEXTCENTER_Y_X(h) (HxX (h, y))
#define TEXTCENTER_Y(h)   (CL (TEXTCENTER_Y_X (h)))
#define TEXTCENTER_X_X(h) (HxX (h, x))
#define TEXTCENTER_X(h)   (CL (TEXTCENTER_X_X (h)))

MAKE_HIDDEN(TCenterRecPtr);
typedef HIDDEN_TCenterRecPtr *TCenterRecHdl;

extern std::string ROMlib_document_paper_sizes;
extern ini_key_t ROMlib_paper_orientation;
extern std::string ROMlib_paper_size;
extern std::string ROMlib_paper_size_name;
extern std::string ROMlib_paper_size_name_terminator;
extern int ROMlib_rotation;
extern int ROMlib_translate_x;
extern int ROMlib_translate_y;
extern int ROMlib_paper_x;
extern int ROMlib_paper_y;

/* optional resolution made available via GetRslData call */
extern INTEGER ROMlib_optional_res_x, ROMlib_optional_res_y;

/* actual resolution chosen -- currently either 72dpi (default) or
   the ROMlib_optional_res */

extern int ROMlib_resolution_x;
extern int ROMlib_resolution_y;

extern boolean_t error_parse_print_size (const char *size_string);

enum { NoSuchRsl = 1, OpNotImpl };

extern boolean_t ROMlib_pick_likely_print_name(StringPtr name);

extern void ROMlib_trytomatch(char *retval, LONGINT index);

extern void C_ROMlib_myjobproc( DialogPtr dp, INTEGER itemno );

extern void C_ROMlib_mystlproc( DialogPtr dp, INTEGER itemno );

extern BOOLEAN C_ROMlib_numsonlyfilterproc( DialogPeek dp,
					      EventRecord *evt, INTEGER *ith );

extern BOOLEAN C_ROMlib_stlfilterproc( DialogPeek dp,
					      EventRecord *evt, INTEGER *ith );

extern void ROMlib_set_default_resolution (THPrint hPrint,
					   INTEGER vres, INTEGER hres);

extern void do_textbegin (TTxtPicHdl h);
extern void do_textcenter (TCenterRecHdl h);
extern void do_textend (void);

extern void print_reinit (void);

extern pascal void C_ROMlib_circle_ok (DialogPeek dp, INTEGER which);
extern pascal void C_ROMlib_orientation (DialogPeek dp, INTEGER which);

extern void printer_init (void);
extern void update_printing_globals (void);
extern char *cstring_from_str255 (Str255 text);

extern void disable_stdtext (void);
extern void enable_stdtext (void);

#define WIN32_TOKEN (ROMlib_win32_token ? ROMlib_win32_token : "Win32")


extern void ROMlib_rotatebegin (LONGINT flippage, LONGINT angle);
extern void ROMlib_rotatecenter (double yoffset, double xoffset);
extern void ROMlib_rotateend (void);
extern void ROMlib_gsave (void);
extern void ROMlib_grestore (void);

extern void ROMlib_acknowledge_job_dialog (THPrint thprint);
}

extern "C" {
extern char *ROMlib_win32_token;
extern uint32 ROMlib_PrDrvrVers;

extern char *ROMlib_new_printer_name;
extern char *ROMlib_new_label;

extern FILE *ROMlib_printfile;
}
#endif /* !defined(__RSYS_PRINT__) */
