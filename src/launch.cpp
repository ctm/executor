/* Copyright 1994 - 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "ResourceMgr.h"
#include "SegmentLdr.h"
#include "MemoryMgr.h"
#include "StdFilePkg.h"
#include "EventMgr.h"
#include "VRetraceMgr.h"
#include "OSUtil.h"
#include "FontMgr.h"
#include "ScrapMgr.h"
#include "ToolboxUtil.h"
#include "FileMgr.h"
#include "ControlMgr.h"
#include "DeviceMgr.h"
#include "SoundDvr.h"
#include "TextEdit.h"
#include "SysErr.h"
#include "MenuMgr.h"
#include "ScriptMgr.h"
#include "DeskMgr.h"
#include "AppleTalk.h"
#include "PrintMgr.h"
#include "StartMgr.h"
#include "ToolboxEvent.h"
#include "TimeMgr.h"
#include "ProcessMgr.h"
#include "AppleEvents.h"
#include "Gestalt.h"
#include "Package.h"
#include "AliasMgr.h"
#include "OSEvent.h"
#include "ADB.h"

#include "rsys/trapglue.h"
#include "rsys/file.h"
#include "rsys/sounddriver.h"
#include "rsys/prefs.h"
#include "rsys/flags.h"
#include "rsys/aboutpanel.h"
#include "rsys/segment.h"
#include "rsys/tesave.h"
#include "rsys/blockinterrupts.h"
#include "rsys/resource.h"
#include "rsys/hfs.h"
#include "rsys/osutil.h"
#include "rsys/stdfile.h"
#include "rsys/notmac.h"
#include "rsys/ctl.h"
#include "rsys/refresh.h"

#include "rsys/options.h"
#include "rsys/cquick.h"
#include "rsys/desk.h"
#include "rsys/parse.h"
#include "rsys/executor.h"
#include "rsys/crc.h"
#include "rsys/float.h"
#include "rsys/mman.h"
#include "rsys/vdriver.h"
#include "rsys/font.h"
#include "rsys/emustubs.h"

#include "rsys/adb.h"
#include "rsys/print.h"
#include "rsys/gestalt.h"
#include "rsys/osevent.h"

#include "rsys/cfm.h"
#include "rsys/launch.h"
#include "rsys/version.h"
#include "rsys/appearance.h"

using namespace Executor;

static bool ppc_launch_p = false;

void Executor::ROMlib_set_ppc(bool val)
{
    ppc_launch_p = val;
}

#define CONFIGEXTENSION ".ecf"
#define OLD_CONFIG_EXTENSION ".econf" /* must be longer than configextension */

FILE *Executor::configfile;
int32_t Executor::ROMlib_options;

static int16_t name0stripappl(StringPtr name)
{
    char *p;
    int16_t retval;

    retval = name[0];
    if(name[0] >= 5)
    {
        p = (char *)name + name[0] + 1 - 5;
        if(p[0] == '.' && (p[1] == 'a' || p[1] == 'A') && (p[2] == 'p' || p[2] == 'P') && (p[3] == 'p' || p[3] == 'P') && (p[4] == 'l' || p[4] == 'L'))
            retval -= 5;
    }
    return retval;
}

/*
 * NOTE: ParseConfigFile now has three arguments.  The second is a standard
 * OSType and will be expanded to 0x1234ABCD.ecf if exefname.ecf
 * doesn't open.  The third is whether or not to do resizes (based on whether
 * the user has changed things by hand).
 */

int Executor::ROMlib_nowarn32;

std::string Executor::ROMlib_configfilename;

int Executor::ROMlib_pretend_help = false;
int Executor::ROMlib_pretend_alias = false;
int Executor::ROMlib_pretend_edition = false;
int Executor::ROMlib_pretend_script = false;

void remalloc(char **strp)
{
    char *new_string;
    long len;

    if(*strp)
    {
        len = strlen(*strp) + 1;
        new_string = (char *)malloc(len);
        if(new_string)
            memcpy(new_string, *strp, len);
        *strp = new_string;
    }
}

void reset_string(char **strp)
{
    if(*strp)
        free(*strp);
    *strp = 0;
}

int Executor::ROMlib_desired_bpp;

