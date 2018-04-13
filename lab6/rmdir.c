
extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern PROC *running;
extern int dev;


int clr_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] &= ~(1 << j);
}

void bdealloc(int dev, unsigned int i_blk)
{
	//int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, bmap, buf);
       clr_bit(buf,i_blk);
       incFreeBlocks(dev);

       put_block(dev, bmap, buf);
}



int incFreeBlocks(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, 2, buf);
}


int incFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

void bdealloc(int dev, unsigned int i_blk)
{
	//int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, bmap, buf);
       clr_bit(buf,i_blk);
       incFreeBlocks(dev);

       put_block(dev, bmap, buf);
}

void idalloc(int dev, MINODE * mip)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  clr_bit(buf, (mip->ino + 1));
  incFreeInodes(dev);

  put_block(dev, imap, buf);

  //return 1;
}

int check_if_empty(MINODE * mip, char * pathname)
{
	INODE *tempip = &(mip->INODE);
	int number = 0;
	char buff[BLKSIZE];
	char temp[256];
	char *cp;
	char *s;


		for (int i=0; i<12; i++){ 

    		if (tempip->i_block[i]==0) break;

    		
    		get_block(dev, tempip->i_block[i], buff);

    		cp = buff;
    		dp = (DIR *)buff;
    		

    	while(cp<buff+1024){
    		strncpy(temp, dp->name, dp->name_len);
       		temp[dp->name_len] = 0;
  
       		//printf("%s\n", dp->name);
       		number++;

       		cp += dp->rec_len;
       		dp = (DIR *)cp;
    	}
    	}
	
    	return number;
}

int my_rmdir(char* path){
	//printf("Hello World: %s\n", path);
	int ino = getino(&dev, path);
	MINODE * mip = iget(dev, ino);	
	mip->refCount--;
	

	if ((mip->INODE.i_mode & 0xF000) != 0x4000){
    	printf("not a DIR\n");
    	//return 0;
  	}
  	else if(mip->refCount > 2)
  	{
  		printf("%d\n", mip->refCount);
  		printf("Directory is Busy\n");
  		//return 0;
  	}
  	else if(check_if_empty(mip, path) > 2)
  	{
  		printf("Directory is not empty\n");
  	}
  	else if(running->uid != 0 && running->uid != mip->INODE.i_uid)
  	{
  		printf("You do not have permission\n");
  	}
  	else if
  	{
  		 for (i=0; i<12; i++){
         if (mip->INODE.i_block[i]==0)
             continue;
         bdealloc(mip->dev, mip->INODE.i_block[i]);
         idealloc(mip->dev, mip->ino);
    	 iput(mip); //(which clears mip->refCount = 0);  
    	 /*rm_child(MINODE *pip, char *name);
         pip->parent Minode, name = entry to remove*/
     }	
  	}
  	printf("running uid = %d\n INODE uid = %d \n",running->uid, mip->INODE.i_uid);


  	//iput(mip);

	//tokenize(path);
	printf("tokenize: %s %d\n", name[1], n);

	return 0;
}