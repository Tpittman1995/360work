
extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern PROC *running;
extern int dev;
extern int bmap, imap;


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

void idealloc(int dev, MINODE * mip)
{
	printf("in idealloc\n");
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);
  printf("after get block %d\n", mip->ino);
  clr_bit(buf, (mip->ino + 1));
  printf("after clr bit\n");
  incFreeInodes(dev);
 	printf("after inc\n");

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

void rm_child(MINODE *pip, char * myname)
{
	printf("in rmchild\n");
	char buf[1024];
	int j = 0;
	char temp[256];
	int i = 0;
	int flag=0;
	int IDEAL_LEN = 0;
	for (i=0; i<12; i++){ // search direct blocks only
     if (pip->INODE.i_block[i] == 0) 
           return -1;
       printf("%d %d %d\n", pip->dev, pip->INODE.i_block[i], buf);
     get_block(pip->dev, pip->INODE.i_block[i], buf);
     DIR * dp = (DIR *)buf;
     char * cp = buf;
     DIR * dp_prev = (DIR *)buf;

     printf("before while\n");
     printf("%d\n", cp);
     printf("%d\n", buf);
     printf("%d\n",dp->rec_len);
     while (cp < buf + BLKSIZE){
     	//printf("%d\n", cp);
     	//printf("in while\n");
     	printf("%s\n", dp->name);
     	printf("%s\n", myname);
       strncpy(temp, dp->name, dp->name_len);
       temp[dp->name_len] = 0;
       IDEAL_LEN = 4*((8 + strlen(temp) + 3)/4);

       //printf("%s  ", temp);
       if(strcmp(dp->name, myname) == 0)
       {

       	
     
       	printf("in first if\n");
       		if (IDEAL_LEN != dp->rec_len){
       			printf("%s %d", dp_prev->name, dp_prev->rec_len);
           		dp_prev->rec_len += dp->rec_len;  
           		printf("%d\n", dp_prev->rec_len);
           		pip->dirty = 1;
    			iput(pip);
    			put_block(pip->dev, pip->INODE.i_block[i], buf);
    			return;

       		}
       		else if(cp == buf && IDEAL_LEN != dp->rec_len)
       		{
       			pip->INODE.i_size -= 1024;
       			printf("%d\n", pip->INODE.i_size);
       			for(j = i; j < 11; j++)
       			{
       				pip->INODE.i_block[j] = pip->INODE.i_block[j+1];
       			}
       			pip->INODE.i_block[11] = 0;
       			pip->dirty = 1;
    			iput(pip);
    			put_block(pip->dev, pip->INODE.i_block[i], buf);
    			return;
       		}
       		else
       		{
       			printf("in else\n");
       			printf("%d\n", cp);
       			printf("%d\n", buf);
       			printf("%d\n",dp->rec_len );
       			//exit(1);
       			while(cp<buf+1024){
       				printf("%u\n", cp);
       			printf("%u\n", buf);

       				//exit(1);

       				dp_prev=(DIR *) cp;
       				cp+=dp->rec_len;
       				dp=(DIR *)cp;
				printf("%u\n", cp);
       			printf("%u\n", buf);   
       			//exit(1);    				
       			printf("%s\n", dp_prev->name);
       			dp_prev=dp;
       			printf("%s\n", dp_prev->name);

       			//exit(1);
       			}

       			dp->rec_len+=IDEAL_LEN;


       			printf("after for loop\n");
       			pip->dirty = 1;
    			iput(pip);
    			put_block(pip->dev, pip->INODE.i_block[i], buf);
    			return;
       		}
       		//else if()

  		}
       if(cp > buf)
       {
       	dp_prev = (DIR *)cp;
       } 
       cp += dp->rec_len;
       dp = (DIR *)cp;

     }
     
 }
}

int my_rmdir(char* path){
	//printf("Hello World: %s\n", path);
	int pino = 0;

	char ppath[256];
	tokenize(path);
	int ino = kcwgetino(dev, path);
	printf("ljalg\n");
	if(ino == 0)
	{
		printf("incorrect path or directory does not exist\n");
		return;
	}
	int i = 0;

	MINODE * mip = iget(dev, ino);
	printf("%d %s\n", mip->refCount, path);
	strcpy(ppath, path);
	if(path[0] == '/')
	{
		printf("in if\n");
		ppath[strlen(ppath)] = 0;
		ppath[strlen(ppath)- 1] = 0;
		ppath[strlen(ppath)- 1] = 0;
		printf("%s\n", ppath);
		pino = kcwgetino(dev, ppath);
	}
	else
	{
		printf("in else\n");
		pino = kcwsearch(mip, "..");
	}

	//printf("%d %s\n", n, name[0]);
	//printf("hello %s \n", name[n-2]);
	//printf("%s\n", name[n-2]);
	//int pino = kcwsearch(name[n-2]);
	printf("hello\n");
	MINODE * pip = iget(dev, pino);
	//char * name;
	//MINODE * mip = iget(dev, ino);	
	mip->refCount--;

	
	printf("Doing checks...\n");

	if ((mip->INODE.i_mode & 0xF000) != 0x4000){
    	printf("not a DIR\n");
    	return;
    	//return 0;
  	}
  	else if(mip->refCount > 2)
  	{
  		printf("%d\n", mip->refCount);
  		printf("Directory is Busy\n");
  		return;
  		//return 0;
  	}
  	else if(check_if_empty(mip, path) > 2)
  	{
  		printf("Directory is not empty\n");
  		return;
  	}
  	else if(running->uid != 0 && running->uid != mip->INODE.i_uid)
  	{
  		printf("You do not have permission\n");
  		return;
  	}
  	else
  	{
  		printf("in else\n");
  		 for (i=0; i<12; i++){
         if (mip->INODE.i_block[i]==0)
             continue;
         printf("before bdealloc\n");
         bdealloc(mip->dev, mip->INODE.i_block[i]);
         printf("before idealloc\n");
         idealloc(mip->dev, mip);
         printf("after idealloc\n");
    	 iput(mip); //(which clears mip->refCount = 0); 
    	 printf("%s\n", name[n]); 
    	 rm_child(pip, name[n]);


     }	
  	}
  	printf("running uid = %d\n INODE uid = %d \n",running->uid, mip->INODE.i_uid);


  	//iput(mip);

	//tokenize(path);
	//printf("tokenize: %s %d\n", name[1], n);

	return 0;
}