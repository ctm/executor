#if !defined (_DPMIMEM_H_)
#define _DPMIMEM_H_

#include <dpmi.h>

#define DPMI_PAGE_SIZE 4096

extern int __djgpp_map_physical_memory (void *_our_addr,
					unsigned long _num_bytes,
					unsigned long _phys_addr);
extern int __djgpp_set_page_attributes (void *_our_addr,
					unsigned long _num_bytes,
					unsigned short _attributes);

#endif /* !_DPMIMEM_H_ */
