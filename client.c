#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char const* argv[])
{
  /*
  if (argc != 4){
    fprintf(stderr, "client: Wrong number of arguments");
    exit(-1);
  }
  */
  //const char *ip_server = argv[1];
  int port = atoi(argv[1]);
  //const char *transfer_file = argv[3];

  int socketfd = socket(PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addrPort;

  addrPort.sin_family = AF_INET;
  addrPort.sin_addr.s_addr = htonl(INADDR_ANY);
  addrPort.sin_port = htons(port);

  if(connect(socketfd, (struct sockaddr *) &addrPort, sizeof(addrPort))){
    printf("Connect failed \n");
  }
  else printf("Connected via Port %d\n", port);

  char msg[50];
  int countR, countS;

  while(strcmp(msg, "Close\n")){
    printf("From server: ");
    countR = recv(socketfd, msg, 50, 0);
    printf("%s", msg);
    if(!strcmp(msg, "Close\n")){
      break;
    }
    printf("To server: ");
    fgets(msg, 50, stdin);
    countS = send(socketfd, msg, 50, 0);
  }
  close(socketfd);
  printf("Socket connection closed\n");

  return 0;
}
