#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define MAX_PASSENGERS 10
#define MAX_CARGO 100
#define CREW_WEIGHT 75
#define MAX_CREW 7
#define MAX_CREW_CARGO 2


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

int main(){
    mssg plane;
    int passengers,cargo;
    int luggage_weight=0,body_weight=0,cargo_weight=0;

    printf("Enter the plane ID: \n");
    scanf("%d",&plane.id);
    printf("Enter the Type of Plane: \n");
    scanf("%d",&plane.type);

    if(plane.type == 1){
        
        printf("Enter Number of Occupied Seats: \n");
        scanf("%d",&passengers);    
        int pid_arr[passengers];
        int (*fd)[2] = malloc(passengers * sizeof(*fd));
        //error handle malloc

        if (fd == NULL) {
            fprintf(stderr, "Failed to allocate memory for fd\n");
            exit(1);
        }

        for(int i=0;i<passengers;i++){
            if(pipe(fd[i])<0){
                perror("pipe");
                exit(1);
            }
            
            int pid = fork();

            if(pid<0){
                perror("fork Error");
                exit(1);
            }

            if(pid==0){
                close(fd[i][0]);
                int pssg_lug_wt;
                int pssg_body_wt;
                printf("Enter Weight of Your Luggage , passenger %d: \n",i+1);
                scanf("%d",&pssg_lug_wt);

                if (write(fd[i][1],&pssg_lug_wt,sizeof(int)) == -1){
                    perror("Error in writing luggage weight");
                    return 1;
                }

                printf("Enter Your Body Weight , passenger %d: \n",i+1);
                scanf("%d",&pssg_body_wt);
                
                if (write(fd[i][1],&pssg_body_wt,sizeof(int)) == -1){
                    perror("Error in writing body weight");
                    return 1;
                }

                close(fd[i][1]);
                exit(0);
            }

            else{
                pid_arr[i] = pid;
                wait(NULL);
            }
        }

        for(int i=0;i<passengers;i++){
            close(fd[i][1]);
            int pssg_lug_wt;
            int pssg_body_wt;

            if (read(fd[i][0],&pssg_lug_wt,sizeof(int)) == -1){
                perror("Error in reading luggage weight");
                return 1;
            }

            if (read(fd[i][0],&pssg_body_wt,sizeof(int)) == -1){
                perror("Error in reading body weight");
                return 1;
            }

            luggage_weight += pssg_lug_wt;
            body_weight += pssg_body_wt;
            close(fd[i][0]);
            
            //printf("Passenger %d has weight - %d\n",i+1,pssg_lug_wt+pssg_body_wt);
            
        }

        for (int i=0;i<passengers;i++){
            
            //printf("Passenger %d has exited - %d\n",i+1,pid_arr[i]);

        }

        free(fd);

        printf("Total Luggage Weight: %d\n",luggage_weight);
        printf("Total Body Weight: %d\n",body_weight);
        printf("Total Weight: %d\n",body_weight+luggage_weight+CREW_WEIGHT*MAX_CREW);

        plane.passengers = passengers;
        plane.weight = body_weight + luggage_weight + CREW_WEIGHT*MAX_CREW;
        plane.mtype = 10+plane.id;
        plane.flag = 5;
    }

    else if(plane.type == 0){

        printf("Enter Number of Cargo Items: \n");
        scanf("%d",&cargo);

        printf("Enter Average Weight of Cargo Items: \n");
        scanf("%d",&cargo_weight);

        printf("Total Weight: %d\n",cargo*cargo_weight + CREW_WEIGHT*MAX_CREW_CARGO);
        plane.passengers = 0;
        plane.weight = cargo*cargo_weight + CREW_WEIGHT*MAX_CREW_CARGO;
        plane.mtype = 10+plane.id;
        plane.flag = 5;
        
    }




        printf("Enter Airport Number for Departure: \n");
        scanf("%d",&plane.departure);

        printf("Enter Airport Number for Arrival: \n");
        scanf("%d",&plane.arrival);




        //message queue for communication between plane and air traffic control
        key_t key = ftok("plane.c",'A');
        if (key == -1){
            perror("Error in ftok of Plane");
            return 1;
        }

        int msgid = msgget(key,IPC_CREAT | 0666);
        if (msgid == -1){
            perror("Error in creating message queue of Plane");
            return 1;
        }

        if(msgsnd(msgid,&plane,sizeof(mssg) - sizeof(long),0) == -1){
            perror("Error in sending message from Plane");
            return 1;
        }

        //wait till the plane lands
        mssg arrival_message_plane;

        if(msgrcv(msgid, &arrival_message_plane, sizeof(mssg) - sizeof(long), 40+plane.id, 0) == -1){
            perror("Error in receiving message from Plane");
            return 1;
        }

        if (arrival_message_plane.flag == 1){
            printf("Plane %d has been rejected by ATC\n due to cleanup request",plane.id);
            return 0;
        }
        printf("Plane %d has succesfully travelled from Airport %d to Airport %d !\n",plane.id,plane.departure,plane.arrival);
        return 0;
}

