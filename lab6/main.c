/*To Do:
  FIXED:fix absolute path of unlink/rm
  FIXED:fix ls. Problem: if you ls something that doesnt exist it breaks 

*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h> 

#include "type.h"

//struct stat mystat, *stp;

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

MINODE minode[NMINODE];
MINODE *root;

PROC   proc[NPROC], *running;
MNTABLE mntable, *mntPtr;

SUPER *sp;
GD    *gp;
INODE *ip;
char * readBuf;

int fd, dev;
int nblocks, ninodes, bmap, imap, iblk;
char line[128], cmd[32], pathname[64], pathname1[64];

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
  //printf("iput(%d %d)\n", mip->dev, mip->ino);
  return kcwiput(mip);
}

int getino(int dev, char *pathename)
{
  return kcwgetino(dev, pathname);
}
#include "util.c"
#include "rmdir.c"
#include "unlink.c"
#include "readlink.c"
#include "openTry.c"
#include "close.c"
#include "read.c"
#include "trywrite.c"
#include "try_mount.c"
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
// load root INODE ansprintfd set root pointer to it
/*
int lseek(int fd, int position)
{
  int originalPosition = 0;
  OFT * oftp = running->fd[fd];
  if(position < 0 || position >= oftp->mptr->INODE.i_size)
  {
    printf("outside of file range\n");
    return 0;
  }
  originalPosition = oftp->offset;
  oftp->offset = position;

  return originalPosition;
}
*/
int pfd()
{
  char temp[256];
  int i = 0;
  printf("fd\tmode\toffset\tINODE\n---\t----\t------\t--------\n");
  //printf("%d\n", running->fd[0]->mptr->ino);
  while(i < 16)
  {
    if(running->fd[i]!=0)
    {
    //printf("fd[%d]=%d\n", i, running->fd[i]);
    switch(running->fd[i]->mode){
         case 0 : strcpy(temp, "READ");     // R: offset = 0
                  break;
         case 1 : strcpy(temp, "WRITE");        // W: truncate file to 0 size          
                  break;
         case 2 : strcpy(temp, "READ/WRITE");     // RW: do NOT truncate file
                  break;
         case 3 : strcpy(temp, "APPEND");  // APPEND mode
                  break;
         default: printf("invalid mode\n");
                  return(-1);
    }
    printf("%2d\t%4s\t%4d\t[%d, %d]\n", i, temp, running->fd[i]->offset, running->fd[i]->mptr->dev, running->fd[i]->mptr->ino);
    }
    i++;
  }
}

/*
void print_indirect()
{
  int iino = 
  MINODE * mip = kcwiget(dev, iino);
  int ibuf[256];
  int i = 0;
  INODE * ip = &(mip->INODE);

  get_block(dev, ip->i_block[12], ibuf);

    //int *temp = (int *)buf;

  for (i = 0; i < 256; i++) //freeing indirect blocks
  {
    printf("in for\n");
    if(ibuf[i] == 0)
    {
      break;
    }
    //bdealloc(dev, ibuf[i]);
    //ibuf[i] = 0;
    printf("%d\n", ibuf[i]);
  }
}
*/
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
    //temp->refCount--;
    if ((temp->INODE.i_mode & 0xF000) != 0x4000){
      printf("not a DIR\n");
      iput(temp);
      return;
    }
    printf("changing directory\n");
    running->cwd = temp;
    iput(temp);
  }
  else
  {
    printf("Does not exist\n");
  }
}



void pwd(MINODE * wd)
{
  MINODE * temp;
  //printf("%d\n", wd->ino);
  char buf[BLKSIZE];
  if(wd->ino == 2)
  {
    printf("/");
    return;
  }
  if(kcwsearch(wd, "..") == 2)
  {
    temp = iget(dev, kcwsearch(wd, ".."));
    ip = &(temp->INODE);
    
    //printf("/\n");
    //strcat(pwdBuf, "/");
    searchForName(buf, wd->ino);
    iput(temp);
    return;
  }
  

  pwd(iget(dev, kcwsearch(wd, "..")));
  //searchForName(buf, wd->ino);
  //printf("hello\n");
  
  //printf("after rpwd\n");
  //ip = &(wd->INODE);
  temp = iget(dev, kcwsearch(wd, ".."));
  ip = &(temp->INODE);
  printf("%d\n", wd->ino);
  searchForName(buf, wd->ino);
  iput(temp);

}



