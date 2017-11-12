#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dpmi.h>
#include <string.h>
#include <ctype.h>

#include "sb_defs.h"
#include "sbdriver.h"

unsigned char *sb_load_sample(char *);

void main(void)
{
    char *sample[3];
    sb_status stat;
    char k;
    int i, j, mode;
    char *buf;
    WORD *buf2;

    sample[0] = sb_load_sample("left.raw");
    sample[1] = sb_load_sample("right.raw");
    sample[2] = sb_load_sample("deedee.raw");

    stat = sb_install_driver(NULL);

    if(stat != SB_SUCCESS)
    {
        cprintf("\n\r\n\rCould not initialize driver because:\n\r%s\n\r",
                sb_driver_error);
        free(sample[0]);
        free(sample[1]);
        free(sample[2]);
        exit(0);
    }

    buf = (char *)malloc(65536);
    buf2 = (WORD *)buf;
    memset(buf, 0, 65536);

    i = sb_get_capabilities();
    mode = SB_8_BIT | SB_MONO;
    sb_set_format(mode);

    do
    {

        clrscr();

        cprintf("Sound capabilities:\n\r8-Bit\n\r");
        if(i & SB_16_BIT)
            cprintf("16-Bit\n\r");
        cprintf("Monophonic\n\r");
        if(i & SB_STEREO)
            cprintf("Stereophonic\n\r");
        cprintf("\n\n\n\rCurrent Mode:");
        if(mode & SB_8_BIT)
            cprintf("8-Bit ");
        else
            cprintf("16-Bit ");
        if(mode & SB_MONO)
            cprintf("Mono\n\r");
        else
            cprintf("Stereo\n\r");

        cprintf("\n\n\n\n\rPress '1' for mono, '2' for stereo.\n\rPress '3' for 8-bit, '4' for 16-bit.\n\r");
        cprintf("\n\rPress <SPACE> to queue a sound.\n\rPress 'P' to pause playback, 'R' to resume playback.\n\rPress <ESC> to quit.");

        k = getch();

        if((k >= '1') && (k <= '4'))
        {
            if(!sb_numInQueue)
            {
                if(k == '1')
                {
                    mode &= SB_8_BIT | SB_16_BIT;
                    mode |= SB_MONO;
                }
                else if(k == '2')
                {
                    if(i & SB_STEREO)
                    {
                        mode &= SB_8_BIT | SB_16_BIT;
                        mode |= SB_STEREO;
                    }
                }
                else if(k == '3')
                {
                    mode &= SB_MONO | SB_STEREO;
                    mode |= SB_8_BIT;
                }
                else
                {
                    if(i & SB_16_BIT)
                    {
                        mode &= SB_MONO | SB_STEREO;
                        mode |= SB_16_BIT;
                    }
                }
            }
        }
        else if(k == ' ')
        {
            if(mode & SB_8_BIT)
            {
                if(mode & SB_MONO)
                {
                    sb_enqueue_sample(sample[2], 16384);
                    sb_enqueue_sample(sample[2] + 16384, 6406);
                }
                else
                {
                    memset(buf, 127, 65536);
                    if(rand() & 1)
                        for(j = 0; j < 14430; j++)
                            *(buf + (j * 2)) = *(sample[0] + j);
                    else
                        for(j = 0; j < 13795; j++)
                            *(buf + 1 + (j * 2)) = *(sample[1] + j);
                    sb_enqueue_sample(buf, 8192);
                    sb_enqueue_sample(buf + 16384, 3119);
                }
            }
            else
            {
                if(mode & SB_MONO)
                {
                    memset(buf, 127, 65536);
                    for(j = 0; j < 22790; j++)
                        *(buf2 + j) = ((WORD)(*(sample[2] + j))) << 8;
                    sb_enqueue_sample((BYTE *)buf2, 8192);
                    sb_enqueue_sample((BYTE *)(buf2 + 8192), 8192);
                    sb_enqueue_sample((BYTE *)(buf2 + 16384), 6406);
                }
                else
                {
                    memset(buf, 127, 65536);
                    if(rand() & 1)
                        for(j = 0; j < 14430; j++)
                            *(buf2 + (j * 2)) = ((WORD)(*(sample[0] + j))) << 8;
                    else
                        for(j = 0; j < 13795; j++)
                            *(buf2 + 1 + (j * 2)) = ((WORD)(*(sample[1] + j))) << 8;
                    sb_enqueue_sample((BYTE *)buf2, 4096);
                    sb_enqueue_sample((BYTE *)(buf2 + 8192), 4096);
                    sb_enqueue_sample((BYTE *)(buf2 + 16384), 4096);
                    sb_enqueue_sample((BYTE *)(buf2 + 24576), 2142);
                }
            }
        }
        else if(toupper(k) == 'P')
            sb_set_playback_enabled(0);
        else if(toupper(k) == 'R')
            sb_set_playback_enabled(1);

        sb_set_format(mode);

    } while(k != 27);

    sb_uninstall_driver();
    free(sample[0]);
    free(sample[1]);
    free(sample[2]);
    free(buf);
}

unsigned char *
sb_load_sample(char *fname)
{
    int length;
    unsigned char *buf;
    FILE *fp = fopen(fname, "rb");

    if(fp == NULL)
        return NULL;

    if(fseek(fp, 0, SEEK_END) == 0)
        if((length = ftell(fp)) != -1)
            if(fseek(fp, 0, SEEK_SET) == 0)
                if((buf = (BYTE *)malloc(length)) != NULL)
                    if(fread(buf, length, 1, fp) == 1)
                    {
                        fclose(fp);
                        return buf;
                    }
                    else
                        free(buf);

    fclose(fp);
    return NULL;
}
