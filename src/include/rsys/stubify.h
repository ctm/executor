#if     !defined(_STUBIFY_H_)
#define _STUBIFY_H_

#include "rsys/ctopflags.h"
#include "rsys/ptocflags.h"
#include "rsys/trapglue.h"

#ifdef __cplusplus
namespace Executor {
#endif
extern long CToPascalCall (void *, unsigned long, ...);
extern toolstuff_t toolstuff[NTOOLENTRIES];

#define SetCTitle(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
   StringPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x015f];\
   ((new_addr == toolstuff[0x015f].orig)\
    ? C_SetCTitle(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetCTitle ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetCTitle(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
   StringPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x015e];\
   ((new_addr == toolstuff[0x015e].orig)\
    ? C_GetCTitle(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetCTitle ,  __stub_arg_1, __stub_arg_2));\
  })
#define HideControl(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0158];\
   ((new_addr == toolstuff[0x0158].orig)\
    ? C_HideControl(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_HideControl ,  __stub_arg_1));\
  })
#define ShowControl(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0157];\
   ((new_addr == toolstuff[0x0157].orig)\
    ? C_ShowControl(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ShowControl ,  __stub_arg_1));\
  })
#define HiliteControl(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x015d];\
   ((new_addr == toolstuff[0x015d].orig)\
    ? C_HiliteControl(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_HiliteControl ,  __stub_arg_1, __stub_arg_2));\
  })
#define DrawControls(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0169];\
   ((new_addr == toolstuff[0x0169].orig)\
    ? C_DrawControls(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DrawControls ,  __stub_arg_1));\
  })
#define Draw1Control(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x016d];\
   ((new_addr == toolstuff[0x016d].orig)\
    ? C_Draw1Control(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Draw1Control ,  __stub_arg_1));\
  })
#define UpdtControl(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   RgnHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0153];\
   ((new_addr == toolstuff[0x0153].orig)\
    ? C_UpdtControl(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_UpdtControl ,  __stub_arg_1, __stub_arg_2));\
  })
#define NewControl(A1, A2, A3, A4, A5, A6, A7, A8, A9) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
   StringPtr __stub_arg_3 = (A3);\
   BOOLEAN __stub_arg_4 = (A4);\
   INTEGER __stub_arg_5 = (A5);\
   INTEGER __stub_arg_6 = (A6);\
   INTEGER __stub_arg_7 = (A7);\
   INTEGER __stub_arg_8 = (A8);\
   LONGINT __stub_arg_9 = (A9);\
 \
   new_addr = tooltraptable[0x0154];\
   ((new_addr == toolstuff[0x0154].orig)\
    ? C_NewControl(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8, __stub_arg_9)\
    : (ControlHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_NewControl ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8, __stub_arg_9));\
  })
#define GetNewControl(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   WindowPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x01be];\
   ((new_addr == toolstuff[0x01be].orig)\
    ? C_GetNewControl(__stub_arg_1, __stub_arg_2)\
    : (ControlHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetNewControl ,  __stub_arg_1, __stub_arg_2));\
  })
#define SetCtlColor(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
   CCTabHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0243];\
   ((new_addr == toolstuff[0x0243].orig)\
    ? C_SetCtlColor(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetCtlColor ,  __stub_arg_1, __stub_arg_2));\
  })
#define DisposeControl(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0155];\
   ((new_addr == toolstuff[0x0155].orig)\
    ? C_DisposeControl(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DisposeControl ,  __stub_arg_1));\
  })
#define KillControls(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0156];\
   ((new_addr == toolstuff[0x0156].orig)\
    ? C_KillControls(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_KillControls ,  __stub_arg_1));\
  })
#define SetCRefCon(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x015b];\
   ((new_addr == toolstuff[0x015b].orig)\
    ? C_SetCRefCon(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetCRefCon ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetCRefCon(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x015a];\
   ((new_addr == toolstuff[0x015a].orig)\
    ? C_GetCRefCon(__stub_arg_1)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetCRefCon ,  __stub_arg_1));\
  })
#define SetCtlAction(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
   ProcPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x016b];\
   ((new_addr == toolstuff[0x016b].orig)\
    ? C_SetCtlAction(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetCtlAction ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetCtlAction(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x016a];\
   ((new_addr == toolstuff[0x016a].orig)\
    ? C_GetCtlAction(__stub_arg_1)\
    : (ProcPtr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetCtlAction ,  __stub_arg_1));\
  })
#define GetCVariant(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0009];\
   ((new_addr == toolstuff[0x0009].orig)\
    ? C_GetCVariant(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetCVariant ,  __stub_arg_1));\
  })
#define GetAuxCtl(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
   GUEST<AuxCtlHandle> * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0244];\
   ((new_addr == toolstuff[0x0244].orig)\
    ? C_GetAuxCtl(__stub_arg_1, __stub_arg_2)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetAuxCtl ,  __stub_arg_1, __stub_arg_2));\
  })
#define FindControl(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Point __stub_arg_1 = (A1);\
   WindowPtr __stub_arg_2 = (A2);\
   GUEST<ControlHandle> * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x016c];\
   ((new_addr == toolstuff[0x016c].orig)\
    ? C_FindControl(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FindControl ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define TrackControl(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
   ProcPtr __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0168];\
   ((new_addr == toolstuff[0x0168].orig)\
    ? C_TrackControl(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TrackControl ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define TestControl(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0166];\
   ((new_addr == toolstuff[0x0166].orig)\
    ? C_TestControl(__stub_arg_1, __stub_arg_2)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TestControl ,  __stub_arg_1, __stub_arg_2));\
  })
#define SetCtlValue(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0163];\
   ((new_addr == toolstuff[0x0163].orig)\
    ? C_SetCtlValue(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetCtlValue ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetCtlValue(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0160];\
   ((new_addr == toolstuff[0x0160].orig)\
    ? C_GetCtlValue(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetCtlValue ,  __stub_arg_1));\
  })
#define SetCtlMin(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0164];\
   ((new_addr == toolstuff[0x0164].orig)\
    ? C_SetCtlMin(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetCtlMin ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetCtlMin(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0161];\
   ((new_addr == toolstuff[0x0161].orig)\
    ? C_GetCtlMin(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetCtlMin ,  __stub_arg_1));\
  })
#define SetCtlMax(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0165];\
   ((new_addr == toolstuff[0x0165].orig)\
    ? C_SetCtlMax(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetCtlMax ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetCtlMax(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0162];\
   ((new_addr == toolstuff[0x0162].orig)\
    ? C_GetCtlMax(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetCtlMax ,  __stub_arg_1));\
  })
#define MoveControl(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0159];\
   ((new_addr == toolstuff[0x0159].orig)\
    ? C_MoveControl(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_MoveControl ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define DragControl(A1, A2, A3, A4, A5) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
   Rect * __stub_arg_4 = (A4);\
   INTEGER __stub_arg_5 = (A5);\
 \
   new_addr = tooltraptable[0x0167];\
   ((new_addr == toolstuff[0x0167].orig)\
    ? C_DragControl(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DragControl ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5));\
  })
#define SizeControl(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   ControlHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x015c];\
   ((new_addr == toolstuff[0x015c].orig)\
    ? C_SizeControl(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SizeControl ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define Alert(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   ProcPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0185];\
   ((new_addr == toolstuff[0x0185].orig)\
    ? C_Alert(__stub_arg_1, __stub_arg_2)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Alert ,  __stub_arg_1, __stub_arg_2));\
  })
#define StopAlert(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   ProcPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0186];\
   ((new_addr == toolstuff[0x0186].orig)\
    ? C_StopAlert(__stub_arg_1, __stub_arg_2)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StopAlert ,  __stub_arg_1, __stub_arg_2));\
  })
#define NoteAlert(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   ProcPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0187];\
   ((new_addr == toolstuff[0x0187].orig)\
    ? C_NoteAlert(__stub_arg_1, __stub_arg_2)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_NoteAlert ,  __stub_arg_1, __stub_arg_2));\
  })
#define CautionAlert(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   ProcPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0188];\
   ((new_addr == toolstuff[0x0188].orig)\
    ? C_CautionAlert(__stub_arg_1, __stub_arg_2)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CautionAlert ,  __stub_arg_1, __stub_arg_2));\
  })
#define CouldAlert(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0189];\
   ((new_addr == toolstuff[0x0189].orig)\
    ? C_CouldAlert(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CouldAlert ,  __stub_arg_1));\
  })
#define FreeAlert(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x018a];\
   ((new_addr == toolstuff[0x018a].orig)\
    ? C_FreeAlert(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FreeAlert ,  __stub_arg_1));\
  })
#define CouldDialog(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0179];\
   ((new_addr == toolstuff[0x0179].orig)\
    ? C_CouldDialog(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CouldDialog ,  __stub_arg_1));\
  })
#define FreeDialog(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x017a];\
   ((new_addr == toolstuff[0x017a].orig)\
    ? C_FreeDialog(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FreeDialog ,  __stub_arg_1));\
  })
#define NewCDialog(A1, A2, A3, A4, A5, A6, A7, A8, A9) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
   StringPtr __stub_arg_3 = (A3);\
   BOOLEAN __stub_arg_4 = (A4);\
   INTEGER __stub_arg_5 = (A5);\
   WindowPtr __stub_arg_6 = (A6);\
   BOOLEAN __stub_arg_7 = (A7);\
   LONGINT __stub_arg_8 = (A8);\
   Handle __stub_arg_9 = (A9);\
 \
   new_addr = tooltraptable[0x024b];\
   ((new_addr == toolstuff[0x024b].orig)\
    ? C_NewCDialog(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8, __stub_arg_9)\
    : (CDialogPtr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_NewCDialog ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8, __stub_arg_9));\
  })
#define NewDialog(A1, A2, A3, A4, A5, A6, A7, A8, A9) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
   StringPtr __stub_arg_3 = (A3);\
   BOOLEAN __stub_arg_4 = (A4);\
   INTEGER __stub_arg_5 = (A5);\
   WindowPtr __stub_arg_6 = (A6);\
   BOOLEAN __stub_arg_7 = (A7);\
   LONGINT __stub_arg_8 = (A8);\
   Handle __stub_arg_9 = (A9);\
 \
   new_addr = tooltraptable[0x017d];\
   ((new_addr == toolstuff[0x017d].orig)\
    ? C_NewDialog(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8, __stub_arg_9)\
    : (DialogPtr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_NewDialog ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8, __stub_arg_9));\
  })
#define GetNewDialog(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   Ptr __stub_arg_2 = (A2);\
   WindowPtr __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x017c];\
   ((new_addr == toolstuff[0x017c].orig)\
    ? C_GetNewDialog(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (DialogPtr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetNewDialog ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define CloseDialog(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   DialogPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0182];\
   ((new_addr == toolstuff[0x0182].orig)\
    ? C_CloseDialog(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CloseDialog ,  __stub_arg_1));\
  })
#define DisposDialog(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   DialogPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0183];\
   ((new_addr == toolstuff[0x0183].orig)\
    ? C_DisposDialog(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DisposDialog ,  __stub_arg_1));\
  })
#define ModalDialog(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ProcPtr __stub_arg_1 = (A1);\
   GUEST<INTEGER> * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0191];\
   ((new_addr == toolstuff[0x0191].orig)\
    ? C_ModalDialog(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ModalDialog ,  __stub_arg_1, __stub_arg_2));\
  })
#define IsDialogEvent(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   EventRecord * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x017f];\
   ((new_addr == toolstuff[0x017f].orig)\
    ? C_IsDialogEvent(__stub_arg_1)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_IsDialogEvent ,  __stub_arg_1));\
  })
#define DrawDialog(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   DialogPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0181];\
   ((new_addr == toolstuff[0x0181].orig)\
    ? C_DrawDialog(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DrawDialog ,  __stub_arg_1));\
  })
#define FindDItem(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   DialogPtr __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0184];\
   ((new_addr == toolstuff[0x0184].orig)\
    ? C_FindDItem(__stub_arg_1, __stub_arg_2)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FindDItem ,  __stub_arg_1, __stub_arg_2));\
  })
#define UpdtDialog(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   DialogPtr __stub_arg_1 = (A1);\
   RgnHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0178];\
   ((new_addr == toolstuff[0x0178].orig)\
    ? C_UpdtDialog(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_UpdtDialog ,  __stub_arg_1, __stub_arg_2));\
  })
#define DialogSelect(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   EventRecord * __stub_arg_1 = (A1);\
   GUEST<DialogPtr> * __stub_arg_2 = (A2);\
   GUEST<INTEGER> * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0180];\
   ((new_addr == toolstuff[0x0180].orig)\
    ? C_DialogSelect(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DialogSelect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define ErrorSound(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ProcPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x018c];\
   ((new_addr == toolstuff[0x018c].orig)\
    ? C_ErrorSound(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ErrorSound ,  __stub_arg_1));\
  })
#define InitDialogs(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ProcPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x017b];\
   ((new_addr == toolstuff[0x017b].orig)\
    ? C_InitDialogs(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InitDialogs ,  __stub_arg_1));\
  })
#define ParamText(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   StringPtr __stub_arg_1 = (A1);\
   StringPtr __stub_arg_2 = (A2);\
   StringPtr __stub_arg_3 = (A3);\
   StringPtr __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x018b];\
   ((new_addr == toolstuff[0x018b].orig)\
    ? C_ParamText(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ParamText ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define GetDItem(A1, A2, A3, A4, A5) \
 ({ \
   syn68k_addr_t new_addr;\
   DialogPtr __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   GUEST<INTEGER> * __stub_arg_3 = (A3);\
   GUEST<Handle> * __stub_arg_4 = (A4);\
   Rect * __stub_arg_5 = (A5);\
 \
   new_addr = tooltraptable[0x018d];\
   ((new_addr == toolstuff[0x018d].orig)\
    ? C_GetDItem(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetDItem ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5));\
  })
#define SetDItem(A1, A2, A3, A4, A5) \
 ({ \
   syn68k_addr_t new_addr;\
   DialogPtr __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   Handle __stub_arg_4 = (A4);\
   Rect * __stub_arg_5 = (A5);\
 \
   new_addr = tooltraptable[0x018e];\
   ((new_addr == toolstuff[0x018e].orig)\
    ? C_SetDItem(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetDItem ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5));\
  })
#define GetIText(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
   StringPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0190];\
   ((new_addr == toolstuff[0x0190].orig)\
    ? C_GetIText(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetIText ,  __stub_arg_1, __stub_arg_2));\
  })
#define SetIText(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
   StringPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x018f];\
   ((new_addr == toolstuff[0x018f].orig)\
    ? C_SetIText(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetIText ,  __stub_arg_1, __stub_arg_2));\
  })
#define SelIText(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   DialogPtr __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   INTEGER __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x017e];\
   ((new_addr == toolstuff[0x017e].orig)\
    ? C_SelIText(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SelIText ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define HideDItem(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   DialogPtr __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0027];\
   ((new_addr == toolstuff[0x0027].orig)\
    ? C_HideDItem(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_HideDItem ,  __stub_arg_1, __stub_arg_2));\
  })
#define ShowDItem(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   DialogPtr __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0028];\
   ((new_addr == toolstuff[0x0028].orig)\
    ? C_ShowDItem(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ShowDItem ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetStdFilterProc(A1) \
   (     C_GetStdFilterProc((A1))     )
#define SetDialogDefaultItem(A1, A2) \
   (     C_SetDialogDefaultItem((A1), (A2))     )
#define SetDialogCancelItem(A1, A2) \
   (     C_SetDialogCancelItem((A1), (A2))     )
#define SetDialogTracksCursor(A1, A2) \
   (     C_SetDialogTracksCursor((A1), (A2))     )
#define FSMakeFSSpec(A1, A2, A3, A4) \
   (     C_FSMakeFSSpec((A1), (A2), (A3), (A4))     )
#define FSpExchangeFiles(A1, A2) \
   (     C_FSpExchangeFiles((A1), (A2))     )
#define FSpOpenDF(A1, A2, A3) \
   (     C_FSpOpenDF((A1), (A2), (A3))     )
#define FSpOpenRF(A1, A2, A3) \
   (     C_FSpOpenRF((A1), (A2), (A3))     )
#define FSpCreate(A1, A2, A3, A4) \
   (     C_FSpCreate((A1), (A2), (A3), (A4))     )
#define FSpDirCreate(A1, A2, A3) \
   (     C_FSpDirCreate((A1), (A2), (A3))     )
#define FSpDelete(A1) \
   (     C_FSpDelete((A1))     )
#define FSpGetFInfo(A1, A2) \
   (     C_FSpGetFInfo((A1), (A2))     )
#define FSpSetFInfo(A1, A2) \
   (     C_FSpSetFInfo((A1), (A2))     )
#define FSpSetFLock(A1) \
   (     C_FSpSetFLock((A1))     )
#define FSpRstFLock(A1) \
   (     C_FSpRstFLock((A1))     )
#define FSpRename(A1, A2) \
   (     C_FSpRename((A1), (A2))     )
#define FSpCatMove(A1, A2) \
   (     C_FSpCatMove((A1), (A2))     )
#define FSpCreateResFile(A1, A2, A3, A4) \
   (     C_FSpCreateResFile((A1), (A2), (A3), (A4))     )
#define FSpOpenResFile(A1, A2) \
   (     C_FSpOpenResFile((A1), (A2))     )
#define LFind(A1, A2, A3, A4) \
   (     C_LFind((A1), (A2), (A3), (A4))     )
#define LNextCell(A1, A2, A3, A4) \
   (     C_LNextCell((A1), (A2), (A3), (A4))     )
#define LRect(A1, A2, A3) \
   (     C_LRect((A1), (A2), (A3))     )
#define LSearch(A1, A2, A3, A4, A5) \
   (     C_LSearch((A1), (A2), (A3), (A4), (A5))     )
#define LSize(A1, A2, A3) \
   (     C_LSize((A1), (A2), (A3))     )
#define LAddColumn(A1, A2, A3) \
   (     C_LAddColumn((A1), (A2), (A3))     )
#define LAddRow(A1, A2, A3) \
   (     C_LAddRow((A1), (A2), (A3))     )
#define LDelColumn(A1, A2, A3) \
   (     C_LDelColumn((A1), (A2), (A3))     )
#define LDelRow(A1, A2, A3) \
   (     C_LDelRow((A1), (A2), (A3))     )
#define LNew(A1, A2, A3, A4, A5, A6, A7, A8, A9) \
   (     C_LNew((A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8), (A9))     )
#define LDispose(A1) \
   (     C_LDispose((A1))     )
#define LDraw(A1, A2) \
   (     C_LDraw((A1), (A2))     )
#define LDoDraw(A1, A2) \
   (     C_LDoDraw((A1), (A2))     )
#define LScroll(A1, A2, A3) \
   (     C_LScroll((A1), (A2), (A3))     )
#define LAutoScroll(A1) \
   (     C_LAutoScroll((A1))     )
#define LUpdate(A1, A2) \
   (     C_LUpdate((A1), (A2))     )
#define LActivate(A1, A2) \
   (     C_LActivate((A1), (A2))     )
#define LClick(A1, A2, A3) \
   (     C_LClick((A1), (A2), (A3))     )
#define LLastClick(A1) \
   (     C_LLastClick((A1))     )
#define LSetSelect(A1, A2, A3) \
   (     C_LSetSelect((A1), (A2), (A3))     )
#define LAddToCell(A1, A2, A3, A4) \
   (     C_LAddToCell((A1), (A2), (A3), (A4))     )
#define LClrCell(A1, A2) \
   (     C_LClrCell((A1), (A2))     )
#define LGetCell(A1, A2, A3, A4) \
   (     C_LGetCell((A1), (A2), (A3), (A4))     )
#define LSetCell(A1, A2, A3, A4) \
   (     C_LSetCell((A1), (A2), (A3), (A4))     )
#define LCellSize(A1, A2) \
   (     C_LCellSize((A1), (A2))     )
#define LGetSelect(A1, A2, A3) \
   (     C_LGetSelect((A1), (A2), (A3))     )
#define InvalMenuBar() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x001d];\
   ((new_addr == toolstuff[0x001d].orig)\
    ? C_InvalMenuBar()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InvalMenuBar  ));\
  })
#define DrawMenuBar() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0137];\
   ((new_addr == toolstuff[0x0137].orig)\
    ? C_DrawMenuBar()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DrawMenuBar  ));\
  })
#define ClearMenuBar() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0134];\
   ((new_addr == toolstuff[0x0134].orig)\
    ? C_ClearMenuBar()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ClearMenuBar  ));\
  })
#define InitMenus() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0130];\
   ((new_addr == toolstuff[0x0130].orig)\
    ? C_InitMenus()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InitMenus  ));\
  })
#define NewMenu(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   StringPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0131];\
   ((new_addr == toolstuff[0x0131].orig)\
    ? C_NewMenu(__stub_arg_1, __stub_arg_2)\
    : (MenuHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_NewMenu ,  __stub_arg_1, __stub_arg_2));\
  })
#define CalcMenuSize(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0148];\
   ((new_addr == toolstuff[0x0148].orig)\
    ? C_CalcMenuSize(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CalcMenuSize ,  __stub_arg_1));\
  })
#define GetMenu(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   int16 __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01bf];\
   ((new_addr == toolstuff[0x01bf].orig)\
    ? C_GetMenu(__stub_arg_1)\
    : (MenuHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetMenu ,  __stub_arg_1));\
  })
#define DisposeMenu(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0132];\
   ((new_addr == toolstuff[0x0132].orig)\
    ? C_DisposeMenu(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DisposeMenu ,  __stub_arg_1));\
  })
#define AppendMenu(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   StringPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0133];\
   ((new_addr == toolstuff[0x0133].orig)\
    ? C_AppendMenu(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_AppendMenu ,  __stub_arg_1, __stub_arg_2));\
  })
#define AddResMenu(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   ResType __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x014d];\
   ((new_addr == toolstuff[0x014d].orig)\
    ? C_AddResMenu(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_AddResMenu ,  __stub_arg_1, __stub_arg_2));\
  })
#define DelMenuItem(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0152];\
   ((new_addr == toolstuff[0x0152].orig)\
    ? C_DelMenuItem(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DelMenuItem ,  __stub_arg_1, __stub_arg_2));\
  })
#define InsertResMenu(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   ResType __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0151];\
   ((new_addr == toolstuff[0x0151].orig)\
    ? C_InsertResMenu(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InsertResMenu ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define InsMenuItem(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   StringPtr __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0026];\
   ((new_addr == toolstuff[0x0026].orig)\
    ? C_InsMenuItem(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InsMenuItem ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define InsertMenu(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0135];\
   ((new_addr == toolstuff[0x0135].orig)\
    ? C_InsertMenu(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InsertMenu ,  __stub_arg_1, __stub_arg_2));\
  })
#define DeleteMenu(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   int16 __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0136];\
   ((new_addr == toolstuff[0x0136].orig)\
    ? C_DeleteMenu(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DeleteMenu ,  __stub_arg_1));\
  })
#define GetNewMBar(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01c0];\
   ((new_addr == toolstuff[0x01c0].orig)\
    ? C_GetNewMBar(__stub_arg_1)\
    : (Handle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetNewMBar ,  __stub_arg_1));\
  })
#define GetMenuBar() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x013b];\
   ((new_addr == toolstuff[0x013b].orig)\
    ? C_GetMenuBar()\
    : (Handle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetMenuBar  ));\
  })
