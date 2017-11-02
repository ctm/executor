/* Copyright 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_blockdev[] =
	    "$Id: blockdev.c 63 2004-12-24 18:19:43Z ctm $";
#endif


#include "rsys/common.h"
#include "rsys/blockdev.h"
#include "rsys/dcache.h"
#include "rsys/file.h"

using namespace Executor;

/* Opens and returns a new blockdev_t structure.  Close it with
 * blockdev_close when you are done with it.
 */
blockdev_t *
blockdev_open (uint32 block_size,
	       uint32 max_xfer_size,
	       int fd,
	       int (*read_func) (int fd, void *buf, int nbytes),
	       int (*write_func) (int fd, const void *buf, int nbytes),
	       off_t (*seek_func) (int fd, off_t where),
	       int (*close_func) (int fd),
	       bool locked_p,
	       bool removable_p)
{
  blockdev_t *b;

  b = (blockdev_t*)malloc (sizeof *b);
  if (b)
    {
      static uint32 unique_dcache_tag;

      memset (b, 0, sizeof *b);
      
      b->block_size = block_size;
      b->max_xfer_size = max_xfer_size;
      b->fd            = fd;
      b->read_func     = read_func;
      b->write_func    = write_func;
      b->seek_func     = seek_func;
      b->close_func    = close_func;
      b->locked_p      = locked_p;
      b->removable_p   = removable_p;
      b->dcache_tag    = ++unique_dcache_tag;
      b->valid_p       = TRUE;
    }

  return b;
}



#warning "I'm not sure if this function should be part of the API, since the read and write routines take offsets anyway.  It's only here because I'm not sure if you need to do a seek when switching from reads to writes and vice versa, like you do with fread/fwrite.  I don't think you do, but..."

/* Reads the specified number of bytes from the given offset for
 * the given device.  Returns TRUE on success, else FALSE.
 */
bool
Executor::blockdev_seek_set (blockdev_t *b, uint32 offset)
{
  bool success_p;
  success_p = (b->seek_func (offset, L_SET) == 0);
  if (success_p)
    b->fpos = offset;
  else
    b->fpos = -1;
  return success_p;
}


/* Reads the specified number of bytes from the given offset for
 * the given device.  Returns TRUE on success, else FALSE.
 */
bool
Executor::blockdev_read (blockdev_t *b, uint32 offset, void *buf, uint32 num_bytes)
{
  bool retval;

  retval = FALSE;  /* default */
  if (b && b->valid_p)
    {
      uint32 n;

      gui_assert ((offset % b->block_size) == 0);
      gui_assert ((num_bytes % b->block_size) == 0);

      for (n = 0; n < num_bytes; )
	{
	  uint8 *p;

	  p = (uint8 *) buf + n;

	  /* First try to read from the cache. */
	  if (dcache_read (b->dcache_tag, p, offset + n, b->block_size))
	    n += b->block_size;
	  else
	    {
	      uint32 bytes_to_read;

	      /* Cache miss.  Seek if necessary. */
	      if (offset + n != b->fpos)
		{
		  if (!blockdev_seek_set (b, offset + n))
		    goto done;
		}

	      /* Actually read the bytes in. */
	      bytes_to_read = MIN (b->max_xfer_size, num_bytes - n);
	      if (b->read_func (b->fd, p, bytes_to_read) != bytes_to_read)
		goto done;
	      b->fpos += bytes_to_read;
	      n += bytes_to_read;

	      /* Note those blocks in the dcache. */
	      dcache_write (b->dcache_tag, p, offset + n, bytes_to_read);
	    }
	}

      retval = TRUE;
    }

 done:
  if (!retval)
    b->fpos = -1;	/* force a fresh seek next time */
  return retval;
}


/* Writes the specified number of bytes at the given offset for
 * the given device.  Returns TRUE on success, else FALSE.
 */
bool
Executor::blockdev_write (blockdev_t *b, uint32 offset, const void *buf,
		uint32 num_bytes)
{
  bool retval;

  retval = FALSE;  /* default */
  if (b && b->valid_p)
    {
      uint32 n;

#if !defined (BLOCKDEV_WRITE_THROUGH_CACHE)
      /* Be paranoid...nuke the cache for this device. */
      dcache_invalidate (b->dcache_tag);
#endif

      gui_assert ((offset % b->block_size) == 0);
      gui_assert ((num_bytes % b->block_size) == 0);

      for (n = 0; n < num_bytes; )
	{
	  uint32 bytes_to_write;
	  uint8 *p;

	  p = (uint8 *) buf + n;

	  /* Seek if necessary. */
	  if (offset + n != b->fpos)
	    {
	      if (!blockdev_seek_set (b, offset + n))
		goto done;
	    }

	  /* Actually write the bytes out. */
	  bytes_to_write = MIN (b->max_xfer_size, num_bytes - n);
	  if (b->write_func (b->fd, p, bytes_to_write) != bytes_to_write)
	    goto done;

#if defined (BLOCKDEV_WRITE_THROUGH_CACHE)
	  /* Note those blocks in the dcache. */
	  dcache_write (b->dcache_tag, p, offset + n, bytes_to_write);
#endif

	  b->fpos += bytes_to_write;
	  n += bytes_to_write;
	}

      retval = TRUE;
    }

 done:
  if (!retval)
    b->fpos = -1;	/* force a fresh seek next time */
  return retval;
}


void
Executor::blockdev_close (blockdev_t *b)
{
  if (!b->valid_p)
    gui_fatal ("Closing an invalid block device!");
  else {
      dcache_invalidate (b->dcache_tag);
      b->close_func (b->fd);
      b->valid_p = FALSE;
      free (b);
    }
}
