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



// he air traffic controller first checks if the termination request was received earlier from the cleanup process. 
// If  yes,  then  it  rejects  the  plane's  departure  request,  informing  the  plane  that  no  further  departures  will 
// happen, else performs the step mentioned in 5.(c).
//Send departure request to arrival airport too

// he arrival airport, on receiving the intimation from the air traffic controller, assigns a runway for the plane’s 
// landing and waits for the plane’s arrival (the plane needs to complete its 30 seconds journey). The arrival 
// airport  handles  the  landing  and  the  deboarding/unloading  processes,  sends  a  message  to  the  air  traffic 
// controller that landing and deboarding/unloading completed successfully and displays the relevant message 
// on the screen


int main(){

    int n_airports=0;
    int active_planes=0;
    int terminate_flag = 1;

    printf("Enter the number of airports: \n");
    scanf("%d",&n_airports);

    //Create a message queue
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
    while (terminate_flag||active_planes){
        //printf("Active Planes %d\n",active_planes);
        //Mtype List
        // 11-20 - plane to ATC flag 5(plane details)
        // 1 - cleanup to ATC flag 2 (terminate_flag)
        // 21-30 - Airports to ATC (departure_board ack and arrival_deboard ack) 
        //         distinguished by flag 0 and 1 cleanup ack flag 3
        // 41-50 - ATC to plane (plane_arrival_deboard ack) flag 0
        // 41-50 - ATC to plane plane rejection flag 1
        // 31-40 - ATC to Airports (departure_board and arrival_deboard)
        //         distinguished by flag 0 and 1
        // 31-40 - ATC to Airports (terminate flag)
        //         distinguished by flag 2
        // 31-40 - ATC to Airports non termination request to arrival airports
        //         distinguished by flag 3

        mssg received_mssg;
        mssg sent_mssg;

        if(msgrcv(msgid,&received_mssg,sizeof(mssg)-sizeof(long),-30,0) == -1){
            perror("Error in receiving message from Plane");
            return 1;
        }

        //printf("MSSG_TYPE -%ld -- Flag %d -- Id %d\n",received_mssg.mtype,received_mssg.flag,received_mssg.id);

        if(received_mssg.flag==5 ){
            if(terminate_flag){
                active_planes++;
                sent_mssg.mtype = 30+received_mssg.departure;
                sent_mssg.flag = 0;

                sent_mssg.id = received_mssg.id;
                sent_mssg.type = received_mssg.type;
                sent_mssg.passengers = received_mssg.passengers;
                sent_mssg.weight = received_mssg.weight;
                sent_mssg.departure = received_mssg.departure;
                sent_mssg.arrival = received_mssg.arrival;

                if(msgsnd(msgid,&sent_mssg,sizeof(mssg)-sizeof(long),0) == -1){
                    perror("Error in sending message from ATC to Airport");
                    return 1;
                }

                sent_mssg.mtype = 30+received_mssg.arrival;
                sent_mssg.flag = 3;

                if(msgsnd(msgid,&sent_mssg,sizeof(mssg)-sizeof(long),0) == -1){
                    perror("Error in sending message from ATC to Airport");
                    return 1;
                }

                FILE *fp = fopen("AirTrafficController.txt","a");
                if(fp == NULL){
                    perror("Error in opening file");
                    return 1;
                }

                if(fprintf(fp,"Plane %d has departed from Airport %d and will land at Airport %d\n",received_mssg.id,received_mssg.departure,received_mssg.arrival) < 0){
                    perror("Error in writing to file");
                    return 1;
                }
                printf("Plane %d has departed from Airport %d and will land at Airport %d\n",received_mssg.id,received_mssg.departure,received_mssg.arrival);
                if(fclose(fp) == EOF){
                    perror("Error in closing file");
                    return 1;
                }
            }

            else{
                sent_mssg.mtype = 40+received_mssg.id;
                sent_mssg.flag = 1;

                if(msgsnd(msgid,&sent_mssg,sizeof(mssg)-sizeof(long),0) == -1){
                    perror("Error in sending message from ATC to Plane");
                    return 1;
                }

            }


        }

        else if(received_mssg.flag == 2 ){

            terminate_flag = 0;

        }

        else if(received_mssg.flag == 0 || received_mssg.flag == 1){

            if(received_mssg.flag == 0){

                //printf("DEPBOARDACK -- Pid: %d\n",received_mssg.id);
                sent_mssg.mtype = 30+received_mssg.arrival;
                sent_mssg.flag = 1;

                sent_mssg.id = received_mssg.id;
                sent_mssg.type = received_mssg.type;
                sent_mssg.passengers = received_mssg.passengers;
                sent_mssg.weight = received_mssg.weight;
                sent_mssg.departure = received_mssg.departure;
                sent_mssg.arrival = received_mssg.arrival;

                if(msgsnd(msgid,&sent_mssg,sizeof(mssg)-sizeof(long),0) == -1){
                    perror("Error in sending message from ATC to Airport");
                    return 1;
                }

            }

            else if(received_mssg.flag == 1){

                //printf("ARRDEBOARDACK -- Pid: %d \n",received_mssg.id);
                active_planes--;

                sent_mssg.mtype = 40+received_mssg.id;
                sent_mssg.id = received_mssg.id;
                sent_mssg.flag = 0;

                if(msgsnd(msgid,&sent_mssg,sizeof(mssg)-sizeof(long),0) == -1){
                    perror("Error in sending message from ATC to Plane");
                    return 1;
                }

            }



        }

       


    }

    //send termination message to all airports

    for(int i=0;i<n_airports;i++){

        mssg sent_mssg;
        sent_mssg.mtype = 30+i+1;
        sent_mssg.flag = 2;

        if(msgsnd(msgid,&sent_mssg,sizeof(mssg)-sizeof(long),0) == -1){
            perror("Error in sending message from ATC to Airport");
            return 1;
        }

    }

    //wait for all airports to terminate
    int active_airports = n_airports;

    while(active_airports){

        mssg received_mssg;

        if(msgrcv(msgid,&received_mssg,sizeof(mssg)-sizeof(long),-30,0) == -1){
            perror("Error in receiving message from Plane");
            return 1;
        }

        if(received_mssg.flag == 3){
            active_airports--;

        }

    }


    //close message queue
    if (msgctl(msgid,IPC_RMID,NULL) == -1){
        perror("Error in closing message queue of Plane");
        return 1;
    }

    return 0;

}



