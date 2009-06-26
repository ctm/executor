#if !defined (__rsys_apple_events_h__)
#  define __rsys_apple_events_h__

/* #### internal */

typedef struct PACKED
{
  DescType type;
  uint32 size;
  char data[0];
} inline_desc_t;

typedef struct PACKED
{
  int32 key;
  DescType type;
  uint32 size;
  char data[0];
} inline_key_desc_t;

typedef struct PACKED list_header
{
  /* #### always zero (?) */
  uint32 unknown_1;
  
  /* #### contains an applzone address */
  uint32 unknown_2;
  
  uint32 param_offset;
  
  /* contains a tick that identifies the object, either `list' or
     `reco'; for an apple event, this fiend contains the offset to the
     parameter section */
  uint32 attribute_count;
  
  uint32 param_count;
  
  /* ### always zero (?) */
  int32 unknown_3;
  
   /* offset: 0x18 */
  char data[0];
} list_header_t;

typedef list_header_t *list_header_ptr;
MAKE_HIDDEN(list_header_ptr);
typedef HIDDEN_list_header_ptr *list_header_h;

#define PARAM_OFFSET_X(aggr_desc_h)					\
  (HxX ((list_header_h) aggr_desc_h, param_offset))
#define PARAM_COUNT_X(aggr_desc_h)					\
  (HxX ((list_header_h) aggr_desc_h, param_count))
#define ATTRIBUTE_COUNT_X(aggr_desc_h)					\
  (HxX ((list_header_h) aggr_desc_h, attribute_count))

#define PARAM_COUNT(aggr_desc_h)	(CL (PARAM_COUNT_X (aggr_desc_h)))
#define PARAM_OFFSET(aggr_desc_h)	(CL (PARAM_OFFSET_X (aggr_desc_h)))
#define ATTRIBUTE_COUNT(aggr_desc_h)	(CL (ATTRIBUTE_COUNT_X (aggr_desc_h)))

typedef struct PACKED ae_header
{
  /* #### always zero (?) */
  uint32 unknown_1;
  
  /* #### contains unknown values */
  uint32 unknown_2;
  
  uint32 param_offset;
  
  uint32 attribute_count;
  uint32 param_count;
  
  /* #### zero pad, use unknown */
  char pad_1[26];
  
  AEEventClass event_class;
  AEEventID event_id;
  
  /* #### takes on various values, no idea */
  uint32 unknown_3;
  
  /* beginning of target inline descriptor; `target->size' determines
     target's actual size */
  inline_desc_t target;
  
#if 0
  /* for show */
  /* #### contains `aevt' tick */
  uint32 unknown_4;
  
  /* contains 0x00010001 */
  uint32 unknown_5;
  
  char attribute_data[...];
  
  /* marker containing tick `;;;;' */
  uint32 unknown_6;
  
  char param_data[...];
#endif
} ae_header_t;

typedef struct subdesc_info
{
  int count;
  int base_offset;
  uint32 *count_p;
  boolean_t key_p;
  int inline_desc_header_size;
} subdesc_info_t;

#endif /* !defined (__rsys_apple_events_h__) */
