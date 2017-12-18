/* Copyright 1988 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "rsys/custom.h"
#include "rsys/version.h"
#include "rsys/aboutbox.h"

using namespace Executor;

PRIVATE custom_block_t custom = {
    CUSTOM_MAGIC,
};

PRIVATE void *
find_custom_string(const char *str)
{
    header_t *headerp;
    void *retval;

    for(headerp = custom.headers;
        headerp->length && headerp->magic != *(uint32_t *)str;
        headerp = (header_t *)((char *)headerp + headerp->length + sizeof *headerp))
        ;
    ;
    if(headerp->length)
        retval = headerp;
    else
        retval = NULL;

    return retval;
}

PUBLIC custom_val_t *ROMlib_checksump;
PUBLIC custom_vals_t *ROMlib_creatorsp;
PUBLIC custom_chars_t *ROMlib_licensep;
PUBLIC custom_val_t *ROMlib_first_snp;
PUBLIC custom_val_t *ROMlib_last_snp;
PUBLIC custom_chars_t *ROMlib_splashp;
PUBLIC custom_val_t *ROMlib_about_boxp;
PUBLIC custom_chars_t *ROMlib_copyright_infop;
PUBLIC custom_chars_t *ROMlib_thank_you_infop;
PUBLIC custom_chars_t *ROMlib_registration_instructionsp;
PUBLIC custom_chars_t *ROMlib_magic_volumesp;
PUBLIC custom_val_t *ROMlib_must_registerp;
PUBLIC custom_chars_t *ROMlib_mac_cdromp;
PUBLIC custom_chars_t *ROMlib_mac_demo_idp;
PUBLIC custom_val_t *ROMlib_disable_command_key_equivsp;
PUBLIC custom_chars_t *ROMlib_restart_stringp;
PUBLIC custom_chars_t *ROMlib_menu_about_stringp;
PRIVATE custom_chars_t *version_stringp;
PUBLIC custom_chars_t *ROMlib_suffix_mapsp;
PUBLIC custom_chars_t *ROMlib_default_appp;
PUBLIC custom_val_t *ROMlib_days_of_demop;

/*
 * The about box string is a pascal string with a leading 0 to satisfy some
 * weird compatibility constraint.
 */

PRIVATE StringPtr
about_box_string_from_c_string(const char *str)
{
    StringPtr retval;
    int len;

    len = strlen(str) + 2;
    retval = (StringPtr)malloc(len);
    if(retval)
    {
        retval[0] = len - 1;
        retval[1] = 0;
        memcpy(retval + 2, str, len - 2);
    }
    return retval;
}

PRIVATE char *
executor_full_name_from_prefix(const char *str)
{
    char *retval;
    int len;

    len = strlen(str) + 1 + strlen(EXECUTOR_VERSION) + 1;
    retval = (char *)malloc(len);
    if(retval)
        sprintf(retval, "%s %s", str, EXECUTOR_VERSION);

    return retval;
}

PUBLIC void
ROMlib_do_custom(void)
{
    ROMlib_checksump = (custom_val_t *)find_custom_string(CUSTOM_CHECKSUM);
    ROMlib_creatorsp = (custom_vals_t *)find_custom_string(CUSTOM_CREATORS);
    ROMlib_licensep = (custom_chars_t *)find_custom_string(CUSTOM_LICENSE);
    ROMlib_first_snp = (custom_val_t *)find_custom_string(CUSTOM_FIRST_SN);
    ROMlib_last_snp = (custom_val_t *)find_custom_string(CUSTOM_LAST_SN);
    ROMlib_splashp = (custom_chars_t *)find_custom_string(CUSTOM_SPLASH);
    ROMlib_about_boxp = (custom_val_t *)find_custom_string(CUSTOM_ABOUT_BOX);
    ROMlib_copyright_infop = (custom_chars_t *)find_custom_string(CUSTOM_COPYRIGHT_INFO);
    ROMlib_thank_you_infop = (custom_chars_t *)find_custom_string(CUSTOM_THANK_YOU_INFO);
    ROMlib_registration_instructionsp = (custom_chars_t *)find_custom_string(CUSTOM_REGISTRATION_INSTRUCTIONS);
    ROMlib_magic_volumesp = (custom_chars_t *)find_custom_string(CUSTOM_MAGIC_VOLUMES);
    ROMlib_suffix_mapsp = (custom_chars_t *)find_custom_string(CUSTOM_SUFFIX_MAPS);
    ROMlib_default_appp = (custom_chars_t *)find_custom_string(CUSTOM_DEFAULT_APP);
    ROMlib_must_registerp = (custom_val_t *)find_custom_string(CUSTOM_MUST_REGISTER);
    ROMlib_mac_cdromp = (custom_chars_t *)find_custom_string(CUSTOM_MAC_CDROM);
    ROMlib_mac_demo_idp = (custom_chars_t *)find_custom_string(CUSTOM_DEMO_IDENTIFIER);
    ROMlib_disable_command_key_equivsp
        = (custom_val_t *)find_custom_string(CUSTOM_DISABLE_COMMAND_KEYS);
    ROMlib_restart_stringp = (custom_chars_t *)find_custom_string(CUSTOM_RESTART_STRING);

    ROMlib_menu_about_stringp = (custom_chars_t *)find_custom_string(CUSTOM_MENU_ABOUT_STRING);
    if(ROMlib_menu_about_stringp)
        about_box_menu_name_pstr
            = about_box_string_from_c_string((char *)
                                                 ROMlib_menu_about_stringp->chars);

    version_stringp = (custom_chars_t *)find_custom_string(CUSTOM_VERSION_STRING);
    if(version_stringp)
        ROMlib_executor_full_name
            = executor_full_name_from_prefix((char *)version_stringp->chars);
}
