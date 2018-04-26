/* Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */
#define CLEANUP_BATCH_FILE_NAME "+/cleanup.bat"
#include <windows.h>

#include <rsys/common.h>
#include <rsys/file.h>
#include <rsys/notmac.h>

#include <stdarg.h>
#include <string>

#include "cleanup.h"

using namespace Executor;

void
add_to_cleanup(const char *s, ...)
{
    std::string batch_file;
    FILE *fp;
    struct stat sbuf;

    batch_file = expandPath(CLEANUP_BATCH_FILE_NAME);
    if(stat(batch_file.c_str(), &sbuf) == 0)
        fp = fopen(batch_file.c_str(), "a");
    else
    {
        fp = fopen(batch_file.c_str(), "w");
        if(fp)
        {
            fprintf(fp, "@echo off\n");
            if(ROMlib_start_drive)
                fprintf(fp, "%c:\n", ROMlib_start_drive);
        }
    }
    if(fp)
    {
        va_list ap;

        va_start(ap, s);
        vfprintf(fp, s, ap);
        fclose(fp);
    }
}

void
call_cleanup_bat(void)
{
    std::string batch_file;
    struct stat sbuf;

    batch_file = expandPath(CLEANUP_BATCH_FILE_NAME);
    if(stat(batch_file.c_str(), &sbuf) == 0)
    {
        add_to_cleanup("del \"%s\"\n", batch_file.c_str());
        {
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            memset(&si, 0, sizeof si);
            si.cb = sizeof si;
            si.dwFlags = STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
            CreateProcess(batch_file.c_str(), NULL, NULL, NULL, false, 0, NULL, NULL,
                          &si, &pi);
        }
    }
}
