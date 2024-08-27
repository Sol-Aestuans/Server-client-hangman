/* server.c - code for server program. Do not rename this file */

#include <arpa/inet.h>
#include <endian.h>
#include <stdint.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "proj.h"

#define QLEN 6 // socket queue length
#include <ctype.h> // for islower()

int main(int argc, char **argv) { // takes port, max guesses, keyword

    if (argc != 4) {
        PRINT_MSG("usage error | ./server [port] [n] [key]\n");
        return -1;
    }

    char* buf = (char*) calloc(16, sizeof(uint8_t)); // read buffer, intially empty
    int opt = 1; // for setsockopt
    u_int16_t port = atoi(argv[1]); // Port
    uint8_t n = atoi(argv[2]); // max guesses

    if (n < 0 || n > 25) {
        PRINT_MSG("usage error | 0 < n < 26\n");
        exit(EXIT_FAILURE);
    }

    char* keyword = argv[3]; // keyword
    uint8_t k = strlen(keyword);
    char* board = (char*) calloc(k, sizeof(char)); // board

    for (int i = 0; i < k; i++) { // checking key
        if (!islower(keyword[i])) {
            PRINT_MSG("usage error | key must be all lowercase letters\n");
            exit(EXIT_FAILURE);
        }
    }

    DEBUG_wARG("K: %d\n", k);

    if (k < 1 || k > 254) { // checking key again
        PRINT_MSG("usage error | 1 < k < 254\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < k; i++) { // init board
        board[i] = '-';
    }

    int server_socket_fd, connection_socket; // sockets

    struct sockaddr_in address;
    address.sin_family = AF_INET; // internet addresses
    address.sin_addr.s_addr = INADDR_ANY; // any address
    address.sin_port = htons(port);

    if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        PRINT_MSG("socket creation error\n");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) { // prevents 'address already in use' error
        PRINT_MSG("setsockopt error");
        exit(EXIT_FAILURE);
    }

    if (bind(server_socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        PRINT_MSG("bind error\n");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket_fd, QLEN) < 0) {
        PRINT_MSG("listen error\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        if ((connection_socket = accept(server_socket_fd, (struct sockaddr*) NULL, NULL)) < 0) { // initial connection
            PRINT_MSG("accept error\n");
            exit(EXIT_FAILURE);
        }

        pid_t child_pid = fork();

        if (child_pid == 0) { // child process

            send(connection_socket, (uint8_t*) &k, sizeof(k), 0); // sending board length
            send(connection_socket, (uint8_t*) &n, sizeof(n), 0); // initializing n for client

            int total_guessed = 0;

            while (n > 0 && (total_guessed < k)) { // gameplay loop    
                send(connection_socket, (uint8_t*) &n, sizeof(n), 0); // sending n
                send(connection_socket, board, k, 0); // sending board

                read(connection_socket, buf, sizeof(char)); // reading guess
                
                DEBUG_wARG("Guessed: %c\n", buf[0]);

                int correct = 0;

                for (int i = 0; i < k; i++) { // comparing the guess to the key
                    if (keyword[i] == buf[0] && board[i] == '-') {
                        DEBUG_MSG("Correct Guess!\n");
                        board[i] = keyword[i];
                        correct = 1;
                        total_guessed++;
                        DEBUG_wARG("total: %d K: %d\n", total_guessed, k);
                    }
                }

                if (correct == 0) {
                    DEBUG_MSG("Incorrect Guess!\n");
                    n--;
                }
            }
            uint8_t* foo = (uint8_t*) malloc(sizeof(uint8_t));

            if (n == 0) { // loss
                DEBUG_MSG("LOSS\n");

                foo[0] = 0;

                send(connection_socket, foo, 1, 0); // sending n = 0
                send(connection_socket, board, k, 0); // sending board final time

            } else { // Win
                DEBUG_MSG("WIN\n");

                foo[0] = 255;

                send(connection_socket, foo, sizeof(uint8_t), 0); //sending n = 0
                send(connection_socket, board, k, 0); // sending board final time 
            }

            free(board);
            free(foo);
            free(buf);

            close(server_socket_fd);
            close(connection_socket);

            return 0;

        } else if (child_pid > 0) {
            close(connection_socket);
            DEBUG_MSG("Back 2 the grind...\n");
        } else {
            PRINT_MSG("fork error\n");
            exit(EXIT_FAILURE);
        }
    }
}

