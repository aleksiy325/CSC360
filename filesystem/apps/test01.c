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

    FILE* hd = InitLLFS(path);

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
    
    return 0;
}