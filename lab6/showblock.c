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
char tempString[250];
char *dirNames[20];
int numberOfItteration=0;



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

  int blk = iblk + (ino-1) / 8;
  get_block(dev, blk, ibuf);
  ip = (INODE *)ibuf + (ino-1) % 8;

  inode = *ip;

  return ip;
}

makeNames(){
	char *s;
	int i=0;
	s=strtok(tempString, "/");
	strcpy(dirNames[i],s);
	i++;
	numberOfItteration++;

	while(s=strtok(0,"/")){
		strcpy(dirNames[i],s);
		i++;
		numberOfItteration++;
	}
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
  int iNodePotato=0;

  for(int i=0;i<20;i++) dirNames[i]=malloc(50);
  


  printf("checking EXT2 FS ....");
  if ((fd = open(argv[1], O_RDWR)) < 0){
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

  //print super block stuff
  printf("s_inodes_count: %d\n", sp->s_inodes_count);

  strcpy(tempString,argv[2]);
  makeNames();


  
  // read GD block to get GD0
  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  // bmap = gp->bg_block_bitmap;
  // imap = gp->bg_inode_bitmap;
   iblk = gp->bg_inode_table;
  // printf("bmp=%d imap=%d iblk = %d\n", bmap, imap, iblk);
  printf("Inode_table: %d\n", iblk);

  iNodePotato = 2;
for(int z=0;z<numberOfItteration;z++){
  printf("%d\n",iNodePotato);
  ip = getInode(dev, iNodePotato);

  printf("%x\n", ip->i_mode);
  printf("imode = %4x\n", ip->i_mode);
  if ((ip->i_mode & 0xF000) == 0x8000){
    printf("This is a file.\n");
    exit(1);
  }
  if ((ip->i_mode & 0xF000) != 0x4000){
    printf("not a DIR\n");
    exit(1);
  }


  //stuff that we need in agrv[2]
  


  
  //ip = getInode(dev, 2);

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

       if (!strcmp(dp->name, dirNames[z])){

       	//printf("%d\n", dp->file_type);

       	iNodePotato=dp->inode;
       	break;

       }

       cp += dp->rec_len;
       dp = (DIR *)cp;
    }
  }
}
}