static void ParseConfigFile(StringPtr exefname, OSType type)
{
    int strwidth;
    char *newtitle;
    char *dot;
    INTEGER fname0;

    reset_string(&ROMlib_WindowName);
    reset_string(&ROMlib_Comments);
    ROMlib_desired_bpp = 0;
    fname0 = name0stripappl(exefname);
    std::string appname(exefname + 1, exefname + 1 + fname0);

    ROMlib_configfilename = ROMlib_ConfigurationFolder + "/" + appname + CONFIGEXTENSION;
    configfile = Ufopen(ROMlib_configfilename.c_str(), "r");
    if(!configfile)
    {
        ROMlib_configfilename = ROMlib_ConfigurationFolder + "/" + appname + OLD_CONFIG_EXTENSION;
        configfile = Ufopen(ROMlib_configfilename.c_str(), "r");
    }
    if(!configfile && type != 0)
    {
        char buf[16];
        sprintf(buf, "%08x", type);
        ROMlib_configfilename = ROMlib_ConfigurationFolder + "/" + buf + CONFIGEXTENSION;
        configfile = Ufopen(ROMlib_configfilename.c_str(), "r");
    }
    if(configfile)
    {
        yyparse();

        if(ROMlib_get_appearance() == appearance_win3)
            ROMlib_options |= ROMLIB_RECT_SCREEN_BIT;

#if 0	
	if (ROMlib_options & ROMLIB_NOCLOCK_BIT)
	    ROMlib_noclock = 1;
#endif
        if(ROMlib_options & ROMLIB_BLIT_OS_BIT)
            ROMlib_WriteWhen(WriteInOSEvent);
        if(ROMlib_options & ROMLIB_BLIT_TRAP_BIT)
            ROMlib_WriteWhen(WriteAtEndOfTrap);
        if(ROMlib_options & ROMLIB_BLIT_OFTEN_BIT)
            ROMlib_WriteWhen(WriteInBltrgn);
#if 0
	if (ROMlib_options & ROMLIB_ACCELERATED_BIT)
	    ROMlib_accelerated = true;
	else
	    ROMlib_accelerated = false;
#endif
        if(ROMlib_options & ROMLIB_REFRESH_BIT)
            ROMlib_refresh = 10;
        if(ROMlib_options & ROMLIB_DIRTY_VARIANT_BIT)
            ROMlib_dirtyvariant = true;
        else
            ROMlib_dirtyvariant = false;
        if(ROMlib_options & ROMLIB_SOUNDOFF_BIT)
            ROMlib_PretendSound = soundoff;
        if(ROMlib_options & ROMLIB_PRETENDSOUND_BIT)
            ROMlib_PretendSound = soundpretend;
        if(ROMlib_options & ROMLIB_SOUNDON_BIT)
            ROMlib_PretendSound = SOUND_WORKS_P() ? soundon : soundpretend;
#if 0
	if (ROMlib_options & ROMLIB_PASSPOSTSCRIPT_BIT)
	    ROMlib_passpostscript = true;
	else
	    ROMlib_passpostscript = false;
#else
        /* #warning ROMlib_passpostscript wired to true */
        ROMlib_passpostscript = true;
#endif
        if(ROMlib_options & ROMLIB_NEWLINETOCR_BIT)
            ROMlib_newlinetocr = true;
        else
            ROMlib_newlinetocr = false;
        if(ROMlib_options & ROMLIB_DIRECTDISKACCESS_BIT)
            ROMlib_directdiskaccess = true;
        else
            ROMlib_directdiskaccess = false;
        if(ROMlib_options & ROMLIB_NOWARN32_BIT)
            ROMlib_nowarn32 = true;
        else
            ROMlib_nowarn32 = false;
        if(ROMlib_options & ROMLIB_FLUSHOFTEN_BIT)
            ROMlib_flushoften = true;
        else
            ROMlib_flushoften = false;

        if(ROMlib_options & ROMLIB_PRETEND_HELP_BIT)
            ROMlib_pretend_help = true;
        else
            ROMlib_pretend_help = false;

        if(ROMlib_options & ROMLIB_PRETEND_ALIAS_BIT)
            ROMlib_pretend_alias = true;
        else
            ROMlib_pretend_alias = false;

        if(ROMlib_options & ROMLIB_PRETEND_EDITION_BIT)
            ROMlib_pretend_edition = true;
        else
            ROMlib_pretend_edition = false;

        if(ROMlib_options & ROMLIB_PRETEND_SCRIPT_BIT)
            ROMlib_pretend_script = true;
        else
            ROMlib_pretend_script = false;

        if(ROMlib_desired_bpp)
            SetDepth(MR(LM(MainDevice)), ROMlib_desired_bpp, 0, 0);
        fclose(configfile);
    }

#if !defined(VDRIVER_DISPLAYED_IN_WINDOW)

#define SetTitle(x)            \
    do                         \
    {                          \
        ROMlib_WindowName = x; \
    } while(0)

#else

#define SetTitle(x)                   \
    do                                \
    {                                 \
        char *_x;                     \
        char *_title;                 \
        int len;                      \
                                      \
        _x = (x);                     \
        len = strlen(_x) + 1;         \
        _title = (char *)alloca(len); \
        memcpy(_title, _x, len);      \
        ROMlib_SetTitle(_title);      \
    } while(0)

#endif

    if(ROMlib_WindowName)
        SetTitle(ROMlib_WindowName);
    else
    {
        strwidth = fname0;
        newtitle = (char *)alloca(strwidth + 1);
        memcpy(newtitle, exefname + 1, strwidth);
        newtitle[strwidth] = 0;
        dot = strrchr(newtitle, '.');
        if(dot && (strcmp(dot, ".appl") == 0 || strcmp(dot, ".APPL") == 0))
            *dot = 0;
        // TODO: convert from MacRoman to UTF-8 (at least for SDL2 frontend)
        SetTitle(newtitle);
    }
#if 0
    if (ROMlib_ScreenLocation.first != INITIALPAIRVALUE)
	ROMlib_HideScreen();
    if (ROMlib_ScreenLocation.first != INITIALPAIRVALUE) {
	ROMlib_SetLocation(&ROMlib_ScreenLocation);
	ROMlib_ShowScreen();
    }
#endif

    remalloc(&ROMlib_WindowName);
    remalloc(&ROMlib_Comments);
}

static void beginexecutingat(LONGINT startpc)
{
#define ALINETRAPNUMBER 0xA
    trap_install_handler(ALINETRAPNUMBER, alinehandler, (void *)0);

    EM_D0 = 0;
    EM_D1 = 0xFFFC000;
    EM_D2 = 0;
    EM_D3 = 0x800008;
    EM_D4 = 0x408;
    EM_D5 = 0x10204000;
    EM_D6 = 0x7F0000;
    EM_D7 = 0x80000800;

    EM_A0 = 0x3EF796;
    EM_A1 = 0x910;
    EM_A2 = EM_D3;
    EM_A3 = 0;
    EM_A4 = 0;
    EM_A5 = CL(guest_cast<LONGINT>(LM(CurrentA5))); /* was smashed when we
					   initialized above */
    EM_A6 = 0x1EF;

    CALL_EMULATOR(startpc);
    C_ExitToShell();
}

LONGINT Executor::ROMlib_appbit;

size_info_t Executor::size_info;

#define VERSFMT "(0x%02x, 0x%02x, 0x%02x, 0x%02x, %d)"
#define VERSSIZE(vp) (sizeof(VERSFMT) + (vp)->shortname[0] + 6)

LONGINT Executor::ROMlib_creator;

void *ROMlib_foolgcc; /* to force the alloca to be done */

void Executor::SFSaveDisk_Update(INTEGER vrefnum, Str255 filename)
{
    ParamBlockRec pbr;
    Str255 save_name;

    str255assign(save_name, filename);
    pbr.volumeParam.ioNamePtr = RM((StringPtr)save_name);
    pbr.volumeParam.ioVolIndex = CWC(-1);
    pbr.volumeParam.ioVRefNum = CW(vrefnum);
    PBGetVInfo(&pbr, false);
    LM(SFSaveDisk) = CW(-CW(pbr.volumeParam.ioVRefNum));
}

uint32_t Executor::ROMlib_version_long;

static bool
cfrg_match(const cfir_t *cfirp, GUEST<OSType> arch_x, uint8 type_x, Str255 name)
{
    bool retval;

    retval = (CFIR_ISA_X(cfirp) == arch_x && CFIR_TYPE_X(cfirp) == type_x && (!name[0] || EqualString(name, (StringPtr)CFIR_NAME(cfirp),
                                                                                                      false, true)));
    return retval;
}

cfir_t *
Executor::ROMlib_find_cfrg(Handle cfrg, OSType arch, uint8 type, Str255 name)
{
    cfrg_resource_t *cfrgp;
    int n_descripts;
    cfir_t *cfirp;
    GUEST<OSType> desired_arch_x;
    uint8 type_x;
    cfir_t *retval;

    cfrgp = (cfrg_resource_t *)STARH(cfrg);
    cfirp = (cfir_t *)((char *)cfrgp + sizeof *cfrgp);
    desired_arch_x = CL(arch);
    type_x = CB(type);
    for(n_descripts = CFRG_N_DESCRIPTS(cfrgp);
        n_descripts > 0 && !cfrg_match(cfirp, desired_arch_x, type_x, name);
        --n_descripts, cfirp = (cfir_t *)((char *)cfirp + CFIR_LENGTH(cfirp)))
        ;
    retval = n_descripts > 0 ? cfirp : 0;

    return retval;
}