#define SetMenuBar(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x013c];\
   ((new_addr == toolstuff[0x013c].orig)\
    ? C_SetMenuBar(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetMenuBar ,  __stub_arg_1));\
  })
#define HiliteMenu(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0138];\
   ((new_addr == toolstuff[0x0138].orig)\
    ? C_HiliteMenu(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_HiliteMenu ,  __stub_arg_1));\
  })
#define MenuSelect(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Point __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x013d];\
   ((new_addr == toolstuff[0x013d].orig)\
    ? C_MenuSelect(__stub_arg_1)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_MenuSelect ,  __stub_arg_1));\
  })
#define FlashMenuBar(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x014c];\
   ((new_addr == toolstuff[0x014c].orig)\
    ? C_FlashMenuBar(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FlashMenuBar ,  __stub_arg_1));\
  })
#define MenuKey(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   CHAR __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x013e];\
   ((new_addr == toolstuff[0x013e].orig)\
    ? C_MenuKey(__stub_arg_1)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_MenuKey ,  __stub_arg_1));\
  })
#define SetItem(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   StringPtr __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0147];\
   ((new_addr == toolstuff[0x0147].orig)\
    ? C_SetItem(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetItem ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define GetItem(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   StringPtr __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0146];\
   ((new_addr == toolstuff[0x0146].orig)\
    ? C_GetItem(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetItem ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define DisableItem(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x013a];\
   ((new_addr == toolstuff[0x013a].orig)\
    ? C_DisableItem(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DisableItem ,  __stub_arg_1, __stub_arg_2));\
  })
#define EnableItem(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0139];\
   ((new_addr == toolstuff[0x0139].orig)\
    ? C_EnableItem(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_EnableItem ,  __stub_arg_1, __stub_arg_2));\
  })
#define CheckItem(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   BOOLEAN __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0145];\
   ((new_addr == toolstuff[0x0145].orig)\
    ? C_CheckItem(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CheckItem ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SetItemMark(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   CHAR __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0144];\
   ((new_addr == toolstuff[0x0144].orig)\
    ? C_SetItemMark(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetItemMark ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define GetItemMark(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   GUEST<INTEGER> * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0143];\
   ((new_addr == toolstuff[0x0143].orig)\
    ? C_GetItemMark(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetItemMark ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SetItemIcon(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   Byte __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0140];\
   ((new_addr == toolstuff[0x0140].orig)\
    ? C_SetItemIcon(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetItemIcon ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define GetItemIcon(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   GUEST<INTEGER> * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x013f];\
   ((new_addr == toolstuff[0x013f].orig)\
    ? C_GetItemIcon(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetItemIcon ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SetItemStyle(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0142];\
   ((new_addr == toolstuff[0x0142].orig)\
    ? C_SetItemStyle(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetItemStyle ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define GetItemStyle(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   GUEST<INTEGER> * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0141];\
   ((new_addr == toolstuff[0x0141].orig)\
    ? C_GetItemStyle(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetItemStyle ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define CountMItems(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0150];\
   ((new_addr == toolstuff[0x0150].orig)\
    ? C_CountMItems(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CountMItems ,  __stub_arg_1));\
  })
#define GetMHandle(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0149];\
   ((new_addr == toolstuff[0x0149].orig)\
    ? C_GetMHandle(__stub_arg_1)\
    : (MenuHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetMHandle ,  __stub_arg_1));\
  })
#define SetMenuFlash(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x014a];\
   ((new_addr == toolstuff[0x014a].orig)\
    ? C_SetMenuFlash(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetMenuFlash ,  __stub_arg_1));\
  })
#define DelMCEntries(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0260];\
   ((new_addr == toolstuff[0x0260].orig)\
    ? C_DelMCEntries(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DelMCEntries ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetMCInfo() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0261];\
   ((new_addr == toolstuff[0x0261].orig)\
    ? C_GetMCInfo()\
    : (MCTableHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetMCInfo  ));\
  })
#define SetMCInfo(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   MCTableHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0262];\
   ((new_addr == toolstuff[0x0262].orig)\
    ? C_SetMCInfo(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetMCInfo ,  __stub_arg_1));\
  })
#define DispMCInfo(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   MCTableHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0263];\
   ((new_addr == toolstuff[0x0263].orig)\
    ? C_DispMCInfo(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DispMCInfo ,  __stub_arg_1));\
  })
#define GetMCEntry(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0264];\
   ((new_addr == toolstuff[0x0264].orig)\
    ? C_GetMCEntry(__stub_arg_1, __stub_arg_2)\
    : (MCEntryPtr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetMCEntry ,  __stub_arg_1, __stub_arg_2));\
  })
#define SetMCEntries(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   MCTablePtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0265];\
   ((new_addr == toolstuff[0x0265].orig)\
    ? C_SetMCEntries(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetMCEntries ,  __stub_arg_1, __stub_arg_2));\
  })
#define InitProcMenu(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0008];\
   ((new_addr == toolstuff[0x0008].orig)\
    ? C_InitProcMenu(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InitProcMenu ,  __stub_arg_1));\
  })
#define MenuChoice() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0266];\
   ((new_addr == toolstuff[0x0266].orig)\
    ? C_MenuChoice()\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_MenuChoice  ));\
  })
#define GetItemCmd(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   GUEST<CHAR> * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x004e];\
   ((new_addr == toolstuff[0x004e].orig)\
    ? C_GetItemCmd(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetItemCmd ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SetItemCmd(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   CHAR __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x004f];\
   ((new_addr == toolstuff[0x004f].orig)\
    ? C_SetItemCmd(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetItemCmd ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define PopUpMenuSelect(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   MenuHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   INTEGER __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x000b];\
   ((new_addr == toolstuff[0x000b].orig)\
    ? C_PopUpMenuSelect(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PopUpMenuSelect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define PrError() \
   (     C_PrError()     )
#define PrSetError(A1) \
   (     C_PrSetError((A1))     )
#define PrOpen() \
   (     C_PrOpen()     )
#define PrClose() \
   (     C_PrClose()     )
#define PrDrvrOpen() \
   (     C_PrDrvrOpen()     )
#define PrDrvrClose() \
   (     C_PrDrvrClose()     )
#define PrCtlCall(A1, A2, A3, A4) \
   (     C_PrCtlCall((A1), (A2), (A3), (A4))     )
#define PrDrvrDCE() \
   (     C_PrDrvrDCE()     )
#define PrDrvrVers() \
   (     C_PrDrvrVers()     )
#define PrJobInit(A1) \
   (     C_PrJobInit((A1))     )
#define PrStlInit(A1) \
   (     C_PrStlInit((A1))     )
#define PrDlgMain(A1, A2) \
   (     C_PrDlgMain((A1), (A2))     )
#define PrGeneral(A1) \
   (     C_PrGeneral((A1))     )
#define PrOpenDoc(A1, A2, A3) \
   (     C_PrOpenDoc((A1), (A2), (A3))     )
#define PrOpenPage(A1, A2) \
   (     C_PrOpenPage((A1), (A2))     )
#define PrClosePage(A1) \
   (     C_PrClosePage((A1))     )
#define PrCloseDoc(A1) \
   (     C_PrCloseDoc((A1))     )
#define PrPicFile(A1, A2, A3, A4, A5) \
   (     C_PrPicFile((A1), (A2), (A3), (A4), (A5))     )
#define PrintDefault(A1) \
   (     C_PrintDefault((A1))     )
#define PrValidate(A1) \
   (     C_PrValidate((A1))     )
#define PrStlDialog(A1) \
   (     C_PrStlDialog((A1))     )
#define PrJobDialog(A1) \
   (     C_PrJobDialog((A1))     )
#define PrJobMerge(A1, A2) \
   (     C_PrJobMerge((A1), (A2))     )
#define CopyBits(A1, A2, A3, A4, A5, A6) \
 ({ \
   syn68k_addr_t new_addr;\
   BitMap * __stub_arg_1 = (A1);\
   BitMap * __stub_arg_2 = (A2);\
   const Rect * __stub_arg_3 = (A3);\
   const Rect * __stub_arg_4 = (A4);\
   INTEGER __stub_arg_5 = (A5);\
   RgnHandle __stub_arg_6 = (A6);\
 \
   new_addr = tooltraptable[0x00ec];\
   ((new_addr == toolstuff[0x00ec].orig)\
    ? C_CopyBits(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CopyBits ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6));\
  })
#define ScrollRect(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   RgnHandle __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x00ef];\
   ((new_addr == toolstuff[0x00ef].orig)\
    ? C_ScrollRect(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ScrollRect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define CMY2RGB(A1, A2) \
   (     C_CMY2RGB((A1), (A2))     )
#define RGB2CMY(A1, A2) \
   (     C_RGB2CMY((A1), (A2))     )
#define HSL2RGB(A1, A2) \
   (     C_HSL2RGB((A1), (A2))     )
#define RGB2HSL(A1, A2) \
   (     C_RGB2HSL((A1), (A2))     )
#define HSV2RGB(A1, A2) \
   (     C_HSV2RGB((A1), (A2))     )
#define RGB2HSV(A1, A2) \
   (     C_RGB2HSV((A1), (A2))     )
#define Fix2SmallFract(A1) \
   (     C_Fix2SmallFract((A1))     )
#define SmallFract2Fix(A1) \
   (     C_SmallFract2Fix((A1))     )
#define OpenCPort(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   CGrafPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0200];\
   ((new_addr == toolstuff[0x0200].orig)\
    ? C_OpenCPort(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_OpenCPort ,  __stub_arg_1));\
  })
#define CloseCPort(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   CGrafPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0202];\
   ((new_addr == toolstuff[0x0202].orig)\
    ? C_CloseCPort(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CloseCPort ,  __stub_arg_1));\
  })
#define InitCPort(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   CGrafPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0201];\
   ((new_addr == toolstuff[0x0201].orig)\
    ? C_InitCPort(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InitCPort ,  __stub_arg_1));\
  })
#define SetPortPix(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   PixMapHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0206];\
   ((new_addr == toolstuff[0x0206].orig)\
    ? C_SetPortPix(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetPortPix ,  __stub_arg_1));\
  })
#define RGBForeColor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RGBColor * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0214];\
   ((new_addr == toolstuff[0x0214].orig)\
    ? C_RGBForeColor(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_RGBForeColor ,  __stub_arg_1));\
  })
#define RGBBackColor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RGBColor * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0215];\
   ((new_addr == toolstuff[0x0215].orig)\
    ? C_RGBBackColor(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_RGBBackColor ,  __stub_arg_1));\
  })
#define GetForeColor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RGBColor * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0219];\
   ((new_addr == toolstuff[0x0219].orig)\
    ? C_GetForeColor(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetForeColor ,  __stub_arg_1));\
  })
#define GetBackColor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RGBColor * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x021a];\
   ((new_addr == toolstuff[0x021a].orig)\
    ? C_GetBackColor(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetBackColor ,  __stub_arg_1));\
  })
#define PenPixPat(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   PixPatHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x020a];\
   ((new_addr == toolstuff[0x020a].orig)\
    ? C_PenPixPat(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PenPixPat ,  __stub_arg_1));\
  })
#define BackPixPat(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   PixPatHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x020b];\
   ((new_addr == toolstuff[0x020b].orig)\
    ? C_BackPixPat(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_BackPixPat ,  __stub_arg_1));\
  })
#define OpColor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RGBColor * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0221];\
   ((new_addr == toolstuff[0x0221].orig)\
    ? C_OpColor(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_OpColor ,  __stub_arg_1));\
  })
#define HiliteColor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RGBColor * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0222];\
   ((new_addr == toolstuff[0x0222].orig)\
    ? C_HiliteColor(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_HiliteColor ,  __stub_arg_1));\
  })
#define NewPixMap() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0203];\
   ((new_addr == toolstuff[0x0203].orig)\
    ? C_NewPixMap()\
    : (PixMapHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_NewPixMap  ));\
  })
#define DisposPixMap(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   PixMapHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0204];\
   ((new_addr == toolstuff[0x0204].orig)\
    ? C_DisposPixMap(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DisposPixMap ,  __stub_arg_1));\
  })
#define CopyPixMap(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   PixMapHandle __stub_arg_1 = (A1);\
   PixMapHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0205];\
   ((new_addr == toolstuff[0x0205].orig)\
    ? C_CopyPixMap(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CopyPixMap ,  __stub_arg_1, __stub_arg_2));\
  })
#define NewPixPat() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0207];\
   ((new_addr == toolstuff[0x0207].orig)\
    ? C_NewPixPat()\
    : (PixPatHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_NewPixPat  ));\
  })
#define GetPixPat(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x020c];\
   ((new_addr == toolstuff[0x020c].orig)\
    ? C_GetPixPat(__stub_arg_1)\
    : (PixPatHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetPixPat ,  __stub_arg_1));\
  })
#define DisposPixPat(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   PixPatHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0208];\
   ((new_addr == toolstuff[0x0208].orig)\
    ? C_DisposPixPat(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DisposPixPat ,  __stub_arg_1));\
  })
#define CopyPixPat(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   PixPatHandle __stub_arg_1 = (A1);\
   PixPatHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0209];\
   ((new_addr == toolstuff[0x0209].orig)\
    ? C_CopyPixPat(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CopyPixPat ,  __stub_arg_1, __stub_arg_2));\
  })
#define MakeRGBPat(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   PixPatHandle __stub_arg_1 = (A1);\
   RGBColor * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x020d];\
   ((new_addr == toolstuff[0x020d].orig)\
    ? C_MakeRGBPat(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_MakeRGBPat ,  __stub_arg_1, __stub_arg_2));\
  })
#define FillCRect(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   PixPatHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x020e];\
   ((new_addr == toolstuff[0x020e].orig)\
    ? C_FillCRect(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FillCRect ,  __stub_arg_1, __stub_arg_2));\
  })
#define FillCRoundRect(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   const Rect * __stub_arg_1 = (A1);\
   short __stub_arg_2 = (A2);\
   short __stub_arg_3 = (A3);\
   PixPatHandle __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x0210];\
   ((new_addr == toolstuff[0x0210].orig)\
    ? C_FillCRoundRect(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FillCRoundRect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define FillCOval(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   const Rect * __stub_arg_1 = (A1);\
   PixPatHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x020f];\
   ((new_addr == toolstuff[0x020f].orig)\
    ? C_FillCOval(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FillCOval ,  __stub_arg_1, __stub_arg_2));\
  })
#define FillCArc(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   const Rect * __stub_arg_1 = (A1);\
   short __stub_arg_2 = (A2);\
   short __stub_arg_3 = (A3);\
   PixPatHandle __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x0211];\
   ((new_addr == toolstuff[0x0211].orig)\
    ? C_FillCArc(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FillCArc ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define FillCPoly(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   PolyHandle __stub_arg_1 = (A1);\
   PixPatHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0213];\
   ((new_addr == toolstuff[0x0213].orig)\
    ? C_FillCPoly(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FillCPoly ,  __stub_arg_1, __stub_arg_2));\
  })
#define FillCRgn(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   PixPatHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0212];\
   ((new_addr == toolstuff[0x0212].orig)\
    ? C_FillCRgn(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FillCRgn ,  __stub_arg_1, __stub_arg_2));\
  })
#define ForeColor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   LONGINT __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0062];\
   ((new_addr == toolstuff[0x0062].orig)\
    ? C_ForeColor(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ForeColor ,  __stub_arg_1));\
  })
#define BackColor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   LONGINT __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0063];\
   ((new_addr == toolstuff[0x0063].orig)\
    ? C_BackColor(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_BackColor ,  __stub_arg_1));\
  })
#define ColorBit(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0064];\
   ((new_addr == toolstuff[0x0064].orig)\
    ? C_ColorBit(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ColorBit ,  __stub_arg_1));\
  })
#define GetCTable(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0218];\
   ((new_addr == toolstuff[0x0218].orig)\
    ? C_GetCTable(__stub_arg_1)\
    : (CTabHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetCTable ,  __stub_arg_1));\
  })
#define DisposCTable(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   CTabHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0224];\
   ((new_addr == toolstuff[0x0224].orig)\
    ? C_DisposCTable(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DisposCTable ,  __stub_arg_1));\
  })
#define QDError() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0240];\
   ((new_addr == toolstuff[0x0240].orig)\
    ? C_QDError()\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_QDError  ));\
  })
#define GetCTSeed() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0228];\
   ((new_addr == toolstuff[0x0228].orig)\
    ? C_GetCTSeed()\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetCTSeed  ));\
  })
#define Color2Index(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RGBColor * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0233];\
   ((new_addr == toolstuff[0x0233].orig)\
    ? C_Color2Index(__stub_arg_1)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Color2Index ,  __stub_arg_1));\
  })
#define Index2Color(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   LONGINT __stub_arg_1 = (A1);\
   RGBColor * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0234];\
   ((new_addr == toolstuff[0x0234].orig)\
    ? C_Index2Color(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Index2Color ,  __stub_arg_1, __stub_arg_2));\
  })
#define InvertColor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RGBColor * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0235];\
   ((new_addr == toolstuff[0x0235].orig)\
    ? C_InvertColor(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InvertColor ,  __stub_arg_1));\
  })
#define RealColor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RGBColor * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0236];\
   ((new_addr == toolstuff[0x0236].orig)\
    ? C_RealColor(__stub_arg_1)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_RealColor ,  __stub_arg_1));\
  })
#define GetSubTable(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   CTabHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   CTabHandle __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0237];\
   ((new_addr == toolstuff[0x0237].orig)\
    ? C_GetSubTable(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetSubTable ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define GetGray(A1, A2, A3) \
   (     C_GetGray((A1), (A2), (A3))     )
#define MakeITable(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   CTabHandle __stub_arg_1 = (A1);\
   ITabHandle __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0239];\
   ((new_addr == toolstuff[0x0239].orig)\
    ? C_MakeITable(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_MakeITable ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define ProtectEntry(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   BOOLEAN __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x023d];\
   ((new_addr == toolstuff[0x023d].orig)\
    ? C_ProtectEntry(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ProtectEntry ,  __stub_arg_1, __stub_arg_2));\
  })
#define ReserveEntry(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   BOOLEAN __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x023e];\
   ((new_addr == toolstuff[0x023e].orig)\
    ? C_ReserveEntry(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ReserveEntry ,  __stub_arg_1, __stub_arg_2));\
  })
#define SetEntries(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   ColorSpec * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x023f];\
   ((new_addr == toolstuff[0x023f].orig)\
    ? C_SetEntries(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetEntries ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define AddSearch(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ProcPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x023a];\
   ((new_addr == toolstuff[0x023a].orig)\
    ? C_AddSearch(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_AddSearch ,  __stub_arg_1));\
  })
#define AddComp(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ProcPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x023b];\
   ((new_addr == toolstuff[0x023b].orig)\
    ? C_AddComp(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_AddComp ,  __stub_arg_1));\
  })
#define DelSearch(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ProcPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x024c];\
   ((new_addr == toolstuff[0x024c].orig)\
    ? C_DelSearch(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DelSearch ,  __stub_arg_1));\
  })
#define DelComp(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ProcPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x024d];\
   ((new_addr == toolstuff[0x024d].orig)\
    ? C_DelComp(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DelComp ,  __stub_arg_1));\
  })
#define SetClientID(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x023c];\
   ((new_addr == toolstuff[0x023c].orig)\
    ? C_SetClientID(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetClientID ,  __stub_arg_1));\
  })
#define SaveEntries(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   CTabHandle __stub_arg_1 = (A1);\
   CTabHandle __stub_arg_2 = (A2);\
   ReqListRec * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0249];\
   ((new_addr == toolstuff[0x0249].orig)\
    ? C_SaveEntries(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SaveEntries ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define RestoreEntries(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   CTabHandle __stub_arg_1 = (A1);\
   CTabHandle __stub_arg_2 = (A2);\
   ReqListRec * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x024a];\
   ((new_addr == toolstuff[0x024a].orig)\
    ? C_RestoreEntries(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_RestoreEntries ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SetCursor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Cursor * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0051];\
   ((new_addr == toolstuff[0x0051].orig)\
    ? C_SetCursor(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetCursor ,  __stub_arg_1));\
  })
#define InitCursor() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0050];\
   ((new_addr == toolstuff[0x0050].orig)\
    ? C_InitCursor()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InitCursor  ));\
  })
#define HideCursor() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0052];\
   ((new_addr == toolstuff[0x0052].orig)\
    ? C_HideCursor()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_HideCursor  ));\
  })
#define ShowCursor() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0053];\
   ((new_addr == toolstuff[0x0053].orig)\
    ? C_ShowCursor()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ShowCursor  ));\
  })
#define ObscureCursor() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0056];\
   ((new_addr == toolstuff[0x0056].orig)\
    ? C_ObscureCursor()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ObscureCursor  ));\
  })
#define ShieldCursor(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0055];\
   ((new_addr == toolstuff[0x0055].orig)\
    ? C_ShieldCursor(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ShieldCursor ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetCCursor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x021b];\
   ((new_addr == toolstuff[0x021b].orig)\
    ? C_GetCCursor(__stub_arg_1)\
    : (CCrsrHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetCCursor ,  __stub_arg_1));\
  })
#define SetCCursor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   CCrsrHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x021c];\
   ((new_addr == toolstuff[0x021c].orig)\
    ? C_SetCCursor(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetCCursor ,  __stub_arg_1));\
  })
#define DisposCCursor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   CCrsrHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0226];\
   ((new_addr == toolstuff[0x0226].orig)\
    ? C_DisposCCursor(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DisposCCursor ,  __stub_arg_1));\
  })
#define AllocCursor() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x021d];\
   ((new_addr == toolstuff[0x021d].orig)\
    ? C_AllocCursor()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_AllocCursor  ));\
  })
#define InitGraf(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x006e];\
   ((new_addr == toolstuff[0x006e].orig)\
    ? C_InitGraf(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InitGraf ,  __stub_arg_1));\
  })
#define SetPort(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   GrafPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0073];\
   ((new_addr == toolstuff[0x0073].orig)\
    ? C_SetPort(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetPort ,  __stub_arg_1));\
  })
#define InitPort(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   GrafPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x006d];\
   ((new_addr == toolstuff[0x006d].orig)\
    ? C_InitPort(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InitPort ,  __stub_arg_1));\
  })
#define OpenPort(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   GrafPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x006f];\
   ((new_addr == toolstuff[0x006f].orig)\
    ? C_OpenPort(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_OpenPort ,  __stub_arg_1));\
  })
#define ClosePort(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   GrafPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x007d];\
   ((new_addr == toolstuff[0x007d].orig)\
    ? C_ClosePort(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ClosePort ,  __stub_arg_1));\
  })
#define GetPort(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   GUEST<GrafPtr> * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0074];\
   ((new_addr == toolstuff[0x0074].orig)\
    ? C_GetPort(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetPort ,  __stub_arg_1));\
  })
#define GrafDevice(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0072];\
   ((new_addr == toolstuff[0x0072].orig)\
    ? C_GrafDevice(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GrafDevice ,  __stub_arg_1));\
  })
#define SetPortBits(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   BitMap * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0075];\
   ((new_addr == toolstuff[0x0075].orig)\
    ? C_SetPortBits(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetPortBits ,  __stub_arg_1));\
  })
#define PortSize(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0076];\
   ((new_addr == toolstuff[0x0076].orig)\
    ? C_PortSize(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PortSize ,  __stub_arg_1, __stub_arg_2));\
  })
