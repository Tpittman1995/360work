#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h> 

#include "type.h"


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


int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

int set_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] &= ~(1 << j);
}

int decFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
}

int ialloc(int dev)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeInodes(dev);

       put_block(dev, imap, buf);

       return i+1;
    }
  }
  printf("ialloc(): no more free inodes\n");
  return 0;
}

int balloc(int dev)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, bmap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeInodes(dev);

       put_block(dev, bmap, buf);

       return i+1;
    }
  }
  printf("ialloc(): no more free inodes\n");
  return 0;
}


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
#include "rmdir.c"
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
    MINODE * temp = iget(dev, nodeChange);
    temp->refCount--;
    if ((temp->INODE.i_mode & 0xF000) != 0x4000){
      printf("not a DIR\n");
      return;
    }
    printf("changing directory\n");
    running->cwd = temp;
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

int mymkdir(MINODE *pip, char *name){
  int ino=0, bno=0;
  MINODE *tempMI;
  DIR   *dp;
  char  *cp;
  char buf[1024];

  ino=ialloc(dev);
  bno=balloc(dev);

  tempMI=iget(dev, ino);
  INODE *ip=&(tempMI->INODE);

  ip->i_mode = 0x41ED;
  ip->i_uid  = running->uid;  // Owner uid 
  ip->i_gid  = running->gid;  // Group Id
  ip->i_size = BLKSIZE;   // Size in bytes 
  ip->i_links_count = 2;          // Links count=2 because of . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
  ip->i_blocks = 2;                 // LINUX: Blocks count in 512-byte chunks 
  ip->i_block[0] = bno;             // new DIR has one data block   
  for(int i=1; i<15;i++) ip->i_block[i]=0;
  //ip->i_block1] to i_block[14] = 0;

  tempMI->dirty=1;

  iput(tempMI);

  get_block(dev, ip->i_block[0], buf);

  cp=buf;
  dp = (DIR *)buf;


  dp->inode=ino;
  dp->rec_len=12;
  dp->name_len=1;
  strcpy(dp->name, ".");

  cp+=dp->rec_len;
  dp=(DIR *)cp;

  dp->inode=pip->ino;
  dp->rec_len=1012;
  dp->name_len=2;
  strcpy(dp->name, "..");

  put_block(dev, ip->i_block[0], buf);

  printf("Allocated Inode and Iblock\nino=%d\nbno=%d\n", ino,bno);

  enter_name(pip, ino, name);
}

int enter_name(MINODE *pip, int myino, char *myname){
  //localvars
  DIR   *dp;
  char  *cp;
  char buf[1024];
  INODE *ip=&(pip->INODE);
  int IDEAL_LEN=0, restMem=0, needLen=0;
  char temp[256];

  //start loop
  for(int i=0;i<12;i++){
    //check if bot is empty or not
    if (ip->i_block[i]==0) break;

    get_block(dev, ip->i_block[i], buf);

    cp=buf;
    dp=(DIR *)cp;

    while(cp < buf+1024){
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      
      IDEAL_LEN = 4*((8 + strlen(temp) + 3)/4);

      printf("IDEAL_LEN=%d rec_len=%d\n", IDEAL_LEN, dp->rec_len);

      if(IDEAL_LEN!=dp->rec_len){
        printf("Found last entry in data block.\n");
        needLen= 4*( (8 + strlen(myname) + 3)/4 );
        if(needLen>dp->rec_len){
          printf("[ERROR] Not enough space to add Another dir.\n");
          return;
        }
        //record the rest of len and set the pervious block to idea
        restMem=dp->rec_len;
        dp->rec_len=IDEAL_LEN;

        //to after the last block and put it dp find out len needed
        cp+=dp->rec_len;
        dp=(DIR *)cp;

        dp->inode=myino;
        dp->rec_len=restMem-IDEAL_LEN;
        dp->name_len=strlen(myname);
        strcpy(dp->name, myname);

        put_block(dev, ip->i_block[i], buf);

        break;


      }


      cp += dp->rec_len;
      dp = (DIR *)cp;
    }

  }
}


