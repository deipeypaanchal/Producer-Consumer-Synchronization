// Name: Deepey Panchal
// U-Number: U80305771
// NetID: deepeypradippanchal
// Description: TODO

#include <stdio.h>                  // Standard input/output definitions
#include <stdlib.h>                 // Standard library definitions
#include <pthread.h>                // POSIX threads (pthreads) library contains the thread-related functions and macros.
#include <string.h>                 // String manipulation functions

// Define constants for buffer size and input size
#define BUFFER_SIZE 15  // Maximum of 15 characters in the buffer
#define INPUT_SIZE 51  // Maximum of 50 characters plus null terminator

// Define a structure to hold shared buffer and synchronization variables
typedef struct {
    char buffer[BUFFER_SIZE];       // The shared buffer to store characters
    int in;                         // Index for the producer to insert
    int out;                        // Index for the consumer to remove
    int count;                      // Number of items in the buffer to consume
    int producer_done;              // Flag to indicate producer is done to exit consumer
    pthread_mutex_t mutex;          // Mutex lock for synchronization to access shared buffer
    pthread_cond_t not_empty;       // Condition variable for buffer not empty to consume
    pthread_cond_t not_full;        // Condition variable for buffer not full to produce
} shared_buffer_t;                  // Typedef for shared buffer structure to use as a type
shared_buffer_t shared_buffer;

void* producer(void* arg) {
    char* input = (char*) arg;  // Cast the argument to a character pointer
    int i = 0;                  // Index to iterate over the input string
    char item;                  // Variable to store the current character being processed

    // Loop over the input string until the null terminator is reached
    while ((item = input[i]) != '\0') {                 // Read the current character from the input string
        pthread_mutex_lock(&shared_buffer.mutex);       // Lock the mutex to access the shared buffer

        while (shared_buffer.count == BUFFER_SIZE) {    
            pthread_cond_wait(&shared_buffer.not_full, &shared_buffer.mutex);
        }

        // Add item to the buffer
        shared_buffer.buffer[shared_buffer.in] = item;
        shared_buffer.in = (shared_buffer.in + 1) % BUFFER_SIZE;
        shared_buffer.count++;

        printf("Produced: %c\n", item);

        pthread_cond_signal(&shared_buffer.not_empty);
        pthread_mutex_unlock(&shared_buffer.mutex);

        i++;
    }

    // Producer is done
    pthread_mutex_lock(&shared_buffer.mutex);
    shared_buffer.producer_done = 1;
    pthread_cond_broadcast(&shared_buffer.not_empty);  // Wake up consumer if waiting
    pthread_mutex_unlock(&shared_buffer.mutex);

    printf("Producer: done\n");
    return NULL;
}

void* consumer(void* arg) {
    char item;

    while (1) {
        pthread_mutex_lock(&shared_buffer.mutex);

        while (shared_buffer.count == 0) {
            if (shared_buffer.producer_done) {
                pthread_mutex_unlock(&shared_buffer.mutex);
                printf("Consumer: done\n");
                return NULL;
            }
            pthread_cond_wait(&shared_buffer.not_empty, &shared_buffer.mutex);
        }

        // Remove item from buffer
        item = shared_buffer.buffer[shared_buffer.out];
        shared_buffer.out = (shared_buffer.out + 1) % BUFFER_SIZE;
        shared_buffer.count--;

        printf("Consumed: %c\n", item);

        pthread_cond_signal(&shared_buffer.not_full);
        pthread_mutex_unlock(&shared_buffer.mutex);
    }
}

int main() {
    char input_buffer[INPUT_SIZE];

    // Initialize the shared buffer and synchronization primitives
    shared_buffer.in = 0;
    shared_buffer.out = 0;
    shared_buffer.count = 0;
    shared_buffer.producer_done = 0;
    pthread_mutex_init(&shared_buffer.mutex, NULL);
    pthread_cond_init(&shared_buffer.not_empty, NULL);
    pthread_cond_init(&shared_buffer.not_full, NULL);

    while (1) {
        printf("Enter input (type 'exit' to quit): ");
        if (fgets(input_buffer, INPUT_SIZE, stdin) == NULL) {
            fprintf(stderr, "Error reading input\n");
            continue;
        }

        // Check if the input was longer than 50 characters and discard extra characters
        size_t len = strlen(input_buffer);
        if (len > 0 && input_buffer[len - 1] == '\n') {
            input_buffer[len - 1] = '\0';
            len--;
        } else {
            // Input too long, discard the rest
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }

        if (strcmp(input_buffer, "exit") == 0) {
            printf("Parent: done\n");
            break;
        }

        // Print input and character count
        printf("Input: %s\n", input_buffer);
        printf("Count: %zu characters\n", len);

        // Reset shared buffer variables
        shared_buffer.in = 0;
        shared_buffer.out = 0;
        shared_buffer.count = 0;
        shared_buffer.producer_done = 0;
        memset(shared_buffer.buffer, 0, sizeof(shared_buffer.buffer));

        pthread_t prod_thread, cons_thread;

        // Duplicate the input buffer for the producer
        char *producer_input = strdup(input_buffer);
        if (producer_input == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            exit(EXIT_FAILURE);
        }

        // Create producer thread
        if (pthread_create(&prod_thread, NULL, producer, (void*) producer_input) != 0) {
            fprintf(stderr, "Error creating producer thread\n");
            exit(EXIT_FAILURE);
        }

        // Create consumer thread
        if (pthread_create(&cons_thread, NULL, consumer, NULL) != 0) {
            fprintf(stderr, "Error creating consumer thread\n");
            exit(EXIT_FAILURE);
        }

        // Wait for threads to finish
        pthread_join(prod_thread, NULL);
        pthread_join(cons_thread, NULL);

        free(producer_input);  // Free the duplicated input buffer
    }

    // Destroy synchronization primitives
    pthread_mutex_destroy(&shared_buffer.mutex);
    pthread_cond_destroy(&shared_buffer.not_empty);
    pthread_cond_destroy(&shared_buffer.not_full);

    return 0;
}