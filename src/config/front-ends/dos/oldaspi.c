/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include <stdio.h>
#include <go32.h>
#include <dos.h>
#include <dpmi.h>

#include "aspi.h.~1.7~"
#include "rsys/hfs.h"
#include "rsys/assert.h"
#include "dosdisk.h"
#include "rsys/flags.h"

#if defined(ASPI_STANDALONE)
#include <assert.h>
#undef gui_assert
#define gui_assert assert
#undef warning_trace_info
#define warning_trace_info printf
#undef warning_unexpected
#define warning_unexpected printf
#define NL "\n"
#else
#define NL
#endif
/*
 * NOTE: This implementation is NOT reentrant.  To make it so would require
 *	 different management of conventional memory, which would either
 *	 require sucking up more memory than we use, or the possibility that
 *	 a reentrant call would fail due to lack of conventional memory
 *	 anyway.  DOS sucks.
 */

/*
 * These typedefs aren't in aspi.h since the size of the wrapper needs to
 * be taken into account, so it is nice to have the wrapper_t and things that
 * depend on it all near the actual code for the wrapper.
 */

typedef unsigned char wrapper_t[0x46];

/*
 * NOTE: The wrapper code below depends on this particular layout for
 *	 low_memory_contents_t.  Don't mess with either unless you are
 *	 fully aware of how the wrapper code works.
 */

typedef struct
{
    wrapper_t wrapper PACKED;
    offset_segment_t aspi_entry PACKED;
    srb_t srb PACKED;
    aspi_command_t u PACKED;
} low_memory_contents_t;

#if !defined(PRIVATE)
#define PRIVATE static
#endif

#if !defined(PUBLIC)
#define PUBLIC
#endif

PUBLIC int ROMlib_skipaspi = 0;

PRIVATE low_memory_contents_t memory_template = {
    {
        /* 0000		 _aspi_wrapper proc far              */
        0x0e, /* 0000  0E	       push cs                       */
        0x1f, /* 0001  1F	       pop  ds                       */
        0xa1, 0x46, 0x00, /* 0002  A1 0046r      mov  ax,word ptr aspi_entry   */
        0x0b, 0x06, 0x48, 0x00, /* 0005  0B 06 0048r   or   ax,word ptr aspi_entry+2 */
        0x75, 0x1d, /* 0009  75 1D	       jne  short @1@422             */
        0xb8, 0x00, 0x3d, /* 000B  B8 3D00       mov  ax, 03d00H               */
        0xba, 0x3d, 0x00, /* 000E  BA 003Dr      lea  dx, scsi_mgr             */
        0xcd, 0x21, /* 0011  CD 21	       int  021H                     */
        0x72, 0x27, /* 0013  72 27	       jc   short @1@478             */
        0x50, /* 0015  50	       push ax                       */
        0x8b, 0xd8, /* 0016  8B D8	       mov  bx, ax                   */
        0xb8, 0x02, 0x44, /* 0018  B8 4402       mov  ax, 04402H               */
        0xba, 0x46, 0x00, /* 001B  BA 0046r      lea  dx, aspi_entry           */
        0xb9, 0x04, 0x00, /* 001E  B9 0004       mov  cx, 4                    */
        0xcd, 0x21, /* 0021  CD 21	       int  021H                     */
        0xb4, 0x3e, /* 0023  B4 3E	       mov  ah, 03eH                 */
        0x5b, /* 0025  5B	       pop  bx                       */
        0xcd, 0x21, /* 0026  CD 21	       int  021H                     */
        /* 0028		  @1@422:                            */
        0xa1, 0x46, 0x00, /* 0028  A1 0046r      mov ax,word ptr aspi_entry    */
        0x0b, 0x06, 0x48, 0x00, /* 002B  0B 06 0048r   or  ax,word ptr aspi_entry+2  */
        0x74, 0x0b, /* 002F  74 0B	       je	     short @1@478    */
        0x1e, /* 0031  1E	       push    ds                    */
        0xb8, 0x4a, 0x00, /* 0032  B8 004Ar      mov     ax,offset haq         */
        0x50, /* 0035  50	       push    ax                    */
        0xff, 0x1e, 0x46, 0x00, /* 0036  FF 1E 0046r   call    dword ptr aspi_entry  */
        0x59, /* 003A  59	       pop     cx                    */
        0x59, /* 003B  59	       pop     cx                    */
        /* 003C		 @1@478:                             */
        0xcb, /* 003C  CB	       retf                          */
        /* 003D		 _aspi_wrapper   endp                */
        /* 003D		 scsi_mgr	     label byte      */
        0x53, 0x43, 0x53, 0x49, /* 003D  53 43 53 49   db	'SCSIMGR$', 0        */
        0x4d, 0x47, 0x52, 0x24, /* 0041  4D 47 52 24				     */
        0x00, /* 0045  00					     */
    },
    {
        0, 0,
    },
    {
        0, 0, 0, 0, 0,
    },
    /* more zeros go here */
};

