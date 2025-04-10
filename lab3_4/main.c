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

pid_t pid;
void handle_signal(int){
    kill(pid, SIGKILL);
    exit(0);
    
}



void generateBoard(int* correctBoard){
    for(int i=0;i<CODE_LENGTH;i++){
        correctBoard[i]=rand()%COLOR_COUNT; 
    }
}

void feedbackCoder(int* guesscolors,char* feedback,int* correctBoard) {
    int correctPlaces=0, correctColors=0;
    for(int i=0;i<CODE_LENGTH;i++){
        if(guesscolors[i]==correctBoard[i])
            correctPlaces++;
        else{
            for(int j=0;j<CODE_LENGTH;j++){
                if(i==j)
                    continue;
                if(guesscolors[i]==correctBoard[j]){
                    correctColors++;
                    break;
                    }
            }
        }

    }
    feedback[0]=correctPlaces;
    feedback[1]=correctColors;
}

int main() {
    int coder_to_decoder[2];  // Pipe from coder to decoder (for the secret code)
    int decoder_to_coder[2];  // Pipe from decoder to code (for guesses and feedback)

    // Create pipes
    if (pipe(coder_to_decoder) == -1 || pipe(decoder_to_coder) == -1) {
        perror("Pipe creation failed");
        exit(1);
    }

    // Create a new game state
    GameState *state = new_game_state();

    // Fork the process
    pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        exit(1);
    }

    if (pid == 0) {  // Decoder process
        // Close unused pipes
        close(coder_to_decoder[1]);
        close(decoder_to_coder[0]);

        int guessCode[CODE_LENGTH] = {0};  // Declare guess properly
        char feedback[2];  // Fixed-size array for feedback: 2 integers (correct colors, correct positions)

        // Decoder generates guesses and sends them to the parent
        for (int i = 0; i < MAX_ATTEMPTS; ++i) {
            guess(state, guessCode,feedback);  // Generate a guess using the guess function

            // Send the guess to the parent process
            write(decoder_to_coder[1], guessCode, sizeof(guessCode));


            // Wait for feedback from the parent process
            read(coder_to_decoder[0], feedback, sizeof(feedback));

           printf("Decoder read feedback number:%d correct place(s), %d in correct colors(s)\n",feedback[0], feedback[1]);

            // If feedback indicates the guess is correct, exit the loop
            if (feedback[0] == CODE_LENGTH) {
                printf("Success secret code guessed\n");
                break;
            }
        }

        // Close pipes before exiting
        close(coder_to_decoder[0]);
        close(decoder_to_coder[1]);
        exit(0);
    } else {  // Coder process
        int correctBoard[CODE_LENGTH];
        for(int i=0;i<7;i++)
            generateBoard(correctBoard);
        printf("correct board ");
        for(int i=0;i<CODE_LENGTH;i++){
            printf("%d ", correctBoard[i]);
        }
        printf("\nstart game\n");
        // Close unused pipes
        close(coder_to_decoder[0]);
        close(decoder_to_coder[1]);

        int guessCode[CODE_LENGTH] = {0};
        char feedback[2];  // Fixed-size array for feedback: 2 integers (correct colors, correct positions)

        // Receive guesses from the child and provide feedback
        for (int i = 0; i < MAX_ATTEMPTS; ++i) {

            // Receive guess from the child
            read(decoder_to_coder[0], guessCode, sizeof(guessCode));
            printf("Coder read guess number: ");
            for (int j = 0; j < CODE_LENGTH; j++) {
                printf("%d ", guessCode[j]);
            }
            printf("\n");

            feedbackCoder(guessCode,feedback,correctBoard);
            
            

            // Send feedback to the Dekoder
            write(coder_to_decoder[1], feedback, sizeof(feedback));
            
            // If the guess is correct, break the loop
            if (feedback[0] == CODE_LENGTH) {
                printf("Dekoder guessed the combination!\n");
                break;
            }
        }

        // Wait for the child process to finish
        printf("waiting for child");
        wait(NULL);

        // Free the allocated memory
        destroy_game_state(state);

        // Close pipes before exiting
        close(coder_to_decoder[1]);
        close(decoder_to_coder[0]);
    }

    return 0;
}