int ls_file_new(int inum, char * name)
{
  time_t rawtime;
  struct tm * timel;
  MINODE * mip = kcwiget(dev, inum);
  ip = &(mip->INODE);
  //printf("in new ls\n");
 // printf("%x\n", ip->i_mode);
  char ftime[256];
  char temp[256];
  char min[256];
  char sec[256];
  char hr[256];
  int i = 0;
 // printf("before ifs\n");
  if ((ip->i_mode & 0xF000) == 0x8000)
     printf("%c",'-');
  if ((ip->i_mode & 0xF000) == 0x4000)
  {
     printf("%c",'d');
  }
  if ((ip->i_mode & 0xF000) == 0xA000)
     printf("%c",'l');
  //printf("after ifs\n");
  for (i=8; i >= 0; i--){
    if (ip->i_mode & (1 << i))
      printf("%c", t1[i]);
    else
      printf("%c", t2[i]);
  }
 // printf("after for\n");
  printf("%4d ",ip->i_links_count);
  printf("%4d ",ip->i_gid);
  printf("%4d ",ip->i_uid);
  printf("%8d ",ip->i_size);
  //printf("after all prints\n");
  // print time

  timel = localtime(&(ip->i_atime));
  strcpy(ftime, asctime(timel));
  ftime[strlen(ftime) - 1] = 0;
  printf("%s  ", ftime);
  printf("%s", name);
  //printf("%d\n", localtime(ip->i_ctime));
  //sprintf(temp, "%d", ip->i_ctime);
  //strcpy(ftime, temp);
 // printf("after string copy\n");
  //ftime[strlen(ftime)-1] = 0;
 // printf("%s  ",ftime);

  // print name
 // printf("%s\n", name);  

  // print -> linkname if it's a symbolic file
  if ((ip->i_mode & 0xF000)== 0xA000){ // YOU FINISH THIS PART
    printf("->%s\n",ip->i_block);
     // use readlink() SYSCALL to read the linkname
     // printf(" -> %s", linkname);
  }
  iput(mip);
  printf("\n");
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
          ls_file_new(dp->inode, temp);
  
       		//printf("%s\n", dp->name);

       		cp += dp->rec_len;
       		dp = (DIR *)cp;
    	}
    	}
	}else{
		//printf("HELLOW\n");
		potato = kcwgetino(dev, pathname);
    if(potato == 0)
    {
      printf("directory does not exist\n");
      return;
    }
		tempMI = iget(dev, potato);
		tempip = &(tempMI->INODE);
		//printf("HELLo\n");
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

  if(path[0]=='\0'){
    printf("[ERROR] usage mkdir <pathname>\n");
    return -100;
  }
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

  printf("HERER\n");


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
    parentInode = kcwiget(dev, parentInodeNumber);
  }else{
  printf("PATH=%s\n", path);
  parentInodeNumber=kcwgetino(dev, path);
  printf("Pin=%d\n", parentInodeNumber);
  parentInode = kcwiget(dev, parentInodeNumber);
  }

  tempip = &(parentInode->INODE);
  printf("Mode=%x Inode=%d Parent=%s\n", tempip->i_mode, parentInode->ino, path);
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

