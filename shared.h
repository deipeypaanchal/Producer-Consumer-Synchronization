/* shared.h

   This header file contains the shared data structures, global variables,
   and function declarations used by the producer and consumer threads.
*/

#ifndef SHARED_H
#define SHARED_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 15    // Size of the circular buffer
#define INPUT_SIZE 51     // Maximum input size (50 characters + null terminator)

// Structure representing the shared buffer and synchronization primitives
typedef struct {
    char buffer[BUFFER_SIZE];       // The circular buffer array
    int in;                         // Index for the next write by producer
    int out;                        // Index for the next read by consumer
    int count;                      // Number of items currently in the buffer
    int producer_done;              // Flag indicating if the producer has finished
    pthread_mutex_t mutex;          // Mutex lock for synchronizing access to buffer
    pthread_cond_t not_empty;       // Condition variable indicating buffer is not empty
    pthread_cond_t not_full;        // Condition variable indicating buffer is not full
} shared_buffer_t;

// Extern declaration of the shared buffer variable
extern shared_buffer_t shared_buffer;

// Function declarations for producer and consumer thread functions
void* producer(void* arg);
void* consumer(void* arg);

#endif // SHARED_H