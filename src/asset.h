#ifndef _CG_ASSET_PACK
#define _CG_ASSET_PACK
#include "types.h"


#define CG_ASSET_BIN_MAGIC 0x4019FFCC
#define CG_ASSID(v) (asset_relative_path_to_hash((v)))
typedef u64 CG_AssetId;

const CG_AssetId CG_ASSET_UNINITIALIZED_ID = 0xFFFFFFFFFFFFFFFF;

enum CG_AssetType{
  CG_ASSET_TYPE_TEXTURE,
  CG_ASSET_TYPE_MODEL,
  CG_ASSET_TYPE_AUDIO,
  CG_ASSET_TYPE_FONT
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


CG_AssetId asset_relative_path_to_hash(const char* string);

void asset_write_assets(const char *_rawAssetsDir, const char *_binFilePath);
CG_RuntimeAssets asset_read_pack(const char *_packPath);


CG_RuntimeAsset *asset_load_from_id(CG_RuntimeAssets *_assets, CG_AssetId _id, enum CG_AssetType _type, b32 _returnRegardlessOfType);
CG_RuntimeAsset *asset_load(CG_RuntimeAssets *_assets, const char* _stringPath, enum CG_AssetType _type, b32 _returnRegardlessOfType);



struct CG_Texture;
struct CG_Texture *asset_load_texture(CG_RuntimeAssets *_assets, CG_AssetId id);

struct CG_Font;
struct CG_Font *asset_load_font(CG_RuntimeAssets *_assets, CG_AssetId id);

#endif

