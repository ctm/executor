#if !defined(__RSYS_HFS_PLUS__)
#define __RSYS_HFS_PLUS__

/*
 * Copyright 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: hfs_plus.h 63 2004-12-24 18:19:43Z ctm $
 */

#if 1
typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned long UInt32;
typedef unsigned long long UInt64;

typedef signed char SInt8;
typedef signed short SInt16;
typedef signed long SInt32;
typedef signed long long SInt64;

typedef unsigned short UniChar;

typedef UInt32 HFSCatalogNodeID;
#endif

typedef struct PACKED HFSUniStr255
{
  UInt16 length;
  UniChar unicode[255];
}
HFSUniStr255;

typedef const HFSUniStr255 *ConstHFSUniStr255Param;

/*
 * May need a set of textEncoding values here
 */

typedef struct PACKED HFSPlusPermissions
{
  UInt32 ownerID;
  UInt32 groupID;
  UInt32 permissions;
  UInt32 specialDevice;
}
HFSPlusPermissions;

typedef struct PACKED HFSPlusExtentDescriptor
{
  UInt32 startBlock;
  UInt32 blockCount;
}
HFSPlusExtentDescriptor;

typedef HFSPlusExtentDescriptor HFSPlusExtentRecord[8];

typedef struct PACKED HFSPlusForkData
{
  UInt64 logicalSize;
  UInt32 clumpSize;
  UInt32 totalBlocks;
  HFSPlusExtentRecord extents;
}
HFSPlusForkData;

typedef struct PACKED HFSPlusVolumeHeader
{
  UInt16 signature;
  UInt16 version;
  UInt32 attributes;
  UInt32 lastMountedVersion;
  UInt32 reserved;
  UInt32 createDate;
  UInt32 modifyDate;
  UInt32 backupDate;
  UInt32 checkedDate;
  UInt32 fileCount;
  UInt32 folderCount;
  UInt32 blockSize;
  UInt32 totalBlocks;
  UInt32 freeBlocks;
  UInt32 nextAllocation;
  UInt32 rsrcClumpSize;
  UInt32 dataClumpSize;
  HFSCatalogNodeID nextCatalogID;
  UInt32 writeCount;
  UInt64 encodingsBitmap;
  UInt8 finderInfo[32];
  HFSPlusForkData allocationFile;
  HFSPlusForkData extentsFile;
  HFSPlusForkData catalogFile;
  HFSPlusForkData attributesFile;
  HFSPlusForkData startupFile;
}
HFSPlusVolumeHeader;

typedef struct PACKED BTNodeDescriptor
{
  UInt32 fLink;
  UInt32 bLink;
  SInt8 kind;
  UInt8 height;
  UInt16 numRecords;
  UInt16 reserved;
}
BTNodeDescriptor;

typedef struct PACKED BTHeaderRec
{
  UInt16 treeDepth;
  UInt32 rootNode;
  UInt32 leafRecords;
  UInt32 firstLeafNode;
  UInt32 lastLeafNode;
  UInt16 nodeSize;
  UInt16 maxKeyLength;
  UInt32 totalNodes;
  UInt32 freeNodes;
  UInt16 reserfed1;
  UInt32 clumpSize;
  UInt8 btreeType;
  UInt8 reserfed2;
  UInt32 attributes;
  UInt32 reserved3[16];
}
BTHeaderRec;

typedef struct PACKED HFSPlusCatalogKey
{
  UInt16 keyLength;
  HFSCatalogNodeID parentID;
  HFSUniStr255 nodeName;
}
HFSPlusCatalogKey;

typedef struct PACKED HFSPlusCatalogFolder
{
  SInt16 recordType;
  UInt16 flags;
  UInt32 valence;
  HFSCatalogNodeID folderID;
  UInt32 createDate;
  UInt32 contentModDate;
  UInt32 attributeModDate;
  UInt32 accessDate;
  UInt32 backupDate;
  HFSPlusPermissions permissions;
  DInfo userInfo;
  DXInfo finderInfo;
  UInt32 textEncoding;
  UInt32 reserved;
}
HFSPlusCatalogFolder;

typedef struct PACKED HFSPlusCatalogFile
{
  SInt16 recordType;
  UInt16 flags;
  UInt32 reserved1;
  HFSCatalogNodeID fileID;
  UInt32 createDate;
  UInt32 contentModDate;
  UInt32 attributeModDate;
  UInt32 accessDate;
  UInt32 backupDate;
  HFSPlusPermissions permissions;
  FInfo userInfo;
  FXInfo finderInfo;
  UInt32 textEncoding;
  UInt32 reserved2;
  HFSPlusForkData dataFork;
  HFSPlusForkData resourceFork;
}
HFSPlusCatalogFile;

typedef struct PACKED HFSPlusCatalogThread
{
  SInt16 recordType;
  SInt16 reserved;
  HFSCatalogNodeID parentID;
  HFSUniStr255 nodeName;
}
HFSPlusCatalogThread;

typedef struct PACKED HFSPlusExtentKey
{
  UInt16 keyLength;
  UInt8 forkType;
  UInt8 pad;
  HFSCatalogNodeID fileID;
  UInt32 startBlock;
}
HFSPlusExtentKey;

typedef struct PACKED HFSPlusAttrForkData
{
  UInt32 recordType;
  UInt32 reserved;
  HFSPlusForkData theFork;
}
HFSPlusAttrForkData;

typedef struct PACKED HFSPlusAttrExtents
{
  UInt32 recordType;
  UInt32 reserved;
  HFSPlusExtentRecord extents;
}
HFSPlusAttrExtents;

extern boolean_t ROMlib_hfs_plus_support;

#endif