#define linear_address_of_segment(segment) ((unsigned long)(segment) << 4)

#define WRAPPER_OFFSET(element) \
    ((char *)&element - (char *)&memory_template)

#define WRAPPER_MEM_GET(element)                                  \
    movedata(aspi_wrap_dos_mem_selector, WRAPPER_OFFSET(element), \
             dos_pm_ds, (unsigned)&element, sizeof(element))

#define WRAPPER_MEM_PUT(element)                                  \
    movedata(dos_pm_ds, (unsigned)&element,                       \
             aspi_wrap_dos_mem_selector, WRAPPER_OFFSET(element), \
             sizeof(element))

#define ASPI_CALL()                                            \
    do                                                         \
    {                                                          \
        memset(&regs, 0, sizeof(regs));                        \
        regs.x.cs = aspi_wrap_dos_mem_segment;                 \
        regs.x.ip = 0;                                         \
        regs.x.ss = dos_buf_segment;                           \
        regs.x.sp = DOS_STACK_TOP;                             \
        __dpmi_simulate_real_mode_procedure_retf(&regs, 0, 0); \
    } while(0)

#define ASPI_WAIT(timeout)                                     \
    do                                                         \
    {                                                          \
        timeout = 409600;                                      \
        do                                                     \
        {                                                      \
            WRAPPER_MEM_GET(memory_template.srb.status);       \
        } while(memory_template.srb.status == 0 && --timeout); \
    } while(0)

#define ASPI_CALL_WAIT(timeout) \
    do                          \
    {                           \
        ASPI_CALL();            \
        ASPI_WAIT(timeout);     \
    } while(0)

PRIVATE unsigned long get_multi_byte_value(int n_bytes, unsigned char *bytes)
{
    unsigned long retval;

    retval = 0;
    while(--n_bytes >= 0)
        retval = (retval << 8) | *bytes++;

    return retval;
}

#define GET_MULTI_BYTE_VALUE(element) \
    get_multi_byte_value(sizeof(element), element)

PRIVATE void set_multi_byte_value(int n_bytes, unsigned char *bytes, unsigned long value)
{
    bytes += n_bytes;

    while(--n_bytes >= 0)
    {
        *--bytes = value;
        value >>= 8;
    }
}

#define SET_MULTI_BYTE_VALUE(element, value) \
    set_multi_byte_value(sizeof(element), (element), (value))

PUBLIC void aspi_iterator_init(aspi_iterator_t *aip)
{
    aip->last_adaptor = -1;
    aip->last_target = 7;
    aip->last_lun = 7;
}

#if defined(FUTILE_HACK)
/*
 * NOTE: tick_timer() is here to allow us to detect when ASPI is messing
 *	 up on our Dell 90 P5.  However, it appears that the low memory
 *	 ticks aren't changing while we're having trouble, so, as written,
 *	 this routine is useless.
 */

PRIVATE unsigned long tick_timer(void)
{
    unsigned long retval;

    dosmemget(0x40 * 16 + 0x6c, sizeof(retval), &retval);
    return retval;
}
#endif /* FUTILE_HACK */

PRIVATE uint16_t aspi_wrap_dos_mem_segment, aspi_wrap_dos_mem_selector;

