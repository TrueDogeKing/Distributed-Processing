#ifndef DEKODER_LIB_H
#define DEKODER_LIB_H

typedef struct GameState GameState;  


GameState* new_game_state();
void guess(GameState *state,int* guesscolors,char* feedback);
void destroy_game_state(GameState *state);
int getAttempts(GameState *state);

#endif
