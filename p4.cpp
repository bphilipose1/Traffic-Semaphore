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
void Logcar(int ID,char Dir, string arrival_time, string start_time, string end_time);
void Logflagperson(string timestamp, string status);
void* carCross(void*arg);
void* northCarGenerator(void* totaC);
void* southCarGenerator(void* totaC);
void* flagHandler(void* x);
int getNorthSize();
int getSouthSize();
string converter(time_t convert);

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
int isEmptycount;

string converter(time_t convert){ // get the current time as a time_t value
    struct tm* timeinfo = localtime(&convert); // convert to local time
    int hours = timeinfo->tm_hour; // extract hours
    int minutes = timeinfo->tm_min; // extract minutes
    int seconds = timeinfo->tm_sec; // extract seconds
    string str1 = to_string(hours);
    string str2 = to_string(minutes);
    string str3 = to_string(seconds);
    
    string time = str1 + ':' + str2 + ':' + str3;
    
    return time; 
}

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
    int coin = (rand() % 10) + 1;
    if(coin <= 8)   {
        
        return true;
    }
    else    {
        return false;
    }
}

void Logcar(int ID,char Dir, string arrival_time, string start_time, string end_time){

    ofstream outdata("car.log",ios::app);

    outdata.is_open();
    if (!outdata)
    {
        perror("Couldn't open Car file");
        exit(1);
    }
    outdata << ID << setw(7) << Dir << setw(10) <<arrival_time << setw(10) <<start_time << setw(10) << end_time << '\n';
    outdata.close();
}

void Logflagperson(string timestamp, string status){

    ofstream outdata("flagperson.log",ios::app);

    outdata.is_open();
    if (!outdata)
    {
        perror("Couldn't open flagperson file");
        exit(1);
    }
    outdata << timestamp << setw(15) << status << endl;
    outdata.close();
}
void* carCross(void*arg) {   
    
    car* my_car = (car*)(arg);
    
    pthread_detach(pthread_self());


    time_t s_time = time(0);//time car starts to cross construction lane
     
    pthread_sleep(1);//car is crossing construction lane

    time_t e_time = time(0);//time car finishes crossing construction lane
   
    
    Logcar(my_car->carID, my_car->directions, converter(my_car->arrivalTime), converter(s_time), converter(e_time));
    cout << "deleting carID: " << my_car->carID << endl;
    delete my_car;  //deallocate car object after completing crossing
    return NULL;
}
void* northCarGenerator(void* totaC) {
    
    int totalCars = *((int*)totaC);//cast input parameters
    while (totalProduced < totalCars) {    
        if (eightyCoin() == true) { 

            sem_post(&isEmpty);
            isEmptycount++;
            sem_wait(&mutex); // potentially change lock name or use difefrent type of lock
            car* newCar = new car(++totalProduced, 'N', time(nullptr));//create car object DYNAMICALLY
            if(newCar != nullptr) {
                cout << "pushing carID: " << newCar->carID << " into north queue" << endl;
                northTrafficQueue.push(newCar); //add car into queue
            } else {
                cout << "Failed to create a new car object." << endl;
            }
            sem_post(&mutex); 
            //cout << "total Produced: " << totalProduced << endl;
        }
        else {
            cout << "breaktime: " << converter(time(nullptr)) << endl;
            pthread_sleep(20); // sleep if another car does not follow
            cout << "end of breaktime: " << converter(time(nullptr)) << endl;
        }
    }
    pthread_exit(NULL);
}

