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


    inodeid_t file = openFile("/test/file1.txt", hd);
    file = openFile("/test/file1.txt", hd);


    FILE* f = fopen("./testfile", "rb+");
    

    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  

    char* buff = malloc(fsize + 1);
    char* testbuff = malloc(fsize + 1);

    fread(buff, fsize, 1, f);
    fclose(f);

  

    writeFile(buff, fsize, file, hd);

    readFile(testbuff, fsize , file, hd);

    FILE* out = fopen("./outtest", "wb+");
    fwrite(testbuff, fsize, 1 , out);

    return 0;
}