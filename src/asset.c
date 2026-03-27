#include "asset.h"
#include "platform.h"
#include "xxhash.h"
#include "cgame.h"
#define CG_XXHASH_SEED 0

CG_AssetPack CG_Asset_CurrentPack;
u32 CG_Asset_CurrentPackWritePos;
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

  if(extension == ".png"){

    CG_Texture* tex = texture_load_from_file(_relativePath, CG_Asset_CurrentWorkingArena);
    u32 pixelDataSize = texture_get_pixel_data_size_in_bytes(tex);
    u32 totalWriteSize = 0;


    CG_TextureAssetBin texAsset;
    texAsset.Width = tex->Width;
    texAsset.Height = tex->Height;
    texAsset.BytesPerPixel = tex->BytesPerPixel;
    texAsset.PixelDataOffset = CG_Asset_CurrentWritePos+sizeof(CG_TextureAsset);
    u32 texAssetSize = sizeof(texAsset);
    u8* dest = (u8*)&CG_Asset_CurrentPack.data[0];
    dest+=CG_CurrentPackWritePos;
    memcpy(dest, &texAsset, texAssetSize);
    dest+=texAssetSize;
    memcpy(dest, tex->Pixels, pixelDataSize);
    
    CG_AssetTableEntry entry;
    entry.id = hash;
    entry.type = CG_ASSET_TYPE_TEXTURE;
    entry.offset = CG_Asset_CurrentPackWritePos;


    CG_Asset_CurrentPack.table.entries[CG_Asset_CurrentPack.header.numAssets] = entry;
    CG_Asset_CurrentPack.header.numAssets++;
    CG_Asset_CurrentWritePos += texAssetSize + size;

    totalWriteSize = texAssetSize + pixelDataSize;
    CG_Asset_CurrentPack.header.dataSize+=totalWriteSize;
    
    arena_pop(CG_Asset_CurrentWorkingArena, texture_get_total_size_in_bytes(tex));
  }
  else if(extension == ".glb"){

  }
  else if(extension == ".wav"){

  }
  else if(extension == ".ttf"){

  }

  printf("Found asset: %s\n", _relativePath); 
  printf("Hash: %llu\n", hash);
}

void asset_write_pack_to_bin(const char* _targetPath, CG_AssetPack *_pack, u32 _packSize){
  u8* bytes = malloc(_packSize);
  u32 writePos = 0;
}



CG_AssetPack asset_write_assets(const char *_rawAssetsDir, const char *_binFilePath){

  if(CG_Asset_CurrentWorkingArena!=NULL){
    CG_Asset_CurrentWorkingArena = arena_create(Gigabytes(4), Megabytes(500), false);
  }
int numAssets = platform_recursively_read_files_in_directory(_rawAssetsDir, NULL);
 CG_AssetPack pack = {0};

 u64 tableTotalSize  = sizeof(CG_AssetTable) + sizeof(CG_AssetTableEntry)*numAssets;
 u64 headerSize = sizeof(CG_AssetHeader);
 pack.header.magic = CG_ASSET_BIN_MAGIC;
 pack.header.version = 0;
 pack.header.numAssets = numAssets;
 pack.header.tableOffset = headerSize;
 pack.header.dataOffset = sizeof(pack);
 pack.header.dataSize = 0;
 pack.table = malloc(tableTotalSize);
 pack.data = malloc(CG_ASSET_PACK_MAX_DATA_SIZE);


 
 CG_Asset_CurrentPack = pack;
 CG_Asset_CurrentPackWritePos = pack.header.dataOffset;


   //@TODO if assets start becoming too large to hold all of them in memory at the same time
  // modify this function to write assets one by one to the file
int numAssets = platform_recursively_read_files_in_directory(_rawAssetsDir, asset_recursive_read_callback);
 

 printf("Total assets found: %d\n", numAssets);
  return CG_Asset_CurrentPack;
}


/* int main(int argc, char** argv){ */

/*   printf("Starting asset writer\n"); */
/*   //  printf("Num args: %d\n", argc); */
/*   if(argc !=3) { */
/*     printf("Please pass path where binary file should be written to using -b <path>\n"); */
/*     return 1; */
/*   }; */

/*   /\* printf("Args:\n"); *\/ */
/*   /\* for(int i=0;i<argc;i++){ *\/ */
/*   /\*   printf("%d: %s\n",i, argv[i]); *\/ */
/*   /\* } *\/ */
  
/*   if(strcmp(argv[1],"-b")){ */
/*     printf("Incorrect arguments, following args are required: -b <path>\n"); */
/*     return 1; */
/*   } */

/*   char* path = argv[2]; */

/*   printf("Finding assets...\n"); */
/*   u32 numAssets = count_assets(AssetsDir); */
/*   printf("Number of assets found: %d\n", numAssets); */

/*   printf("Writing assets bin to %s..\n", path); */
/*   return 0; */
/* } */
