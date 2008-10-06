
#ifndef _win32_h
#define _win32_h

#include <stdio.h>

#if !defined (COMPILE_FOR_HOST)
/* A replacement for stdio in a windowed environment */

#if 0
/* this gets in the way of debugging */
#define fprintf Win_Message
#endif

/* Code in win32/windriver.c */
extern void Win_Message(FILE *stream, const char *fmt, ...);
#endif

/* Function to run the Win32 message processing loop */
/* From winevents.c */
extern void process_win32_events(void);

#endif /* _win32_h */



