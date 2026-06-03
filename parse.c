
#include "parse.h"

GameState* read_d_output(FILE* stream)
{
    GameState* gs = malloc(sizeof(GameState));

    // ASCII board
    fscanf(stream, "%656[^F]", gs->board);
    // fen string
    fscanf(stream, "Fen: %256[^\n]\n", gs->fen);
    // board hash
    fscanf(stream, "Key: %x\n", &(gs->key));
    // any checks?
    char* line = NULL;
    size_t size;
    ssize_t nbytes = getline(&line, &size, stream);
    if (nbytes > 11) {

        int numChecks = (nbytes - 11) / 3;
        gs->checks = malloc(sizeof(char) * (numChecks + 1));
        gs->checks[numChecks] = NULL;

        char delims[] = " \n";
        char* tok = strtok(line + 10, delims);

        int i = 0;
        while (tok != NULL) {
            gs->checks[i] = malloc(sizeof(char) * 3);
            strcpy(gs->checks[i++], tok);
            // fprintf(stderr, "%s\n", tok);
            tok = strtok(NULL, delims);
        }
    } else { // empty checks line
        gs->checks = NULL;
    }
    
    // cleanup after getline
    free(line);

    return gs;
}

void free_game_state(GameState* state)
{
    if (state->checks) {
        for (int i = 0; *(state->checks + 1); i++) {
            free(*(state->checks + i));
        }
        free(state->checks);
    }
    free(state);
}

ChessMoves* read_go_perft_1_output(FILE* stream)
{
    ChessMoves* cm = malloc(sizeof(ChessMoves));
    cm->best_move = NULL;
    cm->movelist = NULL;
    int size = 0;
    int capacity = 0;

    char* line = NULL;
    size_t nbytes;
    // just keep reading input
    while (getline(&line, &nbytes, stream) > 0) {

        // parse 5 characters
        char move[6] = {0};
        if (sscanf(line, "%5[^:]:", move) == 1) {
            // if it's a move, add it
            if (move[0] >= 'a' && move[0] <= 'h') {
                if (size == 0) {
                    cm->movelist = malloc(sizeof(char*));
                    capacity++;
                }
                if (size == capacity) {
                    capacity *= 2;
                    cm->movelist = realloc(cm->movelist, sizeof(char*) * capacity);
                }
                cm->movelist[size] = malloc(sizeof(char) * 6);
                strcpy(cm->movelist[size], move);
                size++;
            }
        }

        // this is the last line in the output
        if (line[0] == 'N') {
            getline(&line, &nbytes, stream);
            cm->movelist = realloc(cm->movelist, sizeof(char*) * (size + 1));
            cm->movelist[size] = NULL;
            break;
        }
    }

    fprintf(stderr, "read %d moves\n", size);

    free(line);
    return cm;
}

void free_chess_moves(ChessMoves* moves)
{
    if (moves->best_move) {
        free(moves->best_move);
    }
    if (moves->movelist) {
        for (int i = 0; *(moves->movelist + i); i++) {
            free(*(moves->movelist + i));
        }
        free(moves->movelist);
    }
    free(moves);
}

ChessMoves* read_bestmove_output(FILE* stream)
{
    ChessMoves* cm = malloc(sizeof(ChessMoves));

    cm->movelist = NULL;
    cm->best_move = NULL;

    return cm;
}