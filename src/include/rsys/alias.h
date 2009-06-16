#if !defined (__rsys_alias_h__)
#  define __rsys_alias_h__

typedef unsigned char Str27[28];

typedef struct
{
  OSType type PACKED;
  INTEGER length PACKED;
  INTEGER usually_2 PACKED;
  INTEGER usually_0 PACKED;
  Str27 volumeName LPACKED;
  LONGINT ioVCrDate PACKED;
  INTEGER ioVSigWord PACKED;
  INTEGER zero_or_one PACKED;
  LONGINT zero_or_neg_one PACKED;
  Str63 fileName LPACKED;
  LONGINT ioDirID PACKED; /* -1 full */
  LONGINT ioFlCrDat PACKED;
  OSType type_info PACKED;
  OSType creator PACKED;
  INTEGER mystery_words[10] PACKED;
}
alias_head_t;

typedef struct /* 0x0000 */
{
  INTEGER parent_length PACKED;
  unsigned char parent_bytes[1] LPACKED;
}
alias_parent_t;

typedef struct /* 0x0001 */
{
  INTEGER eight PACKED;
  LONGINT mystery_longs[2] PACKED;
}
alias_unknown_000100_t;

typedef struct /* 0x0002 */
{
  INTEGER fullpath_length PACKED;
  unsigned char fullpath_bytes[1] LPACKED;
}
alias_fullpath_t;

typedef struct /* 0x0009 */
{
  INTEGER length PACKED;
  INTEGER weird_info[12] PACKED;
  Str32 zone LPACKED;
  Str31 server LPACKED;
  Str27 volumeName LPACKED;
  Str32 network_identity_owner_name LPACKED;
  char filler_zeros[18] LPACKED;
}
alias_tail_t;

typedef struct
{
  alias_head_t *headp PACKED;
  alias_parent_t *parentp PACKED;
  alias_unknown_000100_t *unknownp PACKED;
  alias_fullpath_t *fullpathp PACKED;
  alias_tail_t *tailp PACKED;
}
alias_parsed_t;


#endif /* !defined (__rsys_alias_h__) */
