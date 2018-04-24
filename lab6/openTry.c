extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern PROC *running;
extern int dev;
extern int bmap, imap;

int try_dup2(char* fd, char* gd){

	if (fd[0]=='\0' && gd[0]=='\0'){
		printf("[ERROR] ussage dup2 <fd> <gd>\n");
		return 0;
	}

	int fdNum = atoi(fd);
	int gdNum = atoi(gd);

	printf("dup2()\nTrying to dup fd=%d gd=%d\n", fdNum, gdNum);

	if (fdNum>=0 && fdNum<16)printf("File descriptor in range.\n");
	else return 0;

	if (gdNum>=0 && gdNum<16)printf("gd in range\n");
	else return 0;

	OFT *tempFT = running->fd[fdNum];
	if (tempFT==0){
		printf("fd[%d] doesn't not exist. Please open file.\n", fdNum);
		return 0;
	}
	printf("Found file descriptor.\nfd=%d mode=%d refCount=%d offset=%d\n", fdNum, tempFT->mode, tempFT->refCount, tempFT->offset);

	close(gdNum);

	OFT *newOFT = (OFT *)malloc(sizeof(OFT));

	tempFT->refCount++;
	newOFT->mode=tempFT->mode;
	newOFT->refCount=tempFT->refCount;
	newOFT->offset=tempFT->offset;
	newOFT->mptr=tempFT->mptr;

	running->fd[gdNum] = newOFT;

	printf("Finished dup2.\n");
}


int try_dup(char* fd){

	if(fd[0]=='\0'){
		printf("[ERROR] ussage: dup <fd>\n");
		return 0;
	}

	int fdNum = atoi(fd);

	printf("dup()\nTrying to dup=%d\n", fdNum);

	if (fdNum>=0 && fdNum<16);
	else return 0;


	OFT *tempFT = running->fd[fdNum];
	if (tempFT==0){
		printf("fd[%d] doesn't not exist. Please open file.\n", fdNum);
		return 0;
	}
	printf("Found file descriptor.\nfd=%d mode=%d refCount=%d offset=%d\n", fdNum, tempFT->mode, tempFT->refCount, tempFT->offset);

	OFT *newOFT = (OFT *)malloc(sizeof(OFT));

	tempFT->refCount++;
	newOFT->mode=tempFT->mode;
	newOFT->refCount=tempFT->refCount;
	newOFT->offset=tempFT->offset;
	newOFT->mptr=tempFT->mptr;

	int i=0;
	for(i=0; i<16; i++)
		if(running->fd[i]==0) break;

	running->fd[i] = newOFT;
}


int open_file(char* pathName, char* mode){
	int i;
	char *t1 = "xwrxwrxwr-------";
	char *t2 = "----------------";
	int write=0, read=0;
	char workAround[16], tempPathName[256];
	MINODE *tempMInode=NULL;
	OFT *temp=NULL;
	workAround[9]=0;
	int flag=0;

	strcpy(tempPathName, pathName);

	if (pathName[0]=='/') flag=1;


	//check for passed in vars
	if (pathName[0]==0 || mode[0]==0){
		printf("[ERROR] usage error: open <pathname> <mode>\n");
		return -1;
	}
	
	printf("open()\npathname=%s mode=%s\n", pathName, mode);

	//convert mode to integer
	int modeNum = atoi(mode);
	int dev=0, ino=0;
	MINODE *mip = NULL;

	//check for rel or abosulte path
	if (tempPathName[0]='/'){
		dev = root->dev;
		ino = kcwgetino(dev, pathName);
		if(ino != 0)
		{
			mip = kcwiget(dev,ino);
		}
		else if(modeNum != 0)
		{
			creat_file(pathName);
			ino = kcwgetino(dev, pathName);
			mip = kcwiget(dev, ino);
		}
		else
		{
			printf("[ERROR] File does not exist, Can't open for read\n");
			return -1;
		}
	}
	else {
		dev = running->cwd->dev;
		ino = kcwgetino(running->cwd, tempPathName);
		if(ino != 0)
		{
			mip = kcwiget(dev, ino);
		}
		else if(modeNum != 0)
		{
			creat_file(pathName);
			ino = kcwgetino(running->cwd, tempPathName);
			mip = kcwiget(running->cwd->dev, ino);
		}
		else
		{
			printf("[ERROR] File does not exist, Can't open for read\n");

			return -1;
		}
		printf("INODE=%d\n", ino);
	}

	//get inode number for the path
	
	
	//check for file
	if(((mip->INODE).i_mode & 0xF000)!=0x8000){
		printf("[ERROR] Either not a file or don't have proper permissions.\n");
		return -1;
	}
	printf("Passed all checks.\n");

	for (i=8; i >= 0; i--){
    if ((mip->INODE).i_mode & (1 << i))
    	workAround[i]=t1[i];
    else
    	workAround[i]=t2[i];
	}

	for (i=0; i<=8; i++){
		if ('r'==workAround[i]) read=1;
		if ('w'==workAround[i]) write=1;
	}

	if (read)
		printf("Have read permissions\n");

	if (write)
		printf("Have write permissions\n");



	//checks fd pointer to make sure file isn't open for other reason
	for(i=0; i<16; i++){
		//set the OFT strcut
		temp=running->fd[i];

		//check if struct is not empty
		if (temp==0) continue;
		else{

		//get MINODE
		tempMInode=temp->mptr;

		//compare the inode of the fd pointer to the file you want open
		if (tempMInode->ino==mip->ino){
			//if the fd pointers minode is the same as the one you want to open check what mode is it open in
			if (temp->mode!=0){
				printf("[ERROR] File already opened for w/rw/or append mode.\n");
				return -1;
			}
			if (modeNum==0 && temp->mode!=0){
				printf("[ERROR] File already opened for read\n");
				return -1;
			}
			if (modeNum>0 && temp->mode>=0){
				printf("[ERROR] Already opened can't do it.\n");
				return -1;
			}
		}
	}
	}

	if (modeNum==0 && read==0){
		printf("[ERROR] Don't have read permissions.\n");
		return -1;
	}

	if (modeNum>=1 && write==0){
		printf("[ERROR] Don't have write permissions.\n");
		return -1;
	}

	OFT *oftp=(OFT *)malloc(sizeof(OFT));
	oftp->mode = modeNum;      // mode = 0|1|2|3 for R|W|RW|APPEND 
    oftp->refCount = 1;
    oftp->mptr = mip;  // point at the file's minode[]

    printf("Allocated new OFT Struct.\n");

    switch(modeNum){
         case 0 : 
         	oftp->offset = 0;     // R: offset = 0
            break;
         case 1 : 
         	truncate(mip);        // W: truncate file to 0 size
            oftp->offset = 0;
            break;
         case 2 : 
         	oftp->offset = 0;     // RW: do NOT truncate file
            break;
         case 3 : 
         	oftp->offset =  mip->INODE.i_size;  // APPEND mode
            break;
         default: 
         	printf("invalid mode\n");
            return(-1);
      }



      //itterate through fd array get the lowest one set it to oft and break
    for(i=0; i<NFD; i++){
    	temp=running->fd[i];
    	if (temp==0){
    		running->fd[i]=oftp;

    		break;
    	}
    }


    if (modeNum==0){
    	(mip->INODE).i_atime=time(0L);
    }else{
    	(mip->INODE).i_mtime=(mip->INODE).i_atime=time(0L);
    }

    mip->dirty=1;
    iput(mip);

 
    return i;
}