#if !defined (_CRUFT_H_)
#define _CRUFT_H_

/* These are old #defines, for backwards compatibility, or #defines that
 * I don't understand why we want them.
 */

#if !defined (BINCOMPAT)
# define BINCOMPAT
#endif

#if !defined (PRIVATE)
# define PRIVATE
#endif

#if !defined (PUBLIC)
# define PUBLIC
#endif

#if !defined (pascal)
# define pascal
#endif

#if !defined (trap)
# define trap
#endif

#if !defined (a0trap)
# define a0trap
#endif

#if !defined (OSASSIGN)
# define OSASSIGN(d, s) ((d) = (s))
#endif

#define TRAPBEGIN()
#define TRAPEND()

#endif  /* !_CRUFT_H_ */
