extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
extern PROC *running;

int my_mkdir(char* path){
	printf("Hello World: %s\n", path);
	tokenize(path);
	printf("tokenize: %s %d\n", name[1], n);
}