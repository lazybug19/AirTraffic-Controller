

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>






#define MAX_PLANES 100

key_t key;
int msgid;

int active_planes=0;
int total_planes = 0;

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

sem_t mutex;
sem_t runways;
sem_t backup_runway;

int runway_load[10];
int runway_status[10];


pthread_t plane_thread_ids[MAX_PLANES];

void *plane_thread(void *arg ){
    

    mssg plane = *((mssg *)arg);
    int plane_weight = plane.weight;

    mssg send_message;  

    int weight_limit_flag=0;
    for (int i = 0; i < 10; i++) {
        if (runway_load[i] > plane_weight) {
            weight_limit_flag = 1;
            break;
        }

    }


    //keep trying to find the best fit runway to land or takeoff , ignore unitialized runways
    if(sem_wait(&mutex) == -1){
        perror("Error in waiting for mutex");
        return NULL;
    }

   
    if ( weight_limit_flag == 0){
        
        if(sem_wait(&backup_runway) == -1){
            perror("Error in waiting for backup runway");
            return NULL;
        }

        if(sem_post(&mutex) == -1){
            perror("Error in posting for mutex");
            return NULL;
        }

        if(plane.flag == 0){
            printf("PLane %d taking off on the Backup Runway weight-%d\n",plane.id,plane.weight);
            //Board time
            sleep(3);
            
            sleep(2);

            if(sem_post(&backup_runway) == -1){
                perror("Error in posting for backup runway");
                return NULL;
            }


            send_message.mtype = 20+plane.departure;
            send_message.flag = 0;
            send_message.id = plane.id;
            send_message.type = plane.type;
            send_message.passengers = plane.passengers;
            send_message.weight = plane.weight;
            send_message.departure = plane.departure;
            send_message.arrival = plane.arrival;

            if(msgsnd(msgid,&send_message,sizeof(mssg)-sizeof(long),0) == -1){
                perror("Error in sending message from ATC to Airport");
                return NULL;
            }



        }

        else if(plane.flag == 1){
            
            printf("PLane %d landing on the Backup Runway weight-%d\n",plane.id,plane.weight);
            sleep(2);
            

            if(sem_post(&backup_runway) == -1){
                perror("Error in posting for runways");
                return NULL;
            }
            
            sleep(3);
                    
            send_message.mtype = 20+plane.arrival;
            send_message.flag = 1;
            send_message.id = plane.id;
            send_message.type = plane.type;
            send_message.passengers = plane.passengers;
            send_message.weight = plane.weight;
            send_message.departure = plane.departure;
            send_message.arrival = plane.arrival;

            if(msgsnd(msgid,&send_message,sizeof(mssg)-sizeof(long),0) == -1){
                perror("Error in sending message from ATC to Airport");
                return NULL;
            }

        }
        free(arg);
        pthread_exit(NULL);
    }

    if(sem_post(&mutex) == -1){
        perror("Error in posting for mutex");
        return NULL;
    }
    

    int runway_no = -1;
    int best_weight = 1000000;
    //find the best fit runway that is free
    while(1)
    {   

        if(sem_wait(&mutex) == -1){
            perror("Error in waiting for mutex");
            return NULL;
        }

        if (sem_wait(&runways) == -1) {
            perror("Error in waiting for runways");
            return NULL;
        }
       
        for(int i=0;i<10;i++){
            if(runway_status[i]==0){
                
                if(runway_load[i]>=plane_weight){
                    if(runway_load[i]<best_weight){
                        runway_no = i;
                        best_weight = runway_load[i];
                    }
                }
            }
        }

        if(runway_no==-1){
            

            if(sem_post(&runways) == -1){
                perror("Error in posting for runways");
                return NULL;
            }

            if(sem_post(&mutex) == -1){
                perror("Error in posting for mutex");
                return NULL;
            }
            
        }
        else{

            runway_status[runway_no]=1;

            if(sem_post(&mutex) == -1){
                perror("Error in posting for mutex");
                return NULL;
            }
            break;
        }
    }


   
    
   
    if(plane.flag == 0){
       
        //Board time
        sleep(3);
        //takeoff time
        sleep(2);
        printf("Plane %d has completed boarding /loading and taken off from Runways No. %d of Airport No. %d\n",plane.id,runway_no+1,plane.departure);
        if(sem_post(&runways) == -1){
            perror("Error in posting for runways");
            return NULL;
        }

        runway_status[runway_no]=0;

        send_message.mtype = 20+plane.departure;
        send_message.flag = 0;
        send_message.id = plane.id;
        send_message.type = plane.type;
        send_message.passengers = plane.passengers;
        send_message.weight = plane.weight;
        send_message.departure = plane.departure;
        send_message.arrival = plane.arrival;

        if(msgsnd(msgid,&send_message,sizeof(mssg)-sizeof(long),0) == -1){
            perror("Error in sending message from ATC to Airport");
            return NULL;
        }



    }

    else if(plane.flag == 1){
        
       //landing time
        sleep(2);
        runway_status[runway_no]=0;

        if(sem_post(&runways) == -1){
            perror("Error in posting for runways");
            return NULL;
        }
        //deboarding time
        sleep(3);
        printf("Plane %d has landed on Runways No. %d of Airport No. %d and has completed deboarding/unloading.\n",plane.id,runway_no+1,plane.arrival);


        send_message.mtype = 20+plane.arrival;
        send_message.flag = 1;
        send_message.id = plane.id;
        send_message.type = plane.type;
        send_message.passengers = plane.passengers;
        send_message.weight = plane.weight;
        send_message.departure = plane.departure;
        send_message.arrival = plane.arrival;

        if(msgsnd(msgid,&send_message,sizeof(mssg)-sizeof(long),0) == -1){
            perror("Error in sending message from ATC to Airport");
            return NULL;
        }

    }
    
    


    free(arg);
    pthread_exit(NULL);

}

