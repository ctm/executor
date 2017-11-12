#include "rsys/common.h"
#include <stdio.h>
#include <assert.h>
#include <mach-o/loader.h>

unsigned long dont_swap_long(unsigned long x) { return x; }
unsigned short dont_swap_short(unsigned short x) { return x; }

int main(int argc, char *argv[])
{
    struct mach_header head;
    struct segment_command seg;
    int i;
    FILE *fp;
    long offset, saveoffset;

    unsigned long (*swap_long)(unsigned long);
    unsigned short (*swap_short)(unsigned short);

    assert(argc == 3);
    if(!(fp = fopen(argv[1], "r+")))
    {
        fprintf(stderr, "couldn't open '%s'\n", argv[1]);
        exit(3);
    }

    if(fread(&head, sizeof(head), 1, fp) != 1)
    {
        fprintf(stderr, "couldn't read header\n");
        exit(4);
    }

    if(head.magic == MH_MAGIC)
    {
        swap_long = dont_swap_long;
        swap_short = dont_swap_short;
    }
    else if(head.magic == MH_CIGAM)
    {
        swap_long = NXSwapLong;
        swap_short = NXSwapShort;
    }
    else
    {
        fprintf(stderr, "bad magic number\n");
        exit(5);
    }

    offset = -1;
    for(i = 0; i < swap_long(head.ncmds); ++i)
    {
        saveoffset = ftell(fp);
        if(fread(&seg, sizeof(struct load_command), 1, fp) != 1)
        {
            fprintf(stderr, "couldn't read load command\n");
            exit(6);
        }
        if(swap_long(seg.cmd) == LC_SEGMENT)
        {
            if(fread(&seg.segname, sizeof(seg) - sizeof(struct load_command),
                     1, fp)
               != 1)
            {
                fprintf(stderr, "couldn't read load segment command\n");
                exit(7);
            }
            if(strcmp(seg.segname, SEG_PAGEZERO) == 0)
            {
                offset = saveoffset;
                /*-->*/ break;
            }
        }
        if(fseek(fp, swap_long(seg.cmdsize) - sizeof(struct load_command), SEEK_CUR) == -1)
        {
            fprintf(stderr, "couldn't seek after load command\n");
            exit(8);
        }
    }

    if(offset == -1)
    {
        fprintf(stderr, "coulnd't find lowseg\n");
        exit(9);
    }
    if(fseek(fp, offset, SEEK_SET) == 1)
    {
        fprintf(stderr, "couldn't seek to lowseg\n");
        exit(10);
    }
    sscanf(argv[2], "%lx", &seg.vmsize);
    seg.vmsize = swap_long(seg.vmsize);
    if(fwrite(&seg, sizeof(seg), 1, fp) != 1)
    {
        fprintf(stderr, "couldn't fwrite\n");
        exit(11);
    }
    fclose(fp);
    return 0;
}
