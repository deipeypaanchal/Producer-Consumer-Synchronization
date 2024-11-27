/* producer.c

   This file contains the producer thread function, which reads characters
   from the input string and places them into the shared buffer while
   synchronizing with the consumer thread using mutexes and condition variables.
*/

#include "shared.h"

void* producer(void* arg) {
    char* input = (char*) arg;  // Cast the argument to a character pointer
    int i = 0;                  // Index for traversing the input string
    char item;                  // Variable to hold each character

    // Loop until the end of the input string is reached
    while ((item = input[i]) != '\0') {
        pthread_mutex_lock(&shared_buffer.mutex);  // Acquire the mutex lock

        // Wait if the buffer is full
        while (shared_buffer.count == BUFFER_SIZE) {
            pthread_cond_wait(&shared_buffer.not_full, &shared_buffer.mutex);
        }

        // Place the character into the buffer
        shared_buffer.buffer[shared_buffer.in] = item;
        shared_buffer.in = (shared_buffer.in + 1) % BUFFER_SIZE;  // Circular increment
        shared_buffer.count++;  // Increment the count of items in the buffer

        printf("Produced: %c\n", item);  // Display the produced character

        // Signal the consumer that the buffer is not empty
        pthread_cond_signal(&shared_buffer.not_empty);

        pthread_mutex_unlock(&shared_buffer.mutex);  // Release the mutex lock

        i++;  // Move to the next character in the input string
    }

    // Indicate that the producer has finished producing all characters
    pthread_mutex_lock(&shared_buffer.mutex);
    shared_buffer.producer_done = 1;  // Set the completion flag
    pthread_cond_broadcast(&shared_buffer.not_empty);  // Wake up the consumer if waiting
    pthread_mutex_unlock(&shared_buffer.mutex);

    printf("Producer: done\n");  // Inform that the producer is done
    return NULL;  // Exit the thread
}