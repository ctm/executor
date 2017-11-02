#if !defined(__RSYS_HFS_PLUS__)
#define __RSYS_HFS_PLUS__

/*
 * Copyright 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: hfs_plus.h 63 2004-12-24 18:19:43Z ctm $
 */
namespace Executor {
#if 1
typedef uint8_t UInt8;
typedef uint16_t UInt16;
typedef uint32_t UInt32;
typedef uint64_t UInt64;

typedef int8_t SInt8;
typedef int16_t SInt16;
typedef int32_t SInt32;
typedef int64_t SInt64;

typedef uint16_t UniChar;

typedef UInt32 HFSCatalogNodeID;
#endif

struct HFSUniStr255 { GUEST_STRUCT;
    GUEST< UInt16> length;
    GUEST< UniChar[255]> unicode;
};

typedef const HFSUniStr255 *ConstHFSUniStr255Param;

/*
 * May need a set of textEncoding values here
 */

struct HFSPlusPermissions { GUEST_STRUCT;
    GUEST< UInt32> ownerID;
    GUEST< UInt32> groupID;
    GUEST< UInt32> permissions;
    GUEST< UInt32> specialDevice;
};

struct HFSPlusExtentDescriptor { GUEST_STRUCT;
    GUEST< UInt32> startBlock;
    GUEST< UInt32> blockCount;
};

typedef HFSPlusExtentDescriptor HFSPlusExtentRecord[8];

struct HFSPlusForkData { GUEST_STRUCT;
    GUEST< UInt64> logicalSize;
    GUEST< UInt32> clumpSize;
    GUEST< UInt32> totalBlocks;
    GUEST< HFSPlusExtentRecord> extents;
};

struct HFSPlusVolumeHeader { GUEST_STRUCT;
    GUEST< UInt16> signature;
    GUEST< UInt16> version;
    GUEST< UInt32> attributes;
    GUEST< UInt32> lastMountedVersion;
    GUEST< UInt32> reserved;
    GUEST< UInt32> createDate;
    GUEST< UInt32> modifyDate;
    GUEST< UInt32> backupDate;
    GUEST< UInt32> checkedDate;
    GUEST< UInt32> fileCount;
    GUEST< UInt32> folderCount;
    GUEST< UInt32> blockSize;
    GUEST< UInt32> totalBlocks;
    GUEST< UInt32> freeBlocks;
    GUEST< UInt32> nextAllocation;
    GUEST< UInt32> rsrcClumpSize;
    GUEST< UInt32> dataClumpSize;
    GUEST< HFSCatalogNodeID> nextCatalogID;
    GUEST< UInt32> writeCount;
    GUEST< UInt64> encodingsBitmap;
    GUEST< UInt8[32]> finderInfo;
    GUEST< HFSPlusForkData> allocationFile;
    GUEST< HFSPlusForkData> extentsFile;
    GUEST< HFSPlusForkData> catalogFile;
    GUEST< HFSPlusForkData> attributesFile;
    GUEST< HFSPlusForkData> startupFile;
};

struct BTNodeDescriptor { GUEST_STRUCT;
    GUEST< UInt32> fLink;
    GUEST< UInt32> bLink;
    GUEST< SInt8> kind;
    GUEST< UInt8> height;
    GUEST< UInt16> numRecords;
    GUEST< UInt16> reserved;
};

struct BTHeaderRec { GUEST_STRUCT;
    GUEST< UInt16> treeDepth;
    GUEST< UInt32> rootNode;
    GUEST< UInt32> leafRecords;
    GUEST< UInt32> firstLeafNode;
    GUEST< UInt32> lastLeafNode;
    GUEST< UInt16> nodeSize;
    GUEST< UInt16> maxKeyLength;
    GUEST< UInt32> totalNodes;
    GUEST< UInt32> freeNodes;
    GUEST< UInt16> reserfed1;
    GUEST< UInt32> clumpSize;
    GUEST< UInt8> btreeType;
    GUEST< UInt8> reserfed2;
    GUEST< UInt32> attributes;
    GUEST< UInt32[16]> reserved3;
};

struct HFSPlusCatalogKey { GUEST_STRUCT;
    GUEST< UInt16> keyLength;
    GUEST< HFSCatalogNodeID> parentID;
    GUEST< HFSUniStr255> nodeName;
};

struct HFSPlusCatalogFolder { GUEST_STRUCT;
    GUEST< SInt16> recordType;
    GUEST< UInt16> flags;
    GUEST< UInt32> valence;
    GUEST< HFSCatalogNodeID> folderID;
    GUEST< UInt32> createDate;
    GUEST< UInt32> contentModDate;
    GUEST< UInt32> attributeModDate;
    GUEST< UInt32> accessDate;
    GUEST< UInt32> backupDate;
    GUEST< HFSPlusPermissions> permissions;
    GUEST< DInfo> userInfo;
    GUEST< DXInfo> finderInfo;
    GUEST< UInt32> textEncoding;
    GUEST< UInt32> reserved;
};

struct HFSPlusCatalogFile { GUEST_STRUCT;
    GUEST< SInt16> recordType;
    GUEST< UInt16> flags;
    GUEST< UInt32> reserved1;
    GUEST< HFSCatalogNodeID> fileID;
    GUEST< UInt32> createDate;
    GUEST< UInt32> contentModDate;
    GUEST< UInt32> attributeModDate;
    GUEST< UInt32> accessDate;
    GUEST< UInt32> backupDate;
    GUEST< HFSPlusPermissions> permissions;
    GUEST< FInfo> userInfo;
    GUEST< FXInfo> finderInfo;
    GUEST< UInt32> textEncoding;
    GUEST< UInt32> reserved2;
    GUEST< HFSPlusForkData> dataFork;
    GUEST< HFSPlusForkData> resourceFork;
};

struct HFSPlusCatalogThread { GUEST_STRUCT;
    GUEST< SInt16> recordType;
    GUEST< SInt16> reserved;
    GUEST< HFSCatalogNodeID> parentID;
    GUEST< HFSUniStr255> nodeName;
};

struct HFSPlusExtentKey { GUEST_STRUCT;
    GUEST< UInt16> keyLength;
    GUEST< UInt8> forkType;
    GUEST< UInt8> pad;
    GUEST< HFSCatalogNodeID> fileID;
    GUEST< UInt32> startBlock;
};

struct HFSPlusAttrForkData { GUEST_STRUCT;
    GUEST< UInt32> recordType;
    GUEST< UInt32> reserved;
    GUEST< HFSPlusForkData> theFork;
};

struct HFSPlusAttrExtents { GUEST_STRUCT;
    GUEST< UInt32> recordType;
    GUEST< UInt32> reserved;
    GUEST< HFSPlusExtentRecord> extents;
};

extern bool ROMlib_hfs_plus_support;
}
#endif
