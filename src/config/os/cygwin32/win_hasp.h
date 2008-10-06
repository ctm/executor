#if !defined (_win_hasp_h_)
#define _win_hasp_h_

enum { HASP_TYPE = 0x68617370 };  /* 'hasp' */
enum { HASP_MAGIC = 0xb32b932a }; /* random */

typedef struct
{
  uint32 type; /* see above */
  uint32 magic; /* see above */
  uint32 size; /* total length of parameter block */
  uint32 Service; /* see HASP Programmer's Guide for these parameters */
  uint32 SeedCode;
  uint32 LptNum;
  uint32 Password1;
  uint32 Password2;
  uint32 Par1;
  uint32 Par2;
  uint32 Par3;
  uint32 Par4;
}
hasp_param_block;

enum
{
  HASP_NO_ERROR = 0,
  HASP_BAD_MAGIC_ERROR = -1,
  HASP_BAD_LENGTH_ERROR = -2,
  HASP_NO_LIBRARY_ERROR = -3,
};

enum
{
  IsHasp = 1, /* Basic HASP API */
  HaspCode = 2,
  HaspStatus = 5,

  ReadWord = 3, /* Memo HASP */
  WriteWord = 4,
  HaspID = 6,
  ReadBlock = 50,
  WriteBlock = 51,
};

enum { HASP_SEARCH_ALL = 0 };

enum { DONGLE_GESTALT = 0xb7d20e84 };

#endif
