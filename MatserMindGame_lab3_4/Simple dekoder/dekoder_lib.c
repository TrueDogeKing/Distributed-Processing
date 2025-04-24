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
    char feedbackHistory[MAX_ATTEMPTS+1][2];
    int boardHistory[MAX_ATTEMPTS+1][CODE_LENGTH];
    
} GameState;


GameState *new_game_state() {
    srand(time(NULL));
    GameState *state = malloc(sizeof(GameState));
    state->attempt = 0;

    return state;
}

// Free the memory allocated for the GameState
void destroy_game_state(GameState *state) {
    if (state) {
        free(state);
    }
}


void guess(GameState *state, int *guess,char* feedback) {
    for(int i=0;i<2;i++){
        state->feedbackHistory[state->attempt-1][i]=feedback[i];
    }

    for (int i = 0; i < CODE_LENGTH; i++) {
            guess[i] = rand() % COLOR_COUNT;
        }

    for(int i=0;i<CODE_LENGTH;i++){
        state->boardHistory[state->attempt][i]=guess[i];
    }
    state->attempt++;

}

int getAttempts(GameState *state){
    return state->attempt;
}
