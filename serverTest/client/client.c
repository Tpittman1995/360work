// The echo client client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX 256

// Define variables
struct hostent *hp;              
struct sockaddr_in  server_addr; 

int server_sock, r;
int SERVER_IP, SERVER_PORT; 
char * stringList[8] = {"lcat", "lls", "lcd", "lpwd", "lmkdir", "lrmdir", "lrm", 0};
char cwd[1024];
// clinet initialization code

int client_init(char *argv[])
{
  printf("======= clinet init ==========\n");

  printf("1 : get server info\n");
  hp = gethostbyname(argv[1]);
  if (hp==0){
     printf("unknown host %s\n", argv[1]);
     exit(1);
  }

  SERVER_IP   = *(long *)hp->h_addr;
  SERVER_PORT = atoi(argv[2]);

  printf("2 : create a TCP socket\n");
  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock<0){
     printf("socket call failed\n");
     exit(2);
  }

  printf("3 : fill server_addr with server's IP and PORT#\n");
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = SERVER_IP;
  server_addr.sin_port = htons(SERVER_PORT);

  // Connect to server
  printf("4 : connecting to server ....\n");
  r = connect(server_sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
  if (r < 0){
     printf("connect failed\n");
     exit(1);
  }

  printf("5 : connected OK to \007\n"); 
  printf("---------------------------------------------------------\n");
  printf("hostname=%s  IP=%s  PORT=%d\n", 
          hp->h_name, inet_ntoa(SERVER_IP), SERVER_PORT);
  printf("---------------------------------------------------------\n");

  printf("========= init done ==========\n");
}

int findIndex(char line[MAX])
{
	int i = 0;
	for(i = 0; i < 7; i++)
	{
		if(!strcmp(line, stringList[i]))
		{
			return i;
		}

	}

	return -1;
}



void lcat(char command[MAX], char pathname[MAX])
{
	int fd;
	int n = 0;
	char buffer[1024];
	int l = 0;

if(fd = open(pathname, O_RDONLY))
{

    while (n = read(fd, buffer, 1024))
    {
        for(l = 0; l < n; l++)
        {
            printf("%c",(buffer[l]));
            if(buffer[l] == '\n')
            {
      	      printf("\n");
            }
        }
	}
}


}
   			

void lls(char command[MAX], char pathname[MAX])
{
	
}


void lcd(char command[MAX], char pathname[MAX])
{
	chdir(pathname);
	/*
	getcwd(cwd, 1024);
	if(pathname[0] == '/')
	{
		if(chdir(pathname){
			printf("cd failed");
		}
		return;
	}
	//printf("under construction");
	strcat(cwd, pathname)
	if(chdir(cwd))
	{
		printf("cd failed");

	}
	*/
}

void lpwd(char command[MAX], char pathname[MAX])
{
	getcwd(cwd, 1024);
	printf("%s \n", cwd);
}

void lmkdir(char command[MAX], char pathname[MAX])
{
	r = mkdir(pathname, 0755);
  //  r = mkdir(entry[2].value, 0755);

}

void lrmdir(char command[MAX], char pathname[MAX])
{
	 r = rmdir(pathname);
   //  r = rmdir(entry[2].value);

}

void lrm(char command[MAX], char pathname[MAX])
{
	r = unlink(pathname);
   // r = unlink(entry[2].value);
}

main(int argc, char *argv[ ])
{
  int n;
  char command[MAX];
  char pathname[MAX];
  pathname[0] = '\0';
  command[0] = '\0';
  
  char line[MAX], ans[MAX];

  if (argc < 3){
     printf("Usage : client ServerName SeverPort\n");
     exit(1);
  }

  client_init(argv);
  // sock <---> server
  printf("********  processing loop  *********\n");
  while (1){
    printf("input a line : ");
    bzero(line, MAX);                // zero out line[ ]
    fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

    line[strlen(line)-1] = 0;        // kill \n at end
    if (line[0]==0)                  // exit if NULL line
       exit(0);


     printf("%s\n", line );
   	if(line[0] == 'l' && line[1] != 's')
   	{
   		sscanf(line, "%s %s", command, pathname);
   		n = findIndex(command);
   		switch(n)
   		{
   			case 0: //lcat
   			lcat(command, pathname);
   			break;

   			case 1: //lls
   			lls(command, pathname);

   			break;

   			case 2: // lcd
   			lcd(command, pathname);
   			break;

   			case 3: //lpwd
   			lpwd(command, pathname);
   			break;

   			case 4: //lmkdir
   			lmkdir(command, pathname);
   			break;

   			case 5: //lrmdir
   			lrmdir(command, pathname);
   			break;

   			case 6: // lrm
   			lrm(command, pathname);
   			break;

   			default:
   			printf("Invalid Command\n");
   			break;
   			

   		}
   	}
   	else
   	{
    // Send ENTIRE line to server
      printf("%s\n", line);
      printf("%s\n", ans);
      printf("stuff\n");
    n = 0;
    n = write(server_sock, line, MAX);
  //  printf("test");
    printf("%s\n", line);

    // Read a line from sock and show it
    n = read(server_sock, ans, MAX);
    printf("%s\n", ans);
	 }
  }
}


