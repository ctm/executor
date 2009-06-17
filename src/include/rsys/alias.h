#if !defined (__rsys_alias_h__)
#  define __rsys_alias_h__

typedef unsigned char Str27[28];

typedef struct PACKED
{
  OSType type;
  INTEGER length;
  INTEGER usually_2;
  INTEGER usually_0;
  Str27 volumeName;
  LONGINT ioVCrDate;
  INTEGER ioVSigWord;
  INTEGER zero_or_one;
  LONGINT zero_or_neg_one;
  Str63 fileName;
  LONGINT ioDirID; /* -1 full */
  LONGINT ioFlCrDat;
  OSType type_info;
  OSType creator;
  INTEGER mystery_words[10];
}
alias_head_t;

typedef struct PACKED /* 0x0000 */
{
  INTEGER parent_length;
  unsigned char parent_bytes[1];
}
alias_parent_t;

typedef struct PACKED /* 0x0001 */
{
  INTEGER eight;
  LONGINT mystery_longs[2];
}
alias_unknown_000100_t;

typedef struct PACKED /* 0x0002 */
{
  INTEGER fullpath_length;
  unsigned char fullpath_bytes[1];
}
alias_fullpath_t;

typedef struct PACKED /* 0x0009 */
{
  INTEGER length;
  INTEGER weird_info[12];
  Str32 zone;
  Str31 server;
  Str27 volumeName;
  Str32 network_identity_owner_name;
  char filler_zeros[18];
}
alias_tail_t;

typedef struct PACKED
{
  alias_head_t *headp PACKED_P;
  alias_parent_t *parentp PACKED_P;
  alias_unknown_000100_t *unknownp PACKED_P;
  alias_fullpath_t *fullpathp PACKED_P;
  alias_tail_t *tailp PACKED_P;
}
alias_parsed_t;


#endif /* !defined (__rsys_alias_h__) */
