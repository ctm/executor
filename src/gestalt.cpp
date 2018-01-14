/* Copyright 1992 - 1999 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in Gestalt.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "MemoryMgr.h"
#include "Gestalt.h"

#include "rsys/prefs.h"
#include "rsys/soundopts.h"
#include "rsys/mman.h"
#include "rsys/gestalt.h"
#include "rsys/version.h"
#include "rsys/vdriver.h"

using namespace Executor;

typedef struct
{
    OSType selector;
    /*LONGINT*/
    intptr_t value;
} gestaltentry_t;

#define UNKNOWN 0xFFFF

static gestaltentry_t gtable[] = {
    {
        gestaltAddressingModeAttr, gestalt32BitAddressing | gestalt32BitSysZone | gestalt32BitCapable,
    },

    /*
 * note: PageMaker 4.2 does a Gestalt call for the AliasMgr and then
 *	 proceeds to use the Alias Manager, even if we say it's not
 *	 present.  However, it will avoid use of the Alias Manager
 *	 if the Gestalt call itself returns an error.  This doesn't
 *	 appear to be within spec (IMVI), but it looks like we should
 *	 just not know the descriptors that we don't support.  Boo.
 *
 *       MacInTax '93 does the same thing with the Edition Manager that
 *	 PageMaker does with the Alias Manager.  It's unclear how widespread
 *	 this problem is.
 *
 *	 Claris Works does the same thing with Apple Events... bad!
 */

    /*  gestaltAliasMgrAttr,	0,	*/
    /*  gestaltApplEventsAttr,	0,	*/
    /*  gestaltAppleTalkVersion,	0,	*/
    /*  gestaltAUXVersion,		UNKNOWN,*/ /* Should return gestaltUnknownErr */
    /*  gestaltConnMgrAttr,		0,	*/
    /*  gestaltCRMAttr,		0,	*/
    /*  gestaltCTBVersion,		UNKNOWN,*/ /* should return gestaltUnknownErr */
    /*  gestaltDBAccessMgrAttr,	0,	*/
    {
        gestaltDITLExtAttr, gestaltDITLExtPresent,
    },
    /*  gestaltEasyAccessAttr,	gestaltEasyAccessOff,	*/

    /*  gestaltEditionMgrAttr,	0, */

    { gestaltFindFolderAttr, 1 },

    /*  gestaltFontMgrAttr,		0,	*/
    {
        gestaltFSAttr, gestaltStandardFile58 | gestaltHasFSSpecCalls,
    },
    /*  gestaltFXfrMgrAttr,		0,	*/
    /*  gestaltHardwareAttr,	0,	*/
    /*  gestaltHelpMgrAttr,		0,	*/
    /*  gestaltMiscAttr,		0,	*/
    /*  gestaltNotificatinMgrAttr,	0,	*/
    /*  gestaltNuBusConnectors,	0,	*/
    /*  gestaltOSAttr,		0,	*/
    /*  gestaltPopupAttr,		0,	*/
    /*  gestaltPowerMgrAttr,	0,	*/
    /*  gestaltPPCToolboxAttr,	0,	*/
    /*  gestaltResourceMgrAttr,	0,	*/
    /*  gestaltScriptCount,		0,	*/
    /*  gestaltScriptMgrVersion,	UNKNOWN,*/ /* should return gestaltUnknownErr */
    /*  gestaltSerialAttr,		0,	*/
    {
        gestaltSoundAttr, 0x0C00,
    },
/*  gestaltStdNBPAttr,		0,	*/
/*  gestaltTermMgrAttr,		0,	*/
/*  gestaltParityAttr,		0,	*/

/*
 * TODO: instead of casting these to LONGINT, shouldn't we be using something
 *       like US_TO_SYN68K here, or if not, soemthing that takes ROMlib_offset
 *       into account?
 * HALF-DONE: we're using intptr_t now, so it works on 64bit as well.
 */

    {
        gestaltExtToolboxTable, (intptr_t)(tooltraptable + 0x200),
    },
    {
        gestaltToolboxTable, (intptr_t)tooltraptable,
    },
    {
        gestaltOSTable, (intptr_t)ostraptable,
    },

    {
        gestaltFPUType, 0,
    }, /* gestaltNoFPU */
    {
        gestaltKeyboardType, gestaltMacKbd,
    },
    {
        gestaltLogicalPageSize, 8 * 1024,
    },
    {
        gestaltLogicalRAMSize, (LONGINT)3 * 1024 * 1024,
    },
    {
        gestaltLowMemorySize, 0x1000,
    },
    {
        gestaltMMUType, gestaltNoMMU,
    },
    {
        gestaltPhysicalRAMSize, (LONGINT)3 * 1024 * 1024,
    },
    {
        gestaltProcessorType, gestalt68040,
    },

#if defined(powerpc) || defined(__ppc__)
    {
        gestaltNativeCPUtype, gestaltCPU750,
    },
    {
        gestaltMachineType, gestaltMacQuadra610,
    }, /* change this */

    {
        gestaltSysArchitecture, gestaltPowerPC,
    },
    // FIXME: #warning questionable gestalt values for StuffitExpander
    { gestaltOSAttr, 0x3FF },
    { FOURCC('t', 'h', 'd', 's'), 0 },
#else
    {
        gestaltNativeCPUtype, gestaltCPU68040,
    },
    {
        gestaltMachineType, gestaltMacQuadra610,
    },
#endif

#if defined(ORIGINAL_QD)
    {
        gestaltQuickdrawVersion, gestaltOriginalQD,
    },
#else
    /*
 * Star Trek demo needs 32-bit QD 1.1 at least.
 */
    { gestaltQuickdrawVersion, gestalt32BitQD13 },
#endif
    { gestaltQuickdrawFeatures, ((1 << gestaltHasColor)
                                 | (1 << gestaltHasDeepGWorlds)
                                 | (1 << gestaltHasDirectPixMaps)) },

    {
        gestaltTimeMgrVersion, gestaltStandardTimeMgr,
    },
    {
        gestaltVersion, 1,
    },

/* We don't want ClarisWorks 3 calling funky IMVI memory manager
   * routines so we don't recognize gestaltVMAddr at all.  NOTE:
   * ClarisWorks 3 will also not call those "funky routines" if we
   * recognize the gestaltVMAttr selector, but say we don't support
   * it.  Furthermore, saying we don't support it breaks MacBench
   * (which is kludgily worked around elsewhere).  The ONLY REASON we
   * say we don't even recognize this selector (and do the MacBench
   * hack elsewhere) is paranoia: "our original ClarisWorks workaround
   * was to not recognize this selector, and we've done much testing
   * since then.  Perhaps some other program out there would die if we
   * recognized the VM selector."
   *
   * Once Executor 2 is out and we are next free to potentially break
   * programs, we should take out this #if 0 and recognize the VMAttr
   * selector, but claim we don't have any VM.
   */
#if 0
  { gestaltVMAttr, 0 },
#endif
    {
        gestaltMachineIcon, UNKNOWN,
    },
    {
        gestaltROMSize, (LONGINT)128 * 1024,
    },
    {
        gestaltROMVersion, 0x7c,
    },
    {
        gestaltSystemVersion, 0x607,
    },
    {
        gestaltTextEditVersion, gestaltTE1,
    },

#if 0
// FIXME: #warning DO NOT CHECK THIS IN
  { T ('d', 'r', 'a', 'g'),	0, },
#endif

    /* The following four will be ignored unless they are specifically enabled
   by the corresponding ROMlib_pretend_xxx flags */

    { gestaltHelpMgrAttr, 1 }, /* must be 1 */
    { gestaltScriptMgrVersion, 0x001 }, /* have gotten away with 0x001 */
    { gestaltEditionMgrAttr, 1 }, /* must be 1 */
    { gestaltAliasMgrAttr, 1 }, /* must be 1 */

    { gestaltPHYSICAL, 0 }, /* will be filled in w/ callback address */

#define SYSTEM_7_EXTRAS 2

    {
        gestaltStandardFileAttr, gestaltStandardFile58,
    },
    { gestaltApplEventsAttr, 1 }, /* got away with 0, but that may
				        give us trouble elsewhere */

#if defined(THESE_REALLY_ARENT_NEEDED)
    { FOURCC('i', 'c', 'm', 'p'), 17 }, /* QuickTime ... don't want this! */
    { FOURCC('s', 'y', 's', 'a'), 0x4F30 },
    { gestaltOSAttr, 0x3FF },
    { gestaltPopupAttr, 1 }, /* perhaps we should admit to this */
    { gestaltFontMgrAttr, 7 },
#endif

};

