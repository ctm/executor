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

typedef struct HFSUniStr255
{
  UInt16 length PACKED;
  UniChar unicode[255] PACKED;
}
HFSUniStr255;

typedef const HFSUniStr255 *ConstHFSUniStr255Param;

/*
 * May need a set of textEncoding values here
 */

typedef struct HFSPlusPermissions
{
  UInt32 ownerID PACKED;
  UInt32 groupID PACKED;
  UInt32 permissions PACKED;
  UInt32 specialDevice PACKED;
}
HFSPlusPermissions;

typedef struct HFSPlusExtentDescriptor
{
  UInt32 startBlock PACKED;
  UInt32 blockCount PACKED;
}
HFSPlusExtentDescriptor;

typedef HFSPlusExtentDescriptor HFSPlusExtentRecord[8];

typedef struct HFSPlusForkData
{
  UInt64 logicalSize PACKED;
  UInt32 clumpSize PACKED;
  UInt32 totalBlocks PACKED;
  HFSPlusExtentRecord extents PACKED;
}
HFSPlusForkData;

typedef struct HFSPlusVolumeHeader
{
  UInt16 signature PACKED;
  UInt16 version PACKED;
  UInt32 attributes PACKED;
  UInt32 lastMountedVersion PACKED;
  UInt32 reserved PACKED;
  UInt32 createDate PACKED;
  UInt32 modifyDate PACKED;
  UInt32 backupDate PACKED;
  UInt32 checkedDate PACKED;
  UInt32 fileCount PACKED;
  UInt32 folderCount PACKED;
  UInt32 blockSize PACKED;
  UInt32 totalBlocks PACKED;
  UInt32 freeBlocks PACKED;
  UInt32 nextAllocation PACKED;
  UInt32 rsrcClumpSize PACKED;
  UInt32 dataClumpSize PACKED;
  HFSCatalogNodeID nextCatalogID PACKED;
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

typedef struct BTNodeDescriptor
{
  UInt32 fLink PACKED;
  UInt32 bLink PACKED;
  SInt8 kind PACKED;
  UInt8 height PACKED;
  UInt16 numRecords PACKED;
  UInt16 reserved PACKED;
}
BTNodeDescriptor;

typedef struct BTHeaderRec
{
  UInt16 treeDepth PACKED;
  UInt32 rootNode PACKED;
  UInt32 leafRecords PACKED;
  UInt32 firstLeafNode PACKED;
  UInt32 lastLeafNode PACKED;
  UInt16 nodeSize PACKED;
  UInt16 maxKeyLength PACKED;
  UInt32 totalNodes PACKED;
  UInt32 freeNodes PACKED;
  UInt16 reserfed1 PACKED;
  UInt32 clumpSize PACKED;
  UInt8 btreeType PACKED;
  UInt8 reserfed2 PACKED;
  UInt32 attributes PACKED;
  UInt32 reserved3[16] PACKED;
}
BTHeaderRec;

typedef struct HFSPlusCatalogKey
{
  UInt16 keyLength PACKED;
  HFSCatalogNodeID parentID PACKED;
  HFSUniStr255 nodeName PACKED;
}
HFSPlusCatalogKey;

typedef struct HFSPlusCatalogFolder
{
  SInt16 recordType PACKED;
  UInt16 flags PACKED;
  UInt32 valence PACKED;
  HFSCatalogNodeID folderID;
  UInt32 createDate PACKED;
  UInt32 contentModDate PACKED;
  UInt32 attributeModDate PACKED;
  UInt32 accessDate PACKED;
  UInt32 backupDate PACKED;
  HFSPlusPermissions permissions PACKED;
  DInfo userInfo PACKED;
  DXInfo finderInfo PACKED;
  UInt32 textEncoding PACKED;
  UInt32 reserved PACKED;
}
HFSPlusCatalogFolder;

typedef struct HFSPlusCatalogFile
{
  SInt16 recordType PACKED;
  UInt16 flags PACKED;
  UInt32 reserved1 PACKED;
  HFSCatalogNodeID fileID PACKED;
  UInt32 createDate PACKED;
  UInt32 contentModDate PACKED;
  UInt32 attributeModDate PACKED;
  UInt32 accessDate PACKED;
  UInt32 backupDate PACKED;
  HFSPlusPermissions permissions PACKED;
  FInfo userInfo PACKED;
  FXInfo finderInfo PACKED;
  UInt32 textEncoding PACKED;
  UInt32 reserved2 PACKED;
  HFSPlusForkData dataFork PACKED;
  HFSPlusForkData resourceFork PACKED;
}
HFSPlusCatalogFile;

typedef struct HFSPlusCatalogThread
{
  SInt16 recordType PACKED;
  SInt16 reserved PACKED;
  HFSCatalogNodeID parentID PACKED;
  HFSUniStr255 nodeName PACKED;
}
HFSPlusCatalogThread;

typedef struct HFSPlusExtentKey
{
  UInt16 keyLength PACKED;
  UInt8 forkType PACKED;
  UInt8 pad PACKED;
  HFSCatalogNodeID fileID PACKED;
  UInt32 startBlock PACKED;
}
HFSPlusExtentKey;

typedef struct HFSPlusAttrForkData
{
  UInt32 recordType PACKED;
  UInt32 reserved PACKED;
  HFSPlusForkData theFork PACKED;
}
HFSPlusAttrForkData;

typedef struct HFSPlusAttrExtents
{
  UInt32 recordType PACKED;
  UInt32 reserved PACKED;
  HFSPlusExtentRecord extents PACKED;
}
HFSPlusAttrExtents;

extern boolean_t ROMlib_hfs_plus_support;

#endif
