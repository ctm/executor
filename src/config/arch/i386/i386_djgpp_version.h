#if !defined(__i386_djgpp_version_h__)
#define __i386_djgpp_version_h__

#if(defined(__DJGPP_MINOR__) && __DJGPP_MINOR__ == 1)

#define ADDR32
#define DATA32
#define DATA16
#define QUOTED_DATA16 ""
#define DOT_ALIGN(x) .align(1 << x)

#else

#define ADDR32 addr32
#define DATA32 data32
#define DATA16 data16
#define QUOTED_DATA16 "data16 "
#define DOT_ALIGN(x) .align x

#endif

#endif /* !defined (__i386_djgpp_version_h__) */
