#if !defined (__rsys_screen_dump_h__)
#  define __rsys_screen_dump_h__

struct header
{
  int16 byte_order	__attribute__ ((packed));
  int16 magic_number	__attribute__ ((packed));
  int32 ifd_offset	__attribute__ ((packed));
};

struct directory_entry
{
  int16 tag		__attribute__ ((packed));
  int16 type		__attribute__ ((packed));
  int32 count		__attribute__ ((packed));
  int32 value_offset	__attribute__ ((packed));
};

struct ifd
{
  int16 count				__attribute__ ((packed));
  /* gcc 4.3.0 warns if we leave the following packed attribute in */
  struct directory_entry entries[1]	/* __attribute__ ((packed)) */;
};

#endif /* !defined (__rsys_screen_dump_h__) */
