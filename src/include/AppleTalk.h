#include <ExMacTypes.h>
#include <rsys/lowglobals.h>

namespace Executor
{
    const LowMemGlobal<Byte> PortBUse { 0x291 }; // AppleTalk IMII-305 (true-b);
    const LowMemGlobal<Ptr> ABusVars { 0x2D8 }; // AppleTalk IMII-328 (false);
    const LowMemGlobal<Ptr> ABusDCE { 0x2DC }; // AppleTalk MPW (false);
}