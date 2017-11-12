/* Copyright 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_dcache[] = "$Id: dcache.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/dcache.h"

using namespace Executor;

#define DCACHE_BLOCK_SIZE 512

typedef struct _dcache_entry_t
{
    uint32 fd; /* tag */
    uint32 offset; /* block's offset */
    uint32 when_last_accessed; /* the smaller the older; 0 means invalid */
    uint8 data[DCACHE_BLOCK_SIZE]; /* actual cached data */
    write_callback_funcp_t dirty_callback; /* callback to write dirty data
					   (only set if the data is indeed
					   dirty) */
} dcache_entry_t;

#define DCACHE_ENTRY_VALID_P(d) ((d)->when_last_accessed != 0)
#define DCACHE_ENTRY_INVALIDATE(d) ((void)((d)->when_last_accessed = 0))

/* Number of blocks in the dcache. */
#if !defined(CYGWIN32)
#define DCACHE_NUM_ENTRIES 64
#else
#define DCACHE_NUM_ENTRIES 720 /* enough to cache 1/4 of 1.4 MB floppy */
#endif

/* True iff the cache is enabled. */
static bool dcache_enabled_p;

/* This is the actual cache. */
static dcache_entry_t dcache[DCACHE_NUM_ENTRIES];
#define DCACHE_END (&dcache[DCACHE_NUM_ENTRIES])

/* Incremented counter, so we can do LRU */
static uint32 now = 1;

/* Finds the valid dcache entry corresponding to the given fd and
 * offset, or NULL if none is found.
 */
static dcache_entry_t *
dcache_entry_lookup(uint32 fd, uint32 offset)
{
    dcache_entry_t *d;

    for(d = &dcache[0]; d < DCACHE_END; d++)
        if(d->fd == fd && d->offset == offset && DCACHE_ENTRY_VALID_P(d))
            goto done;
    d = NULL; /* no match */

done:
    return d;
}

/* Sets whether or not the dcache is enabled and returns the old state. */
bool Executor::dcache_set_enabled(bool enabled_p)
{
    bool old_enabled_p;
    old_enabled_p = dcache_enabled_p;
    if(old_enabled_p && !enabled_p)
        dcache_invalidate_all(true);
    dcache_enabled_p = enabled_p;
    return old_enabled_p;
}

#define COALESCE_WRITES

enum
{
    SECTORS_PER_FLOPPY_CYLINDER = 18 * 2
};
enum
{
    MAX_BACKWARDS = SECTORS_PER_FLOPPY_CYLINDER,
    MAX_FORWARDS = SECTORS_PER_FLOPPY_CYLINDER
};

#if defined(COALESCE_WRITES)

static void
fill_run(dcache_entry_t *dps[], int nelems, int *indexp, int offset_increment,
         uint32 offset, uint32 fd)
{
    uint32 candidate_offset;
    int nclean;
    int index;

    index = 0;
    candidate_offset = offset;
    nclean = 0;
    for(index = 0; index < nelems; ++index)
    {
        dcache_entry_t *d;

        candidate_offset += offset_increment;
        d = dcache_entry_lookup(fd, candidate_offset);
        if(!d)
            /*-->*/ break;
        else
        {
            if(d->dirty_callback)
                nclean = 0;
            else
            {
                ++nclean;
                if(nclean == 2)
                {
                    index -= nclean - 1;
                    /*-->*/ break;
                }
            }
            dps[index] = d;
        }
    }
    *indexp = index;
}

static void
copy_buffer(uint8 **bufpp, dcache_entry_t *d)
{
    memcpy(*bufpp, d->data, sizeof d->data);
    *bufpp += DCACHE_BLOCK_SIZE;
    d->dirty_callback = NULL;
}

