#if !defined (_RSYS_SOUNDDRIVER_H_)
#define _RSYS_SOUNDDRIVER_H_

#include "rsys/soundopts.h"

struct _sound_driver_t
{
  boolean_t          (*sound_init)            (struct _sound_driver_t *);
  void               (*sound_shutdown)        (struct _sound_driver_t *);
  boolean_t          (*sound_works_p)         (struct _sound_driver_t *);
  boolean_t          (*sound_silent_p)        (struct _sound_driver_t *);
  void               (*sound_hunger_finish)   (struct _sound_driver_t *);
  void               (*sound_go)              (struct _sound_driver_t *);
  void               (*sound_stop)            (struct _sound_driver_t *);
  void               (*sound_hunger_start)    (struct _sound_driver_t *);
  struct hunger_info (*sound_get_hunger_info) (struct _sound_driver_t *);
  void 		     (*sound_clear_pending)   (struct _sound_driver_t *);
};

typedef struct _sound_driver_t sound_driver_t;

/* Current sound driver in use. */
extern sound_driver_t sound_driver;

extern void sound_init (void);

#define _SOUND_CALL(func) (sound_driver.func (&sound_driver))

#define SOUND_SHUTDOWN()	_SOUND_CALL (sound_shutdown)
#define SOUND_WORKS_P()		_SOUND_CALL (sound_works_p)
#define SOUND_SILENT_P()	_SOUND_CALL (sound_silent_p)
#define SOUND_HUNGER_FINISH()	_SOUND_CALL (sound_hunger_finish)
#define SOUND_GO()		_SOUND_CALL (sound_go)
#define SOUND_STOP()		_SOUND_CALL (sound_stop)
#define SOUND_HUNGER_START()	_SOUND_CALL (sound_hunger_start)
#define SOUND_GET_HUNGER_INFO()	_SOUND_CALL (sound_get_hunger_info)
#define SOUND_GET_HUNGER_INFO()	_SOUND_CALL (sound_get_hunger_info)
#define SOUND_CLEAR_PENDING()	_SOUND_CALL (sound_clear_pending)

#include "sound-config.h"

#endif /* !_RSYS_SOUNDDRIVER_H_ */
