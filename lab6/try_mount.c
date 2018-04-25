extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern PROC *running;
extern int dev;
extern int bmap, imap;
extern char *readBuf;
extern MINODE *root;

int try_umount(char* filesystem){
	MINODE *mip = kcwiget(running->cwd->dev, 2);
	MNTABLE *mtable = mip->mptr;

	int i=0;
	for (i=0; i<6; i++){
		if(mtable->dev != 0 && !strcmp(mtable->devName, filesystem))
			break;
		mtable++;
	}

	if(i==6){
		printf("[ERROR] Couldn't find filesystem.\n");
		return -10;
	}

	printf("Found filesystem to remove.\n");


}


int try_mount(char* filesystem, char* mountpoint){

	if (filesystem[0]=='\0' && mountpoint[0]=='\0'){
		printf("mount()\n");
		printf("Printing all Attached devices.\n");
		printf("Mount Table Content\n");
		MNTABLE *mntable = root->mptr;


		for (int i=0; i<6; i++){
		// printf("%d\n", mntable->dev);
		// printf("%s %s\n", mntable->devName, filesystem);
			if(mntable->dev != 0)
			{
				printf("dev=%d\n", mntable->dev);
				printf("ninodes=%d\n", mntable->ninodes);
				printf("nblocks=%d\n", mntable->nblocks);
				printf("bmap=%d\n", mntable->bmap);
				printf("iblk=%d\n", mntable->iblk);
				printf("nblocks=%d\n", mntable->nblocks);
				printf("devName[0]=%s\n", (mntable->devName));
				printf("mntName[0]=%s\n", (mntable->mntName));
			}
			mntable++;
		}	
		return 0;
	}
	
	if ((filesystem[0]=='\0' || mountpoint[0]=='\0')){
		printf("[ERROR] usage: mount <filesystem> <mountpoint>\n");
		return -1;
	}

	printf("filesystem=%s mountpoint=%s\n", filesystem, mountpoint);

	int minopoint = kcwgetino(dev, mountpoint);
	MINODE *mmip = kcwiget(dev, minopoint);

	if ((mmip->INODE.i_mode & 0xF000) != 0x4000 || mmip->refCount > 2){
		printf("refcount=%d\n", mmip->refCount);
		printf("[ERROR] Mounting point is not a dir/busy.\n");
		iput(mmip);
		return -9001;
	}

	MNTABLE *mntable = root->mptr;

	// printf("Mount Table Content\n");
	// printf("dev=%d\n", mntable->dev);
	// printf("ninodes=%d\n", mntable->ninodes);
	// printf("nblocks=%d\n", mntable->nblocks);
	// printf("bmap=%d\n", mntable->bmap);
	// printf("iblk=%d\n", mntable->iblk);
	// printf("nblocks=%d\n", mntable->nblocks);

	//MINODE *mountMnode = mntable->mntDirPtr;
	// printf("Mount Memory Inode Info:\n");
	// printf("dev=%d ino=%d\n", mountMnode->dev, mountMnode->ino);
	
	// printf("Printing all Attached devices.\n");
	// printf("devName[0]=%s\n", (mntable->devName));
	// printf("mntName[0]=%s\n", (mntable->mntName));

	
	//mntable++;

	// while(mntable!=NULL){
	// 	printf("Found another Mounting point.\n");
	// 	printf("Mount Table Content\n");
	// 	printf("dev=%d\n", mntable->dev);
	// 	printf("ninodes=%d\n", mntable->ninodes);
	// 	printf("nblocks=%d\n", mntable->nblocks);
	// 	printf("bmap=%d\n", mntable->bmap);
	// 	printf("iblk=%d\n", mntable->iblk);
	// 	printf("nblocks=%d\n", mntable->nblocks);

	// 	mntable++;
	// }

	for (int i=0; i<6; i++){
		// printf("%d\n", mntable->dev);
		// printf("%s %s\n", mntable->devName, filesystem);
		if(mntable->dev != 0 && !strcmp((mntable->devName), filesystem))
		{
			printf("[ERROR] Disk Already opened.\n");
			return -199;
		}

		mntable++;
	}

	int i=0;
	mntable = root->mptr;
	for (i=0; i<6; i++){//assume doesn't exist
		if (mntable->dev == 0) break;
		mntable++;
	}

	if (i==6){
		printf("[ERROR] Not enough DEV spots.\n");
		return -199;
	}

	int fd=0, ldev=0;
	printf("checking EXT2 FS ....");
  	if ((fd = open(filesystem, O_RDWR)) < 0){
    	printf("open %s failed\n", filesystem);  
    	exit(1);
  		}
  	ldev = fd;
  	printf("\nOpened filesystem=%s dev=%d\n", filesystem, ldev);

  	char buf[1024];
  	get_block(ldev, 1, buf);
  	SUPER *sp = (SUPER *)buf;

  	/* verify it's an ext2 file system *****************/
	if (sp->s_magic != 0xEF53){
		printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
		exit(1);
	}
	printf("OK\n");
	mntable->dev = fd;	
	mntable->ninodes = sp->s_inodes_count;
	mntable->nblocks = sp->s_blocks_count;
	strcpy(mntable->devName, filesystem);
  	strcpy(mntable->mntName, mountpoint);

	get_block(dev, 2, buf); 
  	GD *gp = (GD *)buf;

  	mntable->bmap = gp->bg_block_bitmap;
  	mntable->imap = gp->bg_inode_bitmap;
  	mntable->iblk = gp->bg_inode_table;
  	printf("bmp=%d imap=%d iblk = %d\n", mntable->bmap, mntable->imap, mntable->iblk);

  	MINODE * devRoot = kcwiget(ldev, 2);
 	devRoot->mounted = 1;
	devRoot->mptr = mntable;
	mntable->mntDirPtr = devRoot;

	return 0;



	// if (mntable==NULL){
	// 	printf("Next Mount table empty\n");
	// }else{
	// 	printf("Found another Mounting point.\n");
	// 	printf("Mount Table Content\n");
	// 	printf("dev=%d\n", mntable->dev);
	// 	printf("ninodes=%d\n", mntable->ninodes);
	// 	printf("nblocks=%d\n", mntable->nblocks);
	// 	printf("bmap=%d\n", mntable->bmap);
	// 	printf("iblk=%d\n", mntable->iblk);
	// 	printf("nblocks=%d\n", mntable->nblocks);

	// }





}