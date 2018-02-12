/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "AppleEvents.h"
#include "MemoryMgr.h"

#include "rsys/mman.h"
#include "rsys/apple_events.h"
#include <algorithm>

using namespace Executor;

#define LIST_CLASS_P(desc)                     \
    (DESC_TYPE_X(desc) == CLC(typeAEList)      \
     || DESC_TYPE_X(desc) == CLC(typeAERecord) \
     || DESC_TYPE_X(desc) == CLC(typeAppleEvent))
#define RECORD_CLASS_P(desc)                \
    (DESC_TYPE_X(desc) == CLC(typeAERecord) \
     || DESC_TYPE_X(desc) == CLC(typeAppleEvent))
#define APPLE_EVENT_CLASS_P(desc) \
    (DESC_TYPE_X(desc) == CLC(typeAppleEvent))

static OSErr
get_subdesc_info(Handle aggr_desc_h, subdesc_info_t *info,
                 bool attribute_p)
{
    if(ATTRIBUTE_COUNT(aggr_desc_h) == typeAEList
       || ATTRIBUTE_COUNT(aggr_desc_h) == typeAERecord)
    {
        if(attribute_p)
            AE_RETURN_ERROR(errAEWrongDataType);

        info->key_p = (ATTRIBUTE_COUNT(aggr_desc_h) == typeAERecord);

        info->count = PARAM_COUNT(aggr_desc_h);
        info->count_p = &PARAM_COUNT_X(aggr_desc_h);

        info->base_offset = 0x18;
    }
    else
    {
        info->key_p = true;

        if(attribute_p)
        {
            inline_key_desc_t *inline_target_desc;
            char *aggr_desc_p;

            info->count = ATTRIBUTE_COUNT(aggr_desc_h);
            info->count_p = &ATTRIBUTE_COUNT_X(aggr_desc_h);

            aggr_desc_p = (char *)STARH(aggr_desc_h);

            inline_target_desc
                = (inline_key_desc_t *)(aggr_desc_p
                                        + offsetof(ae_header_t, target));

            info->base_offset = (offsetof(ae_header_t, target)
                                 /* type, key, size */
                                 + 12
                                 + CL(inline_target_desc->size)
                                 /* two unknown longs */
                                 + 8);
        }
        else
        {
            info->count = PARAM_COUNT(aggr_desc_h);
            info->count_p = &PARAM_COUNT_X(aggr_desc_h);
            info->base_offset = PARAM_OFFSET(aggr_desc_h);
        }
    }

    if(info->key_p)
        info->inline_desc_header_size = 12;
    else
        info->inline_desc_header_size = 8;

    AE_RETURN_ERROR(noErr);
}

static void
desc_offset(Handle aggr_desc_h, int index, subdesc_info_t *info,
            int *offset_return)
{
    int count;
    char *t;

    t = (char *)STARH(aggr_desc_h) + info->base_offset;
    for(count = 1; count < index && count <= info->count; count++)
    {
        inline_desc_t *desc;

        if(info->key_p)
            desc = (inline_desc_t *)(t + 4);
        else
            desc = (inline_desc_t *)t;

        t += (CL(desc->size)
              /* inline key desc header size */
              + info->inline_desc_header_size);
    }

    *offset_return = t - (char *)STARH(aggr_desc_h);
}

/* return a pointer to the storage for the subdesc at index `index' in
   either the attribute or paramter section, depending on
   `attribute_p'.

   if `create_p' is true, then we set the storage size to
   `*size_return' before returning the pointer' */