static void
cfm_launch(Handle cfrg0, OSType desired_arch, FSSpecPtr fsp)
{
    cfir_t *cfirp;

    cfirp = ROMlib_find_cfrg(cfrg0, desired_arch, kApplicationCFrag,
                             (StringPtr) "");

#if(defined(powerpc) || defined(__ppc__)) && !defined(CFM_PROBLEMS)
    if(cfirp)
    {
        Ptr mainAddr;
        Str255 errName;
        ConnectionID c_id;

        ROMlib_release_tracking_values();

        if(CFIR_LOCATION(cfirp) == kOnDiskFlat)
        {
            // #warning were ignoring a lot of the cfir attributes
            GetDiskFragment(fsp, CFIR_OFFSET_TO_FRAGMENT(cfirp),
                            CFIR_FRAGMENT_LENGTH(cfirp), "",
                            kLoadLib,
                            &c_id,
                            &mainAddr,
                            errName);
        }
        else if(CFIR_LOCATION(cfirp) == kOnDiskSegmented)
        {
            ResType typ;
            INTEGER id;
            Handle h;

            typ = (ResType)CFIR_OFFSET_TO_FRAGMENT(cfirp);
            id = CFIR_FRAGMENT_LENGTH(cfirp);
            h = GetResource(typ, id);
            HLock(h);
            GetMemFragment(STARH(h), GetHandleSize(h), "", kLoadLib,
                           &c_id, &mainAddr, errName);

            fprintf(stderr, "Memory leak from segmented fragment\n");
        }
        {
            uint32_t new_toc;
            void *new_pc;

            new_toc = ((uint32_t *)mainAddr)[1];
            new_pc = ((void **)mainAddr)[0];
            ppc_call(new_toc, new_pc, 0);
        }
    }
#endif

    C_ExitToShell();
}

int Executor::ROMlib_uaf;

launch_failure_t Executor::ROMlib_launch_failure = launch_no_failure;
INTEGER Executor::ROMlib_exevrefnum;

