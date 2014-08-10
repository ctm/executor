#if !defined (_RSYS_SOUNDFAKE_H_)
#define _RSYS_SOUNDFAKE_H_

#include "rsys/sounddriver.h"
#include "TimeMgr.h"

namespace Executor {
	class SoundFake: public SoundDriver {
	public:
		virtual bool sound_init();
		virtual void sound_shutdown();
		virtual bool sound_works();
		virtual bool sound_silent();
		virtual void HungerFinish();
		virtual void sound_go();
		virtual void sound_stop();
		virtual void HungerStart();
		virtual struct hunger_info GetHungerInfo();
		virtual void sound_clear_pending();
		virtual bool HasSoundClearPending() {
			return true;
		}
		
	private:
		snd_time t1;
		/* # of fake buffers currently enqueued. */
		int num_fake_buffers_enqueued;
		/* Set to TRUE when we're shutting down, and don't want any new sound
		 * to creep in.
		 */
		syn68k_addr_t fake_sound_callback;
		bool no_more_sound_p;
		TMTask fake_sound_tm_task;
		static syn68k_addr_t handle_fake_sound_callback (syn68k_addr_t addr, void *junk);
		void NoteSoundInterrupt();
		void set_up_tm_task();
	};
}

#endif /* !_RSYS_SOUNDFAKE_H_ */
