//
//  stdfile-OSXPrivate.h
//  CocoaExecutor
//
//  Created by C.W. Betts on 8/9/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

#ifndef __CocoaExecutor__stdfile_OSXPrivate__
#define __CocoaExecutor__stdfile_OSXPrivate__

#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>

#include "stdfile.h"

namespace MacBridge
{
PUBLIC OSStatus MacCFURLToExecutorFSSpec(CFURLRef inMac, Executor::FSSpec *outExec);
}

#endif /* defined(__CocoaExecutor__stdfile_OSXPrivate__) */
