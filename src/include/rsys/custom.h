#if !defined(_custom_h_)
#define _custom_h_

/*
 * Copyright 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

/*
 * misc. quantities needed by both Executor and the customization tool.
 */

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    CUSTOM_BLOCK_SIZE = 393216,
}; /* enough room for splash screen
					 and license */

#define CUSTOM_MAGIC 0xf6e0c5c9cb52e1bfLL

/* the following strings need to be unique in their first four characters */

#define CUSTOM_CHECKSUM "checksum"
#define CUSTOM_CREATORS "creators"
#define CUSTOM_LICENSE "license"
#define CUSTOM_FIRST_SN "sn_first"
#define CUSTOM_LAST_SN "sn_last"
#define CUSTOM_SPLASH "splash"
#define CUSTOM_ABOUT_BOX "about_box"
#define CUSTOM_COPYRIGHT_INFO "copyright_info"
#define CUSTOM_THANK_YOU_INFO "thank_you_info"
#define CUSTOM_REGISTRATION_INSTRUCTIONS "registration_instructions"
#define CUSTOM_MAGIC_VOLUMES "magic_volumes"
#define CUSTOM_MUST_REGISTER "must_register"
#define CUSTOM_MAC_CDROM "mac_cdrom"
#define CUSTOM_DEMO_IDENTIFIER "demo_id"
#define CUSTOM_DISABLE_COMMAND_KEYS "command_key_disable"
#define CUSTOM_RESTART_STRING "restart_string"
#define CUSTOM_MENU_ABOUT_STRING "menu_about_string"
#define CUSTOM_VERSION_STRING "version_string"
#define CUSTOM_SUFFIX_MAPS "suffix_maps"
#define CUSTOM_DEFAULT_APP "default_app"
#define CUSTOM_DEMO_DAYS "days_of_demo"

typedef struct
{
    uint32_t magic;
    uint32_t length;
} header_t;

typedef struct
{
    uint64_t magic;
    header_t headers[0];
    uint8 filler[CUSTOM_BLOCK_SIZE - sizeof(uint64_t)];
} custom_block_t;

typedef struct
{
    header_t head;
    uint32_t val;
} custom_val_t;

typedef struct
{
    header_t head;
    uint32_t vals[0];
} custom_vals_t;

typedef struct
{
    header_t head;
    uint8 chars[0];
} custom_chars_t;

extern custom_val_t *ROMlib_checksump;
extern custom_vals_t *ROMlib_creatorsp;
extern custom_chars_t *ROMlib_licensep;
extern custom_val_t *ROMlib_first_snp;
extern custom_val_t *ROMlib_last_snp;
extern custom_chars_t *ROMlib_splashp;
extern custom_val_t *ROMlib_about_boxp;
extern custom_val_t *ROMlib_must_registerp;
extern custom_chars_t *ROMlib_copyright_infop;
extern custom_chars_t *ROMlib_thank_you_infop;
extern custom_chars_t *ROMlib_registration_instructionsp;
extern custom_chars_t *ROMlib_magic_volumesp;
extern custom_chars_t *ROMlib_mac_cdromp;
extern custom_chars_t *ROMlib_mac_demo_idp;
extern custom_val_t *ROMlib_disable_command_key_equivsp;
extern custom_chars_t *ROMlib_restart_stringp;
extern custom_chars_t *ROMlib_menu_about_stringp;
extern custom_chars_t *ROMlib_suffix_mapsp;
extern custom_chars_t *ROMlib_default_appp;
extern custom_val_t *ROMlib_days_of_demop;

extern void ROMlib_do_custom(void);
#ifdef __cplusplus
}
#endif

#endif