OSErr aggr_desc_get_addr(Handle aggr_desc_h,
                         int index, bool attribute_p,
                         char **addr_return, int *size_return,
                         bool create_p, bool delete_p)
{
    subdesc_info_t info;
    int offset;
    char *aggr_desc_p;

    inline_desc_t *inline_desc;

    aggr_desc_p = (char *)STARH(aggr_desc_h);

    if(attribute_p
       && (ATTRIBUTE_COUNT(aggr_desc_h) == typeAEList
           || ATTRIBUTE_COUNT(aggr_desc_h) == typeAERecord))
        AE_RETURN_ERROR(errAEWrongDataType);

    get_subdesc_info(aggr_desc_h, &info, attribute_p);

    if(!index)
        index = info.count + 1;

    if(index < 1
       || (!create_p && index > info.count)
       || (create_p && index > info.count + 1))
    {
        AE_RETURN_ERROR(errAEIllegalIndex);
    }

    if(!attribute_p
       && ATTRIBUTE_COUNT(aggr_desc_h) != typeAEList
       && ATTRIBUTE_COUNT(aggr_desc_h) != typeAERecord
       && index == 1
       && !info.count)
    {
        int aggr_desc_size;

        aggr_desc_size = GetHandleSize(aggr_desc_h);
        SetHandleSize(aggr_desc_h, aggr_desc_size + 2);

        {
            GUEST<int16_t> *t;

            aggr_desc_p = (char *)STARH(aggr_desc_h);
            t = (GUEST<int16_t> *)(aggr_desc_p + aggr_desc_size);
            /* ';;' */
            t[0] = CWC(0x3b3b);
        }
    }

    /* compute the offset of the desc we are interested in */
    desc_offset(aggr_desc_h, index, &info,
                &offset);

    if(info.key_p)
        inline_desc = (inline_desc_t *)(aggr_desc_p + offset + 4);
    else
        inline_desc = (inline_desc_t *)(aggr_desc_p + offset);

    if(create_p)
    {
        int diff, aggr_desc_size;
        int old_size, new_size;

        aggr_desc_size = GetHandleSize(aggr_desc_h);

        if(index == info.count + 1)
            old_size = 0;
        else
            old_size = CL(inline_desc->size) + info.inline_desc_header_size;

        if(delete_p)
            new_size = 0;
        else
            /* #### */
            new_size = *size_return + info.inline_desc_header_size;

        /* resize it */
        if(old_size < new_size)
        {
            /* grow */
            diff = new_size - old_size;

            SetHandleSize(aggr_desc_h, aggr_desc_size + diff);
            if(LM(MemErr) != CWC(noErr))
                AE_RETURN_ERROR(CW(LM(MemErr)));
            aggr_desc_p = (char *)STARH(aggr_desc_h);
            if(aggr_desc_size < offset + old_size)
                abort();
            memmove(aggr_desc_p + offset + old_size,
                    aggr_desc_p + offset + new_size,
                    aggr_desc_size - offset - old_size);
        }
        else if(old_size > new_size)
        {
            diff = old_size - new_size;

            if(aggr_desc_size < offset + old_size)
                abort();
            memmove(aggr_desc_p + offset + old_size,
                    aggr_desc_p + offset + new_size,
                    aggr_desc_size - offset - old_size);
            SetHandleSize(aggr_desc_h, aggr_desc_size - diff);
            if(LM(MemErr) != CWC(noErr))
                AE_RETURN_ERROR(CW(LM(MemErr)));
            aggr_desc_p = (char *)STARH(aggr_desc_h);
        }
        memset(aggr_desc_p + offset, '\000', new_size);

        /* recompute the subdesc_info since the `aggr_desc_h' handle may
         have been relocated, which could invalidate `info.count_p' */
        get_subdesc_info(aggr_desc_h, &info, attribute_p);

        if(info.key_p)
            inline_desc = (inline_desc_t *)(aggr_desc_p + offset + 4);
        else
            inline_desc = (inline_desc_t *)(aggr_desc_p + offset);

        if(attribute_p)
        {
            PARAM_OFFSET_X(aggr_desc_h)
                = CL(PARAM_OFFSET(aggr_desc_h) - old_size + new_size);
        }

        if(!delete_p)
        {
            if(index == info.count + 1)
            {
                info.count++;
                *(info.count_p) = CL(info.count);
            }

            inline_desc->size = CL(*size_return);
        }
        else
        {
            info.count--;
            *(info.count_p) = CL(info.count);
        }
    }

    if(!delete_p)
    {
        *addr_return = (aggr_desc_p + offset);
        *size_return = CL(inline_desc->size);
    }

    AE_RETURN_ERROR(noErr);
}

