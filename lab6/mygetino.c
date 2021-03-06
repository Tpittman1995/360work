
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include "type.h"


extern MINODE minode[ ];
extern PROC *running;
extern int fd, dev;
extern int bmap, imap, iblk;


extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern MINODE *root;
/*
char mynames[64][128],*myname[64]; 
int  myn;
*/

// return minode pointer to loaded INODE
MINODE *myiget(int dev, int ino)
{
  int i;
  MINODE *mip;
  char buf[BLKSIZE];
  int blk, disp;
  INODE *ip;
  
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->dev == dev && mip->ino == ino){
       mip->refCount++;
      // //("found [%d %d] at minode[%d]: return its addr\n", dev, ino, i);
       return mip;
    }
  }
    
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
      ////("load INODE=[%d %d] into minode[%d]\n", dev, ino, i);
       mip->refCount = 1;
       mip->dev = dev;
       mip->ino = ino;

       // get INODE of ino to buf    
       blk  = (ino-1)/8 + iblk;
       disp = (ino-1) % 8;

       ////("iget: ino=%d blk=%d disp=%d\n", ino, blk, disp);

       get_block(dev, blk, buf);
       ip = (INODE *)buf + disp;
       // copy INODE to mp->INODE
       mip->INODE = *ip;
       return mip;
    }
  }   
  //("PANIC: no more free minodes\n");
  return 0;
}

myiput(MINODE *mip)
{
 int i, block, offset;
 char buf[BLKSIZE];
 INODE *ip;

 if (mip==0) 
     return;

 mip->refCount--;
 
 if (mip->refCount > 0) return;
 if (!mip->dirty)       return;
 
 /* write back */
 ////("myiput: dev=%d ino=%d\n", mip->dev, mip->ino); 

 block =  ((mip->ino - 1) / 8) + iblk;
 offset =  (mip->ino - 1) % 8;

 /* first get the block containing this inode */
 get_block(mip->dev, block, buf);

 ip = (INODE *)buf + offset;
 *ip = mip->INODE;

 put_block(mip->dev, block, buf);

} 

int tokenize(char *pathname)
{
  int i;
  char *s;
  //("tokenize %s\n", pathname);
  strcpy(gpath, pathname);
  n = 0;

  s = strtok(gpath, "/");

  while(s){
    name[n] = s;
    n++;
    s = strtok(0, "/");
  }

  for (i= 0; i<n; i++)
    //("%s  ", name[i]);
  //("\n");

  return n;
}

int mysearch(MINODE *mip, char *name)
{
   int i; 
   char *cp, c, sbuf[BLKSIZE];
   DIR *dp;
   INODE *ip;

   //("mysearch:search for %s in MINODE = [%d, %d]\n", 
          // name,mip->dev,mip->ino);
   ip = &(mip->INODE);

   /**********  search for a file name ***************/
   for (i=0; i<12; i++){ /* search direct blocks only */
        //("mysearch: i=%d  i_block[%d]=%d\n", i, i, ip->i_block[i]);
        if (ip->i_block[i] == 0)
           return 0;

	//getchar();

        get_block(dev, ip->i_block[i], sbuf);
        dp = (DIR *)sbuf;
        cp = sbuf;
        //("   i_number rec_len name_len    name\n");

        while (cp < sbuf + BLKSIZE){
	    c = dp->name[dp->name_len];
            dp->name[dp->name_len] = 0;
           
            //("%8d%8d%8u        %s\n", 
                   // dp->inode, dp->rec_len, dp->name_len, dp->name);
            if (strcmp(dp->name, name)==0){
                //("found %s : ino = %d\n", name, dp->inode);
                return(dp->inode);
            }
            dp->name[dp->name_len] = c;
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
   }
   return(0);
}

int mygetino(int ldev, char *pathname)
{
  int i, ino, blk, disp;
  char buf[BLKSIZE];
  INODE *ip;
  MINODE *mip;
  MINODE *spec;

  //("mygetino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/")==0){
      dev = root->dev;
      return 2;
    }

  if (pathname[0]=='/'){
    dev=root->dev;
    mip = myiget(dev, 2);
  }
  else
  {
  // spec = myiget(running->cwd->dev, running->cwd->ino);
   mip = myiget(running->cwd->dev, running->cwd->ino);
  // spec = myiget(running->cwd->dev, running->cwd->ino);
  }

  strcpy(buf, pathname);
  tokenize(buf);

  for (i=0; i<n; i++){
      //("===========================================\n");
      //("mygetino: i=%d name[%d]=%s\n", i, i, name[i]);
 
      ino = mysearch(mip, name[i]);

      if (ino==0){
         myiput(mip);
         //("name %s does not exist\n", name[i]);
         return 0;
      }
      myiput(mip);
      mip = myiget(dev, ino);
      int x = 0;
      char tempString[256];
      //("before if\n");
      //("%d\n", running->cwd->ino);
      if(running->cwd->ino==2 && running->cwd->dev!=root->dev && !strcmp(pathname, "..")){
        //dev = mip->mptr->dev;

        strcpy(tempString, mip->mptr->mntName);
        //("pathtoset=%s\n", tempString);
        dev = root->dev;
        myiput(mip);
        x = mip->mptr->mntDirPtr->ino;
        //("%d\n", x);
        mip = myiget(root->dev, x);
        x = mysearch(mip, "..");
        //("%d\n", x);

        //mip = myiget(root->dev, x);
        iput(mip);
        return x;
        //x = mygetino(dev, tempString);
        // //("ino=%d\n", x);
       // mip = myiget(dev, 2);
        //ino = x;
        //ino=2;

      }else if(mip->mounted == 1)
      {
        //("changing dev num. was :%d\n", dev);
        dev = mip->mptr->dev;
        myiput(mip);
        mip = myiget(dev, 2);
        ino = 2;
        //("is now : %d\n", dev);
      }

   }

   iput(mip);
   return ino;
}
