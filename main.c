// Name: Deepey Panchal
// U-Number: U80305771
// NetID: deepeypradippanchal
// Description: Implementation of a producer-consumer model using condition variables and a circular buffer.
//              This program creates a circular buffer with 15 positions to facilitate communication between
//              a producer thread and a consumer thread. The producer reads characters from user input one by
//              one and places them into the buffer until it reaches a null character. The consumer thread
//              reads characters from the shared buffer and prints them to the screen. Synchronization between
//              threads is managed using mutex locks and condition variables to ensure safe access to the shared
//              buffer. The parent process initializes the threads and waits for both to finish execution before
//              processing the next input.

#include <stdio.h>                  // Standard input/output definitions
#include <stdlib.h>                 // Standard library definitions
#include <pthread.h>                // POSIX threads (pthreads) library contains the thread-related functions and macros.
#include <string.h>                 // String manipulation functions

// Limited Size Resource: The shared buffer is a circular buffer with a fixed size of 15, representing the limited resource.
// Circular Buffer Implementation: The buffer is implemented as a circular buffer with 15 positions, each storing one character.
// Defining constants for buffer and input sizes
// Maximum of 15 characters in the buffer
#define BUFFER_SIZE 15

// Input Size Limit: The input buffer is defined to hold up to 50 characters plus a null terminator, as per the requirement.
// NOTE: Maximum of 50 characters plus NULL terminator
#define INPUT_SIZE 51

// Define a structure to hold shared buffer and synchronization variables
typedef struct {
    char buffer[BUFFER_SIZE];       // The shared buffer to store characters
    int in;                         // Index for the producer to insert
    int out;                        // Index for the consumer to remove
    int count;                      // Number of items in the buffer to consume
    int producer_done;              // Flag to indicate producer is done to exit consumer
    
    // Mutex Lock for Synchronization: The program uses a mutex lock to synchronize access to the shared buffer between the producer and consumer threads.
    pthread_mutex_t mutex;
    
    /*
    **********************
    * CONDITION VARIABLES:
    **********************
        - The program uses pthread_cond_t condition variables not_empty and not_full to synchronize access to the shared buffer between producer and consumer threads.
    */
    pthread_cond_t not_empty;       // Condition variable for buffer not empty to consume
    pthread_cond_t not_full;        // Condition variable for buffer not full to produce
} shared_buffer_t;                  // Typedef for shared buffer structure to use as a type

// Declaring a shared buffer variable
shared_buffer_t shared_buffer;

// The producer reads characters from the input, adds them to the buffer, and signals the consumer.
void* producer(void* arg) {
    char* input = (char*) arg;      // Casting the argument to a character pointer
    int i = 0;                      // Indexing to iterate over the input string
    char item;                      // Variable to store the current character being processed

    // Loop over the input string until the null terminator is reached
    // Reading Characters One by One: The producer function reads characters from the input string one by one until it reaches the null terminator.
    while ((item = input[i]) != '\0') {                 // Read the current character from the input string
        pthread_mutex_lock(&shared_buffer.mutex);       // Lock the mutex to access the shared buffer
    
        // Protection with Condition Variables: The producer waits when the buffer is full, and the consumer waits when the buffer is empty, ensuring safe access without overfilling or underflowing the buffer.    
        // Looping until there is space in the buffer to add items
        while (shared_buffer.count == BUFFER_SIZE) {    
            // Waiting for the buffer to have space to add items
            pthread_cond_wait(&shared_buffer.not_full, &shared_buffer.mutex);
        }

        // Adding item to the buffer and update the indices
        shared_buffer.buffer[shared_buffer.in] = item;
        // Incrementing the index to add the next character
        shared_buffer.in = (shared_buffer.in + 1) % BUFFER_SIZE;
        // Incrementing the count of items in the buffer
        shared_buffer.count++;

        // Printing the produced item to the console
        printf("Produced: %c\n", item);

        // Signaling the consumer that the buffer is not empty
        pthread_cond_signal(&shared_buffer.not_empty);
        
        // Unlocking the mutex after adding the item
        pthread_mutex_unlock(&shared_buffer.mutex);
        
        // Increment the index to read the next character
        i++;
    }

    // Producer Completion Flag: A flag producer_done is set by the producer to indicate that it has finished producing all characters.
    // Producer is done
    // Locking the mutex before setting the flag
    pthread_mutex_lock(&shared_buffer.mutex);
    
    // Setting the producer done flag
    shared_buffer.producer_done = 1;

    // Waking up consumer if waiting for items
    pthread_cond_broadcast(&shared_buffer.not_empty);
    
    // Unlocking the mutex after setting the flag
    pthread_mutex_unlock(&shared_buffer.mutex);

    // Printing producer done message
    printf("Producer: done\n");
    
    // Exiting the producer thread
    return NULL;
}