try_link(char* source, char* dest){
  //check if empty
  if (source[0]=='\0'||dest[0]=='\0'){
    printf("[ERROR] usaged: link <path> <path>\n");
    return 0;
  }

  int sizeOfparent=0,i=0;
  char childD[64];

  tokenize(dest);

  for(i=0;i<(n-1);i++) sizeOfparent+=(strlen(name[i])+1);
  dest[sizeOfparent]=0;
  strcpy(childD, name[i++]);

  printf("Parentdest=%s childdest=%s\n", dest, childD);

  //get source ino
  int getSourceIno=kcwgetino(dev,source);
  if (getSourceIno==0){
    printf("[ERROR] Source doesn't exist.\n");
    return 0;
  }
  printf("Found sourceINO=%d\n", getSourceIno);

  MINODE *sourceMino=kcwiget(dev, getSourceIno);

  printf("Source Inode Mode:%x\n", (sourceMino->INODE).i_mode);
  //check for dir
  if ((sourceMino->INODE).i_mode & 0xF000 == 0x4000){
    printf("[ERROR] Source is a Dir.\n");
    return 0;
  }

  int getDestParentIno=kcwgetino(dev, dest);
  if (getDestParentIno==0){
    printf("[ERROR] Dest parent doesn't exist.\n");
    return 0;
  }

  if (dest[0]=='\0') getDestParentIno = 2;


  MINODE *destParMInode=kcwiget(dev, getDestParentIno);

  printf("Dest Parent Inode:%d\n", destParMInode->ino);

  int destChildIno=kcwsearch(destParMInode, childD);
  if (destChildIno!=0){
    printf("[ERROR] Dest child %s already exist.\n", childD);
  }

  printf("Passed all checks for link!\n");


  (sourceMino->INODE).i_links_count++;
  enter_name(destParMInode, getSourceIno, childD);
  iput(sourceMino);
}

try_symlink(char* source, char* dest){
  //check if empty
  if (source[0]=='\0'||dest[0]=='\0'){
    printf("[ERROR] usaged: symlink <path> <path>\n");
    return 0;
  }

  int sizeOfparent=0,i=0;
  char childD[64];

  tokenize(dest);

  for(i=0;i<(n-1);i++) sizeOfparent+=(strlen(name[i])+1);
  dest[sizeOfparent]=0;
  strcpy(childD, name[i++]);

  printf("Parentdest=%s childdest=%s\n", dest, childD);

  //get source ino
  int getSourceIno=kcwgetino(dev,source);
  if (getSourceIno==0){
    printf("[ERROR] Source doesn't exist.\n");
    return 0;
  }
  printf("Found sourceINO=%d\n", getSourceIno);

  MINODE *sourceMino=iget(dev, getSourceIno);

  printf("Source Inode Mode:%x\n", (sourceMino->INODE).i_mode);
  //check for dir
  if ((sourceMino->INODE).i_mode & 0xF000 == 0x4000){
    printf("[ERROR] Source is a Dir.\n");
    return 0;
  }

  int getDestParentIno=kcwgetino(dev, dest);
  if (getDestParentIno==0){
    printf("[ERROR] Dest parent doesn't exist.\n");
    return 0;
  }

  MINODE *destParMInode=iget(dev, getDestParentIno);

  printf("Dest Parent Inode:%d\n", destParMInode->ino);

  int destChildIno=kcwsearch(destParMInode, childD);
  if (destChildIno!=0){
    printf("[ERROR] Dest child %s already exist.\n", childD);
    return 0;
  }

  printf("Passed all checks for symlink!\n");

  char temp[65];
  char buf[1024];
  temp[0]=0;
  strcat(temp, dest);
  strcat(temp, "/");
  strcat(temp, childD);

  printf("Dest=%s\n", temp);

  creat_file(temp);
  strcat(temp, "/");
  strcat(temp, childD);
  int getIno=kcwgetino(dev, temp);
  MINODE *mip=kcwiget(dev, getIno);

  mip->INODE.i_mode = 0xA1A4;
  mip->INODE.i_size = strlen(source);
  strcpy((mip->INODE).i_block, source);

  mip->dirty = 1;
  iput(mip);


}

try_touch(char* path){
  if (path[0]==0){
    printf("[ERROR] Usage: link <pathname>\n");
    return 0;
  }

  printf("touch()\npath=%s\n", path);

  //get
  int getPathIno=kcwgetino(dev,path);
  if (getPathIno==0){
    printf("[ERROR] File doesn't exist.\n");
  }

  printf("Found Inode=%d\n", getPathIno);

  MINODE *mip=kcwiget(dev, getPathIno);

  INODE *tempip = &(mip->INODE);
  printf("Mode=%x Inode=%d Path=%s\n", tempip->i_mode, mip->ino, path);
  //check for dir
  if ((tempip->i_mode & 0xF000) == 0x4000){
    printf("[ERROR] Found dir instead of file.\n");
    return 0;
  }

  printf("Updating time fields.\n");

  printf("Old Access Time=%d\n", tempip->i_atime);

  tempip->i_atime=time(0L);

  printf("New Access Time=%d\n", tempip->i_atime);

  mip->dirty=1;
  iput(mip);
}

