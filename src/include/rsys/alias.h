#if !defined (__rsys_alias_h__)
#  define __rsys_alias_h__

namespace Executor {
typedef unsigned char Str27[28];

struct alias_head_t : GuestStruct {
    GUEST< OSType> type;
    GUEST< INTEGER> length;
    GUEST< INTEGER> usually_2;
    GUEST< INTEGER> usually_0;
    GUEST< Str27> volumeName;
    GUEST< LONGINT> ioVCrDate;
    GUEST< INTEGER> ioVSigWord;
    GUEST< INTEGER> zero_or_one;
    GUEST< LONGINT> zero_or_neg_one;
    GUEST< Str63> fileName;
    GUEST< LONGINT> ioDirID;    /* -1 full */
    GUEST< LONGINT> ioFlCrDat;
    GUEST< OSType> type_info;
    GUEST< OSType> creator;
    GUEST< INTEGER[10]> mystery_words;
};

struct alias_parent_t : GuestStruct {
    GUEST< INTEGER> parent_length;
    GUEST< unsigned char[1]> parent_bytes;
};

struct alias_unknown_000100_t : GuestStruct {
    GUEST< INTEGER> eight;
    GUEST< LONGINT[2]> mystery_longs;
};

struct alias_fullpath_t : GuestStruct {
    GUEST< INTEGER> fullpath_length;
    GUEST< unsigned char[1]> fullpath_bytes;
};

struct alias_tail_t : GuestStruct {
    GUEST< INTEGER> length;
    GUEST< INTEGER[12]> weird_info;
    GUEST< Str32> zone;
    GUEST< Str31> server;
    GUEST< Str27> volumeName;
    GUEST< Str32> network_identity_owner_name;
    GUEST< char[18]> filler_zeros;
};

typedef alias_head_t *alias_head_ptr;
typedef alias_parent_t *alias_parent_ptr;
typedef alias_unknown_000100_t *alias_unknown_000100_ptr;
typedef alias_fullpath_t *alias_fullpath_ptr;
typedef alias_tail_t *alias_tail_ptr;

struct alias_parsed_t : GuestStruct {
    GUEST< alias_head_ptr> headp;
    GUEST< alias_parent_ptr> parentp;
    GUEST< alias_unknown_000100_ptr> unknownp;
    GUEST< alias_fullpath_ptr> fullpathp;
    GUEST< alias_tail_ptr> tailp;
};
}

#endif /* !defined (__rsys_alias_h__) */
