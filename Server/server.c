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
    struct addrinfo hints, *servinfo, *p;
    // connector's address information
    struct sockaddr_storage their_addr;
    struct hostent *server;
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

    // checks for address information errors
    if((rv = getaddrinfo("129.74.152.125", argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results in the provided linked list and binds to the first we can
    for(p = servinfo; p != NULL; p=p->ai_next) {
        // unsuccessful 
        if((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        // sets socket options to allow for reusing the address
        if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        // binding 
        if(bind(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock_fd);
            perror("server: bind");
            continue;
        }
        break;
    }

    // clears the allocaated space for this structure 
    // freeaddrinfo(servinfo);

    if(p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if(listen(sock_fd,10)){
        printf("Listen Failed\n");
    }
    else {
        printf("Listening...\n");
        printf("server: waiting for connection...");
    } 

    // //listening for connections
    // if(listen(sock_fd, BACKLOG) == -1) {
    //     perror("listen");
    //     exit(1);
    // }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connection...");

    while(1) {
        printf("enters while loop?");
        sin_size = sizeof their_addr;
        new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &sin_size);
        // logs accepting error
        if(new_fd == -1) {
            perror("accept");
            continue;
        }

        // takes address and converts to string to be printed
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);
        if (send(new_fd, "Hello, world!", 13, 0) == -1) {
            perror("didnt send");
            exit(-1);
        }



        close(new_fd);
        exit(0);
    }

    return 0;

}