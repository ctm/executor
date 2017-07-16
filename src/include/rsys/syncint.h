#if !defined (_syncint_h_)
#define _syncint_h_

namespace Executor {
extern int syncint_init (void);
extern void syncint_post (unsigned long usecs);
}

#endif /* !_syncint_h_ */