#define MovePortTo(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0077];\
   ((new_addr == toolstuff[0x0077].orig)\
    ? C_MovePortTo(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_MovePortTo ,  __stub_arg_1, __stub_arg_2));\
  })
#define SetOrigin(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0078];\
   ((new_addr == toolstuff[0x0078].orig)\
    ? C_SetOrigin(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetOrigin ,  __stub_arg_1, __stub_arg_2));\
  })
#define SetClip(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0079];\
   ((new_addr == toolstuff[0x0079].orig)\
    ? C_SetClip(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetClip ,  __stub_arg_1));\
  })
#define GetClip(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x007a];\
   ((new_addr == toolstuff[0x007a].orig)\
    ? C_GetClip(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetClip ,  __stub_arg_1));\
  })
#define ClipRect(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x007b];\
   ((new_addr == toolstuff[0x007b].orig)\
    ? C_ClipRect(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ClipRect ,  __stub_arg_1));\
  })
#define BackPat(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Byte * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x007c];\
   ((new_addr == toolstuff[0x007c].orig)\
    ? C_BackPat(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_BackPat ,  __stub_arg_1));\
  })
#define SeedFill(A1, A2, A3, A4, A5, A6, A7, A8) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   Ptr __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   INTEGER __stub_arg_4 = (A4);\
   INTEGER __stub_arg_5 = (A5);\
   INTEGER __stub_arg_6 = (A6);\
   INTEGER __stub_arg_7 = (A7);\
   INTEGER __stub_arg_8 = (A8);\
 \
   new_addr = tooltraptable[0x0039];\
   ((new_addr == toolstuff[0x0039].orig)\
    ? C_SeedFill(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SeedFill ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8));\
  })
#define CalcMask(A1, A2, A3, A4, A5, A6) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   Ptr __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   INTEGER __stub_arg_4 = (A4);\
   INTEGER __stub_arg_5 = (A5);\
   INTEGER __stub_arg_6 = (A6);\
 \
   new_addr = tooltraptable[0x0038];\
   ((new_addr == toolstuff[0x0038].orig)\
    ? C_CalcMask(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CalcMask ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6));\
  })
#define CopyMask(A1, A2, A3, A4, A5, A6) \
 ({ \
   syn68k_addr_t new_addr;\
   BitMap * __stub_arg_1 = (A1);\
   BitMap * __stub_arg_2 = (A2);\
   BitMap * __stub_arg_3 = (A3);\
   Rect * __stub_arg_4 = (A4);\
   Rect * __stub_arg_5 = (A5);\
   Rect * __stub_arg_6 = (A6);\
 \
   new_addr = tooltraptable[0x0017];\
   ((new_addr == toolstuff[0x0017].orig)\
    ? C_CopyMask(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CopyMask ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6));\
  })
#define IMVI_CopyDeepMask(A1, A2, A3, A4, A5, A6, A7, A8) \
 ({ \
   syn68k_addr_t new_addr;\
   BitMap * __stub_arg_1 = (A1);\
   BitMap * __stub_arg_2 = (A2);\
   BitMap * __stub_arg_3 = (A3);\
   Rect * __stub_arg_4 = (A4);\
   Rect * __stub_arg_5 = (A5);\
   Rect * __stub_arg_6 = (A6);\
   INTEGER __stub_arg_7 = (A7);\
   RgnHandle __stub_arg_8 = (A8);\
 \
   new_addr = tooltraptable[0x0251];\
   ((new_addr == toolstuff[0x0251].orig)\
    ? C_IMVI_CopyDeepMask(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_IMVI_CopyDeepMask ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8));\
  })
#define CharExtra(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Fixed __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0223];\
   ((new_addr == toolstuff[0x0223].orig)\
    ? C_CharExtra(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CharExtra ,  __stub_arg_1));\
  })
#define SetStdCProcs(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   CQDProcs * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x024e];\
   ((new_addr == toolstuff[0x024e].orig)\
    ? C_SetStdCProcs(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetStdCProcs ,  __stub_arg_1));\
  })
#define GetCPixel(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   RGBColor * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0217];\
   ((new_addr == toolstuff[0x0217].orig)\
    ? C_GetCPixel(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetCPixel ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SetCPixel(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   RGBColor * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0216];\
   ((new_addr == toolstuff[0x0216].orig)\
    ? C_SetCPixel(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetCPixel ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SeedCFill(A1, A2, A3, A4, A5, A6, A7, A8) \
 ({ \
   syn68k_addr_t new_addr;\
   BitMap * __stub_arg_1 = (A1);\
   BitMap * __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
   Rect * __stub_arg_4 = (A4);\
   int16 __stub_arg_5 = (A5);\
   int16 __stub_arg_6 = (A6);\
   ProcPtr __stub_arg_7 = (A7);\
   int32 __stub_arg_8 = (A8);\
 \
   new_addr = tooltraptable[0x0250];\
   ((new_addr == toolstuff[0x0250].orig)\
    ? C_SeedCFill(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SeedCFill ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8));\
  })
#define CalcCMask(A1, A2, A3, A4, A5, A6, A7) \
 ({ \
   syn68k_addr_t new_addr;\
   BitMap * __stub_arg_1 = (A1);\
   BitMap * __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
   Rect * __stub_arg_4 = (A4);\
   RGBColor * __stub_arg_5 = (A5);\
   ProcPtr __stub_arg_6 = (A6);\
   int32 __stub_arg_7 = (A7);\
 \
   new_addr = tooltraptable[0x024f];\
   ((new_addr == toolstuff[0x024f].orig)\
    ? C_CalcCMask(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CalcCMask ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7));\
  })
#define Random() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0061];\
   ((new_addr == toolstuff[0x0061].orig)\
    ? C_Random()\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Random  ));\
  })
#define GetPixel(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0065];\
   ((new_addr == toolstuff[0x0065].orig)\
    ? C_GetPixel(__stub_arg_1, __stub_arg_2)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetPixel ,  __stub_arg_1, __stub_arg_2));\
  })
#define StuffHex(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   StringPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0066];\
   ((new_addr == toolstuff[0x0066].orig)\
    ? C_StuffHex(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StuffHex ,  __stub_arg_1, __stub_arg_2));\
  })
#define ScalePt(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   GUEST<Point> * __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00f8];\
   ((new_addr == toolstuff[0x00f8].orig)\
    ? C_ScalePt(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ScalePt ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define MapPt(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   GUEST<Point> * __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00f9];\
   ((new_addr == toolstuff[0x00f9].orig)\
    ? C_MapPt(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_MapPt ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define MapRect(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00fa];\
   ((new_addr == toolstuff[0x00fa].orig)\
    ? C_MapRect(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_MapRect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define MapRgn(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00fb];\
   ((new_addr == toolstuff[0x00fb].orig)\
    ? C_MapRgn(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_MapRgn ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define MapPoly(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   PolyHandle __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00fc];\
   ((new_addr == toolstuff[0x00fc].orig)\
    ? C_MapPoly(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_MapPoly ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define PMgrVersion() \
   (     C_PMgrVersion()     )
#define ActivatePalette(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0294];\
   ((new_addr == toolstuff[0x0294].orig)\
    ? C_ActivatePalette(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ActivatePalette ,  __stub_arg_1));\
  })
#define RestoreClutDevice(A1) \
   (     C_RestoreClutDevice((A1))     )
#define InitPalettes() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0290];\
   ((new_addr == toolstuff[0x0290].orig)\
    ? C_InitPalettes()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InitPalettes  ));\
  })
#define NewPalette(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   CTabHandle __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   INTEGER __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x0291];\
   ((new_addr == toolstuff[0x0291].orig)\
    ? C_NewPalette(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (PaletteHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_NewPalette ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define GetNewPalette(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0292];\
   ((new_addr == toolstuff[0x0292].orig)\
    ? C_GetNewPalette(__stub_arg_1)\
    : (PaletteHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetNewPalette ,  __stub_arg_1));\
  })
#define DisposePalette(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   PaletteHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0293];\
   ((new_addr == toolstuff[0x0293].orig)\
    ? C_DisposePalette(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DisposePalette ,  __stub_arg_1));\
  })
#define ResizePalette(A1, A2) \
   (     C_ResizePalette((A1), (A2))     )
#define NSetPalette(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   PaletteHandle __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0295];\
   ((new_addr == toolstuff[0x0295].orig)\
    ? C_NSetPalette(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_NSetPalette ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SetPaletteUpdates(A1, A2) \
   (     C_SetPaletteUpdates((A1), (A2))     )
#define GetPaletteUpdates(A1) \
   (     C_GetPaletteUpdates((A1))     )
#define GetPalette(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0296];\
   ((new_addr == toolstuff[0x0296].orig)\
    ? C_GetPalette(__stub_arg_1)\
    : (PaletteHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetPalette ,  __stub_arg_1));\
  })
#define PmForeColor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0297];\
   ((new_addr == toolstuff[0x0297].orig)\
    ? C_PmForeColor(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PmForeColor ,  __stub_arg_1));\
  })
#define PmBackColor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0298];\
   ((new_addr == toolstuff[0x0298].orig)\
    ? C_PmBackColor(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PmBackColor ,  __stub_arg_1));\
  })
#define SaveFore(A1) \
   (     C_SaveFore((A1))     )
#define RestoreFore(A1) \
   (     C_RestoreFore((A1))     )
#define SaveBack(A1) \
   (     C_SaveBack((A1))     )
#define RestoreBack(A1) \
   (     C_RestoreBack((A1))     )
#define AnimateEntry(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   RGBColor * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0299];\
   ((new_addr == toolstuff[0x0299].orig)\
    ? C_AnimateEntry(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_AnimateEntry ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define AnimatePalette(A1, A2, A3, A4, A5) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   CTabHandle __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   INTEGER __stub_arg_4 = (A4);\
   INTEGER __stub_arg_5 = (A5);\
 \
   new_addr = tooltraptable[0x029a];\
   ((new_addr == toolstuff[0x029a].orig)\
    ? C_AnimatePalette(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_AnimatePalette ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5));\
  })
#define GetEntryColor(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   PaletteHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   RGBColor * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x029b];\
   ((new_addr == toolstuff[0x029b].orig)\
    ? C_GetEntryColor(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetEntryColor ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SetEntryColor(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   PaletteHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   RGBColor * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x029c];\
   ((new_addr == toolstuff[0x029c].orig)\
    ? C_SetEntryColor(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetEntryColor ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define GetEntryUsage(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   PaletteHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   GUEST<INTEGER> * __stub_arg_3 = (A3);\
   GUEST<INTEGER> * __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x029d];\
   ((new_addr == toolstuff[0x029d].orig)\
    ? C_GetEntryUsage(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetEntryUsage ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define SetEntryUsage(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   PaletteHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   INTEGER __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x029e];\
   ((new_addr == toolstuff[0x029e].orig)\
    ? C_SetEntryUsage(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetEntryUsage ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define CTab2Palette(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   CTabHandle __stub_arg_1 = (A1);\
   PaletteHandle __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   INTEGER __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x029f];\
   ((new_addr == toolstuff[0x029f].orig)\
    ? C_CTab2Palette(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CTab2Palette ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define Palette2CTab(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   PaletteHandle __stub_arg_1 = (A1);\
   CTabHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x02a0];\
   ((new_addr == toolstuff[0x02a0].orig)\
    ? C_Palette2CTab(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Palette2CTab ,  __stub_arg_1, __stub_arg_2));\
  })
#define Entry2Index(A1) \
   (     C_Entry2Index((A1))     )
#define CopyPalette(A1, A2, A3, A4, A5) \
 ({ \
   syn68k_addr_t new_addr;\
   PaletteHandle __stub_arg_1 = (A1);\
   PaletteHandle __stub_arg_2 = (A2);\
   int16 __stub_arg_3 = (A3);\
   int16 __stub_arg_4 = (A4);\
   int16 __stub_arg_5 = (A5);\
 \
   new_addr = tooltraptable[0x02a1];\
   ((new_addr == toolstuff[0x02a1].orig)\
    ? C_CopyPalette(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CopyPalette ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5));\
  })
#define HidePen() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0096];\
   ((new_addr == toolstuff[0x0096].orig)\
    ? C_HidePen()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_HidePen  ));\
  })
#define ShowPen() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0097];\
   ((new_addr == toolstuff[0x0097].orig)\
    ? C_ShowPen()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ShowPen  ));\
  })
#define GetPen(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Point * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x009a];\
   ((new_addr == toolstuff[0x009a].orig)\
    ? C_GetPen(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetPen ,  __stub_arg_1));\
  })
#define GetPenState(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   PenState * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0098];\
   ((new_addr == toolstuff[0x0098].orig)\
    ? C_GetPenState(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetPenState ,  __stub_arg_1));\
  })
#define SetPenState(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   PenState * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0099];\
   ((new_addr == toolstuff[0x0099].orig)\
    ? C_SetPenState(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetPenState ,  __stub_arg_1));\
  })
#define PenSize(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x009b];\
   ((new_addr == toolstuff[0x009b].orig)\
    ? C_PenSize(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PenSize ,  __stub_arg_1, __stub_arg_2));\
  })
#define PenMode(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x009c];\
   ((new_addr == toolstuff[0x009c].orig)\
    ? C_PenMode(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PenMode ,  __stub_arg_1));\
  })
#define PenPat(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Byte * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x009d];\
   ((new_addr == toolstuff[0x009d].orig)\
    ? C_PenPat(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PenPat ,  __stub_arg_1));\
  })
#define PenNormal() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x009e];\
   ((new_addr == toolstuff[0x009e].orig)\
    ? C_PenNormal()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PenNormal  ));\
  })
#define MoveTo(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0093];\
   ((new_addr == toolstuff[0x0093].orig)\
    ? C_MoveTo(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_MoveTo ,  __stub_arg_1, __stub_arg_2));\
  })
#define Move(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0094];\
   ((new_addr == toolstuff[0x0094].orig)\
    ? C_Move(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Move ,  __stub_arg_1, __stub_arg_2));\
  })
#define LineTo(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0091];\
   ((new_addr == toolstuff[0x0091].orig)\
    ? C_LineTo(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_LineTo ,  __stub_arg_1, __stub_arg_2));\
  })
#define Line(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0092];\
   ((new_addr == toolstuff[0x0092].orig)\
    ? C_Line(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Line ,  __stub_arg_1, __stub_arg_2));\
  })
#define DrawPicture(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   PicHandle __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00f6];\
   ((new_addr == toolstuff[0x00f6].orig)\
    ? C_DrawPicture(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DrawPicture ,  __stub_arg_1, __stub_arg_2));\
  })
#define OpenPicture(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00f3];\
   ((new_addr == toolstuff[0x00f3].orig)\
    ? C_OpenPicture(__stub_arg_1)\
    : (PicHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_OpenPicture ,  __stub_arg_1));\
  })
#define ClosePicture() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x00f4];\
   ((new_addr == toolstuff[0x00f4].orig)\
    ? C_ClosePicture()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ClosePicture  ));\
  })
#define PicComment(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   Handle __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00f2];\
   ((new_addr == toolstuff[0x00f2].orig)\
    ? C_PicComment(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PicComment ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define KillPicture(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   PicHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00f5];\
   ((new_addr == toolstuff[0x00f5].orig)\
    ? C_KillPicture(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_KillPicture ,  __stub_arg_1));\
  })
#define AddPt(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Point __stub_arg_1 = (A1);\
   GUEST<Point> * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x007e];\
   ((new_addr == toolstuff[0x007e].orig)\
    ? C_AddPt(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_AddPt ,  __stub_arg_1, __stub_arg_2));\
  })
#define SubPt(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Point __stub_arg_1 = (A1);\
   GUEST<Point> * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x007f];\
   ((new_addr == toolstuff[0x007f].orig)\
    ? C_SubPt(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SubPt ,  __stub_arg_1, __stub_arg_2));\
  })
#define SetPt(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   GUEST<Point> * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0080];\
   ((new_addr == toolstuff[0x0080].orig)\
    ? C_SetPt(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetPt ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define EqualPt(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Point __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0081];\
   ((new_addr == toolstuff[0x0081].orig)\
    ? C_EqualPt(__stub_arg_1, __stub_arg_2)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_EqualPt ,  __stub_arg_1, __stub_arg_2));\
  })
#define LocalToGlobal(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   GUEST<Point> * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0070];\
   ((new_addr == toolstuff[0x0070].orig)\
    ? C_LocalToGlobal(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_LocalToGlobal ,  __stub_arg_1));\
  })
#define GlobalToLocal(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   GUEST<Point> * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0071];\
   ((new_addr == toolstuff[0x0071].orig)\
    ? C_GlobalToLocal(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GlobalToLocal ,  __stub_arg_1));\
  })
#define OpenPoly() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x00cb];\
   ((new_addr == toolstuff[0x00cb].orig)\
    ? C_OpenPoly()\
    : (PolyHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_OpenPoly  ));\
  })
#define ClosePoly() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x00cc];\
   ((new_addr == toolstuff[0x00cc].orig)\
    ? C_ClosePoly()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ClosePoly  ));\
  })
#define KillPoly(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   PolyHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00cd];\
   ((new_addr == toolstuff[0x00cd].orig)\
    ? C_KillPoly(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_KillPoly ,  __stub_arg_1));\
  })
#define OffsetPoly(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   PolyHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00ce];\
   ((new_addr == toolstuff[0x00ce].orig)\
    ? C_OffsetPoly(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_OffsetPoly ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SetRect(A1, A2, A3, A4, A5) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   INTEGER __stub_arg_4 = (A4);\
   INTEGER __stub_arg_5 = (A5);\
 \
   new_addr = tooltraptable[0x00a7];\
   ((new_addr == toolstuff[0x00a7].orig)\
    ? C_SetRect(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetRect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5));\
  })
#define OffsetRect(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00a8];\
   ((new_addr == toolstuff[0x00a8].orig)\
    ? C_OffsetRect(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_OffsetRect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define InsetRect(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00a9];\
   ((new_addr == toolstuff[0x00a9].orig)\
    ? C_InsetRect(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InsetRect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SectRect(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   const Rect * __stub_arg_1 = (A1);\
   const Rect * __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00aa];\
   ((new_addr == toolstuff[0x00aa].orig)\
    ? C_SectRect(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SectRect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define EmptyRect(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00ae];\
   ((new_addr == toolstuff[0x00ae].orig)\
    ? C_EmptyRect(__stub_arg_1)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_EmptyRect ,  __stub_arg_1));\
  })
#define UnionRect(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00ab];\
   ((new_addr == toolstuff[0x00ab].orig)\
    ? C_UnionRect(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_UnionRect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define PtInRect(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Point __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00ad];\
   ((new_addr == toolstuff[0x00ad].orig)\
    ? C_PtInRect(__stub_arg_1, __stub_arg_2)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PtInRect ,  __stub_arg_1, __stub_arg_2));\
  })
#define Pt2Rect(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Point __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00ac];\
   ((new_addr == toolstuff[0x00ac].orig)\
    ? C_Pt2Rect(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Pt2Rect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define PtToAngle(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
   GUEST<INTEGER> * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00c3];\
   ((new_addr == toolstuff[0x00c3].orig)\
    ? C_PtToAngle(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PtToAngle ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define EqualRect(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   const Rect * __stub_arg_1 = (A1);\
   const Rect * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00a6];\
   ((new_addr == toolstuff[0x00a6].orig)\
    ? C_EqualRect(__stub_arg_1, __stub_arg_2)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_EqualRect ,  __stub_arg_1, __stub_arg_2));\
  })
#define NewRgn() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x00d8];\
   ((new_addr == toolstuff[0x00d8].orig)\
    ? C_NewRgn()\
    : (RgnHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_NewRgn  ));\
  })
#define OpenRgn() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x00da];\
   ((new_addr == toolstuff[0x00da].orig)\
    ? C_OpenRgn()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_OpenRgn  ));\
  })
#define CopyRgn(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   RgnHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00dc];\
   ((new_addr == toolstuff[0x00dc].orig)\
    ? C_CopyRgn(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CopyRgn ,  __stub_arg_1, __stub_arg_2));\
  })
#define CloseRgn(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00db];\
   ((new_addr == toolstuff[0x00db].orig)\
    ? C_CloseRgn(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CloseRgn ,  __stub_arg_1));\
  })
#define DisposeRgn(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00d9];\
   ((new_addr == toolstuff[0x00d9].orig)\
    ? C_DisposeRgn(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DisposeRgn ,  __stub_arg_1));\
  })
#define SetEmptyRgn(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00dd];\
   ((new_addr == toolstuff[0x00dd].orig)\
    ? C_SetEmptyRgn(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetEmptyRgn ,  __stub_arg_1));\
  })
#define SetRectRgn(A1, A2, A3, A4, A5) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   INTEGER __stub_arg_4 = (A4);\
   INTEGER __stub_arg_5 = (A5);\
 \
   new_addr = tooltraptable[0x00de];\
   ((new_addr == toolstuff[0x00de].orig)\
    ? C_SetRectRgn(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetRectRgn ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5));\
  })
#define RectRgn(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00df];\
   ((new_addr == toolstuff[0x00df].orig)\
    ? C_RectRgn(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_RectRgn ,  __stub_arg_1, __stub_arg_2));\
  })
#define OffsetRgn(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00e0];\
   ((new_addr == toolstuff[0x00e0].orig)\
    ? C_OffsetRgn(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_OffsetRgn ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define PtInRgn(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Point __stub_arg_1 = (A1);\
   RgnHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00e8];\
   ((new_addr == toolstuff[0x00e8].orig)\
    ? C_PtInRgn(__stub_arg_1, __stub_arg_2)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PtInRgn ,  __stub_arg_1, __stub_arg_2));\
  })
#define InsetRgn(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00e1];\
   ((new_addr == toolstuff[0x00e1].orig)\
    ? C_InsetRgn(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InsetRgn ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SectRgn(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   RgnHandle __stub_arg_2 = (A2);\
   RgnHandle __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00e4];\
   ((new_addr == toolstuff[0x00e4].orig)\
    ? C_SectRgn(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SectRgn ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define UnionRgn(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   RgnHandle __stub_arg_2 = (A2);\
   RgnHandle __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00e5];\
   ((new_addr == toolstuff[0x00e5].orig)\
    ? C_UnionRgn(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_UnionRgn ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define DiffRgn(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   RgnHandle __stub_arg_2 = (A2);\
   RgnHandle __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00e6];\
   ((new_addr == toolstuff[0x00e6].orig)\
    ? C_DiffRgn(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DiffRgn ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define XorRgn(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   RgnHandle __stub_arg_2 = (A2);\
   RgnHandle __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00e7];\
   ((new_addr == toolstuff[0x00e7].orig)\
    ? C_XorRgn(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_XorRgn ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define RectInRgn(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   RgnHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00e9];\
   ((new_addr == toolstuff[0x00e9].orig)\
    ? C_RectInRgn(__stub_arg_1, __stub_arg_2)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_RectInRgn ,  __stub_arg_1, __stub_arg_2));\
  })
#define EqualRgn(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   RgnHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00e3];\
   ((new_addr == toolstuff[0x00e3].orig)\
    ? C_EqualRgn(__stub_arg_1, __stub_arg_2)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_EqualRgn ,  __stub_arg_1, __stub_arg_2));\
  })
#define EmptyRgn(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00e2];\
   ((new_addr == toolstuff[0x00e2].orig)\
    ? C_EmptyRgn(__stub_arg_1)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_EmptyRgn ,  __stub_arg_1));\
  })
#define FrameRect(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00a1];\
   ((new_addr == toolstuff[0x00a1].orig)\
    ? C_FrameRect(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FrameRect ,  __stub_arg_1));\
  })
