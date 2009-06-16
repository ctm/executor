#if !defined (__rsys_dial_h__)
#  define __rsys_dial_h__

typedef struct
{
  int16 count		PACKED;
  Handle h		PACKED_P;
  Rect r		LPACKED;
  uint8 type		LPACKED;
  uint8 len		LPACKED;
  int16 res_id		PACKED;
} icon_item_template_t;

#endif /* !defined (__rsys_dial_h__) */