static void launchchain(StringPtr fName, INTEGER vRefNum, BOOLEAN resetmemory,
                        LaunchParamBlockRec *lpbp)
{
    OSErr err;
    FInfo finfo;
    Handle code0;
    Handle cfrg0;
    Handle h;
    vers_t *vp;
    LONGINT abovea5, belowa5, jumplen, jumpoff;
    GUEST<LONGINT> *lp;
    INTEGER toskip;
    Byte *p;
    WDPBRec wdpb;
    StringPtr ename;
    INTEGER elen;
    char quickbytes[grafSize];
    LONGINT tmpa5;

    for(p = fName + fName[0] + 1; p > fName && *--p != ':';)
        ;
    toskip = p - fName;
    LM(CurApName)[0] = MIN(fName[0] - toskip, 31);
    BlockMoveData((Ptr)fName + 1 + toskip, (Ptr)LM(CurApName) + 1,
                  (Size)LM(CurApName)[0]);
#if 0
    Munger(MR(LM(AppParmHandle)), 2L*sizeof(INTEGER), (Ptr) 0,
				  (LONGINT) sizeof(AppFile), (Ptr) "", 0L);
#endif
    if(!lpbp || lpbp->launchBlockID != CWC(extendedBlock))
    {
        CInfoPBRec hpb;

        hpb.hFileInfo.ioNamePtr = RM(&fName[0]);
        hpb.hFileInfo.ioVRefNum = CW(vRefNum);
        hpb.hFileInfo.ioFDirIndex = CWC(0);
        hpb.hFileInfo.ioDirID = 0;
        PBGetCatInfo(&hpb, false);
        wdpb.ioVRefNum = CW(vRefNum);
        wdpb.ioWDDirID = hpb.hFileInfo.ioFlParID;
    }
    else
    {
        FSSpecPtr fsp;

        fsp = MR(lpbp->launchAppSpec);
        wdpb.ioVRefNum = fsp->vRefNum;
        wdpb.ioWDDirID = fsp->parID;
    }
    /* Do not do this -- Loser does it SFSaveDisk_Update (vRefNum, fName); */
    wdpb.ioWDProcID = TICKX("Xctr");
    wdpb.ioNamePtr = 0;
    PBOpenWD(&wdpb, false);
    ROMlib_exevrefnum = CW(wdpb.ioVRefNum);
    ROMlib_exefname = LM(CurApName);
#if 0
/* I'm skeptical that this is correct */
    if (LM(CurMap) != LM(SysMap))
	CloseResFile(LM(CurMap));
#endif
    SetVol((StringPtr)0, ROMlib_exevrefnum);
    LM(CurApRefNum) = CW(OpenResFile(ROMlib_exefname));

    err = GetFInfo(ROMlib_exefname, ROMlib_exevrefnum, &finfo);

    process_create(false, CL(finfo.fdType), CL(finfo.fdCreator));

    if(ROMlib_exeuname)
        free(ROMlib_exeuname);
    ROMlib_exeuname = ROMlib_newunixfrommac((char *)ROMlib_exefname + 1,
                                            ROMlib_exefname[0]);
    elen = strlen(ROMlib_exeuname);
    ename = (StringPtr)alloca(elen + 1);
    BlockMoveData((Ptr)ROMlib_exeuname, (Ptr)ename + 1, elen);
    ename[0] = elen;

    ROMlib_creator = CL(finfo.fdCreator);

#define LEMMINGSHACK
#if defined(LEMMINGSHACK)
    {
        if(finfo.fdCreator == CL(TICK("Psyg"))
           || finfo.fdCreator == CL(TICK("Psod")))
            ROMlib_flushoften = true;
    }
#endif /* defined(LEMMINGSHACK) */

#if defined(ULTIMA_III_HACK)
    ROMlib_ultima_iii_hack = (finfo.fdCreator == CL(TICK("Ult3")));
#endif

    h = GetResource(FOURCC('v', 'e', 'r', 's'), 2);
    if(!h)
        h = GetResource(FOURCC('v', 'e', 'r', 's'), 1);

    ROMlib_version_long = 0;
    if(h)
    {
        vp = (vers_t *)STARH(h);
        ROMlib_version_long = ((vp->c[0] << 24) | (vp->c[1] << 16) | (vp->c[2] << 8) | (vp->c[3] << 0));
    }
    

    ROMlib_ScreenSize.first = INITIALPAIRVALUE;
    ROMlib_MacSize.first = INITIALPAIRVALUE;
    ROMlib_directdiskaccess = false;
    ROMlib_clear_gestalt_list();
    ParseConfigFile((StringPtr) "\017ExecutorDefault", 0);
    ParseConfigFile(ename, err == noErr ? CL(finfo.fdCreator) : 0);
    ROMlib_clockonoff(!ROMlib_noclock);
    if((ROMlib_ScreenSize.first != INITIALPAIRVALUE
        || ROMlib_MacSize.first != INITIALPAIRVALUE))
    {
        if(ROMlib_ScreenSize.first == INITIALPAIRVALUE)
            ROMlib_ScreenSize = ROMlib_MacSize;
        if(ROMlib_MacSize.first == INITIALPAIRVALUE)
            ROMlib_MacSize = ROMlib_ScreenSize;
    }
    code0 = Get1Resource(FOURCC('C', 'O', 'D', 'E'), 0);
    cfrg0 = Get1Resource(FOURCC('c', 'f', 'r', 'g'), 0);

    if(cfrg0 && ppc_launch_p && ROMlib_find_cfrg(cfrg0, FOURCC('p', 'w', 'p', 'c'), kApplicationCFrag,
                                                 (StringPtr) ""))
        code0 = NULL;
    else if(!code0)
    {
        if(cfrg0)
        {
            if(ROMlib_find_cfrg(cfrg0, FOURCC('m', '6', '8', 'k'),
                                kApplicationCFrag, (StringPtr) ""))
                ROMlib_launch_failure = launch_cfm_requiring;
            else
                ROMlib_launch_failure = launch_ppc_only;
        }
        else
            ROMlib_launch_failure = launch_damaged;
        C_ExitToShell();
    }

    {
        Handle size_resource_h;
        int16_t size_flags;

        size_resource_h = Get1Resource(FOURCC('S', 'I', 'Z', 'E'), 0);
        if(size_resource_h == NULL)
            size_resource_h = Get1Resource(FOURCC('S', 'I', 'Z', 'E'), -1);
        if(size_resource_h)
        {
            SIZEResource *size_resource;

            size_resource = (SIZEResource *)STARH(size_resource_h);
            size_info.size_flags = CW(size_resource->size_flags);
            size_info.preferred_size = CL(size_resource->preferred_size);
            size_info.minimum_size = CL(size_resource->minimum_size);
            size_info.size_resource_present_p = true;
        }
        else
        {
            memset(&size_info, '\000', sizeof size_info);
            size_info.size_resource_present_p = false;
        }
        size_info.application_p = true;

        size_flags = size_info.size_flags;

        /* we don't accept open app events until a handler is installed */
        application_accepts_open_app_aevt_p = false;
        send_application_open_aevt_p
            = system_version >= 0x700
            && ((size_flags & SZisHighLevelEventAware)
                == SZisHighLevelEventAware);
    }

    if(!code0)
    {
        lp = 0; /* just to shut GCC up */
        jumplen = jumpoff = 0; /* just to shut GCC up */
        EM_A5 = US_TO_SYN68K(&tmpa5);
        LM(CurrentA5) = guest_cast<Ptr>(CL(EM_A5));
        InitGraf((Ptr)quickbytes + grafSize - 4);
    }
    else
    {
        HLock(code0);

        lp = (GUEST<LONGINT> *)STARH(code0);
        abovea5 = CL(*lp++);
        belowa5 = CL(*lp++);
        jumplen = CL(*lp++);
        jumpoff = CL(*lp++);

        /*
	 * NOTE: The stack initialization code that was here has been moved
	 *	     to ROMlib_InitZones in mman.c
	 */
        /* #warning Stack is getting reinitialized even when Chain is called ... */

        EM_A7 -= abovea5 + belowa5;
        LM(CurStackBase) = guest_cast<Ptr>(CL(EM_A7));

        LM(CurrentA5) = RM(MR(LM(CurStackBase)) + belowa5); /* set LM(CurrentA5) */
        LM(BufPtr) = RM(MR(LM(CurrentA5)) + abovea5);
        LM(CurJTOffset) = CW(jumpoff);
        EM_A5 = CL(guest_cast<LONGINT>(LM(CurrentA5)));
    }

    GetDateTime(&LM(Time));
    LM(ROMBase) = RM((Ptr)ROMlib_phoneyrom);
    LM(dodusesit) = LM(ROMBase);
    LM(QDExist) = LM(WWExist) = EXIST_NO;
    LM(TheZone) = LM(ApplZone);
    ROMlib_memnomove_p = true;

#if defined(NEXTSTEP)
    ROMlib_startapp();
#endif
    /*
 * NOTE: this memcpy has to be done after all local variables have been used
 *	 because it will quite possibly smash stuff that's on our stack.
 *	 In reality, we only see this if we compile without optimization,
 *	 but trust me, it was *very* confusing when this memcpy was up
 *	 before we were done with our local varibles.
 */
    if(code0)
    {
        memcpy(MR(LM(CurrentA5)) + jumpoff, lp, jumplen); /* copy in the
							 jump table */
        ROMlib_destroy_blocks(0, ~0, false);
    }
    SetCursor(STARH(GetCursor(watchCursor)));

    /* Call this routine in case the refresh value changed, either just
    * now or when the config file was parsed.  We want to do this
    * before the screen's depth might be changed.
    */
    {
        int save_ROMlib_refresh = ROMlib_refresh;
        dequeue_refresh_task();
        set_refresh_rate(0);
        set_refresh_rate(save_ROMlib_refresh);
    }

    ROMlib_uaf = 0;

    if(code0)
        beginexecutingat(CL(guest_cast<LONGINT>(LM(CurrentA5))) + CW(LM(CurJTOffset)) + 2);
    else
    {
        FSSpec fs;

        FSMakeFSSpec(ROMlib_exevrefnum, 0, ROMlib_exefname, &fs);
        cfm_launch(cfrg0, FOURCC('p', 'w', 'p', 'c'), &fs);
    }
}

void Executor::Chain(StringPtr fName, INTEGER vRefNum)
{
    launchchain(fName, vRefNum, false, 0);
}

