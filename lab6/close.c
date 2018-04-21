extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern PROC *running;
extern int dev;
extern int bmap, imap;


void close(int fd)
{
	OFT * oftp;

	if(!running->fd[fd])
	{
		printf("No Proc structure pointed at by OFT *fd\n");
		return;
	}
	else if(fd > 15)
	{
		printf("fd is:%d which is out of range\n", fd);
		return;
	}

	oftp = running->fd[fd];
	running->fd[fd] = 0;
	oftp->refCount--;
	if(oftp->refCount > 0)
	{
		return;
	}

	MINODE * mip = oftp->mptr;
	iput(mip);

	return;

}