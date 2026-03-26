#include "asset.h"
#include "platform.h"

void asset_recursive_read_callback(int index, const char *_relativePath, const char *_absolutePath){
  /* printf("Found asset: %s\n", _relativePath); */
  /* printf("Absolute path: %s\n", _absolutePath); */
}





u32 asset_write_assets(const char *_rawAssetsDir, const char *_binFilePath){
int numAssets = platform_recursively_read_files_in_directory(_rawAssetsDir, asset_recursive_read_callback);

 printf("Total assets found: %d\n", numAssets);
  return numAssets;
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
