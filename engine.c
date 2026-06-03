/**
 * @Author: jcmann
 * @Date:   2026-02-03 19:27
 * @Last Modified by:   jcmann
 * @Last Modified time: 2026-02-04 16:37
 */

#include "engine.h"

int main(void) {

    char buffer[BUFFER_SIZE];
    ssize_t numBytes;
    errno = 0;

    int p1[2];  // parent writes, child reads
    int p2[2];  // child writes, parent reads
    /* So we have
        child read = p1[0]
        child write = p2[1]
        
        parent read = p2[0]
        parent write = p1[1]
    */

    if (pipe(p1) < 0) {
        perror("First pipe failed");
        exit(1);
    }
    if (pipe(p2) < 0) {
        perror("Second pipe failed");
        exit(1);
    }

    if (!fork()) {  // child process executes this
        close(p1[1]);
        close(p2[0]);
        // redirect file descriptors
        dup2(p1[0], STDIN_FILENO);
        close(p1[0]);
        dup2(p2[1], STDOUT_FILENO);
        close(p2[1]);
        execlp("stockfish", "stockfish", NULL);
        exit(1);
    } else {        // parent process executes this
        close(p1[0]);
        close(p2[1]);
        if (!read(p2[0], buffer, BUFFER_SIZE)) {
            perror("Reading from pipe");
            exit(1);
        }
    }

    FILE* fout = fdopen(p2[0], "r");
    while ((numBytes = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
        write(p1[1], buffer, numBytes);
        buffer[numBytes - 1] = '\0';

        if (!strcmp(buffer, "d")) {
            GameState* state = read_d_output(fout);
            free_game_state(state);
        } else if (!strcmp(buffer, "go perft 1")) {
            ChessMoves* moves = read_go_perft_1_output(fout);
            free_chess_moves(moves);
        } else if (!strncmp(buffer, "position", 8)) {
            // do nothing
        } else {
            numBytes = read(p2[0], buffer, BUFFER_SIZE);
            write(STDOUT_FILENO, buffer, numBytes);
        }
    }

    fclose(fout);
    close(p2[0]);
    close(p1[1]);
    pid_t pid = wait(NULL);

    return 0;
}