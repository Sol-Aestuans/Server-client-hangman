/* client.c - code for client program. Do not rename this file */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#include "proj.h"

int main( int argc, char **argv) { // takes IP, port

    if(argc != 3){
        PRINT_MSG("usage error\n");
        exit(EXIT_FAILURE);
    }

    char* std_buf = (char*) calloc(8, sizeof(uint8_t)); // std_buffer, intially empty
    char* IP = argv[1]; // IP address
    u_int16_t port = atoi(argv[2]); // Port
    int client_socket_fd; // socket
    uint8_t k, n; // game variables

    if ((client_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        PRINT_MSG("socket creation error\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if ((inet_pton(AF_INET, IP, &(address.sin_addr))) < 0) { // IP string -> binary
        PRINT_MSG("inet_pton error\n");
        exit(EXIT_FAILURE);
    }

    if ((connect(client_socket_fd, (struct sockaddr*)&address, addr_len)) < 0) { // initial connection
        PRINT_MSG("connect error\n");
        exit(EXIT_FAILURE);
    }
    
    read(client_socket_fd, std_buf, 1); // initializing k
    k = std_buf[0];

    char* board = (char*) calloc(k, sizeof(char)); // board

    read(client_socket_fd, std_buf, 1); // initializing n
    n = std_buf[0];

    int keep_playing = 1;

    while (keep_playing) { // gameplay loop

        for (int i = 0; i< sizeof(std_buf); i++) {
            DEBUG_wARG("buf[%d] = %d\n", i, std_buf[i]);
        }
        
        read(client_socket_fd, std_buf, 1); // read n
        n = std_buf[0];
        DEBUG_wARG("N: %d\n", n);

        if (n == 0 || n == 255) {
            keep_playing = 0;
        }
            
        read(client_socket_fd, std_buf, k); // read board

        board[k] = '\0'; // adding null terminator

        for (int i = 0; i < k; i++) { // update the board
            board[i] = std_buf[i];
        }

        if (keep_playing) {

            PRINT_wARG("Board: %s (%d guesses remaining)\n", board, n);

            int more;
            char input[10];
            char tmp[10];

            PRINT_MSG("Enter guess: ");

            read_stdin(input, 1, &more);

            DEBUG_wARG("Read: %s\n", input);

            while (more == 1) {

                 DEBUG_MSG("Reading more...\n");
                 read_stdin(tmp, sizeof(tmp), &more);
            }

            PRINT_MSG("\n"); //prints a newline and fflush(stdout)

            send(client_socket_fd, input, 1, 0);
        } 
    }

    if (n == 0) {

        PRINT_wARG("Board: %s\n", board);
        PRINT_MSG("YOU LOST!\n");

    } else if (n == 255) {

        PRINT_wARG("Board: %s\n", board);
        PRINT_MSG("YOU WON!\n");
    }

    //free(board); // double free error? see writeup
    free(std_buf);

    close(client_socket_fd);

    return 0;
}
