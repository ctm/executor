#if !defined(__RSYS_INI_H__)
#define __RSYS_INI_H__

/*
 * Copyright 1997 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include <string>

namespace Executor
{
typedef std::string heading_t;
typedef std::string ini_key_t;
typedef std::string value_t;

typedef struct pair_link_str
{
    struct pair_link_str *next;
    ini_key_t key;
    value_t value;
} pair_link_t;

extern std::string ROMlib_PrintersIni;
extern std::string ROMlib_PrintDef;

extern heading_t new_heading(unsigned char *start, int len);
extern void new_key_value_pair(heading_t heading,
                               unsigned char *keystart, int keylen,
                               unsigned char *valuestart, int valuelen);

#if 0
/* calling discard_all_inis is unsafe */
extern void discard_all_inis (void);
#endif

extern bool read_ini_file(const char *filename);
extern pair_link_t *get_pair_link_n(heading_t heading, int n);
extern FILE *open_ini_file_for_writing(const char *filename);
extern bool add_heading_to_file(FILE *fp, heading_t heading);
extern bool add_key_value_to_file(FILE *fp, ini_key_t key, value_t value);
extern bool close_ini_file(FILE *fp);
extern value_t find_key(heading_t heading, ini_key_t key);
}
#endif
