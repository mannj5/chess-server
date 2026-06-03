/**
 * @Author: jcmann
 * @Date:   2026-02-03 20:54
 * @Last Modified by:   jcmann
 * @Last Modified time: 2026-02-04 14:17
 */

#include "client.h"

int main(int argc, char* argv[])
{
    // get a struct contaning all of the command line info
    Parameters* params = process_command_line(argc, argv);
    if (!strcmp(params->colour, "either")
            && !strcmp(params->opponent, "computer")) {
        params->colour = "white";
    }
    params->socket = attempt_port_connection(params->port);
    printf(welcomeMsg);

    pthread_t wtid;
    void* writeStatus;
    pthread_create(&wtid, NULL, server_write, (void*)params);

    pthread_t rtid;
    void* readStatus;
    pthread_create(&rtid, NULL, server_read, (void*)params);

    pthread_join(wtid, &writeStatus);
    pthread_join(rtid, &readStatus);
    free(params);
}

/* usage_error()
 *      Prints a usage error message to stderr and exits accordingly.
 */
void usage_error(void)
{
    fprintf(stderr, usageErrorMsg);
    exit(USAGE_ERR);
}

/* connection_error()
 *      Prints a connection error message to stderr and exits accordingly.
 */
void connection_error(int port)
{
    fprintf(stderr, socketErrorMsg, port);
    exit(SOCKET_ERR);
}

/* init_params()
 *      helper function to initialise a pointer to a struct of default
 *      parameters that can be specified on the command line.
 */
Parameters* init_params(char* argv[])
{
    Parameters* params = malloc(sizeof(Parameters));
    params->port = atoi(argv[1]);
    params->colour = "either";
    params->opponent = "computer";
    params->isMyTurn = false;
    params->gameActive = false;
    sem_init(&params->lock, 0, 1);
    return params;
}

/* process_command_line()
 *      Parses the arguments specified on the command_line, and calls
 *      usage_error() if there are empty arguments, incomplete arguments or
 *      unfamiliar arguments. Returns a struct of the parameters given.
 */
Parameters* process_command_line(int argc, char* argv[])
{
    if (argc < MIN_ARGS || argc > MAX_ARGS) {
        usage_error();
    }

    Parameters* params = init_params(argv);
    bool player = false;
    bool colour = false;
    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], colourArg) && !colour && argv[i + 1]) {
            char* colour = argv[i + 1];
            if (!strcmp(colour, "white") || !strcmp(colour, "black")) {
                params->isMyTurn = !strcmp(colour, "white") ? true : false;
                params->colour = colour;
                i++;
            } else {
                usage_error();
            }
        } else if (!strcmp(argv[i], playerArg) && !player && argv[i + 1]) {
            char* opponent = argv[i + 1];
            if (!strcmp(opponent, "machine") || !strcmp(opponent, "person")) {
                params->opponent
                        = !strcmp(opponent, "person") ? "human" : "computer";
                i++;
            } else {
                usage_error();
            }
        } else {
            usage_error();
        }
    }
    return params;
}

/* attempt_port_connection()
 *      Creates a socket and connects it to the specified port number.
 *      returns the file descriptor of the socket if successful. Otherwise,
 *      call connection_error().
 */
int attempt_port_connection(int port)
{
    int sockfd;
    struct sockaddr_in address;
    // Use server address details
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    // Create a socket and check if it was successful
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        connection_error(port);
    }
    if (connect(sockfd, (struct sockaddr*)&address, sizeof(address))) {
        connection_error(port);
    }
    return sockfd;
}

/* check_command_valid()
 *      Validates that the command read from STDIN is valid i.e. it matches
 *      one of the possible commands. If it is a move command, the move
 *      must be alphanumeric and within 4-5 characters in length. Returns true
 *      if the move is valid, false otherwise.
 */
bool check_command_valid(char* command)
{
    for (int i = 0; i < NUM_COMMANDS; i++) {
        if (!strcmp(command, clientCmdList[i])) {
            return true;
        }
    }
    if (!strncmp(command, "move ", strlen("move "))
            && strlen(command) > MOVE_INDEX
            && strlen(command) < MAX_COMMAND_LENGTH
            && is_alphanumeric(command + MOVE_INDEX)) {
        return true;
    }
    fprintf(stderr, invalidCommandErrorMsg);
    return false;
}

/* server_write()
 *      Reads input from STDIN and sends it to the server if the command is
 *      valid. The while loop is broken either when STDIN is closed or
 *      when the "quit" command is read.
 */
void* server_write(void* args)
{
    Parameters* params = (Parameters*)args;
    FILE* to = fdopen(params->socket, "w");
    fprintf(to, "start %s %s\n", params->opponent, params->colour);
    fflush(to);
    char line[MAX_BUFFER_SIZE];
    while (fgets(line, sizeof(line), stdin) != NULL) {
        line[strlen(line) - 1] = '\0';
        if (!check_command_valid(line)) {
            continue;
        }
        sem_wait(&params->lock);
        if (!strcmp(line, "newgame")) {
            fprintf(to, "start %s %s\n", params->opponent, params->colour);
        } else if (!strcmp(line, "quit")) {
            exit(SUCCESS);
        } else if (params->gameActive) {
            if (!strcmp(line, "resign")) {
                fprintf(to, "resign\n");
            } else if (!strcmp(line, "print")) {
                fprintf(to, "board\n");
            } else if (params->isMyTurn) {
                if (!strcmp(line, "hint")) {
                    fprintf(to, "hint best\n");
                } else if (!strcmp(line, "possible")) {
                    fprintf(to, "hint all\n");
                } else if (!strncmp(line, "move ", strlen("move "))) {
                    fprintf(to, "%s\n", line);
                    params->isMyTurn = false;
                } else {
                    fprintf(stderr, invalidCommandErrorMsg);
                }
            } else {
                fprintf(stderr, wrongTurnErrorMsg);
            }
        } else {
            fprintf(stderr, noGameErrorMsg);
        }
        fflush(to);
        sem_post(&params->lock);
    }
    fclose(to);
    exit(SUCCESS);
    return NULL;
}

/* server_read()
 *      Blocks until input is received from the server. Changes the game state
 *      (if there is a game and if it is the client's turn) for server_write()
 *      to access when processing input from the user.
 */
void* server_read(void* args)
{
    Parameters* params = (Parameters*)args;
    FILE* from = fdopen(params->socket, "r");
    char line[MAX_BUFFER_SIZE];
    while (fgets(line, sizeof(line), from)) {
        sem_wait(&params->lock);
        line[strlen(line) - 1] = '\0';
        if (!strncmp(line, "moved", strlen("moved"))
                || !strncmp(line, "error", strlen("error"))) {
            params->isMyTurn = true;
        }
        if (!strncmp(line, "started", strlen("started"))) {
            params->gameActive = true;
            params->isMyTurn
                    = !strcmp(line + COLOUR_INDEX, "white") ? true : false;
        }
        sem_post(&params->lock);
        // won't print startboard or endboard
        if (strcmp(line, "startboard") && strcmp(line, "endboard")) {
            printf("%s\n", line);
        }
    }

    fclose(from);
    fprintf(stderr, serverCloseErrorMsg);
    exit(SERVER_CLOSED);
    return NULL;
}

/* is_alphanumeric()
 *      Helper function for determining string format validity. Checks each
 *      character in the given string and returns false if any of them are not
 *      alphanumeric.
 */
bool is_alphanumeric(char* str)
{
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (!isalnum(str[i])) {
            return false;
        }
    }
    return true;
}
