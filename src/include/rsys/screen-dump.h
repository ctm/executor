#if !defined (__rsys_screen_dump_h__)
#  define __rsys_screen_dump_h__

struct header : GuestStruct {
    GUEST< int16> byte_order;
    GUEST< int16> magic_number;
    GUEST< int32> ifd_offset;
};

struct directory_entry : GuestStruct {
    GUEST< int16> tag;
    GUEST< int16> type;
    GUEST< int32> count;
    GUEST< int32> value_offset;
};

struct ifd : GuestStruct {
    GUEST< int16> count;
    GUEST< struct directory_entry[1]> entries;
};

#endif /* !defined (__rsys_screen_dump_h__) */
