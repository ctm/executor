#if !defined(__WIN_DONGLE_H__)
#define __WIN_DONGLE_H__

#define SENTINEL_DLL "sp32w.dll"
#define SENTINEL_STRING "ashi"
#define SENTINEL_FAMILY "AH"

#define HASP_DLL "haspms32.dll"

extern int dongle_query(uint32 *valuep);

extern char *ROMlib_DongleFamily;

#endif