PUBLIC BOOLEAN get_aspi_info(aspi_iterator_t *aip, aspi_info_t *aspi_info_p)
{
    static BOOLEAN beenhere = false, has_aspi = false;
    __dpmi_regs regs;
    unsigned long timeout;
    static int number_host_adaptors;
    BOOLEAN done;
    int seg, sel;
    mode_sense_data_t mode_sense_data;
#if defined(FUTILE_HACK)
    unsigned long ticks;
#endif /* FUTILE_HACK */
    BOOLEAN last_success;
    BOOLEAN retval;

    if(!beenhere)
    {
        beenhere = true;
        if(ROMlib_skipaspi)
            has_aspi = false;
        else
        {
            warning_trace_info("first time in aspi" NL);
            seg = (__dpmi_allocate_dos_memory((sizeof(memory_template) + 15) / 16,
                                              &sel));
            if(seg == -1)
            {
                fprintf(stderr, "Unable to allocate %ld bytes of conventional mem "
                                "for aspi support\n",
                        (long)sizeof(memory_template));
                has_aspi = false;
            }
            else
            {
                aspi_wrap_dos_mem_segment = seg;
                aspi_wrap_dos_mem_selector = sel;
                memory_template.srb.command = HOST_ADAPTOR_INQUIRY;
                memory_template.srb.adaptor = 0;
                memory_template.srb.flags = 0;
                memory_template.srb.reserved = 0;
                WRAPPER_MEM_PUT(memory_template);
                ASPI_CALL();

                WRAPPER_MEM_GET(memory_template.aspi_entry);
                if(memory_template.aspi_entry.segment
                   || memory_template.aspi_entry.offset)
                {
                    ASPI_WAIT(timeout);
                    WRAPPER_MEM_GET(memory_template.u.haq);
                    number_host_adaptors = memory_template.u.haq.number_host_adaptors;
                    has_aspi = true;
                }
                else
                {
                    __dpmi_free_dos_memory(aspi_wrap_dos_mem_selector);
                    aspi_wrap_dos_mem_segment = 0;
                    aspi_wrap_dos_mem_selector = 0;
                    has_aspi = false;
                }
            }
        }
    }
    retval = false;
    last_success = true;
    if(has_aspi)
    {
        done = false;
        do
        {
            ++aip->last_lun;
            if(1 || aip->last_lun > 7 || !last_success)
            {
                aip->last_lun = 0;
                ++aip->last_target;
                if(aip->last_target > 6)
                {
                    aip->last_target = 0;
                    ++aip->last_adaptor;
                    if(aip->last_adaptor >= number_host_adaptors)
                        done = true;
                }
            }
            warning_trace_info("adaptor = %d, target = %d, lun = %d" NL,
                               aip->last_adaptor, aip->last_target, aip->last_lun);
            last_success = false;
            if(!done)
            {
                memory_template.srb.command = GET_DEVICE_TYPE;
                memory_template.srb.adaptor = aip->last_adaptor;
                memory_template.srb.flags = 0;
                memory_template.srb.reserved = 0;
                memory_template.u.gdt.target_id = aip->last_target;
                memory_template.u.gdt.lun = aip->last_lun;
                WRAPPER_MEM_PUT(memory_template.srb);
                WRAPPER_MEM_PUT(memory_template.u.gdt);

#if defined(FUTILE_HACK)
                ticks = tick_timer();
                ASPI_CALL_WAIT(timeout);
                /*
	 * Nasty hack needed on our Dell P5 90
	 */
                if(tick_timer() - ticks > 10)
                {
                    fprintf(stderr, "retrying\n");
                    fflush(stdout);
                    ASPI_CALL_WAIT(timeout);
                }
#else /* !FUTILE_HACK */
                ASPI_CALL_WAIT(timeout);
#endif /* !FUTILE_HACK */
                WRAPPER_MEM_GET(memory_template.u.gdt);
                if(memory_template.srb.status == NO_ERROR && (memory_template.u.gdt.device_type == DIRECT_ACCESS_DEVICE || memory_template.u.gdt.device_type == READ_ONLY_DIRECT_ACCESS_DEVICE))
                {

                    memory_template.srb.command = GET_DISK_DRIVE_INFORMATION;
                    memory_template.srb.adaptor = aip->last_adaptor;
                    memory_template.srb.flags = 0;
                    memory_template.srb.reserved = 0;
                    memory_template.u.gddi.target_id = aip->last_target;
                    memory_template.u.gddi.lun = aip->last_lun;
                    WRAPPER_MEM_PUT(memory_template.srb);
                    WRAPPER_MEM_PUT(memory_template.u.gddi);
                    ASPI_CALL_WAIT(timeout);
                    WRAPPER_MEM_GET(memory_template.u.gddi);
                    if(memory_template.srb.status != NO_ERROR || (memory_template.u.gddi.drive_flags & INT13_MASK) == NOT_ACCESSIBLE_VIA_INT13)
                    {
                        memory_template.srb.command = EXECUTE_SCSI_COMMAND;
                        memory_template.srb.adaptor = aip->last_adaptor;
                        memory_template.srb.flags = DIR_TARGET_TO_HOST;
                        memory_template.srb.reserved = 0;
                        memory_template.u.ec.target_id = aip->last_target;
                        memory_template.u.ec.lun = aip->last_lun;

                        memory_template.u.ec.data_allocation_length = 64;
                        memory_template.u.ec.sense_allocation_length = DATA_SENSE_LENGTH;

                        memory_template.u.ec.data_buffer_pointer.offset = 0;
                        memory_template.u.ec.data_buffer_pointer.segment = dos_buf_segment;
                        memory_template.u.ec.srb_link_pointer.offset = 0;
                        memory_template.u.ec.srb_link_pointer.segment = 0;
                        memory_template.u.ec.post_routine_pointer.offset = 0;
                        memory_template.u.ec.post_routine_pointer.segment = 0;
                        memory_template.u.ec.cdb_length = 6;

                        memory_template.u.ec.u.mode_sense.operation_code = MODE_SENSE;
                        memory_template.u.ec.u.mode_sense.lun_shifted_5 = aip->last_lun << 5;
                        memory_template.u.ec.u.mode_sense.reserved = 0;
                        memory_template.u.ec.u.mode_sense.allocation_length = 64;
                        memory_template.u.ec.u.mode_sense.must_be_zero = 0;

                        WRAPPER_MEM_PUT(memory_template.srb);
                        WRAPPER_MEM_PUT(memory_template.u.ec);
                        ASPI_CALL_WAIT(timeout);
                        if(memory_template.srb.status == NO_ERROR)
                        {
                            movedata(dos_buf_selector, 0,
                                     dos_pm_ds, (unsigned)&mode_sense_data,
                                     sizeof(mode_sense_data));
                            aspi_info_p->adaptor = aip->last_adaptor;
                            aspi_info_p->target = aip->last_target;
                            aspi_info_p->lun = aip->last_lun;
                            aspi_info_p->block_length = GET_MULTI_BYTE_VALUE(mode_sense_data.block_descriptors[0].block_length);
                            aspi_info_p->num_blocks = GET_MULTI_BYTE_VALUE(mode_sense_data.block_descriptors[0].number_of_blocks);
                            aspi_info_p->write_protect = mode_sense_data.wp_shifted_7 >> 7;
                            retval = true;
                            done = true;
                            last_success = true;
                        }
                    }
                }
            }
        } while(!done);
    }
    return retval;
}

