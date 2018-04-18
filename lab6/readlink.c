extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern PROC *running;
extern int dev;
extern int bmap, imap;



void readlink(char * path)
{
	int ino = kcwgetino(dev, path);
	MINODE * mip = kcwiget(dev, ino);

	if((mip->INODE.i_mode & 0xF000) != 0xA000){
    	printf("Not a symlink\n");
    	iput(mip); //(which clears mip->refCount = 0);
    	return;
    	//return 0;
  	}

  	for (int i=0; i<12; i++){ 

    		if (tempip->i_block[i]==0) break;

    		
    		get_block(dev, tempip->i_block[i], buff);

    		printf("%s\n", buff);
    	
    	}
}