#if !defined(_SUFFIX_MAPS_H_)
#define _SUFFIX_MAPS_H_

/*
 * Copyright 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: suffix_maps.h 63 2004-12-24 18:19:43Z ctm $
 */


extern void ROMlib_add_suffix_quad (const char *suffixp,
				    const char *creator_hexp,
				    const char *type_hexp,
				    const char *applicationp);

extern boolean_t ROMlib_creator_and_type_from_suffix (const char *suffix,
						      uint32 *creatorp,
						      uint32 *typep);

extern boolean_t ROMlib_creator_and_type_from_filename (int len,
							const char *filename,
							uint32 *creatorp,
							uint32 *typep);

extern boolean_t ROMlib_delete_suffix (const char *suffix);

extern const char *ROMlib_find_best_creator_type_match (uint32 creator,
							uint32 type);


#endif
