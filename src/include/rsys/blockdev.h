#if !defined (_RSYS_BLOCKDEV_H_)
#define _RSYS_BLOCKDEV_H_
namespace Executor {
typedef struct _blockdev_t
{
  uint32 block_size;
  uint32 max_xfer_size;		/* Max number of bytes to transfer at once. */
  int fd;
  uint32 fpos;		/* Current file position. */
  bool locked_p;		/* read-only? */
  bool removable_p;

  /* Don't call these directly!  Go through the appropriate blockdev
   * function.
   */
  int (*read_func) (int fd, void *buf, int nbytes);
  int (*write_func) (int fd, const void *buf, int nbytes);
  off_t (*seek_func) (int fd, off_t where);
  int (*close_func) (int fd);

/* Other things that might be useful?
  volume_size
 */

  /* Internal use only. */
  uint32 dcache_tag;		/* unique number for dcache lookups. */
  bool valid_p;		/* valid blockdev_t record? */
} blockdev_t;


extern bool blockdev_read (blockdev_t *b, uint32 offset, void *buf,
				uint32 num_bytes);
extern bool blockdev_write (blockdev_t *b, uint32 offset, const void *buf,
				 uint32 num_bytes);
extern bool blockdev_seek_set (blockdev_t *b, uint32 offset);
extern void blockdev_close (blockdev_t *b);
}
#endif /* !_RSYS_BLOCKDEV_H_ */