int main(){
    

    key=ftok("plane.c",'A');
    if (key == -1){
        perror("Error in ftok of Plane");
        return 1;
    }

    msgid = msgget(key,IPC_CREAT | 0666);
    if (msgid == -1){
        perror("Error in creating message queue of Plane");
        return 1;
    }

    int airport_id;
    printf("Enter the Airport Number: \n");
    scanf("%d",&airport_id);

    int runway_no=0;
    printf("Enter the number of Runways : \n");
    scanf("%d",&runway_no);

    if (sem_init(&mutex, 0, 1) == -1) {
        perror("Error in initializing mutex");
        return 1;
    }
    if(sem_init(&runways,0,runway_no) == -1){
        perror("Error in initializing runways");
        return 1;
    }
    if (sem_init(&backup_runway, 0, 1) == -1) {
        perror("Error in initializing backup_runway");
        return 1;
    }

    for(int i=0;i<10;i++){
        runway_load[i]=0;
        runway_status[i]=-1;
    }


    printf("Enter  loadCapacity  of  Runways  (give  as  a  space  separated  list  in  a single line\n)");
    for(int i=0;i<runway_no;i++){
        scanf(" %d",&runway_load[i]);
        runway_status[i]=0;
    }

    mssg recieved_message;



    int terminate_flag = 1;

    while(terminate_flag||active_planes){
        
        if(msgrcv(msgid,&recieved_message,sizeof(mssg)-sizeof(long),30+airport_id,0) == -1){
            perror("Error in receiving message from Plane");
            return 1;
        }

        
        if(recieved_message.flag == 2){
            terminate_flag = 0;
        }

        if(recieved_message.flag == 3){
            active_planes++;
        }


        else if(recieved_message.flag == 0 || recieved_message.flag == 1){
            total_planes++;
            if (recieved_message.flag == 1)
            {
                //travel time 30s
                sleep(30);
            }
            
            mssg* thread_message = malloc(sizeof(mssg));
            *thread_message = recieved_message;
            pthread_create(&plane_thread_ids[total_planes],NULL,plane_thread, thread_message);
        }

        if(recieved_message.flag == 1){
            active_planes--;
        }

    }

    for (int i = 0; i < total_planes; i++) {
        pthread_join(plane_thread_ids[i], NULL);
    }

    //termination and resource deallocation


    if(sem_destroy(&mutex) == -1){
        perror("Error in destroying mutex");
        return 1;
    }

    if(sem_destroy(&runways) == -1){
        perror("Error in destroying runways");
        return 1;
    }

    if(sem_destroy(&backup_runway) == -1){
        perror("Error in destroying backup_runway");
        return 1;
    }


    mssg sent_mssg;

    sent_mssg.mtype = 20+airport_id;
    sent_mssg.flag = 3;


    if(msgsnd(msgid,&sent_mssg,sizeof(mssg)-sizeof(long),0) == -1){
        perror("Error in sending message from ATC to Airport");
        return 1;
    }



    return 0;
}