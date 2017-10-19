#if !defined (__rsys_icon_h__)
#define __rsys_icon_h__

#define N_SUITE_ICONS 6
namespace Executor {
/* NOTE:  Cotton didn't really use a struct and I'm not about to munch his
   code, especially since we don't know the real layout and this is subject
   to change.  I just wanted to paste in the flags field so we can drop a
   label in there. */

struct cotton_suite_layout_t { GUEST_STRUCT;
    GUEST< Handle[N_SUITE_ICONS]> icons;
    GUEST< INTEGER> label;
};
}
#endif /* !defined (__rsys_icon_h__) */
