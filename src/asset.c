#include "asset.h"
#include "platform.h"
#include "xxhash.h"
#include "cgame.h"
#include <stdio.h>
#define CG_XXHASH_SEED 0
#define CG_ASSET_PACK_MAX_DATA_SIZE Gigabytes(5)
CG_AssetPack CG_Asset_CurrentPack;
u64 CG_Asset_CurrentWriteAssetIndex = 0;
u64 CG_Asset_CurrentPackWritePosFromDataPointer;
const char* CG_Asset_CurrentPackWriteRootDir;
Arena *CG_Asset_CurrentWorkingArena;


CG_AssetId asset_relative_path_to_hash(const char* string){
 size_t length = (string == NULL) ? 0 : strlen(string);
  return XXH64(string, length, CG_XXHASH_SEED);
}
void asset_recursive_read_callback(int index, const char *_relativePath, const char *_absolutePath){

  char extension[16];
  u32 len = strlen(_relativePath);

  char relativePathWithoutRoot[CG_PLATFORM_MAX_PATH_SIZE];

  // the -1 in pathLen and the +1 in the copy position are to get rid of the /
  // so that instead of /fonts/... we get fonts/...
  u32 pathLen=strlen(_relativePath) - strlen(CG_Asset_CurrentPackWriteRootDir)-1;
  memcpy(relativePathWithoutRoot, _relativePath+strlen(CG_Asset_CurrentPackWriteRootDir)+1,pathLen);

  relativePathWithoutRoot[pathLen]='\0';


  u64 hash = asset_relative_path_to_hash(relativePathWithoutRoot);
  
  for(int i=len-1;i>=0;i--){
    if(_relativePath[i] == '.'){
      strcpy(extension, _relativePath + i);
      break;
    }
  }

  if(strcmp(extension,".png")==0){
    CG_Texture* tex = texture_load_from_file(_relativePath, CG_Asset_CurrentWorkingArena);
    tex->id = hash;
    u64 pixelDataSize = texture_get_pixel_data_size_in_bytes(tex);
    u64 totalWriteSize = 0;

    u64 texAssetSize = sizeof(*tex);
    totalWriteSize = texture_get_total_size_in_bytes(tex);
    
    /* texAsset.Width = tex->Width; */
    /* texAsset.Height = tex->Height; */
    /* texAsset.BytesPerPixel = tex->BytesPerPixel; */
    /* texAsset.PixelDataRelativeOffset = texAssetSize; */

    u8* dest = (u8*)CG_Asset_CurrentPack.data;
    dest+=CG_Asset_CurrentPackWritePosFromDataPointer;
    memcpy(dest, tex, texAssetSize);
    dest+=texAssetSize;
    memcpy(dest, tex->Pixels, pixelDataSize);


    
    
    CG_AssetTableEntry entry;
    entry.id = hash;
    entry.type = CG_ASSET_TYPE_TEXTURE;
    entry.offset = CG_Asset_CurrentPackWritePosFromDataPointer + CG_Asset_CurrentPack.header.dataOffset;
    entry.dataSize = totalWriteSize;


    
    CG_Asset_CurrentPack.entries[CG_Asset_CurrentWriteAssetIndex] = entry;
    CG_Asset_CurrentWriteAssetIndex++;
    
    CG_Asset_CurrentPackWritePosFromDataPointer += totalWriteSize;
    CG_Asset_CurrentPack.header.dataSize+=totalWriteSize;
    
    arena_pop(CG_Asset_CurrentWorkingArena, texture_get_total_size_in_bytes(tex));

  
  }
  else if(strcmp(extension,".glb") == 0){

  }
  else if(strcmp(extension,".wav") == 0){

  }
  else if(strcmp(extension,".cgfont")==0){
    CG_Font* font = font_load_from_cg_font_file(_relativePath,false);
    font->assetId = hash;
    u64 totalWriteSize = sizeof(*font);
    u8* dest = (u8*)CG_Asset_CurrentPack.data;
    dest+=CG_Asset_CurrentPackWritePosFromDataPointer;
    memcpy(dest, font, totalWriteSize);
    
    CG_AssetTableEntry entry;
    entry.id = hash;
    entry.type = CG_ASSET_TYPE_FONT;
    entry.offset = CG_Asset_CurrentPackWritePosFromDataPointer + CG_Asset_CurrentPack.header.dataOffset;
    entry.dataSize = totalWriteSize;


    CG_Asset_CurrentPack.entries[CG_Asset_CurrentWriteAssetIndex] = entry;
    CG_Asset_CurrentWriteAssetIndex++;
    
    CG_Asset_CurrentPackWritePosFromDataPointer += totalWriteSize;
    CG_Asset_CurrentPack.header.dataSize+=totalWriteSize;

    font_free_cg_font(font);

  }

  //  printf("Found asset: %s\n", _relativePath); 
   //  printf("Hash: %u\n", hash);

  ASSERT_NO_EVAL(CG_Asset_CurrentPack.header.dataSize <= CG_ASSET_PACK_MAX_DATA_SIZE);
}


