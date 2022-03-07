/*

Author : Sourabh S Yelluru
ID : 2018B3A70815P
Network Programming Assignment 3
Date : 07/03/2022

*/

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

/* Helper function to convert integer to string */
void itoa(int num, char** str){
    char* reversed = calloc(10,sizeof(char));
    int ind = 0,pc= num;
    while(pc>0){
        reversed[ind] = ('0'+pc%10);
        pc/=10;
        ind++;
    }
    reversed[ind]='\0';
    *str = calloc(ind+1,sizeof(int));
    for(int j=0;j<ind+1;j++)
        (*str)[j] = reversed[ind-j-1];
    str[ind]='\0';
}


int main(int argc, char* argv[]){
    /* 
        Incorrect usage handling 
        Usage : ./outfile_name google.com 80
                        OR
                ./outfile_name google.com 
    */
    if(argc < 2){
        fprintf(stderr,"Usage: %s <Server IP Address> <Server Port>\n\t\t\tOR\n       %s <Server IP address>\n",argv[0],argv[0]);
        exit(1);
    }
    
    //No of iterations
    int N = 100;
    //Input from user
    char* servIP = argv[1];
    int port = ((argc==3) ? atoi(argv[2]) : 80);
    printf("input hostname -> %s\n", argv[1]);

    /* Displays the IP and port */
    struct hostent* he;
    he = gethostbyname(servIP);
    char * IP = calloc(19,sizeof(char));
    strcpy(IP,inet_ntoa(*(struct in_addr*)he->h_addr));
    printf("%s -> %s:%d\n",servIP,IP,port);
    
    struct addrinfo hints, *results;
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;
    hints.ai_flags    = AI_CANONNAME;
    hints.ai_socktype = SOCK_STREAM;

    //get port number as a string
    char* portstring;
    itoa(port,&portstring);

    if (getaddrinfo(argv[1], portstring, &hints, &results) != 0)
        perror("getaddrinfo() error");
    
    //Buffers for sending and receiving
    char* recv_buf = calloc(BUFFER_SIZE,sizeof *recv_buf);
    char* header = calloc(BUFFER_SIZE,sizeof *header);
    snprintf(header, BUFFER_SIZE,
             "GET / HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Accept: text/html\r\n"
             "\r\n",
             argv[1]);


    
    int sock;

    //Registering start time
    double time_spent = 0.0, avg_time_spent=0.0;
    clock_t begin = clock();


    for(int i=0;i<N;i++){
            
        struct addrinfo *t;
        for(t = results; t!=NULL;t->ai_next){
            if((sock=socket(t->ai_family,t->ai_socktype,t->ai_protocol))<0)
                continue;
            if(connect(sock,t->ai_addr, t->ai_addrlen)==0)
                break;
        }
        if(t==NULL){
            perror("Connection Error");
            continue;
        }
        printf("\nConnection established\n");

        // send
        if (send(sock, header, BUFFER_SIZE, 0) == -1) {
            perror("Send Failed");
            continue;
        }
        // receive
        ssize_t recv_bytes;
        if ((recv_bytes = recv(sock, recv_buf, BUFFER_SIZE, 0)) <= 0) {
            printf("Receive Failed\n");
        }
        else {
            printf("\t\t\t\t\t=========<RESPONSE%d>=========\n", i+1);
            recv_buf[recv_bytes - 1] = '\0'; // truncate response
            printf("%s\n\n", recv_buf);
        }
    }
    //Registering end time
    clock_t end = clock();

    //Calculation of avg time spent per output
    time_spent += (double)(end-begin)/CLOCKS_PER_SEC;
    avg_time_spent = time_spent/N;
    printf("Total time spent = %f seconds\n",time_spent);
    printf("Average time spent = %f seconds\n",avg_time_spent);
}