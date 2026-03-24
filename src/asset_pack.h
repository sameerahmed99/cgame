#ifndef _CG_ASSET_PACK
#define _CG_ASSET_PACK
#include "types.h"


enum CG_AssetType{
  TEXTURE,
  MODEL,
  AUDIO
};

typedef struct CG_AssetPackHeader{
  u32 magic;
  u32 version;
  u32 numAssets;
  u32 assetTableOffset;
  u32 assetDataOffset;
}  CG_AssetPackHeader;


typedef struct CG_AssetTableEntry{
  enum CG_AssetType type;
  u32 assetOffset;
  u32 numBytes;
} CG_AssetTableEntry;

typedef struct CG_Asset{
  enum CG_AssetType type;
  b32 loaded;
  u32 numUsers;
  void* data;
} CG_Asset;

typedef struct CG_AssetTable{
  CG_AssetTableEntry* entries;
} CG_AssetTable;

typedef struct CG_Assets{

  CG_AssetTable assetTable;
  void* binFileHandle;
  
} CG_Assets;




#endif
