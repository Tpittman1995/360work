extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern PROC *running;
extern int dev;
extern int bmap, imap;
extern char * readBuf;
extern char cmd[32];


int myread(int fd, int bufSize, int numbytes)
{
	printf("\n");
	char buf[numbytes];
	free(readBuf);
	readBuf = (char *)malloc(sizeof(char)*numbytes);
	char blkbuf[1024];
	int ibuf[256];
	int count = 0;
	int lbk;
	int blk;
	int remain = 0;
	int startByte = 0;
	OFT * oftp = running->fd[fd];
	MINODE * mip = running->fd[fd]->mptr;
	INODE * ip = &(mip->INODE);
	int avil = ip->i_size - oftp->offset;
	char * cq = buf;
	char * cp;

	while(numbytes && avil)
	{
		lbk = oftp->offset / BLKSIZE;
		startByte = oftp->offset % BLKSIZE;

		if(lbk < 12)
		{
			blk = mip->INODE.i_block[lbk];
		}
		else if (lbk >= 12 && lbk < (256 + 12))
		{
			get_block(mip->dev, ip->i_block[12], ibuf);
			blk = ibuf[lbk - 12];
		}
		else if (lbk >= (256 + 12) && lbk < ((256*256) + 256 + 12))
		{
			get_block(mip->dev, ip->i_block[13], ibuf);
			get_block(mip->dev, ibuf[((lbk - 256 - 12)/256)], ibuf);
			blk = ibuf[(lbk - 256 - 12)%256];
		}

		get_block(mip->dev, blk, blkbuf);
		cp = blkbuf + startByte;
		remain = BLKSIZE - startByte;
		while(remain > 0)
		{
			
			if(remain >= numbytes)
			{
				if(avil >= numbytes)
				{
					//copy numbytes
					strncpy(cq, cp, numbytes);
					cp += numbytes;
					oftp->offset += numbytes;
					cq += numbytes;
					count += numbytes;
					numbytes -= numbytes;
					avil -= numbytes;
					remain -= numbytes;
				}
				else
				{
					strncpy(cq, cp, avil);
					cp += avil;
					oftp->offset += avil;
					cq += avil;
					count += avil;
					numbytes -= avil;
					avil -= avil;
					remain -= avil;
					//copy avil bytes
				}
			}
			else
			{
				if(avil >= remain)
				{
					//copy remain
					strncpy(cq, cp, remain);
					cp += remain;
					oftp->offset += remain;
					cq += remain;
					count += remain;
					numbytes -= remain;
					avil -= remain;
					remain -= remain;
				}
				else
				{
					//copy avil
					strncpy(cq, cp, avil);
					cp += avil;
					oftp->offset += avil;
					cq += avil;
					count += avil;
					numbytes -= avil;
					avil -= avil;
					remain -= avil;
				}
			}
			/*
			
			*cq++ = *cp++;
			oftp->offset++;
			count++;
			avil--;
			numbytes--;
			remain--;
			*/
			if(numbytes <= 0 || avil <= 0)
			{
				break;
			}
		}
	}
	printf("%d bytes read from fd %d\n",count , fd);
	buf[count] = 0;
	//printf("%s\n", buf);
	strcpy(readBuf, buf);
	//printf("%s\n", readBuf);
	return count;

}


int read_file(int fd, int numbytes)
{
	//printf("before if\n");
	if(running->fd[fd] == 0)
	{
		return -1;

	}
	//printf("after if\n");
	
	if(running->fd[fd]->mode == 0 || running->fd[fd] == 2)
	{
		return (myread(fd, running->fd[fd]->mptr->INODE.i_size, numbytes));
	}
	else
	{
		printf("not open for read\n");
		return -1;
	}


}


void myCat(char * pathName)
{
	char mode[2];
	mode[0] = '0';
	mode[1] = 0;;
	int n = 0;
	int fd = open_file(pathName, mode);
	if(fd < 0)
	{
		printf("error\n");
		return;
	}
	//printf("made it here\n");
	//getchar();
	while(n = read_file(fd, running->fd[fd]->mptr->INODE.i_size))
	{
		printf("%s", readBuf);
		//getchar();
	}
	printf("\n");
	//fflush(stdin);
	my_close(fd);
	//cmd[0] = 0;
}