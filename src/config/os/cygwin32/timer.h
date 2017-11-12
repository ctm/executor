
/* Declarations for the Windows Multimedia timer functions */

enum
{
    TIME_ONESHOT,
    TIME_PERIODIC
};

uint32 __attribute__((stdcall)) timeSetEvent(uint32 delay, uint32 resolution,
                                             void *callback, uint32 dummy,
                                             uint32 type);
uint32 __attribute__((stdcall)) timeKillEvent(uint32 id);
