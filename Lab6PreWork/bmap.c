#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "ext2fs.h"

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp;

int assicTimeser = 30; 

#define BLKSIZE 1024

/******************* in <ext2fs/ext2_fs.h>*******************************
struct ext2_super_block {
  u32  s_inodes_count;       // total number of inodes
  u32  s_blocks_count;       // total number of blocks
  u32  s_r_blocks_count;     
  u32  s_free_blocks_count;  // current number of free blocks
  u32  s_free_inodes_count;  // current number of free inodes 
  u32  s_first_data_block;   // first data block in this group
  u32  s_log_block_size;     // 0 for 1KB block size
  u32  s_log_frag_size;
  u32  s_blocks_per_group;   // 8192 blocks per group 
  u32  s_frags_per_group;
  u32  s_inodes_per_group;    
  u32  s_mtime;
  u32  s_wtime;
  u16  s_mnt_count;          // number of times mounted 
  u16  s_max_mnt_count;      // mount limit
  u16  s_magic;              // 0xEF53
  // A FEW MORE non-essential fields
};
**********************************************************************/

char buf[BLKSIZE];
int fd;

int get_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  read(fd, buf, BLKSIZE);
}

makeLine(){
for(int i=0;i<assicTimeser;i++) printf("=");
printf("\n");
}

int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit / 8;  j = bit % 8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

super()
{
  // read Gd block
  //comment for save passowrd
  get_block(fd, 8, buf);  
  //gp = (GD *)buf;
  //now we need  to just use buffer
  makeLine();

  for(int i=1;i<(1439);i++){
    (tst_bit(buf, i)) ? putchar('1') : putchar('0');
    if (i && (i % 8)==0)
       printf(" ");
  }
  printf("\n");
  
  makeLine();


  // check for EXT2 magic number:

  // printf("s_magic = %x\n", sp->s_magic);
  // if (sp->s_magic != 0xEF53){
  //   printf("NOT an EXT2 FS\n");
  //   exit(1);
  // }

  // printf("EXT2 FS OK\n");

  // printf("s_inodes_count = %d\n", sp->s_inodes_count);
  // printf("s_blocks_count = %d\n", sp->s_blocks_count);

  // printf("s_free_inodes_count = %d\n", sp->s_free_inodes_count);
  // printf("s_free_blocks_count = %d\n", sp->s_free_blocks_count);
  // printf("s_first_data_blcok = %d\n", sp->s_first_data_block);
  // printf("s_log_block_size = %d\n", sp->s_log_block_size);
  // printf("s_blocks_per_group = %d\n", sp->s_blocks_per_group);
  // printf("s_inodes_per_group = %d\n", sp->s_inodes_per_group);
  // printf("s_mnt_count = %d\n", sp->s_mnt_count);
  // printf("s_max_mnt_count = %d\n", sp->s_max_mnt_count);
  // printf("s_magic = %x\n", sp->s_magic);
  // printf("s_mtime = %s", ctime(&sp->s_mtime));
  // printf("s_wtime = %s", ctime(&sp->s_wtime));
}

char *disk = "mydisk";

main(int argc, char *argv[ ])

{ 
  if (argc > 1)
    disk = argv[1];
  fd = open(disk, O_RDONLY);
  if (fd < 0){
    printf("open failed\n");
    exit(1);
  }

  super();
}