#define PaintRect(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00a2];\
   ((new_addr == toolstuff[0x00a2].orig)\
    ? C_PaintRect(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PaintRect ,  __stub_arg_1));\
  })
#define EraseRect(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00a3];\
   ((new_addr == toolstuff[0x00a3].orig)\
    ? C_EraseRect(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_EraseRect ,  __stub_arg_1));\
  })
#define InvertRect(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00a4];\
   ((new_addr == toolstuff[0x00a4].orig)\
    ? C_InvertRect(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InvertRect ,  __stub_arg_1));\
  })
#define FillRect(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   Byte * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00a5];\
   ((new_addr == toolstuff[0x00a5].orig)\
    ? C_FillRect(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FillRect ,  __stub_arg_1, __stub_arg_2));\
  })
#define FrameOval(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00b7];\
   ((new_addr == toolstuff[0x00b7].orig)\
    ? C_FrameOval(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FrameOval ,  __stub_arg_1));\
  })
#define PaintOval(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00b8];\
   ((new_addr == toolstuff[0x00b8].orig)\
    ? C_PaintOval(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PaintOval ,  __stub_arg_1));\
  })
#define EraseOval(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00b9];\
   ((new_addr == toolstuff[0x00b9].orig)\
    ? C_EraseOval(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_EraseOval ,  __stub_arg_1));\
  })
#define InvertOval(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00ba];\
   ((new_addr == toolstuff[0x00ba].orig)\
    ? C_InvertOval(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InvertOval ,  __stub_arg_1));\
  })
#define FillOval(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   Byte * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00bb];\
   ((new_addr == toolstuff[0x00bb].orig)\
    ? C_FillOval(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FillOval ,  __stub_arg_1, __stub_arg_2));\
  })
#define FrameRoundRect(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00b0];\
   ((new_addr == toolstuff[0x00b0].orig)\
    ? C_FrameRoundRect(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FrameRoundRect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define PaintRoundRect(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00b1];\
   ((new_addr == toolstuff[0x00b1].orig)\
    ? C_PaintRoundRect(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PaintRoundRect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define EraseRoundRect(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00b2];\
   ((new_addr == toolstuff[0x00b2].orig)\
    ? C_EraseRoundRect(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_EraseRoundRect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define InvertRoundRect(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00b3];\
   ((new_addr == toolstuff[0x00b3].orig)\
    ? C_InvertRoundRect(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InvertRoundRect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define FillRoundRect(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   Byte * __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x00b4];\
   ((new_addr == toolstuff[0x00b4].orig)\
    ? C_FillRoundRect(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FillRoundRect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define FrameArc(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00be];\
   ((new_addr == toolstuff[0x00be].orig)\
    ? C_FrameArc(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FrameArc ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define PaintArc(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00bf];\
   ((new_addr == toolstuff[0x00bf].orig)\
    ? C_PaintArc(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PaintArc ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define EraseArc(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00c0];\
   ((new_addr == toolstuff[0x00c0].orig)\
    ? C_EraseArc(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_EraseArc ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define InvertArc(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00c1];\
   ((new_addr == toolstuff[0x00c1].orig)\
    ? C_InvertArc(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InvertArc ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define FillArc(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   Byte * __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x00c2];\
   ((new_addr == toolstuff[0x00c2].orig)\
    ? C_FillArc(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FillArc ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define FrameRgn(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00d2];\
   ((new_addr == toolstuff[0x00d2].orig)\
    ? C_FrameRgn(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FrameRgn ,  __stub_arg_1));\
  })
#define PaintRgn(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00d3];\
   ((new_addr == toolstuff[0x00d3].orig)\
    ? C_PaintRgn(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PaintRgn ,  __stub_arg_1));\
  })
#define EraseRgn(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00d4];\
   ((new_addr == toolstuff[0x00d4].orig)\
    ? C_EraseRgn(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_EraseRgn ,  __stub_arg_1));\
  })
#define InvertRgn(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00d5];\
   ((new_addr == toolstuff[0x00d5].orig)\
    ? C_InvertRgn(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InvertRgn ,  __stub_arg_1));\
  })
#define FillRgn(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   Byte * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00d6];\
   ((new_addr == toolstuff[0x00d6].orig)\
    ? C_FillRgn(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FillRgn ,  __stub_arg_1, __stub_arg_2));\
  })
#define FramePoly(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   PolyHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00c6];\
   ((new_addr == toolstuff[0x00c6].orig)\
    ? C_FramePoly(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FramePoly ,  __stub_arg_1));\
  })
#define PaintPoly(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   PolyHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00c7];\
   ((new_addr == toolstuff[0x00c7].orig)\
    ? C_PaintPoly(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PaintPoly ,  __stub_arg_1));\
  })
#define ErasePoly(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   PolyHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00c8];\
   ((new_addr == toolstuff[0x00c8].orig)\
    ? C_ErasePoly(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ErasePoly ,  __stub_arg_1));\
  })
#define InvertPoly(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   PolyHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00c9];\
   ((new_addr == toolstuff[0x00c9].orig)\
    ? C_InvertPoly(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InvertPoly ,  __stub_arg_1));\
  })
#define FillPoly(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   PolyHandle __stub_arg_1 = (A1);\
   Byte * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00ca];\
   ((new_addr == toolstuff[0x00ca].orig)\
    ? C_FillPoly(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FillPoly ,  __stub_arg_1, __stub_arg_2));\
  })
#define SetStdProcs(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   QDProcs * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00ea];\
   ((new_addr == toolstuff[0x00ea].orig)\
    ? C_SetStdProcs(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetStdProcs ,  __stub_arg_1));\
  })
#define StdArc(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   GrafVerb __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   INTEGER __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x00bd];\
   ((new_addr == toolstuff[0x00bd].orig)\
    ? C_StdArc(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StdArc ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define StdBits(A1, A2, A3, A4, A5) \
 ({ \
   syn68k_addr_t new_addr;\
   BitMap * __stub_arg_1 = (A1);\
   const Rect * __stub_arg_2 = (A2);\
   const Rect * __stub_arg_3 = (A3);\
   INTEGER __stub_arg_4 = (A4);\
   RgnHandle __stub_arg_5 = (A5);\
 \
   new_addr = tooltraptable[0x00eb];\
   ((new_addr == toolstuff[0x00eb].orig)\
    ? C_StdBits(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StdBits ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5));\
  })
#define StdLine(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Point __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0090];\
   ((new_addr == toolstuff[0x0090].orig)\
    ? C_StdLine(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StdLine ,  __stub_arg_1));\
  })
#define StdOval(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   GrafVerb __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00b6];\
   ((new_addr == toolstuff[0x00b6].orig)\
    ? C_StdOval(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StdOval ,  __stub_arg_1, __stub_arg_2));\
  })
#define StdComment(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   Handle __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00f1];\
   ((new_addr == toolstuff[0x00f1].orig)\
    ? C_StdComment(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StdComment ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define StdGetPic(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00ee];\
   ((new_addr == toolstuff[0x00ee].orig)\
    ? C_StdGetPic(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StdGetPic ,  __stub_arg_1, __stub_arg_2));\
  })
#define StdPutPic(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00f0];\
   ((new_addr == toolstuff[0x00f0].orig)\
    ? C_StdPutPic(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StdPutPic ,  __stub_arg_1, __stub_arg_2));\
  })
#define StdPoly(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   GrafVerb __stub_arg_1 = (A1);\
   PolyHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00c5];\
   ((new_addr == toolstuff[0x00c5].orig)\
    ? C_StdPoly(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StdPoly ,  __stub_arg_1, __stub_arg_2));\
  })
#define StdRRect(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   GrafVerb __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   INTEGER __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x00af];\
   ((new_addr == toolstuff[0x00af].orig)\
    ? C_StdRRect(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StdRRect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define StdRect(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   GrafVerb __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00a0];\
   ((new_addr == toolstuff[0x00a0].orig)\
    ? C_StdRect(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StdRect ,  __stub_arg_1, __stub_arg_2));\
  })
#define StdRgn(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   GrafVerb __stub_arg_1 = (A1);\
   RgnHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00d1];\
   ((new_addr == toolstuff[0x00d1].orig)\
    ? C_StdRgn(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StdRgn ,  __stub_arg_1, __stub_arg_2));\
  })
#define StdText(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   Ptr __stub_arg_2 = (A2);\
   Point __stub_arg_3 = (A3);\
   Point __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x0082];\
   ((new_addr == toolstuff[0x0082].orig)\
    ? C_StdText(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StdText ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define StdTxMeas(A1, A2, A3, A4, A5) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   Ptr __stub_arg_2 = (A2);\
   GUEST<Point> * __stub_arg_3 = (A3);\
   GUEST<Point> * __stub_arg_4 = (A4);\
   FontInfo * __stub_arg_5 = (A5);\
 \
   new_addr = tooltraptable[0x00ed];\
   ((new_addr == toolstuff[0x00ed].orig)\
    ? C_StdTxMeas(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StdTxMeas ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5));\
  })
#define MeasureText(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   Ptr __stub_arg_2 = (A2);\
   Ptr __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0037];\
   ((new_addr == toolstuff[0x0037].orig)\
    ? C_MeasureText(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_MeasureText ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define TextFont(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0087];\
   ((new_addr == toolstuff[0x0087].orig)\
    ? C_TextFont(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TextFont ,  __stub_arg_1));\
  })
#define TextFace(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0088];\
   ((new_addr == toolstuff[0x0088].orig)\
    ? C_TextFace(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TextFace ,  __stub_arg_1));\
  })
#define TextMode(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0089];\
   ((new_addr == toolstuff[0x0089].orig)\
    ? C_TextMode(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TextMode ,  __stub_arg_1));\
  })
#define TextSize(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x008a];\
   ((new_addr == toolstuff[0x008a].orig)\
    ? C_TextSize(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TextSize ,  __stub_arg_1));\
  })
#define SpaceExtra(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Fixed __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x008e];\
   ((new_addr == toolstuff[0x008e].orig)\
    ? C_SpaceExtra(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SpaceExtra ,  __stub_arg_1));\
  })
#define DrawChar(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   CHAR __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0083];\
   ((new_addr == toolstuff[0x0083].orig)\
    ? C_DrawChar(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DrawChar ,  __stub_arg_1));\
  })
#define DrawString(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   StringPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0084];\
   ((new_addr == toolstuff[0x0084].orig)\
    ? C_DrawString(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DrawString ,  __stub_arg_1));\
  })
#define DrawText(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0085];\
   ((new_addr == toolstuff[0x0085].orig)\
    ? C_DrawText(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DrawText ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define CharWidth(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   CHAR __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x008d];\
   ((new_addr == toolstuff[0x008d].orig)\
    ? C_CharWidth(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CharWidth ,  __stub_arg_1));\
  })
#define StringWidth(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   StringPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x008c];\
   ((new_addr == toolstuff[0x008c].orig)\
    ? C_StringWidth(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StringWidth ,  __stub_arg_1));\
  })
#define TextWidth(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0086];\
   ((new_addr == toolstuff[0x0086].orig)\
    ? C_TextWidth(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TextWidth ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define GetFontInfo(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   FontInfo * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x008b];\
   ((new_addr == toolstuff[0x008b].orig)\
    ? C_GetFontInfo(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetFontInfo ,  __stub_arg_1));\
  })
#define NewGWorld(A1, A2, A3, A4, A5, A6) \
   (     C_NewGWorld((A1), (A2), (A3), (A4), (A5), (A6))     )
#define LockPixels(A1) \
   (     C_LockPixels((A1))     )
#define UnlockPixels(A1) \
   (     C_UnlockPixels((A1))     )
#define UpdateGWorld(A1, A2, A3, A4, A5, A6) \
   (     C_UpdateGWorld((A1), (A2), (A3), (A4), (A5), (A6))     )
#define DisposeGWorld(A1) \
   (     C_DisposeGWorld((A1))     )
#define GetGWorld(A1, A2) \
   (     C_GetGWorld((A1), (A2))     )
#define SetGWorld(A1, A2) \
   (     C_SetGWorld((A1), (A2))     )
#define AllowPurgePixels(A1) \
   (     C_AllowPurgePixels((A1))     )
#define NoPurgePixels(A1) \
   (     C_NoPurgePixels((A1))     )
#define GetPixelsState(A1) \
   (     C_GetPixelsState((A1))     )
#define SetPixelsState(A1, A2) \
   (     C_SetPixelsState((A1), (A2))     )
#define GetPixBaseAddr(A1) \
   (     C_GetPixBaseAddr((A1))     )
#define NewScreenBuffer(A1, A2, A3, A4) \
   (     C_NewScreenBuffer((A1), (A2), (A3), (A4))     )
#define DisposeScreenBuffer(A1) \
   (     C_DisposeScreenBuffer((A1))     )
#define GetGWorldDevice(A1) \
   (     C_GetGWorldDevice((A1))     )
#define PixMap32Bit(A1) \
   (     C_PixMap32Bit((A1))     )
#define GetGWorldPixMap(A1) \
   (     C_GetGWorldPixMap((A1))     )
#define NewTempScreenBuffer(A1, A2, A3, A4) \
   (     C_NewTempScreenBuffer((A1), (A2), (A3), (A4))     )
#define OffscreenVersion() \
   (     C_OffscreenVersion()     )
#define GDeviceChanged(A1) \
   (     C_GDeviceChanged((A1))     )
#define PortChanged(A1) \
   (     C_PortChanged((A1))     )
#define PixPatChanged(A1) \
   (     C_PixPatChanged((A1))     )
#define CTabChanged(A1) \
   (     C_CTabChanged((A1))     )
#define QDDone(A1) \
   (     C_QDDone((A1))     )
#define NewGDevice(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x022f];\
   ((new_addr == toolstuff[0x022f].orig)\
    ? C_NewGDevice(__stub_arg_1, __stub_arg_2)\
    : (GDHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_NewGDevice ,  __stub_arg_1, __stub_arg_2));\
  })
#define InitGDevice(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
   GDHandle __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x022e];\
   ((new_addr == toolstuff[0x022e].orig)\
    ? C_InitGDevice(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InitGDevice ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SetDeviceAttribute(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   GDHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   BOOLEAN __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x022d];\
   ((new_addr == toolstuff[0x022d].orig)\
    ? C_SetDeviceAttribute(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetDeviceAttribute ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SetGDevice(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   GDHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0231];\
   ((new_addr == toolstuff[0x0231].orig)\
    ? C_SetGDevice(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetGDevice ,  __stub_arg_1));\
  })
#define DisposGDevice(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   GDHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0230];\
   ((new_addr == toolstuff[0x0230].orig)\
    ? C_DisposGDevice(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DisposGDevice ,  __stub_arg_1));\
  })
#define GetGDevice() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0232];\
   ((new_addr == toolstuff[0x0232].orig)\
    ? C_GetGDevice()\
    : (GDHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetGDevice  ));\
  })
#define GetDeviceList() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0229];\
   ((new_addr == toolstuff[0x0229].orig)\
    ? C_GetDeviceList()\
    : (GDHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetDeviceList  ));\
  })
#define GetMainDevice() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x022a];\
   ((new_addr == toolstuff[0x022a].orig)\
    ? C_GetMainDevice()\
    : (GDHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetMainDevice  ));\
  })
#define GetMaxDevice(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0227];\
   ((new_addr == toolstuff[0x0227].orig)\
    ? C_GetMaxDevice(__stub_arg_1)\
    : (GDHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetMaxDevice ,  __stub_arg_1));\
  })
#define GetNextDevice(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   GDHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x022b];\
   ((new_addr == toolstuff[0x022b].orig)\
    ? C_GetNextDevice(__stub_arg_1)\
    : (GDHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetNextDevice ,  __stub_arg_1));\
  })
#define DeviceLoop(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   DeviceLoopDrawingProcPtr __stub_arg_2 = (A2);\
   LONGINT __stub_arg_3 = (A3);\
   DeviceLoopFlags __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x03ca];\
   ((new_addr == toolstuff[0x03ca].orig)\
    ? C_DeviceLoop(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DeviceLoop ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define TestDeviceAttribute(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   GDHandle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x022c];\
   ((new_addr == toolstuff[0x022c].orig)\
    ? C_TestDeviceAttribute(__stub_arg_1, __stub_arg_2)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TestDeviceAttribute ,  __stub_arg_1, __stub_arg_2));\
  })
#define HasDepth(A1, A2, A3, A4) \
   (     C_HasDepth((A1), (A2), (A3), (A4))     )
#define SetDepth(A1, A2, A3, A4) \
   (     C_SetDepth((A1), (A2), (A3), (A4))     )
#define BitMapToRegion(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   const BitMap * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00d7];\
   ((new_addr == toolstuff[0x00d7].orig)\
    ? C_BitMapToRegion(__stub_arg_1, __stub_arg_2)\
    : (OSErr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_BitMapToRegion ,  __stub_arg_1, __stub_arg_2));\
  })
#define OpenCPicture(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   OpenCPicParams * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0220];\
   ((new_addr == toolstuff[0x0220].orig)\
    ? C_OpenCPicture(__stub_arg_1)\
    : (PicHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_OpenCPicture ,  __stub_arg_1));\
  })
#define GetColor(A1, A2, A3, A4) \
   (     C_GetColor((A1), (A2), (A3), (A4))     )
#define DisposePictInfo(A1) \
   (     C_DisposePictInfo((A1))     )
#define RecordPictInfo(A1, A2) \
   (     C_RecordPictInfo((A1), (A2))     )
#define RecordPixMapInfo(A1, A2) \
   (     C_RecordPixMapInfo((A1), (A2))     )
#define RetrievePictInfo(A1, A2, A3) \
   (     C_RetrievePictInfo((A1), (A2), (A3))     )
#define NewPictInfo(A1, A2, A3, A4, A5) \
   (     C_NewPictInfo((A1), (A2), (A3), (A4), (A5))     )
#define GetPictInfo(A1, A2, A3, A4, A5, A6) \
   (     C_GetPictInfo((A1), (A2), (A3), (A4), (A5), (A6))     )
#define GetPixMapInfo(A1, A2, A3, A4, A5, A6) \
   (     C_GetPixMapInfo((A1), (A2), (A3), (A4), (A5), (A6))     )
#define SetResLoad(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   BOOLEAN __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x019b];\
   ((new_addr == toolstuff[0x019b].orig)\
    ? C_SetResLoad(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetResLoad ,  __stub_arg_1));\
  })
#define CountResources(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ResType __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x019c];\
   ((new_addr == toolstuff[0x019c].orig)\
    ? C_CountResources(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CountResources ,  __stub_arg_1));\
  })
#define Count1Resources(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ResType __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x000d];\
   ((new_addr == toolstuff[0x000d].orig)\
    ? C_Count1Resources(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Count1Resources ,  __stub_arg_1));\
  })
#define GetIndResource(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ResType __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x019d];\
   ((new_addr == toolstuff[0x019d].orig)\
    ? C_GetIndResource(__stub_arg_1, __stub_arg_2)\
    : (Handle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetIndResource ,  __stub_arg_1, __stub_arg_2));\
  })
#define Get1IndResource(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ResType __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x000e];\
   ((new_addr == toolstuff[0x000e].orig)\
    ? C_Get1IndResource(__stub_arg_1, __stub_arg_2)\
    : (Handle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Get1IndResource ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetResource(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ResType __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x01a0];\
   ((new_addr == toolstuff[0x01a0].orig)\
    ? C_GetResource(__stub_arg_1, __stub_arg_2)\
    : (Handle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetResource ,  __stub_arg_1, __stub_arg_2));\
  })
#define Get1Resource(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ResType __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x001f];\
   ((new_addr == toolstuff[0x001f].orig)\
    ? C_Get1Resource(__stub_arg_1, __stub_arg_2)\
    : (Handle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Get1Resource ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetNamedResource(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ResType __stub_arg_1 = (A1);\
   StringPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x01a1];\
   ((new_addr == toolstuff[0x01a1].orig)\
    ? C_GetNamedResource(__stub_arg_1, __stub_arg_2)\
    : (Handle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetNamedResource ,  __stub_arg_1, __stub_arg_2));\
  })
#define Get1NamedResource(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ResType __stub_arg_1 = (A1);\
   StringPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0020];\
   ((new_addr == toolstuff[0x0020].orig)\
    ? C_Get1NamedResource(__stub_arg_1, __stub_arg_2)\
    : (Handle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Get1NamedResource ,  __stub_arg_1, __stub_arg_2));\
  })
#define LoadResource(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle volatile __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01a2];\
   ((new_addr == toolstuff[0x01a2].orig)\
    ? C_LoadResource(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_LoadResource ,  __stub_arg_1));\
  })
#define ReleaseResource(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01a3];\
   ((new_addr == toolstuff[0x01a3].orig)\
    ? C_ReleaseResource(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ReleaseResource ,  __stub_arg_1));\
  })
#define DetachResource(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0192];\
   ((new_addr == toolstuff[0x0192].orig)\
    ? C_DetachResource(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DetachResource ,  __stub_arg_1));\
  })
#define UniqueID(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ResType __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01c1];\
   ((new_addr == toolstuff[0x01c1].orig)\
    ? C_UniqueID(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_UniqueID ,  __stub_arg_1));\
  })
#define Unique1ID(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   ResType __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0010];\
   ((new_addr == toolstuff[0x0010].orig)\
    ? C_Unique1ID(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Unique1ID ,  __stub_arg_1));\
  })
#define GetResInfo(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
   GUEST<INTEGER> * __stub_arg_2 = (A2);\
   GUEST<ResType> * __stub_arg_3 = (A3);\
   StringPtr __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x01a8];\
   ((new_addr == toolstuff[0x01a8].orig)\
    ? C_GetResInfo(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetResInfo ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define GetResAttrs(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01a6];\
   ((new_addr == toolstuff[0x01a6].orig)\
    ? C_GetResAttrs(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetResAttrs ,  __stub_arg_1));\
  })
#define SizeResource(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01a5];\
   ((new_addr == toolstuff[0x01a5].orig)\
    ? C_SizeResource(__stub_arg_1)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SizeResource ,  __stub_arg_1));\
  })
#define CountTypes() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x019e];\
   ((new_addr == toolstuff[0x019e].orig)\
    ? C_CountTypes()\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CountTypes  ));\
  })
#define Count1Types() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x001c];\
   ((new_addr == toolstuff[0x001c].orig)\
    ? C_Count1Types()\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Count1Types  ));\
  })
#define GetIndType(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   GUEST<ResType> * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x019f];\
   ((new_addr == toolstuff[0x019f].orig)\
    ? C_GetIndType(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetIndType ,  __stub_arg_1, __stub_arg_2));\
  })
#define Get1IndType(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   GUEST<ResType> * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x000f];\
   ((new_addr == toolstuff[0x000f].orig)\
    ? C_Get1IndType(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Get1IndType ,  __stub_arg_1, __stub_arg_2));\
  })
#define MaxSizeRsrc(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0021];\
   ((new_addr == toolstuff[0x0021].orig)\
    ? C_MaxSizeRsrc(__stub_arg_1)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_MaxSizeRsrc ,  __stub_arg_1));\
  })
