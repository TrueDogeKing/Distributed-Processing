#include "dekoder_lib.h"
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define CODE_LENGTH 4
#define COLOR_COUNT 6
#define MAX_ATTEMPTS 12

typedef struct {
    int code[CODE_LENGTH];
    char feedback[2];
} Attempt;

struct GameState {
    int attempt_count;
    Attempt history[MAX_ATTEMPTS];
    bool possible_colors[CODE_LENGTH][COLOR_COUNT];
};

GameState* new_game_state() {
    GameState* state = malloc(sizeof(GameState));
    if (!state) return NULL;
    
    state->attempt_count = 0;
    srand(time(NULL));
    
    // Initialize all colors as possible in all positions
    for (int i = 0; i < CODE_LENGTH; i++) {
        for (int j = 0; j < COLOR_COUNT; j++) {
            state->possible_colors[i][j] = true;
        }
    }
    
    return state;
}

void destroy_game_state(GameState* state) {
    free(state);
}

bool is_possible(const GameState* state, const int* guess) {
    // Check against all previous attempts
    for (int i = 0; i < state->attempt_count; i++) {
        const Attempt* prev = &state->history[i];
        int correct_pos = 0;
        int correct_col = 0;
        
        // Count correct positions
        for (int j = 0; j < CODE_LENGTH; j++) {
            if (guess[j] == prev->code[j]) {
                correct_pos++;
            }
        }
        
        // Count correct colors (wrong positions)
        for (int c = 0; c < COLOR_COUNT; c++) {
            int count_guess = 0;
            int count_prev = 0;
            
            for (int j = 0; j < CODE_LENGTH; j++) {
                if (guess[j] == c) count_guess++;
                if (prev->code[j] == c) count_prev++;
            }
            
            correct_col += (count_guess < count_prev) ? count_guess : count_prev;
        }
        correct_col -= correct_pos;
        
        // Check if this guess matches previous feedback
        if (correct_pos != prev->feedback[0] || correct_col != prev->feedback[1]) {
            return false;
        }
    }
    return true;
}

void guess(GameState* state, int* guess, char* feedback) {
    // Store previous feedback if available
    if (state->attempt_count > 0) {
        state->history[state->attempt_count-1].feedback[0] = feedback[0];
        state->history[state->attempt_count-1].feedback[1] = feedback[1];
    }

    // Generate random guesses until we find a possible one
    int attempts = 0;
    do {
        for (int i = 0; i < CODE_LENGTH; i++) {
            guess[i] = rand() % COLOR_COUNT;
        }
        attempts++;
        
        // Prevent infinite loops
        if (attempts > 1000) break;
    } while (state->attempt_count > 0 && !is_possible(state, guess));

    // Record this attempt
    for (int i = 0; i < CODE_LENGTH; i++) {
        state->history[state->attempt_count].code[i] = guess[i];
    }
    state->attempt_count++;
}

int getAttempts(GameState* state) {
    return state->attempt_count;
}
