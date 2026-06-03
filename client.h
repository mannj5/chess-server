/**
 * @Author: jcmann
 * @Date:   2026-02-03 20:54
 * @Last Modified by:   jcmann
 * @Last Modified time: 2026-02-04 14:11
 */

#ifndef CLIENT_H
#define CLIENT_H

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

// define exit statuses
#define USAGE_ERR 13
#define SOCKET_ERR 12
#define SUCCESS 0
#define SERVER_CLOSED 3

#define MAX_BUFFER_SIZE 80
#define NUM_COMMANDS 6
#define MAX_ARGS 6
#define MIN_ARGS 2
#define MAX_COMMAND_LENGTH 10
#define MOVE_INDEX 5
#define COLOUR_INDEX 8

typedef struct {
    int port;
    int socket;
    char* colour;
    char* opponent;
    bool isMyTurn;
    bool gameActive;
    sem_t lock;
} Parameters;

// Function Prototypes ////////////////////////////////////////////////////////
void usage_error(void);
void connection_error(int port);
Parameters* init_params(char* argv[]);
Parameters* process_command_line(int argc, char* argv[]);
int attempt_port_connection(int port);
bool check_command_valid(char* command);
void* server_write(void* args);
void* server_read(void* args);
bool is_alphanumeric(char* str);

///////////////////////////////////////////////////////////////////////////////

const char* const welcomeMsg
        = "Welcome to UQChessClient - written by s4802487\n";
const char* const noGameErrorMsg
        = "Invalid command - game is not in progress\n";
const char* const wrongTurnErrorMsg = "Command is not valid - not your turn\n";
const char* const socketErrorMsg
        = "uqchessclient: unable to connect to port \"%d\"\n";
const char* const serverCloseErrorMsg
        = "uqchessclient: server connection closed\n";
const char* const usageErrorMsg = "Usage: uqchessclient portnum [--col "
                                  "black|white] [--play machine|person]\n";
const char* const invalidCommandErrorMsg = "Try again - command is not valid\n";
const char* const colourArg = "--col";
const char* const playerArg = "--play";
const char* const hostName = "localhost";
const char* const clientCmdList[6]
        = {"print", "hint", "possible", "resign", "quit", "newgame"};

#endif /* __CLIENT_H__ */
