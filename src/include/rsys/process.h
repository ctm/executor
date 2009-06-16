#if !defined (__rsys_process_h__)
#  define __rsys_process_h__

typedef struct size_resource
{
  int16 flags		PACKED;
  int32 pref_size	PACKED;
  int32 min_size	PACKED;
} size_resource_t;

#endif /* !defined (__rsys_process_h__) */
