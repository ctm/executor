#if !defined(__rsys_apple_events_h__)
#define __rsys_apple_events_h__
namespace Executor
{
/* #### internal */

struct inline_desc_t
{
    GUEST_STRUCT;
    GUEST<DescType> type;
    GUEST<uint32_t> size;
    GUEST<char> data[0];
};

struct inline_key_desc_t
{
    GUEST_STRUCT;
    GUEST<int32_t> key;
    GUEST<DescType> type;
    GUEST<uint32_t> size;
    GUEST<char> data[0];
};

/* #### always zero (?) */

/* #### contains an applzone address */

/* contains a tick that identifies the object, either `list' or
     `reco'; for an apple event, this fiend contains the offset to the
     parameter section */

/* ### always zero (?) */

/* offset: 0x18 */
typedef struct list_header
{
    GUEST_STRUCT;
    GUEST<uint32_t> unknown_1;
    GUEST<uint32_t> unknown_2;
    GUEST<uint32_t> param_offset;
    GUEST<uint32_t> attribute_count;
    GUEST<uint32_t> param_count;
    GUEST<int32_t> unknown_3;
    GUEST<char> data[0];
} list_header_t;

typedef list_header_t *list_header_ptr;

typedef GUEST<list_header_ptr> *list_header_h;

#define PARAM_OFFSET_X(aggr_desc_h) \
    (HxX((list_header_h)aggr_desc_h, param_offset))
#define PARAM_COUNT_X(aggr_desc_h) \
    (HxX((list_header_h)aggr_desc_h, param_count))
#define ATTRIBUTE_COUNT_X(aggr_desc_h) \
    (HxX((list_header_h)aggr_desc_h, attribute_count))

#define PARAM_COUNT(aggr_desc_h) (CL(PARAM_COUNT_X(aggr_desc_h)))
#define PARAM_OFFSET(aggr_desc_h) (CL(PARAM_OFFSET_X(aggr_desc_h)))
#define ATTRIBUTE_COUNT(aggr_desc_h) (CL(ATTRIBUTE_COUNT_X(aggr_desc_h)))

/* #### always zero (?) */

/* #### contains unknown values */

/* #### zero pad, use unknown */

/* #### takes on various values, no idea */

/* beginning of target inline descriptor; `target->size' determines
     target's actual size */

/* for show */
/* #### contains `aevt' tick */

/* contains 0x00010001 */

/* marker containing tick `;;;;' */

typedef struct ae_header
{
    GUEST_STRUCT;
    GUEST<uint32_t> unknown_1;
    GUEST<uint32_t> unknown_2;
    GUEST<uint32_t> param_offset;
    GUEST<uint32_t> attribute_count;
    GUEST<uint32_t> param_count;
    GUEST<char[26]> pad_1;
    GUEST<AEEventClass> event_class;
    GUEST<AEEventID> event_id;
    GUEST<uint32_t> unknown_3;
    GUEST<inline_desc_t> target;
#if 0
    GUEST< uint32_t> unknown_4;
    GUEST< uint32_t> unknown_5;
    GUEST< char[...]> attribute_data;
    GUEST< uint32_t> unknown_6;
    GUEST< char[...]> param_data;
#endif
} ae_header_t;

typedef struct subdesc_info
{
    int count;
    int base_offset;
    GUEST<uint32_t> *count_p;
    bool key_p;
    int inline_desc_header_size;
} subdesc_info_t;
}
#endif /* !defined (__rsys_apple_events_h__) */
