// This is the echo SERVER server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#define _POSIX_SOURCE
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#undef _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>

#define  MAX 256

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

int y;
int  mysock, client_sock;              // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;
char lsHelper[MAX];
char buff[MAX];                 // help variables
int SIZE;

// Server initialization code:

int server_init(char *name)
{
   printf("==================== server init ======================\n");   
   // get DOT name and IP address of this host

   printf("1 : get and show server host info\n");
   hp = gethostbyname(name);
   if (hp == 0){
      printf("unknown host\n");
      exit(1);
   }
   printf("    hostname=%s  IP=%s\n",
               hp->h_name,  inet_ntoa(*(long *)hp->h_addr));
  
   //  create a TCP socket by socket() syscall
   printf("2 : create a socket\n");
   mysock = socket(AF_INET, SOCK_STREAM, 0);
   if (mysock < 0){
      printf("socket call failed\n");
      exit(2);
   }

   printf("3 : fill server_addr with host IP and PORT# info\n");
   // initialize the server_addr structure
   server_addr.sin_family = AF_INET;                  // for TCP/IP
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address  
   server_addr.sin_port = 0;   // let kernel assign port

   printf("4 : bind socket to host info\n");
   // bind syscall: bind the socket to server_addr info
   r = bind(mysock,(struct sockaddr *)&server_addr, sizeof(server_addr));
   if (r < 0){
       printf("bind failed\n");
       exit(3);
   }

   printf("5 : find out Kernel assigned PORT# and show it\n");
   // find out socket port number (assigned by kernel)
   length = sizeof(name_addr);
   r = getsockname(mysock, (struct sockaddr *)&name_addr, &length);
   if (r < 0){
      printf("get socketname error\n");
      exit(4);
   }

   // show port number
   serverPort = ntohs(name_addr.sin_port);   // convert to host ushort
   printf("    Port=%d\n", serverPort);

   // listen at port with a max. queue of 5 (waiting clients) 
   printf("5 : server is listening ....\n");
   listen(mysock, 5);
   printf("===================== init done =======================\n");
}

void myLs(char* pathname){
  DIR *dir;
  struct dirent *entry;

  lsHelper[0]='\0';
  

 

  if ((dir = opendir(pathname)) == NULL)
    perror("opendir() error");
  else {
    while ((entry = readdir(dir)) != NULL){
      sprintf(lsHelper, "%s %s", lsHelper, entry->d_name);
      //printf("%s ", entry->d_name);
    }
  //printf("\n");
    closedir(dir);
  }
}

main(int argc, char *argv[])
{
  int temp=0,fd=0;
  char *s;
  char *tempArray[50];
   char *hostname;
   char line[MAX];
   char cwd[128];
   struct stat st;


   for(temp;temp<50;temp++) tempArray[temp] = malloc(150);

    temp=0;

   if (argc < 2)
      hostname = "localhost";
   else
      hostname = argv[1];
 
   server_init(hostname); 

   // Try to accept a client request
   while(1){
     printf("server: accepting new connection ....\n"); 

     // Try to accept a client connection as descriptor newsock
     length = sizeof(client_addr);
     client_sock = accept(mysock, (struct sockaddr *)&client_addr, &length);
     if (client_sock < 0){
        printf("server: accept error\n");
        exit(1);
     }
     printf("server: accepted a client connection from\n");
     printf("-----------------------------------------------\n");
     printf("        IP=%s  port=%d\n", inet_ntoa(client_addr.sin_addr.s_addr),
                                        ntohs(client_addr.sin_port));
     printf("-----------------------------------------------\n");

     // Processing loop: newsock <----> client
     while(1){
       n = read(client_sock, line, MAX);
       if (n==0){
           printf("server: client died, server loops\n");
           close(client_sock);
           break;
      }


      getcwd(cwd, 128);
      
      // show the line string
      printf("server: read  n=%d bytes; line=[%s]\n", n, line);

      temp=0;
      s=strtok(line," ");
      strcpy(tempArray[temp], s);
      temp++;


      while(s=strtok(0," ")){ 
        strcpy(tempArray[temp], s);
        temp++;
      }

      

     printf("%s\n", tempArray[1]);
   if (!strcmp("ls", tempArray[0])) {
    if(strcmp(tempArray[1],"\0")){
      myLs(tempArray[1]);
      //printf("%s\n", lsHelper);
    }else{
      getcwd(cwd, 128);
      myLs(cwd);
      //printf("%s\n", lsHelper);
    }

    strcpy(line, lsHelper);

   }

   if (!strcmp("pwd", tempArray[0])) {
    getcwd(cwd, 128);
    printf("%s\n", cwd);
    sprintf(line,"%s", cwd);
    }

   if (!strcmp("cd", tempArray[0])) {
      chdir(tempArray[1]);
    }

   if (!strcmp("rm", tempArray[0])) {
      unlink(tempArray[1]);
    }

   if (!strcmp("rmdir", tempArray[0])) {
      rmdir(tempArray[1]);
    }
   if (!strcmp("rm", tempArray[0])) {
      unlink(tempArray[1]);
    }

   if (!strcmp("mkdir", tempArray[0])) 
    mkdir(tempArray[1], 0755);

   if(!strcmp("ls", tempArray[0])){
    myLs(tempArray[1]);
   }

   n = write(client_sock, line, MAX);

   if(!strcmp("get", tempArray[0])){
    printf("got get command.\n");
    strcpy(line,"got get command");
    stat(tempArray[1],&st);
    SIZE=st.st_size;
    sprintf(line, "%d", SIZE);
    printf("%s\n", line);
    n=write(client_sock, line, MAX);
    fd = open(tempArray[1],O_RDONLY);
    while(y=read(fd, buff, MAX)){
      n=write(client_sock, buff,MAX);
    }
    close(fd);


   }


    

      



     



      // send the echo line to client 

      

      temp = 0;
      for(temp;temp<50;temp++) tempArray[temp][0] = '\0';

      temp=0;


      
      printf("server: ready for next request\n");
    }
 }
}

