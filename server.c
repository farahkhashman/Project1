#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char const* argv[])
{
  //check if correct number of arguments
  if (argc != 2){
    fprintf(stderr, "server: Wrong number of arguments");
    exit(-1);
  }
  //set port number
  int port = atoi(argv[1]);


  //create socket
  int socketfd = socket(PF_INET, SOCK_STREAM,0); //PF for socket/port in protocol
  struct sockaddr_in addrPort; //not sure why we do this
  addrPort.sin_family = AF_INET; //sets IPv4
  addrPort.sin_addr.s_addr = htonl(INADDR_ANY); //fills a default p address
  addrPort.sin_port = htons(port);

  //bind the socket
  //error message if fails
  if(bind(socketfd, (struct sockaddr *) &addrPort, sizeof(addrPort)) == -1){
    printf("socket: Bind failed to port %d\n",port);
  }
  else printf("socket: Bind success to port %d\n",port);

  if(listen(socketfd,10)){
    printf("Listen Failed\n");
  }
  else printf("Listening...\n");

  struct sockaddr_in client_addr;
  socklen_t len = sizeof(client_addr);

  int socketfd_incoming = accept(socketfd, (struct sockaddr *) &client_addr, &len);
  printf("Accepted a connection \n");

  char msg[50];
  int countR, countS;

  while(strcmp(msg, "Close\n")){
    printf("To Client:");
    fgets(msg, 50, stdin);
    countS = send(socketfd_incoming, msg, 50, 0);
    if(!strcmp(msg,"Close\n")) {
      break;
    }
    printf("From Client : ");
    countR = recv(socketfd_incoming, msg, 50, 0);
    printf("%s", msg);
  }
  close(socketfd);
  close(socketfd_incoming);
  printf("Socket connection closed\n");
  return 0;
}