#define RsrcMapEntry(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01c5];\
   ((new_addr == toolstuff[0x01c5].orig)\
    ? C_RsrcMapEntry(__stub_arg_1)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_RsrcMapEntry ,  __stub_arg_1));\
  })
#define RGetResource(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   ResType __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x000c];\
   ((new_addr == toolstuff[0x000c].orig)\
    ? C_RGetResource(__stub_arg_1, __stub_arg_2)\
    : (Handle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_RGetResource ,  __stub_arg_1, __stub_arg_2));\
  })
#define InitResources() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0195];\
   ((new_addr == toolstuff[0x0195].orig)\
    ? C_InitResources()\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InitResources  ));\
  })
#define RsrcZoneInit() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0196];\
   ((new_addr == toolstuff[0x0196].orig)\
    ? C_RsrcZoneInit()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_RsrcZoneInit  ));\
  })
#define ResError() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x01af];\
   ((new_addr == toolstuff[0x01af].orig)\
    ? C_ResError()\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ResError  ));\
  })
#define GetResFileAttrs(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01f6];\
   ((new_addr == toolstuff[0x01f6].orig)\
    ? C_GetResFileAttrs(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetResFileAttrs ,  __stub_arg_1));\
  })
#define SetResFileAttrs(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x01f7];\
   ((new_addr == toolstuff[0x01f7].orig)\
    ? C_SetResFileAttrs(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetResFileAttrs ,  __stub_arg_1, __stub_arg_2));\
  })
#define SetResInfo(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   StringPtr __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x01a9];\
   ((new_addr == toolstuff[0x01a9].orig)\
    ? C_SetResInfo(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetResInfo ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SetResAttrs(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x01a7];\
   ((new_addr == toolstuff[0x01a7].orig)\
    ? C_SetResAttrs(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetResAttrs ,  __stub_arg_1, __stub_arg_2));\
  })
#define ChangedResource(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01aa];\
   ((new_addr == toolstuff[0x01aa].orig)\
    ? C_ChangedResource(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ChangedResource ,  __stub_arg_1));\
  })
#define AddResource(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
   ResType __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   StringPtr __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x01ab];\
   ((new_addr == toolstuff[0x01ab].orig)\
    ? C_AddResource(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_AddResource ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define RmveResource(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01ad];\
   ((new_addr == toolstuff[0x01ad].orig)\
    ? C_RmveResource(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_RmveResource ,  __stub_arg_1));\
  })
#define UpdateResFile(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0199];\
   ((new_addr == toolstuff[0x0199].orig)\
    ? C_UpdateResFile(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_UpdateResFile ,  __stub_arg_1));\
  })
#define WriteResource(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01b0];\
   ((new_addr == toolstuff[0x01b0].orig)\
    ? C_WriteResource(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_WriteResource ,  __stub_arg_1));\
  })
#define SetResPurge(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   BOOLEAN __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0193];\
   ((new_addr == toolstuff[0x0193].orig)\
    ? C_SetResPurge(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetResPurge ,  __stub_arg_1));\
  })
#define CreateResFile(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   StringPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01b1];\
   ((new_addr == toolstuff[0x01b1].orig)\
    ? C_CreateResFile(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CreateResFile ,  __stub_arg_1));\
  })
#define HCreateResFile(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
   StringPtr __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x001b];\
   ((new_addr == toolstuff[0x001b].orig)\
    ? C_HCreateResFile(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_HCreateResFile ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define OpenRFPerm(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   StringPtr __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   Byte __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x01c4];\
   ((new_addr == toolstuff[0x01c4].orig)\
    ? C_OpenRFPerm(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_OpenRFPerm ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define OpenResFile(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   StringPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0197];\
   ((new_addr == toolstuff[0x0197].orig)\
    ? C_OpenResFile(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_OpenResFile ,  __stub_arg_1));\
  })
#define CloseResFile(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x019a];\
   ((new_addr == toolstuff[0x019a].orig)\
    ? C_CloseResFile(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CloseResFile ,  __stub_arg_1));\
  })
#define HOpenResFile(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
   StringPtr __stub_arg_3 = (A3);\
   SignedByte __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x001a];\
   ((new_addr == toolstuff[0x001a].orig)\
    ? C_HOpenResFile(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_HOpenResFile ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define CurResFile() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0194];\
   ((new_addr == toolstuff[0x0194].orig)\
    ? C_CurResFile()\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CurResFile  ));\
  })
#define HomeResFile(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01a4];\
   ((new_addr == toolstuff[0x01a4].orig)\
    ? C_HomeResFile(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_HomeResFile ,  __stub_arg_1));\
  })
#define UseResFile(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0198];\
   ((new_addr == toolstuff[0x0198].orig)\
    ? C_UseResFile(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_UseResFile ,  __stub_arg_1));\
  })
#define ReadPartialResource(A1, A2, A3, A4) \
   (     C_ReadPartialResource((A1), (A2), (A3), (A4))     )
#define WritePartialResource(A1, A2, A3, A4) \
   (     C_WritePartialResource((A1), (A2), (A3), (A4))     )
#define SetResourceSize(A1, A2) \
   (     C_SetResourceSize((A1), (A2))     )
#define GetNextFOND(A1) \
   (     C_GetNextFOND((A1))     )
#define TESetText(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
   TEHandle __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x01cf];\
   ((new_addr == toolstuff[0x01cf].orig)\
    ? C_TESetText(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TESetText ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define TEGetText(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   TEHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01cb];\
   ((new_addr == toolstuff[0x01cb].orig)\
    ? C_TEGetText(__stub_arg_1)\
    : (CharsHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEGetText ,  __stub_arg_1));\
  })
#define TESetJust(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   TEHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x01df];\
   ((new_addr == toolstuff[0x01df].orig)\
    ? C_TESetJust(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TESetJust ,  __stub_arg_1, __stub_arg_2));\
  })
#define TEUpdate(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   TEHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x01d3];\
   ((new_addr == toolstuff[0x01d3].orig)\
    ? C_TEUpdate(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEUpdate ,  __stub_arg_1, __stub_arg_2));\
  })
#define TextBox(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   int32 __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
   int16 __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x01ce];\
   ((new_addr == toolstuff[0x01ce].orig)\
    ? C_TextBox(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TextBox ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define TEScroll(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   int16 __stub_arg_1 = (A1);\
   int16 __stub_arg_2 = (A2);\
   TEHandle __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x01dd];\
   ((new_addr == toolstuff[0x01dd].orig)\
    ? C_TEScroll(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEScroll ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define TEKey(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   CHAR __stub_arg_1 = (A1);\
   TEHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x01dc];\
   ((new_addr == toolstuff[0x01dc].orig)\
    ? C_TEKey(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEKey ,  __stub_arg_1, __stub_arg_2));\
  })
#define TECopy(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   TEHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01d5];\
   ((new_addr == toolstuff[0x01d5].orig)\
    ? C_TECopy(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TECopy ,  __stub_arg_1));\
  })
#define TECut(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   TEHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01d6];\
   ((new_addr == toolstuff[0x01d6].orig)\
    ? C_TECut(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TECut ,  __stub_arg_1));\
  })
#define TEPaste(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   TEHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01db];\
   ((new_addr == toolstuff[0x01db].orig)\
    ? C_TEPaste(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEPaste ,  __stub_arg_1));\
  })
#define TEDelete(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   TEHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01d7];\
   ((new_addr == toolstuff[0x01d7].orig)\
    ? C_TEDelete(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEDelete ,  __stub_arg_1));\
  })
#define TEInsert(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
   TEHandle __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x01de];\
   ((new_addr == toolstuff[0x01de].orig)\
    ? C_TEInsert(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEInsert ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define TEPinScroll(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   int16 __stub_arg_1 = (A1);\
   int16 __stub_arg_2 = (A2);\
   TEHandle __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0012];\
   ((new_addr == toolstuff[0x0012].orig)\
    ? C_TEPinScroll(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEPinScroll ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define TESelView(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   TEHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0011];\
   ((new_addr == toolstuff[0x0011].orig)\
    ? C_TESelView(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TESelView ,  __stub_arg_1));\
  })
#define TEAutoView(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   BOOLEAN __stub_arg_1 = (A1);\
   TEHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0013];\
   ((new_addr == toolstuff[0x0013].orig)\
    ? C_TEAutoView(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEAutoView ,  __stub_arg_1, __stub_arg_2));\
  })
#define TEStylNew(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x003e];\
   ((new_addr == toolstuff[0x003e].orig)\
    ? C_TEStylNew(__stub_arg_1, __stub_arg_2)\
    : (TEHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEStylNew ,  __stub_arg_1, __stub_arg_2));\
  })
#define SetStylHandle(A1, A2) \
   (     C_SetStylHandle((A1), (A2))     )
#define GetStylHandle(A1) \
   (     C_GetStylHandle((A1))     )
#define GetStylScrap(A1) \
   (     C_GetStylScrap((A1))     )
#define TEStylInsert(A1, A2, A3, A4) \
   (     C_TEStylInsert((A1), (A2), (A3), (A4))     )
#define TEGetOffset(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Point __stub_arg_1 = (A1);\
   TEHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x003c];\
   ((new_addr == toolstuff[0x003c].orig)\
    ? C_TEGetOffset(__stub_arg_1, __stub_arg_2)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEGetOffset ,  __stub_arg_1, __stub_arg_2));\
  })
#define TEGetPoint(A1, A2) \
   (     C_TEGetPoint((A1), (A2))     )
#define TEGetHeight(A1, A2, A3) \
   (     C_TEGetHeight((A1), (A2), (A3))     )
#define TEGetStyle(A1, A2, A3, A4, A5) \
   (     C_TEGetStyle((A1), (A2), (A3), (A4), (A5))     )
#define TEStylPaste(A1) \
   (     C_TEStylPaste((A1))     )
#define TESetStyle(A1, A2, A3, A4) \
   (     C_TESetStyle((A1), (A2), (A3), (A4))     )
#define TEReplaceStyle(A1, A2, A3, A4, A5) \
   (     C_TEReplaceStyle((A1), (A2), (A3), (A4), (A5))     )
#define TEContinuousStyle(A1, A2, A3) \
   (     C_TEContinuousStyle((A1), (A2), (A3))     )
#define SetStylScrap(A1, A2, A3, A4, A5) \
   (     C_SetStylScrap((A1), (A2), (A3), (A4), (A5))     )
#define TECustomHook(A1, A2, A3) \
   (     C_TECustomHook((A1), (A2), (A3))     )
#define TENumStyles(A1, A2, A3) \
   (     C_TENumStyles((A1), (A2), (A3))     )
#define TEInit() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x01cc];\
   ((new_addr == toolstuff[0x01cc].orig)\
    ? C_TEInit()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEInit  ));\
  })
#define TENew(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x01d2];\
   ((new_addr == toolstuff[0x01d2].orig)\
    ? C_TENew(__stub_arg_1, __stub_arg_2)\
    : (TEHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TENew ,  __stub_arg_1, __stub_arg_2));\
  })
#define TEDispose(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   TEHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01cd];\
   ((new_addr == toolstuff[0x01cd].orig)\
    ? C_TEDispose(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEDispose ,  __stub_arg_1));\
  })
#define TEIdle(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   TEHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01da];\
   ((new_addr == toolstuff[0x01da].orig)\
    ? C_TEIdle(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEIdle ,  __stub_arg_1));\
  })
#define TEClick(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Point __stub_arg_1 = (A1);\
   BOOLEAN __stub_arg_2 = (A2);\
   TEHandle __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x01d4];\
   ((new_addr == toolstuff[0x01d4].orig)\
    ? C_TEClick(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEClick ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define TESetSelect(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   int32 __stub_arg_1 = (A1);\
   int32 __stub_arg_2 = (A2);\
   TEHandle __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x01d1];\
   ((new_addr == toolstuff[0x01d1].orig)\
    ? C_TESetSelect(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TESetSelect ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define TEActivate(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   TEHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01d8];\
   ((new_addr == toolstuff[0x01d8].orig)\
    ? C_TEActivate(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEActivate ,  __stub_arg_1));\
  })
#define TEDeactivate(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   TEHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01d9];\
   ((new_addr == toolstuff[0x01d9].orig)\
    ? C_TEDeactivate(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TEDeactivate ,  __stub_arg_1));\
  })
#define TECalText(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   TEHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01d0];\
   ((new_addr == toolstuff[0x01d0].orig)\
    ? C_TECalText(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TECalText ,  __stub_arg_1));\
  })
#define TEFeatureFlag(A1, A2, A3) \
   (     C_TEFeatureFlag((A1), (A2), (A3))     )
#define SetWinColor(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   CTabHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0241];\
   ((new_addr == toolstuff[0x0241].orig)\
    ? C_SetWinColor(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetWinColor ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetAuxWin(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   GUEST<AuxWinHandle> * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0242];\
   ((new_addr == toolstuff[0x0242].orig)\
    ? C_GetAuxWin(__stub_arg_1, __stub_arg_2)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetAuxWin ,  __stub_arg_1, __stub_arg_2));\
  })
#define SetWTitle(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   StringPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x011a];\
   ((new_addr == toolstuff[0x011a].orig)\
    ? C_SetWTitle(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetWTitle ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetWTitle(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   StringPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0119];\
   ((new_addr == toolstuff[0x0119].orig)\
    ? C_GetWTitle(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetWTitle ,  __stub_arg_1, __stub_arg_2));\
  })
#define FrontWindow() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0124];\
   ((new_addr == toolstuff[0x0124].orig)\
    ? C_FrontWindow()\
    : (WindowPtr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FrontWindow  ));\
  })
#define HiliteWindow(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   BOOLEAN __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x011c];\
   ((new_addr == toolstuff[0x011c].orig)\
    ? C_HiliteWindow(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_HiliteWindow ,  __stub_arg_1, __stub_arg_2));\
  })
#define BringToFront(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0120];\
   ((new_addr == toolstuff[0x0120].orig)\
    ? C_BringToFront(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_BringToFront ,  __stub_arg_1));\
  })
#define SelectWindow(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x011f];\
   ((new_addr == toolstuff[0x011f].orig)\
    ? C_SelectWindow(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SelectWindow ,  __stub_arg_1));\
  })
#define ShowHide(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   BOOLEAN __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0108];\
   ((new_addr == toolstuff[0x0108].orig)\
    ? C_ShowHide(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ShowHide ,  __stub_arg_1, __stub_arg_2));\
  })
#define HideWindow(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0116];\
   ((new_addr == toolstuff[0x0116].orig)\
    ? C_HideWindow(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_HideWindow ,  __stub_arg_1));\
  })
#define ShowWindow(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0115];\
   ((new_addr == toolstuff[0x0115].orig)\
    ? C_ShowWindow(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ShowWindow ,  __stub_arg_1));\
  })
#define SendBehind(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   WindowPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0121];\
   ((new_addr == toolstuff[0x0121].orig)\
    ? C_SendBehind(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SendBehind ,  __stub_arg_1, __stub_arg_2));\
  })
#define DrawGrowIcon(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0104];\
   ((new_addr == toolstuff[0x0104].orig)\
    ? C_DrawGrowIcon(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DrawGrowIcon ,  __stub_arg_1));\
  })
#define InitWindows() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0112];\
   ((new_addr == toolstuff[0x0112].orig)\
    ? C_InitWindows()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InitWindows  ));\
  })
#define GetWMgrPort(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   GUEST<GrafPtr> * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0110];\
   ((new_addr == toolstuff[0x0110].orig)\
    ? C_GetWMgrPort(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetWMgrPort ,  __stub_arg_1));\
  })
#define GetCWMgrPort(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   GUEST<CGrafPtr> * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0248];\
   ((new_addr == toolstuff[0x0248].orig)\
    ? C_GetCWMgrPort(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetCWMgrPort ,  __stub_arg_1));\
  })
#define SetDeskCPat(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   PixPatHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0247];\
   ((new_addr == toolstuff[0x0247].orig)\
    ? C_SetDeskCPat(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetDeskCPat ,  __stub_arg_1));\
  })
#define NewWindow(A1, A2, A3, A4, A5, A6, A7, A8) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
   StringPtr __stub_arg_3 = (A3);\
   BOOLEAN __stub_arg_4 = (A4);\
   INTEGER __stub_arg_5 = (A5);\
   WindowPtr __stub_arg_6 = (A6);\
   BOOLEAN __stub_arg_7 = (A7);\
   LONGINT __stub_arg_8 = (A8);\
 \
   new_addr = tooltraptable[0x0113];\
   ((new_addr == toolstuff[0x0113].orig)\
    ? C_NewWindow(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8)\
    : (WindowPtr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_NewWindow ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8));\
  })
#define NewCWindow(A1, A2, A3, A4, A5, A6, A7, A8) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   Rect * __stub_arg_2 = (A2);\
   StringPtr __stub_arg_3 = (A3);\
   BOOLEAN __stub_arg_4 = (A4);\
   INTEGER __stub_arg_5 = (A5);\
   CWindowPtr __stub_arg_6 = (A6);\
   BOOLEAN __stub_arg_7 = (A7);\
   LONGINT __stub_arg_8 = (A8);\
 \
   new_addr = tooltraptable[0x0245];\
   ((new_addr == toolstuff[0x0245].orig)\
    ? C_NewCWindow(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8)\
    : (CWindowPtr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_NewCWindow ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6, __stub_arg_7, __stub_arg_8));\
  })
#define GetNewCWindow(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   Ptr __stub_arg_2 = (A2);\
   CWindowPtr __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0246];\
   ((new_addr == toolstuff[0x0246].orig)\
    ? C_GetNewCWindow(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (CWindowPtr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetNewCWindow ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define GetNewWindow(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   Ptr __stub_arg_2 = (A2);\
   WindowPtr __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x01bd];\
   ((new_addr == toolstuff[0x01bd].orig)\
    ? C_GetNewWindow(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (WindowPtr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetNewWindow ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define CloseWindow(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x012d];\
   ((new_addr == toolstuff[0x012d].orig)\
    ? C_CloseWindow(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CloseWindow ,  __stub_arg_1));\
  })
#define DisposeWindow(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0114];\
   ((new_addr == toolstuff[0x0114].orig)\
    ? C_DisposeWindow(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DisposeWindow ,  __stub_arg_1));\
  })
#define SetWRefCon(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0118];\
   ((new_addr == toolstuff[0x0118].orig)\
    ? C_SetWRefCon(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetWRefCon ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetWRefCon(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0117];\
   ((new_addr == toolstuff[0x0117].orig)\
    ? C_GetWRefCon(__stub_arg_1)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetWRefCon ,  __stub_arg_1));\
  })
#define SetWindowPic(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   PicHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x012e];\
   ((new_addr == toolstuff[0x012e].orig)\
    ? C_SetWindowPic(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetWindowPic ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetWindowPic(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x012f];\
   ((new_addr == toolstuff[0x012f].orig)\
    ? C_GetWindowPic(__stub_arg_1)\
    : (PicHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetWindowPic ,  __stub_arg_1));\
  })
#define PinRect(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x014e];\
   ((new_addr == toolstuff[0x014e].orig)\
    ? C_PinRect(__stub_arg_1, __stub_arg_2)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PinRect ,  __stub_arg_1, __stub_arg_2));\
  })
#define DragTheRgn(A1, A2, A3, A4, A5, A6) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
   Rect * __stub_arg_4 = (A4);\
   INTEGER __stub_arg_5 = (A5);\
   ProcPtr __stub_arg_6 = (A6);\
 \
   new_addr = tooltraptable[0x0126];\
   ((new_addr == toolstuff[0x0126].orig)\
    ? C_DragTheRgn(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DragTheRgn ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6));\
  })
#define DragGrayRgn(A1, A2, A3, A4, A5, A6) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
   Rect * __stub_arg_4 = (A4);\
   INTEGER __stub_arg_5 = (A5);\
   ProcPtr __stub_arg_6 = (A6);\
 \
   new_addr = tooltraptable[0x0105];\
   ((new_addr == toolstuff[0x0105].orig)\
    ? C_DragGrayRgn(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DragGrayRgn ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6));\
  })
#define ClipAbove(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPeek __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x010b];\
   ((new_addr == toolstuff[0x010b].orig)\
    ? C_ClipAbove(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ClipAbove ,  __stub_arg_1));\
  })
#define CheckUpdate(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   EventRecord * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0111];\
   ((new_addr == toolstuff[0x0111].orig)\
    ? C_CheckUpdate(__stub_arg_1)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CheckUpdate ,  __stub_arg_1));\
  })
#define SaveOld(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPeek __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x010e];\
   ((new_addr == toolstuff[0x010e].orig)\
    ? C_SaveOld(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SaveOld ,  __stub_arg_1));\
  })
#define PaintOne(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPeek __stub_arg_1 = (A1);\
   RgnHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x010c];\
   ((new_addr == toolstuff[0x010c].orig)\
    ? C_PaintOne(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PaintOne ,  __stub_arg_1, __stub_arg_2));\
  })
#define PaintBehind(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPeek __stub_arg_1 = (A1);\
   RgnHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x010d];\
   ((new_addr == toolstuff[0x010d].orig)\
    ? C_PaintBehind(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PaintBehind ,  __stub_arg_1, __stub_arg_2));\
  })
#define CalcVis(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPeek __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0109];\
   ((new_addr == toolstuff[0x0109].orig)\
    ? C_CalcVis(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CalcVis ,  __stub_arg_1));\
  })
#define CalcVisBehind(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPeek __stub_arg_1 = (A1);\
   RgnHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x010a];\
   ((new_addr == toolstuff[0x010a].orig)\
    ? C_CalcVisBehind(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CalcVisBehind ,  __stub_arg_1, __stub_arg_2));\
  })
#define DrawNew(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPeek __stub_arg_1 = (A1);\
   BOOLEAN __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x010f];\
   ((new_addr == toolstuff[0x010f].orig)\
    ? C_DrawNew(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DrawNew ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetWVariant(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x000a];\
   ((new_addr == toolstuff[0x000a].orig)\
    ? C_GetWVariant(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetWVariant ,  __stub_arg_1));\
  })
#define FindWindow(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Point __stub_arg_1 = (A1);\
   GUEST<WindowPtr> * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x012c];\
   ((new_addr == toolstuff[0x012c].orig)\
    ? C_FindWindow(__stub_arg_1, __stub_arg_2)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FindWindow ,  __stub_arg_1, __stub_arg_2));\
  })
#define TrackBox(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x003b];\
   ((new_addr == toolstuff[0x003b].orig)\
    ? C_TrackBox(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TrackBox ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define TrackGoAway(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x011e];\
   ((new_addr == toolstuff[0x011e].orig)\
    ? C_TrackGoAway(__stub_arg_1, __stub_arg_2)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TrackGoAway ,  __stub_arg_1, __stub_arg_2));\
  })