static OSType gtable_selectors_to_patch[] = {
    gestaltExtToolboxTable,
    gestaltToolboxTable,
    gestaltOSTable,
};

/*
gestaltSystemVersion   		'sysv' Executor version numeric encoding
gestaltPhysicalRAMSize		'ram '
*/

#define TO_BE_FILLED_IN (0)

static gestaltentry_t phystable[] = {
    { gestaltSystemVersion, EXECUTOR_VERSION_NUMERIC },
#if SIZEOF_CHAR_P == 4
    { gestaltExecutorVersionString, (long)EXECUTOR_VERSION },
#else
// FIXME: #warning gestaltExecutorVersionString not currently available
#endif
#if defined(LINUX) || defined(CYGWIN32)
    { gestaltPhysicalRAMSize, TO_BE_FILLED_IN },
#endif
    { gestaltScreenSize, TO_BE_FILLED_IN },
#if defined(CYGWIN32)
    { gestaltGhostScriptVersion, TO_BE_FILLED_IN },
#endif
};

static OSType phystable_selectors_to_patch[] = {
    gestaltExecutorVersionString,
};

typedef struct gestalt_link_str
{
    OSType selector;
    ProcPtr selectorFunction;
    struct gestalt_link_str *next;
} gestalt_link_t;

static gestalt_link_t *gestalt_head;

