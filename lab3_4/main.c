#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>  // For wait()
#include "dekoder_lib.h"
#include <stdbool.h>

#define CODE_LENGTH 4
#define COLOR_COUNT 6
#define MAX_ATTEMPTS 12

bool running=false;
// Function to display available commands
void handle_signal(int){
    if(running) return;
    else exit(0);
    
}

int main() {
    signal(SIGINT, handle_signal);
    int parent_to_child[2];  // Pipe from parent to child (for the secret code)
    int child_to_parent[2];  // Pipe from child to parent (for guesses and feedback)

    // Create pipes
    if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1) {
        perror("Pipe creation failed");
        exit(1);
    }

    // Create a new game state
    GameState *state = new_game_state();

    // Fork the process
    pid_t pid = fork();
    running=true;
    if (pid == -1) {
        perror("Fork failed");
        exit(1);
    }

    if (pid == 0) {  // Child process
        // Close unused pipes
        close(parent_to_child[1]);
        close(child_to_parent[0]);

        int guessCode[CODE_LENGTH] = {0};  // Declare guess properly
        char feedback[2];  // Fixed-size array for feedback: 2 integers (correct colors, correct positions)

        // Child generates guesses and sends them to the parent
        for (int i = 0; i < MAX_ATTEMPTS; ++i) {

            guess(state, guessCode);  // Generate a guess using the guess function

            // Send the guess to the parent process
            write(child_to_parent[1], guessCode, sizeof(guessCode));


            // Wait for feedback from the parent process
            read(parent_to_child[0], feedback, sizeof(feedback));

            printf("child read feedback: %d correct place(s), %d in correct colors(s)\n", feedback[0], feedback[1]);

            // If feedback indicates the guess is correct, exit the loop
            if (feedback[0] == CODE_LENGTH) {
                break;
            }
        }

        // Close pipes before exiting
        close(parent_to_child[0]);
        close(child_to_parent[1]);
        running=false;
        exit(0);
    } else {  // Parent process
        // Close unused pipes
        close(parent_to_child[0]);
        close(child_to_parent[1]);

        int guessCode[CODE_LENGTH] = {0};
        char feedback[2];  // Fixed-size array for feedback: 2 integers (correct colors, correct positions)

        // Receive guesses from the child and provide feedback
        for (int i = 0; i < MAX_ATTEMPTS; ++i) {

            // Receive guess from the child
            read(child_to_parent[0], guessCode, sizeof(guessCode));
            printf("parent read guess: ");
            for (int j = 0; j < CODE_LENGTH; j++) {
                printf("%d ", guessCode[j]);
            }
            printf("\n");

            // Provide feedback to the guess
            on_feedback(state, guessCode,feedback);
            

            // If the guess is correct, break the loop
            if (feedback[0] == CODE_LENGTH) {
                printf("Dekoder guessed the combination in %d attempts!\n", getAttempts(state));
                break;
            }

            // Send feedback to the child
            write(parent_to_child[1], feedback, sizeof(feedback));
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

