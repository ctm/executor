
/* shutdown manager define/routines */

#define sdOnPowerOff 1
#define sdOnRestart 2
#define sdOnUnmount 4
#define sdOnDrivers 8
#define sdOnRestartOrPower (sdOnPowerOff + sdOnRestart)

extern pascal trap void C_ShutDwnPower (void);
extern pascal trap void C_ShutDwnStart (void);
extern pascal trap void C_ShutDwnInstall (ProcPtr shutdown_proc,
					int16 flags);
extern pascal trap void C_ShutDwnRemove (ProcPtr shutdown_proc);
  
