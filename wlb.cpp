// g++ -o run wlb.cpp -pthread -lrt

// Needed libraries
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

// Constants
#define THREADS 4
#define SEMAPHORES 3

// Semaphore resources
sem_t phase1_semaphore;
sem_t semaphores[SEMAPHORES];

// Thread object
struct ThreadData {
    char id;
    int work;
};

// Phase 1 thread function
void* phase1(void* arg) {
    // Funky cast to satisfy pthreads
    ThreadData* data = static_cast<ThreadData*>(arg);

    // Run continuisly until work has been completely
    while (data->work > 0) {
        // Wait for semaphore to be aquired
        sem_wait(&phase1_semaphore);

        // Do 'work'
        std::cout << "Thread " << data->id << " is running. Work remaining: " << data->work << std::endl;
        data->work--;

        // Wait
        usleep(100000);

        // Release semaphore
        sem_post(&phase1_semaphore);

        // Wait
        usleep(100000);
    }

    // Exit pthread and formating
    std::cout << "Thread " << data->id << " is finished." << std::endl;
    pthread_exit(NULL);
}

// Phase 2 thread function
void* phase2(void* arg) {
    // Funky cast to satisfy pthreads
    ThreadData* data = static_cast<ThreadData*>(arg);

    // Run continuisly until work has been completed
    while (data->work > 0) {
        // Randomly determine how many resources are required
        int resources_needed = (rand() % SEMAPHORES) + 1;
        int semaphoes_needed[SEMAPHORES] = { 0 };

        // Select from  available semaphores
        for (int i = 0; i < resources_needed; ++i) {
            int index;

            do {
                index = rand() % SEMAPHORES;
            } while (semaphoes_needed[index]);

            semaphoes_needed[index] = 1;
        }

        // Attempt to aquire all semaphores
        bool all_acquired = true;
        for (int i = 0; i < SEMAPHORES; ++i) {
            // Check to see if semaphore is active
            if (semaphoes_needed[i]) {
                // check semaphore availability
                if (sem_trywait(&semaphores[i]) != 0) {
                    all_acquired = false;
                    break;
                }

                // Show thread aquiring resource
                std::cout << data->id << " > " << i << std::endl;
            }
        }

        // Code to run when all semaphores are successfully acquired
        if (all_acquired) {
            // Do 'work'
            std::cout << "Thread " << data->id << " is running. Work remaining: " << data->work << std::endl;
            data->work--;

            // Release all semaphores acquired
            for (int i = 0; i < SEMAPHORES; ++i) {
                if (semaphoes_needed[i]) {
                    sem_post(&semaphores[i]);

                    // Show thread relinquishing resource
                    // std::cout << data->id << " < " << i << std::endl;
                }
            }

            // Lil nano nap time
            timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = (rand() % 11) * 1000000;
            nanosleep(&ts, NULL);
        } else {

            // Release all semaphores acquired
            for (int i = 0; i < SEMAPHORES; ++i) {
                if (semaphoes_needed[i]) {
                    sem_post(&semaphores[i]);

                    // Show thread relinquishing resource
                    // std::cout << data->id << " < " << i << std::endl;
                }
            }

            // Lil nano nap time
            timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = (rand() % 11) * 1000000;
            nanosleep(&ts, NULL);
        }
    }

    // Close thread
    pthread_exit(NULL);
}

int main() {
    // Seed random number generator
    srand(time(NULL));

    // Initialize semaphores
    sem_init(&phase1_semaphore, 0, 1);
    
    for (int i = 0; i < SEMAPHORES; ++i) {
        sem_init(&semaphores[i], 0, 1);
    }

    // Initialize threads and their data
    pthread_t threads[THREADS];

    ThreadData thread_data[THREADS] = {
        {'A', 10},
        {'B', 10},
        {'C', 10},
        {'D', 10}
    };

    std::cout << "###########   Stage 1   ###########" << std::endl << std::endl;

    // Initaite threads
    for (int i = 0; i < THREADS; ++i) {
        if (pthread_create(&threads[i], NULL, phase1, &thread_data[i]) != 0) {
            std::cout << "Error: Failed to create thread " << thread_data[i].id << std::endl;
            return 1;
        }
    }

    // Wait for threads to finish
    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    std::cout << "All threads have finished." << std::endl;

    // Reset work for phase 2
    for (int i = 0; i < THREADS; ++i) {
        thread_data[i].work = 10;
    }

    std::cout << std::endl << "###########   Stage 2   ###########" << std::endl << std::endl;

    // Initaite threads
    for (int i = 0; i < THREADS; ++i) {
        if (pthread_create(&threads[i], NULL, phase2, &thread_data[i]) != 0) {
            std::cout << "Error: Failed to create thread " << thread_data[i].id << std::endl;
            return 1;
        }
    }

    // Wait for threads to finish
    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    std::cout << "All threads have finished." << std::endl;

    // Destroy semaphores
    sem_destroy(&phase1_semaphore);

    for (int i = 0; i < SEMAPHORES; ++i) {
        sem_destroy(&semaphores[i]);
    }

    return 1;
}