static gestalt_link_t *
find_selector_on_list(OSType selector)
{
    gestalt_link_t *gp;

    for(gp = gestalt_head; gp && gp->selector != selector; gp = gp->next)
        ;
    return gp;
}

static gestaltentry_t *
find_selector_in_table(OSType selector, gestaltentry_t table[],
                       int table_length)
{
    INTEGER i;
    gestaltentry_t *retval;

    if(system_version < 0x700 && table == gtable)
        table_length -= SYSTEM_7_EXTRAS;

    switch(selector)
    {
        case gestaltHelpMgrAttr:
            if(!ROMlib_pretend_help)
                selector = -1;
            break;
        case gestaltScriptMgrVersion:
            if(!ROMlib_pretend_script)
                selector = -1;
            break;
        case gestaltEditionMgrAttr:
            if(!ROMlib_pretend_edition)
                selector = -1;
            break;
        case gestaltAliasMgrAttr:
            if(!ROMlib_pretend_alias)
                selector = -1;
            break;
    }

    for(i = 0; i < table_length && table[i].selector != selector; ++i)
        ;
    if(i < table_length)
        retval = &table[i];
    else
        retval = 0;

    return retval;
}

static void
replace_selector_in_table(OSType selector, LONGINT new_value,
                          gestaltentry_t table[], int table_length)
{
    gestaltentry_t *gp;

    gp = find_selector_in_table(selector, table, table_length);
    if(gp)
        gp->value = new_value;
}

#define REPLACE_SELECTOR_IN_TABLE(selector, new_value, table) \
    replace_selector_in_table(selector, new_value, table, NELEM(table))

void
Executor::replace_physgestalt_selector(OSType selector, uint32_t new_value)
{
    REPLACE_SELECTOR_IN_TABLE(selector, new_value, phystable);
}

void
Executor::gestalt_set_memory_size(uint32_t size)
{
    REPLACE_SELECTOR_IN_TABLE(gestaltLogicalRAMSize, size, gtable);
    REPLACE_SELECTOR_IN_TABLE(gestaltPhysicalRAMSize, size, gtable);
}

