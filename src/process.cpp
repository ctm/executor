/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "ProcessMgr.h"
#include "ResourceMgr.h"
#include "MemoryMgr.h"
#include "ToolboxEvent.h"

#include "rsys/mman.h"
#include "rsys/process.h"

using namespace Executor;

#define declare_handle_type(type_prefix)        \
    typedef type_prefix##_t *type_prefix##_ptr; \
    typedef GUEST<type_prefix##_ptr> *type_prefix##_handle

declare_handle_type(size_resource);

#define SIZE_FLAGS_X(size) (HxX(size, flags))
#define SIZE_FLAGS(size) (CW(SIZE_FLAGS_X(size)))

static size_resource_handle
get_size_resource()
{
    Handle size;

    size = Get1Resource(FOURCC('S', 'I', 'Z', 'E'), 0);
    if(size == NULL)
        size = Get1Resource(FOURCC('S', 'I', 'Z', 'E'), -1);
    return (size_resource_handle)size;
}

#pragma pack(push, 2)
typedef struct process_info
{
    struct process_info *next;

    uint32_t mode;
    uint32_t type;
    uint32_t signature;
    uint32_t size;
    uint32_t launch_ticks;

    ProcessSerialNumber serial_number;
} process_info_t;
#pragma pack(pop)

static process_info_t *process_info_list;
static process_info_t *current_process_info;

static const int default_process_mode_flags = 0;

/* ### not currently used */
static ProcessSerialNumber system_process = { CLC(0), CLC(kSystemProcess) };
static ProcessSerialNumber no_process = { CLC(0), CLC(kNoProcess) };
static ProcessSerialNumber current_process = { CLC(0), CLC(kCurrentProcess) };

void Executor::process_create(bool desk_accessory_p,
                              uint32_t type, uint32_t signature)
{
    size_resource_handle size;
    process_info_t *info;
    static uint32_t next_free_psn = 4;

    size = get_size_resource();

    {
        TheZoneGuard guard(LM(SysZone));

        info = (process_info_t *)NewPtr(sizeof *info);
    }

    /* ### we are seriously fucked */
    if(info == NULL)
        gui_fatal("unable to allocate process info record");

    info->mode = ((size
                       ? SIZE_FLAGS(size)
                       : default_process_mode_flags)
                  | (desk_accessory_p
                         ? modeDeskAccessory
                         : 0));
    info->type = type;
    info->signature = signature;

    /* ### fixme; major bogosity */
    info->size = (zone_size(MR(LM(ApplZone)))
                  /* + A5 world size */
                  /* + stack size */);
    info->launch_ticks = TickCount();

    info->serial_number.highLongOfPSN = CL(-1);
    info->serial_number.lowLongOfPSN = CL(next_free_psn++);

    info->next = process_info_list;
    process_info_list = info;

    /* ### hack */
    current_process_info = info;
}

process_info_t *
get_process_info(ProcessSerialNumber *serial_number)
{
    process_info_t *t;

    if(PSN_EQ_P(*serial_number, current_process))
        return current_process_info;

    for(t = process_info_list; t; t = t->next)
    {
        if(PSN_EQ_P(*serial_number, t->serial_number))
            return t;
    }
    return NULL;
}

OSErr Executor::C_GetCurrentProcess(ProcessSerialNumber *serial_number)
{
    *serial_number = current_process_info->serial_number;
    return noErr;
}

OSErr Executor::C_GetNextProcess(ProcessSerialNumber *serial_number)
{
    process_info_t *t;

    if(PSN_EQ_P(*serial_number, no_process))
    {
        if(!process_info_list)
            return procNotFound;
        else
        {
            *serial_number = process_info_list->serial_number;
            return noErr;
        }
    }

    t = get_process_info(serial_number);
    if(t == NULL)
        return paramErr;
    else if(t->next == NULL)
    {
        memset(serial_number, 0, sizeof *serial_number);
        return procNotFound;
    }
    else
    {
        *serial_number = t->next->serial_number;
        return noErr;
    }
}

OSErr Executor::C_GetProcessInformation(ProcessSerialNumber *serial_number,
                                        ProcessInfoPtr process_info)
{
    process_info_t *info;
    int32_t current_ticks;

    info = get_process_info(serial_number);
    if(info == NULL
       || PROCESS_INFO_LENGTH(process_info) != sizeof *process_info)
        return paramErr;

    PROCESS_INFO_SERIAL_NUMBER(process_info) = info->serial_number;
    PROCESS_INFO_TYPE_X(process_info) = CL(info->type);
    PROCESS_INFO_SIGNATURE_X(process_info) = CL(info->signature);
    PROCESS_INFO_MODE_X(process_info) = CL(info->mode);
    PROCESS_INFO_LOCATION_X(process_info) = guest_cast<Ptr>(LM(ApplZone));
    PROCESS_INFO_SIZE_X(process_info) = CL(info->size);

    /* ### set current zone to applzone? */
    PROCESS_INFO_FREE_MEM_X(process_info) = CL(FreeMem());

    PROCESS_INFO_LAUNCHER(process_info) = no_process;

    PROCESS_INFO_LAUNCH_DATE_X(process_info) = CL(info->launch_ticks);
    current_ticks = TickCount();
    PROCESS_INFO_ACTIVE_TIME_X(process_info)
        = CL(current_ticks - info->launch_ticks);

    return noErr;
}

OSErr Executor::C_SameProcess(ProcessSerialNumber *serial_number0,
                              ProcessSerialNumber *serial_number1,
                              Boolean *same_out)
{
    process_info_t *info0, *info1;

    info0 = get_process_info(serial_number0);
    info1 = get_process_info(serial_number1);

    if(info0 == NULL
       || info1 == NULL)
        return paramErr;

    *same_out = (info0 == info1);
    return noErr;
}

OSErr Executor::C_GetFrontProcess(int32_t dummy, ProcessSerialNumber *serial_number)
{
    *serial_number = current_process_info->serial_number;
    return noErr;
}

OSErr Executor::C_SetFrontProcess(ProcessSerialNumber *serial_number)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

OSErr Executor::C_WakeUpProcess(ProcessSerialNumber *serial_number)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

OSErr Executor::C_GetProcessSerialNumberFromPortName(
    PPCPortPtr port_name, ProcessSerialNumber *serial_number)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

OSErr Executor::C_GetPortNameFromProcessSerialNumber(
    PPCPortPtr port_name, ProcessSerialNumber *serial_number)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

/* ### temp memory spew; these go elsewhere */

/* ### launch/da spew; these go elsewhere */