#define MAX_ASPI_DISKS 12 /* arbitrary: two full host adaptors */

PRIVATE aspi_info_t aspi_info[MAX_ASPI_DISKS];

PRIVATE aspi_info_t *disk_number_to_aspi_info(int disk)
{
    aspi_info_t *retval;

    if(disk >= 0 && disk < MAX_ASPI_DISKS && aspi_info[disk].block_length)
        retval = &aspi_info[disk];
    else
        retval = 0;

    return retval;
}

/*
 * aspi_disk_open is private because it does no checking to see whether
 * or not the drive is already open.
 */

PRIVATE int aspi_disk_open(int disk, char *ejectablep, LONGINT *bsizep)
{
    BOOLEAN retval;
    aspi_info_t *aitp;

    aitp = disk_number_to_aspi_info(disk);
    if(aitp)
    {
        *ejectablep = false;
        *bsizep = aitp->block_length;
        retval = true;
    }
    else
        retval = false;
    return retval;
}

/*
 * NOTE: currently aspi_disk_close will never be called since the only
 *	 call to it is from eject_disk, and we always claim that SCSI
 *	 disks are not ejectable.  This will change sometime.
 */

PUBLIC int aspi_disk_close(int disk)
{
    return 0;
}

PUBLIC off_t aspi_disk_seek(int fd, off_t pos, int unused)
{
    aspi_info_t *aitp;
    unsigned long bytes_on_disk;

    gui_assert(unused == L_SET);
    aitp = disk_number_to_aspi_info(fd);
    if(!aitp)
    {
        gui_assert(0);
        return -1;
    }
    gui_assert((pos % aitp->block_length) == 0);

    /* Pin the fpos at the end of the disk. */
    bytes_on_disk = aitp->block_length * aitp->num_blocks;
    if(pos > bytes_on_disk)
        pos = bytes_on_disk;
    aitp->fpos = pos;

    return pos;
}

