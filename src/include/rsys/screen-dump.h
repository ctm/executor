#if !defined(__rsys_screen_dump_h__)
#define __rsys_screen_dump_h__

namespace Executor
{
struct header
{
    int16_t byte_order;
    int16_t magic_number;
    int32_t ifd_offset;
};

struct directory_entry
{
    int16_t tag;
    int16_t type;
    int32_t count;
    int32_t value_offset;
};

struct ifd
{
    int16_t count;
    struct directory_entry entries[1];
};
}

#endif /* !defined (__rsys_screen_dump_h__) */
