#if !defined (__WIN_PRINT_H__)
#define __WIN_PRINT_H__

typedef enum
{
  WIN_PRINT_PORTRAIT,
  WIN_PRINT_LANDSCAPE,
} orientation_t;

typedef struct win_print_str *win_printp_t;

extern boolean_t get_info (win_printp_t *wpp,
		      int physx, int physy,
		      orientation_t orientation,
		      int copies,
		      uint32 *last_errorp);

extern boolean_t print_file (win_printp_t wp, const char *spool_namep,
			     uint32 *last_errorp);

#define INCHES(x) ((x) * 72)
#define MMETERS(x) ((x) * 2.835)

#define GSDLLAPI CALLBACK _export

enum
{
  GSDLL_STDIN=1,
  GSDLL_STDOUT,
  GSDLL_DEVICE,
  GSDLL_SYNC,
  GSDLL_PAGE,
  GSDLL_SIZE,
  GSDLL_POLL,
};

typedef int (*GSDLL_CALLBACK)(int, char *, unsigned long);

extern char *get_gs_dll (char **libstringp);

extern void set_gs_gestalt_info (void);

extern void complain_if_no_ghostscript (void);

#endif
