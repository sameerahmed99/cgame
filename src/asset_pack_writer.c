

#include <stdio.h>
#include <string.h>
#include "asset_pack.h"
#include "asset_pack.c"





int main(int argc, char** argv){

  printf("Starting asset writer\n");
  printf("Num args: %d\n", argc);
  if(argc !=3) {
    printf("Please pass path where binary file should be written to using -b <path>\n");
    return 1;
  };

  printf("Args:\n");
  for(int i=0;i<argc;i++){
    printf("%d: %s\n",i, argv[i]);
  }
  
  if(strcmp(argv[1],"-b")){
    printf("Incorrect arguments, following args are required: -b <path>\n");
    return 1;
  }

  char* path = argv[2];
  printf("Writing assets bin to %s..\n", path);
  return 0;

}
