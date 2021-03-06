#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>

#define MAXDATASIZE 100

void write_file(int sockfd, char * fn, int file_length) {
    char file_content[2048];
    int n;
    FILE *fp;
    fp = fopen(fn, "w");
    if(fp==NULL) {
        perror("client: Error in creating file");
        exit(1);
    }

    while(1) {
        sleep(0.5);
        n = recv(sockfd, file_content, 2048, 0);
        printf("client: received %d bytes: %s\n", n, file_content);
        if(n<=0) {
            usleep(100000);
            if((n=recv(sockfd, file_content, 2048, 0))<=0) {
                return;
            }
            else {
                continue;
            }
        }
        fwrite(file_content, n, 1, fp);
        memset(file_content, 0, strlen(file_content));
    }
    
    fclose(fp);
    return;
    
}

void *get_in_addr(struct sockaddr *sa) {
    if(sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
   
    int sockfd, port, rv;
    // struct for server information
    struct addrinfo hints, *servinfo, *p;
    char s[INET6_ADDRSTRLEN];

    // timing variables
    time_t start, end;
    double total_seconds, mbps;

  
    // start time
    time(&start);
    // initialises the pointer and sets all allocated spacees to zero.
    memset(&hints, 0, sizeof hints);
    // sets it to accepts both IPv4 or IPv6
    hints.ai_family = AF_UNSPEC;
    // sets sock of streaming type (i.e TCP)
    hints.ai_socktype = SOCK_STREAM;

    
    if (argc < 4) {
        fprintf(stderr,"usage %s Server IP Address, Server Port, File Name for Transfer\n", argv[0]);
        perror("Input Arguments");
        exit(1);
    }

    // converts the inputed port number to integer value
    port = atoi(argv[2]);

    // returns linked list of one or more addrinfo structures, where each one contains an Internet address that can be connected to
    if((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results in the provided linked list and binds to the first we can
    for(p = servinfo; p != NULL; p=p->ai_next) {
        // sockfd is the socket result file handler
        // attempt to create socket and continue if successful
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        // clients only need to connect to the socket
        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
            continue;
        }
        break;
    }

    if(p==NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    // converts address to string
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("Client: connecting to %s\n", s);

    // frees the allocated memory to servinfo
    freeaddrinfo(servinfo);

    // convert and send length of filename
    int len, bytes_sent;
    char *file_name = argv[3];
    len = strlen(file_name);

    char buf[MAXDATASIZE];
    int network_byte_order;
    network_byte_order = htons(strlen(file_name));
    sprintf(buf,"%d", network_byte_order);
    int lr;
    lr = send(sockfd, buf, strlen(buf), 0);

    sleep(1);

    // send the inputted file name
    bytes_sent = send(sockfd, file_name, strlen(file_name), 0);

    memset(buf, 0, strlen(buf));

    int numbytes;

    // receives the length of the file
    if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    // adds null terminator
    buf[numbytes] = '\0';
    // converts the 32-bit binary value
    int byte_order = htonl(atoi(buf));
    printf("client: received '%d'\n", byte_order);

    

    write_file(sockfd, file_name, byte_order);
    
    // end timing
    time(&end);

    // convert bytes to bits then to megabits then divide by secondsr
    total_seconds = (double) (end-start); // time in seconds
    printf("%lf\n", total_seconds);
    mbps = (double) byte_order;
    mbps = mbps * 8 / 1000000;
    mbps = mbps / total_seconds;
    printf("%lf Mb/s\n", mbps);

    // closes connection
    close(sockfd);

    return 0;

}