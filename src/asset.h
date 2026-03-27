#ifndef _CG_ASSET_PACK
#define _CG_ASSET_PACK
#include "types.h"


#define CG_ASSET_BIN_MAGIC 0x4019FFCC

typedef u64 CG_AssetId;

enum CG_AssetType{
  CG_ASSET_TYPE_TEXTURE,
  CG_ASSET_TYPE_MODEL,
  CG_ASSET_TYPE_AUDIO
};

typedef struct CG_AssetPackHeader{
  u32 magic;
  u32 version;
  u32 numAssets;
  u32 tableOffset;
  u32 dataOffset;
  u32 dataSize;
}  CG_AssetPackHeader;


typedef struct CG_AssetTableEntry{
  CG_AssetId id;
  enum CG_AssetType type;
  u32 offset;
} CG_AssetTableEntry;



typedef struct CG_Asset{
  CG_AssetId id;
  enum CG_AssetType type;
  u32 numUsers;
  void* data;
  u32 numBytes;
  
} CG_Asset;



typedef struct CG_AssetPack{
  CG_AssetPackHeader header;
  CG_AssetTableEntry* entries;
  void* data;
} CG_AssetPack;



typedef struct CG_TextureAssetBin{
  u32 Width;
  u32 Height;
  u32 BytesPerPixel;
  u32 PixelDataOffset;
} CG_TextureAssetBin;




CG_AssetPack asset_write_assets(const char *_rawAssetsDir, const char *_binFilePath);


#endif
