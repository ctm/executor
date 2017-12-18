#if !defined(_RSYS_BLOCKDEV_H_)
#define _RSYS_BLOCKDEV_H_
namespace Executor
{
typedef struct _blockdev_t
{
    uint32_t block_size;
    uint32_t max_xfer_size; /* Max number of bytes to transfer at once. */
    int fd;
    uint32_t fpos; /* Current file position. */
    bool locked_p; /* read-only? */
    bool removable_p;

    /* Don't call these directly!  Go through the appropriate blockdev
   * function.
   */
    int (*read_func)(int fd, void *buf, int nbytes);
    int (*write_func)(int fd, const void *buf, int nbytes);
    off_t (*seek_func)(int fd, off_t where);
    int (*close_func)(int fd);

    /* Other things that might be useful?
  volume_size
 */

    /* Internal use only. */
    uint32_t dcache_tag; /* unique number for dcache lookups. */
    bool valid_p; /* valid blockdev_t record? */
} blockdev_t;

extern bool blockdev_read(blockdev_t *b, uint32_t offset, void *buf,
                          uint32_t num_bytes);
extern bool blockdev_write(blockdev_t *b, uint32_t offset, const void *buf,
                           uint32_t num_bytes);
extern bool blockdev_seek_set(blockdev_t *b, uint32_t offset);
extern void blockdev_close(blockdev_t *b);
}
#endif /* !_RSYS_BLOCKDEV_H_ */
