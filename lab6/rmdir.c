
extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern PROC *running;
extern int dev;


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
	printf("Hello World: %s\n", path);
	int ino = getino(&dev, path);
	MINODE * mip = iget(dev, ino);

	if ((mip->INODE.i_mode & 0xF000) != 0x4000){
    	printf("not a DIR\n");
    	return 0;
  	}
  	else if(mip->refCount > 1)
  	{
  		printf("Directory is Busy\n");
  		return 0;
  	}
  	else if(check_if_empty(mip, path) > 2)
  	{
  		printf("Directory is not empty\n");
  	}
  	
	tokenize(path);
	printf("tokenize: %s %d\n", name[1], n);
}