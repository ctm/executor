#if !defined (_RSYS_DCACHE_H_)
#define _RSYS_DCACHE_H_
namespace Executor {
typedef uint32 (*write_callback_funcp_t) (uint32 fd, const void *buf,
					  uint32 offset, uint32 count);

typedef uint32 (*read_callback_funcp_t) (uint32 fd, void *buf,
					  uint32 offset, uint32 count);

extern boolean_t dcache_set_enabled (boolean_t enabled_p);

extern uint32 dcache_read (uint32 fd, void *buf, uint32 offset, uint32 count,
			   read_callback_funcp_t read_callback = NULL);

extern uint32 dcache_write (uint32 fd, const void *buf, uint32 offset,
			  uint32 count, write_callback_funcp_t write_callback = NULL);

extern boolean_t dcache_invalidate (uint32 fd, boolean_t flush_p = FALSE);
extern boolean_t dcache_flush (uint32 fd);
extern boolean_t dcache_invalidate_all (boolean_t flush_p = FALSE);
}
#endif  /* !_RSYS_DCACHE_H_ */
