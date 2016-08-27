#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <libgen.h>
#include "File.h"


#pragma pack(1) //disable struct padding


/*
----
0 super
----
1 free block vector
----
2 inode
----
.
.
.
----
8 single indirect
----    
9 double indirect
----
*/

const uint32_t magic = 0xdeadbeef;

//bit vector functions taken from http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
#define SetBit(A,k)     ( A[(k/32)] |= (1 << (k%32)) )
#define ClearBit(A,k)   ( A[(k/32)] &= ~(1 << (k%32)) )            
#define TestBit(A,k)    ( A[(k/32)] & (1 << (k%32)) )

void writeZeros(FILE* fp){
    char* zero = (char*) calloc(1, BLOCK_SIZE);
            
    if(zero){
        int i;
        for(i = 0;i < NUM_BLOCKS; i++){
            if(fwrite(zero, BLOCK_SIZE, 1, fp) != 1){
                fprintf(stderr, "Failed write zeroes to block.\n");
                exit(EXIT_FAILURE);
            }
            fflush(fp);
        }
        free(zero);
    }else{
        fprintf(stderr, "Failed to allocate memory for block\n");
        exit(EXIT_FAILURE);
    }
}

void writeSuper(super_t* head, FILE* fp){
    fseek(fp, 0, SEEK_SET);
    fwrite(head, sizeof(*head), 1, fp);
    fflush(fp);
}

void readSuper(super_t* head, FILE* fp){
    fseek(fp, 0, SEEK_SET);
    fread(head, sizeof(*head), 1, fp);
}

void initSuper(super_t* head){
    head->magic = magic;
    head->numblocks = NUM_BLOCKS;
    head->inodes = 1;
    head->root = ROOT_INODE;
}

void writeBitVector(bitvector_ptr_t vector_ptr, FILE* fp ){
    fseek(fp, BLOCK_SIZE * 1, SEEK_SET); //offset is always 1 block
    fwrite(vector_ptr, sizeof(*vector_ptr), 1, fp);
    fflush(fp);
}

void readBitVector(bitvector_ptr_t vector_ptr, FILE* fp){
    fseek(fp, BLOCK_SIZE * 1, SEEK_SET);
    fread(vector_ptr, sizeof(*vector_ptr), 1, fp);
}

void initBitVector(bitvector_ptr_t vector_ptr){
    int i;
    for(i = 0; i < ITEMS_OF(*vector_ptr) ; i++){
        (*vector_ptr)[i] = 0xFFFFFFFF;
    }
    
    uint32_t reservedblocks = 3 + (MAX_INODES * sizeof(inode_t) /BLOCK_SIZE);
    
    for(i = 0; i < reservedblocks; i++){
        ClearBit(*vector_ptr, i);
    }
}

blockid_t reserveFreeDataBlock(bitvector_ptr_t vector_ptr){
    blockid_t i = 0;
    while(!TestBit(*vector_ptr, i) && i < NUM_BLOCKS){ 
        i++;
    }
    if(i == NUM_BLOCKS ){
        i = -1;
        fprintf(stderr, "Hard drive full no free blocks found.\n");
        exit(EXIT_FAILURE);
        //TODO Rollback changes? 
    }
    ClearBit(*vector_ptr, i);
    return i;
}


void readInodeTable(inodetable_ptr_t table_ptr, FILE* fp ){
    fseek(fp, BLOCK_SIZE * 2, SEEK_SET);
    fread(table_ptr, sizeof(*table_ptr), 1, fp);
}

void writeInodeTable(inodetable_ptr_t table_ptr, FILE* fp ){
    fseek(fp, BLOCK_SIZE * 2, SEEK_SET); //offset is always 2 block
    fwrite(table_ptr, sizeof(*table_ptr), 1, fp);
    fflush(fp);
}

void initRootInode(inodetable_ptr_t table_ptr){
    inode_t new;
    memset(&new, 0, sizeof(new));
    new.filesize = 0;
    new.flags = DIR_T;
    (*table_ptr)[ROOT_INODE] = new;
}


void readBlock(blockid_t bid, FILE* fp, block_ptr_t block_ptr){
    printf("Reading block %d , %x \n", bid, bid * BLOCK_SIZE);
    fseek(fp, BLOCK_SIZE *  bid, SEEK_SET);
    fread(block_ptr, sizeof(*block_ptr), 1, fp);
}

