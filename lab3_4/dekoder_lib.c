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
    int correctBoard[CODE_LENGTH];
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


void guess(GameState *state, int *guess) {
    for(int i=0;i<CODE_LENGTH;i++){
        guess[i]=rand()%COLOR_COUNT; 
    }
}

void on_feedback(GameState *state,int* guesscolors,char* feedback) {
    int correctPlaces=0, correctColors=0;
    for(int i=0;i<CODE_LENGTH;i++){
        if(guesscolors[i]==state->correctBoard[i])
            correctPlaces++;
        else{
            for(int j=0;j<CODE_LENGTH;j++){
                if(i==j)
                    continue;
                if(guesscolors[i]==state->correctBoard[j]){
                    correctColors++;
                    break;
                    }
            }
        }

    }
    feedback[0]=correctPlaces;
    feedback[1]=correctColors;
    state->attempt++;
}

int getAttempts(GameState *state){
    return state->attempt;
}
