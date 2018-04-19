extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern PROC *running;
extern int dev;
extern int bmap, imap;



void readlink(char * path)
{
	printf("lagsohad;lasl\n");
	char buff[1024];
	int ino = kcwgetino(dev, path);
	MINODE * mip = kcwiget(dev, ino);
	printf("%x\n", mip->INODE.i_mode);
	if((mip->INODE.i_mode & 0xF000) != 0xA000){
    	printf("Not a symlink\n");
    	iput(mip); //(which clears mip->refCount = 0);
    	return;
    	//return 0;
  	}
    		//get_block(dev, mip->INODE.i_block[i], buff);

    		printf("%s\n", mip->INODE.i_block);
    	
    	
}