static bool
find_key_index(Handle aggr_desc_h, int32_t keyword, bool attribute_p,
               int *index_return)
{
    subdesc_info_t info;
    int count;
    char *t;

    get_subdesc_info(aggr_desc_h, &info, attribute_p);

    t = (char *)STARH(aggr_desc_h) + info.base_offset;
    for(count = 1; count <= info.count; count++)
    {
        inline_key_desc_t *inline_key_desc;

        inline_key_desc = (inline_key_desc_t *)t;

        if(CL(inline_key_desc->key) == keyword)
        {
            *index_return = count;
            return true;
        }

        t += (CL(inline_key_desc->size)
              /* inline key desc header size */
              + 12);
    }

    *index_return = info.count + 1;
    return false;
}

static bool
aggr_delete_index(Handle aggr_handle,
                  bool attr_p,
                  int index)
{
    OSErr err;
    char *dummy_addr;
    int dummy_size;

    err = aggr_desc_get_addr(aggr_handle, index, attr_p,
                             &dummy_addr, &dummy_size, false,
                             true);

    if(err != noErr)
        return false;
    else
        return true;
}

static bool
aggr_put_nth_desc(Handle aggr_handle,
                  int index, OSErr *out_failcode,
                  descriptor_t *in_desc)
{
    Handle in_desc_data;
    DescType in_desc_type;
    inline_desc_t *inline_desc;
    int size;
    OSErr err;

    in_desc_data = DESC_DATA(in_desc);
    in_desc_type = DESC_TYPE(in_desc);
    size = GetHandleSize(in_desc_data);

    err = aggr_desc_get_addr(aggr_handle, index, false,
                             (char **)&inline_desc, &size, true, false);
    if(err != noErr)
    {
        *out_failcode = err;
        return false;
    }

    inline_desc->type = CL(in_desc_type);
    memcpy(inline_desc->data, STARH(in_desc_data), size);

    *out_failcode = noErr;
    return true;
}

static bool
aggr_get_nth_desc(Handle aggr_handle,
                  int index,
                  GUEST<int32_t> *out_keyword,
                  descriptor_t *out_desc)
{
    char *addr;
    int size;
    OSErr err;

    err = aggr_desc_get_addr(aggr_handle, index, false,
                             &addr, &size, false, false);
    if(err != noErr)
        return false;

    if(ATTRIBUTE_COUNT(aggr_handle) == typeAEList)
    {
        inline_desc_t *inline_out_desc;

        inline_out_desc = (inline_desc_t *)addr;

        if(out_keyword)
            *out_keyword = CLC(typeWildCard);
        {
            HLockGuard guard(aggr_handle);
            err = AECreateDesc(CL(inline_out_desc->type),
                               (Ptr)inline_out_desc->data, size,
                               out_desc);
        }
        if(err != noErr)
            return false;
    }
    else
    {
        inline_key_desc_t *inline_out_desc;

        inline_out_desc = (inline_key_desc_t *)addr;

        if(out_keyword)
            *out_keyword = inline_out_desc->key;
        {
            HLockGuard guard(aggr_handle);
            err = AECreateDesc(CL(inline_out_desc->type),
                               (Ptr)inline_out_desc->data, size,
                               out_desc);
        }
        if(err != noErr)
            return false;
    }

    return true;
}

