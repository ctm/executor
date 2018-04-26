#if !defined(_HOOK_H_)
#define _HOOK_H_
namespace Executor
{
typedef enum {
    ctl_actionnumber,
    ctl_cdefnumber,
    desk_deskhooknumber,
    device_number,
    dial_modalnumber,
    dial_soundprocnumber,
    dial_usernumber,
    file_badreturn,
    file_completionnumber,
    iu_localnumber,
    iu_unimplementednumber,
    list_clicknumber,
    list_cmpnumber,
    list_ldefnumber,
    memory_badreturn,
    memory_gznumber,
    memory_purgeprocnumber,
    menu_mbarhooknumber,
    menu_mbdfnumber,
    menu_mdefnumber,
    menu_menuhooknumber,
    notify_procnumber,
    pr_initnumber,
    pr_itemnumber,
    q_arcprocnumber,
    q_bitsprocnumber,
    q_commentprocnumber,
    q_getpicprocnumber,
    q_lineprocnumber,
    q_ovalprocnumber,
    q_polyprocnumber,
    q_putpicprocnumber,
    q_rectprocnumber,
    q_rgnprocnumber,
    q_rrectprocnumber,
    q_textprocnumber,
    q_txmeasprocnumber,
    res_reserrprocnumber,
    resource_badreturn,
    script_notsupported,
    soundmgr_number,
    stdfile_dialhooknumber,
    stdfile_filefiltnumber,
    stdfile_filtnumber,
    te_clikloopnumber,
    te_dotextnumber,
    te_notsupported,
    time_number,
    vbl_number,
    wind_deskhooknumber,
    wind_draghooknumber,
    wind_dragtheregionnumber,
    wind_wdefnumber,
} hooknumber;

#if !defined(NDEBUG)
extern void ROMlib_hook(LONGINT hn);
#else
#define ROMlib_hook(hn)
#endif

}
#endif /* !_HOOK_H_ */