void writeBlock(blockid_t bid, FILE* fp, block_ptr_t block_ptr){
    printf("Block writing at: %d , %x \n", bid, bid * BLOCK_SIZE);
    fseek(fp, BLOCK_SIZE * bid, SEEK_SET);
    fwrite(block_ptr, sizeof(*block_ptr), 1, fp);
    fflush(fp);
}

void readDirBlock(blockid_t bid, FILE* fp, dblock_ptr_t dblock_ptr){
    fseek(fp, BLOCK_SIZE *  bid, SEEK_SET);
    fread(dblock_ptr, sizeof(*dblock_ptr), 1, fp);
}

void writeDirBlock(blockid_t bid, FILE* fp, dblock_ptr_t dblock_ptr){
    fseek(fp, BLOCK_SIZE * bid, SEEK_SET);
    fwrite(dblock_ptr, sizeof(*dblock_ptr), 1, fp);
    fflush(fp);
}

inodeid_t insertNewInode(inode_t entry, inodetable_ptr_t table_ptr){
    inodeid_t i = 1; //0 reserved
    while(i < MAX_INODES && (*table_ptr)[i].flags != 0){
        i++;
    }
    if(i < MAX_INODES){ // space found
        (*table_ptr)[i] = entry;

    }else{
        fprintf(stderr, "Inode table is full.\n");
        exit(EXIT_FAILURE);
    }
    return i;
}


void writeDirEntry(inode_t* dinode, bitvector_ptr_t vector_ptr, directory_t entry, FILE* fp ){
    int i = 0;
    while(i < INODE_DATA_BLOCKS && dinode->blocks[i] != 0){
        blockid_t bid = dinode->blocks[i];
    
        int j = 0;
        
        dblock_t dblock;
        readDirBlock(bid, fp, &dblock);
        
        while(j < DIR_BLOCK_SIZE && dblock[j].inode != 0){
            j++;
        }

        if(j < DIR_BLOCK_SIZE){ // found free block
            printf("found free block %d %d, block number: %d\n", i ,j, dinode->blocks[i]);
            dblock[j] = entry;
            writeDirBlock(bid, fp, &dblock);
            return;
        }
        i++;
    }
    if(i < INODE_DATA_BLOCKS){ //allocate new block
        dblock_t dblock;
        blockid_t bid = reserveFreeDataBlock(vector_ptr);
        dinode->blocks[i] = bid;

        printf("Allocating new block %d \n", bid);

        memset(&dblock, 0, sizeof(dblock));
        dblock[0] = entry;

        writeDirBlock(bid, fp, &dblock);

    }else{
        fprintf(stderr, "Dir blocks is full cannot make more.\n");
        exit(EXIT_FAILURE);
    }
}


FILE* InitLLFS(char* diskpath){
    printf("Initializing hard drive: %s\n", diskpath);
    FILE* fp = fopen(diskpath, "wb+");
    if(fp){
            writeZeros(fp);
            
            super_t head;
            initSuper(&head);
              
            bitvector_t vector;
            initBitVector(&vector);
            
            inodetable_t table;
            readInodeTable(&table, fp); // is all zero could also just memset
            initRootInode(&table);

            writeInodeTable(&table, fp);
            writeBitVector(&vector, fp);
            writeSuper(&head, fp);  
            
    }else{
        fprintf(stderr, "Failed to open hard drive: %s\n", diskpath);
        exit(EXIT_FAILURE);
    }
    fclose(fp);
    fp = fopen(diskpath, "rb+"); //open for rw

    return fp;
}


inodeid_t searchInodeFile(char* filename , inodeid_t inodeid,  inodetable_ptr_t table_ptr, FILE* fp ){
    inode_t inode = (*table_ptr)[inodeid];
    int i = 0;
    for(i; i< INODE_DATA_BLOCKS; i++){
        if(inode.blocks[i] != 0){
            dblock_t dblock;

            readDirBlock(inode.blocks[i], fp, &dblock);
            
            int j = 0;
            
            for(j; j < ITEMS_OF(dblock) ; j++){
                
                if(strcmp(dblock[j].filename, filename) == 0){
                    return dblock[j].inode;
                }
            }
        }
    }
    fprintf(stderr, "Dir not found:  %s\n", filename);
    return 0;
}


