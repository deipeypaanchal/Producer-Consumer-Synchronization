/* main.c

   This file contains the main function that initializes resources,
   handles user input, creates producer and consumer threads, and
   manages the overall flow of the program.
*/

#include "shared.h"

// Instantiate the shared buffer variable
shared_buffer_t shared_buffer;

int main() {
    char input_buffer[INPUT_SIZE];  // Buffer to store user input

    // Initialize the shared buffer indices and flags
    shared_buffer.in = 0;
    shared_buffer.out = 0;
    shared_buffer.count = 0;
    shared_buffer.producer_done = 0;

    // Initialize mutex and condition variables
    pthread_mutex_init(&shared_buffer.mutex, NULL);
    pthread_cond_init(&shared_buffer.not_empty, NULL);
    pthread_cond_init(&shared_buffer.not_full, NULL);

    while (1) {
        printf("Enter input (type 'exit' to quit): ");

        // Read input from the user
        if (fgets(input_buffer, INPUT_SIZE, stdin) == NULL) {
            fprintf(stderr, "Error reading input\n");
            continue;
        }

        // Remove the newline character if present
        size_t len = strlen(input_buffer);
        if (len > 0 && input_buffer[len - 1] == '\n') {
            input_buffer[len - 1] = '\0';
            len--;
        } else {
            // Input too long, discard the rest
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }

        // Check for exit condition
        if (strcmp(input_buffer, "exit") == 0) {
            printf("Parent: done\n");
            break;
        }

        // Display the input and character count
        printf("Input: %s\n", input_buffer);
        printf("Count: %zu characters\n", len);

        // Reset shared buffer indices and flags before starting new threads
        shared_buffer.in = 0;
        shared_buffer.out = 0;
        shared_buffer.count = 0;
        shared_buffer.producer_done = 0;
        memset(shared_buffer.buffer, 0, sizeof(shared_buffer.buffer));  // Clear the buffer

        pthread_t prod_thread, cons_thread;  // Thread identifiers

        // Duplicate the input buffer to pass to the producer thread
        char *producer_input = strdup(input_buffer);
        if (producer_input == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            exit(EXIT_FAILURE);
        }

        // Create the producer thread
        if (pthread_create(&prod_thread, NULL, producer, (void*) producer_input) != 0) {
            fprintf(stderr, "Error creating producer thread\n");
            free(producer_input);
            exit(EXIT_FAILURE);
        }

        // Create the consumer thread
        if (pthread_create(&cons_thread, NULL, consumer, NULL) != 0) {
            fprintf(stderr, "Error creating consumer thread\n");
            free(producer_input);
            exit(EXIT_FAILURE);
        }

        // Wait for both threads to finish execution
        pthread_join(prod_thread, NULL);
        pthread_join(cons_thread, NULL);

        // Free the duplicated input buffer
        free(producer_input);
    }

    // Destroy mutex and condition variables to release resources
    pthread_mutex_destroy(&shared_buffer.mutex);
    pthread_cond_destroy(&shared_buffer.not_empty);
    pthread_cond_destroy(&shared_buffer.not_full);

    return 0;
}