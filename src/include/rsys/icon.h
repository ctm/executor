#if !defined (__rsys_icon_h__)
#define __rsys_icon_h__

#define N_SUITE_ICONS 6
namespace Executor {
/* NOTE:  Cotton didn't really use a struct and I'm not about to munch his
   code, especially since we don't know the real layout and this is subject
   to change.  I just wanted to paste in the flags field so we can drop a
   label in there. */

typedef struct PACKED
{
  Handle icons[N_SUITE_ICONS];
  INTEGER label;
}
cotton_suite_layout_t;
}
#endif /* !defined (__rsys_icon_h__) */
