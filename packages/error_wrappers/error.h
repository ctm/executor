#if !defined(__ERROR_CHECK_H__)
#define __ERROR_CHECK_H__
/*
 * This file, along with error.c provides wrappers for various OS and
 * toolbox calls.  The wrappers automatically check to make sure the
 * routines they wrap are returning noErr.  This works even for
 * routines that require calling special error number accessor
 * routines (i.e. Memory and Resource manager routines).
 *
 * In addition to wrapping functions, the "HANDLE_CHECK" macro is
 * provided to make it trivial to check to make sure that a Handle
 * really contains the right number of bytes.  This may help in
 * catching Executor bugs and catching where a handle has been purged.
 * If you regularly use handles that contain extra information at the
 * end, you may want to split it up into HANDLE_CHECK_EQ to test for
 * equality and HANDLE_CHECK_GE to test to make sure you have at least
 * enough room.
 *
 * Care has been taken to NEVER evaluate arguments more than once, so
 * you don't have to worry about a wrapper inadvertenly breaking your
 * code because it wraps a routine invocation with a parameter that
 * contains a side-effect.  This has been done by using global
 * variables.  As such, these routines CAN NOT be used in any code
 * that is asynchronously called, such as ioCompletion, VBL or Time
 * Manager callbacks.  Similarly, these routines can't be used in
 * multi-threaded code.
 */

#include "MacTypes.h" /* This should be whatever include files are needed to
			 get the type definitions for all the routines we
			 wrap in this file */

extern int die_on_error; /* whether or not we want to pay attention to errors.
			    zero means behave exactly as if we weren't wrapping
			    errors.  non-zero means we want to log some
			    information and then die */

extern long int last_error; /* last error a wrapped or checked routine has
			       encountered, or noErr if no error was found */

extern const char *last_error_file; /* filename of the source file containing
				       the trap that saw the last errror */

extern int last_error_line; /* line number of the last error */


/* Declarations for each of the wrapper routines we use for monitoring.
   Routines which return anything other than void must also pass in the
   filename and the line number where they were invoked.  See error.c for
   a more complete explanation of why this is done. */

extern void _DisposHandle (Handle);
extern Handle _NewHandle (Size s, const char *file, int line);

#define badSizeErr (-512) /* the value we log when a size doesn't match */

/*
 * All our checking is routed through ERROR_CHECK.  We can make it as
 * fancy as we want, but it's important that we only evaluate the first
 * argument once.  Since the first thing we do is copy the first argument
 * into last_error, we can still look at that value as many times as we
 * want, as long as it's done via last_error, and not n.
 */

#define ERROR_CHECK(n, file, line)			\
  ((last_error = (n)) != noErr && die_on_error ? (	\
      last_error_file = file,				\
      last_error_line = line,				\
      *(long *)-1 = 0)					\
    :							\
       noErr)

/*
 * These defines are for nesting pieces of code where you know you may
 * be invoking routines that will create errors and you want to handle
 * the errors yourself.  NOTE:  These defines have to be used in pairs,
 * and all the statements between ERROR_CHECK_INHIBIT() and its matching
 * ERROR_CHECK_RESTORE will be part of the same compound statement created
 * by ERROR_CHECK_INHIBIT.  If you jump out of this compound statement,
 * die_on_error will be left set to zero, which is bad.
 */

#define ERROR_CHECK_INHIBIT()                   \
  do                                            \
    {                                           \
      int save_die_on_error;                    \
                                                \
      save_die_on_error = die_on_error;         \
      die_on_error = 0

#define ERROR_CHECK_RESTORE()			\
      die_on_error = save_die_on_error;		\
    }						\
  while (0)

/*
 * The rest of the definitions we will want to use everywhere except
 * when compiling error.c.  Not only will our #define of OS and toolbox
 * routines (e.g. NewHandle) get in the way when compiling error.c, the
 * #defines that use __FILE__ and __LINE__ would be misleading if they
 * were used in error.c, since their use would result in an error condition
 * pointing to an error in "error.c", which is not very useful.
 */

#if !defined (__COMPILING_ERROR_C__)

#define ERROR_CHECK_INTERNAL(n) ERROR_CHECK(n, __FILE__, __LINE__)

#define MEM_CHECK() ERROR_CHECK_INTERNAL(MemError())

#define RES_CHECK() ERROR_CHECK_INTERNAL(ResError())

#define HANDLE_CHECK(h)							\
do									\
{									\
  Size __s;								\
									\
  __s = GetHandleSize (h);						\
  ERROR_CHECK_INTERNAL (__s == sizeof **(h) ? noErr : badSizeErr);	\
}									\
while (0)

#define DisposHandle(h)				\
do						\
{						\
  _DisposHandle(h);				\
  MEM_CHECK();					\
}						\
while (0)

#define NewHandle(s) _NewHandle(s, __FILE__, __LINE__)

/* TODO: wrap all the Memory manager, Resource manager calls and any other
 calls whose error returns we may want to check automatically */

#endif

#endif
