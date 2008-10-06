#if !defined (__vwin32_h__)

#define __vwin32_h__

/*
  NOTE: DO not change anything in this file, since it's just glue for
  the (relatively undocumented) Microsoft Device_IOCONTROL code interface.
 */

#if !defined (PACKED)
#define PACKED __attribute__((packed))
#endif

typedef enum
{
  VWIN32_IOCTL = 1,
  VWIN32_SECTOR_READ = 2,
  VWIN32_SECTOR_WRITE = 3,
  VWIN32_DRIVE_INFO = 6,
  VWIN32_EXTENDED_OP = 6,
}
DeviceIoControl_function_t;

typedef uint32 drive_number_0t; /* zero based */
typedef uint32 drive_number_1t; /* one based */

typedef struct
{
  uint32 diStartSector PACKED;
  uint16 diSectors     PACKED;
  void *diBuffer       PACKED;
}
disk_io_t;

typedef enum { SECTOR_XFER_MAGIC = 0xFFFF } sector_xfer_magic0_t;
typedef enum { EXTENDED_SECTOR_OP = 0x7305 } sector_xfer_magic1_t;

typedef enum { READ_OP = 0, WRITE_OP = 1 } extended_op_t;

typedef struct
{
  uint32 ebx;
  uint32 edx;
  uint32 ecx;
  uint32 eax;
  uint32 edi;
  uint32 esi;
  uint32 flags;
}
vwin32_regs;

typedef struct
{
  disk_io_t *disk_iop;
  uint32 filler0;
  sector_xfer_magic0_t magic;
  drive_number_0t drive_number_0based;
  uint32 filler1;
  uint32 filler2;
  uint32 success_flag;
}
xfer_sector_t;

typedef struct
{
  disk_io_t *disk_iop;
  drive_number_1t drive_number_1based;
  sector_xfer_magic0_t magic0;
  sector_xfer_magic1_t magic1;
  uint32 filler0;
  extended_op_t op;
  uint32 success_flag;
}
extended_sector_op_t;

#define VWIN32_VXD_NAME "\\\\.\\vwin32"

typedef enum { WIN32_UNKNOWN, WIN32_95, WIN32_NT } which_win32_t;

enum { BYTES_PER_SECTOR = 512, CDROM_BYTES_PER_SECTOR = 2048 };
#define MAX_BYTES_PER_SECTOR (MAX (BYTES_PER_SECTOR, CDROM_BYTES_PER_SECTOR))

#endif
