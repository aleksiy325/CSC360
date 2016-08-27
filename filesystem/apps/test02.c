#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "File.h"


int main(int argc, char *argv[]){

    char* path = "../disk/vdisk";
    if(argc == 2){
        path = argv[1];
    }

      FILE*  hd = InitLLFS(path);



    openFile("/test/file1.txt", hd);
    openFile("/test/test3/file2.txt", hd);
    openFile("/test/test4/test5/file3.txt", hd);
    openFile("/test/test3/test6/file3.txt", hd);
    
     //read superblock
    super_t head;
    fseek(hd, 0, SEEK_SET);
    fread(&head, sizeof(head), 1, hd);
    
    //read bitvector
    bitvector_t vector;
    fseek(hd, 1 * BLOCK_SIZE, SEEK_SET);
    fread(&vector, sizeof(vector), 1, hd);
    
    
    printf("magic: %x\n", head.magic);
    printf("numblocks: %d\n", head.numblocks);
    printf("inodes: %d\n", head.inodes);
    
    fclose(hd);
    return 0;
}