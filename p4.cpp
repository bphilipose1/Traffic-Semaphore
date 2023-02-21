#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <semaphore.h>
using namespace std;

queue<car> northTrafficQueue;
queue<car> southTrafficQueue;
sem_t mutex;
sem_t empty;

struct car { 
    int carID;
    char directions;
    time_t arrivalTime;
};

int pthread_sleep (int seconds) {
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    struct timespec timetoexpire;
    if(pthread_mutex_init(&mutex,NULL)) {
        return -1;
    }
    if(pthread_cond_init(&conditionvar,NULL))   {
        return -1;
    }
    //When to expire is an absolute time, so get the current time and add
    //it to our delay time
    timetoexpire.tv_sec = (unsigned int)time(NULL) + seconds;
    timetoexpire.tv_nsec = 0;
    return pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
}

bool eightyCoin()   {
    srand(time(NULL)); 
    int coin = (rand() % 10) + 1;
    if(coin <= 8)   {
        return true;
    }
    else    {
        return false;
    }
}

void Logcar(int ID,char Dir, time_t arrival_time, time_t start_time, time_t end_time){
    ofstream outdata;

    outdata.open("car.log");
    if (!outdata)
    {
        perror("Couldn't open Car file");
        exit(1);
    }
    outdata << ID << setw(7) << Dir << setw(7) <<arrival_time << setw(7) <<start_time << setw(7) << end_time;
    fflush(stdout);
    outdata.close();
}

void Logflagperson(time_t timestamp, string status){
    ofstream outdata;

    outdata.open("flagperson.log");
    if (!outdata)
    {
        perror("Couldn't open flagperson file");
        exit(1);
    }
    outdata << timestamp << setw(15) << status;
    fflush(stdout);
    outdata.close();
}

void* carCross(void*arg) {   
    
    car* my_car= (car*)(arg);
    
    
    pthread_detach(pthread_self());
    time_t s_time = time(0);//time car starts to cross construction lane
    pthread_sleep(1);//car is crossing construction lane

    time_t e_time = time(0);//time car finishes crossing construction lane
    Logcar(my_car->carID, my_car->directions, my_car->arrivalTime, s_time, e_time);

    return NULL;
}


//------------------------BENS SECTION OF HELPER CODE-------------------------

void* flagHandler(void* x) {
    int totCars = *((int*) x); //see how many cars are allowed to pass cumulative 
    int carCnt = 0;
    
    char laneState = N;   //used to state which lane is currently being allowed to pass
    int n_size=0;
    int s_size=0;
    car cartemp;

    pthread_t carThreads[totCars];

    clock_t tempSleepTime=0;
    clock_t tempAwakeTime=0;
    while(totCars != carCnt)    {
        
        
        tempSleepTime = time(nullptr);
        sem_wait(&empty);   //will sleep thread if there is no cars waiting in either queues
        tempAwakeTime = time(nullptr);
        sem_wait(&mutex);

        //update sizes for both queues
        s_size = getSouthSize();
        n_size = getNorthSize();

        if(s_size>=10) {    //checks if any backups needed flow
            laneState = 'S';
        }
        else if(n_size>=10) {    //checks if any backups needed flow
            laneState = 'N';
        }

        //if statement to check which lane should be allowed to pass first
        if(s_size<0)   {
            laneState = 'S';
        }
        else    {
            laneState = 'N';
        }

        //pops car from decided car queue
        switch (laneState)  {
            case 'N':
                carTemp = northTrafficQueue.pop();
                break;
            case 'S':
                carTemp = southTrafficQueue.pop();
                break;
            default:
                return -1;
        }

        //creates car thread for car object
        if(pthread_create(&carThreads[carCnt], NULL, &carCross, (void *) carTemp)) {
            perror("Pthread_create failed");
            exit(-1);
        }

        
        sem_wait(&mutex);

        if(tempSleepTime < tempAwakeTime)  {// logging flagperson behavior
            logFlag(tempSleepTime, "sleep");
            logFlag(tempAwakeTime, "woken-up");
        }
        //NOTE:CHECK IF WHAT IS CRITICAL SECTION IN THIS CODE   
        carCnt++;
    }
}


int getNorthSize()   {
    int nSize = southTrafficQueue.size();
    return nSize;
}
int getSouthSize()   {
    int sSize = northTrafficQueue.size();
    return sSize;
}


//------------------------BENS SECTION OF HELPER CODE-------------------------
int main(int argc, char* argv[]) {
    if (argc < 2) {
        return -1;
    }
    int cumCarsNum = stoi(argv[1]);
    pthread_t consumerThread;
    //initializing mutex lock and empty semaphores for mutual exclusion/synchronization
    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, 0);



    //create producer, and their thread function  (car generators for North and South queue is producer)

    //create consumer, and their thread function  (flag person consumes cars in queue)
    if(pthread_create(&consumerThread, NULL, &flagHandler, (void *) cumCarsNum)) {
        perror("Pthread_create failed");
        exit(-1);
    }

    //deleting semaphores
    sem_destroy(&mutex);
    sem_destroy(&empty);













    //NOTE: THERE IS RACE CONDITION ON SHARED RESOURCE OF THE NORTH AND SOUTH QUEUES
    //use mutex lock for modification of queues

    //start generating cars and add them into North/South Queue
        //each car is a seperate thread
            //create and detach thread

    //FLAG PERSON BEHAVIOR:
    /*
    if car_arrives
        allow traffic from that direction to pass
    
    only stop letting cars pass if 1 of 2 conditions occur
        1. no more cars are coming from that direction
            a. let cars from other direction pass.
            b. if no cars are in either lanes, put flag person is false
        2. there are 10+ cars waiting in the other direction
            switch and let those cars cross.
    
    NOTE: to simulate car crossing behavior, we put the car thread to "sleep" for 1 second, then we can delete the thread...
    
    */

    
}