// Consumer Functionality: The consumer function removes characters from the shared buffer one by one and prints “Consumed: x” for each character.
// The consumer waits for items in the buffer, removes them, and prints each character.
void* consumer(void* arg) {
    
    // By casting the arg parameter to (void), I am informing the compiler that I am intentionally not using this parameter, suppressing the warning.
    (void)arg;
    
    // Variable to store the consumed item from the buffer
    char item;

    // Consumer Loop: The consumer function loops indefinitely, consuming items from the buffer until the producer is done and the buffer is empty.
    while (1) {
        // Locking the mutex before consuming items
        pthread_mutex_lock(&shared_buffer.mutex);
        // Consumer Checks Completion Flag: The consumer checks the producer_done flag to determine when it should exit.
        // Notification Mechanism: The producer uses pthread_cond_broadcast to notify the consumer that production is complete.
        
        // Looping until there are items to consume in the buffer
        while (shared_buffer.count == 0) {
            // Checking if the producer is done and buffer is empty
            if (shared_buffer.producer_done) {
                // Signaling the producer that the buffer is not full
                pthread_mutex_unlock(&shared_buffer.mutex);
                // Exiting the consumer thread if producer is done
                printf("Consumer: done\n");
                // Exiting the consumer thread
                return NULL;
            }

            // Waiting for items to consume in the buffer
            pthread_cond_wait(&shared_buffer.not_empty, &shared_buffer.mutex);
        }

        // Removing item from buffer
        item = shared_buffer.buffer[shared_buffer.out];
        
        // Updating the out index of the buffer
        shared_buffer.out = (shared_buffer.out + 1) % BUFFER_SIZE;
        
        // Decreasing the count of items in the buffer
        shared_buffer.count--;
        
        // Printing the consumed item
        printf("Consumed: %c\n", item);
        
        // Signaling the producer that the buffer is not full
        pthread_cond_signal(&shared_buffer.not_full);
        
        // Unlocking the mutex after consuming
        pthread_mutex_unlock(&shared_buffer.mutex);
    }
}

