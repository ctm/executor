/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ResourceMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "rsys/appearance.h"

#include "ResourceMgr.h"
#include "FileMgr.h"
#include "MemoryMgr.h"
#include "OSUtil.h"

#include "rsys/error.h"
#include "rsys/options.h"

using namespace Executor;

PRIVATE appearance_t appearance = appearance_sys7;

/*
 * NOTE:  Order of following entries must correspond to appearance_t enum
 */

PRIVATE StringPtr res_filenames[] = {
    (StringPtr) "\010mac.rsrc",
    (StringPtr) "\014windows.rsrc",
};

/* Exactly the same as CountTypes, except only the resource file with the
   refnum of rn is consulted.  */

PRIVATE INTEGER
CountTypesRN(INTEGER rn)
{
    INTEGER savern, retval;

    savern = CurResFile();
    UseResFile(rn);
    retval = Count1Types();
    UseResFile(savern);
    return retval;
}

/* Exactly the same as CountResources, except only the resource file with the
   refnum rn is consulted.  */

PRIVATE INTEGER
CountResourcesRN(INTEGER rn, ResType type)
{
    INTEGER savern, retval;

    savern = CurResFile();
    UseResFile(rn);
    retval = Count1Resources(type);
    UseResFile(savern);
    return retval;
}

/* Exactly like GetResource, except limited to the file specified by rn.  */

PRIVATE Handle
GetResourceRN(INTEGER rn, ResType type, INTEGER id)
{
    INTEGER savern;
    Handle retval;

    savern = CurResFile();
    UseResFile(rn);
    retval = Get1Resource(type, id);
    UseResFile(savern);
    return retval;
}

/* Exactly the same as AddResource, except its action is limited to the file
   with the refnum rn.  */

PRIVATE void
AddResourceRN(INTEGER rn, Handle h, ResType type, INTEGER id, Str255 name)
{
    Handle current_handle;

    current_handle = GetResourceRN(rn, type, id);
    if(current_handle)
    {
        if(current_handle != h)
        {
            Size new_size;
            OSErr err;

            LoadResource(current_handle);
            new_size = GetHandleSize(h);
            SetHandleSize(current_handle, new_size);
            err = MemError();
            if(err != noErr)
                warning_unexpected("err = %d", err);
            else
            {
                memcpy(STARH(current_handle), STARH(h), new_size);

                /* NOTE: we don't call ChangedResource because we don't
		 care if this change gets lost.  In fact, we will try
		 to set up our System file so that it has the union of
		 all the resources that the various appearances use so that
		 the act of setting an appearance won't cause the system
		 file to be marked dirty.  This should lessen the chance
		 of corruption.

		 ChangedResource (current_handle); */
            }
        }
    }
    else
    {
        INTEGER savern;
        Handle hh;

        hh = h;
        if(HandleZone(h) != MR(SysZone))
        {
            Handle save_hand;

            save_hand = h;
            HandToHand(&hh);
            DisposHandle(save_hand);
        }
        savern = CurResFile();
        UseResFile(rn);
        AddResource(hh, type, id, name);
        UseResFile(savern);
    }
}

/* Exactly the same as GetIndType, except its action is limited to the file
   with the refnum rn.  */

PRIVATE void
GetIndTypeRN(INTEGER rn, GUEST<ResType> *typep, INTEGER type_num)
{
    INTEGER savern;

    savern = CurResFile();
    UseResFile(rn);
    Get1IndType(typep, type_num);
    UseResFile(savern);
}

/* Exactly the same as GetIndResource except limited to resources from the
   file with refnum rn. */

PRIVATE Handle
GetIndResourceRN(INTEGER rn, ResType type, INTEGER id)
{
    INTEGER savern;
    Handle retval;

    savern = CurResFile();
    UseResFile(rn);
    retval = Get1IndResource(type, id);
    UseResFile(savern);
    return retval;
}

PRIVATE void
silently_replace_resources(INTEGER master_file_rn, INTEGER from_file_rn)
{
    THz save_zone;
    INTEGER type_num, type_num_max;
    INTEGER save_resload;

    save_zone = GetZone();
    SetZone(MR(SysZone));
    type_num_max = CountTypesRN(from_file_rn);
    save_resload = ResLoad;
    SetResLoad(false);
    for(type_num = 1; type_num <= type_num_max; ++type_num)
    {
        ResType type;
        GUEST<ResType> type_s;
        INTEGER res_num, res_num_max;

        GetIndTypeRN(from_file_rn, &type_s, type_num);
        type = CL(type_s);
        res_num_max = CountResourcesRN(from_file_rn, type);
        for(res_num = 1; res_num <= res_num_max; ++res_num)
        {
            Handle h;
            INTEGER id;
            GUEST<INTEGER> id_s;
            GUEST<ResType> t;

            Str255 name;

            h = GetIndResourceRN(from_file_rn, type, res_num);
            GetResInfo(h, &id_s, &t, name);
            id = CW(id_s);
            LoadResource(h);
            DetachResource(h);
            AddResourceRN(master_file_rn, h, type, id, name);
        }
    }
    SetResLoad(save_resload);
    SetZone(save_zone);
}

PUBLIC void
Executor::ROMlib_set_appearance(void)
{
    INTEGER res_file;

    res_file = OpenRFPerm(res_filenames[appearance], CW(BootDrive), fsRdPerm);
    if(res_file != -1)
    {
        silently_replace_resources(CW(SysMap), res_file);
        CloseResFile(res_file);
    }
    else if(appearance != 0)
    {
        appearance = (appearance_t)0;
        ROMlib_set_appearance();
    }
}

PUBLIC bool
Executor::ROMlib_parse_appearance(const char *appearance_str)
{
    bool retval = true;

    if(strcasecmp(appearance_str, "mac") == 0)
        appearance = appearance_sys7;
    else if(strcasecmp(appearance_str, "windows") == 0)
    {
        ROMlib_options |= ROMLIB_RECT_SCREEN_BIT;
        appearance = appearance_win3;
    }
    else
        retval = false;

    return retval;
}

PUBLIC appearance_t Executor::ROMlib_get_appearance(void)
{
    return appearance;
}