static void reset_low_globals(void)
{
    /*
 * we're about to smash applzone ... we may want to verify a few low-mem
 * globals beforehand
 */

    GUEST<ProcPtr> saveDABeeper;
    GUEST<THz> saveSysZone;
    GUEST<uint32_t> saveTicks;
    GUEST<INTEGER> saveBootDrive;
    GUEST<LONGINT> saveLo3Bytes;
    GUEST<LONGINT> save20, save28, save58, save5C;
    GUEST<Ptr> saveSoundBase;
    GUEST<Ptr> saveVIA;
    GUEST<Ptr> saveSCCRd;
    GUEST<Ptr> saveSCCWr;
    GUEST<Handle> saveAppParmHandle;
    GUEST<QHdr> saveVCBQHdr;
    GUEST<QHdr> saveFSQHdr;
    GUEST<QHdr> saveDrvQHdr;
    GUEST<QHdr> saveEventQueue;
    GUEST<QHdr> saveVBLQueue;
    GUEST<Ptr> saveFCBSPtr;
    GUEST<Ptr> saveWDCBsPtr;
    GUEST<LONGINT> saveCurDirStore;
    GUEST<INTEGER> saveSFSaveDisk;
    GUEST<VCBPtr> saveDefVCBPtr;
    GUEST<char> saveCurApName[sizeof(LM(CurApName))];
    GUEST<INTEGER> saveCurApRefNum;
    GUEST<INTEGER> saveCurMap;
    GUEST<Handle> saveTopMapHndl;
    GUEST<Handle> saveSysMapHndl;
    GUEST<INTEGER> saveSysMap;
    GUEST<LONGINT> saveScrapSize;
    GUEST<Handle> saveScrapHandle;
    GUEST<INTEGER> saveScrapCount;
    GUEST<INTEGER> saveScrapState;
    GUEST<StringPtr> saveScrapName;
    GUEST<Handle> saveROMFont0;
    GUEST<Handle> saveWidthListHand;
    GUEST<Byte> saveSPValid;
    GUEST<Byte> saveSPATalkA;
    GUEST<Byte> saveSPATalkB;
    GUEST<Byte> saveSPConfig;
    GUEST<INTEGER> saveSPPortA;
    GUEST<INTEGER> saveSPPortB;
    GUEST<LONGINT> saveSPAlarm;
    GUEST<INTEGER> saveSPFont;
    GUEST<Byte> saveSPKbd;
    GUEST<Byte> saveSPPrint;
    GUEST<Byte> saveSPVolCtl;
    GUEST<Byte> saveSPClikCaret;
    GUEST<Byte> saveSPMisc2;
    GUEST<INTEGER> saveKeyThresh;
    GUEST<INTEGER> saveKeyRepThresh;
    GUEST<INTEGER> saveMenuFlash;
    GUEST<LONGINT> saveCaretTime;
    GUEST<LONGINT> saveDoubleTime;
    GUEST<LONGINT> saveDefDirID;
    GUEST<Handle> saveDAStrings[4];
    GUEST<Ptr> saveMemTop;
    GUEST<DCtlHandlePtr> saveUTableBase;
    GUEST<INTEGER> saveUnitNtryCnt;
    GUEST<Point> saveMouseLocation;
    GUEST<CGrafPtr> saveWMgrCPort;
    GUEST<Handle> saveMBDFHndl;
    GUEST<ProcPtr> saveJCrsrTask;

    GUEST<AE_info_t *> saveAE_info;

    GUEST<RGBColor> saveHiliteRGB;
    GUEST<GDHandle> saveTheGDevice, saveMainDevice, saveDeviceList;
    GUEST<char> saveKeyMap[sizeof_KeyMap];

    GUEST<Byte> saveFinderName[sizeof(LM(FinderName))];
    virtual_int_state_t bt;

    bt = block_virtual_ints();
    saveSysZone = LM(SysZone);
    saveTicks = LM(Ticks);
    saveBootDrive = LM(BootDrive);
    saveLo3Bytes = LM(Lo3Bytes);
    save20 = *(GUEST<LONGINT> *)SYN68K_TO_US(0x20);
    save28 = *(GUEST<LONGINT> *)SYN68K_TO_US(0x28);
    save58 = *(GUEST<LONGINT> *)SYN68K_TO_US(0x58);
    save5C = *(GUEST<LONGINT> *)SYN68K_TO_US(0x5C);
    saveVIA = LM(VIA);
    saveSCCRd = LM(SCCRd);
    saveSCCWr = LM(SCCWr);
    saveSoundBase = LM(SoundBase);
    saveAppParmHandle = LM(AppParmHandle);
    saveVCBQHdr = LM(VCBQHdr);
    saveFSQHdr = LM(FSQHdr);
    saveDrvQHdr = LM(DrvQHdr);
    saveFCBSPtr = LM(FCBSPtr);
    saveWDCBsPtr = LM(WDCBsPtr);
    saveSFSaveDisk = LM(SFSaveDisk);
    saveCurDirStore = LM(CurDirStore);
    saveEventQueue = LM(EventQueue);
    saveVBLQueue = LM(VBLQueue);
    saveDefVCBPtr = LM(DefVCBPtr);
    memcpy(saveCurApName, LM(CurApName), sizeof(LM(CurApName)));
    saveCurApRefNum = LM(CurApRefNum);
    saveCurMap = LM(CurMap);
    saveTopMapHndl = LM(TopMapHndl);
    saveSysMapHndl = LM(SysMapHndl);
    saveSysMap = LM(SysMap);
    saveScrapSize = LM(ScrapSize);
    saveScrapHandle = LM(ScrapHandle);
    saveScrapCount = LM(ScrapCount);
    saveScrapState = LM(ScrapState);
    saveScrapName = LM(ScrapName);
    saveROMFont0 = LM(ROMFont0);
    saveWidthListHand = LM(WidthListHand);
    saveSPValid = LM(SPValid);
    saveSPATalkA = LM(SPATalkA);
    saveSPATalkB = LM(SPATalkB);
    saveSPConfig = LM(SPConfig);
    saveSPPortA = LM(SPPortA);
    saveSPPortB = LM(SPPortB);
    saveSPAlarm = LM(SPAlarm);
    saveSPFont = LM(SPFont);
    saveSPKbd = LM(SPKbd);
    saveSPPrint = LM(SPPrint);
    saveSPVolCtl = LM(SPVolCtl);
    saveSPClikCaret = LM(SPClikCaret);
    saveSPMisc2 = LM(SPMisc2);
    saveKeyThresh = LM(KeyThresh);
    saveKeyRepThresh = LM(KeyRepThresh);
    saveMenuFlash = LM(MenuFlash);
    saveCaretTime = LM(CaretTime);
    saveDoubleTime = LM(DoubleTime);
    saveDefDirID = DefDirID;

    saveHiliteRGB = LM(HiliteRGB);
    saveTheGDevice = LM(TheGDevice);
    saveMainDevice = LM(MainDevice);
    saveDeviceList = LM(DeviceList);
    saveDAStrings[0] = LM(DAStrings)[0];
    saveDAStrings[1] = LM(DAStrings)[1];
    saveDAStrings[2] = LM(DAStrings)[2];
    saveDAStrings[3] = LM(DAStrings)[3];
    saveMemTop = LM(MemTop);
    saveUTableBase = LM(UTableBase);
    saveUnitNtryCnt = LM(UnitNtryCnt);

    saveMouseLocation = LM(MouseLocation);
    saveDABeeper = LM(DABeeper);

    memcpy(saveFinderName, LM(FinderName), sizeof(saveFinderName));
    saveWMgrCPort = LM(WMgrCPort);
    saveMBDFHndl = LM(MBDFHndl);

    saveJCrsrTask = LM(JCrsrTask);

    saveAE_info = LM(AE_info);
    memcpy(saveKeyMap, LM(KeyMap), sizeof_KeyMap);

    /* Set low globals to 0xFF, but don't touch exception vectors. */
    memset((char *)&LM(nilhandle) + 64 * sizeof(ULONGINT),
           0xFF,
           ((char *)&LM(lastlowglobal) - (char *)&LM(nilhandle)
            - 64 * sizeof(ULONGINT)));

    LM(AE_info) = saveAE_info;

    LM(JCrsrTask) = saveJCrsrTask;

    LM(MBDFHndl) = saveMBDFHndl;
    LM(WMgrCPort) = saveWMgrCPort;
    LM(WindowList) = NULL;
    memcpy(LM(FinderName), saveFinderName, sizeof(LM(FinderName)));

    LM(DABeeper) = saveDABeeper;
    LM(MouseLocation) = saveMouseLocation;
    LM(MouseLocation2) = saveMouseLocation;

    LM(UTableBase) = saveUTableBase;
    LM(UnitNtryCnt) = saveUnitNtryCnt;
    LM(MemTop) = saveMemTop;
    LM(DAStrings)[3] = saveDAStrings[3];
    LM(DAStrings)[2] = saveDAStrings[2];
    LM(DAStrings)[1] = saveDAStrings[1];
    LM(DAStrings)[0] = saveDAStrings[0];
    DefDirID = saveDefDirID;
    LM(DoubleTime) = saveDoubleTime;
    LM(CaretTime) = saveCaretTime;
    LM(MenuFlash) = saveMenuFlash;
    LM(KeyRepThresh) = saveKeyRepThresh;
    LM(KeyThresh) = saveKeyThresh;
    LM(SPMisc2) = saveSPMisc2;
    LM(SPClikCaret) = saveSPClikCaret;
    LM(SPVolCtl) = saveSPVolCtl;
    LM(SPPrint) = saveSPPrint;
    LM(SPKbd) = saveSPKbd;
    LM(SPFont) = saveSPFont;
    LM(SPAlarm) = saveSPAlarm;
    LM(SPPortB) = saveSPPortB;
    LM(SPPortA) = saveSPPortA;
    LM(SPConfig) = saveSPConfig;
    LM(SPATalkB) = saveSPATalkB;
    LM(SPATalkA) = saveSPATalkA;
    LM(SPValid) = saveSPValid;
    LM(WidthListHand) = saveWidthListHand;
    LM(ROMFont0) = saveROMFont0;
    LM(ScrapName) = saveScrapName;
    LM(ScrapState) = saveScrapState;
    LM(ScrapCount) = saveScrapCount;
    LM(ScrapHandle) = saveScrapHandle;
    LM(ScrapSize) = saveScrapSize;
    LM(SysMap) = saveSysMap;
    LM(SysMapHndl) = saveSysMapHndl;
    LM(TopMapHndl) = saveTopMapHndl;
    LM(CurMap) = saveCurMap;
    LM(CurApRefNum) = saveCurApRefNum;
    memcpy(LM(CurApName), saveCurApName, sizeof(LM(CurApName)));
    LM(DefVCBPtr) = saveDefVCBPtr;
    LM(VBLQueue) = saveVBLQueue;
    LM(EventQueue) = saveEventQueue;
    LM(CurDirStore) = saveCurDirStore;
    LM(SFSaveDisk) = saveSFSaveDisk;
    LM(WDCBsPtr) = saveWDCBsPtr;
    LM(FCBSPtr) = saveFCBSPtr;
    LM(DrvQHdr) = saveDrvQHdr;
    LM(FSQHdr) = saveFSQHdr;
    LM(VCBQHdr) = saveVCBQHdr;
    LM(Lo3Bytes) = saveLo3Bytes;
    LM(VIA) = saveVIA;
    LM(SCCRd) = saveSCCRd;
    LM(SCCWr) = saveSCCWr;
    LM(SoundBase) = saveSoundBase;
    LM(Ticks) = saveTicks;
    LM(SysZone) = saveSysZone;
    LM(BootDrive) = saveBootDrive;
    LM(AppParmHandle) = saveAppParmHandle;

    LM(HiliteRGB) = saveHiliteRGB;
    LM(TheGDevice) = saveTheGDevice;
    LM(MainDevice) = saveMainDevice;
    LM(DeviceList) = saveDeviceList;

    restore_virtual_ints(bt);

    LM(nilhandle) = 0; /* so nil dereferences "work" */

    LM(CrsrBusy) = 0;
    LM(TESysJust) = 0;
    LM(DSAlertTab) = 0;
    LM(ResumeProc) = 0;
    LM(GZRootHnd) = 0;
    LM(ANumber) = 0;
    LM(ResErrProc) = 0;
#if 0
    LM(FractEnable) = 0xff;	/* NEW MOD -- QUESTIONABLE */
#else
    LM(FractEnable) = 0;
#endif
    LM(SEvtEnb) = 0;
    LM(MenuList) = 0;
    LM(MBarEnable) = 0;
    LM(MenuFlash) = 0;
    LM(TheMenu) = 0;
    LM(MBarHook) = 0;
    LM(MenuHook) = 0;
    LM(HeapEnd) = 0;
    LM(ApplLimit) = 0;
    LM(SoundActive) = soundactiveoff;
    LM(PortBUse) = 2; /* configured for Serial driver */

    memcpy(LM(KeyMap), saveKeyMap, sizeof_KeyMap);
    LM(OneOne) = CLC(0x00010001);
    LM(DragHook) = 0;
    LM(MBDFHndl) = 0;
    LM(MenuList) = 0;
    LM(MBSaveLoc) = 0;
    LM(SysFontFam) = 0;

    LM(SysVersion) = CW(system_version);
    LM(FSFCBLen) = CWC(94);

    /*
 * TODO:  how does this relate to Launch?
 */
    /* Set up default floating point environment. */
    {
        INTEGER env = 0;
        ROMlib_Fsetenv(&env, 0);
    }

    LM(TEDoText) = RM((ProcPtr)&ROMlib_dotext); /* where should this go ? */

    LM(WWExist) = LM(QDExist) = EXIST_NO; /* TODO:  LOOK HERE! */
    LM(SCSIFlags) = CWC(0xEC00); /* scsi+clock+xparam+mmu+adb
				 (no fpu,aux or pwrmgr) */

    LM(MMUType) = 5;
    LM(KbdType) = 2;

    LM(MCLKPCmiss1) = 0; /* &LM(MCLKPCmiss1) = 0x358 + 72 (MacLinkPC starts
			   adding the 72 byte offset to VCB pointers too
			   soon, starting with 0x358, which is not the
			   address of a VCB) */

    LM(MCLKPCmiss2) = 0; /* &LM(MCLKPCmiss1) = 0x358 + 78 (MacLinkPC misses) */
    LM(AuxCtlHead) = 0;
    LM(CurDeactive) = 0;
    LM(CurActivate) = 0;
    LM(macfpstate)[0] = 0;
    LM(fondid) = 0;
    LM(PrintErr) = 0;
    LM(mouseoffset) = 0;
    LM(heapcheck) = 0;
    LM(DefltStack) = CLC(0x2000); /* nobody really cares about these two */
    LM(MinStack) = CLC(0x400); /* values ... */
    LM(IAZNotify) = 0;
    LM(CurPitch) = 0;
    LM(JSwapFont) = RM((ProcPtr)&FMSwapFont);
    LM(JInitCrsr) = RM((ProcPtr)&InitCursor);

    LM(JHideCursor) = RM((ProcPtr)&HideCursor);
    LM(JShowCursor) = RM((ProcPtr)&ShowCursor);
    LM(JShieldCursor) = RM((ProcPtr)&ShieldCursor);
    LM(JSetCrsr) = RM((ProcPtr)&SetCursor);
    LM(JCrsrObscure) = RM((ProcPtr)&ObscureCursor);

    LM(JUnknown574) = RM ((ProcPtr)&unknown574);

    LM(Key1Trans) = RM((Ptr)&stub_Key1Trans);
    LM(Key2Trans) = RM((Ptr)&stub_Key2Trans);
    LM(JFLUSH) = RM(&stub_flushcache);
    LM(JResUnknown1) = LM(JFLUSH); /* I don't know what these are supposed to */
    LM(JResUnknown2) = LM(JFLUSH); /* do, but they're not called enough for
				   us to worry about the cache flushing
				   overhead */

    LM(CPUFlag) = 4; /* mc68040 */
    LM(UnitNtryCnt) = 0; /* how many units in the table */

    LM(TheZone) = LM(ApplZone);

    *(GUEST<LONGINT> *)SYN68K_TO_US(0x20) = save20;
    *(GUEST<LONGINT> *)SYN68K_TO_US(0x28) = save28;
    *(GUEST<LONGINT> *)SYN68K_TO_US(0x58) = save58;
    *(GUEST<LONGINT> *)SYN68K_TO_US(0x5C) = save5C;

    LM(HiliteMode) = CB(0xFF); /* I think this is correct */
    LM(ROM85) = CWC(0x3FFF); /* We be color now */
    LM(MMU32Bit) = 0x01;
    LM(loadtrap) = 0;
    *(GUEST<LONGINT> *)SYN68K_TO_US(0x1008) = CLC(0x4); /* Quark XPress 3.0 references 0x1008
					explicitly.  It takes the value
					found there, subtracts four from
					it and dereferences that value.
					Yahoo */
    *(GUEST<int16_t> *)SYN68K_TO_US(4) = CWC(0x4e75); /* RTS, so when we dynamically recompile
				    code starting at 0 we won't get far */

    /* Micro-cap dereferences location one of the LM(AppPacks) locations */

    {
        int i;

        for(i = 0; i < (int)NELEM(LM(AppPacks)); ++i)
            LM(AppPacks)[i] = 0;
    }
    LM(SysEvtMask) = CWC(~(1L << keyUp)); /* EVERYTHING except keyUp */
    LM(SdVolume) = 7; /* for Beebop 2 */
    LM(CurrentA5) = guest_cast<Ptr>(CL(EM_A5));
}

