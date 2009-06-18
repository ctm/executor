#if !defined (_I386_MSDOS_GO32_H_)
#define _I386_MSDOS_GO32_H_

#if !defined (SYN68K)
#define SYN68K
#endif

/* Specify which extra functions we need in float.h. */
#define NEED_RINT
#define NEED_LOGB
#define HAVE_LOG2
#define NEED_SCALB
#define NEED_LOG1P

#endif /* !_I386_MSDOS_GO32_H_ */
