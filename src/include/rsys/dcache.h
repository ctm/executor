#if !defined (_RSYS_DCACHE_H_)
#define _RSYS_DCACHE_H_
namespace Executor {
typedef uint32 (*write_callback_funcp_t) (uint32 fd, const void *buf,
					  uint32 offset, uint32 count);

typedef uint32 (*read_callback_funcp_t) (uint32 fd, void *buf,
					  uint32 offset, uint32 count);

extern bool dcache_set_enabled (bool enabled_p);

extern uint32 dcache_read (uint32 fd, void *buf, uint32 offset, uint32 count,
			   read_callback_funcp_t read_callback = NULL);

extern uint32 dcache_write (uint32 fd, const void *buf, uint32 offset,
			  uint32 count, write_callback_funcp_t write_callback = NULL);

extern bool dcache_invalidate (uint32 fd, bool flush_p = false);
extern bool dcache_flush (uint32 fd);
extern bool dcache_invalidate_all (bool flush_p = false);
}
#endif  /* !_RSYS_DCACHE_H_ */
