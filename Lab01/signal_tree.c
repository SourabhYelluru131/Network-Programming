/*

Author : Sourabh S Yelluru
ID : 2018B3A70815P
Network Programming Assignment 1
Date : 15/02/2022

*/
#include<stdio.h>
#include<unistd.h>
#include <signal.h>
#include<stdlib.h>
#include<sys/wait.h>

int points,n,a,s;
int *child;
int sibling;
struct sigaction siga;

void multi_handler(int sig, siginfo_t *siginfo, void *context) {
    //get pid of sender
    pid_t sender_pid = siginfo->si_pid;

    //signal from parent
    if(getppid()==sender_pid){
        points+=a;
        printf("%d RECEIVED SIGNAL FROM PARENT %d. POINTS = %d\n", getpid(), getppid(), points);
    }
    //signal from child
    else if(child[0] == sender_pid || child[1] == sender_pid){
        points-=s;
        printf("%d RECEIVED SIGNAL FROM CHILD %d. POINTS = %d\n", getpid(), getppid(), points);
    }
    //signal from sibling
    else if(sibling == sender_pid){
        points-=s/2;
        printf("%d RECEIVED SIGNAL FROM SIBLING %d. POINTS = %d\n", getpid(), getppid(), points);
    }

    //exit if points hit zero
    if(points<=0){
        printf("%d EXITING\n", getpid());
        exit(0);
    }
}

int main(int argc, char** argv){
    //register signal handler
    siga.sa_sigaction = *multi_handler;
    siga.sa_flags |= SA_SIGINFO;
    sigaction(SIGUSR1, &siga, NULL);


    int mainpid = getpid();
    n = atoi(argv[1]),a = atoi(argv[2]),s =atoi(argv[3]);

    sibling = 0;
    int copy_n = n;
    points=copy_n;

    int rpid,lpid;

    while(n>0){
        n--;

        int left = n/2;
        lpid = 0;

        int right = n/2 + n%2;
        rpid = 0;

        //pipe used to send pid of right child to left child
        int p[2];
        pipe(p);

        if(left!=0){
            lpid = fork();
            if(lpid==0){
                read(p[0], &sibling, sizeof(int));
                n = left;   
                continue;
            }
        }

        if(right!=0){
            rpid = fork();
            if(rpid==0){
                //since left child is created before right, lpid of the parent stores the pid of the left child and used to update sibling pid of right child
                sibling = lpid;
                n = right;
                continue;
            }
        }
        write(p[1], &rpid, sizeof(int));

        child = malloc(2*sizeof(int));
        child[0] = lpid; child[1] = rpid;

        break;
    }
    
    n = copy_n;

    //send signals
    int status=0;
    pid_t start = getppid()-n; pid_t end = getpid()+n;
    for(pid_t i = start; i<=end+n; i++){
        kill(i, SIGUSR1);
        sleep(1);
    }
    //wait(&status);

}
// cd ../../mnt/c/Users/soura/Downloads/0_ISF462_lab1*/