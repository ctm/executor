#if !defined(_RSYS_DCACHE_H_)
#define _RSYS_DCACHE_H_
namespace Executor
{
typedef uint32_t (*write_callback_funcp_t)(uint32_t fd, const void *buf,
                                         uint32_t offset, uint32_t count);

typedef uint32_t (*read_callback_funcp_t)(uint32_t fd, void *buf,
                                        uint32_t offset, uint32_t count);

extern bool dcache_set_enabled(bool enabled_p);

extern uint32_t dcache_read(uint32_t fd, void *buf, uint32_t offset, uint32_t count,
                          read_callback_funcp_t read_callback = NULL);

extern uint32_t dcache_write(uint32_t fd, const void *buf, uint32_t offset,
                           uint32_t count, write_callback_funcp_t write_callback = NULL);

extern bool dcache_invalidate(uint32_t fd, bool flush_p = false);
extern bool dcache_flush(uint32_t fd);
extern bool dcache_invalidate_all(bool flush_p = false);
}
#endif /* !_RSYS_DCACHE_H_ */
