#if !defined (__WIN_DISK_H__)
#define __WIN_DISK_H__

extern int dosdisk_open (int disk, LONGINT *bsizep, drive_flags_t *flagsp);
extern int dosdisk_close (int disk, boolean_t eject_p);
extern off_t dosdisk_seek (int disk, off_t where, int unused);
extern int dosdisk_read (int disk, void *buf, int num_bytes);
extern int dosdisk_write (int disk, const void *buf, int num_bytes);
extern boolean_t is_win_nt (void);
extern uint32 win_GetLogicalDriveStrings (size_t size, char *buf);
extern boolean_t win_direct_accessible_disk (const char *p);
extern boolean_t win_access (const char *drive);

#endif
