#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <iostream>
#include <cstdlib>


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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return -1;
    }
    int cumCars = std::stoi(argv[1]);
    
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