static bool
aggr_put_key_desc(Handle aggr_handle,
                  int32_t keyword,
                  bool attr_p,
                  descriptor_t *in_desc)
{
    Handle in_desc_data;
    DescType in_desc_type;
    inline_key_desc_t *inline_key_desc;
    int size, index;
    OSErr err;

    /* #### test this */
    if(attr_p
       && (keyword == keyAddressAttr
           || keyword == keyEventClassAttr
           || keyword == keyEventIDAttr))
    {
        gui_fatal("can't set builtin AE attribute");
    }

    in_desc_data = DESC_DATA(in_desc);
    in_desc_type = DESC_TYPE(in_desc);
    size = GetHandleSize(in_desc_data);

    find_key_index(aggr_handle, keyword, attr_p, &index);

    err = aggr_desc_get_addr(aggr_handle, index, attr_p,
                             (char **)&inline_key_desc, &size, true, false);
    if(err != noErr)
        return false;

    inline_key_desc->key = CL(keyword);
    inline_key_desc->type = CL(in_desc_type);
    memcpy(inline_key_desc->data, STARH(in_desc_data), size);

    return true;
}

static descriptor_t *
aggr_get_key_desc(Handle aggr_handle,
                  int32_t keyword,
                  bool attr_p,
                  descriptor_t *out_desc)
{
    ae_header_t *event_data;
    inline_key_desc_t *inline_key_desc;
    int size;
    int index;
    OSErr err;

    if(attr_p)
    {
        /* #### stinking apple special-case bullshit */

        event_data = (ae_header_t *)STARH(aggr_handle);

        switch(keyword)
        {
            case keyAddressAttr:
            {
                inline_desc_t *target;

                target = &event_data->target;

                {
                    HLockGuard guard(aggr_handle);
                    err = AECreateDesc(CL(target->type),
                                       (Ptr)target->data, CL(target->size),
                                       out_desc);
                }
                if(err != noErr)
                    return NULL;
                return out_desc;
            }

            case keyEventClassAttr:
            {
                {
                    HLockGuard guard(aggr_handle);
                    err = AECreateDesc(typeType,
                                       (Ptr)&event_data->event_class,
                                       sizeof event_data->event_class,
                                       out_desc);
                }
                if(err != noErr)
                    return NULL;
                return out_desc;
            }

            case keyEventIDAttr:
            {
                {
                    HLockGuard guard(aggr_handle);
                    err = AECreateDesc(typeType,
                                       (Ptr)&event_data->event_id,
                                       sizeof event_data->event_id,
                                       out_desc);
                }
                if(err != noErr)
                    return NULL;
                return out_desc;
            }
        }
    }

    if(!find_key_index(aggr_handle, keyword, attr_p, &index))
        return NULL;

    err = aggr_desc_get_addr(aggr_handle, index, attr_p,
                             (char **)&inline_key_desc, &size, false, false);
    if(err != noErr)
        return NULL;

    {
        HLockGuard guard(aggr_handle);
        err = AECreateDesc(CL(inline_key_desc->type),
                           (Ptr)inline_key_desc->data, size,
                           out_desc);
    }
    if(err != noErr)
        return NULL;

    return out_desc;
}

static bool
aggr_delete_key_desc(Handle aggr_handle,
                     int32_t keyword,
                     bool attr_p)
{
    char *dummy_addr;
    int dummy_size;
    int index;
    OSErr err;

    /* #### test this */
    if(attr_p
       && (keyword == keyAddressAttr
           || keyword == keyEventClassAttr
           || keyword == keyEventIDAttr))
    {
        gui_fatal("can't delete builtin AE attribute");
    }

    if(!find_key_index(aggr_handle, keyword, attr_p, &index))
        return false;

    err = aggr_desc_get_addr(aggr_handle, index, attr_p,
                             &dummy_addr, &dummy_size, false,
                             true);
    if(err != noErr)
        return false;
    else
        return true;
}

static void
ae_desc_to_ptr(descriptor_t *desc,
               Ptr data, uint32_t max_size, GUEST<int32_t> *size_out)
{
    uint32_t copy_size, desc_size;
    Handle desc_data;

    desc_data = DESC_DATA(desc);
    desc_size = GetHandleSize(desc_data);

    copy_size = std::min(desc_size, max_size);

    memcpy(data, STARH(desc_data), copy_size);

    *size_out = CL(copy_size);
}

#if 0

