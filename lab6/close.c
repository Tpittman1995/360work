extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern PROC *running;
extern int dev;
extern int bmap, imap;

int mylseek(int fd, int position)
{
	OFT * oftp;
  int originalPosition = 0;
  if(running->fd[fd])
  {
  	oftp = running->fd[fd];
  }
  else
  {
  	printf("fd does not exist\n");
  	return -1;
  }
  if(position < 0 || position >= oftp->mptr->INODE.i_size)
  {
    printf("outside of file range\n");
    return -1;
  }
  originalPosition = oftp->offset;
  oftp->offset = position;

  return originalPosition;
}


void my_close(int fd)
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