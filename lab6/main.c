#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h> 

#include "type.h"
#include "mkdir.c"


MINODE minode[NMINODE];
MINODE *root;

PROC   proc[NPROC], *running;
MNTABLE mntable, *mntPtr;

SUPER *sp;
GD    *gp;
INODE *ip;

int fd, dev;
int nblocks, ninodes, bmap, imap, iblk;
char line[128], cmd[32], pathname[64];

char gpath[128];   // hold tokenized strings
char *name[64];    // token string pointers
int  n;            // number of token strings 
char pwdBuf[1024];
MINODE *iget(int dev, int ino)
{
  printf("iget(%d %d): ", dev, ino);
  return (MINODE *)kcwiget(dev, ino);
}

int iput(MINODE *mip)
{
  printf("iput(%d %d)\n", mip->dev, mip->ino);
  return kcwiput(mip);
}

int getino(int dev, char *pathename)
{
  return kcwgetino(dev, pathname);
}
#include "util.c"
/*
#include "ls_cd_pwd.c"
*/
int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->uid = 0;
    p->cwd = 0;
    for (j=0; j<NFD; j++)
      p->fd[j] = 0;
  }
}

// load root INODE and set root pointer to it
int mount_root()
{  
  printf("mount_root()\n");
  root = iget(dev, 2);
  root->mounted = 1;
  root->mptr = &mntable;

  mntPtr = &mntable;
  mntPtr->dev = dev;
  mntPtr->ninodes = ninodes;
  mntPtr->nblocks = nblocks;
  mntPtr->bmap = bmap;
  mntPtr->imap = imap;
  mntPtr->iblk = iblk;
  mntPtr->mntDirPtr = root;
  strcpy(mntPtr->devName, "mydisk");
  strcpy(mntPtr->mntName, "/");
}

char *disk = "mydisk";

char * searchForName(char buf[BLKSIZE], int ino)
{
  int i = 0;
  char temp[256];
  char * cp;
  for (i=0; i<12; i++){ // assume: DIRs have at most 12 direct blocks
    if (ip->i_block[i]==0)
      break;
   // printf("i_blokc[%d] = %d\n", i, ip->i_block[i]);
    get_block(dev, ip->i_block[i], buf);

    cp = buf;
    dp = (DIR *)buf;

    while(cp < buf+1024){
       strncpy(temp, dp->name, dp->name_len);
       temp[dp->name_len] = 0;
     // printf("%d %d\n",dp->inode, ino );
       if(dp->inode == ino)
       {
        //printf("%s\n", temp);
        strcat(pwdBuf, "/");
        strcat(pwdBuf, temp);

        return;
       }

       cp += dp->rec_len;
       dp = (DIR *)cp;      
    }
  }
}

void change_dir()
{
  int nodeChange = 0;
  nodeChange = kcwgetino(dev, pathname);
  if(nodeChange)
  {
    running->cwd = iget(dev, nodeChange);
  }
  else
  {
    printf("Does not exist\n");
  }
}



void pwd(MINODE * wd)
{
  //printf("%d\n", wd->ino);
  char buf[BLKSIZE];
  if(wd->ino == 2)
  {
    printf("/");
    return;
  }
  if(kcwsearch(wd, "..") == 2)
  {
    ip = &((iget(dev, kcwsearch(wd, "..")))->INODE);
    
    //printf("/\n");
    //strcat(pwdBuf, "/");
    searchForName(buf, wd->ino);
    return;
  }
  

  pwd(iget(dev, kcwsearch(wd, "..")));
  //searchForName(buf, wd->ino);
  //printf("hello\n");
  
  //printf("after rpwd\n");
  //ip = &(wd->INODE);
  ip = &((iget(dev, kcwsearch(wd, "..")))->INODE);
  printf("%d\n", wd->ino);
  searchForName(buf, wd->ino);

}


void list_file()
{
	INODE *tempip;
	char buff[BLKSIZE];
	char temp[256];
	char *cp;
	char *s;
	int potato=0;
	MINODE *tempMI;


	if (!strcmp(pathname, "\0")){
		tempip = &((running->cwd)->INODE);
		printf("size=%d\n", tempip->i_size);

		for (int i=0; i<12; i++){ // assume: DIRs have at most 12 direct blocks

    		if (tempip->i_block[i]==0) break;

    		printf("i_block[%d] = %d\n", i, tempip->i_block[i]);
    		get_block(dev, tempip->i_block[i], buff);

    		cp = buff;
    		dp = (DIR *)buff;

    	while(cp<buff+1024){
    		strncpy(temp, dp->name, dp->name_len);
       		temp[dp->name_len] = 0;
  
       		printf("%s\n", dp->name);

       		cp += dp->rec_len;
       		dp = (DIR *)cp;
    	}
    	}
	}else{
		printf("HELLOW\n");
		potato = kcwgetino(dev, pathname);
		tempMI = iget(dev, potato);
		tempip = &(tempMI->INODE);
		printf("HELLo\n");
		printf("%d\n", tempMI->ino);
		printf("size=%d\n", tempip->i_size);
		//iget(dev, kcwsearch(wd, ".."));

		printf("Pathname given.\n");

		for (int i=0; i<12; i++){ // assume: DIRs have at most 12 direct blocks

    		if (tempip->i_block[i]==0) break;

    		printf("i_block[%d] = %d\n", i, tempip->i_block[i]);
    		get_block(dev, tempip->i_block[i], buff);

    		cp = buff;
    		dp = (DIR *)buff;

    	while(cp<buff+1024){
    		strncpy(temp, dp->name, dp->name_len);
       		temp[dp->name_len] = 0;
  
       		printf("%s\n", dp->name);

       		cp += dp->rec_len;
       		dp = (DIR *)cp;
    	}
    	}
	}

}



main(int argc, char *argv[ ])
{
  int ino;
  char buf[BLKSIZE];
  pwdBuf[0] = 0;
  if (argc > 1)
    disk = argv[1];

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
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;
  
  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = gp->bg_inode_table;
  printf("bmp=%d imap=%d iblk = %d\n", bmap, imap, iblk);

  init();  
  mount_root();
  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);

  //printf("hit a key to continue : "); getchar();
  while(1){
    pwdBuf[0] = 0;
    printf("input command : [ls|cd|pwd|quit] ");
    fgets(line, 128, stdin);

    line[strlen(line)-1] = 0;


    if (line[0]==0)
      continue;
    pathname[0] = 0;

    sscanf(line, "%s %s", cmd, pathname);
    printf("cmd=%s pathname=%s\n", cmd, pathname);

    if (strcmp(cmd, "ls")==0)
       list_file();
    if (strcmp(cmd, "cd")==0)
       change_dir();
    if (strcmp(cmd, "pwd")==0)
       pwd(running->cwd);
       printf("%s\n", pwdBuf);
    if (strcmp(cmd, "print")==0)
       printf("%d\n", running->cwd->ino);

    if (!strcmp(cmd, "mkdir")) my_mkdir(pathname);

    if (strcmp(cmd, "quit")==0)
       quit();
  }
}
 
int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}
