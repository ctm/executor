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

static OSStatus FSSpecToPOSIXPath (const FSSpec *inSpec, char *ioPath, unsigned long inPathLength)
{
  OSStatus err = noErr;
  FSRef ref;
  FSSpec spec;
  CFStringRef nameString = NULL;
  CFStringRef pathString = NULL;
  CFURLRef pathURL = NULL;
  CFURLRef parentURL = NULL;
  
  // First, try to create an FSRef for the FSSpec
  if (err == noErr) {
	err = FSpMakeFSRef (inSpec, &ref);
  }
  
  if (err == noErr) {
	// It's a directory or a file that exists; convert directly into a path
	err = FSRefMakePath (&ref, (UInt8 *)ioPath, inPathLength);
  } else {
	// The suck case.  It's a file that doesn't exist.
	err = noErr;
	
	// Build an FSSpec for the parent directory, which must exist
	if (err == noErr) {
	  Str31 name;
	  name[0] = 0;
	  
	  err = FSMakeFSSpec (inSpec->vRefNum, inSpec->parID, name, &spec);
	}
    
	// Build an FSRef for the parent directory
	if (err == noErr) {
	  err = FSpMakeFSRef (&spec, &ref);
	}
    
	// Now make a CFURL for the parent
	if (err == noErr) {
	  parentURL = CFURLCreateFromFSRef(CFAllocatorGetDefault (), &ref);
	  if (parentURL == NULL) { err = memFullErr; }
	}
    
	if (err == noErr) {
	  nameString = CFStringCreateWithPascalString (CFAllocatorGetDefault (), inSpec->name,
												   kCFStringEncodingMacRoman);
	  if (nameString == NULL) { err = memFullErr; }
	}
    
	// Now we just add the filename back onto the path
	if (err == noErr) {
	  pathURL = CFURLCreateCopyAppendingPathComponent (CFAllocatorGetDefault (),
													   parentURL, nameString,
													   false /* Not a directory */);
	  if (pathURL == NULL) { err = memFullErr; }
	}
    
	if (err == noErr) {
	  pathString = CFURLCopyFileSystemPath (pathURL, kCFURLPOSIXPathStyle);
	  if (pathString == NULL) { err = memFullErr; }
	}
    
	if (err == noErr) {
	  Boolean converted = CFStringGetCString (pathString, ioPath, inPathLength, CFStringGetSystemEncoding ());
	  if (!converted) { err = fnfErr; }
	}
  }
  
  // Free allocated memory
  if (parentURL != NULL)  { CFRelease (parentURL);  }
  if (nameString != NULL) { CFRelease (nameString); }
  if (pathURL != NULL)    { CFRelease (pathURL);    }
  if (pathString != NULL) { CFRelease (pathString); }
  
  return err;
}

static OSStatus POSIXPathToFSSpec (const char *inPath, FSSpec *outSpec)
{
  OSStatus err = noErr;
  FSRef ref;
  Boolean isDirectory;
  FSCatalogInfo info;
  CFStringRef pathString = NULL;
  CFURLRef pathURL = NULL;
  CFURLRef parentURL = NULL;
  CFStringRef nameString = NULL;
  
  // First, try to create an FSRef for the full path
  if (err == noErr) {
	err = FSPathMakeRef ((UInt8 *) inPath, &ref, &isDirectory);
  }
  
  if (err == noErr) {
	// It's a directory or a file that exists; convert directly into an FSSpec:
	err = FSGetCatalogInfo (&ref, kFSCatInfoNone, NULL, NULL, outSpec, NULL);
  } else {
	// The suck case.  The file doesn't exist.
	err = noErr;
    
	// Get a CFString for the path
	if (err == noErr) {
	  pathString = CFStringCreateWithCString (CFAllocatorGetDefault (), inPath, CFStringGetSystemEncoding ());
	  if (pathString == NULL) { err = memFullErr; }
	}
    
	// Get a CFURL for the path
	if (err == noErr) {
	  pathURL = CFURLCreateWithFileSystemPath (CFAllocatorGetDefault (),
											   pathString, kCFURLPOSIXPathStyle,
											   false /* Not a directory */);
	  if (pathURL == NULL) { err = memFullErr; }
	}
	
	// Get a CFURL for the parent
	if (err == noErr) {
	  parentURL = CFURLCreateCopyDeletingLastPathComponent (CFAllocatorGetDefault (), pathURL);
	  if (parentURL == NULL) { err = memFullErr; }
	}
	
	// Build an FSRef for the parent directory, which must be valid to make an FSSpec
	if (err == noErr) {
	  Boolean converted = CFURLGetFSRef (parentURL, &ref);
	  if (!converted) { err = fnfErr; }
	}
	
	// Get the node ID of the parent directory
	if (err == noErr) {
	  err = FSGetCatalogInfo(&ref, kFSCatInfoNodeFlags|kFSCatInfoNodeID, &info, NULL, outSpec, NULL);
	}
	
	// Get a CFString for the file name
	if (err == noErr) {
	  nameString = CFURLCopyLastPathComponent (pathURL);
	  if (nameString == NULL) { err = memFullErr; }
	}
	
	// Copy the string into the FSSpec
	if (err == noErr) {
	  Boolean converted = CFStringGetPascalString (pathString, outSpec->name, sizeof (outSpec->name),
												   CFStringGetSystemEncoding ());
	  if (!converted) { err = fnfErr; }
	}
    
	// Set the node ID in the FSSpec
	if (err == noErr) {
	  outSpec->parID = info.nodeID;
	}
  }
  
  // Free allocated memory
  if (pathURL != NULL)    { CFRelease (pathURL);    }
  if (pathString != NULL) { CFRelease (pathString); }
  if (parentURL != NULL)  { CFRelease (parentURL);  }
  if (nameString != NULL) { CFRelease (nameString); }
  
  return err;
}

OSStatus MacBridge::MacFSSpecToExecutorFSSpec(const ::FSSpec *inMac, Executor::FSSpec *outExec)
{
  char aPath[PATH_MAX] = {0};
  OSStatus theErr = BigEndianValue(FSSpecToPOSIXPath(inMac, aPath, PATH_MAX));
  CFURLRef aURL = ::CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (UInt8*)aPath, strlen(aPath), false);
  theErr = MacCFURLToExecutorFSSpec(aURL, outExec);
  CFRelease(aURL);

  return theErr;
}

OSStatus MacBridge::MacCFURLToExecutorFSSpec(CFURLRef inMac, Executor::FSSpec *outExec)
{
  OSStatus theErr = CLC((OSStatus)noErr);
  char aPath[PATH_MAX] = {0};
  if (!CFURLGetFileSystemRepresentation(inMac, false, (UInt8*)aPath, PATH_MAX))
  {
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
