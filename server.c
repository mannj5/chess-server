/**
 * @Author: jcmann
 * @Date:   2026-02-03 20:54
 * @Last Modified by:   jcmann
 * @Last Modified time: 2026-02-04 14:28
 */

#include "server.h"

// Signal handler function
void signalHandler(int sig) {
    printf("=== SERVER CLOSED ===\n");
    exit(sig);
}


int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: ./server PORT\n");
        exit(1);
    }

    // Handle signal
    signal(SIGINT, signalHandler);

    int port = atoi(argv[1]);
    Server server = init_server(AF_INET, SOCK_STREAM, 0, INADDR_ANY, port, 10);

    printf("=== SERVER STARTED ===\n");
    int addrlen = sizeof(server.address);
    int new_socket;
    // Blocks on this call until a connection is established with a client
    while (new_socket = accept(server.socket, (struct sockaddr*)&server.address, (socklen_t*)&addrlen),
        new_socket >= 0) {
        // Create a new thread for the client
        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, &new_socket);
        pthread_detach(tid);
    }

    return 0;
}

void* client_thread(void* arg) {
    // Typecast back into integer
    int client_socket = *((int*) arg);
    printf("Client %d connected.\n", client_socket);
    dprintf(client_socket, "> ");
    
    // Keep track of if a game is active
    bool active = false;
    // Constantly block on reading from client
    char buffer[BUFFER_SIZE];
    int bytesRead;
    while ((bytesRead = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
        if (!strncmp("start", buffer, 5)) {
            if (active) {
                dprintf(client_socket, "gameover\n");
            }
            dprintf(client_socket, "started\n");
            active = true;
            // send ucinewgame
            // send isready
        } else if (!strncmp("board", buffer, 5)) {
            // send d
        } else if (!strncmp("hint best", buffer, 9)) {
            // send go 500 depth 15
        } else if (!strncmp("hint all", buffer, 8)) {
            // send go perft 1
        } else if (!strncmp("moves", buffer, 5)) {
            // read second part of buffer
            // send position fen [FEN] moves [MOVE]
        } else if (!strncmp("resign", buffer, 6)) {
            dprintf(client_socket, "gameover\n");
        } else {
            dprintf(client_socket, "error command\n");
        }
        dprintf(client_socket, "> ");
    }
    printf("Client %d has disconnected.\n", client_socket);
    close(client_socket);

    return NULL;
}

Server init_server(int domain, int service,
    int protocol, unsigned long interface, int port, int backlog)
{
    Server server;
    // Define the basic parameters of the server.
    server.domain = domain;
    server.service = service;
    server.protocol = protocol;
    server.interface = interface;
    server.port = port;
    server.backlog = backlog;
    // Use the aforementioned parameters to construct the server's address.
    server.address.sin_family = domain;
    server.address.sin_port = htons(port);
    server.address.sin_addr.s_addr = htonl(interface);
    // Create a socket for the server.
    // Confirm the connection was successful.
    if ((server.socket = socket(domain, service, protocol)) < 0) {
        perror("Failed to connect socket...\n");
        exit(1);
    }
    // Attempt to bind the socket to the network.
    if ((bind(server.socket, (struct sockaddr *)&server.address, sizeof(server.address))) < 0) {
        perror("Failed to bind socket...\n");
        exit(1);
    }
    // Start listening on the network.
    if ((listen(server.socket, server.backlog)) < 0) {
        perror("Failed to start listening...\n");
        exit(1);
    }
    return server;
}