#define ZoomWindow(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   BOOLEAN __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x003a];\
   ((new_addr == toolstuff[0x003a].orig)\
    ? C_ZoomWindow(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ZoomWindow ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define MoveWindow(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   BOOLEAN __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x011b];\
   ((new_addr == toolstuff[0x011b].orig)\
    ? C_MoveWindow(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_MoveWindow ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define DragWindow(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0125];\
   ((new_addr == toolstuff[0x0125].orig)\
    ? C_DragWindow(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DragWindow ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define GrowWindow(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
   Rect * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x012b];\
   ((new_addr == toolstuff[0x012b].orig)\
    ? C_GrowWindow(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GrowWindow ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SizeWindow(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   BOOLEAN __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x011d];\
   ((new_addr == toolstuff[0x011d].orig)\
    ? C_SizeWindow(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SizeWindow ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define InvalRect(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0128];\
   ((new_addr == toolstuff[0x0128].orig)\
    ? C_InvalRect(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InvalRect ,  __stub_arg_1));\
  })
#define InvalRgn(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0127];\
   ((new_addr == toolstuff[0x0127].orig)\
    ? C_InvalRgn(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InvalRgn ,  __stub_arg_1));\
  })
#define ValidRect(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Rect * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x012a];\
   ((new_addr == toolstuff[0x012a].orig)\
    ? C_ValidRect(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ValidRect ,  __stub_arg_1));\
  })
#define ValidRgn(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   RgnHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0129];\
   ((new_addr == toolstuff[0x0129].orig)\
    ? C_ValidRgn(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ValidRgn ,  __stub_arg_1));\
  })
#define BeginUpdate(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0122];\
   ((new_addr == toolstuff[0x0122].orig)\
    ? C_BeginUpdate(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_BeginUpdate ,  __stub_arg_1));\
  })
#define EndUpdate(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   WindowPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0123];\
   ((new_addr == toolstuff[0x0123].orig)\
    ? C_EndUpdate(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_EndUpdate ,  __stub_arg_1));\
  })
#define AEProcessAppleEvent(A1) \
   (     C_AEProcessAppleEvent((A1))     )
#define AESend(A1, A2, A3, A4, A5, A6, A7) \
   (     C_AESend((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define AESuspendTheCurrentEvent(A1) \
   (     C_AESuspendTheCurrentEvent((A1))     )
#define AEResumeTheCurrentEvent(A1, A2, A3, A4) \
   (     C_AEResumeTheCurrentEvent((A1), (A2), (A3), (A4))     )
#define AEGetTheCurrentEvent(A1) \
   (     C_AEGetTheCurrentEvent((A1))     )
#define AESetTheCurrentEvent(A1) \
   (     C_AESetTheCurrentEvent((A1))     )
#define AEGetArray(A1, A2, A3, A4, A5, A6, A7) \
   (     C_AEGetArray((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define AEPutArray(A1, A2, A3, A4, A5, A6) \
   (     C_AEPutArray((A1), (A2), (A3), (A4), (A5), (A6))     )
#define AESetInteractionAllowed(A1) \
   (     C_AESetInteractionAllowed((A1))     )
#define AEGetInteractionAllowed(A1) \
   (     C_AEGetInteractionAllowed((A1))     )
#define AEInteractWithUser(A1, A2, A3) \
   (     C_AEInteractWithUser((A1), (A2), (A3))     )
#define AEResetTimer(A1) \
   (     C_AEResetTimer((A1))     )
#define AECoercePtr(A1, A2, A3, A4, A5) \
   (     C_AECoercePtr((A1), (A2), (A3), (A4), (A5))     )
#define AECoerceDesc(A1, A2, A3) \
   (     C_AECoerceDesc((A1), (A2), (A3))     )
#define AEManagerInfo(A1) \
   (     C_AEManagerInfo((A1))     )
#define AEDisposeToken(A1) \
   (     C_AEDisposeToken((A1))     )
#define AEREesolve(A1, A2, A3) \
   (     C_AEREesolve((A1), (A2), (A3))     )
#define AERemoveObjectAccessor(A1, A2, A3, A4) \
   (     C_AERemoveObjectAccessor((A1), (A2), (A3), (A4))     )
#define AEInstallObjectAccessor(A1, A2, A3, A4, A5) \
   (     C_AEInstallObjectAccessor((A1), (A2), (A3), (A4), (A5))     )
#define AEGetObjectAccessor(A1, A2, A3, A4, A5) \
   (     C_AEGetObjectAccessor((A1), (A2), (A3), (A4), (A5))     )
#define AECallObjectAccessor(A1, A2, A3, A4, A5, A6) \
   (     C_AECallObjectAccessor((A1), (A2), (A3), (A4), (A5), (A6))     )
#define AESetObjectCallbacks(A1, A2, A3, A4, A5, A6, A7) \
   (     C_AESetObjectCallbacks((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define AECreateAppleEvent(A1, A2, A3, A4, A5, A6) \
   (     C_AECreateAppleEvent((A1), (A2), (A3), (A4), (A5), (A6))     )
#define AECreateDesc(A1, A2, A3, A4) \
   (     C_AECreateDesc((A1), (A2), (A3), (A4))     )
#define AEDisposeDesc(A1) \
   (     C_AEDisposeDesc((A1))     )
#define AEDuplicateDesc(A1, A2) \
   (     C_AEDuplicateDesc((A1), (A2))     )
#define AECreateList(A1, A2, A3, A4) \
   (     C_AECreateList((A1), (A2), (A3), (A4))     )
#define AECountItems(A1, A2) \
   (     C_AECountItems((A1), (A2))     )
#define AEGetNthDesc(A1, A2, A3, A4, A5) \
   (     C_AEGetNthDesc((A1), (A2), (A3), (A4), (A5))     )
#define AEGetNthPtr(A1, A2, A3, A4, A5, A6, A7, A8) \
   (     C_AEGetNthPtr((A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8))     )
#define AEPutDesc(A1, A2, A3) \
   (     C_AEPutDesc((A1), (A2), (A3))     )
#define AEPutPtr(A1, A2, A3, A4, A5) \
   (     C_AEPutPtr((A1), (A2), (A3), (A4), (A5))     )
#define AEDeleteItem(A1, A2) \
   (     C_AEDeleteItem((A1), (A2))     )
#define AESizeOfNthItem(A1, A2, A3, A4) \
   (     C_AESizeOfNthItem((A1), (A2), (A3), (A4))     )
#undef AEGetKeyDesc
#undef AEPutKeyDesc
#undef AEGetKeyPtr
#undef AEPutKeyPtr
#undef AEDeleteKeyDesc
#undef AESizeOfKeyDesc
#define AEGetKeyDesc(A1, A2, A3, A4) \
   (     C_AEGetKeyDesc((A1), (A2), (A3), (A4))     )
#define AEPutKeyDesc(A1, A2, A3) \
   (     C_AEPutKeyDesc((A1), (A2), (A3))     )
#define AEGetKeyPtr(A1, A2, A3, A4, A5, A6, A7) \
   (     C_AEGetKeyPtr((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define AEPutKeyPtr(A1, A2, A3, A4, A5) \
   (     C_AEPutKeyPtr((A1), (A2), (A3), (A4), (A5))     )
#define AEDeleteKeyDesc(A1, A2) \
   (     C_AEDeleteKeyDesc((A1), (A2))     )
#define AESizeOfKeyDesc(A1, A2, A3, A4) \
   (     C_AESizeOfKeyDesc((A1), (A2), (A3), (A4))     )
#define AEPutAttributePtr(A1, A2, A3, A4, A5) \
   (     C_AEPutAttributePtr((A1), (A2), (A3), (A4), (A5))     )
#define AEPutAttributeDesc(A1, A2, A3) \
   (     C_AEPutAttributeDesc((A1), (A2), (A3))     )
#define AEGetAttributeDesc(A1, A2, A3, A4) \
   (     C_AEGetAttributeDesc((A1), (A2), (A3), (A4))     )
#define AEGetAttributePtr(A1, A2, A3, A4, A5, A6, A7) \
   (     C_AEGetAttributePtr((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define AEDeleteAttribute(A1, A2) \
   (     C_AEDeleteAttribute((A1), (A2))     )
#define AESizeOfAttribute(A1, A2, A3, A4) \
   (     C_AESizeOfAttribute((A1), (A2), (A3), (A4))     )
#define _AE_hdlr_table_alloc(A1, A2, A3, A4, A5) \
   (     C__AE_hdlr_table_alloc((A1), (A2), (A3), (A4), (A5))     )
#define _AE_hdlr_delete(A1, A2, A3) \
   (     C__AE_hdlr_delete((A1), (A2), (A3))     )
#define _AE_hdlr_lookup(A1, A2, A3, A4) \
   (     C__AE_hdlr_lookup((A1), (A2), (A3), (A4))     )
#define _AE_hdlr_install(A1, A2, A3, A4) \
   (     C__AE_hdlr_install((A1), (A2), (A3), (A4))     )
#define AEInstallEventHandler(A1, A2, A3, A4, A5) \
   (     C_AEInstallEventHandler((A1), (A2), (A3), (A4), (A5))     )
#define AEGetEventHandler(A1, A2, A3, A4, A5) \
   (     C_AEGetEventHandler((A1), (A2), (A3), (A4), (A5))     )
#define AERemoveEventHandler(A1, A2, A3, A4) \
   (     C_AERemoveEventHandler((A1), (A2), (A3), (A4))     )
#define AEInstallCoercionHandler(A1, A2, A3, A4, A5, A6) \
   (     C_AEInstallCoercionHandler((A1), (A2), (A3), (A4), (A5), (A6))     )
#define AEGetCoercionHandler(A1, A2, A3, A4, A5, A6) \
   (     C_AEGetCoercionHandler((A1), (A2), (A3), (A4), (A5), (A6))     )
#define AERemoveCoercionHandler(A1, A2, A3, A4) \
   (     C_AERemoveCoercionHandler((A1), (A2), (A3), (A4))     )
#define AEInstallSpecialHandler(A1, A2, A3) \
   (     C_AEInstallSpecialHandler((A1), (A2), (A3))     )
#define AEGetSpecialHandler(A1, A2, A3) \
   (     C_AEGetSpecialHandler((A1), (A2), (A3))     )
#define AERemoveSpecialHandler(A1, A2, A3) \
   (     C_AERemoveSpecialHandler((A1), (A2), (A3))     )
#define OpenDeskAcc(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   StringPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01b6];\
   ((new_addr == toolstuff[0x01b6].orig)\
    ? C_OpenDeskAcc(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_OpenDeskAcc ,  __stub_arg_1));\
  })
#define CloseDeskAcc(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01b7];\
   ((new_addr == toolstuff[0x01b7].orig)\
    ? C_CloseDeskAcc(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_CloseDeskAcc ,  __stub_arg_1));\
  })
#define SystemClick(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   EventRecord * __stub_arg_1 = (A1);\
   WindowPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x01b3];\
   ((new_addr == toolstuff[0x01b3].orig)\
    ? C_SystemClick(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SystemClick ,  __stub_arg_1, __stub_arg_2));\
  })
#define SystemEdit(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01c2];\
   ((new_addr == toolstuff[0x01c2].orig)\
    ? C_SystemEdit(__stub_arg_1)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SystemEdit ,  __stub_arg_1));\
  })
#define SystemTask() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x01b4];\
   ((new_addr == toolstuff[0x01b4].orig)\
    ? C_SystemTask()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SystemTask  ));\
  })
#define SystemEvent(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   EventRecord * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01b2];\
   ((new_addr == toolstuff[0x01b2].orig)\
    ? C_SystemEvent(__stub_arg_1)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SystemEvent ,  __stub_arg_1));\
  })
#define SystemMenu(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   LONGINT __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01b5];\
   ((new_addr == toolstuff[0x01b5].orig)\
    ? C_SystemMenu(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SystemMenu ,  __stub_arg_1));\
  })
#define DILoad() \
   (     C_DILoad()     )
#define DIUnload() \
   (     C_DIUnload()     )
#define DIBadMount(A1, A2) \
   (     C_DIBadMount((A1), (A2))     )
#define DIFormat(A1) \
   (     C_DIFormat((A1))     )
#define DIVerify(A1) \
   (     C_DIVerify((A1))     )
#define DIZero(A1, A2) \
   (     C_DIZero((A1), (A2))     )
#define ROMlib_Fsetenv(A1, A2) \
   (     C_ROMlib_Fsetenv((A1), (A2))     )
#define ROMlib_Fgetenv(A1, A2) \
   (     C_ROMlib_Fgetenv((A1), (A2))     )
#define ROMlib_Fprocentry(A1, A2) \
   (     C_ROMlib_Fprocentry((A1), (A2))     )
#define ROMlib_Fprocexit(A1, A2) \
   (     C_ROMlib_Fprocexit((A1), (A2))     )
#define ROMlib_Ftestxcp(A1, A2) \
   (     C_ROMlib_Ftestxcp((A1), (A2))     )
#define ROMlib_FsqrtX(A1, A2) \
   (     C_ROMlib_FsqrtX((A1), (A2))     )
#define ROMlib_FscalbX(A1, A2, A3) \
   (     C_ROMlib_FscalbX((A1), (A2), (A3))     )
#define ROMlib_FlogbX(A1, A2) \
   (     C_ROMlib_FlogbX((A1), (A2))     )
#define ROMlib_FabsX(A1, A2) \
   (     C_ROMlib_FabsX((A1), (A2))     )
#define ROMlib_FnegX(A1, A2) \
   (     C_ROMlib_FnegX((A1), (A2))     )
#define ROMlib_Fcpysgnx(A1, A2, A3) \
   (     C_ROMlib_Fcpysgnx((A1), (A2), (A3))     )
#define ROMlib_FrintX(A1, A2) \
   (     C_ROMlib_FrintX((A1), (A2))     )
#define ROMlib_FtintX(A1, A2) \
   (     C_ROMlib_FtintX((A1), (A2))     )
#define ROMlib_Faddx(A1, A2, A3) \
   (     C_ROMlib_Faddx((A1), (A2), (A3))     )
#define ROMlib_Fsubx(A1, A2, A3) \
   (     C_ROMlib_Fsubx((A1), (A2), (A3))     )
#define ROMlib_Fmulx(A1, A2, A3) \
   (     C_ROMlib_Fmulx((A1), (A2), (A3))     )
#define ROMlib_Fdivx(A1, A2, A3) \
   (     C_ROMlib_Fdivx((A1), (A2), (A3))     )
#define ROMlib_Fx2X(A1, A2, A3) \
   (     C_ROMlib_Fx2X((A1), (A2), (A3))     )
#define ROMlib_FX2x(A1, A2, A3) \
   (     C_ROMlib_FX2x((A1), (A2), (A3))     )
#define ROMlib_Fremx(A1, A2, A3) \
   (     C_ROMlib_Fremx((A1), (A2), (A3))     )
#define ROMlib_Fcmpx(A1, A2, A3) \
   (     C_ROMlib_Fcmpx((A1), (A2), (A3))     )
#define ROMlib_FcpXx(A1, A2, A3) \
   (     C_ROMlib_FcpXx((A1), (A2), (A3))     )
#define ROMlib_Fx2dec(A1, A2, A3, A4) \
   (     C_ROMlib_Fx2dec((A1), (A2), (A3), (A4))     )
#define ROMlib_Fdec2x(A1, A2, A3) \
   (     C_ROMlib_Fdec2x((A1), (A2), (A3))     )
#define ROMlib_Fclassx(A1, A2, A3) \
   (     C_ROMlib_Fclassx((A1), (A2), (A3))     )
#define ROMlib_FlnX(A1) \
   (     C_ROMlib_FlnX((A1))     )
#define ROMlib_Flog2X(A1) \
   (     C_ROMlib_Flog2X((A1))     )
#define ROMlib_Fln1X(A1) \
   (     C_ROMlib_Fln1X((A1))     )
#define ROMlib_Flog21X(A1) \
   (     C_ROMlib_Flog21X((A1))     )
#define ROMlib_FexpX(A1) \
   (     C_ROMlib_FexpX((A1))     )
#define ROMlib_Fexp2X(A1) \
   (     C_ROMlib_Fexp2X((A1))     )
#define ROMlib_Fexp1X(A1) \
   (     C_ROMlib_Fexp1X((A1))     )
#define ROMlib_Fexp21X(A1) \
   (     C_ROMlib_Fexp21X((A1))     )
#define ROMlib_Fxpwri(A1, A2) \
   (     C_ROMlib_Fxpwri((A1), (A2))     )
#define ROMlib_Fxpwry(A1, A2) \
   (     C_ROMlib_Fxpwry((A1), (A2))     )
#define ROMlib_Fcompound(A1, A2, A3) \
   (     C_ROMlib_Fcompound((A1), (A2), (A3))     )
#define ROMlib_Fannuity(A1, A2, A3) \
   (     C_ROMlib_Fannuity((A1), (A2), (A3))     )
#define ROMlib_FsinX(A1) \
   (     C_ROMlib_FsinX((A1))     )
#define ROMlib_FcosX(A1) \
   (     C_ROMlib_FcosX((A1))     )
#define ROMlib_FtanX(A1) \
   (     C_ROMlib_FtanX((A1))     )
#define ROMlib_FatanX(A1) \
   (     C_ROMlib_FatanX((A1))     )
#define ROMlib_FrandX(A1) \
   (     C_ROMlib_FrandX((A1))     )
#define ROMlib_Fdec2str(A1, A2, A3) \
   (     C_ROMlib_Fdec2str((A1), (A2), (A3))     )
#define ROMlib_Fcstr2dec(A1, A2, A3, A4) \
   (     C_ROMlib_Fcstr2dec((A1), (A2), (A3), (A4))     )
#define ROMlib_Fpstr2dec(A1, A2, A3, A4) \
   (     C_ROMlib_Fpstr2dec((A1), (A2), (A3), (A4))     )
#define InitFonts() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x00fe];\
   ((new_addr == toolstuff[0x00fe].orig)\
    ? C_InitFonts()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InitFonts  ));\
  })
#define GetFontName(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   StringPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x00ff];\
   ((new_addr == toolstuff[0x00ff].orig)\
    ? C_GetFontName(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetFontName ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetFNum(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   StringPtr __stub_arg_1 = (A1);\
   GUEST<INTEGER> * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0100];\
   ((new_addr == toolstuff[0x0100].orig)\
    ? C_GetFNum(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetFNum ,  __stub_arg_1, __stub_arg_2));\
  })
#define SetFontLock(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   BOOLEAN __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0103];\
   ((new_addr == toolstuff[0x0103].orig)\
    ? C_SetFontLock(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetFontLock ,  __stub_arg_1));\
  })
#define RealFont(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0102];\
   ((new_addr == toolstuff[0x0102].orig)\
    ? C_RealFont(__stub_arg_1, __stub_arg_2)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_RealFont ,  __stub_arg_1, __stub_arg_2));\
  })
#define FMSwapFont(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   FMInput * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0101];\
   ((new_addr == toolstuff[0x0101].orig)\
    ? C_FMSwapFont(__stub_arg_1)\
    : (FMOutPtr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FMSwapFont ,  __stub_arg_1));\
  })
#define FontMetrics(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   FMetricRec * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0035];\
   ((new_addr == toolstuff[0x0035].orig)\
    ? C_FontMetrics(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FontMetrics ,  __stub_arg_1));\
  })
#define SetFScaleDisable(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   BOOLEAN __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0034];\
   ((new_addr == toolstuff[0x0034].orig)\
    ? C_SetFScaleDisable(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetFScaleDisable ,  __stub_arg_1));\
  })
#define SetFractEnable(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   BOOLEAN __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0014];\
   ((new_addr == toolstuff[0x0014].orig)\
    ? C_SetFractEnable(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetFractEnable ,  __stub_arg_1));\
  })
#define IUDatePString(A1, A2, A3, A4) \
   (     C_IUDatePString((A1), (A2), (A3), (A4))     )
#define IUGetIntl(A1) \
   (     C_IUGetIntl((A1))     )
#define IUDateString(A1, A2, A3) \
   (     C_IUDateString((A1), (A2), (A3))     )
#define IUTimePString(A1, A2, A3, A4) \
   (     C_IUTimePString((A1), (A2), (A3), (A4))     )
#define IUTimeString(A1, A2, A3) \
   (     C_IUTimeString((A1), (A2), (A3))     )
#define IUMetric() \
   (     C_IUMetric()     )
#define IUSetIntl(A1, A2, A3) \
   (     C_IUSetIntl((A1), (A2), (A3))     )
#define IUMagString(A1, A2, A3, A4) \
   (     C_IUMagString((A1), (A2), (A3), (A4))     )
#define IUMagIDString(A1, A2, A3, A4) \
   (     C_IUMagIDString((A1), (A2), (A3), (A4))     )
#define IUMystery(A1, A2, A3, A4) \
   (     C_IUMystery((A1), (A2), (A3), (A4))     )
#define IULDateString(A1, A2, A3, A4) \
   (     C_IULDateString((A1), (A2), (A3), (A4))     )
#define IULTimeString(A1, A2, A3, A4) \
   (     C_IULTimeString((A1), (A2), (A3), (A4))     )
#define IUClearCache() \
   (     C_IUClearCache()     )
#define IUMagPString(A1, A2, A3, A4, A5) \
   (     C_IUMagPString((A1), (A2), (A3), (A4), (A5))     )
#define IUMagIDPString(A1, A2, A3, A4, A5) \
   (     C_IUMagIDPString((A1), (A2), (A3), (A4), (A5))     )
#define IUScriptOrder(A1, A2) \
   (     C_IUScriptOrder((A1), (A2))     )
#define IULangOrder(A1, A2) \
   (     C_IULangOrder((A1), (A2))     )
#define IUTextOrder(A1, A2, A3, A4, A5, A6, A7, A8) \
   (     C_IUTextOrder((A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8))     )
#define IUGetItlTable(A1, A2, A3, A4, A5) \
   (     C_IUGetItlTable((A1), (A2), (A3), (A4), (A5))     )
#define AcceptHighLevelEvent(A1, A2, A3, A4) \
   (     C_AcceptHighLevelEvent((A1), (A2), (A3), (A4))     )
#define GetSpecificHighLevelEvent(A1, A2, A3) \
   (     C_GetSpecificHighLevelEvent((A1), (A2), (A3))     )
#define PostHighLevelEvent(A1, A2, A3, A4, A5, A6) \
   (     C_PostHighLevelEvent((A1), (A2), (A3), (A4), (A5), (A6))     )
#define SysBeep(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01c8];\
   ((new_addr == toolstuff[0x01c8].orig)\
    ? C_SysBeep(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SysBeep ,  __stub_arg_1));\
  })
#define InitPack(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01e5];\
   ((new_addr == toolstuff[0x01e5].orig)\
    ? C_InitPack(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InitPack ,  __stub_arg_1));\
  })
#define InitAllPacks() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x01e6];\
   ((new_addr == toolstuff[0x01e6].orig)\
    ? C_InitAllPacks()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InitAllPacks  ));\
  })
#define InfoScrap() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x01f9];\
   ((new_addr == toolstuff[0x01f9].orig)\
    ? C_InfoScrap()\
    : (PScrapStuff) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_InfoScrap  ));\
  })
#define UnloadScrap() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x01fa];\
   ((new_addr == toolstuff[0x01fa].orig)\
    ? C_UnloadScrap()\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_UnloadScrap  ));\
  })
#define LoadScrap() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x01fb];\
   ((new_addr == toolstuff[0x01fb].orig)\
    ? C_LoadScrap()\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_LoadScrap  ));\
  })
#define ZeroScrap() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x01fc];\
   ((new_addr == toolstuff[0x01fc].orig)\
    ? C_ZeroScrap()\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ZeroScrap  ));\
  })
