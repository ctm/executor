
/* Declarations for the Windows Multimedia timer functions */

enum
{
    TIME_ONESHOT,
    TIME_PERIODIC
};

uint32_t __attribute__((stdcall)) timeSetEvent(uint32_t delay, uint32_t resolution,
                                             void *callback, uint32_t dummy,
                                             uint32_t type);
uint32_t __attribute__((stdcall)) timeKillEvent(uint32_t id);
