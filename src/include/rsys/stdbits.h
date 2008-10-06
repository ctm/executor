#if !defined (_STDBITS_H_)
#define _STDBITS_H_

struct cleanup_info
{
  enum
    {
      cleanup_none,
      cleanup_state,
      cleanup_free,
      cleanup_unlock_gworld_pixels,
      cleanup_unlock_gworld_pixels_update,
    } cleanup_type;
  SignedByte h_state;
  union
    {
      Handle handle;
      struct gw_info *gw_info;
      PixMapHandle pixmap_handle;
    } data;
};

extern void ROMlib_bogo_stdbits (BitMap *src_bogo_map, BitMap *dst_bogo_map,
				 const Rect *src_rect, const Rect *dst_rect,
				 short mode, RgnHandle mask);

extern void canonicalize_bogo_map_cleanup (BitMap *bogo_map,
					   struct cleanup_info *info);
extern void canonicalize_bogo_map (BitMap *bogo_map, PixMap **canonical_addr,
				   struct cleanup_info *info);

#endif /* !_STDBITS_H_ */
