/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_AE_desc[] =
  "$Id: AE_desc.c 88 2005-05-25 03:59:37Z ctm $";
#endif

#include "rsys/common.h"

#include "AppleEvents.h"
#include "MemoryMgr.h"

#include "rsys/mman.h"

#define LIST_CLASS_P(desc) \
  (   DESC_TYPE_X (desc) == CLC (typeAEList)	\
   || DESC_TYPE_X (desc) == CLC (typeAERecord)	\
   || DESC_TYPE_X (desc) == CLC (typeAppleEvent))
#define RECORD_CLASS_P(desc) \
  (   DESC_TYPE_X (desc) == CLC (typeAERecord)	\
   || DESC_TYPE_X (desc) == CLC (typeAppleEvent))
#define APPLE_EVENT_CLASS_P(desc) \
  (DESC_TYPE_X (desc) == CLC (typeAppleEvent))

/* #### internal */

typedef struct
{
  DescType type			PACKED;
  uint32 size			PACKED;
  char data[0]			PACKED;
} inline_desc_t;

typedef struct
{
  int32 key			PACKED;
  DescType type			PACKED;
  uint32 size			PACKED;
  char data[0]			PACKED;
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
  char data[0]			PACKED;
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
  char pad_1[26]		PACKED;
  
  AEEventClass event_class	PACKED;
  AEEventID event_id		PACKED;
  
  /* #### takes on various values, no idea */
  uint32 unknown_3		PACKED;
  
  /* beginning of target inline descriptor; `target->size' determines
     target's actual size */
  inline_desc_t target		PACKED;
  
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

static OSErr
get_subdesc_info (Handle aggr_desc_h, subdesc_info_t *info,
		  boolean_t attribute_p)
{
  if (ATTRIBUTE_COUNT (aggr_desc_h) == typeAEList
      || ATTRIBUTE_COUNT (aggr_desc_h) == typeAERecord)
    {
      if (attribute_p)
	AE_RETURN_ERROR (errAEWrongDataType);
      
      info->key_p = (ATTRIBUTE_COUNT (aggr_desc_h) == typeAERecord);
      
      info->count = PARAM_COUNT (aggr_desc_h);
      info->count_p = &PARAM_COUNT_X (aggr_desc_h);
      
      info->base_offset = 0x18;
    }
  else
    {
      info->key_p = TRUE;
      
      if (attribute_p)
	{
	  inline_key_desc_t *inline_target_desc;
	  char *aggr_desc_p;
	  
	  info->count = ATTRIBUTE_COUNT (aggr_desc_h);
	  info->count_p = &ATTRIBUTE_COUNT_X (aggr_desc_h);
	  
	  aggr_desc_p = (char *) STARH (aggr_desc_h);
	  
	  inline_target_desc
	    = (inline_key_desc_t *) (aggr_desc_p
				     + offsetof (ae_header_t, target));
	  
	  info->base_offset = (offsetof (ae_header_t, target)
			       /* type, key, size */
			       + 12
			       + CL (inline_target_desc->size)
			       /* two unknown longs */
			       + 8);
	}
      else
	{
	  info->count = PARAM_COUNT (aggr_desc_h);
	  info->count_p = &PARAM_COUNT_X (aggr_desc_h);
	  info->base_offset = PARAM_OFFSET (aggr_desc_h);
	}
    }
  
  if (info->key_p)
    info->inline_desc_header_size = 12;
  else
    info->inline_desc_header_size = 8;
  
  AE_RETURN_ERROR (noErr);
}

static void
desc_offset (Handle aggr_desc_h, int index, subdesc_info_t *info,
	     int *offset_return)
{
  int count;
  char *t;
  
  t = (char *) STARH (aggr_desc_h) + info->base_offset;
  for (count = 1; count < index && count <= info->count; count ++)
    {
      inline_desc_t *desc;
      
      if (info->key_p)
	desc = (inline_desc_t *) (t + 4);
      else
	desc = (inline_desc_t *) t;
      
      t += (CL (desc->size)
	    /* inline key desc header size */ 
	    + info->inline_desc_header_size);
    }
  
  *offset_return = t - (char *) STARH (aggr_desc_h);
}


/* return a pointer to the storage for the subdesc at index `index' in
   either the attribute or paramter section, depending on
   `attribute_p'.

   if `create_p' is true, then we set the storage size to
   `*size_return' before returning the pointer' */

OSErr
aggr_desc_get_addr (Handle aggr_desc_h,
		    int index, boolean_t attribute_p,
		    char **addr_return,  int *size_return,
		    boolean_t create_p, boolean_t delete_p)
{
  subdesc_info_t info;
  int offset;
  char *aggr_desc_p;
  
  inline_desc_t *inline_desc;
  
  aggr_desc_p = (char *) STARH (aggr_desc_h);

  if (attribute_p
      && (ATTRIBUTE_COUNT (aggr_desc_h) == typeAEList
	  || ATTRIBUTE_COUNT (aggr_desc_h) == typeAERecord))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  get_subdesc_info (aggr_desc_h, &info, attribute_p);
  
  if (! index)
    index = info.count + 1;
  
  if (index < 1
      || (! create_p && index > info.count)
      || (create_p && index > info.count + 1))
    {
      AE_RETURN_ERROR (errAEIllegalIndex);
    }
  
  if (! attribute_p
      && ATTRIBUTE_COUNT (aggr_desc_h) != typeAEList
      && ATTRIBUTE_COUNT (aggr_desc_h) != typeAERecord
      && index == 1
      && ! info.count)
    {
      int aggr_desc_size;
      
      aggr_desc_size = GetHandleSize (aggr_desc_h);
      SetHandleSize (aggr_desc_h, aggr_desc_size + 2);

      {
	int16 *t;
	
	aggr_desc_p = (char *) STARH (aggr_desc_h);
	t = (int16 *) (aggr_desc_p + aggr_desc_size);
	/* ';;' */
	t[0] = CWC (0x3b3b);
      }
    }
  
  /* compute the offset of the desc we are interested in */
  desc_offset (aggr_desc_h, index, &info,
	       &offset);
  
  if (info.key_p)
    inline_desc = (inline_desc_t *) (aggr_desc_p + offset + 4);
  else
    inline_desc = (inline_desc_t *) (aggr_desc_p + offset);
  
  if (create_p)
    {
      int diff, aggr_desc_size;
      int old_size, new_size;
      
      aggr_desc_size = GetHandleSize (aggr_desc_h);

      if (index == info.count + 1)
	old_size = 0;
      else
	old_size = CL (inline_desc->size) + info.inline_desc_header_size;
      
      if (delete_p)
	new_size = 0;
      else
	/* #### */
	new_size = *size_return + info.inline_desc_header_size;
      
      /* resize it */
      if (old_size < new_size)
	{
	  /* grow */
	  diff = new_size - old_size;
	  
	  SetHandleSize (aggr_desc_h, aggr_desc_size + diff);
	  if (MemErr != CWC (noErr))
	    AE_RETURN_ERROR (CW (MemErr));
	  aggr_desc_p = (char *) STARH (aggr_desc_h);
	  if (aggr_desc_size < offset + old_size)
	    abort ();
	  memmove (aggr_desc_p + offset + old_size,
		   aggr_desc_p + offset + new_size,
		   aggr_desc_size - offset - old_size);
	}
      else if (old_size > new_size)
	{
	  diff = old_size - new_size;

	  if (aggr_desc_size < offset + old_size)
	    abort ();
	  memmove (aggr_desc_p + offset + old_size,
		   aggr_desc_p + offset + new_size,
		   aggr_desc_size - offset - old_size);
	  SetHandleSize (aggr_desc_h, aggr_desc_size - diff);
	  if (MemErr != CWC (noErr))
	    AE_RETURN_ERROR (CW (MemErr));
	  aggr_desc_p = (char *) STARH (aggr_desc_h);
	}
      memset (aggr_desc_p + offset, '\000', new_size);
      
      /* recompute the subdesc_info since the `aggr_desc_h' handle may
         have been relocated, which could invalidate `info.count_p' */
      get_subdesc_info (aggr_desc_h, &info, attribute_p);
      
      if (info.key_p)
	inline_desc = (inline_desc_t *) (aggr_desc_p + offset + 4);
      else
	inline_desc = (inline_desc_t *) (aggr_desc_p + offset);
      
      if (attribute_p)
	{
	  PARAM_OFFSET_X (aggr_desc_h)
	    = CL (PARAM_OFFSET (aggr_desc_h) - old_size + new_size);
	}
      
      if (! delete_p)
	{
	  if (index == info.count + 1)
	    {
	      info.count ++;
	      *(info.count_p) = CL (info.count);
	    }
	  
	  inline_desc->size = CL (*size_return);
	}
      else
	{
	  info.count --;
	  *(info.count_p) = CL (info.count);
	}
    }
  
  if (! delete_p)
    {  
      *addr_return = (aggr_desc_p + offset);
      *size_return = CL (inline_desc->size);
    }
  
  AE_RETURN_ERROR (noErr);
}

static boolean_t
find_key_index (Handle aggr_desc_h, int32 keyword, boolean_t attribute_p,
		int *index_return)
{
  subdesc_info_t info;
  int count;
  char *t;
  
  get_subdesc_info (aggr_desc_h, &info, attribute_p);
  
  t = (char *) STARH (aggr_desc_h) + info.base_offset;
  for (count = 1; count <= info.count; count ++)
    {
      inline_key_desc_t *inline_key_desc;
      
      inline_key_desc = (inline_key_desc_t *) t;
      
      if (CL (inline_key_desc->key) == keyword)
	{
	  *index_return = count;
	  return TRUE;
	}
      
      t += (CL (inline_key_desc->size)
	    /* inline key desc header size */ 
	    + 12);
    }
  
  *index_return = info.count + 1;
  return FALSE;
}


static boolean_t
aggr_delete_index (Handle aggr_handle,
		   boolean_t attr_p,
		   int index)
{
  OSErr err;
  char *dummy_addr;
  int dummy_size;
  
  err = aggr_desc_get_addr (aggr_handle, index, attr_p,
			    &dummy_addr, &dummy_size, FALSE,
			    TRUE);
  
  if (err != noErr)
    return FALSE;
  else
    return TRUE;
}

static boolean_t
aggr_put_nth_desc (Handle aggr_handle,
		   int index, OSErr *out_failcode,
		   descriptor_t *in_desc)
{
  Handle in_desc_data;
  DescType in_desc_type;
  inline_desc_t *inline_desc;
  int size;
  OSErr err;
  
  in_desc_data = DESC_DATA (in_desc);
  in_desc_type = DESC_TYPE (in_desc);
  size = GetHandleSize (in_desc_data);
  
  err = aggr_desc_get_addr (aggr_handle, index, FALSE,
			    (char **) &inline_desc, &size, TRUE, FALSE);
  if (err != noErr)
    {
      *out_failcode = err;
      return FALSE;
    }
  
  inline_desc->type = CL (in_desc_type);
  memcpy (inline_desc->data, STARH (in_desc_data), size);
  
  *out_failcode = noErr;
  return TRUE;
}

static boolean_t
aggr_get_nth_desc (Handle aggr_handle,
		   int index,
		   int32 *out_keyword,
		   descriptor_t *out_desc)
{
  char *addr;
  int size;
  OSErr err;
  
  err = aggr_desc_get_addr (aggr_handle, index, FALSE,
			    &addr, &size, FALSE, FALSE);
  if (err != noErr)
    return FALSE;
  
  if (ATTRIBUTE_COUNT (aggr_handle) == typeAEList)
    {
      inline_desc_t *inline_out_desc;

      inline_out_desc = (inline_desc_t *) addr;
      
      if (out_keyword)
	*out_keyword = CLC (typeWildCard);
      LOCK_HANDLE_EXCURSION_1
	(aggr_handle,
	 {
	   err = AECreateDesc (CL (inline_out_desc->type),
			       (Ptr) inline_out_desc->data, size,
			       out_desc);
	 });
      if (err != noErr)
	return FALSE;
    }
  else
    {
      inline_key_desc_t *inline_out_desc;
      
      inline_out_desc = (inline_key_desc_t *) addr;
      
      if (out_keyword)
	*out_keyword = inline_out_desc->key;
	LOCK_HANDLE_EXCURSION_1
	  (aggr_handle,
	   {
	     err = AECreateDesc (CL (inline_out_desc->type),
				 (Ptr) inline_out_desc->data, size,
				 out_desc);
	   });
      if (err != noErr)
	return FALSE;
    }
  
  return TRUE;
}

static boolean_t
aggr_put_key_desc (Handle aggr_handle,
		   int32 keyword,
		   boolean_t attr_p,
		   descriptor_t *in_desc)
{
  Handle in_desc_data;
  DescType in_desc_type;
  inline_key_desc_t *inline_key_desc;
  int size, index;
  OSErr err;

  /* #### test this */
  if (attr_p
      && (keyword == keyAddressAttr
	  || keyword == keyEventClassAttr
	  || keyword == keyEventIDAttr))
    {
      gui_fatal ("can't set builtin AE attribute");
    }
  
  in_desc_data = DESC_DATA (in_desc);
  in_desc_type = DESC_TYPE (in_desc);
  size = GetHandleSize (in_desc_data);
  
  find_key_index (aggr_handle, keyword, attr_p, &index);
  
  err = aggr_desc_get_addr (aggr_handle, index, attr_p,
			    (char **) &inline_key_desc, &size, TRUE, FALSE);
  if (err != noErr)
    return FALSE;
  
  inline_key_desc->key = CL (keyword);
  inline_key_desc->type = CL (in_desc_type);
  memcpy (inline_key_desc->data, STARH (in_desc_data), size);
  
  return TRUE;
}

static descriptor_t *
aggr_get_key_desc (Handle aggr_handle,
		   int32 keyword,
		   boolean_t attr_p,
		   descriptor_t *out_desc)
{
  ae_header_t *event_data;
  inline_key_desc_t *inline_key_desc;
  int size;
  int index;
  OSErr err;
  
  if (attr_p)
    {
      /* #### stinking apple special-case bullshit */
      
      event_data = (ae_header_t *) STARH (aggr_handle);
      
      switch (keyword)
	{
	case keyAddressAttr:
	  {
	    inline_desc_t *target;

	    target = &event_data->target;
	
	    LOCK_HANDLE_EXCURSION_1
	      (aggr_handle,
	       {
		 err = AECreateDesc (CL (target->type),
				     (Ptr) target->data, CL (target->size),
				     out_desc);
	       });
	    if (err != noErr)
	      return NULL;
	    return out_desc;
	  }
	  
	case keyEventClassAttr:
	  {
	    LOCK_HANDLE_EXCURSION_1
	      (aggr_handle,
	       {
		 err = AECreateDesc (typeType,
				     (Ptr) &event_data->event_class,
				     sizeof event_data->event_class,
				     out_desc);
	       });
	    if (err != noErr)
	      return NULL;
	    return out_desc;
	  }
	  
	case keyEventIDAttr:
	  {
	    LOCK_HANDLE_EXCURSION_1
	      (aggr_handle,
	       {
		 err = AECreateDesc (typeType,
				     (Ptr) &event_data->event_id,
				     sizeof event_data->event_id,
				     out_desc);
	       });
	    if (err != noErr)
	      return NULL;
	    return out_desc;
	  }
	}
    }
  
  if (! find_key_index (aggr_handle, keyword, attr_p, &index))
    return NULL;
  
  err = aggr_desc_get_addr (aggr_handle, index, attr_p,
			    (char **) &inline_key_desc, &size, FALSE, FALSE);
  if (err != noErr)
    return NULL;
  
  LOCK_HANDLE_EXCURSION_1
    (aggr_handle,
     {
       err = AECreateDesc (CL (inline_key_desc->type),
			   (Ptr) inline_key_desc->data, size,
			   out_desc);
     });
  if (err != noErr)
    return NULL;
  
  return out_desc;
}

static boolean_t
aggr_delete_key_desc (Handle aggr_handle,
		      int32 keyword,
		      boolean_t attr_p)
{
  char *dummy_addr;
  int dummy_size;
  int index;
  OSErr err;
  
  /* #### test this */
  if (attr_p
      && (keyword == keyAddressAttr
	  || keyword == keyEventClassAttr
	  || keyword == keyEventIDAttr))
    {
      gui_fatal ("can't delete builtin AE attribute");
    }
  
  if (! find_key_index (aggr_handle, keyword, attr_p, &index))
    return FALSE;
  
  err = aggr_desc_get_addr (aggr_handle, index, attr_p,
			    &dummy_addr, &dummy_size, FALSE,
			    TRUE);
  if (err != noErr)
    return FALSE;
  else
    return TRUE;
}

static void
ae_desc_to_ptr (descriptor_t *desc,
		Ptr data, uint32 max_size, int32 *size_out)
{
  uint32 copy_size, desc_size;
  Handle desc_data;
  
  desc_data = DESC_DATA (desc);
  desc_size = GetHandleSize (desc_data);
  
  copy_size = MIN (desc_size, max_size);
  
  memcpy (data, STARH (desc_data), copy_size);
  
  *size_out = CL (copy_size);
}

#if 0

void
dump_union_desc (union desc *foo, boolean_t key_pair_p)
{
  AEDesc *desc;
  uint32 type;
  char data[1024];
  uint32 size;
  
  if (key_pair_p)
    {
      uint32 key;

      key = KEY_DESC_KEYWORD (&foo[0].key);
      fprintf (stderr, "key `%c%c%c%c', ",
	       (key >> 24) & 0xFF,
	       (key >> 16) & 0xFF,
	       (key >>  8) & 0xFF,
	       (key >>  0) & 0xFF);
      desc = &KEY_DESC_CONTENT (&foo[0].key);
    }
  else
    {
      desc = &foo[0].std;
    }
      
  type = DESC_TYPE (desc);
  fprintf (stderr, "type `%c%c%c%c', ",
	       (type >> 24) & 0xFF,
	       (type >> 16) & 0xFF,
	       (type >>  8) & 0xFF,
	       (type >>  0) & 0xFF);
  
  ae_desc_to_ptr (desc, (Ptr) data, 1024, &size);
  size = CL (size);
  
  switch (type)
    {
    case typeType:
      fprintf (stderr, "data (%d) `%c%c%c%c'.",
	       size,
	       data[0], data[1], data[2], data[3]);
      break;
    default:
      {
	char *p;
	int i;
	
	fprintf (stderr, "data (%d)\n", size);
	for (p = data; p - data < size; p += 4)
	  {
	    fprintf (stderr, "  ");
	    for (i = 0; i < 4; i ++)
	      fprintf (stderr, "%02x", p[i]);
	    fprintf (stderr, " ");
	    for (i = 0; i < 4; i ++)
	      fprintf (stderr, "%c", p[i]);
	    fprintf (stderr, "\n");
	  }
	break;
      }
    }
  
  fprintf (stderr, "\n");
}

void
dump_desc (descriptor_t *desc)
{
  uint32 type;
  
  type = DESC_TYPE (desc);
  fprintf (stderr, "type `%c%c%c%c'\n",
	   (type >> 24) & 0xFF,
	   (type >> 16) & 0xFF,
	   (type >>  8) & 0xFF,
	   (type >>  0) & 0xFF);
  if (   type == typeAEList
      || type == typeAERecord
      || type == typeAppleEvent)
    {
      aggregate_descriptor_handle aggr_handle;
      aggregate_descriptor_t *aggr;
      int i, offset;
      
      aggr_handle = (aggregate_descriptor_handle) DESC_DATA (desc);
      aggr = STARH (aggr_handle);
      
      for (offset = i = 0; i < aggr->n_params; i ++, offset ++)
	dump_union_desc (&aggr->descs[offset], aggr->key_pair_p);
      for (i = 0; i < aggr->n_attrs; i ++, offset ++)
	dump_union_desc (&aggr->descs[offset], aggr->key_pair_p);
    }
}

#endif

/* apple events descriptor stuff */

P6 (PUBLIC pascal trap, OSErr, AECreateAppleEvent,
    AEEventClass, event_class, AEEventID, event_id,
    AEAddressDesc *, target,
    int16, return_id, int32, transaction_id,
    AppleEvent *, evt_out)
{
  Handle target_data;
  DescType target_type;
  int target_size, event_size;
  ae_header_t *event_data;
  
  target_data = DESC_DATA (target);
  target_type = DESC_TYPE (target);
  target_size = GetHandleSize (target_data);
  
  event_size = sizeof *event_data + target_size + 10;
  event_data = alloca (event_size);
  memset (event_data, '\000', event_size);
  
  event_data->param_offset = CL (event_size + 2);
  
  event_data->event_class = CL (event_class);
  event_data->event_id = CL (event_id);
  
  event_data->target.size = CL (target_size);
  event_data->target.type = CL (target_type);
  memcpy (&event_data->target.data[0], STARH (target_data), target_size);
  
  {
    int32 *t;
    
    t = (int32 *) ((char *) event_data + sizeof *event_data + target_size);
    
    t[0] = TICKX ("aevt");
    t[1] = CLC (0x00010001);
  }
  
  {
    int16 *t;
    
    t = (int16 *) ((char *) event_data + sizeof *event_data + target_size + 8);
    
    /* ';;' */
    t[0] = CWC (0x3b3b);
  }  
  
  AE_RETURN_ERROR (AECreateDesc (typeAppleEvent,
				 (Ptr) event_data, event_size,
				 evt_out));
}

/* generic descriptor functions */

P4 (PUBLIC pascal trap, OSErr, AECreateDesc,
    DescType, type,
    Ptr, data, Size, data_size,
    AEDesc *, desc_out)
{
  Handle h;
  
  h = NewHandle (data_size);
  if (h == NULL)
    AE_RETURN_ERROR (memFullErr);

  if (data)  
    memcpy (STARH (h), data, data_size);
  else
    {
      if (data_size != 0)
	warning_unexpected ("NULL data, data_size = %d", data_size);
      memset (STARH (h), 0, data_size);
    }
  
  DESC_TYPE_X (desc_out) = CL (type);
  DESC_DATA_X (desc_out) = RM (h);
  
  AE_RETURN_ERROR (noErr);
}

P1 (PUBLIC pascal trap, OSErr, AEDisposeDesc,
    AEDesc *, desc)
{
  DisposHandle (DESC_DATA (desc));
  
  AE_RETURN_ERROR (noErr);
}

P2 (PUBLIC pascal trap, OSErr, AEDuplicateDesc,
    AEDesc *, src, AEDesc *, dst)
{
  OSErr err;
  Handle src_data;
  
  src_data = DESC_DATA (src);
  
  LOCK_HANDLE_EXCURSION_1
    (src_data,
     {
       err = AECreateDesc (DESC_TYPE (src),
			   STARH (src_data), GetHandleSize (src_data),
			   dst);
     });
  AE_RETURN_ERROR (err);
}

/* descriptor functions for lists */

P4 (PUBLIC pascal trap, OSErr, AECreateList,
    Ptr, list_elt_prefix, Size, list_elt_prefix_size,
    Boolean, is_record_p, AEDescList *, list_out)
{
  list_header_t header;
  DescType type;
  
  /* #### */
  gui_assert (! list_elt_prefix_size);
  
  type = is_record_p ? typeAERecord : typeAEList;
  
  memset (&header, '\000', sizeof header);
  
  header.attribute_count = CL (type);
  header.param_offset = CLC (0x18);
  
  AE_RETURN_ERROR (AECreateDesc (type,
				 (Ptr) &header, sizeof header,
				 list_out));
}

P2 (PUBLIC pascal trap, OSErr, AECountItems,
    AEDescList *, list, int32 *, count_out)
{
  subdesc_info_t info;
  Handle aggr_desc_h;
  OSErr err;
  
  aggr_desc_h = DESC_DATA (list);
  err = get_subdesc_info (aggr_desc_h, &info, FALSE);
  if (err != noErr)
    AE_RETURN_ERROR (err);
  
  *count_out = CL (info.count);
  
  AE_RETURN_ERROR (noErr);
}

P5 (PUBLIC pascal trap, OSErr, AEGetNthDesc,
    AEDescList *, list, int32, index,
    DescType, desired_type, AEKeyword *, keyword_out,
    AEDesc *, desc_out)
{
  descriptor_t *desc = alloca (sizeof *desc);
  
  if (! LIST_CLASS_P (list))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  if (! aggr_get_nth_desc (DESC_DATA (list), index, keyword_out, desc))
    AE_RETURN_ERROR (errAEDescNotFound);
  
  AE_RETURN_ERROR (AECoerceDesc (desc, desired_type, desc_out));
}

P8 (PUBLIC pascal trap, OSErr, AEGetNthPtr,
    AEDescList *, list, int32, index,
    DescType, desired_type, AEKeyword *, keyword_out,
    DescType *, type_out,
    Ptr, data, int32, max_size, Size *, size_out)
{
  descriptor_t *desc = alloca (sizeof *desc);
  descriptor_t *coerced_desc = alloca (sizeof *coerced_desc);
  OSErr err;
  
  if (! LIST_CLASS_P (list))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  if (! aggr_get_nth_desc (DESC_DATA (list), index, keyword_out, desc))
    AE_RETURN_ERROR (errAEDescNotFound);
  
  err = AECoerceDesc (desc, desired_type, coerced_desc);
  if (err != noErr)
    AE_RETURN_ERROR (err);
  
  *type_out = DESC_TYPE_X (coerced_desc);
  ae_desc_to_ptr (desc,
		  data, max_size, size_out);
  AE_RETURN_ERROR (AEDisposeDesc (coerced_desc));
}

P3 (PUBLIC pascal trap, OSErr, AEPutDesc,
    AEDescList *, list, int32, index,
    AEDesc *, desc)
{
  OSErr retval = noErr;
  
  if (! LIST_CLASS_P (list))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  aggr_put_nth_desc (DESC_DATA (list), index, &retval, desc);
  AE_RETURN_ERROR (retval);
}

P5 (PUBLIC pascal trap, OSErr, AEPutPtr,
    AEDescList *, list, int32, index, DescType, type,
    Ptr, data, Size, data_size)
{
  descriptor_t *desc;
  OSErr err, retval;
  
  if (! LIST_CLASS_P (list))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  desc = alloca (sizeof *desc);
  err = AECreateDesc (type, data, data_size, desc);
  if (err != noErr)
    AE_RETURN_ERROR (err);
  
  aggr_put_nth_desc (DESC_DATA (list), index, &retval, desc);
  
  AE_RETURN_ERROR (retval);
}

P2 (PUBLIC pascal trap, OSErr, AEDeleteItem,
    AEDescList *, list, int32, index)
{
  if (! LIST_CLASS_P (list))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  if (aggr_delete_index (DESC_DATA (list), FALSE, index))
    AE_RETURN_ERROR (errAEIllegalIndex);
  else
    AE_RETURN_ERROR (noErr);
}

P4 (PUBLIC pascal trap, OSErr, AESizeOfNthItem,
    AEDescList *, list, int32, index,
    DescType *, type_out, Size *, size_out)
{
  descriptor_t *desc = alloca (sizeof *desc);
  
  if (! LIST_CLASS_P (list))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  if (! aggr_get_nth_desc (DESC_DATA (list), index, NULL, desc))
    AE_RETURN_ERROR (errAEIllegalIndex);
  
  *type_out = DESC_TYPE_X (desc);
  *size_out = CL (GetHandleSize ((Handle) DESC_DATA (desc)));

  AE_RETURN_ERROR (noErr);
}

/* descriptor functions for key pair records */

P4 (PUBLIC pascal trap, OSErr, AEGetKeyDesc,
    AERecord *, record, AEKeyword, keyword,
    DescType, desired_type, AEDesc *, desc_out)
{
  descriptor_t *desc;
  
  if (! RECORD_CLASS_P (record))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  desc = aggr_get_key_desc (DESC_DATA (record), keyword, FALSE,
			    alloca (sizeof *desc));
  if (desc == NULL)
    AE_RETURN_ERROR (errAEDescNotFound);
  
  AE_RETURN_ERROR (AECoerceDesc (desc, desired_type, desc_out));
}

P3 (PUBLIC pascal trap, OSErr, AEPutKeyDesc,
    AERecord *, record, AEKeyword, keyword,
    AEDesc *, desc)
{
  if (! RECORD_CLASS_P (record))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  if (aggr_put_key_desc (DESC_DATA (record), keyword, FALSE, desc))
    AE_RETURN_ERROR (noErr);
  else
    AE_RETURN_ERROR (memFullErr);
}

P7 (PUBLIC pascal trap, OSErr, AEGetKeyPtr,
    AERecord *, record, AEKeyword, keyword,
    DescType, desired_type, DescType *, type_out,
    Ptr, data, Size, max_size, Size *, size_out)
{
  descriptor_t *desc = alloca (sizeof *desc);
  descriptor_t *coerced_desc = alloca (sizeof *coerced_desc);
  OSErr err;
  
  if (! RECORD_CLASS_P (record))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  if (! aggr_get_key_desc (DESC_DATA (record), keyword, FALSE, desc))
    AE_RETURN_ERROR (errAEDescNotFound);
  
  err = AECoerceDesc (desc, desired_type, coerced_desc);
  if (err != noErr)
    AE_RETURN_ERROR (err);
  
  *type_out = DESC_TYPE_X (coerced_desc);
  ae_desc_to_ptr (desc,
		  data, max_size, size_out);
  AE_RETURN_ERROR (AEDisposeDesc (coerced_desc));
}

P5 (PUBLIC pascal trap, OSErr, AEPutKeyPtr,
    AERecord *, record, AEKeyword, keyword,
    DescType, type, Ptr, data, Size, data_size)
{
  descriptor_t *desc;
  OSErr err, retval;
  
  if (! RECORD_CLASS_P (record))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  desc = alloca (sizeof *desc);
  err = AECreateDesc (type, data, data_size, desc);
  if (err != noErr)
    AE_RETURN_ERROR (err);
  
  if (aggr_put_key_desc (DESC_DATA (record), keyword, FALSE, desc))
    retval = noErr;
  else
    retval = memFullErr;
  
  AE_RETURN_ERROR (retval);
}

P2 (PUBLIC pascal trap, OSErr, AEDeleteKeyDesc,
    AERecord *, record, AEKeyword, keyword)
{
  if (! RECORD_CLASS_P (record))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  if (! aggr_delete_key_desc (DESC_DATA (record), keyword, FALSE))
    AE_RETURN_ERROR (errAEDescNotFound);
  else
    AE_RETURN_ERROR (noErr);
}

P4 (PUBLIC pascal trap, OSErr, AESizeOfKeyDesc,
    AERecord *, record, AEKeyword, keyword,
    DescType *, type_out, Size *, size_out)
{
  descriptor_t *desc = alloca (sizeof *desc);
  
  if (! RECORD_CLASS_P (record))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  if (! aggr_get_key_desc (DESC_DATA (record), keyword, FALSE, desc))
    AE_RETURN_ERROR (errAEDescNotFound);
  
  *type_out = DESC_TYPE_X (desc);
  *size_out = CL (GetHandleSize ((Handle) DESC_DATA (desc)));

  AE_RETURN_ERROR (noErr);
}

/* descriptor functions for apple events */

P5 (PUBLIC pascal trap, OSErr, AEPutAttributePtr,
    AppleEvent *, evt, AEKeyword, keyword,
    DescType, type, Ptr, data, Size, data_size)
{
  descriptor_t *desc;
  OSErr err, retval;
  
  if (! APPLE_EVENT_CLASS_P (evt))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  desc = alloca (sizeof *desc);
  err = AECreateDesc (type, data, data_size, desc);
  if (err != noErr)
    AE_RETURN_ERROR (err);
  
  if (aggr_put_key_desc (DESC_DATA (evt), keyword, TRUE, desc))
    retval = noErr;
  else
    retval = memFullErr;
  
  AE_RETURN_ERROR (retval);
}

P3 (PUBLIC pascal trap, OSErr, AEPutAttributeDesc,
    AppleEvent *, evt, AEKeyword, keyword,
    AEDesc *, desc)
{
  if (! APPLE_EVENT_CLASS_P (evt))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  if (aggr_put_key_desc (DESC_DATA (evt), keyword, TRUE, desc))
    AE_RETURN_ERROR (noErr);
  else
    AE_RETURN_ERROR (memFullErr);
}

P4 (PUBLIC pascal trap, OSErr, AEGetAttributeDesc,
    AppleEvent *, evt, AEKeyword, keyword,
    DescType, desired_type, AEDesc *, desc_out)
{
  descriptor_t *desc;
  
  if (! APPLE_EVENT_CLASS_P (evt))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  desc = aggr_get_key_desc (DESC_DATA (evt), keyword, TRUE,
			    alloca (sizeof *desc));
  if (desc == NULL)
    AE_RETURN_ERROR (errAEDescNotFound);
  
  AE_RETURN_ERROR (AECoerceDesc (desc, desired_type, desc_out));
}

P7 (PUBLIC pascal trap, OSErr, AEGetAttributePtr,
    AppleEvent *, evt, AEKeyword, keyword,
    DescType, desired_type, DescType *, type_out,
    Ptr, data, Size, max_size, Size *, size_out)
{
  descriptor_t *desc;
  descriptor_t *coerced_desc = alloca (sizeof *coerced_desc);
  OSErr err;
  
  if (! APPLE_EVENT_CLASS_P (evt))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  desc = aggr_get_key_desc (DESC_DATA (evt), keyword, TRUE,
			    alloca (sizeof *desc));
  if (desc == NULL)
    AE_RETURN_ERROR (errAEDescNotFound);
  
  err = AECoerceDesc (desc, desired_type, coerced_desc);
  if (err != noErr)
    AE_RETURN_ERROR (err);
  
  *type_out = DESC_TYPE_X (coerced_desc);
  ae_desc_to_ptr (desc,
		  data, max_size, size_out);
  AE_RETURN_ERROR (AEDisposeDesc (coerced_desc));
}

P2 (PUBLIC pascal trap, OSErr, AEDeleteAttribute,
    AppleEvent *, evt, AEKeyword, keyword)
{
  if (! APPLE_EVENT_CLASS_P (evt))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  if (! aggr_delete_key_desc (DESC_DATA (evt), keyword, TRUE))
    AE_RETURN_ERROR (errAEDescNotFound);
  else
    AE_RETURN_ERROR (noErr);
}

P4 (PUBLIC pascal trap, OSErr, AESizeOfAttribute,
    AppleEvent *, evt, AEKeyword, keyword,
    DescType *, type_out, Size *, size_out)
{
  descriptor_t *desc;
  
  if (! APPLE_EVENT_CLASS_P (evt))
    AE_RETURN_ERROR (errAEWrongDataType);
  
  desc = aggr_get_key_desc (DESC_DATA (evt), keyword, TRUE,
			    alloca (sizeof *desc));
  if (desc == NULL)
    AE_RETURN_ERROR (errAEDescNotFound);
  
  *type_out = DESC_TYPE_X (desc);
  *size_out = CL (GetHandleSize ((Handle) DESC_DATA (desc)));
  
  AE_RETURN_ERROR (noErr);
}

/* parameter functions; these simply call the associated `record'
   functions */

/* they don't exist, since the param and record accessor traps use the
   same trap selectors */
   
