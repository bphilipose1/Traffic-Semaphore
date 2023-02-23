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
#include <mutex>

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

/*
NOTES: PROGRAM WILL ONLY END WHEN BOTH PRODUCERS LAND ON TRUE FOR THE 80% COIN FLIP BECAUSE THEN IT WILL CHECK IF IT CAN MAKE ANOTHER CAR, WHICH WILL BE A NO, AND THEN THE PRODUCER FUNCTION WILL EXIT
*/

// mutex locks
pthread_mutex_t mutexCarLog;
pthread_mutex_t mutexQueue; //for both queues 

// counting semaphore
sem_t isEmpty;
sem_t logDone;

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
    int coin = rand() % 100;
    if(coin < 80)   {
        return true;
    }
    else    {
        return false;
    }
}
void Logcar(int ID,char Dir, string arrival_time, string start_time, string end_time)   {
    ofstream outdata("car.log",ios::app);
    outdata.is_open();
    if (!outdata)
    {
        perror("Couldn't open Car file");
        exit(1);
    }
    outdata << ID << setw(7) << Dir << setw(10) <<arrival_time << setw(10) <<start_time << setw(10) << end_time << '\n';
    outdata.flush();
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
    outdata.flush();
    outdata.close();
}
void* carCross(void*arg) {     
         
    car* my_car = (car*)(arg);
    
    time_t s_time = time(0);//time car starts to cross construction lane   
    pthread_sleep(2);//car is crossing construction lane
    time_t e_time = time(0);//time car finishes crossing construction lane
    pthread_mutex_lock(&mutexCarLog);
    Logcar(my_car->carID, my_car->directions, converter(my_car->arrivalTime), converter(s_time), converter(e_time));
    pthread_mutex_unlock(&mutexCarLog);
    delete my_car;  //deallocate car object after completing crossing
    
    sem_post(&logDone);
    return NULL;   
}
void* northCarGenerator(void* totaC) {
    int totalCars = *((int*)totaC);//cast input parameters
    bool GoN = true;
    
    while (GoN) {   
        if (eightyCoin() == true) {
            
            //check if producer should keep making cars
            pthread_mutex_lock(&mutexQueue);
            if((totalProduced+1) > totalCars) {
                GoN = false;
                pthread_mutex_unlock(&mutexQueue);
                pthread_exit(NULL);
            }
            else    {
                totalProduced++;
            }

            car* newCar = new car(totalProduced, 'N', time(nullptr));   //create car object DYNAMICALLY              
            
            if(newCar != nullptr) {
                northTrafficQueue.push(newCar); //add car into queue
            } else {
                cout << "Failed to create a new car object." << endl;
            }
            pthread_mutex_unlock(&mutexQueue);  //return lock after modifying queues  
            sem_post(&isEmpty);
        }   
        else {
            pthread_sleep(21); // sleep if another car does not follow
        }
    }
    pthread_exit(NULL);
}
void* southCarGenerator(void* totaC) {
    int totalCars = *((int*)totaC);//cast input parameters
    bool GoS = true;
    while (GoS) {   
        if (eightyCoin() == true) {
            //check if producer should keep making cars
            pthread_mutex_lock(&mutexQueue);
            if((totalProduced+1) > totalCars) {
                GoS = false;
                pthread_mutex_unlock(&mutexQueue);
                pthread_exit(NULL);
            }
            else    {
                totalProduced++;
            }
            car* newCar = new car(totalProduced, 'S', time(nullptr));   //create car object DYNAMICALLY
            if(newCar != nullptr) {
                southTrafficQueue.push(newCar); //add car into queue
            } else {
                cout << "Failed to create a new car object." << endl;
            }
            pthread_mutex_unlock(&mutexQueue);  //return lock after modifying queues
            sem_post(&isEmpty);
        }
        else {
            
            pthread_sleep(21); // sleep if another car does not follow
        }
    }
    pthread_exit(NULL);
}
void* flagHandler(void* x) {
    int carCnt = 0;
    char laneState = 'N';   //used to state which lane is currently being allowed to pass default to N
    int n_size = 0;
    int s_size = 0;
    clock_t tempSleepTime = 0;
    clock_t tempAwakeTime = 0;
    car* carTemp;    
    int totCars = *((int*) x); //see how many cars are allowed to pass cumulative

    //creates threads for cumulative total of cars
    pthread_t temp;
    
    while(totCars != carCnt)    {
        carCnt++;    

        tempSleepTime = time(nullptr);
        sem_wait(&isEmpty);   //will sleep thread if there is no cars waiting in either queues
        tempAwakeTime = time(nullptr);
        
        pthread_mutex_lock(&mutexQueue);
        //update sizes for both queues
        s_size = getSouthSize();
        n_size = getNorthSize();
        
        if(s_size>=10) {    //checks if any backups needed flow
            laneState = 'S';
        }
        else if(n_size>=10) {    //checks if any backups needed flow
            laneState = 'N';
        }
        else if(s_size>0)   {
            laneState = 'S';
        }
        else    {
            laneState = 'N';
        }
        
        //will pop from queue depending on laneState
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
                pthread_mutex_unlock(&mutexQueue);
                return NULL;
        }
        pthread_mutex_unlock(&mutexQueue);

        //creates car thread for car object
        if(pthread_create(&temp, NULL, &carCross, (void *)carTemp)) {
            perror("Pthread_create failed");
            exit(-1);
        }
        pthread_detach(temp);//detach thread    
        if(tempSleepTime < tempAwakeTime)  {// logging flagperson behavior
            Logflagperson(converter(tempSleepTime), "sleep");
            Logflagperson(converter(tempAwakeTime), "woken-up");
        }
    }
    sem_wait(&logDone);//try to exit consumer, will wait if not all threads finished
    return NULL;
}
int getNorthSize()   {
    int nSize = northTrafficQueue.size();        
    return nSize;
}
int getSouthSize()   {
    int sSize = southTrafficQueue.size();      
    return sSize;
}
int main(int argc, char* argv[]) {
    //obtaining total cars that will pass the lane in this code execution
    if (argc < 2) {
        return -1;
    }
    int cumCarsNum = stoi(argv[1]);
    srand(time(nullptr));//set rand seed
    
    //create thread for 2 producers (North and South car generator), 1 consumer (flag person)
    pthread_t consumerThread;
    pthread_t producerThreadN;
    pthread_t producerThreadS;
    
    //initialize empty semaphore
    sem_init(&isEmpty, 0, 0);
    sem_init(&logDone, 0, ((-cumCarsNum)+1));//dont let consumer exit before all car threads are done

    //initialize pthread mutex locks
    pthread_mutex_init(&mutexCarLog, NULL);
    pthread_mutex_init(&mutexQueue, NULL);

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

    if(pthread_detach(producerThreadN)) {
        perror("Pthread_detach failed");
        exit(-2);
    }
    if(pthread_detach(producerThreadS)) {
        perror("Pthread_detach failed");
        exit(-2);
    }
    //using pthread_join when consumer is done, to not allow main to exit prematurily
    if(pthread_join(consumerThread, NULL)) {
        perror("Pthread_join failed");
        exit(-2);
    }
    //sleep main for a few secconds to let threads wrap up
    pthread_sleep((5));

    //deleting pthread locks
    pthread_mutex_destroy(&mutexCarLog);
    pthread_mutex_destroy(&mutexQueue);

    //delete semaphores
    sem_destroy(&isEmpty);
    sem_destroy(&logDone);
}