void asset_write_pack_to_bin(const char* _targetPath, CG_AssetPack *_pack){
  //@TEMP_ALLOC_USED
  u8* bytes = malloc(_pack->header.totalPackSize);
  u64 writePos = 0;

  u64 writeSize = sizeof(_pack->header);
  memcpy(bytes, &_pack->header, writeSize);
  CG_AssetPackHeader* header = (CG_AssetPackHeader*)bytes;
  writePos+=writeSize;

  writeSize=_pack->header.numAssets*sizeof(CG_AssetTableEntry);
  memcpy(bytes+writePos, _pack->entries,writeSize);
  writePos+=writeSize;

  writeSize = _pack->header.dataSize;
  memcpy(bytes+writePos, _pack->data, writeSize);

  u8* dataPointer = bytes + writePos;
  CG_Texture* tex = (CG_Texture*) dataPointer;
  writePos+=writeSize;

  platform_write_or_overwrite_file(_targetPath, bytes, _pack->header.totalPackSize);

  //@TEMP_FREE_USED
  free(bytes);
}



void asset_write_assets(const char *_rawAssetsDir, const char *_binFilePath){

  if(CG_Asset_CurrentWorkingArena!=NULL){
    arena_clear(CG_Asset_CurrentWorkingArena);
  }
  else{
    //@TEMP_ALLOC_USED
    CG_Asset_CurrentWorkingArena = arena_create(Gigabytes(4), Megabytes(500), false);
  }

  CG_Asset_CurrentPackWriteRootDir = _rawAssetsDir;
int numAssets = platform_recursively_read_files_in_directory(_rawAssetsDir, NULL);
 CG_AssetPack pack = {0};

 u64 tableTotalSize  = sizeof(CG_AssetTableEntry)*numAssets;
 u64 headerSize = sizeof(CG_AssetPackHeader);
 pack.header.magic = CG_ASSET_BIN_MAGIC;
 pack.header.version = 0;
 pack.header.numAssets = numAssets;
 pack.header.tableOffset = headerSize;
 pack.header.dataOffset = headerSize + tableTotalSize;
 pack.header.dataSize = 0;

 //@TEMP_ALLOC_USED
 pack.entries = malloc(tableTotalSize);
 //@TEMP_ALLOC_USED
 pack.data = malloc(CG_ASSET_PACK_MAX_DATA_SIZE);

 ASSERT_NO_EVAL(pack.entries);
 ASSERT_NO_EVAL(pack.data);


 
 CG_Asset_CurrentPack = pack;
 CG_Asset_CurrentPackWritePosFromDataPointer = 0;


   //@TODO if assets start becoming too large to hold all of them in memory at the same time
  // modify this function to write assets one by one to the file
platform_recursively_read_files_in_directory(_rawAssetsDir, asset_recursive_read_callback);



 
 u64 totalPackSize = CG_Asset_CurrentPack.header.dataSize + headerSize + tableTotalSize;
 CG_Asset_CurrentPack.header.totalPackSize = totalPackSize;
 asset_write_pack_to_bin(_binFilePath,&CG_Asset_CurrentPack);


 //@TEMP_FREE_USED
 free(pack.entries);
 //@TEMP_FREE_USED
 free(pack.data);


 //@TEMP_FREE_USED
 arena_free(CG_Asset_CurrentWorkingArena);
}
CG_RuntimeAssets asset_read_pack(const char *_packPath){
  size_t fileSize = 0;
  size_t headerSize = sizeof(CG_AssetPackHeader);
  void *file = platform_platform_open_file(_packPath, &fileSize);
  void *data = platform_read_part_of_opened_file(file, 0,headerSize, fileSize);

  CG_AssetPackHeader* header = data;
  ASSERT_NO_EVAL(header->magic == CG_ASSET_BIN_MAGIC);
  
  //  printf("BAD u64 FORMATING!! - Magic: %u, Version %u, NumAssets %u, tableOffset %u, dataOffset %u, dataSize %u, totalPackSize: %u\n", header->magic, header->version, header->numAssets, header->tableOffset, header->dataOffset, header->dataSize, header->totalPackSize);

  size_t tableSize = sizeof(CG_AssetTableEntry)*header->numAssets;
  data = platform_read_part_of_opened_file(file,header->tableOffset, tableSize, fileSize);

  CG_AssetTableEntry *assetEntries = data;

  CG_RuntimeAssets ass = {
    .numAssets = header->numAssets,
    .loadedAssets = NULL,
    .packEntries = assetEntries,
    .binHandle = file,
    .binFileSize = fileSize
  };

  //@TEMP_ALLOC_USED
  ass.loadedAssets = arena_create(Gigabytes(4), Megabytes(500), true);
  return ass;
}



