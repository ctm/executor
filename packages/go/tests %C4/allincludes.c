void InitToolbox (void);

#include <ActionAtomIntf.h>
#include <ADSP.h>
#include <AEObjects.h>
#include <AEPackObject.h>
#include <AERegistry.h>
#include <AIFF.h>
#include <Aliases.h>
#include <AppleEvents.h>
#include <AppleTalk.h>
#include <Balloons.h>
#include <BDC.h>
#include <CommResources.h>
#include <Components.h>
#include <Connections.h>
#include <ConnectionTools.h>
#include <Controls.h>
#include <CRMSerialDevices.h>
#include <CTBUtilities.h>
#include <DatabaseAccess.h>
#include <Desk.h>
#include <DeskBus.h>
#include <Devices.h>
#include <Dialogs.h>
#include <DiskInit.h>
#include <Disks.h>
#include <Editions.h>
#include <ENET.h>
#include <EPPC.h>
#include <Errors.h>
#include <Events.h>
#include <Files.h>
#include <FileTransfers.h>
#include <FileTransferTools.h>
#include <Finder.h>
#include <FixMath.h>
#include <Folders.h>
#include <Fonts.h>
#include <GestaltEqu.h>
#include <Graf3D.h>
#include <HyperXCmd.h>
#include <Icons.h>
#include <ImageCodec.h>
#include <ImageCompression.h>
#include <Language.h>
#include <Lists.h>
#include <MediaHandlers.h>
#include <Memory.h>
#include <Menus.h>
#include <MIDI.h>
#include <Movies.h>
#include <MoviesFormat.h>
#include <Notification.h>
#include <OSEvents.h>
#include <OSUtils.h>
#include <Packages.h>
#include <Palette.h>
#include <Palettes.h>
#include <Picker.h>
#include <PictUtil.h>
#include <Power.h>
#include <PPCToolBox.h>
#include <Printing.h>
#include <PrintTraps.h>
#include <Processes.h>
#include <QDOffscreen.h>
#include <Quickdraw.h>
#include <QuickTimeComponents.h>
#include <Resources.h>
#include <Retrace.h>
#include <ROMDefs.h>
#include <SANE.h>
#include <Scrap.h>
#include <Script.h>
#include <SCSI.h>
#include <SegLoad.h>
#include <Serial.h>
#include <ShutDown.h>
#include <Slots.h>
#include <Sound.h>
#include <SoundInput.h>
#include <StandardFile.h>
#include <Start.h>
/* #include <SysEqu.h> */
#include <Terminals.h>
#include <TerminalTools.h>
#include <TextEdit.h>
#include <Timer.h>
#include <ToolUtils.h>
#include <Types.h>
#include <Values.h>
#include <Video.h>
#include <Windows.h>
#include <pascal.h>
#include <asm.h>
#include <THINK.h>
#include <Traps.h>

#include "allincludes.proto.h"


void
InitToolbox (void)
{
  InitGraf ((Ptr) & qd.thePort);
  InitFonts ();
  InitWindows ();
  InitMenus ();
  FlushEvents (everyEvent, 0);
  TEInit ();
  InitDialogs (0L);
  InitCursor ();
}

main (void)
{
  InitToolbox ();

  return 0;
}
