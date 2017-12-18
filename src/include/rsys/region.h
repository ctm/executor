#if !defined(_REGION_H_)
#define _REGION_H_

/* region.h
   $Id: region.h 63 2004-12-24 18:19:43Z ctm $ */

#define RGN_SIZE_MASK_X (CWC(0x7FFF))
#define RGN_SPECIAL_FLAG_X (CWC(0x8000))

#define RGNP_SIZE_X(rgnp) ((rgnp)->rgnSize & RGN_SIZE_MASK_X)
#define RGNP_SPECIAL_P(rgnp) \
    (!!((rgnp)->rgnSize & RGN_SPECIAL_FLAG_X))
#define RGNP_SMALL_P(rgnp) \
    (RGNP_SIZE_X(rgnp) == RGN_SMALL_SIZE_X)

#define RGNP_SET_SIZE_AND_SPECIAL(rgnp, size, special_p)     \
    ((void)((rgnp)->rgnSize = ((CWV(size) & RGN_SIZE_MASK_X) \
                               | ((special_p)                \
                                      ? RGN_SPECIAL_FLAG_X   \
                                      : CWC(0)))))

#define RGNP_SET_SMALL(rgnp) \
    RGNP_SET_SIZE_AND_SPECIAL(rgnp, RGN_SMALL_SIZE, false)

#define RGNP_SET_SIZE(rgnp, size)                               \
    ((void)({                                                   \
        RgnPtr __rgnp = (rgnp);                                 \
        int16_t __orig_size_x;                                    \
                                                                \
        __orig_size_x = __rgnp->rgnSize;                        \
        __rngp->rgnSize = ((__orig_size_x & RGN_SPECIAL_FLAG_X) \
                           | (CWV(size) & RGN_SIZE_MASK_X));    \
    }))
#define RGNP_SET_SPECIAL(rgnp, special_p)                 \
    ((void)({                                             \
        RgnPtr __rgnp = (rgnp);                           \
                                                          \
        if(special_p)                                     \
            __rgnp->rgnSize.raw_or(RGN_SPECIAL_FLAG_X);   \
        else                                              \
            __rgnp->rgnSize.raw_and(~RGN_SPECIAL_FLAG_X); \
    }))

#define RGNP_BBOX(rgnp) ((rgnp)->rgnBBox)

#define RGNP_DATA(rgnp) ((INTEGER *)(rgnp) + 5)

#define RGNP_SIZE(rgnp) (CW(RGNP_SIZE_X(rgnp)))

#define RGN_SIZE_X(rgn) (RGNP_SIZE_X(STARH(rgn)))
#define RGN_SPECIAL_P(rgn) (RGNP_SPECIAL_P(STARH(rgn)))
#define RGN_SMALL_P(rgn) (RGNP_SMALL_P(STARH(rgn)))

#define RGN_SET_SMALL(rgn) \
    RGNP_SET_SMALL(STARH(rgn))

#define RGN_SET_SIZE_AND_SPECIAL(rgn, size, special_p) \
    (RGNP_SET_SIZE_AND_SPECIAL(STARH(rgn), size, special_p))
#define RGN_SET_SIZE(rgn, size) \
    (RGNP_SET_SIZE(STARH(rgn), size))
#define RGN_SET_SPECIAL(rgn, special_p) \
    (RGNP_SET_SPECIAL(STARH(rgn), special_p))

#define RGN_BBOX(rgn) (RGNP_BBOX(STARH(rgn)))

#define RGN_SIZE(rgn) (CW(RGN_SIZE_X(rgn)))

#define RGN_DATA(rgn) (RGNP_DATA(STARH(rgn)))

#endif /* !_REGION_H_ */