static void reset_traps(void)
{
    static syn68k_addr_t savetooltraptable[0x400];
    static syn68k_addr_t saveostraptable[0x100];
    static BOOLEAN beenhere = false;

    ROMlib_reset_bad_trap_addresses();
    if(!beenhere)
    {
        memcpy(saveostraptable, ostraptable, sizeof(saveostraptable));
        memcpy(savetooltraptable, tooltraptable, sizeof(savetooltraptable));
        beenhere = true;
    }
    else
    {
        /*
 * NOTE: I'm not preserving patches that go into the SystemZone.  Right now
 *       that just seems like an unnecessary source of mystery bugs.
 */
        memcpy(ostraptable, saveostraptable, sizeof(saveostraptable));
        memcpy(tooltraptable, savetooltraptable, sizeof(savetooltraptable));
    }
}

static bool
our_special_map(resmaphand map)
{
    bool retval;
    Handle h;

    LM(CurMap) = STARH(map)->resfn;
    h = Get1Resource(TICK("nUSE"), 0);
    retval = h ? true : false;

    return retval;
}

void Executor::empty_timer_queues(void)
{
    TMTask *tp, *nexttp;
    VBLTaskPtr vp, nextvp;
    virtual_int_state_t bt;

    bt = block_virtual_ints();

    dequeue_refresh_task();
    clear_pending_sounds();
    for(vp = (VBLTaskPtr)MR(LM(VBLQueue).qHead); vp; vp = nextvp)
    {
        nextvp = (VBLTaskPtr)MR(vp->qLink);
        VRemove(vp);
    }
    for(tp = (TMTask *)MR(ROMlib_timehead.qHead); tp; tp = nexttp)
    {
        nexttp = (TMTask *)MR(tp->qLink);
        RmvTime((QElemPtr)tp);
    }

    restore_virtual_ints(bt);
}

