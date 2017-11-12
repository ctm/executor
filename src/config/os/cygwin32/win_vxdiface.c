/*
 *  vxdiface.cpp - Interface to cdenable.vxd
 *
 *  Basilisk II 1997-1999 Christian Bauer
 *
 *  Ported to Windows by Lauri Pesonen

>Do you own the copyright to your Windows-specific additions to
>Basilisk?  If so, would you be willing to provide it to ARDI under a
>non-GPL license?  W/o a separate license I can still look at the code
>to find the technique you're using it and re-implement it, but that
>may result in one of us missing bug-fixes or improvements.

Yes, I own the copyright. Everything Windows-specific is by me,
and the fact that I distribute it under GPL doesn't change
anything. So far, there has been no source code contributions, 
and actually I never really expected to have anything.
I have taken no steps to protect it in any way, but the implicit
copyright that a programmer has over his code still holds.

To put it formally:
I (Lauri Pesonen) hereby grant you (Clifford Matthews and ARDI)
a non-exclusive right to use my code from Basilisk II Windows port 
in Executor. There is no charge for the license, and it will
automatically include any future bug fixes and additions to
the Windows-specific code base, but only those written by me.

 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sysdeps.h"
#include <stdio.h>
#include <windows.h>
#include "vxdiface.h"
#include "main_windows.h"

#define CVXD_APIFUNC_1 1
#define CVXD_APIFUNC_2 2
#define CVXD_APIFUNC_3 3
#define CVXD_APIFUNC_4 4
#define CVXD_APIFUNC_5 5
#define CVXD_APIFUNC_6 6

#define KEEP_OPEN

static HANDLE vhndl = INVALID_HANDLE_VALUE;

static HANDLE get_enable(void)
{
#ifdef KEEP_OPEN
    return (vhndl);
#else
    return (CreateFile("\\\\.\\CDENABLE.VXD", 0, 0, 0, CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE, 0));
#endif
}

static void release_enable(HANDLE h)
{
#ifndef KEEP_OPEN
    CloseHandle(h);
#endif
}

BOOL VxdInit(void)
{
    char msg[256];
    DWORD dwErrorCode;

    vhndl = CreateFile("\\\\.\\CDENABLE.VXD", 0, 0, 0, CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE, 0);
    if(vhndl == INVALID_HANDLE_VALUE)
    {
        dwErrorCode = GetLastError();
        if(dwErrorCode == ERROR_NOT_SUPPORTED)
        {
            sprintf(msg, "Unable to open CDENABLE.VXD, \n device does not support DeviceIOCTL");
        }
        else
        {
            sprintf(
                msg,
                "Unable to open CDENABLE.VXD, Error code: %lx\r\n"
                "Please make sure that you copied the file to Windows\\System directory",
                dwErrorCode);
        }
        WarningAlert(msg);
        return (false);
    }
    else
    {
#ifndef KEEP_OPEN
        CloseHandle(vhndl);
        vhndl = INVALID_HANDLE_VALUE;
#endif
        return (true);
    }
}

void VxdFinal(void)
{
    if(vhndl != INVALID_HANDLE_VALUE)
    {
        CloseHandle(vhndl);
        vhndl = INVALID_HANDLE_VALUE;
    }
}

int VxdReadCdSectors(int drive, ULONG LBA, int count, char *buf)
{
    HANDLE hCVxD = 0;
    DWORD cbBytesReturned;
    DWORD ReqInfo[4];
    DWORD ReplyInfo[4];
    int bytes_read = 0;
    char msg[256];
    static int shutup = 0;

    hCVxD = get_enable();

    if(hCVxD != INVALID_HANDLE_VALUE)
    {
        ReqInfo[0] = drive;
        ReqInfo[1] = LBA;
        ReqInfo[2] = count;
        ReqInfo[3] = (DWORD)buf;
        if(DeviceIoControl(hCVxD, CVXD_APIFUNC_1,
                           (LPVOID)ReqInfo, sizeof(ReqInfo),
                           (LPVOID)ReplyInfo, sizeof(ReplyInfo),
                           &cbBytesReturned, NULL))
        {
            bytes_read = ReplyInfo[0];
        }
        else
        {
            sprintf(msg, "Device does not support the requested API\n");
            if(!shutup)
                WarningAlert(msg);
            shutup = 1;
        }
    }
    release_enable(hCVxD);
    return (bytes_read);
}

int VxdReadHdSectors(int drive, ULONG LBA, int count, char *buf)
{
    HANDLE hCVxD = 0;
    DWORD cbBytesReturned;
    DWORD ReqInfo[4];
    DWORD ReplyInfo[4];
    int bytes_read = 0;
    char msg[512];
    static int shutup = 0;

    hCVxD = get_enable();

    if(hCVxD != INVALID_HANDLE_VALUE)
    {
        ReqInfo[0] = drive;
        ReqInfo[1] = LBA;
        ReqInfo[2] = count;
        ReqInfo[3] = (DWORD)buf;
        if(DeviceIoControl(hCVxD, CVXD_APIFUNC_5,
                           (LPVOID)ReqInfo, sizeof(ReqInfo),
                           (LPVOID)ReplyInfo, sizeof(ReplyInfo),
                           &cbBytesReturned, NULL))
        {
            bytes_read = ReplyInfo[0];
        }
        else
        {
            sprintf(
                msg,
                "Device does not support the requested API. Please make sure you copied the latest CDENABLE.VXD to the \"\\Windows\\System\" folder. If you did this already, reboot the computer.");
            if(!shutup)
                WarningAlert(msg);
            shutup = 1;
        }
    }
    release_enable(hCVxD);
    return (bytes_read);
}

int VxdWriteHdSectors(int drive, ULONG LBA, int count, char *buf)
{
    HANDLE hCVxD = 0;
    DWORD cbBytesReturned;
    DWORD ReqInfo[4];
    DWORD ReplyInfo[4];
    int bytes_written = 0;
    char msg[256];
    static int shutup = 0;

    hCVxD = get_enable();

    if(hCVxD != INVALID_HANDLE_VALUE)
    {
        ReqInfo[0] = drive;
        ReqInfo[1] = LBA;
        ReqInfo[2] = count;
        ReqInfo[3] = (DWORD)buf;
        if(DeviceIoControl(hCVxD, CVXD_APIFUNC_6,
                           (LPVOID)ReqInfo, sizeof(ReqInfo),
                           (LPVOID)ReplyInfo, sizeof(ReplyInfo),
                           &cbBytesReturned, NULL))
        {
            bytes_written = ReplyInfo[0];
        }
        else
        {
            sprintf(msg, "Device does not support the requested API\n");
            if(!shutup)
                WarningAlert(msg);
            shutup = 1;
        }
    }
    release_enable(hCVxD);
    return (bytes_written);
}

int VxdPatch(int onoff)
{
    HANDLE hCVxD = 0;
    int func = 0;
    DWORD cbBytesReturned;
    DWORD ReqInfo[2];
    DWORD ReplyInfo[2];
    int ok = 0;

    if(!onoff)
        return (1);

    hCVxD = get_enable();
    if(hCVxD != INVALID_HANDLE_VALUE)
    {
        func = onoff ? CVXD_APIFUNC_3 : CVXD_APIFUNC_4;
        (void)DeviceIoControl(
            hCVxD,
            func,
            (LPVOID)ReqInfo, sizeof(ReqInfo),
            (LPVOID)ReplyInfo, sizeof(ReplyInfo),
            &cbBytesReturned, NULL);
        if(ReplyInfo[0])
            ok = 1;
    }
    release_enable(hCVxD);
    return (ok);
}
