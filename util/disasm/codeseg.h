#ifndef _CODESEG_H_
#define _CODESEG_H_

/* This struct defines a code segment. */
typedef struct _codeseg_t
{
  int segment;
  unsigned long num_code_bytes;
  unsigned char *code;
  unsigned long disasm_start;
  struct _codeseg_t *next;
} codeseg_t;

extern codeseg_t *read_code_segments (const char *file);
extern long read_long (const unsigned char *p);
extern short read_short (const unsigned char *p);

#endif  /* !_CODESEG_H_ */
