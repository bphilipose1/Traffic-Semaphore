// Define global variables
Semaphore north_waiting = 0;
Semaphore south_waiting = 0;
Semaphore mutex = 1;
int north_count = 0;
int south_count = 0;
int cars_completed = 0;

// Define thread functions
void* northbound_cars(void* arg) {
    int id = *(int*)arg;
    int next_car_time = 0;
    while (cars_completed < TOTAL_CARS) {
        // Wait for next car to arrive
        pthread_sleep(next_car_time);
        // Generate next car arrival time
        next_car_time = generate_arrival_time();
        // Acquire mutex to add to queue
        sem_wait(&mutex);
        north_count++;
        printf("Car %d arrived from the North (queue size = %d)\n", id, north_count);
        sem_post(&mutex);
        // Signal flagperson if needed
        if (north_count == 1 && south_count < 10) {
            sem_post(&north_waiting);
        }
        // Wait for flagperson to signal
        sem_wait(&south_waiting);
        // Pass through construction zone
        pthread_sleep(1);
        // Acquire mutex to remove from queue
        sem_wait(&mutex);
        north_count--;
        cars_completed++;
        printf("Car %d passed the construction zone (queue size = %d)\n", id, north_count);
        sem_post(&mutex);
    }
    pthread_exit(NULL);
}

void* southbound_cars(void* arg) {
    int id = *(int*)arg;
    int next_car_time = 0;
    while (cars_completed < TOTAL_CARS) {
        // Wait for next car to arrive
        pthread_sleep(next_car_time);
        // Generate next car arrival time
        next_car_time = generate_arrival_time();
        // Acquire mutex to add to queue
        sem_wait(&mutex);
        south_count++;
        printf("Car %d arrived from the South (queue size = %d)\n", id, south_count);
        sem_post(&mutex);
        // Signal flagperson if needed
        if (south_count == 1 && north_count < 10) {
            sem_post(&south_waiting);
        }
        // Wait for flagperson to signal
        sem_wait(&north_waiting);
        // Pass through construction zone
        pthread_sleep(1);
        // Acquire mutex to remove from queue
        sem_wait(&mutex);
        south_count--;
        cars_completed++;
        printf("Car %d passed the construction zone (queue size = %d)\n", id, south_count);
        sem_post(&mutex);
    }
    pthread_exit(NULL);
}

void* flagperson(void* arg) {
    while (cars_completed < TOTAL_CARS) {
        // Check if there are cars waiting to go North
        if (north_count > 0) {
            // Signal Northbound cars to go
            sem_post(&south_waiting);
            // Wait until all Northbound cars have passed or there are too many Southbound cars
            while (north_count > 0 && south_count < 10) {
                pthread_yield();
            }
        }
        // Check if there are cars waiting to go South
        if (south_count > 0) {
            // Signal Southbound cars to go
            sem_post(&north_waiting);
            // Wait until all Southbound cars have passed or there are too many Northbound cars
            while (south_count > 0 && north_count < 10) {
                pthread_yield();
            }
        }
        // If there are no cars, flagperson can rest
        if (north
