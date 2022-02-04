#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BACKLOG 10
#define MAXDATASIZE 100

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
  return 0;
}


void clearbuf(char *buf, int len)
{
    int i = 0;
    while(i<len) {
        buf[i] = 0;
        i++;
    }
}

int main(int argc, char *argv[]) {

  int port;

    // takes in the port number as an input
  if (argc >= 2) {
    port = atoi(argv[1]);
    printf("port is: %d\n", port);
	}
	else {
    perror("Port Input");
    fprintf(stderr, "Please enter a port number.\n");
		return 0;
	}

  // listen on sock_fd, new connections on new_fd
  int sock_fd, new_fd;
  // struct to specify settings for server information
  //struct addrinfo hints, *servinfo, *p;
  struct addrinfo hints;
  // connector's address information
  struct sockaddr_storage their_addr;
  //struct hostent *server;
  socklen_t sin_size;
  // structure to allow us to specify how to handle signals
  struct sigaction sa;
  //int yes=1;
  // storage for an IPv6 address
  char s[INET6_ADDRSTRLEN];
  // variable for received value
  //int rv;

  // initialises the pointer and sets all allocated spacees to zero.
  memset(&hints, 0, sizeof hints);
  // sets it to accepts both IPv4 or IPv6
  hints.ai_family = AF_UNSPEC;
  // sets sock of streaming type (i.e TCP)
  hints.ai_socktype = SOCK_STREAM;
  // use my IP
  hints.ai_flags = AI_PASSIVE;


  sock_fd = socket(PF_INET, SOCK_STREAM,0); //PF for socket/port in protocol
  struct sockaddr_in addrPort;
  addrPort.sin_family = AF_INET; //sets IPv4
  addrPort.sin_addr.s_addr = inet_addr("129.74.152.73");
  addrPort.sin_port = htons(port);

  //bind the socket
  //error message if fails
  if(bind(sock_fd, (struct sockaddr *) &addrPort, sizeof(addrPort)) == -1){
      printf("socket: Bind failed to port %d\n",port);
      return 0;
  }
  else printf("socket: Bind success to port %d\n",port);

  if(listen(sock_fd,10)){
    printf("Listen Failed\n");
  }
  else {
    printf("Listening...\n");
  }

  sa.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("server: waiting for connection...\n");

  while(1) {
    sin_size = sizeof their_addr;
    new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &sin_size);
    // logs accepting error
    if(new_fd == -1) {
      printf("error");
      perror("accept");
      return 0;
    }

    // takes address and converts to string to be printed
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    printf("server: got connection from %s\n", s);
    // if (send(new_fd, "Hello, world!", 13, 0) == -1) {
    //     perror("didnt send");
    //     exit(-1);
    // }

    char buf[MAXDATASIZE];
    int numbytes;

    if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
      perror("recv");
      exit(1);
    }

    buf[numbytes] = '\0';
    // convert and send length of filename
    int network_byte_order = ntohs(atoi(buf));

    printf("server: received in buf '%s'\n",buf);
    printf("server: received '%d'\n", network_byte_order);

    clearbuf(buf, strlen(buf));

    //int buflen = 2048;
    //char *buf1 = malloc(buflen * sizeof(char));
    //memset(buf1, 0, buflen);

    //printf("server: length of buf1 '%d'\n",strlen(buf1));
    printf("server: length of buf %zu \n\n",strlen(buf));

    int numbytes1;
    buf[0] = 'h';
    buf[1] = '\0';
    printf("server: length of buf %zu \n\n",strlen(buf));
    printf("server: received in buf '%s'\n",buf);
    if ((numbytes1 = read(new_fd, buf, network_byte_order)) == -1) {
      perror("recv");
      exit(1);
    }
    buf[network_byte_order] = '\0';
    printf("server: read in file '%s'\n", buf);
    buf[network_byte_order] = '\0';
    //buf[network_byte_order+1] = 'h';
    //buf[network_byte_order+2] = '\0';


    printf("server: received number of bytes nbo '%d'\n",network_byte_order);
    printf("server: received number of bytes nb '%d'\n",numbytes1);
    printf("server: received in buf '%s'\n",buf);
    printf("server: length of buf '%zu'\n",strlen(buf));
    //printf("server: length of buf1 '%d'\n",strlen(buf1));


    close(new_fd);
    exit(0);
  }

  return 0;
}