static void reinitialize_things(void)
{
    resmaphand map, nextmap;
    filecontrolblock *fcbp, *efcbp;
    INTEGER length;
    INTEGER special_fn;
    int i;

    ROMlib_shutdown_font_manager();
    SetZone(MR(LM(SysZone)));
    /* NOTE: we really shouldn't be closing desk accessories at all, but
       since we don't properly handle them when they're left open, it is
       better to close them down than not.  */

    for(i = DESK_ACC_MIN; i <= DESK_ACC_MAX; ++i)
        CloseDriver(-i - 1);

    empty_timer_queues();
    ROMlib_clock = 0; /* CLOCKOFF */

    special_fn = 0;
    for(map = (resmaphand)MR(LM(TopMapHndl)); map; map = nextmap)
    {
        nextmap = (resmaphand)HxP(map, nextmap);
        if(HxX(map, resfn) == LM(SysMap))
            UpdateResFile(Hx(map, resfn));
        else
        {
            if(!our_special_map(map))
                CloseResFile(Hx(map, resfn));
            else
            {
                special_fn = Hx(map, resfn);
                UpdateResFile(special_fn);
            }
        }
    }

    length = CW(*(GUEST<int16_t> *)MR(LM(FCBSPtr)));
    fcbp = (filecontrolblock *)((short *)MR(LM(FCBSPtr)) + 1);
    efcbp = (filecontrolblock *)((char *)MR(LM(FCBSPtr)) + length);
    for(; fcbp < efcbp;
        fcbp = (filecontrolblock *)((char *)fcbp + CW(LM(FSFCBLen))))
    {
        INTEGER rn;

        rn = (char *)fcbp - (char *)MR(LM(FCBSPtr));
        if(fcbp->fcbCName[0]
           /* && rn != Param_ram_rn */
           && rn != CW(LM(SysMap))
           && rn != special_fn)
            FSClose((char *)fcbp - (char *)MR(LM(FCBSPtr)));
    }

    LM(CurMap) = STARH((resmaphand)MR(LM(TopMapHndl)))->resfn;

    ROMlib_destroy_blocks(0, ~0, false);
}

static OSErr
ROMlib_filename_from_fsspec(char **strp, FSSpec *fsp)
{
    OSErr retval;
    ParamBlockRec pbr;
    char *filename, *endname;
    VCBExtra *vcbp;
    struct stat sbuf;

    memset(&pbr, 0, sizeof pbr);
    pbr.ioParam.ioVRefNum = fsp->vRefNum;
    pbr.ioParam.ioNamePtr = RM((StringPtr)fsp->name);
    retval = ROMlib_nami(&pbr, CL(fsp->parID), NoIndex, strp, &filename,
                         &endname, false, &vcbp, &sbuf);
    return retval;
}

