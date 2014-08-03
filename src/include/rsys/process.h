#if !defined (__rsys_process_h__)
#  define __rsys_process_h__
namespace Executor {
typedef struct PACKED size_resource
{
  int16 flags;
  int32 pref_size;
  int32 min_size;
} size_resource_t;
}
#endif /* !defined (__rsys_process_h__) */
