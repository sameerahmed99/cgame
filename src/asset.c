#include "asset.h"
#include "platform.h"

#define CG_ASSET_PACK_MAX_DATA_SIZE Gigabytes(4)


CG_AssetPack CurrentPack;
Arena *CurrentWorkingArena;
void asset_recursive_read_callback(int index, const char *_relativePath, const char *_absolutePath){

  char extension[16];
  u32 len = strlen(_relativePath);
  for(int i=len-1;i>=0;i--){
    if(_relativePath[i] == '.'){
      strcpy(extension, _relativePath + i);
      break;
    }
  }

  if(extension == ".png"){

    CG_Texture* tex = texture_load_from_file(_relativePath, CurrentWorkingArena);
    u32 size = texture_get_size_in_bytes(tex);

    // asset id hash from relativePath
    // create table entry
    
    arena_pop(CurrentWorkingArena, size);
  }
  else if(extension == ".glb"){

  }
  else if(extension == ".wav"){

  }
  else if(extension == ".ttf"){

  }

   //   printf("Found asset: %s\n", _relativePath); 
  /* printf("Absolute path: %s\n", _absolutePath); */
}





CG_AssetPack asset_write_assets(const char *_rawAssetsDir, const char *_binFilePath){

  if(CurrentWorkingArena!=NULL){
    CurrentWorkingArena = arena_create(Gigabytes(4), Megabytes(500), false);
  }
  //@TODO if assets start becoming too large to hold all of them in memory at the same time
  // modify this function to write assets one by one to the file
int numAssets = platform_recursively_read_files_in_directory(_rawAssetsDir, asset_recursive_read_callback);
 CG_AssetPack pack = {0};

 pack.header.magic = CG_ASSET_BIN_MAGIC;
 pack.header.version = 0;
 pack.header.numAssets = numAssets;

 pack.data = malloc(CG_ASSET_PACK_MAX_DATA_SIZE);


 
 CurrentPack = pack;


 printf("Total assets found: %d\n", numAssets);
  return CurrentPack;
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
