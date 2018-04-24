extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern PROC *running;
extern int dev;
extern int bmap, imap;


void truncate(MINODE * mip)
{
	printf("Truncating ino = %d!!!!!!!!!!!!!!!!!\n", mip->ino);
	INODE * ip = &(mip->INODE);
	int i = 0;
	int j = 0;
	char sbuf[1024];
	int ibuf[256];
	int ibuf2[256];
	ip->i_size = 0;
	ip->i_atime=time(0L);
	mip->dirty = 1;
	for (i=0; i<12; i++){ /* search direct blocks only */
      
        if (ip->i_block[i] == 0){
           return 0;
        }
       	else{
       		bdealloc(dev, ip->i_block[i]);
       		ip->i_block[i] = 0;
       	}

	}

	if(ip->i_block[12] != 0)
	{
		get_block(dev, ip->i_block[12], ibuf);
	}
	else
	{
		return;
	}
    //int *temp = (int *)buf;

	for (i = 0; i < 256; i++) //freeing indirect blocks
	{
		if(ibuf[i] == 0)
		{
			break;
		}
		bdealloc(dev, ibuf[i]);
		ibuf[i] = 0;
	}



	if(ip->i_block[13] != 0)
	{
		get_block(dev, ip->i_block[13], ibuf);
	}
	else
	{
		return;
	}
	//get_block(dev, ip->i_block[13], ibuf);

	for(i = 0; i < 256; i++)
	{
		if(ibuf[i] == 0)
		{
			break;
		}
		get_block(dev, ibuf[i], ibuf2);
		for(j = 0; j < 256; j++)
		{
			if(ibuf2[j] == 0)
			{
				break;
			}

			bdealloc(dev, ibuf2[j]);
			ibuf2[i] = 0;
		}

	}




    

}

void unlink(char * path)
{
	char child[256];
	char parent[256];
	tokenize(path);
	strcpy(child, name[n-1]);
	int childIno = kcwgetino(dev, path);
	printf("%d\n", childIno);
	int pino = 0;

	if(childIno == 0)
	{
		printf("incorrect path or does not exist\n");
		return 0;
	}
	
	printf("%d\n", childIno);
	MINODE * mip = kcwiget(dev, childIno);

	printf("%d\n", mip->ino);

	if((mip->INODE.i_mode & 0xF000) == 0x4000){
    	printf("Cannot unlink a DIR\n");
    	iput(mip); //(which clears mip->refCount = 0);
    	return;
    	//return 0;
  	}

  	mip->INODE.i_links_count--;

  	printf("before ifs\n");


  	if(mip->INODE.i_links_count == 0)
  	{
  		printf("%d\n", mip->ino);
  		truncate(mip);
  		idealloc(mip->dev, mip);
  	}


	if(path[0] == '/' && n == 1)
	{
		printf("in first check\n");
		pino = 2;
	}
	else if(path[0] == '/')
	{
		printf("in second check\n");
		//printf("in if\n");
		strcpy(parent, path);
		parent[strlen(parent)] = 0;
		parent[strlen(parent)- strlen(child)-1] = 0;
		//ppath[strlen(ppath)- 1] = 0;
		//printf("%s\n", ppath);
		pino = kcwgetino(dev, parent);
		printf("%s\n %d\n", parent, pino);
	}
	else
	{
		//printf("in else\n");
		pino = running->cwd->ino;
		//printf("%d\n", pino);
	}

	printf("after ifs\n");

	MINODE * pip = kcwiget(dev, pino);
	printf("%d\n", mip->refCount);
	mip->refCount--;

	rm_child(pip, child);


}