void
Executor::gestalt_set_system_version(uint32_t version)
{
    REPLACE_SELECTOR_IN_TABLE(gestaltSystemVersion, version, gtable);
}

void
gestalt_set_physical_gestalt_callback(void)
{
    REPLACE_SELECTOR_IN_TABLE(gestaltPHYSICAL,
                              (long)US_TO_SYN68K((ProcPtr)&PhysicalGestalt), gtable);
}

#if defined(powerpc) || defined(__ppc__)
void
gestalt_set_cpu_type(uint32_t type)
{
    REPLACE_SELECTOR_IN_TABLE(gestaltPHYSICAL, type, gtable);
}
#endif

static OSErrRET
gestalt_helper(OSType selector, GUEST<LONGINT> *responsep, BOOLEAN searchlist,
               gestaltentry_t table[], int length)
{
    OSErr retval;
    gestalt_link_t *gp;

    retval = gestaltUndefSelectorErr;

    if(selector == gestaltSoundAttr && ROMlib_PretendSound == soundoff)
        selector = 0;

    *responsep = CLC(0); /* better safe than sorry */

    if(searchlist && (gp = find_selector_on_list(selector)))
        retval = CToPascalCall((void *)gp->selectorFunction, ctop(&C_GestaltTablesOnly),
                               selector, responsep);
    else
    {
        gestaltentry_t *gep;

        gep = find_selector_in_table(selector, table, length);
        if(gep)
        {
            if(gep->value == UNKNOWN)
                retval = gestaltUnknownErr;
            else
            {
                retval = noErr;
                *responsep = CL(gep->value);
            }
        }
    }

    /* Log the gestalt call if appropriate. */
    warning_trace_info("sel `%c%c%c%c'; rsp `%d'; retval `%d'",
                       (selector >> 24) & 0xFF,
                       (selector >> 16) & 0xFF,
                       (selector >> 8) & 0xFF,
                       (selector)&0xFF,
                       CL(*responsep),
                       retval);

    return retval;
}

static void
offset_address(gestaltentry_t *gp, int ngp, OSType sel)
{
    for(; ngp > 0 && gp->selector != sel; --ngp, ++gp)
        ;
    if(ngp > 0)
        gp->value = US_TO_SYN68K(gp->value);
}

static void
offset_addresses(gestaltentry_t *gp, int ngp, OSType *selp, int nsels)
{
    while(nsels-- > 0)
        offset_address(gp, ngp, *selp++);
}

