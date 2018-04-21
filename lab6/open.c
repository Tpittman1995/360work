extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern PROC *running;
extern int dev;
extern int bmap, imap;



int open_file(char* pathName, int mode){
	if (pathName[0]==0){
		printf("[ERROR] usage error: open <pathname> <mode>\n");
		return 0;
	}
	
	printf("open()\npathname=%s mode=%d\n", pathname, mode);
	
}