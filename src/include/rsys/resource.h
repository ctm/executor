/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: resource.h 63 2004-12-24 18:19:43Z ctm $
 */

#if !defined (__MYRESOURCE__)

#include "ResourceMgr.h"
#include "rsys/string.h"

namespace Executor {
#define ULTIMA_III_HACK
#if defined (ULTIMA_III_HACK)
extern bool ROMlib_ultima_iii_hack;
#endif

struct reshead { GUEST_STRUCT;
    GUEST< Size> rdatoff;
    GUEST< Size> rmapoff;
    GUEST< Size> datlen;
    GUEST< Size> maplen;
};

struct rsrvrec { GUEST_STRUCT;
    GUEST< Byte[112]> rsrvsys;
    GUEST< Byte[128]> rsrvapp;
};

struct resmap { GUEST_STRUCT;
    GUEST< reshead> rh;
    GUEST< Handle> nextmap;
    GUEST< INTEGER> resfn;
    GUEST< INTEGER> resfatr;
    GUEST< INTEGER> typoff;
    GUEST< INTEGER> namoff;
};

typedef resmap *resmapptr;

typedef GUEST<resmapptr> *resmaphand;

#define NAMEOFF(map)	Hx(map, namoff)
#define NAMEOFFX(map)	((STARH(map))->namoff)

#define TYPEOFF(map)	Hx(map, typoff)
#define TYPEOFFX(map)	((STARH(map))->typoff)

#define NUMTMINUS1(map) CW(*(GUEST<INTEGER> *)((char *)STARH(map) + Hx(map, typoff)))
#define NUMTMINUS1X(map) (*(GUEST<INTEGER> *)((char *)STARH(map) + Hx(map, typoff)))

#define MAPLEN(map)	Hx(map, rh.maplen)
#define MAPLENX(map)	((STARH(map))->rh.maplen)

struct typref { GUEST_STRUCT;
    GUEST< ResType> rtyp;
    GUEST< INTEGER> nres;
    GUEST< INTEGER> rloff;
};

struct resref { GUEST_STRUCT;
    GUEST< INTEGER> rid;
    GUEST< INTEGER> noff;
    GUEST< Byte> ratr;
    GUEST< Byte[3]> doff;
    GUEST< Handle> rhand;
};

struct empty_resource_template_t { GUEST_STRUCT;
    GUEST< reshead> bhead;
    GUEST< rsrvrec> bfill;
    GUEST< resmap> bmap;
    GUEST< INTEGER> negone;
};


extern resmaphand ROMlib_rntohandl(INTEGER rn, Handle *pph );
extern Handle ROMlib_mgetres( resmaphand map, resref *rr );
extern OSErr ROMlib_findres( Handle r, resmaphand *mapp, typref **trp,
								resref **rrp );
extern OSErr ROMlib_findmapres( resmaphand map, Handle r, typref **trp,
								resref **rrp );
extern OSErr ROMlib_typidtop( ResType typ, INTEGER id, resmaphand *pth,
								resref **ptr );
extern OSErr ROMlib_maptypidtop( resmaphand map, ResType typ, INTEGER id,
								resref **ptr );
extern void ROMlib_invalar( void );

extern Handle ROMlib_getrestid( ResType restype, INTEGER id);
extern INTEGER ROMlib_setreserr(INTEGER reserr);
extern void ROMlib_wr(resmaphand map, resref *rr);
extern LONGINT ROMlib_SizeResource(Handle res, BOOLEAN usehandle);
extern Handle ROMlib_mgetres2(resmaphand map, resref *rr);

#define REF0	0	/* special refrence number signifying system file */

#define WALKMAPCUR(map) \
	for (map = ROMlib_rntohandl(CW(CurMap), (Handle *)0); map ; \
					map = (resmaphand)HxP(map, nextmap)) {

#define WALKMAPTOP(map) \
	for (map = (resmaphand)MR(TopMapHndl); map; \
					map = (resmaphand)HxP(map, nextmap)) {

#define EWALKMAP() \
	}

#define WALKTR(map, i, tr) \
	i = NUMTMINUS1(map); \
	tr = (typref *)((char *) STARH(map) + Hx(map, typoff) + sizeof(INTEGER)); \
	while (i-- >= 0) {

#define EWALKTR(tr) \
		tr++; \
	}

#define WALKRR(map, tr, j, rr) \
		rr = (resref *)((char *)STARH(map) + Hx(map, typoff) + Cx(tr->rloff)); \
		j = Cx(tr->nres); \
		while (j-- >= 0) {

#define EWALKRR(rr) \
			rr++; \
		}

#define WALKTANDR(map, i, tr, j, rr) \
	WALKTR(map, i, tr) \
		WALKRR(map, tr, j, rr)
		
#define EWALKTANDR(tr, rr) \
		EWALKRR(rr) \
	EWALKTR(tr)

#define WALKALLTOP(map, i, tr, j, rr) \
	WALKMAPTOP(map) \
		WALKTANDR(map, i, tr, j, rr)
		
#define WALKALLCUR(map, i, tr, j, rr) \
	WALKMAPCUR(map) \
		WALKTANDR(map, i, tr, j, rr)
		
#define EWALKALL(tr, rr) \
		EWALKTANDR(tr, rr) \
	EWALKMAP()
	
#define B3TOLONG(d) \
		  ((LONGINT)(unsigned char)d[0] << 16) + \
		  ((LONGINT)(unsigned char)d[1] <<  8) + \
					(unsigned char)d[2]
					
#define B3ASSIGN(d, s) \
			d[0] = (s >> 16) & 0xFF; \
			d[1] = (s >>  8) & 0xFF; \
			d[2] =  s        & 0xFF

#if !defined (NDEBUG)
#define warn_resource_not_found(type, id)			\
  warning_trap_failure ("resource '%c%c%c%c':%d not found",	\
			((type) >> 24) & 0xFF,			\
			((type) >> 16) & 0xFF,			\
			((type) >>  8) & 0xFF,			\
			(type) & 0xFF, (id))

#define warn_resource_not_found_name(type, name)		   \
  do								   \
  {								   \
    char *c_string;						   \
								   \
    c_string = TEMP_C_STRING_FROM_STR255 (name);		   \
    warning_trap_failure ("resource '%c%c%c%c':%s not found",	   \
	  		  ((type) >> 24) & 0xFF,		   \
			  ((type) >> 16) & 0xFF,		   \
			  ((type) >>  8) & 0xFF,		   \
			  (type) & 0xFF, c_string);		   \
  } while (FALSE)

#else
#define warn_resource_not_found(type, id)
#define warn_resource_not_found_name(type, id)
#endif


enum
{
  COMPRESSED_TAG = 0xa89f6572,
  COMPRESSED_FLAGS = 0x120801,
};

struct dcomp_info_t { GUEST_STRUCT;
    GUEST< LONGINT> compressedResourceTag;
    GUEST< LONGINT> typeFlags;
    GUEST< LONGINT> uncompressedSize;
    GUEST< uint8> workingBufferFractionalRatio;
    GUEST< uint8> expansionBufferSize;
    GUEST< INTEGER> dcmpID;
};
						   
struct res_sorttype_t {
    LONGINT diskoff;
    resref* rrptr;
};
}
#define __MYRESOURCE__
#endif /* __MYRESOURCE__ */