// Main Function: The main function reads user input, creates producer and consumer threads, and waits for them to finish before processing the next input.
int main() {
    // Input Buffer: The program reads user input into a buffer of size 50 characters plus a null terminator, ensuring that the input does not exceed the buffer size.
    // Initializing the input buffer to store user input
    char input_buffer[INPUT_SIZE];

    // Initialize the shared buffer and synchronization primitives
    // Creation and Initialization: The shared buffer is part of the shared_buffer_t structure and is initialized in the main function.
    // Buffer Reset Between Inputs: The shared buffer variables are reset before processing each new input to ensure correct behavior.
    shared_buffer.in = 0;               // Initializing the buffer indices
    shared_buffer.out = 0;
    shared_buffer.count = 0;            // Initializing the buffer count
    shared_buffer.producer_done = 0;    // Initializing producer done flag
    
    // Synchronization: The threads use mutexes and condition variables to synchronize access, ensuring proper operation without race conditions or deadlocks.
    // Initialization of Condition Variables: The condition variables not_empty and not_full are initialized in the main function.
    
    // Initializing mutex to default attributes (NULL)
    pthread_mutex_init(&shared_buffer.mutex, NULL);

    // Initializing condition variables to default attributes (NULL)
    pthread_cond_init(&shared_buffer.not_empty, NULL);
    pthread_cond_init(&shared_buffer.not_full, NULL);

    // Main Loop: The main function reads user input, creates producer and consumer threads, and waits for them to finish before processing the next input.
    while (1) {
        // Printing prompt for user input
        printf("Enter input (type 'exit' to quit): ");
        // Input Handling: The fgets function reads up to INPUT_SIZE - 1 characters, ensuring that the input does not exceed 50 characters.
        if (fgets(input_buffer, INPUT_SIZE, stdin) == NULL) {
            // Error handling for reading input
            fprintf(stderr, "Error reading input\n");
            // Exiting the program with failure status
            continue;
        }

        // Check if the input was longer than 50 characters and discard extra characters
        size_t len = strlen(input_buffer);
        
        // Removing newline character if present
        if (len > 0 && input_buffer[len - 1] == '\n') {
            // Replacing newline with null terminator
            input_buffer[len - 1] = '\0';
            // Decreasing length by 1
            len--;
        
        // If the input is exactly 50 characters, it may contain a newline character
        } else {
            // Input too long, discard the rest
            int ch;
            // Reading characters until newline or EOF
            while ((ch = getchar()) != '\n' && ch != EOF);
        }

        // If the user types 'exit', exit the program
        if (strcmp(input_buffer, "exit") == 0) {
            // Exit the loop if the user types 'exit'
            printf("Parent: done\n");
            // Exiting the program
            break;
        }

        // Print input and character count
        printf("Input: %s\n", input_buffer);
        // Reset shared buffer variables
        printf("Count: %zu characters\n", len);

        // Resetting Shared Buffer: The shared buffer is reset before processing a new input to ensure correct operation.
        shared_buffer.in = 0;                   // Resetting the buffer indices
        shared_buffer.out = 0;
        shared_buffer.count = 0;                // Resetting the buffer count
        shared_buffer.producer_done = 0;        // Reset producer done flag
        
        // Clearing Shared Buffer Between Inputs: The shared buffer is cleared using memset before processing a new input to ensure no residual data affects the next operation.
        memset(shared_buffer.buffer, 0, sizeof(shared_buffer.buffer));
        
        // Producer and Consumer Threads: Two threads are created—one producer and one consumer—that communicate via the shared circular buffer.
        pthread_t prod_thread, cons_thread;
    
        // Duplicating Input Buffer for Thread Safety: The input buffer is duplicated using strdup to prevent the main thread from modifying it while the producer thread is still using it.
        char *producer_input = malloc(strlen(input_buffer) + 1);
        // Copying the input buffer to the duplicated buffer
        if (producer_input == NULL) {
            // Error handling for memory allocation
            fprintf(stderr, "Memory allocation error\n");
            // Exiting the program with failure status
            exit(EXIT_FAILURE);
        }
        strcpy(producer_input, input_buffer);
    
        // Thread Creation and Joining: The main function creates the producer and consumer threads and waits for them to finish using pthread_join.
        // Creating producer thread to produce items
        if (pthread_create(&prod_thread, NULL, producer, (void*) producer_input) != 0) {
            // Error handling for producer thread creation
            fprintf(stderr, "Error creating producer thread\n");
            // Exiting the program with failure status
            exit(EXIT_FAILURE);
        }
    
        // Creating consumer thread to consume the produced items
        if (pthread_create(&cons_thread, NULL, consumer, NULL) != 0) {
            // Error handling for consumer thread creation
            fprintf(stderr, "Error creating consumer thread\n");
            // Exiting the program with failure status
            exit(EXIT_FAILURE);
        }
    
        // Waiting for threads to finish before processing the next input
        pthread_join(prod_thread, NULL);
        pthread_join(cons_thread, NULL);
    
        // Free the duplicated input buffer to release memory and avoid leaks
        free(producer_input);
    }
    
    // Condition Variables must be Released: The condition variables and mutex are destroyed at the end of the main function to release resources.
    // Destroying mutex to release resources
    pthread_mutex_destroy(&shared_buffer.mutex);
    
    // Destroying condition variables to release resources
    pthread_cond_destroy(&shared_buffer.not_empty);
    pthread_cond_destroy(&shared_buffer.not_full);
    
    // Exiting the program    
    return 0;
}