inodeid_t insertFile( char* name, uint32_t ftype,  inodeid_t pinode,  super_t* super_ptr,  bitvector_ptr_t vector_ptr, inodetable_ptr_t table_ptr, FILE* fp){
    //inode entry
    inode_t new;
    inodeid_t id;
    memset(&new, 0, sizeof(new));
    new.filesize = 0;
    new.flags = ftype;
    id = insertNewInode(new, table_ptr);
    super_ptr->inodes++;
    inode_t* par_ptr = &((*table_ptr)[pinode]); //TODO simplify?

    //dir entry
    directory_t dir;
    dir.inode = id;
    strcpy(dir.filename, name);

    writeDirEntry(par_ptr, vector_ptr, dir, fp);

    return id;
}


 inodeid_t recursiveDir(char* ptoken, char* fname,  inodeid_t pinode, super_t* super_ptr,  bitvector_ptr_t vector_ptr, inodetable_ptr_t table_ptr, FILE* fp){
    inodeid_t id = 0;
 
    if(ptoken != NULL){  
         id = searchInodeFile(ptoken , pinode , table_ptr,  fp );

        if(!id){ // Not found 0
            printf("Creating new dir %s\n", ptoken);
            id = insertFile( ptoken , DIR_T, pinode, super_ptr, vector_ptr, table_ptr, fp);
     
        }else{
            printf("%s already exists. Moving on.\n", ptoken);
        }

        ptoken = strtok(NULL, "/");
        id = recursiveDir(ptoken, fname, id, super_ptr, vector_ptr, table_ptr, fp);
    }else{

        id = searchInodeFile(fname , pinode , table_ptr,  fp );
        if(!id){
            printf("Creating new File %s\n", fname);
            id = insertFile( fname , FILE_T, pinode, super_ptr, vector_ptr, table_ptr, fp);
        
        }else{
            printf("File already exists. Opening it. %s\n", fname);
        }
  
        
    }
    return id;
}



inodeid_t openFile(char* filepath, FILE* fp){
    printf("Opening file %s on hard drive: %s\n", filepath );

    char buff[PATH_LEN];
    strcpy(buff, filepath);


    char* bname = basename(buff);
    char* dname = dirname(buff);

    char dirbuff[PATH_LEN];
    strcpy(dirbuff, dname);

    inodeid_t fileid;

    if(fp){
        inodetable_t table;
        super_t super;
        bitvector_t vector;
        
        //read
        readInodeTable(&table, fp);
        readSuper(&super, fp);
        readBitVector(&vector, fp);
        
        //tokenize
        char* ptoken = strtok(dirbuff, "/");
        fileid = recursiveDir(ptoken, bname,  super.root,  &super, &vector, &table, fp);
       
        //write
        writeInodeTable(&table, fp);
        writeBitVector(&vector,fp);
        writeSuper(&super, fp);
        
     }else{
        fprintf(stderr, "Failed to open hard drive: %s\n", filepath);
        exit(EXIT_FAILURE);
    }

    return fileid;
}


void writeFile(char* ptr, size_t len, inodeid_t fid, FILE* fp){
    inodetable_t table;
    super_t super;
    bitvector_t vector;

    //read
    readInodeTable(&table, fp);
    readSuper(&super, fp);
    readBitVector(&vector, fp);

    inode_t* finode = &table[fid];

    

    //TODO clean file or append?

    int i = 0;
    int j = len;
    int c = BLOCK_SIZE;
    block_t block;
    memset(&block, 0 , sizeof(block));

    while( j > 0 && i < INODE_DATA_BLOCKS){ 
       if(j % BLOCK_SIZE != 0){
            c = j % BLOCK_SIZE;
        }
        memcpy(&block, ptr, c);
        finode->blocks[i] = reserveFreeDataBlock(&vector);
        writeBlock(finode->blocks[i], fp,  &block);
        ptr += BLOCK_SIZE;
        j-= BLOCK_SIZE; 
        i++;        
    }

    printf("Starting single inderection write\n");
    
    //single indirection

    finode->filesize = j;

     //write
    writeInodeTable(&table, fp);
    writeBitVector(&vector,fp);
    writeSuper(&super, fp);
}



