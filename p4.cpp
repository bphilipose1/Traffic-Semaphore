#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <semaphore.h>
#include <queue>
#include <iomanip>
using namespace std;

//prototype functions
int pthread_sleep (int seconds);
bool eightyCoin();
void Logcar(int ID,char Dir, time_t arrival_time, time_t start_time, time_t end_time);
void Logflagperson(time_t timestamp, string status);
void* carCross(void*arg);
void* northCarGenerator(void* totaC);
void* southCarGenerator(void* totaC);
void* flagHandler(void* x);
int getNorthSize();
int getSouthSize();
//------------------------------------yonnas--------------------------

struct car { 
    int carID;
    char directions;
    time_t arrivalTime;
    car(int carId, char dir, time_t arrTime) : carID(carId), directions(dir), arrivalTime(arrTime) {}

};

queue<car*> northTrafficQueue;
queue<car*> southTrafficQueue;
int totalProduced = 0;
sem_t mutex;
sem_t isEmpty;

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
    //cout << "In EIGHTYCOIN" << endl;
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
    //cout << "In LOGCAR" << endl;
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
    //cout << "In LOGFLAGPERSON" << endl;
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
    //cout << "In CARCROSS" << endl;
    car* my_car = (car*)(arg);
    
    
    pthread_detach(pthread_self());
    time_t s_time = time(0);//time car starts to cross construction lane
    pthread_sleep(1);//car is crossing construction lane

    time_t e_time = time(0);//time car finishes crossing construction lane
    Logcar(my_car->carID, my_car->directions, my_car->arrivalTime, s_time, e_time);
    delete my_car;  //deallocate car object after completing crossing
    return NULL;
}

//-----------------ryan code------------------------
void* northCarGenerator(void* totaC) {
    //cout << "In NORTHCARGENERATOR" << endl;
    
    int totalCars = *((int*)totaC);//cast input parameters
    while (totalProduced <= totalCars) {    
        //cout << "creating car for north queue" << endl; 
        if (eightyCoin() == true) {            
            sem_post(&isEmpty);
            sem_wait(&mutex); // potentially change lock name or use difefrent type of lock
            car* newCar = new car(++totalProduced, 'N', time(nullptr));//create car object DYNAMICALLY
            northTrafficQueue.push(newCar); //add car into queue
            sem_post(&mutex); 
        }
        else {
            pthread_sleep(20); // sleep if another car does not follow
        }
    }
    pthread_exit(NULL);
}

void* southCarGenerator(void* totaC) {
    //cout << "In SOUTHCARGENERATOR" << endl;
    int totalCars = *((int*)totaC);//cast input parameters
    while (totalProduced <= totalCars) { 
        //cout << "In creating car in south" << endl;       
        if (eightyCoin() == true) {            
            sem_post(&isEmpty);
            sem_wait(&mutex); // potentially change lock name or use difefrent type of lock
            car* newCar = new car(++totalProduced, 'S', time(nullptr));//create car object DYNAMICALLY
            southTrafficQueue.push(newCar); //add car into queue
            sem_post(&mutex); 
        }
        else {
            pthread_sleep(20); // sleep if another car does not follow
        }
    }
    pthread_exit(NULL);
}

//------------------------BENS SECTION OF HELPER CODE-------------------------

void* flagHandler(void* x) {
    //cout << "In FLAGHANDLER" << endl;
    int totCars = *((int*) x); //see how many cars are allowed to pass cumulative 
    int carCnt = 0;
    
    char laneState = 'N';   //used to state which lane is currently being allowed to pass
    int n_size=0;
    int s_size=0;
    car* carTemp;

    //creates threads for cumulative total of cars
    vector<pthread_t> carThreads(totCars);

    clock_t tempSleepTime=0;
    clock_t tempAwakeTime=0;
    while(totCars != carCnt)    {
        
        
        cout<<"we in while loop" << endl;

        tempSleepTime = time(nullptr);
        sem_wait(&isEmpty);   //will sleep thread if there is no cars waiting in either queues
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
                carTemp = northTrafficQueue.front();
                northTrafficQueue.pop();
                break;
            case 'S':
                carTemp = southTrafficQueue.front();
                southTrafficQueue.pop();
                break;
            default:
                return NULL;
        }

        //creates car thread for car object
        if(pthread_create(&carThreads[carCnt], NULL, &carCross, (void *)carTemp)) {
            perror("Pthread_create failed");
            exit(-1);
        }

        
        sem_wait(&mutex);

        if(tempSleepTime < tempAwakeTime)  {// logging flagperson behavior
            Logflagperson(tempSleepTime, "sleep");
            Logflagperson(tempAwakeTime, "woken-up");
        }
        //NOTE:CHECK IF WHAT IS CRITICAL SECTION IN THIS CODE   
        carCnt++;
        //cout << "In FLAGHANDLER" << carCnt << endl;
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
    cout << "In main" << endl;
    //obtaining total cars that will pass the lane in this code execution
    if (argc < 2) {
        return -1;
    }
    int cumCarsNum = stoi(argv[1]);

    //create thread for 2 producers (North and South car generator), 1 consumer (flag person)
    pthread_t consumerThread;
    pthread_t producerThreadN;
    pthread_t producerThreadS;
    
    //initializing mutex lock and isEmpty semaphores for mutual exclusion/synchronization
    sem_init(&mutex, 0, 1);
    sem_init(&isEmpty, 0, 0); //semaphore to check if no cars in either queue
    cout << "about to make pthreads1" << endl;
    //setting up Consumer(flagperson) thread function with thread
    if(pthread_create(&consumerThread, NULL, &flagHandler, (void *) cumCarsNum)) {
        perror("Pthread_create failed");
        exit(-1);
    }
    if(pthread_join(consumerThread, NULL)) {
        perror("Pthread_join failed");
        exit(-2);
    }
    cout << "about to make pthreads2" << endl;
    //setting up Producer(North car generator) thread function with thread
    
    /*
    if(pthread_create(&producerThreadN, NULL, &northCarGenerator, (void *) cumCarsNum)) {
        perror("Pthread_create failed");
        exit(-1);
    }
    if(pthread_join(producerThreadN, NULL)) {
        perror("Pthread_join failed");
        exit(-2);
    }
    cout << "about to make pthreads3" << endl;
    */
    //setting up Producer(South car generator) thread function with thread
    /*
    if(pthread_create(&producerThreadS, NULL, &southCarGenerator, (void *) cumCarsNum)) {
        perror("Pthread_create failed");
        exit(-1);
    }
    if(pthread_join(producerThreadS, NULL)) {
        perror("Pthread_join failed");
        exit(-2);
    }
    */
    cout << "finished making pthreads" << endl;



    cout << "In Threads created" << endl;


    //deleting semaphores
    sem_destroy(&mutex);
    sem_destroy(&isEmpty);













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

