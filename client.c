#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>

#include <arpa/inet.h>

#define MAXDATASIZE 200 //max number of bytes we can get at once

/* cannot use yet
void request_and_write(int sockfd, char * fn) {
    int n, numbytes;
    FILE *fp;
    char *filename = fn;

    char buf[MAXDATASIZE];

    // function to receive a value and handles error if no bytes were received
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0) == -1)) {
        perror("recv");
        exit(1);
    }

    // adds a null terminator at the end of the received string
    buf[numbytes] = '\0';
    printf("client: received file length: '%s'\n", buf);
    //int file_length = atof(buf);
    int file_length = atoi(buf);
    bzero(buf, MAXDATASIZE);
    //printf("client: converted file length to float: '%f'\n", file_length);
    printf("client: converted file length to int: '%d'\n", file_length);

    char file_content[file_length];

    fp = fopen(filename, "w");
    if(fp==NULL) {
        perror("client: Error in creating file\n");
        exit(1);
    }

    while(1) {
        n = recv(sockfd, file_content, file_length, 0);
        if(n<=0) {
            break;
            return;
        }
        fprintf(fp, "%s\n", file_content);
        bzero(file_content, file_length);
    }

    // close(fp);
    return;

}
*/
//get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {

    //int sockfd, numbytes, port, rv;
    int sockfd, numbytes;
    // struct for server information
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc < 4) {
        fprintf(stderr,"usage %s Server IP Address, Server Port, File Name for Transfer\n", argv[0]);
        perror("Input Arguments\n");
        exit(1);
    }


    // initialises the pointer and sets all allocated spaces to zero.
    memset(&hints, 0, sizeof hints);
    // sets it to accepts both IPv4 or IPv6
    hints.ai_family = AF_UNSPEC;
    // sets sock of streaming type (i.e TCP)
    hints.ai_socktype = SOCK_STREAM;

    //reads in inputting port as string
    char *port = argv[2];

    // returns linked list of one or more addrinfo structures, where each one contains an Internet address that can be connected to
    if((rv = getaddrinfo(argv[1], port, &hints, &servinfo)) != 0) {
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
    printf("client_1: connecting to %s\n", s);

    // frees the allocated memory to servinfo
    freeaddrinfo(servinfo);
    char buf[MAXDATASIZE];

    //FIRST CLIENT RECIEVE
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
      perror("recv");
      exit(1);
    }
    buf[numbytes] = '\0';
    printf("client_2: received '%s'\n",buf);
    size_t network_byte_order;
    // convert and send length of filename
    network_byte_order = htons(strlen(argv[3]));
    sprintf(buf,"%ld", network_byte_order);

    //FIRST CLIENT SEND
    if (send(sockfd, buf, sizeof(buf) ,0) == -1){
      perror("send");
      exit(1);
    }

    //SECOND CLIENT RECIEVE
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
      perror("recv");
      exit(1);
    }

    buf[numbytes] = '\0';
    printf("client second recieve: received '%s'\n",buf);


    //SECOND CLIENT SEND
    int client_error = send(sockfd, buf, sizeof(buf) ,0);
    if (client_error == -1){
      perror("send");
      exit(1);
    }
    printf("second client send error %d\n", client_error);
    printf("second client send argv 3: %s\n", argv[3]);
    printf("second client send len of argv 3: %ld\n", strlen(argv[3]));

    printf("second client send buffer: file_name %s\n", buf);
    printf("second client send sizeof buffer: %ld\n", sizeof(buf));
    //close(new_fd);
    //exit(0);

    //int lr, fn;
    //lr = send(sockfd, buf, sizeof(buf), 0);
    //printf("client_2: %d\n",lr);


    //char *file_name = argv[3]+'\0';

    //THIRD CLIENT SEND
    char buf_title[MAXDATASIZE];
    char *file_name = argv[3];
    sprintf(buf_title,"%s", file_name);

    client_error = send(sockfd, buf_title, sizeof(buf_title) ,0);
    if (client_error == -1){
      perror("send");
      exit(1);
    }

    printf("client_error: %d\n", client_error);
    printf("client_3: %s\n", argv[3]);
    printf("client_4: %ld\n", strlen(argv[3]));

    printf("client_5: file_name %s\n", buf_title);
    printf("client_6: %ld\n", sizeof(buf_title));



    //fn = send(sockfd, file_name, strlen(file_name), 0);
    //printf("client_7: %d\n",fn);


    // // sends/requests inputted filename
    // if (fn = write(sockfd, argv[3], strlen(argv[3])) == -1) {
    //     perror("write");
    //     exit(1);
    // }

    // request_and_write(sockfd, argv[3]);

    //TODO: print the final exit of time and stuff

    // closes connection
    close(sockfd);

    return 0;

}