void readFile(char* ptr, size_t len, inodeid_t fid, FILE* fp){
    inodetable_t table;
    super_t super;
    bitvector_t vector;

    //read
    readInodeTable(&table, fp);
    readSuper(&super, fp);
    readBitVector(&vector, fp);

    inode_t* finode = &table[fid];

    if(finode->blocks[0] == 0){
        fprintf(stderr, "File with this inode does not exist.\n");
        exit(EXIT_FAILURE);
    }

    //TODO clean file or append?

    //TODO single double indirection

    int i = 0;
    int j = len;
    int c = BLOCK_SIZE;
    block_t block;
    
    while( j > 0 && i < INODE_DATA_BLOCKS){ 
        readBlock(finode->blocks[i],  fp,  &block); 
        if(j % BLOCK_SIZE != 0){
            c = j % BLOCK_SIZE;
        }
        memcpy(ptr, &block, c);
        j-= BLOCK_SIZE; 
        ptr += BLOCK_SIZE;
        i++;  
    }

    //write
    writeInodeTable(&table, fp);
    writeBitVector(&vector,fp);
    writeSuper(&super, fp);
}


void fsck(FILE* fp){
    inodetable_t table;
    super_t super;
    bitvector_t vector;

    //read
    readInodeTable(&table, fp);
    readSuper(&super, fp);
    readBitVector(&vector, fp);

    super_t n_super;
    initSuper(&n_super);
        
    bitvector_t n_vector;
    initBitVector(&n_vector);

    inodetable_t n_table;
    memset(&n_table, 0 , sizeof(n_table));
    initRootInode(&n_table);


    walkDirs(ROOT_INODE,  &n_super,  &n_vector,  &table,  &n_table, fp);

    writeInodeTable(&n_table, fp);
    writeBitVector(&n_vector, fp);
    writeSuper(&n_super, fp);  
}

void checkInodeBlocks(inode_t inode, bitvector_ptr_t n_vector_ptr){
    int k = 0;
    for(k; k < INODE_DATA_BLOCKS; k++){
        ClearBit(*n_vector_ptr, inode.blocks[k]); // reserve used blocks
    }
}

void checkDirBlock(inodeid_t dbid, super_t* n_super_ptr,  bitvector_ptr_t n_vector_ptr, inodetable_ptr_t table_ptr,  inodetable_ptr_t n_table_ptr, FILE* fp ){
    dblock_t dblock;
    readDirBlock(dbid, fp, &dblock);

    int j = 0;
    for(j; j < DIR_BLOCK_SIZE; j++){ //check inodes of dirs
        if(dblock[j].filename[0] != '\0'){ // filename is not empty
            inodeid_t cid = dblock[j].inode;
            inode_t cinode = (*table_ptr)[cid];
            (*n_table_ptr)[cid] = cinode;

            if(cinode.flags){ // inode exists
                n_super_ptr->inodes++;
                checkInodeBlocks(cinode, n_vector_ptr);
            
                if(cinode.flags == DIR_T){ //recursive if dir
                    walkDirs(cid, n_super_ptr, n_vector_ptr, table_ptr, n_table_ptr, fp ); 
                }
        
            }else{
                memset(&(dblock[j]), 0 , sizeof(dblock[j])); // delete dir
            }
        }
    }
    writeDirBlock(dbid, fp, &dblock); 
}

void walkDirs(inodeid_t root, super_t* n_super_ptr, bitvector_ptr_t n_vector_ptr, inodetable_ptr_t table_ptr,  inodetable_ptr_t n_table_ptr, FILE* fp ){
    printf("Walking dirs of inode id %d\n", root);
    inode_t inode = (*table_ptr)[root];
    int i = 0;
    for(i; i < INODE_DATA_BLOCKS; i++){ // read dir blocks
       
        blockid_t dbid = inode.blocks[i];
        
        if(dbid){
            printf("Dir block %d is being checked\n", dbid);
            checkDirBlock(dbid, n_super_ptr, n_vector_ptr, table_ptr, n_table_ptr, fp);
        }
    }
  
}


void CrashTestFile(FILE* fp){
    super_t super;
    bitvector_t vector;

    initSuper(&super);
    initBitVector(&vector);

    writeBitVector(&vector,fp);
    writeSuper(&super, fp);
}
