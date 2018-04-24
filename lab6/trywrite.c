extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern PROC *running;
extern int dev;
extern int bmap, imap;
extern char *readBuf;

int try_move(char* source, char* dest){

	if (source[0]==0 || dest[0]==0){
		printf("[ERROR] ussage mv <source> <dest>\n");
		return -1;
	}

	//get the ino and put it in memory INODE
	int sourceIno = kcwgetino(running->cwd->dev, source);
	if (sourceIno==0){
		printf("[ERROR] Couldn't find source INODE.\n");
		return -1;
	}
	MINODE *sourceMINODE = kcwiget(running->cwd->dev, sourceIno);

	int destIno = kcwgetino(running->cwd->dev, dest);
	if (destIno!=0){
		printf("[ERROR] Destination already exist.\n");
		return -1;
	}

	printf("Passed Check.\n");

	int check=1;

	printf("source=%s\n", source);
	if (check){
		try_link(source, dest);
		printf("source=%s\n", source);
		unlink(source);
	}else{
		try_cp(source, dest);
		unlink(source);
	}
}

int try_cp(char* source, char* dest){

	if (source[0]==0 || dest[0]==0){
		printf("[ERROR] ussage cp <source> <dest>\n");
		return -1;
	}

	int fd = open_file(source, "0");
	int gd = open_file(dest, "1");
	printf("fd=%d\n", fd);
	printf("gd=%d\n", gd);
	if(fd < 0 || gd < 0)
	{
		printf("Files must be closed to copy\n");
		if(fd >= 0)
		{
			my_close(fd);
		}

		if(gd >= 0)
		{
			my_close(gd);
		}
		return -1;

	}

	if (fd<0){
		printf("[ERROR] Source doesn't exist\n");
		return -1;
	}


	printf("fd=%d\n", fd);
	printf("gd=%d\n", gd);

	char tempGD[256];
	printf("IKNOW\n");
	printf("gd=%d\n", gd);
	sprintf(tempGD, "%d", gd);
	printf("UKNOW\n");
	printf("gd=%s\n", tempGD);


	int n = 0;
	while( n=read_file(fd, running->fd[fd]->mptr->INODE.i_size) ){
       write_file(tempGD, readBuf);  // notice the n in write()
   }

   printf("Finished writing.\n");

   my_close(fd);
   my_close(gd);



}


int try_write(int fd, char* buf, int nbytes){

	int lblk=0, startByte=0, blk=0;
	OFT *currentPROC = running->fd[fd];
	int offsetFD = currentPROC->offset;
	int ibuf[256];
	char cbuff[1024];
	MINODE *mip = currentPROC->mptr;
	INODE *ip = &(mip->INODE);

	printf("fd=%d offset=%d INODE=%d buffertowrite=%s\n", fd, offsetFD, mip->ino, buf);

	while (nbytes > 0){//while I still have to more stuff to copy
		lblk = offsetFD/1024;
		startByte = offsetFD%1024;

		if (lblk<12){//direct block
			if(ip->i_block[lblk]==0){
				ip->i_block[blk] = balloc(mip->dev);
			}
			blk = ip->i_block[lblk];
		}else if (lblk>=12 && lblk<(256+12)){//indirect block
			if(ip->i_block[12]==0){
				ip->i_block[12] = balloc(mip->dev);
				get_block(mip->dev, ip->i_block[12], ibuf);
				for (int i=0; i<256; i++) ibuf[i]=0;
				put_block(mip->dev, ip->i_block[12], ibuf);
			}
			get_block(mip->dev, ip->i_block[12], ibuf);
			blk = ibuf[blk-12];
			if (blk==0){
				ibuf[blk-12]=balloc(mip->dev);
				put_block(mip->dev, ip->i_block[12], ibuf);
			}
			blk = ibuf[blk-12];
		}else{//double indirect block
			if (ip->i_block[13]==0){
				ip->i_block[13] = balloc(mip->dev);
				get_block(mip->dev, ip->i_block[13], ibuf);
				for(int i=0; i < 256; i++) ibuf[i]=0;
				put_block(mip->dev, ip->i_block[13], ibuf);
			}
			get_block(mip->dev, ip->i_block[13], ibuf);
			if (ibuf[(lblk-256-12)/256]==0){
				ibuf[(lblk-256-12)/256] = balloc(mip->dev);
				put_block(mip->dev, ip->i_block[13], ibuf);
				get_block(mip->dev, ibuf[(lblk-256-12)/256], ibuf);
				for(int i=0; i < 256; i++) ibuf[i]=0;
				put_block(mip->dev, ibuf[(lblk-256-12)/256], ibuf);
			}
			get_block(mip->dev, ibuf[(lblk-256-12)/256], ibuf);
			if(ibuf[(lblk-256-12)%256]==0){
				ibuf[(lblk-256-12)%256] = balloc(mip->dev);
				put_block(mip->dev, ibuf[(lblk-256-12)/256], ibuf);
				get_block(mip->dev, ibuf[(lblk-256-12)%256], ibuf);
			}
			blk=ibuf[(lblk-256-12)%256];
		}

		get_block(mip->dev, blk, cbuff);

		char *cp = cbuff + startByte;      // cp points at startByte in wbuf[]
     	int remain = BLKSIZE - startByte;
     	char *cq = buf;





     	while (remain>0){

     		if (nbytes<=remain){//buff is smaller than or equal block
     			strcpy(cp, cq);
     			remain-=nbytes;
     			currentPROC->offset+=nbytes;
     			if (currentPROC->offset > ip->i_size)  // especially for RW|APPEND mode
     				ip->i_size+=nbytes; 
     			nbytes=0;
     			break; 
     		}else{//buff is smaller than the block
     			strncpy(cp, cq, remain);
     			cq+=remain;
     			nbytes-=remain;
     			currentPROC->offset+=remain;
     			if (currentPROC->offset > ip->i_size)  // especially for RW|APPEND mode
     				ip->i_size+=remain; 
     			remain=0;
     		}


     		// *cp++ = *cq++;
     		// nbytes--; remain--;
     		// currentPROC->offset++;
     		// if (currentPROC->offset > ip->i_size)  // especially for RW|APPEND mode
       //         ip->i_size++;    // inc file size (if offset > fileSize)
       //     	if (nbytes <= 0) break;

     	}
     	put_block(mip->dev, blk, cbuff);
	}
	mip->dirty = 1;       // mark mip dirty for iput()            
	return nbytes;
}

int write_file(char* fd1, char* writeThis){

	if(fd1[0]==0 && writeThis==0){
		printf("[ERROR] ussage write <FD> <text to write>\n");
		return 0;
	}

	int fd = atoi(fd1);

	if (fd<0 && fd>16){
		printf("[ERROR] FD is not in range\n");
		return 0;
	}

	OFT *oftFD = running->fd[fd];
	if (oftFD==0){
		printf("[ERROR] FD doesn't exist.\n");
		return 0;
	}
	if (oftFD->mode==0){
		printf("[ERROR] FD Mode is set to READ. Can't write.\n");
		return 0;
	}

	printf("Passed all Test for Write.\n");

	int getLen=strlen(writeThis);
	char localBuffer[getLen];

	printf("Length of String=%d\n", getLen);

	return (try_write(fd, writeThis, getLen));
}