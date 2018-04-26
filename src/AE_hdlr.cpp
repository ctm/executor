/* Copyright 1986, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "AppleEvents.h"
#include "MemoryMgr.h"
#include "SegmentLdr.h"

#include "rsys/system_error.h"
#include "rsys/mman.h"

using namespace Executor;

inline AE_zone_tables_h get_zone_tables(bool system_p)
{
    auto info = MR(LM(AE_info));
    return system_p ? MR(info->system_zone_tables) : MR(info->appl_zone_tables);
}
#define hdlr_table(system_p, class) HxP(get_zone_tables(system_p), class##_hdlr_table)

void Executor::AE_init(void)
{
    AE_info_t *info;
    OSErr err;

    info = (AE_info_t *)NewPtrSysClear(sizeof *info);

    TheZoneGuard guard(LM(SysZone));

    AE_zone_tables_h zone_tables;

    zone_tables
        = (AE_zone_tables_h)NewHandleClear(sizeof(AE_zone_tables_t));

    err = _AE_hdlr_table_alloc(16, 0x80008, 0,
                               /* #### */
                               false,
                               &HxX(zone_tables, event_hdlr_table));
    err = _AE_hdlr_table_alloc(16, 0x80008, 0,
                               /* #### */
                               false,
                               &HxX(zone_tables, coercion_hdlr_table));
    err = _AE_hdlr_table_alloc(16, 0x80008, 0,
                               /* #### */
                               false,
                               &HxX(zone_tables, special_hdlr_table));
    info->system_zone_tables = RM(zone_tables);

    LM(AE_info) = RM(info);
}

void Executor::AE_reinit(void)
{
    AE_info_t *info;
    OSErr err;

    info = MR(LM(AE_info));

    TheZoneGuard guard(LM(ApplZone));

    AE_zone_tables_h zone_tables;

    zone_tables
        = (AE_zone_tables_h)NewHandleClear(sizeof(AE_zone_tables_t));

    err = _AE_hdlr_table_alloc(16, 0x80008, 0,
                               /* #### */
                               false,
                               &HxX(zone_tables, event_hdlr_table));
    err = _AE_hdlr_table_alloc(16, 0x80008, 0,
                               /* #### */
                               false,
                               &HxX(zone_tables, coercion_hdlr_table));
    err = _AE_hdlr_table_alloc(16, 0x80008, 0,
                               /* #### */
                               false,
                               &HxX(zone_tables, special_hdlr_table));
    info->appl_zone_tables = RM(zone_tables);
}

static OSErr
hdlr_table_elt(AE_hdlr_table_h table,
               AE_hdlr_selector_t *selector, AE_hdlr_t *hdlr,
               bool create_p,
               AE_hdlr_table_elt_t **retval)
{
    AE_hdlr_table_elt_t *elts, *elt;
    int32_t n_elts, elt_index;

    elts = AE_TABLE_ELTS(table);
    n_elts = AE_TABLE_N_ELTS(table);

    for(elt_index = 0; elt_index < n_elts; elt_index++)
    {
        elt = &elts[elt_index];

        if((hdlr == NULL
            || hdlr->fn == elt->hdlr.fn)
           && elt->selector.sel0 == selector->sel0
           && elt->selector.sel1 == selector->sel1)
        {
            *retval = elt;
            AE_RETURN_ERROR(noErr);
        }
    }

    if(create_p)
    {
        n_elts++;

        AE_TABLE_N_ELTS_X(table) = CL(n_elts);

        SetHandleSize((Handle)table,
                      (sizeof(AE_hdlr_table_t)
                       + n_elts * sizeof(AE_hdlr_table_elt_t)));

        if(LM(MemErr) != CWC(noErr))
            AE_RETURN_ERROR(CW(LM(MemErr)));

        elts = AE_TABLE_ELTS(table);
        elt = &elts[n_elts - 1];

        elt->pad_1 = CLC(-1);
        elt->selector = *selector;
        elt->pad_2 = CLC(-1);

        *retval = elt;
        AE_RETURN_ERROR(noErr);
    }

    AE_RETURN_ERROR(errAEHandlerNotFound);
}