void
dump_union_desc (union desc *foo, bool key_pair_p)
{
  AEDesc *desc;
  uint32_t type;
  char data[1024];
  uint32_t size;
  
  if (key_pair_p)
    {
      uint32_t key;

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
  uint32_t type;
  
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

OSErr Executor::C_AECreateAppleEvent(AEEventClass event_class,
                                     AEEventID event_id, AEAddressDesc *target,
                                     int16_t return_id, int32_t transaction_id,
                                     AppleEvent *evt_out)
{
    Handle target_data;
    DescType target_type;
    int target_size, event_size;
    ae_header_t *event_data;

    target_data = DESC_DATA(target);
    target_type = DESC_TYPE(target);
    target_size = GetHandleSize(target_data);

    event_size = sizeof *event_data + target_size + 10;
    event_data = (ae_header_t *)alloca(event_size);
    memset(event_data, '\000', event_size);

    event_data->param_offset = CL(event_size + 2);

    event_data->event_class = CL(event_class);
    event_data->event_id = CL(event_id);

    event_data->target.size = CL(target_size);
    event_data->target.type = CL(target_type);
    memcpy(&event_data->target.data[0], STARH(target_data), target_size);

    {
        GUEST<int32_t> *t;

        t = (GUEST<int32_t> *)((char *)event_data + sizeof *event_data + target_size);

        t[0] = TICKX("aevt");
        t[1] = CLC(0x00010001);
    }

    {
        GUEST<int16_t> *t;

        t = (GUEST<int16_t> *)((char *)event_data + sizeof *event_data + target_size + 8);

        /* ';;' */
        t[0] = CWC(0x3b3b);
    }

    AE_RETURN_ERROR(AECreateDesc(typeAppleEvent,
                                 (Ptr)event_data, event_size,
                                 evt_out));
}

/* generic descriptor functions */

OSErr Executor::C_AECreateDesc(DescType type, Ptr data, Size data_size,
                               AEDesc *desc_out)
{
    Handle h;

    h = NewHandle(data_size);
    if(h == NULL)
        AE_RETURN_ERROR(memFullErr);

    if(data)
        memcpy(STARH(h), data, data_size);
    else
    {
        if(data_size != 0)
            warning_unexpected("NULL data, data_size = %d", data_size);
        memset(STARH(h), 0, data_size);
    }

    DESC_TYPE_X(desc_out) = CL(type);
    DESC_DATA_X(desc_out) = RM(h);

    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AEDisposeDesc(AEDesc *desc)
{
    DisposHandle(DESC_DATA(desc));

    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AEDuplicateDesc(AEDesc *src, AEDesc *dst)
{
    OSErr err;
    Handle src_data;

    src_data = DESC_DATA(src);

    {
        HLockGuard guard(src_data);
        err = AECreateDesc(DESC_TYPE(src),
                           STARH(src_data), GetHandleSize(src_data),
                           dst);
    }
    AE_RETURN_ERROR(err);
}

/* descriptor functions for lists */

OSErr Executor::C_AECreateList(Ptr list_elt_prefix, Size list_elt_prefix_size,
                               Boolean is_record_p, AEDescList *list_out)
{
    list_header_t header;
    DescType type;

    /* #### */
    gui_assert(!list_elt_prefix_size);

    type = is_record_p ? typeAERecord : typeAEList;

    memset(&header, '\000', sizeof header);

    header.attribute_count = CL(type);
    header.param_offset = CLC(0x18);

    AE_RETURN_ERROR(AECreateDesc(type,
                                 (Ptr)&header, sizeof header,
                                 list_out));
}

OSErr Executor::C_AECountItems(AEDescList *list, GUEST<int32_t> *count_out)
{
    subdesc_info_t info;
    Handle aggr_desc_h;
    OSErr err;

    aggr_desc_h = DESC_DATA(list);
    err = get_subdesc_info(aggr_desc_h, &info, false);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    *count_out = CL(info.count);

    AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AEGetNthDesc(AEDescList *list, int32_t index,
                               DescType desired_type,
                               GUEST<AEKeyword> *keyword_out, AEDesc *desc_out)
{
    descriptor_t *desc = (descriptor_t *)alloca(sizeof *desc);

    if(!LIST_CLASS_P(list))
        AE_RETURN_ERROR(errAEWrongDataType);

    if(!aggr_get_nth_desc(DESC_DATA(list), index, keyword_out, desc))
        AE_RETURN_ERROR(errAEDescNotFound);

    AE_RETURN_ERROR(AECoerceDesc(desc, desired_type, desc_out));
}

OSErr Executor::C_AEGetNthPtr(AEDescList *list, int32_t index,
                              DescType desired_type,
                              GUEST<AEKeyword> *keyword_out,
                              GUEST<DescType> *type_out, Ptr data,
                              int32_t max_size, GUEST<Size> *size_out)
{
    descriptor_t *desc = (descriptor_t *)alloca(sizeof *desc);
    descriptor_t *coerced_desc = (descriptor_t *)alloca(sizeof *coerced_desc);
    OSErr err;

    if(!LIST_CLASS_P(list))
        AE_RETURN_ERROR(errAEWrongDataType);

    if(!aggr_get_nth_desc(DESC_DATA(list), index, keyword_out, desc))
        AE_RETURN_ERROR(errAEDescNotFound);

    err = AECoerceDesc(desc, desired_type, coerced_desc);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    *type_out = DESC_TYPE_X(coerced_desc);
    ae_desc_to_ptr(desc,
                   data, max_size, size_out);
    AE_RETURN_ERROR(AEDisposeDesc(coerced_desc));
}

OSErr Executor::C_AEPutDesc(AEDescList *list, int32_t index, AEDesc *desc)
{
    OSErr retval = noErr;

    if(!LIST_CLASS_P(list))
        AE_RETURN_ERROR(errAEWrongDataType);

    aggr_put_nth_desc(DESC_DATA(list), index, &retval, desc);
    AE_RETURN_ERROR(retval);
}

OSErr Executor::C_AEPutPtr(AEDescList *list, int32_t index, DescType type,
                           Ptr data, Size data_size)
{
    descriptor_t *desc;
    OSErr err, retval;

    if(!LIST_CLASS_P(list))
        AE_RETURN_ERROR(errAEWrongDataType);

    desc = (descriptor_t *)alloca(sizeof *desc);
    err = AECreateDesc(type, data, data_size, desc);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    aggr_put_nth_desc(DESC_DATA(list), index, &retval, desc);

    AE_RETURN_ERROR(retval);
}

OSErr Executor::C_AEDeleteItem(AEDescList *list, int32_t index)
{
    if(!LIST_CLASS_P(list))
        AE_RETURN_ERROR(errAEWrongDataType);

    if(aggr_delete_index(DESC_DATA(list), false, index))
        AE_RETURN_ERROR(errAEIllegalIndex);
    else
        AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AESizeOfNthItem(AEDescList *list, int32_t index,
                                  GUEST<DescType> *type_out,
                                  GUEST<Size> *size_out)
{
    descriptor_t *desc = (descriptor_t *)alloca(sizeof *desc);

    if(!LIST_CLASS_P(list))
        AE_RETURN_ERROR(errAEWrongDataType);

    if(!aggr_get_nth_desc(DESC_DATA(list), index, NULL, desc))
        AE_RETURN_ERROR(errAEIllegalIndex);

    *type_out = DESC_TYPE_X(desc);
    *size_out = CL(GetHandleSize((Handle)DESC_DATA(desc)));

    AE_RETURN_ERROR(noErr);
}

/* descriptor functions for key pair records */

OSErr Executor::C_AEGetKeyDesc(AERecord *record, AEKeyword keyword,
                               DescType desired_type, AEDesc *desc_out)
{
    descriptor_t *desc;

    if(!RECORD_CLASS_P(record))
        AE_RETURN_ERROR(errAEWrongDataType);

    desc = aggr_get_key_desc(DESC_DATA(record), keyword, false,
                             (descriptor_t *)alloca(sizeof *desc));
    if(desc == NULL)
        AE_RETURN_ERROR(errAEDescNotFound);

    AE_RETURN_ERROR(AECoerceDesc(desc, desired_type, desc_out));
}

OSErr Executor::C_AEPutKeyDesc(AERecord *record, AEKeyword keyword,
                               AEDesc *desc)
{
    if(!RECORD_CLASS_P(record))
        AE_RETURN_ERROR(errAEWrongDataType);

    if(aggr_put_key_desc(DESC_DATA(record), keyword, false, desc))
        AE_RETURN_ERROR(noErr);
    else
        AE_RETURN_ERROR(memFullErr);
}

OSErr Executor::C_AEGetKeyPtr(AERecord *record, AEKeyword keyword,
                              DescType desired_type, GUEST<DescType> *type_out,
                              Ptr data, Size max_size, GUEST<Size> *size_out)
{
    descriptor_t *desc = (descriptor_t *)alloca(sizeof *desc);
    descriptor_t *coerced_desc = (descriptor_t *)alloca(sizeof *coerced_desc);
    OSErr err;

    if(!RECORD_CLASS_P(record))
        AE_RETURN_ERROR(errAEWrongDataType);

    if(!aggr_get_key_desc(DESC_DATA(record), keyword, false, desc))
        AE_RETURN_ERROR(errAEDescNotFound);

    err = AECoerceDesc(desc, desired_type, coerced_desc);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    *type_out = DESC_TYPE_X(coerced_desc);
    ae_desc_to_ptr(desc,
                   data, max_size, size_out);
    AE_RETURN_ERROR(AEDisposeDesc(coerced_desc));
}

OSErr Executor::C_AEPutKeyPtr(AERecord *record, AEKeyword keyword,
                              DescType type, Ptr data, Size data_size)
{
    descriptor_t *desc;
    OSErr err, retval;

    if(!RECORD_CLASS_P(record))
        AE_RETURN_ERROR(errAEWrongDataType);

    desc = (descriptor_t *)alloca(sizeof *desc);
    err = AECreateDesc(type, data, data_size, desc);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    if(aggr_put_key_desc(DESC_DATA(record), keyword, false, desc))
        retval = noErr;
    else
        retval = memFullErr;

    AE_RETURN_ERROR(retval);
}

OSErr Executor::C_AEDeleteKeyDesc(AERecord *record, AEKeyword keyword)
{
    if(!RECORD_CLASS_P(record))
        AE_RETURN_ERROR(errAEWrongDataType);

    if(!aggr_delete_key_desc(DESC_DATA(record), keyword, false))
        AE_RETURN_ERROR(errAEDescNotFound);
    else
        AE_RETURN_ERROR(noErr);
}

OSErr Executor::C_AESizeOfKeyDesc(AERecord *record, AEKeyword keyword,
                                  GUEST<DescType> *type_out,
                                  GUEST<Size> *size_out)
{
    descriptor_t *desc = (descriptor_t *)alloca(sizeof *desc);

    if(!RECORD_CLASS_P(record))
        AE_RETURN_ERROR(errAEWrongDataType);

    if(!aggr_get_key_desc(DESC_DATA(record), keyword, false, desc))
        AE_RETURN_ERROR(errAEDescNotFound);

    *type_out = DESC_TYPE_X(desc);
    *size_out = CL(GetHandleSize((Handle)DESC_DATA(desc)));

    AE_RETURN_ERROR(noErr);
}

/* descriptor functions for apple events */

OSErr Executor::C_AEPutAttributePtr(AppleEvent *evt, AEKeyword keyword,
                                    DescType type, Ptr data, Size data_size)
{
    descriptor_t *desc;
    OSErr err, retval;

    if(!APPLE_EVENT_CLASS_P(evt))
        AE_RETURN_ERROR(errAEWrongDataType);

    desc = (descriptor_t *)alloca(sizeof *desc);
    err = AECreateDesc(type, data, data_size, desc);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    if(aggr_put_key_desc(DESC_DATA(evt), keyword, true, desc))
        retval = noErr;
    else
        retval = memFullErr;

    AE_RETURN_ERROR(retval);
}

OSErr Executor::C_AEPutAttributeDesc(AppleEvent *evt, AEKeyword keyword,
                                     AEDesc *desc)
{
    if(!APPLE_EVENT_CLASS_P(evt))
        AE_RETURN_ERROR(errAEWrongDataType);

    if(aggr_put_key_desc(DESC_DATA(evt), keyword, true, desc))
        AE_RETURN_ERROR(noErr);
    else
        AE_RETURN_ERROR(memFullErr);
}

OSErr Executor::C_AEGetAttributeDesc(AppleEvent *evt, AEKeyword keyword,
                                     DescType desired_type, AEDesc *desc_out)
{
    descriptor_t *desc;

    if(!APPLE_EVENT_CLASS_P(evt))
        AE_RETURN_ERROR(errAEWrongDataType);

    desc = aggr_get_key_desc(DESC_DATA(evt), keyword, true,
                             (descriptor_t *)alloca(sizeof *desc));
    if(desc == NULL)
        AE_RETURN_ERROR(errAEDescNotFound);

    AE_RETURN_ERROR(AECoerceDesc(desc, desired_type, desc_out));
}

OSErr Executor::C_AEGetAttributePtr(AppleEvent *evt, AEKeyword keyword,
                                    DescType desired_type,
                                    GUEST<DescType> *type_out, Ptr data,
                                    Size max_size, GUEST<Size> *size_out)
{
    descriptor_t *desc;
    descriptor_t *coerced_desc = (descriptor_t *)alloca(sizeof *coerced_desc);
    OSErr err;

    if(!APPLE_EVENT_CLASS_P(evt))
        AE_RETURN_ERROR(errAEWrongDataType);

    desc = aggr_get_key_desc(DESC_DATA(evt), keyword, true,
                             (descriptor_t *)alloca(sizeof *desc));
    if(desc == NULL)
        AE_RETURN_ERROR(errAEDescNotFound);

    err = AECoerceDesc(desc, desired_type, coerced_desc);
    if(err != noErr)
        AE_RETURN_ERROR(err);

    *type_out = DESC_TYPE_X(coerced_desc);
    ae_desc_to_ptr(desc,
                   data, max_size, size_out);
    AE_RETURN_ERROR(AEDisposeDesc(coerced_desc));
}
/* This does not exist.
OSErr Executor::C_AEDeleteAttribute(AppleEvent *evt, AEKeyword keyword)
{
    if(!APPLE_EVENT_CLASS_P(evt))
        AE_RETURN_ERROR(errAEWrongDataType);

    if(!aggr_delete_key_desc(DESC_DATA(evt), keyword, true))
        AE_RETURN_ERROR(errAEDescNotFound);
    else
        AE_RETURN_ERROR(noErr);
}
*/
OSErr Executor::C_AESizeOfAttribute(AppleEvent *evt, AEKeyword keyword,
                                    GUEST<DescType> *type_out,
                                    GUEST<Size> *size_out)
{
    descriptor_t *desc;

    if(!APPLE_EVENT_CLASS_P(evt))
        AE_RETURN_ERROR(errAEWrongDataType);

    desc = aggr_get_key_desc(DESC_DATA(evt), keyword, true,
                             (descriptor_t *)alloca(sizeof *desc));
    if(desc == NULL)
        AE_RETURN_ERROR(errAEDescNotFound);

    *type_out = DESC_TYPE_X(desc);
    *size_out = CL(GetHandleSize((Handle)DESC_DATA(desc)));

    AE_RETURN_ERROR(noErr);
}

/* parameter functions; these simply call the associated `record'
   functions */

/* they don't exist, since the param and record accessor traps use the
   same trap selectors */
