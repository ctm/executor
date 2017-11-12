
#if defined(THINK_C)
#include <stdio.h>
#include "Palettes.h"
#endif
namespace Executor
{
extern int dump_verbosity;
extern FILE *o_fp;

extern void dump_init(char *dst);

#define dump_handle(x) dump_handle_real((Handle)x)
extern void dump_handle_real(Handle x);

#define dump_ptr(x) dump_ptr_real((Ptr)x)
extern void dump_ptr_real(Ptr x);

extern void dump_rect(Rect *r);
extern void dump_pattern(Pattern x);
extern void dump_point(GUEST<Point> x);
extern void dump_bits16(GUEST<Bits16> data);
extern void dump_bitmap(BitMap *x, Rect *rect);
extern void dump_bitmap_null_rect(BitMap *x);
extern void dump_bitmap_data(BitMap *x, int depth, Rect *rect);
extern void dump_grafport(GrafPtr x);
extern void dump_grafport_real(GrafPtr x);
extern void dump_cgrafport_real(CGrafPtr x);
extern void dump_rgb_color(RGBColor *x);
extern void dump_ctab(CTabHandle ctab);
extern void dump_itab(ITabHandle itab);
extern void dump_pixpat(PixPatHandle pixpat);
extern void dump_pixmap_ptr(PixMapPtr x, Rect *rect);
extern void dump_pixmap(PixMapHandle pixmap, Rect *rect);
extern void dump_pixmap_null_rect(PixMapHandle pixmap);
extern void dump_theport(void);
extern void dump_gdevice(GDHandle gdev);
extern void dump_thegdevice(void);
extern void dump_maindevice(void);
extern void dump_palette(PaletteHandle palette);
extern void dump_qdprocs(QDProcsPtr x);
extern void dump_aux_win_list(void);
extern void dump_cqdprocs(CQDProcsPtr x);
extern void dump_aux_win(AuxWinHandle awh);
extern void dump_ccrsr(CCrsrHandle ccrsr);

extern void dump_wmgrport(void);
extern void dump_wmgrcport(void);

extern void dump_string(unsigned char *s);
extern void dump_string_handle(StringHandle sh);

extern void dump_window_peek(WindowPeek w);
extern void dump_dialog_peek(DialogPeek w);

extern void dump_window_list(WindowPeek w);
extern void dump_rgn(RgnHandle rgn);

extern void dump_finish(void);

extern void dump_set_field(int field);
extern void dump_clear_field(int field);
}
