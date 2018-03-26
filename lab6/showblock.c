#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include <ext2fs/ext2_fs.h> 

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

#define BLKSIZE   1024
#define ISIZE      128

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}

int fd, dev;
int bmap, imap, iblk;

INODE inode;

INODE *getInode(int dev, int ino)
{
  INODE *ip;
  char ibuf[1024];

  int blk = iblk + (ino-1) / ISIZE;
  get_block(dev, blk, ibuf);
  ip = (INODE *)ibuf + (ino-1) % ISIZE;

  inode = *ip;

  return ip;
}

char *disk = "mydisk";

int main(int argc, char *argv[ ])
{
  SUPER *sp;
  GD    *gp;
  INODE *ip;
  DIR   *dp;
  char  *cp;
  char buf[BLKSIZE], temp[256];
  int i;
  
  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);  exit(1);
  }
  dev = fd;

  /********** read super block at 1024 ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system *****************/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("OK\n");
  
  // read GD block to get GD0
  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = gp->bg_inode_table;
  printf("bmp=%d imap=%d iblk = %d\n", bmap, imap, iblk);

  ip = getInode(dev, 2);
  printf("imode = %4x\n", ip->i_mode);
  if ((ip->i_mode & 0xF000) != 0x4000){
    printf("not a DIR\n");
    exit(1);
  }
  for (i=0; i<12; i++){ // assume: DIRs have at most 12 direct blocks
    if (ip->i_block[i]==0)
      break;
    printf("i_blokc[%d] = %d\n", i, ip->i_block[i]);
    get_block(dev, ip->i_block[i], buf);

    cp = buf;
    dp = (DIR *)buf;

    while(cp < buf+1024){
       strncpy(temp, dp->name, dp->name_len);
       temp[dp->name_len] = 0;
       
       printf("%4d %4d %4d   %s\n", dp->inode, dp->rec_len, dp->name_len, temp);

       cp += dp->rec_len;
       dp = (DIR *)cp;
    }
  }
}