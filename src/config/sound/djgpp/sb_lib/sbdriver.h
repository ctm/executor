#ifndef __SB_DRIVER
#define __SB_DRIVER
#ifdef __cplusplus
extern "C" {
#endif

/* If one of the functions in this module returns an error, look here for a
   description of what happened.                                              */
extern char sb_driver_error[80];
extern volatile int sb_numInQueue;

sb_status sb_install_driver(void (*callback)());
void sb_uninstall_driver(void);

int sb_set_playback_enabled(int);
int sb_set_format(int);
void sb_enqueue_sample(const void *, int);

int sb_get_capabilities(void);

#ifdef __cplusplus
}
#endif
#endif