CG_Texture *asset_load_texture(CG_RuntimeAssets *_assets, CG_AssetId id){
  CG_RuntimeAsset *asset=  asset_load_from_id(_assets, id, CG_ASSET_TYPE_TEXTURE,false);
  CG_Texture *texture = (CG_Texture*)asset->data;
  u8* pixelsLocation = (u8*)texture + sizeof(CG_Texture);
  texture->Pixels = (u32*)pixelsLocation;
  return texture;
}


CG_Font *asset_load_font(CG_RuntimeAssets *_assets, CG_AssetId id){
  CG_RuntimeAsset *asset = asset_load_from_id(_assets, id, CG_ASSET_TYPE_FONT,false);
  CG_Font *font = (CG_Font*)asset->data;
  font->texture = asset_load_texture(_assets, font->textureAssetId);

  return font;
}


b32 asset_unload(CG_RuntimeAssets *_assets, CG_AssetId id){
  u64 s =  sizeof(CG_RuntimeAsset);
  for(int i=0;i<arena_get_num_items(_assets->loadedAssets, s);i++){
    CG_RuntimeAsset *asset = arena_get_at(_assets->loadedAssets,i,s);
    if(asset->id == id){
      // @TEMP_FREE_USED
      platform_free_file_memory(asset->data, asset->dataSize);

      //@TEMP_FREE_USED
      arena_add_to_free_list(_assets->loadedAssets, asset);
      return true;
    }
  }
  return false;
}
CG_RuntimeAsset *asset_load_from_id(CG_RuntimeAssets *_assets, CG_AssetId _id, enum CG_AssetType _type, b32 _returnRegardlessOfType){
  u64 hash = _id;
  u64 s =  sizeof(CG_RuntimeAsset);
  for(int i=0;i<arena_get_num_items(_assets->loadedAssets, s);i++){
    CG_RuntimeAsset *asset = arena_get_at(_assets->loadedAssets,i,s);
    if(asset->id == hash){
      if(_returnRegardlessOfType || _type == asset->type){
	return asset;
      }
      printf("Asset found but type mismatched\n");
      return NULL;
    }
  }

  
  for(int i=0;i<_assets->numAssets;i++){
    if(_assets->packEntries[i].id == hash){
      if(_returnRegardlessOfType || _type == _assets->packEntries[i].type){
	CG_AssetTableEntry en = _assets->packEntries[i];

	//@TEMP_ALLOC_USED
	CG_RuntimeAsset *asset = ARENA_PUSH_TYPE(_assets->loadedAssets, CG_RuntimeAsset);

	asset->id =en.id;
	asset->type = en.type;
	asset->dataSize = en.dataSize;
	asset->data = platform_read_part_of_opened_file(_assets->binHandle,en.offset,en.dataSize, _assets->binFileSize);

	return asset;
      }
      printf("Asset found but type mismatched\n");
      return NULL;


    }
  }

  return NULL;
}
CG_RuntimeAsset *asset_load(CG_RuntimeAssets *_assets, const char* _stringPath, enum CG_AssetType _type, b32 _returnRegardlessOfType){
  u64 hash = asset_relative_path_to_hash(_stringPath);
  return asset_load_from_id(_assets, hash, _type, _returnRegardlessOfType);
}
