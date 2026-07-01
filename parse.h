#ifndef PARSE_H
#define PARSE_H

#define MAX_MOVES 256
#define MAX_FEN 128
#define MAX_CHECKS 64
#define BOARD_SIZE 656

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    const char board[BOARD_SIZE];
    const char fen[MAX_FEN];
    const uint64_t key;
    char** checks;
} GameState;

typedef struct {
    char** movelist;
    char* best_move;
} ChessMoves;

char** split_by_char(char* str, char split, unsigned int maxFields);

GameState* read_d_output(FILE* stream);

void free_game_state(GameState* state);

ChessMoves* read_go_perft_1_output(FILE* stream);

ChessMoves* read_bestmove_output(FILE* stream);

void free_chess_moves(ChessMoves* moves);

char next_player_from_fen_string(const char* fen);

#endif
