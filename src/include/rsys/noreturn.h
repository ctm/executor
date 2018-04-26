#if !defined(_RSYS_NORETURN_H_)
#define _RSYS_NORETURN_H_

/* Macros to declare a function as not returning.  Put _NORET_1_
 * before the function declaration (but after the extern), and
 *_NORET_2_ after the end.
 */

#if !defined(__GNUC__)
#define _NORET_1_
#define _NORET_2_
#elif __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5)
#define _NORET_1_ volatile
#define _NORET_2_
#else /* GNUC > 2.4 */
#define _NORET_1_
#define _NORET_2_ __attribute__((noreturn))
#endif /* GNUC > 2.4 */

#endif /* !_RSYS_NORETURN_H_ */
