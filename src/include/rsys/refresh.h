#if !defined (_refresh_h_)
#define _refresh_h_
namespace Executor {
extern void set_refresh_rate (int new1);
extern void dequeue_refresh_task (void);
void C_handle_refresh (void);
extern BOOLEAN find_changed_rect_and_update_shadow
(const uint32 *screen, uint32 *shadow,
 long row_longs, long num_rows,
 int *top_long, int *left_long,
 int *bottom_long, int *right_long);
}
#endif /* !_refresh_h_ */
