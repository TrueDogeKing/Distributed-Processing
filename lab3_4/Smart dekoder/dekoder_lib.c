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


void guess(GameState *state, int *guess,char* feedback) {\
    int correctPlaces = 0;
    int correctColors = 0;
    int isGuessPlausible = 0;
    int localGuess[CODE_LENGTH];

    //write feedback from last attempt
    if(state->attempt!=0)
        for(int i=0;i<2;i++)
            state->feedbackHistory[state->attempt-1][i]=feedback[i];
    
    //smart version
    int counter=0;
    while (!isGuessPlausible)
    {
    
        counter++;
        if(counter==10000){
            printf("too long loop\n");
            break;
        }
        for (int i = 0; i < CODE_LENGTH; i++)
        {
            localGuess[i] = rand() % COLOR_COUNT;
        }
        if (state->attempt == 0)
        {
            break;
        }

        for (int i = 0; i < state->attempt; i++)
        {
            for (int j = 0; j < CODE_LENGTH; j++)
            {
                if (state->boardHistory[i][j] == localGuess[j])
                {
                    ++correctPlaces;
                }

            }
            if (correctPlaces != state->feedbackHistory[i-1][1])
            {
                --isGuessPlausible;
            }
            correctPlaces = 0;


            for (int j = 0; j < CODE_LENGTH; j++)
            {
                for (int k = 0; k < CODE_LENGTH; k++)
                {
                    if (state->boardHistory[i][j] == localGuess[k])
                    {
                        ++correctColors;
                        break;
                    }
                }
            }
            if (correctColors != state->feedbackHistory[i-1][0])
            {
                --isGuessPlausible;
            }
            correctColors = 0;

        }
        
        if (isGuessPlausible == 0)
        {
            break;
        }
        isGuessPlausible = 0;
    }



    for(int i=0;i<CODE_LENGTH;i++){
        guess[i] = localGuess[i];
        state->boardHistory[state->attempt][i]=guess[i];
    }
    state->attempt++;

}

int getAttempts(GameState *state){
    return state->attempt;
}
