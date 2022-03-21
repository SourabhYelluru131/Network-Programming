#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#define MSGQ_PATH "./tcp_server_N_clients.c"
#define BUFSIZE 4096
int qid;

struct my_msgbuf{
    long mtype;
    int delta;
};

int main(int argc, char** argv){
    int sockfd,childfd,N,optval,portno,num_conn_clients=0;
    struct sockaddr_in serveraddr; /* server's addr */
    struct sockaddr_in clientaddr; /* client addr */
    struct hostent* hostp;
    char * hostaddrp;
    char buf[BUFSIZE];
    struct msqid_ds ctl_buf;
    if(argc<2){
        printf("Usage : ./%s <port_number> <number>",argv[0]);
    }


    //Creating a message queue
    key_t key = ftok(MSGQ_PATH, 'Y');
    int queue_id = msgget(key, IPC_CREAT|0660),ret; 
    if (queue_id == -1) {
        perror("msgget:");
        exit(1);
    }
    qid = queue_id;
    portno = atoi(argv[1]);
    N = atoi(argv[2]);
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0){
        perror("Error opening socket");
    }
    
    //Setting up the buffer and queue ID

    ret = msgctl (qid, IPC_STAT, &ctl_buf);
    ctl_buf.msg_qbytes = 4096;
    ret = msgctl (qid, IPC_SET, &ctl_buf);

    optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
            (const void *)&optval , sizeof(int));

    //Clearing gabage data
    bzero((char *) &serveraddr, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);

    //Bind the server to port
    if (bind(sockfd, (struct sockaddr *) &serveraddr, 
        sizeof(serveraddr)) < 0) 
        perror("error on binding");

    //Listen 
    if(listen(sockfd,N)<0)
        perror("error on listen");
    while(1){
        while(num_conn_clients>=N){
            struct my_msgbuf msgbuf;
            int n = msgrcv(qid, &msgbuf, sizeof(int),0,0);
            if(n==-1 && errno==ENOMSG) continue;
            else{
                if(n==-1){
                    perror("Error reading from queue");
                    exit(-1);
                }
            }
            num_conn_clients += msgbuf.delta;
        }
        if(num_conn_clients<N){
            while(1){
                struct my_msgbuf msgbuf;
                int n = msgrcv(qid, &msgbuf, sizeof(int),0,IPC_NOWAIT);
                
                if(n==-1 && errno==ENOMSG)
                    break;
                else{
                    if(n==-1){
                        perror("Error reading from queue");
                        exit(-1);
                    }
                }
                num_conn_clients += msgbuf.delta;
            }
        }
        int client_len = sizeof(clientaddr);
        childfd = accept(sockfd, (struct sockaddr *)&clientaddr, &client_len);
        if(childfd<0)
            perror("Error on accept");
        
        int pid,status;
        if(num_conn_clients<N){
            num_conn_clients++;
            printf("Conneced to client!\n");
            pid = fork();
            if(pid<0)
                perror("Error creating a child process");
            if(pid==0){
                struct my_msgbuf exit_buff;
                exit_buff.delta = -1;
                exit_buff.mtype = 1;    //Weird
                close(sockfd);
                while(1){

                    //Check if client has terminated
                    int n = recv(childfd,buf,BUFSIZE,0);
                    if(n==0){
                        printf("Client has terminated\n");
                        int tt = msgsnd(qid, &exit_buff, sizeof(int),0);
                        if (tt == -1)
                            perror("msgop: msgsnd failed");
                        exit(0);
                    }
                }
            }
        }
        else{
            //Close connection
           close(childfd); 
        }
    }
}