OSErr Executor::C__AE_hdlr_table_alloc(int32_t unknown_1, int32_t unknown_2,
                                       int32_t unknown_3, int8_t unknown_p,
                                       GUEST<AE_hdlr_table_h> *table_return)
{
    AE_hdlr_table_h table;

    gui_assert(unknown_1 == 16);
    gui_assert(unknown_2 == 0x80008);
    gui_assert(unknown_3 == 0);

    /* #### unknown_p is currently ignored */

    /* #### actual initial allocation size is probably a function of the
     first argument */
    table = (AE_hdlr_table_h)NewHandleClear(52);
    if(LM(MemErr) != CWC(noErr))
        AE_RETURN_ERROR(CW(LM(MemErr)));

    AE_TABLE_N_ALLOCATED_BYTES_X(table) = CLC(52);
    AE_TABLE_N_ELTS_X(table) = CLC(0);

    *table_return = RM(table);
    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C__AE_hdlr_delete(AE_hdlr_table_h table, int32_t unknown_1,
                                  AE_hdlr_selector_t *selector)
{
    AE_hdlr_table_elt_t *elt, *elts;
    int n_elts, elt_offset;
    OSErr err;

    gui_assert(unknown_1 == 0);

    err = hdlr_table_elt(table, selector, NULL, false, &elt);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    elts = AE_TABLE_ELTS(table);
    n_elts = AE_TABLE_N_ELTS(table);
    elt_offset = elt - elts;

    memmove(&elts[elt_offset + 1], &elts[elt_offset],
            (n_elts - elt_offset - 1) * sizeof *elts);

    AE_TABLE_N_ELTS_X(table) = CL(n_elts - 1);

    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C__AE_hdlr_lookup(AE_hdlr_table_h table, int32_t unknown_1,
                                  AE_hdlr_selector_t *selector,
                                  AE_hdlr_t *hdlr_return)
{
    AE_hdlr_table_elt_t *elt;
    OSErr err;

    gui_assert(unknown_1 == 0);

    err = hdlr_table_elt(table, selector, NULL, false, &elt);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    *hdlr_return = elt->hdlr;
    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C__AE_hdlr_install(AE_hdlr_table_h table, int32_t unknown_1,
                                   AE_hdlr_selector_t *selector,
                                   AE_hdlr_t *hdlr)
{
    AE_hdlr_table_elt_t *elt;
    OSErr err;

    gui_assert(unknown_1 == 0);

    err = hdlr_table_elt(table, selector, hdlr, true, &elt);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    elt->hdlr = *hdlr;
    AE_RETURN_ERROR(noErr);
}

bool Executor::application_accepts_open_app_aevt_p;

OSErr Executor::C_AEInstallEventHandler(AEEventClass event_class,
                                        AEEventID event_id,
                                        EventHandlerProcPtr hdlr_fn,
                                        int32_t refcon,
                                        Boolean system_handler_p)
{
    AE_hdlr_table_h table;
    AE_hdlr_table_elt_t *elt;
    AE_hdlr_selector_t selector;
    AE_hdlr_t hdlr;
    OSErr err;

    if(!hdlr_fn)
        AE_RETURN_ERROR(paramErr);

    table = hdlr_table(system_handler_p, event);

    selector.sel0 = CL(event_class);
    selector.sel1 = CL(event_id);

    hdlr.fn = RM((void *)hdlr_fn);
    hdlr.refcon = CL(refcon);

    err = hdlr_table_elt(table, &selector, &hdlr, true, &elt);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    /* warning: `elt' is a pointer into an unlocked handle */
    elt->hdlr = hdlr;

    /* hack because various applications complain if they get sent
     an open application event and don't have a handler for it */
    application_accepts_open_app_aevt_p = true;

    AE_RETURN_ERROR(noErr);
}

/* event handler functions */

static void
dummy(void)
{
}

OSErr Executor::C_AEGetEventHandler(AEEventClass event_class,
                                    AEEventID event_id,
                                    GUEST<EventHandlerProcPtr> *hdlr,
                                    GUEST<int32_t> *refcon,
                                    Boolean system_handler_p)
{
    AE_hdlr_table_h table;
    AE_hdlr_table_elt_t *elt;
    AE_hdlr_selector_t selector;
    OSErr err;

#if 1
    if(event_class == TICK("go b") && event_id == TICK("ears"))
        system_error("This application appears to be using AppleEvents in "
                     "a way that is not currently supported under Executor.  "
                     "You may try to continue, but in all likelihood Executor "
                     "will soon crash because of this incompatibility.",
                     0, "Exit", "Continue",
                     NULL, C_ExitToShell, dummy, NULL);
#endif

    table = hdlr_table(system_handler_p, event);

    selector.sel0 = CL(event_class);
    selector.sel1 = CL(event_id);

    err = hdlr_table_elt(table, &selector, NULL, false, &elt);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    /* warning: `elt' is a pointer into an unlocked handle */
    *hdlr = guest_cast<EventHandlerProcPtr>(elt->hdlr.fn);
    *refcon = elt->hdlr.refcon;

    /* hack because various applications complain if they get sent
     an open application event and don't have a handler for it */
    application_accepts_open_app_aevt_p = true;

    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AERemoveEventHandler(AEEventClass event_class,
                                       AEEventID event_id,
                                       EventHandlerProcPtr hdlr,
                                       Boolean system_handler_p)
{
    AE_hdlr_table_h table;
    AE_hdlr_selector_t selector;

    table = hdlr_table(system_handler_p, event);

    selector.sel0 = CL(event_class);
    selector.sel1 = CL(event_id);

    /* #### fail if `hdlr' is not the currently installed handler? */
    AE_RETURN_ERROR(_AE_hdlr_delete(table, 0, &selector));
}

/* coercion handler functions */

OSErr Executor::C_AEInstallCoercionHandler(
    DescType from_type, DescType to_type, CoerceDescProcPtr hdlr_fn,
    int32_t refcon, Boolean from_type_is_desc_p, Boolean system_handler_p)
{
    AE_hdlr_table_h table;
    AE_hdlr_table_elt_t *elt;
    AE_hdlr_selector_t selector;
    AE_hdlr_t hdlr;
    OSErr err;

    if(hdlr_fn == NULL)
        AE_RETURN_ERROR(paramErr);

    table = hdlr_table(system_handler_p, coercion);

    selector.sel0 = CL(from_type);
    selector.sel1 = CL(to_type);

    hdlr.fn = RM((void *)hdlr_fn);
    hdlr.refcon = CL(refcon);

    err = hdlr_table_elt(table, &selector, &hdlr, true, &elt);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    /* warning: `elt' is a pointer into an unlocked handle */
    elt->hdlr = hdlr;

    /* #### elt->flag = from_type_is_desc_p */

    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AEGetCoercionHandler(DescType from_type, DescType to_type,
                                       GUEST<CoerceDescProcPtr> *hdlr_out,
                                       GUEST<int32_t> *refcon_out,
                                       GUEST<Boolean> *from_type_is_desc_p_out,
                                       Boolean system_handler_p)
{
    AE_hdlr_table_h table;
    AE_hdlr_table_elt_t *elt;
    AE_hdlr_selector_t selector;
    OSErr err;

    table = hdlr_table(system_handler_p, coercion);

    selector.sel0 = CL(from_type);
    selector.sel1 = CL(to_type);

    err = hdlr_table_elt(table, &selector, NULL, false, &elt);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    /* warning: `elt' is a pointer into an unlocked handle */
    *hdlr_out = guest_cast<CoerceDescProcPtr>(elt->hdlr.fn);
    *refcon_out = elt->hdlr.refcon;

    /* #### *from_type_is_desc_p_out = elt->flag; */

    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AERemoveCoercionHandler(
    DescType from_type, DescType to_type, CoerceDescProcPtr hdlr,
    Boolean system_handler_p)
{
    AE_hdlr_table_h table;
    AE_hdlr_selector_t selector;

    table = hdlr_table(system_handler_p, coercion);

    selector.sel0 = CL(from_type);
    selector.sel1 = CL(to_type);

    /* #### fail if `hdlr' is not the currently installed handler? */
    AE_RETURN_ERROR(_AE_hdlr_delete(table, 0, &selector));
}

/* special handler functions */

#define k_special_sel1 (FOURCC('\000', '\000', '\000', '\000'))

syn68k_addr_t Executor::AE_OSL_select_fn;

OSErr Executor::C_AEInstallSpecialHandler(
    AEKeyword function_class, EventHandlerProcPtr hdlr_fn,
    Boolean system_handler_p)
{
    AE_hdlr_table_h table;
    AE_hdlr_table_elt_t *elt;
    AE_hdlr_selector_t selector;
    AE_hdlr_t hdlr;
    OSErr err;

    if(hdlr_fn == NULL)
        AE_RETURN_ERROR(paramErr);

    /* #### OSL internal */
    if(function_class == keySelectProc)
    {
        AE_OSL_select_fn = US_TO_SYN68K((void *)hdlr_fn);
        warning_unimplemented("AEInstallSpecialHandler: keySelectProc set");
        AE_RETURN_ERROR(noErr);
    }

    table = hdlr_table(system_handler_p, special);

    selector.sel0 = CL(function_class);
    selector.sel1 = CL(k_special_sel1);

    hdlr.fn = RM((void *)hdlr_fn);
    hdlr.refcon = CL(-1);

    err = hdlr_table_elt(table, &selector, &hdlr, true, &elt);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    /* warning: `elt' is a pointer into an unlocked handle */
    elt->hdlr = hdlr;

    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AEGetSpecialHandler(AEKeyword function_class,
                                      GUEST<EventHandlerProcPtr> *hdlr_out,
                                      Boolean system_handler_p)
{
    AE_hdlr_table_h table;
    AE_hdlr_table_elt_t *elt;
    AE_hdlr_selector_t selector;
    OSErr err;

    table = hdlr_table(system_handler_p, special);

    selector.sel0 = CL(function_class);
    selector.sel1 = CL(k_special_sel1);

    err = hdlr_table_elt(table, &selector, NULL, false, &elt);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    /* warning: `elt' is a pointer into an unlocked handle */
    *hdlr_out = guest_cast<EventHandlerProcPtr>(elt->hdlr.fn);

    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AERemoveSpecialHandler(
    AEKeyword function_class, EventHandlerProcPtr hdlr,
    Boolean system_handler_p)
{
    AE_hdlr_table_h table;
    AE_hdlr_selector_t selector;

    table = hdlr_table(system_handler_p, special);

    selector.sel0 = CL(function_class);
    selector.sel1 = CL(k_special_sel1);

    /* #### fail if `hdlr' is not the currently installed handler? */
    AE_RETURN_ERROR(_AE_hdlr_delete(table, 0, &selector));
}
