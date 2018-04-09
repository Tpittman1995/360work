
extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern PROC *running;
extern int dev;

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
  	else if(mip->INODE.i_links_count > 2)
  	{
  		printf("Directory is not empty\n");
  	}
  	//else if()
	tokenize(path);
	printf("tokenize: %s %d\n", name[1], n);
}