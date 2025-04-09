#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dekoder_lib.h"

#define CODE_LENGTH 4
#define COLOR_COUNT 6
#define MAX_ATTEMPTS 12

int main() {
    int parent_to_child[2];  // Pipe from parent to child
    int child_to_parent[2];  // Pipe from child to parent

    // Create pipes
    if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1) {
        perror("Pipe creation failed");
        exit(1);
    }

    // Create a new game state
    GameState *state = new_game_state();

    // Fork the process
    pid_t pid = fork();

    if (pid == -1) {
        perror("Fork failed");
        exit(1);
    }

    if (pid == 0) {  // Child process
        // Close unused pipes
        close(parent_to_child[1]);
        close(child_to_parent[0]);

        // Read the secret code from the parent process
        int secret_code[CODE_LENGTH];
        read(parent_to_child[0], secret_code, sizeof(secret_code));

        // Generate guesses and send them to the parent
        for (int i = 0; i < MAX_ATTEMPTS; ++i) {
            char guess[CODE_LENGTH + 1] = {0};
            guess(state, guess);  // Generate a guess

            // Send the guess to the parent process
            write(child_to_parent[1], guess, sizeof(guess));
        }

        // Close pipes before exiting
        close(parent_to_child[0]);
        close(child_to_parent[1]);
        exit(0);
    } else {  // Parent process
        // Close unused pipes
        close(parent_to_child[0]);
        close(child_to_parent[1]);

        // Send the secret code to the child
        write(parent_to_child[1], state->correctBoard, sizeof(state->correctBoard));

        // Receive guesses from the child and provide feedback
        for (int i = 0; i < MAX_ATTEMPTS; ++i) {
            char guess[CODE_LENGTH + 1] = {0};
            read(child_to_parent[0], guess, sizeof(guess));  // Receive guess from child

            // Provide feedback to the guess
            on_feedback(state);
            char* feedback = getGuessedStatus(state);
            
            // If the guess is correct, break the loop
            if (feedback[0] == CODE_LENGTH) {
                printf("Dekoder zgadł kombinację w %d próbach!\n", getAttempts(state));
                break;
            }
        }

        // Wait for the child process to finish
        wait(NULL);

        // Free the allocated memory
        destroy_game_state(state);

        // Close pipes before exiting
        close(parent_to_child[1]);
        close(child_to_parent[0]);
    }

    return 0;
}