PRIVATE int aspi_disk_xfer(operation_code_t op, int disk, void *buf,
                           int num_bytes)
{
    int orig_num_bytes;
    aspi_info_t *aspi_info_p;
    unsigned long blocks_to_xfer, bytes_to_xfer, bytes_xfered;
    __dpmi_regs regs;
    unsigned long timeout;

    gui_assert(op == READ || op == WRITE);
    orig_num_bytes = num_bytes;
    aspi_info_p = disk_number_to_aspi_info(disk);
    if(!aspi_info_p)
    {
        gui_assert(0);
        /**/ return -1;
    }
    gui_assert((num_bytes % aspi_info_p->block_length) == 0);
    while(num_bytes != 0)
    {
        blocks_to_xfer = num_bytes / aspi_info_p->block_length;
        if(blocks_to_xfer > TRANSFER_BUFFER_SIZE / aspi_info_p->block_length)
            blocks_to_xfer = TRANSFER_BUFFER_SIZE / aspi_info_p->block_length;
        bytes_to_xfer = blocks_to_xfer * aspi_info_p->block_length;
        memory_template.srb.command = EXECUTE_SCSI_COMMAND;
        memory_template.srb.adaptor = aspi_info_p->adaptor;
        memory_template.srb.flags = ((op == READ)
                                         ? DIR_TARGET_TO_HOST
                                         : DIR_HOST_TO_TARGET);
        memory_template.srb.reserved = 0;
        memory_template.u.ec.target_id = aspi_info_p->target;
        memory_template.u.ec.lun = aspi_info_p->lun;
        memory_template.u.ec.data_allocation_length = bytes_to_xfer;
        memory_template.u.ec.sense_allocation_length = DATA_SENSE_LENGTH;
        memory_template.u.ec.data_buffer_pointer.offset = 0;
        memory_template.u.ec.data_buffer_pointer.segment = dos_buf_segment;
        memory_template.u.ec.srb_link_pointer.offset = 0;
        memory_template.u.ec.srb_link_pointer.segment = 0;
        memory_template.u.ec.post_routine_pointer.offset = 0;
        memory_template.u.ec.post_routine_pointer.segment = 0;
        memory_template.u.ec.cdb_length = 10;

        memory_template.u.ec.u.read_write.operation_code = op;
        memory_template.u.ec.u.read_write.lun_shifted_5 = aspi_info_p->lun << 5;
        SET_MULTI_BYTE_VALUE(memory_template.u.ec.u.read_write.logical_block_address, aspi_info_p->fpos / aspi_info_p->block_length);
        memory_template.u.ec.u.read_write.reserved = 0;
        SET_MULTI_BYTE_VALUE(memory_template.u.ec.u.read_write.transfer_length, blocks_to_xfer);
        memory_template.u.ec.u.read_write.must_be_zero = 0;

        if(op == WRITE)
            movedata(dos_pm_ds, (unsigned)buf, dos_buf_selector, 0,
                     bytes_to_xfer);

        WRAPPER_MEM_PUT(memory_template.srb);
        WRAPPER_MEM_PUT(memory_template.u.ec);
        ASPI_CALL_WAIT(timeout);
        if(memory_template.srb.status != NO_ERROR)
        {
            /*-->*/ return orig_num_bytes - num_bytes;
        }
        bytes_xfered = bytes_to_xfer;
        if(op == READ)
            movedata(dos_buf_selector, 0, dos_pm_ds, (unsigned)buf,
                     bytes_xfered);
        aspi_info_p->fpos += bytes_xfered;
        buf += bytes_xfered;
        num_bytes -= bytes_xfered;
    }
    return orig_num_bytes - num_bytes;
}

