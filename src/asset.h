
#ifndef _CG_ASSET_PACK
#define _CG_ASSET_PACK
#include "types.h"


#define CG_ASSET_BIN_MAGIC 0x4019FFCC

typedef u64 CG_AssetId;

enum CG_AssetType{
  TEXTURE,
  MODEL,
  AUDIO
};

typedef struct CG_AssetPackHeader{
  u32 magic;
  u32 version;
  u32 numAssets;
  u32 tableOffset;
  u32 dataOffset;
}  CG_AssetPackHeader;


typedef struct CG_AssetTableEntry{
  CG_AssetId id;
  enum CG_AssetType type;
  u32 offset;
  u32 numBytes;
} CG_AssetTableEntry;


typedef struct CG_TextureAsset{
  u32 width;
  u32 height;
  u32 bytesPerPixel;
  u32 dataOffset;
} CG_TextureAsset;

typedef struct CG_ModelAsset{

} CG_ModelAsset;

typedef struct CG_MaterialAsset{
  u32 numVertices;
  u32 numIndices;
  u32 verticesOffset;
  u32 indicesOffset;
} CG_MaterialAsset;

typedef struct CG_Asset{
  CG_AssetId id;
  enum CG_AssetType type;
  u32 numUsers;
  void* data;
  u32 numBytes;
  
} CG_Asset;

typedef struct CG_AssetTable{
  CG_AssetTableEntry* entries;
} CG_AssetTable;

typedef struct CG_Assets{
  CG_AssetTable assetTable;
  CG_Asset* loadedAssets;
  void* binFileHandle;
  
} CG_Assets;

typedef struct CG_AssetPack{
  CG_AssetPackHeader header;
  CG_AssetTable table;
  void* data;
} CG_AssetPack;






CG_AssetPack asset_write_assets(const char *_rawAssetsDir, const char *_binFilePath);


#endif
