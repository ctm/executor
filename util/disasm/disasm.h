#ifndef _DISASM_H_
#define _DISASM_H_

#include "codeseg.h"

extern char *disasm (const unsigned char *mem, unsigned long num_bytes,
		     unsigned long starting_address);
extern char *disasm_code_segments (codeseg_t *seg, unsigned long *entry_point);
extern char *extract_field (const char *c, int field_num, char *buf);

#undef FALSE
#undef TRUE
#define FALSE 0
#define TRUE 1

#endif  /* !_DISASM_H_ */