void* southCarGenerator(void* totaC) {

    int totalCars = *((int*)totaC);//cast input parameters
    while (totalProduced < totalCars) { 
    
        if (eightyCoin() == true) {            
            sem_post(&isEmpty);
            isEmptycount++;
            sem_wait(&mutex); // potentially change lock name or use difefrent type of lock
            car* newCar = new car(++totalProduced, 'S', time(nullptr));//create car object DYNAMICALLY
            if(newCar != nullptr) {
                cout << "pushing carID: " << newCar->carID << " into south queue" << endl;
                southTrafficQueue.push(newCar); //add car into queue
            } else {
                cout << "Failed to create a new car object." << endl;
            }
            sem_post(&mutex); 
            cout << "total Produced: " << totalProduced << endl;
        }
        else {
            cout << "breaktime: " << converter(time(nullptr)) << endl;
            pthread_sleep(20); // sleep if another car does not follow
            cout << "end of breaktime: " << converter(time(nullptr)) << endl;
        }
    }
    pthread_exit(NULL);
}
void* flagHandler(void* x) {

    int carCnt = 0;
    char laneState = 'N';   //used to state which lane is currently being allowed to pass
    int n_size=0;
    int s_size=0;
    car* carTemp;
    
    
    int totCars = *((int*) x); //see how many cars are allowed to pass cumulative
    //creates threads for cumulative total of cars
    vector<pthread_t> carThreads(totCars);

    clock_t tempSleepTime=0;
    clock_t tempAwakeTime=0;

    while(totCars != carCnt)    {
                
        tempSleepTime = time(nullptr);
        cout << "isEmpty: " << isEmptycount << endl;
        sem_wait(&isEmpty);   //will sleep thread if there is no cars waiting in either queues
        isEmptycount--;
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
        if(s_size>0)   {
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
                cout << "CarID: " << carTemp->carID << " Was just popped from north queue." << endl;
                break;
            case 'S':
                carTemp = southTrafficQueue.front();
                southTrafficQueue.pop();
                cout << "CarID: " << carTemp->carID << " Was just popped from south queue." << endl;
                break;
            default:
                return NULL;
        }

        //creates car thread for car object
        if(pthread_create(&carThreads[carCnt], NULL, &carCross, (void *)carTemp)) {
            perror("Pthread_create failed");
            exit(-1);
        }

        sem_post(&mutex);
        
        if(tempSleepTime < tempAwakeTime)  {// logging flagperson behavior
            cout << "Sleep Time: " << converter(tempSleepTime) << " Wake Time: " << converter(tempAwakeTime) << endl; 
            Logflagperson(converter(tempSleepTime), "sleep");
            Logflagperson(converter(tempAwakeTime), "woken-up");
        }
        //NOTE:CHECK IF WHAT IS CRITICAL SECTION IN THIS CODE   
        carCnt++;
    }
    return NULL;
}

int getNorthSize()   {
    int nSize = northTrafficQueue.size();
    cout << "North Queue Size: " << nSize << endl;
    return nSize;
}

int getSouthSize()   {
    int sSize = southTrafficQueue.size();
    cout << "South Queue Size: " << sSize << endl;
    return sSize;
}
int main(int argc, char* argv[]) {
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

    //setting up Consumer(flagperson) thread function with thread
    if(pthread_create(&consumerThread, NULL, &flagHandler, (void *) &cumCarsNum)) {
        perror("Pthread_create failed");
        exit(-1);
    }

    //setting up Producer(North car generator) thread function with thread
    if(pthread_create(&producerThreadN, NULL, &northCarGenerator, (void *) &cumCarsNum)) {
        perror("Pthread_create failed");
        exit(-1);
    }
    //setting up Producer(South car generator) thread function with thread
    if(pthread_create(&producerThreadS, NULL, &southCarGenerator, (void *) &cumCarsNum)) {
        perror("Pthread_create failed");
        exit(-1);
    }
    cout << "In Threads created" << endl;

    if(pthread_join(consumerThread, NULL)) {
        perror("Pthread_join failed");
        exit(-2);
    }
    if(pthread_join(producerThreadN, NULL)) {
        perror("Pthread_join failed");
        exit(-2);
    }
    if(pthread_join(producerThreadS, NULL)) {
        perror("Pthread_join failed");
        exit(-2);
    }
    
    
    cout << "finished making pthreads" << endl;






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
void Logcar(int ID,char Dir, string arrival_time, string start_time, string end_time);
void Logflagperson(string timestamp, string status);
void* carCross(void*arg);
void* northCarGenerator(void* totaC);
void* southCarGenerator(void* totaC);
void* flagHandler(void* x);
int getNorthSize();
int getSouthSize();
string converter(time_t convert);

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
int isEmptycount;

string converter(time_t convert){ // get the current time as a time_t value
    struct tm* timeinfo = localtime(&convert); // convert to local time
    int hours = timeinfo->tm_hour; // extract hours
    int minutes = timeinfo->tm_min; // extract minutes
    int seconds = timeinfo->tm_sec; // extract seconds
    string str1 = to_string(hours);
    string str2 = to_string(minutes);
    string str3 = to_string(seconds);
    
    string time = str1 + ':' + str2 + ':' + str3;
    
    return time; 
}

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
    int coin = (rand() % 10) + 1;
    if(coin <= 8)   {
        
        return true;
    }
    else    {
        return false;
    }
}

