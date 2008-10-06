#if !defined (_I486_LINUX_H_)
#define _I486_LINUX_H_

/* define `SYN68K' if the target uses the sythetic cpu */
#if !defined (SYN68K)
#define SYN68K
#endif /* !SYN68K */

/* define `MMAP_LOW_GLOBALS' if the zero page needs to be
   `mmap ()'ed for the low globals (see main:main.c) */
#define MMAP_LOW_GLOBALS

/* define `REINSTALL_SIGNAL_HANDLER' if signal handlers are
   de-installed after the signals occur, and require reinstallation */
#define REINSTALL_SIGNAL_HANDLER

/* These functions don't exist in the math library, so use some
 * approximately correct versions of our own.
 */
#define NEED_SCALB
#define NEED_LOGB

extern void mmap_lowglobals (void);

#endif /* !I486_LINUX_H_ */