static void
coalesce_writes(uint32 fd, uint8 **bufpp, uint32 *lengthp, uint32 *offsetp)
{
    dcache_entry_t *backwards_dps[MAX_BACKWARDS];
    int backwards_index;

    dcache_entry_t *forwards_dps[MAX_FORWARDS];
    int forwards_index;

    /* find beginning of run */
    fill_run(backwards_dps, NELEM(backwards_dps), &backwards_index,
             -DCACHE_BLOCK_SIZE, *offsetp, fd);

    /* find ending of run */
    fill_run(forwards_dps, NELEM(forwards_dps), &forwards_index,
             DCACHE_BLOCK_SIZE, *offsetp, fd);

    if(backwards_index > 0 || forwards_index > 0)
    {
        int n_bufs;
        uint8 *bufp;
        uint8 *outbufp;
        uint32 length;
        dcache_entry_t *d;
        int i;

        n_bufs = backwards_index + 1 + forwards_index;

        /* allocate space for run */
        length = n_bufs * DCACHE_BLOCK_SIZE;
        bufp = (uint8 *)malloc(length);
        outbufp = bufp;

        for(i = backwards_index - 1; i >= 0; --i)
            copy_buffer(&outbufp, backwards_dps[i]);

        d = dcache_entry_lookup(fd, *offsetp);
        copy_buffer(&outbufp, d);

        for(i = 0; i < forwards_index; ++i)
            copy_buffer(&outbufp, forwards_dps[i]);

        *bufpp = bufp;
        *lengthp = length;
        *offsetp -= DCACHE_BLOCK_SIZE * backwards_index;
    }
}
#endif

static bool
dcache_flush_entry(dcache_entry_t *dp)
{
    bool retval;

    if(!dp->dirty_callback)
        retval = true;
    else
    {
        uint8 *bufp;
        uint32 length;
        uint32 offset;
        write_callback_funcp_t dirty_callback;

        dirty_callback = dp->dirty_callback;
        bufp = dp->data;
        length = sizeof dp->data;
        offset = dp->offset;

#if defined(COALESCE_WRITES)
        coalesce_writes(dp->fd, &bufp, &length, &offset);
#endif

        retval = dirty_callback(dp->fd, bufp, offset, length) == length;
        if(bufp != dp->data)
            free(bufp);
        dp->dirty_callback = NULL;
    }
    return retval;
}

static bool
dcache_invalidate_entry(dcache_entry_t *dp, bool flush_p)
{
    bool retval;

    if(flush_p)
        retval = dcache_flush_entry(dp);
    else
        retval = true;
    DCACHE_ENTRY_INVALIDATE(dp);
    return retval;
}

/* Returns an invalid cache entry, if one exists.  If all dcache entries
 * are valid, returns the one least recently used.
 *
 * NOTE: Do not change the least recently used algorithm or you may break
 *       track at a time reading.  Track at a time reading depends on the
 *       fact that as an entire track is read in, each of the buffers in
 *       that track will be available.
 * 
 * If it's impossible to flush a cache entry, NULL is returned.
 */

static dcache_entry_t *
best_dcache_entry_to_replace(void)
{
    uint32 best_time;
    dcache_entry_t *d, *best;

    best_time = UINT32_MAX;
    best = &dcache[0]; /* failsafe */

    for(d = &dcache[0]; d < DCACHE_END; d++)
    {
        if(!DCACHE_ENTRY_VALID_P(d))
        {
            /* Any invalid entry is a perfect candidate for replacement. */
            best = d;
            break;
        }
        else
        {
            if(d->when_last_accessed < best_time)
            {
                best_time = d->when_last_accessed;
                best = d;
            }
        }
    }

    if(!dcache_invalidate_entry(best, true))
        best = NULL;

    return best;
}

#define READ_CYLINDER_AT_A_TIME

#if defined(READ_CYLINDER_AT_A_TIME)
static dcache_entry_t *
read_cylinder(uint32 fd, uint32 offset, read_callback_funcp_t read_callback)
{
    uint32 nread;
    uint8 buf[SECTORS_PER_FLOPPY_CYLINDER * 512], *bufp;
    dcache_entry_t *retval;
    uint32 new_offset;

    new_offset = offset / sizeof buf * sizeof buf;
    nread = read_callback(fd, buf, new_offset, sizeof buf);
    if(nread != sizeof buf)
        retval = NULL;
    else
    {
        for(bufp = buf;
            bufp + DCACHE_BLOCK_SIZE <= buf + sizeof buf;
            bufp += DCACHE_BLOCK_SIZE, new_offset += DCACHE_BLOCK_SIZE)
        {
            if(!dcache_entry_lookup(fd, new_offset))
            {
                uint32 n_written;

                n_written = dcache_write(fd, bufp, new_offset,
                                         DCACHE_BLOCK_SIZE, NULL);
                if(n_written != DCACHE_BLOCK_SIZE)
                    warning_unexpected(NULL_STRING);
            }
        }
        retval = dcache_entry_lookup(fd, offset);
    }
    return retval;
}
#endif