void Logcar(int ID,char Dir, string arrival_time, string start_time, string end_time){

    ofstream outdata("car.log",ios::app);

    outdata.is_open();
    if (!outdata)
    {
        perror("Couldn't open Car file");
        exit(1);
    }
    outdata << ID << setw(7) << Dir << setw(10) <<arrival_time << setw(10) <<start_time << setw(10) << end_time << '\n';
    outdata.close();
}

void Logflagperson(string timestamp, string status){

    ofstream outdata("flagperson.log",ios::app);

    outdata.is_open();
    if (!outdata)
    {
        perror("Couldn't open flagperson file");
        exit(1);
    }
    outdata << timestamp << setw(15) << status << endl;
    outdata.close();
}
void* carCross(void*arg) {   
    
    car* my_car = (car*)(arg);
    
    pthread_detach(pthread_self());


    time_t s_time = time(0);//time car starts to cross construction lane
     
    pthread_sleep(1);//car is crossing construction lane

    time_t e_time = time(0);//time car finishes crossing construction lane
   
    
    Logcar(my_car->carID, my_car->directions, converter(my_car->arrivalTime), converter(s_time), converter(e_time));
    cout << "deleting carID: " << my_car->carID << endl;
    delete my_car;  //deallocate car object after completing crossing
    return NULL;
}
void* northCarGenerator(void* totaC) {
    
    int totalCars = *((int*)totaC);//cast input parameters
    while (totalProduced < totalCars) {    
        if (eightyCoin() == true) { 

            sem_post(&isEmpty);
            isEmptycount++;
            sem_wait(&mutex); // potentially change lock name or use difefrent type of lock
            car* newCar = new car(++totalProduced, 'N', time(nullptr));//create car object DYNAMICALLY
            if(newCar != nullptr) {
                cout << "pushing carID: " << newCar->carID << " into north queue" << endl;
                northTrafficQueue.push(newCar); //add car into queue
            } else {
                cout << "Failed to create a new car object." << endl;
            }
            sem_post(&mutex); 
            //cout << "total Produced: " << totalProduced << endl;
        }
        else {
            cout << "breaktime: " << converter(time(nullptr)) << endl;
            pthread_sleep(20); // sleep if another car does not follow
            cout << "end of breaktime: " << converter(time(nullptr)) << endl;
        }
    }
    pthread_exit(NULL);
}