OSErr
Executor::NewLaunch(StringPtr fName_arg, INTEGER vRefNum_arg, LaunchParamBlockRec *lpbp)
{
    OSErr retval;
    static char beenhere = false;
    static jmp_buf buf;
    static Str255 fName;
    static INTEGER vRefNum;
    static LaunchParamBlockRec lpb;
    BOOLEAN extended_p;

    retval = noErr;
    if(lpbp && lpbp->launchBlockID == CWC(extendedBlock))
    {
        lpb = *lpbp;
        str255assign(fName, (MR(lpbp->launchAppSpec))->name);
        extended_p = true;
    }
    else
    {
        lpb.launchBlockID = 0;
        str255assign(fName, fName_arg);
        vRefNum = vRefNum_arg;
        extended_p = false;
    }

    if(extended_p && (lpbp->launchControlFlags & CWC(launchContinue)))
    {
        int n_filenames;
        char **filenames;
        int i;
        AppParametersPtr ap;
        int n_filename_bytes;

#if !defined(LETGCCWAIL)
        ap = NULL;
#endif
        n_filenames = 1;
        if(lpbp->launchAppParameters)
        {
            ap = MR(lpbp->launchAppParameters);
            n_filenames += CW(ap->n_fsspec);
        }
        n_filename_bytes = n_filenames * sizeof *filenames;
        filenames = (char **)alloca(n_filename_bytes);
        memset(filenames, 0, n_filename_bytes);
        retval = ROMlib_filename_from_fsspec(&filenames[0],
                                             MR(lpbp->launchAppSpec));
        for(i = 1; retval == noErr && i < n_filenames; ++i)
            retval = ROMlib_filename_from_fsspec(&filenames[1],
                                                 &ap->fsspec[i - 1]);
        if(retval == noErr)
            retval = ROMlib_launch_native_app(n_filenames, filenames);
        for(i = 0; i < n_filenames; ++i)
            free(filenames[i]);
    }
    else
    {
        /* This setjmp/longjmp code might be better put in launchchain */
        if(!beenhere)
        {
            beenhere = true;
            setjmp(buf);
        }
        else
            longjmp(buf, 1);

        retval = noErr;
        reset_adb_vector();
        reinitialize_things();
        reset_traps();
        reset_low_globals();
        ROMlib_InitZones();
        ROMlib_InitGWorlds();
        hle_reinit();
        AE_reinit();
        print_reinit();

        gd_set_bpp(MR(LM(MainDevice)), !vdriver_grayscale_p, vdriver_fixed_clut_p,
                   vdriver_bpp);
        ROMlib_init_stdfile();
#if ERROR_SUPPORTED_P(ERROR_UNEXPECTED)
        if(ERROR_ENABLED_P(ERROR_UNEXPECTED))
        {
            uintptr_t lp;

            for(lp = (uintptr_t)&LM(nilhandle); lp <= (uintptr_t)&LM(lastlowglobal); lp += 2)
                if(lp != (uintptr_t)&LM(TheZone)
                   && lp != (uintptr_t)&LM(ApplZone)
                   && lp != (uintptr_t)&LM(FSFCBLen)
                   && lp != (uintptr_t)&LM(SysMap)
                   && lp != (uintptr_t)SYN68K_TO_US(0x2f6)
                   && lp != (uintptr_t)SYN68K_TO_US(0x8e6)
                   && lp != (uintptr_t)SYN68K_TO_US(0x900)
                   && lp != (uintptr_t)&LM(CurMap)
                   && lp != (uintptr_t)SYN68K_TO_US(0x8a6)
                   && lp != (uintptr_t)SYN68K_TO_US(0x8aa)
                   && lp != (uintptr_t)SYN68K_TO_US(0x268)
                   && lp != (uintptr_t)SYN68K_TO_US(0x982)
                   && lp != (uintptr_t)SYN68K_TO_US(0xaee)
                   && lp != (uintptr_t)SYN68K_TO_US(0xcca)
                   && lp != (uintptr_t)SYN68K_TO_US(0xd50)
                   && lp != (uintptr_t)SYN68K_TO_US(0x18e)
                   && lp != (uintptr_t)SYN68K_TO_US(0x190)
                   && lp != (uintptr_t)SYN68K_TO_US(0x2f2)
                   && lp != (uintptr_t)SYN68K_TO_US(0x11e)
                   && lp != (uintptr_t)SYN68K_TO_US(0x15c)
                   && lp != (uintptr_t)SYN68K_TO_US(0x27e)
                   && lp != (uintptr_t)SYN68K_TO_US(0x31a)
                   && lp != (uintptr_t)SYN68K_TO_US(0x82c)
                   && lp != (uintptr_t)SYN68K_TO_US(0x82e)
                   && lp != (uintptr_t)SYN68K_TO_US(0x830)
                   && lp != (uintptr_t)SYN68K_TO_US(0x832)
                   && lp != (uintptr_t)SYN68K_TO_US(0xa4a)
                   && lp != (uintptr_t)SYN68K_TO_US(0xa52)
                   && lp != (uintptr_t)SYN68K_TO_US(0xa56)
                   && lp != (uintptr_t)SYN68K_TO_US(0xbf4)
                   && lp != (uintptr_t)SYN68K_TO_US(0x828)
                   && lp != (uintptr_t)SYN68K_TO_US(0x82a)
                   && lp != (uintptr_t)SYN68K_TO_US(0x16c))
                    if(MR(*(GUEST<void *> *)lp) >= MR(LM(ApplZone))
                       && MR(*(GUEST<void *> *)lp) < MR(MR(LM(ApplZone))->bkLim))
                        warning_unexpected("Low global at 0x%x may point into "
                                           "LM(ApplZone) and probably shouldn't.",
                                           (unsigned int)US_TO_SYN68K(lp));
        }
#endif
        launchchain(fName, vRefNum, true, &lpb);
    }
    return retval;
}

void Executor::Launch(StringPtr fName_arg, INTEGER vRefNum_arg)
{
#if 0
  /* NOTE: we're messing with launch between Executor 2 beta 1 and Executor 2
     proper.  We really shouldn't do anything differently from what we were
     doing before, so we don't have the liberty to do things cleanly */

  LaunchParamBlockRec pbr;

  memset (&pbr, 0, sizeof pbr);
  pbr.launchBlockID = CWC (extendedBlock);
  pbr.launchEPBLength = CLC (extendedBlockLen);
  pbr.launchControlFlags = CWC (launchNoFileFlags|launchInhibitDaemon);
  FSMakeFSSpec (vRefNum_arg, 0, fName_arg, &pbr.launchAppSpec);
  pbr.launchAppSpec.vRefNum = vRefNum_arg);
  NewLaunch (&pbr);
#else
    NewLaunch(fName_arg, vRefNum_arg, 0);
#endif
}
