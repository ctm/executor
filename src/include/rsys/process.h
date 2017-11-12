#if !defined(__rsys_process_h__)
#define __rsys_process_h__
namespace Executor
{
typedef struct size_resource
{
    GUEST_STRUCT;
    GUEST<int16> flags;
    GUEST<int32> pref_size;
    GUEST<int32> min_size;
} size_resource_t;
}
#endif /* !defined (__rsys_process_h__) */
