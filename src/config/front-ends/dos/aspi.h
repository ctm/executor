#if !defined(_ASPI_H_)
#define _ASPI_H_

#if defined(MSDOS)

#include "dosdisk.h"

/*
 * Copyright 1994-1996 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: aspi.h 63 2004-12-24 18:19:43Z ctm $
 */

typedef struct
{
    uint32_t fpos;
    int adaptor;
    int target;
    int lun;
    uint32_t block_length;
    uint32_t num_blocks;
    bool is_open;
    bool force_write_protect;
    bool write_protect;
    bool removable;
    bool media_present;
} aspi_info_t;

typedef struct
{
    int last_adaptor;
    int last_target;
    int last_lun;
} aspi_iterator_t;

typedef enum {
    HOST_ADAPTOR_INQUIRY,
    GET_DEVICE_TYPE,
    EXECUTE_SCSI_COMMAND,
    ABORT_SCSI_COMMAND,
    RESET_SCSI_DEVICE,
    SET_HOST_ADAPTOR_PARAMETERS,
    GET_DISK_DRIVE_INFORMATION,
} command_t;

#define PACKED __attribute__((packed))

typedef enum {
    DIR_UNSPECIFIED = (0 << 3),
    DIR_TARGET_TO_HOST = (1 << 3),
    DIR_HOST_TO_TARGET = (2 << 3),
    DIR_NO_TRANSFER = (3 << 3),
} dir_flags_t;

typedef enum {
    BUSY,
    NO_ERROR,
    COMPLETED_WITH_ERROR = 4,
} status_t;

typedef struct
{
    command_t command : 8 PACKED;
    status_t status : 8 PACKED;
    uint8 adaptor PACKED;
    uint8 flags PACKED;
    uint32_t reserved PACKED;
} srb_t;

typedef struct
{
    uint8 number_host_adaptors PACKED;
    uint8 host_adaptor_id PACKED;
    char scsi_manager_id[16] PACKED;
    char host_adapter_id[16] PACKED;
    char host_adapter_unique_parameters[16] PACKED;
} host_adaptor_query_t;

typedef enum {
    DIRECT_ACCESS_DEVICE,
    SEQUENTIAL_ACCESS_DEVICE,
    PRINTER_DEVICE,
    PROCESSOR_DEVICE,
    WRITE_ONCE_READ_MULTIPLE_DEVICE,
    READ_ONLY_DIRECT_ACCESS_DEVICE,
    LOGICAL_UNIT_NOT_PRESENT = 0x7F,
} peripheral_type_t;

typedef struct
{
    uint8 target_id PACKED;
    uint8 lun PACKED;
    peripheral_type_t device_type : 8 PACKED;
} get_device_type_t;

typedef enum {
    TEST_UNIT_READY = 0x0,
    READ_6 = 0x8,
    WRITE_6 = 0xA,
    INQUIRY = 0x12,
    MODE_SENSE = 0x1A,
    START_STOP = 0x1B,
    READ_10 = 0x28,
    WRITE_10 = 0x2A,
} operation_code_t;

typedef struct
{
    operation_code_t operation_code : 8 PACKED;
    uint8 lun_shifted_5 PACKED;
    unsigned short reserved PACKED;
    uint8 allocation_length PACKED;
    uint8 must_be_zero PACKED;
} mode_sense_t;

typedef struct
{
    operation_code_t operation_code : 8 PACKED;
    uint8 lun_shifted_5 PACKED;
    unsigned short reserved PACKED;
    uint8 allocation_length PACKED;
    uint8 must_be_zero PACKED;
} inquiry_t;

#define MAX_N_DESCRIPTORS 3

typedef struct
{
    uint8 density_code PACKED;
    uint8 number_of_blocks[3] PACKED;
    uint8 reserved PACKED;
    uint8 block_length[3] PACKED;
} block_descriptor_t;

typedef struct
{
    uint8 sense_data_length PACKED;
    uint8 medium_type PACKED;
    uint8 wp_shifted_7 PACKED;
    uint8 block_descriptor_length PACKED;
    block_descriptor_t block_descriptors[MAX_N_DESCRIPTORS] PACKED;
} mode_sense_data_t;

typedef struct
{
    uint8 peripheral_device_type PACKED;
    uint8 removable_bit_and_qualifier PACKED;
    uint8 iso_and_ecma_and_ansi PACKED;
    uint8 reserved PACKED;
    uint8 additional_length PACKED;
} inquiry_data_t;

typedef struct
{
    operation_code_t operation_code : 8 PACKED;
    uint8 lun_shifted_5_and_logical_msb PACKED;
    uint8 logical PACKED;
    uint8 logical_lsb PACKED;
    uint8 transfer_length PACKED;
    uint8 must_be_zero PACKED;
} read_write_6_t;

