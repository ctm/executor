#if !defined(__TRAPGLUE__)
#define __TRAPGLUE__

typedef struct {
    void *wheretogo;
    ULONGINT magic;
} ptocblock_t;


typedef struct {
    ptocblock_t ptoc;
    void *orig;
} toolstuff_t;

typedef struct {
    void *orig;
    void *func;
} osstuff_t;

#define TOOLBIT		(0x0800)
#define NTOOLENTRIES	(0x400)
#define NOSENTRIES	(0x100)

extern void *tooltraptable[NTOOLENTRIES];
extern void   *ostraptable[NOSENTRIES];

extern syn68k_addr_t PascalToCCall(syn68k_addr_t ignoreme, ptocblock_t *infop);


extern unsigned short mostrecenttrap;

#endif
