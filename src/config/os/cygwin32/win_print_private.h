/* All the information in a win_print_info_t can be derived from
   the return of a PrintDlg call */

typedef struct
{
    HDC hDC;
    int hres; /* dots per inch */
    int vres; /* dots per inch */
    int printx; /* dots */
    int printy; /* dots */
    int physx; /* points */
    int physy; /* points */
    short orientation;
    float scale;
    short copies;
    int rowwidth;
    int nbytes;
} win_print_info_t;

/* The information in a win_print_buf_t is related to the memory
   management games we play so we can have a buffer into which
   we can read bits and quickly image them */

typedef struct
{
    char *bufp;
    HANDLE dib_hand;
    HBITMAP bmap;
    HDC hDC;
} win_print_buf_t;

typedef struct
{
    int current_page_printing;
    char *current_gs_outfile;
    HANDLE file_hand;
} win_print_file_t;

typedef struct win_print_str
{
    win_print_info_t info;
    win_print_buf_t buf;
    win_print_file_t file;
} win_print_t;

#if !defined(NELEM)
#define NELEM(x) (sizeof(x) / sizeof(x)[0])
#endif

#if !defined(MIN)
#define MIN(x, y)          \
    ({                     \
        decltype(x) _x;    \
        decltype(y) _y;    \
                           \
        _x = (x);          \
        _y = (y);          \
        _x < _y ? _x : _y; \
    })
#endif

#if !defined(MAX)
#define MAX(x, y)          \
    ({                     \
        decltype(x) _x;    \
        decltype(y) _y;    \
                           \
        _x = (x);          \
        _y = (y);          \
        _x > _y ? _x : _y; \
    })
#endif

enum
{
    NO_BIT_BLT = 32700,
    MALLOC_FAILED,
    NIL_HANDLE,
    NO_DIBTODEV,
    GSDLL_INIT_FAILED,
};