typedef struct
{
    operation_code_t operation_code : 8 PACKED;
    uint8 lun_shifted_5 PACKED;
    uint32_t logical_block_address PACKED;
    uint8 reserved PACKED;
    uint16_t transfer_length PACKED;
    uint8 must_be_zero PACKED;
} read_write_10_t;

typedef struct
{
    operation_code_t operation_code : 8 PACKED;
    uint8 lun_shifted_5_plus_immed PACKED;
    uint8 reserved[2] PACKED;
    uint8 start_stop_val PACKED;
    uint8 must_be_zero PACKED;
} start_stop_t;

typedef struct
{
    uint16_t offset PACKED;
    uint16_t segment PACKED;
} offset_segment_t;

#define DATA_SENSE_LENGTH 32

typedef struct
{
    uint8 flags;
    uint8 vendor_plus_lba_msb;
    uint8 lba_middle;
    uint8 lba_lsb;
} sense_info_t;

enum
{
    SENSE_FLAG_LBA_VALID = 0x80,
    SENSE_FLAG_ERROR_CLASS_MASK = 0x30,
    SENSE_FLAG_ERROR_CODE_MASK = 0x0F
};

enum
{
    SENSE_FLAG_ERROR_CLASS_SHIFT = 4,
    SENSE_FLAG_ERROR_CODE_SHIFT = 0
};

typedef struct
{
    uint8 target_id PACKED;
    uint8 lun PACKED;
    uint32_t data_allocation_length PACKED;
    uint8 sense_allocation_length PACKED;
    offset_segment_t data_buffer_pointer PACKED;
    offset_segment_t srb_link_pointer PACKED;
    uint8 cdb_length PACKED;
    uint8 host_adaptor_status PACKED;
    uint8 target_status PACKED;
    offset_segment_t post_routine_pointer PACKED;
    uint8 reserved[34] PACKED;
    union {
        mode_sense_t mode_sense PACKED;
        read_write_10_t read_write_10 PACKED;
        inquiry_t inquiry PACKED;
        read_write_6_t read_write_6 PACKED;
        start_stop_t start_stop PACKED;
    } u PACKED;
    /* data sense area immediately follows the cdb.  We can't have
       a field that provides it, because it is in different places
       depending on whether we have a 6 byte cdb (like mode sense)
       or a 10 byte cdb like our read and write.  The padding here
       is just to make sure that we have enough room */
    char data_sense_padding[DATA_SENSE_LENGTH] PACKED;
} execute_command_t;

typedef struct
{
    uint8 target_id PACKED;
    uint8 lun PACKED;
    uint8 reserved[14] PACKED;
    uint8 host_adaptor_status PACKED;
    uint8 target_status PACKED;
    offset_segment_t post_routine_pointer PACKED;
    uint8 reserved2[2] PACKED;
} reset_command_t;

#define INT13_MASK 3

typedef enum {
    NOT_ACCESSIBLE_VIA_INT13,
    INT13_AND_DOS,
    INT13_NO_DOS,
} aspi_drive_flags_t;

typedef struct
{
    uint8 target_id PACKED;
    uint8 lun PACKED;
    aspi_drive_flags_t drive_flags : 8 PACKED;
    uint8 int_13h_drive PACKED;
    uint8 preferred_head_translation PACKED;
    uint8 preferred_sector_translation PACKED;
    uint8 reserved[10] PACKED;
} get_disk_drive_information_t;

typedef struct
{
    srb_t srb PACKED;
    union {
        host_adaptor_query_t haq PACKED;
        get_device_type_t gdt PACKED;
        execute_command_t ec PACKED;
        get_disk_drive_information_t gddi PACKED;
        reset_command_t rc PACKED;
    } u;
} aspi_command_t;

extern int aspi_disk_close(int disk, bool eject);
extern off_t aspi_disk_seek(int fd, off_t pos, int unused);
extern int aspi_disk_read(int fd, void *bufp, int num_bytes);
extern int aspi_disk_write(int fd, const void *bufp, int num_bytes);
extern bool aspi_init(void);
extern void aspi_rescan(void);

/* 
 * Number of milliseconds to wait before timing out
 */
#define ASPI_DEFAULT_TIMEOUT 1000

#define ASPI_COMMAND_OFFSET (TRANSFER_BUFFER_SIZE)
#define ASPI_COMMAND_SPACE (2 * 1024) /* Plenty of space for ASPI. */

#if(ASPI_COMMAND_OFFSET + ASPI_COMMAND_SPACE) > (DOS_BUF_SIZE - DOS_MIN_STACK_SPACE)
#error "Not enough space for ASPI stuff in DOS memory."
#endif

#endif /* MSDOS */

#endif /* !_ASPI_H_ */