int my_mkdir(char* path){
  //local vars
  MINODE *parentInode;
  INODE *tempip;
  int potato=0;

  int parentInodeNumber=0;
  char child[100];
  int i=0, sizeOfparent=0;
  int flag=0;

  printf("mkdir()\n");

  //tokenize path just in case
  tokenize(path);

  //hand ablosute or relative path
  if (path[0]=='/') dev = root->dev;
  else {
    flag=1;
    dev = running->cwd->dev;
    sizeOfparent= -1;
  }

  for(i=0;i<(n-1);i++) sizeOfparent+=(strlen(name[i])+1);
  path[sizeOfparent]=0;
  strcpy(child, name[i++]);

  if (flag){
    parentInodeNumber=kcwsearch(running->cwd, ".");
    parentInode = iget(dev, parentInodeNumber);
  }else{
  parentInodeNumber=getino(dev, path);
  parentInode = iget(dev, parentInodeNumber);
}

  tempip = &(parentInode->INODE);

  //check for dir
  if ((tempip->i_mode & 0xF000) != 0x4000){
    printf("[ERROR] Parent Dir is Not a Dir.\n");
    return 0;
  }
  // char buffer[1024];
  // get_block(dev, parentInode->INODE.i_block[0], buffer);
  // DIR * potat = (DIR *) buffer;
  // printf("%s\n", potat->name);

  potato=kcwsearch(tempip, child);

  if (potato){
    printf("[ERROR] Dir already exist.\n");
    return 0;
  }


  mymkdir(parentInode, child);

  (parentInode->INODE).i_links_count++;
  (parentInode->INODE).i_atime=time(0L);

  iput(parentInode);


  printf("Inode Imode=%x\nPotato Value=%d\n", tempip->i_mode, potato);
}

int my_creat(MINODE *pip, char *name){
  int ino=0;
  MINODE *tempMI;
  DIR   *dp;
  char  *cp;
  char buf[1024];

  ino=ialloc(dev);

  tempMI=iget(dev, ino);
  INODE *ip=&(tempMI->INODE);

  ip->i_mode = 0x81A4;
  ip->i_uid  = running->uid;  // Owner uid 
  ip->i_gid  = running->gid;  // Group Id
  ip->i_size = 0;   // Size in bytes 
  ip->i_links_count = 1;          // Links count=2 because of . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
  ip->i_blocks = 2;                 // LINUX: Blocks count in 512-byte chunks 
  ip->i_block[0] = 0;             // new DIR has one data block   
  for(int i=1; i<15;i++) ip->i_block[i]=0;

  tempMI->dirty=1;

  iput(tempMI);

  enter_name(pip, ino, name);

}

int creat_file(char* path){
  MINODE *parentInode;
  INODE *tempip;
  int potato=0;

  int parentInodeNumber=0;
  char child[100];
  int i=0, sizeOfparent=0;
  int flag=0;

  printf("creat()\n");

  //tokenize path just in case
  tokenize(path);

  //hand ablosute or relative path
  if (path[0]=='/') dev = root->dev;
  else {
    flag=1;
    dev = running->cwd->dev;
    sizeOfparent= -1;
  }

  for(i=0;i<(n-1);i++) sizeOfparent+=(strlen(name[i])+1);
  path[sizeOfparent]=0;
  strcpy(child, name[i++]);

  if (flag){
    parentInodeNumber=kcwsearch(running->cwd, ".");
    parentInode = iget(dev, parentInodeNumber);
  }else{
  parentInodeNumber=getino(dev, path);
  parentInode = iget(dev, parentInodeNumber);
  }

  tempip = &(parentInode->INODE);

  //check for dir
  if ((tempip->i_mode & 0xF000) != 0x4000){
    printf("[ERROR] Parent Dir is Not a Dir.\n");
    return 0;
  }

  potato=kcwsearch(tempip, child);

  if (potato){
    printf("[ERROR] File/Dir already exist.\n");
    return 0;
  }

  my_creat(parentInode, child);


  iput(parentInode);
}




main(int argc, char *argv[ ])
{
  int k = 0;
  for(k = 0; k < 64; k++)
  {
    name[k] = (char *)malloc(sizeof(char)*256);
  }
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
    printf("input command : [ls|cd|pwd|mkdir|creat|quit] ");
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
    if (!strcmp(cmd, "creat")) creat_file(pathname);
    if(!strcmp(cmd, "rmdir"))
    {
      my_rmdir(pathname);
    }
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
