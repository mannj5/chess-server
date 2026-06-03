/**
 * @Author: jcmann
 * @Date:   2026-02-03 20:54
 * @Last Modified by:   jcmann
 * @Last Modified time: 2026-02-04 14:28
 */

#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

// exit statuses
#define USAGE_ERR 13
#define CONN_ERR 18
#define ENGINE_CRASH 17
#define COMMS_ERR 4
#define SUCCESS 0

// important constants
#define BUFFER_SIZE 1024
#define READ_END 0
#define WRITE_END 1
#define MAX_CONNECTIONS 10
#define MIN_ARGS 1
#define MAX_ARGS 3
#define MOVE_INDEX 5
#define COLOUR_LENGTH 5

typedef struct {
    int domain;
    int service;
    int protocol;
    unsigned long interface;
    int port;
    int backlog;
    struct sockaddr_in address;
    int socket;
} Server;

// NEW Function Prototypes ////////////////////////////////////////////////////////
void* client_thread(void* arg);
Server init_server(int domain, int service,
    int protocol, unsigned long interface, int port, int backlog);

///////////////////////////////////////////////////////////////////////////////

const char* const portErrorMsg
        = "uqchessserver: unable to start listening on port \"%s\"\n";
const char* const usageErrorMsg
        = "Usage: ./uqchessserver [--listenOn portno]\n";
const char* const engineFailureErrorMsg
        = "uqchessserver: chess engine exited unexpectedly\n";
const char* const engineComErrorMsg
        = "uqchessserver: cannot start communication with chess engine\n";
const char* const listenOnArg = "--listenOn";
const char* const resignMsg = "gameover resignation %s\n";
const char* const startpos
        = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

typedef enum {
    BLACK,
    WHITE,
    EITHER,
} Colour;

typedef enum {
    COMPUTER,
    HUMAN,
} Opponent;

typedef struct {
    int opponent;
    int colour;
} GameType;

// contains the file wrapped pipes for communicating with stockfish
typedef struct {
    FILE* in;
    FILE* out;
} EngineFiles;

// all of the information necessary for passing to a thread
typedef struct {
    FILE* in;
    FILE* out;
    FILE* oppIn;
    int colour;
    EngineFiles* engineIO;
    char* gameState;
    sem_t lock;
} Client;

// OLD Function Prototypes ////////////////////////////////////////////////////////
/*
void usage_error(void);
char* process_command_line(int argc, char* argv[]);
GameType process_connection(int connfd);
int attempt_port_connection(char* port);
void start_client_thread(
        int clientfd, int oppfd, int colour, EngineFiles engineIO);
void* client_vs_computer(void* args);
void* client_vs_client(void* args);
void print_board(char* fenString, EngineFiles* engineIO, FILE* clientIn);
StockfishGameState* get_board_state(char* fenString, EngineFiles* engineIO);
char* computer_move(char* fenString, EngineFiles* engineIO, FILE* clientIn);
char* handle_move(char* fenString, EngineFiles* engineIO, FILE* clientIn,
        char* move, int colour);
void send_and_read_ucinewgame(EngineFiles* engineIO);
void print_bestmove(char* fenString, EngineFiles* engineIO, FILE* clientIn);
void print_all_moves(char* fenString, EngineFiles* engineIO, FILE* clientIn);
void print_moves(ChessMoves* cm, FILE* clientIn);
char* get_initial_board_state(EngineFiles* engineIO);
bool is_gameover(char* fenString, EngineFiles* engineIO, FILE* clientIn);
EngineFiles start_chess_engine(void);
void close_and_wait(EngineFiles engineIO);
void send_and_read_ready_commands(FILE* in, FILE* out);
void send_start_message(FILE* clientIn, bool isWhite);
*/
	
///////////////////////////////////////////////////////////////////////////////

#endif /* __SERVER_H__ */
