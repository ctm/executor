
/* shutdown manager define/routines */

namespace Executor
{
enum
{
    sdOnPowerOff = 1,
    sdOnRestart = 2,
    sdOnUnmount = 4,
    sdOnDrivers = 8,
    sdOnRestartOrPower = (sdOnPowerOff + sdOnRestart),
};

extern void C_ShutDwnPower(void);
PASCAL_FUNCTION(ShutDwnPower);
extern void C_ShutDwnStart(void);
PASCAL_FUNCTION(ShutDwnStart);
extern void C_ShutDwnInstall(ProcPtr shutdown_proc,
                                         int16_t flags);
PASCAL_FUNCTION(ShutDwnInstall);
extern void C_ShutDwnRemove(ProcPtr shutdown_proc);
PASCAL_FUNCTION(ShutDwnRemove);
}