#define PutScrap(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   LONGINT __stub_arg_1 = (A1);\
   ResType __stub_arg_2 = (A2);\
   Ptr __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x01fe];\
   ((new_addr == toolstuff[0x01fe].orig)\
    ? C_PutScrap(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PutScrap ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define GetScrap(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
   ResType __stub_arg_2 = (A2);\
   GUEST<LONGINT> * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x01fd];\
   ((new_addr == toolstuff[0x01fd].orig)\
    ? C_GetScrap(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetScrap ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define GetEnvirons(A1) \
   (     C_GetEnvirons((A1))     )
#define SetEnvirons(A1, A2) \
   (     C_SetEnvirons((A1), (A2))     )
#define GetScript(A1, A2) \
   (     C_GetScript((A1), (A2))     )
#define SetScript(A1, A2, A3) \
   (     C_SetScript((A1), (A2), (A3))     )
#define Font2Script(A1) \
   (     C_Font2Script((A1))     )
#define Transliterate(A1, A2, A3, A4) \
   (     C_Transliterate((A1), (A2), (A3), (A4))     )
#define FontScript() \
   (     C_FontScript()     )
#define IntlScript() \
   (     C_IntlScript()     )
#define KeyScript(A1) \
   (     C_KeyScript((A1))     )
#define CharType(A1, A2) \
   (     C_CharType((A1), (A2))     )
#define MeasureJust(A1, A2, A3, A4) \
   (     C_MeasureJust((A1), (A2), (A3), (A4))     )
#define NMeasureJust(A1, A2, A3, A4, A5, A6, A7) \
   (     C_NMeasureJust((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define ParseTable(A1) \
   (     C_ParseTable((A1))     )
#define FillParseTable(A1, A2) \
   (     C_FillParseTable((A1), (A2))     )
#define CharacterByteType(A1, A2, A3) \
   (     C_CharacterByteType((A1), (A2), (A3))     )
#define CharacterType(A1, A2, A3) \
   (     C_CharacterType((A1), (A2), (A3))     )
#define TransliterateText(A1, A2, A3, A4, A5) \
   (     C_TransliterateText((A1), (A2), (A3), (A4), (A5))     )
#define Pixel2Char(A1, A2, A3, A4, A5) \
   (     C_Pixel2Char((A1), (A2), (A3), (A4), (A5))     )
#define Char2Pixel(A1, A2, A3, A4, A5) \
   (     C_Char2Pixel((A1), (A2), (A3), (A4), (A5))     )
#define FindWord(A1, A2, A3, A4, A5, A6) \
   (     C_FindWord((A1), (A2), (A3), (A4), (A5), (A6))     )
#define HiliteText(A1, A2, A3, A4) \
   (     C_HiliteText((A1), (A2), (A3), (A4))     )
#define DrawJust(A1, A2, A3) \
   (     C_DrawJust((A1), (A2), (A3))     )
#define String2Time(A1, A2, A3, A4, A5) \
   (     C_String2Time((A1), (A2), (A3), (A4), (A5))     )
#define String2Date(A1, A2, A3, A4, A5) \
   (     C_String2Date((A1), (A2), (A3), (A4), (A5))     )
#define StyledLineBreak(A1, A2, A3, A4, A5, A6, A7) \
   (     C_StyledLineBreak((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define ReplaceText(A1, A2, A3) \
   (     C_ReplaceText((A1), (A2), (A3))     )
#define StringToExtended(A1, A2, A3, A4) \
   (     C_StringToExtended((A1), (A2), (A3), (A4))     )
#define ExtendedToString(A1, A2, A3, A4) \
   (     C_ExtendedToString((A1), (A2), (A3), (A4))     )
#define StringToFormatRec(A1, A2, A3) \
   (     C_StringToFormatRec((A1), (A2), (A3))     )
#define ToggleDate(A1, A2, A3, A4, A5) \
   (     C_ToggleDate((A1), (A2), (A3), (A4), (A5))     )
#define TruncString(A1, A2, A3) \
   (     C_TruncString((A1), (A2), (A3))     )
#define VisibleLength(A1, A2) \
   (     C_VisibleLength((A1), (A2))     )
#define LongDate2Secs(A1, A2) \
   (     C_LongDate2Secs((A1), (A2))     )
#define LongSecs2Date(A1, A2) \
   (     C_LongSecs2Date((A1), (A2))     )
#define InitDateCache(A1) \
   (     C_InitDateCache((A1))     )
#define CharByte(A1, A2) \
   (     C_CharByte((A1), (A2))     )
#define PortionLine(A1, A2, A3, A4, A5) \
   (     C_PortionLine((A1), (A2), (A3), (A4), (A5))     )
#define DrawJustified(A1, A2, A3, A4, A5, A6) \
   (     C_DrawJustified((A1), (A2), (A3), (A4), (A5), (A6))     )
#define FindScriptRun(A1, A2, A3) \
   (     C_FindScriptRun((A1), (A2), (A3))     )
#define PixelToChar(A1, A2, A3, A4, A5, A6, A7, A8, A9) \
   (     C_PixelToChar((A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8), (A9))     )
#define CharToPixel(A1, A2, A3, A4, A5, A6, A7, A8) \
   (     C_CharToPixel((A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8))     )
#define LowercaseText(A1, A2, A3) \
   (     C_LowercaseText((A1), (A2), (A3))     )
#define UppercaseText(A1, A2, A3) \
   (     C_UppercaseText((A1), (A2), (A3))     )
#define StripDiacritics(A1, A2, A3) \
   (     C_StripDiacritics((A1), (A2), (A3))     )
#define UppercaseStripDiacritics(A1, A2, A3) \
   (     C_UppercaseStripDiacritics((A1), (A2), (A3))     )
#define GetAppParms(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   StringPtr __stub_arg_1 = (A1);\
   INTEGER * __stub_arg_2 = (A2);\
   GUEST<Handle> * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x01f5];\
   ((new_addr == toolstuff[0x01f5].orig)\
    ? C_GetAppParms(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetAppParms ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define ExitToShell() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x01f4];\
   ((new_addr == toolstuff[0x01f4].orig)\
    ? C_ExitToShell()\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_ExitToShell  ));\
  })
#define UnloadSeg(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01f1];\
   ((new_addr == toolstuff[0x01f1].orig)\
    ? C_UnloadSeg(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_UnloadSeg ,  __stub_arg_1));\
  })
#define SndPlay(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   SndChannelPtr __stub_arg_1 = (A1);\
   Handle __stub_arg_2 = (A2);\
   BOOLEAN __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0005];\
   ((new_addr == toolstuff[0x0005].orig)\
    ? C_SndPlay(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (pascal trap OSErr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SndPlay ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SndNewChannel(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   GUEST<SndChannelPtr> * __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
   LONGINT __stub_arg_3 = (A3);\
   ProcPtr __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x0007];\
   ((new_addr == toolstuff[0x0007].orig)\
    ? C_SndNewChannel(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (OSErr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SndNewChannel ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define SndAddModifier(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   SndChannelPtr __stub_arg_1 = (A1);\
   ProcPtr __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
   LONGINT __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x0002];\
   ((new_addr == toolstuff[0x0002].orig)\
    ? C_SndAddModifier(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (pascal trap OSErr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SndAddModifier ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define SndDoCommand(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   SndChannelPtr __stub_arg_1 = (A1);\
   SndCommand * __stub_arg_2 = (A2);\
   BOOLEAN __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0003];\
   ((new_addr == toolstuff[0x0003].orig)\
    ? C_SndDoCommand(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (pascal trap OSErr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SndDoCommand ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define SndDoImmediate(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   SndChannelPtr __stub_arg_1 = (A1);\
   SndCommand * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0004];\
   ((new_addr == toolstuff[0x0004].orig)\
    ? C_SndDoImmediate(__stub_arg_1, __stub_arg_2)\
    : (pascal trap OSErr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SndDoImmediate ,  __stub_arg_1, __stub_arg_2));\
  })
#define SndChannelStatus(A1, A2, A3) \
   (     C_SndChannelStatus((A1), (A2), (A3))     )
#define SndControl(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   SndCommand * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0006];\
   ((new_addr == toolstuff[0x0006].orig)\
    ? C_SndControl(__stub_arg_1, __stub_arg_2)\
    : (pascal trap OSErr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SndControl ,  __stub_arg_1, __stub_arg_2));\
  })
#define SndDisposeChannel(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   SndChannelPtr __stub_arg_1 = (A1);\
   BOOLEAN __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0001];\
   ((new_addr == toolstuff[0x0001].orig)\
    ? C_SndDisposeChannel(__stub_arg_1, __stub_arg_2)\
    : (pascal trap OSErr) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SndDisposeChannel ,  __stub_arg_1, __stub_arg_2));\
  })
#define SndGetSysBeepState(A1) \
   (     C_SndGetSysBeepState((A1))     )
#define SndSetSysBeepState(A1) \
   (     C_SndSetSysBeepState((A1))     )
#define SndManagerStatus(A1, A2) \
   (     C_SndManagerStatus((A1), (A2))     )
#define SndSoundManagerVersion() \
   (     C_SndSoundManagerVersion()     )
#define MACEVersion() \
   (     C_MACEVersion()     )
#define SPBVersion() \
   (     C_SPBVersion()     )
#define SndStartFilePlay(A1, A2, A3, A4, A5, A6, A7, A8) \
   (     C_SndStartFilePlay((A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8))     )
#define SndPauseFilePlay(A1) \
   (     C_SndPauseFilePlay((A1))     )
#define SndStopFilePlay(A1, A2) \
   (     C_SndStopFilePlay((A1), (A2))     )
#define SndPlayDoubleBuffer(A1, A2) \
   (     C_SndPlayDoubleBuffer((A1), (A2))     )
#define Comp3to1(A1, A2, A3, A4, A5, A6, A7) \
   (     C_Comp3to1((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define Comp6to1(A1, A2, A3, A4, A5, A6, A7) \
   (     C_Comp6to1((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define Exp1to3(A1, A2, A3, A4, A5, A6, A7) \
   (     C_Exp1to3((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define Exp1to6(A1, A2, A3, A4, A5, A6, A7) \
   (     C_Exp1to6((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define SndRecord(A1, A2, A3, A4) \
   (     C_SndRecord((A1), (A2), (A3), (A4))     )
#define SndRecordToFile(A1, A2, A3, A4) \
   (     C_SndRecordToFile((A1), (A2), (A3), (A4))     )
#define SPBOpenDevice(A1, A2, A3) \
   (     C_SPBOpenDevice((A1), (A2), (A3))     )
#define SPBCloseDevice(A1) \
   (     C_SPBCloseDevice((A1))     )
#define SPBRecord(A1, A2) \
   (     C_SPBRecord((A1), (A2))     )
#define SPBRecordToFile(A1, A2, A3) \
   (     C_SPBRecordToFile((A1), (A2), (A3))     )
#define SPBPauseRecording(A1) \
   (     C_SPBPauseRecording((A1))     )
#define SPBResumeRecording(A1) \
   (     C_SPBResumeRecording((A1))     )
#define SPBStopRecording(A1) \
   (     C_SPBStopRecording((A1))     )
#define SPBGetRecordingStatus(A1, A2, A3, A4, A5, A6, A7) \
   (     C_SPBGetRecordingStatus((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define SPBGetDeviceInfo(A1, A2, A3) \
   (     C_SPBGetDeviceInfo((A1), (A2), (A3))     )
#define SPBSetDeviceInfo(A1, A2, A3) \
   (     C_SPBSetDeviceInfo((A1), (A2), (A3))     )
#define SetupSndHeader(A1, A2, A3, A4, A5, A6, A7, A8) \
   (     C_SetupSndHeader((A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8))     )
#define SetupAIFFHeader(A1, A2, A3, A4, A5, A6, A7) \
   (     C_SetupAIFFHeader((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define SPBSignInDevice(A1, A2) \
   (     C_SPBSignInDevice((A1), (A2))     )
#define SPBSignOutDevice(A1) \
   (     C_SPBSignOutDevice((A1))     )
#define SPBGetIndexedDevice(A1, A2, A3) \
   (     C_SPBGetIndexedDevice((A1), (A2), (A3))     )
#define SPBMillisecondsToBytes(A1, A2) \
   (     C_SPBMillisecondsToBytes((A1), (A2))     )
#define SPBBytesToMilliseconds(A1, A2) \
   (     C_SPBBytesToMilliseconds((A1), (A2))     )
#define FinaleUnknown1() \
   (     C_FinaleUnknown1()     )
#define FinaleUnknown2(A1, A2, A3, A4) \
   (     C_FinaleUnknown2((A1), (A2), (A3), (A4))     )
#define DirectorUnknown3() \
   (     C_DirectorUnknown3()     )
#define DirectorUnknown4(A1, A2, A3, A4) \
   (     C_DirectorUnknown4((A1), (A2), (A3), (A4))     )
#define GetSysBeepVolume(A1) \
   (     C_GetSysBeepVolume((A1))     )
#define SetSysBeepVolume(A1) \
   (     C_SetSysBeepVolume((A1))     )
#define GetDefaultOutputVolume(A1) \
   (     C_GetDefaultOutputVolume((A1))     )
#define SetDefaultOutputVolume(A1) \
   (     C_SetDefaultOutputVolume((A1))     )
#define GetSoundHeaderOffset(A1, A2) \
   (     C_GetSoundHeaderOffset((A1), (A2))     )
#define UnsignedFixedMulDiv(A1, A2, A3) \
   (     C_UnsignedFixedMulDiv((A1), (A2), (A3))     )
#define GetCompressionInfo(A1, A2, A3, A4, A5) \
   (     C_GetCompressionInfo((A1), (A2), (A3), (A4), (A5))     )
#define SetSoundPreference(A1, A2, A3) \
   (     C_SetSoundPreference((A1), (A2), (A3))     )
#define GetSoundPreference(A1, A2, A3) \
   (     C_GetSoundPreference((A1), (A2), (A3))     )
#define SndGetInfo(A1, A2, A3) \
   (     C_SndGetInfo((A1), (A2), (A3))     )
#define SndSetInfo(A1, A2, A3) \
   (     C_SndSetInfo((A1), (A2), (A3))     )
#define SFPPutFile(A1, A2, A3, A4, A5, A6, A7) \
   (     C_SFPPutFile((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define SFPutFile(A1, A2, A3, A4, A5) \
   (     C_SFPutFile((A1), (A2), (A3), (A4), (A5))     )
#define SFPGetFile(A1, A2, A3, A4, A5, A6, A7, A8, A9) \
   (     C_SFPGetFile((A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8), (A9))     )
#define SFGetFile(A1, A2, A3, A4, A5, A6, A7) \
   (     C_SFGetFile((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define CustomPutFile(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) \
   (     C_CustomPutFile((A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8), (A9), (A10))     )
#define CustomGetFile(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) \
   (     C_CustomGetFile((A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8), (A9), (A10), (A11))     )
#define StandardGetFile(A1, A2, A3, A4) \
   (     C_StandardGetFile((A1), (A2), (A3), (A4))     )
#define StandardPutFile(A1, A2, A3) \
   (     C_StandardPutFile((A1), (A2), (A3))     )
#define SysError(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   short __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01c9];\
   ((new_addr == toolstuff[0x01c9].orig)\
    ? C_SysError(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SysError ,  __stub_arg_1));\
  })
#define KeyTrans(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   unsigned short __stub_arg_2 = (A2);\
   LONGINT * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x01c3];\
   ((new_addr == toolstuff[0x01c3].orig)\
    ? C_KeyTrans(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_KeyTrans ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define GetNextEvent(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   EventRecord * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0170];\
   ((new_addr == toolstuff[0x0170].orig)\
    ? C_GetNextEvent(__stub_arg_1, __stub_arg_2)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetNextEvent ,  __stub_arg_1, __stub_arg_2));\
  })
#define WaitNextEvent(A1, A2, A3, A4) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   EventRecord * __stub_arg_2 = (A2);\
   LONGINT __stub_arg_3 = (A3);\
   RgnHandle __stub_arg_4 = (A4);\
 \
   new_addr = tooltraptable[0x0060];\
   ((new_addr == toolstuff[0x0060].orig)\
    ? C_WaitNextEvent(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_WaitNextEvent ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4));\
  })
#define EventAvail(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   EventRecord * __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0171];\
   ((new_addr == toolstuff[0x0171].orig)\
    ? C_EventAvail(__stub_arg_1, __stub_arg_2)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_EventAvail ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetMouse(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   GUEST<Point> * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0172];\
   ((new_addr == toolstuff[0x0172].orig)\
    ? C_GetMouse(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetMouse ,  __stub_arg_1));\
  })
#define Button() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0174];\
   ((new_addr == toolstuff[0x0174].orig)\
    ? C_Button()\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Button  ));\
  })
#define StillDown() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0173];\
   ((new_addr == toolstuff[0x0173].orig)\
    ? C_StillDown()\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_StillDown  ));\
  })
#define WaitMouseUp() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0177];\
   ((new_addr == toolstuff[0x0177].orig)\
    ? C_WaitMouseUp()\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_WaitMouseUp  ));\
  })
#define GetKeys(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   unsigned char * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0176];\
   ((new_addr == toolstuff[0x0176].orig)\
    ? C_GetKeys(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetKeys ,  __stub_arg_1));\
  })
#define TickCount() \
 ({ \
   syn68k_addr_t new_addr;\
    \
   new_addr = tooltraptable[0x0175];\
   ((new_addr == toolstuff[0x0175].orig)\
    ? C_TickCount()\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_TickCount  ));\
  })
#define FracSqrt(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Fract __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0049];\
   ((new_addr == toolstuff[0x0049].orig)\
    ? C_FracSqrt(__stub_arg_1)\
    : (Fract) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FracSqrt ,  __stub_arg_1));\
  })
#define FracSin(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Fixed __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0048];\
   ((new_addr == toolstuff[0x0048].orig)\
    ? C_FracSin(__stub_arg_1)\
    : (Fract) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FracSin ,  __stub_arg_1));\
  })
#define FixAtan2(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   LONGINT __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0018];\
   ((new_addr == toolstuff[0x0018].orig)\
    ? C_FixAtan2(__stub_arg_1, __stub_arg_2)\
    : (Fixed) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FixAtan2 ,  __stub_arg_1, __stub_arg_2));\
  })
#define FracCos(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Fixed __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0047];\
   ((new_addr == toolstuff[0x0047].orig)\
    ? C_FracCos(__stub_arg_1)\
    : (Fract) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FracCos ,  __stub_arg_1));\
  })
#define FixRatio(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0069];\
   ((new_addr == toolstuff[0x0069].orig)\
    ? C_FixRatio(__stub_arg_1, __stub_arg_2)\
    : (Fixed) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FixRatio ,  __stub_arg_1, __stub_arg_2));\
  })
#define FixMul(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Fixed __stub_arg_1 = (A1);\
   Fixed __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0068];\
   ((new_addr == toolstuff[0x0068].orig)\
    ? C_FixMul(__stub_arg_1, __stub_arg_2)\
    : (Fixed) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FixMul ,  __stub_arg_1, __stub_arg_2));\
  })
#define FixRound(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Fixed __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x006c];\
   ((new_addr == toolstuff[0x006c].orig)\
    ? C_FixRound(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FixRound ,  __stub_arg_1));\
  })
#define NewString(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   StringPtr __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0106];\
   ((new_addr == toolstuff[0x0106].orig)\
    ? C_NewString(__stub_arg_1)\
    : (StringHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_NewString ,  __stub_arg_1));\
  })
#define SetString(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   StringHandle __stub_arg_1 = (A1);\
   StringPtr __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0107];\
   ((new_addr == toolstuff[0x0107].orig)\
    ? C_SetString(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SetString ,  __stub_arg_1, __stub_arg_2));\
  })
#define GetString(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01ba];\
   ((new_addr == toolstuff[0x01ba].orig)\
    ? C_GetString(__stub_arg_1)\
    : (StringHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetString ,  __stub_arg_1));\
  })
#define Munger(A1, A2, A3, A4, A5, A6) \
 ({ \
   syn68k_addr_t new_addr;\
   Handle __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
   Ptr __stub_arg_3 = (A3);\
   LONGINT __stub_arg_4 = (A4);\
   Ptr __stub_arg_5 = (A5);\
   LONGINT __stub_arg_6 = (A6);\
 \
   new_addr = tooltraptable[0x01e0];\
   ((new_addr == toolstuff[0x01e0].orig)\
    ? C_Munger(__stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Munger ,  __stub_arg_1, __stub_arg_2, __stub_arg_3, __stub_arg_4, __stub_arg_5, __stub_arg_6));\
  })
#define PackBits(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   GUEST<Ptr> * __stub_arg_1 = (A1);\
   GUEST<Ptr> * __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00cf];\
   ((new_addr == toolstuff[0x00cf].orig)\
    ? C_PackBits(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PackBits ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define UnpackBits(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   GUEST<Ptr> * __stub_arg_1 = (A1);\
   GUEST<Ptr> * __stub_arg_2 = (A2);\
   INTEGER __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x00d0];\
   ((new_addr == toolstuff[0x00d0].orig)\
    ? C_UnpackBits(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_UnpackBits ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define BitTst(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x005d];\
   ((new_addr == toolstuff[0x005d].orig)\
    ? C_BitTst(__stub_arg_1, __stub_arg_2)\
    : (BOOLEAN) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_BitTst ,  __stub_arg_1, __stub_arg_2));\
  })
#define BitSet(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x005e];\
   ((new_addr == toolstuff[0x005e].orig)\
    ? C_BitSet(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_BitSet ,  __stub_arg_1, __stub_arg_2));\
  })
#define BitClr(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Ptr __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x005f];\
   ((new_addr == toolstuff[0x005f].orig)\
    ? C_BitClr(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_BitClr ,  __stub_arg_1, __stub_arg_2));\
  })
#define BitAnd(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   LONGINT __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0058];\
   ((new_addr == toolstuff[0x0058].orig)\
    ? C_BitAnd(__stub_arg_1, __stub_arg_2)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_BitAnd ,  __stub_arg_1, __stub_arg_2));\
  })
#define BitOr(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   LONGINT __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x005b];\
   ((new_addr == toolstuff[0x005b].orig)\
    ? C_BitOr(__stub_arg_1, __stub_arg_2)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_BitOr ,  __stub_arg_1, __stub_arg_2));\
  })
#define BitXor(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   LONGINT __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x0059];\
   ((new_addr == toolstuff[0x0059].orig)\
    ? C_BitXor(__stub_arg_1, __stub_arg_2)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_BitXor ,  __stub_arg_1, __stub_arg_2));\
  })
#define BitNot(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   LONGINT __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x005a];\
   ((new_addr == toolstuff[0x005a].orig)\
    ? C_BitNot(__stub_arg_1)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_BitNot ,  __stub_arg_1));\
  })
#define BitShift(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   LONGINT __stub_arg_1 = (A1);\
   INTEGER __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x005c];\
   ((new_addr == toolstuff[0x005c].orig)\
    ? C_BitShift(__stub_arg_1, __stub_arg_2)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_BitShift ,  __stub_arg_1, __stub_arg_2));\
  })
#undef HiWord
#undef LoWord
#define HiWord(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   LONGINT __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x006a];\
   ((new_addr == toolstuff[0x006a].orig)\
    ? C_HiWord(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_HiWord ,  __stub_arg_1));\
  })
#define LoWord(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   LONGINT __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x006b];\
   ((new_addr == toolstuff[0x006b].orig)\
    ? C_LoWord(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_LoWord ,  __stub_arg_1));\
  })
