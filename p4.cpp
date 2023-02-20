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

// lets us make queue of type car
struct car { 
int carID;
char directions;
string arrivalTime;
string startTime;
string endTime;
};

queue <car> south; // queue of cars going south
queue <car> north; // queue of cars going north

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return -1;
    }
    int cumCarsNum = std::stoi(argv[1]);
    
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

void *produceCar(void *totCars, void* dir, void* arr)
{
	struct car newCar;
    char direction = *((char*) dir);
    int totalCars = *((int*)totCars)
    total_t arrivalTime = *((total_t*)arr)
	
    while(cars <= totCars){
		if(eightyCoin == true){
			sem_wait(&);
			pthread_mutex_lock(&car_mutex);
			if(direction == 'N') {
				++northCount;
                northTrafficQueue.push(newCar);
                //arrivalTime.push();
                pthread_mutex_unlock(&mutex);
            }
			else if (direction == 'S') {
				++southCount;
                southTrafficQueue.push(newCar);
                //arrivalTime.push();
                pthread_mutex_unlock(&mutex);

            }
			pthread_mutex_unlock(&mutex);
			sem_post(&empty);
		}
        else {
			//pthread_sleep(20);
            return -1;
		}
	}
}




/*
void* produceCars(void* totCars, void* dir){
    char direction = *((char*) dir);
    int totalCars = *((int*)totCars);
    struct car newCar;

        if(eightyCoin == true) { // based on num, will unlock corresponding flagger and car thread
            pthread_mutex_unlock((direction == 'N') ? &mutexFlaggerN : &mutexFlaggerS);
            pthread_mutex_unlock((direction == 'N') ? &mutexNumCarsN : &mutexNumCarsS);
            return NULL;
        }

        pthread_mutex_lock((direction == 'N') ? &mutexNumCarsN : &mutexNumCarsS);
        newCar.dir = direction;
        newCar.arrival = arrivalTime;
        newCar.carID = numCars;
        numCars++;
        pthread_mutex_unlock((direction == 'N') ? &mutexNumCarsN : &mutexNumCarsS);
        // conditional operator, returns based on condition
        // in this case either unlocks mutexNumCarsN or mtexNumCarsS based on direction 
        if(direction == 'N'){ // puts cars on queue based on direction
            northTrafficQueue.push(newCar);
        }
        else{
            southTrafficQueue.push(newCar);
        }        

        //
        pthread_mutex_unlock((direction == 'N') ? &mutexFlaggerN : &mutexFlaggerS);
        // unlocks flagger mutex based on direction
        pthread_sleep(20); // sleep for 20 seconds
    }

    return NULL;
}

*/
