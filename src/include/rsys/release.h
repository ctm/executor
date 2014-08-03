#if !defined (_RSYS_RELEASE_H_)
#define _RSYS_RELEASE_H_

/*
 * There used to be different types of releases, e.g. commercial demo,
 * internal.  This header file used to set up different options
 * depending on which release we were.
 *
 * In general, all the old pain-in-the-butt options have been removed
 * and what little is left will be converted to configuration options
 * when (if) we switch to the GNU build system.
 *
 * In the meantime, there's still a little cruft in here.
 */

#undef DISPLAY_SPLASH_SCREEN

/* Set up the bit mask for which classes of debugging information
 * we can generate.  This can be overridden on the command line
 * by defining ERROR_SUPPORTED_MASK there.
 */

#if !defined (ERROR_SUPPORTED_MASK)
#  define ERROR_SUPPORTED_MASK (~0)  /* all errors */
#endif /* !ERROR_SUPPORTED_MASK */

#define SUPPORT_LOG_ERR_TO_RAM

#if defined (EXPERIMENTAL)
#define ALLOW_MOVABLE_MODAL
#endif

#endif /* !_RSYS_RELEASE_H_ */
