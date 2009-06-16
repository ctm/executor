#if !defined (__rsys_apple_events_h__)
#  define __rsys_apple_events_h__

/* #### internal */

typedef struct
{
  DescType type			PACKED;
  uint32 size			PACKED;
  char data[0]			LPACKED;
} inline_desc_t;

typedef struct
{
  int32 key			PACKED;
  DescType type			PACKED;
  uint32 size			PACKED;
  char data[0]			LPACKED;
} inline_key_desc_t;

typedef struct list_header
{
  /* #### always zero (?) */
  uint32 unknown_1		PACKED;
  
  /* #### contains an applzone address */
  uint32 unknown_2		PACKED;
  
  uint32 param_offset		PACKED;
  
  /* contains a tick that identifies the object, either `list' or
     `reco'; for an apple event, this fiend contains the offset to the
     parameter section */
  uint32 attribute_count	PACKED;
  
  uint32 param_count		PACKED;
  
  /* ### always zero (?) */
  int32 unknown_3		PACKED;
  
   /* offset: 0x18 */
  char data[0]			LPACKED;
} list_header_t;

typedef struct { list_header_t *p PACKED_P; } hidden_list_header_ptr;
typedef hidden_list_header_ptr *list_header_h;

#define PARAM_OFFSET_X(aggr_desc_h)					\
  (HxX ((list_header_h) aggr_desc_h, param_offset))
#define PARAM_COUNT_X(aggr_desc_h)					\
  (HxX ((list_header_h) aggr_desc_h, param_count))
#define ATTRIBUTE_COUNT_X(aggr_desc_h)					\
  (HxX ((list_header_h) aggr_desc_h, attribute_count))

#define PARAM_COUNT(aggr_desc_h)	(CL (PARAM_COUNT_X (aggr_desc_h)))
#define PARAM_OFFSET(aggr_desc_h)	(CL (PARAM_OFFSET_X (aggr_desc_h)))
#define ATTRIBUTE_COUNT(aggr_desc_h)	(CL (ATTRIBUTE_COUNT_X (aggr_desc_h)))

typedef struct ae_header
{
  /* #### always zero (?) */
  uint32 unknown_1		PACKED;
  
  /* #### contains unknown values */
  uint32 unknown_2		PACKED;
  
  uint32 param_offset		PACKED;
  
  uint32 attribute_count	PACKED;
  uint32 param_count		PACKED;
  
  /* #### zero pad, use unknown */
  char pad_1[26]		LPACKED;
  
  AEEventClass event_class	PACKED;
  AEEventID event_id		PACKED;
  
  /* #### takes on various values, no idea */
  uint32 unknown_3		PACKED;
  
  /* beginning of target inline descriptor; `target->size' determines
     target's actual size */
  inline_desc_t target		LPACKED;
  
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
