#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dekoder_lib.h"
#include <time.h>

#define CODE_LENGTH 4
#define COLOR_COUNT 6
#define MAX_ATTEMPTS 12

typedef struct GameState {
    int attempt;
    int guesses[MAX_ATTEMPTS][CODE_LENGTH];
    int correctBoard[CODE_LENGTH];
    char guessedStatus[2];
} GameState;


GameState *new_game_state() {
    srand(time(NULL));
    GameState *state = malloc(sizeof(GameState));
    state->attempt = 0;
    printf("correct board ");
    for(int i=0;i<CODE_LENGTH;i++){
        state->correctBoard[i]=rand()%COLOR_COUNT; 
        printf("%d ", state->correctBoard[i]);
    }
    printf("\n");

    return state;
}

// Free the memory allocated for the GameState
void destroy_game_state(GameState *state) {
    if (state) {
        free(state);
    }
}


void guess(GameState *state, char *guesscolors) {
    for(int i=0;i<CODE_LENGTH;i++){
        state->guesses[state->attempt][i]=rand()%COLOR_COUNT; 
    }
    //debug
    printf("new guess %d: Guessing ", state->attempt);
    for (int i = 0; i < CODE_LENGTH; ++i)
        printf("%d ", state->guesses[state->attempt][i]);
    printf("\n");
}

void on_feedback(GameState *state) {
    int correctPlaces=0, correctColors=0;
    for(int i=0;i<CODE_LENGTH;i++){
        if(state->guesses[state->attempt][i]==state->correctBoard[i])
            correctPlaces++;
        else{
            for(int j=0;j<CODE_LENGTH;j++){
                if(i==j)
                    continue;
                if(state->guesses[state->attempt][i]==state->correctBoard[j])
                    correctColors++;
            }
        }

    }
    printf("Feedback: %d correct color(s), %d in correct place(s)\n", correctColors, correctPlaces);
    state->guessedStatus[0]=correctPlaces;
    state->guessedStatus[1]=correctColors;
    state->attempt++;
    // Można dodać eliminację kombinacji, które nie pasują – zaawansowana wersja.
    // Obecna wersja to brute-force, więc ignoruje feedback.
}

int getAttempts(GameState *state){
    return state->attempt;
}
char* getGuessedStatus(GameState *state){
    return state->guessedStatus;
}