void* southCarGenerator(void* totaC) {

    int totalCars = *((int*)totaC);//cast input parameters
    while (totalProduced < totalCars) { 
    
        if (eightyCoin() == true) {            
            sem_post(&isEmpty);
            isEmptycount++;
            sem_wait(&mutex); // potentially change lock name or use difefrent type of lock
            car* newCar = new car(++totalProduced, 'S', time(nullptr));//create car object DYNAMICALLY
            if(newCar != nullptr) {
                cout << "pushing carID: " << newCar->carID << " into south queue" << endl;
                southTrafficQueue.push(newCar); //add car into queue
            } else {
                cout << "Failed to create a new car object." << endl;
            }
            sem_post(&mutex); 
            cout << "total Produced: " << totalProduced << endl;
        }
        else {
            cout << "breaktime: " << converter(time(nullptr)) << endl;
            pthread_sleep(20); // sleep if another car does not follow
            cout << "end of breaktime: " << converter(time(nullptr)) << endl;
        }
    }
    pthread_exit(NULL);
}
void* flagHandler(void* x) {

    int carCnt = 0;
    char laneState = 'N';   //used to state which lane is currently being allowed to pass
    int n_size=0;
    int s_size=0;
    car* carTemp;
    
    
    int totCars = *((int*) x); //see how many cars are allowed to pass cumulative
    //creates threads for cumulative total of cars
    vector<pthread_t> carThreads(totCars);

    clock_t tempSleepTime=0;
    clock_t tempAwakeTime=0;

    while(totCars != carCnt)    {
                
        tempSleepTime = time(nullptr);
        cout << "isEmpty: " << isEmptycount << endl;
        sem_wait(&isEmpty);   //will sleep thread if there is no cars waiting in either queues
        isEmptycount--;
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
        if(s_size>0)   {
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
                cout << "CarID: " << carTemp->carID << " Was just popped from north queue." << endl;
                break;
            case 'S':
                carTemp = southTrafficQueue.front();
                southTrafficQueue.pop();
                cout << "CarID: " << carTemp->carID << " Was just popped from south queue." << endl;
                break;
            default:
                return NULL;
        }

        //creates car thread for car object
        if(pthread_create(&carThreads[carCnt], NULL, &carCross, (void *)carTemp)) {
            perror("Pthread_create failed");
            exit(-1);
        }

        sem_post(&mutex);
        
        if(tempSleepTime < tempAwakeTime)  {// logging flagperson behavior
            cout << "Sleep Time: " << converter(tempSleepTime) << " Wake Time: " << converter(tempAwakeTime) << endl; 
            Logflagperson(converter(tempSleepTime), "sleep");
            Logflagperson(converter(tempAwakeTime), "woken-up");
        }
        //NOTE:CHECK IF WHAT IS CRITICAL SECTION IN THIS CODE   
        carCnt++;
    }
    return NULL;
}

int getNorthSize()   {
    int nSize = northTrafficQueue.size();
    cout << "North Queue Size: " << nSize << endl;
    return nSize;
}

int getSouthSize()   {
    int sSize = southTrafficQueue.size();
    cout << "South Queue Size: " << sSize << endl;
    return sSize;
}
int main(int argc, char* argv[]) {
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

    //setting up Consumer(flagperson) thread function with thread
    if(pthread_create(&consumerThread, NULL, &flagHandler, (void *) &cumCarsNum)) {
        perror("Pthread_create failed");
        exit(-1);
    }

    //setting up Producer(North car generator) thread function with thread
    if(pthread_create(&producerThreadN, NULL, &northCarGenerator, (void *) &cumCarsNum)) {
        perror("Pthread_create failed");
        exit(-1);
    }
    //setting up Producer(South car generator) thread function with thread
    if(pthread_create(&producerThreadS, NULL, &southCarGenerator, (void *) &cumCarsNum)) {
        perror("Pthread_create failed");
        exit(-1);
    }
    cout << "In Threads created" << endl;

    if(pthread_join(consumerThread, NULL)) {
        perror("Pthread_join failed");
        exit(-2);
    }
    if(pthread_join(producerThreadN, NULL)) {
        perror("Pthread_join failed");
        exit(-2);
    }
    if(pthread_join(producerThreadS, NULL)) {
        perror("Pthread_join failed");
        exit(-2);
    }
    
    
    cout << "finished making pthreads" << endl;






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