try_chmod(char *code, char* pathname){
  if (pathname[0]==0||code[0]==0){
    printf("[ERROR] ussage: chmod <option> <pathname>\n");
    return 0;
  }
  printf("chmod()\ncode=%s pathname=%s\n", code, pathname);

  int findINO=kcwgetino(dev, pathname);
  if (findINO==0){
    printf("[ERROR] Couldn't find Inode. Make sure file exist.\n");
    return 0;
  }
  printf("Found Inode=%d\n", findINO);

  MINODE *mip=kcwiget(dev, findINO);

  INODE *tempip = &(mip->INODE);
  //check for dir
  if ((tempip->i_mode & 0xF000) == 0x4000){
    printf("[ERROR] Found dir instead of file.\n");
    return 0;
  }
  printf("Mode=%x Inode=%d Path=%s\n", tempip->i_mode, mip->ino, pathname);

  printf("Old mode=%x\n",tempip->i_mode);

  tempip->i_mode = (tempip->i_mode & 0xF000)| strtol(code, NULL, 8);

  printf("New mode=%x\n",tempip->i_mode);

  mip->dirty=1;
  iput(mip);
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
    printf("open %s failed\n", disk);  
    exit(1);
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
    printf("input command : [ls|cd|pwd|mkdir|creat|rmdir|rm|link|unlink|symlink|readlink|touch|chmod|open|lseek|close|dup|dup2|write|read|cat|mount|umount|quit] ");
    line[0]=0;
    pathname[0]=0;
    pathname1[0]=0;
    cmd[0]=0;
    //getchar();
    fgets(line, 128, stdin);

    line[strlen(line)-1] = 0;


    if (line[0]==0)
      continue;
    pathname[0] = 0;



    sscanf(line, "%s %s %s", cmd, pathname, pathname1);
    printf("cmd=%s pathname=%s\n", cmd, pathname);
   /* if(!strcmp(cmd, "p"))
      print_indirect();*/
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
    if (!strcmp(cmd, "link")) try_link(pathname, pathname1);
    if (!strcmp(cmd, "rm")) unlink(pathname);
    if (!strcmp(cmd, "open")) open_file(pathname, pathname1);
    if(!strcmp(cmd, "read"))
    {
      read_file(atoi(pathname), atoi(pathname1));
    }
    if(!strcmp(cmd, "cat"))
    {
      myCat(pathname);
    }
    if(!strcmp(cmd, "lseek"))
    {
      mylseek(atoi(pathname),atoi(pathname1));
    }
    if(!strcmp(cmd, "close"))
    {
      my_close(atoi(pathname));
    }
    if(!strcmp(cmd, "unlink"))
    {
      unlink(pathname);
    }
    if(!strcmp(cmd, "readlink"))
    {
      printf("%s\n", readlink(pathname));
    }
    if (!strcmp(cmd, "symlink")) try_symlink(pathname, pathname1);
    if (!strcmp(cmd, "touch")) try_touch(pathname);
    if (!strcmp(cmd, "chmod")) try_chmod(pathname, pathname1);
    if (!strcmp(cmd, "pfd")) pfd();
    if (!strcmp(cmd, "dup")) try_dup(pathname);
    if (!strcmp(cmd, "dup2")) try_dup2(pathname, pathname1);
    if (!strcmp(cmd, "write")) write_file(pathname, pathname1);
    if (!strcmp(cmd, "cp")) try_cp(pathname, pathname1);
    if (!strcmp(cmd, "mv")) try_move(pathname, pathname1);
    if (!strcmp(cmd, "mount")) try_mount(pathname, pathname1);
    if (!strcmp(cmd, "umount")) try_umount(pathname);


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
