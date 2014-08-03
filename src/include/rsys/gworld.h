#if !defined (_GWORLD_H_)
#define _GWORLD_H_
namespace Executor {
typedef struct gw_info
{
  GWorldPtr gw;
  PixMapHandle gw_pixmap;
  GDHandle gw_gd;
  PixMapHandle gw_gd_pixmap;
  
  GWorldFlags flags;
  
  int pixel_lock_count;
  boolean_t gd_allocated_p;
  
  struct gw_info *next;
} gw_info_t;

extern gw_info_t *lookup_gw_info_by_gw (GWorldPtr);
extern gw_info_t *lookup_gw_info_by_gw_pixmap (PixMapHandle);
extern gw_info_t *lookup_gw_info_by_gw_pixmap_baseaddr (void *);

extern gw_info_t *lookup_gw_info_by_gw_gd_pixmap (PixMapHandle);
}
#endif /* !_GWORLD_H_ */
