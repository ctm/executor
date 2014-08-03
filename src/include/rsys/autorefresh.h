#if !defined (_RSYS_SCREENCHECKSUM_H_)
#define _RSYS_SCREENCHECKSUM_H_

namespace Executor {
/* Milliseconds between checks for refresh mode. */
#define AUTOREFRESH_CHECK_MSECS 1000

#define NUM_AUTOREFRESH_STRIPS 8U

extern boolean_t do_autorefresh_p;
extern void note_executor_changed_screen (int top, int bottom);
extern boolean_t autodetect_refresh (void);
}
#endif /* !_RSYS_SCREENCHECKSUM_H_ */
