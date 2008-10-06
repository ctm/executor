#if !defined (_RSYS_RELEASE_H_)
#define _RSYS_RELEASE_H_

/* There are four kinds of builds:
 * RELEASE_INTERNAL:
 *    for internal use ONLY, and should not be released to the public.
 * RELEASE_PRE_BETA:
 *    a time-limited demo copy that can be registered to remove the limit.
 * RELEASE_DEMO:
 *    a "crippled" demo copy that cannot be registered.
 * RELEASE_COMMERCIAL:
 *    the full commercial release, for sale but not to be made publicly
 *    available.
 *
 * Other files can expect that Exactly one of these will be #defined.
 * If none are #defined, RELEASE_PRE_BETA will be #defined here.
 *
 * Certain releases enable certain features:
 *
 * DISPLAY_SPLASH_INFO_BOX:
 *    display the old syserr demo/info box on startup
 * DISPLAY_SPLASH_SCREEN:
 *    display a fancy splash screen while loading
 * SUPPORT_ABOUT_EXECUTOR_BOX:
 *    make an "about Executor" menu available that brings up a special
 *    about box.
 * REGISTER_BEFORE_USABLE:
 *    Executor must be registered before it can be used at all.
 * SHORT_TICKS:
 *    low-memory global Ticks is treated as a short and the high 2 bytes
 *    are used (this means if crackers disable the "Time's Up" message,
 *    subtle things will go awry)
 * PRINT_DEMO_WATERMARK:
 *    All output comes with "Demo" printed diagonally down the page
 * DISABLE_FLOPPY_WRITES:
 *    Executor will read HD floppies but not write to them
 * DISABLE_SCSI_WRITES:
 *    Executor will read SCSI drives but not write to them
 * DISABLE_COMMAND_KEY_EQUIVS:
 *    Command-key shortcuts won't work in the demo version
 * DISPLAY_DEMO_ON_MENU_BAR:
 *    Menu Bar says "DEMO" in red, which can be clicked on to get a little
 *    information about where to find the real thing.
 * TIME_OUT n
 *    Executor time's out in n minutes.
 * SHORT_TICKS_SHIFT n
 *    The number of bits to right shift the high short of Ticks.  Could
 *    be computed from TIME_OUT if we could do log2 in #defines.  Not needed
 *    for timeouts up to 18 minutes, but needs to be 1 for 30 minutes.
 */


/*
 * NOTE: Internal builds are now flagged as such
 */                

#if defined (RELEASE_INTERNAL)

#define DISPLAY_DEMO_ON_MENU_BAR
#define DEMO_PREFIX "INTERNAL "

#else

#define DEMO_PREFIX ""

#endif

#if (!defined (RELEASE_DEMO) 		\
      && !defined (RELEASE_COMMERCIAL)	\
      && !defined (RELEASE_PRE_BETA)	\
      && !defined (RELEASE_INTERNAL))
#define RELEASE_PRE_BETA
#endif

#if (1 != (defined (RELEASE_DEMO)		\
	   + defined (RELEASE_COMMERCIAL)	\
	   + defined (RELEASE_PRE_BETA)		\
	   + defined (RELEASE_INTERNAL)))
#error "Exactly one release type must be specified."
#endif

#if defined (RELEASE_PRE_BETA)
#define DISPLAY_SPLASH_INFO_BOX
#define TIME_OUT
#endif

#if !defined (RELEASE_PRE_BETA)
#define DISPLAY_SPLASH_SCREEN
#define SUPPORT_ABOUT_EXECUTOR_BOX
#endif

#if defined (RELEASE_DEMO)

     /* #define SHORT_TICKS */
#define PRINT_DEMO_WATERMARK
#define DISABLE_FLOPPY_WRITES
#define DISABLE_SCSI_WRITES
#define DISPLAY_DEMO_ON_MENU_BAR

#define TIME_OUT_DAYS (ROMlib_days_of_demop ? ROMlib_days_of_demop->val : 30)
#define DISABLE_COMMAND_KEY_EQUIVS

#endif

#if defined (RELEASE_COMMERCIAL)
#define REGISTER_BEFORE_USABLE
#endif

/* Set up the bit mask for which classes of debugging information
 * we can generate.  This can be overridden on the command line
 * by defining ERROR_SUPPORTED_MASK there.
 */
#if !defined (ERROR_SUPPORTED_MASK)
# if defined (RELEASE_INTERNAL)
#  define ERROR_SUPPORTED_MASK (~0)  /* all errors */
# else /* !RELEASE_INTERNAL */
#  define ERROR_SUPPORTED_MASK				\
	(ERROR_BIT_MASK (ERROR_UNIMPLEMENTED)		\
	 | ERROR_BIT_MASK (ERROR_UNEXPECTED)		\
	 | ERROR_BIT_MASK (ERROR_TRACE_INFO)		\
	 | ERROR_BIT_MASK (ERROR_FILESYSTEM_LOG))
# endif /* !RELEASE_INTERNAL */
#endif /* !ERROR_SUPPORTED_MASK */

#if defined (RELEASE_INTERNAL)
#define SUPPORT_LOG_ERR_TO_RAM
#endif

#if defined (EXPERIMENTAL)
#define ALLOW_MOVABLE_MODAL
#endif

#endif /* !_RSYS_RELEASE_H_ */
