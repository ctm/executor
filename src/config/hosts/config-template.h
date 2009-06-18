/* define `NEED_MALLOC' if `malloc ()', `realloc ()' and `free ()'
   should be defined.  they will be implemented in terms of the
   macintosh memory manager routines */
/* #define NEED_MALLOC */

/* define `MMAP_LOW_GLOBALS' if the zero page needs to be `mmap ()'ed
   for the low globals.  this simply forces `main ()' to call
   `mmap_lowglobals ()' immediately.  `mmap_lowglobals ()' should be
   defined in a target-specific c file */
/* #define MMAP_LOW_GLOBALS */