PUBLIC int aspi_disk_read(int disk, void *buf, int num_bytes)
{
    return aspi_disk_xfer(READ, disk, buf, num_bytes);
}

PUBLIC int aspi_disk_write(int disk, const void *buf, int num_bytes)
{
    return aspi_disk_xfer(WRITE, disk, (void *)buf, num_bytes);
}

PRIVATE void aspi_shutdown(void)
{
    if(aspi_wrap_dos_mem_segment)
    {
        __dpmi_free_dos_memory(aspi_wrap_dos_mem_selector);
        aspi_wrap_dos_mem_segment = 0;
        aspi_wrap_dos_mem_selector = 0;
    }
}

/*
 * Code largely taken from futzwithdosdisks in stdfile.c -- If you find
 * bugs here, you should check there for the same bugs.  These should be
 * merged sometime.
 */

#if !defined(ASPI_STANDALONE)
PUBLIC void aspi_init(void)
{
    int i;
    LONGINT mess;
    LONGINT blocksize;
    char ejectable;
    aspi_iterator_t ait;
    char aspi_name[sizeof("aspi(nn,nn,nn)")];
    drive_flags_t flags;

    aspi_iterator_init(&ait);
    for(i = 0; i < NELEM(aspi_info); ++i)
    {
        if(!get_aspi_info(&ait, &aspi_info[i]))
            /*-->*/ break;
        if(aspi_disk_open(i, &ejectable, &blocksize) >= 0)
        {
            sprintf(aspi_name, "aspi(%d,%d,%d)",
                    aspi_info[i].adaptor, aspi_info[i].target, aspi_info[i].lun);
            flags = 0;
            if(!ejectable)
                flags |= DRIVE_FLAGS_FIXED;
            if(aspi_info[i].write_protect)
                flags |= DRIVE_FLAGS_LOCKED;
            try_to_mount_disk(aspi_name, i | ASPIFDBIT, &mess,
                              blocksize, 16 * PHYSBSIZE, flags);
            mess = CL(mess);
            if(mess)
            {
                if(mess >> 16 == 0)
                {
                    PPostEvent(diskEvt, mess, (GUEST<EvQElPtr> *)0);
                    /* TODO: we probably should post if mess returns an
			     error, but I think we get confused if we do */
                }
                else
                {
                    aspi_disk_close(i);
                }
            }
            else
            {
                aspi_disk_close(i);
            }
        }
    }
}
#endif

#if defined(ASPI_STANDALONE)
PUBLIC int main(void)
{
    aspi_iterator_t aspi_iterator;
    aspi_info_t aspi_info;

    if(!init_dos_memory())
    {
        fprintf(stderr, "Unable to initialize DOS memory.\n");
        exit(-1);
    }

    aspi_iterator_init(&aspi_iterator);
    while(get_aspi_info(&aspi_iterator, &aspi_info))
    {
        printf("adaptor = %d, target = %d, lun = %d\n", aspi_info.adaptor,
               aspi_info.target, aspi_info.lun);
        printf("block length = %lu, num blocks = %lu, write protect = %d\n",
               aspi_info.block_length, aspi_info.num_blocks, aspi_info.write_protect);
    }
    return 0;
}
#endif
