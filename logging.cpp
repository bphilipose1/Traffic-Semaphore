#include<iostream>
#include<iomanip>
#include<fstream>
#include<pthread.h>

using namespace std;

struct car{ 
    int carID;
    char directions;
    string arrivalTime;
};

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

void* carCross(void* arrivalTime, void* carID, void* direc)  {   
    time_t a_time = *((time_t*)arrivalTime);
    int cid = *((int*)carID);
    int dir = *((int*)direc);

    car*instance = static_cast<car*>(arg);
    
    time_t s_time = time(0);//time car starts to cross construction lane
    
    pthread_sleep(1);//car is crossing construction lane

    pthread_detach(pthread_self());
    
    time_t e_time = time(0);//time car finishes crossing construction lane
    Logcar(cid,dir,a_time, s_time, e_time);

    return NULL;
}
