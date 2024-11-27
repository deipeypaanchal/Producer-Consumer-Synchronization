/* consumer.c

   This file contains the consumer thread function, which removes characters
   from the shared buffer and displays them. It synchronizes with the producer
   thread using mutexes and condition variables to ensure safe access to the buffer.
*/

#include "shared.h"

void* consumer(void* arg) {
    char item;  // Variable to hold each consumed character

    while (1) {
        pthread_mutex_lock(&shared_buffer.mutex);  // Acquire the mutex lock

        // Wait if the buffer is empty
        while (shared_buffer.count == 0) {
            // Check if the producer has finished producing
            if (shared_buffer.producer_done) {
                pthread_mutex_unlock(&shared_buffer.mutex);  // Release the mutex lock
                printf("Consumer: done\n");  // Inform that the consumer is done
                return NULL;  // Exit the thread
            }
            // Wait for the producer to add items to the buffer
            pthread_cond_wait(&shared_buffer.not_empty, &shared_buffer.mutex);
        }

        // Remove a character from the buffer
        item = shared_buffer.buffer[shared_buffer.out];
        shared_buffer.out = (shared_buffer.out + 1) % BUFFER_SIZE;  // Circular increment
        shared_buffer.count--;  // Decrement the count of items in the buffer

        printf("Consumed: %c\n", item);  // Display the consumed character

        // Signal the producer that the buffer is not full
        pthread_cond_signal(&shared_buffer.not_full);

        pthread_mutex_unlock(&shared_buffer.mutex);  // Release the mutex lock
    }
}