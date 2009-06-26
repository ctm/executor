#if !defined (__rsys_dial_h__)
#  define __rsys_dial_h__

typedef struct PACKED
{
  int16 count;
  PACKED_MEMBER(Handle, h);
  Rect r;
  uint8 type;
  uint8 len;
  int16 res_id;
} icon_item_template_t;

#endif /* !defined (__rsys_dial_h__) */