#define LongMul(A1, A2, A3) \
 ({ \
   syn68k_addr_t new_addr;\
   LONGINT __stub_arg_1 = (A1);\
   LONGINT __stub_arg_2 = (A2);\
   Int64Bit * __stub_arg_3 = (A3);\
 \
   new_addr = tooltraptable[0x0067];\
   ((new_addr == toolstuff[0x0067].orig)\
    ? C_LongMul(__stub_arg_1, __stub_arg_2, __stub_arg_3)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_LongMul ,  __stub_arg_1, __stub_arg_2, __stub_arg_3));\
  })
#define GetPattern(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01b8];\
   ((new_addr == toolstuff[0x01b8].orig)\
    ? C_GetPattern(__stub_arg_1)\
    : (PatHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetPattern ,  __stub_arg_1));\
  })
#define GetCursor(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01b9];\
   ((new_addr == toolstuff[0x01b9].orig)\
    ? C_GetCursor(__stub_arg_1)\
    : (CursHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetCursor ,  __stub_arg_1));\
  })
#define GetPicture(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01bc];\
   ((new_addr == toolstuff[0x01bc].orig)\
    ? C_GetPicture(__stub_arg_1)\
    : (PicHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetPicture ,  __stub_arg_1));\
  })
#define DeltaPoint(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Point __stub_arg_1 = (A1);\
   Point __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x014f];\
   ((new_addr == toolstuff[0x014f].orig)\
    ? C_DeltaPoint(__stub_arg_1, __stub_arg_2)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DeltaPoint ,  __stub_arg_1, __stub_arg_2));\
  })
#define SlopeFromAngle(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   INTEGER __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00bc];\
   ((new_addr == toolstuff[0x00bc].orig)\
    ? C_SlopeFromAngle(__stub_arg_1)\
    : (Fixed) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_SlopeFromAngle ,  __stub_arg_1));\
  })
#define AngleFromSlope(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Fixed __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x00c4];\
   ((new_addr == toolstuff[0x00c4].orig)\
    ? C_AngleFromSlope(__stub_arg_1)\
    : (INTEGER) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_AngleFromSlope ,  __stub_arg_1));\
  })
#define FracMul(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Fract __stub_arg_1 = (A1);\
   Fract __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x004a];\
   ((new_addr == toolstuff[0x004a].orig)\
    ? C_FracMul(__stub_arg_1, __stub_arg_2)\
    : (Fract) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FracMul ,  __stub_arg_1, __stub_arg_2));\
  })
#define FixDiv(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Fixed __stub_arg_1 = (A1);\
   Fixed __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x004d];\
   ((new_addr == toolstuff[0x004d].orig)\
    ? C_FixDiv(__stub_arg_1, __stub_arg_2)\
    : (Fixed) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FixDiv ,  __stub_arg_1, __stub_arg_2));\
  })
#define FracDiv(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   Fract __stub_arg_1 = (A1);\
   Fract __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x004b];\
   ((new_addr == toolstuff[0x004b].orig)\
    ? C_FracDiv(__stub_arg_1, __stub_arg_2)\
    : (Fract) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_FracDiv ,  __stub_arg_1, __stub_arg_2));\
  })
#define Long2Fix(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   LONGINT __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x003f];\
   ((new_addr == toolstuff[0x003f].orig)\
    ? C_Long2Fix(__stub_arg_1)\
    : (Fixed) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Long2Fix ,  __stub_arg_1));\
  })
#define Fix2Long(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Fixed __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0040];\
   ((new_addr == toolstuff[0x0040].orig)\
    ? C_Fix2Long(__stub_arg_1)\
    : (LONGINT) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Fix2Long ,  __stub_arg_1));\
  })
#define Fix2Frac(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Fixed __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0041];\
   ((new_addr == toolstuff[0x0041].orig)\
    ? C_Fix2Frac(__stub_arg_1)\
    : (Fract) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Fix2Frac ,  __stub_arg_1));\
  })
#define Frac2Fix(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   Fract __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0042];\
   ((new_addr == toolstuff[0x0042].orig)\
    ? C_Frac2Fix(__stub_arg_1)\
    : (Fixed) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_Frac2Fix ,  __stub_arg_1));\
  })
#define R_X2Fix(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   extended80 * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0044];\
   ((new_addr == toolstuff[0x0044].orig)\
    ? C_R_X2Fix(__stub_arg_1)\
    : (Fixed) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_R_X2Fix ,  __stub_arg_1));\
  })
#define R_X2Frac(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   extended80 * __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0046];\
   ((new_addr == toolstuff[0x0046].orig)\
    ? C_R_X2Frac(__stub_arg_1)\
    : (Fract) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_R_X2Frac ,  __stub_arg_1));\
  })
#define ShutDwnPower() \
   (     C_ShutDwnPower()     )
#define ShutDwnStart() \
   (     C_ShutDwnStart()     )
#define ShutDwnInstall(A1, A2) \
   (     C_ShutDwnInstall((A1), (A2))     )
#define ShutDwnRemove(A1) \
   (     C_ShutDwnRemove((A1))     )
#define GetCurrentProcess(A1) \
   (     C_GetCurrentProcess((A1))     )
#define GetNextProcess(A1) \
   (     C_GetNextProcess((A1))     )
#define GetProcessInformation(A1, A2) \
   (     C_GetProcessInformation((A1), (A2))     )
#define SameProcess(A1, A2, A3) \
   (     C_SameProcess((A1), (A2), (A3))     )
#define GetFrontProcess(A1, A2) \
   (     C_GetFrontProcess((A1), (A2))     )
#define SetFrontProcess(A1) \
   (     C_SetFrontProcess((A1))     )
#define WakeUpProcess(A1) \
   (     C_WakeUpProcess((A1))     )
#define GetProcessSerialNumberFromPortName(A1, A2) \
   (     C_GetProcessSerialNumberFromPortName((A1), (A2))     )
#define GetPortNameFromProcessSerialNumber(A1, A2) \
   (     C_GetPortNameFromProcessSerialNumber((A1), (A2))     )
#define FindFolder(A1, A2, A3, A4, A5) \
   (     C_FindFolder((A1), (A2), (A3), (A4), (A5))     )
#define NewAlias(A1, A2, A3) \
   (     C_NewAlias((A1), (A2), (A3))     )
#define UpdateAlias(A1, A2, A3, A4) \
   (     C_UpdateAlias((A1), (A2), (A3), (A4))     )
#define ResolveAlias(A1, A2, A3, A4) \
   (     C_ResolveAlias((A1), (A2), (A3), (A4))     )
#define ResolveAliasFile(A1, A2, A3, A4) \
   (     C_ResolveAliasFile((A1), (A2), (A3), (A4))     )
#define MatchAlias(A1, A2, A3, A4, A5, A6, A7, A8) \
   (     C_MatchAlias((A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8))     )
#define GetAliasInfo(A1, A2, A3) \
   (     C_GetAliasInfo((A1), (A2), (A3))     )
#define NewAliasMinimalFromFullPath(A1, A2, A3, A4, A5) \
   (     C_NewAliasMinimalFromFullPath((A1), (A2), (A3), (A4), (A5))     )
#define NewAliasMinimal(A1, A2) \
   (     C_NewAliasMinimal((A1), (A2))     )
#define TempFreeMem() \
   (     C_TempFreeMem()     )
#define TempMaxMem(A1) \
   (     C_TempMaxMem((A1))     )
#define TempTopMem() \
   (     C_TempTopMem()     )
#define TempNewHandle(A1, A2) \
   (     C_TempNewHandle((A1), (A2))     )
#define TempHLock(A1, A2) \
   (     C_TempHLock((A1), (A2))     )
#define TempHUnlock(A1, A2) \
   (     C_TempHUnlock((A1), (A2))     )
#define TempDisposeHandle(A1, A2) \
   (     C_TempDisposeHandle((A1), (A2))     )
#define InitEditionPack(A1) \
   (     C_InitEditionPack((A1))     )
#define NewSection(A1, A2, A3, A4, A5, A6) \
   (     C_NewSection((A1), (A2), (A3), (A4), (A5), (A6))     )
#define RegisterSection(A1, A2, A3) \
   (     C_RegisterSection((A1), (A2), (A3))     )
#define UnRegisterSection(A1) \
   (     C_UnRegisterSection((A1))     )
#define IsRegisteredSection(A1) \
   (     C_IsRegisteredSection((A1))     )
#define AssociateSection(A1, A2) \
   (     C_AssociateSection((A1), (A2))     )
#define CreateEditionContainerFile(A1, A2, A3) \
   (     C_CreateEditionContainerFile((A1), (A2), (A3))     )
#define DeleteEditionContainerFile(A1) \
   (     C_DeleteEditionContainerFile((A1))     )
#define SetEditionFormatMark(A1, A2, A3) \
   (     C_SetEditionFormatMark((A1), (A2), (A3))     )
#define GetEditionFormatMark(A1, A2, A3) \
   (     C_GetEditionFormatMark((A1), (A2), (A3))     )
#define OpenEdition(A1, A2) \
   (     C_OpenEdition((A1), (A2))     )
#define EditionHasFormat(A1, A2, A3) \
   (     C_EditionHasFormat((A1), (A2), (A3))     )
#define ReadEdition(A1, A2, A3, A4) \
   (     C_ReadEdition((A1), (A2), (A3), (A4))     )
#define OpenNewEdition(A1, A2, A3, A4) \
   (     C_OpenNewEdition((A1), (A2), (A3), (A4))     )
#define WriteEdition(A1, A2, A3, A4) \
   (     C_WriteEdition((A1), (A2), (A3), (A4))     )
#define CloseEdition(A1, A2) \
   (     C_CloseEdition((A1), (A2))     )
#define GetLastEditionContainerUsed(A1) \
   (     C_GetLastEditionContainerUsed((A1))     )
#define NewSubscriberDialog(A1) \
   (     C_NewSubscriberDialog((A1))     )
#define NewPublisherDialog(A1) \
   (     C_NewPublisherDialog((A1))     )
#define SectionOptionsDialog(A1) \
   (     C_SectionOptionsDialog((A1))     )
#define NewSubscriberExpDialog(A1, A2, A3, A4, A5, A6) \
   (     C_NewSubscriberExpDialog((A1), (A2), (A3), (A4), (A5), (A6))     )
#define NewPublisherExpDialog(A1, A2, A3, A4, A5, A6) \
   (     C_NewPublisherExpDialog((A1), (A2), (A3), (A4), (A5), (A6))     )
#define SectionOptionsExpDialog(A1, A2, A3, A4, A5, A6) \
   (     C_SectionOptionsExpDialog((A1), (A2), (A3), (A4), (A5), (A6))     )
#define GetEditionInfo(A1, A2) \
   (     C_GetEditionInfo((A1), (A2))     )
#define GoToPublisherSection(A1) \
   (     C_GoToPublisherSection((A1))     )
#define GetStandardFormats(A1, A2, A3, A4, A5) \
   (     C_GetStandardFormats((A1), (A2), (A3), (A4), (A5))     )
#define GetEditionOpenerProc(A1) \
   (     C_GetEditionOpenerProc((A1))     )
#define SetEditionOpenerProc(A1) \
   (     C_SetEditionOpenerProc((A1))     )
#define CallEditionOpenerProc(A1, A2, A3) \
   (     C_CallEditionOpenerProc((A1), (A2), (A3))     )
#define CallFormatIOProc(A1, A2, A3) \
   (     C_CallFormatIOProc((A1), (A2), (A3))     )
#define SetOutlinePreferred(A1) \
   (     C_SetOutlinePreferred((A1))     )
#define GetOutlinePreferred() \
   (     C_GetOutlinePreferred()     )
#define IsOutline(A1, A2) \
   (     C_IsOutline((A1), (A2))     )
#define OutlineMetrics(A1, A2, A3, A4, A5, A6, A7, A8, A9) \
   (     C_OutlineMetrics((A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8), (A9))     )
#define SetPreserveGlyph(A1) \
   (     C_SetPreserveGlyph((A1))     )
#define GetPreserveGlyph() \
   (     C_GetPreserveGlyph()     )
#define FlushFonts() \
   (     C_FlushFonts()     )
#define HMGetBalloons() \
   (     C_HMGetBalloons()     )
#define HMSetBalloons(A1) \
   (     C_HMSetBalloons((A1))     )
#define HMIsBalloon() \
   (     C_HMIsBalloon()     )
#define HMShowBalloon(A1, A2, A3, A4, A5, A6, A7) \
   (     C_HMShowBalloon((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define HMShowMenuBalloon(A1, A2, A3, A4, A5, A6, A7, A8, A9) \
   (     C_HMShowMenuBalloon((A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8), (A9))     )
#define HMRemoveBalloon() \
   (     C_HMRemoveBalloon()     )
#define HMGetHelpMenuHandle(A1) \
   (     C_HMGetHelpMenuHandle((A1))     )
#define HMGetFont(A1) \
   (     C_HMGetFont((A1))     )
#define HMGetFontSize(A1) \
   (     C_HMGetFontSize((A1))     )
#define HMSetFont(A1) \
   (     C_HMSetFont((A1))     )
#define HMSetFontSize(A1) \
   (     C_HMSetFontSize((A1))     )
#define HMSetDialogResID(A1) \
   (     C_HMSetDialogResID((A1))     )
#define HMGetDialogResID(A1) \
   (     C_HMGetDialogResID((A1))     )
#define HMSetMenuResID(A1, A2) \
   (     C_HMSetMenuResID((A1), (A2))     )
#define HMGetMenuResID(A1, A2) \
   (     C_HMGetMenuResID((A1), (A2))     )
#define HMScanTemplateItems(A1, A2, A3) \
   (     C_HMScanTemplateItems((A1), (A2), (A3))     )
#define HMBalloonRect(A1, A2) \
   (     C_HMBalloonRect((A1), (A2))     )
#define HMBalloonPict(A1, A2) \
   (     C_HMBalloonPict((A1), (A2))     )
#define HMGetBalloonWindow(A1) \
   (     C_HMGetBalloonWindow((A1))     )
#define HMExtractHelpMsg(A1, A2, A3, A4, A5) \
   (     C_HMExtractHelpMsg((A1), (A2), (A3), (A4), (A5))     )
#define PlotIconID(A1, A2, A3, A4) \
   (     C_PlotIconID((A1), (A2), (A3), (A4))     )
#define PlotIconMethod(A1, A2, A3, A4, A5) \
   (     C_PlotIconMethod((A1), (A2), (A3), (A4), (A5))     )
#define PlotIcon(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   const Rect * __stub_arg_1 = (A1);\
   Handle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x014b];\
   ((new_addr == toolstuff[0x014b].orig)\
    ? C_PlotIcon(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PlotIcon ,  __stub_arg_1, __stub_arg_2));\
  })
#define PlotIconHandle(A1, A2, A3, A4) \
   (     C_PlotIconHandle((A1), (A2), (A3), (A4))     )
#define PlotCIcon(A1, A2) \
 ({ \
   syn68k_addr_t new_addr;\
   const Rect * __stub_arg_1 = (A1);\
   CIconHandle __stub_arg_2 = (A2);\
 \
   new_addr = tooltraptable[0x021f];\
   ((new_addr == toolstuff[0x021f].orig)\
    ? C_PlotCIcon(__stub_arg_1, __stub_arg_2)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_PlotCIcon ,  __stub_arg_1, __stub_arg_2));\
  })
#define PlotCIconHandle(A1, A2, A3, A4) \
   (     C_PlotCIconHandle((A1), (A2), (A3), (A4))     )
#define PlotSICNHandle(A1, A2, A3, A4) \
   (     C_PlotSICNHandle((A1), (A2), (A3), (A4))     )
#define GetIcon(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   short __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x01bb];\
   ((new_addr == toolstuff[0x01bb].orig)\
    ? C_GetIcon(__stub_arg_1)\
    : (Handle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetIcon ,  __stub_arg_1));\
  })
#define GetCIcon(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   short __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x021e];\
   ((new_addr == toolstuff[0x021e].orig)\
    ? C_GetCIcon(__stub_arg_1)\
    : (CIconHandle) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_GetCIcon ,  __stub_arg_1));\
  })
#define DisposeCIcon(A1) \
 ({ \
   syn68k_addr_t new_addr;\
   CIconHandle __stub_arg_1 = (A1);\
 \
   new_addr = tooltraptable[0x0225];\
   ((new_addr == toolstuff[0x0225].orig)\
    ? C_DisposeCIcon(__stub_arg_1)\
    : (void) CToPascalCall (SYN68K_TO_US(new_addr), CTOP_DisposeCIcon ,  __stub_arg_1));\
  })
#define GetIconSuite(A1, A2, A3) \
   (     C_GetIconSuite((A1), (A2), (A3))     )
#define NewIconSuite(A1) \
   (     C_NewIconSuite((A1))     )
#define AddIconToSuite(A1, A2, A3) \
   (     C_AddIconToSuite((A1), (A2), (A3))     )
#define GetIconFromSuite(A1, A2, A3) \
   (     C_GetIconFromSuite((A1), (A2), (A3))     )
#define PlotIconSuite(A1, A2, A3, A4) \
   (     C_PlotIconSuite((A1), (A2), (A3), (A4))     )
#define ForEachIconDo(A1, A2, A3, A4) \
   (     C_ForEachIconDo((A1), (A2), (A3), (A4))     )
#define GetSuiteLabel(A1) \
   (     C_GetSuiteLabel((A1))     )
#define SetSuiteLabel(A1, A2) \
   (     C_SetSuiteLabel((A1), (A2))     )
#define GetLabel(A1, A2, A3) \
   (     C_GetLabel((A1), (A2), (A3))     )
#define DisposeIconSuite(A1, A2) \
   (     C_DisposeIconSuite((A1), (A2))     )
#define IconSuiteToRgn(A1, A2, A3, A4) \
   (     C_IconSuiteToRgn((A1), (A2), (A3), (A4))     )
#define IconIDToRgn(A1, A2, A3, A4) \
   (     C_IconIDToRgn((A1), (A2), (A3), (A4))     )
#define IconMethodToRgn(A1, A2, A3, A4, A5) \
   (     C_IconMethodToRgn((A1), (A2), (A3), (A4), (A5))     )
#define PtInIconSuite(A1, A2, A3, A4) \
   (     C_PtInIconSuite((A1), (A2), (A3), (A4))     )
#define PtInIconID(A1, A2, A3, A4) \
   (     C_PtInIconID((A1), (A2), (A3), (A4))     )
#define PtInIconMethod(A1, A2, A3, A4, A5) \
   (     C_PtInIconMethod((A1), (A2), (A3), (A4), (A5))     )
#define RectInIconSuite(A1, A2, A3, A4) \
   (     C_RectInIconSuite((A1), (A2), (A3), (A4))     )
#define RectInIconID(A1, A2, A3, A4) \
   (     C_RectInIconID((A1), (A2), (A3), (A4))     )
#define RectInIconMethod(A1, A2, A3, A4, A5) \
   (     C_RectInIconMethod((A1), (A2), (A3), (A4), (A5))     )
#define MakeIconCache(A1, A2, A3) \
   (     C_MakeIconCache((A1), (A2), (A3))     )
#define LoadIconCache(A1, A2, A3, A4) \
   (     C_LoadIconCache((A1), (A2), (A3), (A4))     )
#define GetIconCacheData(A1, A2) \
   (     C_GetIconCacheData((A1), (A2))     )
#define SetIconCacheData(A1, A2) \
   (     C_SetIconCacheData((A1), (A2))     )
#define GetIconCacheProc(A1, A2) \
   (     C_GetIconCacheProc((A1), (A2))     )
#define SetIconCacheProc(A1, A2) \
   (     C_SetIconCacheProc((A1), (A2))     )
#define EnterMovies() \
   (     C_EnterMovies()     )
#define ExitMovies() \
   (     C_ExitMovies()     )
#define MoviesTask(A1, A2) \
   (     C_MoviesTask((A1), (A2))     )
#define PrerollMovie(A1, A2, A3) \
   (     C_PrerollMovie((A1), (A2), (A3))     )
#define SetMovieActive(A1, A2) \
   (     C_SetMovieActive((A1), (A2))     )
#define StartMovie(A1) \
   (     C_StartMovie((A1))     )
#define StopMovie(A1) \
   (     C_StopMovie((A1))     )
#define GoToBeginningOfMovie(A1) \
   (     C_GoToBeginningOfMovie((A1))     )
#define SetMovieGWorld(A1, A2, A3) \
   (     C_SetMovieGWorld((A1), (A2), (A3))     )
#define UpdateMovie(A1) \
   (     C_UpdateMovie((A1))     )
#define DisposeMovie(A1) \
   (     C_DisposeMovie((A1))     )
#define GetMovieVolume(A1) \
   (     C_GetMovieVolume((A1))     )
#define CloseMovieFile(A1) \
   (     C_CloseMovieFile((A1))     )
#define IsMovieDone(A1) \
   (     C_IsMovieDone((A1))     )
#define NewMovieFromFile(A1, A2, A3, A4, A5, A6) \
   (     C_NewMovieFromFile((A1), (A2), (A3), (A4), (A5), (A6))     )
#define GetMoviePreferredRate(A1) \
   (     C_GetMoviePreferredRate((A1))     )
#define GetMovieBox(A1, A2) \
   (     C_GetMovieBox((A1), (A2))     )
#define SetMovieBox(A1, A2) \
   (     C_SetMovieBox((A1), (A2))     )
#define NewMovieController(A1, A2, A3) \
   (     C_NewMovieController((A1), (A2), (A3))     )
#define DisposeMovieController(A1) \
   (     C_DisposeMovieController((A1))     )
#define OpenMovieFile(A1, A2, A3) \
   (     C_OpenMovieFile((A1), (A2), (A3))     )
#define GetSharedLibrary(A1, A2, A3, A4, A5, A6) \
   (     C_GetSharedLibrary((A1), (A2), (A3), (A4), (A5), (A6))     )
#define CloseConnection(A1) \
   (     C_CloseConnection((A1))     )
#define GetMemFragment(A1, A2, A3, A4, A5, A6, A7) \
   (     C_GetMemFragment((A1), (A2), (A3), (A4), (A5), (A6), (A7))     )
#define GetDiskFragment(A1, A2, A3, A4, A5, A6, A7, A8) \
   (     C_GetDiskFragment((A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8))     )
#define CountSymbols(A1, A2) \
   (     C_CountSymbols((A1), (A2))     )
#define GetIndSymbol(A1, A2, A3, A4, A5) \
   (     C_GetIndSymbol((A1), (A2), (A3), (A4), (A5))     )
#define FindSymbol(A1, A2, A3, A4) \
   (     C_FindSymbol((A1), (A2), (A3), (A4))     )
#undef NewRoutineDescriptor
#undef DisposeRoutineDescriptor
#define NewRoutineDescriptor(A1, A2, A3) \
   (     C_NewRoutineDescriptor((A1), (A2), (A3))     )
#define DisposeRoutineDescriptor(A1) \
   (     C_DisposeRoutineDescriptor((A1))     )
#define NewFatRoutineDescriptor(A1, A2, A3) \
   (     C_NewFatRoutineDescriptor((A1), (A2), (A3))     )
#define SaveMixedModeState(A1, A2) \
   (     C_SaveMixedModeState((A1), (A2))     )
#define RestoreMixedModeState(A1, A2) \
   (     C_RestoreMixedModeState((A1), (A2))     )

#ifdef __cplusplus
}
#endif
#endif /* !defined(_STUBIFY_H_) */
