/*

Author : Sourabh S Yelluru
ID : 2018B3A70815P
Network Programming Assignment 2
Date : 01/03/2022

*/
#include <stdio.h> 
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <time.h>
#define MSGQ_PATH "./voting.c"

struct sigaction siga;
int qid;

struct my_msgbuf{
    long mtype;
    int vote;
};

//Custom andler for signal
void multi_handler(int sig, siginfo_t *siginfo, void *context) {
    //get pid of sender
    pid_t sender_pid = siginfo->si_pid;
    
    struct my_msgbuf buf;
    //signal from parent
    if(getppid()==sender_pid){
        //Signal received from parent
        srand(time(0));
        int randno = rand()%2; //since votecan be either 0 or 1
        //Voting
        printf("Child Voted %d\n",randno);
        buf.vote = randno;
        msgsnd(qid,&buf,sizeof(int),0);
    }
}

int main(int argc, char** argv){
    struct msqid_ds ctl_buf;
    key_t key = ftok(MSGQ_PATH, 'Y');
    int queue_id = msgget(key, IPC_CREAT|IPC_EXCL|0600),ret; 
    struct my_msgbuf buf;
    qid = queue_id;
    if (queue_id == -1) {
        perror("msgget:");
        exit(1);
    }

    //Setting up the buffer and queue ID

    ret = msgctl (qid, IPC_STAT, &ctl_buf);
    ctl_buf.msg_qbytes = 4096;
    ret = msgctl (qid, IPC_SET, &ctl_buf);

    //Registering handler for user defined signal 1

    siga.sa_sigaction = *multi_handler;
    siga.sa_flags |= SA_SIGINFO;
    sigaction(SIGUSR1, &siga, NULL);

    int N = atoi(argv[1]);
    int status;
    int* pids = (int*)malloc(N*sizeof(int));
    //Create N children
    printf("Creating %d children...\n",N);
    for(int i=0;i<N;i++){
        int pid = fork();
        if(pid==0){
            //child keeps waiting for parent's signal, when received handler makes the child vote.
            while(1){
                
            }
            printf("Exited %d\n",i);
            exit(0);
        }
        else  pids[i]=pid;  //Parent registers its children's pid, in order to send signal in the future
    }
    int i=0;
    while(1){
        //Sending signal to children , asking them to vote
        kill(pids[i],10);
        i++;
        i = i%N;
        //i keeps updating even after the entire array is filled once, so restart it from the beginning
        
        if(!i){
            //If i is 0, then signal has been sent to all the children
            //Wait for all children to finish executing their signal handler.
            sleep(1);
            int arr[N],cnt=0;
            printf("Parent reading :\n");
            for(int j=0;j<N;j++){
                int rcverror = msgrcv(qid,&buf, sizeof(int), 0, 0);
                if(rcverror == -1){
                    //ERROR while reading
                    perror("msgrcv:");
                }
                //Storing the vote
                arr[j]=(buf.vote);
                printf("Child %d voted -> %d\n",j+1,arr[j]);
                //Counting of vote
                cnt += arr[j];
            }
            printf("\nVotes : ");
            for(int j=0;j<N;j++){
                printf("%d ",arr[j]);
            }

            printf("\nRESULT : ");
            if(cnt>N/2) printf("Accepted!\n");
            else{
                if(N%2){
                    if(cnt<=N/2) printf("Rejected!\n");
                }
                else{
                    if(cnt==N/2) printf("Tie!\n");
                    else printf("Rejected!\n");
                }
            }
        }
        sleep(1);
    }
    //wait for children to exit before exiting
    wait(&status);
}


