#if !defined(_dosdisk_h_)
#define _dosdisk_h_

#include <dpmi.h>
#include "dosmem.h"
#include <rsys/file.h>

extern int int13_num_to_int2f_num(int num13);

extern int dosdisk_open(int disk, LONGINT *bsizep, drive_flags_t *flagsp);
extern int dosdisk_close(int disk, bool eject_p);
extern off_t dosdisk_seek(int disk, off_t where, int unused);
extern ULONGINT dosdisk_tell(int disk);
extern int dosdisk_read(int disk, void *buf, int num_bytes);
extern int dosdisk_write(int disk, const void *buf, int num_bytes);
extern int dosdisk_find_cdrom(void);

/* Temporary space to be used as a buffer communicating with the disk. */
#define TRANSFER_BUFFER_SIZE (16 * 1024) /* Arbitrary. */

#if TRANSFER_BUFFER_SIZE > (DOS_BUF_SIZE - DOS_MIN_STACK_SPACE)
#error "Transfer buffer too big"
#endif

#endif /* Not _dosdisk_h_ */
