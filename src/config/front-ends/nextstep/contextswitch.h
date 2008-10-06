#if !defined (_CONTEXTSWITCH_H_)
#define _CONTEXTSWITCH_H_

extern char *romlib_sp, *nextstep_sp;

extern void contextswitch( char **from_spp, char **to_spp );

extern long ROMlib_printtimeout;

#endif /* !_CONTEXTSWITCH_H_ */
