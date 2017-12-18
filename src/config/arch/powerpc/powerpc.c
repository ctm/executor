/* Copyright 1999 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "rsys/arch.h"
#include "rsys/gestalt.h"

#include "Gestalt.h"

PRIVATE uint32_t
cpu_type_from_string(const char *str)
{
    uint32_t retval;
    unsigned long u;
    static struct
    {
        unsigned long num;
        const char *name;
        uint32_t gestaltValue;
    } entries[] = {
        {
            1, "601", gestaltCPU601,
        },
        {
            3, "603", gestaltCPU603,
        },
        {
            4, "604", gestaltCPU604,
        },
        {
            6, "603e", gestaltCPU603e,
        },
        {
            7, "603ev", gestaltCPU603ev,
        },
        {
            8, "750", gestaltCPU750,
        },
        {
            9, "604e", gestaltCPU604e,
        },
        {
            10, "604ev5", gestaltCPU604ev,
        }, /* (MachV) */
        {
            12, "7400", gestaltCPUG4,
        }, /* (G4) */
        {
            50, "821", 0x132,
        },
        {
            80, "860", 0x150,
        },
    };

    if(sscanf(str, "unknown (%lu)", &u) == 1)
    {
        int i;

        for(i = 0; i < NELEM(entries) && entries[i].num != u; ++i)
            ;
        if(i < NELEM(entries))
            retval = entries[i].gestaltValue;
        else
        {
            warning_unexpected("u = 0x%lx", u);
            retval = 0x100 | u;
        }
    }
    else
    {
        int i;
        char *str2;
        int len;

        str2 = alloca(strlen(str) + 1);
        str2[0] = 0;
        sscanf(str, "%s", str2);
        len = strlen(str2);

        for(i = 0;
            i < NELEM(entries) && strncmp(entries[i].name, str2, len) != 0;
            ++i)
            ;
        if(i < NELEM(entries))
            retval = entries[i].gestaltValue;
        else
        {
            warning_unexpected("str = '%s'", str);
            retval = 0;
        }
    }
    return retval;
}

#define CPU_LINE "cpu"

PRIVATE uint32_t
cpu_type(void)
{
    FILE *fp;
    uint32_t retval;

    retval = 0;
    fp = fopen("/proc/cpuinfo", "r");
    if(fp)
    {
        char buf[1024];

        while(!retval && fgets(buf, sizeof buf, fp) != NULL)
        {
            if(strncmp(buf, CPU_LINE, sizeof CPU_LINE - 1) == 0)
            {
                char *colon;

                colon = strchr(buf, ':');
                if(colon && colon[1])
                    retval = cpu_type_from_string(colon + 2);
            }
        }
        fclose(fp);
    }
    return retval;
}

/* unclear what we need to do here */

bool arch_init(void)
{
    uint32_t type;

    type = cpu_type();
    if(type)
        gestalt_set_cpu_type(type);
    return true;
}
