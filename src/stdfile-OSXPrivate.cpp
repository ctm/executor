//
//  stdfile-OSXPrivate.cpp
//  CocoaExecutor
//
//  Created by C.W. Betts on 8/9/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

#include <Carbon/Carbon.h>
#include "rsys/common.h"
#include "rsys/hfs.h"
#include "stdfile-OSXPrivate.h"
#include <limits.h>

#undef FSMakeFSSpec
using namespace MacBridge;
using namespace ByteSwap;


OSStatus MacBridge::MacCFURLToExecutorFSSpec(CFURLRef inMac, Executor::FSSpec *outExec)
{
  OSStatus theErr = CLC((OSStatus)noErr);
  char aPath[PATH_MAX] = {0};
  if (!CFURLGetFileSystemRepresentation(inMac, false, (UInt8*)aPath, PATH_MAX)) {
	return CLC((OSStatus)fnfErr);
  }
  CFStringRef aStr = CFURLCopyLastPathComponent(inMac);
  Executor::Str255 strName = {0};
  if (!CFStringGetPascalString(aStr, strName, 256, kCFStringEncodingMacRoman)) {
	theErr = CLC((OSStatus)fnfErr);
  }
  CFRelease(aStr);
  
  Executor::HVCB *customPart = Executor::ROMlib_vcbbybiggestunixname(aPath);
  Executor::C_FSMakeFSSpec(customPart->vcbDrvNum, customPart->vcbDirIDM, strName, outExec);
  
  return theErr;
}
