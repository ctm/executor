
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

extern pascal trap void C_ShutDwnPower(void);
PASCAL_FUNCTION(ShutDwnPower);
extern pascal trap void C_ShutDwnStart(void);
PASCAL_FUNCTION(ShutDwnStart);
extern pascal trap void C_ShutDwnInstall(ProcPtr shutdown_proc,
                                         int16_t flags);
PASCAL_FUNCTION(ShutDwnInstall);
extern pascal trap void C_ShutDwnRemove(ProcPtr shutdown_proc);
PASCAL_FUNCTION(ShutDwnRemove);
}
