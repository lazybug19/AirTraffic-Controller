#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>



typedef struct {
    long mtype;
    int id;
    int type;
    int passengers;
    int weight;
    int departure;
    int arrival;
    int flag;
} mssg;

//setting up the message queue



int main(){
    int msgid;
    key_t key=ftok("plane.c",'A');
    if (key == -1){
        perror("Error in ftok of Plane");
        return 1;
    }

    msgid = msgget(key,IPC_CREAT | 0666);
    if (msgid == -1){
        perror("Error in creating message queue of Plane");
        return 1;
    }



    while(1){
        char choice;
        printf("Do you want the Air Traffic Control System to terminate?(Y for Yes and N for No)\n");
        scanf(" %c",&choice);
        if (choice == 'Y' || choice == 'y'){
            mssg cleanup;
            cleanup.mtype = 1;
            cleanup.flag = 2;
            if(msgsnd(msgid,&cleanup,sizeof(mssg)-sizeof(long),0) == -1){
                perror("Error in sending message from Cleanup");
                return 1;
            }
            break;
            
        }

    }
}