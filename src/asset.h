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
  u64 tableOffset;
  u64 dataOffset;
  u64 dataSize;
  u64 totalPackSize;
}  CG_AssetPackHeader;


typedef struct CG_AssetTableEntry{
  CG_AssetId id;
  enum CG_AssetType type;
  u64 offset;
  size_t dataSize;
} CG_AssetTableEntry;



typedef struct CG_RuntimeAsset{
  CG_AssetId id;
  enum CG_AssetType type;
  u32 numUsers;
  size_t dataSize;
  void *data;
} CG_RuntimeAsset;



typedef struct CG_AssetPack{
  CG_AssetPackHeader header;
  CG_AssetTableEntry* entries;
  void* data;
} CG_AssetPack;

typedef struct CG_RuntimeAssets {
  u32 numAssets;
  Arena* loadedAssets;
  CG_AssetTableEntry* packEntries;
  void* binHandle;
  size_t binFileSize;
} CG_RuntimeAssets;

typedef struct CG_TextureAssetBin{
  u32 Width;
  u32 Height;
  u32 BytesPerPixel;
  u64 PixelDataRelativeOffset;
} CG_TextureAssetBin;




void asset_write_assets(const char *_rawAssetsDir, const char *_binFilePath);
CG_RuntimeAssets asset_read_pack(const char *_packPath);

#endif

