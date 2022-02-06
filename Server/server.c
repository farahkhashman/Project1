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
  (void)s;
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
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, char *argv[]) {

    int port;

    // ensures that inputs are provided
    if (argc >= 2) {
        // saves inputed port number to variable
	    port = atoi(argv[1]);

        // validates port number to be between 41000 and 41999
        if(port >= 41000 && port <= 41999)
            printf("port is: %d\n", port);
        else {
            perror("Port Input");
            fprintf(stderr, "Please enter a valid port number between 41000 and 41999.\n");
            return 0;
        }
	}
	else {
        perror("Port Input");
		fprintf(stderr, "Please enter a port number.\n");
		return 0;
	}


    // listen on sock_fd, new connections on new_fd
    int sock_fd, new_fd;
    // struct to specify settings for server information
    struct addrinfo hints, *servinfo, *p;
    // connector's address information
    struct sockaddr_storage their_addr;
    //struct hostent *server;
    socklen_t sin_size;
    // structure to allow us to specify how to handle signals
    struct sigaction sa;
    int yes=1;
    // storage for an IPv6 address
    char s[INET6_ADDRSTRLEN];
    // variable for received value
    int rv;

    // initialises the pointer and sets all allocated spacees to zero.
    memset(&hints, 0, sizeof hints);
    // sets it to accepts both IPv4 or IPv6
    hints.ai_family = AF_UNSPEC;
    // sets sock of streaming type (i.e TCP)
    hints.ai_socktype = SOCK_STREAM;
    // use my IP
    hints.ai_flags = AI_PASSIVE;

    /*
    sock_fd = socket(AF_INET, SOCK_STREAM,0); //PF for socket/port in protocol
    struct sockaddr_in addrPort;
    addrPort.sin_family = AF_INET;
    // sets the ip address for the server
    addrPort.sin_addr.s_addr = inet_addr("129.74.152.125");
    // sets the inputted port number for the server
    addrPort.sin_port = htons(port);
    */
  if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }
  //loop through all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next){
    if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
      perror("server: socket");
      continue;
    }
    if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
      perror("setsocketopt");
      exit(1);
    }
    //bind the socket
    //error message if fails
    if(bind(sock_fd, p->ai_addr, p->ai_addrlen) == -1){
      close(sock_fd);
      printf("socket: Bind failed to port %d\n",port);
      perror("server: bind");
      continue;
    }
    break;
  }
  freeaddrinfo(servinfo); //all dont with this structure

  if (p==NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }
  if(listen(sock_fd, BACKLOG) == -1){
    printf("Listen Failed\n");
    perror("listen");
    exit(1);
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
        // accepts incoming connection
        new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &sin_size);
        // logs accepting error
        if(new_fd == -1) {
            perror("accept");
            continue;
        }

        // takes address and converts to string to be printed
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);

        char buf[MAXDATASIZE];
        int numbytes;
        int numbytes1;

        // receives the 16-bit binary value of the length of the filename
        if((numbytes1 = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }

        // adds null terminator
        buf[numbytes1] = '\0';
        // converts the 16-bit binary value from network to short form
        int network_byte_order = ntohs(atoi(buf));

            // printf("server: received '%s'\n", buf);
            // printf("server: received '%d'\n", numbytes1);
        printf("server: received '%d'\n", network_byte_order);

        // resets the buffer to zero to ensure no character leaks from prior initialisation
        memset(buf, 0, strlen(buf));

        // receives the file name
        if ((numbytes = recv(new_fd, buf, network_byte_order, 0)) == -1) {
            perror("recv");
            exit(1);
        }

        buf[numbytes] = '\0';

        printf("server: received '%s'\n",buf);


        FILE *file;
        file = fopen(buf, "r");

        // attempts to open the given file
        if(file) {
             // seek to end of file
            fseek(file, 0, SEEK_END);
            // get current file pointer
            int size = ftell(file);
                // printf("server: file size: %d\n", size);
            // seek back to beginning of file
            fseek(file, 0, SEEK_SET);

            // converts size to 32-bit binary
            int byteorder = htonl(size);
                // printf("server: file length in binary: %d\n", byteorder);

            // create a buffer to store file length as a string
            int buflen = 2048;
            char file_length[buflen];
            memset(file_length, 0, buflen);
            sprintf(file_length,"%d", byteorder);
                // printf("server: file length in binary string: %s\n", file_length);

            // sends file length to client
            int bytes_sent;
            if((bytes_sent = send(new_fd, file_length, strlen(file_length), 0))==-1) {
                perror("send");
                exit(1);
            }

            printf("server: bytes sent of file: %d %s\n", bytes_sent, file_length);

            sleep(0.1);

            char c = fgetc(file);
            int i = 0;
            int send_bytes;
            char each_character[2048];
            memset(each_character, 0, strlen(each_character));
                // printf("character is %c\n", c);
                // printf("size is %d\n", size);

            int count = 0;
            // iterates and sends from file
            while((c != EOF) && (i < size)) {
                each_character[count] = c;
                printf("buffer %s\n", each_character);
                count++;
                c = fgetc(file);

                // if the size is small enough, we can send the file in chunks of 2048 instead of character by character
                if(c != EOF  && size > 30 && count < 2048) {
                    continue;
                }

                // sends the chunk
                if((send_bytes = send(new_fd, each_character, count, 0)) == -1) {
                    perror("send");
                    exit(1);
                }
                    // printf("sent %d bytes of buffer %s of character %c\n", send_bytes, each_character, c);
                memset(each_character, 0, strlen(each_character));
                count = 0;
                i++;
                sleep(1);
            }
            fclose(file);
        }
        // if file not found in directory, returns error
        else {
            perror("file");
            exit(1);
        }

        // return to listening
        if(listen(sock_fd,10)){
            printf("Listen Failed\n");
            perror("listen");
            exit(1);
        }
        else {
            printf("Listening...\n");
        }

        //TODO: socket timeout?

        close(new_fd);
        exit(0);
    }

    return 0;

}
