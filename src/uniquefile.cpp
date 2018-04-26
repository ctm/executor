/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "rsys/uniquefile.h"
#include "rsys/file.h"

using namespace Executor;

/* This function fills in RESULT with the pathname for a unique file
 * of a specified form.  RESULT will be a 0-terminated Pascal string.
 * It returns true iff successful, else false.  If all names matching
 * the template are already taken by existing files, returns false.
 * TEMPLATE indicates the form of the filename: it should have exactly
 * one "*" character, which will be replaced with an alphanumeric
 * character to try to obtain a unique filename.
 */

bool Executor::unique_file_name(const char *template1, const char *default_template,
                                Str255 result)
{
    static const char suff[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    const char *try_suff;
    struct stat sbuf;
    time_t oldest_time;

    /* The template1 string must have a '*'. */
    if(!template1 || !strchr(template1, '*'))
        template1 = default_template;

    /* Default to an empty string. */
    result[0] = 0;
    result[1] = '\0';

    /* Make sure the resulting string won't be too long. */
    if(strlen(template1) + 1 >= 255)
        return false;

    std::string try1 = expandPath(template1);

    /* Try to find a unique file name.  If we fail, choose the oldest. */
    oldest_time = 0;
    auto pos = try1.rfind('*');
    for(try_suff = suff; *try_suff; try_suff++)
    {
        try1[pos] = *try_suff;
        if(Ustat(try1.c_str(), &sbuf) != 0 && errno == ENOENT)
        {
            /* If this file name isn't taken, grab it !*/
            strcpy((char *)result + 1, try1.c_str());
            result[0] = strlen((char *)result + 1);
            return true;
        }
    }
    /* Failed! */
    return false;
}
