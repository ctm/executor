#if !defined(_SUFFIX_MAPS_H_)
#define _SUFFIX_MAPS_H_

/*
 * Copyright 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

extern void ROMlib_add_suffix_quad(const char *suffixp,
                                   const char *creator_hexp,
                                   const char *type_hexp,
                                   const char *applicationp);

extern bool ROMlib_creator_and_type_from_suffix(const char *suffix,
                                                uint32_t *creatorp,
                                                uint32_t *typep);

extern bool ROMlib_creator_and_type_from_filename(int len,
                                                  const char *filename,
                                                  uint32_t *creatorp,
                                                  uint32_t *typep);

extern bool ROMlib_delete_suffix(const char *suffix);

extern const char *ROMlib_find_best_creator_type_match(uint32_t creator,
                                                       uint32_t type);

#endif