#define OFFSET_ADDRESSES(table)                  \
    offset_addresses(table, NELEM(table),        \
                     table##_selectors_to_patch, \
                     NELEM(table##_selectors_to_patch))

/*
 *  This is for gestalt entries that are hard-wired via configuration
 *  files.  They can't be altered by ReplaceGestalt.
 */

typedef struct
{
    OSType selector;
    OSErr retval;
    GUEST<uint32_t> value;
} gestalt_list_entry_t;

static gestalt_list_entry_t *gestalt_listp = NULL;
static size_t listp_size = 0;

void
Executor::ROMlib_clear_gestalt_list(void)
{
    free(gestalt_listp);
    gestalt_listp = NULL;
    listp_size = 0;
}

void
Executor::ROMlib_add_to_gestalt_list(OSType selector, OSErr retval, uint32_t new_value)
{
    decltype(gestalt_listp) new_listp;

    new_listp = (decltype(gestalt_listp))realloc(gestalt_listp, listp_size + sizeof *gestalt_listp);
    if(!new_listp)
        warning_unexpected("out of memory");
    else
    {
        decltype(gestalt_listp) entryp;

        gestalt_listp = new_listp;
        entryp = (decltype(entryp))((char *)gestalt_listp + listp_size);
        entryp->selector = selector;
        entryp->retval = retval;
        entryp->value = CL(new_value);
        listp_size += sizeof *gestalt_listp;
    }
}

OSErrRET Executor::Gestalt(OSType selector, GUEST<LONGINT> *responsep)
{
    static bool been_here = false;

    warning_trace_info("IN: sel `%c%c%c%c'",
                       (selector >> 24) & 0xFF,
                       (selector >> 16) & 0xFF,
                       (selector >> 8) & 0xFF,
                       (selector)&0xFF);
#if defined(CYGWIN32)
    if((uint32_t)selector == 0xb7d20e84)
    {
        OSErrRET retval;

        warning_trace_info("about to dongle_query");
        retval = dongle_query(responsep);
        warning_trace_info("dongle_queried");
        return retval;
    }
#endif

    {
        gestalt_list_entry_t *p, *ep;

        for(p = gestalt_listp, ep = (decltype(ep))((char *)p + listp_size);
            p != ep && p->selector != selector;
            ++p)
            ;
        if(p != ep)
        {
            *responsep = p->value;
            return p->retval;
        }
    }

    if(!been_here)
    {
        gestalt_set_physical_gestalt_callback();
        OFFSET_ADDRESSES(gtable);
        OFFSET_ADDRESSES(phystable);
        been_here = true;
    }
    return gestalt_helper(selector, responsep, true, gtable, NELEM(gtable));
}

OSErrRET Executor::C_PhysicalGestalt(OSType selector, GUEST<LONGINT> *responsep)
{
    OSErrRET retval;

    switch(selector)
    {
        case gestaltScreenSize:
            replace_physgestalt_selector(gestaltScreenSize,
                                         ((vdriver_height << 16) | (uint16_t)vdriver_width));
            break;
#if defined(CYGWIN32)
        case gestaltGhostScriptVersion:
            set_gs_gestalt_info();
            break;
#endif
        default:
            break;
    }

    retval = gestalt_helper(selector, responsep, false, phystable,
                            NELEM(phystable));
    if(retval == gestaltUndefSelectorErr)
        retval = physicalUndefSelectorErr;
    return retval;
}

OSErrRET Executor::C_GestaltTablesOnly(OSType selector,
                                       GUEST<LONGINT> *responsep)
{
    return gestalt_helper(selector, responsep, false, gtable, NELEM(gtable));
}

static BOOLEAN
syszone_p(ProcPtr p)
{
    THz syszone;

    syszone = MR(LM(SysZone));
    return ((uintptr_t)p >= (uintptr_t)&syszone->heapData
            && (uintptr_t)p < (uintptr_t)MR(syszone->bkLim));
}

static OSErr
new_link(OSType selector, ProcPtr selFunc)
{
    OSErr retval;

    if(!syszone_p(selFunc))
        retval = gestaltLocationErr;
    else
    {
        TheZoneGuard guard(LM(SysZone));
        gestalt_link_t *gp;

        gp = (gestalt_link_t *)NewPtr(sizeof(*gp));
        if(!gp)
            retval = memFullErr;
        else
        {
            retval = noErr;
            gp->selector = selector;
            gp->selectorFunction = selFunc;
            gp->next = gestalt_head;
            gestalt_head = gp;
        }
    }
    return retval;
}

OSErrRET Executor::NewGestalt(OSType selector, ProcPtr selFunc)
{
    OSErr retval;

    if(find_selector_on_list(selector)
       || find_selector_in_table(selector, gtable, NELEM(gtable)))
        retval = gestaltDupSelectorErr;
    else
        retval = new_link(selector, selFunc);
    return retval;
}

OSErrRET Executor::ReplaceGestalt(OSType selector, ProcPtr selFunc,
                                  ProcPtr *oldSelFuncp)
{
    OSErr retval;
    gestalt_link_t *gp;

    gp = find_selector_on_list(selector);
    if(gp)
    {
        if(syszone_p(selFunc))
        {
            *oldSelFuncp = gp->selectorFunction;
            gp->selectorFunction = selFunc;
            retval = noErr;
        }
        else
            retval = gestaltLocationErr;
    }
    else
    {
        gestaltentry_t *gep;

        gep = find_selector_in_table(selector, gtable, NELEM(gtable));
        if(!gep)
            retval = gestaltUndefSelectorErr;
        else
        {
            *oldSelFuncp = (ProcPtr)&GestaltTablesOnly;
            retval = new_link(selector, selFunc);
        }
    }
    return retval;
}
