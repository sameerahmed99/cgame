#include "asset.h"
#include "platform.h"
#include "xxhash.h"
#include "cgame.h"
#include <stdio.h>
#define CG_XXHASH_SEED 0
#define CG_ASSET_PACK_MAX_DATA_SIZE Gigabytes(5)
CG_AssetPack CG_Asset_CurrentPack;
u64 CG_Asset_CurrentWriteAssetIndex = 0;
u64 CG_Asset_CurrentPackWritePos;
Arena *CG_Asset_CurrentWorkingArena;


u64 asset_relative_path_to_hash(const char* string){
 size_t length = (string == NULL) ? 0 : strlen(string);
  return XXH64(string, length, CG_XXHASH_SEED);
}
void asset_recursive_read_callback(int index, const char *_relativePath, const char *_absolutePath){

  char extension[16];
  u32 len = strlen(_relativePath);
  u64 hash = asset_relative_path_to_hash(_relativePath);
  
  for(int i=len-1;i>=0;i--){
    if(_relativePath[i] == '.'){
      strcpy(extension, _relativePath + i);
      break;
    }
  }

   if(strcmp(extension,".png")==0){
     printf("Found .png: %s\n", _relativePath);
        CG_Texture* tex = texture_load_from_file(_relativePath, CG_Asset_CurrentWorkingArena);
    u64 pixelDataSize = texture_get_pixel_data_size_in_bytes(tex);
    u64 totalWriteSize = 0;

    CG_TextureAssetBin texAsset;
    u64 texAssetSize = sizeof(texAsset);

    texAsset.Width = tex->Width;
    texAsset.Height = tex->Height;
    texAsset.BytesPerPixel = tex->BytesPerPixel;
    texAsset.PixelDataOffset = CG_Asset_CurrentPackWritePos+texAssetSize;

    u8* dest = (u8*)&CG_Asset_CurrentPack.data[0];
    dest+=CG_Asset_CurrentPackWritePos;
    memcpy(dest, &texAsset, texAssetSize);
    dest+=texAssetSize;
    memcpy(dest, tex->Pixels, pixelDataSize);
    
    CG_AssetTableEntry entry;
    entry.id = hash;
    entry.type = CG_ASSET_TYPE_TEXTURE;
    entry.offset = CG_Asset_CurrentPackWritePos;

    totalWriteSize = texAssetSize + pixelDataSize;
    
    CG_Asset_CurrentPack.entries[CG_Asset_CurrentWriteAssetIndex] = entry;
    CG_Asset_CurrentWriteAssetIndex++;
    
    CG_Asset_CurrentPackWritePos += totalWriteSize;
    CG_Asset_CurrentPack.header.dataSize+=totalWriteSize;
    
    arena_pop(CG_Asset_CurrentWorkingArena, texture_get_total_size_in_bytes(tex));

    ASSERT_NO_EVAL(CG_Asset_CurrentPack.header.dataSize <= CG_ASSET_PACK_MAX_DATA_SIZE);
  }
  else if(extension == ".glb"){

  }
  else if(extension == ".wav"){

  }
  else if(extension == ".ttf"){

  }

  //  printf("Found asset: %s\n", _relativePath); 
  printf("Hash: %llu\n", hash);
}


void asset_write_pack_to_bin(const char* _targetPath, CG_AssetPack *_pack){
  u8* bytes = malloc(_pack->header.totalPackSize);
  u64 writePos = 0;

  u64 writeSize = sizeof(_pack->header);
  memcpy(bytes, &_pack->header, writeSize);
  writePos+=writeSize;

  writeSize=_pack->header.numAssets*sizeof(CG_AssetTableEntry);
  memcpy(bytes+writePos, &_pack->entries[0],writeSize);
  writePos+=writeSize;

  writeSize = _pack->header.dataSize;
  memcpy(bytes+writePos, _pack->data, writeSize);
  writePos+=writeSize;

  platform_write_or_overwrite_file(_targetPath, bytes, _pack->header.totalPackSize);

  free(bytes);
}



void asset_write_assets(const char *_rawAssetsDir, const char *_binFilePath){

  if(CG_Asset_CurrentWorkingArena!=NULL){
    arena_clear(CG_Asset_CurrentWorkingArena);
  }
  else{
    CG_Asset_CurrentWorkingArena = arena_create(Gigabytes(4), Megabytes(500), false);
  }
int numAssets = platform_recursively_read_files_in_directory(_rawAssetsDir, NULL);
 CG_AssetPack pack = {0};

 u64 tableTotalSize  = sizeof(CG_AssetTableEntry)*numAssets;
 u64 headerSize = sizeof(CG_AssetPackHeader);
 pack.header.magic = CG_ASSET_BIN_MAGIC;
 pack.header.version = 0;
 pack.header.numAssets = numAssets;
 pack.header.tableOffset = headerSize;
 pack.header.dataOffset = sizeof(pack);
 pack.header.dataSize = 0;
 pack.entries = malloc(tableTotalSize);
 pack.data = malloc(CG_ASSET_PACK_MAX_DATA_SIZE);

 ASSERT_NO_EVAL(pack.entries);
 ASSERT_NO_EVAL(pack.data);


 
 CG_Asset_CurrentPack = pack;
 CG_Asset_CurrentPackWritePos = pack.header.dataOffset;


   //@TODO if assets start becoming too large to hold all of them in memory at the same time
  // modify this function to write assets one by one to the file
platform_recursively_read_files_in_directory(_rawAssetsDir, asset_recursive_read_callback);



 
 u64 totalPackSize = CG_Asset_CurrentPack.header.dataSize + headerSize + tableTotalSize;
 CG_Asset_CurrentPack.header.totalPackSize = totalPackSize;
 asset_write_pack_to_bin(_binFilePath,&CG_Asset_CurrentPack);


 free(pack.entries);
 free(pack.data);
}
CG_RuntimeAssets asset_read_pack(const char *_packPath){
  CG_RuntimeAssets ass = {0};
  size_t fileSize = 0;
  size_t headerSize = sizeof(CG_AssetPackHeader);
  void *file = platform_platform_open_file(_packPath, &fileSize);
  void *data = platform_read_part_of_opened_file(file, 0,headerSize, fileSize);
  
  CG_AssetPackHeader* header = data;
  ASSERT_NO_EVAL(header->magic == CG_ASSET_BIN_MAGIC);
  printf("Magic: %lu, Version %lu, NumAssets %lu, tableOffset %lu, dataOffset %lu, dataSize %lu, totalPackSize: %lu\n", header->magic, header->version, header->numAssets, header->tableOffset, header->dataOffset, header->dataSize, header->totalPackSize);
  return ass;
}
