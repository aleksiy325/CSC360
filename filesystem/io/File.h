#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>


#define BLOCK_SIZE 512
#define NUM_BLOCKS 4096
#define MAX_INODES 255 // 1 byte ids 0 reserved for empty
#define DIR_T 8
#define FILE_T 16
#define ROOT_INODE 0
#define INODE_DATA_BLOCKS 10
#define FNAME_LEN 31
#define PATH_LEN 255
#define BITVECTOR_LEN (NUM_BLOCKS / (sizeof(uint32_t) * 8))
#define ITEMS_OF(a) (sizeof(a) / sizeof(a[0]))



#pragma pack(1) //disable struct padding


typedef uint16_t blockid_t;
typedef uint8_t inodeid_t;
typedef uint16_t dirid_t;

typedef blockid_t blocktable_t[BLOCK_SIZE / sizeof(blockid_t)];

typedef struct super{
   uint32_t magic; 
   uint32_t numblocks;
   inodeid_t inodes;
   inodeid_t root;
}super_t;

typedef struct inode{ //each inode is 32 bytes long
    uint32_t filesize;
    uint32_t flags;
    blockid_t blocks[INODE_DATA_BLOCKS];
    blockid_t sing;
    blockid_t doub;
} inode_t;

typedef struct directory{ 
    inodeid_t inode;
    char filename[FNAME_LEN];
} directory_t;

typedef char block_t[BLOCK_SIZE];
typedef block_t* block_ptr_t;

typedef inode_t inodetable_t[MAX_INODES];
typedef inodetable_t* inodetable_ptr_t;

typedef uint32_t bitvector_t[BITVECTOR_LEN];
typedef bitvector_t* bitvector_ptr_t;

#define DIR_BLOCK_SIZE (BLOCK_SIZE / sizeof(directory_t))

typedef directory_t dblock_t[DIR_BLOCK_SIZE];
typedef dblock_t* dblock_ptr_t;

FILE* InitLLFS(char* diskpath);

inodeid_t createFile(char* filepath, FILE* fp);
void fsck(FILE* fp);
void readFile(char* ptr, size_t len, inodeid_t fid, FILE* fp);

void writeFile(char* ptr, size_t len, inodeid_t fid, FILE* fp);
inodeid_t openFile(char* filepath, FILE* fp);