/* Trys to read the specified number of bytes from the cache,
 * using the callback function if necessary.  Returns how many bytes
 * were read.
 */
uint32
Executor::dcache_read(uint32 fd, void *buf, uint32 offset, uint32 count,
                      read_callback_funcp_t read_callback)
{
    uint32 n;
    uint32 retval;

    retval = 0;
    if(dcache_enabled_p)
    {
        ++now;

        for(n = 0; n < count; n += DCACHE_BLOCK_SIZE)
        {
            dcache_entry_t *d;

            d = dcache_entry_lookup(fd, offset);
            if(d == NULL)
            {
                if(!read_callback)
                    /*-->*/ break;
                else
                {
#if !defined(READ_CYLINDER_AT_A_TIME)
                    d = best_dcache_entry_to_replace();
                    if(!d)
                        /*-->*/ break;
                    else
                    {
                        uint32 nread;

                        nread = read_callback(fd, d->data, offset,
                                              DCACHE_BLOCK_SIZE);
                        if(nread != DCACHE_BLOCK_SIZE)
                            /*-->*/ break;
                        else
                        {
                            d->fd = fd;
                            d->offset = offset;
                            d->dirty_callback = NULL;
                        }
                    }
#else
                    d = read_cylinder(fd, offset, read_callback);
                    if(!d)
                        /*-->*/ break;
#endif
                }
            }
            d->when_last_accessed = now;
            {
                uint32 n_to_copy;

                n_to_copy = MIN((uint32)DCACHE_BLOCK_SIZE, count - n);
                memcpy((uint8 *)buf + n, d->data, n_to_copy);
                retval += n_to_copy;
            }
            offset += DCACHE_BLOCK_SIZE;
        }
    }
    return retval;
}

/* Caches the specified bytes for possible later use. */
uint32
Executor::dcache_write(uint32 fd, const void *buf, uint32 offset, uint32 count,
                       write_callback_funcp_t dirty_callback)
{
    uint32 retval;

    retval = 0;
    if(dcache_enabled_p)
    {
        uint32 n;

        ++now;

        /* Record all full blocks into the cache.  Ignore partial blocks. */
        for(n = 0; n + DCACHE_BLOCK_SIZE <= count; n += DCACHE_BLOCK_SIZE)
        {
            dcache_entry_t *d;

            d = dcache_entry_lookup(fd, offset + n); /* replace existing? */
            if(d == NULL)
            {
                d = best_dcache_entry_to_replace();
                if(!d)
                    /*-->*/ break;
                else
                {
                    d->fd = fd;
                    d->offset = offset + n;
                }
            }
            d->dirty_callback = dirty_callback;
            memcpy(d->data, (const uint8 *)buf + n, DCACHE_BLOCK_SIZE);
            d->when_last_accessed = now;
            retval += DCACHE_BLOCK_SIZE;
        }
    }
    return retval;
}

/* Throws away all cached information associated with FD. */
bool Executor::dcache_invalidate(uint32 fd, bool flush_p)
{
    bool retval;

    retval = true;
    if(dcache_enabled_p)
    {
        dcache_entry_t *d;

        for(d = &dcache[0]; d < DCACHE_END; d++)
            if(d->fd == fd)
                if(!dcache_invalidate_entry(d, flush_p))
                    retval = false;
    }
    return retval;
}

bool Executor::dcache_flush(uint32 fd)
{
    bool retval;

    retval = true;
    if(dcache_enabled_p)
    {
        dcache_entry_t *d;

        for(d = &dcache[0]; d < DCACHE_END; d++)
            if(d->fd == fd)
                if(!dcache_flush_entry(d))
                    retval = false;
    }
    return retval;
}

/* Throws away all cached information. */
bool Executor::dcache_invalidate_all(bool flush_p)
{
    bool retval;

    retval = true;
    if(dcache_enabled_p)
    {
        dcache_entry_t *d;

        for(d = &dcache[0]; d < DCACHE_END; d++)
            if(!dcache_invalidate_entry(d, flush_p))
                retval = false;
    }
    return retval;
}
