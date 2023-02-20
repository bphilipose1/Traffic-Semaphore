#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>

using namespace std;

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

int logCar(int carID, char dir, clock_t arrival_time, clock_t start_time, clock_t end_time) {
    ofstream logfile("car.log");
    if (logfile.is_open()) {
        logfile << carID << "," << dir << "," << arrival_time << "," << start_time << "," << end_time << "\n";
        logfile.close(); // close the file
    } else {
        error -2;
    }
}

int logFlag(clock_t timeStamp, String State) {
    ofstream logfile("flagperson.log");
    if (logfile.is_open()) {
        logfile << timeStamp << "," << State << "\n";
        logfile.close(); // close the file
    } else {
        error -2;
    }
}

void* carCross(void* arrivalTime, void* carID, void* direc)  {    //function that thread will execute when crossing construction area
    time_t a_time = *((time_t*)arrivalTime);
    int cid = *((int*)carID);
    int dir = *((int*)direc);

    time_t s_time = time(0);//time car starts to cross construction lane
    pthread_sleep(1);//car is crossing construction lane
    time_t e_time = time(0);//time car finishes crossing construction lane
    logCar(cid,dir,a_time, s_time, e_time);
    return NULL;
}


void* carxProducerFunc(void* totCars, void* dir)    {
    int totalCars = *((int*)totCars);
    char direction = *((char*)dir);
    for(int i = 0; i<totalCars; i++)    {
        //create car objects
    }
    if(direction = "N")    {
        //add to north queue
    }
    else if(direction = "S")    {
        //add to south queue
    }
    else    {
        return -1;
    }
    //set car objects arrival time parameter = time(0);
}



int main(int argc, char* argv[]) {
    if (argc < 2) {
        return -1;
    }
    int cumCarsNum = stoi(argv[1]);
    

    //create producer, and their thread function  (car generators for North and South queue is producer)

    //create consumer, and their thread function  (flag person consumes cars in queue)


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

